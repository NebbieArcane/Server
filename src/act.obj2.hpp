/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_OBJ2_HPP
#define _ACT_OBJ2_HPP
namespace Alarmud {
int IsRestricted(struct obj_data* obj, int Class) ;
void do_drink(struct char_data* ch,const char* argument, int cmd) ;
void do_eat(struct char_data* ch,const char* argument, int cmd) ;
void do_grab(struct char_data* ch,const char* argument, int cmd) ;
void do_pour(struct char_data* ch,const char* argument, int cmd) ;
void do_remove(struct char_data* ch,const char* argument, int cmd) ;
void do_sip(struct char_data* ch,const char* argument, int cmd) ;
void do_taste(struct char_data* ch,const char* argument, int cmd) ;
void do_wear(struct char_data* ch,const char* argument, int cmd) ;
void do_wield(struct char_data* ch,const char* argument, int cmd) ;
void name_from_drinkcon(struct obj_data* obj) ;
void name_to_drinkcon(struct obj_data* obj,int type) ;
void perform_wear( struct char_data* ch, struct obj_data* obj_object,long keyword);
void wear(struct char_data* ch, struct obj_data* obj_object, long keyword) ;
void weight_change_object(struct obj_data* obj, int weight) ;
} // namespace Alarmud
#endif // _ACT_OBJ2_HPP

