/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <array>
/***************************  Local    include ************************************/
#include "phase_boss_config.hpp"

namespace Alarmud {

/*--------------------------------------------------------------------------
 * Configurazione PhaseBoss — SOLO DATI.
 * Interrupt: skill di classe (Hilt Smash, Fury Howl, Spellshatter, Mind Spike, …).
 *--------------------------------------------------------------------------*/

const PhaseBossProfile kPhaseBossZeus = {
	"zeus",
	13784,
	120,
	{{
		{100, nullptr, nullptr, 0, 0, nullptr, 3},
		{70,
		 "$c0009$n si infuria! Il cielo sopra l'Olimpo si oscura!$c0007",
		 "$c0011Un tuono scuote le pareti.$c0007",
		 0, 0, nullptr, 3},
		{40,
		 "$c0001$n urla e il monte trema! 'OSATE SFIDARE IL RE DEGLI DEI!'$c0007",
		 nullptr,
		 13861, 2,
		 "$c0009Due aquile del fulmine piombano al fianco di $n!$c0007",
		 2}
	}},
	{{
		/* Grezzo ~315-480 (avg ~400): su tank sanct+elec mira a ~100-150 netti. */
		{"bolt", 2, 40, PhaseBossInterrupt::ClassBreak,
		 PhaseBossTarget::Tank, PhaseBossDmg::Lightning,
		 300, 15, 12, 1, 3, 0, 0,
		 "$c0011$n alza la mano destra: un fulmine si forma tra le dita!$c0007",
		 nullptr,
		 nullptr,
		 "$c0011$n scaglia il fulmine!$c0007",
		 "$c0014Il fulmine di $n si spezza a meta' strada!$c0007"},
		{"storm", 3, 100, PhaseBossInterrupt::ClassBreak,
		 PhaseBossTarget::Aoe, PhaseBossDmg::Lightning,
		 70, 8, 10, 2, 5, 1, 0,
		 "$c0009$n apre le braccia: nuvole nere turbinano nella stanza!$c0007",
		 "$c0015La tempesta sta per aprirsi: puoi ancora spezzarla!$c0007",
		 "$c0009I tuoni si avvicinano...$c0007",
		 "$c0009$n scatena una tempesta di fulmini!$c0007",
		 "$c0014La tempesta di $n si spezza in una pioggia elettrica!$c0007"},
		{"judgment", 5, 120, PhaseBossInterrupt::ClassBreak,
		 PhaseBossTarget::Aoe, PhaseBossDmg::Lightning,
		 120, 12, 10, 2, 5, 2, 90,
		 "$c0001$n alza lo sguardo: una luce accecante riempie la sala!$c0007",
		 "$c0015La luce del giudizio e' ancora instabile!$c0007",
		 "$c0011La luce cresce...$c0007",
		 "$c0001Il giudizio di $n si abbatte su di voi!$c0007",
		 "$c0014Il giudizio di $n vi raggiunge.$c0007"},
		{"slam", 2, 9999, PhaseBossInterrupt::None,
		 PhaseBossTarget::Aoe, PhaseBossDmg::Bludgeon,
		 80, 10, 8, 2, 5, 2, 0,
		 "$c0001$n alza l'Egida: questa volta non puoi fermarlo!$c0007",
		 nullptr, nullptr,
		 "$c0001$n abbatte l'Egida! L'urto vi investe.$c0007",
		 nullptr}
	}},
	4,
	{{
		{{{ "bolt", 40 }}},
		{{{ "storm", 35 }, { "bolt", 70 }}},
		{{{ "judgment", 25 }, { "slam", 25 }, { "storm", 55 }, { "bolt", 80 }}}
	}},
	{{1, 2, 4}},
	"$c0011$n cade in ginocchio, la tempesta sembra essersi placata.$c0007",
	"L'azione di Zeus e' gia' stata interrotta.\n\r"
};

namespace {

const PhaseBossProfile* const kAll[] = {
	&kPhaseBossZeus,
};

} // namespace

const PhaseBossProfile* const* phase_boss_profiles(std::size_t& count) noexcept {
	count = sizeof(kAll) / sizeof(kAll[0]);
	return kAll;
}

const PhaseBossProfile* phase_boss_find_by_id(std::string_view id) noexcept {
	std::size_t n = 0;
	const PhaseBossProfile* const* all = phase_boss_profiles(n);
	for(std::size_t i = 0; i < n; ++i) {
		if(all[i] && all[i]->id == id) {
			return all[i];
		}
	}
	return nullptr;
}

const PhaseBossProfile* phase_boss_find_by_vnum(int vnum) noexcept {
	std::size_t n = 0;
	const PhaseBossProfile* const* all = phase_boss_profiles(n);
	for(std::size_t i = 0; i < n; ++i) {
		if(all[i] && all[i]->bind_vnum > 0 && all[i]->bind_vnum == vnum) {
			return all[i];
		}
	}
	return nullptr;
}

const PhaseBossAbility* phase_boss_find_ability(const PhaseBossProfile& profile,
											   std::string_view key) noexcept {
	for(int i = 0; i < profile.ability_count; ++i) {
		if(profile.abilities[static_cast<std::size_t>(i)].key == key) {
			return &profile.abilities[static_cast<std::size_t>(i)];
		}
	}
	return nullptr;
}

} // namespace Alarmud
