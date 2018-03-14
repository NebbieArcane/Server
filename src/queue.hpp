/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __QUEUE_HPP
#define __QUEUE_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* ************************************************************************
*  File: queue.h                                                          *
*                                                                         *
*  Usage: structures and prototypes for queues                            *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Moved defines and structs to queue.c.                 *
************************************************************************ */

struct queue* queue_init(void);
struct q_element* queue_enq(struct queue* q, void* data, long key);
void queue_deq(struct queue* q, struct q_element* qe);
void* queue_head(struct queue* q);
long queue_key(struct queue* q);
long queue_elmt_key(struct q_element* qe);
void queue_free(struct queue* q);
void queue_deq(struct queue* q, struct q_element* qe) ;
long queue_elmt_key(struct q_element* qe) ;
struct q_element* queue_enq(struct queue* q, void* data, long key) ;
void queue_free(struct queue* q) ;
void* queue_head(struct queue* q) ;
struct queue* queue_init(void) ;
long queue_key(struct queue* q) ;
} // namespace Alarmud
#endif // __QUEUE_HPP

