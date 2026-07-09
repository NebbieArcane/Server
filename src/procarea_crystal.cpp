/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "handler.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "procarea.hpp"
#include "procarea_internal.hpp"
#include <cctype>
#include <cstring>
namespace Alarmud {

namespace {

[[nodiscard]] char_data* procarea_crystal_group_leader(char_data* ch) {
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

[[nodiscard]] bool procarea_ch_is_crystal_chooser(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		return true;
	}
	return procarea_crystal_group_leader(ch) == ch;
}

} // namespace

void procarea_send_crystal_entry_spacing(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	send_to_char("\n\r\n\r\n\r\n\r\n\r", ch);
}

void procarea_send_crystal_entrance_brief(char_data* ch) {
	(void)ch;
}

bool procarea_try_crystal_touch(char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || !IS_PC(ch)) {
		return false;
	}

	procarea_internal::ProcAreaInstance* inst =
		procarea_internal::find_instance_by_vnum(ch->in_room);
	if(inst == nullptr || ch->in_room != inst->entrance_vnum) {
		return false;
	}
	if(inst->crystal_resolved) {
		send_to_char("La sintonia e' gia' fissata: la dimensione obbedisce al cristallo scelto.\n\r",
					 ch);
		return true;
	}

	const procarea_internal::ProcCrystalTier tier = procarea_internal::parse_crystal_tier(arg);
	if(tier == procarea_internal::ProcCrystalTier::Pending) {
		if(std::strstr(arg, "cristall") != nullptr) {
			send_to_char(
				"Quale cristallo? $c0014tocca$c0007 "
				"$c0010verde$c0007, $c0012blu$c0007, $c0009rosso$c0007, $c0011arancione$c0007 o "
				"$c1013fucsia$c0007.\n\r",
				ch);
			return true;
		}
		return false;
	}

	if(!procarea_ch_is_crystal_chooser(ch)) {
		send_to_char("Solo il capogruppo puo' sintonizzare un cristallo.\n\r", ch);
		return true;
	}

	if(!procarea_internal::apply_crystal_choice(*inst, tier)) {
		send_to_char("Il cristallo non risponde al tuo tocco.\n\r", ch);
		return true;
	}

	act("$n accosta la mano a un cristallo: la nebbia obbedisce.", TRUE, ch, nullptr, nullptr,
		TO_ROOM);
	return true;
}

void procarea_tick_crystal_timeouts() {
	const time_t now = time(nullptr);
	std::vector<int> expired;
	expired.reserve(procarea_internal::g_instances.size());

	for(const procarea_internal::ProcAreaInstance& inst : procarea_internal::g_instances) {
		if(inst.crystal_resolved) {
			continue;
		}
		if(now - inst.created_at >= PROCAREA_CRYSTAL_CHOICE_TIMEOUT_SEC) {
			expired.push_back(inst.id);
		}
	}

	for(int instance_id : expired) {
		procarea_abort_pending_crystal(instance_id);
	}
}

} // namespace Alarmud
