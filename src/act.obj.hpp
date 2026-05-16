/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _ACT_OBJ_HPP
#define _ACT_OBJ_HPP
#include "act.obj_get.hpp"
#include "act.obj_wear.hpp"
namespace Alarmud {

ACTION_FUNC(do_drop);
ACTION_FUNC(do_get);
ACTION_FUNC(do_give);
ACTION_FUNC(do_pquest);
ACTION_FUNC(do_put);
void get_trophy(struct char_data* ch, struct obj_data* ob);
void givexp(struct char_data* ch, struct char_data* victim, int amount);
int newstrlen(const char* p);
void truegivexp(struct char_data* ch, struct char_data* victim, int amount);

ACTION_FUNC(do_drink);
ACTION_FUNC(do_eat);
ACTION_FUNC(do_grab);
ACTION_FUNC(do_pour);
ACTION_FUNC(do_remove);
ACTION_FUNC(do_sip);
ACTION_FUNC(do_taste);
ACTION_FUNC(do_wield);
void name_from_drinkcon(struct obj_data* obj);
void name_to_drinkcon(struct obj_data* obj, int type);
void weight_change_object(struct obj_data* obj, int weight);

} // namespace Alarmud
#endif // _ACT_OBJ_HPP
