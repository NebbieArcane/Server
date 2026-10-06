/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __EDIT_AFFECT_BROKER_HPP
#define __EDIT_AFFECT_BROKER_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "typedefs.hpp"

namespace Alarmud {

/**
 * Mob special: trasferimento affect tra due edit PERSONAL dello stesso toon
 * (ask ... trasferisci) e soft-delete con rimborso XP (ask ... distruggi).
 * Aggancio: myst.spe  M <vnum> EditAffectBroker
 */
MOBSPECIAL_FUNC(EditAffectBroker);

} // namespace Alarmud
#endif // __EDIT_AFFECT_BROKER_HPP
