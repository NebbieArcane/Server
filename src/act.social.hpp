#ifndef __ACT_SOCIAL_HPP
#define __ACT_SOCIAL_HPP
#include "structs.hpp"
#include <stdio.h>
struct social_messg {
	int act_nr;
	int hide;
	int min_victim_position; /* Position of victim */

	/* No argument was supplied */
	char* char_no_arg;
	char* others_no_arg;

	/* An argument was there, and a victim was found */
	char* char_found;                /* if NULL, read no further, ignore args */
	char* others_found;
	char* vict_found;

	/* An argument was there, but no victim was found */
	char* not_found;

	/* The victim turned out to be the character */
	char* char_auto;
	char* others_auto;
};
extern struct social_messg * soc_mess_list;
struct pose_type {
	int level;          /* minimum level for poser */
	char* poser_msg[4];  /* message to poser        */
	char* room_msg[4];   /* message to room         */
};
extern struct pose_type pose_messages[MAX_MESSAGES];


void boot_pose_messages() ;
void boot_social_messages() ;
void do_action(struct char_data* ch, char* argument, int cmd) ;
void do_insult(struct char_data* ch, char* argument, int cmd) ;
void do_pose(struct char_data* ch, char* argument, int cmd) ;
int find_action(int cmd) ;
char* fread_action(FILE* fl) ;
#endif // __ACT_SOCIAL_HPP
