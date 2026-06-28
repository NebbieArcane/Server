/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include <algorithm>

#include "procarea_exp.hpp"
#include "procarea.hpp"
#include "procarea_internal.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "utils.hpp"

namespace Alarmud {

namespace {

constexpr int kProcPcTiers = 8;
constexpr int kProcExpTrapPct = 135;
constexpr int kProcExpBossMult = 4;

/** XP normali (corridoio/tesoro/add), gruppo a 3 PG — identici in solitaria. */
constexpr int kProcareaNormalXp[kProcPcTiers][PROCAREA_TEMPLATE_BANDS] = {
	/* newbie      */ {400, 900, 1600, 2600, 4000, 5800, 8000, 11000, 15000, 20000},
	/* allievo     */ {700, 1400, 2400, 3800, 5600, 7800, 10500, 14000, 18500, 24000},
	/* apprendista */ {1500, 2800, 4500, 6800, 9500, 13000, 17500, 23000, 30000, 39000},
	/* iniziato    */ {5000, 8500, 13000, 19000, 27000, 37000, 50000, 66000, 86000, 110000},
	/* esperto     */ {15000, 24000, 36000, 52000, 72000, 98000, 130000, 172000, 225000, 290000},
	/* maestro     */ {28000, 42000, 60000, 82000, 110000, 145000, 188000, 242000, 308000, 390000},
	/* barone      */ {40000, 58000, 80000, 108000, 142000, 184000, 235000, 297000, 372000, 462000},
	/* principe    */ {50000, 72000, 98000, 130000, 168000, 215000, 272000, 340000, 422000, 520000},
};

/** Tetto XP per kill in dimensione (boss fascia 10 + margine). */
constexpr long kProcareaSoloExpCap = 200000L;
/** Principe in solitaria: stessa formula multiclasse del mondo, base piu' alta. */
constexpr long kProcareaPrinceSoloExpCap = 250000L;

constexpr long kProcareaGroupCap1 = 250000L;
constexpr long kProcareaGroupCap2 = 350000L;
constexpr long kProcareaGroupCap3 = 450000L;
constexpr long kProcareaGroupCap4 = 500000L;
constexpr long kProcareaGroupCap5 = 550000L;
constexpr long kProcareaGroupCap6 = 600000L;
constexpr long kProcareaGroupCap7 = 700000L;
constexpr long kProcareaGroupCap8 = 800000L;
constexpr long kProcareaGroupCap9 = 900000L;
constexpr long kProcareaGroupCapOther = 1000000L;

[[nodiscard]] long procarea_exp_cap_base(char_data* ch, int group_count) {
	if(group_count >= 1) {
		switch(group_count) {
		case 1:
			return kProcareaGroupCap1;
		case 2:
			return kProcareaGroupCap2;
		case 3:
			return kProcareaGroupCap3;
		case 4:
			return kProcareaGroupCap4;
		case 5:
			return kProcareaGroupCap5;
		case 6:
			return kProcareaGroupCap6;
		case 7:
			return kProcareaGroupCap7;
		case 8:
			return kProcareaGroupCap8;
		case 9:
			return kProcareaGroupCap9;
		default:
			return kProcareaGroupCapOther;
		}
	}
	if(ch != nullptr && GetMaxLevel(ch) >= PRINCIPE) {
		return kProcareaPrinceSoloExpCap;
	}
	return kProcareaSoloExpCap;
}

[[nodiscard]] int procarea_extra_class_count(char_data* ch) {
	if(ch == nullptr) {
		return 0;
	}
	int classes = 0;
	for(int i = MAGE_LEVEL_IND; i <= MAX_CLASS; ++i) {
		if(GET_LEVEL(ch, i)) {
			++classes;
		}
	}
	return std::max(0, classes - 1);
}

[[nodiscard]] int procarea_kind_exp_mult(procarea_internal::ProcMobKind kind) {
	using procarea_internal::ProcMobKind;
	switch(kind) {
	case ProcMobKind::Boss:
		return kProcExpBossMult;
	case ProcMobKind::Trap:
		return kProcExpTrapPct;
	default:
		return 100;
	}
}

} // namespace

int procarea_pc_tier(int group_max_level) {
	const int level = std::max(1, group_max_level);
	if(level <= 10) {
		return 0;
	}
	if(level <= 20) {
		return 1;
	}
	if(level <= 30) {
		return 2;
	}
	if(level <= 40) {
		return 3;
	}
	if(level <= 45) {
		return 4;
	}
	if(level <= 49) {
		return 5;
	}
	if(level <= 50) {
		return 6;
	}
	return 7;
}

int procarea_compute_mob_exp(int group_max_level, int effective_band,
							 procarea_internal::ProcMobKind kind, bool solo_mode) {
	(void)solo_mode;
	const int tier = procarea_pc_tier(group_max_level);
	const int band =
		std::clamp(effective_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	int xp = kProcareaNormalXp[tier][band];
	const int mult = procarea_kind_exp_mult(kind);
	if(mult == kProcExpBossMult) {
		xp *= mult;
	} else if(mult != 100) {
		xp = (xp * mult) / 100;
	}
	return std::max(1, xp);
}

bool procarea_is_procarea_victim(const char_data* victim) {
	if(victim == nullptr || !IS_NPC(victim) || victim->nr != -1) {
		return false;
	}
	if(victim->in_room < 0 || !procarea_is_generated_room(victim->in_room)) {
		return false;
	}
	const int kind = victim->commandp;
	return kind >= static_cast<int>(procarea_internal::ProcMobKind::Normal) &&
		   kind <= static_cast<int>(procarea_internal::ProcMobKind::Trap);
}

long procarea_exp_cap(struct char_data* ch, int group_count, long passedtotal) {
	if(ch == nullptr || passedtotal <= 0) {
		return passedtotal;
	}
	long total = procarea_exp_cap_base(ch, group_count);
	if(passedtotal > total) {
		const int extra_classes = procarea_extra_class_count(ch);
		total += (total / 2) * extra_classes;
	}
	if(passedtotal > total) {
		return total;
	}
	return passedtotal;
}

} // namespace Alarmud
