/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
* $Id: opinion.c,v 1.1.1.1 2002/02/13 11:14:54 root Exp $
 *  AlarMUD v2.0
 * See license.doc for distribution terms.
*/
/***************************  System  include ************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "opinion.hpp"
#include "comm.hpp"
#include "db.hpp"

namespace Alarmud {



/*
*/


void FreeHates( struct char_data* ch) {
	struct char_list* k, *n;

	for (k=ch->hates.clist; k; k = n) {
		n = k->next;
		free( k );
	}

	ch->hates.clist = NULL;
	REMOVE_BIT( ch->hatefield, HATE_CHAR );
	if( !ch->hatefield )
	{ REMOVE_BIT( ch->specials.act, ACT_HATEFUL ); }
}


void FreeFears( struct char_data* ch) {
	struct char_list* k, *n;

	for (k=ch->fears.clist; k; k = n) {
		n = k->next;
		free( k );
	}

	ch->fears.clist = NULL;
	REMOVE_BIT(ch->fearfield, FEAR_CHAR);
	if (!ch->fearfield)
	{ REMOVE_BIT(ch->specials.act, ACT_AFRAID); }
}


int RemHated( struct char_data* ch, struct char_data* pud) {
	struct char_list* t, **ppOp;

	if( IS_PC( ch ) ) {
		return FALSE;
	}

	if (pud) {
		for( ppOp = &ch->hates.clist; *ppOp; ) {
			if( (*ppOp)->op_ch ) {
				if( (*ppOp)->op_ch == pud ) {
					if (IS_IMMORTAL(pud)) {
						send_to_char("$c0004Qualcuno smette di odiarti.\n\r",pud);
					}
					t = *ppOp;
					*ppOp = (*ppOp)->next;
					free(t);
					continue;
				}
			}
			else {
				if( !strcmp( (*ppOp)->name, GET_NAME( pud ) ) ) {
					if (IS_IMMORTAL(pud)) {
						send_to_char("$c0004Qualcuno smette di odiarti.\n\r",pud);
					}
					t = *ppOp;
					*ppOp = (*ppOp)->next;
					free(t);
					continue;
				}
			}
			ppOp = &(*ppOp)->next;
		}
	}

	if( !ch->hates.clist )
	{ REMOVE_BIT( ch->hatefield, HATE_CHAR ); }
	if( !ch->hatefield )
	{ REMOVE_BIT( ch->specials.act, ACT_HATEFUL ); }

	return( (pud) ? TRUE : FALSE);
}



int AddHated( struct char_data* ch, struct char_data* pud) {
	struct char_list* newpud;

	if (ch == pud)
	{ return(FALSE); }

	if( IS_PC( ch ) ) {
		return FALSE;
	}

	if (pud) {
		if (!CAN_SEE(ch, pud))
		{ return(FALSE); }

		CREATE(newpud, struct char_list, 1);
		newpud->op_ch = pud;
		strcpy(newpud->name, GET_NAME(pud));
		newpud->next = ch->hates.clist;
		ch->hates.clist = newpud;
		if (!IS_SET(ch->specials.act, ACT_HATEFUL))
		{ SET_BIT(ch->specials.act, ACT_HATEFUL); }
		if (!IS_SET(ch->hatefield, HATE_CHAR))
		{ SET_BIT(ch->hatefield, HATE_CHAR); }

		if (pud->in_room != ch->in_room) {
			/* log("setting hunt because mob was not in same as attacker"); */
			SetHunting(ch,pud);
		}

		if (IS_IMMORTAL(pud)) {
			send_to_char("$c0004Qualcuno ti sta odiando.\n\r",pud);
		}
	}
	return( (pud) ? TRUE : FALSE );
}

int AddHatred( struct char_data* ch, int parm_type, int parm) {
	if( IS_PC( ch ) ) {
		return FALSE;
	}

	switch(parm_type) {
	case OP_SEX :
		SET_BIT(ch->hatefield, HATE_SEX);
		ch->hates.sex = parm;
		break;
	case OP_RACE:
		SET_BIT(ch->hatefield, HATE_RACE);
		ch->hates.race = parm;
		break;
	case OP_GOOD:
		SET_BIT(ch->hatefield, HATE_GOOD);
		ch->hates.good = parm;
		break;
	case OP_EVIL:
		SET_BIT(ch->hatefield, HATE_EVIL);
		ch->hates.evil = parm;
		break;
	case OP_CLASS:
		SET_BIT(ch->hatefield, HATE_CLASS);
		ch->hates.iClass = parm;
		break;
	case OP_VNUM:
		SET_BIT(ch->hatefield, HATE_VNUM);
		ch->hates.vnum = parm;
		break;
	default:
		mudlog( LOG_ERROR, "Invaild parm type in AddHatred (Opinion.c)" );
		return FALSE;
		break;
	}
	SET_BIT(ch->specials.act, ACT_HATEFUL);
	return TRUE;
}

void RemHatred( struct char_data* ch, unsigned short bitv) {
	if( IS_PC( ch ) ) {
		return;
	}

	REMOVE_BIT(ch->hatefield, bitv);
	if (!ch->hatefield)
	{ REMOVE_BIT(ch->specials.act, ACT_HATEFUL); }
}


int Hates( struct char_data* ch, struct char_data* v) {
	struct char_list* i;

	if (IS_AFFECTED(ch, AFF_PARALYSIS))
	{ return(FALSE); }

	if (ch == v)
	{ return(FALSE); }

	if (IS_SET(ch->hatefield, HATE_CHAR)) {
		if (ch->hates.clist) {
			for (i = ch->hates.clist; i; i = i->next) {
				if (i->op_ch) {
					if ((i->op_ch == v) &&
							(!strcmp(i->name, GET_NAME(v))))
					{ return(TRUE); }
				}
				else {
					if (!strcmp(i->name, GET_NAME(v)))
					{ return(TRUE); }
				}
			}
		}
	}
	if (IS_SET(ch->hatefield, HATE_RACE)) {
		if (ch->hates.race != -1) {
			if (ch->hates.race == GET_RACE(v)) {
				char buf[256];
				snprintf(buf, 255, "Odi la razza %s\n\r", RaceName[GET_RACE(v)]);
				send_to_char(buf, ch);
				return(TRUE);
			}
		}
	}

	if (IS_SET(ch->hatefield, HATE_SEX)) {
		if (ch->hates.sex == GET_SEX(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->hatefield, HATE_GOOD)) {
		if (ch->hates.good < GET_ALIGNMENT(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->hatefield, HATE_EVIL)) {
		if (ch->hates.evil > GET_ALIGNMENT(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->hatefield, HATE_CLASS)) {
		if (HasClass(v, ch->hates.iClass)) {
			return(TRUE);
		}
	}
	if (IS_SET(ch->hatefield, HATE_VNUM)) {
		if (ch->hates.vnum == mob_index[v->nr].iVNum)
		{ return(TRUE); }
	}
	return(FALSE);
}

int Fears( struct char_data* ch, struct char_data* v) {
	struct char_list* i;

	if( IS_AFFECTED( ch, AFF_PARALYSIS ) )
	{ return( FALSE ); }

	if( !IS_SET( ch->specials.act, ACT_AFRAID ) )
	{ return( FALSE ); }

	if( IS_SET( ch->fearfield, FEAR_CHAR ) ) {
		if( ch->fears.clist ) {
			for( i = ch->fears.clist; i; i = i->next ) {
				if( i ) {
					if( i->op_ch ) {
						if( i->name[0] != '\0' ) {
							if( i->op_ch == v && strcmp( i->name, GET_NAME( v ) ) == 0 )
							{ return TRUE; }
						}
						else {
							/* lets see if this clears the problem */
							mudlog( LOG_ERROR, "NULL name in ch->fears.clist" );
							RemFeared(ch, i->op_ch);
						}
					}
					else {
						if (i->name[0] != '\0') {
							if (!strcmp(i->name, GET_NAME(v)))
							{ return(TRUE); }
						}
					}
				}
			}
		}
	}
	if (IS_SET(ch->fearfield, FEAR_RACE)) {
		if (ch->fears.race != -1) {
			if (ch->fears.race == GET_RACE(v))
			{ return(TRUE); }
		}
	}
	if (IS_SET(ch->fearfield, FEAR_SEX)) {
		if (ch->fears.sex == GET_SEX(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->fearfield, FEAR_GOOD)) {
		if (ch->fears.good < GET_ALIGNMENT(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->fearfield, FEAR_EVIL)) {
		if (ch->fears.evil > GET_ALIGNMENT(v))
		{ return(TRUE); }
	}
	if (IS_SET(ch->fearfield, FEAR_CLASS)) {
		if (HasClass(v, ch->hates.iClass)) {
			return(TRUE);
		}
	}
	if (IS_SET(ch->fearfield, FEAR_VNUM)) {
		if (ch->fears.vnum == mob_index[v->nr].iVNum)
		{ return(TRUE); }
	}
	return(FALSE);
}

int RemFeared( struct char_data* ch, struct char_data* pud) {

	struct char_list* t, **ppOp;

	if( IS_PC( ch ) ) {
		return FALSE;
	}

	if (!IS_SET(ch->specials.act, ACT_AFRAID))
	{ return(FALSE); }

	if( pud ) {
		for( ppOp = &ch->fears.clist; *ppOp; ) {
			if( (*ppOp)->op_ch ) {
				if( (*ppOp)->op_ch == pud ) {
					if (IS_IMMORTAL(pud)) {
						send_to_char("$c0004Qualcuno smette di temerti.\n\r",pud);
					}
					t = *ppOp;
					*ppOp = (*ppOp)->next;
					free(t);
					continue;
				}
			}
			else {
				if( !strcmp( (*ppOp)->name, GET_NAME( pud ) ) ) {
					if (IS_IMMORTAL(pud)) {
						send_to_char("$c0004Qualcuno smette di temerti.\n\r",pud);
					}
					t = *ppOp;
					*ppOp = (*ppOp)->next;
					free(t);
					continue;
				}
			}
			ppOp = &(*ppOp)->next;
		}
	}
	if (!ch->fears.clist)
	{ REMOVE_BIT(ch->fearfield, FEAR_CHAR); }
	if (!ch->fearfield)
	{ REMOVE_BIT(ch->specials.act, ACT_AFRAID); }
	return( (pud) ? TRUE : FALSE);
}



int AddFeared( struct char_data* ch, struct char_data* pud) {

	struct char_list* newpud;

	if( IS_PC( ch ) ) {
		return FALSE;
	}

	if (pud) {

		if (!CAN_SEE(ch, pud))
		{ return(FALSE); }

		CREATE(newpud, struct char_list, 1);
		newpud->op_ch = pud;
		strcpy(newpud->name,GET_NAME(pud));
		newpud->next = ch->fears.clist;
		ch->fears.clist = newpud;

		if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
			SET_BIT(ch->specials.act, ACT_AFRAID);
		}
		if (!IS_SET(ch->fearfield, FEAR_CHAR)) {
			SET_BIT(ch->fearfield, FEAR_CHAR);
		}
		if (IS_IMMORTAL(pud))
		{ send_to_char("$c0004Qualcuno ti teme (giustamente).\n\r",pud); }
	}

	return( (pud) ? TRUE : FALSE);
}


int AddFears( struct char_data* ch, int parm_type, int parm) {
	if( IS_PC( ch ) ) {
		return FALSE;
	}

	switch(parm_type) {
	case OP_SEX :
		SET_BIT(ch->fearfield, FEAR_SEX);
		ch->fears.sex = parm;
		break;
	case OP_RACE:
		SET_BIT(ch->fearfield, FEAR_RACE);
		ch->fears.race = parm;
		break;
	case OP_GOOD:
		SET_BIT(ch->fearfield, FEAR_GOOD);
		ch->fears.good = parm;
		break;
	case OP_EVIL:
		SET_BIT(ch->fearfield, FEAR_EVIL);
		ch->fears.evil = parm;
		break;
	case OP_CLASS:
		SET_BIT(ch->fearfield, FEAR_CLASS);
		ch->fears.iClass = parm;
		break;
	case OP_VNUM:
		SET_BIT(ch->fearfield, FEAR_VNUM);
		ch->fears.vnum = parm;
		break;
	default:
		mudlog( LOG_ERROR, "Invaild parm type in AddFears (Opinion.c)" );
		return FALSE;
		break;
	}
	SET_BIT(ch->specials.act, ACT_AFRAID);
	return TRUE;
}


struct char_data* FindAHatee( struct char_data* ch) {
	struct char_data* tmp_ch;

	if (ch->in_room < 0)
	{ return(0); }

	for( tmp_ch=real_roomp(ch->in_room)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room) {
		if (Hates(ch, tmp_ch) && (CAN_SEE(ch, tmp_ch))) {
			if (ch->in_room == tmp_ch->in_room) {
				if (ch != tmp_ch) {
					return(tmp_ch);
				}
				else {
					RemHated(ch,tmp_ch);
					return(0);
				}
			}
		}
	}
	return(0);
}

struct char_data* FindAFearee( struct char_data* ch) {
	struct char_data* tmp_ch;

	if (ch->in_room < 0)
	{ return(0); }

	for( tmp_ch=real_roomp(ch->in_room)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room) {
		if (Fears(ch, tmp_ch) && (CAN_SEE(ch, tmp_ch))) {
			if( (ch->in_room == tmp_ch->in_room) && (ch != tmp_ch)) {
				return(tmp_ch);
			}
		}
	}
	return(0);
}


/*
 * these two procedures zero out the character pointer
 * for quiting players, without removing names
 * thus the monsters will still hate them
 */


void ZeroHatred(struct char_data* ch, struct char_data* v) {
	struct char_list* oldpud;

	for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
		if (oldpud) {
			if (oldpud->op_ch) {
				if (oldpud->op_ch == v) {
					oldpud->op_ch = 0;
				}
			}
		}
	}
}


void ZeroFeared(struct char_data* ch, struct char_data* v) {
	struct char_list* oldpud;

	for (oldpud = ch->fears.clist; oldpud; oldpud = oldpud->next) {
		if (oldpud) {
			if (oldpud->op_ch) {
				if (oldpud->op_ch == v) {
					oldpud->op_ch = 0;
				}
			}
		}
	}
}


/*
  these two are to make the monsters completely forget about them.
*/
void DeleteHatreds(struct char_data* ch) {
	struct char_data* i;

	for (i = character_list; i; i = i->next) {
		if (Hates(i, ch))
		{ RemHated(i, ch); }
	}
}


void DeleteFears(struct char_data* ch) {
	struct char_data* i;

	for (i = character_list; i; i = i->next) {
		if (Fears(i, ch))
		{ RemFeared(i, ch); }
	}
}
} // namespace Alarmud

