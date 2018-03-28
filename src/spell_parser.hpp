/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * spell_parser.hpp
 *
 *  Created on: 14 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef SRC_SPELL_PARSER_HPP_
#define SRC_SPELL_PARSER_HPP_
#include "spells.hpp"
namespace Alarmud {

extern const char* spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];

/* spell_parser.cpp */
bool ImpSaveSpell(struct char_data* ch, sh_int save_type, int mod);
bool circle_follow(struct char_data* ch, struct char_data* victim);
bool saves_spell(struct char_data* ch, sh_int save_type);
void spellid(int nr, struct char_data* ch, int cl, int sl);
void spello(int nr, byte beat, byte pos, byte mlev, byte clev, byte dlev, byte slev, byte plev, byte rlev, byte ilev, ubyte mana, sh_int tar, spell_func, sh_int sf, sh_int align, sh_int ostile);
int SPELL_LEVEL(struct char_data* ch, int sn);
void SpellWearOffSoon(int s, struct char_data* ch);
int check_falling(struct char_data* ch);
void check_drowning(struct char_data* ch);
void check_decharm(struct char_data* ch);
void SpellWearOff(int s, struct char_data* ch);
int check_nature(struct char_data* i);
int CheckMulti(struct char_data* i);
void CheckSpecialties(struct char_data* ch, struct affected_type* af);
void affect_update(unsigned long pulse);
void stop_follower(struct char_data* ch);
void die_follower(struct char_data* ch);
void add_follower(struct char_data* ch, struct char_data* leader);
void say_spell(struct char_data* ch, int si);
char* skip_spaces(char* string);
void do_cast(struct char_data* ch, const char* argument, int cmd);
void assign_spell_pointers(void);
void check_falling_obj(struct obj_data* obj, int room);
} // namespace Alarmud
#endif /* SRC_SPELL_PARSER_HPP_ */

