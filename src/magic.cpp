/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD */
/* $Id: magic.c,v 1.3 2002/03/14 21:48:56 Thunder Exp $ */
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
#include "magic.hpp"
#include "act.info.hpp"
#include "act.obj2.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic2.hpp"
#include "magicutils.hpp"
#include "modify.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "skills.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells2.hpp"
namespace Alarmud {


/* For future use in blinding those with infravision who are fireballed
   or otherwise subjected to lotsa heat quickly in dark rooms. */

void heat_blind(struct char_data* ch) {
	struct affected_type af;
	byte tmp;

	tmp=number(1,4);

	if(!ch) {              /* Dunno if this does anything */
		return;
	}

	if(IS_AFFECTED(ch, AFF_BLIND)) {
		return;                        /* no affect */
	}
	else if((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
			(!IS_AFFECTED(ch, AFF_TRUE_SIGHT)) &&
			(IS_AFFECTED(ch, AFF_INFRAVISION)))  {
		send_to_char("Aaarrrggghhh!!! La $c0015luce$c0007 ti $c0008acceca$c0007!!!\n\r", ch);
		af.type      = SPELL_BLINDNESS;
		af.location  = APPLY_HITROLL;
		af.modifier  = -4;  /* Make hitroll worse */
		af.duration  = tmp;
		af.bitvector = AFF_BLIND;
		affect_to_char(ch, &af);
	}
}


/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);

	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(static_cast<int>((level / 2))+1,4)+(level / 2)+1;

	if(affected_by_spell(victim,SPELL_SHIELD)) {
		dam = 0;
	}

	MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE, 5);
}



void spell_chill_touch(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = number(level, 3*level);

	if(!saves_spell(victim, SAVING_SPELL)) {
		af.type      = SPELL_CHILL_TOUCH;
		af.duration  = 6;
		af.modifier  = -1;
		af.location  = APPLY_STR;
		af.bitvector = 0;
		affect_join(victim, &af, TRUE, FALSE);
	}
	else {
		dam >>= 1;
	}
	damage(ch, victim, dam, SPELL_CHILL_TOUCH, 5);
}

void spell_burning_hands(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(1,4) + level/2 + 1;

	send_to_char("Dalle tue mani si sprigionano lingue di $c0001fuoco$c0007!\n\r", ch);
	act("Ad un gesto di $n lingue di $c0001fuoco$c0007 scaturiscono dalle sue mani!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);

	for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = temp) {
		temp = tmp_victim->next_in_room;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL) && (!IS_NPC(tmp_victim))) { // SALVO non deve agire su un immortale
				return;
			}
			if(!in_group(ch, tmp_victim)) {
				act("$c0001Vieni raggiunt$b dalle fiamme!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				heat_blind(tmp_victim);
				if(saves_spell(tmp_victim, SAVING_SPELL)) {
					dam = 0;
				}
				MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS, 5);
			}
			else {
				act("Fortunatamente riesci ad evitare le $c0001fiamme$c0007.\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				heat_blind(tmp_victim);
			}
		}
	}
}



void spell_shocking_grasp(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = number(1,8)+level;

	if(GET_HIT(victim) < -4 && IsHumanoid(victim) &&
			!IsUndead(victim)) {
		act("$n pronuncia le parole '$c0012clear$c0007', e tocca il torace di $N.",
			FALSE,ch, 0, victim, TO_ROOM);
		GET_HIT(victim) +=dam;
		alter_hit(victim,0);
		update_pos(victim);
		return;
	}

	if(!HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
		dam = 0;
	}

	damage(ch, victim, dam, SPELL_SHOCKING_GRASP, 5);
}



void spell_lightning_bolt(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(level,6);

	if(saves_spell(victim, SAVING_SPELL)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT, 5);
}



void spell_colour_spray(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = 4 * level;

	if(saves_spell(victim, SAVING_SPELL)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY, 5);

}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int dam;
	int tmp;
	void set_title(struct char_data *ch);
	void gain_exp(struct char_data *ch, int gain);

	assert(victim && ch);
	if(IS_PC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch)) {
		actall("Cerchi invano di risucchiare l'energia vitale di",
			   "cerca invano di risucchiarti l'energia vitale",
			   "cerca di risucchiare l'energia vitale di",
			   ch,victim);
		return;
	}

	if(level < 0 || level > ABS_MAX_LVL) {
		return;
	}

	if(!saves_spell(victim, SAVING_SPELL)) {
		if(!IS_IMMORTAL(ch)) {
			GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)-200);
		}

		if(GetMaxLevel(victim) <= 1) {
			damage(ch, victim, 100, SPELL_ENERGY_DRAIN, 5); /* Kill the sucker */
		}
		else if((!IS_NPC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
			send_to_char("Un misero mortale cerca di risucchiare la tua energia...\n\r",victim);
		}
		else { /* Ne' primo livello, ne' immortale */
			if(!IS_SET(victim->M_immune, IMM_DRAIN) &&
					(!IS_SET(victim->immune, IMM_DRAIN) ||
					 !saves_spell(victim, SAVING_SPELL))) {
				/* Fallito secondo tiro salvezza */
				send_to_char("$c0008La tua energia vitale viene risucchiata!\n\r", victim);
				dam = 4;
				damage(ch, victim, dam, SPELL_ENERGY_DRAIN, 5);
				if(IS_PC(victim)) {
					GET_EXP(victim)=GET_EXP(victim)- (static_cast<int>(GET_EXP(victim))/10);
					/*if ( GET_EXP(victim)>=200000000 )
					{
					       if (HowManyClasses(victim) == 1)
					          par = 50000000;
					       else if (HowManyClasses(victim) == 2)
					          par = 37500000;
					       else if (HowManyClasses(victim) == 3)
					          par = 33333333;
					   GET_EXP(victim)=GET_EXP(victim)- par;
					}
					else
					{
					   drop_level(victim, BestClassBIT(victim),FALSE);
					   set_title(victim);
					}*/

				}
				else {
					/* ATTENZIONE Se victim e' morta ci potrebbero essere dei problemi */
					tmp = GET_MAX_HIT(victim)/GetMaxLevel(victim);
					victim->points.max_hit -=tmp;
					victim->points.hit -=tmp;
					tmp = GET_EXP(victim)/GetMaxLevel(victim);
					GET_EXP(ch)+=tmp;
					GET_EXP(victim)-=tmp;
					victim->points.hitroll+=1;
				}
			}/* Tiro salvezza riuscito  */
			else {
				if(!IS_SET(ch->M_immune, IMM_DRAIN)) {
					send_to_char("Il tuo incantesimo ti si ritorce contro!\n\r",ch);
					dam = 1;
					damage(ch, victim, dam, SPELL_ENERGY_DRAIN, 5);
				}
				else {
					send_to_char("Il tuo incantesimo fallisce miseramente.\n\r",ch);
				}
			}
		}
	}
	else { /* Azzeccato primo tiro salvezza */
		damage(ch, victim, 0, SPELL_ENERGY_DRAIN, 5); /* Miss */
	}
}



void spell_fireball(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(level,8);
	for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if(!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
				if(saves_spell(tmp_victim, SAVING_SPELL)) {
					dam >>= 1;
				}
				heat_blind(tmp_victim);
				MissileDamage(ch, tmp_victim, dam, SPELL_FIREBALL, 5);
			}
			else {
				act("Riesci ad evitare la $c0001massa infuocata$c0007!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				heat_blind(tmp_victim);
			}
		}
		else {
			if(tmp_victim->in_room != NOWHERE) {
				if(real_roomp(ch->in_room)->zone ==
						real_roomp(tmp_victim->in_room)->zone) {
					send_to_char("Senti un $c0008boato$c0007 ed un soffio di aria $c0001rovente$c0007 sul volto.\n\r", tmp_victim);
				}
			}
		}
	}
}


void spell_earthquake(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	int dam;

	struct char_data* tmp_victim, *temp;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam =  dice(1,4) + level + 1;

	send_to_char("$c0003La terra sotto di te inizia a tremare!\n\r", ch);
	act("$c0003Ad un gesto di $n $c0003la terra trema e si squote.",
		FALSE, ch, 0, 0, TO_ROOM);

	for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if(!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {

				if(GetMaxLevel(tmp_victim) > 4) {
					act("Cadi a terra e ti fai male!!\n\r",
						FALSE, ch, 0, tmp_victim, TO_VICT);
					MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE, 5);
				}
				else {
					act("Vieni inghiottit$b da una $c0003voragine$c0007 che si apre sotto i tuoi piedi!", FALSE,
						ch, 0, tmp_victim, TO_VICT);
					act("La terra si apre ed inghiotte $N!", FALSE,
						ch, 0, tmp_victim, TO_NOTVICT);
					MissileDamage(ch, tmp_victim, GET_MAX_HIT(tmp_victim)*12,
								  SPELL_EARTHQUAKE, 5);
				}
			}
			else {
				act("Per poco non cadi facendoti male!!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
		else {
			if(real_roomp(ch->in_room)->zone ==
					real_roomp(tmp_victim->in_room)->zone) {
				send_to_char("$c0003La terra trema...\n\r", tmp_victim);
			}
		}
	}
}



void spell_dispel_evil(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam=1;
	assert(ch && victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(IS_PC(victim) || IS_POLY(victim)) {
		send_to_char("Non riesci a scacciarlo.\n\r", ch);
		return;
	}


	if(IsExtraPlanar(victim)) {
		if(IS_EVIL(ch)) {
			send_to_char("Non hai lo $c0015spirito$c0007 adatto per scacciare una $c0001creatura malvagia$c0007.\n\r", ch);
			return;
		}
		else {
			if(IS_GOOD(victim)) {
				act("La sua $c0014bonta'$c0007 protegge $N.", FALSE, ch, 0, victim, TO_CHAR);
				return;
			}
		}
		if(!saves_spell(victim, SAVING_SPELL)) {
			act("$c0014$n scaccia $N $c0014da questo piano di esistenza.", TRUE, ch, 0, victim, TO_ROOM);
			act("$c0014Scacci $N $c0014da questo piano di esistenza.", TRUE, ch, 0, victim, TO_CHAR);
			act("$c0014$n ti scaccia da questo piano di esistenza.", TRUE, ch, 0, victim,TO_VICT);
			gain_exp(ch, MIN(GET_EXP(victim)/2, 150000));
			extract_char(victim);
		}
		else {
			act("$N resiste all'attacco.",TRUE, ch, 0, victim, TO_CHAR);
			act("Resisti all'attacco di $n.", TRUE, ch, 0, victim, TO_VICT);
			damage(ch, victim, dam, SPELL_EARTHQUAKE, 5);
		}
	}
	else {
		act("$N ti ride in faccia.", TRUE, ch, 0, victim, TO_CHAR);
		act("$N ride in faccia a $n.", TRUE,ch, 0, victim, TO_NOTVICT);
		act("Ridi in faccia a $n.", TRUE,ch,0,victim,TO_VICT);
	}
}


void spell_call_lightning(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(level+2, 8);

	if(OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
		if(saves_spell(victim, SAVING_SPELL)) {
			dam >>= 1;
		}

		MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING, 5);
	}
	else {
		send_to_char("Non ci sono le condizioni atmosferiche adeguate.\n\r", ch);
		return;
	}
}



void spell_harm(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = GET_HIT(victim) - dice(1,4);

	if(dam < 0) {
		dam = 100;    /* Kill the suffering bastard */
	}
	else {
		if(GET_RACE(ch) == RACE_GOD) {
			dam = 0;
		}
		if(!HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
			dam = 0;
		}
	}
	dam = MIN(dam, 100);

	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
		GET_ALIGNMENT(ch)-=4;
	}

	damage(ch, victim, dam, SPELL_HARM, 5);
}



/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_ARMOR)
			&& !affected_by_spell(victim, SPELL_BARKSKIN)) { // SALVO non si puo' fare armor
		af.type      = SPELL_ARMOR;
		af.duration  = level / 2;
		af.modifier  = -20;
		af.location  = APPLY_AC;
		af.bitvector = 0;

		affect_to_char(victim, &af);
		send_to_char("Senti scendere su di te una $c0011protezione Divina$c0007.\n\r", victim);
	}
	else {
		send_to_char("Non sembra accadere nulla.\n\r", ch);
	}
}

#define ASTRAL_ENTRANCE   2701

void spell_astral_walk(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct char_data* tmp, *tmp2;
	struct room_data* rp;

	if(IS_SET(SystemFlags, SYS_NOASTRAL)) {
		send_to_char("$c0012I piani astrali sono in movimento, non puoi raggiungerli!\n",ch);
		return;
	}

	rp = real_roomp(ch->in_room);
	if(IS_SET(rp->room_flags,NO_ASTRAL)) {
		send_to_char("Non riesci a raggiungere i $c0012piani astrali$c0007, sono troppo lontani!\n",ch);
		return;
	}
	for(tmp = rp->people; tmp; tmp = tmp2) {
		tmp2 = tmp->next_in_room;
		if(in_group_strict(ch, tmp) && !tmp->specials.fighting) {
			act("$c0008$n $c0008sbiadisce e scompare.", FALSE, tmp, 0, 0, TO_ROOM);
			char_from_room(tmp);
			char_to_room(tmp, ASTRAL_ENTRANCE);
			do_look(tmp, "", 15);
			act("$c0008$n $c0008passa lentamente in questo piano di esistenza.", FALSE, tmp,
				NULL, NULL, TO_ROOM);
		}
	}
}

void spell_teleport(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	int to_room, iTry = 0;
	struct room_data* room;

	assert(ch && victim);

	if(victim != ch) {
		if(saves_spell(victim,SAVING_SPELL)) {
			send_to_char("Il tuo incantesimo non ha effetto.\n\r",ch);
			if(IS_NPC(victim)) {
				if(!victim->specials.fighting) {
					set_fighting(victim, ch);
				}
			}
			else {
				send_to_char("Hai una strana $c0013sensazione$c0007, ma passa presto.\n\r",victim);
			}
			return;
		}
		else {
			ch = victim;  /* the character (target) is now the victim */
		}
	}

	if(!IsOnPmp(victim->in_room)) {
		send_to_char("Sei in un piano extra-dimensionale!\n\r", ch);
		return;
	}


	do {
		to_room = number(0, top_of_world);
		room = real_roomp(to_room);
		if(room) {
			if((IS_SET(room->room_flags, PRIVATE)) ||
					(IS_SET(room->room_flags, DEATH) && IS_NPC(victim)) ||
					(IS_SET(room->room_flags, TUNNEL)) ||
					(IS_SET(room->room_flags, NO_SUM)) ||
					(IS_SET(room->room_flags, NO_MAGIC)) ||
					!IsOnPmp(to_room) ||
					((room->number >= 34000) && (room->number <= 34999))
			  ) {
				room = 0;
				iTry++;
			}
		}

	}
	while(!room && iTry < 10);

	if(iTry >= 10) {
		send_to_char("La magia fallisce.\n\r", ch);
		return;
	}

	act("$n scompare lentamente dalla vista.", FALSE, ch,0,0,TO_ROOM);
	char_from_room(ch);
	mudlog(LOG_CHECK,"teleport %s to_room %ld", GET_NAME(ch), to_room); // SALVO loggo il teleport
	char_to_room(ch, to_room);
	act("$n torna $c0008visibile$c0007 un po' per volta.", FALSE, ch,0,0,TO_ROOM);

	do_look(ch, "", 15);

	check_falling(ch);

}



void spell_bless(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && (victim || obj));
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(obj) {
		if((5*GET_LEVEL(ch,CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
				(GET_POS(ch) != POSITION_FIGHTING) &&
				!IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
			act("$p $c0015brilla$c0007 per un istante.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
	else {

		if((GET_POS(victim) != POSITION_FIGHTING) &&
				(!affected_by_spell(victim, SPELL_BLESS))) {

			send_to_char("Senti scendere su di te la $c0015benedizione Divina$c0007.\n\r", victim);
			af.type      = SPELL_BLESS;
			af.duration  = 6;
			af.modifier  = 1;
			af.location  = APPLY_HITROLL;
			af.bitvector = 0;
			affect_to_char(victim, &af);

			af.location = APPLY_SAVING_SPELL;
			af.modifier = -1;                 /* Make better */
			affect_to_char(victim, &af);
		}
	}
}



void spell_blindness(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}


	if(saves_spell(victim, SAVING_SPELL) ||
			affected_by_spell(victim, SPELL_BLINDNESS)) {
		return;
	}

	act("Sembra che $n sia diventat$b $c0008ciec$b$c0007!", TRUE, victim, 0, 0, TO_ROOM);
	act("Sei $c0008ciec$b$c0007! Non vedi piu' $c0008nulla$c0007!\n\r", FALSE, victim, NULL, NULL, TO_CHAR);

	af.type      = SPELL_BLINDNESS;
	af.location  = APPLY_HITROLL;
	af.modifier  = -4;  /* Make hitroll worse */
	af.duration  = level / 2;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);


	af.location = APPLY_AC;
	af.modifier = +20; /* Make AC Worse! */
	affect_to_char(victim, &af);

	if(!victim->specials.fighting && victim != ch) {
		set_fighting(victim, ch);
	}

}



void spell_clone(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {

	assert(ch && (victim || obj));
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	send_to_char("Clone is not ready yet.", ch);

	if(obj) {

	}
	else {
		/* clone_char(victim); */
	}
}



void spell_control_weather(byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj) {
	/* Control Weather is not possible here!!! */
	/* Better/Worse can not be transferred     */
}



void spell_create_food(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct obj_data* tmp_obj;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	tmp_obj->name = static_cast<char*>(strdup("fungo"));
	tmp_obj->short_description =  static_cast<char*>(strdup("un fungo magico"));
	tmp_obj->description =  static_cast<char*>(strdup("Un delizioso fungo magico e' qui a terra."));

	tmp_obj->obj_flags.type_flag = ITEM_FOOD;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
	tmp_obj->obj_flags.value[0] = 5+level;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 10;
	tmp_obj->obj_flags.cost_per_day = 1;

	tmp_obj->next = object_list;
	object_list = tmp_obj;

	obj_to_room(tmp_obj,ch->in_room);

	tmp_obj->item_number = -1;

	act("$p appare improvvisamente.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p appare improvvisamente.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int water;
	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

	assert(ch && obj);

	if(GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if((obj->obj_flags.value[2] != LIQ_WATER)
				&& (obj->obj_flags.value[1] != 0)) {

			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);

		}
		else {

			water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

			/* Calculate water it can contain, or water created */
			water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

			if(water > 0) {
				obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;

				weight_change_object(obj, water);

				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("Hai riempito d'$c0012acqua$c0007 $p.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}



void spell_cure_blind(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {

	if(!victim || level <0 || level > ABS_MAX_LVL) {
		return;
	}

	if(IS_AFFECTED(victim, AFF_BLIND)) {
		REMOVE_BIT(victim->specials.affected_by, AFF_BLIND);
		send_to_char("Torni a $c0015vedere$c0007 di nuovo!\n\r", victim);
	}

	if(affected_by_spell(victim, SPELL_BLINDNESS)) {
		affect_from_char(victim, SPELL_BLINDNESS);
		send_to_char("Torni a $c0015vedere$c0007 di nuovo!\n\r", victim);
	}

	/* ACIDUS 2003 l'heal su se stessi non fa cambiare align */
	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch) && !(ch == victim)) {
		GET_ALIGNMENT(ch)+=1;
	}

}



void spell_cure_critic(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int healpoints;

	if(!victim) {
		send_to_char("Chi vuoi curare?",ch);
		mudlog(LOG_SYSERR,"Cure critic faile check");
		return;
	}

	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	healpoints = dice(3,8)+3;

	if((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
		GET_HIT(victim) = hit_limit(victim);
		alter_hit(victim,0);
	}
	else {
		GET_HIT(victim) += healpoints;
		alter_hit(victim,0);
	}

	send_to_char("Ti senti decisamente meglio!\n\r", victim);

	update_pos(victim);
	/*
	  if (IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL( ch ) )
	    GET_ALIGNMENT(ch)+=3;
	*/
}


void spell_cure_light(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	int healpoints;
	if(!victim) {
		send_to_char("Chi vuoi curare?",ch);
		mudlog(LOG_SYSERR,"Cure light failed check");
		return;
	}

	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	healpoints = dice(1,8);

	if((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
		GET_HIT(victim) = hit_limit(victim);
		alter_hit(victim,0);
	}
	else {
		GET_HIT(victim) += healpoints;
		alter_hit(victim,0);
	}

	send_to_char("Ti senti leggermente meglio!\n\r", victim);

	update_pos(victim);
	/*
	  if (IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL( ch ) )
	    GET_ALIGNMENT(ch)+=1;
	*/
}





void spell_curse(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim || obj);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(obj) {
		SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
		SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

		/* LOWER ATTACK DICE BY -1 */
		if(obj->obj_flags.type_flag == ITEM_WEAPON) {
			obj->obj_flags.value[2]--;
		}
		act("$p $c0015brilla$c0007 per un attimo di $c0009luce rossa$c0007.", FALSE, ch, obj, 0, TO_CHAR);
	}
	else {
		if(saves_spell(victim, SAVING_SPELL) ||
				affected_by_spell(victim, SPELL_CURSE)) {
			return;
		}

		af.type      = SPELL_CURSE;
		af.duration  = 24*7;       /* 7 Days */
		af.modifier  = -1;
		af.location  = APPLY_HITROLL;
		af.bitvector = AFF_CURSE;
		affect_to_char(victim, &af);

		af.location = APPLY_SAVING_PARA;
		af.modifier = 1; /* Make worse */
		affect_to_char(victim, &af);

		act("$n viene avvolto per un attimo da un $c0009alone rosso$c0007!", FALSE, victim, 0, 0, TO_ROOM);
		act("$c0009Ti senti molto a disagio.",FALSE,victim,0,0,TO_CHAR);
		if(IS_NPC(victim) && !victim->specials.fighting) {
			set_fighting(victim,ch);
		}

		if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
			GET_ALIGNMENT(ch)-=2;
		}

	}
}



void spell_detect_evil(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(affected_by_spell(victim, SPELL_DETECT_EVIL)) {
		return;
	}

	af.type      = SPELL_DETECT_EVIL;
	af.duration  = level*5;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;

	affect_to_char(victim, &af);

	act("Gli occhi di $n $c0015brillano$c0007 per un attimo di una $c0015luce bianca$c0007.", FALSE, victim, 0, 0, TO_ROOM);
	send_to_char("I tuoi occhi $c0015brillano$c0007 per un attimo di una $c0015luce bianca$c0007.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data* ch,
							   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(affected_by_spell(victim, SPELL_DETECT_INVISIBLE)) {
		return;
	}

	af.type      = SPELL_DETECT_INVISIBLE;
	af.duration  = level*5;
	af.modifier  = level;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVISIBLE;

	affect_to_char(victim, &af);
	act("Gli occhi di $n $c0015brillano$c0007 per un attimo di una $c0011luce gialla$c0007.", FALSE, victim, 0, 0, TO_ROOM);
	send_to_char("I tuoi occhi $c0015brillano$c0007 per un attimo di una $c0011luce gialla$c0007.\n\r", victim);
}



void spell_detect_magic(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	if(affected_by_spell(victim, SPELL_DETECT_MAGIC)) {
		return;
	}

	af.type      = SPELL_DETECT_MAGIC;
	af.duration  = level*5;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;

	affect_to_char(victim, &af);
    act("Gli occhi di $n $c0015brillano$c0007 per un attimo di una $c0012luce blu$c0007.", FALSE, victim, 0, 0, TO_ROOM);
	send_to_char("I tuoi occhi $c0015brillano$c0007 per un attimo di una $c0012luce blu$c0007.\n\r", victim);
}



#if 1
void spell_detect_poison(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	assert(ch && (victim || obj));

	if(victim) {
		if(victim == ch)
			if(IS_AFFECTED(victim, AFF_POISON)) {
				send_to_char("Senti che un $c0010veleno$c0007 mortale circola nel tuo $c0001sangue$c0007.\n\r", ch);
			}
			else {
				send_to_char("Ti senti a posto.\n\r", ch);
			}
		else if(IS_AFFECTED(victim, AFF_POISON)) {
			act("Percepisci che in $E e' presente un $c0010veleno$c0007.",FALSE,ch,0,victim,TO_CHAR);
		}
		else {
			act("Non percepisci alcun $c0010veleno$c0007 scorrere nel $c0001sangue$c0007 di $E.",FALSE,ch,0,victim,TO_CHAR);
		}
	}
	else {   /* It's an object */
		if((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
				(obj->obj_flags.type_flag == ITEM_FOOD)) {
			if(obj->obj_flags.value[3]) {
				act("Percepisci il $c0010veleno$c0007 presente.",FALSE, ch, 0, 0, TO_CHAR);
			}
			else {
				send_to_char("Sembra perfettamente commestibile.\n\r", ch);
			}
		}
	}
}
#else
void spell_detect_poison(byte level, struct char_data* ch, struct char_data* victim, struct obj_data* obj) {
	assert(ch && (victim || obj));

	if(victim) {
		if(victim == ch)
			if(IS_AFFECTED(victim, AFF_POISON)) {
				send_to_char("Senti il $c0010veleno$c0007 circolare nel tuo $c0001sangue$c0007!\n\r", ch);
			}
			else {
				send_to_char("Non senti alcun $c0010veleno$c0007 nel tuo $c0001sangue$c0007.\n\r", ch);
			}
		else if(IS_AFFECTED(victim, AFF_POISON)) {
			act("Senti che $E e' $c0010avvelenat$b$c0007.",FALSE,ch,0,victim,TO_CHAR);
		}
		else {
			act("Non senti alcun $c0010veleno$c0007 nel $c0001sangue$c0007 di $E.",FALSE,ch,0,victim,TO_CHAR);
		}
	}
	else {   /* It's an object */
		if((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
				(obj->obj_flags.type_flag == ITEM_FOOD)) {
			if(obj->obj_flags.value[3]) {
				act("Riveli la presenza di un $c0010veleno$c0007.",FALSE, ch, 0, 0, TO_CHAR);
			}
			else {
				send_to_char("Non senti la presenza di alcun $c0010veleno$c0007.\n\r", ch);
			}
		}
	}
}
#endif



void spell_enchant_weapon(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int i;
	int count=0;

	assert(ch && obj);
	assert(MAX_OBJ_AFFECT >= 2);

	if((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
			!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

		for(i=0; i < MAX_OBJ_AFFECT; i++) {
			if(obj->affected[i].location == APPLY_NONE) {
				count++;
			}
			if(obj->affected[i].location == APPLY_HITNDAM ||
					obj->affected[i].location == APPLY_HITROLL ||
					obj->affected[i].location == APPLY_DAMROLL) {
				return;
			}
		}

		if(count < 2) {
			return;
		}
		/*  find the slots */
		i = getFreeAffSlot(obj);

		SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

		obj->affected[i].location = APPLY_HITROLL;
		obj->affected[i].modifier = 1;
		if(level > 20) {
			obj->affected[i].modifier += 1;
		}
		if(level > 40) {
			obj->affected[i].modifier += 1;
		}
		if(level > MAX_MORT) {
			obj->affected[i].modifier += 1;
		}
		if(level >= MAESTRO_DEL_CREATO) {
			obj->affected[i].modifier += 1;
		}

		i = getFreeAffSlot(obj);

		obj->affected[i].location = APPLY_DAMROLL;
		obj->affected[i].modifier = 1;
		if(level > 15) {
			obj->affected[i].modifier += 1;
		}
		if(level > 30) {
			obj->affected[i].modifier += 1;
		}
		if(level > MAX_MORT) {
			obj->affected[i].modifier += 1;
		}
		if(level >=MAESTRO_DEL_CREATO) {
			obj->affected[i].modifier += 1;
		}

		if(IS_GOOD(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL);
			act("Un $c0012alone blu$c0007 avvolge per un attimo $p.",FALSE,ch,obj,0,TO_CHAR);
		}
		else if(IS_EVIL(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL);
			act("Un $c0009alone rosso$c0007 avvolge per un attimo $p.",FALSE,ch,obj,0,TO_CHAR);
		}
		else {
			act("Un $c0011alone giallo$c0007 avvolge per un attimo $p.",FALSE,ch,obj,0,TO_CHAR);
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL);
		}
	}
}


void spell_heal(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {

	if(!victim) {
		send_to_char("Chi vuoi curare?",ch);
		mudlog(LOG_SYSERR,"Heal failde check");
		return;
	}


	spell_cure_blind(level, ch, victim, obj);

	GET_HIT(victim) +=100;
	alter_hit(victim,0);

	if(GET_HIT(victim) >= hit_limit(victim)) {
		GET_HIT(victim) = hit_limit(victim)-dice(1,4);
		alter_hit(victim,0);
	}
	update_pos(victim);

    if(ch != victim)
    {
        act("$c0015Curi $N.",FALSE,ch,0,victim,TO_CHAR);
    }
	send_to_char("$c0015Senti un caldo formicolio pervadere il tuo corpo.\n\r", victim);

	/* ACIDUS 2003 l'heal su se stessi non fa cambiare align */
	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch) && !(ch == victim)) {
		GET_ALIGNMENT(ch) += 5;
	}

}


void spell_invisibility(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	int durata=0;
	assert((ch && obj) || victim);

	if(obj) {
		if(!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
			act("$c0011$p $c0011diventa invisibile.",FALSE,ch,obj,0,TO_CHAR);
			act("$c0011$p $c0011diventa invisibile.",TRUE,ch,obj,0,TO_ROOM);
			SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}
	}
	else {                /* Then it is a PC | NPC */
		if(!affected_by_spell(victim, SPELL_INVISIBLE)) {

			act("$c0011$n $c0011scompare lentamente dalla vista.", TRUE, victim,0,0,TO_ROOM);
			send_to_char("$c0011Svanisci.\n\r", victim);
			durata=(GET_AVE_LEVEL(victim)/2)+number(0,6)-3;

			af.type      = SPELL_INVISIBLE;
			af.duration  = durata;
			af.modifier  = -(40+level);
			af.location  = APPLY_AC;
			af.bitvector = AFF_INVISIBLE;
			affect_to_char(victim, &af);
			af.type      = SPELL_INVISIBLE;
			af.duration  = durata;
			af.modifier  = level;
			af.location  = APPLY_NONE;
			af.bitvector = 0;
			affect_to_char(victim, &af);
		}
	}
}


#if 1
void spell_locate_object(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct char_data* k;
	struct obj_data* i;
	char name[256], buf2[256];
	char buf[MAX_STRING_LENGTH];
	int j;
	bool isgod = FALSE;
	assert(ch);

	if(!obj) {
		send_to_char("Ovunque, li percepisci ovunque!?!\n\r",ch);
		return;
	}

	if(!obj->name || !(*obj->name)) {
		send_to_char("Cosa stai cercando??\n\r", ch);
		return;
	}
	isgod=IS_CREATORE(ch);

	strcpy(name, obj->name);

	j=level>>1;

	sprintf(buf,"%s","");
	for(i = object_list; i && (j>0); i = i->next) {
		k=(struct char_data*)NULL;
		sprintf(buf2,"%s","");
		if(isname(name, i->name)) {
			if((k=i->carried_by)) {
				if(strlen(PERS(i->carried_by, ch))>0) {
					sprintf(buf2,"$c0015%s$c0007 fra gli oggetti trasportati da $c0015%s$c0007.\n\r",
							i->short_description,PERS(i->carried_by,ch));
				}
			}
			else if((k=i->equipped_by)) {
				if(strlen(PERS(i->equipped_by, ch))>0) {
					sprintf(buf2,"$c0015%s$c0007 addosso a $c0015%s$c0007.\n\r",
							i->short_description,PERS(i->equipped_by,ch));
				}
			}
			else if(i->in_obj) {
				sprintf(buf2,"$c0015%s$c0007 dentro $c0015%s$c0007.\n\r",i->short_description,
						i->in_obj->short_description);
			}
			else
				sprintf(buf2,"$c0015%s$c0007 in $c0015%s$c0007.\n\r",i->short_description,
						(i->in_room == NOWHERE ? "uso da qualche parte non meglio precisata." : real_roomp(i->in_room)->name));
			j--;
		}/* if isname */
		if((isgod || (k && !IS_DIO_MINORE(k)))
#ifdef ZONE_LOCATE
				&& (
					IS_IMMORTAL(ch) ||
					real_roomp(ch->in_room)->zone ==
					real_roomp(i->in_room)->zone
				)
#endif
		  ) {
			strncat(buf,buf2,MAX_STRING_LENGTH);
		}
	}/* fine for */
	page_string(ch->desc, buf, 0);

	if(j==0) {
		send_to_char("Hai molta confusione in testa.\n\r",ch);
	}
	if(j==level>>1) {
		send_to_char("Non trovi nulla di simile.\n\r",ch);
	}
}
#else
void spell_locate_object(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct obj_data* i;
	char name[256];
	char buf[MAX_STRING_LENGTH],buf2[256];
	int j;

	assert(ch);

	if(!obj) {
		send_to_char("Ovunque, senti la loro presenza ovunque!?!\n\r",ch);
		return;
	}

	if(!obj->name || !(*obj->name)) {
		send_to_char("Si, ma che cosa?\n\r", ch);
		return;
	}


	strcpy(name, obj->name);

	j=level>>2;
	if(j<2) {
		j=2;
	}

	buf[ 0 ] = 0;

	for(i = object_list; i && (j>0); i = i->next)
		if(isname(name, i->name)) {
			if(i->carried_by) {
				if(strlen(PERS(i->carried_by, ch))>0) {
					sprintf(buf2,"$c0015%s$c0007 fra gli oggetti trasportati da $c0015%s$c0007.\n\r",
							i->short_description,PERS(i->carried_by,ch));
					strcat(buf,buf2);
				}
			}
			else if(i->equipped_by) {
				if(strlen(PERS(i->equipped_by, ch))>0) {
					sprintf(buf2,"$c0015%s$c0007 addosso a $c0015%s$c0007.\n\r",
							i->short_description,PERS(i->equipped_by,ch));
					strcat(buf,buf2);
				}
			}
			else if(i->in_obj) {
				sprintf(buf2,"$c0015%s$c0007 dentro $c0015%s$c0007.\n\r",i->short_description,
						i->in_obj->short_description);
				strcat(buf,buf2);
			}
			else {
				sprintf(buf2,"$c0015%s$c0007 in $c0015%s$c0007.\n\r",i->short_description,
						(i->in_room == NOWHERE ? "uso non si sa dove." : real_roomp(i->in_room)->name));
				strcat(buf,buf2);
				j--;
			}
		}

	page_string(ch->desc,buf,0);

	if(j==0) {
		send_to_char("Hai molta confusione in testa.\n\r",ch);
	}
	if(j==level>>1) {
		send_to_char("Non trovi nulla di simile.\n\r",ch);
	}
}


#endif


void spell_poison(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim || obj);

	if(check_peaceful(victim, "")) { // SALVO metto il controllo peaceful
		return;
	}
	if(victim) {
		if(IS_SET(victim->M_immune,IMM_POISON)  /* should fix imm_poison bug */
		  ) {
			/* find better place later   */
			send_to_char("Il $c0010veleno$c0007 non sembra sortire alcun effetto!\n\r",victim);
			return;
		}

		if(IS_NPC(ch)) {
			if(!IS_SET(ch->specials.act, ACT_DEADLY)) {
				if(!ImpSaveSpell(victim, SAVING_PARA, 0)) {
					af.type = SPELL_POISON;
					af.duration = level*2;
					af.modifier = -2;
					af.location = APPLY_STR;
					af.bitvector = AFF_POISON;
					affect_join(victim, &af, FALSE, FALSE);
					send_to_char("$c0003Ti senti molto male.\n\r", victim);
					GET_HIT(victim) -= 1 ; // Gaia 2001
					alter_hit(victim, 0);    // Gaia 2001
					if(!IS_PC(victim)) {
						AddHated(victim, ch);
					}
					if(!victim->specials.fighting) {
						set_fighting(victim, ch);
					}
				}
				else {
					FailPoison(victim,ch);
					return;
				}
			}
			else {
				if(!ImpSaveSpell(victim, SAVING_PARA, 0)) {
					act("$c0010Un veleno mortale entra nelle tue vene.", FALSE, victim,
						0, 0, TO_CHAR);
					if(!IS_PC(victim)) {
						AddHated(victim, ch);
					}
					damage(victim, victim, MAX(10, GET_HIT(victim)*2), SPELL_POISON, 5);
				}
				else {
					FailPoison(victim,ch);
					return;
				}
			}
		}
		else {
			if(!ImpSaveSpell(victim, SAVING_PARA, 0)) {
				af.type = SPELL_POISON;
				af.duration = level*2;
				af.modifier = -2;
				af.location = APPLY_STR;
				af.bitvector = AFF_POISON;
				affect_join(victim, &af, FALSE, FALSE);

				send_to_char("$c0003Improvvisamente ti senti molto male.\n\r", victim);
                act("$c0003Sembra che $N $c0003si senta molto male.", FALSE, ch, 0,victim, TO_ROOM);
				GET_HIT(victim) -= 1 ; // Gaia 2001
				alter_hit(victim, 0);    // Gaia 2001
				if(!IS_PC(victim)) {
					AddHated(victim, ch);
				}
				if(!victim->specials.fighting && !IS_PC(victim)) {
					set_fighting(victim, ch);
				}
			}
			else {
				FailPoison(victim,ch);
			}
		}
	}
	else if(obj) {
		/* Object poison */
		if(obj->obj_flags.type_flag == ITEM_DRINKCON) {
			SET_BIT(obj->obj_flags.value[3], DRINK_POISON);
		}
		else if(obj->obj_flags.type_flag == ITEM_FOOD) {
			obj->obj_flags.value[3] = 1;
		}
	}
	else {
		mudlog(LOG_SYSERR, "!victim and !obj in spell_poison (magic.c)");
	}
}


void spell_protection_from_evil(byte level, struct char_data* ch,
								struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) && !affected_by_spell(victim, SPELL_PROT_FROM_EVIL_GROUP)) {
		af.type      = SPELL_PROTECT_FROM_EVIL;
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_PROTECT_FROM_EVIL;
		affect_to_char(victim, &af);
		send_to_char("$c0014Senti scendere su di te una protezione contro il Male!\n\r", victim);
	}
}

void spell_protection_from_evil_group(byte level, struct char_data* ch,
									  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	struct char_data* dude;

	for(dude=real_roomp(ch->in_room)->people; dude; dude=dude->next_in_room)
		if(IS_FOLLOWING(ch,dude)) {
			if(!affected_by_spell(dude, SPELL_PROTECT_FROM_EVIL) && !affected_by_spell(dude, SPELL_PROT_FROM_EVIL_GROUP)) {
				af.type      = SPELL_PROT_FROM_EVIL_GROUP;
				af.duration  = 24;
				af.modifier  = 0;
				af.location  = APPLY_NONE;
				af.bitvector = AFF_PROTECT_FROM_EVIL;
				affect_to_char(dude, &af);
				send_to_char("$c0014Senti scendere su di te una protezione contro il Male!\n\r", dude);
			}
		}
}


void spell_remove_curse(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int i, found=FALSE;

	assert(ch && (victim || obj));

	if(obj) {
		if(IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
			act("$p $c0015brilla$c0007 per un attimo di una $c0012luce azzurra$c0007.", TRUE, ch, obj, 0, TO_CHAR);
			act("$p, nelle mani di $n, $c0015brilla$c0007 per un attimo di una $c0012luce azzurra$c0007.", TRUE, ch, obj, 0, TO_ROOM);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
		}
	}
	else {
		/* Then it is a PC | NPC */
		if(affected_by_spell(victim, SPELL_CURSE)) {
			act("$n $c0015brilla$c0007 per un attimo di $c0009rosso$c0007, poi di $c0012azzurro$c0007.",FALSE,victim,0,0,TO_ROOM);
			act("Ti senti meglio.",FALSE,victim,0,0,TO_CHAR);
			affect_from_char(victim, SPELL_CURSE);
		}
		else {
			/* uncurse items equiped by a person */
			i=0;
			do {
				if(victim->equipment[i]) {
					if(IS_SET(victim->equipment[i]->obj_flags.extra_flags, ITEM_NODROP)) {
						spell_remove_curse(level,victim,NULL,victim->equipment[i]);
						found=TRUE;
					}
				}
				i++;
			}
			while(i<MAX_WEAR && !found);
		}

		/* ACIDUS 2003 l'heal su se stessi non fa cambiare align */
		if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch) && !(ch == victim)) {
			GET_ALIGNMENT(ch)+=2;
		}
	}
}




void spell_remove_poison(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {

	assert(ch && (victim || obj));

	if(victim) {
		if(affected_by_spell(victim,SPELL_POISON)) {
			affect_from_char(victim,SPELL_POISON);
			act("$c0014Un caldo formicolio pervade il tuo corpo.", FALSE, victim, NULL,
				NULL, TO_CHAR);
			act("$c0014Sembra che $n $c0014stia meglio.", FALSE, victim, NULL, NULL, TO_ROOM);
		}
	}
	else {
		if((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
				(obj->obj_flags.type_flag == ITEM_FOOD)) {
			obj->obj_flags.value[3] = 0;
			act("Una nuvola di $c0014vapore$c0007 avvolge momentaneamente $p.", FALSE, ch, obj,
				NULL, TO_CHAR);
			act("Una nuvola di $c0014vapore$c0007 avvolge momentaneamente $p.", TRUE, ch, obj,
				NULL, TO_ROOM);
		}
	}
}



void spell_fireshield(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if(!affected_by_spell(victim, SPELL_FIRESHIELD)) {

		act("$n esegue un gesto con le mani ed uno $c0001scudo di fuoco$c0007 l$b avvolge.",TRUE,victim,0,0,TO_ROOM);
		act("Inizi a $c0015brillare$c0007 avvolt$b dalle $c0001fiamme$c0007 di uno $c0001scudo di fuoco$c0007.",TRUE,victim,0,0,TO_CHAR);

		af.type      = SPELL_FIRESHIELD;
		af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_FIRESHIELD;
		affect_to_char(victim, &af);
	}
}

void spell_sanctuary(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if((!affected_by_spell(victim, SPELL_SANCTUARY)) &&
			(!IS_AFFECTED(victim, AFF_SANCTUARY))) {

		act("$n viene avvolt$b da un alone di $c0015luce Divina$c0007.",TRUE,victim,0,0,TO_ROOM);
		act("Inizi a brillare di una $c0015luce Divina$c0007.",TRUE,victim,0,0,TO_CHAR);

		af.type      = SPELL_SANCTUARY;
		af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SANCTUARY;
		affect_to_char(victim, &af);
	}
}



void spell_sleep(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(IsImmune(victim, IMM_SLEEP)) {
		FailSleep(victim, ch);
		return;
	}
	if(IsResist(victim, IMM_SLEEP)) {
		if(saves_spell(victim, SAVING_SPELL)) {
			FailSleep(victim, ch);
			return;
		}
		if(saves_spell(victim, SAVING_SPELL)) {
			FailSleep(victim, ch);
			return;
		}
	}
	else if(!IsSusc(victim, IMM_SLEEP)) {
		if(saves_spell(victim, SAVING_SPELL)) {
			FailSleep(victim, ch);
			return;
		}
	}

	af.type      = SPELL_SLEEP;
	af.duration  = 4+level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_SLEEP;
	affect_join(victim, &af, FALSE, FALSE);

	if(GET_POS(victim) > POSITION_SLEEPING) {
		act("Hai molto... molto sonno... zzzzzz!",FALSE,victim,0,0,TO_CHAR);
		act("$n si mette a dormire.",TRUE,victim,0,0,TO_ROOM);
		GET_POS(victim)=POSITION_SLEEPING;
	}
}



void spell_strength(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);


	if(!affected_by_spell(victim,SPELL_STRENGTH)) {
		act("Ti senti piu' $c0009forte$c0007!", FALSE, victim,0,0,TO_CHAR);
		act("$n sembra piu' $c0009forte$c0007!",
			FALSE, victim, 0, 0, TO_ROOM);
		af.type      = SPELL_STRENGTH;
		af.duration  = 2*level;
		if(IS_NPC(victim))
			if(level >= CREATOR) {
				af.modifier = 25 - GET_STR(victim);
			}
			else {
				af.modifier = number(1,6);
			}
		else {

			if(HasClass(ch, CLASS_WARRIOR) || HasClass(ch,CLASS_BARBARIAN)) {
				af.modifier = number(1,8);
			}
			else if(HasClass(ch, CLASS_CLERIC) ||
					HasClass(ch, CLASS_THIEF)) {
				af.modifier = number(1,6);
			}
			else {
				af.modifier = number(1,4);
			}
		}
		af.location  = APPLY_STR;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
	else {

		act("Non succede nulla.", FALSE, ch,0,0,TO_CHAR);

	}
}



void spell_ventriloquate(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	/* Not possible!! No argument! */
}



void spell_word_of_recall(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int location;


	assert(victim);

	if(IS_NPC(victim)) {
		return;
	}

	/*  loc_nr = GET_HOME(ch); */

	if(victim->player.hometown) {
		location = victim->player.hometown;
	}
	else {
		location = 3001;
	}

	if(!real_roomp(location)) {
		send_to_char("Non hai la piu' pallida idea di dove sei!\n\r", victim);
		location = 0;
		return;
	}

	if(victim->specials.fighting) {
		send_to_char("Eh gia', mentre combatti... e poi?!?\n\r",ch);
		return;
	}

	if(IS_AFFECTED2((IS_POLY(victim)) ? victim->desc->original : victim, AFF2_PKILLER)) {  // SALVO sistemato controllo pkiller
		send_to_char("L'incantesimo si spegne inefficace.\n\r",ch);
		return;
	}


	if(!IsOnPmp(victim->in_room)) {
		send_to_char("Non puoi! Sei in un altro piano dimensionale!\n\r",
					 victim);
		return;
	}

	/* a location has been found. */

	act("$n si $c0010smaterializza$c0007 e $c0008scompare$c0007.", TRUE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("La $c0008figura$c0007 di $n si $c0010materializza$c0007 improvvisamente davanti a te.", TRUE, victim, 0, 0, TO_ROOM);
	do_look(victim, "",15);

}


void spell_summon(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	struct char_data* tmp;
	struct room_data* rp;
	int count;

	assert(ch && victim);

	if(victim->in_room <= NOWHERE) {
		send_to_char("Non c'e' nessuno con quel nome!\n\r", ch);
		return;
	}

	if((rp = real_roomp(ch->in_room)) == NULL) {
		return;
	}
    
    if(!IS_PC(victim) && affected_by_spell(victim,STATUS_QUEST)) {
        act("Non si bara! ;)\n\r", FALSE, ch, 0, ch, TO_CHAR);
        return;
    }

	if(IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
		send_to_char("$c0008Un'oscura magia ti blocca.\n\r", ch);
		return;
	}

	if(IS_SET(rp->room_flags, TUNNEL)) {
		send_to_char("Non c'e' abbastanza spazio!\n\r", ch);
		return;
	}

	if((rp->sector_type == SECT_AIR) || rp->sector_type == SECT_UNDERWATER) {
		send_to_char("$c0008Oscuri poteri$c0007 bloccano la tua evocazione.\n\r", ch);
		return;
	}

	if(check_peaceful(ch, "Antichi poteri bloccano la tua magia.\n\r")) {
		return;
	}

	if(check_peaceful(victim, "")) {
		send_to_char("Non riesci a superare le difese magiche.\n\r", ch);
		return;
	}

	if(IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
		send_to_char("Un'antica $c00012magia$c0007 blocca l'evocazione.\n\r", ch);
		return;
	}

	if(GetMaxLevel(victim) >= LOW_IMMORTAL) {  // SALVO non deve agire su un immortale
		send_to_char("Una grossa mano appare improvvisamente e ti da' una botta in testa!\n\r", ch);
		return;
	}

	if(victim->specials.fighting) {
		send_to_char("Non riesci a mettere a $c0009fuoco$c0007 la sua posizione... sta combattendo!\n", ch);
		return;
	}

	if(IS_SET(SystemFlags,SYS_NOSUMMON)) {
		send_to_char("Una $c0008nebbia oscura$c0007 blocca il tuo tentativo!\n",ch);
		return;
	}

	if(!IsOnPmp(victim->in_room)) {
		send_to_char("E' in un altro piano dimensionale!\n", ch);
		return;
	}

	if(CanFightEachOther(ch,victim)) {
		if(saves_spell(victim, SAVING_SPELL) &&
				(GetMaxLevel(ch)-GetMaxLevel(victim)+number(1,100))>50) {
			act("Non riesci ad evocare $c0015$N$c0007!",FALSE,ch,0,victim,TO_CHAR);
			act("$c0015$n$c0007 ha appena cercato di evocarti!",FALSE,ch,0,victim,TO_VICT);
			return;
		}
	}


	if(!IS_PC(victim)) {
		count = 0;
		for(tmp=real_roomp(victim->in_room)->people;
				tmp; tmp = tmp->next_in_room) {
			count++;
		}

		if(count==0) {
			send_to_char("Non riesci.\n\r", ch);
			return;
		}
		else {
			count = number(0,count);

			for(tmp=real_roomp(victim->in_room)->people;
					count && tmp;
					tmp = tmp->next_in_room, count--)
				;

			if((tmp && GET_MAX_HIT(tmp) < GET_HIT(ch) &&
					!saves_spell(tmp,SAVING_SPELL))) {
				RawSummon(tmp, ch);
			}
			else {
				send_to_char("Non riesci.\n\r", ch);
				return;
			}
		}
	}
	else {
		RawSummon(victim, ch);
	}
}


void RawSummon(struct char_data* v, struct char_data* c) {
	long   target;
	struct obj_data* o, *n;
	int    j;
	char buf[400];

	/* this section run if the mob is above 3 levels above the caster */
	/* destroys the mobs EQ */

	if(IS_NPC(v) && (!IS_SET(v->specials.act, ACT_POLYSELF)) &&
			(GetMaxLevel(v) > GetMaxLevel(c)+3)) {
		act("$N resiste, e tutto quello che indossava viene distrutto!", TRUE, c, 0, v, TO_CHAR);
		/* remove objects from victim */
		for(j = 0; j < MAX_WEAR; j++) {
			if(v->equipment[j]) {
				o = unequip_char(v, j);
				extract_obj(o);
			}
		}
		for(o = v->carrying; o; o = n) {
			n = o->next_content;
			obj_from_char(o);
			extract_obj(o);
		}
		AddHated(v, c);
	}
	else {
		if(!EasySummon) {
			send_to_char("Perdi conoscenza!\n\r",c);
			WAIT_STATE(c, PULSE_VIOLENCE*6); // summon
			GET_POS(c) = POSITION_STUNNED;
		}
	}

	act("$n $c0008scompare$c0007 improvvisamente.",TRUE,v,0,0,TO_ROOM);
	target = c->in_room;
	char_from_room(v);
	char_to_room(v,target);

	act("$c0015$n$c0007 appare improvvisamente.",TRUE,v,0,0,TO_ROOM);

	sprintf(buf, "$c0015%s$c0007 ti ha evocato a se!\n\r",
			(IS_NPC(c)?c->player.short_descr:GET_NAME(c)));
	send_to_char(buf, v);
	do_look(v,"",15);

	if(IS_NPC(v) && !(IS_SET(v->specials.act,ACT_POLYSELF)) &&
			((IS_SET(v->specials.act, ACT_AGGRESSIVE) ||
			  (IS_SET(v->specials.act, ACT_META_AGG))))) {
		if(CAN_SEE(v,c)) {
			act("$n ti $c0009ringhia$c0007 contro.", 1, v, 0, c, TO_VICT);
			act("$n $c0009ringhia$c0007 contro $N.", 1, v, 0, c, TO_NOTVICT);
			hit(v, c, TYPE_UNDEFINED);
		}
	}
}


void spell_charm_person(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if(victim == ch) {
		send_to_char("Ti piaci ancora di piu'!\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
		if(circle_follow(victim, ch)) {
			send_to_char("State giocando a rincorrervi???\n\r", ch);
			return;
		}

		if(!IsPerson(victim)) {
			send_to_char("Uhm,  non e' che sia esattamente una persona...\n\r",ch);
			return;
		}

        if(IS_NPC(victim) && (mob_index[victim->nr].iVNum >= QUEST_ZONE && mob_index[victim->nr].iVNum <= QUEST_ZONE+99))
        {
            act("$N non vuole seguire nessuno!", TRUE, ch, 0, victim, TO_CHAR);
            return;
        }

		if(GetMaxLevel(victim) > GetMaxLevel(ch)+3) {
			FailCharm(victim, ch);
			return;
		}

		if(too_many_followers(ch)) {
			act("$N da un'occhiata alla quantita' di gente che ti segue e si rifiuta si aggiungersi!",
				TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
			act("$N da un'occhiata alla quantita' di gente che segue $n e si rifiuta di seguirl$b!",
				TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
			return;
		}

		if(IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
			FailCharm(victim,ch);
			return;
		}

		if(IsResist(victim, IMM_CHARM)) {
			if(saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}

			if(saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}
		}
		else {
			if(!IsSusc(victim, IMM_CHARM)) {
				if(saves_spell(victim, SAVING_PARA)) {
					FailCharm(victim,ch);
					return;
				}
			}
		}

		if(victim->master) {
			stop_follower(victim);
		}

		add_follower(victim, ch);

		af.type      = SPELL_CHARM_PERSON;

		if(GET_CHR(ch)) {
			af.duration  = follow_time(ch);
		}
		else {
			af.duration  = 24*18;
		}

		if(IS_GOOD(victim) && IS_GOOD(ch)) {
			af.duration *= 2;
		}
		if(IS_EVIL(victim) && IS_EVIL(ch)) {
			af.duration  += af.duration >> 1;
		}


		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char(victim, &af);

		act("Non ti pare che $n sia proprio simpatic$b?",FALSE,ch,0,victim,TO_VICT);

		if(!IS_PC(ch)) {
			REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
			SET_BIT(victim->specials.act, ACT_SENTINEL);
		}
	}
}



void spell_charm_monster(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if(victim == ch) {
		send_to_char("Ti piaci ancora di piu'!\n\r", ch);
		return;
	}

	if(IsVeggie(victim)) {
		send_to_char("Non puoi incantare una pianta!\n\r", ch);
		return;
	}
    
    if(IS_NPC(victim) && (mob_index[victim->nr].iVNum >= QUEST_ZONE && mob_index[victim->nr].iVNum <= QUEST_ZONE+99))
    {
        act("$N non vuole seguire nessuno!", TRUE, ch, 0, victim, TO_CHAR);
        return;
    }

	if(GetMaxLevel(victim) > GetMaxLevel(ch)+3) {
		FailCharm(victim, ch);
		return;
	}

	if(too_many_followers(ch)) {
		act("$N da' uno sguardo alla folla che ti segue e rifiuta di aggiungersi!",
			TRUE, ch, 0, victim, TO_CHAR);
		act("$N da' uno sguardo alla folla che segue $n e rifiuta di aggiungersi!",
			TRUE, ch, 0, victim, TO_ROOM);
		return;
	}

	if(!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
		if(circle_follow(victim, ch)) {
			send_to_char("State giocando a rincorrevi?\n\r", ch);
			return;
		}
		if(IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
			FailCharm(victim,ch);
			return;
		}
		if(IsResist(victim, IMM_CHARM)) {
			if(saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}

			if(saves_spell(victim, SAVING_PARA)) {
				FailCharm(victim,ch);
				return;
			}
		}
		else {
			if(!IsSusc(victim, IMM_CHARM)) {
				if(saves_spell(victim, SAVING_PARA)) {
					FailCharm(victim,ch);
					return;
				}
			}
		}

		if(victim->master) {
			stop_follower(victim);
		}

		add_follower(victim, ch);

		af.type      = SPELL_CHARM_PERSON;

		if(GET_CHR(ch)) {
			af.duration  = follow_time(ch);
		}
		else {
			af.duration  = 24*18;
		}

		if(IS_GOOD(victim) && IS_GOOD(ch)) {
			af.duration *= 2;
		}
		if(IS_EVIL(victim) && IS_EVIL(ch)) {
			af.duration  += af.duration >> 1;
		}


		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char(victim, &af);

		act("Non ti sembra che $n sia proprio simpatic$b?",FALSE,ch,0,victim,TO_VICT);

		if(!IS_PC(ch)) {
			REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
			SET_BIT(victim->specials.act, ACT_SENTINEL);
		}

	}
}


void spell_sense_life(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(!affected_by_spell(victim, SPELL_SENSE_LIFE))  {
		if(ch != victim) {
			act("$n muove le sue mani davanti agli occhi di $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Aiuti $N a percepire le forme di vita $c0008nascoste$c0007.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n muove le sue mani davanti ai tuoi occhi, senti i tuoi sensi diventare piu' $c0015acuti$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("Gli occhi di $n vengono attraversati da un $c0015lampo di luce$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Senti i tuoi sensi migliorare.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_SENSE_LIFE;
		af.duration  = 5*level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SENSE_LIFE;
		affect_to_char(victim, &af);
	}
	else {
		act("$N e' gia capace di vedere le forme di vita nascoste.",FALSE,ch,0,victim,TO_CHAR);
	}

}

void spell_globe_minor_inv(byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(!affected_by_spell(victim, SPELL_GLOBE_MINOR_INV)) {
		if(ch != victim) {
			act("$n evoca un piccolo $c0012globo$c0007 di protezione attorno a se.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi un piccolo $c0012globo$c0007 di protezione attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca un piccolo $c0012globo$c0007 di protezione attorno a te.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca un piccolo $c0012globo$c0007 di protezione attorno a se.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi un piccolo $c0012globo$c0007 di protezione attorno a te.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_GLOBE_MINOR_INV;
		af.duration  = (level<LOW_IMMORTAL) ? level/10 : level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
            act("Non appena lanci l'incantesimo capisci che era gia' protett$B.", FALSE, ch, 0, victim, TO_CHAR);
		}
		else {
            act("Non appena lanci l'incantesimo capisci che eri gia' protett$b.", FALSE, ch, 0, victim, TO_CHAR);
		}
	}
}

void spell_globe_major_inv(byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(!affected_by_spell(victim, SPELL_GLOBE_MAJOR_INV)) {
		if(ch != victim) {
			act("$n evoca un $c0014globo$c0007 di protezione attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi un $c0014globo$c0007 di protezione attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca un $c0014globo$c0007 di protezione attorno a te.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca un $c0014globo$c0007 di protezione attorno a se.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi un $c0014globo$c0007 di protezione attorno a te.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_GLOBE_MAJOR_INV;
		af.duration  = (level<LOW_IMMORTAL) ? level/10 : level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			act("Non appena lanci l'incantesimo capisci che era gia' protett$B.", FALSE, ch, 0, victim, TO_CHAR);
		}
		else {
			act("Non appena lanci l'incantesimo capisci che eri gia' protett$b.", FALSE, ch, 0, victim, TO_CHAR);
		}
	}

}

void spell_anti_magic_shell(byte level, struct char_data* ch,
							struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if(!victim) {
		return;
	}

    if(!affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
        if(ch != victim) {
            act("$n evoca uno $c0012scudo anti-magia$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("Evochi uno $c0012scudo anti-magia$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n evoca uno $c0012scudo anti-magia$c0007 attorno a te.", FALSE, ch, 0, victim, TO_VICT);
        }
        else {
            act("$N evoca uno $c0012scudo anti-magia$c0007 attorno a se.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("$c0012Crei uno scudo anti-magia attorno a te.", FALSE, ch, 0, victim, TO_CHAR);
        }
        
		spell_dispel_magic(CREATORE,ch,ch,0);

		af.type      = SPELL_ANTI_MAGIC_SHELL;
		af.duration  = (level<IMMORTALE) ? 1 : level;                                  /* one tic only! */
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
	else {
        if(ch != victim) {
            act("Hai gia' addosso uno $c0012scudo anti-magia$c0007.", FALSE, ch, 0, victim, TO_CHAR);
        }
        else {
            act("Ha gia' addosso uno $c0012scudo anti-magia$c0007.", FALSE, ch, 0, victim, TO_CHAR);
        }
    }

}


void spell_prismatic_spray(byte level, struct char_data* ch,
						   struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct char_data* tmp_victim;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	/* spell effects
	 * red 1 40 dam
	 * orange  2 80
	 * yellow 3 100
	 * green 4 poison
	 * blue 5 petrify
	 * indigo  6 feeble
	 * violet  7 teleport
	 */

	send_to_char("Lanci un fascio di luce $c0009p$c0010r$c0011i$c0012s$c0013m$c0014a$c0015t$c0009i$c0010c$c0011a$c0007 dalle tue mani.\n\r",ch);
	act("$n lancia un fascio di luce $c0009p$c0010r$c0011i$c0012s$c0013m$c0014a$c0015t$c0009i$c0010c$c0011a$c0007 dalle sue mani!",FALSE,ch,0,0,TO_ROOM);

	for(tmp_victim=real_roomp(ch->in_room)->people;
			tmp_victim ; tmp_victim=tmp_victim->next_in_room) {
		if(ch->in_room == tmp_victim->in_room && ch != tmp_victim) {
			if(!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
				switch(number(1, 7)) {
				case 1:
					dam=40;
					if(!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("Colpisci $N con un raggio di $c0009luce rossa$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
						act("$N viene colpit$b da un raggio di $c0009luce rossa$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
						if(MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY, 5) !=
								AllLiving) {
							return;
						}
					}
					break;
				case 2:
					dam=80;
					if(!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("Colpisci $N con un raggio di $c0015luce bianca$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
						act("$N viene colpit$b da un raggio di $c0015luce bianca$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
						if(MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY, 5) !=
								AllLiving) {
							return;
						}
					}
					break;
				case 3:
					dam=100;
					if(!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("Colpisci $N con un raggio di $c0011luce gialla$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
						act("$N viene colpit$ da un raggio di $c0011luce gialla$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
						if(MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY, 5) !=
								AllLiving) {
							return;
						}
					}
					break;
				/* these have internal saves already */
				case 4:
                    act("Colpisci $N con un raggio di $c0010luce verde$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
					act("$N viene colpit$b da un raggio di $c0010luce verde$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
					cast_poison(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
					break;
				case 5:
                    act("Colpisci $N con un raggio di $c0012luce blu$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
					act("$N viene colpit$b da un raggio di $c0012luce blu$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
					cast_paralyze(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
					break;
				case 6:
                    act("Colpisci $N con un raggio di $c0014luce celeste$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
					act("$N viene colpit$b da un raggio di $c0014luce celeste$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
					cast_feeblemind(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
					break;
				case 7:
                    act("Colpisci $N con un raggio di $c0013luce violetta$c0007!",FALSE,ch,0,tmp_victim,TO_CHAR);
					act("$N viene colpit$b da un raggio di $c0013luce violetta$c0007!",FALSE,ch,0,tmp_victim,TO_ROOM);
					cast_teleport(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
					break;
				default:        /* should never get here */
					dam=100;
					if(MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY, 5) !=
							AllLiving) {
						return;
					}
					break;
				} /* end switch */
				if(!tmp_victim->specials.fighting &&      /* make them hate the caster */
						ch->specials.fighting != tmp_victim) {
					set_fighting(ch,tmp_victim);
				}
			}
			else
				act("Eviti il raggio $c0009c$c0010o$c0011l$c0012o$c0013r$c0014a$c0015t$c0009o$c0007!\n\r",FALSE, ch, 0, tmp_victim,
					TO_VICT);
		}
	}
} /* end pris */

void spell_incendiary_cloud(byte level, struct char_data* ch,struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	dam = dice(level,10);

	send_to_char("Nuvole di $c0001gas fiammeggiante$c0007 vengono sprigionate dalle tue mani.\n\r",ch);
	act("Nuvole di $c0001gas infuocato$c0007 si sprigionano dalle mani di $n!",FALSE,ch,0,0,TO_ROOM);

	for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if(!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
				if(!saves_spell(tmp_victim, SAVING_SPELL))                         {
					heat_blind(tmp_victim);
					MissileDamage(ch,tmp_victim,dam,SPELL_INCENDIARY_CLOUD, 5);
					/* damage here */
				}
				else           {
					dam >>=1;  /* half dam */
					heat_blind(tmp_victim);
					MissileDamage(ch,tmp_victim,dam,SPELL_INCENDIARY_CLOUD, 5);
				}
			}
			else {
				act("Eviti di essere avvolto dalle fiamme!\n\r",FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
	}          /* end for */

}


void spell_comp_languages(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[128];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_COMP_LANGUAGES)) {

		if(ch != victim) {
			act("$c0003$n tocca dolcemente le orecchie di $N$c0003.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("$c0003Tocchi dolcemente le orecchie di $N$c0003.", FALSE, ch, 0, victim, TO_CHAR);
			act("$c0003$n $c0003tocca dolcemente le tue orecchie, ora riesci a comprendere tutto quello che viene detto!", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$c0003$n$c0003 si tocca le orecchie.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("$c0003Tocchi le tue orecchie, ora riesci a comprende tutto quello che viene detto!", FALSE, ch, 0, victim, TO_CHAR);
		}
		af.type      = SPELL_COMP_LANGUAGES;
		af.duration  = (level<LOW_IMMORTAL) ? static_cast<int>(level)/2 : level;                                  /* one tic only! */
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}

	else {
		if(ch != victim) {
			sprintf(buf,"$N puo' gia' capire tutti i linguaggi.");
		}
		else {
			sprintf(buf,"Riesci gia' a capire gli altri linguaggi.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}

}


/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */


void sprintbit(unsigned long, char* [], char*);

void spell_identify(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	char buf[256], buf2[256];
	int i;
	bool found;

	struct time_info_data age(struct char_data *ch);

	/* Spell Names */

	/* For Objects */
	/*
	*/

	assert(ch && (obj || victim));

	if(obj)
    {
        if (!FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(MobIdent)))
        {
            send_to_char("$c0011La conoscenza ti pervade:\n\r", ch);
        }

		sprintf(buf, "Oggetto: '$c0015%s$c0007', Tipo di Oggetto ", obj->name);
		sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
		strcat(buf,buf2);
		if IS_DIO(ch) {
            sprintf(buf2," $c0007V-Number Originario: $c0015%d",static_cast<int>(obj->char_vnum));
			strcat(buf,buf2);
		}

		strcat(buf,"\n\r");
		send_to_char(buf, ch);

		if(obj->obj_flags.bitvector) {
			send_to_char("L'oggetto dona le seguenti abilita':  ", ch);
			sprintbit(static_cast<unsigned>(obj->obj_flags.bitvector),affected_bits,buf2);
            sprintf(buf, "$c0015");
            strcat(buf, buf2);
			strcat(buf,"\n\r");
			send_to_char(buf, ch);
		}

		send_to_char("L'oggetto e': ", ch);
		sprintbit2(static_cast<unsigned>(obj->obj_flags.extra_flags),extra_bits,static_cast<unsigned>(obj->obj_flags.extra_flags2),extra_bits2,buf2);
        sprintf(buf, "$c0015");
        strcat(buf, buf2);
		strcat(buf,"\n\r");
		send_to_char(buf,ch);

		sprintf(buf,"Peso: $c0015%d$c0007, Valore: $c0015%d$c0007, Costo di rent: $c0015%d$c0007  %s\n\r",
				obj->obj_flags.weight, obj->obj_flags.cost,
				obj->obj_flags.cost_per_day,
				obj->obj_flags.cost >= LIM_ITEM_COST_MIN ? "$c0009[RARO]" : " ");
		send_to_char(buf, ch);


		switch(GET_ITEM_TYPE(obj)) {
		case ITEM_SCROLL :
		case ITEM_POTION :
			sprintf(buf, "Livello $c0015%d$c0007 dell'incantesimo:\n\r", obj->obj_flags.value[0]);
			send_to_char(buf, ch);
			if(obj->obj_flags.value[1] >= 1) {
				sprinttype(obj->obj_flags.value[1]-1,spells,buf2);
                sprintf(buf, "$c0015");
                strcat(buf, buf2);
				strcat(buf,"\n\r");
				send_to_char(buf, ch);
			}
			if(obj->obj_flags.value[2] >= 1) {
				sprinttype(obj->obj_flags.value[2]-1,spells,buf2);
                sprintf(buf, "$c0015");
                strcat(buf, buf2);
				strcat(buf,"\n\r");
				send_to_char(buf, ch);
			}
			if(obj->obj_flags.value[3] >= 1) {
				sprinttype(obj->obj_flags.value[3]-1,spells,buf2);
                sprintf(buf, "$c0015");
                strcat(buf, buf2);
				strcat(buf,"\n\r");
				send_to_char(buf, ch);
			}
			break;

		case ITEM_WAND :
		case ITEM_STAFF :
			sprintf(buf, "Ha $c0015%d$c0007 cariche totali, con $c0015%d$c0007 cariche rimanenti.\n\r",
					obj->obj_flags.value[1],
					obj->obj_flags.value[2]);
			send_to_char(buf, ch);

			sprintf(buf, "Livello $c0015%d$c0007 dell'incantesimo:\n\r", obj->obj_flags.value[0]);
			send_to_char(buf, ch);

			if(obj->obj_flags.value[3] >= 1) {
				sprinttype(obj->obj_flags.value[3]-1,spells,buf2);
                sprintf(buf, "$c0015");
                strcat(buf, buf2);
                strcat(buf,"\n\r");
				send_to_char(buf, ch);
			}
			break;

		case ITEM_WEAPON :
			sprintf(buf, "Dado di danno: '$c0015%d$c0007d$c0015%d$c0007'\n\r",
					obj->obj_flags.value[1],
					obj->obj_flags.value[2]);
			send_to_char(buf, ch);
			sprinttype(obj->obj_flags.value[3],aszWeaponType,buf2);
			sprintf(buf,"Tipo di danno: '$c0015%s$c0007'\n\r",buf2);
			send_to_char(buf, ch);
			break;

		case ITEM_ARMOR :
			sprintf(buf, "AC-apply di $c0015%d.\n\r",
					obj->obj_flags.value[0]);
			send_to_char(buf, ch);
			break;

		}

		found = FALSE;

		for(i=0; i<MAX_OBJ_AFFECT; i++) {
			if((obj->affected[i].location != APPLY_NONE) &&
					(obj->affected[i].modifier != 0) &&
					//(obj->affected[i].location !=APPLY_AFF2) &&
					(obj->affected[i].location !=APPLY_SKIP)) {
				if(!found) {
					send_to_char("Caratteristiche: \n\r", ch);
					found = TRUE;
				}

				sprinttype(obj->affected[i].location,apply_types,buf2);
				sprintf(buf,"    Ti puo' dare : $c0015%s$c0007 by ", buf2);
				send_to_char(buf,ch);
				switch(obj->affected[i].location) {
				case APPLY_M_IMMUNE:
				case APPLY_IMMUNE:
				case APPLY_SUSC:
                    sprintf(buf, "$c0015");
					sprintbit(obj->affected[i].modifier,immunity_names,buf2);
                    strcat(buf, buf2);
                    strcat(buf,"\n\r");
					break;
				case APPLY_ATTACKS:
					sprintf(buf,"$c0015%f\n\r", static_cast<double>(obj->affected[i].modifier / 10));
					break;
				case APPLY_WEAPON_SPELL:
				case APPLY_EAT_SPELL:
					sprintf(buf,"$c0015%s\n\r", spells[obj->affected[i].modifier-1]);
					break;
				case APPLY_SPELL:
                    sprintf(buf, "$c0015");
					sprintbit(obj->affected[i].modifier,affected_bits, buf2);
                    strcat(buf, buf2);
                    strcat(buf,"\n\r");
					break;
                case APPLY_AFF2:
                    sprintf(buf, "$c0015");
                    sprintbit(obj->affected[i].modifier,affected_bits2, buf2);
                    strcat(buf, buf2);
                    strcat(buf,"\n\r");
                    break;
				case APPLY_RACE_SLAYER:
					sprintf(buf2, "$c0015%s\n\r", RaceName[ obj->affected[i].modifier ]);
					break;
				case APPLY_ALIGN_SLAYER:
                    sprintf(buf, "$c0015");
					sprintbit(static_cast<unsigned>(obj->affected[i].modifier), gaszAlignSlayerBits,
							  buf2);
                    strcat(buf, buf2);
                    strcat(buf,"\n\r");
					break;
				default:
					sprintf(buf,"$c0015%d\n\r", obj->affected[i].modifier);
					break;
				}
				send_to_char(buf, ch);
			}
		}
	}
	else {
		/* victim */

		if(!IS_NPC(victim)) {
			struct time_info_data ma;

			age3(victim, &ma);
			sprintf(buf,"$c0015%d$c0007 Anni,  $c0015%d$c0007 Mesi,  $c0015%d$c0007 Giorni,  $c0015%d$c0007 Or%s di vita.\n\r",
					ma.year, ma.month,
                    ma.day, ma.hours, (ma.hours > 1 ? "e" : "a") );
			send_to_char(buf,ch);

			sprintf(buf,"Altezza $c0015%d$c0007 cm  Peso $c0015%d$c0007 kg.\n\r",
					GET_HEIGHT(victim), (int)(GET_WEIGHT(victim) * 0.4536));
			send_to_char(buf,ch);

			sprintf(buf,"Classe Armatura $c0015%d$c0007.\n\r",victim->points.armor);
			send_to_char(buf,ch);

			if(level > 30) {
				sprintf(buf,"Forza $c0015%d$c0007/$c0015%d$c0007, Intelligenza $c0015%d$c0007, Saggezza $c0015%d$c0007, Destrezza $c0015%d$c0007, Costituzione $c0015%d$c0007, Carisma $c0015%d$c0007.\n\r",
						GET_STR(victim), GET_ADD(victim),
						GET_INT(victim),
						GET_WIS(victim),
						GET_DEX(victim),
						GET_CON(victim),
						GET_CHR(victim));
				send_to_char(buf,ch);
			}
		}
		else {
			send_to_char("Non scopri nulla di nuovo.\n\r", ch);
		}
	}

	if(GetMaxLevel(ch)<LOW_IMMORTAL && !FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(MobIdent)))
    {
		act("Vieni sopraffatt$b da un'ondata di stanchezza.",FALSE,ch,0,0,TO_CHAR);
		act("$c0008$n$c0008 cade a terra privo di sensi.",FALSE,ch,0,0,TO_ROOM);
		WAIT_STATE(ch,PULSE_VIOLENCE*2); // identify
		//GET_POS(ch) = POSITION_STUNNED;
	}
}





void spell_enchant_armor(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	int i;
	int count=0;

	assert(ch && obj);
	assert(MAX_OBJ_AFFECT >= 2);

	if((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
			!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

		for(i=0; i < MAX_OBJ_AFFECT; i++) {
			if(obj->affected[i].location == APPLY_NONE) {
				count++;
			}
			if(obj->affected[i].location == APPLY_AC ||
					obj->affected[i].location == APPLY_SAVE_ALL ||
					obj->affected[i].location == APPLY_SAVING_PARA ||
					obj->affected[i].location == APPLY_SAVING_ROD ||
					obj->affected[i].location == APPLY_SAVING_PETRI ||
					obj->affected[i].location == APPLY_SAVING_BREATH ||
					obj->affected[i].location == APPLY_SAVING_SPELL ||
					obj->affected[i].location == APPLY_SAVE_ALL) {
				send_to_char("L'oggetto non puo' sostenere ulteriori miglioramenti.\n\r", ch);
				return;
			}
		}

		if(count < 2) {
			send_to_char("L'oggetto non puo' essere incantato.\n\r", ch);
			return;
		}
		/*  find the slots */
		i = getFreeAffSlot(obj);

		SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

		obj->affected[i].location = APPLY_AC;
		obj->affected[i].modifier = -1;
		if(level >= APPRENDISTA) {
			obj->affected[i].modifier -= 1;
		}
		if(level >= INIZIATO) {
			obj->affected[i].modifier -= 1;
		}
		if(level >= BARONE) {
			obj->affected[i].modifier -= 1;
		}
		if(level > MAX_MORT) {
			obj->affected[i].modifier -= 1;
		}
		if(level >= IMMENSO) {
			obj->affected[i].modifier -= 1;
		}

		i = getFreeAffSlot(obj);

		obj->affected[i].location = APPLY_SAVE_ALL;
		obj->affected[i].modifier = 0;
		if(level >=INIZIATO) {
			obj->affected[i].modifier -= 1;
		}
		if(level >=BARONE) {
			obj->affected[i].modifier -= 1;
		}
		if(level > MAX_MORT) {
			obj->affected[i].modifier -= 1;
		}
		if(level >= IMMENSO) {
			obj->affected[i].modifier -= 1;
		}

		if(IS_GOOD(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
			act("$p $c0015brilla$c0007 di $c0012luce blu$c0007.",FALSE,ch,obj,0,TO_CHAR);
		}
		else if(IS_EVIL(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
			act("$p $c0015brilla$c0007 di $c0009luce rossa$c0007.",FALSE,ch,obj,0,TO_CHAR);
		}
		else {
			act("$p $c0015brilla$c0007 di $c0011luce gialla$c0007.",FALSE,ch,obj,0,TO_CHAR);
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL);
		}
	}
}

void spell_quest(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if((!affected_by_spell(victim, STATUS_QUEST))) {

		act("$n viene colt$b dal senso del dovere.",TRUE,victim,0,0,TO_ROOM);
		act("$c0011Senti di avere un compito su questa terra.",TRUE,victim,0,0,TO_CHAR);

		af.type      = STATUS_QUEST;
        af.duration  = level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
}

/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) {
		hpch=10;
	}

	dam = hpch;

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}
    
	MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH, 5);
    
    /* la possibilità di danneggiare l'equipaggiamento è raddoppiata per i soffi */
    DamageStuff(victim, SPELL_FIRE_BREATH, dam, 5);
}


void spell_frost_breath(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) {
		hpch=10;
	}

	dam = hpch;

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_FROST_BREATH, 5);

    /* la possibilità di danneggiare l'equipaggiamento è raddoppiata per i soffi */
    DamageStuff(victim, SPELL_FROST_BREATH, dam, 5);
}


void spell_acid_breath(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	int apply_ac(struct char_data *ch, int eq_pos);

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) {
		hpch=10;
	}

	dam = hpch;

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_ACID_BREATH, 5);
    
    /* la possibilità di danneggiare l'equipaggiamento è raddoppiata per i soffi */
    DamageStuff(victim, SPELL_ACID_BREATH, dam, 5);
}


void spell_gas_breath(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) {
		hpch=10;
	}

	dam = hpch;

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_GAS_BREATH, 5);
    
    /* la possibilità di danneggiare l'equipaggiamento è raddoppiata per i soffi */
    DamageStuff(victim, SPELL_GAS_BREATH, dam, 5);
}


void spell_lightning_breath(byte level, struct char_data* ch,
							struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	assert(victim && ch);
	if(level <0 || level >ABS_MAX_LVL) {
		return;
	}

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) {
		hpch=10;
	}

	dam = hpch;

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH, 5);
    
    /* la possibilità di danneggiare l'equipaggiamento è raddoppiata per i soffi */
    DamageStuff(victim, SPELL_LIGHTNING_BREATH, dam, 5);
    
}


void spell_wizard_eye(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	if(!affected_by_spell(victim, SPELL_WIZARDEYE)) {
		act("$n evoca un $c0011occhio magico$c0007 per esplorare i dintorni.",TRUE,victim,0,0,TO_ROOM);
		act("Evochi un $c0011occhio magico$c0007 per esplorare i dintorni.",TRUE,victim,0,0,TO_CHAR);

		af.type      = SPELL_WIZARDEYE;
		af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SCRYING;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			act("$N ha gia' evocato un occhio magico.",FALSE,ch,0,victim,TO_CHAR);
		}
		else {
			act("Hai gia' un occhio magico.",FALSE,ch,0,victim,TO_CHAR);
		}
	}

}


void spell_disintegrate(byte level, struct char_data* ch,  struct char_data* victim, struct obj_data* obj) {
	int i,damage;
	struct obj_data* x;

	if(!ch) {
		mudlog(LOG_SYSERR, "!ch in spell_disintegrate");
		return;
	}

	if(!victim) {
		mudlog(LOG_SYSERR, "!victim in spell_disintegrate");
		return;
	}

	damage = dice(level,10);
	if(!saves_spell(victim, SAVING_SPELL)) {

		/* frag thier EQ */
		i=0;
		do {
			/* could make this check the carried EQ as well... */
			if(victim->equipment[i]) {
				obj=victim->equipment[i];
				if(!ItemSave(obj,FIRE_DAMAGE)) {
					act("$p di $N diviene $c0001incadescente$c0007 e sparisce "
						"in una $c0008nuvola$c0007 di $c0008fumo$c0007!", TRUE, ch, obj, victim, TO_CHAR);
					if(obj->equipped_by || obj->carried_by)
						act("$p di $N diviene $c0001incandescente$c0007 e "
							"sparisce in una $c0008nuvola$c0007 di $c0008fumo$c0007!", TRUE, ch, obj, victim,
							TO_ROOM);
					if(obj->carried_by) {
						/* remove the obj */
						obj_from_char(obj);
					}
					else if(obj->equipped_by) {
						obj = unequip_char(obj->equipped_by, obj->eq_pos);
					}
					else if(obj->in_obj) {
						obj_from_obj(obj);
						obj_to_room(obj,ch->in_room);
					}
					else if(obj->contains) {
						while(obj->contains) {
							x = obj->contains;
							obj_from_obj(x);
							obj_to_room(x, ch->in_room);
						} /* end while */
					}  /* end contains */

					if(obj) {
						extract_obj(obj);
					}
				}
				else { /* saved */
					if(obj) {
						act("$c0010Gli atomi di $p $c0010rimangono saldamente uniti.",
							TRUE, ch, obj, victim, TO_VICT);
						act("$c0010Gli atomi di $p $c0010di $c0015$N$c0010 rimangono saldamente uniti.",
							FALSE, ch, obj, victim, TO_ROOM);
					}
				} /* end saved obj */
			}
			i++;
		}
		while(i<MAX_WEAR);
	}
	else {
		/* we saved ! 1/2 dam and no EQ frag */
		damage>>=1;
	}

	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
		GET_ALIGNMENT(ch)-=2;
	}

	MissileDamage(ch, victim, damage, SPELL_DISINTEGRATE, 5);
}

} // namespace Alarmud

