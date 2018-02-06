/*$Id: doreg2.c,v 1.2 2002/02/13 12:30:58 root Exp $
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
# include "locks.c"
#endif
#define dbFILENAME "register.db"
#define txtFILENAME "register.txt"
static int aggiornato=0;
static int registrati=0;

GDBM_FILE regdb;
datum str2datum(const char *stringa) 
{
   datum key;
   key.dsize=strlen(stringa)+1;
   key.dptr=calloc(1,key.dsize);
   strcpy(key.dptr,stringa);
//   printf("%d orig= %s new= %s\n",key.dsize,stringa,key.dptr);
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
//      printf("About to store:  \n\tkey\t%s \n\tvalue\t%s\n",chiave.dptr,contenuto.dptr);
//      fflush(NULL);
      gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
      gdbm_close(regdb);
   }
   else
   {
      printf("%s\n",gdbm_strerror(gdbm_errno));
   }
   return(gdbm_errno);
}

char *gdbGet(char *fname,const char *key) 
{
   datum appo;
   datum chiave;
   char *result =NULL;
   if (regdb=gdbm_open(fname,4096,GDBM_READER,0664,NULL)) 
   {
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

GDBM_FILE RegOpen(int timeout)
{
   GDBM_FILE fd;
#if ALAR
   mudlog(LOG_PLAYERS,"Opening register");
#endif
   fd=gdbm_open(dbFILENAME,4096,GDBM_READER,0664,NULL);
   return fd;
}

struct RegInfoData *RegRead(datum valore)
{
   struct RegInfoData *ri=NULL;
   char *buf=NULL;
   char *pbuf=NULL;
   int p;
   char newbuf[REG_MAX][500];
   if (valore.dptr)
   {
      buf=datum2str(valore);
      pbuf=buf;
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
      sscanf(buf,"%s %s %s %s %s %s\n",newbuf[0],newbuf[1],
	     newbuf[2],newbuf[3],newbuf[4],newbuf[5]);
      ri=(struct RegInfoData *)calloc(1,sizeof(struct RegInfoData));
      for(p=0;p<REG_MAX;p++) 
      {
	 pbuf=newbuf[p];
	 newbuf[p][499]=0;
	 for(;*pbuf;pbuf++) 
	 {
	    if (*pbuf  == '_') *pbuf =' ';
	 }
	 ri->info[p]=strdup(newbuf[p]);
      }
      free(buf);
      free(valore.dptr);
      return ri;
   }
   return (struct RegInfoData *) NULL;
}
void RegInfoClean(struct RegInfoData *ri)
{
   int i;
   if (!ri) return;
#if ALAR
   PushStatus("RegInfoClean");
#endif
   for(i=0;i<REG_MAX;i++)
   {
      if (ri->info[i]) free(ri->info[i]);
   }
   free(ri);
   ri = (struct RegInfoData *) NULL;
#if ALAR
   PopStatus();
#endif
}

struct RegInfoData *RegGet(int campo, char *chiave)
{
   struct RegInfoData *ri=NULL;
   struct RegInfoData *rt=NULL;
   GDBM_FILE dbf;
   datum key,valore,nextkey;
   if (!chiave) return rt;
   dbf=RegOpen(0);
   if (campo==REG_PC) 
   {
      valore=gdbm_fetch(dbf,str2datum(chiave));
      if (valore.dptr) 
      {
	 rt=RegRead(valore);
      }
      gdbm_close(dbf);
      return(rt);
   }
   else 
   {
      key=gdbm_firstkey(dbf);
   }
   while (key.dptr) 
   {
      nextkey=gdbm_nextkey(dbf,key);
      valore=gdbm_fetch(dbf,key);
      ri=RegRead(valore);
      if (ri) 
      {
	 if (ri->info[campo] && (!strcasecmp(ri->info[campo],chiave)))
	 {
	    rt=ri;
	    gdbm_close(dbf);
	    return rt;
	 }
	 else
	 {
	    RegInfoClean(ri);
	 }
	 
      }
      key=nextkey;
   }
   gdbm_close(dbf);
   return rt;
}

long RegGetNewCode()
{
   struct RegInfoData *ri=NULL;
   long LastCode=0;
   long temp=0;
   datum key,nextkey,valore;
   GDBM_FILE dbf;
   dbf=RegOpen(0);
   key=gdbm_firstkey(dbf);
   while (key.dptr) 
   {
      nextkey=gdbm_nextkey(dbf,key);
      ri=RegRead(key);
      if (ri) 
      {
	 temp=atol(ri->info[REG_CODICE]);
	 LastCode=(temp>LastCode?temp:LastCode);
	 RegInfoClean(ri);
      }
      key=nextkey;
   }
   return ++LastCode;
}

struct RegInfoData *RegSeek(int campo, char *chiave)
{
   struct RegInfoData *rv;
   rv= RegGet(campo,chiave);
   return rv;
   
}

long RegCount()
{
   long howmany=0;
   datum key,nextkey;
   GDBM_FILE dbf;
   if (aggiornato) return(registrati);
   dbf=RegOpen(0);
   key=gdbm_firstkey(dbf);
   while (key.dptr) 
   {
      howmany++;
      nextkey=gdbm_nextkey(dbf,key);
      key=nextkey;
   }
   aggiornato=1;
   registrati=howmany;
   return howmany;
}

char *Reg2Char(struct RegInfoData *RI,char *buf) 
{
#if ALAR   
   sprintf(buf,"$c0015%s  $c0008%s  $c0005%s  $c0003%s  $c0006%s  %s$c0007",
	   RI->info[REG_PC],RI->info[REG_DIO],
	   RI->info[REG_CODICE],RI->info[REG_NOME],
	   RI->info[REG_EMAIL],RI->info[REG_ALTRO]);
#else
   sprintf(buf,"%s  %s  %s  %s  %s  %s",
	   RI->info[REG_PC],RI->info[REG_DIO],
	   RI->info[REG_CODICE],RI->info[REG_NOME],
	   RI->info[REG_EMAIL],RI->info[REG_ALTRO]);
#endif
   return buf;
}
char *Reg2File(struct RegInfoData *ri,char *buf) 
{
   if (ri) 
   {
      
   sprintf(buf,"%s,%s,%s,%s,%s,%s",
	   ri->info[REG_PC],ri->info[REG_DIO],
	   ri->info[REG_CODICE],ri->info[REG_NOME],
	   ri->info[REG_EMAIL],ri->info[REG_ALTRO]);
   }
   else
   {
      sprintf(buf,"Nessuna registrazione presente");
   }
   return buf;
}
void Char2Reg(struct RegInfoData *ri,char **c1) 
{
   int i=0;
   for(i=0;i<=REG_LAST;i++) 
   {
      ri->info[i]=strdup(c1[i]);
   }
   return;
}

char *RegGetCode(char *Nome)
{
   struct RegInfoData *ri=NULL;
   char *p=NULL;
   ri=RegSeek(REG_PC,Nome);
   if (ri)
   {
      p= (ri->info[REG_CODICE]);
      RegInfoClean(ri);
   }
   return(p);
}

#ifndef ALAR
int main(int argc, char **argv)
#define P0 "alar"
#define P1 argv[1]
#define P2 argv[2]
#define P3 argv[3]
#define P4 argv[4]
#define P5 argv[5]
#define P6 argv[6]
#define PRINT printf("%s",buf)
#else
void doreg(struct char_data *ch,int argc,char *argv[10])
#define P0 GET_NAME(ch)
#define P1 argv[0]
#define P2 argv[1]
#define P3 argv[2]
#define P4 argv[3]
#define P5 argv[4]
#define P6 argv[5]
#define PRINT send_to_char(buf,ch)
#endif
{
   /* Implementazione della sintassi per il mud
    * 1) register <nome>
    *      visualizza i dati di un player
    * 2) register <nome> new <nome giocatore> <email> <note> [<god>] 
    *     registra il player (accoda nel file )
    * 3) register <nome> god
    *     il peronsaggio appartiene al dio e viene registrato col suo nome
    * 4) register <nome1> <nome2> [<god>]
    *     nome 1 viene registrato come nuovo personaggio di nome2
    * 5) register <nome1> email <email address>
    *    varia la mail di un giocatore
    * 6) register <nome> show 
    *    mostra tutti i personaggi di un giocatore
    * */
   
   FILE *fd=NULL;
   int command=0;
   char buf[500];
   char tbuf[500];
   char *c1[10];
   char *tmp;
   struct RegInfoData *RI=NULL;
#ifndef ALAR
   argc--;
#endif
   if (argc<=5) P6=NULL;
   if (argc<=4) P5=NULL;
   if (argc<=3) P4=NULL;
   if (argc<=2) P3=NULL;
   if (argc<=1) P2=NULL;
   if (argc<=0) P1=NULL;
   if (!P6) argc=5;
   if (!P5) argc=4;
   if (!P4) argc=3;
   if (!P3) argc=2;
   if (!P2) argc=1;
   if (!P1) argc=0;
   if (argc==0) command=0;
   else if (argc==1) command=1;
   else if (argc==2 && !strcasecmp(P2,"show")) command=6;
   else if (argc<=3 && !strcasecmp(P2,"god")) command=3;
   else if (argc<=3 && !strcasecmp(P2,"mail")) command = 5;
   else if (argc>=4 && !strcasecmp(P2,"new")) command =2;
   else if (argc<=3) command =4;
#if ALAR
   mudlog(LOG_PLAYERS,"Reg2: %s %d %s %s %s %s %s %s",
	  P0,command,P1,P2,P3,P4,P5,P6);
#else
   printf("Reg: %s %d %s %s %s %s %s %s\n",
	  P0,command,P1,P2,P3,P4,P5,P6);
#endif
   switch (command) 
   {
    case 0: 
      {
	 sprintf(buf,"Il comando register ha 6 formati\r\n\r\n");
	 PRINT;
         sprintf(buf,"1) register <nome> Visualizza i dati di un PC\r\n");
	 PRINT;
         sprintf(buf,"2) register <nome> new <nome giocatore> <email> <note> [<nome dio>] Registra un PC\r\n"); 
	 PRINT;
         sprintf(buf,"3) register <nome> god Registra un PC divino\r\n");
	 PRINT;
         sprintf(buf,"4) register <nome1> <nome2> [<nome  dio>] Registra un secondo PC di nome2. Nome2 deve essere gia` stato registrato\r\n");
	 PRINT;
         sprintf(buf,"5) register <nome> email <email address> Varia la email\r\n");
	 PRINT;
         sprintf(buf,"6) register <nome> show Mostra tutti i PC di un player\r\n"); 
	 PRINT;
	 sprintf(buf,"N.B.\n\r  Ricordarsi di sostituire eventuali spazi con underscore\n\r");
	 PRINT;
         sprintf(buf,"  Se <nome dio> manca viene assunto quello del dio che esegue la registrazione\n\r");
	 PRINT;
	 sprintf(buf,"  Se non avete niente da mettere nelle note indicate * se\r\n");
	 PRINT;
	 sprintf(buf,"  volete indicare il dio, altrimenti potete non mettere nulla\r\n");
	 PRINT;
	 break;
      }
    case 1: 
      {
	 RI=(RegSeek(REG_PC,P1));
	 if (RI) 
	 {
	    sprintf(buf,"%s\r\n",Reg2Char(RI,tbuf));
	    PRINT;
	 }
	 else 
	 {
	    sprintf(buf,"Non ho trovato %s\r\n",P1);
	    PRINT;
	 }
	 break;
      }
    case 2: /* new */
    case 3: /* god */
    case 4: /* da altro pc */
      {
	 if ((tmp=(char*)RegGetCode(P1)))
	 {
	    sprintf(buf,"%s e' gia' stato registrato\n",P1);
	    PRINT;
	    break;
	 }
	 else 
	 {
	    if (command==4) 
	    {
	       if (!(RI=RegGet(REG_PC,P2))) 
	       {
		  sprintf(buf,"%s non registrato. Registrazione di %s fallita\n",
			  P3,P1);
		  PRINT;
		  break;
	       }
	       else
	       {
		  free(RI->info[REG_PC]);
		  RI->info[REG_PC]=strdup(P1);
		  free(RI->info[REG_DIO]);
		  RI->info[REG_DIO]=strdup(P3?P3:P0);
	       }
	    }
	    else  /* fine command =4 */
	    {
	       RI=(struct RegInfoData *)calloc(1,sizeof(struct RegInfoData));
	       c1[REG_PC]=P1;
	       if (command==3) 
	       {
		  P6=(char*)calloc(1,strlen(P3?P3:P0));
		  strcpy(P6,P3?P3:P0);
		  tmp=strdup(P1);
		  strncpy(tmp,P1,6);
	       }
	       else
	       {
		  printf("Cerco nuovo codice\n");
		  rewind(fd);
		  sprintf(buf,"%06ld",RegGetNewCode(fd));
		  tmp=strdup(buf);
	       }
	       c1[REG_DIO]=P6?P6:P0;
	       if (strlen(tmp)>REG_CODELEN)
		 tmp[6]='\0';
	       c1[REG_CODICE]=tmp;
	       c1[REG_NOME]=P3?P3:"*";
	       c1[REG_EMAIL]=P4?P4:"*";
	       c1[REG_ALTRO]=P5?P5:"*";
	       Char2Reg(RI,c1);
	    }
	 }
	 sprintf(buf,"%s\n",Reg2File(RI,tbuf));
	 if (gdbPut(dbFILENAME,RI->info[REG_PC],tbuf)==GDBM_NO_ERROR) 
	 {
	    aggiornato=0;
	    sprintf(buf,"Registrazione effettuata:%s\n",Reg2Char(RI,tbuf));
	 }
	 else 
	 {
	    sprintf(buf,"Registrazione fallita");
	 }
	 PRINT;
	 break;
      }
    case 5:
      {
	 printf(buf,"NOT YET IMPLEMENTED\r\n");
	 PRINT;
	 break;
      }
      
    case 6:
      {
	 if (!(tmp=(char*)RegGetCode(P1)))
	 {
	    sprintf(buf,"%s non registrato.\n",
		    P1);
	    PRINT;
	    break;
	 }
	 /*
	  if ((fd=RegOpen(0)))
	  {
	     while ((RI=RegGet(REG_CODICE,tmp))) 
	     {
		sprintf(buf,"%s\r\n",Reg2Char(RI,tbuf));
		PRINT;
	     }
	     fclose(fd);
	  }
	  */
	 break;
      }
   }
   if (RI) RegInfoClean(RI);
   //   printf("Tradotti %d records\n",Converti());
   if (fd) fclose(fd);
   
}

