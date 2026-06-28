/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include <algorithm>
#include <cmath>
#include <array>

#include "procarea_exp.hpp"
#include "procarea.hpp"
#include "procarea_internal.hpp"
#include "procarea_band_stats.inc"
#include "autoenums.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "utility.hpp"

namespace Alarmud {

namespace {

constexpr int kProcPcTiers = 8;
constexpr int kProcExpTrapPct = 135;
constexpr int kProcExpBossMult = 4;
constexpr float kProcExpThreatMin = 0.65f;
constexpr float kProcExpThreatMax = 2.25f;

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

struct ProcBandPoolRef {
	float avg_hp = 1.0f;
	float avg_dam = 1.0f;
	float avg_toughness = 1.0f;
	float avg_hitroll = 0.0f;
};

[[nodiscard]] int procarea_kind_pool_index(procarea_internal::ProcMobKind kind) {
	using procarea_internal::ProcMobKind;
	switch(kind) {
	case ProcMobKind::Boss:
		return 1;
	case ProcMobKind::Trap:
		return 2;
	default:
		return 0;
	}
}

[[nodiscard]] float procarea_combat_raw_hp(const ProcArchetypeCombat& combat) {
	return static_cast<float>(std::max(1, dice(combat.level, 8) + combat.hp_bonus));
}

[[nodiscard]] float procarea_combat_raw_dam(const ProcArchetypeCombat& combat) {
	return static_cast<float>(combat.dam_n) *
			   (static_cast<float>(combat.dam_s) + 1.0f) * 0.5f +
		   static_cast<float>(combat.dam_plus);
}

[[nodiscard]] float procarea_effective_toughness(float armor) {
	return std::max(1.0f, -armor);
}

void procarea_archetype_pool_range(procarea_internal::ProcMobKind kind,
												 int& start, int& count) {
	using procarea_internal::ProcMobKind;
	switch(kind) {
	case ProcMobKind::Boss:
		start = 0;
		count = PROCAREA_BOSS_COUNT;
		break;
	case ProcMobKind::Trap:
		start = PROCAREA_BOSS_COUNT + PROCAREA_MOB_POOL_SIZE;
		count = PROCAREA_TRAP_POOL_SIZE;
		break;
	default:
		start = PROCAREA_BOSS_COUNT;
		count = PROCAREA_MOB_POOL_SIZE;
		break;
	}
}

[[nodiscard]] const ProcBandPoolRef& procarea_band_pool_ref(
	int band, procarea_internal::ProcMobKind kind) {
	static std::array<std::array<ProcBandPoolRef, 3>, PROCAREA_TEMPLATE_BANDS> cache{};
	static std::array<std::array<bool, 3>, PROCAREA_TEMPLATE_BANDS> initialized{};
	const int clamped_band = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int kind_index = procarea_kind_pool_index(kind);
	if(initialized[clamped_band][kind_index]) {
		return cache[clamped_band][kind_index];
	}

	int start = 0;
	int count = 0;
	procarea_archetype_pool_range(kind, start, count);

	ProcBandPoolRef ref{};
	if(count > 0) {
		float hp_sum = 0.0f;
		float dam_sum = 0.0f;
		float tough_sum = 0.0f;
		float hit_sum = 0.0f;
		for(int i = 0; i < count; ++i) {
			const int archetype = start + i;
			if(archetype < 0 || archetype >= PROCAREA_ARCHETYPE_COUNT) {
				continue;
			}
			const ProcArchetypeCombat& combat = kProcBandCombat[clamped_band][archetype];
			hp_sum += procarea_combat_raw_hp(combat);
			dam_sum += procarea_combat_raw_dam(combat);
			tough_sum += procarea_effective_toughness(static_cast<float>(combat.armor));
			hit_sum += static_cast<float>(combat.hitroll);
		}
		const float denom = static_cast<float>(count);
		ref.avg_hp = std::max(1.0f, hp_sum / denom);
		ref.avg_dam = std::max(1.0f, dam_sum / denom);
		ref.avg_toughness = std::max(1.0f, tough_sum / denom);
		ref.avg_hitroll = hit_sum / denom;
	}

	cache[clamped_band][kind_index] = ref;
	initialized[clamped_band][kind_index] = true;
	return cache[clamped_band][kind_index];
}

[[nodiscard]] int procarea_popcount_unsigned(unsigned value) {
	int count = 0;
	while(value != 0U) {
		count += static_cast<int>(value & 1U);
		value >>= 1U;
	}
	return count;
}

[[nodiscard]] float procarea_caster_factor(const char_data* mob) {
	if(mob == nullptr) {
		return 1.0f;
	}
	float factor = 1.0f;
	const bool casts =
		mob->specials.spellfail == 0 ||
		IS_SET(mob->specials.act,
			   ACT_MAGIC_USER | ACT_CLERIC | ACT_DRUID | ACT_PSI | ACT_PALADIN | ACT_RANGER);
	if(mob->specials.spellfail == 0) {
		factor += 0.08f;
	}
	if(IS_SET(mob->specials.act, ACT_MAGIC_USER)) {
		factor += 0.14f;
	}
	if(IS_SET(mob->specials.act, ACT_CLERIC)) {
		factor += 0.11f;
	}
	if(IS_SET(mob->specials.act, ACT_DRUID)) {
		factor += 0.09f;
	}
	if(IS_SET(mob->specials.act, ACT_PSI)) {
		factor += 0.10f;
	}
	if(IS_SET(mob->specials.act, ACT_PALADIN)) {
		factor += 0.06f;
	}
	if(IS_SET(mob->specials.act, ACT_RANGER)) {
		factor += 0.05f;
	}
	if(casts) {
		const int spellpower = SpellpowerTotal(mob);
		if(spellpower > 0) {
			const int level = std::max(1, static_cast<int>(GET_LEVEL(mob, WARRIOR_LEVEL_IND)));
			const float ref =
				std::max(4.0f, static_cast<float>(level) * 0.12f + 2.0f);
			const float sp_ratio = static_cast<float>(spellpower) / ref;
			factor += std::min(0.14f, sp_ratio * 0.07f);
		}
		const int mana = mob->points.max_mana;
		if(mana > 100) {
			factor += std::min(0.06f, static_cast<float>(mana - 100) * 0.0006f);
		}
		int save_sum = 0;
		for(int i = 0; i < 5; ++i) {
			save_sum += static_cast<int>(mob->specials.apply_saving_throw[i]);
		}
		const float avg_save = static_cast<float>(save_sum) / 5.0f;
		if(avg_save < 12.0f) {
			factor += std::min(0.05f, (12.0f - avg_save) * 0.008f);
		}
	}
	return factor;
}

[[nodiscard]] float procarea_aff_factor(const char_data* mob) {
	if(mob == nullptr) {
		return 1.0f;
	}
	float factor = 1.0f;
	if(IS_AFFECTED(mob, AFF_SANCTUARY)) {
		factor += 0.18f;
	}
	if(IS_AFFECTED(mob, AFF_FIRESHIELD)) {
		factor += 0.09f;
	}
	if(IS_AFFECTED(mob, AFF_FLYING)) {
		factor += 0.04f;
	}
	if(IS_AFFECTED(mob, AFF_TRUE_SIGHT)) {
		factor += 0.03f;
	}
	return factor;
}

[[nodiscard]] float procarea_immunity_factor(const char_data* mob,
											 procarea_internal::ProcMobKind kind) {
	if(mob == nullptr) {
		return 1.0f;
	}
	const int immune_bits = procarea_popcount_unsigned(mob->immune);
	const int m_immune_bits = procarea_popcount_unsigned(mob->M_immune);
	float factor = 1.0f + static_cast<float>(immune_bits + m_immune_bits) * 0.012f;
	if(kind == procarea_internal::ProcMobKind::Boss &&
	   (mob->immune & (IMM_BLUNT | IMM_PIERCE | IMM_SLASH)) ==
		   (IMM_BLUNT | IMM_PIERCE | IMM_SLASH)) {
		factor += 0.07f;
	}
	return std::min(factor, 1.38f);
}

[[nodiscard]] float procarea_clamp_ratio(float ratio) {
	return std::clamp(ratio, 0.75f, 1.45f);
}

[[nodiscard]] float procarea_geom_mean4(float a, float b, float c, float d) {
	a = procarea_clamp_ratio(a);
	b = procarea_clamp_ratio(b);
	c = procarea_clamp_ratio(c);
	d = procarea_clamp_ratio(d);
	return std::exp((std::log(a) + std::log(b) + std::log(c) + std::log(d)) * 0.25f);
}

[[nodiscard]] float procarea_mob_threat_multiplier(const char_data* mob, int band,
												   int archetype_index,
												   procarea_internal::ProcMobKind kind) {
	if(mob == nullptr) {
		return 1.0f;
	}
	const int clamped_band = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int archetype =
		std::clamp(archetype_index, 0, PROCAREA_ARCHETYPE_COUNT - 1);
	const ProcArchetypeCombat& combat = kProcBandCombat[clamped_band][archetype];
	const ProcBandPoolRef& ref = procarea_band_pool_ref(clamped_band, kind);

	const float hp_ratio = procarea_combat_raw_hp(combat) / ref.avg_hp;
	const float dam_ratio = procarea_combat_raw_dam(combat) / ref.avg_dam;
	const float tough_ratio =
		procarea_effective_toughness(static_cast<float>(combat.armor)) / ref.avg_toughness;
	const float hit_ratio =
		(static_cast<float>(combat.hitroll) + 15.0f) / (ref.avg_hitroll + 15.0f);

	float mult = procarea_geom_mean4(hp_ratio, dam_ratio, tough_ratio, hit_ratio);
	mult *= procarea_caster_factor(mob);
	mult *= procarea_aff_factor(mob);
	mult *= procarea_immunity_factor(mob, kind);
	mult *= std::clamp(mob->mult_att, 0.5f, 4.0f);

	return std::clamp(mult, kProcExpThreatMin, kProcExpThreatMax);
}

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

int procarea_compute_mob_exp(const char_data* mob, int group_max_level, int effective_band,
							 procarea_internal::ProcMobKind kind, int archetype_index) {
	const int tier = procarea_pc_tier(group_max_level);
	const int band =
		std::clamp(effective_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	int xp = kProcareaNormalXp[tier][band];
	const int kind_mult = procarea_kind_exp_mult(kind);
	if(kind_mult == kProcExpBossMult) {
		xp *= kind_mult;
	} else if(kind_mult != 100) {
		xp = (xp * kind_mult) / 100;
	}

	const float threat =
		procarea_mob_threat_multiplier(mob, band, archetype_index, kind);
	xp = static_cast<int>(std::lround(static_cast<float>(xp) * threat));
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
