/*
 * act.obj1.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_OBJ1_HPP_
#define SRC_ACT_OBJ1_HPP_

void do_drop(struct char_data* ch, char* argument, int cmd);
void do_get(struct char_data* ch, char* argument, int cmd);
void do_give(struct char_data* ch, char* argument, int cmd);
void do_pquest(struct char_data* ch, char* argument, int cmd);
void do_put(struct char_data* ch, char* argument, int cmd);
void get(struct char_data* ch, struct obj_data* obj_object, struct obj_data* sub_object);
void get_trophy(struct char_data* ch, struct obj_data* ob);
void givexp(struct char_data* ch, struct char_data* victim, int amount);
int newstrlen(char* p);
void truegivexp(struct char_data* ch, struct char_data* victim, int amount);




#endif /* SRC_ACT_OBJ1_HPP_ */
