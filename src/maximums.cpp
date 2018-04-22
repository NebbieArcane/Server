/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: maximums.c,v 1.2 2002/02/13 12:30:58 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "maximums.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

/* struct room_data *real_roomp(int); */

/*ALAR modificato. Invecchiando il
 * mana gain diventa SEMPRE +
 * alto. In compenso ho peggiorato
 * ancora sia il max, che il
 * gain di hp
 * */

char* ClassTitles(struct char_data* ch) {
	unsigned int i, count=0;
	static char buf[256];

	for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
		int nLev = GET_LEVEL( ch, i );
		if( nLev ) {
			count++;
			if(count > 1) {
				sprintf(buf + strlen(buf), "/%s", GET_CLASS_TITLE(ch, i, nLev));
			}
			else {
				sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, nLev));
			}
		}
	}
	return(buf);
}


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6) {

	if(age < 15) {
		return(p0);    /* < 15   */
	}
	else if(age <= 29) {
		return (int)(p1+(((age-15)*(p2-p1))/15));    /* 15..29 */
	}
	else if(age <= 44) {
		return (int)(p2+(((age-30)*(p3-p2))/15));    /* 30..44 */
	}
	else if(age <= 59) {
		return (int)(p3+(((age-45)*(p4-p3))/15));    /* 45..59 */
	}
	else if(age <= 79) {
		return (int)(p4+(((age-60)*(p5-p4))/20));    /* 60..79 */
	}
	else {
		return(p6);    /* >= 80 */
	}
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data* ch) {
	int max, tmp, chClasses;

	max = 0;

	if(!IS_PC(ch)) {
		return(100);
	}
	if (HasClass(ch, CLASS_MAGIC_USER)) {
		max += 100;
		max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
	}

	/* actually this is worthless as Sorcerer's do not */
	/* use mana at all.... */
	if (HasClass(ch, CLASS_SORCERER)) {
		max += 100;
		max += GET_LEVEL(ch, SORCERER_LEVEL_IND) * 5;
	}

	if (HasClass(ch, CLASS_PSI)) {
		max += 100;
		max += GET_LEVEL(ch, PSI_LEVEL_IND) * 5;
	}


	if (HasClass(ch, CLASS_PALADIN)) {
		max += 100;
		max += (GET_LEVEL(ch, PALADIN_LEVEL_IND)/4) * 5;
	}

	if (HasClass(ch, CLASS_RANGER)) {
		max += 100;
		max += (GET_LEVEL(ch, RANGER_LEVEL_IND)/4) * 5;
	}

	if (HasClass(ch, CLASS_CLERIC)) {
		max += 100;
		max += (GET_LEVEL(ch, CLERIC_LEVEL_IND)/3) * 5;
	}

	if (HasClass(ch, CLASS_DRUID)) {
		max += 100;
		max += (GET_LEVEL(ch, DRUID_LEVEL_IND)/3) * 5;
	}

	if (HasClass(ch, CLASS_THIEF)) {
		max += 100;
	}

	if (HasClass(ch, CLASS_WARRIOR)) {
		max += 100;
	}

	if (HasClass(ch, CLASS_MONK)) {
		max += 100;
	}

	chClasses = HowManyClasses(ch);

	if(chClasses) {
		max /= HowManyClasses(ch);
	}
	else {
		mudlog(LOG_ERROR, "Character with no levels! Name [%s]", ch->player.name);
	}


	/*
	 * new classes should be inserted here.
	 */

	tmp = 0;


	tmp = GET_INT(ch)/3;
	tmp += 2;
	tmp = tmp*3;

	max += tmp;
	max += ch->points.max_mana;
	/* bonus mana. In questo campo vanno gli affect  */

	struct time_info_data ma;
	age2(ch, &ma);
	max += (graf(ma.year, 6,8,12,16,20,24,28))-10;

	/*
	 * Add class mana maximums here...
	 */

	if (OnlyClass(ch,CLASS_BARBARIAN)) { /* 100 mana max for barbs */
		max=100;  /* barbarians only get 100 mana... */
	}

	return(max);
}


int hit_limit(struct char_data* ch) {
	int max;

	if (IS_PC(ch)) {
		struct time_info_data ma;
		age2(ch, &ma);
		max = (ch->points.max_hit) + (graf(ma.year, 2,4,17,14,8,-5,-15));

	}
	else {
		max = (ch->points.max_hit);
	}

	/* Class/Level calculations */

	/* Skill/Spell calculations */

	return (max);
}


int move_limit(struct char_data* ch) {
	int max;

	if (IS_PC(ch)) {
		max = 100 ;
	}
	else {
		max = ch->points.max_move;
		if(IsRideable(ch)) {
			max *= 2;
		}
		return(max);
	}
	struct time_info_data ma;
	age2(ch, &ma);
	max -= (graf(ma.year, -4,0,2,6,10,14,18));



	if(GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_GNOME) {
		max -= 35;
	}
	else if (GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_DARK_ELF  ||
			 GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
			 GET_RACE(ch) == RACE_SEA_ELF ||
			GET_RACE(ch)== RACE_HALF_ELVEN) {
		max += 20;
	}
	else if(GET_CLASS(ch) == CLASS_BARBARIAN) {
		max +=45;    /* barbs get more move ... */
	}
	else if(GET_RACE(ch) == RACE_HALFLING) {
		max -= 45 ;
	}
	else if(GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL) {
		max +=60;
	}
	else if(GET_RACE(ch) == RACE_HALF_OGRE) {
		max +=50        ;
	}

	max += ch->points.max_move;  /* move bonus */

	return (max);
}


/* manapoint gain pr. game hour */
int mana_gain(struct char_data* ch) {
	int gain;

	if( !IS_PC(ch) ) {
		/* Neat and fast */
		gain = 8;
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
#ifdef NEW_GAIN
		gain = graf(ma.year, 3,9,12,16,20,24,30);
#else
		gain = graf(ma.year, 2,4,6,8,10,16,2);
#endif
	}

	/* Position calculations    */
	/*switch (GET_POS_PREV(ch))*/
	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += gain;
		break;
	case POSITION_RESTING:
		gain+= (gain>>1);  /* Divide by 2 */
		break;
	case POSITION_SITTING:
		gain += (gain>>2); /* Divide by 4 */
		break;
	}

	gain += gain;

	gain += wis_app[ (int)GET_WIS(ch) ].bonus*2;

	gain += ch->points.mana_gain;

	if(IS_AFFECTED(ch, AFF_POISON)) {
		gain >>= 2;
	}

	if(GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0) {
		gain >>= 2;
	}

	if( GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_GNOME ||
			GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
			GET_RACE(ch) == RACE_SEA_ELF ||
			GET_RACE(ch) == RACE_DEMON ||
			GET_RACE(ch) == RACE_DARK_ELF || GET_RACE(ch) == RACE_HALF_ELVEN) {
		gain+=2;
	}

	if(GET_COND(ch, DRUNK) > 10) {
		gain += (gain >> 1);
	}
	else if(GET_COND(ch, DRUNK) > 0) {
		gain += (gain >> 2);
	}

	/* Class calculations */

	/* magic type people get quicker mana re-gen, fighter/paladin/rangers get it slower */
	if( !HasClass( ch, CLASS_MAGIC_USER | CLASS_SORCERER | CLASS_CLERIC |
				   CLASS_DRUID | CLASS_PSI ) ) {
		gain -= 2;
	}
	/* these guys get mana even slower */
	if(HasClass(ch, CLASS_BARBARIAN)) {
		gain -= 2;
	}

	/* Skill/Spell calculations */

	if(affected_by_spell(ch, SKILL_MEDITATE)) {
		gain += 3*GET_LEVEL(ch, PSI_LEVEL_IND)/10 ;
	}

	return( gain );
}

int hit_gain(struct char_data* ch) // Gaia 2001
/* Hitpoint gain pr. game hour */
{

	int gain, dam, i;

	if(IS_NPC(ch)) {
		gain = 8;
		/* Neat and fast */
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
#ifdef NEW_GAIN

		gain = graf(ma.year, 3,9,12,16,12,6,0);
#else
		gain = graf(ma.year, 2,4,6,8,6,3,1);
#endif
		/*if (GET_POS_PREV(ch) == POSITION_FIGHTING)*/
		if (GET_POS(ch) == POSITION_FIGHTING) {
#if defined( ALAR )
			gain = ((GET_RACE(ch) == RACE_TROLL)?gain:0);
#else
			gain = 0;
#endif
		}
	}


	/* Position calculations    */

	/*switch (GET_POS_PREV(ch))*/
	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += gain>>2;
		break;
	case POSITION_RESTING:
		gain+= gain>>3;
		break;
	case POSITION_SITTING:
		gain += gain>>4;
		break;
	}


	if(GET_RACE(ch) == RACE_DWARF) {
		gain += 2;
	}

	if(GET_RACE(ch) == RACE_HALFLING) {
		gain += 1;
	}
#if defined (ALAR)
	if(GET_RACE(ch) == RACE_TROLL) {
		gain += 5;
	}
#endif
	if(GET_RACE(ch) == RACE_HALF_GIANT) { /* faster hps regen */
		gain+=3;
	}
	if(GET_RACE(ch) == RACE_HALF_OGRE) {
		gain +=2;
	}
	if(GET_RACE(ch) == RACE_HALF_ORC) {
		gain +=1;
	}

	if(GET_CLASS(ch) == CLASS_BARBARIAN) {
		gain += 4;    /* barbs gain hits faster... */
	}

	gain += con_app[ (int)GET_CON(ch) ].hitp/2;

	gain += ch->points.hit_gain;

	if(GET_COND(ch,FULL) == 0 || GET_COND(ch, THIRST) == 0) {
		gain >>= 4;
	}

	if(GET_COND(ch, DRUNK)>10) {
		gain += (gain >> 1);
	}
	else if(GET_COND(ch, DRUNK)>0) {
		gain += (gain >> 2);
	}

	/* Class/Level calculations */
	/* non-warrior types get slower hps re-gen */
	if( IS_PC( ch ) &&
			!HasClass( ch, CLASS_WARRIOR | CLASS_PALADIN |
					   CLASS_RANGER | CLASS_BARBARIAN ) &&
			gain>2) {
		gain -=2;
	}

	if( IS_AFFECTED( ch, AFF_POISON ) ) {
		gain = 0;
		dam = number(10,32);
		if(GET_RACE(ch) == RACE_HALFLING) {
			dam = number(1,20);
		}
		/*I nani sono i piu' coriacei...*/
		if(GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_DARK_DWARF) {
			dam = number(1,10);
		}
		if(affected_by_spell(ch, SPELL_SLOW_POISON)) {
			dam /= 4;
		}
		/*  if( damage( ch, ch, dam, SPELL_POISON, location ) == VictimDead )
		   	return 0;   Elimino la chiamata a damage e lo metto come modificatore
		                sull HP-regen in modo che scali in continuo
		                inoltre ho potenziato lo spell con un *5  Gaia 2001 */
		gain -= dam*5 ;
	}

	if( IS_AFFECTED2( ch, AFF2_HEAT_STUFF ) ) {
		dam = 0;
		/*
		count items in eq
		*/
		for (i=0; i<=HOLD; i++) {
			if (ch->equipment[i]) {
				dam += 15;   /* Potenziato, utile per un pkill.. Gaia 2001 */
			}
		}

		/*  if( damage( ch, ch, dam, SPELL_HEAT_STUFF, location ) == VictimDead )
		   	return 0;   Elimino la chiamata a damage e lo metto come modificatore
		                sull HP-regen in modo che scali in continuo Gaia 2001 */
		gain -= dam ;
	}

	/*  if( gain < 0 && !ch->specials.fighting )
	  damage( ch, ch, gain * -1, TYPE_SUFFERING, location );
	  lo commento perche' il damage me lo vado a vedere solo se faccio
	  un regen event, anche se e' vero che devo stare attento alle no regen.
	  Gaia 2001 */
	/* Skill/Spell calculations */
	if(affected_by_spell(ch, SKILL_MEDITATE)) {
		gain += 3*GET_LEVEL(ch, PSI_LEVEL_IND)/10 ;
	}

	return( gain );
}



int move_gain(struct char_data* ch)
/* move gain pr. game hour */
{
	int gain;

	if(IS_NPC(ch)) {
		gain = 22;
		if(IsRideable(ch)) {
			gain += gain/2;
		}

		/* Neat and fast */
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
		if (GET_POS(ch) != POSITION_FIGHTING)
#ifdef NEW_GAIN
			gain = graf(ma.year, 15,21,25,28,20,7,1);
#else
			gain = graf(ma.year, 10,15,20,22,15,7,1);
#endif

		else {
			gain = 0;
		}
	}

	/* Position calculations    */
	/*switch (GET_POS_PREV(ch))*/
	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += (gain>>2); /* Divide by 4 */
		break;
	case POSITION_RESTING:
		gain+= (gain>>3);  /* Divide by 8 */
		break;
	case POSITION_SITTING:
		gain += (gain>>4); /* Divide by 16 */
		break;
	}


	if(GET_RACE(ch) == RACE_DWARF) {
		gain += 4;
	}

	if(GET_RACE(ch) == RACE_HALF_GIANT) {
		gain +=6;
	}
	if(GET_RACE(ch) == RACE_HALF_OGRE) {
		gain +=5;
	}
	if(GET_RACE(ch) == RACE_HALF_ORC) {
		gain +=4;
	}

	gain += ch->points.move_gain;

	if(IS_AFFECTED(ch,AFF_POISON)) {
		gain >>= 5;
	}

	if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0)) {
		gain >>= 3;
	}

	/* Class specific stuff */

	/* non-thief/monks types regen move slower */
	if(!HasClass(ch,CLASS_THIEF|CLASS_MONK)) {
		gain -= 2;
	}

	return (gain);
}

int GetHpGain(struct char_data* ch, int iClass,int livello,int compat,int check) {
	int add_hp;

	if (livello>=CHUMP) {
		if (iClass == WARRIOR_LEVEL_IND || iClass == BARBARIAN_LEVEL_IND ||
				iClass == PALADIN_LEVEL_IND || iClass == RANGER_LEVEL_IND) {
			add_hp = con_app[(int)(compat?18:GET_RCON(ch)) ].hitp;
		}
		else {
			add_hp = MIN(con_app[(int)(compat?18:GET_RCON(ch)) ].hitp,3);
		}
	}
	else {
		if (iClass == WARRIOR_LEVEL_IND || iClass == BARBARIAN_LEVEL_IND ||
				iClass == PALADIN_LEVEL_IND || iClass == RANGER_LEVEL_IND) {
			add_hp = con_app[(int)(compat?18:GET_RCON(ch)) ].hitp;
		}
		else {
			add_hp = MIN(con_app[(int)(compat?18:GET_RCON(ch)) ].hitp,3);
		}
	}


	switch(iClass) {

	case MAGE_LEVEL_IND :
		if(livello < 12) {
			add_hp += number(compat?6:2,6);
		}
		else {
			add_hp += number(compat?2:1,2);
		}
		break;

	case SORCERER_LEVEL_IND :
		if(livello < 12) {
			add_hp += number(compat?6:2,6);
		}
		else {
			add_hp += number(compat?2:1,2);
		}
		break;

	case CLERIC_LEVEL_IND :
		if(livello < 12) {
			add_hp += number(compat?15:3,15);
		}
		else {
			add_hp += number(compat?6:1,6);
		}
		break;

	case THIEF_LEVEL_IND :
		if(livello < 12) {
			add_hp +=number(compat?10:2,10);
		}
		else {
			add_hp += number(compat?4:1,4);
		}
		break;

	case PSI_LEVEL_IND :
		if(livello < 12) {
			add_hp +=number(compat?10:2,10);
		}
		else {
			add_hp += number(compat?4:1,4);
		}
		break;

	case WARRIOR_LEVEL_IND :
		if(livello < 10) {
			add_hp += number(compat?16:3,16);
		}
		else {
			add_hp += number(compat?8:2,8);
		}

		break;

	case RANGER_LEVEL_IND :
		if(livello < 11) {
			add_hp += number(compat?13:3,13);
		}
		else {
			add_hp += number(compat?8:2,8);
		}

		break;

	case PALADIN_LEVEL_IND :
		if(livello < 10) {
			add_hp += number(compat?16:3,16);
		}
		else {
			add_hp += number(compat?8:2,8);
		}

		break;

	case BARBARIAN_LEVEL_IND :
		if(livello < 15) {
			add_hp += number(compat?22:3,22);
		}
		else {
			add_hp += number(compat?12:3,12);
		}
		break;


	case DRUID_LEVEL_IND:
		if(livello < 15) {
			add_hp += number(compat?14:2,14);
		}
		else {
			add_hp += number(compat?6:1,6);
		}
		break;

	case MONK_LEVEL_IND:
		if(livello < 17) {
			add_hp += number(compat?10:2,10);
		}
		else {
			add_hp += number(compat?5:1,5);
		}
		break;

	}

	add_hp /= HowManyClasses(ch);
	if (compat) {
		add_hp += number(0,HowManyClasses(ch));
	}

	return(add_hp);
}

int GetExtimatedHp(struct char_data* ch) {
	int i=0;
	int j=0;
	int ext_hp=0;
	if (GetMaxLevel(ch) < IMMORTALE) {

		for (i=0; i<MAX_CLASS; i++) {

			if (GET_LEVEL(ch,i) > 0) {
				for(j=1; j<=GET_LEVEL(ch,i); j++) {
					ext_hp+=GetHpGain(ch,i,j,1,1);
				}
				/* for */
			}

		}
	}
	else {
		ext_hp=hit_limit(ch)*12/10;
	}
	return(ext_hp);
}


void advance_level(struct char_data* ch, int iClass)

/* Gain maximum in various points */
{
	int i, check_hp;


	if (iClass > MAX_CLASS) {
		mudlog( LOG_SYSERR, "Bad advance class.. no such class");
		return;
	}

	if (GET_LEVEL(ch, iClass) > 0 &&
			GET_EXP(ch) < titles[iClass][GET_LEVEL(ch, iClass)+1].exp) {
		/*  they can't advance here */
		mudlog( LOG_ERROR, "Bad advance_level, can't advance in this class.");
		return;
	}

	GET_LEVEL(ch, iClass) += 1;


	check_hp=GetHpGain(ch,iClass,GET_LEVEL(ch,iClass),0);

	if (check_hp != -1) {

		ch->points.max_hit += MAX( 1, check_hp );
	}

	if( ch->specials.spells_to_learn < 100 )
		ch->specials.spells_to_learn +=
			MAX( 2, wis_app[ (int)GET_RWIS( ch ) ].bonus );
	else {
		send_to_char("Stai perdendo le sessioni di allenamento!", ch);
	}

	ClassSpecificStuff(ch);

	if( GetMaxLevel(ch) >= IMMORTALE )
		for(i = 0; i < 3; i++) {
			ch->specials.conditions[ i ] = -1;
		}

	/* start regening new points */
	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);
}





/* Lose in various points */

/*
** Damn tricky for multi-class...
*/

void drop_level(struct char_data* ch, int iClass, int goddrain) {
	int add_hp, lin_class;

	if (!goddrain) {
		if(GetMaxLevel(ch) >= IMMORTALE) {
			return;
		}
	}

	if(GetMaxLevel(ch) == 1) {
		return;
	}

	add_hp = con_app[ (int)GET_RCON(ch) ].hitp;

	switch(iClass) {

	case CLASS_MAGIC_USER :
		lin_class = MAGE_LEVEL_IND;
		if(GET_LEVEL(ch, MAGE_LEVEL_IND) < 12) {
			add_hp += number(2, 8);
		}
		else {
			add_hp += 2;
		}
		break;

	case CLASS_SORCERER :
		lin_class = SORCERER_LEVEL_IND;
		if(GET_LEVEL(ch, SORCERER_LEVEL_IND) < 12) {
			add_hp += number(2, 8);
		}
		else {
			add_hp += 2;
		}
		break;

	case CLASS_CLERIC :
		lin_class = CLERIC_LEVEL_IND;
		if(GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12) {
			add_hp += number(2, 12);
		}
		else {
			add_hp += 5;
		}
		break;

	case CLASS_THIEF :
		lin_class = THIEF_LEVEL_IND;
		if(GET_LEVEL(ch, THIEF_LEVEL_IND) < 12) {
			add_hp += number(2,10);
		}
		else {
			add_hp += 4;
		}
		break;

	case CLASS_PSI :
		lin_class = PSI_LEVEL_IND;
		if(GET_LEVEL(ch, PSI_LEVEL_IND) < 12) {
			add_hp += number(2,10);
		}
		else {
			add_hp += 4;
		}
		break;

	case CLASS_WARRIOR :
		lin_class = WARRIOR_LEVEL_IND;
		if(GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 10) {
			add_hp += number(2,16);
		}
		else {
			add_hp += 6;
		}
		break;

	case CLASS_PALADIN :
		lin_class = PALADIN_LEVEL_IND;
		if(GET_LEVEL(ch, PALADIN_LEVEL_IND) < 10) {
			add_hp += number(2,16);
		}
		else {
			add_hp += 6;
		}
		break;


	case CLASS_RANGER :
		lin_class = RANGER_LEVEL_IND;
		if(GET_LEVEL(ch, RANGER_LEVEL_IND) < 11) {
			add_hp += number(2,13);
		}
		else {
			add_hp += 6;
		}
		break;

	case CLASS_DRUID:
		lin_class = DRUID_LEVEL_IND;
		if(GET_LEVEL(ch,  DRUID_LEVEL_IND) < 15) {
			add_hp += number(2,12);
		}
		else {
			add_hp += 5;
		}
		break;

	case CLASS_MONK:
		lin_class = MONK_LEVEL_IND;
		if(GET_LEVEL(ch, MONK_LEVEL_IND) < 17) {
			add_hp += number(2,10);
		}
		else {
			add_hp += 4;
		}
		break;

	case CLASS_BARBARIAN :
		lin_class = BARBARIAN_LEVEL_IND;
		if(GET_LEVEL(ch, BARBARIAN_LEVEL_IND) < 15) {
			add_hp += number(2,14);
		}
		else {
			add_hp += 8;
		}
		break;
	default:
		mudlog( LOG_SYSERR, "Classe %d non valida in drop_level (limits.c).",
				iClass );
		lin_class = 0; /* Per evitare l'avertimento del compilatore. */
		return;

	} /*end switch */


	GET_LEVEL(ch, lin_class) -= 1;

	if (GET_LEVEL(ch, lin_class) < 1) {
		GET_LEVEL(ch, lin_class) = 1;
		if(ch->points.max_hit > 20) {
			ch->points.max_hit = 20;
		}
	}


	if(iClass == CLASS_WARRIOR) {
		add_hp = MAX(add_hp, 6);
	}
	if(iClass == CLASS_BARBARIAN) {
		add_hp = MAX(add_hp, 8);
	}
	if(iClass == CLASS_PALADIN) {
		add_hp = MAX(add_hp, 6);
	}
	if(iClass == CLASS_RANGER) {
		add_hp = MAX(add_hp, 5);
	}
	if(iClass == CLASS_CLERIC) {
		add_hp = MAX(add_hp, 5);
	}
	if(iClass == CLASS_THIEF) {
		add_hp = MAX(add_hp, 4);
	}
	if(iClass == CLASS_PSI) {
		add_hp = MAX(add_hp, 4);
	}
	if(iClass == CLASS_MAGIC_USER) {
		add_hp = MAX(add_hp, 3);
	}
	if(iClass == CLASS_SORCERER) {
		add_hp = MAX(add_hp, 3);
	}
	if(iClass == CLASS_MONK) {
		add_hp = MAX(add_hp, 4);
	}
	if(iClass == CLASS_DRUID) {
		add_hp = MAX(add_hp, 5);
	}


	add_hp /= HowManyClasses(ch);


	if(add_hp <=2) {
		add_hp=3;
	}

	ch->points.max_hit -= MAX(1,add_hp);
	if(ch->points.max_hit < 1) {
		ch->points.max_hit = 1;
	}

	ch->specials.spells_to_learn -=
		MAX( 1, MAX( 2, wis_app[ (int)GET_RWIS(ch) ].bonus ) );
	if(ch->specials.spells_to_learn < 0) {
		ch->specials.spells_to_learn = 0;
	}

	if( ch->points.exp >
			MIN(titles[lin_class][ (int)GET_LEVEL(ch, lin_class) ].exp, GET_EXP(ch)))
		ch->points.exp =
			MIN(titles[lin_class][ (int)GET_LEVEL(ch, lin_class) ].exp, GET_EXP(ch));

	if(ch->points.exp < 0) {
		ch->points.exp = 0;
	}

	send_to_char("Perdi un livello.\n\r", ch);

}



void set_title(struct char_data* ch) {

	char buf[256];

	if(IS_SET(ch->player.user_flags,SHOW_CLASSES)) {
		sprintf(buf,
				"the %s %s", RaceName[ch->race], ClassTitles(ch));
	}
	else {
		sprintf(buf, "the %s", RaceName[ch->race] );
	}

	if (GET_TITLE(ch)) {
		free(GET_TITLE(ch));
		CREATE(GET_TITLE(ch),char,strlen(buf)+1);
	}
	else {
		CREATE(GET_TITLE(ch),char,strlen(buf)+1);
	}

	strcpy(GET_TITLE(ch), buf);

}
int gain_corretto(struct char_data* ch,int gain) {
	if(!IS_PC(ch) || gain <= 0) {
		return(gain);    // Gaia 2001
	}
#if NEW_EQ_GAIN // Gaia 2001
	float eqindex=0.0;
	float eqratio=0.0;
	float tmp=0.0;
	int newgain=0;
	char buf[255];
	eqindex=(float)GetCharBonusIndex(ch);
	eqratio=eqindex-EQINDEX;
	if(abs((int)eqratio)<100) {
		return(gain);
	}
	tmp=eqratio/20;
	if(tmp < -30) {
		tmp = -30 ;
	}
	if(tmp > 30) {
		tmp = 30 ;
	}
	tmp=gain*tmp/100;
	newgain=(int)(gain-tmp);
	buglog(LOG_CHECK,"Eq: %s I: %f R:%f gain da %d %d",
		   GET_NAME(ch),eqindex,eqratio,gain,newgain);

	sprintf( buf, "Il tuo equipaggiamento modifica il guadagno di esperienza in  %d",
			 newgain);
	act( buf, 0, ch, 0, ch, TO_CHAR );
	gain = newgain ;
	/*#else
	   sprintf( buf, "La qualita' del tuo equipaggiamento modificherebbe il tuo guadagno di esperienza in  %d",
		   newgain);
	   act( buf, 0, ch, 0, ch, TO_CHAR );*/
#endif

	return (gain);
}

void gain_exp_rev( struct char_data* ch, int gain ) {
	/* gain completo se positivo, diviso per le classi se negativo */
	int i;
	char buf[256];
	short chrace;
	int nClassiNonMaxxate=0;
	save_char(ch,AUTO_RENT, 0);
	gain=gain_corretto(ch,gain);
	if( !IS_PC( ch ) && ch->master && IS_AFFECTED( ch, AFF_CHARM ) ) {
		if( ch->master->in_room == ch->in_room ) {
			if( gain > 1 ) {
				gain /= 2;
				sprintf( buf, "Guadagni i %d punti di esperienza di $N", gain );
				act( buf, 0, ch->master, 0, ch, TO_CHAR );
				gain_exp( ch->master, gain );
			}
		}
		return;
	}

	if( !IS_IMMORTAL( ch ) ) {
		if( gain > 0 ) {

			if( GetMaxLevel( ch ) == 1 ) {
				gain *= 2;
			}

			if( IS_PC( ch ) ) {
				if(ch->desc && ch->desc->original) {
					chrace = ch->desc->original->race;
				}
				else {
					chrace = GET_RACE(ch);
				}
				for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
					/*ALAR:check sui limiti razziali per i  livelli */
					if( GET_LEVEL( ch, i ) &&
							GET_LEVEL( ch, i ) < RacialMax[ chrace ][ i ] ) {
						nClassiNonMaxxate++;
						if( GET_EXP( ch ) >=
								titles[ i ][ GET_LEVEL( ch, i ) + 1 ].exp ) {
							/* do nothing..this is cool */
						}
						else if( ( GET_EXP( ch ) + gain ) >=
								 titles[ i ][ GET_LEVEL( ch, i ) + 1].exp ) {
							sprintf( buf, "Sei abbastanza esperto per essere un %s\n\r",
									 GET_CLASS_TITLE( ch, i, GET_LEVEL( ch, i ) + 1 ) );
							send_to_char( buf, ch );
							send_to_char( "Devi passare dalla tua gilda per crescere di "
										  "livello.\n\r", ch );
							if( ( GET_EXP( ch ) + gain ) >=
									titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp) {

								/*BUG BUG overflow*/
								if((titles[ i ][ GET_LEVEL(ch, i) + 2 ].exp - 1)>0) {
									GET_EXP(ch) = titles[ i ][ GET_LEVEL(ch, i) + 2 ].exp - 1;
								}
								else {
									send_to_char("Non prendi exp perche` oltrepassi la soglia massima!!!",ch);
								}
								return;
							}
						}
					}
				}

				if((GET_EXP(ch) += gain)>0) {
					GET_EXP(ch) += gain;
				}
				else {
					send_to_char("Non prendi exp perche` oltrepassi la soglia massima!!!",ch);
				}

				for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
					if( GET_LEVEL( ch, i ) &&
							GET_LEVEL( ch, i ) < RacialMax[ chrace ][ i ] ) {
						if( GET_EXP( ch ) > titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp ) {
							GET_EXP( ch ) = titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp - 1;
						}
					}
				}
			}
			else { /*IS_NPC*/
				GET_EXP( ch ) += gain;
			}
		}
		else if( gain < 0 ) {
			/*	 gain/=MAX(1,nClassiNonMaxxate); */
			gain/=MAX(1,HowManyClasses(ch));
			if( IS_PC( ch ) ) {
				mudlog( LOG_PLAYERS, "%s ha perso %d punti di esperienza.",
						GET_NAME( ch ), -gain );
				mudlog( LOG_SYSERR, "PKill loss 1");
			}
			GET_EXP( ch ) += gain;
			if(GET_EXP(ch) < 0) {
				GET_EXP(ch) = 0;
			}
		}
	}
}


void gain_exp( struct char_data* ch, int gain ) {
	/* gain completo se negativo, diviso per le classi se positivo
	 * Per le classi che hanno una razza maxxata, consente il gain completo
	 * */

	int i,q;
	char buf[256];
	short chrace;
	short nClassiNonMaxxate=0;
	save_char(ch,AUTO_RENT, 0 );
	gain=gain_corretto(ch,gain);
	if( !IS_PC( ch ) && ch->master && IS_AFFECTED( ch, AFF_CHARM ) ) {
		if( ch->master->in_room == ch->in_room ) {
			if( gain > 1 ) {
				gain /= 2;
				sprintf( buf, "Guadagni i %d punti di esperienza di $N", gain );
				act( buf, 0, ch->master, 0, ch, TO_CHAR );
				gain_exp( ch->master, gain );
			}
		}
		return;
	}

	if( !IS_IMMORTAL( ch ) ) {
		if( gain > 0 ) {

			if( GetMaxLevel( ch ) == 1 ) {
				gain *= 2;
			}

			if( IS_PC( ch ) ) {
				if(ch->desc && ch->desc->original) {
					chrace = ch->desc->original->race;
				}
				else {
					chrace = GET_RACE(ch);
				}
				for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
					/*ALAR:check sui limiti razziali per i  livelli
					* Conta le classi attive
					*/
					if( GET_LEVEL( ch, i ) && GET_LEVEL( ch, i ) < RacialMax[ chrace ][ i ] ) {
						nClassiNonMaxxate++;
					}
				}
				/*	    gain/=MAX(1,nClassiNonMaxxate); */

				gain/=MAX(1,HowManyClasses(ch));

				/* Qui sistemiamo l'overflow dei mono... Flyp*/
				q=GET_EXP( ch );
				q+=gain;
				if (q < 0) {
					mudlog (LOG_PLAYERS, "WARNING: %s EXP OVERFLOW!", GET_NAME(ch) );
				}
				else {
					GET_EXP( ch ) += gain;
				}
				/* Ma i multiclasse mi overflowano ancora... Flyp */

				/* Primo loop, maxa le classi maxate */
				for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
					if( GET_LEVEL( ch, i ) && GET_LEVEL( ch, i ) < RacialMax[ chrace ][ i ] ) {
						if( GET_EXP( ch ) > titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp ) {
							send_to_char( "Devi passare alla tua gilda prima di guadagnare ulteriore esperienza.\n\r", ch );
							GET_EXP( ch ) = titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp - 1;
							mudlog(LOG_SYSERR,"(LIMITS)Maxxa la classe %d a %d",i,
								   (titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp - 1));
						}
					}
				}

				/* Secondo loop, avvisa per il possibile passaggio */
				for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ ) {
					if((GET_LEVEL(ch,i) < BARONE && GET_LEVEL(ch,i) > 0 && ( GET_EXP( ch ) > titles[ i ][ GET_LEVEL(ch,i)+1].exp )) && GET_LEVEL(ch,i)<RacialMax[chrace][i]) {
						sprintf( buf, "Sei sufficientemente esperto per essere un %s\n\r",
								 GET_CLASS_TITLE( ch, i, GET_LEVEL( ch, i ) + 1 ) );
						send_to_char( buf, ch );
						send_to_char( "Devi passare dalla tua gilda per crescere di livello.\n\r", ch );
					}

					/* Aggointa la promozione automatica a Principe se la classe
					   acquisisce l' esperienza necessaria */

					if(GET_LEVEL(ch,i) == BARONE && GET_LEVEL(ch,i) > 0 && GET_EXP(ch)>PRINCEEXP) {
						GET_LEVEL(ch,i)=PRINCIPE;
					}
				}
			}

			else { /*IS_NPC*/
				GET_EXP( ch ) += gain;
			}
		}
		else if( gain < 0 ) {
			if( IS_PC( ch ) ) {
				mudlog( LOG_PLAYERS, "%s ha perso %d punti di esperienza.", GET_NAME( ch ), -gain );
				mudlog( LOG_SYSERR, "PKill loss 2");
			}
			GET_EXP( ch ) += gain;
			if(GET_EXP(ch) < 0) {
				GET_EXP(ch) = 0;
			}
		}
	}
}


void gain_exp_regardless( struct char_data* ch, int gain, int iClass,
						  int iMaxLevel ) {
	int i,q;
	bool is_altered = FALSE;

	save_char( ch, AUTO_RENT, 0 );
	if( !IS_NPC( ch ) ) {
		if( gain >= 0 ) {
			/* Qui vediamo l'overflow per i multi... Flyp */
			q=GET_EXP( ch );
			q+=gain;
			if (q < 0) {
				mudlog (LOG_PLAYERS, "WARNING: %s EXP OVERFLOW!", GET_NAME(ch) );
			}
			/* ...e se non parte il warning, possiamo assegnare gli xp.. Flyp */

			else {
				GET_EXP( ch ) += gain;
				for( i = 0; i < ABS_MAX_LVL && titles[ iClass ][ i ].exp <= GET_EXP( ch ); i++ ) {
					if( i > GET_LEVEL( ch, iClass ) && GET_LEVEL( ch, iClass ) < iMaxLevel ) {
						send_to_char( "Cresci di un livello!\n\r", ch );
						advance_level( ch, iClass );
						is_altered = TRUE;
						mudlog(LOG_SYSERR,"(LIMITS2)Maxxa la classe %d a %d",i,
							   (titles[ i ][ GET_LEVEL( ch, i ) + 2 ].exp - 1));
					}
				}
			}
		}
		else if(gain < 0) {
			GET_EXP(ch) += gain;
	}
		if(GET_EXP(ch) < 0)	{
			GET_EXP(ch) = 0;
		}
	}
	if(is_altered) {
		set_title(ch);
	}
}

void gain_condition(struct char_data* ch,int condition,int value) {
	bool intoxicated=false;
	if(IS_IMMORTAL(ch)) {
		return;
	}


	if(GET_COND(ch, condition)==-1) { /* No change */
		return;
	}

	intoxicated=(GET_COND(ch, DRUNK) > 0);

	GET_COND(ch, condition)  += value;
	if (GetMaxLevel(ch) <=5 and (condition == FULL or condition==THIRST)) {
		GET_COND(ch,condition) = MAX(1,GET_COND(ch,condition));
	}
	else {
		GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
	}
	GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));
	if(GET_COND(ch,condition)) {
		return;
	}

	switch(condition) {
	case FULL : {
		if (!affected_by_spell(ch, SKILL_MIND_OVER_BODY) ) {
			send_to_char("Hai Fame.\n\r",ch);
		}

		return;
	}
	case THIRST : {
		if (!affected_by_spell(ch, SKILL_MIND_OVER_BODY) ) {
			send_to_char("Hai sete.\n\r",ch);
		}
		return;
	}
	case DRUNK : {
		if(intoxicated) {
			send_to_char("Sei sobrio.\n\r",ch);
		}
		return;
	}
	default :
		break;
	}

}


#define VOID_PULL_TIME   30
#define FORCE_RENT_TIME  40
void check_idling(struct char_data* ch) {
	if(IS_LINKDEAD(ch) && ch->specials.timer < 30) {
		ch->specials.timer=30;
	}
	if( ++(ch->specials.timer) == 8 ) {
		do_save(ch, "", 0);

	}
	else if( ch->specials.timer == VOID_PULL_TIME ) {
		if( ch->in_room != NOWHERE && ch->in_room != 0 ) {
			ch->specials.was_in_room = ch->in_room;
			if( ch->specials.fighting ) {
				stop_fighting( ch->specials.fighting );
				stop_fighting( ch );
			}
			act( "$n sparisce nel nulla.", TRUE, ch, 0, 0, TO_ROOM );
			act( "Sei ferm$b da troppo tempo e finisci nel nulla.", FALSE, ch, 0, 0,
				 TO_CHAR );
			char_from_room(ch);
			char_to_room(ch, 1);  /* Into room number 1 */
			ch->specials.timer=0;
			if(IS_POLY(ch)) {
				force_return(ch, "", 1);
			}

		}
	}
	else if( ch->specials.timer == FORCE_RENT_TIME ) {
		struct obj_cost cost;
		if(ch->in_room != NOWHERE) {
			char_from_room(ch);
		}
		char_to_room( ch, 4 );

		mudlog( LOG_CHECK,
				"It is now time to force rent %s ",
				GET_NAME( ch ) );

		if(IS_POLY(ch)) {
			return;
		}

		if(ch->desc) {
			close_socket(ch->desc);
		}

		mudlog( LOG_PLAYERS,
				"%s socket has been closed", GET_NAME( ch ) );
		ch->desc = 0;

		if( recep_offer( ch, NULL, &cost,1 ) ) {
			/* if above fails they lose their EQ!                       */
			/* cost.total_cost = 100;                                   */
			/* but the Players use the feature to avoid to pay their rent
			   so let's leave them to lose the EQ! Gaia 2001             */

			save_obj( ch, &cost, 1 );
		}
		else {
			mudlog( LOG_PLAYERS,
					"%s had a failed recep_offer, they are losing EQ!", // Gaia 2001
					GET_NAME( ch ) );
		}
		extract_char(ch);
	}
}

void ObjFromCorpse( struct obj_data* c) {
	struct obj_data* jj, *next_thing;

	for( jj = c->contains; jj; jj = next_thing ) {
		next_thing = jj->next_content; /* Next in inventory */
		if (jj->in_obj) {
			obj_from_obj(jj);
			if(c->in_obj) {
				obj_to_obj(jj,c->in_obj);
			}
			else if (c->carried_by) {
				obj_to_room(jj,c->carried_by->in_room);
				check_falling_obj(jj, c->carried_by->in_room);
			}
			else if( c->in_room != NOWHERE ) {
				obj_to_room(jj,c->in_room);
				check_falling_obj(jj, c->in_room);
			}
			else {
				assert(FALSE);
			}
		}
		else {
			/*
			 **  hmm..  it isn't in the object it says it is in.
			 **  don't extract it.
			 */
			c->contains = 0;
			mudlog( LOG_SYSERR, "Memory lost in ObjFromCorpse.");
			return;
		}
	}
	extract_obj(c);
}




void ClassSpecificStuff( struct char_data* ch) {
	int addlev ; // Gaia 2001
	/* Elimino slowness e haste*/
	if (affected_by_spell(ch,SPELL_SLOW) || affected_by_spell(ch,SPELL_HASTE)) {
		affect_from_char(ch,SPELL_SLOW);
		affect_from_char(ch,SPELL_HASTE);
		send_to_char("Xanathon ti rimette in equilibrio.",ch);
	}

    reset_original_numattacks(ch);

	/* other stuff.. immunities, etc, are set here */

	if (HasClass(ch, CLASS_MONK)) {
		/*
		 */
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 10) {
			SET_BIT(ch->M_immune, IMM_HOLD);
		}
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 18) {
			SET_BIT(ch->immune, IMM_CHARM);
		}
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 22) {
			SET_BIT(ch->M_immune, IMM_POISON);
		}
		if(OnlyClass(ch, CLASS_MONK) && GET_LEVEL(ch, MONK_LEVEL_IND) > 49) {
			SET_BIT(ch->M_immune, IMM_DRAIN);
		}


	}
	else {
		if (HasClass(ch, CLASS_DRUID)) {
			if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 14) {
				SET_BIT(ch->immune, IMM_CHARM);
			}
			if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 32) {
				SET_BIT(ch->M_immune, IMM_POISON);
			}
		}

		if (HasClass(ch, CLASS_THIEF)) {

			if(OnlyClass(ch, CLASS_THIEF)) {
				GET_CHR(ch)+=1;
			}

			GET_CHR(ch) += GET_LEVEL(ch, THIEF_LEVEL_IND)/10;
		}
	}
}


} // namespace Alarmud

