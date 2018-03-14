/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __MAIL_HPP
#define __MAIL_HPP
/***************************  System  include ************************************/
//#include <iostream>
//#include <ostream>
/***************************  Local    include ************************************/
//#include "general.hpp"
namespace Alarmud {
/* ************************************************************************
*   File: mail.h                                        Part of CircleMUD *
*  Usage: header file for mail system                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/******* MUD MAIL SYSTEM HEADER FILE ****************************
 ***     written by Jeremy Elson (jelson@server.cs.jhu.edu)   ***
 ****   compliments of CircleMUD (circle.cs.jhu.edu 4000)    ****
 ***************************************************************/

/* INSTALLATION INSTRUCTIONS in MAIL.C */

/* You can modify the following constants to fit your own MUD.  */

/* command numbers of the "mail", "check", and "receive" commands
   in your interpreter. */
#define CMD_MAIL        366
#define CMD_CHECK        367
#define CMD_RECEIVE        368

/* minimum level a player must be to send mail        */
#define MIN_MAIL_LEVEL 2

/* # of gold coins required to send mail        */
#define STAMP_PRICE 50

/* Maximum size of mail in bytes (arbitrary)        */
#define MAX_MAIL_SIZE 4000

/* Max size of player names                        */
#define NAME_SIZE  15

/* size of mail file allocation blocks                */
#define BLOCK_SIZE 100

/* NOTE:  Make sure that your block size is big enough -- if not,
   HEADER_BLOCK_DATASIZE will end up negative.  This is a bad thing.
   Check the define below to make sure it is >0 when choosing values
   for NAME_SIZE and BLOCK_SIZE.  100 is a nice round number for
   BLOCK_SIZE and is the default ... why bother trying to change it
   anyway?

   The mail system will always allocate disk space in chunks of size
   BLOCK_SIZE.
*/

/* USER CHANGABLE DEFINES ABOVE **
***************************************************************************
**   DON'T TOUCH DEFINES BELOW  */

int        scan_file(void);
int        has_mail(char* recipient);
void        store_mail(char* to, char* from, char* message_pointer);
char*        read_delete(char* recipient, char* recipient_formatted);

#define INT_SIZE  sizeof(int)
#define CHAR_SIZE sizeof(char)
#define LONG_SIZE sizeof(long)

#define HEADER_BLOCK_DATASIZE (BLOCK_SIZE-1-((CHAR_SIZE*(NAME_SIZE+1)*2)+(3*INT_SIZE)))
/* size of the data part of a header block */

#define DATA_BLOCK_DATASIZE (BLOCK_SIZE-INT_SIZE-1)
/* size of the data part of a data block */

/* note that an extra space is allowed in all string fields for the
   terminating null character.  */

#define HEADER_BLOCK  -1
#define LAST_BLOCK    -2
#define DELETED_BLOCK -3

/* note: next_block is part of header_blk in a data block; we can't combine
   them here because we have to be able to differentiate a data block from a
   header block when booting mail system.
*/
#pragma pack(push,4)
struct header_block_type {
	int        block_type;          /* is this a header block or data block? */
	int        next_block;        /* if header block, link to next block   */
	char        from[NAME_SIZE+1]; /* who is this letter from?                 */
	char        to[NAME_SIZE+1];/* who is this letter to?                 */
	int        mail_time;        /* when was the letter mailed?                 */
	char        txt[HEADER_BLOCK_DATASIZE+1]; /* the actual text        */
};
struct data_block_type {
	int        block_type;          /* -1 if header block, -2 if last data block
                                         in mail, otherwise a link to the next */
	char        txt[DATA_BLOCK_DATASIZE+1]; /* the actual text                 */
};
#pragma pack(pop)
static_assert (sizeof(data_block_type)==BLOCK_SIZE,"Check align, data_block_tpe size is wrong");
static_assert (sizeof(header_block_type)==BLOCK_SIZE,"Check align, header_block_tpe size is wrong");

struct position_list_type {
	int        position;
	struct position_list_type* next;
};


struct mail_index_type {
	char        recipient[NAME_SIZE+1]; /* who the mail is for */
	position_list_type*             list_start;  /* list of mail positions    */
	struct mail_index_type* next;
};

mail_index_type* find_char_in_index(char* searchee) ;
struct char_data* find_mailman(struct char_data* ch) ;
int has_mail(char* recipient) ;
void index_mail(char* raw_name_to_index, int pos) ;
int mail_ok(struct char_data* ch) ;
int pop_free_list(void) ;
void postmaster_check_mail(struct char_data* ch, int cmd, char* arg) ;
void postmaster_receive_mail(struct char_data* ch, int cmd, char* arg) ;
void postmaster_send_mail(struct char_data* ch, int cmd, char* arg) ;
void push_free_list(int pos) ;
char* read_delete(char* recipient, char* recipient_formatted);
void read_from_file(void* buf, int size, int filepos) ;
int scan_mail_file(void) ;
void store_mail(char* to, char* from, char* message_pointer) ;
void write_to_file(void* buf, int size, int filepos) ;
} // namespace Alarmud
#endif // __MAIL_HPP

