/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_GATES_HPP
#define __SPEC_GATES_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Blocchi passaggio, teleport, trigger/utility di debug. */
/* Skeleton fase 0: implementazioni ancora nei file storici; da spostare qui. */

ROOMSPECIAL_FUNC(BlockWay);
ROOMSPECIAL_FUNC(sBlockWay);
MOBSPECIAL_FUNC(MobBlockWay);
MOBSPECIAL_FUNC(sMobBlockWay);
ROOMSPECIAL_FUNC(BlockAlign);
MOBSPECIAL_FUNC(MobBlockAlign);
ROOMSPECIAL_FUNC(sTeleport);
MOBSPECIAL_FUNC(TreeThrowerMob);
MOBSPECIAL_FUNC(camino);
MOBSPECIAL_FUNC(astral_portal);
MOBSPECIAL_FUNC(ForceMobToAction);
MOBSPECIAL_FUNC(ChangeDam);
MOBSPECIAL_FUNC(spGeneric);
MOBSPECIAL_FUNC(spTest);
} // namespace Alarmud
#endif // __SPEC_GATES_HPP
