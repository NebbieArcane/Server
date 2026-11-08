/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_GUILD_HPP
#define __SPEC_GUILD_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* Gilde di classe: practice/gain, guardie gilda, teacher skill, challenge monk/druid. */
/* Skeleton fase 0: implementazioni ancora nei file storici; da spostare qui. */

MOBSPECIAL_FUNC(MageGuildMaster);
MOBSPECIAL_FUNC(ClericGuildMaster);
MOBSPECIAL_FUNC(ThiefGuildMaster);
MOBSPECIAL_FUNC(WarriorGuildMaster);
MOBSPECIAL_FUNC(PaladinGuildmaster);
MOBSPECIAL_FUNC(PsiGuildmaster);
MOBSPECIAL_FUNC(RangerGuildmaster);
MOBSPECIAL_FUNC(barbarian_guildmaster);
MOBSPECIAL_FUNC(DruidGuildMaster);
MOBSPECIAL_FUNC(monk_master);
MOBSPECIAL_FUNC(mage_specialist_guildmaster);
MOBSPECIAL_FUNC(ninja_master);
MOBSPECIAL_FUNC(PaladinGuildGuard);
MOBSPECIAL_FUNC(WizardGuard);
MOBSPECIAL_FUNC(guild_guard);
MOBSPECIAL_FUNC(sailor);
MOBSPECIAL_FUNC(loremaster);
MOBSPECIAL_FUNC(hunter);
MOBSPECIAL_FUNC(determine_teacher);
MOBSPECIAL_FUNC(miner_teacher);
MOBSPECIAL_FUNC(DemonTeacher);
MOBSPECIAL_FUNC(forge_teacher);
MOBSPECIAL_FUNC(equilibrium_teacher);
MOBSPECIAL_FUNC(archer_instructor);
MOBSPECIAL_FUNC(DruidChallenger);
MOBSPECIAL_FUNC(MonkChallenger);
ROOMSPECIAL_FUNC(druid_challenge_prep_room);
ROOMSPECIAL_FUNC(druid_challenge_room);
ROOMSPECIAL_FUNC(monk_challenge_prep_room);
ROOMSPECIAL_FUNC(monk_challenge_room);
} // namespace Alarmud
#endif // __SPEC_GUILD_HPP
