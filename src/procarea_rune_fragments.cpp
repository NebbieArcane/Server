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
#include "procarea.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_internal.hpp"
#include "procarea_rune_fragments.hpp"
#include "reception.hpp"
#include "utility.hpp"
#include <algorithm>
#include <climits>
#include <cctype>
#include <cstring>
#include <sstream>
#include <vector>
namespace Alarmud {

namespace {

enum class RuneFragmentDropKind {
	Corridor,
	Treasure,
	Add,
	Trap,
	Boss,
};

static constexpr int kProcRuneFragDropPct[] = {
	15, /* Corridor */
	35, /* Treasure */
	25, /* Add */
	100, /* Trap */
	100, /* Boss */
};

static constexpr int kProcRuneFragQtyMult[] = {
	100, /* Corridor */
	130, /* Treasure */
	110, /* Add */
	250, /* Trap */
	400, /* Boss */
};

static constexpr int kProcRuneFragBaseFixed[PROCAREA_TEMPLATE_BANDS] = {
	3, 4, 5, 6, 7, 8, 10, 12, 14, 16,
};
static constexpr int kProcRuneFragBaseDie[PROCAREA_TEMPLATE_BANDS] = {
	4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
};
static constexpr int kProcRuneFragBaseDice[PROCAREA_TEMPLATE_BANDS] = {
	1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
};

void procarea_rune_fragments_persist(char_data* ch, int value) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}
#if USE_MYSQL
	procarea_rune_fragments_save_mysql(name, value);
#else
	(void)value;
#endif
}

[[nodiscard]] int procarea_rune_fragments_clamp(int value) {
	return std::max(0, value);
}

[[nodiscard]] bool procarea_is_treasure_room(const procarea_internal::ProcAreaInstance& inst,
											 long room_vnum) {
	return std::find(inst.treasure_vnums.begin(), inst.treasure_vnums.end(), room_vnum) !=
		   inst.treasure_vnums.end();
}

[[nodiscard]] RuneFragmentDropKind procarea_classify_fragment_drop(
	const char_data* victim, const procarea_internal::ProcAreaInstance& inst) {
	const auto kind = static_cast<procarea_internal::ProcMobKind>(victim->commandp);
	if(kind == procarea_internal::ProcMobKind::Boss) {
		return RuneFragmentDropKind::Boss;
	}
	if(kind == procarea_internal::ProcMobKind::Trap) {
		return RuneFragmentDropKind::Trap;
	}
	if(victim->master != nullptr) {
		const auto master_kind =
			static_cast<procarea_internal::ProcMobKind>(victim->master->commandp);
		if(master_kind == procarea_internal::ProcMobKind::Boss ||
		   master_kind == procarea_internal::ProcMobKind::Trap) {
			return RuneFragmentDropKind::Add;
		}
	}
	if(procarea_is_treasure_room(inst, victim->in_room)) {
		return RuneFragmentDropKind::Treasure;
	}
	return RuneFragmentDropKind::Corridor;
}

[[nodiscard]] int procarea_roll_fragment_base(int band) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	int dice_sum = 0;
	for(int d = 0; d < kProcRuneFragBaseDice[b]; ++d) {
		dice_sum += number(1, kProcRuneFragBaseDie[b]);
	}
	return kProcRuneFragBaseFixed[b] + dice_sum;
}

[[nodiscard]] int procarea_compute_fragment_amount(int band, RuneFragmentDropKind drop_kind,
												  int fatigue_tier) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int kind_idx = static_cast<int>(drop_kind);
	const int base = procarea_roll_fragment_base(b);
	const int fatigue_pct = procarea_fatigue_gold_drop_pct(fatigue_tier);
	long long qty = static_cast<long long>(base) * kProcRuneFragQtyMult[kind_idx] * fatigue_pct;
	qty /= 10000LL;
	qty = qty * number(90, 110) / 100;
	return static_cast<int>(std::clamp(qty, 1LL, static_cast<long long>(INT_MAX)));
}

[[nodiscard]] std::vector<char_data*> procarea_pcs_in_room(long room_vnum) {
	std::vector<char_data*> pcs;
	struct room_data* rp = real_roomp(room_vnum);
	if(rp == nullptr) {
		return pcs;
	}
	for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
		if(IS_PC(ch)) {
			pcs.push_back(ch);
		}
	}
	return pcs;
}

void procarea_grant_fragments_split(const std::vector<char_data*>& pcs, int total,
									RuneFragmentDropKind drop_kind) {
	if(pcs.empty() || total <= 0) {
		return;
	}

	const int share = total / static_cast<int>(pcs.size());
	int remainder = total % static_cast<int>(pcs.size());
	for(size_t i = 0; i < pcs.size(); ++i) {
		const int grant = share + (remainder > 0 ? 1 : 0);
		if(remainder > 0) {
			--remainder;
		}
		if(grant <= 0) {
			continue;
		}
		const int new_total = procarea_rune_fragments_add(pcs[i], grant);
		std::ostringstream msg;
		if(drop_kind == RuneFragmentDropKind::Boss) {
			msg << "\n\r$c0010Dal custode caduto si stacca un frammento di runa: $c0014+" << grant
				<< " frammenti$c0007 (totale " << new_total << ").\n\r";
		} else if(drop_kind == RuneFragmentDropKind::Trap) {
			msg << "\n\r$c0010La trappola lascia cadere un frammento di runa: $c0014+" << grant
				<< " frammenti$c0007 (totale " << new_total << ").\n\r";
		} else {
			msg << "$c0010Raccogli $c0014" << grant
				<< " frammenti di runa$c0007 (totale " << new_total << ").\n\r";
		}
		send_to_char(msg.str().c_str(), pcs[i]);
	}
}

[[nodiscard]] bool procarea_prayer_requests_fragment_conversion(const char* prayer) {
	if(prayer == nullptr || *prayer == '\0') {
		return false;
	}
	char lowered[MAX_INPUT_LENGTH]{};
	std::strncpy(lowered, prayer, sizeof(lowered) - 1);
	for(char& c : lowered) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return std::strstr(lowered, "converti") != nullptr ||
		   std::strstr(lowered, "frammenti") != nullptr ||
		   std::strstr(lowered, "consacra") != nullptr;
}

} // namespace

int procarea_rune_fragments_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return procarea_rune_fragments_clamp(ch->specials.procarea_rune_fragments);
}

int procarea_rune_fragments_get_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_rune_fragments_get(ch);
	}
#if USE_MYSQL
	int value = 0;
	if(procarea_rune_fragments_load_mysql(name, value)) {
		return procarea_rune_fragments_clamp(value);
	}
#endif
	return 0;
}

int procarea_rune_fragments_add(char_data* ch, int delta) {
	if(ch == nullptr || !IS_PC(ch) || delta == 0) {
		return procarea_rune_fragments_get(ch);
	}
	const long long next =
		static_cast<long long>(procarea_rune_fragments_get(ch)) + static_cast<long long>(delta);
	const int clamped = procarea_rune_fragments_clamp(
		static_cast<int>(std::clamp(next, 0LL, static_cast<long long>(INT_MAX))));
	ch->specials.procarea_rune_fragments = clamped;
	procarea_rune_fragments_persist(ch, clamped);
	return clamped;
}

void procarea_rune_fragments_set(char_data* ch, int value) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	const int clamped = procarea_rune_fragments_clamp(value);
	ch->specials.procarea_rune_fragments = clamped;
	procarea_rune_fragments_persist(ch, clamped);
}

void procarea_rune_fragments_on_mob_death(char_data* victim,
										  const procarea_internal::ProcAreaInstance& inst) {
	if(victim == nullptr || !IS_NPC(victim)) {
		return;
	}

	const RuneFragmentDropKind drop_kind = procarea_classify_fragment_drop(victim, inst);
	const int kind_idx = static_cast<int>(drop_kind);
	if(number(0, 99) >= kProcRuneFragDropPct[kind_idx]) {
		return;
	}

	const int fatigue_tier = procarea_fatigue_treasure_tier_for_instance(inst);
	const int band = std::clamp(inst.effective_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int total = procarea_compute_fragment_amount(band, drop_kind, fatigue_tier);
	const std::vector<char_data*> pcs = procarea_pcs_in_room(victim->in_room);
	procarea_grant_fragments_split(pcs, total, drop_kind);
}

bool procarea_try_convert_rune_fragments(char_data* ch, const char* prayer) {
	if(ch == nullptr || !IS_PC(ch) || prayer == nullptr) {
		return false;
	}
	if(ch->in_room != PROCAREA_DARKSTAR_TEMPLE) {
		return false;
	}
	if(!procarea_prayer_requests_fragment_conversion(prayer)) {
		return false;
	}

	const int fragments = procarea_rune_fragments_get(ch);
	if(fragments < PROCAREA_RUNE_FRAGMENTS_PER_RUNE) {
		std::ostringstream msg;
		msg << "Ti servono almeno " << PROCAREA_RUNE_FRAGMENTS_PER_RUNE
			<< " frammenti di runa per forgiarne una intera (ne hai " << fragments << ").\n\r";
		send_to_char(msg.str().c_str(), ch);
		return true;
	}

	const int runes = fragments / PROCAREA_RUNE_FRAGMENTS_PER_RUNE;
	const int spent = runes * PROCAREA_RUNE_FRAGMENTS_PER_RUNE;
	const long long new_runes =
		static_cast<long long>(GET_RUNEDEI(ch)) + static_cast<long long>(runes);
	if(new_runes > USHRT_MAX) {
		send_to_char("Non puoi portare altre rune degli Dei tatuate sul corpo.\n\r", ch);
		return true;
	}

	const int fragments_left = procarea_rune_fragments_add(ch, -spent);
	GET_RUNEDEI(ch) = static_cast<ush_int>(new_runes);

	std::ostringstream msg;
	msg << "$c0014DarkStar$c0007 accoglie i tuoi frammenti: ";
	if(runes == 1) {
		msg << "una runa degli Dei brucia sulla tua pelle";
	} else {
		msg << runes << " rune degli Dei bruciano sulla tua pelle";
	}
	msg << " (totale " << static_cast<int>(GET_RUNEDEI(ch)) << "). ";
	msg << "Frammenti rimasti: " << fragments_left << ".\n\r";
	send_to_char(msg.str().c_str(), ch);
	act("$n offre frammenti di runa al tempio: la Dea li forgia in rune viventi.", TRUE, ch,
		nullptr, nullptr, TO_ROOM);
	return true;
}

} // namespace Alarmud
