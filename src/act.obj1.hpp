/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_OBJ1_HPP
#define _ACT_OBJ1_HPP
namespace Alarmud {
ACTION_FUNC(do_drop) ;
ACTION_FUNC(do_get) ;
ACTION_FUNC(do_give) ;
ACTION_FUNC(do_pquest) ;
ACTION_FUNC(do_put) ;
void get(struct char_data* ch, struct obj_data* obj_object,struct obj_data* sub_object);
void get_trophy(struct char_data* ch, struct obj_data* ob) ;
void givexp(struct char_data* ch, struct char_data* victim, int amount);
int newstrlen(const char* p) ;
void truegivexp(struct char_data* ch, struct char_data* victim, int amount) ;
} // namespace Alarmud
#endif // _ACT_OBJ1_HPP

