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
struct char_data* FindMobInRoomWithFunction(int room,genericspecial_func);
struct obj_data* FindObjInRoomWithFunction(int room, genericspecial_func func);
struct char_data* FindMobInRoomWithVNum(int room, int VNum) ;
int GainLevel(struct char_data* ch, int iClass) ;
int chalice(struct char_data* ch, int cmd, char* arg) ;
void exec_social(struct char_data* npc, char* cmd, int next_line,int* cur_line, void** thing);
struct char_data* find_mobile_here_with_spec_proc(genericspecial_func fcn, int rnumber);
char* how_good(int percent) ;
FIND_FUNC(is_target_room_p);
int kings_hall(struct char_data* ch, int cmd, char* arg) ;
FIND_FUNC(named_object_on_ground) ;
void RakdaCast(struct char_data* ch, struct char_data* vict) ;
void RakdaGraphic(struct char_data* ch, struct char_data* vict1) ;
void RakdaMajor(struct char_data* ch, struct char_data* vict) ;
void RakdaMinor(struct char_data* ch, struct char_data* vict) ;
void String_mob(struct char_data* ch, struct char_data* vict, const char* stringa, int campo) ;
void ThrowChar(struct char_data* ch, struct char_data* v, int dir) ;
void npc_steal(struct char_data* ch,struct char_data* victim) ;
void zm_init_combat(struct char_data* zmaster, struct char_data* target) ;
int zm_kill_aggressor(struct char_data* zmaster) ;
int zm_kill_fidos(struct char_data* zmaster) ;
int zm_stunned_followers(struct char_data* zmaster) ;
int zm_tired(struct char_data* zmaster) ;
MOBSPECIAL_FUNC(AGGRESSIVE) ;
MOBSPECIAL_FUNC(AbbarachDragon) ;
MOBSPECIAL_FUNC(BiosKaiThanatos) ;
MOBSPECIAL_FUNC(CaravanGuildGuard) ;
MOBSPECIAL_FUNC(CarrionCrawler) ;
MOBSPECIAL_FUNC(ClericGuildMaster) ;
ROOMSPECIAL_FUNC(Donation);
MOBSPECIAL_FUNC(DracoLich) ;
MOBSPECIAL_FUNC(Drow) ;
ROOMSPECIAL_FUNC(Fountain) ;
ROOMSPECIAL_FUNC(House);
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
MOBSPECIAL_FUNC(Pungiglione_maggiore) ;
OBJSPECIAL_FUNC(Rakda);
MOBSPECIAL_FUNC(Ringwraith);
MOBSPECIAL_FUNC(RustMonster) ;
ROOMSPECIAL_FUNC(SaveRoomContens) ;
MOBSPECIAL_FUNC(SporeCloud);
MOBSPECIAL_FUNC(SputoVelenoso);
MOBSPECIAL_FUNC(StatTeller) ;
MOBSPECIAL_FUNC(StormGiant) ;
MOBSPECIAL_FUNC(SultanGuard) ;
MOBSPECIAL_FUNC(ThiefGuildMaster) ;
MOBSPECIAL_FUNC(ThrowerMob) ;
MOBSPECIAL_FUNC(Tsuchigumo) ;
MOBSPECIAL_FUNC(Tyrannosaurus_swallower) ;
MOBSPECIAL_FUNC(Tytan) ;
MOBSPECIAL_FUNC(WarrenGuard) ;
MOBSPECIAL_FUNC(WarriorGuildMaster) ;
MOBSPECIAL_FUNC(WizardGuard) ;
MOBSPECIAL_FUNC(andy_wilcox);
MOBSPECIAL_FUNC(banana) ;
ROOMSPECIAL_FUNC(bank) ;
MOBSPECIAL_FUNC(blink) ;
MOBSPECIAL_FUNC(citizen) ;
MOBSPECIAL_FUNC(delivery_beast) ;
MOBSPECIAL_FUNC(delivery_elf) ;
ROOMSPECIAL_FUNC(dump);
OBJSPECIAL_FUNC(enter_obj);
MOBSPECIAL_FUNC(eric_johnson);
MOBSPECIAL_FUNC(fido) ;
MOBSPECIAL_FUNC(fighter) ;
MOBSPECIAL_FUNC(flame) ;
MOBSPECIAL_FUNC(geyser) ;
MOBSPECIAL_FUNC(ghoul) ;
MOBSPECIAL_FUNC(green_slime);
MOBSPECIAL_FUNC(guild_guard) ;
MOBSPECIAL_FUNC(jabberwocky) ;
MOBSPECIAL_FUNC(janitor) ;
MOBSPECIAL_FUNC(jugglernaut) ;
MOBSPECIAL_FUNC(magic_user2) ;
MOBSPECIAL_FUNC(mayor) ;
OBJSPECIAL_FUNC(nodrop);
MOBSPECIAL_FUNC(paramedics) ;
ROOMSPECIAL_FUNC(pet_shops) ;
ROOMSPECIAL_FUNC(pray_for_items) ;
MOBSPECIAL_FUNC(puff) ;
MOBSPECIAL_FUNC(regenerator) ;
MOBSPECIAL_FUNC(replicant) ;
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
MOBSPECIAL_FUNC(zombie_master);
OBJSPECIAL_FUNC(zone_obj);
} // namespace Alarmud
#endif // __SPEC_PROCS_HPP
