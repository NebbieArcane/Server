/*$Id: comm.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
#ifndef __COMM
#define __COMM 1
#include "configuration.hpp"
#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3
#ifndef BLOCK_WRITE
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)
#else
#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
#endif


#define PLAYER_AUTH 0
#define DFLT_DIR		"lib"				/* default data directory     */


char *ParseAnsiColors( int UsingAnsi, char *txt );


#endif
