/*AlarMUD*/
/* $Id: interpreter.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
 * */
#define _XOPEN_SOURCE 
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef CYGWIN
#include <crypt.h>
#endif
#include "snew.h"
#include "protos.h"
#include "cmdid.h"
#include "breath.h"
#include "version.h"
#include "utility.h"
#include "Registered.hpp"
using Nebbie::Registered;
#define NOT !
#define AND &&
#define OR ||
#define SEND_TO_Q2(msg,d) write_to_descriptor(d->descriptor,msg);
#define STATE(d) ((d)->connected)
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern const char  *RaceName[];
extern const int RacialMax[MAX_RACE+1][MAX_CLASS];
extern const int RacialHome[MAX_RACE+1][2];
extern char motd[MAX_STRING_LENGTH];
extern char wmotd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern char *pc_class_types[];
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif

unsigned char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
unsigned char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
int HowManyConnection(int ToAdd);
int WizLock;
int Silence = 0;
long SystemFlags=0;         /* used for making teleport/astral/stuff not work */
int plr_tick_count=0;


void show_race_choice(struct descriptor_data *d);

/* this is how we tell which race gets which class ! */
/* to add a new class seletion add the CLASS_NAME above the */
/* message 'NEW CLASS SELETIONS HERE'  */

/* these are the allowable PC races */
const int race_choice[]= 
{
 RACE_DWARF,
 RACE_ELVEN,
 RACE_GOLD_ELF,
 RACE_WILD_ELF,
 RACE_HUMAN,
 RACE_GNOME,
   RACE_HALFLING,       /*Nascono allo shire*/
 RACE_HALF_ELVEN,
 RACE_HALF_ORC,
 RACE_HALF_OGRE,
 RACE_HALF_GIANT,
   RACE_DROW,                /* bad guys here */
#if 1 /*Era 0, adesso tutte le razze gia' previste sono abilitate. GGPATCH*/
   RACE_ORC,             /*Nascono allo shire */
   RACE_GOBLIN,          /*Nascono allo shire */
   RACE_TROLL,           /*Nascono allo shire */
   RACE_DARK_DWARF,      /*Nascono allo shire */
   RACE_DEEP_GNOME,      /*Nascono allo shire */
   RACE_SEA_ELF,         /*Nascono vicino al mare */
 RACE_DEMON,
#endif  
 -1  
};

/* WARNING: do not remove the 0 at the end of the const, will cause your */
/*             mud to die hard! */


const int elf_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_MONK,
 CLASS_PALADIN,
 CLASS_RANGER,
 CLASS_PSI,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
 CLASS_RANGER+CLASS_CLERIC,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int wild_elf_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_RANGER,
 CLASS_PSI,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_CLERIC+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_DRUID,
 CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_CLERIC,
 CLASS_WARRIOR+CLASS_CLERIC,
 CLASS_RANGER+CLASS_CLERIC,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int dwarf_class_choice[]= 
{
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_PALADIN,
 CLASS_PSI,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_CLERIC,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int halfling_class_choice[]=
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_MONK,
 CLASS_THIEF+CLASS_WARRIOR,
 CLASS_DRUID+CLASS_WARRIOR,
 CLASS_PSI+CLASS_WARRIOR,
 CLASS_PSI+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int gnome_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_MONK,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_PSI,
 CLASS_PSI,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int deep_gnome_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_MONK,
 CLASS_CLERIC+CLASS_MONK,
 CLASS_CLERIC+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int human_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_MONK,
 CLASS_BARBARIAN,
 CLASS_PALADIN,
 CLASS_RANGER,
 CLASS_PSI,
 CLASS_WARRIOR+CLASS_DRUID,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_CLERIC,
 CLASS_WARRIOR+CLASS_CLERIC+CLASS_MAGIC_USER,
 CLASS_THIEF+CLASS_CLERIC+CLASS_MAGIC_USER,
   /* NEW CLASS SELECTIONS HERE */
 0
};


const int half_orc_class_choice[]= 
{
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_CLERIC+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int half_ogre_class_choice[]= 
{
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_CLERIC+CLASS_WARRIOR,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int half_giant_class_choice[]= 
{
 CLASS_WARRIOR,
 CLASS_BARBARIAN,
 CLASS_PSI,
   /* NEW CLASS SELECTIONS HERE */
 0
};


const int orc_class_choice[]= 
{
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC,
 CLASS_BARBARIAN,
 CLASS_THIEF+CLASS_CLERIC,
   /* new class below here */
 0
};

const int goblin_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC,
 CLASS_BARBARIAN,
 CLASS_WARRIOR+CLASS_THIEF,
   /* new class below here */  
 0
};

const int demon_class_choice[]=
{
 CLASS_MAGIC_USER,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_MONK,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_MONK+CLASS_THIEF,
 CLASS_THIEF+CLASS_BARBARIAN,
 0
};

const int troll_class_choice[]= 
{
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC,
 CLASS_BARBARIAN,
 CLASS_WARRIOR+CLASS_CLERIC,
   /* new clases below here */  
 0
};

const int default_class_choice[]= 
{
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC,
 CLASS_MAGIC_USER,
 CLASS_BARBARIAN,
 CLASS_PALADIN,
 CLASS_RANGER,
 CLASS_PSI,
   /* new clases below here */
 0
};

const int dark_dwarf_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_MONK,
 CLASS_CLERIC+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_WARRIOR+CLASS_CLERIC,
   /* NEW CLASS SELECTIONS HERE */
 0
};

const int dark_elf_class_choice[]= 
{
 CLASS_MAGIC_USER,
 CLASS_CLERIC,
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_DRUID,
 CLASS_MONK,
 CLASS_PALADIN,
 CLASS_RANGER,
 CLASS_PSI,
 CLASS_MAGIC_USER+CLASS_CLERIC,
 CLASS_WARRIOR+CLASS_MAGIC_USER,
 CLASS_WARRIOR+CLASS_THIEF,
 CLASS_MAGIC_USER+CLASS_THIEF,
 CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */
 0
};

void do_cset(struct char_data *ch, char *arg, int cmd);
void do_auth(struct char_data *ch, char *arg, int cmd); /* jdb 3-1 */
void do_wiznews(struct char_data *ch, char *arg, int cmd); /* poi lo inseriro' in protos*/


char *fill[]=
{ 
 "in",
 "from",
 "with",
 "and",
 "the",
 "on",
 "at",
 "to",
 "\n"
};


int search_block(char *arg, char **list, bool exact)
{
 register int i,l;
 
   /* Make into lower case, and get length of string */
 for(l=0; *(arg+l); l++)
   *(arg+l)=LOWER(*(arg+l));
 
 if (exact) {
  for(i=0; **(list+i) != '\n'; i++)
    if (!strcmp(arg, *(list+i)))
      return(i);
  } else {
    if (!l)
l=1; /* Avoid "" to match the first available string */
      for(i=0; **(list+i) != '\n'; i++)
        if (!strncmp(arg, *(list+i), l))
          return(i);
      }
      
      return(-1);
    }


    int old_search_block(char *argument,int begin,int length,char **list,int mode)
    {
     int guess, found, search;
     
     
   /* If the word contain 0 letters, then a match is already found */
     found = (length < 1);
     
     guess = 0;
     
   /* Search for a match */
     
     if(mode)
       while ( NOT found AND *(list[guess]) != '\n' )      
       {
        found=(length==(int)strlen(list[guess]));
        for(search=0;( search < length AND found );search++)
          found=(*(argument+begin+search)== *(list[guess]+search));
        guess++;
      } else {
        while ( NOT found AND *(list[guess]) != '\n' ) {
          found=1;
          for(search=0;( search < length AND found );search++)
           found=(*(argument+begin+search)== *(list[guess]+search));
         guess++;
       }
     }
     
     return ( found ? guess : -1 ); 
   }

   void command_interpreter( struct char_data *ch, char *argument )
   {
     char buf[254];
     extern int no_specials;        
     NODE *n;
     char buf1[255], buf2[255];
     struct char_data *temp;
     
     REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

 /* PARRY is removed as HIDE GAIA 2001 */

     if(IS_AFFECTED2(ch,AFF2_PARRY) && !ch->specials.fighting)
     {
      act( "$c0006$n smette di ripararsi con lo scudo.", TRUE, ch, 0, 0, TO_ROOM);
      act( "$c0006Smetti di proteggerti con lo scudo.", TRUE, ch, 0, 0, TO_CHAR);
      REMOVE_BIT( ch->specials.affected_by2, AFF2_PARRY );
    }                                                                                      

    if (ch->listening_to)
    {
     REMOVE_FROM_LIST(ch, real_roomp(ch->listening_to)->listeners, next_listener);
     ch->listening_to = 0;
     act( "Smetti di origliare la stanza.", TRUE, ch, 0, 0, TO_CHAR);
   }

   if(IS_AFFECTED2(ch,AFF2_AFK)) 
   {
    act( "$c0006$n ritorna alla sua tastiera", TRUE, ch, 0, 0, TO_ROOM);
    act( "$c0006Ritorni alla tua tastiera.", TRUE, ch, 0, 0, TO_CHAR);
    REMOVE_BIT( ch->specials.affected_by2, AFF2_AFK );
  }
  
  if( MOUNTED( ch ) ) 
  {
    if( ch->in_room != MOUNTED(ch)->in_room )
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
  }
  
   /*
    *  a bug check.
    */
  if( !IS_NPC(ch) )
  {
    int i, found=FALSE;
    if ((!ch->player.name[0]) || (ch->player.name[0]<' ')) 
    {
      mudlog( LOG_SYSERR, "Error in character name.  Changed to 'Error'");
      free(ch->player.name);
      ch->player.name = (char *)malloc(10);
      strncpy(ch->player.name, "Error\0",6); 
/*    return; Vediamo se stracrasha o no.... */
    }
    strcpy(buf, ch->player.name);
    for (i = 0; i< (int)strlen(buf) && !found; i++) 
    {
      if (buf[i]<65) 
      {
        found = TRUE;
      }
    }
    if (found) 
    {
      mudlog( LOG_SYSERR, "Error in character name.  Changed to 'Error'");
      free(ch->player.name);
      ch->player.name = (char *)malloc(6);
      strcpy(ch->player.name, "Error");
/*    return; Vediamo se stracrasha o no.... */
    }
   } /* Fine if IS_NPC */
    
   /* Toglie gli spazi prima del comando. */
    for(;isspace(*argument); argument++) ;
     
     if( *argument && *argument != '\n' )
     {
      if( !isalpha( *argument ) ) 
      {
        buf1[0] = *argument;
        buf1[1] = '\0';
        if( argument + 1 )
         strcpy( buf2, argument + 1 );
       else
         buf2[0] = '\0';
     }
     else 
     {
      register int i;
      half_chop(argument, buf1, buf2);
      i = 0;
      while(buf1[i] != '\0') 
      {
        buf1[i] = LOWER(buf1[i]);
        i++;   
      }
    }
    
      /* New parser by DM */
    if( *buf1 )
      n = FindValidCommand( buf1 );
    else 
      n = NULL;
    
      /* GGNOTA test sui livelli dei comandi*/
    
    if( n != NULL && GetMaxLevel( ch ) >= n->min_level ) 
    {
      if( n->func != 0 )
      {
        if( GET_POS( ch ) < n->min_pos ) 
        {
         switch(GET_POS(ch))
         {
          case POSITION_DEAD:
          send_to_char( "Non puoi essere ancora qui. Sei MORTO!!!\n\r", 
           ch );
          break;
          case POSITION_INCAP:
          case POSITION_MORTALLYW:
          send_to_char( "Sei in una brutta posizione per poter fare "
           "qualunque cosa!\n\r", ch);
          break;
          
          case POSITION_STUNNED:
          send_to_char( "Tutto quello che puoi fare ora, e` respirare "
           "e sperare che passi!\n\r", ch);
          break;
          case POSITION_SLEEPING:
          send_to_char("Nei tuoi sogni o cosa?\n\r", ch);
          break;
          case POSITION_RESTING:
          send_to_char( "Nah... Sei troppo rilassato...\n\r", ch);
          break;
          case POSITION_SITTING:
          send_to_char( "Prima forse dovresti alzarti, non credi?\n\r",ch);
          break;
          case POSITION_FIGHTING:
          send_to_char( "Non puoi! Stai combattendo per la tua vita!\n\r", 
           ch);
          break;
          case POSITION_STANDING:
          send_to_char( "Ho paura che tu non lo possa fare.\n\r", ch);
          break;
       } /* switch */
        } 
        else if( !IS_AFFECTED( ch, AFF_PARALYSIS ) )
        {
         
       /* They can't move, must have pissed off an immo!         */
       /* make sure polies can move, some mobs have this bit set */ 
         
         if( IS_SET( ch->specials.act, PLR_FREEZE ) && 
          ( IS_SET( ch->specials.act, ACT_POLYSELF) || IS_PC( ch ) ) )
         {
          send_to_char( "Sei stato immobilizzato e non puoi fare "
           "nulla!\n\r", ch );
        }
        else
        {
          
          if( ( IS_SET( SystemFlags, SYS_LOGALL ) && 
           ( IS_PC( ch ) || 
            IS_SET( ch->specials.act, ACT_POLYSELF ) ) ) ||
           ( IS_SET( SystemFlags, SYS_LOGMOB ) && 
            ( IS_NPC( ch ) && 
             !IS_SET( ch->specials.act, ACT_POLYSELF ) ) ) )
            
          {
           mudlog( LOG_CHECK | LOG_SILENT,
            "[%5ld]ACMD %s:%s", ch->in_room, ch->player.name, 
            argument);
         } 
         else if( n->log )
         {
           mudlog( LOG_CHECK | LOG_SILENT,
            "[%5ld]CCMD %s:%s", ch->in_room, ch->player.name, 
            argument);
         }
         else if( IS_AFFECTED2( ch, AFF2_LOG_ME ) ) 
         {
           mudlog( LOG_CHECK | LOG_SILENT,
            "[%5ld]PCMD %s:%s", ch->in_room, ch->player.name, 
            argument);
         } 
         else if( GetMaxLevel( ch ) >= IMMORTALE && 
          GetMaxLevel( ch ) < 60 ) 
         {
           mudlog( LOG_CHECK | LOG_SILENT,
            "[%5ld]ICMD %s:%s", ch->in_room, ch->player.name, 
            argument );
         }
         else if (GET_GOLD(ch) > 2000000) 
         {
           mudlog( LOG_CHECK | LOG_SILENT,
            "[%5ld]GCMD %s:%s", ch->in_room, ch->player.name, 
            argument );
         }
         
  /* special() restituisce TRUE se il comando e` stato 
   * interpretato da una procedura speciale.
   */
         PushStatus(argument);
         MARKS("Prima del test procedure speciali");
         if( no_specials || !special( ch, n->number, buf2 ) )
         {
     /* Finalmente viene esequito il comando */
           MARKS("Esecuzione comando");
           ( ( *n->func )( ch, buf2, n->number ) );
         }
         PopStatus();
       }
     } 
     else 
      send_to_char( "Sei paralizzato, non puoi fare molto!\n\r",ch);
  }
else /* n->func == 0 */
  send_to_char( "Mi spiace, ma il comando non e` stato ancora "
    "implementato.\n\r",ch);
}
      else /* n == NULL || GetMaxLevel( ch ) < n->min_level */
send_to_char("Pardon?\n\r", ch);
}

}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
 int look_at, begin;
 
 begin = 0;
 
 do
 {
      /* Find first non blank */
  for( ; *( argument + begin ) == ' ' ; begin++ ) ;
    
      /* Find length of first word */
    for( look_at = 0; *( argument + begin + look_at ) > ' ' ; look_at++ )
/* Make all letters lower case, AND copy them to first_arg */
      *( first_arg + look_at ) = LOWER( *( argument + begin + look_at ) );
    
    *( first_arg + look_at ) = '\0';
    begin += look_at;
  }
  while( fill_word( first_arg ) );
  
  do
  {
      /* Find first non blank */
    for( ;*( argument + begin ) == ' ' ; begin++ ) ;
      
      /* Find length of first word */
      for( look_at = 0; *( argument + begin + look_at ) > ' ' ; look_at++ )
/* Make all letters lower case, AND copy them to second_arg */
        *( second_arg + look_at ) = LOWER( *( argument + begin + look_at ) );
      
      *( second_arg + look_at ) = '\0';
      begin += look_at;
      
    }
    while( fill_word(second_arg));
  }

  void ThreeArgumentInterpreter( char *pchArgument, char *pchFirstArg, 
    char *pchSecondArg, char *pchThirdArg )
  {
   int nLookAt, nBegin;
   
   nBegin = 0;
   
   do
   {
      /* Find first non blank */
    for( ; *( pchArgument + nBegin ) == ' ' ; nBegin++ ) ;
      
      /* Find length of first word */
      for( nLookAt = 0; *( pchArgument + nBegin + nLookAt ) > ' ' ; nLookAt++ )
/* Make all letters lower case, AND copy them to first_arg */
        *( pchFirstArg + nLookAt ) = LOWER( *( pchArgument + nBegin + nLookAt ) );
      
      *( pchFirstArg + nLookAt ) = '\0';
      nBegin += nLookAt;
    }
    while( fill_word( pchFirstArg ) );
    
    do
    {
      /* Find first non blank */
      for( ;*( pchArgument + nBegin ) == ' ' ; nBegin++ ) ;
        
      /* Find length of first word */
        for( nLookAt = 0; *( pchArgument + nBegin + nLookAt ) > ' ' ; nLookAt++ )
/* Make all letters lower case, AND copy them to second_arg */
          *( pchSecondArg + nLookAt ) = 
        LOWER( *( pchArgument + nBegin + nLookAt ) );
        
        *( pchSecondArg + nLookAt ) = '\0';
        nBegin += nLookAt;
        
      }
      while( fill_word( pchSecondArg ) );
      
      do
      {
      /* Find first non blank */
        for( ;*( pchArgument + nBegin ) == ' ' ; nBegin++ ) ;
          
      /* Find length of first word */
          for( nLookAt = 0; *( pchArgument + nBegin + nLookAt ) > ' ' ; nLookAt++ )
/* Make all letters lower case, AND copy them to second_arg */
            *( pchThirdArg + nLookAt ) = LOWER( *( pchArgument + nBegin + nLookAt ) );
          
          *( pchThirdArg + nLookAt ) = '\0';
          nBegin += nLookAt;
          
        }
        while( fill_word( pchThirdArg ) );
      }

      int is_number( char *str )
      {
       int look_at;
       
       if (*str =='-' || *str =='+')
        look_at =1;
      else
        look_at =0;
      for( ; str[ look_at ] != '\0'; look_at++ )
       if( str[ look_at ] < '0' || str[ look_at ] > '9' )
         return FALSE; 
       return TRUE; 
     }

/* find the first sub-argument of a string, return pointer to first char in
 *  primary argument, following the sub-arg  
 */
     char *one_argument(char *argument, char *first_arg )
     {
   /* Ritorna un argomento, ignorando le parole definite in filler */
       int begin,look_at;
       
       begin = 0;
       
       do
       {
      /* Find first non blank */
        for ( ;isspace(*(argument + begin)); begin++);
          
      /* Find length of first word */
          for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

/* Make all letters lower case,
* AND copy them to first_arg */
            *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
          
          *(first_arg + look_at)='\0';
          begin += look_at;
        } while (fill_word(first_arg));
        
        return(argument+begin);
      }

      char *OneArgumentNoFill( char *argument, char *first_arg )
      {
       int begin,look_at;
       
       begin = 0;
       
   /* Find first non blank */
       for ( ;isspace(*(argument + begin)); begin++);
         
   /* Find length of first word */
         for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
           
     /* Make all letters lower case,
      * AND copy them to first_arg */
           *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
         
         *(first_arg + look_at)='\0';
         begin += look_at;
         
         return(argument+begin);
       }


       void only_argument(char *argument, char *dest)
/* Trimma i blanks iniziali e copia argument in dest */
       {
         while (*argument && isspace(*argument))
           argument++;
         strcpy(dest, argument);
       }




       int fill_word(char *argument)
       {
         return ( search_block(argument,fill,TRUE) >= 0);
       }





/* determine if a given string is an abbreviation of another */
       int is_abbrev(char *arg1, char *arg2)
       {
         if (!*arg1)
           return(0);
         
         for (; *arg1; arg1++, arg2++)
           if (LOWER(*arg1) != LOWER(*arg2))
             return(0);
           
           return(1);
         }




/* return first 'word' plus trailing substring of input string */
/* Si aspetta che il primo parametro sia il nome di un personaggio
 * Siccome potrebbe non essere e potrei quindi trovarmi con un buffer
 * overrun, eseguo in temporanei e poi copio solo i primi 50 caratteri
 * del primo argomento.
 * Come bonus, tmp1 e tmp2 sono fillate a zero
 * */
         void half_chop(char *string, char *arg1, char *arg2)
         {
           char *tmp1;
           char *tmp2;
           char *p1;
           char *p2;
           int i;
           i=0;
           tmp1=(char *)calloc(1,strlen(string)+1);
           tmp2=(char *)calloc(1,strlen(string)+1);
           p1=tmp1;
           p2=tmp2;
           for (; isspace(*string); string++);
             
             for (; !isspace(*p1 = *string) && *string && i <49; i++,string++, p1++);
               *p1='\0';
             for (; isspace(*p1) && *p1; p1++);
               
               for (; isspace(*string); string++);
                 
                 for (; ( *p2 = *string ) != 0; string++, p2++);
   /* Poto tmp1 a 49 caratteri ovvero a una stringa lunga max 50 */
                   
                   strncpy(arg1,tmp1,50);
                 strcpy(arg2,tmp2);
                 free(tmp1);
                 free(tmp2);
               }



               int special(struct char_data *ch, int cmd, char *arg)
               {
                 register struct obj_data *i;
                 register struct char_data *k;
                 int j;
                 MARKS("Special procedure");
                 if( ch->in_room == NOWHERE )
                 {
                  char_to_room( ch, 3001 );
                  return FALSE;
                }
                
   /* special in room? */
                if( real_roomp( ch->in_room )->funct )
                {
                  PushStatus(real_roomp( ch->in_room )->specname);
                  if( ( *real_roomp( ch->in_room )->funct )( ch, cmd, arg, 
                    real_roomp( ch->in_room ), 
                    EVENT_COMMAND ) )
                  {
                    PopStatus();
                    return( TRUE );
                  }
                  PopStatus();
                }
                
   /* special in equipment list? */
#define CUR_OBJ ch->equipment[j]
                
                for( j = 0; j <= ( MAX_WEAR - 1 ); j++ )
                {
                  if( CUR_OBJ && CUR_OBJ->item_number >= 0 )
                  {
                    if( IS_SET( CUR_OBJ->obj_flags.extra_flags, ITEM_ANTI_SUN ) ) 
                     AntiSunItem( ch, cmd, arg, CUR_OBJ, EVENT_COMMAND );
                   if( CUR_OBJ)
                   {
                    if( obj_index[ CUR_OBJ->item_number ].func ) 
                    {
                     PushStatus(obj_index[CUR_OBJ->item_number].specname);
                     if( ( *obj_index[ CUR_OBJ->item_number ].func )
                      ( ch, cmd, arg, CUR_OBJ, EVENT_COMMAND ) )
                     {
                      PopStatus();
                      return( TRUE );
                    }
                    PopStatus();
                  }
                }                 
              }         
            }                
            
   /* special in inventory? */
            for( i = ch->carrying; i; i = i->next_content )
             if( i->item_number >= 0 )
               if( obj_index[ i->item_number ].func )
               {
                PushStatus(obj_index[i->item_number].specname);
                if( ( *obj_index[ i->item_number ].func )( ch, cmd, arg, i,
                  EVENT_COMMAND ) )
                {
                  PopStatus();
                  return( TRUE );
                }
                PopStatus();
              }
              
   /* special in mobile present? */
              for( k = real_roomp( ch->in_room )->people; k; k = k->next_in_room )
               if( IS_MOB( k ) )
                 if( mob_index[ k->nr ].func )
                 {
                  PushStatus(mob_index[k->nr].specname);
                  
                  if( ( *mob_index[ k->nr ].func )( ch, cmd, arg, k, EVENT_COMMAND ) )
                  {
                    PopStatus();
                    return( TRUE );
                  }
                  PopStatus();
                }
                
   /* special in object present? */
                for( i = real_roomp( ch->in_room )->contents; i; i = i->next_content )
                 if( i->item_number >=0 )
                   if( obj_index[ i->item_number ].func )
                   {
                    PushStatus(obj_index[i->item_number].specname);
                    if( ( *obj_index[ i->item_number ].func)( ch, cmd, arg, i, 
                     EVENT_COMMAND ) )
                    {
                      PopStatus();
                      return( TRUE );
                    }
                    PopStatus();
                  }
                  
                  return( FALSE );
                }

                void assign_command_pointers ()
                {
                 InitRadix();
                 AddCommand("north", do_move, CMD_NORTH, POSITION_STANDING, TUTTI );
                 AddCommand("east",  do_move, CMD_EAST,  POSITION_STANDING, TUTTI );
                 AddCommand("south", do_move, CMD_SOUTH, POSITION_STANDING, TUTTI );
                 AddCommand("west",  do_move, CMD_WEST,  POSITION_STANDING, TUTTI );
                 AddCommand("up",    do_move, CMD_UP,    POSITION_STANDING, TUTTI );
                 AddCommand("down",  do_move, CMD_DOWN,  POSITION_STANDING, TUTTI );
                 AddCommand("enter", do_enter, CMD_ENTER, POSITION_STANDING, TUTTI );
                 AddCommand("exits",do_exits,8,POSITION_RESTING,TUTTI);
                 AddCommand("kiss",do_action,9,POSITION_RESTING,TUTTI);
                 AddCommand("get", do_get, CMD_GET,POSITION_RESTING,1);
                 AddCommand("drink",do_drink,11,POSITION_RESTING,1);
                 AddCommand("eat",do_eat,12,POSITION_RESTING,1);
                 AddCommand("wear",do_wear,13,POSITION_RESTING,TUTTI);
                 AddCommand("wield",do_wield,14,POSITION_RESTING,1);
                 AddCommand("look", do_look, CMD_LOOK, POSITION_RESTING, TUTTI );
                 AddCommand("score",do_score,16,POSITION_DEAD,TUTTI);
                 AddCommand("say", do_new_say, CMD_SAY, POSITION_RESTING, TUTTI );
                 AddCommand("shout", do_shout, CMD_SHOUT,POSITION_RESTING,2);
                 AddCommand("tell", do_tell, CMD_TELL, POSITION_RESTING, TUTTI );
                 AddCommand("inventory",do_inventory,20,POSITION_DEAD,TUTTI);
                 AddCommand("qui",do_qui,21,POSITION_DEAD,TUTTI);
                 AddCommand("bounce",do_action,22,POSITION_STANDING,TUTTI);
                 AddCommand("smile",do_action,23,POSITION_RESTING,TUTTI);
                 AddCommand("dance",do_action,24,POSITION_STANDING,TUTTI);
                 AddCommand("kill",do_kill,25,POSITION_FIGHTING,1);
                 AddCommand("cackle",do_action,26,POSITION_RESTING,TUTTI);
                 AddCommand("laugh",do_action,27,POSITION_RESTING,TUTTI);
                 AddCommand("giggle",do_action,28,POSITION_RESTING,TUTTI);
                 AddCommand("shake", do_action, CMD_SHAKE, POSITION_RESTING, TUTTI );
                 AddCommand("puke",do_vomita,30,POSITION_RESTING,TUTTI);
                 AddCommand("growl",do_action,31,POSITION_RESTING,TUTTI);
                 AddCommand("scream",do_action,32,POSITION_RESTING,TUTTI);
                 AddCommand("insult",do_insult,33,POSITION_RESTING,TUTTI);
                 AddCommand("comfort",do_action,34,POSITION_RESTING,TUTTI);
                 AddCommand("nod",do_action,35,POSITION_RESTING,TUTTI);
                 AddCommand("sigh",do_action,36,POSITION_RESTING,TUTTI);
                 AddCommand("sulk",do_action,37,POSITION_RESTING,TUTTI);
                 AddCommand("help",do_help,38,POSITION_DEAD,TUTTI);
                 AddCommand("who", do_who, CMD_WHO, POSITION_DEAD, TUTTI );
                 AddCommand("emote",do_emote,40,POSITION_SLEEPING,TUTTI);
                 AddCommand(":",do_emote,40,POSITION_SLEEPING,TUTTI);
                 AddCommand(",",do_emote,177,POSITION_SLEEPING,TUTTI);
                 AddCommand("echo",do_echo,41,POSITION_SLEEPING,1);
                 AddCommand("stand",do_stand,42,POSITION_RESTING,TUTTI);
                 AddCommand("sit",do_sit,43,POSITION_RESTING,TUTTI);
                 AddCommand("rest",do_rest,44,POSITION_RESTING,TUTTI);
                 AddCommand("sleep",do_sleep,45,POSITION_SLEEPING,TUTTI);
                 AddCommand("wake",do_wake,46,POSITION_SLEEPING,TUTTI);
                 AddCommand("force",do_force,47,POSITION_SLEEPING,MAESTRO_DEL_CREATO);
                 AddCommand("transfer",do_trans,48,POSITION_SLEEPING,QUESTMASTER);
                 AddCommand("hug",do_action,49,POSITION_RESTING,TUTTI);
                 AddCommand("snuggle",do_action,50,POSITION_RESTING,TUTTI);
                 AddCommand("cuddle",do_action,51,POSITION_RESTING,TUTTI);
                 AddCommand("nuzzle",do_action,52,POSITION_RESTING,TUTTI);
                 AddCommand("cry",do_action,53,POSITION_RESTING,TUTTI);
                 AddCommand("news",do_news,54,POSITION_SLEEPING,TUTTI);
                 AddCommand("equipment",do_equipment,55,POSITION_SLEEPING,TUTTI);
                 AddCommand("buy", do_not_here, CMD_BUY, POSITION_STANDING, TUTTI );
                 AddCommand("sell", do_not_here, CMD_SELL, POSITION_STANDING, TUTTI );
                 AddCommand("value", do_value, CMD_VALUE, POSITION_RESTING, TUTTI );
                 AddCommand("list", do_not_here, CMD_LIST, POSITION_STANDING, TUTTI );
                 AddCommand("drop", do_drop, CMD_DROP, POSITION_RESTING, 1 );
                 AddCommand("goto",do_goto,61,POSITION_SLEEPING,IMMORTALE);
                 AddCommand("weather",do_weather,62,POSITION_RESTING,TUTTI);
                 AddCommand("read", do_read, CMD_READ, POSITION_RESTING, TUTTI );
                 AddCommand("pour",do_pour,64,POSITION_STANDING,TUTTI);
                 AddCommand("grab",do_grab,65,POSITION_RESTING,TUTTI);
                 AddCommand("remove", do_remove, CMD_REMOVE, POSITION_RESTING, TUTTI );
                 AddCommand("put",do_put,67,POSITION_RESTING,TUTTI);
                 AddCommand("shutdow",do_shutdow,68,POSITION_DEAD,IMMENSO);
                 AddCommand("save",do_save,69,POSITION_SLEEPING,TUTTI);
                 AddCommand("hit",do_hit,70,POSITION_FIGHTING,1);
                 AddCommand("string",do_string,71,POSITION_SLEEPING,DIO);
                 AddCommand("give", do_give, CMD_GIVE, POSITION_RESTING, 1 );
                 AddCommand("quit",do_quit,73,POSITION_DEAD,TUTTI);
                 AddCommand("wiznews",do_wiznews,74,POSITION_DEAD,IMMORTALE);
                 AddCommand("guard",do_guard,75,POSITION_STANDING,1);
                 AddCommand("time",do_time,76,POSITION_DEAD,TUTTI);
                 AddCommand("oload",do_oload,77,POSITION_DEAD,QUESTMASTER);
                 AddCommand("purge",do_purge,78,POSITION_DEAD,DIO_MINORE);
                 AddCommand("shutdown", do_shutdown, 79, POSITION_DEAD,CREATORE);
                 AddCommand("idea",do_action,80,POSITION_DEAD,TUTTI);
                 AddCommand("typo",do_action,81,POSITION_DEAD,TUTTI);
                 AddCommand("bug",do_action,82,POSITION_DEAD,TUTTI);
                 AddCommand("whisper", do_whisper, CMD_WHISPER, POSITION_RESTING,TUTTI);
                 AddCommand("cast", do_cast, CMD_CAST, POSITION_SITTING, 1 );
                 AddCommand("know", do_cast, CMD_SPELLID, POSITION_SITTING, 1 );
                 AddCommand("at",do_at,85,POSITION_DEAD,DIO);
                 AddCommand("ask", do_ask, CMD_ASK, POSITION_RESTING,TUTTI);
                 AddCommand("order",do_order,87,POSITION_RESTING,1);
                 AddCommand("sip",do_sip, 88,POSITION_RESTING,TUTTI);
                 AddCommand("taste",do_taste,89,POSITION_RESTING,TUTTI);
                 AddCommand("snoop",do_snoop,90,POSITION_DEAD,QUESTMASTER);
                 AddCommand("follow",do_follow,91,POSITION_RESTING,TUTTI);
                 AddCommand("rent",do_not_here,CMD_RENT,POSITION_STANDING,1);
                 AddCommand("offer",do_not_here,CMD_OFFER,POSITION_STANDING,1);
   //AddCommand("store",do_not_here,CMD_STORE,POSITION_STANDING,1);
                 AddCommand("poke",do_action,94,POSITION_RESTING,TUTTI);
                 AddCommand("advance",do_advance,95,POSITION_DEAD,MAESTRO_DEI_CREATORI);
                 AddCommand("accuse",do_action,96,POSITION_SITTING,TUTTI);
                 AddCommand("grin",do_action,97,POSITION_RESTING,TUTTI);
                 AddCommand("bow",do_action,98,POSITION_STANDING,TUTTI);
                 
                 AddCommand( "open",   do_open,   CMD_OPEN,   POSITION_SITTING, TUTTI );
                 AddCommand( "close",  do_close,  CMD_CLOSE,  POSITION_SITTING, TUTTI );
                 AddCommand( "lock",   do_lock,   CMD_LOCK,   POSITION_SITTING, TUTTI );
                 AddCommand( "unlock", do_unlock, CMD_UNLOCK, POSITION_SITTING, TUTTI );
                 
                 AddCommand("leave",do_leave,103,POSITION_STANDING,TUTTI);
                 AddCommand("applaud",do_action,104,POSITION_RESTING,TUTTI);
                 AddCommand("blush",do_action,105,POSITION_RESTING,TUTTI);
                 AddCommand("burp",do_action,106,POSITION_RESTING,TUTTI);
                 AddCommand("chuckle",do_action,107,POSITION_RESTING,TUTTI);
                 AddCommand("nose",do_action,108,POSITION_RESTING,TUTTI); 
                 AddCommand("cough",do_action,109,POSITION_RESTING,TUTTI);
                 AddCommand("curtsey",do_action,110,POSITION_STANDING,TUTTI);
                 AddCommand("rise",do_action,111,POSITION_RESTING,DIO);
                 AddCommand("flip",do_action,112,POSITION_STANDING,TUTTI);
                 AddCommand("fondle",do_action,113,POSITION_RESTING,TUTTI);
                 AddCommand("frown",do_action,114,POSITION_RESTING,TUTTI);
                 AddCommand("gasp",do_action,115,POSITION_RESTING,TUTTI);
                 AddCommand("glare", do_action, CMD_GLARE, POSITION_RESTING, TUTTI );
                 AddCommand("groan",do_action,117,POSITION_RESTING,TUTTI);
                 AddCommand("grope",do_action,118,POSITION_RESTING,TUTTI);
                 AddCommand("hiccup",do_action,119,POSITION_RESTING,TUTTI);
                 AddCommand("lick",do_action,120,POSITION_RESTING,TUTTI);
                 AddCommand("love",do_action,121,POSITION_RESTING,TUTTI);
                 AddCommand("moan",do_action,122,POSITION_RESTING,TUTTI);
                 AddCommand("nibble",do_action,123,POSITION_RESTING,TUTTI);
                 AddCommand("pout",do_action,124,POSITION_RESTING,TUTTI);
                 AddCommand("purr",do_action,125,POSITION_RESTING,TUTTI);
                 AddCommand("ruffle",do_action,126,POSITION_STANDING,TUTTI);
                 AddCommand("shiver",do_action,127,POSITION_RESTING,TUTTI);
                 AddCommand("shrug",do_action,128,POSITION_RESTING,TUTTI);
                 AddCommand("sing",do_action,129,POSITION_RESTING,TUTTI);
                 AddCommand("slap",do_action,130,POSITION_RESTING,TUTTI);
                 AddCommand("smirk",do_action,131,POSITION_RESTING,TUTTI);
                 AddCommand("support",do_support,CMD_SUPPORT,POSITION_STANDING,TUTTI);
                 AddCommand("sneeze",do_action,133,POSITION_RESTING,TUTTI);
                 AddCommand("snicker",do_action,134,POSITION_RESTING,TUTTI);
                 AddCommand("sniff",do_action,135,POSITION_RESTING,TUTTI);
                 AddCommand("snore",do_action,136,POSITION_SLEEPING,TUTTI);
                 AddCommand("spit",do_action,137,POSITION_STANDING,TUTTI);
                 AddCommand("squeeze",do_action,138,POSITION_RESTING,TUTTI);
                 AddCommand("associa",do_associa,CMD_ASSOCIA,POSITION_STANDING,PRINCIPE);
                 AddCommand("stare",do_action,139,POSITION_RESTING,TUTTI);
                 AddCommand("associate",do_associa,CMD_ASSOCIA,POSITION_STANDING,PRINCIPE);
                 AddCommand("thank",do_action,141,POSITION_RESTING,TUTTI);
                 AddCommand("twiddle",do_action,142,POSITION_RESTING,TUTTI);
                 AddCommand("wave",do_action,143,POSITION_RESTING,TUTTI);
                 AddCommand("whistle",do_action,144,POSITION_RESTING,TUTTI);
                 AddCommand("wiggle",do_action,145,POSITION_STANDING,TUTTI);
                 AddCommand("wink",do_action,146,POSITION_RESTING,TUTTI);
                 AddCommand("yawn",do_action,147,POSITION_RESTING,TUTTI);
                 AddCommand("snowball",do_action,148,POSITION_STANDING,IMMORTALE);
                 AddCommand("write", do_write, CMD_WRITE, POSITION_STANDING, 1 );
                 AddCommand("hold",do_grab,150,POSITION_RESTING,1);
                 AddCommand("flee", do_flee, CMD_FLEE, POSITION_SITTING, 1 );
                 AddCommand("sneak",do_sneak,152,POSITION_STANDING,1);
                 AddCommand("hide",do_hide,153,POSITION_RESTING,1);
                 AddCommand("camouflage",do_hide,153,POSITION_STANDING,1);     
                 AddCommand("backstab",do_backstab,154,POSITION_STANDING,1);
                 AddCommand("pick",do_pick,155,POSITION_STANDING,1);
                 AddCommand("steal",do_steal,156,POSITION_STANDING,1);
                 AddCommand("bash", do_bash, CMD_BASH, POSITION_FIGHTING, 1 );
                 AddCommand("rescue",do_rescue,158,POSITION_FIGHTING,1);
                 AddCommand("kick",do_kick,159,POSITION_FIGHTING,1);
                 AddCommand("french",do_action,160,POSITION_RESTING,ALLIEVO);
                 AddCommand("comb",do_action,161,POSITION_RESTING,TUTTI);
                 AddCommand("massage",do_action,162,POSITION_RESTING,TUTTI);
                 AddCommand("tickle",do_action,163,POSITION_RESTING,TUTTI);
                 AddCommand("practice", do_practice, CMD_PRACTICE, POSITION_RESTING, 1 );
                 AddCommand("practise", do_practice, CMD_PRACTICE, POSITION_RESTING, 1 );
                 AddCommand("pat", do_action, CMD_PAT, POSITION_RESTING, TUTTI );
                 AddCommand("examine", do_examine, 166, POSITION_RESTING, TUTTI );
   AddCommand("take", do_get, CMD_TAKE, POSITION_RESTING, 1 ); /* TAKE */
                 AddCommand("info",do_info,168,POSITION_SLEEPING,TUTTI);
                 AddCommand("'", do_new_say, CMD_SAY_APICE, POSITION_RESTING, TUTTI );
                 AddCommand("curse",do_action,171,POSITION_RESTING,5);
                 AddCommand("use", do_use, CMD_USE, POSITION_SITTING, 1 );
                 AddCommand("where",do_where,173,POSITION_DEAD,1);
                 AddCommand("levels",do_levels,174,POSITION_DEAD,TUTTI);
                 AddCommand("register",do_register,175,POSITION_DEAD,4);
                 
                 AddCommand("pray",do_pray,176,POSITION_SITTING,1);
                 
                 AddCommand("beg",do_action,178,POSITION_RESTING,TUTTI);
                 AddCommand("bleed",do_not_here,179,POSITION_RESTING,TUTTI);
                 AddCommand("cringe",do_action,180,POSITION_RESTING,TUTTI);
                 AddCommand("daydream",do_action,181,POSITION_SLEEPING,TUTTI);
                 AddCommand("fume",do_action,182,POSITION_RESTING,TUTTI);
                 AddCommand("grovel",do_action,183,POSITION_RESTING,TUTTI);
                 AddCommand("hop",do_action,184,POSITION_RESTING,TUTTI);
                 AddCommand("nudge",do_action,185,POSITION_RESTING,TUTTI);
                 AddCommand("peer",do_action,186,POSITION_RESTING,TUTTI);
                 AddCommand("point",do_action,187,POSITION_RESTING,TUTTI);
                 AddCommand("ponder",do_action,188,POSITION_RESTING,TUTTI);
                 AddCommand("punch",do_action,189,POSITION_RESTING,TUTTI);
                 AddCommand("snarl",do_action,190,POSITION_RESTING,TUTTI);
                 AddCommand("spank",do_action,191,POSITION_RESTING,TUTTI);
                 AddCommand("steam",do_action,192,POSITION_RESTING,TUTTI);
                 AddCommand("tackle",do_action,193,POSITION_RESTING,TUTTI);
                 AddCommand("taunt",do_action,194,POSITION_RESTING,TUTTI);
                 AddCommand("think",do_commune,195,POSITION_RESTING,IMMORTALE);
                 AddCommand("\"", do_commune, 195, POSITION_RESTING, IMMORTALE);
                 AddCommand("?",do_commune,CMD_THINK_SUPERNI,POSITION_RESTING,CREATORE);
                 AddCommand("whine",do_action,196,POSITION_RESTING,TUTTI);
                 AddCommand("worship",do_action,197,POSITION_RESTING,TUTTI);
                 AddCommand("yodel",do_action,198,POSITION_RESTING,TUTTI);
                 AddCommand("brief",do_brief,199,POSITION_DEAD,TUTTI);
                 AddCommand("wizlist",do_wizlist,200,POSITION_DEAD,TUTTI);
                 AddCommand("consider",do_consider,201,POSITION_RESTING,TUTTI);
                 AddCommand("group",do_group,202,POSITION_RESTING,1);
                 AddCommand("restore",do_restore,203,POSITION_DEAD,MAESTRO_DEGLI_DEI);
                 AddCommand("return",do_return,204,POSITION_RESTING,TUTTI);
                 AddCommand("switch",do_switch  ,205,POSITION_DEAD,CREATORE);
                 AddCommand("quaff",do_quaff,206,POSITION_RESTING,TUTTI);
                 AddCommand("recite",do_recite,207,POSITION_STANDING,TUTTI);
                 AddCommand("users",do_users,208,POSITION_DEAD,DIO_MINORE);
                 AddCommand("pose",do_pose,209,POSITION_STANDING,TUTTI);
                 AddCommand("noshout",do_noshout,210,POSITION_SLEEPING,IMMORTALE);
                 AddCommand("wizhelp",do_wizhelp,211,POSITION_SLEEPING,DIO_MINORE);
                 AddCommand("credits",do_credits,212,POSITION_DEAD,TUTTI);
                 AddCommand("compact",do_compact,213,POSITION_DEAD,TUTTI);
                 AddCommand("daimoku",do_daimoku,CMD_DAIMOKU,POSITION_SLEEPING,1);
                 AddCommand("forge",do_forge,CMD_FORGE,POSITION_STANDING,1);
                 AddCommand("slay",do_slay,216,POSITION_FIGHTING,DIO_MINORE);
                 AddCommand("wimpy",do_wimp,217,POSITION_DEAD,TUTTI);
                 AddCommand("junk",do_junk,218,POSITION_RESTING,1);
                 AddCommand("deposit", do_not_here, CMD_DEPOSIT, POSITION_RESTING, 1 );
                 AddCommand("withdraw", do_not_here, CMD_WITHDRAW, POSITION_RESTING, 1 );
                 AddCommand("balance", do_not_here, CMD_BALANCE, POSITION_RESTING, 1 );
                 AddCommand("nohassle",do_nohassle,222,POSITION_DEAD,DIO);
                 AddCommand("system",do_system,223,POSITION_DEAD,QUESTMASTER);
                 AddCommand("pull", do_open_exit, CMD_PULL, POSITION_STANDING,1);
                 AddCommand("stealth",do_stealth,225,POSITION_DEAD,DIO);
                 AddCommand("edit",do_edit,226,POSITION_DEAD,MAESTRO_DEGLI_DEI);
                 AddCommand("@",do_set,227,POSITION_DEAD,MAESTRO_DEI_CREATORI);
                 AddCommand("rsave",do_rsave,228,POSITION_DEAD,MAESTRO_DEGLI_DEI);
                 AddCommand("rload",do_rload,229,POSITION_DEAD,MAESTRO_DEGLI_DEI);
                 AddCommand("track",do_track,230,POSITION_DEAD,1);
                 AddCommand("wizlock",do_wizlock,231,POSITION_DEAD,IMMENSO);
                 AddCommand("highfive",do_highfive,232,POSITION_DEAD,TUTTI);
                 AddCommand("title",do_title,233,POSITION_DEAD,INIZIATO-1);
                 AddCommand("whozone", do_who, CMD_WHOZONE, POSITION_DEAD, TUTTI );
                 AddCommand("assist",do_assist,235,POSITION_FIGHTING,1);
                 AddCommand("attribute",do_attribute,236,POSITION_DEAD,5);
                 AddCommand("world",do_world,237,POSITION_DEAD,TUTTI);
                 AddCommand("allspells",do_spells,238,POSITION_DEAD,TUTTI);
                 AddCommand("breath",do_breath,239,POSITION_FIGHTING,1);
                 AddCommand("show",do_show,240,POSITION_DEAD,DIO_MINORE);
                 AddCommand("debug",do_debug,241,POSITION_DEAD,MAESTRO_DEI_CREATORI);
                 AddCommand("invisible",do_invis,242,POSITION_DEAD,IMMORTALE);
                 AddCommand("gain", do_gain, CMD_GAIN, POSITION_DEAD, 1 );
                 
                 AddCommand("mload",do_mload,244,POSITION_DEAD,DIO); 
                 
                 AddCommand("disarm",do_disarm,245,POSITION_FIGHTING,1);
                 AddCommand("bonk",do_action,246,POSITION_SITTING,3);
                 AddCommand("chpwd",do_passwd,247,POSITION_SITTING,MAESTRO_DEI_CREATORI);
                 AddCommand("passwd",do_passwd,247,POSITION_SITTING,MAESTRO_DEI_CREATORI);
                 AddCommand("fill",do_not_here,248,POSITION_SITTING,TUTTI);
                 AddCommand("mantra",do_mantra,CMD_MANTRA,POSITION_SITTING,TUTTI);
                 AddCommand("shoot",do_fire,250,POSITION_STANDING,  1);
                 AddCommand("fire" ,do_fire ,250, POSITION_DEAD,TUTTI);
                 AddCommand("silence",do_silence,251,POSITION_STANDING, DIO);  
                 
                 AddCommand("teams", do_not_here, CMD_TEAMS, POSITION_STANDING, MAESTRO_DEL_CREATO );  
                 AddCommand("player", do_not_here, CMD_PLAYER, POSITION_STANDING,MAESTRO_DEL_CREATO );
                 
                 AddCommand("create",do_create,254,POSITION_STANDING, QUESTMASTER);  
                 AddCommand("bamfin",do_bamfin,255,POSITION_STANDING,IMMORTALE);
                 AddCommand("bamfout",do_bamfout,256,POSITION_STANDING, IMMORTALE);
                 AddCommand("vis",do_invis,257,POSITION_RESTING,  TUTTI);  
                 AddCommand("doorbash", do_doorbash, CMD_DOORBASH, POSITION_STANDING, 1 );
                 AddCommand("mosh",do_action,259,POSITION_FIGHTING, 1);
                 
   /* alias commands */
                 AddCommand("alias",do_alias,260,POSITION_SLEEPING, 1);
                 AddCommand("1", do_alias,261,POSITION_DEAD, 1);
                 AddCommand("2", do_alias,262,POSITION_DEAD, 1);
                 AddCommand("3", do_alias,263,POSITION_DEAD, 1);
                 AddCommand("4", do_alias,264,POSITION_DEAD, 1);
                 AddCommand("5", do_alias,265,POSITION_DEAD, 1);
                 AddCommand("6", do_alias,266,POSITION_DEAD, 1);
                 AddCommand("7", do_alias,267,POSITION_DEAD, 1);
                 AddCommand("8", do_alias,268,POSITION_DEAD, 1);
                 AddCommand("9", do_alias,269,POSITION_DEAD, 1);
                 AddCommand("0", do_alias,270,POSITION_DEAD, 1);
                 
                 AddCommand("swim", do_swim,271,POSITION_STANDING, 1);
                 AddCommand("spy", do_spy,272,POSITION_STANDING, 1);
                 AddCommand("springleap",do_springleap,273,POSITION_RESTING, 1);
                 AddCommand("quivering palm",do_quivering_palm,274,POSITION_FIGHTING, INIZIATO-1);
                 AddCommand("feign death",do_feign_death,275,POSITION_FIGHTING, 1);
                 AddCommand("mount",do_mount,276,POSITION_STANDING, 1);
                 AddCommand("dismount",do_mount,277,POSITION_MOUNTED, 1);
                 AddCommand("ride",do_mount,278,POSITION_STANDING, 1);
                 AddCommand("sign",do_sign,279, POSITION_RESTING, 1);
                 
   /* had to put this here BEFORE setsev so it would get this and not setsev */
                 AddCommand("set",do_set_flags,280,POSITION_DEAD,TUTTI);
                 
                 AddCommand("first aid",do_first_aid,281, POSITION_RESTING, 1);
                 AddCommand("log",do_set_log,282, POSITION_DEAD, MAESTRO_DEL_CREATO);
                 AddCommand("recall", do_cast, CMD_RECALL, POSITION_SITTING, 1 );
                 AddCommand("reload",reboot_text,284, POSITION_DEAD, MAESTRO_DEI_CREATORI);
                 AddCommand("event",do_event,285, POSITION_DEAD, MAESTRO_DEI_CREATORI);
                 AddCommand("disguise",do_disguise,286, POSITION_STANDING, 1);
                 AddCommand("climb",do_climb,287, POSITION_STANDING,1);
                 AddCommand("beep",do_beep,288, POSITION_DEAD, DIO_MINORE);
                 AddCommand("bite",do_action,289, POSITION_RESTING, 1);
                 AddCommand("redit", do_redit, 290, POSITION_SLEEPING, MAESTRO_DEGLI_DEI);
                 AddCommand("display", do_display, 291, POSITION_SLEEPING, 1);
                 AddCommand("resize", do_resize, 292, POSITION_SLEEPING, 1);
                 AddCommand("ripudia",do_ripudia,CMD_RIPUDIA, POSITION_STANDING,VASSALLO);
                 AddCommand("#", do_cset, 294, POSITION_DEAD, QUESTMASTER);
                 AddCommand("spell", do_stat, 295, POSITION_DEAD, DIO_MINORE);
                 AddCommand("name", do_stat, 296, POSITION_DEAD, DIO_MINORE);
                 AddCommand("stat", do_stat, 297, POSITION_DEAD, MAESTRO_DEGLI_DEI);
                 AddCommand("immortali", do_immort, 298, POSITION_DEAD, 1);
                 AddCommand("auth", do_auth, 299, POSITION_SLEEPING, MAESTRO_DEGLI_DEI);
                 AddCommand("sacrifice", do_not_here, 300, POSITION_STANDING, MEDIUM);
                 AddCommand("nogossip",do_plr_nogossip,301,POSITION_RESTING,TUTTI);
                 AddCommand("gossip", do_gossip, CMD_GOSSIP, POSITION_RESTING,TUTTI);
                 AddCommand("noauction",do_plr_noauction,303,POSITION_RESTING,TUTTI);
                 AddCommand("auction", do_auction,CMD_AUCTION,POSITION_RESTING,ALLIEVO);
                 AddCommand("discon",do_disconnect,305,POSITION_RESTING,DIO);
                 AddCommand("freeze",do_freeze,306,POSITION_SLEEPING,QUESTMASTER);
                 AddCommand("drain",do_drainlevel,307,POSITION_SLEEPING,MAESTRO_DEI_CREATORI);
                 AddCommand("oedit",do_oedit,308,POSITION_DEAD,QUESTMASTER);
                 AddCommand("report",do_report,309,POSITION_RESTING,1);
                 AddCommand("interven",do_god_interven,310,POSITION_DEAD,MAESTRO_DEI_CREATORI);
                 AddCommand("gtell", do_gtell, CMD_GTELL, POSITION_SLEEPING, 1 );
                 AddCommand("raise",do_action,312,POSITION_RESTING,1);
                 AddCommand("tap",do_action,313,POSITION_STANDING,1);
                 AddCommand("liege",do_action,314,POSITION_RESTING,1);
                 AddCommand("sneer",do_action,315,POSITION_RESTING,1);
                 AddCommand("howl",do_action,316,POSITION_RESTING,1);
                 AddCommand("kneel", do_action, CMD_KNEEL, POSITION_STANDING, 1 );
                 AddCommand("finger",do_finger,318,POSITION_RESTING,1);
                 AddCommand("pace",do_action,319,POSITION_STANDING,1);  
                 AddCommand("tongue",do_action,320,POSITION_RESTING,1);
                 AddCommand("flex",do_action,321,POSITION_STANDING,1);
                 AddCommand("ack",do_action,322,POSITION_RESTING,1);
                 AddCommand("ckeq",do_stat,CMD_CKEQ,POSITION_RESTING,MAESTRO_DEGLI_DEI);
                 AddCommand("caress",do_action,324,POSITION_RESTING,1);  
                 AddCommand("cheer",do_action,325,POSITION_RESTING,1);  
                 AddCommand("jump",do_action,326,POSITION_STANDING,1);  
                 
                 AddCommand("join", do_action, CMD_JOIN, POSITION_RESTING, 1 );
                 
                 AddCommand("split",do_split,328,POSITION_RESTING,1);
                 AddCommand("berserk",do_berserk,329,POSITION_FIGHTING,1);  
                 AddCommand("tan",do_tan,330,POSITION_STANDING,1);
                 
                 AddCommand("memorize",do_memorize,331,POSITION_RESTING,1);     
                 
                 AddCommand("find",do_find,332,POSITION_STANDING,1);    
                 AddCommand("bellow",do_bellow,333,POSITION_FIGHTING,1);      
                 AddCommand("carve",do_carve,335,POSITION_STANDING,1);  
                 AddCommand("nuke",do_nuke,336,POSITION_DEAD,IMMENSO);     
                 AddCommand("skills",do_show_skill,337,POSITION_SLEEPING,TUTTI);
                 
                 AddCommand("doorway",do_doorway,338,POSITION_STANDING,TUTTI);
                 AddCommand("portal",do_psi_portal,339,POSITION_STANDING,TUTTI);
                 AddCommand("summon",do_mindsummon,340,POSITION_STANDING,TUTTI);
                 AddCommand("canibalize",do_canibalize,341,POSITION_STANDING,TUTTI);
                 AddCommand("cannibalize",do_canibalize,341,POSITION_STANDING,TUTTI);
                 AddCommand("flame",do_flame_shroud,342,POSITION_STANDING,TUTTI);
                 AddCommand("aura",do_aura_sight,343,POSITION_RESTING,TUTTI);
                 AddCommand("great",do_great_sight,344,POSITION_RESTING,TUTTI);
                 AddCommand("psionic invisibility",do_invisibililty,345,POSITION_STANDING,TUTTI);
                 AddCommand("blast",do_blast,346,POSITION_FIGHTING,TUTTI);
                 
                 AddCommand("medit",do_medit,347,POSITION_DEAD,QUESTMASTER);
                 
                 AddCommand("hypnotize",do_hypnosis,348,POSITION_STANDING,TUTTI);
                 AddCommand("scry",do_scry,349,POSITION_RESTING,TUTTI);
                 AddCommand("adrenalize",do_adrenalize,350,POSITION_STANDING,TUTTI);
                 AddCommand("brew",do_brew,351,POSITION_STANDING,TUTTI);
                 AddCommand("meditate",do_meditate,352,POSITION_RESTING,TUTTI);
                 AddCommand("forcerent",do_force_rent,353,POSITION_DEAD,QUESTMASTER);    
                 AddCommand("warcry",do_holy_warcry,354,POSITION_FIGHTING,TUTTI);
                 AddCommand("lay on hands",do_lay_on_hands,355,POSITION_RESTING,TUTTI);
                 AddCommand("blessing",do_blessing,356,POSITION_STANDING,TUTTI);
                 AddCommand("heroic",do_heroic_rescue,357,POSITION_FIGHTING,TUTTI);
                 AddCommand("scan",do_scan,358,POSITION_STANDING,DIO_MINORE);    
                 AddCommand("shield",do_psi_shield,359,POSITION_STANDING,TUTTI);
                 AddCommand("notell",do_plr_notell,360,POSITION_DEAD,TUTTI);
                 AddCommand("commands",do_command_list,361,POSITION_DEAD,TUTTI);
                 AddCommand("ghost",do_ghost,362,POSITION_DEAD,MAESTRO_DEL_CREATO);
                 AddCommand("speak",do_speak,363,POSITION_DEAD,TUTTI);
                 AddCommand("setsev",do_setsev,364, POSITION_DEAD, DIO_MINORE);
                 AddCommand("esp",do_esp,365, POSITION_STANDING,TUTTI);
                 AddCommand("mail",do_not_here,366, POSITION_STANDING,ALLIEVO-1);
                 AddCommand("check",do_not_here,367, POSITION_STANDING,TUTTI);
                 AddCommand("receive",do_not_here,368, POSITION_STANDING,TUTTI);
                 AddCommand("telepathy",do_telepathy,369, POSITION_RESTING,TUTTI);
                 AddCommand("mind", do_cast, CMD_MIND, POSITION_SITTING,TUTTI);
                 
                 AddCommand("twist", do_open_exit, CMD_TWIST, POSITION_STANDING,TUTTI);
                 AddCommand("turn", do_open_exit, CMD_TURN, POSITION_STANDING,TUTTI);
                 AddCommand("lift", do_open_exit, CMD_LIFT, POSITION_STANDING,TUTTI);
                 AddCommand("push", do_open_exit, CMD_PUSH, POSITION_STANDING,TUTTI);
                 
   /*AddCommand("zload", do_zload,375, POSITION_STANDING,MAESTRO_DEGLI_DEI); */               
                 AddCommand("zsave", do_zsave,376, POSITION_STANDING,MAESTRO_DEGLI_DEI);                
                 AddCommand("zclean",do_zclean ,377, POSITION_STANDING,MAESTRO_DEGLI_DEI);
                 AddCommand("wrebuild",do_WorldSave ,378, POSITION_STANDING,IMMENSO);
   AddCommand("wreset",do_wreset , CMD_WRESET, POSITION_STANDING,CREATORE); // SALVO aggiunto comando wreset
   AddCommand("gwho",list_groups ,379, POSITION_DEAD,TUTTI);            
   
   AddCommand("mforce" ,do_mforce ,380, POSITION_DEAD,DIO_MINORE ); /* CREATOR */
   AddCommand("clone" ,do_clone ,381, POSITION_DEAD,MAESTRO_DEGLI_DEI+2 );                
   AddCommand("bodyguard" ,do_bodyguard ,CMD_BODYGUARD, POSITION_STANDING,MEDIUM );                
   AddCommand("throw" ,do_throw ,383, POSITION_SITTING,TUTTI);            
   AddCommand("run" , do_run , CMD_RUN, POSITION_STANDING,TUTTI);
   AddCommand("notch" ,do_weapon_load ,385, POSITION_RESTING,TUTTI);            
   AddCommand("load" ,do_weapon_load ,385, POSITION_RESTING,TUTTI);            
   
   AddCommand("spot" ,do_scan ,387, POSITION_STANDING,TUTTI);            
   AddCommand("view" ,do_viewfile ,388, POSITION_DEAD,DIO);
   AddCommand("afk" ,do_set_afk,389, POSITION_DEAD,1);
   
   AddCommand( "stopfight", do_stopfight, CMD_STOPFIGHT, POSITION_FIGHTING,1);
   AddCommand("principi",do_prince,CMD_PRINCE,POSITION_RESTING,TUTTI);
   AddCommand("tspy",do_tspy,CMD_TSPY,POSITION_STANDING,1);
   AddCommand("bid", do_auction, CMD_BID, POSITION_RESTING, ALLIEVO );
   AddCommand("eavesdrop", do_eavesdrop, CMD_EAVESDROP, POSITION_STANDING,INIZIATO);
   AddCommand("pquest", do_pquest, CMD_PQUEST, POSITION_STANDING,QUESTMASTER);
   AddCommand("parry", do_parry, CMD_PARRY, POSITION_RESTING, TUTTI);                     
   /* Liberi 397-399 */
   
   /* lots of Socials */
   AddCommand("adore",do_action,400,POSITION_RESTING,TUTTI);
   AddCommand("agree",do_action,401,POSITION_RESTING,TUTTI);
   AddCommand("bleed",do_action,402,POSITION_RESTING,TUTTI);
   AddCommand("blink",do_action,403,POSITION_RESTING,TUTTI);
   AddCommand("blow",do_action,404,POSITION_RESTING,TUTTI);
   AddCommand("blame",do_action,405,POSITION_RESTING,TUTTI);
   AddCommand("bark",do_action,406,POSITION_RESTING,TUTTI);
   AddCommand("bhug",do_action,407,POSITION_RESTING,TUTTI);
   AddCommand("bcheck",do_action,408,POSITION_RESTING,TUTTI);
   AddCommand("boast",do_action,409,POSITION_RESTING,TUTTI);
   AddCommand("chide",do_action,410,POSITION_RESTING,TUTTI);
   AddCommand("compliment",do_action,411,POSITION_RESTING,TUTTI);
   AddCommand("ceyes",do_action,412,POSITION_RESTING,TUTTI);
   AddCommand("cears",do_action,413,POSITION_RESTING,TUTTI);
   AddCommand("cross",do_action,414,POSITION_RESTING,TUTTI);
   AddCommand("console",do_action,415,POSITION_RESTING,TUTTI);
   AddCommand("calm",do_action,416,POSITION_RESTING,TUTTI);
   AddCommand("cower",do_action,417,POSITION_RESTING,TUTTI);
   AddCommand("confess",do_action,418,POSITION_RESTING,TUTTI);
   AddCommand("drool",do_action,419,POSITION_RESTING,TUTTI);
   AddCommand("grit",do_action,420,POSITION_RESTING,TUTTI);
   AddCommand("greet",do_action,421,POSITION_RESTING,TUTTI);
   AddCommand("gulp",do_action,422,POSITION_RESTING,TUTTI);
   AddCommand("gloat",do_action,423,POSITION_RESTING,TUTTI);
   AddCommand("gaze",do_action,424,POSITION_RESTING,TUTTI);
   AddCommand("hum",do_action,425,POSITION_RESTING,TUTTI);
   AddCommand("hkiss",do_action,426,POSITION_RESTING,TUTTI);
   AddCommand("ignore",do_action,427,POSITION_RESTING,TUTTI);
   AddCommand("interrupt",do_action,428,POSITION_RESTING,TUTTI);
   AddCommand("knock",do_action,429,POSITION_RESTING,TUTTI);
   AddCommand("listen",do_action,430,POSITION_RESTING,TUTTI);
   AddCommand("muse",do_action,431,POSITION_RESTING,TUTTI);
   AddCommand("pinch",do_action,432,POSITION_RESTING,TUTTI);
   AddCommand("praise",do_action,433,POSITION_RESTING,TUTTI);
   AddCommand("plot",do_action,434,POSITION_RESTING,TUTTI);
   AddCommand("pie",do_action,435,POSITION_RESTING,TUTTI);
   AddCommand("setalign",do_setalign,436,POSITION_RESTING,TUTTI);
   AddCommand("pant",do_action,437,POSITION_RESTING,TUTTI);
   AddCommand("rub",do_action,438,POSITION_RESTING,TUTTI);
   AddCommand("roll",do_action,439,POSITION_RESTING,TUTTI);
   AddCommand("recoil",do_action,440,POSITION_RESTING,TUTTI);
   AddCommand("roar",do_action,441,POSITION_RESTING,TUTTI);
   AddCommand("relax",do_action,442,POSITION_RESTING,TUTTI);
   AddCommand("snap",do_action,443,POSITION_RESTING,TUTTI);
   AddCommand("strut",do_action,444,POSITION_RESTING,TUTTI);
   AddCommand("stroke",do_action,445,POSITION_RESTING,TUTTI);
   AddCommand("stretch",do_action,446,POSITION_RESTING,TUTTI);
   AddCommand("swave",do_action,447,POSITION_RESTING,TUTTI);
   AddCommand("sob",do_action,448,POSITION_RESTING,TUTTI);
   AddCommand("scratch",do_action,449,POSITION_RESTING,TUTTI);
   AddCommand("squirm",do_action,450,POSITION_RESTING,TUTTI);
   AddCommand("strangle",do_action,451,POSITION_RESTING,TUTTI);
   AddCommand("scowl",do_action,452,POSITION_RESTING,TUTTI);
   AddCommand("shudder",do_action,453,POSITION_RESTING,TUTTI);
   AddCommand("strip",do_action,454,POSITION_RESTING,TUTTI);
   AddCommand("scoff",do_action,455,POSITION_RESTING,TUTTI);
   AddCommand("salute",do_action,456,POSITION_RESTING,TUTTI);
   AddCommand("scold",do_action,457,POSITION_RESTING,TUTTI);
   AddCommand("stagger",do_action,458,POSITION_RESTING,TUTTI);
   AddCommand("toss",do_action,459,POSITION_RESTING,TUTTI);
   AddCommand("twirl",do_action,460,POSITION_RESTING,TUTTI);
   AddCommand("toast",do_action,461,POSITION_RESTING,TUTTI);
   AddCommand("tug",do_action,462,POSITION_RESTING,TUTTI);
   AddCommand("touch",do_action,463,POSITION_RESTING,TUTTI);
   AddCommand("tremble",do_action,464,POSITION_RESTING,TUTTI);
   AddCommand("twitch",do_action,465,POSITION_RESTING,TUTTI);
   AddCommand("whimper",do_action,466,POSITION_RESTING,TUTTI);
   AddCommand("whap",do_action,467,POSITION_RESTING,TUTTI);
   AddCommand("wedge",do_action,468,POSITION_RESTING,TUTTI);
   AddCommand("apologize",do_action,469,POSITION_RESTING,TUTTI);
   
   /* Liberi 470-479 (-488)*/
   
   /* DIMD stuff 
    AddCommand("dmanage",do_dmanage,480,POSITION_RESTING,MAESTRO_DEL_CREATO);
    AddCommand("drestrict",do_drestrict,481,POSITION_RESTING,DIO_MINORE);
    AddCommand("dlink",do_dgossip,482,POSITION_RESTING,DIO_MINORE);
    AddCommand("dunlink",do_dgossip,483,POSITION_RESTING,DIO_MINORE);
    AddCommand("dlist",do_dlist,484,POSITION_RESTING,2);
    AddCommand("dwho",do_dwho,485,POSITION_RESTING,2);
    AddCommand("dgossip",do_dgossip,486,POSITION_RESTING,MAESTRO_DEL_CREATO);
    AddCommand("dtell",do_dtell,487,POSITION_RESTING,MAESTRO_DEL_CREATO);
    AddCommand("dthink",do_dthink,488,POSITION_RESTING,MAESTRO_DEL_CREATO);
    */

   AddCommand("sending",do_sending,489,POSITION_STANDING,TUTTI);
   AddCommand("messenger",do_sending,490,POSITION_STANDING,TUTTI);
   AddCommand("prompt", do_set_prompt,491,POSITION_RESTING,TUTTI);

   AddCommand("ooedit",do_ooedit,492,POSITION_DEAD,MAESTRO_DEGLI_DEI);
   AddCommand("whois",do_whois,493,POSITION_DEAD,ALLIEVO-1);
   AddCommand("osave",do_osave,494,POSITION_DEAD,QUESTMASTER);
   AddCommand("dig", do_open_exit, CMD_DIG, POSITION_STANDING, TUTTI );
   AddCommand("cut", do_open_exit, CMD_SCYTHE, POSITION_STANDING, TUTTI );
   AddCommand("status", do_status, CMD_STATUS, POSITION_DEAD, TUTTI );
   AddCommand("showsk", do_showskills, CMD_SHOWSKILLS, POSITION_DEAD, 
     DIO );
   AddCommand( "resetsk", do_resetskills, CMD_RESETSKILLS, POSITION_DEAD, 
     MAESTRO_DEL_CREATO );
   AddCommand( "setsk", do_setskill, CMD_SETSKILLS, POSITION_DEAD, 
     MAESTRO_DEI_CREATORI );
//AddCommand("perdono", do_perdono, CMD_PERDONO, POSITION_STANDING, TUTTI ); //FLYP 2003 Perdono
   AddCommand("immolation", do_immolation, CMD_IMMOLATION, POSITION_FIGHTING, TUTTI); // Flyp 20180129: demon can sacrifice life for mana
 }


/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */




/* locate entry in p_table with entry->name == name. -1 mrks failed search */
 int find_name(char *name)
 {
   FILE *fl;
   char szFileName[ 41 ];
   
   sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( name ) );
   if( ( fl = fopen( szFileName, "r" ) ) != NULL )
   {
    fclose( fl );
    return TRUE;
  }
  else
   return FALSE;
}


int _parse_name(char *arg, char *name)
{
 int i;
 
   /* skip whitespaces */
 for (; isspace(*arg); arg++);
   for (i = 0; ( *name = *arg ) != 0; arg++, i++, name++) 
   {
    if ((*arg <0) || !isalpha(*arg) || i > 15 )
      return(1); 
  }      
  
  if (!i)
   return(1);
 
 return(0);
}

#define ASSHOLE_FNAME "asshole.list"

int _check_ass_name(char *name)
{
   /* 0 - full match
    *   1 - from start of string
    *   2 - from end of string
    *   3 - somewhere in string
    */
 static struct shitlist 
 {
  int how;
  char name[80];
} *shitlist = NULL;
FILE *f;
char buf[512];
int i,j,k;
if (strlen(name)<4) return(1);

if(!shitlist) 
{
  if((f=fopen(ASSHOLE_FNAME,"rt"))==NULL) 
  {
    mudlog( LOG_ERROR, "can't open asshole names list");
    shitlist=(struct shitlist *)calloc(1,sizeof(struct shitlist));
    *shitlist[0].name=0;
    return 0;
  }
  for(i=0;fgets(buf,180,f)!=NULL;i++) ;
    shitlist=(struct shitlist *)calloc((i+3), sizeof(struct shitlist));
  rewind(f);
  for(i=0;fgets(buf,180,f)!=NULL;i++) 
  {
    if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r')
      buf[strlen(buf)-1]=0;
    if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r')
      buf[strlen(buf)-1]=0;
    if(*buf=='*') 
    {
      if(buf[strlen(buf)-1]=='*') 
      {
       shitlist[i].how=3;
       buf[strlen(buf)-1]=0;
       strcpy(shitlist[i].name,buf+1);
     } 
     else 
     {
       shitlist[i].how=2;
       strcpy(shitlist[i].name,buf+1);
     }
   } 
   else 
   {
    if(buf[strlen(buf)-1]=='*') 
    {
     shitlist[i].how=1;
     buf[strlen(buf)-1]=0;
     strcpy(shitlist[i].name,buf);
   }
   else
   {
     shitlist[i].how=0;
     strcpy(shitlist[i].name,buf);
   }
 }
}
*shitlist[i].name = 0;
for(i=0;*shitlist[i].name;i++) 
{
  sprintf( buf, "mode: %d, name: %s", shitlist[i].how, shitlist[i].name );
/*      log(buf);*/
}
}
#define NAME shitlist[j].name
for(j=0;*NAME;j++)
 switch(shitlist[j].how) 
{
  case 0:
  if(!str_cmp(name,NAME))
    return 1;
  break;
  case 1:
  if(!strn_cmp(name,NAME,strlen(NAME)))
    return 1;
  break;
  case 2:
  if(strlen(name)<strlen(NAME)) 
    break;
  if(!str_cmp(name+(strlen(name)-strlen(NAME)), NAME))
    return 1;
  break;
  case 3:
  if(strlen(name)<strlen(NAME)) 
    break;
  for(k=0;k<=(int)strlen(name)-(int)strlen(NAME);k++)
    if(!strn_cmp(name+k, NAME, strlen(NAME)))
      return 1;
    break;
    default:
    mudlog( LOG_SYSERR, 
     "Invalid value in shitlist, interpereter.c _parse_name" );
    return 1;
  }
#undef NAME
  return(0);
}

void ShowStatInstruction( struct descriptor_data *d )
{
 char buf[ 100 ];
 
 sprintf( buf, "Seleziona le priorita` per le caratteristiche di %s, elencandole\n\r",
   GET_NAME( d->character ) );
 SEND_TO_Q( buf, d );
 SEND_TO_Q( "dalla piu` alta a quella piu` bassa, separate da spazi senza duplicarle.\n\r", d );    
 SEND_TO_Q( "Per esempio: 'F I S A CO CA' dara` il punteggio piu` alto alla Forza,\n\r"
   "seguite, nell'ordine, da Intelligenza, Saggezza, Agilita`, COstituzione e, per\n\r"
   "ultimo, CArisma\n\r\n\r", d);
 SEND_TO_Q("   Considera che le abilita' influenzano (anche) questo: \n\r",d);
 SEND_TO_Q("F = capacita' di usare oggetti pesanti, bonus nel combattere\n\r",d);
 SEND_TO_Q("I = velocita' nell'apprendere e capacita' di usare spells\n\r",d);
 SEND_TO_Q("S = velocita' di recupero mana e migliori tiri salvezza\n\r",d);
 SEND_TO_Q("A = capacita' di schivare attacchi e (guerrieri) di caricare(bash)\n\r",d);
 SEND_TO_Q("CO= quantita' di punti ferita e velocita' nel recuperarli\n\r",d);
 SEND_TO_Q("CA= quantita' di seguaci, reazioni dei mostri e dei mercanti\n\r",d);
#if defined(NEW_ROLL)
 SEND_TO_Q("Se non sai che pesci pigliare, puoi semplicemente premere [INVIO]\n\r",d);
 SEND_TO_Q("   Provvedera' il sistema ad assegnarti delle caratteristiche\n\r",d);
 SEND_TO_Q("   compatibili con la classe che sceglierai\n\r",d);
 SEND_TO_Q("Se invece ti senti un esperto, digita <nuovo>\n\r",d);
 SEND_TO_Q("   Potrai indicare dettagliatamente le tue caratteristiche\n\r",d);
#endif
 
 SEND_TO_Q( "\n\rLa tua scelta ? (premi <b> per tornare indietro):\n\r",d);
}
void ShowRollInstruction( struct descriptor_data *d )
{
 char buf[ 200 ];
 
 sprintf( buf, "Hai scelto la creazione del personaggio per esperti.\n\r");
 SEND_TO_Q( buf, d );
 sprintf(buf, "Ad ogni caratteristica viene assegnato il valore minimo  %d .\n\r", 
   STAT_MIN_VAL);
 SEND_TO_Q( buf, d );
 sprintf(buf, "Hai a disposizione ulteriori  %d  punti da distribuire a piacere.\n\r",
   STAT_MAX_SUM);
 SEND_TO_Q(buf,d);   
 SEND_TO_Q("Per assegnarli, inserisci i numeri in questo ordine:\n\r",d);
 SEND_TO_Q("FOrza INtelligenza SAggezza AGilita' COstituzione CArisma.\n\r",d);
 SEND_TO_Q("Puoi anche chiedere che i valori da te immessi siano \n\r",d);
 SEND_TO_Q("randomizzati aggiungendo un valore da -1 a +1.\n\r",d);
 SEND_TO_Q("Per chiedere la randomizzazione basta digitare qualsiasi cosa dopo i numeri.\n\r",d);
 SEND_TO_Q("Esempio, per avere un pg con:\n\r",d);
 sprintf(buf,"F=%d I=%d S=%d A=%d Co=%d Ca=%d senza randomizzazione:\n\r",
   STAT_MIN_VAL+18-STAT_MIN_VAL,
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
 sprintf(buf,"%sFO IN SA AG CO CA RN\n\r",buf);
 sprintf(buf,"%s%2d %2d %2d %2d %2d %2d\n\r\n\r",buf,
   18-STAT_MIN_VAL,
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
 SEND_TO_Q(buf,d);
 sprintf(buf,"F=%d I=%d S=%d A=%d Co=%d Ca=%d con randomizzazione:\n\r",
   STAT_MIN_VAL+18-STAT_MIN_VAL,
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
   STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
 sprintf(buf,"%sFO IN SA AG CO CA RN\n\r",buf);
 sprintf(buf,"%s%2d %2d %2d %2d %2d %2d %2s\n\r\n\r",buf,
   18-STAT_MIN_VAL,
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
   MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)),
   "r");
 SEND_TO_Q(buf,d);
 
 
 SEND_TO_Q("Bene, premi invio per rollare. Auguri!.\n\r",d);
#if !defined (NEW_ROLL)
 SEND_TO_Q("Sorry, not yet implemented \r\n",d);
#endif
 SEND_TO_Q( "\r\n[Batti INVIO] ", d );
}

void RollPrompt( struct descriptor_data *d )
{
 char buf[254];
 SEND_TO_Q("FO IN SA AG CO CA RN (puoi usare <b> per rinunciare)\n\r",d);
   /* STAT LIMITS */
 sprintf(buf,"%2d %2d %2d %2d %2d %2d    (valori limite per la tua razza)\n\r",
   MaxStrForRace(d->character)-STAT_MIN_VAL,
   MaxIntForRace(d->character)-STAT_MIN_VAL,
   MaxWisForRace(d->character)-STAT_MIN_VAL,
   MaxDexForRace(d->character)-STAT_MIN_VAL,
   MaxConForRace(d->character)-STAT_MIN_VAL,
   MaxChrForRace(d->character)-STAT_MIN_VAL);
 SEND_TO_Q(buf,d);
}

void AskRollConfirm( struct descriptor_data *d )
{
 SEND_TO_Q("Sei soddisfatto(S/N)? (puoi usare <b> per rinunciare)\n\r",d);
}
#if defined(NEW_ROLL)

void InterpretaRoll( struct descriptor_data *d, char *riga )
#define BACKWARD 1
#define AGAIN    2
#define GOON     0
{
 char buf[ 254 ];
 short doafter=GOON;
 int FO,IN,SA,AG,CO,CA,t;
 char c7[2]="\0";
 if (strlen(riga)==1 && (*riga == 'B' || *riga == 'b'))    
   doafter=BACKWARD;
 if (strlen(riga)>18-17)
   sscanf(riga,"%2d %2d %2d %2d %2d %2d %s",&FO,&IN,&SA,&AG,&CO,&CA,c7);
 else
   FO=IN=SA=AG=CO=CA=0;
 if (FO < 0) FO=0;
 if (IN < 0) IN=0;
 if (SA < 0) SA=0;
 if (AG < 0) AG=0;
 if (CO < 0) CO=0;
 if (CA < 0) CA=0;
 t=FO+IN+SA+AG+CO+CA;
 if (t < 0) t=STAT_MAX_SUM+1;
 if (t>STAT_MAX_SUM)
 {
  sprintf(buf,"Hai usato piu' dei %2d punti disponibili (%2d)\n\r",
    STAT_MAX_SUM,t);
  SEND_TO_Q(buf,d);
  doafter=AGAIN;
}
else
{
  FO=MIN(MaxStrForRace(d->character)-STAT_MIN_VAL,FO);
  IN=MIN(MaxIntForRace(d->character)-STAT_MIN_VAL,IN);
  SA=MIN(MaxWisForRace(d->character)-STAT_MIN_VAL,SA);
  AG=MIN(MaxDexForRace(d->character)-STAT_MIN_VAL,AG);
  CO=MIN(MaxConForRace(d->character)-STAT_MIN_VAL,CO);
  CA=MIN(MaxChrForRace(d->character)-STAT_MIN_VAL,CA);
  sprintf(buf,"Ecco le stats risultanti dalla tua scelta:\n\r");
  sprintf(buf,"%s%2d %2d %2d %2d %2d %2d %s\n\r",buf,FO+STAT_MIN_VAL,
    IN+STAT_MIN_VAL,
    SA+STAT_MIN_VAL,
    AG+STAT_MIN_VAL,
    CO+STAT_MIN_VAL,
    CA+STAT_MIN_VAL,(!*c7?"\0":"piu' la randomizzazione (-1/+1)"));
  if (t<STAT_MAX_SUM)
    sprintf(buf,"%sATTENZIONE. Hai usato solo %2d dei %2d disponibili\n\r",
      buf,t,STAT_MAX_SUM);
  SEND_TO_Q(buf,d);
}
d->stat[0]=(char)FO;
d->stat[1]=(char)IN;
d->stat[2]=(char)SA;
d->stat[3]=(char)AG;
d->stat[4]=(char)CO;
d->stat[5]=(char)CA;
d->TipoRoll=(*c7?'N':'R');
mudlog(LOG_PLAYERS,"%s ha rollato: S%2d I%2d W%2d D%2d Co%2d Ch%2d %s",
  GET_NAME(d->character),FO,IN,SA,AG,CO,CA,c7);
switch(doafter)
{
  case BACKWARD:
  ShowStatInstruction(d);
  STATE(d)=CON_STAT_LIST;
  break;
  case AGAIN:
  RollPrompt(d);
  break;
  case GOON:
  AskRollConfirm(d);
  STATE(d)=CON_CONF_ROLL;
}
return;
}
#endif

/* deal with newcomers and other non-playing sockets */

void nanny(struct descriptor_data *d, char *arg)
{
 char buf[ 254 ];
 int count=0, oops=FALSE, index=0;
 char tmp_name[20];
 struct char_file_u tmp_store;
 struct char_data *tmp_ch;
   struct room_data *rp; // Gaia 2001
   struct descriptor_data *k;
   extern struct descriptor_data *descriptor_list;
   extern int WizLock;
   extern int plr_tick_count;
   extern struct RegInfoData RI;
   
   void do_look(struct char_data *ch, char *argument, int cmd);
   void load_char_objs(struct char_data *ch);
   int load_char(char *name, struct char_file_u *char_element);
   void show_class_selection(struct descriptor_data *d, int r);
   /*struct RegInfoData *ri;*/
   
   write(d->descriptor, echo_on, 6);
   /*GGPATCH Inserita possibilita' di tornare indietro con "B"  */
   switch (STATE(d))        
   {
    case CON_NME:                /* wait for input of name        */
    for (; isspace(*arg); arg++)  ;
      if (!*arg) 
      {
        PushStatus("CON_NME");
        close_socket(d);
        PopStatus();
        return;
      }
      else 
      {
#if defined (NEW_CONNECT)   
        d->AlreadyInGame=FALSE;
#endif   
        if(_parse_name(arg, tmp_name))         
        {
          SEND_TO_Q("Nome non ammesso. Scegline un altro, per favore.\r\n", d);
          SEND_TO_Q("Nome: ", d);
          return;
        }
        if( SiteLock( d->host ) )  
        {
          SEND_TO_Q("Sorry, this site is temporarily banned.\n\r",d);
          STATE(d)=CON_WIZLOCK;
          PushStatus("Banned site");
          close_socket(d);
          PopStatus();
          return;
        }
        if (!d->character) 
        {
          CREATE(d->character, struct char_data, 1);
          clear_char(d->character);
          d->character->desc = d;
          SET_BIT( d->character->player.user_flags, USE_PAGING );
        }
/* Carico il personaggio adesso perche' mi serve il codice di autorizzazione */    
        if( load_char( tmp_name, &tmp_store ) )
        {
          store_to_char(&tmp_store, d->character);               
          strcpy(d->pwd, tmp_store.pwd);
        } 
        else 
        {
    /* player unknown gotta make a new */
          if(_check_ass_name(tmp_name)) 
          {
           SEND_TO_Q("Nome non valido. Scegline un'altro, per favore.\n\r", d);
           SEND_TO_Q("Nome: ", d);
           return;
         }
         if( !WizLock ) 
         {
           CREATE( GET_NAME( d->character ), char, strlen( tmp_name ) + 1 );
           CAP( tmp_name );
           strcpy( GET_NAME( d->character ), tmp_name );
           sprintf( buf, "E` realmente '%s' il nome che vuoi ? (si/no): ", 
             tmp_name );
           SEND_TO_Q(buf, d);
           STATE(d) = CON_NMECNF;
           return;
         } 
         else 
         {
           sprintf( buf, 
             "Mi dispiace. Non sono ammessi nuovi personaggi, "
             "per il momento.\n\r" );
           SEND_TO_Q(buf,d);
           STATE(d) = CON_WIZLOCK;
           return;
         }
       }
/* Check if already playing with some or other name*/


       for( k=descriptor_list; k; k = k->next ) 
       {
        if ((k->character != d->character) && k->character) 
        {
         sprintf(buf,"%s",GET_NAME( k->original?k->original:k->character)  );
         if ( !str_cmp( buf,GET_NAME(d->character ) ) )
         {
#if !defined ( NEW_CONNECT )
          sprintf( buf, "%s e` gia` nel gioco, non puoi riconnetterti."
            "\n\r",buf );
          SEND_TO_Q( buf, d);
          SEND_TO_Q("Nome: ", d);
#else
          d->AlreadyInGame=TRUE;
          d->ToBeKilled=k;
          mudlog(LOG_CONNECT,"%s : gia' in gioco.",buf);
          
#endif
        }
      }
      
    }
/* Tutto ok, chiediamogli la password */   
    SEND_TO_Q("Password: ", d);
    write(d->descriptor, echo_off, 4); 
    STATE(d) = CON_PWDNRM;
    return;   
  }
  
    case CON_NMECNF:        /* wait for conf. of new name        */
      /* skip whitespaces */
  for( ; isspace(*arg); arg++ );
    
    if( *arg == 's' || *arg == 'S' )
    {
      write( d->descriptor, echo_on, 4); 
      SEND_TO_Q("Nuovo personaggio.\n\r", d);

      sprintf( buf, "Inserisci una password per %s: ", 
        GET_NAME( d->character ) );

      SEND_TO_Q(buf, d);
      write( d->descriptor, echo_off, 4 );
      STATE(d) = CON_PWDGET;
    } 
    else if( *arg == 'n' || *arg == 'N') 
    {
      SEND_TO_Q("Va bene. Allora, quale sarebbe il nome ? ", d);
      free(GET_NAME(d->character));
      GET_NAME(d->character) = NULL;
      STATE(d) = CON_NME;
    } 
    else 
      { /* Please do Y or N */
      SEND_TO_Q("Per favore, si o no ? ", d);
  }
  break;
  
    case CON_PWDNRM:        /* get pwd for known player        */
      /* skip whitespaces */
  for( ; isspace(*arg); arg++);
#if KGB
    mudlog( LOG_PLAYERS | LOG_SILENT, 
     "%s ha inserito la password '%s'", GET_NAME( d->character ), arg );
#endif
  if (!*arg)
  {
    PushStatus("Password check");
    close_socket(d);
    PopStatus();
  }
  
  else  
  {
    if (!IsTest()) 
    {
      if( strncmp( (char *)crypt( arg, d->pwd ), d->pwd, strlen(d->pwd) ) )
      {
       write_to_descriptor(d->descriptor,"Password errata.\n\r");
       mudlog( LOG_PLAYERS, 
        "%s [HOST:%s] ha inserito una password errata'", 
        GET_NAME( d->character ),
        d->host,
        strlen(d->host)
        );
       PushStatus("Password errata");
       close_socket( d );
       PopStatus();
       return;
     }
   }

#if IMPL_SECURITY
   if( top_of_p_table > 0 )
   {
    if( GetMaxLevel( d->character ) >= 59 ) 
    {
     switch(SecCheck(GET_NAME(d->character), d->host)) 
     {
      case -1:
      SEND_TO_Q2("Security file not found\n\r", d);
      break;
      case 0: 
      SEND_TO_Q2("Security check reveals invalid site\n\r", d);
      SEND_TO_Q2("Speak to an implementor to fix problem\n\r", d);
      SEND_TO_Q2("If you are an implementor, add yourself to the\n\r",d);
      SEND_TO_Q2("Security directory (lib/security)\n\r",d);
      PushStatus("Security check");
      close_socket(d);
      PopStatus();
      return;
    }
  } 
  else 
  {
  }
}
#endif
/* Ok, il ragazzo ha azzeccato la password */
d->wait=0;
#if defined ( NEW_CONNECT )
/* Se era gia` in gioco assumo ld non riconosciuto e disconnetto il
  * vecchio char*/

if (d->AlreadyInGame) 
 
{
  mudlog( LOG_PLAYERS, "%s[HOST:%s] riconnesso su se stesso.", GET_NAME(d->character), 
   d->host );
  PushStatus("Riconnessione");
  close_socket(d->ToBeKilled);
  PopStatus();
}
#endif       
#ifdef ACCESSI
#ifndef NOREGISTER 
mudlog(LOG_PLAYERS,"%s: verifica codice",GET_NAME(d->character));
if (GET_AUTHCODE(d->character)) free(GET_AUTHCODE(d->character));
if (GET_AUTHBY(d->character)) free(GET_AUTHBY(d->character));
MARKS("Reginfoclean");
RegInfoClean();
MARKS("strcpy");
strcpy(RI.info[REG_PC],lower(GET_NAME(d->character)));
MARKS("gdbget");
if (gdbGet())
{
 GET_AUTHCODE(d->character)=strdup(RI.info[REG_CODICE]);
 GET_AUTHBY(d->character)=strdup(RI.info[REG_DIO]);
}
#endif
#endif 
for( tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next )
{
  if( ( !str_cmp( GET_NAME( d->character ), GET_NAME( tmp_ch ) ) &&
    !tmp_ch->desc && !IS_NPC( tmp_ch ) ) || 
   ( IS_NPC( tmp_ch ) && tmp_ch->orig && 
    !str_cmp( GET_NAME( d->character ), 
      GET_NAME( tmp_ch->orig ) ) ) ) 
  {
       /* Se riconnessione, abbandono il nuovo Char creato 
* e aggancio al descrittore corrente il char ld */
   
   write(d->descriptor, echo_on, 6);
   SEND_TO_Q("Riconnessione...\n\r", d);
   
   free_char(d->character);
   tmp_ch->desc = d;
   d->character = tmp_ch;
   tmp_ch->specials.timer = 0;
   if (!IS_IMMORTAL(tmp_ch)) 
     tmp_ch->invis_level = 0;
   if (tmp_ch->orig) 
   {
    tmp_ch->desc->original = tmp_ch->orig;
    tmp_ch->orig = 0;
  }
  d->character->persist = 0;
  STATE(d) = CON_PLYNG;
  
  act("$n si e` riconnesso.", TRUE, tmp_ch, 0, 0, TO_ROOM);
  mudlog( LOG_CONNECT, "%s[HOST:%s] has reconnected.", 
   GET_NAME(d->character), d->host);
  
       /* inserisco qui la ripartenza dei regen interrotti per i link dead */
  alter_hit( tmp_ch, 0 ) ;
  alter_mana( tmp_ch, 0 ) ;
  alter_move( tmp_ch, 0 ) ;

  return;
}
#ifdef ACCESSI
if((!d->AlreadyInGame) && 
 (GET_AUTHCODE(tmp_ch)) &&
 (GET_AUTHCODE(d->character)))
{
 if ( !str_cmp( GET_AUTHCODE(tmp_ch),
   GET_AUTHCODE(d->character))&& !IS_DIO_MINORE(tmp_ch) && !IS_DIO_MINORE(d->character))
 {
  sprintf(buf,"Hai gia' un personaggio nel gioco.\n\r");
  SEND_TO_Q( buf, d);
  SEND_TO_Q("Nome: ", d);
  mudlog(LOG_PLAYERS,
    "WARNING %s respinto per violazione MP.",
    GET_NAME(d->character));
  STATE(d)=CON_NME;
  return;
}
}
#endif 

}
/* Ok, non si tratta di riconnessione... 
  * gli immortali entrano invisibili */
if (IS_IMMORTAL(d->character))
  d->character->invis_level=ADEPT;
if (IS_DIO(d->character))
  d->character->invis_level = GetMaxLevel(d->character);
HowManyConnection(1);
/* Le ombre vengono loggate ma non viene dato l'avviso on line*/
if (GetMaxLevel(d->character) >= MAESTRO_DEL_CREATO)
  mudlog( LOG_CONNECT | LOG_SILENT, "%s [HOST:%s] has connected.", 
    GET_NAME(d->character), 
    d->host );
else
  mudlog( LOG_CONNECT , "%s [HOST:%s] has connected.", 
    GET_NAME(d->character), 
    d->host );
SEND_TO_Q( ParseAnsiColors( IS_SET( d->character->player.user_flags, 
  USE_ANSI ),
motd ), d);
SEND_TO_Q("\n\r[Batti INVIO] ", d);
STATE(d) = CON_RMOTD;
}
break;

    case CON_PWDGET:        /* get pwd for new player        */
      /* skip whitespaces */
for (; isspace(*arg); arg++);
  if (!*arg || strlen(arg) > 10 || 
    !strcasecmp(arg,d->character->player.name))        
  {
    write(d->descriptor, echo_on, 6);
    SEND_TO_Q("Password non valida.(MAx 10 caratteri - diversa dal nome)\n\r", d);
    SEND_TO_Q("Password: ", d);

    write(d->descriptor, echo_off, 4);
    return;
  }
  strncpy(d->pwd,(char *)crypt(arg, d->character->player.name), 10);
  *(d->pwd + 10) = '\0';
  write(d->descriptor, echo_on, 6);
  SEND_TO_Q("Per favore, reinserisci la password: ", d);
  write(d->descriptor, echo_off, 4);
  STATE(d) = CON_PWDCNF;
  break;
  
    case CON_PWDCNF:        /* get confirmation of new pwd        */
      /* skip whitespaces */
  for (; isspace(*arg); arg++);
    
    if (strncmp((char *)crypt(arg, d->pwd), d->pwd, strlen(d->pwd))) 
    {
      write(d->descriptor, echo_on, 6);

      SEND_TO_Q("Le password non coincidono.\n\r", d);
      SEND_TO_Q("Reinserisci la password: ", d);
      STATE(d) = CON_PWDGET;
      write(d->descriptor, echo_off, 4);
      return;
    } 
    else 
    {
      write(d->descriptor, echo_on, 6);
      show_race_choice(d);
      STATE(d) = CON_QRACE;
    }
    break;
    
    case CON_QRACE:
    for (; isspace(*arg); arg++)  ;
      if (!*arg) 
      {
        show_race_choice(d);
        STATE(d) = CON_QRACE;
      } 
      else 
      {
        if (*arg == '?') 
        {
          page_string( d, RACEHELP, 1 );
          STATE( d ) = CON_ENDHELPRACE;
        } 
        else
        {
          int i=0,tmpi=0;
          
          while (race_choice[i]!=-1) 
            i++;      
          tmpi=atoi(arg);
          if (tmpi>=0 && tmpi <=i-1)
          {
       /* set the chars race to this */
           GET_RACE(d->character) = race_choice[tmpi];
           sprintf( buf, "Quale'e` il sesso di %s ? (maschio/femmina) (b per tornare indietro): ", 
             GET_NAME( d->character ) );
           SEND_TO_Q( buf, d);
           STATE(d) = CON_QSEX;
         } 
         else 
         {
           SEND_TO_Q("\n\rScelta non valida.\n\r\n\r", d);
           show_race_choice(d);
           STATE(d) = CON_QRACE;
       /* bogus race selection! */
         }
       }
     }
     break;
     
     case CON_HELPRACE:
     SEND_TO_Q( "\r\n[Batti INVIO] ", d );
     STATE( d ) = CON_ENDHELPRACE;
     break;
     
     case CON_HELPROLL:
#if defined(NEW_ROLL)
     mudlog(LOG_CHECK,"%s sta rollando con metodo new",GET_NAME(d->character));
     RollPrompt(d);
     STATE( d ) = CON_QROLL;
#else
     ShowStatInstruction(d);
     STATE( d ) = CON_STAT_LIST;
#endif      
     break;
     case CON_QROLL:
#if defined(NEW_ROLL)
     mudlog(LOG_CHECK,"%s ha rollato con metodo new",GET_NAME(d->character));
     InterpretaRoll(d,arg);
      /* Lo stato viene impostatto da InterpretaRoll */
#endif
     break;
     return;      
    case CON_CONF_ROLL : /*Conferma la rollata*/
     mudlog(LOG_CHECK,"%s sta confermando la rollata",GET_NAME(d->character));
     switch(*arg)
     {
      for (; isspace(*arg); arg++) ;
        mudlog(LOG_CHECK,"%s ha digitato: %s",GET_NAME(d->character),arg);
      case 's':
      case 'S':
      show_class_selection(d,GET_RACE(d->character));
      STATE( d ) = CON_QCLASS;
      return;
      case 'n':
      case 'N':
      ShowRollInstruction(d);
      STATE(d) = CON_HELPROLL;
      return;
      case 'b':
      case 'B':
      ShowStatInstruction(d);
      STATE(d)=CON_STAT_LIST;
      return;
    }
    case CON_ENDHELPRACE:
    show_race_choice(d);
    STATE(d) = CON_QRACE;
    break;
    case CON_QSEX:                /* query sex of new user        */
      /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
      switch (*arg)
    {
     case 'm':
     case 'M':
/* sex MALE */
     d->character->player.sex = SEX_MALE;
     break;

     case 'f':
     case 'F':
/* sex FEMALE */
     d->character->player.sex = SEX_FEMALE;
     break;

     case 'b':
     case 'B':
/* backward */
     show_race_choice(d);
     STATE(d)=CON_QRACE;
     return;

     default:
     SEND_TO_Q("Qui su Nebbie Arcane amiamo le cose semplici.... accontentati di due sessi.\n\r", d);
     SEND_TO_Q("Quel'e` il tuo sesso ? (maschio/femmina): (b per tornare indietro)", d);
     return;
   }
   
   ShowStatInstruction( d );
   STATE(d) = CON_STAT_LIST;
   break;
   
   case CON_STAT_LIST:
      /* skip whitespaces */
      if (strlen(arg)==1 && (*arg == 'B' || *arg == 'b')) /* Backward */
   {
    sprintf( buf, "Quale'e` il sesso di %s ? (maschio/Femmina) (b per tornare indietro): ", 
      GET_NAME( d->character ) );
    SEND_TO_Q(buf,d);
    STATE( d ) = CON_QSEX;
    return;
  }
      if (!strncasecmp(arg,"nuovo",5) || !strncasecmp(arg,"new",3)) /* New roll */
  {
    ShowRollInstruction(d);
    STATE( d ) = CON_HELPROLL;
    return;
  }
  
      for (; isspace(*arg); arg++);/* Skip blanks*/
  
#if defined (NEW_ROLL)
  if (!*arg)
  {
    d->TipoRoll='S';
    show_class_selection(d,GET_RACE(d->character));
    if (IS_SET(SystemFlags,SYS_REQAPPROVE)) 
    {
    /* set the AUTH flags */
    /* (3 chances) */
      d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;  
    }
    STATE(d) = CON_QCLASS;
    break;
  }
#endif
  index = 0;
  while (*arg && index < MAX_STAT) 
  {
    if (*arg == 'F' || *arg == 'f') 
      d->stat[index++] = 's';
    if (*arg == 'I' || *arg == 'i')
      d->stat[index++] = 'i';
    if (*arg == 'S' || *arg == 's')
      d->stat[index++] = 'w';
    if (*arg == 'A' || *arg == 'a')
      d->stat[index++] = 'd';
    if (*arg == 'C' || *arg == 'c') 
    {
      arg++;
      if (*arg == 'O' || *arg == 'o') 
      {
       d->stat[index++] = 'o';
     } 
     else if (*arg == 'A' || *arg == 'a') 
     {
       d->stat[index++] = 'h';
     } 
   }
   arg++;      
 }

 if (index < MAX_STAT) 
 {
   SEND_TO_Q( "Non hai inserito tutte le statistiche richieste o qualche scelta e` sbagliata.\n\r\n\r", d);
   ShowStatInstruction( d );
   STATE(d) = CON_STAT_LIST;
   break;
 } 
 else  
 {
#if defined (NEW_ROLL)
   d->TipoRoll='V';
#endif      
   
   show_class_selection(d,GET_RACE(d->character));
   
   if (IS_SET(SystemFlags,SYS_REQAPPROVE)) 
   {
      /* set the AUTH flags */
      /* (3 chances) */
    d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;  
  }
  STATE(d) = CON_QCLASS;
  break;
}

case CON_QCLASS : 
{
 int ii=0;
   /* skip whitespaces */
 
 for (; isspace(*arg); arg++);
   
   d->character->player.iClass = 0;
 count=0;
 oops=FALSE;
 
 switch (*arg) 
 {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': 
  {
    switch(GET_RACE(d->character)) 
    {
      case RACE_ELVEN: 
      case RACE_GOLD_ELF:
      case RACE_SEA_ELF:
      case RACE_HALF_ELVEN:
      {
       ii=0;
       while (d->character->player.iClass==0 && elf_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass=elf_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD; 
       else 
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_WILD_ELF:
     {
       ii=0;
       while( d->character->player.iClass==0 &&
         wild_elf_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass = wild_elf_class_choice[ii];
        ii++;
       } /* end while */
        if( d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     } 
     
     case RACE_HUMAN:
     {
       ii=0;
       while (d->character->player.iClass==0 && human_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass=human_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else 
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_HALFLING:
     {
       ii=0;
       while (d->character->player.iClass==0 && halfling_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass=halfling_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else 
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_DEEP_GNOME:
     ii=0;
     while( d->character->player.iClass == 0 && 
      deep_gnome_class_choice[ii] != 0 )
     {
       if( atoi( arg ) == ii ) 
         d->character->player.iClass = deep_gnome_class_choice[ii];
       ii++;
    } /* end while */
       if (d->character->player.iClass != 0)
        STATE(d) = CON_RNEWD;
      else 
        show_class_selection( d, GET_RACE( d->character ) );
      break;
      
      case RACE_GNOME:
      {
       ii=0;
       while (d->character->player.iClass==0 && gnome_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass=gnome_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else 
         show_class_selection(d,GET_RACE(d->character));
       break;
     } 
     
     case RACE_DWARF:
     {
       ii=0;
       while (d->character->player.iClass==0 && dwarf_class_choice[ii] !=0) 
       {
        if( atoi( arg ) == ii ) 
          d->character->player.iClass=dwarf_class_choice[ii];
        ii++;
       } /* end while */
        if( d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_HALF_OGRE:
     {
       ii=0;
       while (d->character->player.iClass==0 && half_ogre_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=half_ogre_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_HALF_GIANT:
     {
       ii=0;
       while (d->character->player.iClass==0 && half_giant_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=half_giant_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_HALF_ORC:
     {
       ii=0;
       while (d->character->player.iClass==0 && half_orc_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=half_orc_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_ORC:
     {
       ii=0;
       while (d->character->player.iClass==0 && orc_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=orc_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_GOBLIN:
     {
       ii=0;
       while (d->character->player.iClass==0 && goblin_class_choice[ii] !=0)
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=goblin_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_DROW:
     {
       ii=0;
       while (d->character->player.iClass==0 && dark_elf_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=dark_elf_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_DARK_DWARF:
     {
       ii=0;
       while (d->character->player.iClass==0 && dark_dwarf_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=dark_dwarf_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     case RACE_DEMON:
     {
       ii=0;
       while (d->character->player.iClass==0 && demon_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=demon_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }

     case RACE_TROLL:
     {
       ii=0;
       while (d->character->player.iClass==0 && troll_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=troll_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
     
     default:
     {
       ii=0;
       while (d->character->player.iClass==0 && default_class_choice[ii] !=0) 
       {
        if (atoi(arg) == ii) 
          d->character->player.iClass=default_class_choice[ii];
        ii++;
       } /* end while */
        if (d->character->player.iClass != 0)
         STATE(d) = CON_RNEWD;
       else
         show_class_selection(d,GET_RACE(d->character));
       break;
     }
} /* end race switch */
     break;
   }
   
   case '?' : 
   { 
    page_string( d, CLASS_HELP, 1 ); 
    STATE( d ) = CON_ENDHELPCLASS;
    return;
  }
  case 'b' : 
  case 'B' :
  { 
    ShowStatInstruction(d);
    STATE( d ) = CON_STAT_LIST;
    return;
  }
  default : 
  {
    SEND_TO_Q("\n\rSelezione non valida!\n\r\n\r",d);
    show_class_selection(d,GET_RACE(d->character));
    break; 
  }               
   } /* end arg switch */   
  
  if( STATE( d ) != CON_QCLASS && IS_SET( SystemFlags, SYS_REQAPPROVE ) ) 
  {
    STATE(d) = CON_AUTH;
    SEND_TO_Q("\r\n[Batti INVIO] ", d);
  }
  else
  {
    if( STATE( d ) != CON_QCLASS ) 
    {
      mudlog( LOG_CONNECT, "%s [HOST:%s] nuovo giocatore.", 
        GET_NAME(d->character), d->host);
/*
  ** now that classes are set, initialize
  */
      init_char(d->character);

/* crea i files relativi a char */

      save_char(d->character, AUTO_RENT, 0);

      if (HasClass(d->character,CLASS_MAGIC_USER)) 
      {
        SEND_TO_Q( RU_SORCERER, d );
        STATE( d ) = CON_CHECK_MAGE_TYPE;
        break;
      }
      else
      {
    /* show newbies a instructional note from interpreter.h */
    /*page_string(d,NEWBIE_NOTE,1);*/
        SEND_TO_Q( NEWBIE_NOTE, d );
        SEND_TO_Q( "\n\r[Batti INVIO] ", d );
        STATE( d ) = CON_RNEWD;
      }
    }
  }
  break;
}
case CON_HELPCLASS:
SEND_TO_Q( "\n\r[Batti INVIO] ", d );
STATE( d ) = CON_ENDHELPCLASS;
break;

case CON_ENDHELPCLASS:
show_class_selection(d,GET_RACE(d->character));
STATE( d ) = CON_QCLASS;
break;

case CON_RNEWD:
{
 SEND_TO_Q( ParseAnsiColors( IS_SET( d->character->player.user_flags, 
  USE_ANSI ),
 motd ), d);
 SEND_TO_Q("\r\n[Batti INVIO] ", d);
 STATE(d) = CON_RMOTD;
 break;
}
#if 0
case CON_AUTH: 
{   /* notify gods */
   if (d->character->generic >= NEWBIE_START) 
   {
      /*
       ** now that classes are set, initialize
       */
      init_char(d->character);
      /* create an entry in the file */
      d->pos = create_entry(GET_NAME(d->character));
      save_char(d->character, AUTO_RENT, 0);
      SEND_TO_Q( ParseAnsiColors( IS_SET( d->character->player.user_flags, 
USE_ANSI ),
motd ), d);
      SEND_TO_Q("\r\n[Batti INVIO] ", d);
      STATE(d) = CON_RMOTD;            
   } 
   else if (d->character->generic >= NEWBIE_REQUEST) 
   {
      mudlog( LOG_PLAYERS, "%s [HOST:%s] nuovo giocatore.", GET_NAME(d->character), 
     d->host);
      if (!strncmp(d->host,"128.197.152",11))
      d->character->generic=1;
      /* I decided to give them another chance.  -Steppenwolf  */
      /* They blew it. -DM */
      if( !strncmp(d->host,"oak.grove", 9)
|| !strncmp(d->host,"143.195.1.20",12)) 
      {
d->character->generic=1;         
      } 
      else 
      {
if (top_of_p_table > 0) 
{
    mudlog( LOG_CONNECT, "Type Authorize %s to allow into game.", 
   GET_NAME( d->character ) );
    mudlog( LOG_CONNECT, 
   "type 'Wizhelp Authorize' for other commands" );
} 
else 
{
    mudlog( LOG_CHECK, "Initial character.  Authorized Automatically" );
    d->character->generic = NEWBIE_START+5;
}
      }
      /*
       **  enough for gods.  now player is told to shut up.
       */
      d->character->generic--;   /* NEWBIE_START == 3 == 3 chances */
      sprintf( buf, "Please wait. You have %d requests remaining.\n\r", 
      d->character->generic);
      SEND_TO_Q(buf, d);
      if (d->character->generic == 0) 
      {
SEND_TO_Q("Arrivederci.", d);
STATE(d) = CON_WIZLOCK;   /* axe them */
break;
      } 
      else 
      {
SEND_TO_Q("Un momento, per favore.\n\r", d);
STATE(d) = CON_AUTH;
      }
   } 
   else 
   {   /* Axe them */
      STATE(d) = CON_WIZLOCK;
   }
   break;  
}
#endif
case CON_CHECK_MAGE_TYPE:
{
  for (;isspace(*arg);arg++);
    if (!strcmp(arg,"si")) 
    {
      d->character->player.iClass -=CLASS_MAGIC_USER;
      d->character->player.iClass +=CLASS_SORCERER;
} /* end we wanted Sorcerer class! */
      SEND_TO_Q( NEWBIE_NOTE, d );
      SEND_TO_Q( "\n\r[Batti INVIO] ", d );
      STATE( d ) = CON_RNEWD;
      break;
    }
    
    case CON_RMOTD: /* read CR after printing motd  */
    if(GetMaxLevel(d->character) > IMMORTALE) 
    {
      SEND_TO_Q( ParseAnsiColors( IS_SET( d->character->player.user_flags, 
        USE_ANSI ),
      wmotd ), d);
      SEND_TO_Q("\r\n[Batti INVIO] ", d);
      STATE(d) = CON_WMOTD;
      break; 
    }
    if(d->character->term != 0)
      ScreenOff(d->character);
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    if (WizLock || SiteLock(d->host))  
    {
      if (GetMaxLevel(d->character) < DIO) 
      {
        sprintf(buf, "Sorry, the game is locked up for repair or your site is bannedr\n\r");
        SEND_TO_Q(buf,d);
        STATE(d) = CON_WIZLOCK;
      }
    }
    break;
    
    case CON_WMOTD: /* read CR after printing motd */
    
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    if (WizLock || SiteLock(d->host)) 
    {
      if (GetMaxLevel(d->character) < DIO) 
      {
        sprintf(buf, "Sorry, the game is locked up for repair or your site is banned.\n\r");
        SEND_TO_Q(buf,d);
        STATE(d) = CON_WIZLOCK;
      }
    }
    break;
    
    case CON_WIZLOCK:
    PushStatus("WIZLOCK");
    close_socket(d);
    PopStatus();
    break;
    
    case CON_CITY_CHOICE:
      /* skip whitespaces */
    for (; isspace(*arg); arg++);
      if (d->character->in_room != NOWHERE) 
      {
        SEND_TO_Q("This choice is only valid when you have been auto-saved\n\r",d);
        STATE(d) = CON_SLCT;
      } 
      else 
      {
        switch (*arg)          
        {
          case '1':
          reset_char(d->character);
          mudlog( LOG_CONNECT, "1.Loading %s's equipment", 
           d->character->player.name);
          load_char_objs(d->character);
          SetStatus("int 1",NULL,NULL);
          save_char(d->character, AUTO_RENT, 0);
          SetStatus("int 2",NULL,NULL);
          send_to_char(WELC_MESSG, d->character);
          SetStatus("int 3",NULL,NULL);
          d->character->next = character_list;
          SetStatus("int 4",NULL,NULL);
          character_list = d->character;
          SetStatus("int 5",NULL,NULL);
          char_to_room(d->character, 3001);
          SetStatus("int 6",NULL,NULL);
          d->character->player.hometown = 3001;
          SetStatus("int 7",NULL,NULL);
          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
            plr_tick_count=0;
          SetStatus("int 8",NULL,NULL);
          
          act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          SetStatus("int 9",NULL,NULL);
          if( !GetMaxLevel( d->character ) )
            do_start( d->character );
          SetStatus("int A",NULL,NULL);
          do_look( d->character, "", 15 );
          SetStatus("int B",NULL,NULL);
          d->prompt_mode = 1;
          
          break;
          
          case '2':
          reset_char( d->character );
          mudlog( LOG_CONNECT, "2.Loading %s's equipment", 
           d->character->player.name);
          load_char_objs(d->character);
          save_char(d->character, AUTO_RENT, 0);
          send_to_char(WELC_MESSG, d->character);
          d->character->next = character_list;
          character_list = d->character;
          
          char_to_room(d->character, 1103);
          d->character->player.hometown = 1103;
          
          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
            plr_tick_count=0;
          
          act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
            do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;
          
          break;
          
          case '3':
          if (GetMaxLevel(d->character) > 5) 
          {
           reset_char(d->character);
           mudlog( LOG_CONNECT, "3.Loading %s's equipment", 
            d->character->player.name);
           load_char_objs(d->character);
           save_char(d->character, AUTO_RENT, 0);
           send_to_char(WELC_MESSG, d->character);
           d->character->next = character_list;
           character_list = d->character;
           
           char_to_room(d->character, 18221);
           d->character->player.hometown = 18221;
           
           d->character->specials.tick = plr_tick_count++;
           if (plr_tick_count == PLR_TICK_WRAP)
            plr_tick_count=0;
          
          act( "$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, 
           TO_ROOM );
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
            do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;
        } 
        else 
        {
         SEND_TO_Q("Questa scelta non e` valida.\n\r", d);
         STATE(d) = CON_SLCT;
       }
       break;
       
       case '4':
       if (GetMaxLevel(d->character) > 5) 
       {
         reset_char(d->character);
         mudlog( LOG_CONNECT, "4.Loading %s's equipment", 
          d->character->player.name);
         load_char_objs(d->character);
         save_char(d->character, AUTO_RENT, 0);
         send_to_char(WELC_MESSG, d->character);
         d->character->next = character_list;
         character_list = d->character;
         
         char_to_room(d->character, 3606);
         d->character->player.hometown = 3606;
         
         d->character->specials.tick = plr_tick_count++;
         if (plr_tick_count == PLR_TICK_WRAP)
          plr_tick_count=0;
        
        act( "$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, 
         TO_ROOM);
        STATE(d) = CON_PLYNG;
        if (!GetMaxLevel(d->character))
          do_start(d->character);
        do_look(d->character, "",15);
        d->prompt_mode = 1;
      } 
      else 
      {
       SEND_TO_Q("That was an illegal choice.\n\r", d);
       STATE(d) = CON_SLCT;
     }
     break;
     
     case '5':
     if (GetMaxLevel(d->character) > 5) 
     {
       reset_char(d->character);
       mudlog( LOG_CONNECT, "5.Loading %s's equipment", 
        d->character->player.name);
       load_char_objs(d->character);
       save_char(d->character, AUTO_RENT, 0);
       send_to_char(WELC_MESSG, d->character);
       d->character->next = character_list;
       character_list = d->character;
       
       char_to_room(d->character, 16107);
       d->character->player.hometown = 16107;
       
       d->character->specials.tick = plr_tick_count++;
       if (plr_tick_count == PLR_TICK_WRAP)
        plr_tick_count=0;
      
      act("$n has entered the game.", 
       TRUE, d->character, 0, 0, TO_ROOM);
      STATE(d) = CON_PLYNG;
      if (!GetMaxLevel(d->character))
        do_start(d->character);
      do_look(d->character, "",15);
      d->prompt_mode = 1;
    }
    else
    {
     SEND_TO_Q("That was an illegal choice.\n\r", d);
     STATE(d) = CON_SLCT;
   }
   break;
   
   default:
   SEND_TO_Q("That was an illegal choice.\n\r", d);
   STATE(d) = CON_SLCT;
   break;
 }
}
break;

case CON_DELETE_ME:
{

  for (; isspace(*arg); arg++);

    if (!strcmp(arg,"si") && strcmp("Guest",GET_NAME(d->character)) ) 
    {
      char buf[256];
      
      mudlog( LOG_PLAYERS, "%s just killed self!",
       GET_NAME( d->character ) );
      sprintf( buf, "rm %s/%s.dat", PLAYERS_DIR, 
        lower( GET_NAME( d->character ) ) );
      system( buf );
      sprintf( buf, "rm %s/%s", RENT_DIR, lower( GET_NAME( d->character ) ) );
      system( buf );
      sprintf( buf, "rm %s/%s.aux", RENT_DIR, lower(GET_NAME(d->character)));
      system( buf );
      PushStatus("killedself");
      regdelete(GET_NAME(d->character));
      close_socket(d);
      PopStatus();
    }
    else
    {
      SEND_TO_Q(MENU,d);
      STATE(d)= CON_SLCT;
    }
    break;
  }
  
    case CON_SLCT:                /* get selection from main menu        */
      /* skip whitespaces */
  for (; isspace(*arg); arg++);
    switch (*arg)
  {
   case '0':
   PushStatus("CON_SLCT");
   close_socket(d);
   PopStatus();
   break;

   case 'C':
   case 'c': 
   {   
    if (GetMaxLevel(d->character)>=CHUMP)
    {
     SEND_TO_Q("Sei sicuro di volerti cancellare ? (si/no): ",d);
     STATE(d)=CON_DELETE_ME;
     break;
   }
 }
 case '1':
 reset_char(d->character);
 mudlog( LOG_PLAYERS, "M1.Loading %s's equipment", 
  d->character->player.name );
 load_char_objs(d->character);
 mudlog( LOG_CHECK, "Sending Welcome message to %s",
  d->character->player.name );
 send_to_char(WELC_MESSG, d->character);
#ifdef NOREGISTER 
 send_to_char("$c0001 NON E' NECESSARIO REGISTRARSI\r\n", 
  d->character);
#endif
 mudlog( LOG_CHECK, "Putting %s in list",
  d->character->player.name );
 d->character->next = character_list;
 character_list = d->character;
 mudlog( LOG_CHECK, "Putting %s in game",
  d->character->player.name );
 if( d->character->in_room == NOWHERE ||
  d->character->in_room == AUTO_RENT) 
{ /* returning from autorent */
  if( GetMaxLevel( d->character ) < DIO_MINORE ) 
  {
/* Per gli IMMORTALI che rentavano ad Asgaard, gli tolgo
la start room cosi' gli viene calcolata di nuovo */  
   if( d->character->specials.start_room == 1000 && IS_IMMORTALE(d->character))
    d->character->specials.start_room = -1;

  if( d->character->specials.start_room <= 0 )
/*GGPATCH Ogni razza ha la sua HomeTown definita in constants.c*/

  {
    mudlog(LOG_PLAYERS,"%s = Razza: %d Stanza0: %d Stanza2: %d",
      GET_NAME(d->character),
      GET_RACE(d->character),
      RacialHome[GET_RACE( d->character)][0],
      RacialHome[GET_RACE( d->character)][1]);
    char_to_room(d->character,
     RacialHome[GET_RACE( d->character)][0]);
    d->character->player.hometown =
    RacialHome[GET_RACE( d->character)][0];
    mudlog(LOG_PLAYERS,"%s in room %5d by normal",
      GET_NAME(d->character),d->character->player.hometown);
  } 
  else
  {
    char_to_room(d->character, d->character->specials.start_room);
    d->character->player.hometown = 
    d->character->specials.start_room;
    mudlog(LOG_PLAYERS,"%s in room %5d by special",
      GET_NAME(d->character),d->character->player.hometown);
  }
} 
else
    { /* Gli immortali tornano sempre ad Asgaard :-) */
 char_to_room(d->character, 1000);
d->character->player.hometown = 1000;
}
}
else
{
  if( IS_DIO( d->character ) )
    d->character->in_room = 1000;

  if (real_roomp(d->character->in_room))
  {
   char_to_room( d->character, d->character->in_room);          
   d->character->player.hometown = d->character->in_room;
 }
 else
    { /* Qualcosa e' andato storto o nuovo PC stanza di default */
   char_to_room(d->character, 
    RacialHome[GET_RACE( d->character)][1]);
 d->character->player.hometown = 
 RacialHome[GET_RACE( d->character)][1];
 mudlog(LOG_PLAYERS,"%s in room %5d by default",
  GET_NAME(d->character),d->character->player.hometown);
}
}

d->character->specials.tick = plr_tick_count++;
if (plr_tick_count == PLR_TICK_WRAP)
 plr_tick_count=0;

act( "$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
STATE(d) = CON_PLYNG;
if( !GetMaxLevel(d->character))
 do_start(d->character);
if( has_mail( d->character->player.name ) )
 send_to_char( "$c0014C'e` posta per te dallo scriba.\n\r\n\r",
  d->character );
sprintf(buf,"%s%s",version(),release());
do_look(d->character, "",15);
if( !d->character->specials.lastversion || 
  strcmp(d->character->specials.lastversion,buf ) )
 send_to_char( 
  "$c0115           C'E` UNA NUOVA VERSIONE DI MYST IN LINEA.\n\r",
  d->character );
if (IsTest())
 send_to_char( 
  "                 $c0115SEI SU MYST2!!!!!!!!!!!!!!!!!!!!!\n\r",
  d->character );
d->prompt_mode = 1;
if (IS_SET(d->character->player.user_flags,RACE_WAR))
 send_to_char( 
  "$c0115            RICORDATI CHE  SEI PKILL!!.\n\r",
  d->character );
mudlog( LOG_CHECK, "%s is in game.", d->character->player.name );

rp = real_roomp(d->character->in_room);

   /* qui metto un controllo. Se un PG e' pkiller e il mud ha crashato
      allora rischia di restare bloccato, lo sparo via da qualche parte :-)
      Gaia 2001 */

if ( IS_AFFECTED2(d->character,AFF2_PKILLER) && rp->room_flags&PEACEFUL ) {
 mudlog( LOG_CHECK, "A Pkill character has entered in game in a peaceful room" );
 send_to_char("La magia che avvolge questo luogo di pace ti rigetta!\n\r",d->character);
 send_to_room("Un brezza sottile si alza improvvisamente \n\r", d->character->in_room);
 char_from_room(d->character);
      char_to_room(d->character, 3001); //mando il PG in piazza 
    }                                                                                       

    break;

    case '2':
    SEND_TO_Q( "Inserisci il testo che vuoi che venga visualizzato "
     "quando gli altri\n\r", d);
    SEND_TO_Q( "ti guardano. Concludilo con un '@'.\n\r", d);
    if (d->character->player.description)
    {
      SEND_TO_Q("Vecchia descrizione :\n\r", d);
      SEND_TO_Q(d->character->player.description, d);
      free(d->character->player.description);
      d->character->player.description = 0;
    }
    d->str = &d->character->player.description;
    d->max_str = 240;
    STATE(d) = CON_EXDSCR;
    break;

    case '3':
    SEND_TO_Q(STORY, d);
    STATE(d) = CON_WMOTD;
    break;

    case '4':
    SEND_TO_Q("Inserisci la nuova password: ", d);
    write(d->descriptor, echo_off, 4);
    STATE(d) = CON_PWDNEW;
    break;
#if 0
       case '5':
SEND_TO_Q("Where would you like to enter?\n\r", d);
SEND_TO_Q("1.    ShadowSprings\n\r", d);
SEND_TO_Q("2.    Shire\n\r",    d);
if (GetMaxLevel(d->character) > 5)
   SEND_TO_Q("3.    Mordilnia\n\r", d);
if (GetMaxLevel(d->character) > 10)
   SEND_TO_Q("4.    New  Thalos\n\r", d);
if (GetMaxLevel(d->character) > 20)
   SEND_TO_Q("5.    The Gypsy Village\n\r", d);
SEND_TO_Q("Your choice? ",d);
STATE(d) = CON_CITY_CHOICE;
break;
#endif
    default:
    SEND_TO_Q("Opzione errata.\n\r", d);
    SEND_TO_Q(MENU, d);
    break;
  }
  break;
  
  case CON_PWDNEW:
      /* skip whitespaces */
  for (; isspace(*arg); arg++);
    
    if (!*arg || strlen(arg) > 10)
    {
      write(d->descriptor, echo_on, 6);
      SEND_TO_Q("Password non valida.\n\r", d);
      SEND_TO_Q("Password: ", d);
      write(d->descriptor, echo_off, 4);
      return;
    }

    strncpy(d->pwd,(char *) crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    write(d->descriptor, echo_on, 6);
    SEND_TO_Q("Reinserisci la password: ", d);
    STATE(d) = CON_PWDNCNF;
    write(d->descriptor, echo_off, 4);
    break;
    
    case CON_PWDNCNF:
      /* skip whitespaces */
    for (; isspace(*arg); arg++);
      
      if (strncmp((char *)crypt(arg, d->pwd), d->pwd, strlen(d->pwd)))
      {
        write(d->descriptor, echo_on, 6);
        SEND_TO_Q("Password errata.\n\r", d);
        SEND_TO_Q("Reinserisci la password: ", d);
        write(d->descriptor, echo_off, 4);

        STATE(d) = CON_PWDNEW;
        return;
      }
      write(d->descriptor, echo_on, 6);
      
      SEND_TO_Q(
        "\n\rFatto. Devi entrare nel gioco per rendere attivo il cambio.\n\r",
        d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
      break;
      
      default: 
      {
        mudlog( LOG_SYSERR, "Nanny: illegal state of con'ness (%d)",STATE(d));
        abort();
      }
      break;
    }
  }

  void show_class_selection(struct descriptor_data *d, int r)
  {
   int i=0;
   char buf[254],buf2[254];
   extern char *pc_class_types[];
   
   sprintf( buf, "\n\rSeleziona la classe di %s.\n\r\n\r", 
     GET_NAME( d->character ) );
   SEND_TO_Q( buf, d );
   
   switch(r) 
   {
    case RACE_ELVEN:
    case RACE_GOLD_ELF:
    case RACE_SEA_ELF:
    case RACE_HALF_ELVEN:
    for (i=0;elf_class_choice[i]!=0;i++) 
    {
      sprintf(buf,"%d) ",i);
      sprintbit((unsigned)elf_class_choice[i],pc_class_types, buf2);
      strcat(buf,buf2);  strcat(buf,"\n\r");
      SEND_TO_Q(buf,d); 
      } /* end for */
      break;
      case RACE_WILD_ELF:
      for (i=0;wild_elf_class_choice[i]!=0;i++) 
      {
        sprintf(buf,"%d) ",i);
        sprintbit((unsigned)wild_elf_class_choice[i],pc_class_types, buf2);
        strcat(buf,buf2);  strcat(buf,"\n\r");
        SEND_TO_Q(buf,d); 
      } /* end for */
        break;
        case RACE_DEEP_GNOME:
        for( i = 0; deep_gnome_class_choice[ i ] != 0; i++ ) 
        {
          sprintf(buf,"%d) ",i);
          sprintbit((unsigned)deep_gnome_class_choice[i],pc_class_types, buf2);
          strcat(buf,buf2);  strcat(buf,"\n\r");
          SEND_TO_Q(buf,d); 
      } /* end for */
          break;
          case RACE_GNOME:
          for (i=0;gnome_class_choice[i]!=0;i++) 
          {
            sprintf(buf,"%d) ",i);
            sprintbit((unsigned)gnome_class_choice[i],pc_class_types, buf2);
            strcat(buf,buf2);  strcat(buf,"\n\r");
            SEND_TO_Q(buf,d); 
      } /* end for */
            break;
            case RACE_DWARF:
            for (i=0;dwarf_class_choice[i]!=0;i++) 
            {
              sprintf(buf,"%d) ",i);
              sprintbit((unsigned)dwarf_class_choice[i],pc_class_types, buf2);
              strcat(buf,buf2);  strcat(buf,"\n\r");
              SEND_TO_Q(buf,d); 
      } /* end for */
              break;
              case RACE_HALFLING:
              for (i=0;halfling_class_choice[i]!=0;i++) 
              {
                sprintf(buf,"%d) ",i);
                sprintbit((unsigned)halfling_class_choice[i],pc_class_types, buf2);
                strcat(buf,buf2);  strcat(buf,"\n\r");
                SEND_TO_Q(buf,d); 
      } /* end for */
                break;
                case RACE_HUMAN: 
                for (i=0;human_class_choice[i]!=0;i++) 
                {
                  sprintf(buf,"%d) ",i);
                  sprintbit((unsigned)human_class_choice[i],pc_class_types, buf2);
                  strcat(buf,buf2);  strcat(buf,"\n\r");
                  SEND_TO_Q(buf,d); 
      } /* end for */
                  break;
                  case RACE_HALF_ORC:
                  for (i=0;half_orc_class_choice[i]!=0;i++) 
                  {
                    sprintf(buf,"%d) ",i);
                    sprintbit((unsigned)half_orc_class_choice[i],pc_class_types, buf2);
                    strcat(buf,buf2);  strcat(buf,"\n\r");
                    SEND_TO_Q(buf,d); 
      } /* end for */
                    break;
                    case RACE_HALF_OGRE:
                    for (i=0;half_ogre_class_choice[i]!=0;i++) 
                    {
                      sprintf(buf,"%d) ",i);
                      sprintbit((unsigned)half_ogre_class_choice[i],pc_class_types, buf2);
                      strcat(buf,buf2);  strcat(buf,"\n\r");
                      SEND_TO_Q(buf,d); 
      } /* end for */
                      break;
                      case RACE_HALF_GIANT:
                      for (i=0;half_giant_class_choice[i]!=0;i++) 
                      {
                        sprintf(buf,"%d) ",i);
                        sprintbit((unsigned)half_giant_class_choice[i],pc_class_types, buf2);
                        strcat(buf,buf2);  strcat(buf,"\n\r");
                        SEND_TO_Q(buf,d); 
      } /* end for */
                        break;
                        case RACE_ORC:
                        for (i=0;orc_class_choice[i]!=0;i++) 
                        {
                          sprintf(buf,"%d) ",i);
                          sprintbit((unsigned)orc_class_choice[i],pc_class_types, buf2);
                          strcat(buf,buf2);  strcat(buf,"\n\r");
                          SEND_TO_Q(buf,d); 
      } /* end for */
                          break;
                          case RACE_GOBLIN:
                          for (i=0;goblin_class_choice[i]!=0;i++) 
                          {
                            sprintf(buf,"%d) ",i);
                            sprintbit((unsigned)goblin_class_choice[i],pc_class_types, buf2);
                            strcat(buf,buf2);  strcat(buf,"\n\r");
                            SEND_TO_Q(buf,d); 
      } /* end for */
                            break;
                            case RACE_DROW:
                            for (i=0;dark_elf_class_choice[i]!=0;i++) 
                            {
                              sprintf(buf,"%d) ",i);
                              sprintbit((unsigned)dark_elf_class_choice[i],pc_class_types, buf2);
                              strcat(buf,buf2);  strcat(buf,"\n\r");
                              SEND_TO_Q(buf,d); 
      } /* end for */
                              break;
                              case RACE_DARK_DWARF:
                              for (i=0;dark_dwarf_class_choice[i]!=0;i++) 
                              {
                                sprintf(buf,"%d) ",i);
                                sprintbit((unsigned)dark_dwarf_class_choice[i],pc_class_types, buf2);
                                strcat(buf,buf2);  strcat(buf,"\n\r");
                                SEND_TO_Q(buf,d); 
      } /* end for */
                                break;
                                case RACE_TROLL:
                                for (i=0;troll_class_choice[i]!=0;i++) 
                                {
                                  sprintf(buf,"%d) ",i);
                                  sprintbit((unsigned)troll_class_choice[i],pc_class_types, buf2);
                                  strcat(buf,buf2);  strcat(buf,"\n\r");
                                  SEND_TO_Q(buf,d); 
      } /* end for */
                                  break;

                                  case RACE_DEMON:
                                  for (i=0;demon_class_choice[i]!=0;i++) 
                                  {
                                    sprintf(buf,"%d) ",i);
                                    sprintbit((unsigned)demon_class_choice[i],pc_class_types, buf2);
                                    strcat(buf,buf2);  strcat(buf,"\n\r");
                                    SEND_TO_Q(buf,d); 
      } /* end for */
                                    break;

                                    default:
                                    for (i=0;default_class_choice[i]!=0;i++) 
                                    {
                                      sprintf(buf,"%d) ",i);
                                      sprintbit((unsigned)default_class_choice[i],pc_class_types, buf2);
                                      strcat(buf,buf2);  strcat(buf,"\n\r");
                                      SEND_TO_Q(buf,d); 
      } /* end for */
                                      break;
   } /* end switch */
                                      
                                      SEND_TO_Q( "\n\r\n\rClasse (batti <?> per un aiuto, <b> per tornare indietro): ", d );
                                    }

                                    void show_race_choice(struct descriptor_data *d)
                                    {
                                     int ii,i=0;
                                     char buf[255],buf2[254];
                                     
                                     SEND_TO_Q( "                               Limiti di livello\n\r",d);
                                     sprintf( buf,"%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s\n\r",
                                       "#","Razza","ma","cl","wa","th","dr","mk","ba","so","pa","ra","ps");        
                                     SEND_TO_Q(buf,d);
                                     
                                     while (race_choice[i]!=-1) 
                                     {
                                      sprintf(buf,"%-3d) %-15s",i,RaceName[race_choice[i]]);
                                      
      /* show level limits */
                                      for (ii=0;ii<MAX_CLASS;ii++) 
                                      {
                                        sprintf(buf2," %-3d",RacialMax[race_choice[i]][ii]); 
                                        strcat(buf,buf2);
      } /* end for */
                                        
                                        strcat(buf,"\n\r");
                                        SEND_TO_Q(buf,d); 
                                        i++;
                                      }
                                      
                                      SEND_TO_Q( "  ma=magic user, cl=cleric, wa=warrior, th=thief, dr=druid, "
                                       "mk=monk\n\r",d);
                                      SEND_TO_Q( "  ba=barbarian, so=sorcerer, pa=paladin, ra=ranger,"
                                       " ps=psionist\n\r\n\r",d);
                                      SEND_TO_Q( "Razza (batti <?> per un aiuto): ", d );
                                    }



#define A_LOG_NAME "affected.log"
#define A_MAX_LINES 5000
                                    void check_affected(char *msg)
                                    {
                                     struct affected_type  *hjp;
                                     struct char_data *c;
                                     static FILE *f=NULL;
                                     static long lines=0;
                                     char *(b[5]);
                                     char buf[5000];
                                     int i,j;
                                     
                                     if(!f && !(f=fopen(A_LOG_NAME,"wr"))) {
                                      perror(A_LOG_NAME);
                                      return;
                                    }
                                    
                                    if(lines>A_MAX_LINES) {
                                      rewind(f);
                                      for(i=0;i<5;i++) {
                                        b[i]=(char *)malloc(MAX_STRING_LENGTH);
                                        if(fgets(b[i], MAX_STRING_LENGTH, f)==NULL) break;
                                      }
                                      fclose(f);
                                      if(!(f=fopen(A_LOG_NAME,"wr"))) {
                                        perror(A_LOG_NAME);
                                        return;
                                      }
                                      for(j=0;j<i;j++) {
                                        fputs(b[j],f);
                                        free(b[j]);
                                      }  
                                      lines=0;
                                    }
                                    
                                    if(msg) 
                                     fprintf(f,"%s : ", msg);
                                   else
                                     fprintf(f,"check_affected: ");
                                   
                                   for(c=character_list;c;c=c->next)
                                     if(c && c->affected) 
                                       for(hjp=c->affected;hjp;hjp=hjp->next)
                                         if(hjp->type > MAX_EXIST_SPELL || hjp->type < 0) {
                                          sprintf(buf,"bogus hjp->type for (%s).", GET_NAME(c));
                                          fprintf(f,buf);
      /*          abort();    in test site this will be ok.. */
                                        }
                                        
                                        fprintf(f,"ok\n");
                                        fflush(f);
                                        lines++;
                                        
                                        return;
                                      }


