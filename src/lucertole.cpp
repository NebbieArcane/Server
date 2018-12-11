/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: lucertole.c,v 1.2 2002/02/13 12:31:00 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
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
#include "lucertole.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.off.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "modify.hpp"
#include "opinion.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells2.hpp"

namespace Alarmud {


#define RESCUE_VIRGIN  1950
#define RESCUE_ROOM    1983
#define SNAKE_GOD      1953

MOBSPECIAL_FUNC(Lizardman) {
	if(type == EVENT_TICK) {
		if(mob->specials.fighting && AWAKE(mob)) {
			struct char_data* pVictim = mob->specials.fighting;

			if(GET_POS(mob) < POSITION_FIGHTING &&
					GET_POS(mob) > POSITION_STUNNED) {
				StandUp(mob);
				return TRUE;
			}
			else if(number(1, 3) == 1) {
				act("$n rotea violentemente la coda.", TRUE, mob, NULL, NULL,
					TO_ROOM);

				act("Ruoti violentemente la coda.", TRUE, mob, NULL, NULL,
					TO_CHAR);
				if(HitOrMiss(mob, pVictim, CalcThaco(mob, pVictim))) {
					int iDam = number(GetMaxLevel(mob), GetMaxLevel(mob) * 2);

					if(damage(mob, pVictim, iDam, TYPE_BLUDGEON, 7) == AllLiving) {
						if(iDam >= 30) {
							act("$c0011Il colpo di coda di $n fa perdere i sensi a $N.",
								TRUE, mob, NULL, pVictim, TO_NOTVICT);
							act("$c0011Il colpo di coda di $n ti fa perdere i sensi.", TRUE,
								mob, NULL, pVictim, TO_VICT);
							act("$c0011Il tuo colpo di coda fa perdere i sensi a $N.", TRUE,
								mob, NULL, pVictim, TO_CHAR);

							if(pVictim->specials.fighting) {
								stop_fighting(pVictim);
							}


							GET_POS(pVictim) = POSITION_STUNNED;
							WAIT_STATE(pVictim, 2 * PULSE_VIOLENCE);
						}
					}
				}
				else {
					damage(mob, pVictim, 0, TYPE_BLUDGEON, 7);
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}



MOBSPECIAL_FUNC(lizardman_shaman) {
	if(type == EVENT_TICK && AWAKE(mob)) {
		if(mob->specials.fighting) {
			if(!Lizardman(ch, cmd, arg, mob, type)) {
				switch(number(1, 3)) {
				case 1:
					return cleric(mob, cmd, arg, mob, type);
					break;
				case 2:
					return fighter(mob, cmd, arg, mob, type);
					break;
				case 3:
					if(GET_HIT(mob) >= GET_MAX_HIT(mob) / 2)
						do_emote(mob,
								 "con un sorriso sinistro dice: 'Morirai, infedele!'",
								 0);
					else
						do_emote(mob, "sibila arrabbiato: 'Non la farai franca! "
								 "Il serpente ti punira'!'", 0);
					break;
				} /* end switch */
			}
			else {
				return TRUE;
			}
		}/* fighting */
		else {
			if(number(1, 4) == 4) {
				switch(number(1, 3)) {
				case 1:
					do_emote(mob, "muove la mani pronunciando alcune strane parole.",
							 0);
					break;
				case 2:
					do_emote(mob, "guarda in alto, invocando un Dio invisibile.", 0);
					break;
				case 3:
					do_emote(ch,
							 "si colpisce il petto e grida $c0009'Morte ai tuoi "
							 "nemici!'", 0);
					break;
				} /* end switch */
				return TRUE;
			}   /* random emote */
		} /* else not fighting */
	}

	return FALSE;
}

#define Abitante 1905
MOBSPECIAL_FUNC(village_woman) {
	const char* aszInvocazioni[ 10 ] = {
		"Aiuto!",
		"Fermati! Che fai!",
		"Ma perche'? Che ti ho fatto!",
		"Ahi! Mi fai male!",
		"No! Ti prego!",
		"Non mi uccidere!",
		"Qualcuno mi aiuti!",
		"Che ti ho fatto! Aiuto!",
		"Argh! Mi attaccano",
		"Fermati! Sono solo una povera donna!"
	};

	if(type != EVENT_TICK) {
		return FALSE;
	}
	if(!AWAKE(ch)) {
		return FALSE;
	}

	if(ch->specials.fighting) {
		struct char_data* pAttacker;
		for(pAttacker = real_roomp(mob->in_room)->people; pAttacker;
				pAttacker = pAttacker->next_in_room) {
			if(pAttacker->specials.fighting == mob && !Fears(mob, pAttacker)) {
				AddFeared(mob, pAttacker);
			}
		}

		if(GET_POS(ch) < POSITION_FIGHTING &&
				GET_POS(ch) > POSITION_STUNNED) {
			StandUp(ch);
		}
		else if(number(1, 3) > 1) {
			do_shout(mob, aszInvocazioni[ number(0, 9) ], 0);
			CallForMobs(mob, ch->specials.fighting, 3, Abitante);
		}
		else {
			do_flee(mob, "", 0);
		}

		return TRUE;
	} /* not fighting... */
	return FALSE;
}

MOBSPECIAL_FUNC(snake_avt2) {
	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(type != EVENT_TICK) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		struct char_data* vict;
		byte lspell=0;


		if(GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
			StandUp(ch);
		}

		vict = ch->specials.fighting;
		if(!vict) {
			vict = FindVictim(ch);
		}
		if(!vict) {
			return(FALSE);
		}
		lspell = number(0,GetMaxLevel(ch));
		if(!IS_PC(ch)) {
			lspell += GetMaxLevel(ch)/5;
		}
		lspell = MIN(GetMaxLevel(ch), lspell);

		if(lspell < 1) {
			lspell=1;
		}

		if(IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
			act("$n pronuncia le parole 'A me la luce!'",
				TRUE, ch, 0, 0, TO_ROOM);
			cast_cure_blind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return TRUE;
		}

		if(GET_MOVE(ch) < 0) {
			act("$n spruzza sangue umano nell'aria. Tonificante.",
				TRUE, ch,0,0,TO_ROOM);
			cast_refresh(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return TRUE;
		}

		if(!(IS_AFFECTED(ch,AFF_FIRESHIELD)) && (lspell > 39)) {
			act("$n pronuncia la parola 'zimja'",1,ch,0,0,TO_ROOM);
			cast_fireshield(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return TRUE;
		}

		if(!(IS_AFFECTED(ch,AFF_SANCTUARY)) && (lspell > 25)) {
			act("$n pronuncia la parola 'zija'",1,ch,0,0,TO_ROOM);
			cast_sanctuary(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return TRUE;
		}

		if(IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 25 &&
				GetMaxLevel(ch) >= GetMaxLevel(vict)) {
			act("$n pronuncia le parole 'paf zija'",
				TRUE, ch, 0, 0, TO_ROOM);
			cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			return TRUE;
		}

		if(IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 25 &&
				GetMaxLevel(ch) >= GetMaxLevel(vict)) {
			act("$n pronuncia le parole 'paf zimja'",
				TRUE, ch, 0, 0, TO_ROOM);
			cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			return TRUE;
		}

		if(lspell > 40) {
			act("$n pronuncia la parola 'zerumba'",
				TRUE, ch, 0, 0, TO_ROOM);
			cast_feeblemind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			return TRUE;
		}
		/* well, spells failed or not needed, let's kick someone :) */

		switch(number(0,3)) {
		case 1:
			return(magic_user(mob,cmd,arg,mob,type));
			break;
		case 2:
			return(cleric(mob,cmd,arg,mob,type));
			break;
		case 3:
			return(snake(mob,cmd,arg,mob,type));
			break;
		default:
			return(fighter(mob,cmd,arg,mob,type));
			break;

		} /* end switch */

	}  /* end fighting */

	return FALSE;
}

#define PORTAL 31

void MakePortal(int iFromRoom, int iToRoom) {
	struct room_data* pFromRoom, *pToRoom;
	struct obj_data* pPortal;
	struct extra_descr_data* pExtraDesc;
	char szBuffer[ 512 ];

	pFromRoom = real_roomp(iFromRoom);
	pToRoom = real_roomp(iToRoom);
	if(pFromRoom && pToRoom) {
		pPortal = read_object(PORTAL, VIRTUAL);
		if(pPortal) {

			sprintf(szBuffer,
					"Attraverso la nebbia del portale riesci ad intravedere %s",
					pToRoom->name);

			CREATE(pExtraDesc, struct extra_descr_data, 1);
			pExtraDesc->nMagicNumber = EXDESC_VALID_MAGIC;
			pExtraDesc->next = pPortal->ex_description;
			pPortal->ex_description = pExtraDesc;

			if(pPortal->name) {
				pExtraDesc->keyword = strdup(pPortal->name);
			}
			else
				mudlog(LOG_SYSERR,
					   "Obj PORTAL has no name in MakePortal (lucertole.c).");

			pExtraDesc->description = strdup(szBuffer);

			pPortal->obj_flags.value[0] = 5;
			pPortal->obj_flags.value[1] = iToRoom;

			obj_to_room(pPortal, iFromRoom);
		}
		else {
			mudlog(LOG_ERROR, "Cannot make portal in MakePortal (lucertole.c)");
		}
	}
	else
		mudlog(LOG_ERROR,
			   "Cannot find room %d or %d in MakePortal (lucertole.c)",
			   iFromRoom, iToRoom);

}



MOBSPECIAL_FUNC(snake_avt) {
	if(type == EVENT_DEATH && mob->in_room == RESCUE_ROOM) {
		struct char_data* pVergine;
		MakePortal(mob->in_room, 3005);
		pVergine = read_mobile(real_mobile(RESCUE_VIRGIN), REAL);
		if(pVergine) {
			char_to_room(pVergine, RESCUE_ROOM);
			act("\nAppena il Paladino del Dio dei serpenti muore,\n\r"
				"appare un portale magico davanti a te.",
				FALSE, ch, NULL, NULL, TO_ROOM);
			act("Svaniti i fumi della battaglia ti accorgi che qualcuno\n\r"
				"e' legato all'altare. Avvicinandoti vedi $N.",
				FALSE, ch, NULL, pVergine, TO_ROOM);
			act("L$B liberi velocemente e ti affretti ad entrare nel portale\n\r"
				"ed a lasciare questo orribile posto.",
				FALSE, ch, NULL, pVergine, TO_ROOM);
			act("\n$c0015[$c0013$N$c0015] dice 'Si, entra nel portale prima che "
				"scompaia, ma guarda dentro\n\r"
				"$c0015l'altare e prendi il bottino che il tuo gruppo si e' "
				"guadagnato'.", FALSE, ch, NULL, pVergine, TO_ROOM);
			act("\n", FALSE, ch, NULL, NULL, TO_ROOM);
		}
		return TRUE;
	}

	return snake_avt2(ch, cmd, arg, mob, type);

}


MOBSPECIAL_FUNC(virgin_sac) {
	struct char_data* pPaladino;

	if(type == EVENT_DEATH && ch->in_room == RESCUE_ROOM) {
		if((pPaladino = read_mobile(real_mobile(SNAKE_GOD), REAL))) {
			char_to_room(pPaladino, RESCUE_ROOM);
			act("Come $n muore, senti un'agghiacciante risata e $N appare "
				"nuovamente nella stanza.", FALSE, mob, NULL, pPaladino, TO_ROOM);
			act("$c0015[$c0013$n$c0015] dice 'Avete completato il mio sacrificio!\n\r"
				"La vostra ricompensa sara' la $c0011MORTE$c0015! AH AH AH AH!'",
				FALSE, pPaladino, NULL, NULL, TO_ROOM);
		}
		return(TRUE);
	}

	if(type != EVENT_TICK) {
		return(FALSE);
	}

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		if(GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
			StandUp(ch);
			return TRUE;
		}
		else if(GET_POS(ch) >= POSITION_FIGHTING) {
			if(!number(0, 4))
				do_gossip(mob, "No, ti prego, ma che fai! Ho appena evitato di "
						  "essere sacrificata.", 0);
		}
	}
	else if(mob->in_room == RESCUE_ROOM &&
			MobCountInRoom(real_roomp(mob->in_room)->people) == 1) {
		StandUp(ch);
		REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
		return(special(mob, CMD_ENTER, "portal"));
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(snake_guardian) {
	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(type == EVENT_TICK) {
		if(mob->specials.fighting) {
			return snake(ch, cmd, arg, mob, type);
		}
	}
	else if(type == EVENT_COMMAND) {
		if(cmd >= CMD_NORTH && cmd <= CMD_DOWN) {
			if(cmd == CMD_SOUTH) {
				return(FALSE);    /* can always go south */
			}
			/* everything else gets ya attacked */
			if(IS_PC(ch) && !IS_SET(ch->specials.act, PLR_NOHASSLE)) {
				act("$N ti blocca la strada e ti attacca!", FALSE, ch, NULL, mob,
					TO_CHAR);
				act("$N ferma $n e l$b attacca!",TRUE, ch, NULL, mob, TO_ROOM);
				hit(mob, ch, 0);
				return TRUE;
			}
		}
	}
	return FALSE;
}

/***************************************************************************
 * Questa procedura gestisce il tiro della catena nella capanna principale
 * del villaggio. Se dallo ultimo reboot la catena non e' gia' stata tirata,
 * viene giu' della fuliggine ed un oggetto a caso fra dieci possibili
 * (dal numero 1970 al 1979).
 * ************************************************************************/

#define PRIMO_OGGETTO 1970

ROOMSPECIAL_FUNC(CapannaVillaggio) {
	static int bGiaFatto = FALSE;

	if(type == EVENT_COMMAND && cmd == CMD_PULL) {
		char szArg[ 256 ];

		one_argument(arg, szArg);

		if(!strcasecmp(szArg, "catena")) {
			if(!bGiaFatto) {
				struct obj_data* pOggetto;

				act("Come tiri la catena, una nuvola di fuliggine viene giu' dalla "
					"cappa.", FALSE, ch, NULL, NULL, TO_CHAR);
				act("$n tira la catena ed una nuvola di fuliggine viene giu' dalla "
					"cappa avvolgendol$b.", TRUE, ch, NULL, NULL, TO_ROOM);
				pOggetto = read_object(PRIMO_OGGETTO + number(0, 9), VIRTUAL);
				if(pOggetto) {
					send_to_room("Oltre alla fuliggine ti e' sembrato sia venuto giu' "
								 "qualcos'altro.\n\r", ch->in_room);

					obj_to_room(pOggetto, ch->in_room);
				}
				bGiaFatto = TRUE;
			}
			else {
				act("E' gia tirata al massimo.", FALSE, ch, NULL, NULL, TO_CHAR);
				act("$n cerca di tirare la catena, ma senza effetto.", TRUE, ch,
					NULL, NULL, TO_ROOM);
			}
			return TRUE;
		}
	}

	return FALSE;
}

/***************************************************************************
 * Quando nella locazione di questa procedura si guardano gli occhi del
 * serpente, si finisce incantati da questi E si ha un ritardo di 5 turni.
 * *************************************************************************/

ROOMSPECIAL_FUNC(ColloSerpente) {
	if(type == EVENT_COMMAND && cmd == CMD_LOOK) {
		char szArg[ 256 ];

		one_argument(arg, szArg);

		if(!strcasecmp(szArg, "occhi") || !strcasecmp(szArg, "occhio")) {

			char* pExDesc = find_ex_description(szArg,
												real_roomp(ch->in_room)->ex_description);
			if(pExDesc) {
				page_string(ch->desc, pExDesc, 0);

				act("La vista degli occhi del serpente ti incanta!", FALSE, ch,
					NULL, NULL, TO_CHAR);
				act("$n si e' incantat$b a guardare gli occhi del serpente.", TRUE,
					ch, NULL, NULL, TO_ROOM);
				WAIT_STATE(ch, PULSE_VIOLENCE * 10);

				return TRUE;
			}
			else {
				mudlog(LOG_ERROR,
					   "Non trovo la descrizione degli occhi in "
					   "ColloSerpente (lucertole.c)");
			}
		}
	}
	return FALSE;
}

/****************************************************************************
 * Se si guarda verso il basso dalle locazioni con questa procedura si viene
 * presi dalle vertigine e si cade verso il basso, anche se si sta volando.
 * *************************************************************************/

ROOMSPECIAL_FUNC(Rampicante) {
	if(type == EVENT_COMMAND && cmd == CMD_LOOK) {
		char szArg[ 256 ];

		one_argument(arg, szArg);

		if(*szArg && !strncasecmp(szArg, "down", strlen(szArg))) {
			if(!saves_spell(ch, SAVING_PARA) &&
					(IS_NPC(ch) ||
					 !IS_SET(ch->specials.act, PLR_NOHASSLE))) {
				if(!IS_AFFECTED(ch, AFF_FLYING)) {
					send_to_char("Oh no! L'altezza ti da' le vertigini, facendoti "
								 "mollare la presa...\n\r", ch);
					act("$n guarda verso il basso, viene preso dalle vertigini e molla "
						"la presa.", TRUE, ch, NULL, NULL, TO_ROOM);
				}
				else if(affected_by_spell(ch, SPELL_FLY)) {
					send_to_char("Oh no! L'altezza ti da' le vertigini, facendoti "
								 "perdere il controllo sul volo...\n\r", ch);
					act("$n guarda verso il basso, viene preso dalle vertigini e "
						"perde il controllo sul volo...", TRUE, ch, NULL, NULL,
						TO_ROOM);
					affect_from_char(ch, SPELL_FLY);
				}
				else if(affected_by_spell(ch, SKILL_LEVITATION)) {
					send_to_char("Oh no! L'altezza ti da' le vertigini, facendoti "
								 "perdere la concentrazione...\n\r", ch);
					act("$n guarda verso il basso, viene preso dalle vertigini e "
						"perde la concentrazione...", TRUE, ch, NULL, NULL,
						TO_ROOM);
					affect_from_char(ch, SKILL_LEVITATION);
				}
				else {
					send_to_char("Oh no! L'altezza ti da' le vertigini, ma, per "
								 "fortuna, qualcosa ti impedisce di cadere.\n\r",
								 ch);
				}

				if(!IS_AFFECTED(ch, AFF_FLYING)) {
					act("$n cade verso il basso.", TRUE, ch, NULL, NULL, TO_ROOM);
					char_from_room(ch);
					char_to_room(ch, 1947);
					act("$n arriva giu' dall'alto e si schianta sul terreno!", FALSE,
						ch, NULL, NULL, TO_ROOM);

					if(ch->skills &&
							number(1,101) < ch->skills[SKILL_SAFE_FALL].learned) {
						send_to_char("Cadi verso il basso, ma in qualche modo riesci a "
									 "rallentare la caduta.\n\r", ch);
					}
					else {
						send_to_char("Cadi verso il basso e ti schianti sul terreno!\n\r",
									 ch);

						DamageAllStuff(ch, BLOW_DAMAGE);
					}

					if(GET_HIT(ch) > 30) {
						GET_HIT(ch) -= number(30, GET_HIT(ch));
					}
					else {
						GET_HIT(ch) = 0;
					}

					/*** SALVO non rieco a capire perche', potrebbe ancora avere diversi hp
					        e quindi non so' se fargli ripartire la rigenerazione !!! ***/
					GET_POS(ch) = POSITION_STUNNED;
				}

				WAIT_STATE(ch, PULSE_VIOLENCE * 5);
			}
			else {

				send_to_char("Oh no! L'altezza ti da le vertigini, ma, per "
							 "fortuna, ti passano presto.", ch);
			}
			return TRUE;
		}
	}
	return FALSE;
}


} // namespace Alarmud

