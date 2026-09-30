/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Classificazione kit PG per istanze solitarie.
 * I numeri (kit_base, none, caster_keep, beholder) stanno in procarea_balance
 * (DB + WIZ: dimensione kit ...).
 */
#ifndef __PROCAREA_SOLO_KIT_INC
#define __PROCAREA_SOLO_KIT_INC

#include "multiclass.hpp"
#include "procarea_balance.hpp"
#include "utils.hpp"
#include <algorithm>

/* Incluso dentro namespace Alarmud (come le altre .inc procarea). */
namespace procarea_internal {

struct ProcSoloKitProfile {
	ProcSoloKit kit = ProcSoloKit::PureMelee;
	float kit_base = 1.0f;
	float beholder_mult = 1.0f;
	float toughness_mult = 1.0f;
	float corridor_none_mult = 1.0f;
	float caster_keep_mult = 1.0f;
	const char* label = "pure-melee";
};

[[nodiscard]] inline const char* procarea_solo_kit_label(ProcSoloKit kit) noexcept {
	switch(kit) {
	case ProcSoloKit::ClMk:
		return "cl/mk";
	case ProcSoloKit::FullHybrid:
		return "full-hybrid";
	case ProcSoloKit::HealHybrid:
		return "heal-hybrid";
	case ProcSoloKit::SupportCaster:
		return "support-caster";
	case ProcSoloKit::DpsHybrid:
		return "dps-hybrid";
	case ProcSoloKit::PureMelee:
		return "pure-melee";
	case ProcSoloKit::PureHealer:
		return "pure-healer";
	case ProcSoloKit::BeholderCaster:
		return "beholder-caster";
	case ProcSoloKit::OtherCaster:
		return "other-caster";
	default:
		return "pure-melee";
	}
}

[[nodiscard]] inline bool procarea_solo_has_beholder_poly(const char_data* ch) {
	return ch != nullptr && HasClass(ch, CLASS_MAGIC_USER | CLASS_SORCERER);
}

[[nodiscard]] inline ProcSoloKit procarea_solo_classify_kit(const char_data* ch) {
	if(ch == nullptr) {
		return ProcSoloKit::PureMelee;
	}
	const bool mu = HasClass(ch, CLASS_MAGIC_USER | CLASS_SORCERER);
	const bool cl = HasClass(ch, CLASS_CLERIC);
	const bool wa = HasClass(ch, CLASS_WARRIOR);
	const bool th = HasClass(ch, CLASS_THIEF);
	const bool dr = HasClass(ch, CLASS_DRUID);
	const bool mk = HasClass(ch, CLASS_MONK);
	const bool ba = HasClass(ch, CLASS_BARBARIAN);
	const bool pa = HasClass(ch, CLASS_PALADIN);
	const bool ra = HasClass(ch, CLASS_RANGER);
	const bool psi = HasClass(ch, CLASS_PSI);
	const int n = HowManyClasses(const_cast<char_data*>(ch));

	if(cl && mk && n == 2) {
		return ProcSoloKit::ClMk;
	}
	if(n >= 3) {
		return ProcSoloKit::FullHybrid;
	}
	if(n == 2) {
		if(cl && mu) {
			return ProcSoloKit::SupportCaster;
		}
		if(cl && (wa || ra || th)) {
			return ProcSoloKit::HealHybrid;
		}
		return ProcSoloKit::DpsHybrid;
	}
	if(cl) {
		return ProcSoloKit::PureHealer;
	}
	if(mu) {
		return ProcSoloKit::BeholderCaster;
	}
	if(psi || dr) {
		return ProcSoloKit::OtherCaster;
	}
	if(wa || ba || mk || th || pa || ra) {
		return ProcSoloKit::PureMelee;
	}
	return ProcSoloKit::PureMelee;
}

[[nodiscard]] inline float procarea_solo_beholder_mult_for_eq(float eq_index) noexcept {
	const ProcSoloKitConfig& cfg = procarea_solo_kit_config();
	const float span = PROCAREA_POWER_SCALE_MAX - PROCAREA_POWER_SCALE_MIN;
	if(span <= 0.0f) {
		return 1.0f;
	}
	const float t = std::clamp((eq_index - PROCAREA_POWER_SCALE_MIN) / span, 0.0f, 1.0f);
	return cfg.beholder_mult_low +
		   (cfg.beholder_mult_high - cfg.beholder_mult_low) * t;
}

[[nodiscard]] inline ProcSoloKitProfile procarea_solo_kit_profile(const char_data* ch,
																 float eq_index) {
	const ProcSoloKitConfig& cfg = procarea_solo_kit_config();
	ProcSoloKitProfile p{};
	p.kit = procarea_solo_classify_kit(ch);
	const int idx = static_cast<int>(p.kit);
	p.kit_base = cfg.kit_base[idx];
	p.corridor_none_mult = cfg.corridor_none_mult[idx];
	p.caster_keep_mult = cfg.caster_keep_mult[idx];
	p.label = procarea_solo_kit_label(p.kit);
	if(procarea_solo_has_beholder_poly(ch)) {
		p.beholder_mult = procarea_solo_beholder_mult_for_eq(eq_index);
	}
	else {
		p.beholder_mult = 1.0f;
	}
	p.toughness_mult = std::min(cfg.toughness_max, p.kit_base * p.beholder_mult);
	p.toughness_mult = std::clamp(p.toughness_mult, cfg.toughness_min, cfg.toughness_max);
	return p;
}

} // namespace procarea_internal

#endif /* __PROCAREA_SOLO_KIT_INC */
