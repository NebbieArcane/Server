/*
 * act.other.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_OTHER_HPP_
#define SRC_ACT_OTHER_HPP_
char* Condition(struct char_data* ch);
char* Tiredness(struct char_data* ch);
int CanMemorize(struct char_data* ch);
int CheckContempMemorize(struct char_data* pChar);
int TotalMaxCanMem(struct char_data* ch);
int TotalMemorized(struct char_data* ch);
/* src/act.other.cpp */
void check_memorize(struct char_data* ch, struct affected_type* af);
void ck_eq_action(struct char_data* ch, struct obj_data* obj);
void Dismount(struct char_data* ch, struct char_data* h, int pos);
void do_alias(struct char_data* ch, char* arg, int cmd);
void do_brief(struct char_data* ch, char* argument, int cmd);
void do_bug(struct char_data* ch, char* argument, int cmd);
void do_compact(struct char_data* ch, char* argument, int cmd);
void do_ego_eq_action(struct char_data* ch);
void do_gain(struct char_data* ch, char* argument, int cmd);
void do_group_name(struct char_data* ch, char* arg, int cmd);
void do_group(struct char_data* ch, char* argument, int cmd);
void do_guard(struct char_data* ch, char* argument, int cmd);
void do_hide(struct char_data* ch, char* argument, int cmd);
void do_idea(struct char_data* ch, char* argument, int cmd);
void do_junk(struct char_data* ch, char* argument, int cmd);
void do_memorize(struct char_data* ch, char* argument, int cmd);
void do_mount(struct char_data* ch, char* arg, int cmd);
void do_not_here(struct char_data* ch, char* argument, int cmd);
void do_plr_noauction(struct char_data* ch, char* argument, int cmd);
void do_plr_nogossip(struct char_data* ch, char* argument, int cmd);
void do_plr_noshout(struct char_data* ch, char* argument, int cmd);
void do_plr_notell(struct char_data* ch, char* argument, int cmd);
void do_practice(struct char_data* ch, char* arg, int cmd);
void do_quaff(struct char_data* ch, char* argument, int cmd);
void do_qui(struct char_data* ch, char* argument, int cmd);
void do_quit(struct char_data* ch, char* argument, int cmd);
void do_recite(struct char_data* ch, char* argument, int cmd);
void do_save(struct char_data* ch, char* argument, int cmd);
void do_set_afk(struct char_data* ch, char* argument, int cmd);
void do_set_flags(struct char_data* ch, char* argument, int cmd);
void do_set_prompt(struct char_data* ch, char* argument, int cmd);
void do_sneak(struct char_data* ch, char* argument, int cmd);
void do_steal(struct char_data* ch, char* argument, int cmd);
void do_title(struct char_data* ch, char* argument, int cmd);
void do_tspy(struct char_data* ch, char* argument, int cmd);
void do_typo(struct char_data* ch, char* argument, int cmd);
void do_use(struct char_data* ch, char* argument, int cmd);
void do_whois(struct char_data* ch, char* argument, int cmd);
void parse_eq_action(struct char_data* ch, struct obj_data* obj);
#endif /* SRC_ACT_OTHER_HPP_ */
