/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "hash.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "procarea.hpp"
#include "procarea_internal.hpp"
#include "fight.hpp"
#include "snew.hpp"
#include "utility.hpp"
#include "maximums.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <sys/stat.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace Alarmud {

using procarea_internal::ProcAreaInstance;
using procarea_internal::ProcMobKind;

namespace procarea_internal {

std::vector<ProcAreaInstance> g_instances;
int g_next_instance_id = 1;

[[nodiscard]] ProcAreaInstance* find_instance(int instance_id) {
	for(ProcAreaInstance& inst : g_instances) {
		if(inst.id == instance_id) {
			return &inst;
		}
	}
	return nullptr;
}

[[nodiscard]] ProcAreaInstance* find_instance_by_vnum(long vnum) {
	const long instance_id = procarea_vnum_to_instance(vnum);
	if(instance_id < 0) {
		return nullptr;
	}
	return find_instance(static_cast<int>(instance_id));
}

int assign_zone(long vnum) {
	if(top_of_zone_table < 0) {
		return 0;
	}

	int zone = 0;
	for(; vnum > zone_table[zone].top && zone <= top_of_zone_table; ++zone) {
		;
	}
	if(zone > top_of_zone_table) {
		zone = top_of_zone_table;
	}
	return zone;
}
static struct reset_com* procarea_empty_zone_reset() {
	struct reset_com* cmd = nullptr;
	CREATE(cmd, struct reset_com, 1);
	cmd->command = 'S';
	return cmd;
}

static void procarea_init_boot_zone(int idx, int num, const char* name, int top, int lifespan,
									int reset_mode) {
	struct zone_data* z = &zone_table[idx];
	z->num = num;
	z->name = strdup(name);
	z->top = top;
	z->lifespan = lifespan;
	z->reset_mode = reset_mode;
	z->bottom = -1;
	z->age = 0;
	z->start = 0;
	z->cmd = procarea_empty_zone_reset();
	memset(z->races, 0, sizeof(z->races));
	z->CurrTotGold = 0.0f;
	z->LastTotGold = 0.0f;
	z->CurrToPCGold = 0.0f;
	z->LastToPCGold = 0.0f;
}

static void procarea_boot_zone_impl() {
	if(zone_table == nullptr || top_of_zone_table < 0) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: zone table missing");
		return;
	}

	for(int i = 0; i <= top_of_zone_table; ++i) {
		if(zone_table[i].num == PROCAREA_ZONE_NUM && zone_table[i].top == PROCAREA_ZONE_TOP) {
			mudlog(LOG_CHECK, "procarea_boot_zone: zone #%d already installed", PROCAREA_ZONE_NUM);
			return;
		}
	}

	int buffer_idx = -1;
	for(int i = top_of_zone_table; i >= 0; --i) {
		if(zone_table[i].num == PROCAREA_LAST_AREA_ZONE_NUM) {
			buffer_idx = i;
			break;
		}
	}
	if(buffer_idx < 0) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: zone #%d (last_area) not found",
			   PROCAREA_LAST_AREA_ZONE_NUM);
		return;
	}
	if(buffer_idx != top_of_zone_table) {
		mudlog(LOG_SYSERR,
			   "procarea_boot_zone: last_area at index %d, expected last index %d", buffer_idx,
			   top_of_zone_table);
		return;
	}
	if(zone_table[buffer_idx].top < PROCAREA_ZONE_BUFFER_TOP) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: last_area top %d too low (need >= %d)",
			   zone_table[buffer_idx].top, PROCAREA_ZONE_BUFFER_TOP);
		return;
	}

	zone_table[buffer_idx].top = PROCAREA_ZONE_BUFFER_TOP;

	const int antro_idx = buffer_idx + 1;
	const int pin_idx = buffer_idx + 2;
	const int new_capacity = pin_idx + 10;
	zone_table = static_cast<struct zone_data*>(
		realloc(zone_table, static_cast<size_t>(new_capacity) * sizeof(struct zone_data)));
	if(zone_table == nullptr) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: realloc failed");
		return;
	}

	procarea_init_boot_zone(antro_idx, PROCAREA_ZONE_NUM, "Dimensione Effimera", PROCAREA_ZONE_TOP, 60,
							ZONE_ASTRAL);
	procarea_init_boot_zone(pin_idx, PROCAREA_LAST_AREA_ZONE_NUM, "last_area", PROCAREA_ZONE_PIN, 60,
							ZONE_NEVER);

	top_of_zone_table = pin_idx;

	mudlog(LOG_CHECK,
		   "procarea_boot_zone: #%d buffer top=%d, #%d antro %ld-%d (astral), #%d pin=%d",
		   PROCAREA_LAST_AREA_ZONE_NUM, PROCAREA_ZONE_BUFFER_TOP, PROCAREA_ZONE_NUM,
		   PROCAREA_VNUM_BASE, PROCAREA_ZONE_TOP, PROCAREA_LAST_AREA_ZONE_NUM, PROCAREA_ZONE_PIN);
}
static void procarea_clear_room_contents(struct room_data* rp) {
	if(rp == nullptr) {
		return;
	}

	while(rp->people != nullptr) {
		struct char_data* ch = rp->people;
		char_from_room(ch);
		if(IS_PC(ch)) {
			char_to_room(ch, 3001);
		}
		else {
			extract_char(ch);
		}
	}

	while(rp->contents != nullptr) {
		struct obj_data* obj = rp->contents;
		obj_from_room(obj);
		extract_obj(obj);
	}
}

static void procarea_destroy_room(long vnum) {
	struct room_data* rp = real_roomp(vnum);
	if(rp == nullptr) {
		return;
	}

	procarea_clear_room_contents(rp);
	cleanout_room(rp);
#if HASH
	hash_remove(&Alarmud::room_db, vnum);
	free(rp);
#else
	room_remove(Alarmud::room_db, vnum);
#endif
}

void destroy_rooms(const std::vector<long>& room_vnums) {
	for(long vnum : room_vnums) {
		procarea_destroy_room(vnum);
	}
}

} // namespace procarea_internal

namespace {

static void procarea_capture_pc_reentry_on_death(char_data* ch);
static void procarea_restore_pc_reentries(const std::vector<char_data*>& group);

struct ProcFountainVeil {
	bool spirit_dismissed = false;
	time_t spirit_dismissed_at = 0;
	bool active = false;
	time_t opened_at = 0;
	time_t expires_at = 0;
};

static ProcFountainVeil g_fountain_veil {};
static constexpr int kFountainVeilLifetimeSec = 180;

struct ProcSoloVortex {
	bool active = false;
	std::string opener;
	time_t opened_at = 0;
};

static ProcSoloVortex g_solo_vortex {};
/** Scade se nessuno entra; scompare subito all'ingresso. */
static constexpr int kSoloVortexLifetimeSec = 45;


[[nodiscard]] static char_data* procarea_group_leader(char_data* ch);

/** Sentiero solitario: nessun compagno di gruppo (AFF_GROUP) in piazza con te. */
[[nodiscard]] static bool procarea_solo_entry_eligible(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		return true;
	}
	char_data* leader = procarea_group_leader(ch);
	if(leader == nullptr) {
		return true;
	}
	const long room = ch->in_room;
	if(leader != ch && leader->in_room == room) {
		return false;
	}
	for(follow_type* fol = leader->followers; fol != nullptr; fol = fol->next) {
		char_data* member = fol->follower;
		if(member != nullptr && member != ch && IS_PC(member) &&
		   IS_AFFECTED(member, AFF_GROUP) && member->in_room == room) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static const char* procarea_solo_entry_reject_reason(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return "non puoi attraversare il vortice adesso.";
	}
	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		return nullptr;
	}
	char_data* leader = procarea_group_leader(ch);
	if(leader == nullptr) {
		return nullptr;
	}
	const long room = ch->in_room;
	if(leader != ch && leader->in_room == room) {
		return "il capogruppo e' ancora in piazza con te.";
	}
	for(follow_type* fol = leader->followers; fol != nullptr; fol = fol->next) {
		char_data* member = fol->follower;
		if(member != nullptr && member != ch && IS_PC(member) &&
		   IS_AFFECTED(member, AFF_GROUP) && member->in_room == room) {
			return "un compagno di gruppo e' ancora in piazza con te.";
		}
	}
	return nullptr;
}

[[nodiscard]] static bool procarea_resolve_solo_entry(char_data* ch, float& eq_index,
													  int& max_level) {
	eq_index = 0.0f;
	max_level = 0;
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(GetMaxLevel(ch) < PROCAREA_MIN_LEVEL) {
		return false;
	}
	eq_index = GetCharBonusIndex(ch);
	max_level = std::min(GetMaxLevel(ch), PROCAREA_PC_MAX_LEVEL);
	return true;
}


[[nodiscard]] static char_data* procarea_group_leader(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return ch;
	}
	char_data* leader = ch;
	while(leader->master != nullptr && IS_PC(leader->master) &&
		  IS_AFFECTED(leader->master, AFF_GROUP)) {
		leader = leader->master;
	}
	return leader;
}

[[nodiscard]] static bool procarea_party_member_ready(char_data* member, long room) {
	return member != nullptr && IS_PC(member) && member->in_room == room &&
		   GET_POS(member) >= POSITION_STANDING && member->specials.fighting == nullptr;
}

static void procarea_party_add_unique(std::vector<char_data*>& party, char_data* member,
									  long room) {
	if(!procarea_party_member_ready(member, room)) {
		return;
	}
	if(std::find(party.begin(), party.end(), member) != party.end()) {
		return;
	}
	party.push_back(member);
}

[[nodiscard]] static std::vector<char_data*> procarea_party_in_room(char_data* ch) {
	std::vector<char_data*> party;
	if(ch == nullptr || !IS_PC(ch)) {
		return party;
	}
	const long room = ch->in_room;
	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		procarea_party_add_unique(party, ch, room);
		for(follow_type* fol = ch->followers; fol != nullptr; fol = fol->next) {
			procarea_party_add_unique(party, fol->follower, room);
		}
		return party;
	}

	char_data* leader = procarea_group_leader(ch);
	procarea_party_add_unique(party, leader, room);
	for(follow_type* fol = leader->followers; fol != nullptr; fol = fol->next) {
		char_data* member = fol->follower;
		if(member != nullptr && IS_PC(member) && IS_AFFECTED(member, AFF_GROUP)) {
			procarea_party_add_unique(party, member, room);
		}
	}
	return party;
}

[[nodiscard]] static float procarea_group_equipment_index(
	const std::vector<char_data*>& group) {
	if(group.empty()) {
		return 0.0f;
	}
	float sum = 0.0f;
	for(char_data* member : group) {
		sum += GetCharBonusIndex(member);
	}
	return sum / static_cast<float>(group.size());
}

[[nodiscard]] static int procarea_group_max_level(const std::vector<char_data*>& group) {
	int max_level = 0;
	for(char_data* member : group) {
		if(member == nullptr) {
			continue;
		}
		max_level = std::max(max_level,
							 std::min(GetMaxLevel(member), PROCAREA_PC_MAX_LEVEL));
	}
	return max_level;
}


[[nodiscard]] static bool procarea_group_can_enter(const std::vector<char_data*>& group) {
	if(group.empty()) {
		return false;
	}
	for(char_data* member : group) {
		if(GetMaxLevel(member) < PROCAREA_MIN_LEVEL) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static bool procarea_resolve_entry(char_data* ch, float& group_eq_index) {
	group_eq_index = 0.0f;
	const std::vector<char_data*> party = procarea_party_in_room(ch);
	if(!procarea_group_can_enter(party)) {
		return false;
	}
	group_eq_index = procarea_group_equipment_index(party);
	return true;
}







static void procarea_destroy_instance(int instance_id) {
	ProcAreaInstance* inst = procarea_internal::find_instance(instance_id);
	if(inst == nullptr) {
		return;
	}

	const std::vector<long> rooms = inst->room_vnums;
	procarea_internal::clear_world_links(*inst);
	procarea_internal::destroy_rooms(rooms);

	const auto it = std::find_if(procarea_internal::g_instances.begin(), procarea_internal::g_instances.end(),
								 [instance_id](const ProcAreaInstance& candidate) {
									 return candidate.id == instance_id;
								 });
	if(it != procarea_internal::g_instances.end()) {
		procarea_internal::g_instances.erase(it);
	}

	mudlog(LOG_CHECK, "procarea: destroyed instance %d", instance_id);
}

static bool procarea_instance_has_players(const ProcAreaInstance& inst) {
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(struct char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(IS_PC(ch)) {
				return true;
			}
		}
	}
	return false;
}

static void procarea_touch_instance(int instance_id) {
	ProcAreaInstance* inst = procarea_internal::find_instance(instance_id);
	if(inst != nullptr) {
		inst->last_activity = time(nullptr);
	}
}

[[nodiscard]] static const char* procarea_instance_owner_key(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return nullptr;
	}
	if(ch->master != nullptr && IS_PC(ch->master)) {
		return GET_NAME(ch->master);
	}
	return GET_NAME(ch);
}

[[nodiscard]] static bool procarea_instance_has_member(const ProcAreaInstance& inst,
													   const char* name) {
	if(name == nullptr || *name == '\0') {
		return false;
	}
	for(const std::string& member : inst.member_names) {
		if(member == name) {
			return true;
		}
	}
	return false;
}

static void procarea_record_instance_members(ProcAreaInstance& inst,
											 const std::vector<char_data*>& group) {
	for(char_data* member : group) {
		if(member == nullptr || !IS_PC(member)) {
			continue;
		}
		const char* name = GET_NAME(member);
		if(name == nullptr || *name == '\0') {
			continue;
		}
		if(!procarea_instance_has_member(inst, name)) {
			inst.member_names.emplace_back(name);
		}
	}
}

[[nodiscard]] static procarea_internal::ProcAreaInstance* procarea_find_instance_for_ch(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return nullptr;
	}
	const char* const self = GET_NAME(ch);
	const char* const owner_key = procarea_instance_owner_key(ch);
	for(ProcAreaInstance& inst : procarea_internal::g_instances) {
		if(owner_key != nullptr && inst.owner_name == owner_key) {
			return &inst;
		}
		if(procarea_instance_has_member(inst, self)) {
			return &inst;
		}
	}
	if(IS_AFFECTED(ch, AFF_GROUP)) {
		char_data* leader = procarea_group_leader(ch);
		const char* leader_name = GET_NAME(leader);
		if(leader_name != nullptr && *leader_name != '\0') {
			for(ProcAreaInstance& inst : procarea_internal::g_instances) {
				if(inst.owner_name == leader_name) {
					return &inst;
				}
			}
		}
	}
	return nullptr;
}


static void procarea_teleport_char(struct char_data* ch, long dest, bool announce_departure) {
	if(ch == nullptr || real_roomp(dest) == nullptr) {
		return;
	}

	if(announce_departure) {
		act("$n scompare in un vortice di nebbia.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	}

	if(ch->specials.fighting) {
		stop_fighting(ch);
	}

	char_from_room(ch);
	char_to_room(ch, dest);
	act("$n emerge dalla nebbia.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	do_look(ch, "", CMD_LOOK);
}

static void procarea_teleport_solo(char_data* ch, long dest) {
	if(ch == nullptr || real_roomp(dest) == nullptr) {
		return;
	}
	act("$n viene risucchiat$b da un vortice effimero sopra la fontana.", TRUE, ch, nullptr, nullptr,
		TO_ROOM);
	procarea_teleport_char(ch, dest, false);
}

static void procarea_teleport_party(const std::vector<char_data*>& party, long dest) {
	for(size_t i = 0; i < party.size(); ++i) {
		procarea_teleport_char(party[i], dest, i == 0);
	}
}

static void procarea_teleport_group(char_data* ch, long dest) {
	procarea_teleport_party(procarea_party_in_room(ch), dest);
}

static void procarea_send_fountain_plaza(const char* msg) {
	struct room_data* plaza = real_roomp(PROCAREA_FOUNTAIN_ROOM);
	if(plaza == nullptr || msg == nullptr) {
		return;
	}
	for(char_data* person = plaza->people; person != nullptr; person = person->next_in_room) {
		if(IS_PC(person) && person->desc != nullptr) {
			send_to_char(msg, person);
		}
	}
}

static void procarea_restore_fountain_spirit(bool announce) {
	if(!g_fountain_veil.spirit_dismissed) {
		return;
	}
	g_fountain_veil.spirit_dismissed = false;
	g_fountain_veil.spirit_dismissed_at = 0;

	if(!announce) {
		return;
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Un filo di luce livida ricade sul bordo della fontana:\n\r"
		"lo spirito della $c0014Dea DarkStar$c0010 riprende a vegliare sul bacino,\n\r"
		"muto ed immobile.$c0007\n\r\n\r");
}

static void procarea_dissolve_fountain_veil(bool announce) {
	if(!g_fountain_veil.active) {
		return;
	}
	g_fountain_veil.active = false;
	g_fountain_veil.opened_at = 0;
	g_fountain_veil.expires_at = 0;

	if(announce) {
		procarea_send_fountain_plaza(
			"\n\r$c0010Un sibilo lontano attraversa la piazza:\n\r"
			"la bruma viene risucchiata verso il centro, come marea al rovescio.$c0007\n\r");
		procarea_send_fountain_plaza(
			"$c0010Il vortice $c0014implode$c0010 in un battito\n\r"
			"d'ali d'acqua silenzioso.$c0007\n\r");
		procarea_send_fountain_plaza(
			"$c0010La Fontana della Vita ricompare al suo posto,\n\r"
			"lucida ed immobile... come se la nebbia non fosse mai passata.$c0007\n\r");
	}

	procarea_restore_fountain_spirit(announce);
}

static void procarea_dismiss_fountain_spirit(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' oltre il velo del mondo.\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi disturbare la fontana mentre combatti.\n\r", ch);
		return;
	}

	if(g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"Non c'e' piu' nulla da strappare al bacino:\n\r"
			"lo spirito della Dea e' gia' lontano.\n\r",
			ch);
		return;
	}

	if(g_fountain_veil.active) {
		send_to_char("La nebbia turbinante rende impossibile raggiungere lo spirito.\n\r", ch);
		return;
	}

	const time_t now = time(nullptr);
	g_fountain_veil.spirit_dismissed = true;
	g_fountain_veil.spirit_dismissed_at = now;

	send_to_char(
		"Ti aggrappi al bordo scivoloso e $c0014tira$c0007:\n\r"
		"l'acqua si incupisce, come se qualcuno respirasse sotto la pietra.\n\r",
		ch);
	send_to_char(
		"Una figura eterea si stacca dalla schiuma...\n\r"
		"$c0014DarkStar Luce Oscura$c0007 ti fissa un istante,\n\r"
		"poi viene risucchiata verso l'alto.\n\r",
		ch);
	act("$n tira con forza la Fontana della Vita.", TRUE, ch, nullptr, nullptr, TO_ROOM);

	procarea_send_fountain_plaza(
		"\n\r$c0010La Fontana della Vita vibra:\n\r"
		"un'aura di $c0014luce oscura$c0010 si stacca dal centro del bacino,\n\r"
		"sospesa tra cielo ed acqua.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010Per un attimo la $c0014Dea DarkStar$c0010 sembra volersi opporre...\n\r"
		"poi lo spirito svanisce nel cielo di Myst,\n\r"
		"lasciando l'acqua limpida e muta.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0015Il presidio divino e' caduto.$c0010\n\r"
		"Ora la fontana obbedisce solo a chi osa spingerla.$c0007\n\r");
}

static void procarea_invoke_fountain_veil(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' oltre il velo del mondo.\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi convocare la nebbia mentre combatti.\n\r", ch);
		return;
	}

	if(g_solo_vortex.active) {
		send_to_char(
			"Un vortice solitario turbinando sopra l'acqua\n\r"
			"impedisce alla nebbia di radunarsi.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"L'acqua resiste, ancora protetta da un residuo divino.\n\r"
			"Allontana prima lo spirito della Dea $c0014tirando via lafontana$c0007.\n\r",
			ch);
		return;
	}

	const time_t now = time(nullptr);
	if(g_fountain_veil.active) {
		g_fountain_veil.expires_at = now + kFountainVeilLifetimeSec;
		send_to_char(
			"La nebbia risponde al tuo tocco e si infittisce,\n\r"
			"turbinando piu' bassa intorno al bacino.\n\r",
			ch);
		act("$n sfiora ancora la fontana: il velo di bruma si addensa.", TRUE, ch, nullptr, nullptr,
			TO_ROOM);
		return;
	}

	g_fountain_veil.active = true;
	g_fountain_veil.opened_at = now;
	g_fountain_veil.expires_at = now + kFountainVeilLifetimeSec;

	send_to_char(
		"Appoggi le mani sulla pietra fredda:\n\r"
		"sotto l'acqua qualcosa $c0014trema$c0007,\n\r"
		"come un respiro trattenuto da secoli.\n\r",
		ch);
	send_to_char(
		"Un rantolo sommerso risponde alla tua spinta...\n\r"
		"e il mondo sembra inclinarsi verso il basso.\n\r",
		ch);
	act("$n preme contro la Fontana della Vita.", TRUE, ch, nullptr, nullptr, TO_ROOM);

	procarea_send_fountain_plaza(
		"\n\r$c0010Gocce si staccano dal bordo della fontana\n\r"
		"e svaniscono in vapore prima di toccare il selciato.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010$c0014Un velo di nebbia argentata$c0010 si alza dal bacino,\n\r"
		"lambisce le statue e ammorbidisce i contorni dei palazzi.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010Per un istante la fontana sembra $c0014dissolversi$c0010 nel nulla;\n\r"
		"al suo posto danza un turbine di bruma silente,\n\r"
		"largo quanto la piazza.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0015I rumori di Myst si affievoliscono.$c0010\n\r"
		"Resta solo un sussurro d'acqua\n\r"
		"ed un gelido invito ad entrare nella $c0014nebbia$c0010.$c0007\n\r");
}

static void procarea_clear_solo_vortex(bool announce) {
	if(!g_solo_vortex.active) {
		return;
	}
	g_solo_vortex.active = false;
	g_solo_vortex.opener.clear();
	g_solo_vortex.opened_at = 0;
	if(announce) {
		procarea_send_fountain_plaza(
			"\n\r$c0010Il vortice solitario si spegne:\n\r"
			"resta solo il riflesso della fontana.$c0007\n\r");
	}
}

static void procarea_invoke_solo_vortex(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' oltre il velo del mondo.\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi sfiorare la fontana mentre combatti.\n\r", ch);
		return;
	}

	if(ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return;
	}

	if(!procarea_solo_entry_eligible(ch)) {
		const char* reason = procarea_solo_entry_reject_reason(ch);
		if(reason != nullptr) {
			std::ostringstream os;
			os << "Il sentiero solitario si rifiuta: " << reason << "\n\r";
			send_to_char(os.str().c_str(), ch);
		} else {
			send_to_char("Il sentiero solitario si rifiuta: non puoi entrare adesso.\n\r", ch);
		}
		return;
	}

	if(g_fountain_veil.active) {
		send_to_char(
			"Il velo di nebbia di gruppo avvolge la fontana.\n\r"
			"Attendi che si dissolva, oppure $c0014entra nella nebbia$c0007.\n\r",
			ch);
		return;
	}

	ProcAreaInstance* existing = procarea_find_instance_for_ch(ch);
	if(existing != nullptr && !existing->solo_mode) {
		send_to_char(
			"Hai gia' una Dimensione Effimera di gruppo.\n\r"
			"Usa $c0014enter nebbia$c0007 per rientrare.\n\r",
			ch);
		return;
	}

	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}

	g_solo_vortex.active = true;
	g_solo_vortex.opener = name;
	g_solo_vortex.opened_at = time(nullptr);

	send_to_char(
		"Appoggi le dita sull'acqua gelida:\n\r"
		"la fontana risponde solo a te, come ad un nome pronunciato a voce bassa.\n\r",
		ch);
	send_to_char(
		"Sul bacino si apre un $c0014vortice stretto$c0007, effimero e silente.\n\r"
		"Una voce rimbomba nella tua testa,\n\r"
		"ti sta incitando: $c0014entra nel vortice$c0007.\n\r",
		ch);
	act("$n sfiora la Fontana della Vita: un vortice silente le cresce sopra.", TRUE, ch, nullptr,
		nullptr, TO_ROOM);
}

static void procarea_enter_via_solo_vortex(struct char_data* ch) {
	if(!IS_PC(ch)) {
		return;
	}

	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' dentro una Dimensione Effimera.\n\r", ch);
		return;
	}

	if(ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		send_to_char(
			"Il vortice solitario si apre solo davanti alla Fontana della Vita.\n\r",
			ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi entrare nel vortice mentre combatti.\n\r", ch);
		return;
	}

	if(!procarea_solo_entry_eligible(ch)) {
		const char* reason = procarea_solo_entry_reject_reason(ch);
		if(reason != nullptr) {
			std::ostringstream os;
			os << "Il vortice si chiude: " << reason << "\n\r";
			send_to_char(os.str().c_str(), ch);
		} else {
			send_to_char("Il vortice si chiude: non puoi entrare adesso.\n\r", ch);
		}
		procarea_clear_solo_vortex(false);
		return;
	}

	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}

	ProcAreaInstance* existing = procarea_find_instance_for_ch(ch);
	if(existing != nullptr) {
		if(!existing->solo_mode) {
			send_to_char(
				"La nebbia di gruppo ti lega a un'altra dimensione.\n\r"
				"$c0014Entra nella nebbia$c0007 per tornarci.\n\r",
				ch);
			return;
		}
		if(existing->entrance_vnum <= 0 || real_roomp(existing->entrance_vnum) == nullptr) {
			send_to_char("La tua Dimensione Solitaria non risponde piu'.\n\r", ch);
			return;
		}
		if(!g_solo_vortex.active || g_solo_vortex.opener != name) {
			send_to_char(
				"Prima devi richiamare il vortice, ti basta $c0014toccare la fontana$c0007.\n\r",
				ch);
			return;
		}
		procarea_record_instance_members(*existing, { ch });
		send_to_char(
			"Il vortice ti riconosce e ti risucchia di nuovo nel sentiero solitario.\n\r",
			ch);
		procarea_clear_solo_vortex(true);
		procarea_teleport_solo(ch, existing->entrance_vnum);
		procarea_restore_pc_reentries({ ch });
		procarea_touch_instance(existing->id);
		return;
	}

	if(!g_solo_vortex.active || g_solo_vortex.opener != name) {
		send_to_char(
			"Non c'e' alcun vortice che ti appartenga.\n\r"
			"$c0014Tocca la fontana$c0007 per aprirlo.\n\r",
			ch);
		return;
	}

	float eq_index = 0.0f;
	int max_level = 0;
	if(!procarea_resolve_solo_entry(ch, eq_index, max_level)) {
		send_to_char("Il vortice non riconosce la tua essenza.\n\r", ch);
		procarea_clear_solo_vortex(false);
		return;
	}

	long entrance = 0;
	const int instance_id = procarea_internal::create_instance(eq_index, max_level, PROCAREA_FOUNTAIN_ROOM,
													 entrance, name, true);
	if(instance_id < 0 || entrance <= 0) {
		send_to_char(
			"Il vortice trema e si spezza:\n\r"
			"l'aldila' rifiuta di aprirsi. Riprova tra poco.\n\r",
			ch);
		procarea_clear_solo_vortex(false);
		return;
	}

	if(ProcAreaInstance* inst = procarea_internal::find_instance(instance_id); inst != nullptr) {
		procarea_record_instance_members(*inst, { ch });
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Il vortice solitario implode in un battito\n\r"
		"e la piazza torna immobile.$c0007\n\r");

	send_to_char(
		"Un solo passo dentro l'acqua...\n\r"
		"il mondo si stringe attorno a te, come un corridoio di specchi.\n\r",
		ch);
	std::ostringstream enter_msg;
	enter_msg << "$c0010Ti attende una $c0014Dimensione Solitaria$c0007 (eq "
			  << std::fixed << std::setprecision(0) << eq_index << ").\n\r";
	send_to_char(enter_msg.str().c_str(), ch);

	procarea_clear_solo_vortex(false);
	procarea_teleport_solo(ch, entrance);
	procarea_touch_instance(instance_id);
}

static void procarea_enter_via_veil(struct char_data* ch) {
	if(!IS_PC(ch)) {
		return;
	}

	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' dentro una Dimensione Effimera.\n\r", ch);
		return;
	}

	if(ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		send_to_char(
			"Il velo si apre solo davanti alla Fontana della Vita,\n\r"
			"in Piazza delle Nebbie.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"Una luce gelida ancora custodisce il bacino:\n\r"
			"devi prima allontanare lo spirito della Dea DarkStar\n\r"
			"$c0014tirando via la fontana$c0007.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.active) {
		send_to_char(
			"Non c'e' alcun velo di nebbia da attraversare.\n\r"
			"Evocalo, ti basta spingere la fontana per farlo apparire.\n\r",
			ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi entrare nella nebbia mentre combatti.\n\r", ch);
		return;
	}

	const std::vector<char_data*> party = procarea_party_in_room(ch);
	if(party.empty()) {
		send_to_char("Non sei pront$b per attraversare la nebbia.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_GROUP)) {
		char_data* leader = procarea_group_leader(ch);
		if(leader == nullptr || leader->in_room != PROCAREA_FOUNTAIN_ROOM) {
			send_to_char(
				"Il capogruppo deve essere in Piazza delle Nebbie con il gruppo.\n\r",
				ch);
			return;
		}
	}

	ProcAreaInstance* existing = procarea_find_instance_for_ch(ch);
	if(existing != nullptr) {
		if(existing->solo_mode) {
			send_to_char(
				"Hai una Dimensione Solitaria aperta.\n\r"
				"Sfiora la fontana ed $c0014entra nel vortice$c0007 per rientrare.\n\r",
				ch);
			return;
		}
		if(existing->entrance_vnum <= 0 || real_roomp(existing->entrance_vnum) == nullptr) {
			send_to_char("La Dimensione Effimera del gruppo non risponde piu'.\n\r", ch);
			return;
		}
		procarea_record_instance_members(*existing, party);
		for(char_data* member : party) {
			send_to_char(
				"La nebbia si riapre sul sentiero gia' aperto dal tuo gruppo:\n\r"
				"ti richiama dentro la Dimensione Effimera.\n\r",
				member);
		}
		if(!party.empty()) {
			act("$n ed il suo gruppo vengono risucchiati di nuovo nel vortice di nebbia.", TRUE,
				party[0], nullptr, nullptr, TO_ROOM);
		}
		procarea_dissolve_fountain_veil(true);
		procarea_teleport_party(party, existing->entrance_vnum);
		procarea_restore_pc_reentries(party);
		procarea_touch_instance(existing->id);
		return;
	}

	float group_eq_index = 0.0f;
	if(!procarea_resolve_entry(ch, group_eq_index)) {
		send_to_char(
			"La nebbia si rifiuta di riconoscere il gruppo:\n\r"
			"qualcuno non e' pronto, o non e' adatto.\n\r",
			ch);
		return;
	}

	char_data* owner_ch = IS_AFFECTED(ch, AFF_GROUP) ? procarea_group_leader(ch) : ch;
	const char* owner = owner_ch != nullptr ? GET_NAME(owner_ch) : GET_NAME(ch);
	const int group_max_level = procarea_group_max_level(party);
	long entrance = 0;
	const int instance_id = procarea_internal::create_instance(group_eq_index, group_max_level,
													 PROCAREA_FOUNTAIN_ROOM, entrance, owner);
	if(instance_id < 0 || entrance <= 0) {
		send_to_char(
			"La bruma trema, poi si spezza:\n\r"
			"l'aldila' rifiuta di aprirsi. Riprova tra poco.\n\r",
			ch);
		return;
	}

	if(ProcAreaInstance* inst = procarea_internal::find_instance(instance_id); inst != nullptr) {
		procarea_record_instance_members(*inst, party);
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Il vortice $c0014si contrae$c0010 su se stesso\n\r"
		"con un suono di vetro sott'acqua.$c0007\n\r");

	for(char_data* member : party) {
		send_to_char(
			"Fai un passo... un solo passo...\n\r"
			"e la nebbia ti ruba il peso del corpo, la voce, persino il nome.\n\r",
			member);
		send_to_char(
			"Senti scale di ghiaccio sotto i piedi che non esistono,\n\r"
			"corridoi che si piegano come alghe al fondo di un mare dimenticato.\n\r",
			member);
		std::ostringstream enter_msg;
		enter_msg << "$c0010Oltre il velo attende una Dimensione Effimera$c0007 (eq medio gruppo "
				  << std::fixed << std::setprecision(0) << group_eq_index << ").\n\r";
		send_to_char(enter_msg.str().c_str(), member);
	}

	if(!party.empty()) {
		act("$n ed il suo gruppo vengono risucchiati nel vortice di nebbia.", TRUE, party[0], nullptr,
			nullptr, TO_ROOM);
	}

	procarea_dissolve_fountain_veil(true);
	procarea_teleport_party(party, entrance);
	procarea_touch_instance(instance_id);
}

static void procarea_tick_fountain_veil() {
	const time_t now = time(nullptr);
	if(g_solo_vortex.active && g_solo_vortex.opened_at > 0 &&
	   now > g_solo_vortex.opened_at + kSoloVortexLifetimeSec) {
		procarea_clear_solo_vortex(true);
	}
	if(g_fountain_veil.active && now > g_fountain_veil.expires_at) {
		procarea_send_fountain_plaza(
			"\n\r$c0010La nebbia si stanca:\n\r"
			"filamenti di bruma si distaccano dal vortice\n\r"
			"e svaniscono nel cielo di Myst.$c0007\n\r");
		procarea_dissolve_fountain_veil(true);
		return;
	}

	if(g_fountain_veil.spirit_dismissed && !g_fountain_veil.active &&
	   g_fountain_veil.spirit_dismissed_at > 0 &&
	   now > g_fountain_veil.spirit_dismissed_at + kFountainVeilLifetimeSec) {
		procarea_restore_fountain_spirit(true);
	}
}

static void procarea_leave_via_portal(struct char_data* ch) {
	ProcAreaInstance* inst = procarea_internal::find_instance_by_vnum(ch->in_room);
	if(inst == nullptr) {
		send_to_char("Non sei in una Dimensione Effimera.\n\r", ch);
		return;
	}

	if(!inst->exit_portal_open) {
		send_to_char(
			"Devi ancora ripulire la Dimensione Effimera: il portale di ritorno non e' aperto.\n\r",
			ch);
		return;
	}

	if(ch->in_room != inst->boss_vnum) {
		send_to_char("Devi raggiungere la sala finale dove si e' aperto il portale.\n\r", ch);
		return;
	}

	const long dest =
		inst->return_room > 0 ? inst->return_room : PROCAREA_FOUNTAIN_ROOM;
	const std::vector<char_data*> group = procarea_party_in_room(ch);
	for(char_data* member : group) {
		send_to_char(
			"Attraversi il portale: un soffio gelido\n\r"
			"ti restituisce alla Piazza delle Nebbie.\n\r",
			member);
	}
	if(dest == PROCAREA_FOUNTAIN_ROOM) {
		for(char_data* member : group) {
			send_to_char(
				"La Fontana della Vita scintilla al sole, indifferente...\n\r"
				"come se non avesse mai chiuso occhio.\n\r",
				member);
		}
	}
	procarea_teleport_group(ch, dest);
	procarea_touch_instance(inst->id);
	procarea_maybe_destroy(inst->id);
}

[[nodiscard]] static bool procarea_prayer_requests_aid(const char* prayer) {
	if(prayer == nullptr || *prayer == '\0') {
		return false;
	}
	std::array<char, MAX_INPUT_LENGTH> lowered{};
	strncpy(lowered.data(), prayer, lowered.size() - 1);
	lowered[lowered.size() - 1] = '\0';
	for(char& c : lowered) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return strstr(lowered.data(), "aiuto") != nullptr;
}

[[nodiscard]] static bool procarea_can_request_darkstar_aid(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(procarea_internal::find_instance_by_vnum(ch->in_room) != nullptr) {
		return true;
	}
	if(ch->in_room == PROCAREA_FOUNTAIN_ROOM) {
		return true;
	}
	if(ch->in_room == PROCAREA_DARKSTAR_TEMPLE && procarea_find_instance_for_ch(ch) != nullptr) {
		return true;
	}
	return false;
}

static std::unordered_map<std::string, time_t> g_procarea_darkstar_exit_until;

[[nodiscard]] static bool procarea_darkstar_exit_on_cooldown(char_data* ch, int& seconds_left) {
	seconds_left = 0;
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return false;
	}
	const auto it = g_procarea_darkstar_exit_until.find(name);
	if(it == g_procarea_darkstar_exit_until.end()) {
		return false;
	}
	const time_t now = time(nullptr);
	if(now >= it->second) {
		g_procarea_darkstar_exit_until.erase(it);
		return false;
	}
	seconds_left = static_cast<int>(it->second - now);
	return true;
}

static void procarea_mark_darkstar_exit_cooldown(const std::vector<char_data*>& group) {
	const time_t until = time(nullptr) + PROCAREA_DARKSTAR_EXIT_COOLDOWN_SEC;
	for(char_data* member : group) {
		if(member == nullptr || !IS_PC(member)) {
			continue;
		}
		const char* name = GET_NAME(member);
		if(name == nullptr || *name == '\0') {
			continue;
		}
		g_procarea_darkstar_exit_until[name] = until;
	}
}

[[nodiscard]] static bool procarea_group_darkstar_exit_blocked(const std::vector<char_data*>& group,
															  char_data*& blocked_member,
															  int& seconds_left) {
	for(char_data* member : group) {
		int left = 0;
		if(procarea_darkstar_exit_on_cooldown(member, left)) {
			blocked_member = member;
			seconds_left = left;
			return true;
		}
	}
	return false;
}

static bool procarea_darkstar_aid_impl(struct char_data* ch, const char* prayer) {
	if(ch == nullptr || prayer == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!procarea_prayer_requests_aid(prayer)) {
		return false;
	}
	if(!procarea_can_request_darkstar_aid(ch)) {
		send_to_char(
			"La preghiera si perde nel vento: DarkStar ti ascolta solo\n\r"
			"dalle Dimensioni Effimere o dalla Piazza delle Nebbie.\n\r",
			ch);
		return true;
	}
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) == nullptr) {
		send_to_char("Il tempio di DarkStar non risponde.\n\r", ch);
		return true;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi invocare DarkStar mentre combatti.\n\r", ch);
		return true;
	}

	const std::vector<char_data*> group = procarea_party_in_room(ch);
	if(group.empty()) {
		return true;
	}

	ProcAreaInstance* const in_inst = procarea_internal::find_instance_by_vnum(ch->in_room);
	if(in_inst != nullptr) {
		char_data* blocked = nullptr;
		int seconds_left = 0;
		if(procarea_group_darkstar_exit_blocked(group, blocked, seconds_left)) {
			const int minutes = (seconds_left + 59) / 60;
			if(blocked == ch) {
				std::ostringstream os;
				os << "DarkStar ti ha gia' condotto al tempio: attendi ancora "
				   << minutes << " minut" << (minutes == 1 ? "o" : "i")
				   << " prima di invocarla di nuovo per uscire.\n\r";
				send_to_char(os.str().c_str(), ch);
			} else {
				std::ostringstream os;
				os << GET_NAME(blocked)
				   << " ha gia' invocato DarkStar di recente: il gruppo deve attendere ancora "
				   << minutes << " minut" << (minutes == 1 ? "o" : "i") << ".\n\r";
				send_to_char(os.str().c_str(), ch);
			}
			return true;
		}
		for(size_t i = 0; i < group.size(); ++i) {
			send_to_char(
				"$c0014DarkStar Luce Oscura$c0007 distende un velo di nebbia argentea:\n\r"
				"la foresta del suo tempio ti accoglie.\n\r",
				group[i]);
		}
		if(!group.empty()) {
			act("$n invoca DarkStar: la nebbia li avvolge e li conduce al tempio.", TRUE, group[0],
				nullptr, nullptr, TO_ROOM);
		}
		procarea_teleport_group(ch, PROCAREA_DARKSTAR_TEMPLE);
		procarea_mark_darkstar_exit_cooldown(group);
		procarea_touch_instance(in_inst->id);
		return true;
	}

	ProcAreaInstance* const return_inst = procarea_find_instance_for_ch(ch);
	if(return_inst != nullptr &&
	   (ch->in_room == PROCAREA_FOUNTAIN_ROOM || ch->in_room == PROCAREA_DARKSTAR_TEMPLE)) {
		if(return_inst->entrance_vnum <= 0 ||
		   real_roomp(return_inst->entrance_vnum) == nullptr) {
			send_to_char("La Dimensione Effimera non risponde piu' alla preghiera.\n\r", ch);
			return true;
		}
		procarea_record_instance_members(*return_inst, group);
		for(size_t i = 0; i < group.size(); ++i) {
			send_to_char(
				"$c0014DarkStar Luce Oscura$c0007 apre un varco nella nebbia:\n\r"
				"ti richiama dentro la Dimensione Effimera.\n\r",
				group[i]);
		}
		if(!group.empty()) {
			act("$n invoca DarkStar: la nebbia li risucchia di nuovo nella Dimensione Effimera.", TRUE,
				group[0],
				nullptr, nullptr, TO_ROOM);
		}
		procarea_teleport_group(ch, return_inst->entrance_vnum);
		procarea_restore_pc_reentries(group);
		procarea_touch_instance(return_inst->id);
		return true;
	}

	for(size_t i = 0; i < group.size(); ++i) {
		send_to_char(
			"$c0014DarkStar Luce Oscura$c0007 distende un velo di nebbia argentea:\n\r"
			"la foresta del suo tempio ti accoglie.\n\r",
			group[i]);
	}
	if(!group.empty()) {
		act("$n invoca DarkStar: la nebbia li avvolge e li conduce al tempio.", TRUE, group[0],
			nullptr, nullptr, TO_ROOM);
	}
	procarea_teleport_group(ch, PROCAREA_DARKSTAR_TEMPLE);
	return true;
}

static void procarea_on_mob_death_impl(struct char_data* victim) {
	if(victim == nullptr || !IS_NPC(victim)) {
		return;
	}

	ProcAreaInstance* inst = procarea_internal::find_instance_by_vnum(victim->in_room);
	if(inst == nullptr) {
		return;
	}

	if(victim->commandp == static_cast<int>(ProcMobKind::Boss)) {
		procarea_internal::break_treasure_seals(*inst, victim);
	}

	if(inst->exit_portal_open) {
		return;
	}

	if(procarea_internal::count_mobs(*inst) != 1) {
		return;
	}

	procarea_internal::open_exit_portal(*inst);
}

static void procarea_link_rooms_one_way(long from_vnum, int dir, long to_vnum,
										const char* keyword, const char* description) {
	struct room_data* const from = real_roomp(from_vnum);
	if(from == nullptr || dir < 0 || dir > 5) {
		return;
	}

	if(!from->dir_option[dir]) {
		CREATE(from->dir_option[dir], struct room_direction_data, 1);
		from->dir_option[dir]->general_description =
			strdup(description != nullptr ? description : "");
		from->dir_option[dir]->keyword = strdup(keyword != nullptr ? keyword : "");
		from->dir_option[dir]->exit_info = 0;
		from->dir_option[dir]->key = -1;
		from->dir_option[dir]->open_cmd = -1;
	}
	from->dir_option[dir]->to_room = to_vnum;
}

static void procarea_boot_darkstar_temple_impl() {
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) != nullptr) {
		mudlog(LOG_CHECK,
			   "procarea: DarkStar temple %ld already exists (world file?); skipping boot create",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}
	if(real_roomp(PROCAREA_FOUNTAIN_ROOM) == nullptr) {
		mudlog(LOG_SYSERR,
			   "procarea: cannot create DarkStar temple: fountain square %ld missing",
			   PROCAREA_FOUNTAIN_ROOM);
		return;
	}

	allocate_room(PROCAREA_DARKSTAR_TEMPLE);
	struct room_data* rp = real_roomp(PROCAREA_DARKSTAR_TEMPLE);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "procarea: allocate_room(%ld) failed for DarkStar temple",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}

	memset(rp, 0, sizeof(*rp));
	rp->number = PROCAREA_DARKSTAR_TEMPLE;
	rp->zone = procarea_internal::assign_zone(PROCAREA_DARKSTAR_TEMPLE);
	rp->sector_type = SECT_FOREST;
	rp->room_flags = static_cast<long>(NO_MOB | PEACEFUL | INDOORS | BRIGHT);
	rp->light = 1;
	rp->name = strdup("Il Tempio di DarkStar");
	rp->description = strdup(
		"Il sentiero si perde tra cipressi e nebbia fitta; lungo il cammino\n"
		"torce d'argento fissate ai tronchi gettano una luce fredda e costante.\n"
		"La radura circolare accoglie un tempio di pietra scura e argento ornato\n"
		"da simboli della Dea DarkStar; bracieri e torce ai quattro angoli\n"
		"tengono l'oscurita' lontana anche quando la notte avvolge la foresta.\n"
		"L'incenso e l'umido della foresta si mescolano.\n"
		"Qui la Dea accoglie i caduti oltre il velo delle Dimensioni Effimere.\n"
		"Verso est, oltre gli alberi, scorge la luce della citta'.\n");

	procarea_link_rooms_one_way(PROCAREA_DARKSTAR_TEMPLE, 1, PROCAREA_FOUNTAIN_ROOM,
								"La Piazza delle Nebbie",
								"Un varco tra i tronchi lascia intravedere la piazza e la fontana.\n");

	mudlog(LOG_CHECK, "procarea: created DarkStar temple (vnum %ld) east exit to square %ld",
		   PROCAREA_DARKSTAR_TEMPLE, PROCAREA_FOUNTAIN_ROOM);
}

[[nodiscard]] static long procarea_decode_load_room(sh_int stored) {
	if(stored < -2) {
		return static_cast<long>(stored) + 65536L;
	}
	return stored;
}

[[nodiscard]] static sh_int procarea_encode_load_room(long room) {
	if(room > 32767) {
		return static_cast<sh_int>(room - 65536L);
	}
	return static_cast<sh_int>(room);
}

[[nodiscard]] static bool procarea_is_valid_saved_reentry(long room) {
	return room > 0 && room != PROCAREA_DARKSTAR_TEMPLE &&
		   procarea_internal::find_instance_by_vnum(room) == nullptr;
}

static bool procarea_read_pc_reentry_from_store(const char* name, long& load_room,
												int& start_room) {
	if(name == nullptr || *name == '\0') {
		return false;
	}
	char_file_u st {};
	bool ok = false;
#if USE_MYSQL
	ok = load_char_mysql(name, &st) != FALSE;
#endif
	if(!ok) {
		ok = load_char(name, &st) != FALSE;
	}
	if(!ok) {
		return false;
	}
	load_room = procarea_decode_load_room(st.load_room);
	start_room = st.startroom;
	return procarea_is_valid_saved_reentry(load_room);
}

static void procarea_capture_pc_reentry_on_death(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch) ||
	   procarea_internal::find_instance_by_vnum(ch->in_room) == nullptr) {
		return;
	}
	ProcAreaInstance* inst = procarea_internal::find_instance_by_vnum(ch->in_room);
	if(inst == nullptr) {
		return;
	}
	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}

	long load_room = 0;
	int start_room = ch->specials.start_room;
	if(!procarea_read_pc_reentry_from_store(name, load_room, start_room)) {
		if(procarea_is_valid_saved_reentry(inst->return_room)) {
			load_room = inst->return_room;
		} else {
			return;
		}
	}

	inst->member_saved_load_room[name] = load_room;
	inst->member_saved_start_room[name] = start_room;
}

static void procarea_restore_pc_reentry(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	ProcAreaInstance* inst = procarea_find_instance_for_ch(ch);
	if(inst == nullptr) {
		return;
	}
	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}

	const auto load_it = inst->member_saved_load_room.find(name);
	if(load_it == inst->member_saved_load_room.end()) {
		return;
	}
	const long load_room = load_it->second;
	if(!procarea_is_valid_saved_reentry(load_room)) {
		inst->member_saved_load_room.erase(load_it);
		inst->member_saved_start_room.erase(name);
		return;
	}

	int start_room = ch->specials.start_room;
	if(const auto start_it = inst->member_saved_start_room.find(name);
	   start_it != inst->member_saved_start_room.end()) {
		start_room = start_it->second;
	}
	if(ch->specials.start_room != 2) {
		ch->specials.start_room = start_room;
	}
	if(ch->desc != nullptr) {
		save_char(ch, procarea_encode_load_room(load_room), 0);
	}

	inst->member_saved_load_room.erase(load_it);
	inst->member_saved_start_room.erase(name);
}

static void procarea_restore_pc_reentries(const std::vector<char_data*>& group) {
	for(char_data* member : group) {
		procarea_restore_pc_reentry(member);
	}
}


} // namespace

long procarea_vnum_to_instance(long vnum) {
	if(vnum < PROCAREA_VNUM_BASE) {
		return -1;
	}
	const long offset = vnum - PROCAREA_VNUM_BASE;
	const long instance_id = offset / PROCAREA_SLOTS_PER_INSTANCE + 1;
	const long slot = offset % PROCAREA_SLOTS_PER_INSTANCE;
	if(instance_id < 1 || slot < 0 || slot >= PROCAREA_SLOTS_PER_INSTANCE) {
		return -1;
	}
	return instance_id;
}

bool procarea_is_generated_room(long vnum) {
	return procarea_internal::find_instance_by_vnum(vnum) != nullptr;
}

void procarea_boot_zone() {
	procarea_internal::procarea_boot_zone_impl();
}

void procarea_boot_darkstar_temple() {
	procarea_boot_darkstar_temple_impl();
}

void procarea_boot_reward_shields() {
	procarea_internal::boot_reward_shields_impl();
}

void procarea_boot_reward_gear() {
	procarea_internal::boot_reward_gear_impl();
}

long procarea_reward_gear_vnum(ProcRewardGearSlot slot, int band, int sub_variant) {
	return procarea_internal::reward_gear_vnum(slot, band, sub_variant);
}

void procarea_roll_reward_weapon(struct obj_data* obj, int template_band,
								 bool instance_has_ranger) {
	procarea_internal::roll_reward_weapon_impl(obj, template_band, instance_has_ranger);
}

void procarea_relocate_pc_corpse_to_temple(struct char_data* ch, struct obj_data* corpse) {
	if(ch == nullptr || corpse == nullptr || IS_NPC(ch)) {
		return;
	}
	if(!procarea_is_generated_room(ch->in_room)) {
		return;
	}
	procarea_capture_pc_reentry_on_death(ch);
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) == nullptr) {
		return;
	}
	obj_from_room(corpse);
	obj_to_room(corpse, PROCAREA_DARKSTAR_TEMPLE);
	send_to_char(
		"$c0014DarkStar$c0007 raccoglie il tuo corpo e lo depone nel Tempio nella foresta.\n\r",
		ch);
}

bool procarea_try_darkstar_aid(struct char_data* ch, const char* prayer) {
	return procarea_darkstar_aid_impl(ch, prayer);
}

void procarea_on_mob_death(struct char_data* victim) {
	procarea_on_mob_death_impl(victim);
}

void procarea_maybe_destroy(long instance_id) {
	ProcAreaInstance* inst = procarea_internal::find_instance(static_cast<int>(instance_id));
	if(inst == nullptr) {
		return;
	}
	if(!procarea_instance_has_players(*inst)) {
		procarea_destroy_instance(static_cast<int>(instance_id));
	}
}

void procarea_tick_cleanup() {
	const time_t now = time(nullptr);
	std::vector<int> stale;
	stale.reserve(procarea_internal::g_instances.size());

	for(const ProcAreaInstance& inst : procarea_internal::g_instances) {
		if(!procarea_instance_has_players(inst) &&
		   (now - inst.last_activity) > 600) {
			stale.push_back(inst.id);
		}
	}

	for(int instance_id : stale) {
		procarea_destroy_instance(instance_id);
	}
	procarea_tick_fountain_veil();
}

[[nodiscard]] static bool procarea_is_immortal_auditor(char_data* ch) {
	return ch != nullptr && IS_PC(ch) && GetMaxLevel(ch) >= 56;
}

[[nodiscard]] static bool procarea_ch_inside_instance(char_data* ch, const ProcAreaInstance& inst) {
	if(ch == nullptr) {
		return false;
	}
	const ProcAreaInstance* here = procarea_internal::find_instance_by_vnum(ch->in_room);
	return here != nullptr && here->id == inst.id;
}

static void procarea_append_mobs_by_kind(std::ostringstream& info, const ProcAreaInstance& inst,
										 ProcMobKind kind, const char* label) {
	bool any = false;
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(struct char_data* mob = rp->people; mob != nullptr; mob = mob->next_in_room) {
			if(!IS_NPC(mob) || IS_SET(mob->specials.act, ACT_POLYSELF)) {
				continue;
			}
			if(mob->commandp != static_cast<int>(kind)) {
				continue;
			}
			if(!any) {
				info << label;
				any = true;
			} else {
				info << ", ";
			}
			const char* name =
				(mob->player.short_descr != nullptr && mob->player.short_descr[0] != '\0') ?
					mob->player.short_descr :
					"(senza nome)";
			info << name << " [" << vnum << "]";
		}
	}
	if(!any) {
		info << label << "nessuno";
	}
	info << "\n\r";
}

static void procarea_send_dimension_immortal_info(char_data* ch, const ProcAreaInstance& inst) {
	if(ch == nullptr) {
		return;
	}

	std::ostringstream info;
	info << "$c0011--- Vista immortale ---$c0007\n\r";
	procarea_append_mobs_by_kind(info, inst, ProcMobKind::Boss, "Boss: ");
	procarea_append_mobs_by_kind(info, inst, ProcMobKind::Trap, "Trappole: ");

	if(inst.treasure_vnums.empty()) {
		info << "Tesori: nessuna stanza tesoro.\n\r";
	} else {
		info << "Stanze tesoro:\n\r";
		for(long vnum : inst.treasure_vnums) {
			struct room_data* rp = real_roomp(vnum);
			const char* room_name = (rp != nullptr && rp->name != nullptr) ? rp->name : "?";
			const bool claimed = inst.treasure_claimed.count(vnum) != 0;
			info << "  - " << room_name << " [" << vnum << "]";
			if(claimed) {
				info << " (aperto)";
			} else if(!inst.boss_key_dropped) {
				info << " (sigillo attivo)";
			} else {
				info << " (sbloccato)";
			}
			info << "\n\r";

			if(rp == nullptr) {
				continue;
			}
			bool guards = false;
			for(struct char_data* mob = rp->people; mob != nullptr; mob = mob->next_in_room) {
				if(!IS_NPC(mob) || IS_SET(mob->specials.act, ACT_POLYSELF)) {
					continue;
				}
				if(!guards) {
					info << "      guardie: ";
					guards = true;
				} else {
					info << ", ";
				}
				const char* name =
					(mob->player.short_descr != nullptr && mob->player.short_descr[0] != '\0') ?
						mob->player.short_descr :
						"(senza nome)";
				info << name;
			}
			if(guards) {
				info << "\n\r";
			}
		}
	}

	info << "Ingresso [" << inst.entrance_vnum << "] | sala finale [" << inst.boss_vnum << "]\n\r";
	send_to_char(info.str().c_str(), ch);
}

static void procarea_send_dimension_info(char_data* ch, const ProcAreaInstance& inst) {
	if(ch == nullptr) {
		return;
	}

	const bool inside = procarea_ch_inside_instance(ch, inst);
	const char* theme_label = procarea_internal::theme_set(inst.theme_id).label;
	const int mobs_left = procarea_internal::count_mobs(inst);
	const int band = std::clamp(inst.template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);

	std::ostringstream info;
	info << "$c0014=== Dimensione Effimera #" << inst.id;
	if(inst.solo_mode) {
		info << " (solitaria)";
	}
	info << " ===$c0007\n\r";
	info << "Tema: " << theme_label;
	if(inst.solo_mode) {
		info << " | eq personale ";
	} else {
		info << " | eq medio gruppo ";
	}
	info << std::fixed << std::setprecision(0) << inst.group_eq_index
		 << " | fascia " << (band + 1) << "/" << PROCAREA_TEMPLATE_BANDS << "\n\r";
	if(inst.solo_mode) {
		info << "Modalita': solitaria | esploratore: " << inst.owner_name << " | stanze: "
			 << inst.room_vnums.size() << "\n\r";
	} else {
		info << "Stanze: " << inst.room_vnums.size() << " | capogruppo: " << inst.owner_name
			 << " | membri: " << inst.member_names.size() << "\n\r";
	}

	if(!inst.member_names.empty() && !inst.solo_mode) {
		info << "Gruppo: ";
		for(std::size_t i = 0; i < inst.member_names.size(); ++i) {
			if(i > 0) {
				info << ", ";
			}
			info << inst.member_names[i];
		}
		info << "\n\r";
	}

	if(inside) {
		if(ch->in_room == inst.boss_vnum) {
			info << "Sei nella $c0014sala finale$c0007.\n\r";
		} else if(inst.entrance_vnum > 0 && ch->in_room == inst.entrance_vnum) {
			info << "Sei all'$c0014ingresso$c0007 della dimensione.\n\r";
		}
	} else if(!procarea_is_immortal_auditor(ch)) {
		info << "Sei $c0013fuori$c0007 dalla dimensione: $c0014pray darkstar aiuto$c0007 per rientrare.\n\r";
	}

	info << "Nemici rimasti: " << mobs_left;
	if(inst.exit_portal_open) {
		info << " | $c0010portale di ritorno APERTO$c0007";
	} else if(mobs_left > 0) {
		info << " | elimina tutti i nemici per aprire il portale";
	}
	info << "\n\r";

	if(inst.exit_portal_open) {
		info << "Uscita: raggiungi la sala finale ed usa $c0014enter portale$c0007"
				" (o $c0014dimensione esci$c0007).\n\r";
	} else {
		info << "Uscita: il portale si aprira' nella sala finale quando la dimensione sara' ripulita.\n\r";
	}

	if(!inst.treasure_vnums.empty()) {
		int treasures_unclaimed = 0;
		for(long vnum : inst.treasure_vnums) {
			if(inst.treasure_claimed.find(vnum) == inst.treasure_claimed.end()) {
				++treasures_unclaimed;
			}
		}
		info << "Tesori: " << inst.treasure_vnums.size() << " cumuli, "
			 << treasures_unclaimed << " ancora da aprire";
		if(inst.boss_key_dropped) {
			info << " | $c0010sigilli sbloccati$c0007";
		} else {
			info << " | sigilli attivi finche' vive il custode della dimensione";
		}
		info << " — $c0014apri cumulo$c0007 in ogni stanza tesoro.\n\r";
	}

	info << "Ripiego: $c0014pray darkstar aiuto$c0007 (tempio o rientro).\n\r";
	send_to_char(info.str().c_str(), ch);
}

ACTION_FUNC(do_antro) {
	if(!IS_PC(ch)) {
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	const std::string_view subcmd = buf.data();

	if(subcmd.empty() || procarea_internal::cmd_is(subcmd, { "entra", "enter" })) {
		send_to_char(
			"$c0014=== Dimensione Effimera ===$c0007\n\r"
			"In $c0014Piazza delle Nebbie$c0007 (Fontana della Vita):\n\r"
			"  $c0010Gruppo:$c0007\n\r"
			"  1) $c0014pull fontana$c0007 — allontana lo spirito di DarkStar\n\r"
			"  2) $c0014push fontana$c0007 — evoca il velo di nebbia\n\r"
			"  3) $c0014enter nebbia$c0007 — entra con tutto il gruppo in piazza\n\r"
			"  $c0010Solitario:$c0007 (senza compagni di gruppo in piazza)\n\r"
			"  1) $c0014touch fontana$c0007 — apre un vortice personale\n\r"
			"  2) $c0014entra nel vortice$c0007 — entra subito (il vortice scompare)\n\r"
			"Dentro o con istanza attiva:\n\r"
			"  $c0014dimensione info$c0007 — stato run, nemici, tesori, portale\n\r"
			"  $c0014pray darkstar aiuto$c0007 — tempio di rifugio o rientro\n\r"
			"Sala finale (portale aperto):\n\r"
			"  $c0014enter portale$c0007 oppure $c0014dimensione esci$c0007\n\r"
			"Tesoro: abbatti il custode della dimensione per sbloccare i cumuli, poi "
			"$c0014apri cumulo$c0007 nella dimensione effimera.\n\r",
			ch);
		return;
	}

	if(procarea_internal::cmd_is(subcmd, { "esci", "exit", "leave" })) {
		procarea_leave_via_portal(ch);
		return;
	}

	if(procarea_internal::cmd_is(subcmd, { "info", "status" })) {
		if(procarea_is_immortal_auditor(ch)) {
			if(procarea_internal::g_instances.empty()) {
				send_to_char("Nessuna Dimensione Effimera attiva.\n\r", ch);
				return;
			}
			if(procarea_internal::g_instances.size() > 1) {
				std::ostringstream header;
				header << "$c0011Dimensioni attive: " << procarea_internal::g_instances.size() << "$c0007\n\r";
				send_to_char(header.str().c_str(), ch);
			}
			for(const ProcAreaInstance& inst : procarea_internal::g_instances) {
				procarea_send_dimension_info(ch, inst);
				procarea_send_dimension_immortal_info(ch, inst);
				if(&inst != &procarea_internal::g_instances.back()) {
					send_to_char("\n\r", ch);
				}
			}
			return;
		}

		ProcAreaInstance* inst = procarea_internal::find_instance_by_vnum(ch->in_room);
		if(inst == nullptr) {
			inst = procarea_find_instance_for_ch(ch);
		}
		if(inst == nullptr) {
			send_to_char(
				"Non hai una Dimensione Effimera attiva.\n\r"
				"Gruppo: pull/push/enter nebbia — solitario: touch fontana/entra nel vortice.\n\r",
				ch);
			return;
		}
		procarea_send_dimension_info(ch, *inst);
		return;
	}

	send_to_char(
		"Uso: $c0014dimensione$c0007 (help) | $c0014dimensione info$c0007 | "
		"$c0014dimensione esci$c0007 (sala finale)\n\r"
		"Piazza gruppo: pull → push → enter nebbia | solitario: touch fontana → entra nel vortice\n\r",
		ch);
}

ROOMSPECIAL_FUNC(procarea_portal) {
	if(type != EVENT_COMMAND || cmd != CMD_ENTER) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	return procarea_try_enter_nebbia(ch, buf.data()) ? TRUE : FALSE;
}

ROOMSPECIAL_FUNC(procarea_boss_exit) {
	if(type != EVENT_COMMAND || cmd != CMD_ENTER) {
		return FALSE;
	}

	ProcAreaInstance* inst = procarea_internal::find_instance_by_vnum(room->number);
	if(inst == nullptr || !inst->exit_portal_open) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	const std::string_view target = buf.data();
	if(!target.empty() && !procarea_internal::cmd_is(target, { "portale", "commiato" })) {
		return FALSE;
	}

	procarea_leave_via_portal(ch);
	return TRUE;
}

ROOMSPECIAL_FUNC(procarea_treasure) {
	if(type != EVENT_COMMAND) {
		return FALSE;
	}
	if(cmd != CMD_OPEN && cmd != CMD_UNLOCK) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	return procarea_internal::try_open_treasure(ch, room, buf.data()) ? TRUE : FALSE;
}

ROOMSPECIAL_FUNC(procarea_t1_portal) {
	return procarea_portal(ch, cmd, arg, room, type);
}

ROOMSPECIAL_FUNC(procarea_t1_exit) {
	return procarea_boss_exit(ch, cmd, arg, room, type);
}

[[nodiscard]] static bool procarea_is_fountain_target(const char* token) {
	return token != nullptr && token[0] != '\0' &&
		   procarea_internal::cmd_is(token, { "fontana", "fountain", "vita" });
}

bool procarea_try_pull_fountain(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(!procarea_is_fountain_target(target.data())) {
		return false;
	}

	procarea_dismiss_fountain_spirit(ch);
	return true;
}

bool procarea_try_push_fountain(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(target[0] == '\0') {
		return false;
	}
	if(!procarea_is_fountain_target(target.data())) {
		return false;
	}

	procarea_invoke_fountain_veil(ch);
	return true;
}

bool procarea_try_enter_nebbia(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(target[0] == '\0' ||
	   !procarea_internal::cmd_is(target.data(), { "nebbia", "bruma", "velo" })) {
		return false;
	}

	procarea_enter_via_veil(ch);
	return true;
}

bool procarea_try_touch_solo_fountain(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(!procarea_is_fountain_target(target.data())) {
		return false;
	}

	procarea_invoke_solo_vortex(ch);
	return true;
}

bool procarea_try_enter_vortice(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(target[0] == '\0' ||
	   !procarea_internal::cmd_is(target.data(), { "vortice", "vortex", "turbinio" })) {
		return false;
	}

	procarea_enter_via_solo_vortex(ch);
	return true;
}

int procarea_mob_iVNum(const char_data* mob) {
	if(mob == nullptr || !IS_NPC(mob)) {
		return 0;
	}
	if(mob->nr >= 0 && mob->nr <= top_of_mobt) {
		return mob_index[mob->nr].iVNum;
	}
	return mob->generic;
}

} // namespace Alarmud
