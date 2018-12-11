/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
* DaleMUD v2.0        Released 2/1994
* See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
* AlarMUD
* $Id: act.off.c,v 1.5 2002/03/23 20:49:38 Thunder Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
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
#include "act.off.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.other.hpp"
#include "comm.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "snew.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

ACTION_FUNC(do_ripudia) {
	char tmp[80];
	struct char_data* victim=NULL;
	only_argument(arg, tmp);

	if(*tmp) {
		victim = get_char_room_vis(ch, tmp);
	}
	if(!victim) {
		if (!IS_VASSALLOOF(ch,tmp)) {
			act("Capisco la concitazione.... ma non ne sei vassall$b!!",
				TRUE,ch,NULL,NULL,TO_CHAR);
		}
		else {
			free(GET_PRINCE(ch));
			GET_PRINCE(ch)= (char*) NULL;
			act("Beh... a quanto sembra il coraggio non e' il tuo forte....\n\r"
				"in ogni modo.... adesso sei liber$b",
				TRUE,ch,NULL,NULL,TO_CHAR);
			GET_EXP(ch)-=((int)GET_EXP(ch)/100*5);
		}
		return;
	}

	if(ch->specials.fighting || victim->specials.fighting) {
		send_to_char("Pensate a combattere!\n\r",ch);
		return;
	}
	if(IS_VASSALLOOF(ch,GET_NAME(victim))) {
		if IS_POLY(ch) {
			send_to_char("Non puoi farlo in qeusta forma\n\r",ch);
			return;
		}
		act("Guardi negli occhi $N e rompi il tuo giuramento di fedelta'!",
			TRUE,ch,NULL,victim,TO_CHAR);
		act("$n rompe il suo giuramento di fedelta'!",
			TRUE,ch,NULL,victim,TO_VICT);
		act("$n rompe il suo giuramento di fedelta' a $N!",
			TRUE,ch,NULL,victim,TO_NOTVICT);
		free(GET_PRINCE(ch));
		GET_PRINCE(ch)=(char*)NULL;
	}
	else if(IS_PRINCEOF(GET_NAME(ch),victim)) {
		if IS_POLY(victim) {
			send_to_char("Non puoi farlo in qeusta forma.\n\r",victim);
			return;
		}
		act("Guardi negli occhi $N e l$B scacci dal tuo casato!",
			TRUE,ch,NULL,victim,TO_CHAR);
		act("$n ti scaccia dal su$b casato!",
			TRUE,ch,NULL,victim,TO_VICT);
		act("$n scaccia $N dal su$b casato!",
			TRUE,ch,NULL,victim,TO_NOTVICT);
		free(GET_PRINCE(victim));
		GET_PRINCE(victim)=(char*)NULL;
	}
	return;
}


ACTION_FUNC(do_associa) {
	char tmp[80];
	long costobase=500000;
	long costo;
	struct char_data* victim=NULL;
	if(!IS_PRINCE(ch)) {
		send_to_char("Presuntuosetto, eh?\n\r",ch);
		return;
	}
	only_argument(arg, tmp);

	if(*tmp) {
		victim = get_char_room_vis(ch, tmp);
	}
	if(!victim) {
		send_to_char("Ottima idea nominare dei vassalli....."
					 "ma almeno cerca di scrivere bene il loro nome!\n\r",ch);
		return;
	}
	if IS_POLY(victim) {
		send_to_char("Fare tuo vassallo un animale? Ma chi ti credi d'essere?"
					 " Caligola???\n\r",ch);
		return;
	}

	if(ch->specials.fighting || victim->specials.fighting) {
		send_to_char("Pensate a combattere!\n\r",ch);
		return;
	}
	if(GetMaxLevel(victim)<VASSALLO) {
		act("$N e' troppo giovane per giurarti fedelta'",
			TRUE,ch,NULL,victim,TO_CHAR);
		act("Sei troppo giovane per giurare fedelta' a $n",
			TRUE,ch,NULL,victim,TO_VICT);
		return;
	}

	if(GET_PRINCE(victim)) {
		if IS_VASSALLOOF(victim,GET_NAME(ch)) {
			act("$N e' gia' tu$b vassall$b",
				TRUE,ch,NULL,victim,TO_CHAR);
			act("$n ha cercato  di nominarti ANCORA su$b vassall$b!!",
				TRUE,ch,NULL,victim,TO_VICT);
		}
		else {
			act("$N ha' gia' giurato fedelta' a $T",
				TRUE,ch,NULL,GET_PRINCE(victim),TO_CHAR);
		}
		return;
	}
	/* Pant, pant.... tutto a posto..... controlliamo se ha abbastanza soldini
	 * */
	costo = ((17-(GET_RCHR(ch)))* 50000)+((GET_RCHR(victim)-12) * 25000);
	costo = costo + costobase ;
	if(GET_GOLD(ch)<costo) {
		act("Ti costerebbe troppo....",
			TRUE,ch,NULL,NULL,TO_CHAR);
		return;
	}
	GET_GOLD(ch)=GET_GOLD(ch)-costo;
	snprintf(tmp,79,"Il che ti costa %d monete d'oro!",(int)costo);
	GET_PRINCE(victim)=strdup(GET_NAME(ch));
	act("Nomini $N tu$B vassall$B",
		TRUE,ch,NULL,victim,TO_CHAR);
	act(tmp,TRUE,ch,NULL,victim,TO_CHAR);
	act("Ti inginocchi e giuri fedelta' a $n",
		TRUE,ch,NULL,victim,TO_VICT);
	act("$N si inginocchia e $n l$B nomina su$B vassall$B!",
		TRUE,ch,NULL,victim,TO_NOTVICT);
	return;
}
ACTION_FUNC(do_vomita) {
	act("Ti  ficchi un dito in gola e cerchi di vomitare... che schifo!",
		TRUE,ch,NULL,NULL,TO_CHAR);
	act("$n si ficca un dito in gola....!",
		TRUE,ch,NULL,NULL,TO_ROOM);
	if(number(0,1)) {
		act("Il risultato..... beh, e' quello che puoi immaginare!",
			TRUE,ch,NULL,NULL,TO_CHAR);
		act("$n si vomita addosso!",
			TRUE,ch,NULL,NULL,TO_ROOM);
		GET_COND(ch,FULL)=0;
		GET_COND(ch,DRUNK)=0;
		GET_COND(ch,THIRST)=0;
		WAIT_STATE(ch,PULSE_VIOLENCE * 3);
	}
	else {
		act("Oltretutto riesci solo a morderti un dito!",
			TRUE,ch,NULL,NULL,TO_CHAR);
		act("$n si morde un dito!",
			TRUE,ch,NULL,NULL,TO_ROOM);
	}
	WAIT_STATE(ch,PULSE_VIOLENCE * 3);
	return;
}

ACTION_FUNC(do_hit) {
	char tmp[80];
	struct char_data* victim;

	if(check_peaceful(ch,
					  "Non in questo luogo di pace.\n\r")) {
		return;
	}

	only_argument(arg, tmp);

	if(*tmp) {
		victim = get_char_room_vis(ch, tmp);
		if(victim) {
			if(victim == ch) {
				send_to_char("You hit yourself..OUCH!.\n\r", ch);
				act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
				GET_EXP(ch)-=5;
			}
			else {
				if(!IS_PC(ch) && !IS_PC(victim) && affected_by_spell(victim,STATUS_QUEST)) {
					act("Hai l'impressione che $n voglia aggredire $N... ma qualcosa lo frena.",
						FALSE, ch,0,victim,TO_ROOM);
					return;
				}
				if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
					act("$N is just such a good friend, you simply can't hit $M.",
						FALSE, ch,0,victim,TO_CHAR);
					return;
				}
				if(GET_POS(ch)>=POSITION_STANDING && !ch->specials.fighting) {
					if(hit(ch, victim, TYPE_UNDEFINED) == AllLiving) {   // Attempting align check on a dead victim could crash the mud
						ActionAlignMod(ch,victim,cmd);
						WAIT_STATE(ch, PULSE_VIOLENCE+2);
					}
				}
				else {
					if(victim != ch->specials.fighting) {
						if(ch->skills && ch->skills[SKILL_SWITCH_OPP].learned) {
							if(number(1,101) < ch->skills[SKILL_SWITCH_OPP].learned) {
								stop_fighting(ch);
								if(victim->attackers < 5) {
									ActionAlignMod(ch,victim,cmd);
									set_fighting(ch, victim);
								}
								else {
									send_to_char("There's no room to switch!\n\r", ch);
								}
								send_to_char("You switch opponents\n\r", ch);
								act("$n switches targets", FALSE, ch, 0, 0, TO_ROOM);
								WAIT_STATE(ch, PULSE_VIOLENCE+2); // switch
							}
							else {
								send_to_char("You try to switch opponents, but you become confused!\n\r", ch);
								stop_fighting(ch);
								LearnFromMistake(ch, SKILL_SWITCH_OPP, 0, 95);
								WAIT_STATE(ch, PULSE_VIOLENCE*2); // switch
							}
						}
						else {
							send_to_char("You do the best you can!\n\r",ch);
						}
					}
					else {
						send_to_char("You do the best you can!\n\r",ch);
					}
				}
			}
		}
		else {
			send_to_char("They aren't here.\n\r", ch);
		}
	}
	else {
		send_to_char("Hit who?\n\r", ch);
	}
}

ACTION_FUNC(do_slay) {
	static char tmp[MAX_INPUT_LENGTH];
	struct char_data* victim;

	only_argument(arg, tmp);

	if(!*tmp) {
		send_to_char("Uccidere chi?\n\r", ch);
	}
	else {
		if(!(victim = get_char_room_vis(ch, tmp))) {
			send_to_char("Non e' qui.\n\r", ch);
		}
		else {

			if(!IS_NPC(victim)) {
				act("Non e' un bersaglio valido.", FALSE, ch, 0, victim, TO_CHAR);
			}
			else {
				if(ch == victim) {
					send_to_char("Tua madre potrebbe rattristarsi per questo... :(\n\r", ch);
				}
				else {
					act("Distruggi senza alcuna pieta' $N!", FALSE, ch, 0, victim, TO_CHAR);
					raw_kill(victim, 0);
				}
			}
		}
	}
}
ACTION_FUNC(do_kill) {
	static char tmp[MAX_INPUT_LENGTH];
	struct char_data* victim;

	if((GetMaxLevel(ch) < CREATORE) || IS_NPC(ch)) {
		do_hit(ch, arg, 0);
		return;
	}

	only_argument(arg, tmp);

	if(!*tmp) {
		send_to_char("Kill who?\n\r", ch);
	}
	else {
		if(!(victim = get_char_room_vis(ch, tmp))) {
			send_to_char("They aren't here.\n\r", ch);
		}
		else if(ch == victim) {
			send_to_char("Your mother would be so sad.. :(\n\r", ch);
		}
		else {
			act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
			act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
			act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
			raw_kill(victim, 0);
		}
	}
}



ACTION_FUNC(do_backstab) {
	struct char_data* victim;
	char name[256];
	byte percent, base=0;
	int location = 12;
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Chi vuoi accoltellare?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Come puoi pugnalare te stesso?\n\r", ch);
		return;
	}

	if(!HasClass(ch, CLASS_THIEF)) {
		send_to_char("Non sei un ladro!\n\r", ch);
		return;
	}

	if(!ch->equipment[WIELD]) {
		send_to_char("E' necessario impugnare un'arma.\n\r",ch);
		return;
	}

	if(ch->attackers) {
		send_to_char("Non c'e' modo di raggiungere la schiena mentre stai "
					 "combattendo!\n\r", ch);
		return;
	}

	if(victim->attackers >= 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r", ch);
		return;
	}

	if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
		if(!IsGiant(ch)) {
			act("$N e' troppo gross$B per pugnalarl$B alla schiena.", FALSE, ch, 0,
				victim, TO_CHAR);
			return;
		}
	}

	if(ch->equipment[WIELD]->obj_flags.value[3] != 11 &&
			ch->equipment[WIELD]->obj_flags.value[3] != 1  &&
			ch->equipment[WIELD]->obj_flags.value[3] != 10) {
		send_to_char("Puoi pugnalare solo con armi taglienti od appuntite.\n\r",
					 ch);
		return;
	}

	if(ch->specials.fighting) {
        act("Sei troppo impegnat$b, ora.", FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Come puoi sorprendere qualcuno con tutto il rumore che fa "
					 "la tua cavalcatura?\n\r", ch);
		return;
	}

	if(victim->specials.fighting) {
		base = 0;
	}
	else {
		base = 4;
	}

	if(victim->skills && victim->skills[SKILL_AVOID_BACK_ATTACK].learned &&
			GET_POS(victim) > POSITION_SITTING) {
		percent=number(1,101); /* 101% is a complete failure */
		if(percent < victim->skills[SKILL_AVOID_BACK_ATTACK].learned) {
			act("Ti accorgi del tentativo di attacco di $N e lo eviti abilmente!",
				FALSE, victim, 0, ch, TO_CHAR);
			act("$n evita l'attacco alla schiena di $N!", FALSE, victim, 0, ch,
				TO_ROOM);
			SetVictFighting(ch,victim); /* he avoided, so make him hit! */
			SetCharFighting(ch,victim);
			if(IS_NPC(victim)) {
				AddHated(victim, ch);
			}
			return;
		}
		else {
			act("Non ti sei accorto dell'attacco alla schiena di $N!",
				FALSE, victim, 0, ch, TO_CHAR);
			act("$n non si accorge dell'attacco alla schiena di $N!",
				FALSE, victim, 0, ch, TO_ROOM);
			LearnFromMistake(victim, SKILL_AVOID_BACK_ATTACK, 0, 95);
		}

	}  /* ^ they had skill avoid ba and where awake! ^ */


	percent = number(1, 101);   /* 101% is a complete failure */

	if(ch->skills[SKILL_BACKSTAB].learned) {
		if(percent > MIN(100, ch->skills[SKILL_BACKSTAB].learned)) {
			LearnFromMistake(ch, SKILL_BACKSTAB, 0, 95);
			if(AWAKE(victim)) {
				if(damage(ch, victim, 0, SKILL_BACKSTAB, location) == AllLiving)
					if(IS_NPC(victim)) {
						AddHated(victim, ch);
					}
			}
			else {
				/* failed but vic is asleep */
				ActionAlignMod(ch,victim,cmd);
				base += 2;
				if(IS_NPC(victim)) {
					AddHated(victim, ch);
				}
				GET_HITROLL(ch) += base;
				if(hit(ch, victim, SKILL_BACKSTAB) == SubjectDead) {
					return;
				}
				GET_HITROLL(ch) -= base;
			}
		}
		else {
			ActionAlignMod(ch,victim,cmd);
			if(IS_NPC(victim)) {
				AddHated(victim, ch);
			}
#if !defined NEW_ALIGN
			if(IS_PC(ch) && IS_PC(victim)) {
				GET_ALIGNMENT(ch) -= 50;
			}
#endif
			GET_HITROLL(ch) += base;
			if(hit(ch, victim, SKILL_BACKSTAB) == SubjectDead) {
				return;
			}
			GET_HITROLL(ch) -= base;
		}
	}
	else {
		if(IS_NPC(victim)) {
			AddHated(victim, ch);
		}
		if(damage(ch, victim, 0, SKILL_BACKSTAB, location) == SubjectDead) {
			return;
		}
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}



ACTION_FUNC(do_order) {
	char name[100], message[256];
	char buf[256];
	bool found = FALSE;
	int org_room;
	struct char_data* victim;
	struct follow_type* k;


	if(apply_soundproof(ch)) {
		return;
	}

	half_chop(arg, name, message,sizeof name -1,sizeof message -1);

	/* Gia' che me lo sto passando faccio pure le traduzioni :-) */

	if(!*name || !*message)
		/*send_to_char("Order who to do what?\n\r", ch);*/
	{
		send_to_char("Ordinare a chi di fare cosa?!?\n\r", ch);
	}
	else if(!(victim = get_char_room_vis(ch, name)) &&
			str_cmp("follower", name) && str_cmp("followers", name))
		/*send_to_char("That person isn't here.\n\r", ch);*/
	{
		send_to_char("Quella persona non e' qui.\n\r", ch);
	}
	else if(ch == victim)
		/*send_to_char("You obviously suffer from Multiple Personality Disorder.\n\r", ch);*/
	{
		send_to_char("Mi sa che soffri di seri disturbi della personalita'...\n\r", ch);
	}

	else {
		if(IS_AFFECTED(ch, AFF_CHARM)) {
			/*send_to_char("Your superior would not approve of you giving orders.\n\r",ch);*/
			send_to_char("Il tuo padrone di sicuro non approverebbe che tu ti metta a dare ordini...\n\r",ch);
			return;
		}

		if(victim) {
			if(check_soundproof(victim)) {
				return;
			}
			/*snprintf(buf, 255,"$N orders you to '%s'", message);*/
			snprintf(buf, 255,"$N ti ordina di '%s'", message);
			act(buf, FALSE, victim, 0, ch, TO_CHAR);

			if(GetMaxLevel(ch)<IMMORTALE)																/* Aggiungo il check per non */
				/*act("$n gives $N an order.", FALSE, ch, 0, victim, TO_NOTVICT);*/ {		/*	far vedere ai player		  */
				act("$n da' un ordine a $N . ", FALSE, ch, 0, victim, TO_NOTVICT);    /* un ordine IMMORTALE		  */
			}
			/******* FLYP 20020610 *******/
			if(victim->master != ch || !IS_AFFECTED(victim, AFF_CHARM)) {
				if(RIDDEN(victim) == ch) {
					int check;
					check = MountEgoCheck(ch, victim);
					if(check > 5) {
						if(RideCheck(ch, -5)) {
							/*act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);*/
							act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
						}
						else {
							Dismount(ch, victim, POSITION_SITTING);
							act("$n gets pissed and $N falls on $S butt!",    /* Questa non so proprio come tradurla... */
								FALSE, victim, 0, ch, TO_NOTVICT);
							act("$n gets pissed you fall off!",
								FALSE, victim, 0, ch, TO_VICT);
						}
					}
					else if(check > 0) {
						/*act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);*/
						act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
					}
					else {
						send_to_char("Ok.\n\r", ch);
						command_interpreter(victim, message);
					}
				}
				else if(IS_AFFECTED2(victim, AFF2_CON_ORDER) && victim->master==ch &&
						((!victim->desc && !victim->specials.tick_to_lag) ||
						 (victim->desc && victim->desc->wait <= 1))) {
					send_to_char("Ok.\n\r", ch);
					command_interpreter(victim, message);
				}
				else {
					/*act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);*/
					act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
				}
			}
			else {
				send_to_char("Ok.\n\r", ch);
				WAIT_STATE(victim, (19-GET_CHR(ch)) * PULSE_VIOLENCE);   // order

				command_interpreter(victim, message);
			}
		}
		else {
			/* This is order "followers" */

			if(GetMaxLevel(ch)<IMMORTALE)													/* Aggiungo il check per non */
				/*snprintf(buf,255, "$n issues the order '%s'.", message);*/ {	/*	far vedere ai player		  */
				snprintf(buf,255, "$n ordina '%s'.", message);    /* un ordine IMMORTALE		  */
			}
			/******* FLYP 20020610 *******/

			act(buf, FALSE, ch, 0, victim, TO_ROOM);

			org_room = ch->in_room;

			for(k = ch->followers; k; k = k->next) {
				if(org_room == k->follower->in_room) {
					if(IS_AFFECTED(k->follower, AFF_CHARM)) {
						found = TRUE;
					}
				}
			}

			if(found) {
				for(k = ch->followers; k; k = k->next) {
					if(org_room == k->follower->in_room) {
						if(IS_AFFECTED(k->follower, AFF_CHARM)) {
							command_interpreter(k->follower, message);
						}
					}
				}
				send_to_char("Ok.\n\r", ch);
			}
			else {
				/*send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);*/
				send_to_char("Nessuno da queste parti e' un tuo suddito fedele!\n\r", ch);
			}
		}
	}
}

ACTION_FUNC(do_order_old) {
	char name[100], message[256];
	char buf[256];
	bool found = FALSE;
	int org_room;
	struct char_data* victim;
	struct follow_type* k;


	if(apply_soundproof(ch)) {
		return;
	}

	half_chop(arg, name, message,sizeof name -1,sizeof message -1);

	if(!*name || !*message) {
		send_to_char("Order who to do what?\n\r", ch);
	}
	else if(!(victim = get_char_room_vis(ch, name)) &&
			str_cmp("follower", name) && str_cmp("followers", name)) {
		send_to_char("That person isn't here.\n\r", ch);
	}
	else if(ch == victim) {
		send_to_char("You obviously suffer from Multiple Personality Disorder.\n\r", ch);
	}

	else {
		if(IS_AFFECTED(ch, AFF_CHARM)) {
			send_to_char("Your superior would not approve of you giving orders.\n\r",ch);
			return;
		}

		if(victim) {
			if(check_soundproof(victim)) {
				return;
			}
			snprintf(buf, 255,"$N orders you to '%s'", message);
			act(buf, FALSE, victim, 0, ch, TO_CHAR);
			act("$n gives $N an order.", FALSE, ch, 0, victim, TO_NOTVICT);

			if(victim->master != ch || !IS_AFFECTED(victim, AFF_CHARM)) {
				if(RIDDEN(victim) == ch) {
					int check;
					check = MountEgoCheck(ch, victim);
					if(check > 5) {
						if(RideCheck(ch, -5)) {
							act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
						}
						else {
							Dismount(ch, victim, POSITION_SITTING);
							act("$n gets pissed and $N falls on $S butt!",
								FALSE, victim, 0, ch, TO_NOTVICT);
							act("$n gets pissed you fall off!",
								FALSE, victim, 0, ch, TO_VICT);
						}
					}
					else if(check > 0) {
						act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
					}
					else {
						send_to_char("Ok.\n\r", ch);
						command_interpreter(victim, message);
					}
				}
				else if(IS_AFFECTED2(victim, AFF2_CON_ORDER) && victim->master==ch &&
						((!victim->desc && !victim->specials.tick_to_lag) ||
						 (victim->desc && victim->desc->wait <= 1))) {
					send_to_char("Ok.\n\r", ch);
					command_interpreter(victim, message);
				}
				else {
					act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
				}
			}
			else {
				send_to_char("Ok.\n\r", ch);
				WAIT_STATE(victim, (19-GET_CHR(ch)) * PULSE_VIOLENCE);   // order

				command_interpreter(victim, message);
			}
		}
		else {
			/* This is order "followers" */
			snprintf(buf,255, "$n issues the order '%s'.", message);
			act(buf, FALSE, ch, 0, victim, TO_ROOM);

			org_room = ch->in_room;

			for(k = ch->followers; k; k = k->next) {
				if(org_room == k->follower->in_room) {
					if(IS_AFFECTED(k->follower, AFF_CHARM)) {
						found = TRUE;
					}
				}
			}

			if(found) {
				for(k = ch->followers; k; k = k->next) {
					if(org_room == k->follower->in_room) {
						if(IS_AFFECTED(k->follower, AFF_CHARM)) {
							command_interpreter(k->follower, message);
						}
					}
				}
				send_to_char("Ok.\n\r", ch);
			}
			else {
				send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
			}
		}
	}
}



ACTION_FUNC(do_flee) {
	int i, attempt, loose, die, percent, charm;
	int panic=FALSE;
	int bonus=0;

	void gain_exp(struct char_data *ch, int gain);
	int special(struct char_data *ch, int cmd, char* tmp);
	char buf[250];
	if(IS_AFFECTED(ch, AFF_PARALYSIS)) {
		return;
	}

	if(GET_POS(ch) < POSITION_SLEEPING) {
		send_to_char("Not like this you can't!\n\r",ch);
		return;
	}

	if(IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
		send_to_char("You can think of nothing but the battle!\n\r",ch);
		return;
	}

	if(affected_by_spell(ch, SPELL_WEB)) {
		if(!saves_spell(ch, SAVING_PARA)) {
			WAIT_STATE(ch, PULSE_VIOLENCE); // flee
			send_to_char("You are ensared in webs, you cannot move!\n\r", ch);
			act("$n struggles against the webs that hold $m", FALSE,
				ch, 0, 0, TO_ROOM);
			return;
		}
		else {
			send_to_char("You pull free from the sticky webbing!\n\r", ch);
			act("$n manages to pull free from the sticky webbing!", FALSE,
				ch, 0, 0, TO_ROOM);
			affect_from_char(ch,SPELL_WEB);
			GET_MOVE(ch) -=50;
			alter_move(ch,0);
		}
	}

	if(GET_POS(ch) <= POSITION_SITTING) {
		GET_MOVE(ch) -=10;
		alter_move(ch,0);
		act("$n si rialza in preda al panico.", TRUE, ch, 0, 0, TO_ROOM);
		act("In preda al panico, riesci a malapena ad alzarti.", TRUE, ch, 0, 0,
			TO_CHAR);
		GET_POS(ch) = POSITION_STANDING;
		WAIT_STATE(ch, PULSE_VIOLENCE); // flee
		return;
	}

	if(!ch->specials.fighting) {
		for(i = 0; i < 6; i++) {
			attempt = number(0, 5);  /* Select a random direction */
			if(CAN_GO(ch, attempt) &&
					!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
				snprintf(buf,249,"$n cerca di fuggire %s, apparentemente senza motivo.",
						 dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);

				if(RIDDEN(ch)) {
					if((die = MoveOne(RIDDEN(ch), attempt, TRUE)) == TRUE) {
						/* The escape has succeded */
						snprintf(buf,249,"Corri a gambe levate %s.\n\r",dirsTo[attempt]);
						send_to_char(buf, ch);
						return;
					}
					else {
						if(!die)
							act("$n cerca di correre via ma e' incapace di muoversi!", TRUE, ch, 0, 0,
								TO_ROOM);
						return;
					}
				}
				else {
					if((die = MoveOne(ch, attempt, TRUE)) == TRUE) {
						/* The escape has succeded */
						snprintf(buf,249,"Corri a gambe levate %s.\n\r",dirsTo[attempt]);
						send_to_char(buf, ch);
						StopAllFightingWith(ch);
						return;
					}
					else {
						if(!die)
							act("$n cerca di fuggire ma e' incapace di muoversi!", TRUE, ch, 0, 0,
								TO_ROOM);
						return;
					}
				}
			}
		} /* for */
		/* No exits was found */
		send_to_char("PANICO! Non riesci a scappare!\n\r", ch);
		return;
	} /* fine se non fighting */

	/* Bene , stiamo combattendo.. le cose sono un po' piu' incasinate! */
	if(IS_PC(ch->specials.fighting)) {
		bonus=20;
	}
	for(i = 0; i < 3; i++) {
		attempt = number(0, 5);  /* Select a random direction */
		if(CAN_GO(ch, attempt) &&
				!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {

			if(!ch->skills || (number(1,101) > ch->skills[SKILL_RETREAT].learned+bonus)) {
				snprintf(buf,249,"$n va in panico e cerca di fuggire %s",dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);
				panic = TRUE;
				LearnFromMistake(ch, SKILL_RETREAT, 0, 90);
			}
			else {
				snprintf(buf,249,"$n abilmente si ritira %s.",dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);
				panic = FALSE;
			}


			if(IS_AFFECTED(ch, AFF_CHARM)) {
				charm = TRUE;
				REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);
			}
			else {
				charm = FALSE;
			}

			if(RIDDEN(ch)) {
				die = MoveOne(RIDDEN(ch), attempt, TRUE);
			}
			else {
				die = MoveOne(ch, attempt, TRUE);
			}

			if(charm) {
				SET_BIT(ch->specials.affected_by, AFF_CHARM);
			}

			if(die == TRUE) {
				loose = 0;
				/* The escape has succeded. We'll be nice. */
				if(bonus>0) {
					panic =FALSE;
				}
				if(GetMaxLevel(ch) > CHUMP) {
					if(panic) {
						/*loose = GetMaxLevel(ch)+( GetSecMaxLev(ch) / 2 )+( GetThirdMaxLev(ch) / 3 );
						loose -= GetMaxLevel(ch->specials.fighting)+
							( GetSecMaxLev(ch->specials.fighting) / 2 )+
							( GetThirdMaxLev(ch->specials.fighting ) / 3 );
						if( loose < 0 )
							loose = 1;
						loose *=10000;*/
					}
				}
				if(IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF) &&
								   !(IS_SET(ch->specials.act, ACT_AGGRESSIVE)))) {
					AddFeared(ch, ch->specials.fighting);
				}
				else {
					percent = (100 * GET_HIT(ch->specials.fighting)) /
							  GET_MAX_HIT(ch->specials.fighting);
					if(Hates(ch->specials.fighting, ch)) {
						SetHunting(ch->specials.fighting, ch);
					}
					else if((IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
							(IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))) {
						AddHated(ch->specials.fighting, ch);
						SetHunting(ch->specials.fighting, ch);
					}
					else if(number(1,101) < percent) {
						AddHated(ch->specials.fighting, ch);
						SetHunting(ch->specials.fighting, ch);
					}
				}

				if(IS_PC(ch) && panic) {
					if(HasClass(ch, CLASS_MONK | CLASS_WARRIOR | CLASS_BARBARIAN |
								CLASS_PALADIN | CLASS_RANGER))
						if(loose>0 && cmd!=999) {
							gain_exp(ch, -(loose/HowManyClasses(ch)));
							snprintf(buf,249,"La tua vigliaccheria ti e' costata"
									 " %d punti esperienza.\n\r", loose);
							send_to_char(buf,ch);
						}

				}
				if(panic) {
					send_to_char("Scappi a gambe levate.\n\r", ch);
				}
				else {
					char szBuffer[ 100 ];
					snprintf(szBuffer, 99,"Ti ritiri abilmente %s.\n\r",
							 dirsTo[ attempt ]);
					send_to_char(szBuffer, ch);
				}
				StopAllFightingWith(ch);
				if(ch->specials.fighting) {
					stop_fighting(ch);
				}
				return;
			}
			else {
				if(!die)
					act("$n cerca di scappare ma e' incapace di muoversi!", TRUE, ch, 0, 0,
						TO_ROOM);
				return;
			}
		} /*If can go */
	} /* for */
	/* No exits were found */
	send_to_char("PANICO! Non riesci a scappare!\n\r", ch);
}



ACTION_FUNC(do_bash) {
	struct char_data* victim;
	char name[256];
	int percent=0;
	int pesi=0;
	room_data* pRoom = real_roomp(ch->in_room);
	int location = 5 ;  /* Gaia 2001 */

	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch,"C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {

		if(!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
					 CLASS_BARBARIAN)) {
			send_to_char("Solo i combattenti possono farlo!\n\r", ch);
			return;
		}

		if(!IsHumanoid(ch)) {
			send_to_char("Non hai la forma adatta!\n\r", ch);
			return;
		}
	}

	if(pRoom->sector_type == SECT_UNDERWATER) {
		send_to_char("Sotto'acqua? Meglio di no.\n", ch);
		return;
	}


	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Chi vuoi colpire?\n\r", ch);
			return;
		}
	}


	if(victim == ch) {
		send_to_char("Molto spiritoso...\n\r", ch);
		return;
	}

	if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
		if(!IsGiant(ch)) {
			act("$N e' TROPPO gross$B per essere colpit$B!", FALSE, ch, 0, victim,
				TO_CHAR);
			return;
		}
	}

	if(MOUNTED(victim)) {
		send_to_char("Non puoi colpire la cavalcatura di qualcun altro!\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Non puoi colpire mentre cavalchi!\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 6) {
		send_to_char("Non riesci ad avvicinarti abbastanza per colpirlo!\n\r", ch);
		return;
	}

	percent = number(10, 100);   /* Lo skill bash arriva al massimo a 90 */
	/* GGPATCH : migliora la percentuale di riuscita del primo bash*/
	if(!ch->specials.fighting) {

		percent = number(10,60);
		/*
		 mudlog( LOG_CHECK,   "percent for 1st bash %d vs %d", percent,
		       ch->skills[ SKILL_BASH ].learned );
		*/
	}
	else {
		mudlog(LOG_CHECK, "percent for    bash %d vs %d", percent,
			   ch->skills[ SKILL_BASH ].learned);
	}


	/* GGEND   */
	/* some modifications to account for dexterity, and level */

	percent -= dex_app[(int)GET_DEX(ch) ].reaction * 10;
	percent += dex_app[(int)GET_DEX(victim) ].reaction * 10;
	percent += (int)(((GetMaxLevel(victim)-10) - GetMaxLevel(ch)) / 1.5);
#if NEW_BASH
	/* aggiungo un coefficiente di peggioramento legato al numero di oggetti
	 * trasportati e al loro peso - Aggiungo una parentesi Gaia 2001 */
	pesi += (((35 * IS_CARRYING_W(ch))/CAN_CARRY_W(ch))-20);
	pesi += (((30 * IS_CARRYING_N(ch))/CAN_CARRY_N(ch))-15);
	percent+=MAX(30,MAX(-5,pesi));
#endif
	/*  mudlog( LOG_CHECK, "percent for bash adj %d vs %d", percent,
	          ch->skills[ SKILL_BASH ].learned );
	 */
	if(percent > MIN(100, ch->skills[ SKILL_BASH ].learned)) {
		if(GET_POS(victim) > POSITION_DEAD) {
			if(damage(ch, victim, 0, SKILL_BASH, location) == SubjectDead) {
				return;
			}

			/* ACIDUS 2003: viene messo a sedere anche se \E8 polymorphato (\E8 un pg) */
			if(HasHands(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
				GET_POS(ch) = POSITION_SITTING;
			}

		}
		LearnFromMistake(ch, SKILL_BASH, 0, 90);
		if(CheckEquilibrium(ch)) {   //Acidus 2003 - skill better equilibrium
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);   // bash
		}
		else {
			WAIT_STATE(ch, PULSE_VIOLENCE * 3);    // bash
		}
	}
	else {
		if(!CheckMirror(victim)) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				if(damage(ch, victim, 2, SKILL_BASH, location) != VictimDead) {
					if(CheckEquilibrium(victim)) {
						WAIT_STATE(victim, PULSE_VIOLENCE * 1);   // bash
					}
					else {
						WAIT_STATE(victim, PULSE_VIOLENCE * 2);    // bash
					}

					if(HasHands(victim) || IS_SET(victim->specials.act,ACT_POLYSELF)) {
						GET_POS(victim) = POSITION_SITTING;
					}
				}
			}
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);
		}
	}
}




ACTION_FUNC(do_rescue) {
	struct char_data* victim, *tmp_ch;
	int percent;
	char victim_name[240];


	if(!ch->skills) {
		send_to_char("You fail the rescue.\n\r", ch);
		return;
	}

#if 0
	if(!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF)) {
		return;
	}
#endif

	if(check_peaceful(ch,"No one should need rescuing here.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
		if(!HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_PALADIN |
					 CLASS_RANGER)) {
			/**** SALVO skills prince ****/
			if(!IS_PRINCE(ch)) {
				/**** fine skills prince ****/
				send_to_char("Non sei un guerriero!\n\r", ch);
				return;
			}
		}
	}

	only_argument(arg, victim_name);

	if(!(victim = get_char_room_vis(ch, victim_name))) {
		if(HAS_PRINCE(ch)) {
			victim=get_char_room_vis(ch,GET_PRINCE(ch));
		}
		if(!victim) {
			send_to_char("Who do you want to rescue?\n\r", ch);
			return;
		}
	}

	if(victim == ch) {
		send_to_char("What about fleeing instead?\n\r", ch);
		return;
	}

	if(MOUNTED(victim)) {
		send_to_char("You can't rescue a mounted person!\n\r", ch);
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
		return;
	}

	if(victim->attackers >= 6) {
		send_to_char("You can't get close enough to them to rescue!\n\r", ch);
		return;
	}

	for(tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
			(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room) ;

	if(!tmp_ch) {
		act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}


	percent=number(1,101); /* 101% is a complete failure */
	if(IS_PRINCEOF(GET_NAME(victim),ch)) {
		percent=0;
	}
	if((percent > ch->skills[SKILL_RESCUE].learned)) {
		send_to_char("You fail the rescue.\n\r", ch);
		LearnFromMistake(ch, SKILL_RESCUE, 0, 90);
		WAIT_STATE(ch, PULSE_VIOLENCE); // rescue
		return;
	}
	ActionAlignMod(ch,victim,cmd);
	send_to_char("Banzai! To the rescue...\n\r", ch);
	act("You are rescued by $N, you are confused!", FALSE, victim,0,ch, TO_CHAR);
	act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
		GET_ALIGNMENT(ch)+=20;
	}

	if(victim->specials.fighting == tmp_ch) {
		stop_fighting(victim);
	}
	if(tmp_ch->specials.fighting) {
		stop_fighting(tmp_ch);
	}
	if(ch->specials.fighting) {
		stop_fighting(ch);
	}

	set_fighting(ch, tmp_ch);
	set_fighting(tmp_ch, ch);

	WAIT_STATE(victim, 2*PULSE_VIOLENCE); // rescue

}



ACTION_FUNC(do_support) {
	struct char_data* victim;
	char victim_name[240];
    char buf[255];

	only_argument(arg, victim_name);

    if(!*victim_name && ch->specials.supporting && !ch->specials.fighting)
    {
        sprintf(buf, "Al momento stai supportando %s, non te lo ricordavi?\n\r", ch->specials.supporting);
        send_to_char(buf, ch);
        return;
    }
    
	if(!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Chi vorresti supportare, esattamente?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Oh beh, chi fa da se' fa per tre!\n\r", ch);
		if(ch->specials.supporting) { //ACIDUS 2003 - il support deve andare via come con il bodyguard
			free(ch->specials.supporting);
			ch->specials.supporting=NULL;
		}
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Non mi sembra una grande idea\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Pensa a combattere!\n\r",ch);
		return;
	}
	if(ch->specials.supporting) {
		free(ch->specials.supporting);
	}
	ActionAlignMod(ch,victim,cmd);
	ch->specials.supporting=strdup(victim->player.name);
    sprintf(buf, "Ok, ora supporti %s.\n\r", ch->specials.supporting);
    send_to_char(buf, ch);
    
}
       
ACTION_FUNC(do_bodyguard) {
	struct char_data* victim,*lg;
	char victim_name[240];

	only_argument(arg, victim_name);

	if(!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Di chi vorresti fare la guardia del corpo.. esattamente?\n\r", ch);
		return;
	}

	if(victim == ch) {
		if(ch->specials.bodyguarding) {
			lg=get_char(ch->specials.bodyguarding);
			if(lg) {
				actall("Smetti di proteggere",
					   "smette di proteggerti",
					   "smette di proteggere",
					   ch,lg);
				free(ch->specials.bodyguarding);
				ch->specials.bodyguarding=(char*)NULL;
				free(lg->specials.bodyguard);
				lg->specials.bodyguard=(char*)NULL;
			}
			else {
				send_to_char("Chi fa  da se.. fa per tre!", ch);
			}
		}
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Non mi sembra una grande idea\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Pensa a combattere!\n\r",ch);
		return;
	}
	if(GetMaxLevel(ch)-GetMaxLevel(victim)>5) {
		act("BODYGUARD.. non BALIA! Cerca uno piu' grosso di $N a cui offrire"
			" i tuoi servigi!",FALSE,ch,0,victim,TO_CHAR);
		act("$n e' di livello troppo alto per farti da guardia del corpo!",
			FALSE,ch,0,victim,TO_VICT);
		return;
	}

	if(ch->specials.bodyguarding) {
		free(ch->specials.bodyguarding);
	}
	ch->specials.bodyguarding=strdup(GET_NAME(victim)); // SALVO meglio il vero nome
	if(victim->specials.bodyguard) {
		free(victim->specials.bodyguard);
	}
	victim->specials.bodyguard=strdup(GET_NAME(ch));
	act("Giuri di proteggere $N a rischio della tua stessa vita.",
		FALSE,ch,0,victim,TO_CHAR);
	act("$n si impegna a proteggerti, a rischio della sua stessa vita!",
		FALSE,ch,0,victim,TO_VICT);
	act("$n si impegna a proteggere $N a rischio della sua stessa vita!",
		FALSE,ch,0,victim,TO_NOTVICT);
	if(victim->desc) {
		ActionAlignMod(ch,victim,cmd);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE); /* same as hit */
}

ACTION_FUNC(do_assist) {
	struct char_data* victim, *tmp_ch;
	char victim_name[240];

	if(check_peaceful(ch,"Noone should need assistance here.\n\r")) {
		return;
	}

	only_argument(arg, victim_name);

	if(!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Who do you want to assist?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Oh, by all means, help yourself...\n\r", ch);
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("That would be counterproductive?\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("You have your hands full right now\n\r",ch);
		return;
	}

	if(victim->attackers >= 6) {
		send_to_char("You can't get close enough to them to assist!\n\r", ch);
		return;
	}


	tmp_ch = victim->specials.fighting;
	if(!tmp_ch) {
		act("But $E's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if(tmp_ch->in_room !=ch->in_room) {
		send_to_char("Woops, they left in a hurry, must have scared them off!\n\r",ch);
		return;
	}

	/*if(ch->specials.supporting) ACIDUS 2003 - il support non deve andare via da solo
	{
	   free(ch->specials.supporting);
	   ch->specials.supporting=NULL;
	}*/
	hit(ch, tmp_ch, TYPE_UNDEFINED);
	ActionAlignMod(ch,victim,cmd);
	WAIT_STATE(victim, PULSE_VIOLENCE+2); /* same as hit */
}



ACTION_FUNC(do_kick) {
	struct char_data* victim;
	char name[80];
	int dam;
	byte percent;
	int location = 5;    /* Gaia 2001 */
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch,
					  "Non in questo luogo di pace.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {

		if(!HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_RANGER|CLASS_PALADIN)
				&& !HasClass(ch, CLASS_MONK)) {
			send_to_char("You're no warrior!\n\r", ch);
			return;
		}

		if(!IsHumanoid(ch)) {
			send_to_char("Non hai la forma adatta!\n\r", ch);
			return;
		}
	}

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Kick who?\n\r", ch);
			return;
		}
	}

	if(victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}


	if(MOUNTED(victim)) {
		send_to_char("Non puoi calciare a cavallo!\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 4) {
		send_to_char("Non riesci ad avvicinarti abbastanza per calciare!\n\r", ch);
		return;
	}

	percent=((10-(GET_AC(victim)/10))) + number(1,101);
	/* 101% is a complete failure */

	if(GET_RACE(victim)==RACE_GHOST) {
		kick_messages(ch,victim,0);
		SetVictFighting(ch,victim);
		return;
	}
	else if(!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)) {
		kick_messages(ch,victim,0);
		SetVictFighting(ch,victim);
		SetCharFighting(ch,victim);
		return;
	}

	if(GET_POS(victim) <= POSITION_SLEEPING) {
		percent = 1;
	}

	if(percent > MIN(100, ch->skills[SKILL_KICK].learned)) {
		LearnFromMistake(ch, SKILL_KICK, 0, 90);
		if(GET_POS(victim) > POSITION_DEAD) {
			kick_messages(ch,victim,0);
			damage(ch, victim, 0, SKILL_KICK, location);
		}
	}
	else {
		if(!CheckMirror(victim)) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				dam = GET_LEVEL(ch, BestFightingClass(ch));
				if(!HasClass(ch, CLASS_MONK) || IS_NPC(ch))
					/* so guards use fighter dam */
				{
					dam/=2;
				}
				if(OnlyClass(ch, CLASS_MONK)) {
					dam = (int)(dam * 1.3);
				}
				kick_messages(ch,victim,dam);
				if(damage(ch, victim, dam, SKILL_KICK, location) != VictimDead) {
					WAIT_STATE(victim, PULSE_VIOLENCE*1);
				}
			}
		}
	}
	if(HasClass(ch,CLASS_MONK)) {
		WAIT_STATE(ch, PULSE_VIOLENCE*1);
	}
	else {
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
	}
}

/* Skill di parata, evita un numero di attacchi pari al proprio
   Se lo skill riesce gli attacchi vengono automaticamente
   indirizzati sullo scudo. Gaia( 7/2000 ) */

ACTION_FUNC(do_parry) {
	char name[100];
	struct char_data* victim;

	if(check_peaceful(ch,
					  "Non in questo luogo di pace.\n\r")) {
		return;
	}

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting)     {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Se stai combattendo con qualcuno riesce meglio.\n\r", ch);
			return;
		}
	}
	if(IS_SET(ch->specials.act,ACT_POLYSELF)) {
		send_to_char("In questa forma non sei in grado di parare i colpi\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Impugni la spada e ti dai delle gran mazzate sullo scudo\n\r", ch);
		return;
	}

	/* il PARRY resta attivo come sistema di combattimento
	   di default finche il comando non viene ridato o il
	   combattimento finisce */

	if(IS_SET(ch->specials.affected_by2, AFF2_PARRY)) {
		send_to_char("Smetti di proteggerti con lo scudo.\n\r",ch);
		REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
		return;
	}

	/* Fine dei controlli, si attiva il flag. Che lo skill riesca o meno
	   viene controllato nelle routine di combat (fight.c) volta per volta. */

	if(GET_POS(victim) > POSITION_DEAD)     {
		SET_BIT(ch->specials.affected_by2, AFF2_PARRY);
		act("$c1012$n cerca di proteggersi dai colpi usando lo scudo!", FALSE, ch, 0, victim, TO_ROOM);
		act("$c1012Cerchi di proteggerti con lo scudo.",FALSE,ch,0,0,TO_CHAR);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE);
}




ACTION_FUNC(do_wimp) {

	/* sets the character in wimpy mode.  */
	char name[80];
	short value;
	only_argument(arg, name);
	if(!*name) {
		snprintf(name,79,"Current wimpy level is: %d",ch->specials.WimpyLevel);
		send_to_char(name,ch);
	}

	else {
		value=MAX(atoi(name),0);
		ch->specials.WimpyLevel=value;
		if(!value) {
			REMOVE_BIT(ch->specials.act, PLR_WIMPY);
		}
		else {
			SET_BIT(ch->specials.act, PLR_WIMPY);
		}
		snprintf(name,79,"Your new wimpy level is: %d",ch->specials.WimpyLevel);
		send_to_char(name,ch);
	}

}

ACTION_FUNC(do_shoot) {
#if 0
	char tmp[80],dirstr[80],name[80];
	char buf[255];
	struct char_data* victim;
	struct room_data* this_room,*to_room,*next_room;
	struct room_direction_data* exitp;
	struct char_data* mob;
	struct obj_data* weapon;
	int i,dir,room_num=0,room_count, MAX_DISTANCE_SHOOT;

	if(check_peaceful(ch,"Non in questo luogo di pace.\n\r")) {
		return;
	}

	arg =  one_argument(arg, tmp);

	mudlog(LOG_CHECK, "begin do_shoot");

	if(*tmp) {
		victim = get_char_room_vis(ch, tmp);

		if(!victim) {
			i = ch->in_room;
			room_count=1;
			weapon = ch->equipment[HOLD];

			if(!weapon) {
				send_to_char("You do not hold a missile weapon?!?!\n\r",ch);
				return;
			}

# if 0
			MAX_DISTANCE_SHOOT = weapon->obj_flags.value[1];
# else
			MAX_DISTANCE_SHOOT = 1;
# endif



			switch(*tmp) {
			case 'N':
			case 'n':
				dir=0;
				break;
			case 'S':
			case 's':
				dir=2;
				break;
			case 'E':
			case 'e':
				dir=1;
				break;
			case 'W':
			case 'w':
				dir=3;
				break;
			case 'd':
			case 'D':
				dir=4;
				break;
			case 'u':
			case 'U':
				dir=5;
				break;

			default:
				send_to_char("What direction did you wish to fire?\n\r",ch);
				return;
				break;
			} /* end switch */

			arg= one_argument(arg,name);
			if(strn_cmp(name,"at",2) && isspace(name[2])) {
				arg=one_argument(arg,name);
			}


			if(!exit_ok(EXIT_NUM(i,dir),NULL)) {
				send_to_char("You can't shoot in that direction.\n\r",ch);
				return;
			}

			while(room_count<=MAX_DISTANCE_SHOOT && !victim &&
					exit_ok(EXIT_NUM(i,dir),NULL)) {

				this_room = real_roomp(i);
				to_room   = real_roomp(this_room->dir_option[dir]->to_room);
				room_num  = this_room->dir_option[dir]->to_room;

				mob = get_char_near_room_vis(ch,name,room_num);
				if(mob) {
					snprintf(buf,254,"You spot your quarry %s.\n",listexits[dir]);
					act(buf,FALSE,ch,0,0,TO_CHAR);
					victim=mob;
				}

				i = room_num;
				room_count++;

			} /* end while */
		} /* !victim */

		if(victim) {
			if(victim == ch) {
				send_to_char("You can't shoot things at yourself!", ch);
				return;
			}
			else {
				if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
					act("$N is just such a good friend, you simply can't shoot at $M.",
						FALSE, ch,0,victim,TO_CHAR);
					return;
				}
				if(ch->specials.fighting) {
					send_to_char("You're at too close range to fire a weapon!\n\r", ch);
					return;
				}

				if(check_peaceful(victim,"")) {
					send_to_char("That is a peaceful room\n\r",ch);
					return;
				}
				ActionAlignMod(ch,victim,cmd);
				shoot(ch, victim);
				WAIT_STATE(ch, PULSE_VIOLENCE); // shoot
			}
		}
		else {
			send_to_char("They aren't here.\n\r", ch);
		}
	}
	else {
		send_to_char("Shoot who?\n\r", ch);
	}
	mudlog(LOG_CHECK, "end do_shoot, act.off.c");

#endif
}


ACTION_FUNC(do_springleap) {
	struct char_data* victim;
	char name[256];
	byte percent;
	int danno=0;
	int location = 5 ;
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Non sei un monaco!\n\r", ch);
			return;
		}

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Spring-leap a chi?\n\r", ch);
			return;
		}
	}

	if(GET_POS(ch) > POSITION_SITTING || !ch->specials.fighting) {
		send_to_char("Non sei nella giusta posizione per farlo!\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Non mi va di scherzare oggi...\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 3) {
		send_to_char("Non riesci ad avvicinarti abbastanza.\n\r", ch);
		return;
	}

	percent=number(1,101);

	act("$n fa un'abile mossa allungando una gamba verso $N.", FALSE,
		ch, 0, victim, TO_ROOM);
	act("Sali a gamba tesa verso $N.", FALSE, ch, 0, victim, TO_CHAR);
	act("$n salta a gamba tesa verso te.", FALSE, ch, 0, victim, TO_VICT);


	if(percent > ch->skills[SKILL_SPRING_LEAP].learned) {
		if(GET_POS(victim) > POSITION_DEAD) {
			damage(ch, victim, 0, SKILL_KICK, location);
			LearnFromMistake(ch, SKILL_SPRING_LEAP, 0, 90);
			send_to_char("Rovini a terra rumorosamente.\n\r", ch);
			act("$n rovina a terra rumorosamente", FALSE, ch, 0, 0, TO_ROOM);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*2); // spring
		return;

	}
	else {
		ActionAlignMod(ch,victim,cmd);
		danno=GET_LEVEL(ch,BestFightingClass(ch));
		danno*=2;
		if(OnlyClass(ch,CLASS_MONK)) {
			danno = (int)(danno * 1.3);
		}
		danno+=(danno*(20-number(0,40))/100);
		kick_messages(ch, victim,  danno);
		if(GET_POS(victim) > POSITION_DEAD) {

			if(damage(ch, victim, danno, SKILL_KICK, location) != VictimDead) {

				WAIT_STATE(victim, PULSE_VIOLENCE);
			}
		}

	}
	WAIT_STATE(ch, PULSE_VIOLENCE*1);
	GET_POS(ch)=POSITION_STANDING;
	update_pos(ch);
	return;
}


ACTION_FUNC(do_quivering_palm) {
	struct char_data* victim;
	struct affected_type af;
	byte percent;
	char name[256];
	int location = 12 ;
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Non sei un monaco!\n\r", ch);
			return;
		}

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Su chi vuoi usare il favoloso palmo vibrante?\n\r", ch);
			return;
		}
	}

	if(ch->attackers > 3) {
		send_to_char("Non hai abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 3) {
		send_to_char("Non riesci ad avvicinarti abbastanza.\n\r", ch);
		return;
	}

	if(!IsHumanoid(victim)) {
		send_to_char("Rispondono alle vibrazioni solo gli umanoidi.\n\r", ch);
		return;
	}

	send_to_char("Cominci a generare una vibrazione con il palmo della tua "
				 "mano.\n\r", ch);
	act("$n comincia a generare una vibrazione con il palmo della mano rivolto verso di te.",
		TRUE,ch,NULL,victim,TO_VICT);

	if(affected_by_spell(ch, SKILL_QUIV_PALM)) {
		send_to_char("Puoi farlo solo una volta alla settimana.\n\r", ch);
		return;
	}

	percent=number(1,101);

	if(percent > ch->skills[SKILL_QUIV_PALM].learned) {
		send_to_char("La vibrazione si spegne inefficace.\n\r", ch);
		if(GET_POS(victim) > POSITION_DEAD) {
			LearnFromMistake(ch, SKILL_QUIV_PALM, 0, 95);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3); // quivering
		return;

	}
	else {
		if(GET_MAX_HIT(victim) > GET_MAX_HIT(ch) * 2 ||
				GetMaxLevel(victim) > GetMaxLevel(ch)) {
			damage(ch, victim, 0, SKILL_QUIV_PALM, location);
			return;
		}
		if(saves_spell(victim, SAVING_SPELL)) {
			send_to_char("La tua vittima resiste al palmo vibrante.\n\r", ch);
			act("Resisti al palmo vibrante di $n.",
				TRUE,ch,NULL,victim,TO_VICT);
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);
			return;
		}
		if(HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				damage(ch, victim, GET_MAX_HIT(victim)*20,SKILL_QUIV_PALM, location);
			}
		}
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 1);   // quivering

	af.type = SKILL_QUIV_PALM;
	af.duration = 168;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}


void kick_messages(struct char_data* ch, struct char_data* victim, int damage) {
	int i;

	switch(GET_RACE(victim)) {
	case RACE_HUMAN:
	case RACE_ELVEN:
	case RACE_DWARF:
	case RACE_DARK_ELF:
	case RACE_ORC:
	case RACE_LYCANTH:
	case RACE_TROLL:
	case RACE_DEMON:
	case RACE_DEVIL:
	case RACE_MFLAYER:
	case RACE_ASTRAL:
	case RACE_PATRYN:
	case RACE_SARTAN:
	case RACE_DRAAGDIM:
	case RACE_GOLEM:
	case RACE_TROGMAN:
	case RACE_LIZARDMAN:
	case RACE_HALF_ELVEN:
	case RACE_HALF_OGRE:
	case RACE_HALF_ORC:
	case RACE_HALF_GIANT:
		i=number(0,3);
		break;
	case RACE_PREDATOR:
	case RACE_HERBIV:
	case RACE_LABRAT:
		i=number(4,6);
		break;
	case RACE_REPTILE:
	case RACE_DRAGON:
	case RACE_DRAGON_RED:
	case RACE_DRAGON_BLACK:
	case RACE_DRAGON_GREEN:
	case RACE_DRAGON_WHITE:
	case RACE_DRAGON_BLUE:
	case RACE_DRAGON_SILVER:
	case RACE_DRAGON_GOLD:
	case RACE_DRAGON_BRONZE:
	case RACE_DRAGON_COPPER:
	case RACE_DRAGON_BRASS:
		i=number(4,7);
		break;
	case RACE_TREE:
		i=8;
		break;
	case RACE_PARASITE:
	case RACE_SLIME:
	case RACE_VEGGIE:
	case RACE_VEGMAN:
		i=9;
		break;
	case RACE_ROO:
	case RACE_GNOME:
	case RACE_HALFLING:
	case RACE_GOBLIN:
	case RACE_SMURF:
	case RACE_ENFAN:
		i=10;
		break;
	case RACE_GIANT:
	case RACE_GIANT_HILL:
	case RACE_GIANT_FROST:
	case RACE_GIANT_FIRE:
	case RACE_GIANT_CLOUD:
	case RACE_GIANT_STORM:
	case RACE_GIANT_STONE:

	case RACE_TYTAN:
	case RACE_GOD:
		i=11;
		break;
	case RACE_GHOST:
		i=12;
		break;
	case RACE_BIRD:
	case RACE_SKEXIE:
		i=13;
		break;
	case RACE_UNDEAD:
	case RACE_UNDEAD_VAMPIRE:
	case RACE_UNDEAD_LICH:
	case RACE_UNDEAD_WIGHT:
	case RACE_UNDEAD_GHAST:
	case RACE_UNDEAD_SPECTRE:
	case RACE_UNDEAD_ZOMBIE:
	case RACE_UNDEAD_SKELETON:
	case RACE_UNDEAD_GHOUL:
		i=14;
		break;
	case RACE_DINOSAUR:
		i=15;
		break;
	case RACE_INSECT:
	case RACE_ARACHNID:
		i=16;
		break;
	case RACE_FISH:
		i=17;
		break;
	default:
		i=18;
	};
	if(!damage) {
		act(att_kick_miss_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act(att_kick_miss_victim[i],FALSE, ch, ch->equipment[WIELD],victim,
			TO_VICT);
		act(att_kick_miss_room[i],FALSE, ch, ch->equipment[WIELD], victim,
			TO_NOTVICT);
	}
	else if(GET_HIT(victim) - DamageTrivia(ch,victim,damage,SKILL_KICK, 7) < -10) {
		act(att_kick_kill_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act(att_kick_kill_victim[i],FALSE, ch, ch->equipment[WIELD],victim,
			TO_VICT);
		act(att_kick_kill_room[i],FALSE, ch, ch->equipment[WIELD], victim,
			TO_NOTVICT);
	}
	else {
		act(att_kick_hit_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act(att_kick_hit_victim[i],FALSE, ch, ch->equipment[WIELD],victim,
			TO_VICT);
		act(att_kick_hit_room[i],FALSE, ch, ch->equipment[WIELD], victim,
			TO_NOTVICT);
	}
}

ACTION_FUNC(do_berserk) {
	int skillcheck=0;
	char name[100];
	struct char_data* victim;

	if((!ch->skills) && IS_PC(ch)) {
		send_to_char("You do not know any skills!\n\r",ch);
		return;
	}

	if(check_peaceful(ch,
					  "Non in questo luogo di pace.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_BARBARIAN) && cmd !=0)  {
			send_to_char("You're no berserker!\n\r", ch);
			return;
		}

#if 0
	if(!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF)) {
		return;
	}
#endif

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting)     {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("You need to begin fighting before you can go berserk.\n\r", ch);
			return;
		}
	}

	if(victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}


	if(MOUNTED(victim)) {
		send_to_char("You can't berserk while mounted!\n\r", ch);
		return;
	}

	if(IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
		send_to_char("You are already berserked!\n\r",ch);
		return;
	}

	if(GET_MANA(ch)<=15)    {  // SALVO non si entra in berserk con poco mana
		send_to_char("You do not have the energy to go berserk!\n\r",ch);
		return;
	}
	/* all the checks passed, now get on with it! */

	skillcheck = number(0,101);


	if(IS_PC(ch) && (skillcheck > ch->skills[SKILL_BERSERK].learned))  {
		act("$c1012$n growls at $mself, and looks very angry!", FALSE, ch, 0, victim, TO_ROOM);
		act("$c1012You can't seem to get mad enough right now.",FALSE,ch,0,0,TO_CHAR);
		LearnFromMistake(ch, SKILL_BERSERK, 0, 90);
	}
	else   {
		if(GET_POS(victim) > POSITION_DEAD)     {
			GET_MANA(ch) -=15;
			alter_mana(ch,0);  /* cost 15 mana to do it.. */
			SET_BIT(ch->specials.affected_by2,AFF2_BERSERK);
			act("$c1012$n growls at $mself, and whirls into a killing frenzy!", FALSE, ch, 0, victim, TO_ROOM);
			act("$c1012The madness overtakes you quickly!",FALSE,ch,0,0,TO_CHAR);
		}
		WAIT_STATE(victim, PULSE_VIOLENCE); // berserk
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*1); // Messo il tempo ad 1 per compensare GAIA2002
	//WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void throw_weapon(struct obj_data* o, int dir, struct char_data* targ,
				  struct char_data* ch) {
	int w = o->obj_flags.weight, sz, max_range, range, there;
	int rm = ch->in_room, opdir[] = { 2, 3, 0, 1, 5, 4 };
	int broken=FALSE;
	char buf[MAX_STRING_LENGTH];
	struct char_data* spud, *next_spud;
	const char* dir_name[] = {
		"da nord",
		"da est",
		"da sud",
		"da ovest",
		"dall'alto",
		"dal basso"
	};

	if(w > 100) {
		sz = 3;
	}
	else if(w > 25) {
		sz = 2;
	}
	else if(w > 5) {
		sz = 1;
	}
	else {
		sz = 0;
	}
	max_range = (((GET_STR(ch) + GET_ADD(ch) / 30) - 3) / 8) + 2;
	max_range = max_range / (sz + 1);
	if(o->obj_flags.type_flag == ITEM_MISSILE &&
			ch->equipment[ WIELD ] &&
			ch->equipment[ WIELD ]->obj_flags.type_flag == ITEM_FIREWEAPON) {
		/* Add bow's range bonus */
		max_range += ch->equipment[ WIELD ]->obj_flags.value[ 2 ];
	}
	if(max_range == 0) {
		act("$p colpisce il terreno davanti a $n.", TRUE, ch, o, 0, TO_ROOM);
		act("$p cade fiaccamente in terra davanti a te.", TRUE, ch, o, 0,
			TO_CHAR);
		obj_to_room(o, ch->in_room);
		return;
	}
	range = 0;
	while(range < max_range && broken == FALSE) {
		/* Check for target */
		there = 0;
		for(spud = real_roomp(rm)->people; spud; spud = next_spud) {
			next_spud = spud->next_in_room;
			if(spud == targ) {
				there = 1;
				if(range_hit(ch, targ, range, o, dir, max_range)) {
					if(targ && GET_POS(targ) > POSITION_DEAD) {
						if(o->obj_flags.type_flag == ITEM_MISSILE &&
								number(1, 100) < o->obj_flags.value[ 0 ]) {
							act("$p finisce in pezzi.", TRUE, targ, o, 0, TO_ROOM);
							broken = TRUE;
							obj_to_room(o, 3);   /* storage for broken arrows */
							/* for some reason this causes the obj to get placed in very
							   weird places */
#if 0
							obj_from_room(o);
							extract_obj(o);
#endif
						}
						else {
							obj_to_room(o, rm);
						}
					}
					mudlog(LOG_CHECK, "Throw weapon has hit!");
					return;
				}
				break;
			}
		}
		if(dir >= 0) {
			if(broken == FALSE) {
				if(clearpath(ch, rm, dir)) {
					if(!there && rm != ch->in_room) {
						snprintf(buf, MAX_STRING_LENGTH-1,"%s passa veloce %s!\n\r", o->short_description,
								 dir_name[ opdir[ dir ] ]);
						send_to_room(buf, rm);
					}
					else {
						there = 0;
					}
					rm = real_roomp(rm)->dir_option[ dir ]->to_room;
				}
				else {
					if(range > 1 && dir >= 0) {
						snprintf(buf,MAX_STRING_LENGTH-1,"%s vola %s, colpisce una parete ",
								 o->short_description, dir_name[ opdir[ dir ] ]);
					}
					else {
						snprintf(buf,MAX_STRING_LENGTH-1,"%s colpisce una parete ",o->short_description);
					}
					send_to_room(buf, rm);
					if(o->obj_flags.type_flag == ITEM_MISSILE &&
							number(1, 100) < o->obj_flags.value[ 0 ]) {
						snprintf(buf,MAX_STRING_LENGTH-1, "e finisce in pezzi.\n\r");
						obj_to_room(o, 3);   /* storage for broken arrows */
						broken=TRUE;
					}
					else {
						snprintf(buf, MAX_STRING_LENGTH-1,"e cade in terra.\n\r");
						obj_to_room(o, rm);
					}
					send_to_room(buf, rm);
					return;
				}
			}
		}
		else {
			break;
		}
		range++;
	}
	if(broken == FALSE) {
		snprintf(buf,MAX_STRING_LENGTH-1,"%s cade in terra.\n\r", o->short_description);
		send_to_room(buf, rm);
		obj_to_room(o, rm);
	}
}

void throw_object(struct obj_data* o, int dir, int from) {
#if 0
	struct char_data* catcher;
#endif
	const char* directions[][2]= {
		{ "nord",     "da sud"    },
		{ "est",      "da ovest"  },
		{ "sud",      "da nord"   },
		{ "ovest",    "da est"    },
		{ "l'alto",   "dal basso" },
		{ "il basso", "dall'alto" }
	};

	char buf1[100];
	int distance=0;

	while(distance < 20 && real_roomp(from)->dir_option[ dir ] &&
			real_roomp(from)->dir_option[ dir ]->exit_info < 2 &&
			real_roomp(from)->dir_option[ dir ]->to_room > 0) {
		if(distance) {
			snprintf(buf1,99, "%s vola %s.\n\r",
					 o->short_description,directions[ dir ][ 1 ]);
			send_to_room(buf1,from);

#if 0
			for(catcher=real_roomp(from)->people; catcher; catcher=catcher->next_in_room) {
				if(!strcmp(catcher->catch,o->name)) {
					switch(number(1,3)) {
					case 1:
						act("$n dives and catches $p",FALSE,catcher,o,0,TO_ROOM);
						break;
					default:
						act("$n catches $p",FALSE,catcher,o,0,TO_ROOM);
					}
					send_to_char("You caught it!\n\r",catcher);
					obj_from_room(o);
					obj_to_char(o,catcher);
					return;
				}
			}
#endif

			snprintf(buf1, 99,"%s vola verso %s.\n\r",
					 o->short_description,directions[ dir ][ 0 ]);
			send_to_room(buf1, from);
		}
		distance++;
		obj_from_room(o);
		from = real_roomp(from)->dir_option[dir]->to_room;
		obj_to_room(o,from);
	}
	if(distance == 20) {
		snprintf(buf1,99, "%s vola %s ed atterra qui.\n\r",
				 o->short_description, directions[ dir ][ 1 ]);
		send_to_room(buf1, from);
		return;
	}
	snprintf(buf1, 99,"%s vola %s e colpisce una parete.\n\r",
			 o->short_description,directions[dir][1]);
	send_to_room(buf1,from);
}



int clearpath(struct char_data* ch, long room, int direc) {
	int opdir[] = {2, 3, 0, 1, 5, 4};
	struct room_direction_data* exitdata;

	if(!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) { // SALVO controllo se caduto in DT
		return 0;
	}
	exitdata = real_roomp(room)->dir_option[ direc ];

	if(exitdata &&
			!real_roomp(exitdata->to_room)) {
		return 0;
	}
	if(!CAN_GO(ch, direc)) {
		return 0;
	}
	if(!real_roomp(room)->dir_option[ direc ]) {
		return 0;
	}
	if(real_roomp(room)->dir_option[ direc ]->to_room < 1) {
		return 0;
	}
#if 0
	if(real_roomp(room)->zone !=
			real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->zone) {
		return 0;
	}
#endif
	if(IS_SET(real_roomp(room)->dir_option[ direc ]->exit_info, EX_CLOSED)) {
		return 0;
	}
	if(!IS_SET(real_roomp(room)->dir_option[ direc ]->exit_info, EX_ISDOOR)
			&& real_roomp(room)->dir_option[ direc ]->exit_info > 0) {
		return 0;
	}
	/* One-way windows are allowed... no see through 1-way exits */
	if(!real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]) {
		return 0;
	}

	if(real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]->to_room < 1) {
		return 0;
	}

	if(real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]->to_room != room) {
		return 0;
	}


	return real_roomp(room)->dir_option[ direc ]->to_room;
}

ACTION_FUNC(do_weapon_load) {
	struct obj_data* fw, *ms;
	char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];


	fw = ch->equipment[WIELD];
	if(!fw || fw->obj_flags.type_flag != ITEM_FIREWEAPON) {
		send_to_char("Devi impugnare un arma per lanciare proiettili.\n\r", ch);
		return;
	}
    if((GET_STR(ch) + GET_ADD(ch) / 3) < fw->obj_flags.value[ 0 ] && !IS_NPC(ch) ) // Montero 10-Sep-2018 act.off.cpp: se è un mob non considero la forza per il load missile
    {
		mudlog(LOG_CHECK, "(%s) can't load (%s) because it requires (%d) strength "
			   "to wield",
			   GET_NAME(ch), fw->name, fw->obj_flags.value[ 0 ]);
		send_to_char("Non sei abbastanza forte per usare un arma cosi' "
					 "potente.\n\r", ch);
		return;
	}
	if(ch->equipment[ LOADED_WEAPON ]) {
		if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
			ms = unequip_char(ch, LOADED_WEAPON);
			act("Prima togli $p.", TRUE, ch, ms, 0, TO_CHAR);
			obj_to_char(ms, ch);
			act("$n scarica $p.", FALSE, ch, ms, 0, TO_ROOM);
		}
		else {
			send_to_char("Hai le mani troppo piene per scaricare l'arma.\n\r", ch);
			return;
		}
	}

	half_chop(arg, arg1, arg2,sizeof arg1 -1,sizeof arg1 -1);
	if(!*arg1) {
		send_to_char("Che proiettile vuoi caricare ?\n\r",ch);
		return;
	}
	ms = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if(!ms) {
		send_to_char("Non hai niente del genere.\n\r", ch);
		return;
	}
	if(ms->obj_flags.type_flag != ITEM_MISSILE) {
		act("Non puoi lanciare $p.",TRUE,ch,ms,0,TO_CHAR);
		return;
	}
	if(ms->obj_flags.value[ 3 ] != fw->obj_flags.value[ 3 ]) {
		act("Non puoi usare $p con l'arma che stai impugnando.", TRUE, ch, ms, 0,
			TO_CHAR);
		return;
	}

	obj_from_char(ms);
	equip_char(ch, ms, LOADED_WEAPON);
	act("Carichi $p.", TRUE, ch, ms, 0, TO_CHAR);
	act("$n carica $p.", FALSE, ch, ms, 0, TO_ROOM);
	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACTION_FUNC(do_fire) {
	struct obj_data* fw, *ms;
	char tmp[MAX_STRING_LENGTH];
	struct char_data* targ;
	int tdir, rng, dr;


	fw = ch->equipment[WIELD];
	if(!fw || fw->obj_flags.type_flag != ITEM_FIREWEAPON) {
		send_to_char("Devi impugnare un'arma per lanciare proiettili!\n\r", ch);
		return;
	}
	only_argument(arg, tmp);

	if(!*tmp) {
		send_to_char("Il giusto formato per fire (o shoot) e': "
					 "fire [<dir> at] <target>\n\r",ch);
		return;
	}
	targ = get_char_linear(ch, tmp, &rng, &dr);
	if(targ && targ == ch) {
		send_to_char("Non puoi colpire te stesso!\n\r", ch);
		return;
	}

	if(dr == -1 && !targ) {
		send_to_char("Quella non e' ne' una direzione, ne' la descrizione di una "
					 "creatura.\n\r", ch);
		return;
	}

	if(!targ) {
		send_to_char("Non vedi nessuno con quella descrizione.\n\r", ch);
		return;
	}
	else {
		tdir = dr;
	}
	if(check_peaceful(targ,
					  "Qualcuno ha cercato di disturbare la tua pace.")) {
		send_to_char("Mi dispiace ma c'e' troppa pace li' per lanciarci "
					 "qualcosa.", ch);
		return;
	}

	if(ch->equipment[ LOADED_WEAPON ]) {
		ms = unequip_char(ch, LOADED_WEAPON);
	}
	else {
		act("$p non e' caricata!", TRUE, ch, fw, 0, TO_CHAR);
		return;
	}

	act("Lanci $p verso $N.", TRUE, ch, ms, targ, TO_CHAR);
	act("$n lancia $p!", TRUE, ch, ms, 0, TO_ROOM);
	throw_weapon(ms, tdir, targ, ch);
}



ACTION_FUNC(do_throw) {
	struct obj_data* pObjThrow;
	char arg1[100],arg2[100];
	int rng, tdir;
	struct char_data* targ;

	half_chop(arg, arg1, arg2,sizeof arg1 -1,sizeof arg2 -1);
	if(!*arg1 || !*arg2) {
		send_to_char("Il giusto formato per throw e': "
					 "throw <oggetto> [<dir> at] <target>.\n\r", ch);
		return;
	}

	if(ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight >
			str_app[ STRENGTH_APPLY_INDEX(ch) ].wield_w) {
		send_to_char("Non puoi impugnare un'arma a due mani e lanciare "
					 "qualcosa.\n\r", ch);
	}
	else if(ch->equipment[ WIELD ] && ch->equipment[ HOLD ]) {
		send_to_char("Hai solo due mani.\n\r", ch);
	}
	else if(real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER) {
		send_to_char("Non puoi lanciare nulla sott'acqua.\n\r", ch);
	}
	else if((pObjThrow = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
		/* Check if second argument is a character or direction */
		targ = get_char_linear(ch, arg2, &rng, &tdir);
		if(targ && targ == ch) {
			act("Non puoi lanciare $p verso te stess$b!", FALSE, ch, pObjThrow,
				NULL, TO_CHAR);
			return;
		}

		if(targ) {
			if(IS_SET(pObjThrow->obj_flags.wear_flags, ITEM_THROW)) {
				if(pObjThrow->obj_flags.type_flag != ITEM_WEAPON &&
						pObjThrow->obj_flags.type_flag != ITEM_MISSILE) {  // Gaia 2001
					/* Friendly throw */
					act("Lanci $p.", FALSE, ch, pObjThrow, NULL, TO_CHAR);
					obj_from_char(pObjThrow);
					act("$n lancia $p!", TRUE, ch, pObjThrow, 0, TO_ROOM);
					obj_to_room(pObjThrow, ch->in_room);
					throw_object(pObjThrow, tdir, ch->in_room);
				}
				else {
					if(check_peaceful(targ,
									  "Qualcuno ha cercato di disturbare la tua "
									  "pace.")) {
						send_to_char("Mi dispiace ma c'e' troppa pace li' per lanciarci "
									 "qualcosa.", ch);
						return;
					}
					act("Lanci $p verso $N.", FALSE, ch, pObjThrow, targ, TO_CHAR);
					obj_from_char(pObjThrow);
					act("$n lancia $p!", TRUE, ch, pObjThrow, NULL, TO_ROOM);
					throw_weapon(pObjThrow, tdir, targ, ch);
				}
			}
			else {
				act("Non puoi lanciare $p.", FALSE, ch, pObjThrow, NULL, TO_CHAR);
			}
		}
		else {
			send_to_char("Non c'e' nessuno qui intorno con quella descrizione.\n\r",
						 ch);
		}
	}
	else {
		send_to_char("Non hai niente del genere!\n\r",ch);
	}
}

ACTION_FUNC(do_stopfight) {
	if(ch == NULL) {
		mudlog(LOG_SYSERR, "pChar == NULL in do_stopfight( act.off.c )");
		return;
	}

	if(ch->specials.fighting) {
		if(!IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
			stop_fighting(ch);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			act("Hai smesso di combattere.", TRUE, ch, 0, 0, TO_CHAR);
			act("$n smette di combattere.", TRUE, ch, 0, 0, TO_ROOM);
		}
		else
			send_to_char("Tutto quello a cui pensi adesso, e' alla tua "
						 "battaglia\n\r", ch);
	}
	else {
		send_to_char("Ma se non stai combattendo!\n\r", ch);
	}

}
} // namespace Alarmud

