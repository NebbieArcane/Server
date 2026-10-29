/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_OBJ_HPP
#define __SPEC_OBJ_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Oggetti speciali (portali, ego blade, trappole, utilita, thion_loader). */
/* Skeleton fase 0: implementazioni ancora nei file storici; da spostare qui. */

OBJSPECIAL_FUNC(portal);
OBJSPECIAL_FUNC(enter_obj);
OBJSPECIAL_FUNC(zone_obj);
OBJSPECIAL_FUNC(soap);
OBJSPECIAL_FUNC(nodrop);
OBJSPECIAL_FUNC(scraps);
OBJSPECIAL_FUNC(key_one_use);
OBJSPECIAL_FUNC(Rakda);
OBJSPECIAL_FUNC(jive_box);
OBJSPECIAL_FUNC(EvilBlade);
OBJSPECIAL_FUNC(GoodBlade);
OBJSPECIAL_FUNC(NeutralBlade);
OBJSPECIAL_FUNC(BerserkerItem);
OBJSPECIAL_FUNC(AntiSunItem);
OBJSPECIAL_FUNC(TrueDam);
OBJSPECIAL_FUNC(trap_obj);
OBJSPECIAL_FUNC(ModHit);
OBJSPECIAL_FUNC(msg_obj);
OBJSPECIAL_FUNC(antioch_grenade);
MOBSPECIAL_FUNC(sEgoWeapon);
OBJSPECIAL_FUNC(thion_loader);
} // namespace Alarmud
#endif // __SPEC_OBJ_HPP
