/*$Id: mkver.c,v 1.2 2002/02/13 12:30:58 root Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "../src/snew.hpp"
#include "release.h"
int main (int argc, char **argv)
{
  int port, pos=1;
  char *dir;


  while ((pos < argc) && (*(argv[pos]) == '-'))
  {
    switch (*(argv[pos] + 1))
    {
     case 'f':
     case 'F':
      printf("%s%s%s",MAJORVERSION,MINORVERSION,RELEASE);
      break;
     case 'D':
     case 'd':
      printf("AlarMUD, Versione %s.%s%s\n\r",
	     MAJORVERSION,MINORVERSION,RELEASE);
      break;
     default:
      break;
    }
    pos++;
  }

}
