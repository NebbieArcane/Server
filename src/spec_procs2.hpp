/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_PROCS2_HPP
#define __SPEC_PROCS2_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
MOBSPECIAL_FUNC(AbyssGateKeeper) ;
MOBSPECIAL_FUNC(Demon) ;
MOBSPECIAL_FUNC(DemonTeacher) ;
MOBSPECIAL_FUNC(Devil) ;
MOBSPECIAL_FUNC(DogCatcher);
MOBSPECIAL_FUNC(DragonHunterLeader);
MOBSPECIAL_FUNC(DruidAttackSpells);
MOBSPECIAL_FUNC(DruidChallenger) ;
MOBSPECIAL_FUNC(DruidGuildMaster) ;
MOBSPECIAL_FUNC(DwarvenMiners) ;
MOBSPECIAL_FUNC(GenericCityguard) ;
MOBSPECIAL_FUNC(GenericCityguardHateUndead) ;
MOBSPECIAL_FUNC(HuntingMercenary) ;
ROOMSPECIAL_FUNC(Magic_Fountain) ;
MOBSPECIAL_FUNC(MonkChallenger) ;
MOBSPECIAL_FUNC(PaladinGuildmaster) ;
MOBSPECIAL_FUNC(PrisonGuard) ;
MOBSPECIAL_FUNC(PrydainGuard) ;
MOBSPECIAL_FUNC(PsiGuildmaster) ;
MOBSPECIAL_FUNC(RangerGuildmaster) ;
MOBSPECIAL_FUNC(RepairGuy) ;
MOBSPECIAL_FUNC(Samah) ;
OBJSPECIAL_FUNC(SlotMachine);
MOBSPECIAL_FUNC(StatMaster) ;
MOBSPECIAL_FUNC(Summoner) ;
MOBSPECIAL_FUNC(XpMaster) ;
OBJSPECIAL_FUNC(antioch_grenade) ;
MOBSPECIAL_FUNC(archer_instructor) ;
MOBSPECIAL_FUNC(astral_portal) ;
MOBSPECIAL_FUNC(attack_rats) ;
MOBSPECIAL_FUNC(barbarian_guildmaster) ;
MOBSPECIAL_FUNC(camino);
MOBSPECIAL_FUNC(cleric);
MOBSPECIAL_FUNC(creeping_death);
MOBSPECIAL_FUNC(determine_teacher);
MOBSPECIAL_FUNC(druid) ;
ROOMSPECIAL_FUNC(druid_challenge_prep_room);
ROOMSPECIAL_FUNC(druid_challenge_room) ;
MOBSPECIAL_FUNC(druid_protector) ;
MOBSPECIAL_FUNC(equilibrium_teacher) ;
MOBSPECIAL_FUNC(forge_teacher) ;
MOBSPECIAL_FUNC(ghost) ;
MOBSPECIAL_FUNC(hunter) ;
OBJSPECIAL_FUNC(jive_box);
MOBSPECIAL_FUNC(loremaster) ;
MOBSPECIAL_FUNC(mage_specialist_guildmaster) ;
MOBSPECIAL_FUNC(magic_user) ;
MOBSPECIAL_FUNC(miner_teacher) ;
MOBSPECIAL_FUNC(monk) ;
ROOMSPECIAL_FUNC(monk_challenge_prep_room) ;
ROOMSPECIAL_FUNC(monk_challenge_room) ;
MOBSPECIAL_FUNC(monk_master) ;
MOBSPECIAL_FUNC(ninja_master) ;
OBJSPECIAL_FUNC(portal);
MOBSPECIAL_FUNC(real_fox) ;
MOBSPECIAL_FUNC(real_rabbit) ;
MOBSPECIAL_FUNC(sailor) ;
OBJSPECIAL_FUNC(scraps) ;
int MakeQuest( struct char_data* ch, struct char_data* gm, int iClass,const char* arg, int cmd);
void DruidHeal(struct char_data* ch, int level) ;
void DruidMob(struct char_data* ch) ;
void DruidTree(struct char_data* ch) ;
void GreetPeople(struct char_data* ch) ;
void SayHello(struct char_data* ch, struct char_data* t) ;
void Submit(struct char_data* ch, struct char_data* t) ;
} // namespace Alarmud
#endif // __SPEC_PROCS2_HPP

