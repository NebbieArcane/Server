/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PHASE_BOSS_HPP
#define __PHASE_BOSS_HPP

#include "phase_boss_config.hpp"

namespace Alarmud {

enum class PhaseBossInterruptResult {
	NotBoss,        /* target non ha PhaseBoss / no profile */
	NotCasting,     /* boss ma non in telegraph */
	Soft,           /* cast indebolita */
	Hard,           /* cast spezzata */
	AlreadyBroken   /* gia' hard-interrupted */
};

/**
 * Tenta interrupt class-skill sul telegraph PhaseBoss.
 * power tipico: 40 (1 skill = hard su cast_hp 40).
 */
[[nodiscard]] PhaseBossInterruptResult phase_boss_try_interrupt(struct char_data* ch,
															   struct char_data* victim,
															   int power);

/** Mob con special PhaseBoss (o profilo bindato per vnum). */
[[nodiscard]] bool phase_boss_is_boss(const struct char_data* ch) noexcept;

/**
 * Bonus % danno enrage (0 se off / sopra soglia).
 * Vale per melee e ability Special.
 */
[[nodiscard]] int phase_boss_enrage_bonus_pct(const struct char_data* ch) noexcept;

/** Immuni a lag/atterramento/CC (WAIT_STATE, pos sitting, gust, sleep, para, …).
 *  Bash su PhaseBoss fallisce sempre (rimbalzo); kick/springleap/disarm partono
 *  (danno / strip arma) ma senza CC sul boss. Class-interrupt: soft/hard telegraph. */
[[nodiscard]] bool phase_boss_resists_crowd_control(const struct char_data* ch) noexcept;

/**
 * Applica GET_POS solo se non PhaseBoss.
 * @return true se la pos e' stata cambiata.
 */
bool phase_boss_try_set_pos(struct char_data* victim, int pos) noexcept;

MOBSPECIAL_FUNC(PhaseBoss);

} // namespace Alarmud

#endif // __PHASE_BOSS_HPP
