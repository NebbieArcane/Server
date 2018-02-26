#ifndef __CREATE_MOB_HPP
#define __CREATE_MOB_HPP
#include "config.hpp"
void ChangeMobActFlags(struct char_data* ch, char* arg, int type) ;
void ChangeMobAffFlags(struct char_data* ch, char* arg, int type) ;
void ChangeMobAlign(struct char_data* ch, char* arg, int type) ;
void ChangeMobArmor(struct char_data* ch, char* arg, int type) ;
void ChangeMobDamnumb(struct char_data* ch, char* arg, int type) ;
void ChangeMobDamplus(struct char_data* ch, char* arg, int type) ;
void ChangeMobDamsize(struct char_data* ch, char* arg, int type) ;
void ChangeMobDesc(struct char_data* ch, char* arg, int type) ;
void ChangeMobDpos(struct char_data* ch, char* arg, int type) ;
void ChangeMobDsound(struct char_data* ch, char* arg, int type) ;
void ChangeMobExp(struct char_data* ch, char* arg, int type) ;
void ChangeMobHitp(struct char_data* ch, char* arg, int type) ;
void ChangeMobImmune(struct char_data* ch, char* arg, int type) ;
void ChangeMobLong(struct char_data* ch, char* arg, int type) ;
void ChangeMobMultatt(struct char_data* ch, char* arg, int type) ;
void ChangeMobName(struct char_data* ch, char* arg, int type) ;
void ChangeMobRace(struct char_data* ch, char* arg, int type) ;
void ChangeMobResist(struct char_data* ch, char* arg, int type) ;
void ChangeMobShort(struct char_data* ch, char* arg, int type) ;
void ChangeMobSound(struct char_data* ch, char* arg, int type) ;
void ChangeMobSuscep(struct char_data* ch, char* arg, int type) ;
void MobEdit(struct char_data* ch, char* arg) ;
void MobHitReturn(struct char_data* ch, char* arg, int type) ;
void UpdateMobMenu(struct char_data* ch) ;
void do_medit(struct char_data* ch, char* argument, int cmd) ;
#endif // __CREATE_MOB_HPP
