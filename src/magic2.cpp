/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
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
#include "magic2.hpp"
#include "act.off.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "magic.hpp"
#include "magicutils.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"

namespace Alarmud {


/*
 * cleric spells
 */

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

void spell_resurrection(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct char_file_u st;
	struct affected_type af;
	struct obj_data* obj_object, *next_obj;
	FILE* fl;
	FILE* fdeath;
	char szFileName[ 40 ];

	if(!obj) {
		return;
	}


	if(IS_CORPSE(obj)) {

		if(obj->char_vnum) {
            if(obj->char_vnum >= QUEST_ZONE && obj->char_vnum <= QUEST_ZONE+99) {
                send_to_char("Gli Dei non ti concedono questo potere su questa creatura!\n\r",ch);
                return;
            }

			/* corpse is a npc */
			/* Modifica Urhar, toglie ai multi la possibilita' di resurrectare mob */
			if(!IS_IMMORTALE(ch)) {
                if(!IS_SINGLE(ch) || isname2("BossKill",mob_index[victim->nr].specname)) {
                    send_to_char("Gli Dei non ti concedono questo potere su questa creatura!\n\r",ch);
                    return;
                }
			}
			/* fine modifica */
			if(GET_GOLD(ch) < 25000) {
				send_to_char("Gli dei non sono soddisfatti del tuo sacrificio.\n\r",ch);
				return;
			}
			else {
				GET_GOLD(ch) -= 25000;
			}

			victim = read_mobile(obj->char_vnum, VIRTUAL);
			char_to_room(victim, ch->in_room);
			GET_GOLD(victim)=0;
			GET_EXP(victim)=0;
			GET_HIT(victim)=1;
			alter_hit(victim,0);
			GET_POS(victim)=POSITION_STUNNED;

			act("$c0015Con un arcano rituale, $n riporta in vita un cadavere.", TRUE, ch,
				0, 0, TO_ROOM);
			act("$n si rialza lentamente da terra.", FALSE, victim, 0, victim,
				TO_ROOM);

			/* should be charmed and follower ch */

			if(!IS_IMMORTAL(ch) && (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM))) {
				act("$n dice 'Grazie!'", FALSE, victim, 0, victim, TO_ROOM);

			}
			else if(too_many_followers(ch)) {
				act("$N si rifiuta di unirsi alla folla che ti segue!",
					TRUE, ch, 0, victim, TO_CHAR);
				act("Dai uno sguardo alla folla che segue $n e rifiuti di unirti!",
					TRUE, ch, 0, victim, TO_ROOM);
			}
			else {
				af.type      = SPELL_CHARM_PERSON;
				af.duration  = follow_time(ch);
				af.modifier  = 0;
				af.location  = 0;
				af.bitvector = AFF_CHARM;

				affect_to_char(victim, &af);

				add_follower(victim, ch);
			}

			IS_CARRYING_W(victim) = 0;
			IS_CARRYING_N(victim) = 0;

			/* take all from corpse, and give to person */

			for(obj_object=obj->contains; obj_object; obj_object=next_obj) {
				next_obj = obj_object->next_content;
				obj_from_obj(obj_object);
				obj_to_char(obj_object, victim);
			}

			/* get rid of corpse */
			extract_obj(obj);

            if(HasClass(ch, CLASS_CLERIC) && IS_PC(ch))
            {
                if(IS_POLY(ch))
                {
                    ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_CLERIC_2] += 1;
                    if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                        SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                }
                else
                {
                    ch->specials.achievements[CLASS_ACHIE][ACHIE_CLERIC_2] += 1;
                    if(!IS_SET(ch->specials.act,PLR_ACHIE))
                        SET_BIT(ch->specials.act, PLR_ACHIE);
                }

                CheckAchie(ch, ACHIE_CLERIC_2, CLASS_ACHIE);
            }

		}
		else {
			/* corpse is a pc  */

			if(GET_GOLD(ch) < 75000) {
				send_to_char("Gli Dei non sono soddisfatti del tuo sacrificio.\n\r",ch);
				return;
			}
			else {
				GET_GOLD(ch) -= 75000;
			}
			sprintf(szFileName, "%s/%s.dat", PLAYERS_DIR, lower(obj->oldfilename));
			if((fl = fopen(szFileName, "r+")) == NULL) {
				mudlog(LOG_SYSERR, "Cannot find player file %s in resurrect.",
					   szFileName);
				send_to_char("Problemi con il file del giocatore da resuscitare.\n\r",
							 ch);
				send_to_char("Contattare un Dio.\n\r", ch);
				return;
			}
#if DEATH_FIX
			sprintf(szFileName, "%s/%s.dead", PLAYERS_DIR, lower(obj->oldfilename));
			if((fdeath = fopen(szFileName, "r+")) == NULL) {
				mudlog(LOG_SYSERR, "Cannot find dead file %s in resurrect.",
					   szFileName);
				send_to_char("Problemi con il file del giocatore da resuscitare.\n\r",
							 ch);
				send_to_char("Contattare un Dio.\n\r", ch);
				fclose(fl);
				return;
			}
#endif
			fread(&st, sizeof(struct char_file_u), 1, fl);
			if(!get_char(st.name) && st.abilities.con > 3) {
#if DEATH_FIX
				long xp;
				long ora;
				fscanf(fdeath,"%ld : %ld",&xp,&ora);
				st.points.exp=xp;
				ora=(long)(time(0)-ora);
				mudlog(LOG_PLAYERS, "%s resuscitato dopo %ld secondi",
					   obj->oldfilename,ora);

#else
				st.points.exp *= 5;
				st.points.exp /= 4;
#endif
				if(GetMaxLevel(ch)<MAESTRO_DEL_CREATO) {
					st.abilities.con -= 1;
					st.agemod=MIN(st.agemod,-30);
				}
				act("Una musica $c0014celestiale$c0007 risuona nella stanza.",
					TRUE, ch, 0, 0, TO_CHAR);
				act("Uno $c0015spirito$c0007 appare improvvisamente e si dissolve in un $c0011lampo di luce$c0007.",
					TRUE, ch, 0, 0, TO_CHAR);
				act("Una musica $c0014celestiale$c0007 risuona nella stanza.",
					TRUE, ch, 0, 0, TO_ROOM);
				act("Uno $c0015spirito$c0007 appare improvvisamente e si dissolve in un $c0011lampo di luce$c0007.",
					TRUE, ch, 0, 0, TO_ROOM);
				act("$p scompare in un battito di ciglia.",
					TRUE, ch, obj, 0, TO_ROOM);
				act("$p scompare in un battito di ciglia.",
					TRUE, ch, obj, 0, TO_ROOM);
				if(GetMaxLevel(ch)<IMMORTALE) {
					GET_MANA(ch) = 1;
					alter_mana(ch,0);
					GET_MOVE(ch) = 1;
					alter_move(ch,0);
					GET_HIT(ch) = 1;
					alter_hit(ch,0);
					GET_POS(ch) = POSITION_STUNNED;
					act("$n cade a terra privo di sensi!",TRUE, ch, 0, 0, TO_ROOM);
					send_to_char("Svieni e crolli a terra.\n\r",ch);
				}

                if(HasClass(ch, CLASS_CLERIC) && IS_PC(ch))
                {
                    if(IS_POLY(ch))
                    {
                        ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_CLERIC_2] += 1;
                        if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                            SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                    }
                    else
                    {
                        ch->specials.achievements[CLASS_ACHIE][ACHIE_CLERIC_2] += 1;
                        if(!IS_SET(ch->specials.act,PLR_ACHIE))
                            SET_BIT(ch->specials.act, PLR_ACHIE);
                    }

                    CheckAchie(ch, ACHIE_CLERIC_2, CLASS_ACHIE);
                }

				rewind(fl);
				fwrite(&st, sizeof(struct char_file_u), 1, fl);
				mudlog(LOG_PLAYERS, "%s e' stato resuscitato da %s.",
					   obj->oldfilename, GET_NAME(ch));
				ObjFromCorpse(obj);

			}
			else {
				send_to_char("Questo corpo e' troppo debole per essere "
							 "riportato in vita.\n\r", ch);
			}
			fclose(fl);
			fclose(fdeath);
		}
	}
}

void spell_cause_light(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(ch && victim);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(1,8);

	if(!HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
		dam = 0;
	}

	damage(ch, victim, dam, SPELL_CAUSE_LIGHT, 5);

}

void spell_cause_critical(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(ch && victim);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(3,8) + 3;

	if(!HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
		dam = 0;
	}

	damage(ch, victim, dam, SPELL_CAUSE_CRITICAL, 5);

}

void spell_cause_serious(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(ch && victim);

	dam = dice(2,8) + 2;

	if(!HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
		dam = 0;
	}

	damage(ch, victim, dam, SPELL_CAUSE_SERIOUS, 5);

}

void spell_cure_serious(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int healpoints;
    char buf[MAX_STRING_LENGTH];

	if(!ch && victim) {
		send_to_char("Chi vuoi curare?",ch);
		mudlog(LOG_SYSERR,"Cure serious failed ch && victim");
		return;
	}

	assert((level >= 1) && (level <= ABS_MAX_LVL));

	healpoints = dice(2,8)+2;

	if((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        healpoints = hit_limit(victim) - GET_HIT(victim);
		GET_HIT(victim) = hit_limit(victim);
		alter_hit(victim,0);
	}
	else {
		GET_HIT(victim) += healpoints;
		alter_hit(victim,0);
	}

    if(ch != victim)
    {
        sprintf(buf, "$c0015Curi $N$c0015.");
        if(IS_SET(ch->player.user_flags,PWP_MODE))
            sprintf(buf, "%s $c0014[%d]$c0007",buf, healpoints);
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        act("$c0015$n$c0015 cura $N$c0015.", FALSE, ch, 0, victim, TO_NOTVICT);
        sprintf(buf, "$c0015$n$c0015 ti cura.");
        if(IS_SET(victim->player.user_flags,PWP_MODE))
            sprintf(buf, "%s $c0014[%d]$c0007",buf, healpoints);
        act(buf, FALSE, ch, 0, victim, TO_VICT);
    }
    if(ch == victim)
    {
        act("$c0015$n$c0015 si cura.", FALSE, ch, 0, victim, TO_NOTVICT);
        sprintf(buf, "$c0015Ti curi.");
        if(IS_SET(victim->player.user_flags,PWP_MODE))
            sprintf(buf, "%s $c0014[%d]$c0007",buf, healpoints);
        act(buf, FALSE, ch, 0, victim, TO_VICT);
    }

    if(healpoints > 0) {
        send_to_char("Ti senti meglio!\n\r", victim);
    }

	update_pos(victim);
#if 0
	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
		GET_ALIGNMENT(ch)+=2;
	}
#endif
}

void spell_mana(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct affected_type af;

	assert(ch);

	if(level > 0) {
		dam = level*4;
		if(GET_MANA(ch)+dam > GET_MAX_MANA(ch)) {
			GET_MANA(ch) = GET_MAX_MANA(ch);
			alter_mana(ch,0);
		}
		else {
			GET_MANA(ch) += dam;
			alter_mana(ch,0);
		}
	}
	else {
		/* Durata 4 ticks */
		af.type      = SPELL_MANA;
		af.duration  = 4;
		af.modifier  = GET_MANA(ch);
		af.location  = APPLY_HIT;
		af.bitvector = AFF_NONE;
		affect_to_char(ch, &af);
		GET_HIT(ch)=GET_MANA(ch);
		alter_hit(ch,0);
		GET_MANA(ch)=0;
		alter_mana(ch,0);
		act("$n viene improvvisamente avvolt$b da un $c0011globo pulsante$c0007 di $c0011energia$c0007.",TRUE,ch,0,victim,TO_NOTVICT);
		act("Vieni avvolto da un $c0011globo pulsante$c0007 di $c0011energia$c0007.", TRUE, ch, 0, victim, TO_CHAR);
	}
}


void spell_second_wind(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(ch && victim);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,8)+level;

	if((dam + GET_MOVE(victim)) > move_limit(victim)) {
		GET_MOVE(victim) = move_limit(victim);
		alter_move(victim,0);
	}
	else {
		GET_MOVE(victim) += dam;
		alter_move(victim,0);
	}

	send_to_char("Senti passare la stanchezza.\n\r", victim);

}


void spell_flamestrike(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(6,8);

	if(saves_spell(victim, SAVING_SPELL)) {
		dam >>= 1;
	}
	heat_blind(victim);
	MissileDamage(ch, victim, dam, SPELL_FLAMESTRIKE, 5);

}



void spell_dispel_good(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {

	assert(ch && victim);
	assert((level >= 1) && (level<=ABS_MAX_LVL));

	if(IS_PC(victim) || IS_POLY(victim)) {
		send_to_char("Non riesci a scacciarlo.\n\r", ch);
		return;
	}

	if(IsExtraPlanar(victim)) {
		if(IS_GOOD(ch)) {
			send_to_char("Non hai lo $c0015spirito$c0007 adatto per scacciare una $c0014creatura$c0007 cosi' $c0014buona$c0007.\n\r", ch);
			return;;
		}
		else if(IS_EVIL(victim)) {
			act("Il $c0001Male$c0007 protegge $N.", FALSE, ch, 0, victim, TO_CHAR);
			return;
		}

		if(!saves_spell(victim, SAVING_SPELL)) {
			act("$c0009$n scaccia $N $c0009da questo piano di esistenza.",TRUE,ch,0,victim,TO_NOTVICT);
			act("$c0009Scacci $N $c0009da questo piano di esistenza.", TRUE, ch, 0, victim, TO_CHAR);
			act("$c0009$n ti scaccia da questo piano di esistenza.", TRUE, ch, 0, victim,TO_VICT);
			gain_exp(ch, MIN(GET_EXP(victim)/2, 50000));
			extract_char(victim);
		}
	}
	else {
		act("$N ti ride in faccia.", TRUE, ch, 0, victim, TO_CHAR);
		act("$N ride in faccia a $n.", TRUE,ch, 0, victim, TO_NOTVICT);
		act("Ridi in faccia a $n.", TRUE,ch,0,victim,TO_VICT);
	}
}

void spell_turn(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	struct char_data* next_victim;
	int diff;
	assert(ch);
	assert((level >= 1) && (level<=ABS_MAX_LVL));
	/* Potenziato:
	 * Adesso se il tiro salvezza fallisce o la differenza di livello
	 * supera 20 il mob viene eliminato
	 * */
	for(victim=real_roomp(ch->in_room)->people; victim; victim = next_victim) {
		next_victim=victim->next_in_room;
		diff = (level - (GetAverageLevel(victim) *2));

		if(in_group(ch,victim) || !CanFightEachOther(ch,victim)) {
			/* Ramo vuoto... nulla di fatto */
		}
		else if(!IsUndead(victim)) {

			act("$n ha appena cercato di scacciarti... poverino!", TRUE, ch, 0, victim, TO_VICT);
			act("$N pensa che $n sia molto strano.", TRUE, ch, 0, victim, TO_NOTVICT);
			act("Uhm... $N non e' un Non-Morto...", TRUE, ch, 0, victim, TO_CHAR);
		}
		else {
			act("Cerchi di scacciare $N da qui.", TRUE, ch, 0, victim, TO_CHAR);
			if(diff >= 0 && (!saves_spell(victim, SAVING_SPELL) || diff > 0)) {
				if(diff > 0) {

					act("$c0015$n scaccia $N $c0015da questo piano di esistenza.",TRUE,ch,0,victim,TO_NOTVICT);
					act("$c0015Scacci $N $c0015da questo piano di esistenza.", TRUE, ch, 0, victim, TO_CHAR);
					act("$c0015$n ti scaccia da questo piano di esistenza.", TRUE, ch, 0, victim,TO_VICT);
					gain_exp(ch, MIN(GET_EXP(victim)/2, 100000));
					extract_char(victim);
				}
				else {

					act("$n scaccia $N da qui.",TRUE,ch,0,victim,TO_NOTVICT);
					act("Scacci $N da qui.", TRUE, ch, 0, victim, TO_CHAR);
					act("$n ti scaccia da qui.", TRUE, ch, 0, victim,TO_VICT);
					do_flee(victim,"",0);
				}
			}
			else {
				if(diff > -5) {
					act("$n scaccia $N da qui.",TRUE,ch,0,victim,TO_NOTVICT);
					act("Scacci $N da qui.", TRUE, ch, 0, victim, TO_CHAR);
					act("$n ti scaccia da qui.", TRUE, ch, 0, victim,TO_VICT);
					do_flee(victim,"",0);
				}
				else {
					act("$N ride degli sforzi di $n.",TRUE,ch,0,victim,TO_NOTVICT);
					act("$N ride dei tuoi sforzi.", TRUE, ch, 0, victim, TO_CHAR);
					act("Ridi degli sforzi di $n", TRUE, ch, 0, victim,TO_VICT);
					do_flee(victim,"",0);
				}
			}

			/* Fine chek tiro salvezza */

		}

	}
}



void spell_remove_paralysis(byte level, struct char_data* ch,
							struct char_data* victim, struct obj_data* obj) {

	assert(ch && victim);

	if(affected_by_spell(victim,SPELL_PARALYSIS)) {
		affect_from_char(victim,SPELL_PARALYSIS);
		act("$c0014Un dolce sensazione di $c0009calore$c0014 ti pervade.", TRUE, victim, 0, 0, TO_CHAR);
        act("$c0014Ti senti molto meglio.", TRUE, ch, 0, victim, TO_VICT);
		act("$c0014$N $c0014sembra stare meglio.", TRUE, ch, 0, victim, TO_NOTVICT);
	}

    if(IS_AFFECTED(victim, AFF_PARALYSIS)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_PARALYSIS);
    }

}


void spell_holy_word(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
}

void spell_unholy_word(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
}

void spell_succor(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	struct obj_data* o;

	o = read_object(3052,VIRTUAL);
	obj_to_char(o,ch);

	o->obj_flags.cost = 0;
	o->obj_flags.cost_per_day = -1;

	act("$n esegue un gesto con le mani e crea $p", TRUE, ch, o, 0, TO_ROOM);
	act("Esegui un gesto con le mani e crei $p.", TRUE, ch, o, 0, TO_CHAR);

}

void spell_detect_charm(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {

}

void spell_true_seeing(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim && ch);

	if(!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
		if(ch != victim) {
			send_to_char("I tuoi occhi si illuminano d'$c0015argento$c0007 per un attimo.\n\r", victim);
			act("Gli occhi di $n vengono illuminati da una $c0015luce argentea$c0007.\n\r", FALSE, victim, 0, 0, TO_ROOM);
		}
		else {
			send_to_char("I tuoi occhi si illuminano d'$c0015argento$c0007.\n\r", ch);
			act("Gli occhi di $n vengono illuminati da una $c0015luce argentea$c0007.\n\r", FALSE, ch, 0, 0, TO_ROOM);
		}

		af.type      = SPELL_TRUE_SIGHT;
		af.duration  = 2*level;
		af.modifier  = level+10;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_TRUE_SIGHT;
		affect_to_char(victim, &af);
	}
	else {
		send_to_char("Non sembra succedere nulla.\n\r", ch);
	}
}




/*
   magic user spells
*/

void spell_track(byte level, struct char_data* ch,
				 struct char_data* targ, struct obj_data* obj) {
	struct affected_type af;

	if(ch != targ) {
		send_to_char("I tuoi occhi $c0015brillano$c0007 per un attimo di un color $c0010verde smeraldo$c0007!\n\r", targ);
	}
	else {
		send_to_char("I tuoi occhi $c0015brillano$c0007 per un attimo di un color $c0010verde smeraldo$c0007!\n\r", ch);
	}

	act("Gli occhi di $N $c0015brillano$c0007 per un attimo di un riflesso color $c0010verde smeraldo$c0007.", 0,  ch, 0, targ, TO_NOTVICT);

	if(!obj) {
		af.type      = SPELL_MINOR_TRACK;
		af.duration  = level;
	}
	else {
		af.type      = SPELL_MAJOR_TRACK;
		af.duration  = level*2;
	}

	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(targ, &af);


}

void spell_poly_self(byte level, struct char_data* ch,
					 struct char_data* mob, struct obj_data* obj) {

	char* buf;

	/* Check to make sure that there is no snooping going on.  */
	if(!ch->desc || ch->desc->snoop.snooping) {
		send_to_char("Gli Dei interferiscono con la riuscita dell'incantesimo.",
					 ch);
		extract_char(mob);
		return;
	}

	if(ch->desc->snoop.snoop_by) { /* force the snooper to stop */
		do_snoop(ch->desc->snoop.snoop_by, GET_NAME(ch->desc->snoop.snoop_by), 0);
	}

	/* Put mobile in correct room */

	char_to_room(mob, ch->in_room);
	SwitchStuff(ch, mob);

    if(GET_SEX(ch) == SEX_FEMALE)
    {
        if(GET_SEX(ch) != GET_SEX(mob) )
        {
            GET_SEX(mob) = GET_SEX(ch);
        }

        if(mob_index[mob->nr].iVNum == 1347)
        {
            mob->player.short_descr = (char*)strdup("una Incubus");
            mob->player.long_descr = (char*)strdup("Una Incubus, viaggiatrice dell'inconscio e' qui.\n");
        }
        else if(mob_index[mob->nr].iVNum == 1368)
        {
            mob->player.short_descr = (char*)strdup("una Drow");
            mob->player.long_descr = (char*)strdup("Una drow ti fissa minacciosamente poco prima di estrarre la sua spada.\n");
        }
        else if(mob_index[mob->nr].iVNum == 1372)
        {
            mob->player.short_descr = (char*)strdup("una CacciaCanguRo");
            mob->player.long_descr = (char*)strdup("Una Cacciatrice... Diamine, e' armata di una Canna!\n");
        }
    }

	/* move char to storage */

	act("Le carni di $c0015$n$c0007 si sciolgono e si riformano nella figura di $c0015$N$c0007!",
		TRUE, ch, 0, mob, TO_ROOM);

	act("Le tue carni si sciolgono e si riformano nella figura di $c0015$N$c0007!",
		TRUE, ch, 0, mob, TO_CHAR);

	char_from_room(ch);
	char_to_room(ch, 3);

	/* stop following whoever you are following..  */
	if(ch->master) {
		add_follower(mob,ch->master);
		stop_follower(ch);
	}

	/* switch caster into mobile */

	ch->desc->character = mob;
	ch->desc->original = ch;

	mob->desc = ch->desc;
	ch->desc = 0;

	SET_BIT(mob->specials.act, ACT_POLYSELF);
	SET_BIT(mob->specials.act, ACT_NICE_THIEF);
	SET_BIT(mob->specials.act, ACT_SENTINEL);
	REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
	REMOVE_BIT(mob->specials.act, ACT_META_AGG);
	REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
	REMOVE_BIT(mob->specials.act, ACT_HATEFUL); // SALVO nel poly e' bene togliere anche questo
	REMOVE_BIT(mob->specials.act, ACT_STAY_ZONE); // SALVO nel poly e' bene togliere anche questo
	/* ALAR: azzero i bonus move e mana */
	mob->points.max_mana=0;
	mob->points.max_move=0+GET_CON(mob)+number(1,20)-10;
	GET_MANA(mob) = (GET_MANA(ch));
	alter_mana(mob,0);
	WAIT_STATE(mob, PULSE_VIOLENCE*2); // poly

	/* do some fiddling with the strings */
	buf = (char*)malloc(strlen(GET_NAME(mob)) + strlen(GET_NAME(ch)) + 2);
	/*sprintf(buf, "%s %s", GET_NAME(ch), GET_NAME(mob));*/
    sprintf(buf, "%s", GET_NAME(ch));

	if HAS_PRINCE(ch) {
		GET_PRINCE(mob)=strdup(GET_PRINCE(ch));
	}

	/* ATTENZIONE ardent titans say that 'this code crashes' */

	if(GET_NAME(mob)) {
		free(GET_NAME(mob));
	}
	GET_NAME(mob) = buf;

	buf = (char*)malloc(strlen(mob->player.short_descr) +
						strlen(GET_NAME(ch)) + 2);
	sprintf(buf, "%s %s", GET_NAME(ch), mob->player.short_descr);

	if(mob->player.short_descr) {
		free(mob->player.short_descr);
	}
	mob->player.short_descr = buf;

	buf = (char*)malloc(strlen(mob->player.short_descr) + 12);
	sprintf(buf, "%s e' qui.\n\r", mob->player.short_descr);

	if(mob->player.long_descr) {
		free(mob->player.long_descr);
	}
	mob->player.long_descr = buf;

}

void spell_minor_create(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {

	assert(ch && obj);

	act("$n batte le mani e si concentra.", TRUE, ch, 0, 0, TO_ROOM);
	act("Batti le mani e ti concentri.", TRUE, ch, 0, 0, TO_CHAR);
	act("Con un $c0011lampo di luce$c0007 si materializza $p.", TRUE, ch, obj, 0, TO_ROOM);
	act("Con un $c0011lampo di luce$c0007 si materializza $p.", TRUE, ch, obj, 0, TO_CHAR);

	obj_to_room(obj, ch->in_room);

}


void spell_stone_skin(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch);

	if(!affected_by_spell(ch, SPELL_STONE_SKIN)) {
		act("La pelle di $n diventa $c0015grigia$c0007 ed assume la consistenza del $c0003granito$c0007.", TRUE, ch, 0, 0, TO_ROOM);
		act("$c0003La tua pelle si tramuta in una sostanza simile alla roccia.", TRUE, ch, 0, 0, TO_CHAR);

		af.type      = SPELL_STONE_SKIN;
		af.duration  = level;
		/*   af.modifier  = -40+(level/10)*5; questo e' chiaramente un BUG Gaia 2001 */
		af.modifier  = -40-(level/10)*5;
		af.location  = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(ch, &af);

		/* resistance to piercing weapons */

		af.type      = SPELL_STONE_SKIN;
		af.duration  = level;
		af.modifier  = IMM_PIERCE;
		af.location  = APPLY_IMMUNE;
		af.bitvector = 0;
		affect_to_char(ch, &af);
	}
}
void spell_mirror_images(byte level, struct char_data* ch,
						 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	int i = 0;
	assert(ch);

	if(!affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
		act("$n moltiplica la sua $c0015i$c0007i$c0015m$c0007m$c0015m$c0007m$c0015a$c0007a$c0015g$c0007gi$c0015n$c0007n$c0015e$c0007e!", TRUE, ch, 0, 0, TO_ROOM);
		act("Evochi delle $c0012immagini illusorie$c0007 per confondere i nemici.", TRUE, ch, 0, 0, TO_CHAR);
		for(i=1+(level/10); i; i--) {
			af.type      = SPELL_MIRROR_IMAGES;
			af.duration  = number(1,4)+(level/5);
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = 0;
			affect_to_char(ch, &af);
		}

	}
}



void spell_infravision(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim && ch);

	if(!IS_AFFECTED(victim, AFF_INFRAVISION)) {
		if(ch != victim) {
			send_to_char("I tuoi occhi $c0015brillano$c0007 di un $c0009alone rosso$c0007.\n\r", victim);
			act("Gli occhi di $n $c0015brillano$c0007 di un alone $c0009rosso$c0007.\n\r", FALSE, victim, 0, 0, TO_ROOM);
		}
		else {
			send_to_char("I tuoi occhi $c0015brillano$c0007 di un $c0009alone rosso$c0007.\n\r", ch);
			act("Gli occhi di $n $c0015brillano$c0007 di un $c0009alone rosso$c0007.\n\r", FALSE, ch, 0, 0, TO_ROOM);
		}

		af.type      = SPELL_INFRAVISION;
		af.duration  = 4*level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_INFRAVISION;
		affect_to_char(victim, &af);

	}
}

void spell_shield(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim && ch);

	if(!affected_by_spell(victim, SPELL_SHIELD)) {
		act("$N viene avvolto da un potente $c0011scudo$c0007 magico.",
			TRUE, ch, 0, victim, TO_NOTVICT);
		if(ch != victim) {
			act("$N viene avvolto da un potente $c0011scudo$c0007 magico.",
				TRUE, ch, 0, victim, TO_CHAR);
			act("Vieni avvolto da un potente $c0011scudo$c0007 magico.",
				TRUE, ch, 0, victim, TO_VICT);
		}
		else {
			act("Vieni avvolto da un potente $c0011scudo$c0007 magico.",
				TRUE, ch, 0, 0, TO_CHAR);
		}

		af.type      = SPELL_SHIELD;
		af.duration  = 8+level;
		af.modifier  = -10;
		af.location  = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(victim, &af);
	}
}

void spell_weakness(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	float modifier;

	assert(ch && victim);

	if(!affected_by_spell(victim,SPELL_WEAKNESS))
		if(!saves_spell(victim, SAVING_SPELL)) {
			modifier = level/200.0;
			act("$c0014Ti senti piu' debole.", FALSE, victim,0,0,TO_VICT);
			act("$c0014$n$c0014 sembra piu' debole.", FALSE, victim, 0, 0, TO_ROOM);

			af.type      = SPELL_WEAKNESS;
			af.duration  = (int) level/2;
			af.modifier  = (long)(- (victim->abilities.str * modifier));
			if(victim->abilities.str_add) {
				af.modifier -= 2;
			}
			af.location  = APPLY_STR;
			af.bitvector = 0;

			affect_to_char(victim, &af);
		}
}

/* invis_group non sembra assegnata a nessuna classe */
void spell_invis_group(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct char_data* tmp_victim;
	struct affected_type af;

	assert(ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = tmp_victim->next_in_room) {
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
			if(in_group(ch,tmp_victim)) {
				if(!affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {

					act("$c0011$n $c0011scompare lentamente dalla vista.", TRUE, tmp_victim,0,0,TO_ROOM);
					send_to_char("$c0011Svanisci.\n\r", tmp_victim);

					af.type      = SPELL_INVISIBLE;
					af.duration  = 24;
					af.modifier  = -(40+level);
					af.location  = APPLY_AC;
					af.bitvector = AFF_INVISIBLE;
					affect_to_char(tmp_victim, &af);
					af.type      = SPELL_INVISIBLE;
					af.duration  = 24;
					af.modifier  = level;
					af.location  = APPLY_NONE;
					af.bitvector = 0;
					affect_to_char(tmp_victim, &af);
				}
			}
	}
}


void spell_acid_blast(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {

	int dam;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,6);

	if(saves_spell(victim, SAVING_SPELL)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_ACID_BLAST, 5);

}

void spell_cone_of_cold(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {

	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,5);

	send_to_char("Un $c0014cono$c0007 di $c0014aria gelida$c0007 scaturisce dalle tue mani.\n\r", ch);
	act("Un $c0014cono$c0007 di $c0014aria gelida$c0007 scaturisce dalle mani di $n!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);

	for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = temp) {
		temp = tmp_victim->next_in_room;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL) && (!IS_NPC(tmp_victim))) { // SALVO non deve agire su un immortale
				return;
			}
			if(!in_group(ch, tmp_victim)) {
				act("Vieni $c0014congelato$c0007 fino alle $c0015ossa$c0007!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				if(saves_spell(tmp_victim, SAVING_SPELL)) {
					dam >>= 1;
				}
				MissileDamage(ch, tmp_victim, dam, SPELL_CONE_OF_COLD, 5);
			}
			else {
				act("Riesci ad evitare il $c0014cono gelido$c0007!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
	}
}

void spell_ice_storm(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	int dam;
	struct char_data* tmp_victim, *temp;

	assert(ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,4);

	send_to_char("Evochi una $c0015tempesta di ghiaccio$c0007!\n\r", ch);
	act("$n evoca una $c0015tempesta di ghiaccio$c0007!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);

	for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = temp) {
		temp = tmp_victim->next_in_room;
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL) && (!IS_NPC(tmp_victim))) { // SALVO non deve agire su un immortale
				return;
			}
			if(!in_group(ch, tmp_victim)) {
				act("Vieni colpito dalla $c0015tempesta$c0007!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
				if(saves_spell(tmp_victim, SAVING_SPELL)) {
					dam >>= 1;
				}
				MissileDamage(ch, tmp_victim, dam, SPELL_ICE_STORM, 5);
			}
			else {
				act("Riesci ad evitare la $c0015tempesta di ghiaccio$c0007!\n\r",
					FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
	}
}


void spell_poison_cloud(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {



}

void spell_major_create(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
}


void spell_sending(byte level, struct char_data* ch,
				   struct char_data* victim, struct obj_data* obj) {
}

void spell_meteor_swarm(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,12);

	if(saves_spell(victim, SAVING_SPELL)) {
		dam >>= 1;
	}

	MissileDamage(ch, victim, dam, SPELL_METEOR_SWARM, 5);

}

void spell_Create_Monster(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	struct char_data* mob;
	int rnum;
	struct room_data* rp;

	/* load in a monster of the correct type, determined by
	   level of the spell */

	/* really simple to start out with */

	if((rp = real_roomp(ch->in_room)) == NULL) {
		return;
	}

	if(IS_SET(rp->room_flags, TUNNEL)) {
		send_to_char("Non c'e' piu' spazio qui...\n\r", ch);
		return;
	}

	if(level <= 5) {
		mob = read_mobile(16034, VIRTUAL);
#if 0
		rnum = number(1,10)+200;      /* removed this plot of mobs */
		mob = read_mobile(rnum, VIRTUAL);
#endif
	}
	else if(level <= 7) {
		mob = read_mobile(9215, VIRTUAL); // SALVO monsum two ex 9191 che non esiste
#if 0
		rnum = number(1,10)+210;                /* removed this plot of mobs */
		mob = read_mobile(rnum, VIRTUAL);
#endif
	}
	else if(level <= 9) {
		rnum = number(1,10)+220;
		mob = read_mobile(rnum, VIRTUAL);
	}
	else if(level <= 11) {
		rnum = number(1,10)+230;
		mob = read_mobile(rnum, VIRTUAL);
	}
	else if(level <= 13) {
		rnum = number(1,10)+240;
		mob = read_mobile(rnum, VIRTUAL);
	}
	else if(level <= 15) {
		rnum = 250+number(1,10);
		mob = read_mobile(rnum, VIRTUAL);
	}
	else {
		rnum = 260+number(1,10);
		mob = read_mobile(rnum, VIRTUAL);
	}

	if(!mob) {
		send_to_char("L'evocazione fallisce!\n\r", ch);
		return;
	}

	char_to_room(mob, ch->in_room);
	RelateMobToCaster(ch, mob);
	act("$n esegue un gesto con la mano ed evoca $N!", TRUE, ch, 0, mob, TO_ROOM);
	act("Esegui un gesto con la mano ed evochi $N!", TRUE, ch, 0, mob, TO_CHAR);

	if(too_many_followers(ch)) {
		act("$N dice 'Non ci penso proprio ad unirmi a tutta questa gente!'",
			TRUE, ch, 0, mob, TO_ROOM);
		act("$N si rifiuta di unirsi al tuo gruppo!!", TRUE, ch, 0,
			mob, TO_CHAR);
	}
	else {


		/* charm them for a while */
		if(mob->master) {
			stop_follower(mob);
		}

		add_follower(mob, ch);

		af.type      = SPELL_CHARM_PERSON;

		if(GET_CHR(ch)) {
			if(IS_PC(ch) || ch->master) {
				af.duration  = follow_time(ch);
				af.modifier  = 0;
				af.location  = 0;
				af.bitvector = AFF_CHARM;
				affect_to_char(mob, &af);

			}
			else {
				SET_BIT(mob->specials.affected_by, AFF_CHARM);
			}
		}
		else {

			af.duration  = 24*18;

			af.modifier  = 0;
			af.location  = 0;
			af.bitvector = AFF_CHARM;
			affect_to_char(mob, &af);
		}
	}

	/*
	  adjust the bits...
	*/

	/*
	 get rid of aggressive, add sentinel
	*/

	if(IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
	}
	if(!IS_SET(mob->specials.act, ACT_SENTINEL)) {
		SET_BIT(mob->specials.act, ACT_SENTINEL);
	}


}




/*
   either
*/

void spell_light(byte level, struct char_data* ch,
				 struct char_data* victim, struct obj_data* obj) {

	/*
	   creates a ball of light in the hands.
	*/
	struct obj_data* tmp_obj;

	assert(ch);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

#if 0
	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	tmp_obj->name = strdup("sfera luce");
	tmp_obj->short_description = strdup("una sfera di $c0015luce$c0007");
	tmp_obj->description = strdup("C'e' una sfera di $c0015luce$c0007 qui per terra.");

	tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
	tmp_obj->obj_flags.value[2] = 24+level;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 10;
	tmp_obj->obj_flags.cost_per_day = 1;

	tmp_obj->next = object_list;
	object_list = tmp_obj;

	obj_to_char(tmp_obj,ch);

	tmp_obj->item_number = -1;
#else
	tmp_obj = read_object(20, VIRTUAL);  /* this is all you have to do */
	if(tmp_obj) {
		tmp_obj->obj_flags.value[2] = 24+level;
		obj_to_char(tmp_obj,ch);
	}
	else {
		send_to_char("Mi spiace, non riesco a creare la sfera di $c0015luce$c0007.\n\r", ch);
		return;
	}


#endif

	act("$n apre la mano e delle piccole sfere di $c0015energia$c0007 si uniscono nel suo palmo e formano $p.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("Apri la mano e delle piccole sfere di $c0015energia$c0007 si uniscono nel tuo palmo formando $p.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_fly(byte level, struct char_data* ch,
			   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if(victim && affected_by_spell(victim, SPELL_FLY)) {
		send_to_char("L'incantesimo sembra sprecato.\n\r",ch);
		return;
	}

	act("Ti senti piu' leggero dell'$c0012aria$c0007!", TRUE, ch, 0, victim, TO_VICT);
	if(victim != ch) {
		act("$N si alza in volo.", TRUE, ch, 0, victim, TO_CHAR);
	}
	else {
		send_to_char("Ti alzi in volo!\n\r", ch);
	}
	act("$N si alza in volo.", TRUE, ch, 0, victim, TO_NOTVICT);

	af.type      = SPELL_FLY;
	af.duration  = GET_LEVEL(ch, BestMagicClass(ch))+3;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = AFF_FLYING;
	affect_to_char(victim, &af);
}

void spell_fly_group(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	struct char_data* tch;

	assert(ch);

	if(real_roomp(ch->in_room) == NULL)  {
		return;
	}

	for(tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
		if(in_group(ch, tch)) {
			act("Ti senti piu' leggero dell'$c0012aria$c0007!", TRUE, ch, 0, tch, TO_VICT);
			if(tch != ch) {
				act("$N si alza in volo.", TRUE, ch, 0, tch, TO_CHAR);
			}
			else {
				send_to_char("Ti alzi in volo.", ch);
			}
			act("$N si alza in volo.", TRUE, ch, 0, tch, TO_NOTVICT);

			af.type      = SPELL_FLY;
			af.duration  = GET_LEVEL(ch, BestMagicClass(ch))+3;
			af.modifier  = 0;
			af.location  = 0;
			af.bitvector = AFF_FLYING;
			affect_to_char(tch, &af);
		}
	}
}

void spell_refresh(byte level, struct char_data* ch,
				   struct char_data* victim, struct obj_data* obj) {
	int dam;

	assert(ch && victim);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(level,4)+level;
	dam = MAX(dam,20);

	if((dam + GET_MOVE(victim)) > move_limit(victim)) {
		GET_MOVE(victim) = move_limit(victim);
		alter_move(victim,0);
	}
	else {
		GET_MOVE(victim) += dam;
		alter_move(victim,0);
	}

	send_to_char("Senti svanire la stanchezza.\n\r", victim);

}


void spell_water_breath(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {

	struct affected_type af;

	assert(ch && victim);

	act("$c0012Senti di poter respirare come un pesce!", TRUE, ch, 0, victim, TO_VICT);
	if(victim != ch) {
		act("$c0012$N$c0012 sembra respirare diversamente.", TRUE, ch, 0, victim, TO_CHAR);
	}
	act("$c0012$N$c0012 sembra respirare diversamente.", TRUE, ch, 0, victim, TO_NOTVICT);

	af.type      = SPELL_WATER_BREATH;
	af.duration  = GET_LEVEL(ch, BestMagicClass(ch))+3;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_WATERBREATH;
	affect_to_char(victim, &af);


}



void spell_cont_light(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	/*
	   creates a ball of light in the hands.
	*/
	struct obj_data* tmp_obj;

	assert(ch);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

#if 0
	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	tmp_obj->name = strdup("sfera luce brillante");
	tmp_obj->short_description = strdup("una sfera di $c0011luce$c0007");
	tmp_obj->description = strdup("C'e' una $c0015brillante$c0007 sfera di $c0011luce$c0007 per terra.");

	tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
	tmp_obj->obj_flags.value[2] = -1;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 40;
	tmp_obj->obj_flags.cost_per_day = 1;

	tmp_obj->next = object_list;
	object_list = tmp_obj;

	obj_to_char(tmp_obj,ch);

	tmp_obj->item_number = -1;
#else
	tmp_obj = read_object(20, VIRTUAL);  /* this is all you have to do */
	if(tmp_obj) {
		obj_to_char(tmp_obj,ch);
	}
	else {
		send_to_char("Non riesci a creare la sfera di $c0011luce$c0007.\n\r", ch);
		return;
	}
#endif

	act("$n batte due volte fra di loro le sue mani ed un $c0011lampo$c0007 illumina la stanza. \n\r$p appare nelle sue mani.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("Batti due volte fra di loro le tue mani ed un $c0011lampo$c0007 di $c0015luce$c0007 illumina la stanza. \n\r$p appare nelle tue mani.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_animate_dead(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* corpse) {
	struct char_data* mob;
	struct obj_data* obj_object,*next_obj;
	char buf[MAX_STRING_LENGTH];
	int r_num=100; /* virtual # for zombie */

	/*
	  some sort of check for corpse hood
	  */
	if((GET_ITEM_TYPE(corpse)!=ITEM_CONTAINER)||
			(!corpse->obj_flags.value[3])) {
		send_to_char("L'evocazione fallisce bruscamente!\n\r",ch);
		return;
	}

	mob = read_mobile(r_num, VIRTUAL);
	char_to_room(mob, ch->in_room);

	act("Con un $c0015arcano$c0007 rituale, $n anima un cadavere.", TRUE, ch,
		0, 0, TO_ROOM);
	act("$c0008$N$c0008 si alza da terra barcollando.", FALSE, ch, 0, mob, TO_ROOM);

	/*
	  zombie should be charmed and follower ch
	  */

	if(too_many_followers(ch)) {
		act("$N vede quanta gente ti sta seguendo e rifiuta di aggiungersi!",
			TRUE, ch, 0, mob, TO_CHAR);
		act("Vedi quanta gente sta seguendo $n e rifiuti di aggregarti!",
			TRUE, ch, 0, mob, TO_ROOM);
	}
	else {
		SET_BIT(mob->specials.affected_by, AFF_CHARM);
		add_follower(mob, ch);
	}

	GET_EXP(mob) = 0;
	IS_CARRYING_W(mob) = 0;
	IS_CARRYING_N(mob) = 0;

	/*
	  take all from corpse, and give to zombie
	  */

	for(obj_object=corpse->contains; obj_object; obj_object=next_obj) {
		next_obj = obj_object->next_content;
		obj_from_obj(obj_object);
		obj_to_char(obj_object, mob);
	}

	/*
	  set up descriptions and such
	  */
	sprintf(buf,"%s si muove lentamente qui.\n\r",corpse->short_description);
	mob->player.long_descr = (char*)strdup(buf);

	/*
	set up hitpoints
	*/
	mob->points.max_hit = dice(MAX(level/2,5),8);
	mob->points.hit = mob->points.max_hit/2;
	mob->player.sex = 0;

	GET_RACE(mob) = RACE_UNDEAD_ZOMBIE;
	mob->player.iClass = ch->player.iClass;

	/*
	  get rid of corpse
	  */
	extract_obj(corpse);

}

void spell_know_alignment(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	int ap;
	char buf[200], name[100];

	assert(victim && ch);

	if(IS_NPC(victim)) {
		strcpy(name,victim->player.short_descr);
	}
	else {
		strcpy(name,GET_NAME(victim));
	}

	ap = GET_ALIGNMENT(victim);

	if(ap > 700) {
		sprintf(buf,"$c0014%s$c0014 ha un animo candido come la neve.\n\r",name);
	}
	else if(ap > 350) {
		sprintf(buf, "$c0014%s$c0014 ha uno spiccato senso morale.\n\r",name);
	}
	else if(ap > 100) {
		sprintf(buf, "$c0010%s$c0010 e' spesso gentile e di buoni propositi.\n\r",name);
	}
	else if(ap > 25) {
		sprintf(buf, "$c0010%s$c0010 non e' poi cosi' male dopotutto...\n\r",name);
	}
	else if(ap > -25) {
		sprintf(buf, "$c0010%s$c0010 non sembra avere un comportamento ben definito.\n\r",name);
	}
	else if(ap > -100) {
		sprintf(buf, "$c0010Hai conosciuto di peggio rispetto a %s$c0010.\n\r",name);
	}
	else if(ap > -350) {
		sprintf(buf, "$c0009%s$c0009 potrebbe anche essere piu' gentile...\n\r",name);
	}
	else if(ap > -700) {
		sprintf(buf, "$c0009Probabilmente %s$c0009 ha avuto soltanto un'infanzia difficile...\n\r",name);
	}
	else {
		sprintf(buf,"Meglio che non dica nulla su %s...\n\r",name);
	}

	send_to_char(buf,ch);

}

void spell_dispel_magic(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	int yes=0;
	int i;

	int check_falling(struct char_data *ch);

	assert(ch && (victim || obj));

	if(obj) {
        if((IS_SET(obj->obj_flags.extra_flags2, ITEM2_EDIT) || IS_SET(obj->obj_flags.extra_flags2, ITEM2_INSERT) ||IS_SET(obj->obj_flags.extra_flags2, ITEM2_PERSONAL) || IS_SET(obj->obj_flags.extra_flags2, ITEM2_QUEST)) && level <= IMMORTALE)
        {
            act("$p resiste al tuo incantesimo.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }


		if(IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}


		if(IS_SET(obj->obj_flags.extra_flags, ITEM_BLESS)) {
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
		}

		if(level >= MAESTRO) {                   /* if level 45> then they can do this */
			if(IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
				REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
			}
			/* strip off everything */
			for(i=0; i<MAX_OBJ_AFFECT; i++) {
				obj->affected[i].location = 0;
			}
		}

		if(level >= IMMORTALE) {
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_HUM);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
		}

		return;
	}

	/* gets rid of infravision, invisibility, detect, etc */

	if(GetMaxLevel(victim)<=GetMaxLevel(ch)) {
		yes = TRUE;
	}
	else {
		yes = FALSE;
	}

	if(affected_by_spell(victim,SPELL_INVISIBLE))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_INVISIBLE);
			send_to_char("Torni visibile.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_DETECT_INVISIBLE))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_DETECT_INVISIBLE);
			send_to_char("Non percepisci piu' l'$c0011invisibile$c0007.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_DETECT_EVIL))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_DETECT_EVIL);
			send_to_char("Non percepisci piu' il $c0001Male$c0007.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_DETECT_MAGIC))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_DETECT_MAGIC);
			send_to_char("Non percepisci piu' la $c0012magia$c0007 che ti circonda.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_SENSE_LIFE))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_SENSE_LIFE);
			send_to_char("Ti senti meno in contatto con le forme di vita.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_SANCTUARY)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_SANCTUARY);
			send_to_char("Non ti senti piu' cosi' invulnerabile.\n\r",victim);
			act("L'aura di $c0015luce Divina$c0007 che avvolge $n scompare.",FALSE,victim,0,0,TO_ROOM);
		}
		/*
		 *  aggressive Act.
		 */
		if((victim->attackers < 6) && (!victim->specials.fighting) &&
				(IS_NPC(victim))) {
			set_fighting(victim, ch);
		}
	}

	if(IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
			send_to_char("Non ti senti piu' cosi' invulnerabile.\n\r",victim);
			act("La $c0015luce Divina$c0007 che avvolgeva il corpo di $n scompare.",FALSE,victim,0,0,TO_ROOM);
		}
		/*
		 *  aggressive Act.
		 */
		if((victim->attackers < 6) && (!victim->specials.fighting) &&
				(IS_NPC(victim))) {
			set_fighting(victim, ch);
		}
	}

	if(affected_by_spell(victim,SPELL_PROTECT_FROM_EVIL)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROTECT_FROM_EVIL);
			send_to_char("Ti senti meno protetto dalla $c0001malvagita'$c0007.\n\r",victim);
		}
	}
    if(affected_by_spell(victim,SPELL_PROT_FROM_EVIL_GROUP)) {
        if(yes || !saves_spell(victim, SAVING_SPELL)) {
            affect_from_char(victim,SPELL_PROT_FROM_EVIL_GROUP);
            send_to_char("Ti senti meno protetto dalla $c0001malvagita'$c0007.\n\r",victim);
        }
    }
	if(affected_by_spell(victim,SPELL_INFRAVISION)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_INFRAVISION);
			send_to_char("La tua visione diventa piu' $c0008scura$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_SLEEP)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_SLEEP);
			send_to_char("Non hai piu' cosi' tanto sonno.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_CHARM_PERSON)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_CHARM_PERSON);
			send_to_char("$c0014Ti senti nuovamente padrone delle tue azioni.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_WEAKNESS)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_WEAKNESS);
			send_to_char("Non ti senti piu' $c0014debole$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_STRENGTH)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_STRENGTH);
			send_to_char("Non ti senti piu' cosi' $c0009forte$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_ARMOR)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_ARMOR);
			send_to_char("Perdi la tua $c0011armatura Divina$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_DETECT_POISON)){
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_DETECT_POISON);
			send_to_char("Non riesci piu' a percepire i $c0010veleni$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_BLESS)){
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_BLESS);
			send_to_char("Senti di aver perso la tua $c0015benedizione Divina$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_FLY)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_FLY);
			send_to_char("Non voli piu'.\n\r",victim);
			check_falling(victim);
		}
	}
	if(affected_by_spell(victim,SPELL_WATER_BREATH)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_WATER_BREATH);
			send_to_char("Senti di non poter piu' $c0012respirare$c0007 come un $c0012pesce$c0007.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_FIRE_BREATH)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_FIRE_BREATH);
			send_to_char("Il $c0001fuoco$c0007 che ti avvolgeva si estingue.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_LIGHTNING_BREATH)){
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_LIGHTNING_BREATH);
			send_to_char("L'$c0012elettricita'$c0007 che ti avvolgeva si placa.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_GAS_BREATH)){
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_GAS_BREATH);
			send_to_char("La $c0011nube di gas$c0007 che ti avvolgeva scompare.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_FROST_BREATH)){
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_FROST_BREATH);
			send_to_char("Il $c0015ghiaccio$c0007 che ti avvolgeva si scioglie.\n\r",victim);
		}
	}
	if(affected_by_spell(victim,SPELL_FIRESHIELD)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_FIRESHIELD);
			send_to_char("Senti svanire il tuo $c0001scudo di fuoco$c0007.\n\r",victim);
			act("Lo $c0001scudo di fuoco$c0007 che proteggeva il corpo di $n svanisce.",FALSE,victim,0,0,TO_ROOM);
		}
		/*
		 *  aggressive Act.
		 */
		if((victim->attackers < 6) && (!victim->specials.fighting) &&
				(IS_NPC(victim))) {
			set_fighting(victim, ch);
		}
	}

	if(IS_AFFECTED(victim, AFF_FIRESHIELD)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			REMOVE_BIT(victim->specials.affected_by, AFF_FIRESHIELD);
			send_to_char("Senti svanire il tuo $c0001scudo di fuoco$c0007.\n\r",victim);
			act("Lo $c0001scudo di fuoco$c0007 che normalmente avvolge $n svanisce.",FALSE,victim,0,0,TO_ROOM);
		}
		/*
		 *  aggressive Act.
		 */
		if((victim->attackers < 6) && (!victim->specials.fighting) &&
				(IS_NPC(victim))) {
			set_fighting(victim, ch);
		}
	}

	if(affected_by_spell(victim,SPELL_FAERIE_FIRE))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_FAERIE_FIRE);
			send_to_char("Non ti senti piu' cosi' $c0013rosa$c0007.\n\r",victim);
			act("L'$c0013alone rosa$c0007 che circondava $n svanisce.", TRUE, ch, 0, 0, TO_ROOM);
		}

	if(affected_by_spell(victim,SPELL_MINOR_TRACK))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_MINOR_TRACK);
			send_to_char("Non riesci piu' a seguire le tracce.\n\r",victim);

		}

	if(affected_by_spell(victim,SPELL_MAJOR_TRACK))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_MAJOR_TRACK);
			send_to_char("Non riesci piu' a seguire le tracce.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_WEB))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_WEB);
            send_to_char("Le $c0008ragnatele$c0007 che ti avvolgevano si dissolvono.\n\r",victim);
        }

	if(affected_by_spell(victim, SPELL_SILENCE))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_SILENCE);
            send_to_char("Torni a parlare di nuovo.\n\r",victim);
        }

	if(affected_by_spell(victim, SPELL_TREE_TRAVEL))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_TREE_TRAVEL);
            send_to_char("Non ti senti piu' cosi' in contatto con gli $c0003alberi$c0007.\n\r", victim);
        }

	if(affected_by_spell(victim, SPELL_HEAT_STUFF))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_HEAT_STUFF);
            send_to_char("Non ti senti piu' cosi' $c0009incandescente$c0007.\n\r", victim);
        }

	if(affected_by_spell(victim, SPELL_HASTE))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_HASTE);
            send_to_char("$c0008Senti che il mondo va molto piu' piano adesso.\n\r", victim);
        }

	if(affected_by_spell(victim, SPELL_SLOW))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_SLOW);
            send_to_char("$c0015Senti i tuoi movimenti accellerare rapidamente.\n\r", victim);
        }

	if(affected_by_spell(victim, SPELL_BARKSKIN))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_BARKSKIN);
            send_to_char("Senti che la tua $c0003pelle$c0007 perde la consistenza del $c0003granito$c0007.\n\r", victim);
        }

	if(affected_by_spell(victim,SPELL_AID))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_AID);
			send_to_char("Perdi l'$c0015aiuto Divino$c0007.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_SHIELD))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_SHIELD);
			send_to_char("Senti che il tuo $c0011scudo$c0007 magico viene dissolto.\n\r",victim);
		}

	if(affected_by_spell(victim,SPELL_TRUE_SIGHT))
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			REMOVE_BIT(victim->specials.affected_by, AFF_TRUE_SIGHT);
			affect_from_char(victim,SPELL_TRUE_SIGHT);
			send_to_char("L'alone d'$c0015argento$c0007 nei tuoi occhi scompare.\n\r",victim);
		}

	if(affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_INVIS_TO_ANIMALS);
            send_to_char("Torni visibile... anche agli animali.\n\r",victim);
        }

	if(affected_by_spell(victim, SPELL_DRAGON_RIDE))
        if(yes || !saves_spell(victim, SAVING_SPELL))
        {
            affect_from_char(victim,SPELL_DRAGON_RIDE);
            send_to_char("Non ti senti piu' in grado di cavalcare i draghi!\n\r",victim);
        }

	if(affected_by_spell(victim,SPELL_GLOBE_DARKNESS)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			REMOVE_BIT(victim->specials.affected_by, AFF_GLOBE_DARKNESS);
			affect_from_char(victim,SPELL_GLOBE_DARKNESS);
			send_to_char("Il $c0008globo di oscurita'$c0007 che ti avvolgeva scompare.\n\r",victim);
			act("Il $c0008globo di oscurita'$c0007 che avvolge $n si dissolve.",FALSE,victim,0,0,TO_ROOM);
		}
	}

	if(affected_by_spell(victim,SPELL_GLOBE_MINOR_INV)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_GLOBE_MINOR_INV);
			send_to_char("Vedi svanire il tuo $c0012globo di protezione$c0014.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_GLOBE_MAJOR_INV)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_GLOBE_MAJOR_INV);
			send_to_char("Vedi svanire il tuo $c0014globo di protezione$c0014.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_ENERGY_DRAIN)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_ENERGY_DRAIN);
			send_to_char("Senti di dover temere i $c0008Non-Morti$c0007.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_DRAGON_BREATH)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_DRAGON_BREATH);
			send_to_char("La tua protezione contro il soffio dei $c0003draghi$c0007 scompare.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_WIZARDEYE)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_WIZARDEYE);
			send_to_char("Il tuo $c0011occhio magico$c0007 viene dissolto.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_BREATH_FIRE)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_BREATH_FIRE);
			send_to_char("La tua protezione contro il soffio di $c0009fuoco$c0007 dei draghi scompare.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_BREATH_FROST)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_BREATH_FROST);
			send_to_char("La tua protezione contro il soffio di $c0014ghiaccio$c0007 dei draghi scompare.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_BREATH_ELEC)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_BREATH_ELEC);
			send_to_char("La tua protezione contro il soffio $c0012elettrico$c0007 dei draghi scompare.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_BREATH_ACID)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_BREATH_ACID);
			send_to_char("La tua protezione contro il soffio $c0010acido$c0007 dei draghi scompare.\n\r",victim);
		}
	}

	if(affected_by_spell(victim,SPELL_PROT_BREATH_GAS)) {
		if(yes || !saves_spell(victim, SAVING_SPELL)) {
			affect_from_char(victim,SPELL_PROT_BREATH_GAS);
			send_to_char("La tua protezione contro il soffio $c0011gassoso$c0007 dei draghi scompare.\n\r",victim);
		}
	}

	if(level >= MAESTRO_DEGLI_DEI)  {

		if(affected_by_spell(victim,SPELL_ANTI_MAGIC_SHELL)) {
			if(yes || !saves_spell(victim, SAVING_SPELL)) {
				affect_from_char(victim,SPELL_ANTI_MAGIC_SHELL);
				send_to_char("Il tuo scudo $c0012anti-magia$c0007 si dissolve.\n\r",victim);
			}
		}

		if(affected_by_spell(victim,SPELL_BLINDNESS)) {
			if(yes || !saves_spell(victim, SAVING_SPELL)) {
				affect_from_char(victim,SPELL_BLINDNESS);
				send_to_char("Torni a $c0015vedere$c0007.\n\r",victim);
			}
		}

		if(affected_by_spell(victim,SPELL_PARALYSIS)) {
			if(yes || !saves_spell(victim, SAVING_SPELL)) {
				affect_from_char(victim,SPELL_PARALYSIS);
				act("Ti senti liber$b di muoverti.", FALSE, victim, 0, 0, TO_CHAR);
                act("$n sembra liber$b di muoversi ora.", FALSE, victim, 0, 0, TO_ROOM);
			}
		}


		if(affected_by_spell(victim,SPELL_POISON)) {
			if(yes || !saves_spell(victim, SAVING_SPELL)) {
				affect_from_char(victim,SPELL_POISON);
                send_to_char("$c0014Ti senti meglio.\n\r", victim);
			}
		}
	}

    if(level >= QUESTMASTER) {

        if(affected_by_spell(victim,STATUS_QUEST) && IS_PC(victim)) {
            affect_from_char(victim,STATUS_QUEST);
            if(victim->specials.quest_ref)
            {
                if(real_roomp((victim->specials.quest_ref)->in_room)->people)
                {
                    act("\n\r$c0014$n$c0014 ha perso il senso della sua esistenza...$c0007", FALSE, victim->specials.quest_ref, 0, 0, TO_ROOM);
                }
                extract_char(victim->specials.quest_ref);
            }
            victim->specials.quest_ref = NULL;

        // Quest Achievement
            CheckQuestFail(victim);

            send_to_char("$c0011Non sei piu' in missione.\n\r",victim);
        }

    }
}



void spell_paralyze(byte level, struct char_data* ch,
					struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);


	if(!IS_AFFECTED(victim, AFF_PARALYSIS)) {
		if(IsImmune(victim, IMM_HOLD)) {
			FailPara(victim, ch);
			return;
		}
		if(IsResist(victim, IMM_HOLD)) {
			if(saves_spell(victim, SAVING_PARA)) {
				FailPara(victim, ch);
				return;
			}
			if(saves_spell(victim, SAVING_PARA)) {
				FailPara(victim, ch);
				return;
			}
		}
		else if(!IsSusc(victim, IMM_HOLD)) {
			if(saves_spell(victim, SAVING_PARA)) {
				FailPara(victim, ch);
				return;
			}
		}

		af.type      = SPELL_PARALYSIS;
		af.duration  = 4+level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_PARALYSIS;
		affect_join(victim, &af, FALSE, FALSE);

		act("$c0011Non riesci piu' a muoverti!",FALSE,victim,0,0,TO_CHAR);
		act("$c0011$n viene paralizzat$b!",TRUE,victim,0,0,TO_ROOM);
		GET_POS(victim)=POSITION_STUNNED;

	}
	else {
		send_to_char("Qualcuno cerca di $c0011paralizzarti$c0007 ANCORA!\n\r",victim);
	}
}

void spell_fear(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {

	assert(victim && ch);

	if(GetMaxLevel(ch) >= GetMaxLevel(victim)-2) {
		if(!saves_spell(victim, SAVING_SPELL))  {
			do_flee(victim, "", 0);

		}
		else {
			send_to_char("Hai $c0008paura$c0007, ma la sensazione passa.\n\r",victim);
			return;
		}
	}
}

void spell_calm(byte level, struct char_data* ch,
				struct char_data* victim, struct obj_data* obj) {
	assert(ch && victim);
	/*
	 * removes aggressive bit from monsters
	 */
	if(IS_NPC(victim)) {
		if(IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
			if(HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
				REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
				send_to_char("Hai una sensazione di $c0014pace$c0007.\n\r", ch);
			}
		}
		else {
			send_to_char("Ti senti in $c0014pace$c0007 con il mondo.\n\r", victim);
		}
	}
	else {
		send_to_char("Ti senti in $c0014pace$c0007 con il mondo.\n\r", victim);
	}
}

void spell_web(byte level, struct char_data* ch,
			   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	bool fail,pissed,big,no_effect;

	assert(ch && victim);

	big=pissed=fail=no_effect=FALSE;

	switch(GET_RACE(victim)) {
	case RACE_ARACHNID:
	case RACE_SLIME:
	case RACE_GHOST:
		act("$N ride della $c0008ragnatela$c0007 che hai lanciato!",FALSE, ch, 0, victim, TO_CHAR);
		act("Ahah, $n ti ha appena lanciato addosso una $c0008ragnatela$c0007, che stupidaggine...",FALSE,ch,0,victim,TO_VICT);
		act("$N ride delle $c0008ragnatele$c0007 lanciate da $n!",FALSE, ch, 0, victim, TO_NOTVICT);
		return;
		break;
	}

	if(!saves_spell(victim, SAVING_PARA)) {
		fail=TRUE;
	}

	if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
		big=TRUE;
	}

	if(IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		if(IsSmall(victim) && !fail && !number(0,3)) {
			pissed=TRUE;    /* 25% */
		}
		else if(big) {
			if(fail) {
				if(number(0,4) < 2) {
					pissed=TRUE;    /* 40% */
				}
			}
			else {
				if(number(0,4) < 3) {
					pissed=TRUE;    /* 60% */
				}
			}
		}
		else {
			if(fail) {
				if(!number(0,4)) {
					pissed=TRUE;    /* 20% */
				}
			}
			else {
				if(!number(0,2)) {
					pissed=TRUE;    /* 33% */
				}
			}
		}
	}
	else {                          /* assume if not indoors, outdoors and */
		/* web is less affective at blocking the */
		/* victim from the caster. */
		if(IsSmall(victim) && !fail && !number(0,2)) {
			pissed=TRUE;    /* 33% */
		}
		else if(big) {
			if(fail) {
				if(number(0,4) < 3) {
					pissed=TRUE;    /* 60% */
				}
			}
			else {
				pissed=TRUE;    /* 100% */
			}
		}
		else {
			if(fail) {
				if(number(0,4) < 2) {
					pissed=TRUE;    /* 40% */
				}
			}
			else {
				if(number(0,4) < 3) {
					pissed=TRUE;    /* 60% */
				}
			}
		}
	}

	if(fail) {
		af.type      = SPELL_WEB;
		af.duration  = level;
		af.modifier  = -50;
		af.location  = APPLY_MOVE;
		af.bitvector = 0;

		affect_to_char(victim, &af);
		if(!pissed) {
			act("Una $c0008ragnatela$c0007 appiccicosa ti blocca!", FALSE, ch, 0, victim, TO_VICT);
			act("Una $c0008ragnatela$c0007 appiccicosa avvolge e blocca $N!", FALSE, ch, 0, victim,TO_NOTVICT);
			act("Avvolgi $N in una $c0008ragnatela$c0007 appicciosa!",FALSE, ch, 0, victim, TO_CHAR);
		}
		else {
			act("La $c0008ragnatela$c0007 ti avvolge ma non ti ferma!", FALSE, ch,
				0, victim, TO_VICT);
			act("$N attacca, prestando poca attenzione alle $c0008ragnatele$c0007 che ha addosso.", FALSE,
				ch, 0, victim, TO_NOTVICT);
			act("Riesci soltanto a disturbare $N con le tue $c0008ragnatele$c0007, ack!", FALSE, ch,
				0, victim, TO_CHAR);
		}
	}
	else {
		if(pissed) {
			act("Sei quasi bloccato da una $c0008ragnatela$c0007, GRRRR!",
				FALSE, ch, 0, victim, TO_VICT);
			act("$N ringhia ed evita la $c0008ragnatela$c0007 di $n!",
				FALSE, ch, 0, victim,TO_NOTVICT);
			act("Manchi $N con le tue $c0008ragnatele$c0007! Uh oh, penso che siano guai ora.",
				FALSE, ch, 0, victim, TO_CHAR);
		}
		else {
			act("Osservi con divertimento $n lanciare $c0008ragnatele$c0007 per la stanza.",
				FALSE, ch, 0, victim, TO_VICT);
			act("$n manca $N con le sue $c0008ragnatele$c0007!", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Manchi $N con le $c0008ragnatele$c0007, ma non sembra accorgersene.",
				FALSE, ch, 0, victim, TO_CHAR);
		}
	}
	if(pissed)
		if(IS_NPC(victim) && !victim->specials.fighting) {
			set_fighting(victim,ch);
		}
}

void spell_heroes_feast(byte level, struct char_data* ch,
						struct char_data* victim, struct obj_data* obj) {
	struct char_data* tch;

	if(real_roomp(ch->in_room) == NULL)  {
		return;
	}

	for(tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
		if((in_group_strict(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
			send_to_char("$c0014Prendi parte ad un magnifico banchetto!\n\r", tch);
			gain_condition(tch,FULL,24);
			gain_condition(tch,THIRST,24);
			if(GET_HIT(tch) < GET_MAX_HIT(tch)) {
				GET_HIT(tch) +=1;
				alter_hit(tch,0);
			}
			GET_MOVE(tch) = GET_MAX_MOVE(tch);
			alter_move(tch,0);
		}
	}
}




void spell_conjure_elemental(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	/*
	 *   victim, in this case, is the elemental
	 *   object could be the sacrificial object
	 */

	assert(ch && victim && obj);

	/*
	** objects:
	**     fire  : red stone
	**     water : pale blue stone
	**     earth : grey stone
	**     air   : clear stone
	*/

	act("$n esegue uno strano rituale ed una $c0015nuvola$c0007 di $c0008fumo$c0007 si sprigiona nella stanza.", TRUE, ch, 0, 0, TO_ROOM);
	act("$n esegue uno strano rituale ed una $c0015nuvola$c0007 di $c0008fumo$c0007 si sprigiona nella stanza.", TRUE, ch, 0, 0, TO_CHAR);
	act("$p esplode con un sonoro $c0011BANG$c0007!", TRUE, ch, obj, 0, TO_ROOM);
	act("$p esplode con un sonoro $c0011BANG$c0007!", TRUE, ch, obj, 0, TO_CHAR);
	obj_from_char(obj);
	extract_obj(obj);
	char_to_room(victim, ch->in_room);
	act("Dal $c0008fumo$c0007 emerge $N.", TRUE, ch, 0, victim, TO_NOTVICT);

	/* charm them for a while */

	if(too_many_followers(ch)) {
		act("$N ti dice 'Non sperare che mi unisca a tutta quella gente, ma grazie del giro'",TRUE, ch, 0, victim, TO_ROOM);
		act("$N rifiuta di unirsi a tutta la gente che ti segue, ma ti ringrazia del giretto.", TRUE, ch, 0, victim, TO_CHAR);
	}
	else {

        RelateMobToCaster(ch,victim);


		if(victim->master) {
			stop_follower(victim);
		}

		add_follower(victim, ch);

		af.type      = SPELL_CHARM_PERSON;
		af.duration  = 20;
		af.modifier  = follow_time(ch);
		af.location  = 0;
		af.bitvector = AFF_CHARM;

		affect_to_char(victim, &af);
	}
}

void spell_faerie_fire(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim);

	if(affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
		send_to_char("Non succede nulla di nuovo.",ch);
		return;
	}

	act("$n indica $N.", TRUE, ch, 0, victim, TO_ROOM);
	act("Indichi $N che viene avvolt$b da un $c0013alone rosa$c0007.", TRUE, ch, 0, victim, TO_CHAR);
	act("$N viene avvolto da un $c0013alone rosa$c0007.", TRUE, ch, 0, victim, TO_ROOM);
	act("Evidenzi $N con un $c0013alone rosa$c0007.", TRUE, ch, 0, victim, TO_CHAR);

	af.type      = SPELL_FAERIE_FIRE;
	af.duration  = level;
	af.modifier  = 20;
	af.location  = APPLY_AC;
	af.bitvector = 0;

	affect_to_char(victim, &af);

}

void spell_faerie_fog(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct char_data* tmp_victim;

	assert(ch);

	act("$n schiocca le dita ed una $c0015nuvola$c0007 di vapore $c0005purpureo$c0007 si sprigiona improvvisamente.",
		TRUE, ch, 0, 0, TO_ROOM);
	act("Schiocchi le dita ed una $c0015nuvola$c0007 di vapore $c0005purpureo$c0007 si sprigiona improvvisamente.",
		TRUE, ch, 0, 0, TO_CHAR);


	for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
			tmp_victim = tmp_victim->next_in_room) {
		if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			if(IS_IMMORTAL(tmp_victim)) {
				break;
			}
			if(!in_group(ch, tmp_victim)) {
				if(IS_AFFECTED(tmp_victim, AFF_HIDE)) {
					if(saves_spell(tmp_victim, SAVING_SPELL))
                    {
                        if(saves_spell(tmp_victim, SAVING_SPELL))
                        {
                            act("Per un attimo ti sembra scorgere una sagoma, ma immediatamente dopo la $c0008nube$c0007 si dissolve.", TRUE, tmp_victim, 0, 0, TO_ROOM);
                            act("Diventi visibile per un attimo, ma $c0008scompari$c0007 di nuovo.", TRUE, tmp_victim, 0, 0, TO_CHAR);
                        }
                        else
                        {
                            REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                            act("$n diventa visibile per un attimo, ma improvvisamente $c0008scompare$c0007 di nuovo.", TRUE, tmp_victim, 0, 0, TO_ROOM);
                            act("Diventi visibile per un attimo, ma $c0008scompari$c0007 di nuovo.", TRUE, tmp_victim, 0, 0, TO_CHAR);
                            SET_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        }
					}
					else {
						REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
						act("$n viene scopert$b!",
							TRUE, tmp_victim, 0, 0, TO_ROOM);
						act("Vieni scopert$b!",
							TRUE, tmp_victim, 0, 0, TO_CHAR);
					}
				}
			}
		}
	}
}



void spell_cacaodemon(byte level, struct char_data* ch,
					  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(ch && victim && obj);

    act("$n esegue uno strano rituale ed una $c0015nuvola$c0007 di $c0008fumo nero$c0007 si sprigiona nella stanza.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n esegue uno strano rituale ed una $c0015nuvola$c0007 di $c0008fumo nero$c0007 si sprigiona nella stanza.", TRUE, ch, 0, 0, TO_CHAR);
	if(GET_LEVEL(ch, CLERIC_LEVEL_IND) > 40 && IS_EVIL(ch)) {
		act("$p emette un po' di $c0008fumo$c0007...", TRUE, ch, obj, 0, TO_ROOM);
		act("$p emette un po' di $c0008fumo$c0007...", TRUE, ch, obj, 0, TO_CHAR);
		obj->obj_flags.cost /= 2;
		if(obj->obj_flags.cost < 100) {
			act("$p improvvisamente prende $c0001fuoco$c0007 e si $c0001disintegra$c0007!",
				TRUE, ch, obj, 0, TO_ROOM);
			act("$p improvvisamente prende $c0001fuoco$c0007 e si $c0001disintegra$c0007!",
				TRUE, ch, obj, 0, TO_CHAR);
			obj_from_char(obj);
			extract_obj(obj);
		}
	}
	else {
		act("$p improvvisamente prende $c0001fuoco$c0007 e si $c0001disintegra$c0007!", TRUE, ch, obj, 0, TO_ROOM);
		act("$p improvvisamente prende $c0001fuoco$c0007 e si $c0001disintegra$c0007!", TRUE, ch, obj, 0, TO_CHAR);
		obj_from_char(obj);
		extract_obj(obj);
		if(!IS_IMMORTAL(ch)) {
			GET_ALIGNMENT(ch)-=5;
		}
	}
	char_to_room(victim, ch->in_room);

	act("Con una risata $c0001malvagia$c0007 $N emerge dal $c0008fumo$c0007!", TRUE, ch, 0, victim, TO_NOTVICT);

	if(too_many_followers(ch)) {
		act("$N dice 'Non c'e' nessuna possibilita' che mi unisca con tutta questa folla!'",
			TRUE, ch, 0, victim, TO_ROOM);
		act("$N rifiuta di unirsi a tutta la folla che gia' ti segue!", TRUE, ch, 0,
			victim, TO_CHAR);
	}
	else {

		/* charm them for a while */
		if(victim->master) {
			stop_follower(victim);
		}

		add_follower(victim, ch);

		af.type      = SPELL_CHARM_PERSON;
		af.duration  = follow_time(ch);
		af.modifier  = 0;
		af.location  = 0;
		af.bitvector = AFF_CHARM;

		affect_to_char(victim, &af);
	}
	if(IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
		REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
	}

	if(!IS_SET(victim->specials.act, ACT_SENTINEL)) {
		SET_BIT(victim->specials.act, ACT_SENTINEL);
	}
}

/*
 * neither
 */

void spell_improved_identify(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
}



void spell_geyser(byte level, struct char_data* ch,
				  struct char_data* victim, struct obj_data* obj) {
	int dam;

	struct char_data* tmp_victim, *temp;

	if(ch->in_room<0) {
		return;
	}
	dam =  dice(level,3);

	act("Il geyser erutta in un'enorme colonna di $c0015vapore$c0007!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);


	for(tmp_victim = real_roomp(ch->in_room)->people;
			tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next_in_room;
		if((ch != tmp_victim) && (ch->in_room == tmp_victim->in_room)) {
			if((GetMaxLevel(tmp_victim)<LOW_IMMORTAL)||(IS_NPC(tmp_victim))) {
				act("Sei scottat$b dall'$c0012acqua$c0007 $c0009bollente$c0007!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
				MissileDamage(ch, tmp_victim, dam, SPELL_GEYSER, 5);
			}
			else {
				act("Per tua fortuna riesci ad evitare la colonna di $c0015vapore$c0007!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
	}
}



void spell_green_slime(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	if(hpch < 10) {
		hpch = 10;
	}

	dam = (int)(hpch / 5);

	if(saves_spell(victim, SAVING_BREATH)) {
		dam >>= 1;
	}

	act("$c0010Le esalazioni emanate da $n$c0010 ti fanno star male!", FALSE, ch,
		NULL, victim, TO_VICT);

	damage(ch, victim, dam, SPELL_GREEN_SLIME, 5);
}

void spell_prot_dragon_breath(byte level, struct char_data* ch,
							  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_DRAGON_BREATH)) {
		if(ch != victim) {
			act("$n evoca attorno a se un $c0012globo di protezione$c0007 contro i draghi.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi un $c0012globo di protezione$c0007 contro i draghi attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te un $c0012globo di protezione$c0007 contro i draghi.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se un $c0012globo di protezione$c0007 contro i draghi.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te un $c0012globo di protezione$c0007 contro i draghi.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_DRAGON_BREATH;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"Il $c0012globo di protezione$c0007 dal soffio dei draghi avvolge gia' $N.");
		}
		else {
			sprintf(buf,"Il $c0012globo di protezione$c0007 dal soffio dei draghi ti avvolge gia'.");
		}
		act(buf, FALSE, ch, 0, victim, TO_CHAR);
	}
}




void spell_prot_energy_drain(byte level, struct char_data* ch,
							 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;

	assert(victim);

	if(!affected_by_spell(victim, SPELL_PROT_ENERGY_DRAIN)) {
		send_to_char("Evochi uno $c0014scudo$c0007 protettivo contro i $c0008Non-Morti$c0007.\n\r", ch);
        act("$n evoca uno $c0014scudo$c0007 protettivo contro i $c0008Non-Morti$c0007.", TRUE, ch, 0, 0, TO_ROOM);
		af.type      = SPELL_PROT_ENERGY_DRAIN;
		af.duration  = level >= LOW_IMMORTAL ? level: 3;
		af.modifier  = IMM_DRAIN;
		af.location  = APPLY_IMMUNE;
		af.bitvector = 0;
		affect_to_char(ch, &af);
	}
	else {
		send_to_char("Sei gia' protetto contro i $c0008Non-Morti$c0007.\n\r",ch);
	}
}

void spell_globe_darkness(byte level, struct char_data* ch,
						  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	assert(victim);

	if(!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
		if(ch != victim) {
			act("$n evoca attorno a $N il potere dell'$c0008oscurita'$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi il potere dell'$c0008oscurita'$c0007 intorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te il potere dell'$c0008oscurita'$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se il potere dell'$c0008oscurita'$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi intorno a te il potere dell'$c0008oscurita'$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_GLOBE_DARKNESS;
		af.duration  = level;
		af.modifier  = 5;
		af.location  = APPLY_HIDE;
		af.bitvector = AFF_GLOBE_DARKNESS;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"L'$c0008oscurita'$c0007 avvolge gia' %s!\n\r",GET_NAME(victim));
		}
		else {
			sprintf(buf,"L'$c0008oscurita'$c0007 ti avvolge gia'!\n\r");
		}
		send_to_char(buf,ch);
	}
}



void spell_prot_fire(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_FIRE)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_FIRE;
		af.modifier  = IMM_FIRE;
		af.location  = APPLY_IMMUNE; /* res to fire */
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007 attorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal $c0009fuoco$c0007 attorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}

void spell_prot_cold(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_COLD)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evochi attorno a te uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_COLD;
		af.modifier  = IMM_COLD;
		af.location  = APPLY_IMMUNE; /* res to fire */
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal $c0014freddo$c0007 attorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}

void spell_prot_energy(byte level, struct char_data* ch,
					   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_ENERGY)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_ENERGY;
		af.modifier  = IMM_ENERGY;
		af.location  = APPLY_IMMUNE; /* res to fire */
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dall'$c0011energia$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}

void spell_prot_elec(byte level, struct char_data* ch,
					 struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_ELEC)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_ELEC;
		af.modifier  = IMM_ELEC;
		af.location  = APPLY_IMMUNE; /* res */
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dall'$c0012elettricita'$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}

void spell_prot_dragon_breath_fire(byte level, struct char_data* ch,
								   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_BREATH_FIRE)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 protettivo contro il soffio $c0009infuocato$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 protettivo contro il soffio $c0009infuocato$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0009infuocato$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 protettivo contro il soffio $c0009infuocato$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0009infuocato$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_BREATH_FIRE;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal soffio $c0009infuocato$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal soffio $c0009infuocato$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}

void spell_prot_dragon_breath_frost(byte level, struct char_data* ch,
									struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_BREATH_FROST)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 protettivo contro il soffio $c0014ghiacciato$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 protettivo contro il soffio $c0014ghiacciato$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0014ghiacciato$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 protettivo contro il soffio $c0014ghiacciato$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0014ghiacciato$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_BREATH_FROST;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal soffio $c0014ghiacciato$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal soffio $c0014ghiacciato$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}


void spell_prot_dragon_breath_elec(byte level, struct char_data* ch,
								   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 protettivo contro il soffio $c0012elettrico$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 protettivo contro il soffio $c0012elettrico$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0012elettrico$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 protettivo contro il soffio $c0012elettrico$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0012elettrico$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_BREATH_ELEC;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal soffio $c0012elettrico$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal soffio $c0012elettrico$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}


void spell_prot_dragon_breath_acid(byte level, struct char_data* ch,
								   struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 protettivo contro il soffio $c0010acido$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 protettivo contro il soffio $c0010acido$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0010acido$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 protettivo contro il soffio $c0010acido$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0010acido$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_BREATH_ACID;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal soffio $c0010acido$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal soffio $c0010acido$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}


void spell_prot_dragon_breath_gas(byte level, struct char_data* ch,
								  struct char_data* victim, struct obj_data* obj) {
	struct affected_type af;
	char buf[254];

	if(!victim) {
		return;
	}

	if(!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
		if(ch != victim) {
			act("$n evoca uno $c0011scudo$c0007 protettivo contro il soffio $c0011gassoso$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi uno $c0011scudo$c0007 protettivo contro il soffio $c0011gassoso$c0007 attorno a $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n evoca attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0011gassoso$c0007.", FALSE, ch, 0, victim, TO_VICT);
		}
		else {
			act("$n evoca attorno a se uno $c0011scudo$c0007 protettivo contro il soffio $c0011gassoso$c0007.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("Evochi attorno a te uno $c0011scudo$c0007 protettivo contro il soffio $c0011gassoso$c0007.", FALSE, ch, 0, victim, TO_CHAR);
		}

		af.type      = SPELL_PROT_BREATH_GAS;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		af.duration  = (int)level/10;
		affect_to_char(victim, &af);
	}
	else {
		if(ch != victim) {
			sprintf(buf,"$N ha gia' uno $c0011scudo$c0007 di protezione dal soffio $c0011gassoso$c0007 intorno a se.");
		}
		else {
			sprintf(buf,"Hai gia' uno $c0011scudo$c0007 di protezione dal soffio $c0011gassoso$c0007 intorno a te.");
		}
		act(buf,FALSE,ch,0,victim,TO_CHAR);
	}
}
} // namespace Alarmud
