/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/****************************************************************************
 * BenemMUD. Server per MUD.
 * $Id: breath.h,v 1.2 2002/02/13 12:30:57 root Exp $
 * Files: breath.c breath.h
 * Contengono le funzioni per la gestione dei soffi dei draghi.
 * */
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __BREATH_HPP
#define __BREATH_HPP
#include "typedefs.hpp"
namespace Alarmud {

struct breath_victim {
	struct char_data* ch;
	int yesno; /* 1 0 */
	struct breath_victim* next;
};



struct breather {
	int vnum;
	int cost;
	breath_func* breaths;
};

extern breath_func bweapons[];
MOBSPECIAL_FUNC(BreathWeapon) ;
void breath_weapon(struct char_data* ch, struct char_data* target,int mana_cost, breath_func);
struct breath_victim* choose_victims(struct char_data* ch,struct char_data* first_victim) ;
ACTION_FUNC(do_breath) ;
void free_victims(struct breath_victim* head) ;
void use_breath_weapon(struct char_data* ch, struct char_data* target,int cost, breath_func) ;
} // namespace Alarmud
#endif // __BREATH_HPP

