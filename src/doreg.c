/*$Id: doreg.c,v 1.4 2002/03/23 16:43:20 Thunder Exp $
*/
#if 0
#include <gdbm.h>
#include <unistd.h>
#include <crypt.h>
#include "doreg.h"
#include "snew.h"

//typedef struct __GDBM_FILE_RECORD {int dummy[10];} *GDBM_FILE;
//extern GDBM_FILE gdbm_open(char *, int, int, int, void (*)());

struct RegInfoData RI;

char *Info2Char(int liv, char *cbuf) 
{
#if ALAR   
   sprintf(cbuf,"$c0015%s  $c0005%s  $c0006%s  $c0008%s$c0007",
	   RI.info[REG_PC],    (liv >=MAESTRO_DEL_CREATO) ? RI.info[REG_CODICE] : "",
	   RI.info[REG_EMAIL], RI.info[REG_DIO]);
#else
   sprintf(cbuf,"%s %s %s %s",
	   RI.info[REG_PC],    (liv >=MAESTRO_DEL_CREATO) ? RI.info[REG_CODICE] : "",
	   RI.info[REG_EMAIL], RI.info[REG_DIO]);
#endif
   return cbuf;
}

void Gdbm2Info(char *cbuf) 
{
   sscanf(cbuf,"%s %s", &RI.info[REG_CODICE][0], &RI.info[REG_DIO][0]); // SALVO cosi' non esce il warning
}

void RegInfoClean()
{
   int i;

   for(i=0;i<REG_MAX;i++)
      if (RI.info[i])
         strcpy(RI.info[i], "");
}

bool gdbDel() 
{
   GDBM_FILE regdb;
   datum contenuto;
   datum chiave;
   char buf[512];
   long LastCode, MaxCode;
   long conta =gdbShow(NULL);

   if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL)) )
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto =gdbm_fetch(regdb,chiave);
      if (contenuto.dptr)
      {
         sscanf(contenuto.dptr, "%06ld %06ld", &LastCode, &MaxCode);
         sprintf(contenuto.dptr, "%06ld %06ld", LastCode, --MaxCode);
         gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
         free(contenuto.dptr);
      }
      else
      {
         mudlog(LOG_SYSERR,"Attenzione gdbDel() non esiste il campo totalizzatore in %s", NOMEDB1);
         gdbm_close(regdb);
         return FALSE;
      }
      chiave.dsize =strlen(RI.info[REG_PC])+1;
      chiave.dptr =RI.info[REG_PC];
      gdbm_delete(regdb,chiave);
      gdbm_close(regdb);
      if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL)) )
      {
         gdbm_reorganize(regdb);
         gdbm_sync(regdb);
         gdbm_close(regdb);
      }
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register gdbDel(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }

   if (conta >1L)
      return TRUE;

   if ((regdb=gdbm_open(NOMEDB2,4096,GDBM_WRCREAT,DBMASK,NULL)))
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto =gdbm_fetch(regdb,chiave);
      if (contenuto.dptr)
      {
         sscanf(contenuto.dptr, "%06ld %06ld", &LastCode, &MaxCode);
         sprintf(contenuto.dptr, "%06ld %06ld", LastCode, --MaxCode);
         gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
         free(contenuto.dptr);
      }
      else
      {
         mudlog(LOG_SYSERR,"Attenzione gdbDel() non esiste il campo totalizzatore in %s", NOMEDB2);
         gdbm_close(regdb);
         return FALSE;
      }
      chiave.dsize =strlen(RI.info[REG_CODICE])+1;
      chiave.dptr =RI.info[REG_CODICE];
      gdbm_delete(regdb,chiave);
      gdbm_close(regdb);
      if ((regdb=gdbm_open(NOMEDB2,4096,GDBM_WRCREAT,DBMASK,NULL)))
      {
         gdbm_reorganize(regdb);
         gdbm_sync(regdb);
         gdbm_close(regdb);
      }
      return TRUE;
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register gdbDel(): %s Chiave %s, %s\n", NOMEDB2, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   return FALSE;
}

bool gdbPut(sh_int fl_new) 
{
   GDBM_FILE regdb;
   datum contenuto;
   datum chiave;
   char buf[512];
   long LastCode, MaxCode;
   
   mudlog(LOG_SYSERR, "INIZIO GDBPUT");
   
   if (!strcmp(RI.info[REG_PC],"~~~~~~"))
      return FALSE;

   if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_WRCREAT,DBMASK,NULL)))
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      mudlog(LOG_SYSERR, "Chiave? *%s*", chiave);
      
      //gdbm_store(regdb,chiave,contenuto,GDBM_INSERT);
      
      if (gdbm_exists(regdb,chiave))
        mudlog(LOG_SYSERR,"ESISTE!!!");
      else
        mudlog(LOG_SYSERR,"NON ESISTE!!!");
        
      contenuto =gdbm_fetch(regdb,chiave);
      
      
      if (contenuto.dptr)
      {
        mudlog(LOG_SYSERR,"1");
         sscanf(contenuto.dptr, "%06ld %06ld", &LastCode, &MaxCode);
         sprintf(contenuto.dptr, "%06ld %06ld", (fl_new==NEW) ? ++LastCode : LastCode, (fl_new!=CHG) ? ++MaxCode : MaxCode);
         if (fl_new ==NEW)
            sprintf(RI.info[REG_CODICE], "%06ld", LastCode);
         gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
         free(contenuto.dptr);
      }
      else
      {
         mudlog(LOG_SYSERR,"Attenzione gdbPut() non esiste il campo totalizzatore in %s error: %s", NOMEDB1,gdbm_strerror(gdbm_errno));
         gdbm_close(regdb);
         return FALSE;
      }
      if (fl_new !=CHG)
      {
         mudlog(LOG_SYSERR,"2");
         chiave.dsize =strlen(RI.info[REG_PC])+1;
         chiave.dptr =RI.info[REG_PC];
         sprintf(buf,"%s %s",RI.info[REG_CODICE],RI.info[REG_DIO]);
         contenuto.dsize =strlen(buf)+1;
         contenuto.dptr =buf;
         gdbm_store(regdb,chiave,contenuto,GDBM_INSERT);
      }
      gdbm_close(regdb);
   }
   else
   {
      mudlog(LOG_SYSERR,"1 Error register gdbPut(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }

   if (fl_new ==ADD)
      return TRUE;

   if ((regdb=gdbm_open(NOMEDB2,4096,GDBM_WRCREAT,DBMASK,NULL)))
   {
      gdbm_store(regdb,chiave,contenuto,GDBM_INSERT);
      
      chiave.dsize =strlen(RI.info[REG_CODICE])+1;
      chiave.dptr =RI.info[REG_CODICE];
      contenuto.dsize =strlen(RI.info[REG_EMAIL])+1;
      contenuto.dptr =RI.info[REG_EMAIL];
      if (fl_new !=CHG)
      {
         gdbm_store(regdb,chiave,contenuto, GDBM_INSERT);

         chiave.dsize =7;
         chiave.dptr ="~~~~~~";
         
         //gdbm_store(regdb,chiave,contenuto, GDBM_INSERT);
         
         if (gdbm_exists(regdb,chiave))
            mudlog(LOG_SYSERR,"ESISTE!!!");
         else
            mudlog(LOG_SYSERR,"NON ESISTE!!!");
         
         contenuto =gdbm_fetch(regdb,chiave);
         if (contenuto.dptr)
         {
            sscanf(contenuto.dptr, "%06ld %06ld", &LastCode, &MaxCode);
            sprintf(contenuto.dptr, "%06ld %06ld", LastCode, ++MaxCode);
            gdbm_store(regdb,chiave,contenuto,GDBM_REPLACE);
            free(contenuto.dptr);
            gdbm_close(regdb);
            return TRUE;
         }
         else
         {
            mudlog(LOG_SYSERR,"2 Attenzione gdbPut() non esiste il campo totalizzatore in %s", NOMEDB2);
            gdbm_close(regdb);
            return FALSE;
         }
      }
      else
      {
         gdbm_store(regdb,chiave,contenuto, GDBM_REPLACE);
         gdbm_close(regdb);
         return TRUE;
      }
   }
   else
   {
      mudlog(LOG_SYSERR,"3 Error register gdbPut(): %s Chiave %s, %s\n", NOMEDB2, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   gdbm_close(regdb);
   return FALSE;
}

long gdbShow(struct char_data *chi) 
{
   GDBM_FILE regdb;
   datum chiave;
   datum nextkey;
   datum tmpval;
   char tmpcod[7];
   char buf[81];
   long conta=0L;

   if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_READER,DBMASK,NULL)))
   {
      strcpy(tmpcod,RI.info[REG_CODICE]);
      chiave =gdbm_firstkey (regdb);
      while (chiave.dptr)
      {
         tmpval =gdbm_fetch (regdb, chiave);
         if (tmpval.dptr)
         {
            if (strcmp(chiave.dptr,"~~~~~~"))
            {
               Gdbm2Info(tmpval.dptr);
               if (!strcmp(RI.info[REG_CODICE], tmpcod))
               {
                   if (chi)
                   {
                      sprintf(buf,"$c0015%s$c0007  $c0008[%s]$c0007\n", chiave.dptr, RI.info[REG_DIO]);
                      send_to_char(buf, chi);
                   }
                   conta++;
               }
            }
            free (tmpval.dptr);
         }
         nextkey =gdbm_nextkey (regdb, chiave);
         free (chiave.dptr);
         chiave =nextkey;
      }
      strcpy(RI.info[REG_CODICE],tmpcod);
      gdbm_close(regdb);
      return conta;
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register gdbShow(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
   }
   return conta;
}

bool gdbGet() 
{
   GDBM_FILE regdb;
   datum chiave;
   datum result;

   if (!strcmp(RI.info[REG_PC],"~~~~~~"))
      return FALSE;
   mudlog(LOG_SYSERR,"NOMEDB1 %s, DBMASK %i", NOMEDB1,DBMASK);
   //if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_READER,DBMASK,NULL)))
   if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_READER,DBMASK,NULL)))
   {
      chiave.dsize =strlen(RI.info[REG_PC])+1;
      mudlog(LOG_SYSERR,"CHIAVE DPTR: **%s**", RI.info[REG_PC]);
      chiave.dptr =RI.info[REG_PC];
      result=gdbm_fetch(regdb,chiave);
      MARK;
      gdbm_close(regdb);
      MARK;
      if (result.dptr)
      {
         Gdbm2Info(result.dptr);
         free(result.dptr);
      }
      else
      {
         return FALSE;
      }
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register gdbGet(): %s Chiave %s, %s\n", NOMEDB1, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   if ((regdb=gdbm_open(NOMEDB2,4096,GDBM_READER,DBMASK,NULL)))
   {
      chiave.dsize =strlen(RI.info[REG_CODICE])+1;
      chiave.dptr =RI.info[REG_CODICE];
      result=gdbm_fetch(regdb,chiave);
      gdbm_close(regdb);
      if (result.dptr)
      {
         strcpy(RI.info[REG_EMAIL], result.dptr);
         free(result.dptr);
      }
      else
      {
         return FALSE;
      }
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register gdbGet(): %s Chiave %s, %s\n", NOMEDB2, chiave.dptr, gdbm_strerror(gdbm_errno));
      return FALSE;
   }
   return TRUE;
}

long RegCount()
{
   GDBM_FILE regdb;
   datum contenuto;
   datum chiave;
   long LastCode, MaxCode;
#ifdef NOREGISTER
   return(1000);
#endif   
   if ((regdb=gdbm_open(NOMEDB1,4096,GDBM_READER,DBMASK,NULL)))
   {
      chiave.dsize =7;
      chiave.dptr ="~~~~~~";
      contenuto =gdbm_fetch(regdb,chiave);
      if (contenuto.dptr)
      {
         sscanf(contenuto.dptr, "%06ld %06ld", &LastCode, &MaxCode);
         free(contenuto.dptr);
         gdbm_close(regdb);
         return MaxCode;
      }
      else
      {
         gdbm_close(regdb);
         return 0L;
      }
   }
   else
   {
      mudlog(LOG_SYSERR,"Error register RegCount(): %s Chiave %s, %s\n", NOMEDB1, "~~~~~~", gdbm_strerror(gdbm_errno));
   }
   return 0L;
}

bool ck_immortal(char *nome)
{
   register int i;
   sh_int max;
   struct char_file_u tmp_store;

   if (load_char(lower(nome), &tmp_store))
   {
      for (i=0, max=0; i< MAX_CLASS; i++)
         if (tmp_store.level[i] > max)
            max = tmp_store.level[i];
      if (max > IMMORTAL) // SALVO tolgo l'uguale per allinearsi alla wizlist, setto max=0 nel for
         return TRUE;
   }
   return FALSE;
}

void pwd2info()
{
   char newpwd[8], pass[8];
   register int i;
   extern char RandomChar();

   for (i=0; i<8; i++)
      newpwd[i] =RandomChar();
         
   newpwd[8] ='\0';
   
   mudlog(LOG_PLAYERS,"La pass generata -1- \E8: %s",newpwd);
   
   //strcpy(pass,newpwd);
   
   strcpy(RI.info[REG_ALTRO], newpwd);
   //RI.info[REG_ALTRO]=newpwd;
   //RI.info[5]=newpwd;
   mudlog(LOG_PLAYERS,"Nuova password per %s",RI.info[REG_PC]);
   mudlog(LOG_PLAYERS,"La pass salvata \E8: %s",RI.info[REG_ALTRO]);
}

bool GeneraMail()
{
   FILE *fd;
   char buf[81];
   char nome[81];

  sprintf(nome,"/tmp/%smail",RI.info[REG_PC]);
   if ((fd = fopen(nome, "wt+")) != NULL)
   {
      fputs("Ciao,\n", fd);
      fputs("la tua richiesta di registrazione e' stata eseguita.\n", fd);
      fprintf(fd,"%s si consacra a %s\n",RI.info[REG_PC],RI.info[REG_DIO]);
      fprintf(fd, "La nuova password di %s e' %s\n", RI.info[REG_PC], RI.info[REG_ALTRO]);
      mudlog(LOG_SYSERR, "La pass mandata per email \E8: %s", RI.info[REG_ALTRO]);
      fputs("\nGrazie e Buon Divertimento\nLo staff di Nebbie Arcane\n", fd);
      fclose(fd);
      sprintf(buf,"mail -s \"Registrazione %s\" %s <%s", 
	      RI.info[REG_PC],
	      RI.info[REG_EMAIL],
	      nome);
      system(buf);
      unlink(nome);
      return TRUE;
   }
   return FALSE;
}

bool chgpwd()
{
   char name[30], npasswd[20], pass[20];
   char szFileName[50];
   struct char_file_u tmp_store;
   FILE *fl;
   
   strcpy(name,RI.info[REG_PC]);
   strcpy(npasswd,RI.info[REG_ALTRO]);
   
   mudlog(LOG_SYSERR,"Cmabio password a %s (%s)",name, npasswd);
   
   if (load_char(name,&tmp_store))
   {
   		mudlog(LOG_SYSERR,"La nuova pass salvata \E8: %s",npasswd);
      /*  encrypt new password. */
      bzero(pass, sizeof(pass)) ;
#ifdef CYGWIN
      strncpy(pass,(char *)npasswd, sizeof(pass)-1);
#else
      strncpy(pass,(char *)crypt(npasswd, tmp_store.name), sizeof(pass)-1);
#endif
	 
      /*  put new password in place of old password */
      bzero(tmp_store.pwd,sizeof(tmp_store.pwd));
      strncpy( tmp_store.pwd, pass,sizeof(tmp_store.pwd)-1 );
      
      /*   save char to file */
      sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( name ) );
      if( ( fl = fopen( szFileName, "w+b" ) ) == NULL )
      {
	 mudlog( LOG_ERROR, "Cannot open file %s for saving player.", szFileName );
	 return FALSE;
      }
      fwrite( &tmp_store, sizeof( tmp_store ), 1, fl );
      fclose( fl );
      return TRUE;
   }
   mudlog(LOG_SYSERR,"Reg: non e' stato possibile cambiare la password a %s.\n", szFileName);
   return FALSE;
}

bool autorent(struct char_data *victim)
{
    struct obj_cost cost;
    
    mudlog(LOG_SYSERR,"Comincio autorent");
    
    if (victim->desc)
     write_to_descriptor(victim->desc->descriptor,"Autorent!");
    
    mudlog(LOG_SYSERR,"secondo if");
    
    mudlog(LOG_SYSERR,"stanza dove sono: %i",victim->in_room);
    
    //forzo la reception che ho casini...
    
    victim->in_room=1000;
    
    if (victim->in_room != NOWHERE)
      char_from_room(victim);
    
    mudlog(LOG_SYSERR,"dopo secondo if");
    
    char_to_room(victim, 4);
    
    mudlog(LOG_SYSERR,"terzo if (close soket)");
    
    mudlog(LOG_SYSERR,"desc? %s",victim->desc->descriptor);
    
    if (victim->desc){
      mudlog(LOG_SYSERR,"Chiudo il socket");
      //close_socket(victim->desc);
      mudlog(LOG_SYSERR,"Socket Chiuso");
    }
    victim->desc = 0;
    
    if (recep_offer(victim, NULL, &cost,1))
    {
      mudlog(LOG_SYSERR,"forzo reception");
      cost.total_cost = 100;
      save_obj( victim, &cost,1 );
    }
    mudlog(LOG_SYSERR,"Fine sistemo il pg");
    extract_char(victim);
    return TRUE;
}

#ifndef ALAR
int main(int argc, char **argv)
#define P0 "Alar"
#define P1 argv[1]
#define P2 argv[2]
#define P3 argv[3]
#define P4 argv[4]
#define P5 argv[5]
#define P6 argv[6]
#define PRINT printf("%s",buf)
#else
void doreg(struct char_data *ch,int argc,char *argv[10])
#define P0 lower(GET_NAME(ch))
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
    * 2) register new <nome> <email> <god> 
    *     registra il player
    * 3) register delete <nome>
    *     il peronsaggio viene cancellato dalle registrazioni
    * 4) register add <nome1> <nome2> <god>
    *     nome1 viene registrato come nuovo personaggio di nome2
    * 5) register email <nome1> <email address>
    *    varia la mail di un giocatore
    * 6) register show <nome>
    *    mostra tutti i personaggi di un giocatore
    * */
   
   int command=0;
   char buf[500];
   char tbuf[500];
   char *c1[10];
   char *tmp;
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
   if (argc==0) command=0; /* Help */
   else if (argc==1) command=1; /* Nome */
   else if (argc==2 && !strcasecmp(P1,"show")) command=6;
   else if (argc==4 && !strcasecmp(P1,"new")) command=2;
   else if (argc==4 && !strcasecmp(P1,"add")) command=4;
   else if (argc==2 && !strcasecmp(P1,"delete") && GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) command=3;
   else if (argc==3 && !strcasecmp(P1,"email")) command = 5;
   else command =0;
#ifdef NOREGISTER
   sprintf(buf,"Il comando register e' temporaneamente inattivo\r\n");
   PRINT;
#if ALAR
   if (! IS_IMMENSO(ch))
     return;
#endif   
#endif   
    
#if ALAR
   mudlog(LOG_PLAYERS,"Reg: %s %d %s %s %s %s %s %s",
	  P0,command,P1,P2,P3,P4,P5,P6);
#else
   printf("Reg: %s %d %s %s %s %s %s %s\n",
	  P0,command,P1,P2,P3,P4,P5,P6);
#endif


mudlog(LOG_SYSERR,"ENTRO?!?");
//preparo il db....
   
mudlog(LOG_SYSERR,"2ENTRO?!?");
   switch (command) 
   {
    case 0: 
      {
         if (IS_MAESTRO_DEL_CREATO(ch))
	    sprintf(buf,"Il comando register ha 6 formati\r\n\r\n");
         else
	    sprintf(buf,"Il comando register ha 5 formati\r\n\r\n");
      	 PRINT;
         sprintf(buf,"register <nome> Visualizza i dati di un personaggio\r\n");
	 PRINT;
         sprintf(buf,"register new <nome> <email> <nome dio> Registra un personaggio\r\n"); 
	 PRINT;
         if (IS_MAESTRO_DEL_CREATO(ch))
         {
            sprintf(buf,"register delete <nome> cancella dalle registrazioni un personaggio\r\n"); 
	    PRINT;
         }
         sprintf(buf,"register add <nome1> <nome2> <nome dio> Registra un secondo personaggio a nome2.\n  Nome2 deve essere gia` stato registrato\r\n");
	 PRINT;
         sprintf(buf,"register email <nome> <email address> Varia la email\r\n");
	 PRINT;
         sprintf(buf,"register show <nome> Mostra tutti i personaggi di un player\r\n"); 
	 PRINT;
	 sprintf(buf,"N.B.\n\r  Ricordarsi di sostituire eventuali spazi con underscore\n\r");
	 PRINT;
         sprintf(buf,"  Per scegliere <nome dio> guarda la wizlist.\n\r");
	 PRINT;
         sprintf(buf,"  $c0001ATTENZIONE $c0015che il personaggio viene rentato automaticamente e viene\r\n");
	 PRINT;
         sprintf(buf,"    cambiata la password al personaggio registrato, la nuova password\r\n");
	 PRINT;
         sprintf(buf,"    vi verra' inviata all'indirizzo di email che avete specificato.$c0007\r\n");
	 PRINT;

	 break;
      }
    case 1: 
      {
/* nome */
         RegInfoClean();
         if (!IS_MAESTRO_DEL_CREATO(ch) && strcasecmp(P1, P0))
         {
	    sprintf(buf,"Puoi vedere solo la tua registrazione.\r\n");
	    PRINT;
            break;
         }
         strcpy(RI.info[REG_PC], P1);
         if (gdbGet()){
	    sprintf(buf,"%s\r\n",Info2Char(GetMaxLevel(ch), buf));
	    mudlog(LOG_SYSERR,"!!!!!!!!! %s",buf);}
	 else {
	    sprintf(buf,"%s non registrato.\n", RI.info[REG_PC]);
	    mudlog(LOG_PLAYERS,"+++++++++ %s",buf);}
	 PRINT;
	 break;
      }
    case 2: /* new */
      {
        mudlog(LOG_SYSERR,"inizio registrazione");
         RegInfoClean();
        mudlog(LOG_SYSERR,"RegInfoClean1");
         if (!IS_MAESTRO_DEL_CREATO(ch) && strcasecmp(P2, P0))
         {
	         sprintf(buf,"Puoi registrare solo il tuo PC.\r\n");
	         PRINT;
           break;
         }
         strcpy(RI.info[REG_PC], P2);
         mudlog(LOG_SYSERR,"sto registrando *%s*",RI.info[REG_PC]);
         
         mudlog(LOG_SYSERR,"GDBGET");
         
         if (gdbGet())
         {
	         sprintf(buf,"%s e' gia' stato registrato\n",RI.info[REG_PC]);
	         PRINT;
	         break;
	       }
         else
         {
          RegInfoClean();
          strcpy(RI.info[REG_PC], P2);
          strcpy(RI.info[REG_EMAIL], P3);
          if (ck_immortal(P4))
          {
            strcpy(RI.info[REG_DIO], P4);
            mudlog(LOG_SYSERR,"sto registrando *%s*",RI.info[REG_PC]);
            mudlog(LOG_SYSERR,"alla mail *%s*",RI.info[REG_EMAIL]);
            mudlog(LOG_SYSERR,"dio protettore *%s*",RI.info[REG_DIO]);
            if (gdbPut(NEW))
            {
   	           sprintf(buf,"Registrazione effettuata\n");
   	           mudlog(LOG_SYSERR,buf);
	             PRINT;
               pwd2info();
               mudlog(LOG_SYSERR,"Genero la nuova password");
               /*if (GeneraMail())
               {*/
	               sprintf(buf,"Messaggio inviato correttamente a %s.\n", RI.info[REG_EMAIL]);
	               PRINT;
	               sprintf(buf,"Rent Automatico su %s.\n",RI.info[REG_PC]);
	               PRINT;
                 mudlog(LOG_SYSERR,"prima del change pass");
                 if (IS_MAESTRO_DEL_CREATO(ch))
                 {
                 mudlog(LOG_SYSERR,"prima del change pass2");
                    sprintf(tbuf,"forcerent %s",RI.info[REG_PC]);
                    command_interpreter(ch,tbuf);
                 } else {
                 mudlog(LOG_SYSERR,"prima del change pass3 e autorent");
                   // autorent(ch);
                 }
                 mudlog(LOG_SYSERR,"Adesso: change pass");
                 chgpwd();
               /*} else {
	                 sprintf(buf,"Errore nell'invio del messaggio.\n");
	                 PRINT;
               }*/
             }
             else
            {
   	                sprintf(buf,"Registrazione fallita");
	                PRINT;
                     }
               }
               else
               {
   	          sprintf(buf,"Devi sceglire un vero Dio, guarda wizlist");
	          PRINT;
               }
         }
         break;
      }
    case 3: /* delete */
      {
/* cancellazione */
         RegInfoClean();
         strcpy(RI.info[REG_PC], P2);
         if (gdbGet())
	 {
            if (gdbDel())
	       sprintf(buf,"Cancellazione effettuata");
            else
	       sprintf(buf,"Cancellazione fallita");
	    PRINT;
	 }
         else
	 {
	    sprintf(buf,"%s non registrato.\n", P2);
	    PRINT;
	    break;
	 }
	 break;
      }
    case 4: /* da altro pc */
      {
/* creazione */
         RegInfoClean();
         if (!IS_MAESTRO_DEL_CREATO(ch) && strcasecmp(P3, P0))
         {
	    sprintf(buf,"Puoi registrare %s solo se entri in gioco con %s.\r\n", P2,P3);
	    PRINT;
            break;
         }
         strcpy(RI.info[REG_PC], P2);
         if (!gdbGet())
	 {
            strcpy(RI.info[REG_PC], P3);
            if (!gdbGet())
            {
	       sprintf(buf,"%s non registrato.\n", P3);
	       PRINT;
	       break;
            }
            else
            {
               strcpy(RI.info[REG_PC], P2);
               if (ck_immortal(P4))
               {
                  strcpy(RI.info[REG_DIO], P4);
                  if (gdbPut(ADD))
	             sprintf(buf,"Registrazione effettuata");
                  else
	             sprintf(buf,"Registrazione fallita");
               }
               else
   	          sprintf(buf,"Devi sceglire un vero Dio, guarda wizlist");
	       PRINT;
            }
	 }
         else
	 {
	    sprintf(buf,"%s gia' registrato.\n", RI.info[REG_PC]);
	    PRINT;
	    break;
	 }
         break;
      }
    case 5: /* mail */
      {
         RegInfoClean();
         if (!IS_MAESTRO_DEL_CREATO(ch) && strcasecmp(P2, P0))
         {
	    sprintf(buf,"Puoi modificare solo il tuo personaggio.\r\n");
	    PRINT;
            break;
         }
         strcpy(RI.info[REG_PC], P2);
         if (!gdbGet())
	 {
	    sprintf(buf,"%s non registrato.\n", RI.info[REG_PC]);
	    PRINT;
	    break;
	 }
         else
         {
            strcpy(RI.info[REG_EMAIL], P3);
            if (gdbPut(CHG))
               sprintf(buf,"Sostituzione effettuata\n");
            else
               sprintf(buf,"Sostituzione fallita\n");
            PRINT;
         }
	 break;
      }
    case 6:
      {
/* show */
         RegInfoClean();
         if (!IS_MAESTRO_DEL_CREATO(ch) && strcasecmp(P2, P0))
         {
	    sprintf(buf,"Puoi vedere solo il tuo personaggio.\r\n");
	    PRINT;
            break;
         }
         strcpy(RI.info[REG_PC], P2);
         if (!gdbGet())
	    sprintf(buf,"%s non registrato.\n", RI.info[REG_PC]);
         else
	    sprintf(buf,"Totale %ld / %ld.\n", gdbShow(ch), RegCount());
	 PRINT;
	 break;
      }
   }
}
bool regdelete(char *nome)
{
   /* cancellazione */
#ifdef NOREGISTER
mudlog(LOG_SYSERR,"PROOOOOOOOOOOOOOOOOT!");
   return(TRUE);
#endif   

mudlog(LOG_SYSERR,"PROOOOOOOOOOOOOOOOOT222222222222222!");
   RegInfoClean();
   strcpy(RI.info[REG_PC], lower(nome));
   if (gdbGet())
   {
	   if (gdbDel()) {
	     mudlog(LOG_PLAYERS,"Cancellazione %s effettuata",nome);
	     return(TRUE);
	   }
	 else{ 
	  mudlog(LOG_PLAYERS,"Cancellazione %sfallita",nome);
   }
   }
   return(FALSE);
}
#endif
