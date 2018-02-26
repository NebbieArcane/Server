#ifndef _ACT_OFF_HPP
#define _ACT_OFF_HPP
#include "config.hpp"
int clearpath(struct char_data* ch, long room, int direc) ;
void do_assist(struct char_data* ch, char* argument, int cmd) ;
void do_associa(struct char_data* ch, char* argument, int cmd) ;
void do_backstab(struct char_data* ch, char* argument, int cmd) ;
void do_bash(struct char_data* ch, char* argument, int cmd) ;
void do_berserk( struct char_data* ch, char* arg, int cmd) ;
void do_bodyguard(struct char_data* ch, char* argument, int cmd) ;
void do_fire(struct char_data* ch, char* argument, int cmd ) ;
void do_flee(struct char_data* ch, char* argument, int cmd) ;
void do_hit(struct char_data* ch, char* argument, int cmd) ;
void do_kick(struct char_data* ch, char* argument, int cmd) ;
void do_kill(struct char_data* ch, char* argument, int cmd) ;
void do_order(struct char_data* ch, char* argument, int cmd) ;
void do_order_old(struct char_data* ch, char* argument, int cmd) ;
void do_parry( struct char_data* ch, char* arg, int cmd) ;
void do_quivering_palm( struct char_data* ch, char* arg, int cmd) ;
void do_rescue(struct char_data* ch, char* argument, int cmd) ;
void do_ripudia(struct char_data* ch, char* argument, int cmd) ;
void do_shoot(struct char_data* ch, char* argument, int cmd) ;
void do_slay(struct char_data* ch, char* argument, int cmd) ;
void do_springleap(struct char_data* ch, char* argument, int cmd) ;
void do_stopfight( struct char_data* pChar, char* szArgument, int nCmd ) ;
void do_support(struct char_data* ch, char* argument, int cmd) ;
void do_throw(struct char_data* ch, char* argument, int cmd) ;
void do_vomita(struct char_data* ch, char* argument, int cmd) ;
void do_weapon_load( struct char_data* ch, char* argument, int cmd ) ;
void do_wimp(struct char_data* ch, char* argument, int cmd) ;
void kick_messages(struct char_data* ch, struct char_data* victim, int damage) ;
void throw_object(struct obj_data* o, int dir, int from) ;
void throw_weapon( struct obj_data* o, int dir, struct char_data* targ,struct char_data* ch );
#endif // _ACT_OFF_HPP
