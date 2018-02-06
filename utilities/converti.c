#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <gdbm.h>

#define REG_PC      0
#define REG_DIO     1
#define REG_CODICE  2
#define REG_NOME    3
#define REG_EMAIL   4
#define REG_ALTRO   5
#define REG_MAX     6
#define REG_LAST    5
#define REG_CODELEN 6
#define NEW 1
#define ADD 2
#define CHG 3
#define NOMEDB1 "regdb_d.db"
#define NOMEDB2 "regdb_i.db"
#define NOMETXT "register.txt"
#define DBMASK 0666
#define sh_int short
#define FALSE 0
#define TRUE  1

char *Info2Char(char *cbuf);
void Gdbm2Info(char *cbuf);
void RegInfoClean();
bool gdbPut(short fl);
bool gdbGet();
bool gdbDel();
long gdbShow(char *ch);
long RegCount();
void RegRead(FILE *fd);
long Converti();


struct RegInfoData
{
   char info[REG_MAX][81];
};

struct RegInfoData RI;

char *Info2Char(char *cbuf) 
{
   sprintf(cbuf,"%s %s %s %s %s %s",
	   RI.info[REG_PC],RI.info[REG_DIO],
	   RI.info[REG_CODICE],RI.info[REG_NOME],
	   RI.info[REG_EMAIL],RI.info[REG_ALTRO]);
   return cbuf;
}

void Gdbm2Info(char *cbuf)
{
   RegInfoClean();
   sscanf(cbuf,"%s %s", &RI.info[REG_CODICE], &RI.info[REG_DIO]);
}

void RegInfoClean()
{
   int i;

   for(i=0;i<REG_MAX;i++)
      if (RI.info[i])
         strcpy(RI.info[i], "");
}

bool gdbPut(sh_int fl_new) 
{
   GDBM_FILE regdb;
   datum contenuto;
   datum chiave;
   static long MaxCode;
   char buf[512];
    
   if (regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL))
   {
      chiave.dsize =strlen(RI.info[REG_PC])+1;
      chiave.dptr =RI.info[REG_PC];
      sprintf(buf,"%s %s",RI.info[REG_CODICE],RI.info[REG_DIO]);
      contenuto.dsize =strlen(buf)+1;
      contenuto.dptr =buf;
      if (!gdbm_store(regdb,chiave,contenuto,GDBM_INSERT))
         printf("About to store %s:  \n\tkey\t%s \n\tvalue\t%s\n",NOMEDB1,chiave.dptr,contenuto.dptr);
      gdbm_close(regdb);
   }
   else
   {
      printf("Erroring register gdbPut(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }

   if (regdb=gdbm_open(NOMEDB2,4096,GDBM_WRCREAT,DBMASK,NULL)) 
   {
      chiave.dsize =strlen(RI.info[REG_CODICE])+1;
      chiave.dptr =RI.info[REG_CODICE];
      contenuto.dsize =strlen(RI.info[REG_EMAIL])+1;
      contenuto.dptr =RI.info[REG_EMAIL];
      if (!gdbm_store(regdb,chiave,contenuto,GDBM_INSERT))
      {
         MaxCode++;
         printf("About to store %s:  \n\tkey\t%s \n\tvalue\t%s\n",NOMEDB2,chiave.dptr,contenuto.dptr);
         chiave.dsize =7;
         chiave.dptr ="~~~~~~";
         contenuto.dsize =14;
         sprintf(buf,"%06ld %06ld", 0L, MaxCode);
         contenuto.dptr =buf;
         gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
      }
      gdbm_close(regdb);
      return TRUE;
   }
   else
   {
      printf("Erroring register gdbPut(): %s Chiave %s, %s\n", NOMEDB2, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   gdbm_close(regdb);
   return FALSE;
}

void RegRead(FILE *fd)
{
   char buf[500];
   char *pbuf=NULL;
   int p;
   char newbuf[REG_MAX][500];

   while (!feof(fd))
   {
      fgets(buf,499,fd);
      pbuf=buf;
      if (*pbuf && *pbuf != '*') 
      {
	 for(;*pbuf;pbuf++) 
	 {
	    if (*pbuf  == ' ') *pbuf ='_';
	 }
	 pbuf=buf;
	 for(;*pbuf;pbuf++) 
	 {
	    if (*pbuf == ',') *pbuf =' ';
	 }
         for(p=0;p<6;p++) newbuf[p][0]='\0';
	 sscanf(buf,"%s %s %s %s %s %s",newbuf[0],newbuf[1],
		newbuf[2],newbuf[3],newbuf[4],newbuf[5]);
         RegInfoClean();
	 for(p=0;p<REG_MAX;p++) 
	 {
	    pbuf=newbuf[p];
	    newbuf[p][499]=0;
	    for(;*pbuf;pbuf++) 
	    {
	       if (*pbuf  == '_') *pbuf =' ';
	    }
	    strcpy(RI.info[p],newbuf[p]);
	 }
	 return;
      }
   }
   return;
}

long Converti()
{
   GDBM_FILE regdb;
   datum chiave, contenuto;
   FILE *fd;
   char buf[512];
   long howmany=0L, LastCode=0L;

   fd=fopen(NOMETXT,"r+");
   if (!fd) 
     return (0);
   else
   for (RegRead(fd);!feof(fd);RegRead(fd))
      if (RI.info[0]) 
      {
	 howmany++;
	 Info2Char(buf);
         printf("\nCONVERTO: %s\n",buf);
	 gdbPut(NEW);
         if (LastCode < atol(RI.info[REG_CODICE]))
            LastCode =atol(RI.info[REG_CODICE]);
	 RegInfoClean();
      }
   fclose(fd);
   if (regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL)) 
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto.dsize =14;
      sprintf(buf,"%06ld %06ld", LastCode, howmany);
      printf("RISULTATO: ultimo=%06ld max=%06ld\n", LastCode, howmany);
      contenuto.dptr =buf;
      gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
      gdbm_close(regdb);
   }
   else
   {
      printf("Erroring register Converti(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   return howmany;
}

int main(int argc, char **argv)
{
   GDBM_FILE regdb;
   datum chiave, contenuto, nextkey;
   long conta=0L;

   unlink(NOMEDB1);
   unlink(NOMEDB2);

   if (regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL)) 
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto.dsize =14;
      contenuto.dptr ="000000 000000";
      gdbm_store(regdb,chiave,contenuto,GDBM_INSERT);
      gdbm_close(regdb);
   }
   else
   {
      printf("Erroring register Converti(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }

   if (regdb=gdbm_open(NOMEDB2,4096,GDBM_WRCREAT,DBMASK,NULL)) 
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto.dsize =14;
      contenuto.dptr ="000000 000000";
      gdbm_store(regdb,chiave,contenuto,GDBM_INSERT);
      gdbm_close(regdb);
   }
   else
   {
      printf("Erroring register Converti(): %s Chiave %s, %s\n", NOMEDB2, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   RegInfoClean();
   printf("Convertiti %d records\r\n",Converti());

   printf("VERIFICA record scritti in %s:\n",NOMEDB1);
   if (regdb=gdbm_open(NOMEDB1,4096,GDBM_READER,DBMASK,NULL))
   {
      chiave =gdbm_firstkey (regdb);
      while (chiave.dptr)
      {
         contenuto =gdbm_fetch (regdb, chiave);
         if (contenuto.dptr)
         {
            printf("%s  %s\n", chiave.dptr, contenuto.dptr);
            conta++;
            free (contenuto.dptr);
         }
         nextkey =gdbm_nextkey (regdb, chiave);
         free (chiave.dptr);
         chiave =nextkey;
      }
      gdbm_close(regdb);
   }
   printf("VERIFICATI %ld records.\n",conta);

   conta =0L;
   printf("VERIFICA record scritti in %s:\n",NOMEDB2);
   if (regdb=gdbm_open(NOMEDB2,4096,GDBM_READER,DBMASK,NULL))
   {
      chiave =gdbm_firstkey (regdb);
      while (chiave.dptr)
      {
         contenuto =gdbm_fetch (regdb, chiave);
         if (contenuto.dptr)
         {
            printf("%s  %s\n", chiave.dptr, contenuto.dptr);
            conta++;
            free (contenuto.dptr);
         }
         nextkey =gdbm_nextkey (regdb, chiave);
         free (chiave.dptr);
         chiave =nextkey;
      }
      gdbm_close(regdb);
   }
   printf("VERIFICATI %ld records.\n",conta);
}
