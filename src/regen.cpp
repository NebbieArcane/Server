/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* ************************************************************************
*  File: regen.c                                                          *
*                                                                         *
*  Usage: Contains routines to handle event based point regeneration      *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
************************************************************************ */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
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
#include "regen.hpp"
#include "fight.hpp"
#include "spells.hpp"

namespace Alarmud {

/*************************************************************************
*                                                                        *
*  The routine has been modified i From the continuos regen version of   *
*   Marco Paglioni 1999          to make a semi-continuos REGEN          *
*  Instead of having a fixed 1-point regen event every TIME pulses       *
*  where TIME is a variable, we now have a variable REG_POINT regen      *
*  event every REGEN_EVENTS_DELAY  time.                                 *
*  This allows negative regen and should correct for several bugs that   *
*  affected the continuos regen. The drawback is that only a few regen   *
*  ranges are considered. The range thickness is defined by              *
*  PULSES_PER_MUD_HOUR/REGEN_EVENTS_DELAY.                               *
*  To account for this on average we put a random increase for the       *
*  remaining fractional part of the regen                                *
*                                                 Gaia 2001              *
************************************************************************ */

void alter_mana(struct char_data* ch, int amount);
void alter_move(struct char_data* ch, int amount);

/* Planer point types for events */
#define REGEN_HIT      0
#define REGEN_MANA     1
#define REGEN_MOVE     2

#define PULSES_PER_MUD_HOUR      (SECS_PER_MUD_HOUR*PULSE_PER_SEC)
#define REGEN_EVENTS_DELAY       50
#define NUMBER_REGEN_EVENTS      (PULSES_PER_MUD_HOUR/REGEN_EVENTS_DELAY)

/* event object structure for point regen */
struct regen_event_obj {
	struct char_data* ch;		/* character regening */
	int type;			/* HIT, MOVE, or MANA */
};


EVENTFUNC(points_event) {
	struct regen_event_obj* regen = (struct regen_event_obj*) event_obj;
	struct char_data* ch;
	struct room_data* rp;
	int time, type, gain, hgain;
	int regainroom=1;
	double fGain;
	double dum ;
	char logbuf[100];
	int rnd, r_mult;

	r_mult = NUMBER_REGEN_EVENTS*10 ;

	time = REGEN_EVENTS_DELAY;
	ch = regen->ch;
	if (ch->nMagicNumber != CHAR_VALID_MAGIC)  {
		if (event_obj) {
			free(event_obj);
		}
		return 0;
	}
	type = regen->type;

	GET_POINTS_EVENT(ch, type) = NULL;

	/*  no help for the dying
	if (GET_HIT(ch) <= HIT_INCAP )
	{
	  if (event_obj)
	    free(event_obj);
	  return 0;
	}   I comment this line as now the regen can be negative.
	    This could solve some problems that arises with poisoned chars
	    Gaia 2001 */

	/* aggiungo controllo per PG rentati o link dead */

	if (!IS_NPC(ch) && !ch->desc ) {
		if (event_obj) {
			free(event_obj);
		}
		return 0;
	}

#ifdef EGO_RARE
	/* Chiamata per il controllo dell'EGO dell'eq raro
	 Gaia 2001 */

	if( IS_SET( ch->specials.act, ACT_POLYSELF) || IS_PC( ch )
			&& !EgoSave( ch ) ) { do_ego_eq_action( ch ); }

#endif
	if (ch->desc) {
		mudlog(LOG_CHECK,"Regen: %s",G::translate(static_cast<e_connection_types>(ch->desc->connected)));
		if (!ch->desc->ch) {
			mudlog(LOG_ERROR,"Player in state %s with no character in descriptor",G::translate(static_cast<e_connection_types>(ch->desc->connected)));
			if (event_obj) {
				free(event_obj);
			}
			return 0;
		}
	}
	mudlog(LOG_CHECK,"RegenGeneric: %s", GET_NAME(ch));

	rp = real_roomp(ch->in_room);

	if (!IS_NPC( ch ) && rp && rp->room_flags&NO_REGAIN)
	{ regainroom=0; }

	/*   In the old version we increment type of points by one.
	     If not at max, reenqueue the event. Now we increase at fixed
	     times by a larger amount of points.
	*/

	switch (type) {
	case REGEN_HIT:

		mudlog(LOG_CHECK,"RegenHit: %s", GET_NAME(ch));

		hgain = hit_gain(ch);
		if( hgain > 0 && regainroom == 0 ) { gain = 0 ; }

		if ( hgain > 0 ) {

			fGain = modf(((double)(hgain)/(double)(NUMBER_REGEN_EVENTS)), &dum );
			fGain = fGain*(double)(r_mult) ;
			gain = (int)( dum );
			rnd =  ( rand()% r_mult) ;
			/* Here we add a line that on average helps to take
			   into account fractional gains */

			if ( rnd < fGain ) { gain++ ; }

			/*     mudlog(LOG_CHECK, "Total hit_gain: %d \n\r", gain); */

			GET_HIT(ch) = MIN( (GET_HIT(ch) +  gain), GET_MAX_HIT(ch));
			if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
				/* reenqueue the event. NOW AT A FIXED TIME */
				mudlog(LOG_CHECK,"Requeue Hits: %s", GET_NAME(ch));
				GET_POINTS_EVENT(ch, REGEN_HIT) =
					event_create(points_event, regen, time);
			}
			else if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
				GET_HIT(ch) = GET_MAX_HIT(ch) ;
			}
		}

		else if (hgain < 0) {

			gain = hgain/NUMBER_REGEN_EVENTS ;

			if( IS_AFFECTED(ch,AFF_POISON)) {
				if( damage( ch, ch, -1*gain, SPELL_POISON, 5 ) == VictimDead )
				{ return 0; }
			}
			else if(IS_AFFECTED2(ch,AFF2_HEAT_STUFF)) {
				if( damage( ch, ch, -1*gain, SPELL_HEAT_STUFF, 5 ) == VictimDead )
				{ return 0; }
			}
			else {
				if( damage( ch, ch, -1*gain, TYPE_SUFFERING, 5 ) == VictimDead )
				{ return 0; }
			}
		}

		else if ( hgain == 0 ) {

			GET_HIT(ch) = MIN( GET_HIT(ch), GET_MAX_HIT(ch) );
			if (GET_HIT(ch) < GET_MAX_HIT(ch) )
				GET_POINTS_EVENT(ch, REGEN_HIT) =
					event_create(points_event, regen, time);
		}
		else if (event_obj) {
			free(event_obj);
		}

		if (GET_POS(ch) <= POSITION_STUNNED)
		{ update_pos(ch); }
		break;

	case REGEN_MANA:
		mudlog(LOG_CHECK,"RegenMana: %s", GET_NAME(ch));

		fGain = modf(((double)(mana_gain(ch))/(double)(NUMBER_REGEN_EVENTS)), &dum );
		fGain = fGain*(double)(r_mult) ;
		gain = (int)( dum );
		rnd =  ( rand()% r_mult) ;
		/* Here we add a line that on average helps to take
		   into account fractional gains */

		if ( rnd < fGain ) { gain++ ; }
		if(gain > 0 && regainroom == 0) { gain = 0 ; }

		GET_MANA(ch) = MIN((GET_MANA(ch) + gain), GET_MAX_MANA(ch));

		if (GET_MANA(ch) < GET_MAX_MANA(ch)) {
			/* reenqueue the event */
			mudlog(LOG_CHECK,"Requeue Mana: %s", GET_NAME(ch));
			GET_POINTS_EVENT(ch, REGEN_MANA) =
				event_create(points_event, regen, time);
		}
		else if (GET_MANA(ch) > GET_MAX_MANA(ch)) {
			GET_MANA(ch) = GET_MAX_MANA(ch) ;
		}
		else if (event_obj) {
			free(event_obj);
		}
		break;

	case REGEN_MOVE:
		mudlog(LOG_CHECK,"RegenMove: %s", GET_NAME(ch));
		fGain = modf(((double)(move_gain(ch))/(double)(NUMBER_REGEN_EVENTS)), &dum );
		fGain = fGain*(double)(r_mult) ;
		gain = (int)( dum );
		rnd =  ( rand()% r_mult) ;
		/* Here we add a line that on average helps to take
		   into account fractional gains */

		if ( rnd < fGain ) { gain++ ; }
		if(gain > 0 && regainroom == 0) { gain = 0 ; }

		GET_MOVE(ch) = MIN((GET_MOVE(ch) + gain), GET_MAX_MOVE(ch));
		if (GET_MOVE(ch) < GET_MAX_MOVE(ch)) {
			/* reenqueue the event */
			mudlog(LOG_CHECK,"Requeue Move: %s", GET_NAME(ch));
			GET_POINTS_EVENT(ch, REGEN_MOVE) =
				event_create(points_event, regen, time);
		}
		else if (GET_MOVE(ch) > GET_MAX_MOVE(ch)) {
			GET_MOVE(ch) = GET_MAX_MOVE(ch) ;
		}
		else if (event_obj) {
			free(event_obj);
		}
		break;

	default:
		sprintf(logbuf, "Unknown points event type %d", type);
		mudlog(LOG_SYSERR,logbuf);
		break;
	}
	/* kill this event
	GET_POINTS_EVENT(ch, type) = NULL;
	free(event_obj);*/
	return 0;
}


/*
 * subtracts amount of hitpoints from ch's current and starts points event
 */
void alter_hit(struct char_data* ch, int amount) {
	struct regen_event_obj* regen;
	long time;
	int gain;
	time = REGEN_EVENTS_DELAY;

	if ( amount > 0) {
		GET_HIT(ch) = GET_HIT(ch) - amount;
	}
	else {
		GET_HIT(ch) = MIN(GET_HIT(ch) - amount, GET_MAX_HIT(ch));
	}
#ifdef NOEVENTS
	return;
#endif
	/*  if (IS_AFFECTED(ch,AFF_POISON) || IS_AFFECTED2(ch,AFF2_HEAT_STUFF))
	     return;
	  if (GET_HIT(ch) <= HIT_INCAP)
	     return; */
	if (GET_HIT(ch) < GET_MAX_HIT(ch) && !GET_POINTS_EVENT(ch, REGEN_HIT) /* &&
      !IS_AFFECTED(ch,AFF_POISON) && !IS_AFFECTED2( ch, AFF2_HEAT_STUFF) */ ) {
		CREATE(regen, struct regen_event_obj, 1);
		regen->ch = ch;
		regen->type = REGEN_HIT;
		GET_POINTS_EVENT(ch, REGEN_HIT) = event_create(points_event, regen, time);
	}
}


/*
 * subtracts amount of mana from ch's current and starts points event
 */
void alter_mana(struct char_data* ch, int amount) {
	struct regen_event_obj* regen;
	long time;
	int gain;
	time = REGEN_EVENTS_DELAY;

	GET_MANA(ch) = MIN(GET_MANA(ch) - amount, GET_MAX_MANA(ch));
#ifdef NOEVENTS
	return;
#endif
	if (!GET_POINTS_EVENT(ch, REGEN_MANA) && (GET_MANA(ch) < GET_MAX_MANA(ch))) {

		/* make sure the character isn't dying */
		if (GET_POS(ch) >= POSITION_STUNNED) {
			CREATE(regen, struct regen_event_obj, 1);
			regen->ch = ch;
			regen->type = REGEN_MANA;
			GET_POINTS_EVENT(ch, REGEN_MANA) = event_create(points_event, regen, time);
		}
	}
}


/*
 * subtracts amount of moves from ch's current and starts points event
 */
void alter_move(struct char_data* ch, int amount) {
	struct regen_event_obj* regen;
	long time;
	int gain;
	time = REGEN_EVENTS_DELAY;

	GET_MOVE(ch) = MIN(GET_MOVE(ch) - amount, GET_MAX_MOVE(ch));
#ifdef NOEVENTS
	return;
#endif

	if (!GET_POINTS_EVENT(ch, REGEN_MOVE) && (GET_MOVE(ch) < GET_MAX_MOVE(ch))) {

		/* make sure the character isn't dying */
		if (GET_POS(ch) >= POSITION_STUNNED) {
			CREATE(regen, struct regen_event_obj, 1);
			regen->ch = ch;
			regen->type = REGEN_MOVE;
			GET_POINTS_EVENT(ch, REGEN_MOVE) = event_create(points_event, regen, time);
		}
	}
}
} // namespace Alarmud

