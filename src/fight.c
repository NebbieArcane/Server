/*AlarMUD */
/* $Id: fight.c,v 2.0 2002/03/24 19:46:06 Thunder Exp $ */

/* File modificato da GAIA nel 7/2000 per aggiungere la locazione 
   negli oggetti DamageResult. Inoltre implementato il comando
   PARRY e la dipendenza del danno dalla locazione colpita.
   Per trovare le zone modificate cercare la keyword Gaia */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "protos.h"
#include "fight.h"
#include "cmdid.h"
#include "snew.h"
#include "utility.h"

#define DUAL_WIELD(ch) (ch->equipment[WIELD] && ch->equipment[HOLD]&&			ITEM_TYPE(ch->equipment[WIELD])==ITEM_WEAPON && 			ITEM_TYPE(ch->equipment[HOLD])==ITEM_WEAPON)
#define GET_GRP_LEVEL(ch) (GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+ 			   (GetThirdMaxLev(ch)/3))
/* Structures */


struct char_data *combat_list = 0;   /* head of l-list of fighting chars    */
struct char_data *missile_list = 0;   /* head of l-list of fighting chars    */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */
struct char_data *missile_next_dude = 0; /* Next dude global trick           */
extern struct zone_data *zone_table;         /* table of reset data          */

extern struct con_app_type con_app[];
extern struct index_data *mob_index,*obj_index;
char PeacefulWorks = 1;  /* set in @set */
char DestroyedItems;  /* set in MakeScraps */

/* External structures */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct index_data *mob_index;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *spells[];
extern char *ItemDamType[];
extern int ItemSaveThrows[22][5];
extern struct str_app_type str_app[];
extern int WizLock;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern char *room_bits[];
extern int thaco[MAX_CLASS][ABS_MAX_LVL];

/**************************************************************************
* Prototipi per le procedure interne
* ***********************************************************************/
int can_see_linear( struct char_data *ch, struct char_data *targ, int *rng, 
		   int *dr) ;

int BarbarianToHitMagicBonus ( struct char_data *ch);
int berserkthaco ( struct char_data *ch);
int berserkdambonus ( struct char_data *ch, int dam);
long ExpCaps(struct char_data *ch, int group_count, long total );
long GroupLevelRatioExp( struct char_data *ch,int group_max_level,
			long experincepoints);
void appear(struct char_data *ch);
int LevelMod(struct char_data *ch, struct char_data *v, int exp);
int RatioExp( struct char_data *ch, struct char_data *victim, int total);
void change_alignment(struct char_data *ch, struct char_data *victim);
void group_gain(struct char_data *ch, struct char_data *victim);
int group_loss(struct char_data *ch, int loss);
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type, int location);
int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type);
int DamDetailsOk( struct char_data *ch, struct char_data *v, int dam, 
		 int type);
DamageResult DoDamage( struct char_data *ch, struct char_data *v, int dam, 
		      int type, int location );
void DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype, int location );
void increase_blood(int rm);
int DamageEpilog( struct char_data *ch, struct char_data *victim,
		 int killedbytype, int dam);
int GetWeaponType(struct char_data *ch, struct obj_data **wielded);
int Getw_type(struct obj_data *wielded);
int HitCheckDeny( struct char_data *ch, struct char_data *victim, int type, 
		 int DistanceWeapon);
DamageResult MissVictim( struct char_data *ch, struct char_data *v, int type, 
			int w_type, 
			DamageResult (*dam_func)
                         ( struct char_data *,
			 struct char_data *, int,
			 int, int ), int location);
int GetWeaponDam(struct char_data *ch, struct char_data *v, 
		 struct bj_data *wielded, int location);
int LoreBackstabBonus(struct char_data *ch, struct char_data *v);
DamageResult HitVictim( struct char_data *ch, struct char_data *v, int dam,
		       int type, int w_type,
		       DamageResult (*dam_func)( struct char_data *,
						struct char_data *,
						int, int, int ), int location);

DamageResult root_hit( struct char_data *ch, struct char_data *victim, int type, 
		      DamageResult (*dam_func)( struct char_data *, 
					       struct char_data *,
					       int, int, int ), 
		      int DistanceWeapon, int location);
DamageResult MissileHit( struct char_data *ch, struct char_data *victim, 
			int type);
void BreakLifeSaverObj( struct char_data *ch );
int BrittleCheck(struct char_data *ch, struct char_data *v, int dam);
int DamageItem(struct char_data *ch, struct obj_data *o, int num);
int DamagedByAttack( struct obj_data *i, int dam_type);
int GetItemDamageType( int type);
void WeaponSpell( struct char_data *c, struct char_data *v, 
		 struct obj_data *obj, int type );
int GetFormType(struct char_data *ch);
int MonkDodge( struct char_data *ch, struct char_data *v, int *dam);
int Hit_Location( struct char_data *v );
char *replace_string( char *str, char *weapon, char *weapon_s,
		     char *location_hit, char *location_hit_s);
void one_affect_from_char(struct char_data *ch,short skill);
/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
   {"hit",    "hits"},            /* TYPE_HIT      */
   {"pound",  "pounds"},          /* TYPE_BLUDGEON */
   {"pierce", "pierces"},         /* TYPE_PIERCE   */
   {"slash",  "slashes"},         /* TYPE_SLASH    */
   {"whip",   "whips"},           /* TYPE_WHIP     */
   {"claw",   "claws"},           /* TYPE_CLAW     */
   {"bite",   "bites"},           /* TYPE_BITE     */
   {"sting",  "stings"},          /* TYPE_STING    */
   {"crush",  "crushes"},         /* TYPE_CRUSH    */
   {"cleave", "cleaves"},
   {"stab",   "stabs"},
   {"smash",  "smashes"},
   {"smite",  "smites"},
   {"blast",  "blasts"},
   {"strike","strikes"}                /* type RANGE_WEAPON */
};

/* Location of attack texts */
/* Modificate per renderle compatibili con gli indici 
   dell equip indossato. I valori da 0-2 sono messi per
   aumentare la probabilita' di colpire il body
   Gaia 7/2000 */

struct attack_hit_type location_hit_text[] =
{
   {"in $S body","body",},                /* 0 */
   {"in $S shoulder","shoulder",},        /* 1 */     
   {"in $S chest","chest",},              /* 2 */    
   {"in $S neck",      "neck"},           /* 3 */     
   {"in $S neck",      "neck"},           /* 4 */   
   {"in $S body",     "body"},            /* 5 */   
   {"in $S head",      "head"},           /* 6 */
   {"in $S leg",  "leg"},                 /* 7 */
   {"in $S foot","foot"},                 /* 8 */ 
   {"in $S left hand", "left hand"},      /* 9 */ 
   {"in $S right arm", "right arm"},      /* 10 */     
   {"in $S left arm",  "left arm"},       /* 11 */      
   {"in $S back",      "back"},           /* 12 */     
   {"in $S stomach",   "stomach"},        /* 13 */          
   {"in $S left wrist",  "left wrist"},   /* 14 */    
   {"in $S right wrist", "right wrist"},  /* 15 */      
   {"in $S right hand", "right hand"},    /* 16 */     
   {"in $S left hand","left hand"}        /* 17 */
};

/* Questa routine viene tolta dal damage message
   per poter implementare piu' agilmente la gestione
   contemporanea di locazione/modifica del danno
   e scrap in modo consistente.

   Se il PG sta parando la locazione diventa lo scudo

   Viene fatto in questa routine il check per il PARRY
   il PARRY dipende dal numero dei mobs che ti attaccano
   dalla tua dexterity e dalla classe. Questo e' fatto in 
   modo da evitare un abuso di parry da parte dei caster.
   In pratica il parry ad un caster non entra (quasi) mai.
   Infine messa la dipendenza dal numero di classi sempre
   per evitare che un multi MU/WA vada in  facilmente in 
   parry e usi solo
   le spell per infliggere danno.

   Aggiunto che i ranger parano con la seconda arma invece che 
   con lo scudo.

   Gaia (7/2000) */

int Hit_Location(struct char_data *victim)
 { 
  int hitloc ;
  int riuscita;
  int abilita;
  int mult = 0 ;

   /* Check su PARRY e generate random hit location */

    if(IS_SET(victim->specials.affected_by2, AFF2_PARRY))

   {

      riuscita = number(1, 120) ; /* da 100 a 120 si sbaglia sempre */

      abilita = GetMaxLevel(victim)/8 + GET_DEX(victim) ;

      if(HasClass(victim, CLASS_WARRIOR) || HasClass(victim, CLASS_PALADIN)
         || HasClass(victim, CLASS_RANGER) ) 
       { mult = 5; }
      else if (HasClass(victim, CLASS_BARBARIAN ))
       { mult = 4; }
      else if (HasClass(victim, CLASS_DRUID) || HasClass(victim, CLASS_MONK))
       { mult = 3; }
      else if (HasClass(victim, CLASS_PSI) || HasClass(victim, CLASS_CLERIC))
       { mult = 2; }
      else
       { mult = 1; }

     abilita = (abilita*mult)/HowManyClasses(victim) ;
           
     abilita -= victim->attackers * 10 ;

      if(riuscita<=abilita && riuscita <= 100) 
      {
        if (DUAL_WIELD(victim) && HasClass(victim, CLASS_RANGER))
         { hitloc = 17; }
          else
         { hitloc = 11; }
       return(hitloc) ;
      }
  }

   hitloc = number(0,17);

   /* make body/chest hits happen more often than the others */

   if( hitloc !=0 && hitloc != 1 && hitloc != 2 && hitloc != 5)
   hitloc = number(0,17);

   /* make sure the mob has this body part first! */                                         if( !HasHands( victim ) )
   hitloc = 0;   /* if not then just make it a body hit hitloc=0 */

   return(hitloc) ;
}

/* The Fight related routines */
bool CheckMirror(struct char_data *victim)
{
   int n=0;
   n=how_many_spell(victim,SPELL_MIRROR_IMAGES);
   if ( n &&
       (!(victim->specials.fighting) || number(0,n)))
   {
      SpellWearOff(SPELL_MIRROR_IMAGES,victim);
      one_affect_from_char(victim,SPELL_MIRROR_IMAGES);
      return(TRUE);
   }
   else
   return(FALSE);
}

//ACIDUS 2003: skill Better Equilibrium
bool CheckEquilibrium(struct char_data *ch)
{
   int val;
   int percent;

   if ( !(GET_RACE(ch) == RACE_DWARF) ) return(FALSE);

   percent = number(1,100);

   if( ch->skills && ch->skills[SKILL_EQUILIBRIUM].learned 
       && (percent < ch->skills[SKILL_EQUILIBRIUM].learned)
     )
   {

     percent = number(1,100);
     val = (int) (GetMaxLevel(ch) * 0.333); // (livello/51)*17 - 17 è l'apporto massimo dato dal livello
     val += (int) ((GET_DEX(ch)-5) * 1.417); // (dex-5/12)*17 - 17 è l'apporto massimo dato dalla dex

     if ( percent > val )
       return(FALSE);
     else
     {
       send_to_char("Riesci a tenerti in equilibrio.\n\r", ch);
       return(TRUE);
     }
   }
   return(FALSE);
}


void appear(struct char_data *ch)
{
   act("$n appare improvvisamente!", FALSE, ch,0,0,TO_ROOM);
   
   if (affected_by_spell(ch, SPELL_INVISIBLE))
   affect_from_char(ch, SPELL_INVISIBLE);
   
   if (affected_by_spell(ch, SPELL_INVIS_TO_ANIMALS))
   affect_from_char(ch, SPELL_INVIS_TO_ANIMALS);
   
   REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}



int LevelMod( struct char_data *ch, struct char_data *v, int exp )
{
   float ratio = 0.0;
   float fexp;
   
   ratio = (float)GET_GRP_LEVEL( v ) / GET_GRP_LEVEL( ch );
   
   if (ratio < 1.0)
   {
      fexp = ratio * exp;
   }
   else
   {
      fexp = exp;
   }
   
   return (int)fexp;
}


int RatioExp( struct char_data *ch, struct char_data *victim, int total)
{
   if( !IS_SET( victim->specials.act, ACT_AGGRESSIVE ) &&
      !IS_SET( victim->specials.act, ACT_META_AGG ) &&
      !IS_AFFECTED( victim, AFF_CHARM ) )
   {
      if( GetMaxLevel( ch ) > 15 )
      total = LevelMod( ch, victim, total );
   }
   
   if( ( IS_SET( victim->specials.act, ACT_AGGRESSIVE ) ||
	IS_SET( victim->specials.act, ACT_META_AGG ) ) &&
      !IS_AFFECTED( victim, AFF_CHARM ) )
   {
      /* make sure that poly mages don't abuse, by reducing their bonus */
      if( IS_NPC( ch ) )
      {
	 total *= 3;
	 total /= 4;
      }
   }
   
   return total;
}



void load_messages()
{
   FILE *f1;
   int i,type;
   struct message_type *messages;
   char chk[100];
   
   if (!(f1 = fopen(MESS_FILE, "r"))){
      perror("read messages");
      assert(0);
   }
   
   /*
    * find the memset way of doing this...
    */
   
   for (i = 0; i < MAX_MESSAGES; i++)        { 
      fight_messages[i].a_type = 0;
      fight_messages[i].number_of_attacks=0;
      fight_messages[i].msg = 0;
   }
   
   fscanf(f1, " %s \n", chk);
   
   i = 0;
   
   while(*chk == 'M')
   {
      fscanf(f1," %d\n", &type);
      
      if(i>=MAX_MESSAGES)
      {
	 mudlog( LOG_SYSERR, "Too many combat messages." );
	 exit(0);
      }
      
      CREATE(messages,struct message_type,1);
      fight_messages[i].number_of_attacks++;
      fight_messages[i].a_type=type;
      messages->next=fight_messages[i].msg;
      fight_messages[i].msg=messages;
      
      messages->die_msg.attacker_msg      = fread_string(f1);
      messages->die_msg.victim_msg        = fread_string(f1);
      messages->die_msg.room_msg          = fread_string(f1);
      messages->miss_msg.attacker_msg     = fread_string(f1);
      messages->miss_msg.victim_msg       = fread_string(f1);
      messages->miss_msg.room_msg         = fread_string(f1);
      messages->hit_msg.attacker_msg      = fread_string(f1);
      messages->hit_msg.victim_msg        = fread_string(f1);
      messages->hit_msg.room_msg          = fread_string(f1);
      messages->god_msg.attacker_msg      = fread_string(f1);
      messages->god_msg.victim_msg        = fread_string(f1);
      messages->god_msg.room_msg          = fread_string(f1);
      fscanf(f1, " %s \n", chk);
      i++;
   }
   
   fclose(f1);
}


void update_pos( struct char_data *victim )
{
   
   if( ( GET_HIT( victim ) > 0 ) && ( GET_POS(victim) > POSITION_STUNNED ) ) 
   {
      return;
   } 
   else if (GET_HIT(victim) > 0 ) 
   {
      if( GET_POS(victim) == POSITION_STUNNED && IS_WAITING( victim ) )
      return;
      if (!IS_AFFECTED(victim, AFF_PARALYSIS)) 
      {
	 if (!MOUNTED(victim))
	 GET_POS(victim) = POSITION_STANDING;
	 else 
	 GET_POS(victim) = POSITION_MOUNTED;
      }
      else 
      {
	 GET_POS(victim) = POSITION_STUNNED;
      }
   } 
   else if (GET_HIT(victim) <= -11) 
   {
      GET_POS(victim) = POSITION_DEAD;
   } 
   else if (GET_HIT(victim) <= -6) 
   {
      GET_POS(victim) = POSITION_MORTALLYW;
   } 
   else if (GET_HIT(victim) <= -3) 
   {
      GET_POS(victim) = POSITION_INCAP;
   } 
   else 
   {
      GET_POS(victim) = POSITION_STUNNED;
   }
}


int check_peaceful(struct char_data *ch, char *msg)
{
   struct room_data *rp;
   
   extern char PeacefulWorks;
   
   if (!PeacefulWorks) return(0);
   
   if (!ch) 
   return(FALSE);
   
   rp = real_roomp(ch->in_room);
   if (rp && rp->room_flags&PEACEFUL) {
      send_to_char(msg, ch);
      return 1;
   }
   return 0;
}

/* start one char fighting another */

void set_fighting(struct char_data *ch, struct char_data *vict)
{
   
   if( ch->specials.fighting )
   {
      mudlog( LOG_SYSERR, "Fighting character set to fighting another." );
      return;
   }
   
   if( vict->attackers <= 5 ) 
   {
      vict->attackers++;
   } 
   else 
   {
      mudlog( LOG_SYSERR, "more than 6 people attacking one target" );
   }
   
   ch->next_fighting = combat_list;
   combat_list = ch;
   
   if( IS_AFFECTED( ch, AFF_SLEEP ) )
   affect_from_char( ch, SPELL_SLEEP );
   
   /* if memorizing, distirb it and remove it */
   
   if( affected_by_spell( ch, SKILL_MEMORIZE ) ) 
   affect_from_char( ch, SKILL_MEMORIZE );
   
   /* same here */
   
   if( affected_by_spell( ch, SKILL_MEDITATE ) )
   affect_from_char( ch, SKILL_MEDITATE );
   
   
   ch->specials.fighting = vict;
   GET_POS(ch) = POSITION_FIGHTING;
   
   /* player has lost link and mob is killing him, force PC to flee */
#ifdef DOFLEEFIGHTINGLD
   if( IS_LINKDEAD( ch ) && !IS_POLY( ch ))
   {
      do_flee(ch,"\0",0);
   }  
   
   if( IS_LINKDEAD( vict ) && !IS_POLY( vict ))
   {
      do_flee(vict,"\0",0);
   }  
#endif
}

void StopAllFightingWith( char_data *pChar )
{
   char_data *pFighting, *pNextFighting;
   for( pFighting = combat_list; pFighting ; pFighting = pNextFighting )
   {
      pNextFighting = pFighting->next_fighting;
      if( pFighting->specials.fighting == pChar )
      stop_fighting( pFighting );
   }
}


/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
   struct char_data *tmp;
   
   if( !ch->specials.fighting )
   {
      char buf[300];
      sprintf( buf, "%s not fighting at invocation of stop_fighting",
	      GET_NAME( ch ) );
      return;
   }
   
   ch->specials.fighting->attackers--;
   
   if( ch->specials.fighting->attackers < 0 )
   {
      mudlog( LOG_SYSERR, "too few people attacking");
      ch->specials.fighting->attackers = 0;
   }
   
   /* Dovrebbe togliere il flag AFF2_BERSERK solo se il numero di attaccanti
    e` uguale a zero. Purtroppo non funziona. */

   if( /*ch->attackers <= 0 && */
      IS_SET( ch->specials.affected_by2, AFF2_BERSERK ) )
   {
      REMOVE_BIT( ch->specials.affected_by2, AFF2_BERSERK );
      act( "$n sembra essersi calmato!", FALSE, ch, 0, 0, TO_ROOM );
      act( "Ti sei calmato.", FALSE,ch, 0, 0, TO_CHAR );
   }

   /* Rimuovo il flag PARRY Gaia 7/2000 */

   if(IS_SET( ch->specials.affected_by2, AFF2_PARRY ) )
   {
      REMOVE_BIT( ch->specials.affected_by2, AFF2_PARRY );
      act( "$n smette di proteggersi con lo scudo.", FALSE, ch, 0, 0, TO_ROOM );
      act( "Smetti di proteggerti con lo scudo.", FALSE,ch, 0, 0, TO_CHAR );
   }  
   
   if( ch == combat_next_dude )
   combat_next_dude = ch->next_fighting;
   
   if( combat_list == ch )
   combat_list = ch->next_fighting;
   else
   {
      for( tmp = combat_list; tmp && ( tmp->next_fighting != ch ); 
	  tmp = tmp->next_fighting );
      if (!tmp) 
      {
	 mudlog( LOG_SYSERR, 
		"Char fighting not found Error (fight.c, stop_fighting)" );
	 assert( 0 );
      }    
      tmp->next_fighting = ch->next_fighting;
   }
   
   ch->next_fighting = 0;
   ch->specials.fighting = 0;
   if( MOUNTED( ch ) )
   GET_POS(ch) = POSITION_MOUNTED;
   else 
   GET_POS(ch) = POSITION_STANDING;
   update_pos( ch );
}


#define REAL 0
#define MAX_NPC_CORPSE_TIME 8
#define MAX_PC_CORPSE_TIME 12
#define SEVERED_HEAD       60   
void make_corpse(struct char_data *ch, int killedbytype)
{
   struct obj_data *corpse, *o, *cp;
   struct obj_data *money;        
   char buf[MAX_INPUT_LENGTH], 
   spec_desc[255]; /* used in describing the corpse */
   int r_num,i, ADeadBody=FALSE;
   
   /*   char *strdup(char *source); */
   
   struct obj_data *create_money( int amount );
   
   CREATE(corpse, struct obj_data, 1);
   clear_object(corpse);
   
   corpse->item_number = NOWHERE;
   corpse->in_room = NOWHERE;
   
   if (!IS_NPC(ch) || (!IsUndead(ch))) 
   {
      /* this is so we drop a severed head at the corpse, just for visual */
      if( GET_HIT(ch) < -20 && ( killedbytype == TYPE_SLASH || 
				killedbytype == TYPE_CLEAVE ) )
      {
	 if( ( r_num = real_object( SEVERED_HEAD ) ) >= 0 ) 
	 {
	    cp = read_object(r_num, REAL);
	    sprintf(buf,"head severed %s",corpse->name);
	    cp->name=strdup(buf);
	    sprintf( buf,"the severed head of %s",
		    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	    cp->short_description=strdup(buf);
	    cp->action_description=strdup(buf);
	    sprintf(buf,"%s is lying on the ground.",buf);
	    cp->description=strdup(buf);
	    
	    cp->obj_flags.type_flag = ITEM_CONTAINER;
	    cp->obj_flags.wear_flags = ITEM_TAKE;
	    cp->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
	    cp->affected[0].modifier=GET_RACE(ch);     /* race of corpse not used */
	    cp->affected[1].modifier=GetMaxLevel(ch);  /* level of corpsenot used */
	    cp->obj_flags.value[3] = 1; /* corpse identifyer */
	    if (IS_NPC(ch)) 
	    {
	       cp->obj_flags.timer= MAX_NPC_CORPSE_TIME;  
	    }
	    
	    else
	    cp->obj_flags.timer = MAX_PC_CORPSE_TIME; /*Non per i PC */
	    
	    obj_to_room(cp,ch->in_room);
	 } /* we got the numerb of the item... */
      }
      
      /* so we can have some description on the corpse */
      /* msw                                           */
      switch(killedbytype) 
      {
       case SPELL_COLOUR_SPRAY:
	 sprintf( spec_desc," resti multicolori di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	 break;
       case SPELL_ACID_BLAST: 
	 sprintf( spec_desc," resti corrosi di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	 break;
       case SPELL_FIREBALL:
	 sprintf( spec_desc," resti carbonizzati di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	 break;
       case SPELL_CHAIN_LIGHTNING:
       case SPELL_LIGHTNING_BOLT:
	 sprintf( spec_desc," resti strinati di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	 break;
       case SKILL_PSIONIC_BLAST:
	 sprintf( spec_desc," resti gelatinosi di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	 break;
       default:
	 sprintf( spec_desc,"l corpo di %s",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));             
	 break;
      } /* end switch */
      
      sprintf(buf, "corpo %s",GET_NAME(ch));
      corpse->name = strdup(buf);
      if (IS_AFFECTED(ch,AFF_FLYING)) 
      sprintf(buf, "I%s, ancora a mezz'aria.", spec_desc);
      else
      sprintf(buf, "I%s.", spec_desc);
      
      corpse->description = strdup(buf);
      
      sprintf(buf, "il corpo di %s",  /* for the dissolve message */
	      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));             
      corpse->short_description = strdup(buf);
      
      ADeadBody = TRUE;
      
   }
   else if (IsUndead(ch)) 
   {
      corpse->name = strdup("dust pile bones");
      corpse->description = strdup("A pile of dust and bones is here.");
      corpse->short_description = strdup("a pile of dust and bones");           
   } 
   
   
   corpse->contains = ch->carrying;
   if( GET_GOLD( ch ) > 0 ) 
   {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch)=0;
      obj_to_obj(money,corpse);
   }
   
   corpse->obj_flags.type_flag = ITEM_CONTAINER;
   corpse->obj_flags.wear_flags = ITEM_TAKE;
   corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
   
   corpse->affected[0].modifier=GET_RACE(ch); /* race of corpse */
   corpse->affected[1].modifier=GetMaxLevel(ch);  /* level of corpse */
   
   corpse->obj_flags.value[3] = 1; /* corpse identifyer */
   if (ADeadBody) 
   {
      corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
   }
   else 
   {
      corpse->obj_flags.weight = 1+IS_CARRYING_W(ch);
   }
   corpse->obj_flags.cost_per_day = 100000;
   if (IS_NPC(ch))
   corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
   else
   corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
   
   for (i=0; i<MAX_WEAR; i++)
   if (ch->equipment[i])
   obj_to_obj(unequip_char(ch, i), corpse);
   
   ch->carrying = 0;
   IS_CARRYING_N(ch) = 0;
   IS_CARRYING_W(ch) = 0;
   
   if (IS_NPC(ch)) 
   {
      corpse->char_vnum = mob_index[ch->nr].iVNum;
      corpse->oldfilename[ 0 ] = 0;
   }
   else
   {
      if (ch->desc) 
      {
	 strcpy( corpse->oldfilename, GET_NAME( ch ) );
	 corpse->char_vnum = 0;
      }
      else 
      {
	 strcpy( corpse->oldfilename, GET_NAME( ch ) );
	 corpse->char_vnum = 100;
      }
   }
   
   
   corpse->carried_by = 0;
   corpse->equipped_by = 0;
   
   corpse->next = object_list;
   object_list = corpse;
   
   for( o = corpse->contains; o; o = o->next_content )
   o->in_obj = corpse;
   
   object_list_new_owner(corpse, 0);
   
   obj_to_room(corpse, ch->in_room);
   
   
   /* this must be set before dispel_magic, because if they */
   /* are flying and in a fly zone then the mud will crash  */
   if( GET_POS(ch) != POSITION_DEAD )
   GET_POS(ch) = POSITION_DEAD;
   
   /*
    remove spells
    */
   RemAllAffects(ch); /* new msw, 8/7/94 */
   
   /*   spell_dispel_magic(MAESTRO_DEI_CREATORI,ch,ch,0); */
   
   check_falling_obj(corpse, ch->in_room); /* hmm */
}

void change_alignment(struct char_data *ch, struct char_data *victim)
{
   int change, diff, d2;
   
   if( IS_NPC( ch ) || IS_IMMORTAL( ch ) ) 
   return;
   
   if( IS_GOOD(ch) && (IS_GOOD(victim)))
   {
      change = (GET_ALIGNMENT(victim)  / 200) * (MAX(1,GetMaxLevel(victim) - 
						     GetMaxLevel(ch)));
   }
   else if (IS_EVIL(ch) && (IS_GOOD(victim)))
   {
      change = (GET_ALIGNMENT(victim) / 30) * (MAX(1, GetMaxLevel(victim) - 
						   GetMaxLevel(ch)));
   }
   else if (IS_EVIL(victim) && (IS_GOOD(ch)))
   {
      change = (GET_ALIGNMENT(victim) / 30) * (MAX(1, GetMaxLevel(victim) - 
						   GetMaxLevel(ch)));
   }
   else if (IS_EVIL(ch) && (IS_EVIL(victim)))
   {
      change = ((GET_ALIGNMENT(victim) / 200)+1) * (MAX(1, GetMaxLevel(victim) - 
							GetMaxLevel(ch)));
   }
   else
   {
      change = ((GET_ALIGNMENT(victim) / 200)+1) * (MAX(1, GetMaxLevel(victim) - 
							GetMaxLevel(ch)));
   }
#if defined NEW_ALIGN
   change/=10;
#endif
   if (change == 0)
   {
      if (GET_ALIGNMENT(victim) > 0) 
      change = 1;
      else if (GET_ALIGNMENT(victim) < 0)
      change = -1;
   }
   
   if (HasClass(ch, CLASS_DRUID) && (GetMaxLevel(ch)<IMMORTALE)) 
   {
      diff = 0 - GET_ALIGNMENT(ch);
      d2 = 0 - (GET_ALIGNMENT(ch)-change);
      if (diff < 0) diff = -diff;
      if (d2 < 0) d2 = -d2;
      if (d2 > diff) 
      {
	 send_to_char("Beware, you are straying from the path\n\r", ch);
	 if (d2 > 150) 
	 {
	    send_to_char("Your lack of faith is disturbing\n\r", ch);
	    if (d2 > 275) 
	    {
	       send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	       if (d2 > 425) 
	       {
		  send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
		  drop_level(ch, CLASS_DRUID,FALSE);
	       }
	    }
	 }
      }    
   }  
   
   if (HasClass(ch, CLASS_PALADIN) && (GetMaxLevel(ch)<IMMORTALE))
   {
      diff = GET_ALIGNMENT(ch);
      d2 = (GET_ALIGNMENT(ch)-change);
      if (diff < 0) diff = -diff;
      if (d2 < 0) d2 = -d2;
      if (d2 < diff)    
      {
	 send_to_char("Beware, you are straying from the path\n\r", ch);
	 if (d2 < 950)      
	 {
	    send_to_char("Your lack of faith is disturbing\n\r", ch);
	    if (d2 < 550)        
	    {
	       send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	       if (d2 < 350)          
	       {
		  send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
		  drop_level(ch, CLASS_PALADIN,FALSE);
	       }
	    }
	 }
      }
   }  
   
   if (HasClass(ch, CLASS_RANGER) && (GetMaxLevel(ch)<IMMORTALE)) 
   {
      diff = GET_ALIGNMENT(ch);
      d2 = (GET_ALIGNMENT(ch)-change);
      if (diff < 0) diff = -diff;
      if (d2 < 0) d2 = -d2;
      if (d2 < diff)    
      {
	 send_to_char("Beware, you are straying from the path\n\r", ch);
	 if (d2 < 500)      
	 {
	    send_to_char("Your lack of faith is disturbing\n\r", ch);
	    if (d2 < 0)        
	    {
	       send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	       if (d2 < -350)          
	       {
		  send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
		  drop_level(ch, CLASS_RANGER,FALSE);
	       }
	    }
	 }
      }
   }  
   
   switch( GET_RACE( ch ) )
   {
    case RACE_ORC:
    case RACE_GOBLIN:
    case RACE_TROLL:
    case RACE_DARK_DWARF:
    case RACE_DEEP_GNOME:
      GET_ALIGNMENT( ch ) = -1000;
      break;
    default:
      GET_ALIGNMENT(ch) -= change;
      break;
   }  
   
   if (HasClass(ch, CLASS_DRUID)  && (GetMaxLevel(ch)<IMMORTALE) )
   {
      if (GET_ALIGNMENT(ch) > 600 || GET_ALIGNMENT(ch) < -600)
      {
	 send_to_char("Eldath, patron of druids, has excommunicated you for your heresies\n\r", ch);
	 send_to_char("You are forever more a mere cleric\n\r", ch);
	 REMOVE_BIT(ch->player.iClass, CLASS_DRUID);
	 if (!HasClass(ch, CLASS_CLERIC))
	 {
	    GET_LEVEL(ch, CLERIC_LEVEL_IND) = GET_LEVEL(ch, DRUID_LEVEL_IND);
	 }
	 GET_LEVEL(ch, DRUID_LEVEL_IND) = 0;
	 SET_BIT(ch->player.iClass, CLASS_CLERIC);
      }
   }
   
   
   if (HasClass(ch, CLASS_PALADIN)  && (GetMaxLevel(ch)<IMMORTALE) )
   {
      if (GET_ALIGNMENT(ch) < 350)
      {
	 send_to_char("Torm, patron of paladins, has excommunicated you for your heresies\n\r", ch);
	 send_to_char("You are forever more a mere warrior!\n\r", ch);
	 REMOVE_BIT(ch->player.iClass, CLASS_PALADIN);
	 if (!HasClass(ch, CLASS_WARRIOR))
	 {
	    GET_LEVEL(ch, WARRIOR_LEVEL_IND) = GET_LEVEL(ch, PALADIN_LEVEL_IND);
	 }
	 GET_LEVEL(ch, PALADIN_LEVEL_IND) = 0;
	 SET_BIT(ch->player.iClass, CLASS_WARRIOR);
      }
   }
   
   if (HasClass(ch, CLASS_RANGER)  && (GetMaxLevel(ch)<IMMORTALE) )
   {
      if (GET_ALIGNMENT(ch) < -350)
      {
	 send_to_char("Eldath, patron of rangers and druids, has excommunicated you for your heresies\n\r", ch);
	 send_to_char("You are forever more a mere warrior!\n\r", ch);
	 REMOVE_BIT(ch->player.iClass, CLASS_RANGER);
	 if (!HasClass(ch, CLASS_WARRIOR))
	 {
	    GET_LEVEL(ch, WARRIOR_LEVEL_IND) = GET_LEVEL(ch, RANGER_LEVEL_IND);
	 }
	 GET_LEVEL(ch, RANGER_LEVEL_IND) = 0;
	 SET_BIT(ch->player.iClass, CLASS_WARRIOR);
      }
   }
   
   
   GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
   GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);
   
}

void death_cry(struct char_data *ch)
{
   int door, was_in;
   
   if (ch->in_room == -1)
   return;
   
   act( "$c0005Il tuo sangue si gela al grido di morte di $c0015$n's$c0005.", 
       FALSE, ch,0,0,TO_ROOM );
   was_in = ch->in_room;
   
   for (door = 0; door <= 5; door++)
   {
      if (CAN_GO(ch, door))
      {
	 ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
	 act( "$c0005Il tuo sangue si gela quando odi un grido di morte.",
	     FALSE, ch, 0, 0, TO_ROOM );
	 ch->in_room = was_in;
      }
   }
}



void raw_kill(struct char_data *ch,int killedbytype)
{
   struct char_data *tmp, *tch;
   /* tell mob to hate killer next load here, or near here */
   
   if( ( tmp = ch->specials.fighting ) != NULL )
   {
      if( is_murdervict( ch ) && 
	 ( IS_PC(tmp) || IS_SET(tmp->specials.act,ACT_POLYSELF) ) && ch != tmp )
      {
	 int i = 0;
	 for( tch = real_roomp( ch->in_room )->people; tch; 
	     tch = tch->next_in_room )
	 {
	    i++;
	    if( i > 60 )
	    {
	       mudlog( LOG_SYSERR, "Problemi nella locazione %d. "
		      "Piu` di 60 caratteri.", ch->in_room );
	       break;
	    }
	    
	    if( ch != tch && GET_POS( tch ) > POSITION_SLEEPING && 
	       IS_NPC( tch ) && CAN_SEE( tch, tmp ) )
	    {
	       if( IS_GOOD(tch) || IS_NEUTRAL(tch) )
	       {
		  mudlog( LOG_PLAYERS, "Setting MURDER bit on %s for killing %s.",
			 GET_NAME(tmp),GET_NAME(ch) );
		  SET_BIT(tmp->player.user_flags,MURDER_1);
		  act( "$c0009[$c0015$n$c0009] punta a $N ed urla 'ASSASSINO'!", 
		      FALSE, tch, 0, tmp, TO_NOTVICT );
		  act( "$c0009[$c0015$n$c0009] punta verso di te ed urla 'ASSASSINO'!",
		      FALSE, tch, 0, tmp, TO_VICT );
	       } /* good/neut */
	    } /* npc */
	 } /* for */
      } /* start murder stuff */
      
      stop_fighting(ch);
   }
   
   death_cry( ch );
   
   if( IS_MOB( ch ) && !IS_SET( ch->specials.act, ACT_POLYSELF ) &&
      mob_index[ ch->nr ].func )
   (*mob_index[ch->nr].func)( ch, 0, "", ch, EVENT_DEATH );
   
   /*
    * give them some food and water so they don't whine.
*/
   if (GetMaxLevel(ch)<IMMORTALE)
   GET_COND(ch,THIRST)=10;
   if (GetMaxLevel(ch)<IMMORTALE)
   GET_COND(ch,FULL)=10;
   /* Non sono riuscito a trovare il bug della morte in reception 
    * per cui gli rialzo gli hp */
   GET_HIT(ch)=MIN(50,GET_MAX_HIT(ch)); alter_hit(ch,0);

   /* remove berserk after they flee/die... */
   
   if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK))  
   {
      REMOVE_BIT(ch->specials.affected_by2,AFF2_BERSERK);
   }  

   /* Rimuovo anche il flag del PARRY Gaia 7/2000 */

   if (IS_SET(ch->specials.affected_by2, AFF2_PARRY))
   {
      REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
   }

 
   /*
    *   return them from polymorph
    */
   
   make_corpse(ch,killedbytype);
   zero_rent(ch);
   extract_char(ch);
}

int clan_gain(struct char_data *ch,int gain)
{
   
   char buf[512];
   struct char_data *k;
   int quota=0;
   if (IS_PC(ch) && HAS_PRINCE(ch))
   {
      
      k=get_char_room(GET_PRINCE(ch),ch->in_room);
      if (k) 
      {
	 quota=(int)(gain/100*6);
	 gain-=quota;
	 sprintf( buf,"Paghi il giusto tributo di esperienza (%d punti) a $N!",
		 quota);
	 act( buf, FALSE, ch, 0, k, TO_CHAR );
	 sprintf( buf,"Ricevi da $N un tributo di esperienza (%d punti)!",
		 quota);
	 act( buf, FALSE, k,0,ch, TO_CHAR );
	 gain_exp(k,quota);
      }
   }
   return(gain);
}
void save_exp_to_file(struct char_data *ch,int xp) 
{
   
   FILE *fdeath;
   char nomefile[1000];
   sprintf(nomefile,"%s/%s.dead",PLAYERS_DIR,lower(GET_NAME(ch)));
   mudlog(LOG_PLAYERS,"Opening %s",nomefile);
   if ((fdeath=fopen(nomefile,"w+")))
   {
      mudlog(LOG_PLAYERS,"Saving xp per %s",GET_NAME(ch));
      fprintf(fdeath,"%d : %ld",(int)GET_EXP(ch),(long)time(0));
      fclose(fdeath);
   }
   else 
   {
      mudlog(LOG_PLAYERS,"Impossibile salvare xp per %s",GET_NAME(ch));
   }
}

void die(struct char_data *ch,int killedbytype, struct char_data *killer)

{
   struct char_data *pers;
   struct affected_type af;
   int i,tmp;
   char buf[80];
   int fraction;
   int HaPersoUnLivello=0;
   /* need at least 1/fraction worth of exp for the minimum needed for */
   /* the pc's current level, or else you lose a level.  If all three  */
   /* classes are lacking in exp, you lose one level in each class. */
   increase_blood(ch->in_room);

   fraction = 3;
   if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) 
   {
      /*
       *   take char from storage, to room
       */
      if (ch->desc) 
      {
	      pers = ch->desc->original;
	      char_from_room(pers);
	      char_to_room(pers, ch->in_room);
	      SwitchStuff(ch, pers);
	      extract_char(ch);
	      ch = pers;
      }
      else 
      {
	 /* we don't know who the original is.  Gets away with it, i guess*/
      }
   }
   
#if LEVEL_LOSS
   for(i=0;i<MAX_CLASS;i++) 
   {
      if (GET_LEVEL(ch,i) > 1) 
      {
	 if (GET_LEVEL(ch,i) >= IMMORTALE) 
	 break;
	 if (GET_EXP(ch) < (titles[i][ (int)GET_LEVEL(ch, i) ].exp/fraction)) 
	 {
	    tmp = (ch->points.max_hit)/GetMaxLevel(ch);
	    ch->points.max_hit -= tmp;
	    GET_LEVEL(ch, i) -= 1;
	    ch->specials.spells_to_learn -= 
	    MAX(1, MAX(2, wis_app[ (int)GET_RWIS(ch) ].bonus)/HowManyClasses(ch));
	    if (ch->specials.spells_to_learn < 0) // SALVO corregger bug pracche negative
	        ch->specials.spells_to_learn = 0;
	    send_to_char("\n\rL'insufficiente esperienza ti costa un livello!\n\r",
			 ch);
	    HaPersoUnLivello=1;
	 }
      }
   }
#endif
   /* GGPATCH Morte et similia */
   if (!HaPersoUnLivello) /* Quando si cominciano a perdere livelli 
			   * si smette di perdere xp
			   * */
   {
#if DEATH_FIX
      if (IS_PC(ch))
      {
	 save_exp_to_file(ch,GET_EXP(ch));
      }
#endif

#if NICE_PKILL
      if (killer && (killer->master)) // SALVO il pkill dei mob schiavi va' al padrone
        if (!IS_PC(killer)) killer = killer->master; 

      if(killer && IS_PC(killer))
      {
         if ( killedbytype == SPELL_CHANGE_FORM )
           gain_exp_rev(ch,-200000);
         else
           {
            if (!IS_PRINCE(ch))
	              gain_exp_rev(ch,-GET_EXP(ch)/100);
            else
              gain_exp_rev(ch,-5000000);
           } 
	         
	 if (IS_PC(ch) && !(killedbytype == SPELL_CHANGE_FORM) ) 
	 {
	    if (!IS_PRINCE(ch))
        gain_exp_rev(killer,-GET_EXP(killer)/100);
	    else
	       gain_exp_rev(killer,-5000000);
      af.type=SPELL_NO_MESSAGE;
	    af.duration=MAX(5,20+GetMaxLevel(killer)-GetMaxLevel(ch));
	    af.modifier=0;
	    af.location=APPLY_BV2;
	    af.bitvector=AFF2_PKILLER;
	    if (killer!=ch) // SALVO non si puo' mettere il pkill per un suicidio
	    {
	      if (IS_POLY(killer))  // SALVO il flag pkiller va' all'originale, corretto
	        affect_to_char(killer->desc->original,&af);
	      else
	        affect_to_char(killer,&af);
	    }
	 }
      }
      else
      {
#endif	 
#if NEW_EXP
   if (GetMaxLevel(ch) > 15)
	 gain_exp(ch, -GET_EXP(ch)/2);
	 else if (GetMaxLevel(ch) > 10)
	 gain_exp(ch, -GET_EXP(ch)/3);
	 else if (GetMaxLevel(ch) > 5)
	 gain_exp(ch, -GET_EXP(ch)/4);
	 else
	 gain_exp(ch, -GET_EXP(ch)/5);
#else
#if OLD_EXP
	 gain_exp(ch, -GET_EXP(ch)/2);
#else
	 
	 /* New exp loss... Thanatos, da qualche parte, sorride....  */
#if NEWER_EXP
	 int loss=0;
	 int oldloss=0;
	 int mclass=0,mlevel=0;
	 mlevel=GetMaxLevel(ch);
	 mclass=GetMaxClass(ch);
	 if ((mlevel< 6) ||  
	     (GET_EXP(ch)<titles[mclass][mlevel-1].exp)
	     )
	 {
	    loss=GET_EXP(ch)/20; // Xp loss al 5% per i novizi <6^ livello
	 }
	 else
	 {
	    loss=GET_EXP(ch)/10; // Xp loss al 10% per gli altri
	 }
#if NICE_MULTICLASS
	 loss=loss/((HowManyClasses(ch)+1)/2);
#endif
#if GROUP_DEATH
	 oldloss=loss;
	 loss=group_loss(ch,loss);
	 if (loss<oldloss) 
	 {
	    send_to_char("Il gruppo ha preso su di se' una parte della tua perdita\n",ch);
	 }
#endif
	 //if (IS_PRINCE(ch))
	 //  loss=50000000;
   if(loss>20000000) loss=20000000; // Capapggio della hole per tutti a 20MXp
   gain_exp(ch,-loss);
	 mudlog(LOG_PLAYERS,"%s ha perso %d(%d) xp",GET_NAME(ch),loss,oldloss);
#endif NEWER
#endif NEW
#endif OLD
#if NICE_PKILL
      }
#endif
#if LEVEL_LOSS
      
      /* warn people if their next death will result in a level loss */
      for(i=0;i<MAX_CLASS;i++) 
      {
	 if( GET_LEVEL(ch,i) > 1 ) 
	 {
	    if( GET_EXP(ch) < ( titles[i][ (int)GET_LEVEL(ch, i) ].exp / fraction ) ) 
	    {
	       send_to_char( "\n\r\n\rATTENZIONE! ATTENZIONE! ATTENZIONE! ATTENZIONE!\n\r",
			    ch);
	       send_to_char( "La tua prossima morte ti costera' la perdita di un livello,\n\r",
			    ch);
	       sprintf( buf, "a meno che tu non acquisisca almeno %d ulteriori punti esperienza.\n\r",
		       ( titles[i][ (int)GET_LEVEL(ch, i) ].exp / fraction ) - 
		       GET_EXP(ch) );
	       send_to_char(buf,ch);
	    }
	 }
      }
#endif
   }
   DeleteHatreds(ch);
   DeleteFears(ch);

   raw_kill(ch,killedbytype);
}

#define EXP_CAP          150000 
#define EXP_CAP_1        250000 
#define EXP_CAP_2        350000 
#define EXP_CAP_3        450000 
#define EXP_CAP_4        500000
#define EXP_CAP_5        550000 
#define EXP_CAP_6        600000 
#define EXP_CAP_7        700000 
#define EXP_CAP_8        800000 
#define EXP_CAP_9        900000 
#define EXP_CAP_OTHER   1000000
long ExpCaps(struct char_data *ch,int group_count, long passedtotal)
{
   int k;
   int i;
   long total; /*GGPATCH*/      
   if( group_count >= 1 )
   {    
      switch(group_count) 
      {
       case 1: 
	 total = EXP_CAP_1;
	 break;
       case 2:
	 total=EXP_CAP_2;
	 break;
       case 3:
	 total=EXP_CAP_3;
	 break;
       case 4:
	 total=EXP_CAP_4;
	 break;
       case 5:
	 total=EXP_CAP_5;
	 break;
       case 6:
	 total=EXP_CAP_6;
	 break;
       case 7:
	 total=EXP_CAP_7;
	 break;
       case 8:
	 total=EXP_CAP_8;
	 break;
       case 9:
	 total=EXP_CAP_9;
	 break;
       default:
	 total=EXP_CAP_OTHER;
	 break;
      } /* end switch */
   }  
   else
   {
      /* not grouped, so limit max exp gained so */
      total = EXP_CAP;        /* grouping will be used more and benifical */
   }
   if (passedtotal > total)
   {
      k=0;      
      for (i=MAGE_LEVEL_IND;i<=MAX_CLASS;i++)
      {
	 if (GET_LEVEL(ch,i))
	 k++;
      }
      k--;
      total+=((total/2)*k);
   }
   if (passedtotal> total) 
   passedtotal=total;
   return(passedtotal);
}

long GroupLevelRatioExp( struct char_data *ch,int group_max_level,
			long experiencepoints)
{
   unsigned int diff=0;
   //diff=abs(group_max_level-GetMaxLevel(ch));
   /*Eliminata la penalizzazione eccessiva sui gruppi verticali*/
   
//#ifndef ALAR    
   
   if (diff)
   {
      /* More than 10 levels difference, then we knock down 
       * the ratio of EXP he gets, keeping high level people
       * from getting newbies up to fast... */
      /* Inadatto alle nuove tabelle di gain, dobbiamo 
         aumentare i rapporti altrimenti un novizio che
         gruppa con un principe diventa chump al primo MOB
         e Allievo al secondo! E' una necessità, non una
         limitazione!

      if ( diff >= 45 )
      experiencepoints = 1;
      else if( diff >= 40 )
      experiencepoints = experiencepoints / 10;
      else if( diff >= 30 )
      experiencepoints = experiencepoints / 5;
      else if( diff >= 20 )
      experiencepoints = experiencepoints / 3;
      else if( diff >= 10 )
      experiencepoints = experiencepoints / 2; */
      //else if( diff >= 40 )
      //experiencepoints = 1;
      /*if ( diff >= 45 )
        experiencepoints = 1;
      else if ( diff >= 40 )
        experiencepoints /= 4000;
      else if( diff >= 35 )
        experiencepoints /= 1500;
      else if( diff >= 30 )
        experiencepoints /= 400;
      else if( diff >= 25 )
        experiencepoints /= 144;
      else if( diff >= 20 )
        experiencepoints /= 50;
      else if( diff >= 15 )
        experiencepoints /= 18;
      else if( diff >= 10 )
        experiencepoints /= 6;
      else if( diff >= 5 )
        experiencepoints /= 2;*/
   }
   
//#else
//   /* Qui su myst preferiamo che i gruppi siano verticali */
//   if (diff >=30) 
//   experiencepoints/=2;
//   else if (diff >= 40)                           /*GG*/
//   experiencepoints /=3;   /*GG*/
//#endif 
   
   return experiencepoints;
}

#define XPDISTRATIO 1.0/2.5

float GroupXPRatio[ABS_MAX_LVL]={
  0,
  1,
  pow((titles[0][3].exp-titles[0][2].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][4].exp-titles[0][3].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][5].exp-titles[0][4].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][6].exp-titles[0][5].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][7].exp-titles[0][6].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][8].exp-titles[0][7].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][9].exp-titles[0][8].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][10].exp-titles[0][9].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][11].exp-titles[0][10].exp)/titles[0][2].exp,XPDISTRATIO), // 10
  pow((titles[0][12].exp-titles[0][11].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][13].exp-titles[0][12].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][14].exp-titles[0][13].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][15].exp-titles[0][14].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][16].exp-titles[0][15].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][17].exp-titles[0][16].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][18].exp-titles[0][17].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][19].exp-titles[0][18].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][20].exp-titles[0][19].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][21].exp-titles[0][20].exp)/titles[0][2].exp,XPDISTRATIO), // 20
  pow((titles[0][22].exp-titles[0][21].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][23].exp-titles[0][22].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][24].exp-titles[0][23].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][25].exp-titles[0][24].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][26].exp-titles[0][25].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][27].exp-titles[0][26].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][28].exp-titles[0][27].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][29].exp-titles[0][28].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][30].exp-titles[0][29].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][31].exp-titles[0][30].exp)/titles[0][2].exp,XPDISTRATIO), // 30
  pow((titles[0][32].exp-titles[0][31].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][33].exp-titles[0][32].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][34].exp-titles[0][33].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][35].exp-titles[0][34].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][36].exp-titles[0][35].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][37].exp-titles[0][36].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][38].exp-titles[0][37].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][39].exp-titles[0][38].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][40].exp-titles[0][39].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][41].exp-titles[0][40].exp)/titles[0][2].exp,XPDISTRATIO), // 40
  pow((titles[0][42].exp-titles[0][41].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][43].exp-titles[0][42].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][44].exp-titles[0][43].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][45].exp-titles[0][44].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][46].exp-titles[0][45].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][47].exp-titles[0][46].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][48].exp-titles[0][47].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][49].exp-titles[0][48].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((titles[0][50].exp-titles[0][49].exp)/titles[0][2].exp,XPDISTRATIO),
  pow((184000000-titles[0][50].exp)/titles[0][2].exp,XPDISTRATIO), // 50
  pow((184000000-titles[0][50].exp)/titles[0][2].exp,XPDISTRATIO),
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0 // 60
};

void group_gain( struct char_data *ch,struct char_data *victim )
{
   char buf[ 256 ];
   // int no_members, share; Substituted by EleiMiShill
   float no_members, share;
   struct char_data *k;
   struct follow_type *f;
   int total; 
   int pc, group_count = 0,
   group_max_level = 1; /* the highest level number the group has */
   
   /*GGPATCH PKILLING STUFF*/
   /* can't get exp for killing players */
   /* A regime l'uccisione di un PC deve far perdere xp */
   if( IS_PC( victim ) ) return;

   if( !( k = ch->master ) ) 
    k = ch; /*Questo PC non ha master.. vuol dire che e' il leader*/

   if( IS_AFFECTED( k, AFF_GROUP ) && k->in_room == ch->in_room )
   {
      group_max_level = GetMaxLevel( k );
      //no_members = GET_GRP_LEVEL(k); Substituted by EleiMiShill
      no_members = GroupXPRatio[GetMaxLevel( k )];
   }
   else
   no_members = 0;
   
   pc = 0;
   
   
   for( f = k->followers; f; f = f->next )
   {
      if( IS_AFFECTED( f->follower, AFF_GROUP ) &&
	      ( f->follower->in_room == ch->in_room ) ) 
      {
	      // no_members += GET_GRP_LEVEL( f->follower );
        no_members += GroupXPRatio[GetMaxLevel(f->follower)];
	      if( group_max_level < GetMaxLevel( f->follower ) )
	        group_max_level = GetMaxLevel( f->follower );
	      if (IS_PC(f->follower))
	        pc++;
      }
   }
   
   if( IS_AFFECTED( k, AFF_GROUP ) &&
      ( k->in_room == ch->in_room ) && IS_PC( k ) &&
      GetMaxLevel( k ) > group_max_level - 25 )
   {
      group_count++;
   }
   
   for( f = k->followers; f; f = f->next )
   {
      if( IS_AFFECTED( f->follower, AFF_GROUP ) &&
	      ( f->follower->in_room == ch->in_room ) && IS_PC( f->follower ) 
	 
	 /*GGPATCH  && GetMaxLevel( f->follower ) > group_max_level - 25*/ )
      {
	      group_count++;
      }
   }
   
   if( group_count > 0 )
    group_count--;
   
   if( pc > 10 )
    pc = 10;
   
   if( no_members >= 1 )
    share = GET_EXP( victim ) / no_members;
   else
    share = 0;
   
   if( IS_AFFECTED( k, AFF_GROUP ) && k->in_room == ch->in_room )
   {
      //total = share * GET_GRP_LEVEL( k ); Substituted by EleiMiShill
      total =(long) (share * GroupXPRatio[GetMaxLevel(k)]);
      if( pc )
      {
	      total *= 100 + ( 3 * pc );
	      total /= 100;
      }
      
      total = RatioExp( k, victim, total );
      total = GroupLevelRatioExp( k, group_max_level, total );
      total = ExpCaps( k, group_count, total );   /* figure EXP MAXES */
      total = clan_gain( k, total );
      gain_exp(k,total);
      sprintf(buf,"La tua parte di esperienza e` di %d punti.",total );
      act( buf, FALSE, k, 0, 0, TO_CHAR );
      change_alignment( k, victim );
   }
   
   for( f = k->followers; f; f = f->next ) 
   {
      if( IS_AFFECTED( f->follower, AFF_GROUP ) && 
	       f->follower->in_room == ch->in_room )
      {
	      //total = share * GET_GRP_LEVEL( f->follower ); Substituted by EleiMiShill
        total = (long)(share * GroupXPRatio[GetMaxLevel(f->follower)]);

	      if (IS_PC(f->follower)) 
	      {
	        total *= 100 + pc;
	        total /= 100;
	      }
	      else 
	        total /= 2;
	 
	     if (IS_PC(f->follower)) 
	     {
	        total = RatioExp( f->follower, victim, total );
	        total = GroupLevelRatioExp( f->follower, group_max_level, total );
	        total = ExpCaps( f->follower, group_count, total );  /* figure EXP MAXES */
	        total = clan_gain( f->follower,  total );
	        gain_exp(f->follower,total);
	        sprintf( buf,"La tua parte di esperienza e` di %d punti.", total );
	        act( buf, FALSE, f->follower, 0, 0, TO_CHAR );
	        change_alignment( f->follower, victim );
	     }
	     else 
	     {
	       if( f->follower->master && IS_AFFECTED( f->follower, AFF_CHARM ) )
	       {
	          total = RatioExp( f->follower->master, victim, total );
	          total = GroupLevelRatioExp( f->follower, group_max_level, total );
	          total = ExpCaps( f->follower, group_count, total );  /* figure EXP MAXES */
	          if( f->follower->master->in_room == f->follower->in_room )
	          {
		            sprintf( buf,"Ricevi %d punti della parte di esperienza di $N'.",
			            total );
		            act( buf, FALSE, f->follower->master, 0, f->follower, TO_CHAR );
		            gain_exp( f->follower->master,  total );
		            change_alignment( f->follower, victim );
	          }
	    }
	    else 
	    {
	       total = RatioExp( f->follower, victim, total );
	       total = GroupLevelRatioExp( f->follower, group_max_level, total );
	       total= ExpCaps(f->follower, group_count, total );  /* figure EXP MAXES */
	       sprintf( buf,"La tua parte di esperienza e` di %d.", total );
	       act( buf, FALSE, f->follower, 0, 0, TO_CHAR );
	       gain_exp( f->follower,  total );
	       
	       change_alignment( f->follower, victim );
	    }
	 }
      }
   }
}

int group_loss( struct char_data *ch ,int loss)
{
   /* La morte di chi e` di livello inferiore penalizza i membri del gruppo
    * di livello superiore
    * */
 const int perdita[10] =
   {
   /* 4  9 14 19 24 29 34  39  44  49 */
      0, 0, 1, 5,15,35,60,100,200,300
   };
   char buf[512];
   struct char_data *k;
   struct follow_type *f;
   int diff=0;
   int lose=0;
   int multi=100;
   int victlevel=0;
   if( !IS_AFFECTED( ch, AFF_GROUP ) )
   return(loss);
   PushStatus("Group-Loss");
   loss/=100;
   if( !( k = ch->master ) ) 
   k = ch; /*Questo PC non ha master.. vuol dire che era il leader*/
   victlevel=GET_AVE_LEVEL(ch);
   PushStatus("Followers");
   for( f = k->followers; f; f = f->next )
   {
      if( IS_AFFECTED( f->follower, AFF_GROUP ))
      {
	 diff=(GET_AVE_LEVEL(f->follower)-victlevel);
	 if (diff>0) 
	 {
	    diff=MAX(0,(int)(diff/5));
	    lose=(loss*perdita[diff]);
	    sprintf( buf,"La morte di $N ti e` costata %d punti.", 
		    lose );
	    act( buf, FALSE, f->follower, 0, ch, TO_CHAR );
	    mudlog(LOG_PLAYERS,"GD dead:%s(%d), perdita=%d, diff=%d, %s(%d) perde:%d",
		   GET_NAME(ch),
		   GET_AVE_LEVEL(ch),
		   loss*100,diff,
		   GET_NAME(f->follower),
		   GET_AVE_LEVEL(f->follower),lose);
	    if (lose>0)
	    {
	       multi-=(4+number(1,8));
	       gain_exp( f->follower, -lose );
	    }
	 }
      }
   }
   PopStatus();
   if (k!=ch) 
   {
      PushStatus("k");
      if( IS_AFFECTED( k, AFF_GROUP ))
      {
	 diff=(GET_AVE_LEVEL(k)-victlevel);
	 if (diff>0) 
	 {
	    diff=MAX(0,(int)(diff/5));
	    lose=(loss*perdita[diff]);
	    sprintf( buf,"La morte di $N ti e` costata %d punti.", 
		    lose );
	    act( buf, FALSE, k, 0, ch, TO_CHAR );
	    mudlog(LOG_PLAYERS,"GD dead:%s(%d), perdita=%d, diff=%d, %s(%d) perde:%d",
		   GET_NAME(ch),
		   GET_AVE_LEVEL(ch),
		   loss*100,
		   diff,
		   GET_NAME(k),
		   GET_AVE_LEVEL(k),
		   lose);
	    if (lose > 0) 
	    {
	       multi-=(4+number(1,8));
	       gain_exp( k, -lose );
	    }
	 }
      }
      PopStatus();
   }
   PopStatus();
   return(loss*MAX(70,multi));
}

char *replace_string(char *str, char *weapon, char *weapon_s,
		     char *location_hit, char *location_hit_s)
{
   static char buf[256];
   char *cp;
   
   cp = buf;
   
   for (; *str; str++) {
      if (*str == '#') {
	 switch(*(++str)) {
	  case 'W' : 
	    for (; *weapon; *(cp++) = *(weapon++));
	    break;
	  case 'w' : 
	    for (; *weapon_s; *(cp++) = *(weapon_s++));
	    break;
	    
	    /* added this to show where the person was hit */
	  case 'L' : 
	    for (; *location_hit; *(cp++) = *(location_hit++));
	    break;
	  case 'l' : 
	    for (; *location_hit_s; *(cp++) = *(location_hit_s++));
	    break;

	  default :
	    *(cp++) = '#';
	    break;
	 }
      } else {
	 *(cp++) = *str;
      }
      
      *cp = 0;
   } /* For */
   
   return(buf);
}




void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type, int location)
{
   struct obj_data *wield;
   char *buf;
   int snum;
   
   static struct dam_weapon_type {
      char *to_room;
      char *to_char;
      char *to_victim;
   } dam_weapons[] = {
      
      {
	 "$n misses $N.",                           /*    0    */
	 "You miss $N.",
	 "$n misses you." 
      }, 
      
      { 
	 "$n bruises $N with $s #w #l.",                       /*  1.. 2  */
	 "You bruise $N as you #w $M #l.",
	 "$n bruises you as $e #W your #L." 
      }, 
      
      {
	 "$n barely #W $N #l.",                                   /*  3.. 4  */
	 "You barely #w $N #l.",
	 "$n barely #W your #L."
      }, 
      
      {
	 "$n #W $N #l.",                                          /*  5.. 10   */
	 "You #w $N #l.",
	 "$n #W your #L."
      }, 
      
      {
	 "$n #W $N hard #l.",                                     /*  11..15 */
	 "You #w $N hard #l.",
	 "$n #W you hard on your #L."
      }, 
      
      {
	 "$n #W $N very hard #l.",                                /* 16..25  */
	 "You #w $N very hard #l.",
	 "$n #W you very hard on you #L."
      }, 
      
      {
	 "$n #W $N extremely well #l.",                          /* 26..35  */
	 "You #w $N extremely well #l.",
	 "$n #W you extremely well on your #L."
      }, 
      
      {
	 "$n $c0011massacres$c0007 $N with $s #w #l.",     /* 36..45    */
	 "You $c0010massacre$c0007 $N with your #w #l.",
	 "$n $c0009massacres$c0007 you with $s #w on your #L."
      },
      
      {
	 "$n $c0011devastates$c0007 $N with $s #w #l.",    /* > 45 */
	 "You $c0010devastate$c0007 $N with your #w #l.",
	 "$n $c0009devastates$c0007 you with $s #w on your #L."
      }
      
   };
   
   
   w_type -= TYPE_HIT;   /* Change to base of table with text */
   
   
   wield = ch->equipment[WIELD];
   
   if (dam <= 0)
   {
      snum = 0;
   }
   else if (dam <= 2)
   {
      snum = 1;
   }
   else if (dam <= 4)
   {
      snum = 2;
   }
   else if (dam <= 10)
   {
      snum = 3;
   }
   else if (dam <= 15)
   {
      snum = 4;
   }
   else if (dam <= 25)
   {
      snum = 5;
   }
   else if (dam <= 35)
   {
      snum = 6;
   }
   else if (dam <= 45)
   {
      snum = 7;
   }
   else
   {
      snum = 8;
   }
   buf = replace_string(dam_weapons[snum].to_room, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
			location_hit_text[location].plural,   location_hit_text[location].singular);
   act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
   
   buf = replace_string(dam_weapons[snum].to_char, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
			location_hit_text[location].plural,   location_hit_text[location].singular);
   act(buf, FALSE, ch, wield, victim, TO_CHAR);
   
   buf = replace_string(dam_weapons[snum].to_victim, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
			location_hit_text[location].plural,   location_hit_text[location].singular);
   act(buf, FALSE, ch, wield, victim, TO_VICT);
   
}

int DamCheckDeny( struct char_data *ch, struct char_data *victim, int type )
{
   struct room_data *rp;
   char buf[MAX_INPUT_LENGTH];
   
   /*  assert(GET_POS(victim) > POSITION_DEAD);  */
   
   if( GET_POS( victim ) == POSITION_DEAD )
   {
      mudlog( LOG_SYSERR, 
	     "GET_POS(%s) == POSITION_DEAD in DamCkeckDeny fight.c" ,
	     GET_NAME(victim));
      return( TRUE );
   }
     
   rp = real_roomp(ch->in_room);
   if( rp && (rp->room_flags&PEACEFUL) && type!=SPELL_POISON && 
      type!=SPELL_HEAT_STUFF && type != TYPE_SUFFERING) 
   {
      mudlog( LOG_SYSERR, "damage %s vs %s called in PEACEFUL room", GET_NAME(ch), GET_NAME(victim), type ); // SALVO fix buf non ci vuole
      return( TRUE ); /* true, they are denied from fighting */
   }
   return( FALSE );
}

int DamDetailsOk( struct char_data *ch, struct char_data *v, int dam, int type )
{
   
   if( dam < 0 ) 
   return( FALSE );
   
   /* we check this already I think, be sure to keep an eye out. msw */
   if( type != TYPE_RANGE_WEAPON && 
      ( type < TYPE_GENERIC_FIRST || type > TYPE_GENERIC_LAST ) &&
      ch->in_room != v->in_room ) 
   return( FALSE );
   
   if( ch == v && type != SPELL_POISON && type != SPELL_HEAT_STUFF && 
       type != SPELL_CHANGE_FORM &&       /*ACIDUS shock change_form */
      ( type < TYPE_GENERIC_FIRST || type > TYPE_GENERIC_LAST ) &&
      type != TYPE_SUFFERING ) 
   return( FALSE );
   
   if( MOUNTED( ch ) )
   {
      if( MOUNTED( ch ) == v )
      {
	 FallOffMount( ch, v );
	 Dismount( ch, MOUNTED( ch ), POSITION_SITTING );
      }
   }
   
   return( TRUE );
}


int SetCharFighting(struct char_data *ch, struct char_data *v)
{
   if( v != ch && GET_POS(ch) > POSITION_STUNNED && !ch->specials.fighting &&
      v->attackers < 6 ) 
   {
      set_fighting(ch, v);
      /*GET_POS(ch) = POSITION_FIGHTING;*/
      return(TRUE);
   }
   return(FALSE);
}


int SetVictFighting(struct char_data *ch, struct char_data *v)
{
   
   if( v != ch && GET_POS(v) > POSITION_STUNNED && !v->specials.fighting &&
      ch->attackers < 6 ) 
   {
      set_fighting(v, ch);
      /*GET_POS(v) = POSITION_FIGHTING;*/
      return(TRUE);
   }
   return(FALSE);
}

int DamageTrivia(struct char_data *ch, struct char_data *v, 
                   int dam, int type, int location)
{
   
   char buf[255];
   int classe=-1;
   int dummy,result;
   if( (type == TYPE_SUFFERING) || (type == SPELL_CHANGE_FORM) ) /*ACIDUS shock change_form */
   return( dam );
   
   // ACIDUS modifica per berserk in gruppo
   if( (v->master == ch) && !IS_SET(ch->specials.affected_by2, AFF2_BERSERK) )
   stop_follower( v );
   
   if( IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED2( ch, AFF2_ANIMAL_INVIS ) )
   appear( ch );
   
   if( IS_AFFECTED(ch, AFF_SNEAK ) ) 
   affect_from_char( ch, SKILL_SNEAK );
   
   if(IS_AFFECTED(ch, AFF_HIDE))
   REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
   
   if( IS_AFFECTED( v, AFF_SANCTUARY ) ) 
   dam = MAX( (int)( dam / 2 ), 0 );  /* Max 1/2 damage when sanct'd */
   
   if( IS_SET( ch->specials.affected_by2, AFF2_BERSERK )
      && type >= TYPE_HIT) /*chec to see if berserked and using a weapon */
   {
      dam = berserkdambonus( ch, dam );     /* More damage if berserked */
   }
   /* Qui testa le immunita`... se ha classe Monk NON la chiamo.
    * Un monaco a mani nude secondo me deve colpire chiunque.
    * Testo anche hold e totale oggetti portati
    */ 
   WEARING_N(ch,dummy,result);
   if (  HasClass(ch,CLASS_MONK) && 
       !((ch->equipment[WIELD]) &&
	 (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)
	 ) &&
       !((ch->equipment[HOLD]) &&
	 (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON)
	 ) &&
       ((IS_CARRYING_N(ch)+result)<(MONK_MAX_RENT +5))
       )
   classe=CLASS_MONK;
   if (HasClass(ch,CLASS_BARBARIAN))
   classe=CLASS_BARBARIAN;
   dam = PreProcDam( v, type, dam, classe);
   
   
#if PREVENT_PKILL
   if(!IS_IMMORTAL(ch))
   if( ch != v && !CanFightEachOther( ch, v ) )
   {
      act("Il tuo attacco verso $N non ha effetto!",FALSE,ch,0,v,TO_CHAR);
      act("L'attacco di $n non ha effetto!",FALSE,ch,0,v,TO_VICT);
      dam = -1;        
   }
#endif
   
   /* shield makes you immune to magic missle! */
   if( affected_by_spell( v, SPELL_SHIELD ) && type == SPELL_MAGIC_MISSILE )
   {
      act( "Il missile magico di $n e` deviato dallo scudo di $N!", FALSE, ch, 
	  0, v, TO_NOTVICT );
      act( "Lo scudo di $N devia il tuo missile magico!", FALSE, ch, 0, v, 
	  TO_CHAR);
      act( "Il tuo scudo devia il missile magico di $n!", FALSE, ch, 0, v, 
	  TO_VICT);
      dam = -1;
   }
   
   if( affected_by_spell( v, SKILL_TOWER_IRON_WILL ) && 
      type == SKILL_PSIONIC_BLAST ) 
   {
      act( "L'attacco psionico di $n e ignorato da $N!", FALSE, ch, 0, v, 
	  TO_NOTVICT );
      act( "Lo scudo psionico di $N lo protegge dal tuo attacco!", FALSE, ch, 
	  0, v, TO_CHAR);
      act( "La tua protezione psionica ti protegge dall'attacco di $n!", FALSE, 
	  ch, 0, v, TO_VICT);
      dam = -1;
   }
   
   /* we check for prot from breath weapons here */
   if( type >= FIRST_BREATH_WEAPON && type <= LAST_BREATH_WEAPON) 
   {
      int right_protection=FALSE;
      
      if (affected_by_spell(v,SPELL_PROT_DRAGON_BREATH)) 
      {        /* immune to all breath */
	 right_protection = TRUE;
      } 
      else if( affected_by_spell(v,SPELL_PROT_BREATH_FIRE) && 
	      type == SPELL_FIRE_BREATH) 
      {
	 right_protection = TRUE;            
      } 
      else if( affected_by_spell(v,SPELL_PROT_BREATH_GAS) && 
	      type == SPELL_GAS_BREATH) 
      {
	 right_protection = TRUE;            
      } 
      else if( affected_by_spell(v,SPELL_PROT_BREATH_FROST) && 
	      type == SPELL_FROST_BREATH) 
      {
	 right_protection = TRUE;            
      } 
      else if( affected_by_spell(v,SPELL_PROT_BREATH_ACID) && 
	      type == SPELL_ACID_BREATH) 
      {
	 right_protection = TRUE;            
      } 
      else if( affected_by_spell(v,SPELL_PROT_BREATH_ELEC) && 
	      type == SPELL_LIGHTNING_BREATH) 
      {
	 right_protection = TRUE;            
      }
      
      if (right_protection) 
      {
	 act( "$N sorride come parte del soffio e` deviato dal suo globo "
	     "protettivo!", FALSE, ch, 0, v, TO_NOTVICT);
	 act( "Il globo protettivo di $N devia parte del tuo soffio!", FALSE, ch, 
	     0, v, TO_CHAR);
	 act( "Il tuo globo protettivo devia parte del soffio di $n!", FALSE, ch, 
	     0, v, TO_VICT);
	 dam = (int)dam/4;  /* 1/4 half damage */
      }
      
   } /* else non-breath type hit/spell */
   else 
   {
      if( affected_by_spell( v, SPELL_ANTI_MAGIC_SHELL ) && 
	 IsMagicSpell( type ) )
      {
	 sprintf( buf,"$N ridacchia come il %s da $n muore sul suo globo "
		 "anti-magia!", spells[ type - 1 ] );
	 act( buf, FALSE, ch, 0, v, TO_NOTVICT );
	 sprintf( buf,"Il globo anti-magia di $N ferma il tuo %s", 
		 spells[ type - 1 ] ); 
	 act( buf, FALSE, ch, 0, v, TO_CHAR);
	 sprintf( buf,"Il tuo globo anti-magia ferma il %s di $n!",
		 spells[ type - 1 ] );
	 act( buf, FALSE, ch, 0, v, TO_VICT);
	 dam = -1;
      } 
      else if( affected_by_spell( v, SPELL_GLOBE_MINOR_INV ) && 
	      type < TYPE_HIT && spell_info[ type ].min_level_magic < 6
	      && spell_info[ type ].spell_pointer) // SALVO se torna 0 non sono nella spell_list.h 
      {
	 /* minor globe check here immune to level 1-5 and below magic user spells */
	 sprintf( buf,"$N snickers as the %s from $n fizzles on $S globe!",spells[type-1]);
	 act( buf, FALSE, ch, 0, v, TO_NOTVICT );
	 sprintf( buf,"$N's globes deflects your %s", spells[ type - 1 ] ); 
	 act(buf, FALSE, ch, 0, v, TO_CHAR);
	 sprintf(buf,"Your globe deflects the %s from $n!",spells[type-1]);
	 act(buf, FALSE, ch, 0, v, TO_VICT);
	 dam = -1;
      }
      
      /* major globe immune to level 5-10 magic user spells       */
      if( affected_by_spell(v,SPELL_GLOBE_MAJOR_INV) && type < TYPE_HIT && 
	 spell_info[type].min_level_magic < 11
      && spell_info[ type ].spell_pointer) // SALVO se torna 0 non sono nella spell_list.h 
      {
	 
	 sprintf(buf,"$N laughs as the %s from $n bounces off $S globe!",
		 spells[type-1]);
	 act(buf, FALSE, ch, 0, v, TO_NOTVICT);
	 sprintf(buf,"$N's globes completely deflects your %s",spells[type-1]); 
	 act(buf, FALSE, ch, 0, v, TO_CHAR);
	 sprintf(buf,"Your globe completely deflects the %s from $n!",
		 spells[type-1]);
	 act(buf, FALSE, ch, 0, v, TO_VICT);
	 dam = -1;
      }
   } /* was not breath spell */
   
   if( dam > -1 )
   {
      dam = WeaponCheck(ch, v, type, dam);
      
      DamageStuff( v, type, dam, location );
      
      dam=MAX(dam,0);
      
      
      /*
       *  check if this hit will send the target over the edge to -hits
       */
      if( GET_HIT( v ) - dam < 1) 
      {
	 if( IS_AFFECTED( v, AFF_LIFE_PROT ) )
	 {
	    BreakLifeSaverObj(v);
	    dam = 0;
	    GET_HIT(v) = GET_MAX_HIT(v);
	    REMOVE_BIT( ch->specials.affected_by, AFF_LIFE_PROT );
	    if (number(1,55)>GetMaxLevel(v))
	    {
	       act("La mano di Alar scende su di te e"
		   " ti allontana dalla mischia",FALSE,
		   v,0,0,TO_CHAR);
	       act("La mano di Alar scende su $n e"
		   " l$b allontana dalla mischia",FALSE,
		   v,0,0,TO_ROOM);
	       stop_fighting(v);
	       char_from_room(v);
	       char_to_room(v,GET_HOME(v));
	       do_look(v,"room",CMD_LOOK);
	    }
	 }
	 
	 if( MOUNTED( v ) ) 
	 {
	    if( !RideCheck( v, -( dam / 2 ) ) ) 
	    {
	       FallOffMount( v, MOUNTED( v ) );
	       WAIT_STATE( v, PULSE_VIOLENCE * 2 ); // mount
	       Dismount( v, MOUNTED( v ), POSITION_SITTING );
	    }
	 } 
	 else if (RIDDEN(v)) 
	 {
	    if( !RideCheck( RIDDEN( v ), -dam ) ) 
	    {
	       FallOffMount( RIDDEN( v ), v );
	       WAIT_STATE( RIDDEN( v ), PULSE_VIOLENCE * 2 ); // mount
	       Dismount( RIDDEN( v ), v, POSITION_SITTING );
	    }
	 }
      }
   }
   
   return( dam );
}

DamageResult DoDamage( struct char_data *ch, struct char_data *v, int dam, 
		      int type, int location)
{
   
   if (dam >= 0) 
   {
      GET_HIT(v) -=dam; 
      alter_hit(v,0);
      
      if( type >= TYPE_HIT && type < TYPE_SUFFERING && ch != v )
      {
	 if( IS_AFFECTED( v, AFF_FIRESHIELD ) && 
	    !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
	 {
	    damage( v, ch, dam, SPELL_FIREBALL, location );
	 }
      }
      
      update_pos( v );
      
      /* Nel caso qui sotto, il soggetto e` stato ucciso dal fireshield, 
       * a meno che, ovviamente, ch non sia uguale a victim.
       * */
      if( ch != v && GET_POS( ch ) == POSITION_DEAD )
      return SubjectDead;
   } 
   
   return AllLiving;
}


void DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype, int location)
{
   int nr, max_hit, i, j;
   struct message_type *messages;
   char buf[500];
   
   /* filter out kicks, hard coded in do_kick */
   if (attacktype == SKILL_KICK)
   {
   return; 
   } 
   else if( attacktype >= TYPE_HIT && attacktype <= TYPE_RANGE_WEAPON ) 
   {
      dam_message( dam, ch, v, attacktype, location );
      /* do not wanna frag the bow, frag the arrow instead! */
      if( ch->equipment[ WIELD ] && attacktype != TYPE_RANGE_WEAPON )
      {
	 BrittleCheck(ch,v, dam); 
      }
   } 
   else 
   {
      
      for(i = 0; i < MAX_MESSAGES; i++) 
      {
	 if (fight_messages[i].a_type == attacktype) 
	 {
	    nr=dice(1,fight_messages[i].number_of_attacks);
	    
	    for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	    messages=messages->next;
	    
	    if (!IS_NPC(v) && (GetMaxLevel(v) > MAX_MORT))
	    {
	       act(messages->god_msg.attacker_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	       act(messages->god_msg.victim_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	       act(messages->god_msg.room_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	    }
	    else if (dam > 0) 
	    {
	       if (GET_POS(v) == POSITION_DEAD) 
	       {
		  act(messages->die_msg.attacker_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
		  act(messages->die_msg.victim_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
		  act(messages->die_msg.room_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	       }
	       else 
	       {
		  act(messages->hit_msg.attacker_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
		  act(messages->hit_msg.victim_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
		  act(messages->hit_msg.room_msg, 
		      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	       }
	    } /* dam >0 */
	    else if (dam == 0) 
	    {
	       act(messages->miss_msg.attacker_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	       act(messages->miss_msg.victim_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	       act(messages->miss_msg.room_msg, 
		   FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	    } /* dam == 0 */
	 } /* fight messages == atk type */
      }
   }
   switch (GET_POS(v)) 
   {
    case POSITION_MORTALLYW:
      act( "$n is mortally wounded, and will die soon, if not aided.", 
	  TRUE, v, 0, 0, TO_ROOM );
      send_to_char( "$c0009You are mortally wounded, and will die soon, if not "
		   "aided.\n", v );
      break;
    case POSITION_INCAP:
      act( "$n is incapacitated and will slowly die, if not aided.", 
	  TRUE, v, 0, 0, TO_ROOM );
      send_to_char( "$c0009You are incapacitated and you will slowly die, if "
		   "not aided.\n", v );
      break;
    case POSITION_STUNNED:
      act( "$n is stunned, but will probably regain consciousness again.", 
	  TRUE, v, 0, 0, TO_ROOM );
      send_to_char( "$c0009You're stunned, but you will probably regain "
		   "consciousness again.\n", v );
      break;
    case POSITION_DEAD:
      break;
      
    default:  /* >= POSITION SLEEPING */
      
      max_hit=hit_limit(v);
      
      if (dam > (max_hit/5)) 
      {
	 /* ALAR: system shock */
	 
	 act("That Really $c0010HURT!$c0007",FALSE, v, 0, 0, TO_CHAR);  
	 if (con_app[(int)GET_CON(v)].shock<number(1,120-GetMaxLevel(v))) 
	 {
	    act("$c0010La paura ti paralizza!!!$c0007",FALSE, v, 0, 0, TO_CHAR);
	    act("$c0010La paura paralizza $n!!!$c0007",FALSE, v, 0, 0, TO_ROOM);
	    stop_fighting(v);
	    WAIT_STATE(v,PULSE_VIOLENCE); // hurt
	 }
	 
      }
      if( GET_HIT( v ) < max_hit / 5 && GET_HIT( v ) > 0  && GET_HIT( v ) < 50 ) 
      {
	 act("You wish that your wounds would stop $c0010BLEEDING$c0007 so much!",
	     FALSE,v,0,0,TO_CHAR);
      }
      
      if( !IS_WAITING( v ) && 
	 ( ( IS_NPC( v ) && IS_SET( v->specials.act, ACT_WIMPY ) ) ||
	  ( !IS_NPC( v ) && IS_SET( v->specials.act, PLR_WIMPY ) 
	   && GET_HIT(v)<v->specials.WimpyLevel)))
      {
	 strcpy( buf, "flee" );
	 command_interpreter( v, buf );
      }
      if( MOUNTED( v ) ) 
      {
	 /* chance they fall off */
	 RideCheck( v, -dam / 2 );
      }
      if( RIDDEN( v ) ) 
      {
	 /* chance the rider falls off */
	 RideCheck( RIDDEN( v ), -dam );
      }
      break;
   }
}


int DamageEpilog( struct char_data *ch, struct char_data *victim, 
		 int killedbytype, int dam)
{
   int exp;
   char buf[256];
   struct room_data *rp;
   
   extern char DestroyedItems;
   
   if( IS_LINKDEAD( victim ) )
   {
      if( GET_POS( victim ) != POSITION_DEAD )
      {
	 do_flee(victim,"\0",0);
	 return(FALSE);
      }
      else 
      {
	 die( victim, killedbytype, ch );
	 return(TRUE);
      }
   }
   
   if( !AWAKE( victim ) )
   if( victim->specials.fighting )
   stop_fighting( victim );
   
   if( GET_POS( victim ) == POSITION_DEAD )
   {
      /* special for no-death rooms */
      if( ( rp = real_roomp( victim->in_room ) ) )
      {
	 if( rp->funct )
	 if( ( rp->funct )( victim, 0, "", rp, EVENT_DEATH ) )
	 return TRUE;
	 
	 if( IS_SET( rp->room_flags, NO_DEATH ) ) 
	 {
	    GET_HIT(victim) = GET_MAX_HIT( victim ) / 2; alter_hit(victim,0);
	    GET_POS(victim) = POSITION_STANDING;
	    if( !special( victim, CMD_FLEE, "" ) )
	    {
	       act( "La magia di questa stanza impedisce a $n di morire e $d infonde "
		   "nuova energia.", FALSE, victim, NULL, NULL, TO_ROOM );
	       act( "La magia di questa stanza ti impedisce di morire e ti infonde "
		   "nuova energia.", FALSE, victim, NULL, NULL, TO_CHAR );
	       do_flee( victim, "", 0 );
	    }
	    return(TRUE);
	 }
	 act("$c0015$n is dead! $c0011R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
	 send_to_char("$c0008You are dead!  Sorry...\n", victim );
      }
      
      
      if( ch->specials.fighting == victim )
      stop_fighting( ch );
      if( IS_NPC( victim ) && !IS_SET( victim->specials.act, ACT_POLYSELF ) &&
	 victim != ch )
      {
	 if( IS_AFFECTED( ch, AFF_GROUP ) ) 
	 {
	    group_gain( ch, victim );
	 } 
	 else 
	 {
	    /* Calculate level-difference bonus */
	    exp = GET_EXP(victim);
	    
	    exp = MAX(exp, 1);
	    
	    if( !IS_PC( victim ) ) 
	    { 
	       exp = RatioExp( ch, victim, exp );
	       exp = ExpCaps( ch, 0, exp );  /* bug fix for non_grouped peoples */
	       
	       if( !IS_IMMORTAL( ch ) )
	       {
		  sprintf( buf,"La tua esperienza e` aumentata di %d punti.", 
			  exp );
		  act( buf, FALSE, ch, 0, 0, TO_CHAR );
	       }
	       gain_exp(ch, exp);
	    }
	    change_alignment(ch, victim);
	 }
      }


      if( IS_PC( victim ) ) 
      {
	 if( victim == ch )
	 {
	    switch( killedbytype )
	    {
	     case SPELL_POISON:
	       sprintf( buf, "%s e` stat%s uccis%s dal veleno a %s\n\r",
		       GET_NAME(victim), SSLF( victim ), SSLF( victim ),
		       victim->in_room > -1 ? 
		       (real_roomp(victim->in_room))->name : "Nowere" );
	       break;
	     case TYPE_SUFFERING:
	       sprintf( buf, "%s e` mort%s di sofferenza.\n\r",
		       GET_NAME(victim), SSLF( victim ) );
	       break;
	     case SPELL_HEAT_STUFF:
	       sprintf( buf, "%s e` mort%s bruciat%s dal suo equipaggiamento.\n\r",
		       GET_NAME(victim), SSLF( victim ), SSLF( victim ) );
	       break;
	     case SPELL_CHANGE_FORM: /* ACIDUS shock da change form */
	       sprintf( buf, "%s e` stat%s uccis%s dal trauma della mutazione.\n\r",
		       GET_NAME(victim), SSLF( victim ), SSLF( victim ) );
	       break;
	     default:
	       sprintf( buf, "%s e` mort%s\n\r", GET_NAME(victim), 
		       SSLF( victim ) );
	    }
	    
	    /* global death messages */
	    send_to_all(buf);
	 }
	 else /* victim != ch */
	 {
	    if( !IS_PC( ch ) )
	    {
	       /* killed by npc */
	       
	       if (IS_MURDER(victim)) 
	       {
		  REMOVE_BIT(victim->player.user_flags,MURDER_1);
	       }
	       
	       /* same here, with stole */
	       if (IS_STEALER(victim)) 
	       {
		  REMOVE_BIT(victim->player.user_flags,STOLE_1);
	       }
	       sprintf( buf, "%s e` stat%s uccis%s da %s a %s\n\r",
		       GET_NAME(victim), SSLF( victim ), SSLF( victim ),
		       ch->player.short_descr,
		       victim->in_room > -1 ?
		       (real_roomp(victim->in_room))->name : "Nowhere" );
	       
	       /* global death messages */
	    }
	    else 
	    {
	       /* killed by PC */
	       if( IS_PC(victim) )
	       {
		  if( !IS_IMMORTAL(victim) )
		  {
		     SET_BIT(ch->player.user_flags,MURDER_1);
		     mudlog( LOG_PLAYERS, "Setting MURDER bit on %s for killing %s.",
			    GET_NAME(ch),GET_NAME(victim));
		     
		  }
	       }
	       
	       sprintf( buf, "%s e` stat%s uccis%s da %s a %s\n\r",
		       GET_NAME(victim), SSLF( victim ), SSLF( victim ),
		       GET_NAME(ch),victim->in_room > -1 ?
		       (real_roomp(victim->in_room))->name : "Nowhere" );
	    }
	    send_to_all(buf);
	 }
	 mudlog( LOG_PLAYERS, buf );
      }
      else /* victim is not pc */
      {
	 if( IS_PC( ch ) )
	 {
	    mudlog( LOG_PLAYERS, "%s ha ucciso %s", GET_NAME( ch ), 
		   GET_NAME_DESC( victim ) );
	 }
      }
      die( victim, killedbytype, ch );
      /*
       *  if the victim is dead, return TRUE.
       */
      victim = 0;
      return( TRUE );
   } /* Fine caso morte victim */
   else 
   {
      /* ALAR Modificato per regenerate dei troll */
      switch(GET_RACE(victim))
      {
       case RACE_TROLL:
	 switch(killedbytype)
	 {
	  case TYPE_GENERIC_FIRE:
	  case TYPE_GENERIC_ACID:
	  case TYPE_SUFFERING:
	  case SPELL_BURNING_HANDS:
	  case SPELL_FIREBALL:
	  case SPELL_POISON:
	  case SPELL_FLAMESTRIKE:
	  case SPELL_ACID_BLAST:
	  case SPELL_FIRESTORM:
	  case SKILL_FLAME_SHROUD:
	  case SPELL_FIRE_BREATH:
	  case SPELL_ACID_BREATH:
	    break;
	  default:
	    GET_HIT(victim)+=MIN((con_app[(int)GET_CON(victim)].hitp+ number(0,GetMaxLevel(ch))), dam/2); alter_hit(victim,0);
	    if (dam>0)
	    {
	       act("You regenerate!",TRUE,victim,0,ch,TO_CHAR);
	       act("$n regenerates!",TRUE,victim,0,ch,TO_ROOM);
	    }
	    break;
	 }
       default:
	 break;
      }
      
      if (DestroyedItems) 
      {
	 if (check_falling(victim)) /* 0 = ok, 1 = dead */
	 return(TRUE);
	 DestroyedItems = 0;
      }
      return(FALSE);
   }
}

DamageResult MissileDamage( struct char_data *ch, struct char_data *victim,
			   int dam, int attacktype, int location )
{
   if (DamCheckDeny(ch, victim, attacktype))
   return AllLiving;
   
   dam = SkipImmortals(victim, dam, attacktype);
   
   if( !DamDetailsOk( ch, victim, dam, attacktype ) )
   return AllLiving;
   
   SetVictFighting(ch, victim);
   /*
    * make the ch hate the loser who used a missile attack on them.
    */
   if( !IS_PC( victim ) )
   {
      if( !Hates( victim, ch ) )
      {
	 AddHated( victim, ch );
      }
   }
   dam = DamageTrivia( ch, victim, dam, attacktype, location );
   
   if( DoDamage( ch, victim, dam, attacktype, location ) == SubjectDead )
   return SubjectDead;
   
   DamageMessages( ch, victim, dam, attacktype, location );
   
   if( DamageEpilog( ch, victim,attacktype,dam ) ) 
   return VictimDead;
   
   return AllLiving;  /* not dead */
   
}

DamageResult damage( struct char_data *ch, struct char_data *victim,
		    int dam, int attacktype, int location )
{
   
   if( DamCheckDeny( ch, victim, attacktype ) )
   return AllLiving;
   
   dam = SkipImmortals( victim, dam, attacktype );
   
   if( !DamDetailsOk( ch, victim, dam, attacktype ) )
   return AllLiving;

   if( attacktype != TYPE_RANGE_WEAPON && attacktype != SPELL_GREEN_SLIME &&
       attacktype != SPELL_CHANGE_FORM &&   /* Acidus shock da change_form */
      ( attacktype < TYPE_GENERIC_FIRST || attacktype > TYPE_GENERIC_LAST ) )
   { /*this ain't smart, pc's wielding bows? */

      /* ACIDUS modifica per berserk in gruppo:
         ch e vict si attaccano se ch non è in berserk oppure se non è in gruppo
         con vict */
      if ( !IS_PC(ch) || !IS_SET(ch->specials.affected_by2, AFF2_BERSERK) ||
           ( (ch->master==NULL) && (victim->master==NULL) ) ||
           !( (victim->master==ch) || (victim==ch->master) || (victim->master==ch->master) )
         )
      {
        SetVictFighting( ch, victim );
        SetCharFighting( ch, victim );
      }
   }
   
   dam = DamageTrivia( ch, victim, dam, attacktype, location );
   /* Ecco la riga responsabile del raddoppio del danno.
    * Meno male che c'e' papa' Alar che di sabato corregge il mud  :-))) 
    * */
/*   GET_HIT(victim) -=dam; alter_hit(victim,0);*/

   if( DoDamage( ch, victim, dam, attacktype, location ) == SubjectDead )
   return SubjectDead;
   
   DamageMessages( ch, victim, dam, attacktype, location );
   
   if( DamageEpilog( ch, victim, attacktype, dam ) ) 
   return VictimDead;
   
   return AllLiving;  /* not dead */
}



int GetWeaponType(struct char_data *ch, struct obj_data **wielded) 
{
   int w_type;
   
   if (ch->equipment[WIELD] &&
       (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON) ) {
	  
	  *wielded = ch->equipment[WIELD];
	  w_type = Getw_type(*wielded);
	  
       }        else {
	  if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
	  w_type = ch->specials.attack_type;
	  else
	  w_type = TYPE_HIT;
	  
	  *wielded = 0;  /* no weapon */
	  
       }
   return(w_type);
   
}

int Getw_type(struct obj_data *wielded) 
{
   int w_type;
   
   
   switch (wielded->obj_flags.value[3]) {
    case 0  : w_type = TYPE_SMITE; break;
    case 1  : w_type = TYPE_STAB;  break;
    case 2  : w_type = TYPE_WHIP; break;
    case 3  : w_type = TYPE_SLASH; break;
    case 4  : w_type = TYPE_SMASH; break;
    case 5  : w_type = TYPE_CLEAVE; break;
    case 6  : w_type = TYPE_CRUSH; break;
    case 7  : w_type = TYPE_BLUDGEON; break;
    case 8  : w_type = TYPE_CLAW; break;
    case 9  : w_type = TYPE_BITE; break;
    case 10 : w_type = TYPE_STING; break;
    case 11 : w_type = TYPE_PIERCE; break;
    case 12 : w_type = TYPE_BLAST; break;
    case 13 : w_type = TYPE_RANGE_WEAPON; break;
    default : w_type = TYPE_HIT; break;
   }
   return(w_type);
}

int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type, 
		 int DistanceWeapon)
{
   struct room_data *rp;
   extern char PeacefulWorks;
   
   rp = real_roomp(ch->in_room);
   if (rp && rp->room_flags&PEACEFUL && PeacefulWorks) 
   {
      mudlog( LOG_SYSERR, "hit() called in PEACEFUL room");
      stop_fighting(ch);
      return(TRUE);
   }
   
   if ((ch->in_room != victim->in_room) && !DistanceWeapon) 
   {
      mudlog( LOG_SYSERR, "NOT in same room when fighting : %s, %s", 
	     ch->player.name, victim->player.name );
      stop_fighting(ch);
      return(TRUE);
   }
   
#if PREVENT_PKILL
   /* this should help stop pkills */
   if(!IS_IMMORTAL(ch))
   if( ch != victim && !CanFightEachOther( ch, victim ) ) 
   {
      mudlog( LOG_PLAYERS, "%s was found fighting %s!", GET_NAME(ch),
	     GET_NAME( victim ) );
      act( "Provi un profondo senso di vergogna e scappi imbarazzato!", FALSE,
	  ch, NULL, victim, TO_CHAR);
      act( "$n sembra quasi attaccarti, ma all'improvviso cambia espressione "
	  "e scappa!", FALSE, ch, NULL, victim, TO_VICT );
      do_flee( ch, "", 999 );
      stop_fighting(ch);
      return(TRUE);
   }
#endif
   
   
   if (GET_MOVE(ch) < -10) 
   {
      send_to_char("You're too exhausted to fight\n\r",ch);
      stop_fighting(ch);
      return(TRUE);
   }
   
   
   if (victim->attackers >= 6 && ch->specials.fighting != victim) 
   {
      send_to_char("You can't attack them,  no room!\n\r", ch);
      return(TRUE);
   }
   
   /*
    *  if the character is already fighting several opponents, and he wants
    *  to hit someone who is not currently attacking him, then deny them.
    *  if he is already attacking that person, he can continue, even if they
    *  stop fighting him.
    */  
   if ((ch->attackers >= 6) && (victim->specials.fighting != ch) &&
       ch->specials.fighting != victim) 
   {
      send_to_char("There are too many other people in the way.\n\r", ch);
      return(TRUE);
   }
   
#if 1
   /* forces mob/pc to flee if person fighting cuts link */
   if (!IS_PC(ch)) 
   {
      if( ch->specials.fighting && IS_PC(ch->specials.fighting) &&
	 !ch->specials.fighting->desc) 
      {
	 do_flee(ch,"\0",0);
	 return(TRUE);
      }
   }
#endif
   
   /* force link dead persons to flee from all battles */
   if (IS_LINKDEAD(victim) && (victim->specials.fighting)) 
   {
      do_flee(victim,"",0);
   }
   
   if( IS_LINKDEAD(ch) && ch->specials.fighting ) 
   {
      do_flee(ch,"",0);
   }
   /* end link dead flees */    
   
#if 1
   
   if (IS_LINKDEAD(ch)) 
   {
      return(TRUE);
   }
   
#endif
   
   if (victim == ch) 
   {
      if (Hates(ch,victim)) 
      {
	 RemHated(ch, victim);
      }
      return(TRUE);
   }
   
   if (GET_POS(victim) == POSITION_DEAD)
   return(TRUE);
   
   if (MOUNTED(ch)) 
   {
      if (!RideCheck(ch, -5)) 
      {
	 FallOffMount(ch, MOUNTED(ch));
	 Dismount(ch, MOUNTED(ch), POSITION_SITTING);
	 return(TRUE);
      }
   } 
   else 
   {
      if (RIDDEN(ch)) 
      {
	 if (!RideCheck(RIDDEN(ch),-10)) 
	 {
	    FallOffMount(RIDDEN(ch), ch);
	    Dismount(RIDDEN(ch), ch, POSITION_SITTING);
	    return(TRUE);
	 }
      }
   }
   return(FALSE);
}

int CalcThaco(struct char_data *ch, struct char_data *victim)
{  
   int calc_thaco;
   extern struct str_app_type str_app[];
   
   /* Calculate the raw armor including magic armor */
   /* The lower AC, the better                      */
   
   if (!IS_NPC(ch))
   calc_thaco = thaco[ BestFightingClass( ch ) ]
   [ (int)GET_LEVEL( ch, BestFightingClass( ch ) ) ];
   else
   /* THAC0 for monsters is set in the HitRoll */
   calc_thaco = 20;
   
   /*  Drow are -4 to hit during daylight or lighted rooms. */
   if( !IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DROW && IS_PC(ch)
      && !affected_by_spell(ch,SPELL_GLOBE_DARKNESS) && !IS_UNDERGROUND(ch))
   {
      calc_thaco += 4;
   }
   
   if( IS_SET(ch->specials.affected_by2,AFF2_BERSERK) && IS_PC(ch) )
   {
      calc_thaco += berserkthaco(ch);
   }
   
   /* you get -4 to hit a mob if your evil and he has */
   /* prot from evil */
   if( victim )
   {
      if( IS_AFFECTED( victim, SPELL_PROTECT_FROM_EVIL ) &&
	 IS_EVIL( ch ) )
      {
	 calc_thaco += 4;
      }
   }
   
   calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
   calc_thaco -= GET_HITROLL(ch);
   calc_thaco += GET_COND(ch, DRUNK)/5;

  /* Added by Gaia for tired and wounded PCS 
     Not applied to MONK, PSI and berserk
     see also damage routine */

   if( IS_SET(ch->specials.affected_by2,AFF2_BERSERK) ) 
   {
     calc_thaco -= 1 - (int)(10*(GET_MAX_HIT(ch) - GET_HIT(ch))/GET_MAX_HIT(ch)) ;
   }
     else if (!HasClass(ch,CLASS_MONK | CLASS_PSI ) || IS_NPC( ch ) ) 
   {
     calc_thaco += (int)(GET_MOVE(ch) - GET_MAX_MOVE(ch))/20 + 1 ;
     calc_thaco -= 1 - (int)(10*(GET_MAX_HIT(ch) - GET_HIT(ch))/GET_MAX_HIT(ch)) ;
   }
   

#if defined (GGDEBUG)
   mudlog(LOG_CHECK,"%s ha thaco = %d",GET_NAME(ch),calc_thaco);
#endif
   return(calc_thaco);
}

int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco)

{
   int diceroll, victim_ac;
   extern struct dex_app_type dex_app[];
   struct obj_data *wielded=0;  /* this is rather important. */
   long indice = 0;

   GetWeaponType(ch, &wielded);
   diceroll = number(1,20);
   /* L'affected blind influenza il to hit SEMPRE */
   if (IS_AFFECTED(ch,AFF_BLIND) && !affected_by_spell(ch,SPELL_BLINDNESS))
   {
      diceroll-=5;
   }
   
   if (wielded) 
   {
      indice=wielded->item_number;
      if ((indice > -1) && obj_index[indice].func && 
	  !strcasecmp(obj_index[indice].specname,"ModHit")
	  )
      {
	 diceroll+=( (*obj_index[indice].func)
		    (ch, indice, "", wielded, EVENT_FIGHTING) );
      }
   }
   
   victim_ac  = GET_AC(victim)/10;
   
   if (!AWAKE(victim))
   victim_ac -= dex_app[ (int)GET_DEX(victim) ].defensive;
   
   victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */
#if defined (GGDEBUG)   
   mudlog(LOG_CHECK,"%s thaco %d - hit %d = %d vs armor %d",
	  GET_NAME(ch),calc_thaco,diceroll,calc_thaco-diceroll,victim_ac);
#endif
   if( diceroll < 20 && AWAKE(victim) &&
      ( diceroll == 1 || (calc_thaco - diceroll) > victim_ac ) )
   {
      return(FALSE);
   }
   else
   {
      return(!CheckMirror(victim));
   }
}

DamageResult MissVictim( struct char_data *ch, struct char_data *v, int type,
			int w_type,
			DamageResult (*dam_func)( struct char_data *,
						 struct char_data *, int,
						 int, int ), int location)
{
   struct obj_data *o;
   
   if( type <= 0 )
   type = w_type;
   
   if (dam_func == MissileDamage)
   {
      if (ch->equipment[WIELD])
      {
	 o = unequip_char(ch, WIELD);
	 if (o)
	 {
	    act( "$p cade a terra senza far danno.", FALSE, ch, o, 0, TO_CHAR);
	    act( "$p cade a terra senza far danno.", FALSE, ch, o, 0, TO_ROOM);
	    obj_to_room(o, ch->in_room);
	 }
      }
   }
   return (*dam_func)(ch, v, 0, w_type, location);
}

int GetWeaponDam(struct char_data *ch, struct char_data *v, 
		 struct obj_data *wielded, int location )
{
   int dam, j;
   struct obj_data *obj;
   extern struct str_app_type str_app[];
   long indice=0;
   
   dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
   dam += GET_DAMROLL(ch);
 
   /* Modifico il danno se uno e' affaticato o ferito Gaia 2000 */

   if( IS_SET(ch->specials.affected_by2,AFF2_BERSERK) )
     {
       dam += (int)(10*(GET_MAX_HIT(ch)-GET_HIT(ch))/GET_MAX_HIT(ch)) ;        
     }
   else if (!HasClass(ch,CLASS_MONK | CLASS_PSI ) || IS_NPC( ch ))
     {
       dam += (int)((GET_MOVE(ch) - GET_MAX_MOVE(ch))/30) + 1 ;
       dam -= (int)(8*(GET_MAX_HIT(ch)-GET_HIT(ch))/GET_MAX_HIT(ch)) + 1 ;
     }
                                                                               
 
   if( !wielded )
   {
      if (IS_NPC(ch) || HasClass(ch, CLASS_MONK ))
      dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
      else
      dam += number(0,2);  /* Max. 2 dam with bare hands */
   }
   else
   {
      indice=wielded->item_number;
      if( wielded->obj_flags.value[2] > 0 )
      {
#if 0      
	 if( IS_SET( wielded->obj_flags.value[0], WSPEC_FOR_GIANTS ) )
	 { 

	 }
	 
#endif
	 {
	    if ((indice>-1) && obj_index[indice].func && 
		!strcasecmp(obj_index[indice].specname,"TrueDam")) 
	    {
	       dam+=( (*obj_index[indice].func)
		     (ch, indice, "", wielded, EVENT_FIGHTING) );
	    }
	    else 
	    {
	       dam += dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]);
            }
	    
	 }
      }
      else 
      {
	 act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_CHAR);
	 act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_ROOM);
	 if ((obj = unequip_char(ch, WIELD))!=NULL)
	 {
	    MakeScrap(ch,v, obj);
	    dam += 1;
	 }
      }
      /* aarcerak bug fix..get_str(ch) can't be used because of additional str */
      if( wielded->obj_flags.weight > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
      {
	 if (ch->equipment[HOLD])
	 {
	    /*
	     * its too heavy to wield properly
	     */
	    dam /= 2;
	 }
      }
      
      /*
       * check for the various APPLY_RACE_SLAYER and APPLY_ALIGN_SLAYR
       * here.
       */
      
      
      for(j=0; j<MAX_OBJ_AFFECT; j++)
      {
	 if (wielded->affected[j].location == APPLY_RACE_SLAYER)
	 {
	    if (wielded->affected[j].modifier == GET_RACE(v))
	    {
	       mudlog( LOG_CHECK, "race slay X2");
	       dam *= 2;
	    }
	 }
	 
	 if (wielded->affected[j].location ==  APPLY_ALIGN_SLAYER)
	 {
	    if( IS_SET( wielded->affected[j].modifier, SLAYER_GOOD ) )
	    {
	       if (IS_GOOD(v))
	       {
		  mudlog( LOG_CHECK, "X2 good");
		  dam *= 2;
	       }
	    }
	    
	    if( IS_SET( wielded->affected[j].modifier, SLAYER_NEUTRAL ) )
	    {
	       if( !IS_EVIL( v ) && !IS_GOOD( v ) )
	       {
		  mudlog( LOG_CHECK, "X2 neutral");
		  dam *= 2;
	       }
	    }
	    
	    if( IS_SET( wielded->affected[j].modifier, SLAYER_EVIL ) )
	    {
	       if( IS_EVIL( v ) )
	       {
		  mudlog( LOG_CHECK, "X2 evil");
		  dam *= 2;
	       }
	    }
	 }
      }
   }
   
   if (GET_POS(v) < POSITION_FIGHTING)
   dam *= 1+(POSITION_FIGHTING-GET_POS(v))/3;
   /* Position  sitting  x 1.33 */
   /* Position  resting  x 1.66 */
   /* Position  sleeping x 2.00 */
   /* Position  stunned  x 2.33 */
   /* Position  incap    x 2.66 */
   /* Position  mortally x 3.00 */
   
   if (GET_POS(v) <= POSITION_DEAD)
   return(0);

#if defined (GGDEBUG)
   mudlog(LOG_CHECK,"%s damage (prima di AC mod) = %d",GET_NAME(ch),dam);
#endif

  /* GAIA 2000 Parte che controlla che pezzo di Eq e' a proteggere
     la parte colpita e diminuisce o aumenta il danno 
     di conseguenza da +4 a -4. 
     Nel caso dei mobs non viene guardata
     la zona colpita ma semplicemente la AC complessiva

     Se la locazione e' il braccio sinistro viene colpito lo scudo
     in questo caso la protezione e' elevata (AC*2), ma in compenso
     lo scudo si danneggia facilmente (vedi DamageStuff) 

     Se un ranger para in dual wield viene protetto dall'arma

     Gaia (7/2000) */

    if (IS_NPC(v) || !HasHands(v)) 
     {
       dam = dam + (int)( GET_AC(v)/25 );
     }

    else

     {

      if (location <= 2) { location = 5 ; }

     if (DUAL_WIELD(v) && HasClass(v, CLASS_RANGER) && location == 17 ) {
        dam = dam - (int)( v->equipment[location]->obj_flags.value[2] * 2.5 );
        }
      else if
      ( v->equipment[location] && ITEM_TYPE( v->equipment[location] ) == ITEM_ARMOR )
       { 
           if ( location == 11 ) {
            dam = dam - ( v->equipment[location]->obj_flags.value[0] * 3 );
            } else {
            dam = dam - ( v->equipment[location]->obj_flags.value[0] - 3 );
            }
       }
       else 
       {
        dam = dam + 5 ;
       }
     }

  /* da qui in poi e' possibile facilmente introdurre
     special effects a seconda delle parti colpite */
   
   dam = MAX(1, dam);  /* Not less than 0 damage */

#if defined (GGDEBUG)
   mudlog(LOG_CHECK,"%s danno= %d",GET_NAME(ch),dam);
#endif
   return(dam);
}

int LoreBackstabBonus(struct char_data *ch, struct char_data *v)
{
   int mult = 0;
   int learn=0;
   
   if (IsAnimal(v) && ch->skills[SKILL_CONS_ANIMAL].learned) {
      learn = ch->skills[SKILL_CONS_ANIMAL].learned;
   }
   if (IsVeggie(v) && ch->skills[SKILL_CONS_VEGGIE].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
   }
   if (IsDiabolic(v) && ch->skills[SKILL_CONS_DEMON].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
   }
   if (IsReptile(v) && ch->skills[SKILL_CONS_REPTILE].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
   }
   if (IsUndead(v) && ch->skills[SKILL_CONS_UNDEAD].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
   }  
   if (IsGiantish(v)&& ch->skills[SKILL_CONS_GIANT].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
   }
   if (IsPerson(v) && ch->skills[SKILL_CONS_PEOPLE].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
   }
   if (IsOther(v)&& ch->skills[SKILL_CONS_OTHER].learned) {
      learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned/2);
   }
   
   if (learn > 40)  
   mult += 1;
   if (learn > 74)
   mult += 1;
   
   if (mult > 0) 
   send_to_char("Your lore aids your attack!\n\r", ch);
   
   return(mult);
}

DamageResult HitVictim( struct char_data *ch, struct char_data *v, int dam, 
		       int type, int w_type, 
		       DamageResult (*dam_func)( struct char_data *,
						struct char_data *,
						int, int, int ), int location)
{
   extern byte backstab_mult[];
   DamageResult dead;
   
   if( type == SKILL_BACKSTAB )
   {
      int tmp;
      if( GET_LEVEL( ch, THIEF_LEVEL_IND ) )
      {
	 tmp = backstab_mult[ (int)GET_LEVEL( ch, THIEF_LEVEL_IND ) ];
	 tmp += LoreBackstabBonus( ch, v );
      }
      else 
      {
	 tmp = backstab_mult[ GetMaxLevel( ch ) ];
      }
      
      dam *= tmp;
      dead = (*dam_func)(ch, v, dam, type, location);
      
   }
   else 
   {
      /* reduce damage for dodge skill: */
      if( v->skills && v->skills[ SKILL_DODGE ].learned ) 
      {
	 if (number(1,101) <= v->skills[SKILL_DODGE].learned) 
	 {
	    dam -= number(1,3);
	    if( HasClass( v, CLASS_MONK ) )
	    MonkDodge(ch, v, &dam);
	 }
      }
      dead = (*dam_func)(ch, v, dam, w_type, location);
   }
   
   /*  if the victim survives, lets hit him with a weapon spell */
   if( dead == AllLiving )
   WeaponSpell( ch, v, 0, w_type );
   
   return dead;
}


DamageResult root_hit( struct char_data *ch, struct char_data *orig_victim, 
		      int type,
		      DamageResult (*dam_func)( struct char_data *, 
					       struct char_data *,
					       int, int, int ), 
		      int DistanceWeapon, int location )
{
   int w_type, thaco, dam ;
   struct char_data *tmp_victim, *temp, *victim ;
   struct obj_data *wielded=0;  /* this is rather important. */
  
   victim = orig_victim ;
 
   if( HitCheckDeny( ch, victim, type, DistanceWeapon ) )
   return AllLiving;

   /* Evita che uno usi il parry per far finire il move ai mobs
      se il pg e' in parry il mob non perde move 
      Gaia 7/2000 */   

   if ( !IS_SET(victim->specials.affected_by2, AFF2_PARRY) ) 
    {
   GET_MOVE(ch) -=1; alter_move(ch,0);
    }

   w_type = GetWeaponType(ch, &wielded);
   if (w_type == TYPE_HIT)
   w_type = GetFormType(ch);  /* races have different types of attack */

   /* Qui si mette un bello switch per il berserk. Un barbaro in berserk
      potrebbe sbagliare il target e beccare qualcuno in giro per 
      per la stanza Gaia 2001 */

  if( ch->attackers >= 1 && 
      IS_SET( ch->specials.affected_by2, AFF2_BERSERK ))
      {
        for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
        tmp_victim = temp ) {
          temp = tmp_victim->next_in_room;
           if ( (ch->in_room == tmp_victim->in_room) 
             && (ch != tmp_victim) 
             && CAN_SEE(ch, tmp_victim)
             && number(1,20) == 1)
           {   victim = tmp_victim ;                           
             /*  act( "$n vuole attaccare $N", FALSE, ch, 0, victim,
                   TO_ROOM ); */ }                                                                    
        }
      } 

     /* in pratica ogni volte che ci si mena con una terza persona presente
        si vede se per caso non parte una randellata fuori bersaglio Gaia 2001 */
 
   thaco = CalcThaco(ch, victim);
  
   /* Modifico la chiamata a GetWeaponDam 
      per aggiungere la HIT LOCATION Gaia 7/2000 */

   if (HitOrMiss(ch, victim, thaco))

   { 
      if ((dam = GetWeaponDam(ch, victim, wielded, location)) > 0)
      {
	 return HitVictim(ch, victim, dam, type, w_type, dam_func, location);
      }
      else
      {
	 return MissVictim(ch, victim, type, w_type, dam_func, location);
      }
   } 
   else
   {
      return MissVictim(ch, victim, type, w_type, dam_func, location);
   }
   return AllLiving;
}

DamageResult MissileHit( struct char_data *ch, struct char_data *victim, 
			int type )
{  int location ;
   location = Hit_Location( victim ) ; /* determina la locazione */    
   return root_hit( ch, victim, type, MissileDamage, TRUE, location );
}

DamageResult hit(struct char_data *ch, struct char_data *victim, 
                       int type )
{  int location ;
   location = Hit_Location( victim ) ; /* determina la locazione */ 
   return root_hit( ch, victim, type, damage, FALSE, location );
}

void PCAttacks( char_data *pChar )
{
   float fAttacks = pChar->mult_att;
   struct obj_data *pTmp = NULL;
   struct obj_data *pWeapon = NULL; // SALVO la setto NULL mi serve per dopo
   int perc;

   /* Controlla se il tipo e' in parrying, in questo caso 
      diminuisce gli attacchi di uno per ogni attacco 
      avversario. Se il tipo e' un ranger non usa il dual wield
      ma i suoi attacchi calcolati sono aumentati di uno.
      Gaia (7/2000) */

   if (IS_SET(pChar->specials.affected_by2, AFF2_PARRY))
    { 

     if (HasClass( pChar, CLASS_RANGER ) && DUAL_WIELD(pChar) )
       { fAttacks += 1 ; }

      fAttacks -= pChar->specials.fighting->mult_att; 
      if (fAttacks <= 0.0 ) 
       { 
        fAttacks = 0.0 ; 
       }
     }

   /* if dude is a monk, and is wielding something */
   
   if( HasClass( pChar, CLASS_MONK ) ) 
   {
      if( pChar->equipment[ WIELD ] || pChar->equipment[LOADED_WEAPON] )
      {
	 /* set it to one, they only get one attack */
	 fAttacks = 1.00;
      }
      
      /* have to check for monks holding things. */
      else if( pChar->equipment[ HOLD ] &&
	 ITEM_TYPE( pChar->equipment[ HOLD ] ) == ITEM_WEAPON && 
	 HasClass( pChar, CLASS_MONK ) ) 
      {
	 fAttacks = 1.01;
      }
      else if (IS_CARRYING_N(pChar) > MONK_MAX_RENT+5) 
      {
	 fAttacks = MIN((int)fAttacks,(int)2)+0.1;
      }
      
   }
   
   if( MOUNTED( pChar ) )
   {
      fAttacks /= 2.0;
   }
   /* work through all of their attacks, until there is not
    * a full attack left */
   
   
   if( DUAL_WIELD( pChar ) )
   {
      pTmp = unequip_char( pChar, HOLD );
   }
   
   
   while( fAttacks > 0.999 )
   {
      if( pChar->specials.fighting )
      {
	 if( hit( pChar, pChar->specials.fighting, 
		 TYPE_UNDEFINED ) == SubjectDead )
	 return;
      }
      else
      {
	 fAttacks = 0.0;
	 break;
      }
      fAttacks -= 1.0;
   }
#if 0          
   if(GET_RACE(pChar) == RACE_MFLAYER && pChar->specials.fighting)
   MindflayerAttack(pChar, pChar->specials.fighting);
#endif        
   if( fAttacks > .01 )
   {
#if 1

       perc = number(1,100);

      /* check to see if the chance to make the last attack
       * is successful */
      /* Added modifier for weight of wielded weapon Gaia 2001 */

      if (pChar->equipment[WIELD] &&
       ( pChar->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON) )
          {
             if( perc <= ( (int)(fAttacks * 100.0) + 10 -
                ( pChar->equipment[ WIELD ]->obj_flags.weight )*2 ) )
               {
	         if( pChar->specials.fighting )
	         if ( hit( pChar, pChar->specials.fighting, 
                  TYPE_UNDEFINED ) == SubjectDead ) 
                 return;
               }
          }
      else if ( perc <= (fAttacks * 100.0) ) 
        {
	   if( pChar->specials.fighting )
	   if ( hit( pChar, pChar->specials.fighting, 
                  TYPE_UNDEFINED ) == SubjectDead ) 
           return;
        }
#else            
      /* lets give them the hit */
      if( pChar->specials.fighting )
      if( hit( pChar, pChar->specials.fighting, 
	      TYPE_UNDEFINED ) == SubjectDead )
      return;
#endif   
   }
   
   if( pTmp )
   equip_char( pChar, pTmp, HOLD );

   /* check for the second attack */
   /* Se sei in parry non puoi usare il dual wield Gaia 7/2000 */
   if( DUAL_WIELD( pChar ) && pChar->skills
       && !IS_SET(pChar->specials.affected_by2, AFF2_PARRY) )
   {
      /* check the skill */
      if( ( perc = number( 1, 101 ) ) <
	 pChar->skills[ SKILL_DUAL_WIELD ].learned )
      {
	 /* if a success, remove the weapon in the wielded hand,
	  * place the weapon in the off hand in the wielded hand.
	  */
	 pWeapon = unequip_char( pChar, WIELD );
	 pTmp = unequip_char( pChar, HOLD );
	 equip_char( pChar, pTmp, WIELD );
	 /* adjust to_hit based on dex */
	 if( pChar->specials.fighting )
	 {
	    if( hit( pChar, pChar->specials.fighting, 
		    TYPE_UNDEFINED ) == SubjectDead )
        {
          if (pChar->equipment[WIELD]!=pTmp && pWeapon) // SALVO si e' distrutta la 2nd arma
            equip_char( pChar, pWeapon, WIELD );
	    return;
	    }
	 }
	 /* get rid of the to_hit adjustment */
	 /* put the weapons back, checking for destroyed items */
	 if( pChar->equipment[ WIELD ] )
	 {
	    pTmp = unequip_char( pChar, WIELD );
	    equip_char( pChar, pTmp, HOLD );
	    equip_char( pChar, pWeapon, WIELD );
	 }
     else if (pWeapon) // SALVO si e' distrutta la 2nd arma
	 {
       equip_char( pChar, pWeapon, WIELD );
	 }
      }
      else
      {
	 if( !HasClass( pChar, CLASS_RANGER ) || 
	    number( 1, 20 ) > GET_DEX( pChar ) )
	 {
	    pTmp = unequip_char( pChar, HOLD );
	    obj_to_room( pTmp, pChar->in_room );
	    act( "$p ti cade dalle mani", 0, pChar, pTmp, pTmp, TO_CHAR );
	    act( "$p cade dalle mani di $n", 0, pChar, pTmp, pTmp, TO_ROOM);
	    if( number( 1, 20 ) > GET_DEX( pChar ) )
	    {
	       pTmp = unequip_char( pChar, WIELD );
	       obj_to_room( pTmp, pChar->in_room );
	       act( "e ti casca anche $p!", 0, pChar, pTmp, pTmp, TO_CHAR);
	       act( "e $d casca anche $p!", 0, pChar, pTmp, pTmp, TO_ROOM);
	       if( HasClass( pChar, CLASS_RANGER ) )
	       {
		  LearnFromMistake( pChar, SKILL_DUAL_WIELD, FALSE, 95 );
	       }
	    }
	 }
      }
   }
}

void NPCAttacks( char_data *pChar )
{
   
   float fAttacks = pChar->mult_att;
   char_data *pVictim = NULL;
   
   while( fAttacks > 0.999 )
   {
      if( pChar->specials.fighting )
      {
	 if( hit( pChar, pChar->specials.fighting, 
		 TYPE_UNDEFINED ) == SubjectDead )
	 return;
      }
      else
      {
	 if( ( pVictim = FindAHatee( pChar ) ) != NULL )
	 {
	    if( pVictim->attackers < 6 )
	    if( hit(pChar, pVictim, TYPE_UNDEFINED) == SubjectDead )
	    return;
	 }
	 else if( ( pVictim = FindAnAttacker( pChar ) ) != NULL )
	 {
	    if( pVictim->attackers < 6 )
	    if( hit( pChar, pVictim, TYPE_UNDEFINED ) == SubjectDead )
	    return;
	 }
      }
      fAttacks -= 1.0;
   }
#if 0
   if(GET_RACE(pChar) == RACE_MFLAYER && pChar->specials.fighting)
   MindflayerAttack(pChar, pChar->specials.fighting);
#endif
   if( fAttacks > .01 ) 
   {
      /* check to see if the chance to make the last attack is 
       * successful */
      int iPerc = number( 1, 100 );
      if( iPerc <= (int)( fAttacks * 100.0 ) ) 
      {
	 if( pChar->specials.fighting )
	 {
	    if( hit( pChar, pChar->specials.fighting, 
		    TYPE_UNDEFINED ) == SubjectDead )
	    return;
	 }
	 else 
	 {
	    if( ( pVictim = FindAHatee( pChar ) ) != NULL )
	    {
	       if( pVictim->attackers < 6 )
	       if( hit( pChar, pVictim, TYPE_UNDEFINED ) == SubjectDead )
	       return;
	    }
	    else if( ( pVictim = FindAnAttacker( pChar ) ) != NULL ) 
	    {
	       if( pVictim->attackers < 6 )
	       if( hit( pChar, pVictim, TYPE_UNDEFINED ) == SubjectDead )
	       return;
	    }
	 } /* was not fighting */
      } /* made percent check */
   }
}

/* control the fights going on */

void perform_violence(unsigned long pulse)

{
   struct char_data *ch;
   int tdir, cmv, max_cmv, caught, rng, tdr;
   
   for( ch = combat_list; ch; ch = combat_next_dude )
   {
      struct room_data *rp;
      
      combat_next_dude = ch->next_fighting;
      
      rp = real_roomp( ch->in_room );

      if( !ch->specials.fighting )
      {  
	 mudlog( LOG_SYSERR, 
		"!ch->specials.fighting in perform violence fight.c" );
	 return;
      }
      else if( rp && rp->room_flags&PEACEFUL ) 
      {
	 mudlog( LOG_SYSERR, 
		"perform_violence() found %s fighting in a PEACEFUL room.",
		ch->player.name);
	 stop_fighting( ch );
      }
      else if( ch == ch->specials.fighting ) 
      {
	 stop_fighting( ch );
      }
      else
      {
	 if( IS_NPC( ch ) )
	 {
	    struct char_data *rec;
	    
	    DevelopHatred(ch, ch->specials.fighting);
	    rec = ch->specials.fighting;
	    if( !IS_PC( ch->specials.fighting ) )
	    {
	       while( rec->master )
	       {
		  if( rec->master->in_room == ch->in_room )
		  {
		     AddHated( ch, rec->master );
		     rec = rec->master;
		  }
		  else
		  {
		     break;
		  }
	       }
	    }
	 }
	 
	 if( AWAKE( ch ) && ch->in_room == ch->specials.fighting->in_room &&
	    !IS_AFFECTED( ch, AFF_PARALYSIS ) )
	 {
	    if( !IS_NPC( ch ) )
	    {
	       PCAttacks( ch );
	    }
	    else
	    {
	       NPCAttacks( ch );
	    }
	 }
	 else /* Not in same room or not awake */
	 {
	    stop_fighting(ch);
	 }
      }
   }
   
   char_data *pNext;
   /* charging loop */
   for( ch = character_list; ch; ch = pNext )
   {
      pNext = ch->next;
      /* If charging deal with that */
      if( ch->specials.charging )
      {
	 caught = 0;
	 max_cmv = 2;
	 cmv = 0;
	 while( cmv < max_cmv && caught == 0 )
	 {
	    if( ch->in_room == ch->specials.charging->in_room )
	    {
	       caught = 1;
	    }
	    else
	    {
	       /* Continue in a straight line */
	       if( clearpath( ch, ch->in_room, ch->specials.charge_dir ) )
	       {
		     do_move( ch, "\0", ch->specials.charge_dir + 1 ); 
MARK;        if (!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) // SALVO controllo che non sia caduto in DT
	         {
               ch = pNext;
	           break;
	         }
		     cmv++;
	       }
	       else
	       {
		     caught = 2;
	       }
	    }
	 }
	 switch( caught )
	 {
	  case 1 : /* Caught him */
	    act( "$n vede $N ed attacca!", TRUE, ch, 0, ch->specials.charging,
		TO_NOTVICT );
	    act( "$n ti vede ed attacca!", TRUE, ch, 0, ch->specials.charging,
		TO_VICT );
	    act( "Vedi $N e l$B attacchi!", TRUE, ch, 0, ch->specials.charging,
		TO_CHAR );
	    if( hit( ch, ch->specials.charging, TYPE_UNDEFINED ) != SubjectDead )
	    ch->specials.charging = NULL;
	    break;
	  case 2 : /* End of line and didn't catch him */
	    tdir = can_see_linear( ch, ch->specials.charging, &rng, &tdr );
	    if( tdir > -1 )
	    {
	       ch->specials.charge_dir = tdr; 
	    }
	    else
	    {
	       ch->specials.charging = NULL;
	       act( "$n si guarda intorno scoraggiat$b.", FALSE, ch, 0, 0, 
		   TO_ROOM );
	    }
	    break;
	  default : /* Still charging */
	    break;
	 } 
      }
   }
}

/****************************************************************************
* Questa procedura da (e restituisce) un punteggio a pNewChar in base alle
* preferenze di pChar in tema di attacco. In pratica serve per scegliere
* quale personaggio attaccare in caso di piu` personaggi in una locazione
* *************************************************************************/

int GetBonusToAttack( struct char_data *pChar, struct char_data *pNewChar )
{
   int aiClassBonus[ 4 ][ 7 ] =
   {
      /* W  C  T   N   M   K  D   */ 
      { 4, 0, 1, -2, -3,  3, 2 },
      { 4, 2, 1, -2, -1,  3, 1 },
      { 4, 3, 0, -2,  1, -1, 1 },
      { 1, 6, 0, -2,  2, -1, 2 }
   };
   int iBonus = 0, iIntIdx;
   
   
   if( IS_AFFECTED( pNewChar, AFF_FIRESHIELD ) )
   {
#if ALAR
      return -1*number(4,14);
#else
      return -4;
#endif
   }
   
   /*
    * A seconda della intelligenza, la creatura preferisce attaccare i 
    * guerrieri od i seguaci della magia (maghi e chierici).
    * 
    */
   
   if( pChar->abilities.intel <= 8 ) 
   {
      iIntIdx = 0;
   } 
   else if( pChar->abilities.intel <= 10 ) 
   {
      iIntIdx = 1;
   } 
   else if( pChar->abilities.intel <= 12 ) 
   {
      iIntIdx = 2;
   } 
   else 
   {  
      iIntIdx = 3;
   }
   
   if( !IS_NPC( pNewChar ) ) 
   {
      if( affected_by_spell( pNewChar, SKILL_DISGUISE ) ||
	 affected_by_spell( pNewChar, SKILL_PSYCHIC_IMPERSONATION ) ) 
      {
	 /* 50% chance to not attack disguised person */
	 if( number( 1, 101 ) > 50 ) 
	 return -10;
      }
      if( HasClass( pNewChar, CLASS_WARRIOR | CLASS_BARBARIAN |
		   CLASS_PALADIN | CLASS_RANGER ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 0 ];
      }
      
      if( HasClass( pNewChar, CLASS_CLERIC ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 1 ];
      }
      
      if( HasClass( pNewChar, CLASS_MAGIC_USER ) ||
	 HasClass( pNewChar, CLASS_SORCERER ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 4 ];
      }
      
      if( HasClass( pNewChar, CLASS_THIEF | CLASS_PSI ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 2 ];
      }
      
      if( HasClass( pNewChar, CLASS_DRUID ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 6 ];
      }
      
      if( HasClass( pNewChar, CLASS_MONK ) )
      {
	 iBonus += aiClassBonus[ iIntIdx ][ 5 ];
      }
   } 
   else 
   {
      iBonus += aiClassBonus[ iIntIdx ][ 3 ];
   }
   
   if( pChar->abilities.intel >= 15 )
   iBonus -= pNewChar->attackers*2;
#if !ALAR  
   if( IS_GOOD( pChar ) )
   {
      if( IS_EVIL( pNewChar ) )
      iBonus++;
      else if( IS_GOOD( pNewChar ) )
      iBonus--;
   }
   else if( IS_EVIL( pChar ) )
   {
      if( IS_NEUTRAL( pNewChar ) )
      iBonus += 1;
      else if( IS_GOOD( pChar ) )
      iBonus += 2;
   }
#else
   if( IS_GOOD( pChar ) )
   {
      if( IS_EVIL( pNewChar ) )
      iBonus+=5;
      else if( IS_GOOD( pNewChar ) )
      iBonus-=10;
   }
   else if( IS_EVIL( pChar ) )
   {
      if( IS_EVIL( pNewChar ) )
      iBonus -= 5;
      else if( IS_GOOD( pNewChar ) )
      iBonus += 10;
   }
   iBonus +=(GetMaxLevel(pChar)-GetMaxLevel(pNewChar));
   iBonus +=(int)((GET_HIT(pChar)-GET_HIT(pNewChar))/50);
#endif
/* I multiclasse vengono attaccati per primi */
   iBonus += (HowManyClasses(pNewChar)-1) * 5;
   
   return iBonus;
}

/***************************************************************************
* Questa procedura , chiamata dalle varie findvictim, switcha dalla vittima
* al suo principe se questi e` in vista.
* *************************************************************************/
struct char_data *SwitchVictimToPrince( struct char_data *pAtt, 
				       struct char_data *pVict)
{
   struct char_data *pTemp=(struct char_data *)NULL;
   struct char_data *pTemp2=(struct char_data *)NULL;
   bool isBG;
   isBG=HAS_BODYGUARD(pVict);
   PushStatus("SwitchVictimToPrince");
   if (pVict && pAtt && (HAS_PRINCE(pVict) || isBG)) 
   {
      if (isBG) 
      {
	 pTemp=get_char_room_vis(pAtt,GET_BODYGUARD(pVict));
      }
      if (!pTemp &&HAS_PRINCE(pVict)) 
      {
	 pTemp=get_char_room_vis(pAtt,GET_PRINCE(pVict));
      }
      if (pTemp &&IS_PRINCE(pTemp) &&HAS_BODYGUARD(pTemp))
      /* Prince, se con la guardia del corpo riswitcho */
      {
	    pTemp2=get_char_room_vis(pAtt,GET_BODYGUARD(pTemp));
	    if (pTemp2 && in_group_strict(pTemp,pTemp2)) pTemp=pTemp2;
      }
      if (pTemp && 
	  in_group_strict(pTemp,pVict) && 
	  number(1,19<isBG?GET_DEX(pTemp):20))
      {
	 mudlog(LOG_CHECK,"SWITCH3: %s attaccato al posto di %s",
		GET_NAME(pTemp),GET_NAME(pVict));
         act("Stavi per essere attaccat$B ma $n interviene!", 
	     FALSE, pTemp,0,pVict,TO_VICT);
         act("$N stava per essere attaccat$B ma tu intervieni!", 
	     FALSE, pTemp,0,pVict,TO_CHAR);
	 act("$N stava per essere attaccat$B ma $n interviene!",
	     FALSE, pTemp,0,pVict,TO_NOTVICT);
	 pVict=pTemp;
	 
      }
   }
   PopStatus();
   return(pVict);
}


/***************************************************************************
* Le tre procedure che seguono restituiscono il puntatore ad un personaggio
* da attaccare. La prima e` usata in genere per le creature agressive e 
* restituisce solo PC o creature che non appartengono alla stessa razza o
* zona di pChar. La seconda cerca qualunque vittima senza preoccuparsi
* se la creatura appartiene alla propria zona.
* La terza viene usata per le creature che proteggono il loro padrone o per
* quelle attaccate che devono decidere a chi fare qualcosa di cattivo.
* ************************************************************************/

struct char_data *FindVictim( struct char_data *pChar )
{
   struct char_data *pLoopChar, *pBetterChar = NULL;
   int iBonus, iBetterBonus = -1;
   
   if( pChar->in_room < 0 ) 
   return NULL;
   PushStatus("FindVictim");
   
   for( pLoopChar = ( real_roomp( pChar->in_room ) )->people; pLoopChar;
       pLoopChar = pLoopChar->next_in_room ) 
   {
      if( IS_PC( pLoopChar ) || 
	 ( ( pLoopChar->specials.zone != pChar->specials.zone &&
	    !strchr( zone_table[ pChar->specials.zone ].races, 
		    GET_RACE( pLoopChar ) ) ) ||
	  IS_SET( pLoopChar->specials.act, ACT_ANNOYING ) ) )
      {
	 if( !IS_SET( pChar->specials.act, ACT_WIMPY ) || !AWAKE( pLoopChar ) ) 
	 {
	    if( !in_group( pChar, pLoopChar ) ) 
	    {
	       
	       if( ( IS_NPC( pLoopChar ) || 
		    !IS_SET( pLoopChar->specials.act, PLR_NOHASSLE ) ) &&
		  !IS_AFFECTED( pLoopChar, AFF_SNEAK ) && pChar != pLoopChar &&
		  CAN_SEE( pChar, pLoopChar ) )
	       {
		  
		  iBonus = GetBonusToAttack( pChar, pLoopChar );
		  
		  if( pBetterChar == NULL || 
		     ( iBonus >= iBetterBonus && iBonus > -10 ) )
		  {
		     pBetterChar = pLoopChar;
		     iBetterBonus = iBonus;
		  }
	       }
	    }

	 }
      }
   }
   pBetterChar=SwitchVictimToPrince(pChar,pBetterChar);
   PopStatus();
   return(pBetterChar);
}

struct char_data *FindAnyVictim( struct char_data *pChar )
{
   struct char_data *pLoopChar, *pBetterChar = NULL;
   int iBonus, iBetterBonus = -1;
   
   if( pChar->in_room < 0 ) 
   return NULL;
   PushStatus("FindAnyVictim");
   for( pLoopChar = ( real_roomp( pChar->in_room ) )->people; pLoopChar;
       pLoopChar = pLoopChar->next_in_room ) 
   {
      if( IS_PC( pLoopChar ) || !SameRace( pChar, pLoopChar ) )
      {            
	 if( !IS_AFFECTED( pChar, AFF_CHARM ) || pChar->master != pLoopChar ) 
	 {
	    if( ( IS_NPC( pLoopChar ) || 
		 !IS_SET( pLoopChar->specials.act, PLR_NOHASSLE ) ) &&
	       CAN_SEE( pChar, pLoopChar ) )
	    {
	       iBonus = GetBonusToAttack( pChar, pLoopChar );
	       
	       if( pBetterChar == NULL || iBonus >= iBetterBonus )
	       {
		  pBetterChar = pLoopChar;
		  iBetterBonus = iBonus;
	       }
	    }
	 }
      }
   }
   pBetterChar=SwitchVictimToPrince(pChar,pBetterChar);
   PopStatus();
   return pBetterChar;
}

struct char_data *FindAnAttacker( struct char_data *pChar ) 
{
   struct char_data *pLoopChar, *pBetterChar = NULL;
   int iBonus, iBetterBonus = -1;
   
   if( pChar->in_room < 0 ) 
   return NULL;
   PushStatus("FindAnAttacker");
   for( pLoopChar = ( real_roomp( pChar->in_room ) )->people; pLoopChar;
       pLoopChar = pLoopChar->next_in_room)
   {
      if( pChar != pLoopChar && pLoopChar->specials.fighting == pChar )
      {
	 iBonus = GetBonusToAttack( pChar, pLoopChar );
	 
	 if( pBetterChar == NULL || iBonus >= iBetterBonus )
	 {
	    pBetterChar = pLoopChar;
	    iBetterBonus = iBonus;
	 }
      }
   }
   PopStatus();
   return pBetterChar;    
}

void BreakLifeSaverObj( struct char_data *ch)
{
   int found=-1, i, j;
   struct obj_data *o;
   
   /* check eq for object with the effect */
   for (i = 0; i< MAX_WEAR && (found<0); i++) 
   {
      if (ch->equipment[i]) 
      {
	 o = ch->equipment[i];
	 for (j=0; j<MAX_OBJ_AFFECT; j++) 
	 {
	    if (o->affected[j].location == APPLY_SPELL) 
	    {
	       if (IS_SET(o->affected[j].modifier,AFF_LIFE_PROT)) 
	       {
		  mudlog(LOG_CHECK,"LIFE-PROT ha salvato la pelle a %s",
			 GET_NAME(ch));
		  found = i;
	       }
	       
	    }
	 }
      }
   }
   if (found>=0) 
   {
      /* break the object. */
      act( "$c0011$p $c0007si $c0009frantuma $c0007"
	  "in un accecante lampo di luce!", TRUE, ch, 
	  ch->equipment[ found ], 0, TO_CHAR );
      if( ( o = unequip_char( ch, found ) ) != NULL )
      MakeScrap(ch,NULL, o);
   }
}

int BrittleCheck(struct char_data *ch, struct char_data *v, int dam)
{
   char buf[200];
   struct obj_data *obj;
   
   if (dam <= 0)
   return FALSE;

   if (ch->equipment[LOADED_WEAPON]) 
      {
	 if ((obj = unequip_char(ch,LOADED_WEAPON))!=NULL) 
	 {
	sprintf(buf, "%s si danneggia nel colpo.\n\r", obj->short_description);
	    send_to_char(buf, ch);
	    MakeScrap(ch,v, obj);
	    return TRUE;
	 }
      }

   if (ch->equipment[WIELD]) 
   {
      if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_BRITTLE)) 
      {
	 if ((obj = unequip_char(ch,WIELD))!=NULL) 
	 {
	    sprintf(buf, "%s si distrugge.\n\r", obj->short_description);
	    send_to_char(buf, ch);
	    MakeScrap(ch,v, obj);
	    return TRUE;
	 }
      }
   }
   return FALSE;
}

int PreProcDam(struct char_data *ch, int type, int dam, int classe)
{
   unsigned Our_Bit;
   
   /*
    * long, intricate list, with the various bits and the various spells and
    * such determined
    */
   
   switch (type) 
   {
    case SPELL_FIREBALL:
    case SPELL_BURNING_HANDS:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_HEAT_STUFF:
    case SPELL_FIRESTORM:
    case SPELL_INCENDIARY_CLOUD:
    case SKILL_MIND_BURN:
    case TYPE_GENERIC_FIRE:
      Our_Bit = IMM_FIRE;
      break;
      
    case SPELL_SHOCKING_GRASP:
    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
    case SPELL_CHAIN_LIGHTNING:
    case TYPE_GENERIC_ELEC:
      Our_Bit = IMM_ELEC;
      break;
      
    case SPELL_CHILL_TOUCH:                     
    case SPELL_CONE_OF_COLD:                     
    case SPELL_ICE_STORM:                             
    case SPELL_FROST_BREATH:
    case TYPE_GENERIC_COLD:
      Our_Bit = IMM_COLD;
      break;
      
    case SPELL_MAGIC_MISSILE:
    case SPELL_COLOUR_SPRAY:
    case SPELL_GAS_BREATH:
    case SPELL_METEOR_SWARM:
    case SPELL_SUNRAY:
    case SPELL_DISINTEGRATE:
    case TYPE_GENERIC_ENERGY:
      Our_Bit = IMM_ENERGY;
      break;
      
    case SPELL_ENERGY_DRAIN:
      Our_Bit = IMM_DRAIN;
      break;
      
    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
    case TYPE_GENERIC_ACID:
      Our_Bit = IMM_ACID;
      break;
      
    case SKILL_BACKSTAB:
    case TYPE_PIERCE:
    case TYPE_STING:
    case TYPE_STAB:
    case TYPE_RANGE_WEAPON:
      Our_Bit = IMM_PIERCE;
      break;
      
    case TYPE_SLASH:
    case TYPE_WHIP:
    case TYPE_CLEAVE:
    case TYPE_CLAW:
      Our_Bit = IMM_SLASH;
      break;
      
    case TYPE_BLUDGEON:
    case TYPE_HIT:
    case SKILL_KICK:
    case TYPE_CRUSH:
    case TYPE_BITE:
    case TYPE_SMASH:
    case TYPE_SMITE:
    case TYPE_BLAST:
      Our_Bit = IMM_BLUNT;
      break;
      
    case SPELL_POISON:
      Our_Bit = IMM_POISON;
      break;
      
    default:
      return(dam);
      break;
   }
   if (classe !=CLASS_MONK)  
   {
      if( IS_SET( ch->susc, Our_Bit ) )
      dam <<= 1;
      
      if( IS_SET( ch->immune, Our_Bit ) )
      dam >>= 1;
      
      if (classe !=CLASS_BARBARIAN) 
      {
	 if( IS_SET( ch->M_immune, Our_Bit ) )
	 dam = -1;
      }
      else 
      {
	 if( IS_SET( ch->M_immune, Our_Bit ) )
	 dam >>=1;
      }
      
   }
   
   return(dam);
}


int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj)
{
   int num;
   char buf[256];
   
   num = DamagedByAttack(obj, dam_type);
   if (num)
   {
      sprintf(buf, "%s is %s.\n\r",obj->short_description, 
	      ItemDamType[dam_type-1]);
      send_to_char(buf,ch);
      if (num == -1) {  /* destroy object*/
	 return(TRUE);
	 
      } else {   /* "damage item"  (armor), (weapon) */
	 if (DamageItem(ch, obj, num)) {
	    return(TRUE);
	 }
      }
   }
   return(FALSE);
   
}


void MakeScrap( struct char_data *ch,struct char_data *v, struct obj_data *obj)
{
   char buf[200];
   struct obj_data *t, *x;
   
   extern char DestroyedItems;
   
   act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
   act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);
   
   t = read_object(30, VIRTUAL);
   
   sprintf(buf, "Scraps from %s lie in a pile here.", 
	   obj->short_description);
   
   free(t->description);
   t->description = (char *)strdup(buf);
   if (obj->carried_by)
   {
      obj_from_char(obj);
   } 
   else if (obj->equipped_by)
   {
      obj = unequip_char(ch, obj->eq_pos);
   }
   
   if (v)
   {
      obj_to_room(t, ch->in_room);
   }
   else 
   obj_to_room(t, ch->in_room);
   
   t->obj_flags.value[0] = 20;
   
   while (obj->contains)
   {
      x = obj->contains;
      obj_from_obj(x);
      obj_to_room(x, ch->in_room);
   }
   
   
   check_falling_obj(t, ch->in_room);
   
   extract_obj(obj);
   
   DestroyedItems = 1;
   
}

void DamageAllStuff( struct char_data *ch, int dam_type)
{
   int j;
   struct obj_data *obj, *next;
   
   /* this procedure takes all of the items in equipment and inventory
    and damages the ones that should be damaged */
   
   /* equipment */
   
   for (j = 0; j < MAX_WEAR; j++) 
   {
      if (ch->equipment[j] && ch->equipment[j]->item_number>=0) 
      {
	 obj = ch->equipment[j];
	 if (DamageOneItem(ch, dam_type, obj))
	 { 
	    /* TRUE == destroyed */
	    if ((obj = unequip_char(ch,j))!=NULL)
	    {
	       MakeScrap(ch,NULL, obj);
	    }
	    else
	    {
	       mudlog( LOG_SYSERR, "hmm, really wierd in DamageAllStuff!");
	    }
	 }
      }
   }
   
   /* inventory */
   
   obj = ch->carrying;
   while (obj)
   {
      next = obj->next_content;
      if (obj->item_number >= 0)
      {
	 if (DamageOneItem(ch, dam_type, obj))
	 {
	    MakeScrap(ch,NULL, obj);
	 }
      }
      obj = next;
   }
   
}


int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
   /*  damage weapons or armor */
   
   if (ITEM_TYPE(o) == ITEM_ARMOR) {
      o->obj_flags.value[0] -= num;
      if (o->obj_flags.value[0] < 0) {
	 return(TRUE);
      }    
   } else if (ITEM_TYPE(o) == ITEM_WEAPON) {
      o->obj_flags.value[2] -= num;
      if (o->obj_flags.value[2] <= 0) {
	 return(TRUE);
      }
   }
   return(FALSE);
}

int ItemSave( struct obj_data *i, int dam_type) 
{
   int num, j;
   
   /* obj fails save automatically it brittle */ 
   if (IS_OBJ_STAT(i,ITEM_BRITTLE))
   {
      return(FALSE);
   }
   
   /* this is to keep immune objects from getting dammaged */
   if (IS_OBJ_STAT(i, ITEM_IMMUNE))
   {
      return(TRUE);
   }
   
   /* this is to give resistant magic items a better chance to save */
   if (IS_OBJ_STAT(i, ITEM_RESISTANT))
   {
      if (number(1,100)>=50) 
      return(TRUE);
   }
   
   num = number(1,20);
   /* ALAR */
   /* Inserito drastico miglioramento dei tiri salvezza..
    * solo una su 4 testa il tiro*/
   /* Rimosso per riammettere lo scrap nel mondo */
#if NOSCRAP
   if (number(0,3))
   return(TRUE);
#endif
   if (num <= 1)
   return(FALSE);
   if (num >= 20)
   return(TRUE);
   
   for(j=0; j<MAX_OBJ_AFFECT; j++)
   {
      if ((i->affected[j].location == APPLY_SAVING_SPELL) || 
	  (i->affected[j].location == APPLY_SAVE_ALL))
      {
	 num -= i->affected[j].modifier;
      }
   }
   if (ITEM_TYPE(i) != ITEM_ARMOR)
   num += 1;
   
   if (num <= 1) 
   return(FALSE);
   if (num >= 20) 
   return(TRUE);
   
   if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i)-1][dam_type-1])
   {
      return(TRUE);
   }
   else
   {
      return(FALSE);
   }
}



int DamagedByAttack( struct obj_data *i, int dam_type)
{
   int num = 0;
   
   if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)){
      while (!ItemSave(i,dam_type)) {
	 num+=1;
	 if (num > 75)
	 return(num);  /* so anything with over 75 ac points will not be
			destroyed */
      }
      return(num);
   } else {
      if (ItemSave(i, dam_type)) {
	 return(0);
      } else {
	 return(-1);
      }
   }
}

int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
   int Immunity, total, j;
   
   Immunity = -1;
   if( IS_SET( v->M_immune, IMM_NONMAG ) ) 
   {
      Immunity = 0;
   }
   if( IS_SET( v->M_immune, IMM_PLUS1 ) ) 
   {
      Immunity = 1;
   }
   if( IS_SET( v->M_immune, IMM_PLUS2 ) ) 
   {
      Immunity = 2;
   }
   if( IS_SET( v->M_immune, IMM_PLUS3 ) )
   {
      Immunity = 3;
   }
   if( IS_SET( v->M_immune, IMM_PLUS4 ) )
   {
      Immunity = 4;
   }
   
   if( Immunity < 0 )
   return( dam );
   
   if( ( type < TYPE_HIT ) || ( type > TYPE_RANGE_WEAPON ) )  
   {
      return( dam );
   } 
   else 
   {
      if( type == TYPE_HIT || IS_NPC( ch ) ) 
      {
	 if( GetMaxLevel( ch ) > ( ( Immunity+1 ) * ( Immunity+1 ) )+6 || 
	    ( HasClass( ch, CLASS_BARBARIAN )  && 
	     BarbarianToHitMagicBonus( ch ) >= Immunity ) ||
	    (HasClass(ch,CLASS_MONK) && !WIELDING(ch) &&
	     BarbarianToHitMagicBonus(ch)+1 >= Immunity))
	 {
	    return( dam );
	 }
	 else 
	 {
	    act( "Il tuo colpo e` inefficace contro $N.", FALSE, ch, 0, v, 
		TO_CHAR );
	    return( 0 );
	 } /* was not TYPE_HIT or NPC */
	 
      }
      else 
      {
	 total = 0;
	 if( !ch->equipment[ WIELD ] )
	 return(0);
	 
	 for(j=0; j<MAX_OBJ_AFFECT; j++)
	 {
	    if( ( ch->equipment[ WIELD ]->affected[ j ].location == APPLY_HITROLL )
	       || ( ch->equipment[ WIELD ]->affected[ j ].location == 
		   APPLY_HITNDAM ) ) 
	    {
	       total += ch->equipment[ WIELD ]->affected[ j ].modifier;
	    }
	 }
	 
	 if( HasClass( ch, CLASS_BARBARIAN ) && BarbarianToHitMagicBonus( ch ) > 
	    total )
	 {
	    total = BarbarianToHitMagicBonus( ch );
	 }
	 
	 if( total > Immunity ) 
	 {
	    return( dam );
	 }
	 else 
	 {
	    act("$N ignora il stupido attacco.", FALSE, ch, 0, v, TO_CHAR );
	    return(0);
	 }     
      }
   }
}

void DamageStuff(struct char_data *v, int type, int dam, int location)
{
   int num, dam_type;
   struct obj_data *obj;
   
   /* add a check for anti-magic shell or some other item protection */
   /* spell right here I would think */
   
   
   if( type >= TYPE_HIT && type <= TYPE_RANGE_WEAPON )
   {
      num = location ; /* modified to be consistent with location Gaia (7/2000)
                          also the shield is now subject to easy breaking */
      if( v->equipment[ num ] ) 
      {
	 if( (type == TYPE_BLUDGEON && dam > 10) ||
	    (type == TYPE_CRUSH && dam > 5) ||
	    (type == TYPE_SMASH && dam > 10) ||
	    (type == TYPE_BITE && dam > 15) ||
	    (type == TYPE_CLAW && dam > 20) ||
	    (type == TYPE_SLASH && dam > 30) ||
	    (type == TYPE_SMITE && dam > 10) ||
	    (type == TYPE_HIT && dam > 40) ||
            ( num == 11 && dam > 1 )) 
	 {
	    if( DamageOneItem( v, BLOW_DAMAGE, v->equipment[ num ] ) )
	    {
	       if( ( obj = unequip_char( v, num ) ) != NULL ) 
	       {
		  MakeScrap( v, NULL, obj );
	       }
	    }
	 }
      }
   } 
   else 
   {
      dam_type = GetItemDamageType( type );
      if( dam_type )
      {
	 num = number( 1, 50 ); /* as this number increases or decreases
				 * the chance of item damage decreases
				 * or increases */
	 if (dam >= num)
	 DamageAllStuff( v, dam_type );
      }
   }
}


int GetItemDamageType( int type)
{
   
   switch(type) 
   {
    case SPELL_FIREBALL:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_INCENDIARY_CLOUD:
    case SKILL_MIND_BURN:
    case TYPE_GENERIC_FIRE:
      return( FIRE_DAMAGE );
      break;
      
    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
    case TYPE_GENERIC_ELEC:
      return( ELEC_DAMAGE );
      break;
      
    case SPELL_CONE_OF_COLD:
    case SPELL_ICE_STORM:
    case SPELL_FROST_BREATH:
    case TYPE_GENERIC_COLD:
      return( COLD_DAMAGE );
      break;
      
    case SPELL_COLOUR_SPRAY:
    case SPELL_METEOR_SWARM:
    case SPELL_GAS_BREATH:
    case SPELL_DISINTEGRATE:
    case TYPE_GENERIC_ENERGY:
      return( BLOW_DAMAGE );
      break;
      
    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
    case TYPE_GENERIC_ACID:
      return( ACID_DAMAGE );
    default:
      return(0);
      break;  
   }
   
}

int SkipImmortals(struct char_data *v, int amnt,int attacktype)
{
   /* You can't damage an immortal! */
   
   if( ( GetMaxLevel( v ) > MAX_MORT ) && IS_PC( v ) ) 
   amnt = 0;
   
   /* special type of monster */                
   if( IS_NPC( v ) && ( IS_SET( v->specials.act, ACT_IMMORTAL ) ) )
   {
      amnt = -1;
   }
   if( IS_PC( v ) && IS_LINKDEAD( v ) && 
      ( attacktype == TYPE_SUFFERING ||
       attacktype == SPELL_POISON  ||
       attacktype == SPELL_HEAT_STUFF ) )
   {  /* link dead pc, no damage */
      amnt = -1;
   }
   
   return( amnt );
   
}

void WeaponSpell( struct char_data *c, struct char_data *v, 
		 struct obj_data *obj, int type )
{
   int j, num;
   
   if( ( c->in_room == v->in_room && GET_POS( v ) != POSITION_DEAD ) ||
      ( GET_POS( v ) != POSITION_DEAD && type == TYPE_RANGE_WEAPON ) ) 
   {
    if(GET_LEVEL( c , BestMagicClass( c )  ) > 6 )  /* GAIA 2000 bug fix */
     {
      if( ( c->equipment[WIELD] && type >= TYPE_BLUDGEON && type <= TYPE_SMITE ) 
	 || ( type == TYPE_RANGE_WEAPON && obj ) )
      {
	 struct obj_data *weapon;
	 
	 if (type == TYPE_RANGE_WEAPON)
	 weapon = obj;
	 else
	 weapon = c->equipment[WIELD];
	 
	 for( j = 0; j < MAX_OBJ_AFFECT; j++ )
	 {
	    if( weapon->affected[j].location == APPLY_WEAPON_SPELL )
	    {
	       int iLevel;
	       num = weapon->affected[j].modifier;
	       if( num <= 0 ) 
	       num = 1;
	       iLevel = MIN( 4, GET_LEVEL( c, BestMagicClass( c ) ) / 6 );
	       (*spell_info[num].spell_pointer)( iLevel, c, "", SPELL_TYPE_WAND, 
						v, 0 );
	    } /* was weapon spell */
	 } /* MAX_OBJ for */
      } /* type check */
    } /* Check if newbie */
   } /* in same room */
}

void shoot( struct char_data *ch, struct char_data *victim)
{
#if 0
   struct obj_data *bow, *arrow;
   int oldth,oldtd;
   int tohit=0, todam=0;
   
   /*
    **  check for bow and arrow.
    */
   
   
   bow = ch->equipment[HOLD];
   arrow = ch->equipment[WIELD];
   
   /* this is checked in do_shoot now */
   if (!bow)
   {
      send_to_char("You need a missile weapon (like a bow)\n\r", ch);
      return;
   }
   else if (!arrow)
   {
      send_to_char("You need a projectile to shoot!\n\r", ch);
   }
   else if (!bow && !arrow)
   {
      send_to_char("You need a bow-like item, and a projectile to shoot!\n\r",ch);
   }
   else
   {
      /*
       **  for bows:  value[0] = arrow type
       **             value[1] = type 0=short,1=med,2=longranged 
       **             value[2] = + to hit
       **             value[3] = + to damage
       */
      
      
      if (bow->obj_flags.value[0] != arrow->obj_flags.value[0])
      {
	 send_to_char("That projectile does not fit in that projector.\n\r", ch);
	 return;
      }
      
      /*
       **  check for bonuses on the bow.
       */
      tohit = bow->obj_flags.value[2];
      todam = bow->obj_flags.value[3];
      
      /*
       **   temporarily remove other stuff and add bow bonuses.
       */
      oldth=GET_HITROLL(ch);
      oldtd=GET_DAMROLL(ch);
      /* figure range mods for this weapon */
      if (victim->in_room != ch->in_room)
      {
	 switch(bow->obj_flags.value[1])
	 {
	  case 0:
	    tohit -=4;  /* short range weapon -4 to hit */
	    break;
	  case 1:
	    tohit -=3;  /* med range weapon -3 to hit */
	    break;
	  case 2:
	    tohit -=2;  /* long range weapon -2 to hit */
	    break;
	  default:
	    tohit-=1;  /* unknown, default to -1 tohit */
	    break;
	 } /* end switch */
      }
      
      /* set tohit and dam to bows only, lets not use cumalitive of what */
      /* they already have */
      
      GET_HITROLL(ch)=tohit;
      GET_DAMROLL(ch)=todam;
      
      act("$n shoots $p at $N!", FALSE, ch, arrow, victim, TO_NOTVICT);
      act("$n launches $p at you", FALSE, ch, arrow, victim, TO_VICT);
      act("You shoot at $N with $p", FALSE, ch, arrow, victim, TO_CHAR);
      
      /*
       **   fire the weapon.
       */
      MissileHit(ch, victim, TYPE_UNDEFINED);
      
      GET_HITROLL(ch)=oldth;
      GET_DAMROLL(ch)=oldtd;
      
   }
   
   mudlog( LOG_CHECK, "end shoot, fight.c");
   
#endif
}

struct char_data *FindMetaVictim( struct char_data *ch)
{
   struct char_data *tmp_ch;
   unsigned char found=FALSE;
   unsigned short total=0;
   
   PushStatus("FindMetaVictim");
   if (ch->in_room < 0) return(0);
   
   for( tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
       tmp_ch=tmp_ch->next_in_room)
   {
      if( CAN_SEE(ch,tmp_ch) && !IS_SET(tmp_ch->specials.act,PLR_NOHASSLE) )
      {
	 if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch))
	 {
	    if (!SameRace(ch, tmp_ch))
	    {
	       found = TRUE;
	       total++;
	    }
	 }
      }
   }
   
   /* if no legal enemies have been found, return 0 */
   
   if (!found)
   {
      PopStatus();
      return(0);
   }
   
   total = number(1,(int)total);
   
   for( tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
       tmp_ch=tmp_ch->next_in_room )
   {
      if( CAN_SEE(ch,tmp_ch) && !IS_SET(tmp_ch->specials.act,PLR_NOHASSLE) )
      {
	 if (!SameRace(tmp_ch, ch))
	 {
	    total--;
	    if (total == 0) 
	    {
	       tmp_ch=SwitchVictimToPrince(ch,tmp_ch);
	       PopStatus();
	       return(tmp_ch);
	    }
	    
	 }
      }
   }
   PopStatus();
   if (ch->specials.fighting)
   return(ch->specials.fighting);
   
   return(0);
   
}


/*
* returns, extracts, switches etc.. anyone.
*/
void NailThisSucker( struct char_data *ch)
{
   
   struct char_data *pers;
   long room_num;
   struct room_data *rp;
   struct obj_data *obj, *next_o;
   
   rp = real_roomp(ch->in_room);
   room_num=ch->in_room;
   
   death_cry(ch);
   
   if( IS_NPC( ch ) && IS_SET( ch->specials.act, ACT_POLYSELF ) ) 
   {
      /*
       *   take char from storage, to room     
       */
      pers = ch->desc->original;
      char_from_room(pers);
      char_to_room(pers, ch->in_room);
      SwitchStuff(ch, pers);
      extract_char(ch);
      ch = pers;
   }
   
   if (IS_SET(rp->room_flags,DEATH)) 
   {
      mudlog( LOG_PLAYERS, "%s hit a DeathTrap in room %s[%ld]\r\n",
	     GET_NAME_DESC(ch), real_roomp(room_num)->name,room_num );
   }
   
   zero_rent(ch);
   extract_char(ch);
   
   /* delete EQ dropped by them if room was a DT */
   if (IS_SET(rp->room_flags,DEATH)) 
   {
      for (obj = real_roomp(room_num)->contents;obj; obj = next_o) 
      {
	 next_o = obj->next_content;
	 extract_obj(obj);
      }  /* end DT for */
   }
}


int GetFormType(struct char_data *ch)
{
   int num;
   
   num = number(1,100);
   switch(GET_RACE(ch))
   {
    case RACE_REPTILE:
      if (num <= 50)
      {
	 return(TYPE_CLAW);
      }
      else
      {
	 return(TYPE_BITE);
      }
      break;
    case RACE_LYCANTH:
    case RACE_DRAGON:
    case RACE_DRAGON_RED    :
    case RACE_DRAGON_BLACK  :
    case RACE_DRAGON_GREEN  :
    case RACE_DRAGON_WHITE  :
    case RACE_DRAGON_BLUE   :
    case RACE_DRAGON_SILVER :
    case RACE_DRAGON_GOLD   :
    case RACE_DRAGON_BRONZE :
    case RACE_DRAGON_COPPER :
    case RACE_DRAGON_BRASS  :  
    case RACE_PREDATOR:
    case RACE_LABRAT:
      if (num <= 33)
      {
	 return(TYPE_BITE);
      }
      else
      {
	 return(TYPE_CLAW);
      }
      break;
    case RACE_INSECT:
      if (num <= 50)
      {
	 return(TYPE_BITE);
      }
      else
      {
	 return(TYPE_STING);
      }
      break;
    case RACE_ARACHNID:
    case RACE_DINOSAUR:
    case RACE_FISH:
    case RACE_SNAKE:
      return(TYPE_BITE);
      break;
    case RACE_BIRD:
    case RACE_SKEXIE:
      return(TYPE_CLAW);
      break;
    case RACE_GIANT:
    case RACE_GIANT_HILL   :
    case RACE_GIANT_FROST  :
    case RACE_GIANT_FIRE   :
    case RACE_GIANT_CLOUD  :
    case RACE_GIANT_STORM  :
    case RACE_GIANT_STONE  :
    case RACE_GOLEM:
      return(TYPE_BLUDGEON);
      break;
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
      return(TYPE_CLAW);
      break;
    case RACE_TREE:
      return(TYPE_SMITE);
      break;
    case RACE_MFLAYER:
      if (num <= 60)
      {
	 return(TYPE_WHIP);
      }
      else if (num < 80)
      {
	 return(TYPE_BITE);
      }
      else
      {
	 return(TYPE_HIT);
      }
      break;
    case RACE_PRIMATE:
      if (num <= 70)
      {
	 return(TYPE_HIT);
      }
      else
      {
	 return(TYPE_BITE);
      }
      break;
    case RACE_TYTAN:
      return(TYPE_BLAST);
      break;
    default:
      return(TYPE_HIT);
   }
}

int MonkDodge( struct char_data *ch, struct char_data *v, int *dam)
{
   if (number(1, 20000) < v->skills[SKILL_DODGE].learned*
       GET_LEVEL(v , MONK_LEVEL_IND))
   {
      *dam = 0;
      act("You dodge the attack", FALSE, ch, 0, v, TO_VICT);
      act("$N dodges the attack", FALSE, ch, 0, v, TO_CHAR);
      act("$N dodges $n's attack", FALSE, ch, 0, v, TO_NOTVICT);
   }
   else
   {
      *dam -= GET_LEVEL(ch, MONK_LEVEL_IND)/10;
   }
   
   return(0);
}

int BarbarianToHitMagicBonus( struct char_data *ch )
{
   int bonus=0;
   
   if (GetMaxLevel(ch) <=7)
   bonus = 1;
   else if (GetMaxLevel(ch) <= 8)  
   bonus = 2;
   else if (GetMaxLevel(ch) <= 20)   
   bonus = 3;
   else if (GetMaxLevel(ch) <= 28)   
   bonus = 4;
   else if (GetMaxLevel(ch) <= 35)   
   bonus = 5;
   else if (GetMaxLevel(ch) >35)
   bonus = 5;
   
   return(bonus);
}

int berserkthaco ( struct char_data *ch)
{
   if( GetMaxLevel(ch) <= 10 ) /* -5 to hit when berserked */
   return(5);
   if( GetMaxLevel(ch) <= 25 ) /* -3 */
   return(3);
   if( GetMaxLevel(ch) <= 40 ) /* -2 */
   return(2);
   return(2);
}

int berserkdambonus ( struct char_data *ch, int dam)
{
   if( GetMaxLevel(ch) <= 10 )     /* 1.33 dam when berserked */
   return (int)( dam * 1.33 ); 
   else if( GetMaxLevel(ch) <= 25 )     /* 1.5 */
   return (int)( dam * 1.5 ); 
   else if( GetMaxLevel(ch) <= 40 )     /* 1.7 */
   return (int)( dam * 1.7 );
   else
   return (int)( dam * 1.7 );
}


int range_hit( struct char_data *ch, struct char_data *targ, int rng, struct
	      obj_data *missile, int tdir, int max_rng) 
{
   /* Returns 1 on a hit, 0 otherwise */
   /* Does the roll, damage, messages, and everything */
   
   int calc_thaco, i, dam = 0, diceroll, victim_ac, location;
   char *dir_name[] = 
   {
      "da nord",
      "da est",
      "da sud",
      "da ovest",
      "dall'alto",
      "dal basso"
   };
   int opdir[] = { 2, 3, 0, 1, 5, 4 }, rmod, cdir, rang, cdr;
   char buf[MAX_STRING_LENGTH];
   extern struct dex_app_type dex_app[];
   
   if (!IS_NPC(ch)) 
   {
      calc_thaco=20;
      for(i=1;i<5;i++) 
      {
	 if( thaco[ i-1 ][ GetMaxLevel( ch ) ] < calc_thaco )
	 {
	    calc_thaco=thaco[i-1][GetMaxLevel(ch)];
	 }
      }
   }
   else 
   {
      /* THAC0 for monsters is set in the HitRoll */
      calc_thaco = 20;
   }
   
   calc_thaco -= GET_HITROLL(ch);
   rmod = 20*rng/max_rng;
   calc_thaco += rmod;
   if (GET_POS(targ)==POSITION_SITTING) 
   {
      calc_thaco += 7;
   }
   if (GET_POS(targ)==POSITION_RESTING) 
   {
      calc_thaco += 10;
   }
   
   diceroll = number(1,20);
   
   victim_ac  = GET_AC(targ)/10;
   
   if (AWAKE(targ))
   victim_ac += dex_app[ (int)GET_DEX( targ ) ].defensive;
   
   victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */
   
   if( (diceroll < 20) && 
      ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) 
   {
      /* Missed! */
      if( rng > 0 && tdir >= 0 ) 
      {
	 sprintf( buf,"$p %s sfiora $n!", dir_name[ opdir[ tdir ] ] );
	 act( buf, FALSE, targ, missile, 0, TO_ROOM );
	 act( "$p sfiora $N.", TRUE, ch, missile, targ, TO_CHAR );
      }
      else 
      {
	 act( "$p sfiora $n!",FALSE,targ,missile,0,TO_ROOM);
      }
      if (AWAKE(targ)) 
      {
	 if( rng > 0 && tdir >= 0 ) 
	 {
	    sprintf( buf,"$p %s ti passa a pochi millimetri!",
		    dir_name[ opdir[ tdir ] ] );
	    act( buf, TRUE, targ, missile, 0, TO_CHAR );
	 }
	 else 
	 {
	    act( "$n lancia $p mancandoti di poco!", TRUE, ch, missile, targ,
		TO_VICT );
	 }
	 if( IS_NPC( targ ) ) 
	 {
	    if( rng == 0 ) 
	    {
	       hit( targ, ch, TYPE_UNDEFINED );
	    }
	    else 
	    {
	       cdir = can_see_linear( targ, ch, &rang, &cdr );
	       if( IS_NPC( targ) && !targ->specials.charging ) 
	       {
		  if( cdir != -1 ) 
		  {
		     if( GET_POS( targ ) != POSITION_FIGHTING &&
			GET_POS( targ ) > POSITION_STUNNED ) 
		     {
			if( number( 1, 2 ) == 1 )
			{
			   GET_POS( targ ) = POSITION_STANDING;
			   /* Ain't gonna take any more of this missile crap! */
			   AddHated( targ, ch );
			   act( "$n appare piuttosto contrariato!", TRUE, targ, 0, 0,
			       TO_ROOM );
			   targ->specials.charging = ch;
			   targ->specials.charge_dir = cdr;
			}
		     }
		  }
	       }
	    }
	 }
      }
      return 0;
   }
   else 
   {
      dam += dice(missile->obj_flags.value[1],missile->obj_flags.value[2]);
      dam = MAX(1, dam);  /* Not less than 0 damage */
      if( missile->obj_flags.type_flag != ITEM_MISSILE )
      {
	 dam *= 3;
      }
      
      if( !IS_PC( targ ) )
      AddHated(targ,ch);
      if( tdir >= 0 )
      sprintf( buf, "$p %s colpisce $n!", dir_name[ opdir[ tdir ] ] );
      else
      sprintf( buf, "$p colpisce $n!" );
      act( buf, TRUE, targ, missile, 0, TO_ROOM );
      if( ch->in_room != targ->in_room )
      act( "$p colpisce $N!", TRUE, ch, missile, targ, TO_CHAR );
      if( tdir >= 0 )
      sprintf( buf, "$p %s ti ha colpito!",dir_name[ opdir[ tdir ] ] );
      else
      sprintf( buf, "$p ti ha colpito!" );
      act(buf,TRUE,targ,missile,0,TO_CHAR);
      location = Hit_Location( targ ) ;
      if( damage( ch, targ, dam, TYPE_RANGE_WEAPON, location ) == VictimDead )
      targ = 0;
      
      if( !targ || GET_HIT(targ) < 1 ) 
      return 1;
      else 
      {
	 if( ch->in_room != targ->in_room ) 
	 {
	    WeaponSpell( ch, targ, missile, TYPE_RANGE_WEAPON );
	 }
      }
      
      if( GET_POS( targ ) != POSITION_FIGHTING &&
	 GET_POS( targ ) > POSITION_STUNNED ) 
      {
	 if( IS_NPC( targ ) && !targ->specials.charging ) 
	 {
	    GET_POS( targ ) = POSITION_STANDING;
	    if( rng == 0 ) 
	    {
	       hit( targ, ch, TYPE_UNDEFINED );
	    }
	    else 
	    {
	       cdir = can_see_linear( targ, ch, &rang, &cdr );
	       if( cdir != -1 ) 
	       {
		  act( "$n appare piuttosto contrariato!",TRUE,targ,0,0,TO_ROOM );
		  targ->specials.charging = ch;
		  targ->specials.charge_dir = cdr;
	       }
	    }
	 }
      }
      return 1;
   }
}

void increase_blood(int rm)
	{
			RM_BLOOD(rm) = MIN(RM_BLOOD(rm) +1, 10);
			/*mudlog(LOG_ERROR,"Blood %d nella stanza %d",RM_BLOOD(rm),rm);*/
	}

