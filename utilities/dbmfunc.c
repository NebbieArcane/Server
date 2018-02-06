/*$Id: dbmfunc.c,v 1.2 2002/02/13 12:30:57 root Exp $
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdbm.h>

#include "../src/doreg.hpp"
#include "../src/locks.hpp"
#if ALAR
#include "snew.h"
#include "protos.h"
#else
#define MARK /* none */
#include "locks.c"
#endif
#define GDBMNAME "registrazioni.db"
GDBM_FILE regdb;

datum str2datum(const char *stringa) 
{
   datum key;
   key.dsize=strlen(stringa)+1;
   key.dptr=calloc(1,key.dsize);
   strcpy(key.dptr,stringa);
   printf("%d orig= %s new= %s\n",key.dsize,stringa,key.dptr);
   return(key);
}
char *datum2str(datum key) 
{
   char *result;
   if (key.dptr) 
   {
      result=calloc(1,key.dsize);
      strncpy(result,key.dptr,key.dsize);
   }
   else
     result=NULL;
   return(result);
}
int gdbPut(char *fname,const char *key,const char *valore) 
{
   datum contenuto;
   datum chiave;
   if (regdb=gdbm_open(fname,4096,GDBM_WRCREAT,0664,NULL)) 
   {
      chiave=str2datum(key);
      contenuto=str2datum(valore);
      printf("About to store:  \n\tkey\t%s \n\tvalue\t%s\n",chiave.dptr,contenuto.dptr);
      fflush(NULL);
      gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
      gdbm_close(regdb);
   }
   else
   {
      printf("%s\n",gdbm_strerror(gdbm_errno));
   }
   
}

char *gdbGet(char *fname,const char *key) 
{
   datum appo;
   datum chiave;
   char *result =NULL;
   if (regdb=gdbm_open(fname,4096,GDBM_READER,0664,NULL)) 
   {
      printf("Conversione in corso\n");
      chiave=str2datum(key);
      appo=gdbm_fetch(regdb,chiave);
      result=datum2str(appo);
      gdbm_close(regdb);
   }
   else
   {
      printf("%s\n",gdbm_strerror(gdbm_errno));
   }
   
   return(result);
}
int main() 
{
   gdbPut("test.db","roby","contenuto di roby");
   gdbPut("test.db","nino","contenuto di nino");
   gdbPut("test.db","pluto","contenuto di pluto");
   gdbPut("test.db","paperino","contenuto di paperino");
   printf("key: %s val %s\n","roby",gdbGet("test.db","roby"));
   return 1;
}
