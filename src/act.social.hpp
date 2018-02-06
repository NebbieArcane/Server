/*
 * act.social.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_ACT_SOCIAL_HPP_
#define SRC_ACT_SOCIAL_HPP_

char* fread_action(FILE* fl);
void boot_social_messages();
int find_action(int cmd);
void do_action(struct char_data* ch, char* argument, int cmd);
void do_insult(struct char_data* ch, char* argument, int cmd);
void boot_pose_messages();
void do_pose(struct char_data* ch, char* argument, int cmd);



#endif /* SRC_ACT_SOCIAL_HPP_ */
