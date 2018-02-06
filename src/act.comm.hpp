/*
 * act.comm.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_COMM_HPP_
#define SRC_ACT_COMM_HPP_
void do_telepathy(struct char_data* ch, char* argument, int cmd);
void do_new_say(struct char_data* ch, char* argument, int cmd);
void do_speak(struct char_data* ch, char* argument, int cmd);
void UpdateScreen(struct char_data* ch, int update);
void InitScreen(struct char_data* ch);
void do_report(struct char_data* ch, char* argument, int cmd);
void do_say(struct char_data* ch, char* argument, int cmd);
void do_shout(struct char_data* ch, char* argument, int cmd);
void do_auction(struct char_data* ch, char* argument, int cmd);
void do_gossip(struct char_data* ch, char* argument, int cmd);
void do_commune(struct char_data* ch, char* argument, int cmd);
void do_tell(struct char_data* ch, char* argument, int cmd);
void do_gtell(struct char_data* ch, char* argument, int cmd);
void do_split(struct char_data* ch, char* argument, int cmd);
void do_whisper(struct char_data* ch, char* argument, int cmd);
void do_ask(struct char_data* ch, char* argument, int cmd);
void do_write(struct char_data* ch, char* argument, int cmd);
void do_pray(struct char_data* ch, char* argument, int cmd);
char* RandomWord();
void do_sign(struct char_data* ch, char* argument, int cmd);
void do_move(struct char_data* ch, char* argument, int cmd);
void do_enter(struct char_data* ch, char* argument, int cmd);
void do_rest(struct char_data* ch, char* argument, int cmd);
void do_stand(struct char_data* ch, char* argument, int cmd);
void do_sit(struct char_data* ch, char* argument, int cmd);
void do_lock(struct char_data* ch, char* argument, int cmd);
void do_unlock(struct char_data* ch, char* argument, int cmd);
void do_pick(struct char_data* ch, char* argument, int cmd);
void do_sleep(struct char_data* ch, char* argument, int cmd);
void do_wake(struct char_data* ch, char* argument, int cmd);
void do_trans(struct char_data* ch, char* argument, int cmd);
void do_follow(struct char_data* ch, char* argument, int cmd);
void do_open(struct char_data* ch, char* argument, int cmd);
void do_close(struct char_data* ch, char* argument, int cmd);
void do_enter(struct char_data* ch, char* argument, int cmd);
void do_leave(struct char_data* ch, char* argument, int cmd);
void do_eavesdrop(struct char_data* ch, char* argument, int cmd);
void RecurseRoom( long lInRoom, int iLevel, int iMaxLevel,
				  unsigned char* achVisitedRooms );
void thief_listen(struct char_data* ch,struct char_data* victim,
				  char* frase,int cmd);
char* scrambler(struct char_data* ch, char* message);
char RandomChar();





#endif /* SRC_ACT_COMM_HPP_ */
