/*$Id: comm.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/

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

char *ParseAnsiColors( int UsingAnsi, char *txt );
