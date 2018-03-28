/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_PROCS_HPP
#define __SPEC_PROCS_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "specass2.hpp"
#include "specialproc_other.hpp"
namespace Alarmud {
MOBSPECIAL_FUNC(AGGRESSIVE) ;
MOBSPECIAL_FUNC(AbbarachDragon) ;
MOBSPECIAL_FUNC(BiosKaiThanatos) ;
MOBSPECIAL_FUNC(CaravanGuildGuard) ;
MOBSPECIAL_FUNC(CarrionCrawler) ;
MOBSPECIAL_FUNC(ClericGuildMaster) ;
int Donation(struct char_data* ch, int cmd, char* arg, struct room_data* rp,int type);
MOBSPECIAL_FUNC(DracoLich) ;
MOBSPECIAL_FUNC(Drow) ;
struct char_data* FindMobInRoomWithFunction( int room,genericspecial_func);
struct char_data* FindMobInRoomWithVNum( int room, int VNum) ;
int Fountain(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type) ;
int GainLevel(struct char_data* ch, int iClass) ;
int House(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type);
MOBSPECIAL_FUNC(Inquisitor) ;
MOBSPECIAL_FUNC(Keftab) ;
MOBSPECIAL_FUNC(Kraken) ;
MOBSPECIAL_FUNC(Leader) ;
MOBSPECIAL_FUNC(MageGuildMaster) ;
MOBSPECIAL_FUNC(Manticore) ;
MOBSPECIAL_FUNC(MidgaardCitizen) ;
MOBSPECIAL_FUNC(MidgaardCityguard) ;
MOBSPECIAL_FUNC(MordGuard) ;
MOBSPECIAL_FUNC(MordGuildGuard) ;
MOBSPECIAL_FUNC(NewThalosCitizen) ;
MOBSPECIAL_FUNC(NewThalosGuildGuard) ;
MOBSPECIAL_FUNC(NewThalosMayor) ;
MOBSPECIAL_FUNC(NudgeNudge) ;
MOBSPECIAL_FUNC(PaladinGuildGuard) ;
MOBSPECIAL_FUNC(Pungiglione) ;
int Pungiglione_maggiore( struct char_data* ch, int cmd, char* arg,  struct char_data* mob, int type) ;
int Rakda(struct char_data* ch, int cmd, char* arg, struct obj_data* rakda, int type);
void RakdaCast(struct char_data* ch, struct char_data* vict) ;
void RakdaGraphic(struct char_data* ch, struct char_data* vict1) ;
void RakdaMajor(struct char_data* ch, struct char_data* vict) ;
void RakdaMinor(struct char_data* ch, struct char_data* vict) ;
int Ringwraith( struct char_data* ch, int cmd, char* arg,struct char_data* mob, int type );
MOBSPECIAL_FUNC(RustMonster) ;
int SaveRoomContens( struct char_data* pChar, int nCmd, char* pchArg,struct room_data* pRoom, int nEvent ) ;
int SporeCloud( struct char_data* pChar, int nCmd, char* szArg, struct char_data* pMob, int nType );
int SputoVelenoso( struct char_data* ch, int cmd, char* arg,struct char_data* mob, int type );
MOBSPECIAL_FUNC(StatTeller) ;
MOBSPECIAL_FUNC(StormGiant) ;
void String_mob(struct char_data* ch, struct char_data* vict, char* stringa, int campo) ;
MOBSPECIAL_FUNC(SultanGuard) ;
int ThiefGuildMaster( struct char_data* ch, int cmd, char* arg,  struct char_data* mob, int type) ;
void ThrowChar(struct char_data* ch, struct char_data* v, int dir) ;
MOBSPECIAL_FUNC(ThrowerMob) ;
int Tsuchigumo( struct char_data* ch, int cmd, char* arg,  struct char_data* mob, int type) ;
MOBSPECIAL_FUNC(Tyrannosaurus_swallower) ;
MOBSPECIAL_FUNC(Tytan) ;
MOBSPECIAL_FUNC(WarrenGuard) ;
int WarriorGuildMaster(struct char_data* ch, int cmd, char* arg,struct char_data* mob, int type) ;
MOBSPECIAL_FUNC(WizardGuard) ;
MOBSPECIAL_FUNC(andy_wilcox);
MOBSPECIAL_FUNC(banana) ;
int bank (struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type) ;
MOBSPECIAL_FUNC(blink) ;
int chalice(struct char_data* ch, int cmd, char* arg) ;
MOBSPECIAL_FUNC(citizen) ;
MOBSPECIAL_FUNC(delivery_beast) ;
MOBSPECIAL_FUNC(delivery_elf) ;
int dump( struct char_data* ch, int cmd, char* arg, struct room_data* rp,int type);
OBJSPECIAL_FUNC(enter_obj);
MOBSPECIAL_FUNC(eric_johnson);
void exec_social(struct char_data* npc, char* cmd, int next_line,int* cur_line, void** thing);
MOBSPECIAL_FUNC(fido) ;
MOBSPECIAL_FUNC(fighter) ;
struct char_data* find_mobile_here_with_spec_proc(genericspecial_func fcn, int rnumber);
MOBSPECIAL_FUNC(flame) ;
MOBSPECIAL_FUNC(geyser) ;
MOBSPECIAL_FUNC(ghoul) ;
MOBSPECIAL_FUNC(green_slime);
MOBSPECIAL_FUNC(guild_guard) ;
char* how_good(int percent) ;
int is_target_room_p(int room, void* tgt_room) ;
MOBSPECIAL_FUNC(jabberwocky) ;
MOBSPECIAL_FUNC(janitor) ;
MOBSPECIAL_FUNC(jugglernaut) ;
int kings_hall(struct char_data* ch, int cmd, char* arg) ;
MOBSPECIAL_FUNC(magic_user2) ;
MOBSPECIAL_FUNC(mayor) ;
int named_object_on_ground(int room, void* c_data) ;
OBJSPECIAL_FUNC(nodrop);
void npc_steal(struct char_data* ch,struct char_data* victim) ;
MOBSPECIAL_FUNC(paramedics) ;
int pet_shops(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type) ;
int pray_for_items(struct char_data* ch, int cmd, char* arg, struct room_data* rp, int type) ;
MOBSPECIAL_FUNC(puff) ;
MOBSPECIAL_FUNC(regenerator) ;
int replicant( struct char_data* ch, int cmd, char* arg,struct char_data* mob1, int type ) ;
MOBSPECIAL_FUNC(shadow) ;
MOBSPECIAL_FUNC(sisyphus) ;
MOBSPECIAL_FUNC(snake) ;
MOBSPECIAL_FUNC(snake_plus) ;
OBJSPECIAL_FUNC(soap);
MOBSPECIAL_FUNC(temple_labrynth_liar) ;
MOBSPECIAL_FUNC(temple_labrynth_sentry) ;
MOBSPECIAL_FUNC(thief) ;
MOBSPECIAL_FUNC(tormentor) ;
MOBSPECIAL_FUNC(vampire) ;
MOBSPECIAL_FUNC(wraith) ;
void zm_init_combat(struct char_data* zmaster, struct char_data* target) ;
int zm_kill_aggressor(struct char_data* zmaster) ;
int zm_kill_fidos(struct char_data* zmaster) ;
int zm_stunned_followers(struct char_data* zmaster) ;
int zm_tired(struct char_data* zmaster) ;
MOBSPECIAL_FUNC(zombie_master);
OBJSPECIAL_FUNC(zone_obj);
} // namespace Alarmud
#endif // __SPEC_PROCS_HPP

