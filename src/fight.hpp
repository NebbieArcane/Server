/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: fight.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __FIGHT_HPP
#define __FIGHT_HPP
#include "typedefs.hpp"
namespace Alarmud {
extern struct char_data* combat_list;   /* head of l-list of fighting chars    */
extern struct char_data* missile_list;   /* head of l-list of fighting chars    */
extern struct char_data* combat_next_dude; /* Next dude global trick           */
extern struct char_data* missile_next_dude; /* Next dude global trick           */
extern char PeacefulWorks;  /* set in @set */
extern char DestroyedItems;  /* set in MakeScraps */
int BarbarianToHitMagicBonus(struct char_data* ch) ;
void BreakLifeSaverObj(struct char_data* ch) ;
int BrittleCheck(struct char_data* ch, struct char_data* v, int dam) ;
int CalcThaco(struct char_data* ch, struct char_data* victim) ;
bool CheckEquilibrium(struct char_data* ch) ;
bool CheckMirror(struct char_data* victim) ;
int DamCheckDeny(struct char_data* ch, struct char_data* victim, int type) ;
int DamDetailsOk(struct char_data* ch, struct char_data* v, int dam, int type) ;
void DamageAllStuff(struct char_data* ch, int dam_type) ;
int DamageEpilog(struct char_data* ch, struct char_data* victim,int killedbytype, int dam);
int DamageItem(struct char_data* ch, struct obj_data* o, int num) ;
void DamageMessages(struct char_data* ch, struct char_data* v, int dam,int attacktype, int location);
int DamageOneItem(struct char_data* ch, int dam_type, struct obj_data* obj) ;
void DamageStuff(struct char_data* v, int type, int dam, int location) ;
int DamageTrivia(struct char_data* ch, struct char_data* v, int dam, int type, int location);
int DamagedByAttack(struct obj_data* i, int dam_type) ;
DamageResult DoDamage(struct char_data* ch, struct char_data* v, int dam,int type, int location);
long ExpCaps(struct char_data* ch,int group_count, long passedtotal) ;
char_data* Fighting(char_data* pChar) ;
struct char_data* FindAnAttacker(struct char_data* pChar) ;
struct char_data* FindAnyVictim(struct char_data* pChar) ;
struct char_data* FindMetaVictim(struct char_data* ch) ;
struct char_data* FindVictim(struct char_data* pChar) ;
int GetBonusToAttack(struct char_data* pChar, struct char_data* pNewChar) ;
int GetFormType(struct char_data* ch) ;
int GetItemDamageType(int type) ;
int GetWeaponDam(struct char_data* ch, struct char_data* v,struct obj_data* wielded, int location);
int GetWeaponType(struct char_data* ch, struct obj_data** wielded) ;
int Getw_type(struct obj_data* wielded) ;
long GroupLevelRatioExp(struct char_data* ch,int group_max_level,long experiencepoints);
int HitCheckDeny(struct char_data* ch, struct char_data* victim, int type, int DistanceWeapon);
int HitOrMiss(struct char_data* ch, struct char_data* victim, int calc_thaco);
DamageResult HitVictim(struct char_data* ch, struct char_data* v, int dam,int type, int w_type,damage_func dam_func, int location);
int Hit_Location(struct char_data* victim) ;
int ItemSave(struct obj_data* i, int dam_type) ;
int LevelMod(struct char_data* ch, struct char_data* v, int exp) ;
int LoreBackstabBonus(struct char_data* ch, struct char_data* v) ;
void MakeScrap(struct char_data* ch,struct char_data* v, struct obj_data* obj) ;
DamageResult MissVictim(struct char_data* ch, struct char_data* v, int type,int w_type, damage_func dam_func, int location);
DamageResult MissileDamage(struct char_data* ch, struct char_data* victim,int dam, int attacktype, int location);
DamageResult MissileHit(struct char_data* ch, struct char_data* victim,int type);
int MonkDodge(struct char_data* ch, struct char_data* v, int* dam) ;
void NPCAttacks(char_data* pChar) ;
void NailThisSucker(struct char_data* ch) ;
void PCAttacks(char_data* pChar) ;
int PreProcDam(struct char_data* ch, int type, int dam, int classe) ;
int RatioExp(struct char_data* ch, struct char_data* victim, int total) ;
int SetCharFighting(struct char_data* ch, struct char_data* v);
int SetVictFighting(struct char_data* ch, struct char_data* v);
int SkipImmortals(struct char_data* v, int amnt,int attacktype) ;
void StopAllFightingWith(char_data* pChar) ;
struct char_data* SwitchVictimToPrince(struct char_data* pAtt,struct char_data* pVict);
int WeaponCheck(struct char_data* ch, struct char_data* v, int type, int dam) ;
void WeaponSpell(struct char_data* c, struct char_data* v,struct obj_data* obj, int type);
void appear(struct char_data* ch) ;
int berserkdambonus(struct char_data* ch, int dam) ;
int berserkthaco(struct char_data* ch) ;
void change_alignment(struct char_data* ch, struct char_data* victim) ;
int check_peaceful(struct char_data* ch, const char* msg) ;
int clan_gain(struct char_data* ch,int gain) ;
void dam_message(int dam, struct char_data* ch, struct char_data* victim,int w_type, int location);
DamageResult damage(struct char_data* ch, struct char_data* victim,int dam, int attacktype, int location);
void death_cry(struct char_data* ch) ;
void die(struct char_data* ch,int killedbytype, struct char_data* killer);
void group_gain(struct char_data* ch,struct char_data* victim) ;
int group_loss(struct char_data* ch,int loss) ;
DamageResult hit(struct char_data* ch, struct char_data* victim, int type);
void increase_blood(int rm) ;
void load_messages() ;
void make_corpse(struct char_data* ch, int killedbytype) ;
void perform_violence(unsigned long pulse);
int range_hit(struct char_data* ch, struct char_data* targ, int rng, struct obj_data* missile, int tdir, int max_rng);
void raw_kill(struct char_data* ch,int killedbytype) ;
char* replace_string(const char* str, const char* weapon, const char* weapon_s, const char* location_hit, const char* location_hit_s);
DamageResult root_hit(struct char_data* ch, struct char_data* orig_victim,int type, damage_func dam_func, int DistanceWeapon, int location);
void save_exp_to_file(struct char_data* ch,int xp) ;
void set_fighting(struct char_data* ch, struct char_data* vict) ;
void shoot(struct char_data* ch, struct char_data* victim);
void stop_fighting(struct char_data* ch) ;
void update_pos(struct char_data* victim) ;
// FLYP 20180221: moved the keech calculation to a function
int leechResult(struct char_data* ch, int dam);
int canLeech(struct char_data* ch, struct char_data* victim);
} // namespace Alarmud
#endif // __FIGHT_HPP

