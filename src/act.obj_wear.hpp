/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _ACT_OBJ_WEAR_HPP
#define _ACT_OBJ_WEAR_HPP
#include "act.obj_wear_infer.hpp"
namespace Alarmud {

struct char_data;
struct obj_data;

int obj_infer_wear_keyword(const struct obj_data* obj);
int obj_infer_wear_equip_pos(const struct obj_data* obj);

int IsRestricted(struct obj_data* obj, int Class);
void perform_wear(struct char_data* ch, struct obj_data* obj_object, long keyword);
void wear(struct char_data* ch, struct obj_data* obj_object, long keyword);
ACTION_FUNC(do_wear);

} // namespace Alarmud
#endif // _ACT_OBJ_WEAR_HPP
