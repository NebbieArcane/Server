/*$Id: mkrel.c,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/* From input file "<stdin>" */
#include <stdio.h>
#include <stdlib.h>
#include "snew.h"
main()
{ 
   char *riga; 
   riga=(char *)calloc(255,1);
   gets(riga);
   printf("#define MAJORVERSION \"%s\"\n", riga);

   gets(riga);
   printf("#define MINORVERSION \"%s\"\n", riga);
   
   gets(riga);
   printf("#define RELEASE \"%s\"\n",riga);

   return(0);
}

