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

MOBSPECIAL_FUNC(PhaseBoss);

} // namespace Alarmud

#endif // __PHASE_BOSS_HPP
