/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PHASE_BOSS_CONFIG_HPP
#define __PHASE_BOSS_CONFIG_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace Alarmud {

/**
 * Dati PhaseBoss (profili, ability, pesi).
 * Nuovo boss: aggiungi un PhaseBossProfile in phase_boss_config.cpp e
 * registralo in kAll[]; myst.spe → M <vnum> PhaseBoss <id>
 */

enum class PhaseBossInterrupt : unsigned {
	None       = 0,
	ClassBreak = 1u << 0 /* skill interrupt 1x classe */
};

enum class PhaseBossDmg : std::uint8_t { Lightning = 0, Bludgeon };

enum class PhaseBossTarget : std::uint8_t { Tank = 0, Aoe };

[[nodiscard]] constexpr PhaseBossInterrupt operator|(PhaseBossInterrupt a,
													 PhaseBossInterrupt b) noexcept {
	return static_cast<PhaseBossInterrupt>(static_cast<unsigned>(a) |
										   static_cast<unsigned>(b));
}

[[nodiscard]] constexpr PhaseBossInterrupt operator&(PhaseBossInterrupt a,
													 PhaseBossInterrupt b) noexcept {
	return static_cast<PhaseBossInterrupt>(static_cast<unsigned>(a) &
										   static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool phase_boss_any(PhaseBossInterrupt mask,
											PhaseBossInterrupt bit) noexcept {
	return (mask & bit) != PhaseBossInterrupt::None;
}

struct PhaseBossAbility {
	std::string_view key;
	int windup = 0;
	int cast_hp = 0;
	PhaseBossInterrupt mask = PhaseBossInterrupt::None;
	PhaseBossTarget target = PhaseBossTarget::Tank;
	PhaseBossDmg dmg = PhaseBossDmg::Lightning;
	int dam_base = 0;
	int dice_n = 0;
	int dice_s = 0;
	/** soft damage = full * soft_num / soft_den */
	int soft_num = 2;
	int soft_den = 5;
	int min_phase = 0;       /* 0 Idle / 1 Rage / 2 Wrath */
	int min_fight_ticks = 0; /* gate ultimate */
	const char* emote_start = nullptr;
	const char* emote_hint = nullptr;
	const char* emote_tick = nullptr;
	const char* emote_full = nullptr;
	const char* emote_soft = nullptr;
};

struct PhaseBossPhase {
	int enter_hp = 100; /* Idle=100, Rage=70, Wrath=40 */
	const char* enter_emote = nullptr;
	const char* enter_emote2 = nullptr;
	int add_vnum = 0;
	int add_max = 0;
	const char* add_emote = nullptr;
	int cd_after_cast = 3;
};

/** Chance assoluta 1..100, valutati in ordine (primo match vince). */
struct PhaseBossWeight {
	std::string_view ability_key;
	int chance = 0;
};

struct PhaseBossProfile {
	std::string_view id;
	int bind_vnum = 0; /* 0 = solo per nome da spe param */
	int soft_enrage_ticks = 120;
	std::array<PhaseBossPhase, 3> phases{};
	std::array<PhaseBossAbility, 8> abilities{};
	int ability_count = 0;
	std::array<std::array<PhaseBossWeight, 6>, 3> weights{};
	std::array<int, 3> weight_count{};
	const char* death_emote = nullptr;
	const char* already_broken = "L'azione del boss e' gia' stata spezzata.\n\r";
};

[[nodiscard]] const PhaseBossProfile* const* phase_boss_profiles(std::size_t& count) noexcept;
[[nodiscard]] const PhaseBossProfile* phase_boss_find_by_id(std::string_view id) noexcept;
[[nodiscard]] const PhaseBossProfile* phase_boss_find_by_vnum(int vnum) noexcept;
[[nodiscard]] const PhaseBossAbility* phase_boss_find_ability(const PhaseBossProfile& profile,
															 std::string_view key) noexcept;

extern const PhaseBossProfile kPhaseBossZeus;

} // namespace Alarmud

#endif // __PHASE_BOSS_CONFIG_HPP
