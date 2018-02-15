/*
 * act.off.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_OFF_HPP_
#define SRC_ACT_OFF_HPP_
#include "act.wizard.hpp"
#include "act.info.hpp"
void do_weapon_load(struct char_data* ch, char* argument, int cmd);
struct char_data* get_char_near_room_vis(struct char_data* ch, char* name, long next_room);
void do_fire(struct char_data* ch, char* argument, int cmd);
void do_throw(struct char_data* ch, char* argument, int cmd);
void do_hit(struct char_data* ch, char* argument, int cmd);
void do_slay(struct char_data* ch, char* argument, int cmd);
void do_kill(struct char_data* ch, char* argument, int cmd);
void do_backstab(struct char_data* ch, char* argument, int cmd);
void do_order(struct char_data* ch, char* argument, int cmd);
void do_flee(struct char_data* ch, char* argument, int cmd);
void do_bash(struct char_data* ch, char* argument, int cmd);
void do_bodyguard(struct char_data* ch, char* argument, int cmd);
void do_rescue(struct char_data* ch, char* argument, int cmd);
void do_assist(struct char_data* ch, char* argument, int cmd);
void do_support(struct char_data* ch, char* argument, int cmd);
void do_kick(struct char_data* ch, char* argument, int cmd);
void do_wimp(struct char_data* ch, char* argument, int cmd);
void do_shoot(struct char_data* ch, char* argument, int cmd);
void do_springleap(struct char_data* ch, char* argument, int cmd);
void do_quivering_palm( struct char_data* ch, char* arg, int cmd);
void kick_messages(struct char_data* ch, struct char_data* victim, int damage);
int clearpath( struct char_data* ch, long room, int direc );
void do_stopfight( struct char_data* pChar, char* szArgument, int nCmd );




#endif /* SRC_ACT_OFF_HPP_ */
