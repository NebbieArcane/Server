#ifndef __REGEN_HPP
#define __REGEN_HPP
#include "events.hpp"
EVENTFUNC(points_event) ;
void alter_hit(struct char_data* ch, int amount) ;
void alter_mana(struct char_data* ch, int amount) ;
void alter_move(struct char_data* ch, int amount) ;
#endif // __REGEN_HPP
