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

void load_messages();
void update_pos( struct char_data* victim );
int check_peaceful(struct char_data* ch, char* msg);
void set_fighting(struct char_data* ch, struct char_data* vict);
void stop_fighting(struct char_data* ch);
void death_cry(struct char_data* ch);
int SetCharFighting(struct char_data* ch, struct char_data* v);
int SetVictFighting(struct char_data* ch, struct char_data* v);
int DamageTrivia(struct char_data* ch, struct char_data* v, int dam, int type, int location);
void make_corpse(struct char_data* ch, int killedbytype);
void raw_kill(struct char_data* ch,int killedbytype);
void die(struct char_data* ch,int killedbytype, struct char_data* killer);
DamageResult MissileDamage( struct char_data* ch, struct char_data* victim,
							int dam, int attacktype, int location );
DamageResult damage(struct char_data* ch, struct char_data* victim,
					int dam, int attacktype, int location);
int CalcThaco( struct char_data* ch, struct char_data* victim = NULL );
int HitOrMiss(struct char_data* ch, struct char_data* victim, int calc_thaco);
DamageResult hit(struct char_data* ch, struct char_data* victim, int type);
void perform_violence(unsigned long pulse);
struct char_data* FindVictim( struct char_data* ch);
struct char_data* FindAnyVictim( struct char_data* ch);
int PreProcDam(struct char_data* ch, int type, int dam, int classe);
int DamageOneItem( struct char_data* ch, int dam_type, struct obj_data* obj);
void MakeScrap( struct char_data* ch,struct char_data* v,
				struct obj_data* obj);
void DamageAllStuff( struct char_data* ch, int dam_type);
int ItemSave( struct obj_data* i, int dam_type);
int WeaponCheck(struct char_data* ch, struct char_data* v, int type, int dam);
void DamageStuff( struct char_data* v, int type, int dam, int location );
int SkipImmortals(struct char_data* v, int amnt, int attacktype);
struct char_data* FindAnAttacker(struct char_data* ch);
void shoot( struct char_data* ch, struct char_data* victim);
struct char_data* FindMetaVictim( struct char_data* ch);
void NailThisSucker( struct char_data* ch);

char_data* Fighting( char_data* pChar );

// FLYP 20180221: moved the keech calculation to a function
int leechResult(struct char_data* ch, int dam);
int canLeech(struct char_data* ch, struct char_data* victim);

#endif
