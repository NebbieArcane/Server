/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _ACT_OBJ_GET_HPP
#define _ACT_OBJ_GET_HPP
namespace Alarmud {

struct char_data;
struct obj_data;

void get(struct char_data* ch, struct obj_data* obj_object, struct obj_data* sub_object);
void obj_run_get(struct char_data* ch, const char* arg, int cmd);

} // namespace Alarmud
#endif // _ACT_OBJ_GET_HPP
