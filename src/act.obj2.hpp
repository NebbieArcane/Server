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
ACTION_FUNC(do_drink) ;
ACTION_FUNC(do_eat) ;
ACTION_FUNC(do_grab) ;
ACTION_FUNC(do_pour) ;
ACTION_FUNC(do_remove) ;
ACTION_FUNC(do_sip) ;
ACTION_FUNC(do_taste) ;
ACTION_FUNC(do_wear) ;
ACTION_FUNC(do_wield) ;
void name_from_drinkcon(struct obj_data* obj) ;
void name_to_drinkcon(struct obj_data* obj,int type) ;
void perform_wear(struct char_data* ch, struct obj_data* obj_object,long keyword);
void wear(struct char_data* ch, struct obj_data* obj_object, long keyword) ;
void weight_change_object(struct obj_data* obj, int weight) ;
} // namespace Alarmud
#endif // _ACT_OBJ2_HPP

