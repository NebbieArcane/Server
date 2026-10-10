/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_QUEST_HPP
#define __SPEC_QUEST_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Motore quest dinamiche/fisse e craft legati a premi. */
/* Skeleton fase 0: implementazioni ancora nei file storici; da spostare qui. */

MOBSPECIAL_FUNC(AssignQuest);
MOBSPECIAL_FUNC(MobCaccia);
MOBSPECIAL_FUNC(MobSalvataggio);
ROOMSPECIAL_FUNC(MobKillInRoom);
MOBSPECIAL_FUNC(ItemGiven);
OBJSPECIAL_FUNC(ItemPut);
MOBSPECIAL_FUNC(quest_item_shop);
MOBSPECIAL_FUNC(Capo_Fucina);
MOBSPECIAL_FUNC(Interact);
} // namespace Alarmud
#endif // __SPEC_QUEST_HPP
