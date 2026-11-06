/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_CITY_SHARED_HPP
#define __SPEC_CITY_SHARED_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Servizi urbani riusabili / guardie generiche (non legati a una sola citta). */
/* Skeleton fase 0: implementazioni ancora nei file storici; da spostare qui. */

MOBSPECIAL_FUNC(GenericCityguard);
MOBSPECIAL_FUNC(GenericCityguardHateUndead);
MOBSPECIAL_FUNC(PrydainGuard);
ROOMSPECIAL_FUNC(Magic_Fountain);
MOBSPECIAL_FUNC(RepairGuy);
OBJSPECIAL_FUNC(SlotMachine);
MOBSPECIAL_FUNC(PostMaster);
MOBSPECIAL_FUNC(PrisonGuard);
MOBSPECIAL_FUNC(DwarvenMiners);
MOBSPECIAL_FUNC(DragonHunterLeader);
MOBSPECIAL_FUNC(HuntingMercenary);
ROOMSPECIAL_FUNC(ChurchBell);
} // namespace Alarmud
#endif // __SPEC_CITY_SHARED_HPP
