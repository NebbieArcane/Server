/*$Id: version.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include "release.h"
#include <string.h>
#include "version.h"
char *version()
{
   return VERSION;
}

char *release()
{
	return BUILD;
}

char *compilazione()
{
   static char COMPILAZIONE[50];
   snprintf(COMPILAZIONE,49,"%s %s",__DATE__,__TIME__);
   return(COMPILAZIONE);
}
char *xtest(char *appo)
{
   return strstr(appo,"ee");
}
char *ytest(char *appo)
{
   return strchr(appo,'e');
}

char *ztest(char *appo)
{
   return strtok(appo," ");
}
#ifndef ALAR
int main()
{
  printf("%s %s %s \n\r",version(),release(),compilazione());
   return(0);
}
#endif


