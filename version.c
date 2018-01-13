/*$Id: version.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include "release.h"
#include <string.h>
#include "version.h"
//#include "C:\Nebbie\SRC\version.h"
char *version()
{
   static char buf[6];
   snprintf(buf,5,"%d.%d",MAJORVERSION,MINORVERSION);
   return(buf);
}

char *release()
{
      static char buf[2];
   snprintf(buf,2,"%d",RELEASE);
   return(buf);
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


