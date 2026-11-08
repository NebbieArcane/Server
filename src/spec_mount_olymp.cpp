/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>
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
#include "procarea_rune_fragments.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"
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

[[nodiscard]] bool isOlympAttendantVnum(int vnum) noexcept {
	return std::any_of(kOlympCourts.begin(), kOlympCourts.end(),
		[vnum](const OlympCourt& c) { return c.attendant_vnum == vnum; });
}

/** Dei con roll immunita' danno (corte + Efesto). */
[[nodiscard]] bool isOlympDamageImmGodVnum(int vnum) noexcept {
	return isOlympGodVnum(vnum) || vnum == kOlympEfestoVnum;
}

constexpr unsigned kOlympDamageImmMask =
	IMM_FIRE | IMM_COLD | IMM_ELEC | IMM_ENERGY | IMM_ACID |
	IMM_BLUNT | IMM_PIERCE | IMM_SLASH;

constexpr unsigned kOlympPhysPool[] = {IMM_BLUNT, IMM_PIERCE, IMM_SLASH};
constexpr unsigned kOlympMagPool[] = {
	IMM_FIRE, IMM_COLD, IMM_ELEC, IMM_ENERGY, IMM_ACID};

/** bit0 di generic: immunita' danno gia' rollate per questo spawn. */
constexpr int kOlympImmRolledBit = 1;
/** bit1/2: rinforzo corte in fight gia' riuscito (fase 1 / fase 2). */
constexpr int kOlympAttPhase1Bit = 2;
constexpr int kOlympAttPhase2Bit = 4;
/** bits 8-15: ultimo HP% processato in fight (0xFF = non in tracking). */
constexpr int kOlympAttLastPctShift = 8;
constexpr int kOlympAttLastPctMask = 0xFF << kOlympAttLastPctShift;
constexpr int kOlympAttLastPctUnset = 0xFF;

constexpr int kOlympAttPhase1Hi = 30;
constexpr int kOlympAttPhase1Lo = 21;
constexpr int kOlympAttPhase2Hi = 20;
constexpr int kOlympAttPhase2Lo = 5;
constexpr int kOlympAttChanceBase = 10;
constexpr int kOlympAttChanceStep = 2;

[[nodiscard]] unsigned pick_random_bits(const unsigned* pool, int pool_n, int k) {
	if(k <= 0 || pool == nullptr || pool_n <= 0) {
		return 0;
	}
	if(k > pool_n) {
		k = pool_n;
	}
	unsigned chosen[8] = {};
	for(int i = 0; i < pool_n; ++i) {
		chosen[i] = pool[i];
	}
	for(int i = pool_n - 1; i > 0; --i) {
		const int j = number(0, i);
		const unsigned tmp = chosen[i];
		chosen[i] = chosen[j];
		chosen[j] = tmp;
	}
	unsigned out = 0;
	for(int i = 0; i < k; ++i) {
		out |= chosen[i];
	}
	return out;
}

void olympian_roll_damage_immunities(char_data* god) {
	if(god == nullptr || !IS_NPC(god) || god->nr < 0) {
		return;
	}
	if(!isOlympDamageImmGodVnum(mob_index[god->nr].iVNum)) {
		return;
	}
	if(god->generic & kOlympImmRolledBit) {
		return;
	}

	const int lvl = GetMaxLevel(god);
	int n_phys = 0;
	int n_mag = 0;
	long gold_lo = 20000;
	long gold_hi = 50000;
	if(lvl >= 60) {
		/* Zeus */
		n_phys = 2;
		n_mag = 4;
		gold_lo = 200000;
		gold_hi = 300000;
	}
	else if(lvl >= 54) {
		n_phys = 2;
		n_mag = 3;
		gold_lo = 85000;
		gold_hi = 100000;
	}
	else if(lvl >= 53) {
		n_phys = number(1, 2);
		n_mag = (n_phys == 2) ? 2 : 4;
		gold_lo = 55000;
		gold_hi = 80000;
	}
	else {
		/* 52 e sotto (dei minori) */
		n_phys = 1;
		n_mag = 3;
		gold_lo = 20000;
		gold_hi = 50000;
	}

	const unsigned rolled =
		pick_random_bits(kOlympPhysPool, 3, n_phys) |
		pick_random_bits(kOlympMagPool, 5, n_mag);

	/* Togli resist/imm danno statiche; tieni status/+4 da .mob.
	 * Roll = immunita'; tutto il resto del pool danno = resist. */
	god->base_immune &= ~kOlympDamageImmMask;
	god->base_M_immune &= ~kOlympDamageImmMask;
	god->base_M_immune |= rolled;
	god->base_immune |= (kOlympDamageImmMask & ~rolled);
	god->points.gold = number(static_cast<int>(gold_lo), static_cast<int>(gold_hi));
	god->generic |= kOlympImmRolledBit;
	SyncImmunities(god);
}

void olympian_maybe_mirror_tank_fireshield(char_data* god) {
	if(god == nullptr || !IS_NPC(god) || god->nr < 0) {
		return;
	}
	if(!isOlympDamageImmGodVnum(mob_index[god->nr].iVNum)) {
		return;
	}
	char_data* tank = god->specials.fighting;
	if(tank == nullptr || !IS_AFFECTED(tank, AFF_FIRESHIELD)) {
		return;
	}
	if(IS_SET(god->M_immune, IMM_FIRE)) {
		return;
	}
	if(affected_by_spell(god, SPELL_FIRESHIELD) || IS_AFFECTED(god, AFF_FIRESHIELD)) {
		return;
	}
	/* Affect silenzioso: niente emote/tell. */
	affected_type af{};
	af.type = SPELL_FIRESHIELD;
	af.duration = static_cast<sh_int>(GetMaxLevel(god));
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_FIRESHIELD;
	affect_to_char(god, &af);
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

/** Chiave Artemide: serve solo se la porta e' ancora locked e non esiste in mondo. */
[[nodiscard]] bool needsArtemisKey() {
	const auto it = std::find_if(kOlympEntrances.begin(), kOlympEntrances.end(),
		[](const OlympEntrance& e) { return e.key_vnum == kOlympArtemisKeyVnum; });
	if(it == kOlympEntrances.end()) {
		return false;
	}
	return exitIsLockedDoor(entranceExit(*it))
		&& countExistingObjects(kOlympArtemisKeyVnum) == 0;
}

[[nodiscard]] bool isOlympWandererVnum(int vnum) noexcept {
	return vnum >= 13894 && vnum <= 13899;
}

/** Altri erranti ancora vivi (su EVENT_DEATH il morente e' gia' POSITION_DEAD). */
[[nodiscard]] int countOtherLiveOlympWanderers(const char_data* self) {
	int n = 0;
	for(char_data* t = character_list; t != nullptr; t = t->next) {
		if(t == self || !IS_NPC(t) || t->nr < 0) {
			continue;
		}
		if(!isOlympWandererVnum(mob_index[t->nr].iVNum)) {
			continue;
		}
		if(GET_POS(t) <= POSITION_DEAD) {
			continue;
		}
		++n;
	}
	return n;
}

/** Drop chiave Artemide: 10%, o garantito se e' l'ultimo errante vivo. */
void tryDropArtemisKeyOnWandererDeath(char_data* mob) {
	if(mob == nullptr || mob->in_room == NOWHERE
	   || real_roomp(mob->in_room) == nullptr) {
		return;
	}
	if(!needsArtemisKey()) {
		return;
	}
	const bool last = countOtherLiveOlympWanderers(mob) == 0;
	if(!last && number(1, 100) > 10) {
		return;
	}

	auto* key = read_object(kOlympArtemisKeyVnum, VIRTUAL);
	if(key == nullptr) {
		mudlog(LOG_ERROR, "OlympianWanderer: cannot load key #%d",
			   kOlympArtemisKeyVnum);
		return;
	}
	obj_to_room(key, mob->in_room);
	act("Dalla salma di $n cade $p.", FALSE, mob, key, 0, TO_ROOM);
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

[[nodiscard]] int olympAttLastPct(const char_data* god) noexcept {
	return (god->generic & kOlympAttLastPctMask) >> kOlympAttLastPctShift;
}

void olympAttSetLastPct(char_data* god, int pct) noexcept {
	god->generic = (god->generic & ~kOlympAttLastPctMask)
		| ((pct & 0xFF) << kOlympAttLastPctShift);
}

void olympAttClearFightReinforce(char_data* god) noexcept {
	god->generic &= ~(kOlympAttPhase1Bit | kOlympAttPhase2Bit | kOlympAttLastPctMask);
	olympAttSetLastPct(god, kOlympAttLastPctUnset);
}

[[nodiscard]] int olympGodHitPct(char_data* god) noexcept {
	const int maxHit = GET_MAX_HIT(god);
	if(maxHit <= 0) {
		return 0;
	}
	return std::max(0, (100 * GET_HIT(god)) / maxHit);
}

[[nodiscard]] bool spawnOlympAttendant(room_data* rp, char_data* god,
		const OlympCourt* court) {
	if(rp == nullptr || god == nullptr || court == nullptr) {
		return false;
	}
	if(countAttendantsInRoom(rp, court->attendant_vnum) >= kOlympAttendantsMax) {
		return false;
	}
	auto* att = read_mobile(court->attendant_vnum, VIRTUAL);
	if(att == nullptr) {
		mudlog(LOG_ERROR, "OlympianCourt: cannot load attendant #%d",
			court->attendant_vnum);
		return false;
	}
	char_to_room(att, rp->number);
	att->specials.zone = god->specials.zone;
	att->lStartRoom = rp->number;
	att->mult_att = 0.0f;
	if(att->master == nullptr) {
		add_follower(att, god);
	}
	act("$n prende forma al fianco di $N.", FALSE, att, 0, god, TO_ROOM);
	return true;
}

/**
 * Tiro rinforzo a ogni HP% attraversato verso il basso.
 * Fase1 30-21, fase2 20-5: chance = 10% + 2%*(hi-pct); 1 successo/fase.
 * Corte piena: il successo consuma comunque la fase (niente refill dei morti).
 */
void tryOlympFightAttendantReinforce(room_data* rp, char_data* god,
		const OlympCourt* court) {
	if(rp == nullptr || god == nullptr || court == nullptr) {
		return;
	}

	const int curPct = olympGodHitPct(god);
	int lastPct = olympAttLastPct(god);
	if(lastPct == kOlympAttLastPctUnset) {
		olympAttSetLastPct(god, curPct);
		return;
	}
	if(curPct >= lastPct) {
		olympAttSetLastPct(god, curPct);
		return;
	}

	for(int pct = lastPct - 1; pct >= curPct; --pct) {
		int phaseBit = 0;
		int phaseHi = 0;
		if(pct >= kOlympAttPhase1Lo && pct <= kOlympAttPhase1Hi) {
			phaseBit = kOlympAttPhase1Bit;
			phaseHi = kOlympAttPhase1Hi;
		}
		else if(pct >= kOlympAttPhase2Lo && pct <= kOlympAttPhase2Hi) {
			phaseBit = kOlympAttPhase2Bit;
			phaseHi = kOlympAttPhase2Hi;
		}
		else {
			continue;
		}
		if(god->generic & phaseBit) {
			continue;
		}

		const int chance = kOlympAttChanceBase
			+ kOlympAttChanceStep * (phaseHi - pct);
		if(number(1, 100) > chance) {
			continue;
		}

		if(!spawnOlympAttendant(rp, god, court)) {
			continue;
		}
		act("$n invoca un servo della sua corte!", FALSE, god, 0, 0, TO_ROOM);
		god->generic |= phaseBit;
	}

	olympAttSetLastPct(god, curPct);
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
	 * In combattimento: niente despawn / niente refill dei morti.
	 * Solo tiri di rinforzo a fasce HP (vedi tryOlympFightAttendantReinforce).
	 */
	if(god->specials.fighting != nullptr) {
		tryOlympFightAttendantReinforce(rp, god, court);
		return;
	}

	olympAttClearFightReinforce(god);

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
		if(!spawnOlympAttendant(rp, god, court)) {
			return;
		}
	}
}

} // namespace

bool is_olympus_area_vnum(int vnum) noexcept {
	return (vnum >= kOlympZone1VnumMin && vnum <= kOlympZone1VnumMax)
		|| (vnum >= kOlympZone2VnumMin && vnum <= kOlympZone2VnumMax);
}

bool is_olympus_npc(const char_data* ch) noexcept {
	if(ch == nullptr || !IS_NPC(ch) || ch->nr < 0) {
		return false;
	}
	return is_olympus_area_vnum(mob_index[ch->nr].iVNum);
}

bool is_olympian_god_vnum(int vnum) noexcept {
	return isOlympGodVnum(vnum);
}

void olympian_ensure_god_stamina(char_data* god) {
	if(god == nullptr || !IS_NPC(god)) {
		return;
	}
	if(god->points.max_move < kOlympGodMaxMove) {
		god->points.max_move = kOlympGodMaxMove;
		god->points.move = kOlympGodMaxMove;
	}
	olympian_roll_damage_immunities(god);
	olympian_maybe_mirror_tank_fireshield(god);
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

namespace {

/**
 * Pool QuestNebbie Olimpo (achievements_data): peso 2 base, 1 per armi/scudi.
 * Tenere allineato a QuestNebbie[ACHIE_QUEST_OLIMPO].
 */
constexpr std::array<std::pair<int, int>, 23> kOlympQuestDrops{{
	{13781, 1}, /* Egida Fulmine (scudo) */
	{13782, 2}, /* veste */
	{13783, 2}, /* diadema */
	{13786, 2}, /* collana fiamme */
	{13788, 2}, /* armatura Ares */
	{13790, 1}, /* lancia Atena (arma) */
	{13791, 2}, /* sandali */
	{13792, 2}, /* brocca */
	{13794, 1}, /* Egida Arcana (scudo) */
	{13861, 1}, /* freccia (arma) */
	{13862, 1}, /* falce (arma) */
	{13863, 2}, /* anello focolare */
	{13864, 2}, /* girocollo */
	{13865, 2}, /* anello armonia */
	{13866, 2}, /* corona alloro */
	{13867, 2}, /* cintura */
	{13868, 1}, /* bastone (arma) */
	{13869, 2}, /* cestus */
	{13870, 2}, /* calzari */
	{13871, 2}, /* torcia */
	{13872, 1}, /* martello (arma) */
	{37074, 1}, /* ampolla destino - re roll hp */
	{37075, 1}, /* pergamena ricordo - pergamena per annullare effetto ampolla destino */
}};

[[nodiscard]] int pickOlympQuestDropVnum() {
	int total = 0;
	for(const auto& e : kOlympQuestDrops) {
		total += e.second;
	}
	if(total <= 0) {
		return -1;
	}
	int roll = number(1, total);
	for(const auto& e : kOlympQuestDrops) {
		roll -= e.second;
		if(roll <= 0) {
			return e.first;
		}
	}
	return kOlympQuestDrops.back().first;
}

} // namespace

void olympian_zeus_death_treasure(char_data* god) {
	if(god == nullptr || god->nr < 0) {
		return;
	}
	if(GET_MOB_VNUM(god) != kOlympZeusVnum) {
		return;
	}
	if(real_roomp(kOlympTreasureRoom) == nullptr) {
		mudlog(LOG_ERROR, "olympian_zeus_death_treasure: missing room #%d",
			   kOlympTreasureRoom);
		return;
	}

	const int vnum = pickOlympQuestDropVnum();
	if(vnum < 0) {
		return;
	}
	auto* prize = read_object(vnum, VIRTUAL);
	if(prize == nullptr) {
		mudlog(LOG_ERROR, "olympian_zeus_death_treasure: cannot load #%d", vnum);
		return;
	}
	obj_to_room(prize, kOlympTreasureRoom);
	if(god->in_room != NOWHERE && real_roomp(god->in_room) != nullptr) {
		act("Sotto la botola del trono risuona un fragore di ricchezze.",
			FALSE, god, 0, 0, TO_ROOM);
	}

	send_to_all(
		"\n\r$c0011Un tuono scuote i cieli: Zeus, Re degli Dei, e' caduto!$c0007\n\r\n\r");
}

void olympian_try_rune_fragment_drop(char_data* victim) {
	if(victim == nullptr || !IS_NPC(victim) || victim->nr < 0) {
		return;
	}
	if(!is_olympus_npc(victim)) {
		return;
	}
	const int vnum = mob_index[victim->nr].iVNum;
	if(isOlympAttendantVnum(vnum)) {
		return;
	}
	if(victim->in_room == NOWHERE || real_roomp(victim->in_room) == nullptr) {
		return;
	}

	const int lvl = GetMaxLevel(victim);
	if(lvl <= 50) {
		return;
	}
	const bool is_god = (GET_RACE(victim) == RACE_GOD);
	const int god_mult = is_god ? 2 : 1;
	const int chance_pct = std::min(100, (lvl - 50) * 10 * god_mult);
	if(number(1, 100) > chance_pct) {
		return;
	}

	int qty = (lvl - 50) * 100 * god_mult;
	qty = qty * number(90, 110) / 100;
	if(qty <= 0) {
		return;
	}

	std::vector<char_data*> pcs;
	for(char_data* ch = real_roomp(victim->in_room)->people; ch != nullptr;
		ch = ch->next_in_room) {
		if(IS_PC(ch)) {
			pcs.push_back(ch);
		}
	}
	if(pcs.empty()) {
		return;
	}

	const int share = qty / static_cast<int>(pcs.size());
	int remainder = qty % static_cast<int>(pcs.size());
	for(char_data* pc : pcs) {
		const int grant = share + (remainder > 0 ? 1 : 0);
		if(remainder > 0) {
			--remainder;
		}
		if(grant <= 0) {
			continue;
		}
		const int new_total = procarea_rune_fragments_add(pc, grant);
		std::ostringstream msg;
		msg << "\n\r$c0010Un eco divino si spezza: $c0014+" << grant
			<< " frammenti di runa$c0007 (totale " << new_total << ").\n\r";
		send_to_char(msg.str().c_str(), pc);
	}
}

MOBSPECIAL_FUNC(OlympianWanderer) {
	if(mob == nullptr || mob->nr < 0) {
		return FALSE;
	}
	const int vnum = mob_index[mob->nr].iVNum;
	switch(vnum) {
	case 13894:
		mob->mult_att = 5.0f;
		break;
	case 13895:
		mob->mult_att = 4.0f;
		break;
	case 13896:
		mob->mult_att = 3.0f;
		break;
	case 13897:
		mob->mult_att = 7.0f;
		break;
	case 13898:
		mob->mult_att = 4.0f;
		break;
	case 13899:
		mob->mult_att = 5.0f;
		break;
	default:
		break;
	}

	if(type == EVENT_DEATH) {
		tryDropArtemisKeyOnWandererDeath(mob);
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(OlympianAttendant) {
	if(mob == nullptr) {
		return FALSE;
	}
	/* Mai attaccare: zero attacchi e fuori combat (anche su COMMAND). */
	mob->mult_att = 0.0f;
	if(mob->specials.fighting != nullptr) {
		stop_fighting(mob);
	}

	if(type != EVENT_TICK || !AWAKE(mob)) {
		return FALSE;
	}

	auto* rp = real_roomp(mob->in_room);
	char_data* god = mob->master;
	if(god == nullptr || god->in_room != mob->in_room) {
		const OlympCourt* court = rp != nullptr ? courtForRoom(rp->number) : nullptr;
		if(court != nullptr) {
			god = findGodInRoom(rp, court->god_vnum);
			if(god != nullptr && mob->master == nullptr) {
				add_follower(mob, god);
			}
		}
	}
	if(god == nullptr) {
		extract_char(mob);
		return TRUE;
	}
	if(god->in_room != mob->in_room) {
		/* TRUE: non far partire AssistFriend/guardian nello stesso tick. */
		return TRUE;
	}

	/*
	 * Fuori fight a HP pieni: sparisci subito (evita use-after-free se
	 * maintainOlympianCourt facesse extract di tutti incluso mob).
	 */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) >= GET_MAX_HIT(god)) {
		act("$n si dissolve, il suo compito e' compiuto.", FALSE, mob, 0, 0, TO_ROOM);
		extract_char(mob);
		return TRUE;
	}

	/* Completa la corte se la divinita' e' ferita e non in fight. */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) < GET_MAX_HIT(god)) {
		maintainOlympianCourt(rp, true);
	}

	if(GET_HIT(god) < GET_MAX_HIT(god)) {
		const int healed = number(50, 100);
		GET_HIT(god) = std::min(GET_MAX_HIT(god), GET_HIT(god) + healed);
		act("$n riversa energia divina su $N.", FALSE, mob, 0, god, TO_ROOM);
	}

	/* Stesso tick: se il heal ha portato il dio a full, dissolve subito. */
	if(god->specials.fighting == nullptr
			&& GET_HIT(god) >= GET_MAX_HIT(god)) {
		act("$n si dissolve, il suo compito e' compiuto.", FALSE, mob, 0, 0, TO_ROOM);
		extract_char(mob);
		return TRUE;
	}

	/*
	 * Sempre TRUE sul tick: altrimenti AssistFriend (SameRace/in_group) li
	 * rimette in fight e colpiscono i PG dopo lo stop_fighting.
	 */
	return TRUE;
}

MOBSPECIAL_FUNC(OlympianKeyDrop) {
	if(mob == nullptr) {
		return FALSE;
	}

	if(type == EVENT_TICK) {
		olympian_ensure_god_stamina(mob);
		maintainOlympianCourt(real_roomp(mob->in_room), true);
		return FALSE;
	}

	if(type != EVENT_DEATH) {
		return FALSE;
	}
	if(IS_SET(mob->specials.act, ACT_IMMORTAL)) {
		return FALSE;
	}
	if(mob->in_room == NOWHERE || real_roomp(mob->in_room) == nullptr) {
		return FALSE;
	}

	olympian_clear_court_on_god_death(mob);

	const int keyVnum = pickOlympKeyDropVnum();
	if(keyVnum < 0) {
		return FALSE;
	}

	auto* key = read_object(keyVnum, VIRTUAL);
	if(key == nullptr) {
		mudlog(LOG_ERROR, "OlympianKeyDrop: cannot load key #%d", keyVnum);
		return FALSE;
	}

	obj_to_room(key, mob->in_room);
	act("Dalla salma di $n cade $p.", FALSE, mob, key, 0, TO_ROOM);
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
