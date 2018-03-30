/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_PROCS3_HPP
#define __SPEC_PROCS3_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
MOBSPECIAL_FUNC(AcidBlob);
MOBSPECIAL_FUNC(AcidBreather);
OBJSPECIAL_FUNC(AntiSunItem);
MOBSPECIAL_FUNC(Barbarian);
MOBSPECIAL_FUNC(Beholder);
OBJSPECIAL_FUNC(BerserkerItem);
ROOMSPECIAL_FUNC(BlockWay);
MOBSPECIAL_FUNC(Capo_Fucina);
ROOMSPECIAL_FUNC(ChurchBell);
MOBSPECIAL_FUNC(Cockatrice);
MOBSPECIAL_FUNC(Esattore);
OBJSPECIAL_FUNC(EvilBlade);
MOBSPECIAL_FUNC(FireBreather);
MOBSPECIAL_FUNC(ForceMobToAction);
MOBSPECIAL_FUNC(FrostBreather);
MOBSPECIAL_FUNC(GasBreather);
OBJSPECIAL_FUNC(GoodBlade);
MOBSPECIAL_FUNC(Interact);
MOBSPECIAL_FUNC(ItemGiven);
OBJSPECIAL_FUNC(ItemPut);
MOBSPECIAL_FUNC(LegionariV);
MOBSPECIAL_FUNC(LightningBreather);
ROOMSPECIAL_FUNC(MOBKilled);
MOBSPECIAL_FUNC(MobBlockWay);
OBJSPECIAL_FUNC(ModHit);
OBJSPECIAL_FUNC(NeutralBlade);
int ObjIsOnGround	(struct char_data* ch, const char* argument ) ;
MOBSPECIAL_FUNC(Paladin);
MOBSPECIAL_FUNC(PostMaster);
MOBSPECIAL_FUNC(PrimoAlbero);
MOBSPECIAL_FUNC(Psionist);
MOBSPECIAL_FUNC(Ranger);
MOBSPECIAL_FUNC(Slavalis);
MOBSPECIAL_FUNC(TreeThrowerMob);
OBJSPECIAL_FUNC(TrueDam);
MOBSPECIAL_FUNC(acid_monster);
MOBSPECIAL_FUNC(archer);
int archer_hth		(struct char_data* ch) ;
int archer_sub		(struct char_data* ch) ;
MOBSPECIAL_FUNC(avatar_celestian);
MOBSPECIAL_FUNC(baby_bear);
struct room_data* back_left_square(struct room_data* room) ;
struct room_data* back_right_square(struct room_data* room) ;
struct room_data* back_square(struct room_data* room) ;
MOBSPECIAL_FUNC(banshee);
MOBSPECIAL_FUNC(banshee_lorelai);
MOBSPECIAL_FUNC(chess_game);
MOBSPECIAL_FUNC(cleric_imp);
MOBSPECIAL_FUNC(cleric_mage);
MOBSPECIAL_FUNC(death_knight);
MOBSPECIAL_FUNC(fighter_cleric);
MOBSPECIAL_FUNC(fighter_mage);
struct room_data* forward_left_square(struct room_data* room) ;
struct room_data* forward_right_square(struct room_data* room) ;
struct room_data* forward_square(struct room_data* room) ;
MOBSPECIAL_FUNC(goblin_sentry);
struct room_data* left_square(struct room_data* room) ;
MOBSPECIAL_FUNC(lich_church);
MOBSPECIAL_FUNC(mad_cyrus);
MOBSPECIAL_FUNC(mad_gertruda);
MOBSPECIAL_FUNC(magic_user_imp);
MOBSPECIAL_FUNC(medusa);
OBJSPECIAL_FUNC(msg_obj);
int pick_archer_target( struct char_data* ch, int maxr,struct char_data** targ, int* rn, int* dr );
int range_estimate(struct char_data* ch, struct obj_data* o, int type);
MOBSPECIAL_FUNC(raven_iron_golem);
struct room_data* right_square(struct room_data* room) ;
ROOMSPECIAL_FUNC(sTeleport);
MOBSPECIAL_FUNC(spGeneric);
MOBSPECIAL_FUNC(spTest);
struct char_data* square_contains_enemy(struct room_data* square) ;
int square_contains_friend(struct room_data* square) ;
int square_empty	(struct room_data* square) ;
MOBSPECIAL_FUNC(strahd_vampire);
MOBSPECIAL_FUNC(strahd_zombie);
OBJSPECIAL_FUNC(thion_loader);
MOBSPECIAL_FUNC(timnus);
OBJSPECIAL_FUNC(trap_obj);
void trap_obj_damage(struct char_data* v, int damtype, int amnt, struct obj_data* t) ;
MOBSPECIAL_FUNC(winger);
} // namespace Alarmud
#endif // __SPEC_PROCS3_HPP

