/*$Id: fight.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/****************************************************************************
 * In questo file ci sono i prototipi e le strutture usate per le procedure
 * nel modulo FIGHT
 * *************************************************************************/
#if !defined( _FIGHT_H)
#define _FIGHT_H

#include "structs.hpp"
enum DamageResult {
	AllLiving, SubjectDead, VictimDead
};
/* fight.cpp */
int Hit_Location(struct char_data *victim);
void appear(struct char_data *ch);
int LevelMod(struct char_data *ch, struct char_data *v, int exp);
int RatioExp(struct char_data *ch, struct char_data *victim, int total);
void load_messages(void);
void update_pos(struct char_data *victim);
int check_peaceful(struct char_data *ch, const char *msg);
void set_fighting(struct char_data *ch, struct char_data *vict);
void stop_fighting(struct char_data *ch);
void make_corpse(struct char_data *ch, int killedbytype);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch, int killedbytype);
int clan_gain(struct char_data *ch, int gain);
void save_exp_to_file(struct char_data *ch, int xp);
void die(struct char_data *ch, int killedbytype, struct char_data *killer);
long ExpCaps(struct char_data *ch, int group_count, long passedtotal);
long GroupLevelRatioExp(struct char_data *ch, int group_max_level, long experiencepoints);
void group_gain(struct char_data *ch, struct char_data *victim);
int group_loss(struct char_data *ch, int loss);
char *replace_string(char *str, char *weapon, char *weapon_s, char *location_hit, char *location_hit_s);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type, int location);
int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type);
int DamDetailsOk(struct char_data *ch, struct char_data *v, int dam, int type);
int SetCharFighting(struct char_data *ch, struct char_data *v);
int SetVictFighting(struct char_data *ch, struct char_data *v);
int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type, int location);
void DamageMessages(struct char_data *ch, struct char_data *v, int dam, int attacktype, int location);
int DamageEpilog(struct char_data *ch, struct char_data *victim, int killedbytype, int dam);
int GetWeaponType(struct char_data *ch, struct obj_data **wielded);
int Getw_type(struct obj_data *wielded);
int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type, int DistanceWeapon);
int CalcThaco(struct char_data *ch, struct char_data *victim);
int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco);
int GetWeaponDam(struct char_data *ch, struct char_data *v, struct obj_data *wielded, int location);
int LoreBackstabBonus(struct char_data *ch, struct char_data *v);
void perform_violence(unsigned long pulse);
int GetBonusToAttack(struct char_data *pChar, struct char_data *pNewChar);
struct char_data *SwitchVictimToPrince(struct char_data *pAtt, struct char_data *pVict);
struct char_data *FindVictim(struct char_data *pChar);
struct char_data *FindAnyVictim(struct char_data *pChar);
struct char_data *FindAnAttacker(struct char_data *pChar);
void BreakLifeSaverObj(struct char_data *ch);
int BrittleCheck(struct char_data *ch, struct char_data *v, int dam);
int PreProcDam(struct char_data *ch, int type, int dam, int classe);
int DamageOneItem(struct char_data *ch, int dam_type, struct obj_data *obj);
void MakeScrap(struct char_data *ch, struct char_data *v, struct obj_data *obj);
void DamageAllStuff(struct char_data *ch, int dam_type);
int DamageItem(struct char_data *ch, struct obj_data *o, int num);
int ItemSave(struct obj_data *i, int dam_type);
int DamagedByAttack(struct obj_data *i, int dam_type);
int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam);
void DamageStuff(struct char_data *v, int type, int dam, int location);
int GetItemDamageType(int type);
int SkipImmortals(struct char_data *v, int amnt, int attacktype);
void WeaponSpell(struct char_data *c, struct char_data *v, struct obj_data *obj, int type);
void shoot(struct char_data *ch, struct char_data *victim);
struct char_data *FindMetaVictim(struct char_data *ch);
void NailThisSucker(struct char_data *ch);
int GetFormType(struct char_data *ch);
int MonkDodge(struct char_data *ch, struct char_data *v, int *dam);
int BarbarianToHitMagicBonus(struct char_data *ch);
int berserkthaco(struct char_data *ch);
int berserkdambonus(struct char_data *ch, int dam);
int range_hit(struct char_data *ch, struct char_data *targ, int rng, struct obj_data *missile, int tdir, int max_rng);
void increase_blood(int rm);
DamageResult damage( struct char_data* ch, struct char_data* victim,int dam, int attacktype, int location );
DamageResult MissileDamage( struct char_data* ch, struct char_data* victim,int dam, int attacktype, int location );
DamageResult hit(struct char_data* ch, struct char_data* victim,int type );
#endif
