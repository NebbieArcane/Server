/*$Id: events.c,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/* ************************************************************************
*  File: events.c                                                         *
*                                                                         *
*  Usage: Contains routines to handle events                              *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Changed event_process to check return value on the    *
*                   event function.  If > 0, reenqueue to expire in       *
*                   retval time.                                          *
*                   Added check in event_cancel to make sure event_obj    *
*                   is non-NULL.                                          *
*                   Moved struct event definition from events.h.          *
************************************************************************ */


#include "events.hpp"

#include <stdlib.h>

#include "hash.hpp"
#include "queue.hpp"
#include "snew.hpp"
#include "structs.hpp"
#include "utility.hpp"
#include "utils.hpp"


struct event {
	EVENTFUNC(*func);
	void* event_obj;
	struct q_element* q_el;
};


struct queue* event_q;          /* the event queue */


/* external variables */
extern unsigned long pulse;
extern int hit_gain(struct char_data* ch);
extern int move_gain(struct char_data* ch);
extern int mana_limit(struct char_data* ch);
extern int hit_limit(struct char_data* ch);
extern int move_limit(struct char_data* ch);
extern int mana_gain(struct char_data* ch);

/* initializes the event queue */
void event_init(void) {
#ifndef NOEVENTS
	event_q = queue_init();
#endif
}


/* creates an event and returns it */
struct event* event_create(EVENTFUNC(*func), void* event_obj, long when) {
	struct event* new_event;

	if (when < 1) /* make sure its in the future */
	{ when = 1; }

	CREATE(new_event, struct event, 1);
	new_event->func = func;
	new_event->event_obj = event_obj;
	new_event->q_el = queue_enq(event_q, new_event, when + pulse);

	return new_event;
}


/* removes the event from the system */
void event_cancel(struct event* event) {
#ifndef NOEVENTS
	if (!event) {
		mudlog(LOG_SYSERR,"Attempted to cancel a NULL event"); // Gaia 2001
		return;
	}
	MARK;
	queue_deq(event_q, event->q_el);
	MARK;
	if (event->event_obj)
	{ free(event->event_obj); }
	if (event)
	{ free(event); }
#endif
	return;
}


/* Process any events whose time has come. */
void event_process(void) {
	struct event* the_event;
#ifndef NOEVENTS
	while ((long) pulse >= queue_key(event_q)) {
		if (!(the_event = (struct event*) queue_head(event_q))) {
			mudlog(LOG_SYSERR,"Attempt to get a NULL event");
			return;
		}

		(the_event->func)(the_event->event_obj);
		if (the_event)
		{ free(the_event); }
	}
#endif
}


/* returns the time remaining before the event */
long event_time(struct event* event) {
	long when;
#ifndef NOEVENTS
	when = queue_elmt_key(event->q_el);
#else
	when=pulse;
#endif
	return (when - pulse);

}


/* frees all events in the queue */
void event_free_all(void) {
	struct event* the_event;
#ifndef NOEVENTS
	while ((the_event = (struct event*) queue_head(event_q))) {
		if (the_event->event_obj)
		{ free(the_event->event_obj); }
		if (the_event)
		{ free(the_event); }
	}

	queue_free(event_q);
#endif
}
