/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_MOVE_HPP
#define _ACT_MOVE_HPP
namespace Alarmud {
int AddToCharHeap(struct char_data* heap[50], int* top, int total[50],struct char_data* k) ;
void DisplayGroupMove(struct char_data* ch, int dir, int was_in, int total) ;
void DisplayMove(struct char_data* ch, int dir, int was_in, int total) ;
void DisplayOneMove(struct char_data* ch, int dir, int was_in) ;
void MoveGroup(struct char_data* ch, int dir) ;
int MoveOne(struct char_data* ch, int dir, int bCheckSpecial) ;
void NotLegalMove(struct char_data* ch) ;
int RawMove(struct char_data* ch, int dir, int bCheckSpecial) ;
int ValidMove(struct char_data* ch, int cmd) ;
int canDig(struct char_data* ch) ;
int canScythe(struct char_data* ch) ;
ACTION_FUNC(do_close) ;
ACTION_FUNC(do_enter) ;
ACTION_FUNC(do_follow) ;
ACTION_FUNC(do_leave) ;
ACTION_FUNC(do_lock) ;
ACTION_FUNC(do_move) ;
ACTION_FUNC(do_open) ;
ACTION_FUNC(do_open_exit) ;
ACTION_FUNC(do_pick) ;
ACTION_FUNC(do_rest) ;
ACTION_FUNC(do_run) ;
ACTION_FUNC(do_sit) ;
ACTION_FUNC(do_sleep) ;
ACTION_FUNC(do_stand) ;
ACTION_FUNC(do_unlock) ;
ACTION_FUNC(do_wake) ;
int find_door(struct char_data* ch, char* type, char* dir) ;
int has_key(struct char_data* ch, int key) ;
void open_door(struct char_data* ch, int dir) ;
void raw_lock_door(struct char_data* ch,struct room_direction_data* exitp, int door);
void raw_open_door(struct char_data* ch, int dir) ;
void raw_unlock_door(struct char_data* ch, struct room_direction_data* exitp, int door);
} // namespace Alarmud
#endif // _ACT_MOVE_HPP

