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
#include "protos.hpp"
void do_telepathy(struct char_data* ch, const char* argument, int cmd);
void do_new_say(struct char_data* ch, const char* argument, int cmd);
void do_speak(struct char_data* ch, const char* argument, int cmd);
void do_report(struct char_data* ch, const char* argument, int cmd);
void do_say(struct char_data* ch, const char* argument, int cmd);
void do_shout(struct char_data* ch, const char* argument, int cmd);
void do_auction(struct char_data* ch, const char* argument, int cmd);
void do_gossip(struct char_data* ch, const char* argument, int cmd);
void do_commune(struct char_data* ch, const char* argument, int cmd);
void do_tell(struct char_data* ch, const char* argument, int cmd);
void do_gtell(struct char_data* ch, const char* argument, int cmd);
void do_split(struct char_data* ch, const char* argument, int cmd);
void do_whisper(struct char_data* ch, const char* argument, int cmd);
void do_ask(struct char_data* ch, const char* argument, int cmd);
void do_write(struct char_data* ch, const char* argument, int cmd);
void do_pray(struct char_data* ch, const char* argument, int cmd);
char* RandomWord();
void do_sign(struct char_data* ch, const char* argument, int cmd);
void do_eavesdrop(struct char_data* ch, const char* argument, int cmd);

void thief_listen(struct char_data* ch,struct char_data* victim,
				  char* frase,int cmd);
char* scrambler(struct char_data* ch, char* message);
char RandomChar();





#endif /* SRC_ACT_COMM_HPP_ */
