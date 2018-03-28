/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __CREATE_MOB_HPP
#define __CREATE_MOB_HPP
namespace Alarmud {
void ChangeMobActFlags(struct char_data* ch, const char* arg, int type) ;
void ChangeMobAffFlags(struct char_data* ch, const char* arg, int type) ;
void ChangeMobAlign(struct char_data* ch, const char* arg, int type) ;
void ChangeMobArmor(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDamnumb(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDamplus(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDamsize(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDesc(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDpos(struct char_data* ch, const char* arg, int type) ;
void ChangeMobDsound(struct char_data* ch, const char* arg, int type) ;
void ChangeMobExp(struct char_data* ch, const char* arg, int type) ;
void ChangeMobHitp(struct char_data* ch, const char* arg, int type) ;
void ChangeMobImmune(struct char_data* ch, const char* arg, int type) ;
void ChangeMobLong(struct char_data* ch, const char* arg, int type) ;
void ChangeMobMultatt(struct char_data* ch, const char* arg, int type) ;
void ChangeMobName(struct char_data* ch, const char* arg, int type) ;
void ChangeMobRace(struct char_data* ch, const char* arg, int type) ;
void ChangeMobResist(struct char_data* ch, const char* arg, int type) ;
void ChangeMobShort(struct char_data* ch, const char* arg, int type) ;
void ChangeMobSound(struct char_data* ch, const char* arg, int type) ;
void ChangeMobSuscep(struct char_data* ch, const char* arg, int type) ;
void MobEdit(struct char_data* ch, const char* arg) ;
void MobHitReturn(struct char_data* ch, const char* arg, int type) ;
void UpdateMobMenu(struct char_data* ch) ;
void do_medit(struct char_data* ch, const char* argument, int cmd) ;
} // namespace Alarmud
#endif // __CREATE_MOB_HPP

