/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <string_view>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "spec_mount_olymp.hpp"
#include "act.move.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "spell_parser.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

struct OlympEntrance {
	long from_room;
	int dir;
	long god_room;
	int key_vnum;
};

/**
 * Porta divina fissa <-> chiave fissa.
 * Tenere allineata a mount_olymp*.wld / .obj.
 */
constexpr std::array<OlympEntrance, 19> kOlympEntrances{{
	{13783, 0, 13785, 13874}, // Afrodite
	{13786, 0, 13787, 13875}, // Era
	{13788, 1, 13789, 13876}, // Apollo
	{13790, 1, 13791, 13877}, // Dioniso
	{13792, 2, 13793, 13878}, // Ares
	{13794, 2, 13795, 13879}, // Atena
	{13796, 3, 13797, 13880}, // Ermes
	{13875, 0, 13862, 13881}, // Artemide
	{13876, 0, 13863, 13882}, // Demetra
	{13877, 3, 13864, 13883}, // Estia
	{13878, 0, 13865, 13884}, // Iris
	{13879, 1, 13866, 13885}, // Muse
	{13880, 1, 13867, 13886}, // Nike
	{13881, 0, 13868, 13887}, // Pan
	{13882, 5, 13869, 13888}, // Asclepio
	{13883, 2, 13870, 13889}, // Ebe
	{13884, 1, 13871, 13890}, // Eolo
	{13885, 1, 13872, 13891}, // Ecate
	/* Efesto: porta aperta, niente chiave (OverflowClaim / IMMORTAL). */
	{13874, 1, kOlympZeusRoom, kOlympZeusKeyVnum}, // Zeus (ultima nel drop)
}};

struct OlympCourt {
	long room;
	int god_vnum;
	int attendant_vnum;
};

/** Sale divine -> dio -> attendant tematico (vnum 13874-13891, 13893; no Efesto). */
constexpr std::array<OlympCourt, 19> kOlympCourts{{
	{13785, 13785, 13874}, // Afrodite -> colombe
	{13787, 13787, 13875}, // Era -> pavoni
	{13789, 13789, 13876}, // Apollo -> raggi
	{13791, 13791, 13877}, // Dioniso -> menadi
	{13793, 13793, 13878}, // Ares -> spettri
	{13795, 13795, 13879}, // Atena -> civette
	{13797, 13797, 13880}, // Ermes -> messaggeri
	{13862, 13862, 13881}, // Artemide -> cerve
	{13863, 13863, 13882}, // Demetra -> driadi
	{13864, 13864, 13883}, // Estia -> fiammelle
	{13865, 13865, 13884}, // Iris -> bagliori
	{13866, 13866, 13885}, // Muse -> echi
	{13867, 13867, 13886}, // Nike -> ali
	{13868, 13868, 13887}, // Pan -> spiritelli
	{13869, 13869, 13888}, // Asclepio -> serpenti
	{13870, 13870, 13889}, // Ebe -> coppiere
	{13871, 13871, 13890}, // Eolo -> soffi
	{13872, 13872, 13891}, // Ecate -> ombre
	{13784, 13784, 13893}, // Zeus -> aquile
}};

[[nodiscard]] bool isOlympGodVnum(int vnum) noexcept {
	return std::any_of(kOlympCourts.begin(), kOlympCourts.end(),
		[vnum](const OlympCourt& c) { return c.god_vnum == vnum; });
}

[[nodiscard]] bool isOlympKeyVnum(int vnum) noexcept {
	return std::any_of(kOlympEntrances.begin(), kOlympEntrances.end(),
		[vnum](const OlympEntrance& e) { return e.key_vnum == vnum; });
}

[[nodiscard]] bool isOlympGodRoom(long room) noexcept {
	return std::any_of(kOlympEntrances.begin(), kOlympEntrances.end(),
		[room](const OlympEntrance& e) { return e.god_room == room; })
		|| room == 13873;
}

[[nodiscard]] const OlympCourt* courtForRoom(long room) noexcept {
	const auto it = std::find_if(kOlympCourts.begin(), kOlympCourts.end(),
		[room](const OlympCourt& c) { return c.room == room; });
	return it == kOlympCourts.end() ? nullptr : &(*it);
}

[[nodiscard]] room_direction_data* entranceExit(const OlympEntrance& e) {
	auto* const rp = real_roomp(e.from_room);
	return rp != nullptr ? rp->dir_option[e.dir] : nullptr;
}

[[nodiscard]] bool exitIsLockedDoor(const room_direction_data* ex) noexcept {
	return ex != nullptr
		&& IS_SET(ex->exit_info, EX_ISDOOR)
		&& IS_SET(ex->exit_info, EX_LOCKED);
}

[[nodiscard]] int countExistingObjects(int vnum) {
	const int rnum = real_object(vnum);
	if(rnum < 0) {
		return 0;
	}
	return obj_index[rnum].number;
}

[[nodiscard]] int pickOlympKeyDropVnum() {
	std::array<int, kOlympEntrances.size()> lesser{};
	std::size_t nLesser = 0;

	for(const auto& e : kOlympEntrances) {
		if(e.god_room == kOlympZeusRoom) {
			continue;
		}
		if(e.key_vnum == kOlympArtemisKeyVnum) {
			continue;
		}
		if(!exitIsLockedDoor(entranceExit(e))) {
			continue;
		}
		if(countExistingObjects(e.key_vnum) > 0) {
			continue;
		}
		lesser[nLesser++] = e.key_vnum;
	}

	if(nLesser > 0) {
		return lesser[static_cast<std::size_t>(number(0, static_cast<int>(nLesser) - 1))];
	}

	const auto* zeus = std::find_if(kOlympEntrances.begin(), kOlympEntrances.end(),
		[](const OlympEntrance& e) { return e.god_room == kOlympZeusRoom; });
	if(zeus == kOlympEntrances.end()) {
		return -1;
	}
	if(!exitIsLockedDoor(entranceExit(*zeus))) {
		return -1;
	}
	if(countExistingObjects(zeus->key_vnum) > 0) {
		return -1;
	}
	return zeus->key_vnum;
}

void destroyOlympKey(char_data* ch, obj_data* key) {
	if(key == nullptr) {
		return;
	}
	if(key->carried_by != nullptr) {
		obj_from_char(key);
	}
	else if(key->equipped_by != nullptr) {
		key = unequip_char(key->equipped_by, key->eq_pos);
	}
	if(key == nullptr) {
		return;
	}
	act("$p si spezza e diventa polvere.", FALSE, ch, key, 0, TO_CHAR);
	act("$p, nelle mani di $n, si spezza e diventa polvere.", FALSE, ch, key, 0, TO_ROOM);
	extract_obj(key);
}

[[nodiscard]] bool keywordIsSecret(const char* keyword) noexcept {
	return keyword != nullptr
		&& std::string_view{fname(keyword)} == "secret";
}

[[nodiscard]] char_data* findGodInRoom(room_data* rp, int godVnum) {
	if(rp == nullptr) {
		return nullptr;
	}
	for(char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
		if(IS_NPC(t) && GET_MOB_VNUM(t) == godVnum) {
			return t;
		}
	}
	return nullptr;
}

[[nodiscard]] int countAttendantsInRoom(room_data* rp, int attendantVnum) {
	if(rp == nullptr) {
		return 0;
	}
	int n = 0;
	for(char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
		if(IS_NPC(t) && GET_MOB_VNUM(t) == attendantVnum) {
			++n;
		}
	}
	return n;
}

void extractAttendantsInRoom(room_data* rp, int attendantVnum) {
	if(rp == nullptr) {
		return;
	}
	char_data* next = nullptr;
	for(char_data* t = rp->people; t != nullptr; t = next) {
		next = t->next_in_room;
		if(IS_NPC(t) && GET_MOB_VNUM(t) == attendantVnum) {
			extract_char(t);
		}
	}
}

void clearOlympianGodHunting(char_data* god) {
	if(god == nullptr) {
		return;
	}
	REMOVE_BIT(god->specials.act, ACT_HUNTING);
	god->specials.hunting = nullptr;
	god->hunt_dist = 0;
	god->persist = 0;
}

void maintainOlympianCourt(room_data* rp, bool allowExtract) {
	if(rp == nullptr) {
		return;
	}
	const OlympCourt* court = courtForRoom(rp->number);
	if(court == nullptr) {
		return;
	}

	char_data* god = findGodInRoom(rp, court->god_vnum);
	if(god == nullptr) {
		return;
	}

	clearOlympianGodHunting(god);

	/*
	 * In combattimento: corte congelata (ne' spawn ne' despawn).
	 * Serve se la divinita' e' stata ingaggiata sotto il 100% con i curatori
	 * gia' presenti: restano anche se viene riportata al massimo.
	 */
	if(god->specials.fighting != nullptr) {
		return;
	}

	/* Fuori combattimento a HP pieni: i curatori spariscono (solo se safe). */
	if(GET_HIT(god) >= GET_MAX_HIT(god)) {
		if(allowExtract) {
			extractAttendantsInRoom(rp, court->attendant_vnum);
		}
		return;
	}

	/* Fuori combattimento e ferita: fino a kOlympAttendantsMax. */
	const int have = countAttendantsInRoom(rp, court->attendant_vnum);
	for(int i = have; i < kOlympAttendantsMax; ++i) {
		auto* att = read_mobile(court->attendant_vnum, VIRTUAL);
		if(att == nullptr) {
			mudlog(LOG_ERROR, "OlympianCourt: cannot load attendant #%d",
				court->attendant_vnum);
			return;
		}
		char_to_room(att, rp->number);
		att->mult_att = 0.0f;
		REMOVE_BIT(att->specials.act, ACT_AGGRESSIVE | ACT_META_AGG | ACT_SENTINEL);
		if(att->master == nullptr) {
			add_follower(att, god);
		}
		act("$n prende forma al fianco di $N.", FALSE, att, 0, god, TO_ROOM);
	}
}

} // namespace

bool is_olympian_god_vnum(int vnum) noexcept {
	return isOlympGodVnum(vnum);
}

void olympian_maintain_court_room(room_data* rp) {
	maintainOlympianCourt(rp, true);
}

void olympian_spawn_court_room(room_data* rp) {
	maintainOlympianCourt(rp, false);
}

void olympian_clear_court_on_god_death(char_data* god) {
	if(god == nullptr || god->nr < 0) {
		return;
	}
	if(!isOlympGodVnum(GET_MOB_VNUM(god))) {
		return;
	}
	if(god->in_room == NOWHERE) {
		return;
	}
	auto* rp = real_roomp(god->in_room);
	if(rp == nullptr) {
		return;
	}
	const OlympCourt* court = courtForRoom(rp->number);
	if(court == nullptr || court->god_vnum != GET_MOB_VNUM(god)) {
		return;
	}
	extractAttendantsInRoom(rp, court->attendant_vnum);
}

MOBSPECIAL_FUNC(OlympianWanderer) {
	if(ch == nullptr || ch->nr < 0) {
		return FALSE;
	}
	const int vnum = mob_index[ch->nr].iVNum;
	switch(vnum) {
	case 13894:
		ch->mult_att = 5.0f;
		break;
	case 13895:
		ch->mult_att = 4.0f;
		break;
	case 13896:
		ch->mult_att = 3.0f;
		break;
	case 13897:
		ch->mult_att = 7.0f;
		break;
	case 13898:
		ch->mult_att = 4.0f;
		break;
	case 13899:
		ch->mult_att = 5.0f;
		break;
	default:
		break;
	}

	if(type == EVENT_TICK && countExistingObjects(kOlympArtemisKeyVnum) == 0) {
		constexpr std::array<int, 6> wanderers{{13894, 13895, 13896, 13897, 13898, 13899}};
		const int chosen = wanderers[static_cast<std::size_t>(number(0, 5))];
		if(vnum == chosen && countExistingObjects(kOlympArtemisKeyVnum) == 0) {
			auto* key = read_object(kOlympArtemisKeyVnum, VIRTUAL);
			if(key == nullptr) {
				mudlog(LOG_ERROR, "OlympianWanderer: cannot load key #%d",
					kOlympArtemisKeyVnum);
			}
			else {
				obj_to_char(key, ch);
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(OlympianAttendant) {
	if(ch == nullptr) {
		return FALSE;
	}
	ch->mult_att = 0.0f;
	REMOVE_BIT(ch->specials.act, ACT_AGGRESSIVE | ACT_META_AGG);

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return FALSE;
	}
	if(ch->specials.fighting != nullptr) {
		stop_fighting(ch);
	}

	auto* rp = real_roomp(ch->in_room);
	char_data* god = ch->master;
	if(god == nullptr || god->in_room != ch->in_room) {
		const OlympCourt* court = rp != nullptr ? courtForRoom(rp->number) : nullptr;
		if(court != nullptr) {
			god = findGodInRoom(rp, court->god_vnum);
			if(god != nullptr && ch->master == nullptr) {
				add_follower(ch, god);
			}
		}
	}
	if(god == nullptr) {
		extract_char(ch);
		return TRUE;
	}
	if(god->in_room != ch->in_room) {
		return FALSE;
	}

	/*
	 * Fuori fight a HP pieni: sparisci subito (evita use-after-free se
	 * maintainOlympianCourt facesse extract di tutti incluso ch).
	 */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) >= GET_MAX_HIT(god)) {
		act("$n si dissolve, il suo compito compiuto.", FALSE, ch, 0, 0, TO_ROOM);
		extract_char(ch);
		return TRUE;
	}

	/* Completa la corte se la divinita' e' ferita e non in fight. */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) < GET_MAX_HIT(god)) {
		maintainOlympianCourt(rp, true);
	}

	if(GET_HIT(god) >= GET_MAX_HIT(god)) {
		return FALSE;
	}

	const int healed = number(50, 100);
	GET_HIT(god) = std::min(GET_MAX_HIT(god), GET_HIT(god) + healed);
	act("$n riversa energia divina su $N.", FALSE, ch, 0, god, TO_ROOM);

	/* Stesso tick: se il heal ha portato il dio a full, dissolve subito. */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) >= GET_MAX_HIT(god)) {
		act("$n si dissolve, il suo compito compiuto.", FALSE, ch, 0, 0, TO_ROOM);
		extract_char(ch);
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(OlympianKeyDrop) {
	if(ch == nullptr) {
		return FALSE;
	}

	if(type == EVENT_TICK) {
		maintainOlympianCourt(real_roomp(ch->in_room), true);
		return FALSE;
	}

	if(type != EVENT_DEATH) {
		return FALSE;
	}
	if(IS_SET(ch->specials.act, ACT_IMMORTAL)) {
		return FALSE;
	}
	if(ch->in_room == NOWHERE || real_roomp(ch->in_room) == nullptr) {
		return FALSE;
	}

	olympian_clear_court_on_god_death(ch);

	const int keyVnum = pickOlympKeyDropVnum();
	if(keyVnum < 0) {
		return FALSE;
	}

	auto* key = read_object(keyVnum, VIRTUAL);
	if(key == nullptr) {
		mudlog(LOG_ERROR, "OlympianKeyDrop: cannot load key #%d", keyVnum);
		return FALSE;
	}

	obj_to_room(key, ch->in_room);
	act("Dalla salma di $n cade $p.", FALSE, ch, key, 0, TO_ROOM);
	return TRUE;
}

ROOMSPECIAL_FUNC(OlympianCourt) {
	/*
	 * Su COMMAND: solo spawn (niente extract) cosi' entrare in sala ferita
	 * popola subito la corte senza UAF se il mover e' un attendant.
	 * Extract/full maintain restano sul tick del dio.
	 */
	if(type == EVENT_COMMAND) {
		olympian_spawn_court_room(room);
	}
	return FALSE;
}

OBJSPECIAL_FUNC(OlympianKey) {
	if(type != EVENT_COMMAND || cmd != CMD_UNLOCK || ch == nullptr || obj == nullptr) {
		return FALSE;
	}

	const int keyVnum = GET_OBJ_VNUM(obj);
	if(!isOlympKeyVnum(keyVnum)) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> tipo{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	argument_interpreter(arg, tipo.data(), dir.data());

	if(tipo[0] == '\0') {
		send_to_char("Che cosa vuoi aprire?\n\r", ch);
		return TRUE;
	}

	char_data* victim = nullptr;
	obj_data* container = nullptr;
	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &container)) {
		return FALSE;
	}

	const int door = find_door(ch, tipo.data(), dir.data());
	if(door < 0) {
		/* find_door ha gia' mandato il messaggio: non far rilanciare do_unlock. */
		return TRUE;
	}

	auto* const exitp = EXIT(ch, door);
	if(exitp == nullptr
			|| !IS_SET(exitp->exit_info, EX_ISDOOR)
			|| exitp->key != keyVnum
			|| !isOlympGodRoom(exitp->to_room)) {
		return FALSE;
	}

	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		send_to_char("Ma se non e' nemmeno chiusa!\n\r", ch);
		return TRUE;
	}
	if(!has_key(ch, keyVnum)) {
		send_to_char("Non hai la chiave giusta.\n\r", ch);
		return TRUE;
	}
	if(!IS_SET(exitp->exit_info, EX_LOCKED)) {
		send_to_char("Non e' chiusa a chiave, dopotutto...\n\r", ch);
		return TRUE;
	}

	if(exitp->keyword != nullptr && !keywordIsSecret(exitp->keyword)) {
		act("$n apre $F.", FALSE, ch, 0, exitp->keyword, TO_ROOM);
	}
	else {
		act("$n sblocca la porta.", FALSE, ch, 0, 0, TO_ROOM);
	}
	send_to_char("*Click*\n\r", ch);
	raw_unlock_door(ch, exitp, door);
	destroyOlympKey(ch, obj);
	return TRUE;
}

} // namespace Alarmud
