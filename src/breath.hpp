/****************************************************************************
 * BenemMUD. Server per MUD.
 * $Id: breath.h,v 1.2 2002/02/13 12:30:57 root Exp $
 * Files: breath.c breath.h
 * Contengono le funzioni per la gestione dei soffi dei draghi.
 * */

#ifndef __BREATH_HPP
#define __BREATH_HPP
#include "config.hpp"
struct breath_victim {
	struct char_data* ch;
	int yesno; /* 1 0 */
	struct breath_victim* next;
};


typedef void (*bfuncp)( char, struct char_data*, char*, int,
						struct char_data*, struct obj_data* );

struct breather {
	int vnum;
	int cost;
	bfuncp* breaths;
};

extern bfuncp bweapons[];
int BreathWeapon( struct char_data* ch, int cmd, char* arg,struct char_data* mob, int type) ;
void breath_weapon( struct char_data* ch, struct char_data* target,int mana_cost, bfuncp) ;
struct breath_victim* choose_victims(struct char_data* ch,struct char_data* first_victim) ;
void do_breath(struct char_data* ch, char* argument, int cmd) ;
void free_victims(struct breath_victim* head) ;
void use_breath_weapon( struct char_data* ch, struct char_data* target,int cost, bfuncp) ;
#endif // __BREATH_HPP
