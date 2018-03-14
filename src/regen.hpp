/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __REGEN_HPP
#define __REGEN_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "events.hpp"
namespace Alarmud {
EVENTFUNC(points_event) ;
void alter_hit(struct char_data* ch, int amount) ;
void alter_mana(struct char_data* ch, int amount) ;
void alter_move(struct char_data* ch, int amount) ;
} // namespace Alarmud
#endif // __REGEN_HPP

