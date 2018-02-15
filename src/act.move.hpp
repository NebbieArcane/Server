/*
 * act.move.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_MOVE_HPP_
#define SRC_ACT_MOVE_HPP_
int AddToCharHeap(struct char_data* heap[50], int* top, int total[50], struct char_data* k);
int canDig(struct char_data* ch );
int canScythe(struct char_data* ch );
void DisplayGroupMove(struct char_data* ch, int dir, int was_in, int total);
void DisplayMove(struct char_data* ch, int dir, int was_in, int total);
void DisplayOneMove(struct char_data* ch, int dir, int was_in);
void do_close(struct char_data* ch, const char* argument, int cmd);
void do_enter(struct char_data* ch, const char* argument, int cmd);
void do_follow(struct char_data* ch, const char* argument, int cmd);
void do_leave(struct char_data* ch, const char* argument, int cmd);
void do_lock(struct char_data* ch, const char* argument, int cmd);
void do_move(struct char_data* ch, const char* argument, int cmd);
void do_open_exit(struct char_data* ch, const char* argument, int cmd);
void do_open(struct char_data* ch, const char* argument, int cmd);
void do_pick(struct char_data* ch, const char* argument, int cmd);
void do_rest(struct char_data* ch, const char* argument, int cmd);
void do_run(struct char_data* ch, const char* argument, int cmd);
void do_sit(struct char_data* ch, const char* argument, int cmd);
void do_sleep(struct char_data* ch, const char* argument, int cmd);
void do_stand(struct char_data* ch, const char* argument, int cmd);
void do_unlock(struct char_data* ch, const char* argument, int cmd);
void do_wake(struct char_data* ch, const char* argument, int cmd);
int find_door(struct char_data* ch, char* type, char* dir);
int has_key(struct char_data* ch, int key);
void MoveGroup(struct char_data* ch, int dir);
int MoveOne(struct char_data* ch, int dir, int bCheckSpecial );
void NotLegalMove(struct char_data* ch);
void open_door(struct char_data* ch, int dir);
void raw_lock_door(struct char_data* ch, struct room_direction_data* exitp, int door);
void raw_open_door(struct char_data* ch, int dir);
void raw_unlock_door(struct char_data* ch, struct room_direction_data* exitp, int door);
int RawMove(struct char_data* ch, int dir, int bCheckSpecial );
int ValidMove(struct char_data* ch, int cmd);




#endif /* SRC_ACT_MOVE_HPP_ */
