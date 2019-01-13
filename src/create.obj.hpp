/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __CREATE_OBJ_HPP
#define __CREATE_OBJ_HPP
namespace Alarmud {
void ChangeAffectMod(struct char_data* ch, const char* arg, int type) ;
void ChangeObjAffect(struct char_data* ch, const char* arg, int type) ;
void ChangeObjAffects(struct char_data* ch, const char* arg, int type) ;
void ChangeObjCost(struct char_data* ch, const char* arg, int type) ;
void ChangeObjDesc(struct char_data* ch, const char* arg, int type) ;
void ChangeObjFlags(struct char_data* ch, const char* arg, int type) ;
void ChangeObjName(struct char_data* ch, const char* arg, int type) ;
void ChangeObjPrice(struct char_data* ch, const char* arg, int type) ;
void ChangeObjShort(struct char_data* ch, const char* arg, int type) ;
void ChangeObjSpecial(struct char_data* ch, const char* arg, int type) ;
void ChangeObjType(struct char_data* ch, const char* arg, int type) ;
void ChangeObjValue(struct char_data* ch, const char* arg, int type) ;
void ChangeObjValues(struct char_data* ch, const char* arg, int type) ;
void ChangeObjWear(struct char_data* ch, const char* arg, int type) ;
void ChangeObjWeight(struct char_data* ch, const char* arg, int type) ;
void ObjEdit(struct char_data* ch, const char* arg) ;
void ObjHitReturn(struct char_data* ch, const char* arg, int type) ;
void UpdateObjMenu(struct char_data* ch) ;
void do_oedit(struct char_data* ch, const char* argument, int cmd) ;
} // namespace Alarmud
#endif // __CREATE_OBJ_HPP

