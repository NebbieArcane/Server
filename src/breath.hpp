/****************************************************************************
 * BenemMUD. Server per MUD.
 * $Id: breath.h,v 1.2 2002/02/13 12:30:57 root Exp $
 * Files: breath.c breath.h
 * Contengono le funzioni per la gestione dei soffi dei draghi.
 * */

#if !defined( _BREATH_H )


typedef void (*bfuncp)( char, struct char_data*, char*, int,
						struct char_data*, struct obj_data* );

struct breather {
	int vnum;
	int cost;
	bfuncp* breaths;
};

extern bfuncp bweapons[];

int BreathWeapon( struct char_data* ch, int cmd, char* arg,
				  struct char_data* mob, int type);

void do_breath( struct char_data* ch, char* argument, int cmd );

#define _BREATH_H
#endif
