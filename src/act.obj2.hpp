/*
 * act.obj2.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_OBJ2_HPP_
#define SRC_ACT_OBJ2_HPP_

void weight_change_object(struct obj_data* obj, int weight);
void name_from_drinkcon(struct obj_data* obj);
void name_to_drinkcon(struct obj_data* obj,int type);
void do_drink(struct char_data* ch, char* argument, int cmd);
void do_eat(struct char_data* ch, char* argument, int cmd);
void do_pour(struct char_data* ch, char* argument, int cmd);
void do_sip(struct char_data* ch, char* argument, int cmd);
void do_taste(struct char_data* ch, char* argument, int cmd);
void perform_wear( struct char_data* ch, struct obj_data* obj_object,
				   long keyword);
int IsRestricted( struct obj_data* obj, int Class );
void wear(struct char_data* ch, struct obj_data* obj_object, long keyword);
void do_wear(struct char_data* ch, char* argument, int cmd);
void do_wield(struct char_data* ch, char* argument, int cmd);
void do_grab(struct char_data* ch, char* argument, int cmd);
void do_remove(struct char_data* ch, char* argument, int cmd);




#endif /* SRC_ACT_OBJ2_HPP_ */
