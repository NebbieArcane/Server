/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
***  AlarMUD
* $Id: skills.c,v 1.10 2002/03/23 16:55:46 Thunder Exp $
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
#include "skills.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.off.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magicutils.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
#include "trap.hpp"

namespace Alarmud {


struct hunting_data {
	char*        name;
	struct char_data** victim;
};


/*
**  Disarm:
*/

ACTION_FUNC(do_disarm) {
	char name[ MAX_INPUT_LENGTH ];
	int percent;
	struct char_data* victim;
	struct obj_data* w, *trap;

	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r")) {
		return;
	}

	if(!IS_PC(ch) && cmd) {
		return;
	}

	/*
	 *   get victim
	 */
	only_argument(arg, name);
	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {

			if(!ch->skills) {
				send_to_char("You do not have skills!\n\r",ch);
				return;
			}
			if(!ch->skills[SKILL_REMOVE_TRAP].learned) {
				send_to_char("Disarm who?\n\r", ch);
				return;
			}
			else {

				if(MOUNTED(ch)) {
					send_to_char("Yeah... right... while mounted\n\r", ch);
					return;
				}

				if(!(trap = get_obj_in_list_vis(ch, name,
												real_roomp(ch->in_room)->contents))) {
					if(!(trap = get_obj_in_list_vis(ch, name, ch->carrying))) {
						send_to_char("Disarm what?\n\r", ch);
						return;
					}
				}

				if(trap) {
					remove_trap(ch, trap);
					return;
				}
			}
		}
	}


	if(victim == ch) {
		send_to_char("Molto spiritoso....\n\r", ch);
		return;
	}
	/* Ora si puo' iniziare un combattimento col disarm */
#ifndef ALAR
	if(victim != ch->specials.fighting) {
		send_to_char("but you aren't fighting them!\n\r", ch);
		return;
	}
#endif
	if(IS_PC(victim) && !IS_PKILLER(victim)) { // SALVO non si disarmano i non pk
		return;
	}
	if(ch->attackers > 3) {
		send_to_char("Non c'e' spazio per disarmare!\n\r", ch);
		return;
	}

	if(!HasClass(ch, CLASS_WARRIOR | CLASS_MONK | CLASS_BARBARIAN |
				 CLASS_RANGER | CLASS_PALADIN)) {
		send_to_char("You're no warrior!\n\r", ch);
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
		if(!IsHumanoid(ch)) {
			send_to_char("Non hai la forma adatta!\n\r", ch);
			return;
		}
	}

	/*
	 *   make roll - modified by dex && level
	 */
	percent=number(1,101); /* 101% is a complete failure */

	percent -= dex_app[(int)GET_DEX(ch) ].reaction * 10;
	percent += dex_app[(int)GET_DEX(victim) ].reaction * 10;
	if(!ch->equipment[WIELD] && HasClass(ch, CLASS_MONK)) {
		percent -= 50;
	}

	percent += GetMaxLevel(victim);
	if(HasClass(victim, CLASS_MONK)) {
		percent += GetMaxLevel(victim);
	}

	if(HasClass(ch, CLASS_MONK)) {
		percent -= GetMaxLevel(ch);
	}
	else {
		percent -= GetMaxLevel(ch)>>1;
	}

	if(percent > ch->skills[SKILL_DISARM].learned) {
		/*
		 *   failure.
		 */
		act("$c0009You try to disarm $N, but fail miserably.",
			TRUE, ch, 0, victim, TO_CHAR);
		act("$c0009$n tries to disarm you, but fail miserably.",
			TRUE, ch, 0, victim, TO_VICT);
		act("$n does a nifty fighting move, but then falls on $s butt.",
			TRUE, ch, 0, 0, TO_ROOM);
		GET_POS(ch) = POSITION_SITTING;
		if((CanFightEachOther(ch,victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
				(!victim->specials.fighting)) {
			set_fighting(victim, ch);
		}
		LearnFromMistake(ch, SKILL_DISARM, 0, 95);
		if(CheckEquilibrium(ch))    //Acidus 2003 - skill better equilibrium
			WAIT_STATE(ch, PULSE_VIOLENCE*2) // disarm
			else {
				WAIT_STATE(ch, PULSE_VIOLENCE*3);    // disarm
			}
	}
	else {
		/*
		 *  success
		 */
		if(victim->equipment[WIELD]) {
			w = unequip_char(victim, WIELD);
			act("$n makes an impressive fighting move.",
				TRUE, ch, 0, 0, TO_ROOM);
			act("$c0010You send $p flying from $N's grasp.", TRUE, ch, w, victim,
				TO_CHAR);
			act("$p flies from your grasp.", TRUE, ch, w, victim, TO_VICT);
			/*
			 * send the object to a nearby room, instead
			 */
			obj_to_room(w, victim->in_room);
			ActionAlignMod(ch,victim,cmd);
		}
		else {
			act("You try to disarm $N, but $E doesn't have a weapon.",
				TRUE, ch, 0, victim, TO_CHAR);
			act("$n makes an impressive fighting move, but does little more.",
				TRUE, ch, 0, 0, TO_ROOM);
		}
		if((CanFightEachOther(ch,victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
				(!victim->specials.fighting)) {
			set_fighting(victim, ch);
		}
		WAIT_STATE(victim, PULSE_VIOLENCE*2); // disarm
		WAIT_STATE(ch, PULSE_VIOLENCE*2); // disarm
	}
}
/* finger: nuova skill dei monaci: acceca l'avversario */

ACTION_FUNC(do_finger) {
	char name[ MAX_INPUT_LENGTH ];
	int percent;
	struct char_data* victim;

	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r")) {
		return;
	}

	if(!IS_PC(ch) && cmd) {
		return;
	}

	/*
	 *   get victim
	 */
	only_argument(arg, name);
	if(!(victim = get_char_room_vis(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {

			send_to_char("Finger who?\n\r", ch);
			return;
		}
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}
	/* Non si puo' iniziare un combattimento con finger ne fingerare
	 * qualcuno da diverso da quello contro cui si sta combattendo */
	if(victim != ch->specials.fighting) {
		send_to_char("but you aren't fighting them!\n\r", ch);
		return;
	}
	if(ch->attackers > 3) {
		send_to_char("There is no room to finger!\n\r", ch);
		return;
	}

	if(WIELDING(ch)) {
		send_to_char("Non stai combattendo a mani nude",ch);
		return;
	}

	/*
	 *   make roll - modified by dex && level
	 */
	percent=number(1,101); /* 101% is a complete failure */

	percent -= dex_app[(int)GET_DEX(ch) ].reaction * 10;
	percent += dex_app[(int)GET_DEX(victim) ].reaction * 10;

	percent += GetMaxLevel(victim);
	if(HasClass(victim, CLASS_MONK)) {
		percent += GetMaxLevel(victim);
	}

	percent -= GetMaxLevel(ch);
	if(HasClass(ch,CLASS_MONK)) {
		percent -= GetMaxLevel(ch);
	}
	if(percent > ch->skills[SKILL_FINGER].learned) {
		/*
		 *   failure.
		 */
		act("$c0009Cerchi di accecare $N, ma $D ammacchi appena il naso.",
			TRUE, ch, 0, victim, TO_CHAR);
		act("$n cerca senza troppo successo di accecare $N.",
			TRUE, ch, 0, victim, TO_NOTVICT);
		act("$n cerca di accecarti.",
			TRUE, ch, 0, victim, TO_VICT);

		GET_POS(ch) = POSITION_SITTING;
		if((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
				(!victim->specials.fighting)) {
			set_fighting(victim, ch);
		}
		LearnFromMistake(ch, SKILL_FINGER, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*1); // finger
	}
	else {
		/*
		 *  success
		 */
		act("$c0010Le dita di $n saettano verso gli occhi di $N e l$B accecano.",
			TRUE, ch, 0, victim, TO_NOTVICT);
		act("Veloci come serpenti le tue dita raggiungono gli occhi"
			" di $N e l$B accecano.", TRUE, ch, 0, victim,
			TO_CHAR);
		act("$c0001AAAAARGH! $c0007$n ti ha accecato$B!!.",
			TRUE, ch, 0, victim, TO_VICT);
		ActionAlignMod(ch,victim,cmd);
		if((GET_POS(victim) > POSITION_SLEEPING) &&
				(!victim->specials.fighting)) {
			set_fighting(victim, ch);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3); // finger
		SET_BIT(victim->specials.affected_by,AFF_BLIND);
		spell_blindness(2,ch,victim,0);
	}
}


/*
**   Track:
*/

int named_mobile_in_room(int room, struct hunting_data* c_data) {
	struct char_data*        scan;

	for(scan = real_roomp(room)->people; scan; scan = scan->next_in_room)
		if(isname(c_data->name, scan->player.name)) {
			*(c_data->victim) = scan;
			return 1;
		}
	return 0;
}

ACTION_FUNC(do_track) {
	char name[256], buf[256], found=FALSE;
	int dist, code;
	struct hunting_data        huntd;
	struct char_data* scan;

#if NOTRACK
	send_to_char("Sorry, tracking is disabled. Try again after reboot.\n\r",ch);
	return;
#endif

	only_argument(arg, name);

	found = FALSE;
	for(scan = character_list; scan; scan = scan->next)
		if(isname(name, scan->player.name)) {
			found = TRUE;
		}


	if(!found) {
		send_to_char("You are unable to find traces of one.\n\r", ch);
		return;
	}

	if(!ch->skills) {
		dist = 10;
	}
	else {
		dist = ch->skills[SKILL_HUNT].learned;
	}


	if(IS_SET(ch->player.iClass, CLASS_RANGER)) {
		dist *= 3;
	}
	if(IS_SET(ch->player.iClass, CLASS_THIEF)) {
		dist *= 2;
	}

	switch(GET_RACE(ch)) {
	case RACE_ELVEN:
		dist *= 2;               /* even better */
		break;
	case RACE_DEVIL:
	case RACE_DEMON:
		dist = MAX_ROOMS;   /* as good as can be */
		break;
	default:
		break;
	}

	if(GetMaxLevel(ch) >= IMMORTAL) {
		dist = MAX_ROOMS;
	}


	if(affected_by_spell(ch, SPELL_MINOR_TRACK)) {
		dist = MAX(dist,GetMaxLevel(ch) * 50);
	}
	else if(affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
		dist = MAX(dist,GetMaxLevel(ch) * 100);
	}

	if(dist == 0) {
		return;
	}

	ch->hunt_dist = dist;

	ch->specials.hunting = 0;
	huntd.name = name;
	huntd.victim = &ch->specials.hunting;

	if((GetMaxLevel(ch) < MIN_GLOB_TRACK_LEV) ||
			(!ch->skills) ||
			!ch->skills[SKILL_HUNT].learned) {
		code = find_path(ch->in_room, reinterpret_cast<find_func>(named_mobile_in_room), &huntd, -dist, 1);
	}
	else {
		code = find_path(ch->in_room, reinterpret_cast<find_func>(named_mobile_in_room), &huntd, -dist, 0);
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*1);

	if(code == -1) {
		send_to_char("You are unable to find traces of one.\n\r", ch);
		return;
	}
	else {
		if(IS_LIGHT(ch->in_room)) {
			SET_BIT(ch->specials.act, (!IS_POLY(ch)) ? PLR_HUNTING : ACT_HUNTING); // SALVO fix track del poly
			if(code <6) {
				sprintf(buf, "Vedi una traccia della tua preda che va %s\n\r",
						dirsTo[code]);
			}
			send_to_char(buf,ch);
		}
		else {
			ch->specials.hunting = 0;
			send_to_char("E` troppo scuro qui per seguire delle traccie...\n\r",ch);
			return;
		}
	}
}

int track(struct char_data* ch, struct char_data* vict) {

	char buf[256];
	int code;

	if((!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) || (!vict || vict->nMagicNumber != CHAR_VALID_MAGIC)) { // SALVO migliorato il controllo
		return(-1);
	}

	if(GetMaxLevel(ch) < MIN_GLOB_TRACK_LEV ||
			affected_by_spell(ch, SPELL_MINOR_TRACK) ||
			affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
		code = choose_exit_in_zone(ch->in_room, vict->in_room, ch->hunt_dist);
	}
	else {
		code = choose_exit_global(ch->in_room, vict->in_room, ch->hunt_dist);
	}
	if((!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) || (!vict || vict->nMagicNumber != CHAR_VALID_MAGIC)) { // SALVO migliorato il controllo
		return(-1);
	}


	if(ch->in_room == vict->in_room) {
		send_to_char("$c0012Hai trovato la tua preda!\n\r",ch);
		return(FALSE);  /* false to stop the hunt */
	}
	if(code == -1) {
		send_to_char("$c0012Hai perso la traccia.\n\r",ch);
		return(FALSE);
	}
	else if(code <6) {  // SALVO migliorato controllo
		sprintf(buf, "$c0012Vedi una traccia della tua preda %s\n\r",
				dirsTo[code]);
		send_to_char(buf, ch);
		return(TRUE);
	}
	return false;
}

int dir_track(struct char_data* ch, struct char_data* vict) {

	char buf[256];
	int code;

	if((!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) || (!vict || vict->nMagicNumber != CHAR_VALID_MAGIC)) { // SALVO migliorato il controllo
		return(-1);
	}


	if((GetMaxLevel(ch) >= MIN_GLOB_TRACK_LEV) ||
			(affected_by_spell(ch, SPELL_MINOR_TRACK)) ||
			(affected_by_spell(ch, SPELL_MAJOR_TRACK))) {
		code = choose_exit_global(ch->in_room, vict->in_room, ch->hunt_dist);
	}
	else {
		code = choose_exit_in_zone(ch->in_room, vict->in_room, ch->hunt_dist);
	}
	if((!ch || ch->nMagicNumber != CHAR_VALID_MAGIC) || (!vict || vict->nMagicNumber != CHAR_VALID_MAGIC)) { // SALVO migliorato il controllo
		return(-1);
	}

	if(code == -1) {
		if(ch->in_room == vict->in_room) {
			send_to_char("$c0012Hai trovato la tua preda!!!!\n\r",ch);
		}
		else {
			send_to_char("$c0012hai perso le traccie.\n\r",ch);
		}
		return(-1);  /* false to continue the hunt */
	}
	else {
		sprintf(buf, "$c0012Vedi una debole traccia %s\n\r", dirsTo[code]);
		send_to_char(buf, ch);
		return(code);
	}

}




/** Perform breadth first search on rooms from start (in_room) **/
/** until end (tgt_room) is reached. Then return the correct   **/
/** direction to take from start to reach end.                 **/

/* thoth@manatee.cis.ufl.edu
 * if dvar<0 then search THROUGH closed but not locked doors,
 * for mobiles that know how to open doors.
 */

#define IS_DIR    (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK  (!IS_SET(IS_DIR->exit_info,EX_CLOSED)\
				&& (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER  (!IS_SET(IS_DIR->exit_info,EX_LOCKED)\
						&& (IS_DIR->to_room != NOWHERE))

void donothing(void* pDummy) {
	return;
}

int aRoom[ WORLD_SIZE ]; // E` definita esterna in modo da non sovraccaricare
// lo stack. Inoltre viene usata anche in utility.c

int find_path(int in_room, find_func predicate, const void* c_data,
			  int depth, int in_zone) {
	struct room_q* tmp_q, *q_head, *q_tail;
	int i, tmp_room, count=0, thru_doors;
	struct room_data* herep, *therep;
	struct room_data* startp;
	struct room_direction_data* exitp;

	/* If start = destination we are done */
	if((predicate)(in_room, c_data)) {
		return -1;
	}


	/* so you cannot track mobs in no_tracking rooms */
	if(in_room) {
		struct room_data* rp = real_roomp(in_room);
		if(rp && IS_SET(rp->room_flags, NO_TRACK)) {
			return(-1);
		}
	}


	if(depth < 0) {
		thru_doors = TRUE;
		depth = - depth;
	}
	else {
		thru_doors = FALSE;
	}

	startp = real_roomp(in_room);

	memset(aRoom, 0, sizeof(aRoom));
	aRoom[ in_room ] = -1;

	/* initialize queue */
	q_head = (struct room_q*) malloc(sizeof(struct room_q));
	q_tail = q_head;
	q_tail->room_nr = in_room;
	q_tail->next_q = 0;

	while(q_head) {
		herep = real_roomp(q_head->room_nr);
		/* for each room test all directions */
		if(herep->zone == startp->zone || !in_zone) {
			/* only look in this zone..
			 *  saves cpu time.  makes world
			 *  safer for players
			 */
			for(i = 0; i <= 5; i++) {
				exitp = herep->dir_option[i];
				if(exit_ok(exitp, &therep) && (thru_doors ? GO_OK_SMARTER : GO_OK)) {
					/* next room */
					tmp_room = herep->dir_option[i]->to_room;
					if(!((predicate)(tmp_room, c_data))) {
						/* shall we add room to queue ? */
						/* count determines total breadth and depth */
						if(!aRoom[ tmp_room ] && (count < depth)
								&& !IS_SET(RM_FLAGS(tmp_room),DEATH)) {
							count++;
							/* mark room as visted and put on queue */

							tmp_q = (struct room_q*) malloc(sizeof(struct room_q));
							tmp_q->room_nr = tmp_room;
							tmp_q->next_q = 0;
							q_tail->next_q = tmp_q;
							q_tail = tmp_q;

							/* ancestor for first layer is the direction */
							aRoom[ tmp_room ] = aRoom[ q_head->room_nr ] == -1 ?
												i + 1 : aRoom[ q_head->room_nr ];
						}
					}
					else {
						/* have reached our goal so free queue */
						tmp_room = q_head->room_nr;
						for(; q_head; q_head = tmp_q) {
							tmp_q = q_head->next_q;
							free(q_head);
						}
						/* return direction if first layer */
						if(aRoom[ tmp_room ] == -1
								|| aRoom[ tmp_room ] >= WORLD_SIZE) { // SALVO aggiungo controllo WORLD_SIZE
							return(i);
						}
						else {
							/* else return the ancestor */
							return(aRoom[ tmp_room ] - 1);
						}
					}
				}
			}
		}

		/* free queue head and point to next entry */
		tmp_q = q_head->next_q;
		free(q_head);
		q_head = tmp_q;
	}
	return(-1);
}

int choose_exit_global(int in_room, int tgt_room, int depth) {
	return find_path(in_room, is_target_room_p, reinterpret_cast<const void*>(tgt_room), depth, 0);
}

int choose_exit_in_zone(int in_room, int tgt_room, int depth) {
	return find_path(in_room, is_target_room_p, reinterpret_cast<const void*>(tgt_room), depth, 1);
}

void go_direction(struct char_data* ch, int dir) {
	if(ch->specials.fighting) {
		return;
	}

	if(!IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED)) {
		do_move(ch, "", dir+1);
		SetStatus("Returned from move in go_direction", NULL);
	}
	else if(IsHumanoid(ch) && !IS_SET(EXIT(ch,dir)->exit_info, EX_LOCKED) &&
			!IS_SET(EXIT(ch,dir)->exit_info,EX_SECRET)) {
		open_door(ch, dir);
	}
}


void slam_into_wall(struct char_data* ch, struct room_direction_data* exitp) {
	char doorname[128];
	char buf[256];

	if(exitp->keyword && *exitp->keyword) {
		if((strcmp(fname(exitp->keyword), "secret")==0) ||
				(IS_SET(exitp->exit_info, EX_SECRET))) {
			strcpy(doorname, "wall");
		}
		else {
			strcpy(doorname, fname(exitp->keyword));
		}
	}
	else {
		strcpy(doorname, "barrier");
	}
	sprintf(buf, "You slam against the %s with no effect\n\r", doorname);
	send_to_char(buf, ch);
	send_to_char("OUCH!  That REALLY Hurt!\n\r", ch);
	sprintf(buf, "$n crashes against the %s with no effect\n\r", doorname);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	GET_HIT(ch) -= number(1, 10)*2;
	alter_hit(ch,0);
	if(GET_HIT(ch) < 0) {
		GET_HIT(ch) = 0;
		alter_hit(ch,0);
	}
	GET_POS(ch) = POSITION_STUNNED;
	return;
}

void ChangeAlignmentDoorBash(struct char_data* pChar, int nAmount) {
	if(HasClass(pChar, CLASS_PALADIN) && GetMaxLevel(pChar) < IMMORTALE) {
		GET_ALIGNMENT(pChar) -= nAmount;
		send_to_char("Attento, stai tradendo la tua fede.\n\r", pChar);
		if(GET_ALIGNMENT(pChar) < 950) {
			send_to_char("La tua mancanza di fede disturba gli Dei.\n\r", pChar);
			if(GET_ALIGNMENT(pChar) < 550) {
				send_to_char("Se non ti allinei immediatamente al volere"
							 " degli Dei, sarai punito.\n\r", pChar);
				if(GET_ALIGNMENT(pChar) < 350) {
					send_to_char("La tua mancanza di fede ha richiesto una "
								 "punizione!\n\r", pChar);
					drop_level(pChar, CLASS_PALADIN, FALSE);
				}
			}
		}
	}
}

/*
 * skill to allow fighters to break down doors
 */
ACTION_FUNC(do_doorbash) {
	int dir;
	int ok;
	struct room_direction_data* exitp;
	int was_in, roll;
	char buf[256], type[128], direction[128];

	if(!ch->skills ||
			!ch->skills[SKILL_DOORBASH].learned) { // SALVO il doorbash lo fa' chi ne ha la conoscenza
		send_to_char("Non possiedi la conoscenza necessaria.\n\r", ch);
		return;
	}

	if(GET_MOVE(ch) < 10) {
		send_to_char("You're too tired to do that\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	/*
	 * make sure that the argument is a direction, or a keyword.
	 */

	for(; *arg == ' '; arg++);

	argument_interpreter(arg, type, direction);

	if((dir = find_door(ch, type, direction)) >= 0) {
		ok = TRUE;
	}
	else {
		act("$n looks around, bewildered.", FALSE, ch, 0, 0, TO_ROOM);
		return;
	}

	if(!ok) {
		send_to_char("Hmm, you shouldn't have gotten this far\n\r", ch);
		return;
	}

	exitp = EXIT(ch, dir);
	if(!exitp) {
		send_to_char("you shouldn't have gotten here.\n\r", ch);
		return;
	}

	if(dir == UP) {
#if 1
		/* disabledfor now */
		send_to_char("Are you crazy, you can't door bash UPWARDS!\n\r",ch);
		return;
#else
		if(real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
				!IS_AFFECTED(ch, AFF_FLYING)) {
			send_to_char("You have no way of getting there!\n\r", ch);
			return;
		}
#endif
	}

	sprintf(buf, "$n carica %s", dirsTo[dir]);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "Carichi %s\n\r", dirsTo[dir]);
	send_to_char(buf, ch);

	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		was_in = ch->in_room;
		char_from_room(ch);
		char_to_room(ch, exitp->to_room);
		do_look(ch, "", 15);

		DisplayMove(ch, dir, was_in, 1);
		if(!check_falling(ch)) {
			if(IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
					!IS_IMMORTAL(ch)) {
				NailThisSucker(ch);
				return;
			}
			else {
				WAIT_STATE(ch, PULSE_VIOLENCE*3);
				GET_MOVE(ch) -= 10;
				alter_move(ch,0);
				/*** SALVO controllare se qui manca il return ***/
			}
		}
		else {
			return;
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		GET_MOVE(ch) -= 10;
		alter_move(ch,0);
		return;
	}

	GET_MOVE(ch) -= 10;
	alter_move(ch,0);

	/* Provvisorio. Convertire le stanze e togliere EX_PICKPROOF */
	if(IS_SET(exitp->exit_info, EX_LOCKED) &&
			(IS_SET(exitp->exit_info, EX_PICKPROOF) ||
			 IS_SET(exitp->exit_info, EX_NOTBASH))) {
		slam_into_wall(ch, exitp);
		return;
	}

	/*
	 * now we've checked for failures, time to check for success;
	 */
	if(ch->skills) {
		if(ch->skills[SKILL_DOORBASH].learned) {
			roll = number(1, 100);
			if(roll > ch->skills[SKILL_DOORBASH].learned) {
				slam_into_wall(ch, exitp);
				ChangeAlignmentDoorBash(ch, 5);
				LearnFromMistake(ch, SKILL_DOORBASH, 0, 95);
			}
			else {
				/*
				 * unlock and open the door
				 */
				sprintf(buf, "$n slams into the %s, and it bursts open!",
						fname(exitp->keyword));
				act(buf, FALSE, ch, 0, 0, TO_ROOM);
				sprintf(buf, "You slam into the %s, and it bursts open!\n\r",
						fname(exitp->keyword));
				send_to_char(buf, ch);
				raw_unlock_door(ch, exitp, dir);
				raw_open_door(ch, dir);
				GET_HIT(ch) -= number(1,5);
				alter_hit(ch,0);
				ChangeAlignmentDoorBash(ch, 10);
				/*
				 * Now a dex check to keep from flying into the next room
				 */
				roll = number(1, 20);
				if(roll > GET_DEX(ch)) {
					was_in = ch->in_room;

					char_from_room(ch);
					char_to_room(ch, exitp->to_room);
					do_look(ch, "", 15);
					DisplayMove(ch, dir, was_in, 1);
					if(!check_falling(ch)) {
						if(IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
								!IS_IMMORTAL(ch)) {
							NailThisSucker(ch);
							return;
						}
					}
					else {
						return;
					}
					WAIT_STATE(ch, PULSE_VIOLENCE*3);
					GET_MOVE(ch) -= 10;
					alter_move(ch,0);
					return;
				}
				else {
					WAIT_STATE(ch, PULSE_VIOLENCE*1);
					GET_MOVE(ch) -= 5;
					alter_move(ch,0);
					return;
				}
			}
		}
		else {
			send_to_char("You just don't know the nuances of door-bashing.\n\r", ch); // Gaia 2001
			slam_into_wall(ch, exitp); // Gaia 2001
			return;
		}
	}
	else {
		send_to_char("You're just a goofy mob.\n\r", ch);
		return;
	}
}

/*
 * skill to allow anyone to move through rivers and underwater
*/

ACTION_FUNC(do_swim) {

	struct affected_type af;
	byte percent;


	send_to_char("Ok, you'll try to swim for a while.\n\r", ch);

	if(IS_AFFECTED(ch, AFF_WATERBREATH)) {
		/* kinda pointless if they don't need to...*/
		return;
	}

	if(affected_by_spell(ch, SKILL_SWIM)) {
		send_to_char("You're too exhausted to swim right now\n", ch);
		return;
	}

	percent=number(1,101); /* 101% is a complete failure */

	if(!ch->skills) {
		return;
	}

	if(percent > ch->skills[SKILL_SWIM].learned) {
		send_to_char("You're too afraid to enter the water\n\r",ch);
		if(ch->skills[SKILL_SWIM].learned < 95 &&
				ch->skills[SKILL_SWIM].learned > 0) {
			if(number(1,101) > ch->skills[SKILL_SWIM].learned) {
				send_to_char("You feel a bit braver, though\n\r", ch);
				ch->skills[SKILL_SWIM].learned++;
			}
		}
		return;
	}

	af.type = SKILL_SWIM;
	af.duration = (ch->skills[SKILL_SWIM].learned/10)+1;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_WATERBREATH;
	affect_to_char(ch, &af);

	af.type = SKILL_SWIM;
	af.duration = 13;
	af.modifier = -10;
	af.location = APPLY_MOVE;
	af.bitvector = 0;
	affect_to_char(ch, &af);

}
/*
 * skill to allow monks to lower AC
*/

ACTION_FUNC(do_mantra) {

	struct affected_type af;
	int sulcorpo=0,i=0;
	struct obj_data* ob;
	/* Attenzione, questo test e` ripetuto anche in spell_parser */
	for(i=0; i<=MAX_WEAR_POS; i++) {
		ob=(struct obj_data*)WEARING(ch,i);

		if(ob && (isname("tunica",ob->name) || isname("robe",ob->name))) {
			sulcorpo=1;
			break;
		}

	}

	if(!sulcorpo) {
		send_to_char("Devi indossare vesti piu` adatte alla meditazione",ch);
		return;
	}
	if(!affected_by_spell(ch,SKILL_MANTRA)) {
		if(!HasClass(ch,CLASS_MONK)) {

			act("La tua mente e il tuo corpo vibrano all'unisono.... piu` o meno.",
				FALSE,ch,0,0,TO_CHAR);
			act("$n chiude gli occhi e biascica un mantra accovacciat$b in terra",
				TRUE,ch,0,0,TO_ROOM);
		}
		else {

			act("La tua mente e il tuo corpo vibrano all'unisono",
				FALSE,ch,0,0,TO_CHAR);
			act("$n chiude gli occhi e recita un mantra accovacciat$b in terra",
				TRUE,ch,0,0,TO_ROOM);
		}
		sulcorpo=GET_LEVEL(ch,MONK_LEVEL_IND);
		af.type = SKILL_MANTRA;
		af.duration =sulcorpo;
		af.modifier = -MAX(90,2*sulcorpo);
		af.location = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(ch, &af);

		af.type = SPELL_NO_MESSAGE;
		af.duration = 2;
		af.modifier = MAX(90,2*sulcorpo);
		af.location = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(ch, &af);
		/* L`effetto della spell inizia solo dopo 2 tick */
		GET_POS(ch)=POSITION_RESTING;
	}

}

/*
* skill to allow monks to regen hp
* */

ACTION_FUNC(do_daimoku) {

	int sulcorpo;
	int dummy;
	struct affected_type af;

	WEARING_N(ch,dummy,sulcorpo);

	if(sulcorpo || !HasClass(ch,CLASS_MONK)) {
		send_to_char("Cerchi di visualizzare la struttura del tuo corpo"
					 ", ma qualcosa te l'impedisce",ch);
		return;
	}
	act("$n sembra trasfigurare nello sforzo di concentrarsi.....",
		TRUE,ch,0,0,TO_ROOM);
	act("Poi, mentre le sue ferite inziano a "
		"rimarginarsi, cade a terra stremat$b",
		TRUE,ch,0,0,TO_ROOM);
	act("Riesci a visualizzare l'intima struttura del tuo corpo",
		FALSE,ch,0,0,TO_CHAR);
	act("Riannodi i nervi e ricongiungi le fibre... "
		"poi sprofondi nel nirvana",
		FALSE,ch,0,0,TO_CHAR);
	dummy =number(0,2);
	GET_POS(ch)=POSITION_INCAP;
	GET_HIT(ch)=-1;
	alter_hit(ch,0);
	af.type = SPELL_NO_MESSAGE;
	af.duration = dummy;
	af.modifier = MAX(100,GET_MAX_HIT(ch));
	af.location = APPLY_HIT_REGEN;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	af.type      = SPELL_PARALYSIS;
	af.duration  = dummy;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_PARALYSIS;
	affect_join(ch, &af, FALSE, FALSE);

}


int SpyCheck(struct char_data* ch) {
	if(!ch->skills) {
		return FALSE;
	}

	if(number(1, 101) > ch->skills[ SKILL_SPY ].learned &&
			!IS_SET(ch->skills[ SKILL_SPY ].special, SKILL_SPECIALIZED)) {
		return FALSE;
	}

	return TRUE;

}

ACTION_FUNC(do_spy) {

	struct affected_type af;
	byte percent;

	send_to_char("Cerchi di usare le tue doti di cacciatore per guardare "
				 "avanti.\n\r", ch);

	if(IS_AFFECTED(ch, AFF_SCRYING))
		/* kinda pointless if they don't need to...*/
	{
		return;
	}

	if(affected_by_spell(ch, SKILL_SPY)) {
		send_to_char("Ti stai gia` comportando da cacciatore.\n\r", ch);
		return;
	}

	percent = number(1, 101);   /* 101% is a complete failure */

	if(!ch->skills) {
		return;
	}

	if(percent > ch->skills[ SKILL_SPY ].learned &&
			!IS_SET(ch->skills[ SKILL_SPY ].special, SKILL_SPECIALIZED)) {
		if(ch->skills[ SKILL_SPY ].learned < 95 &&
				ch->skills[SKILL_SPY].learned > 0) {
			if(number(1, 101) > ch->skills[ SKILL_SPY ].learned) {
				ch->skills[SKILL_SPY].learned++;
			}
		}
		af.type = SKILL_SPY;
		af.duration = (ch->skills[ SKILL_SPY ].learned / 10) + 1;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(ch, &af);
	}
	else {
		af.type = SKILL_SPY;
		af.duration = (ch->skills[ SKILL_SPY ].learned / 10) + 1;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = AFF_SCRYING;
		affect_to_char(ch, &af);
	}
}

int remove_trap(struct char_data* ch, struct obj_data* trap) {
	int num;
	struct obj_data* t;
	/* to disarm traps inside item */
	if(ITEM_TYPE(trap)  == ITEM_CONTAINER) {
		for(t=trap->contains; t; t=t->next_content) {
			if(ITEM_TYPE(t) == ITEM_TRAP && GET_TRAP_CHARGES(t) >0) {
				return(remove_trap(ch,t));
			}
		} /* end for */
	}                                 /* not container, trap on floor */
	if(ITEM_TYPE(trap) != ITEM_TRAP) {  // SALVO tolto else perche' il disarm veniva effettuato su qualunque container
		send_to_char("That's no trap!\n\r", ch);
		return(FALSE);
	}

	if(GET_TRAP_CHARGES(trap) <= 0) {
		send_to_char("That trap is already sprung!\n\r", ch);
		return(FALSE);
	}
	num = number(1,101);
	if(num < ch->skills[SKILL_REMOVE_TRAP].learned) {
		send_to_char("<Click>\n\r", ch);
		act("$n disarms $p", FALSE, ch, trap, 0, TO_ROOM);
		GET_TRAP_CHARGES(trap) = 0;
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return(TRUE);
	}
	else {
		send_to_char("<Click>\n\r(uh oh)\n\r", ch);
		act("$n attempts to disarm $p", FALSE, ch, trap, 0, TO_ROOM);
		TriggerTrap(ch, trap);

		return(TRUE);
	}
}

ACTION_FUNC(do_feign_death) {
	struct room_data* rp;
	struct char_data* t;

	if(!ch->skills) {
		return;
	}

	if(!ch->specials.fighting) {
		send_to_char("But you are not fighting anything...\n\r", ch);
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_MONK) && !IS_PRINCE(ch)) {
			send_to_char("You're no monk!\n\r", ch);
			return;
		}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	rp = real_roomp(ch->in_room);
	if(!rp) {
		return;
	}

	send_to_char("You try to fake your own demise\n\r", ch);

	death_cry(ch);
	act("$n is dead! R.I.P.", FALSE, ch, 0, 0, TO_ROOM);

	if(number(1,101) < ch->skills[SKILL_FEIGN_DEATH].learned) {
		stop_fighting(ch);
		for(t = rp->people; t; t=t->next_in_room) {
			if(t->specials.fighting == ch) {
				stop_fighting(t);
				if(number(1,101) < ch->skills[SKILL_FEIGN_DEATH].learned/2) {
					SET_BIT(ch->specials.affected_by, AFF_HIDE);
				}
				GET_POS(ch) = POSITION_SLEEPING;
			}
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
	}
	else {
		GET_POS(ch) = POSITION_SLEEPING;
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		if(ch->skills[SKILL_FEIGN_DEATH].learned < 95 &&
				ch->skills[SKILL_FEIGN_DEATH].learned > 0) {
			if(number(1,101) > ch->skills[SKILL_FEIGN_DEATH].learned) {
				ch->skills[SKILL_FEIGN_DEATH].learned++;
			}
		}
	}
}


ACTION_FUNC(do_first_aid) {
	struct affected_type af;
	int exp_level = 0;

	if(!ch->skills) {
		return;
	}

	if(!affected_by_spell(ch, SKILL_FIRST_AID)) {
		send_to_char("Cerchi di medicare alla meglio le tue ferite.\n\r", ch);
	}
	else {
		send_to_char("Devi aspettare ancora un po` prima di poter medicare "
					 "ancora le tue ferite.\n\r", ch);
		return;
	}

	if(IS_PC(ch)) {
		if(HasClass(ch, CLASS_BARBARIAN)) {
			exp_level =(int)(GET_LEVEL(ch, BARBARIAN_LEVEL_IND) / 1.5);
		}
		else if(HasClass(ch, CLASS_MONK)) {
			exp_level = GET_LEVEL(ch, MONK_LEVEL_IND) /2;
		}
		else if(HasClass(ch, CLASS_WARRIOR)) {
			exp_level = GET_LEVEL(ch, WARRIOR_LEVEL_IND)/ 4;
		}
		else if(HasClass(ch, CLASS_RANGER)) {
			exp_level = GET_LEVEL(ch, RANGER_LEVEL_IND);
		}
		else if(HasClass(ch, CLASS_PALADIN)) {
			exp_level = GET_LEVEL(ch, PALADIN_LEVEL_IND) / 2;    /* Bug fix GAia 2001 */
		}
	}

	if(number(1, 101) < ch->skills[SKILL_FIRST_AID].learned) {
		GET_HIT(ch) += number(1, 4) + (exp_level*2);
		if(GET_HIT(ch) > GET_MAX_HIT(ch)) {
			GET_HIT(ch) = GET_MAX_HIT(ch);
		}

		af.duration = 6;  /* Aumentata la frequenza Gaia 2001 */
	}
	else {
		af.duration = 3;
		for(exp_level=0; exp_level<5; exp_level++) {

			if(ch->skills[SKILL_FIRST_AID].learned < 95 &&
					ch->skills[SKILL_FIRST_AID].learned > 0) {
				if(number(1, 101) > ch->skills[ SKILL_FIRST_AID ].learned) {
					ch->skills[ SKILL_FIRST_AID ].learned++;
				}
			}
		}

	}

	af.type = SKILL_FIRST_AID;
	af.modifier = 0;
	af.location = APPLY_SKIP;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	return;
}


ACTION_FUNC(do_disguise) {
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	send_to_char("You attempt to disguise yourself\n\r", ch);

	if(affected_by_spell(ch, SKILL_DISGUISE)) {
		send_to_char("You can only do this once per day\n\r", ch);
		return;
	}

	if(number(1,101) < ch->skills[SKILL_DISGUISE].learned) {
		struct char_data* k;

		for(k=character_list; k; k=k->next) {
			if(k->specials.hunting == ch) {
				k->specials.hunting = 0;
			}
			if(number(1,101) < ch->skills[SKILL_DISGUISE].learned) {
				if(Hates(k, ch)) {
					ZeroHatred(k, ch);
				}
				if(Fears(k, ch)) {
					ZeroFeared(k, ch);
				}
			}
		}
	}
	else {
		if(ch->skills[SKILL_DISGUISE].learned < 95 &&
				ch->skills[SKILL_DISGUISE].learned > 0) {
			if(number(1,101) > ch->skills[SKILL_DISGUISE].learned) {
				ch->skills[SKILL_DISGUISE].learned++;
			}
		}
	}

	af.type = SKILL_DISGUISE;
	af.duration = 24;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	return;
}

/* Skill for climbing walls and the like -DM */
ACTION_FUNC(do_climb) {
	int dir;
	struct room_direction_data* exitp;
	int was_in, roll;

	char buf[256], direction[128];

	if(GET_MOVE(ch) < 10) {
		send_to_char("You're too tired to do that\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	/*
	 * make sure that the argument is a direction, or a keyword.
	 */

	for(; *arg == ' '; arg++);

	only_argument(arg,direction);

	if((dir = search_block(direction, dirs, FALSE)) < 0) {
		send_to_char("You can't climb that way.\n\r", ch);
		return;
	}


	exitp = EXIT(ch, dir);
	if(!exitp) {
		send_to_char("You can't climb that way.\n\r", ch);
		return;
	}

	if(!IS_SET(exitp->exit_info, EX_CLIMB)) {
		send_to_char("You can't climb that way.\n\r", ch);
		return;
	}

	if(dir == UP) {
		if(real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
				!IS_AFFECTED(ch, AFF_FLYING)) {
			send_to_char("You have no way of getting there!\n\r", ch);
			return;
		}
	}

	if(IS_SET(exitp->exit_info, EX_ISDOOR) &&
			IS_SET(exitp->exit_info, EX_CLOSED)) {
		send_to_char("You can't climb that way.\n\r", ch);
		return;
	}

	sprintf(buf, "$n attempts to climb %swards", dirs[dir]);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "You attempt to climb %swards\n\r", dirs[dir]);
	send_to_char(buf, ch);

	GET_MOVE(ch) -= 10;
	alter_move(ch,0);

	/*
	 * now we've checked for failures, time to check for success;
	 */
	if(ch->skills) {
		if(ch->skills[SKILL_CLIMB].learned) {
			roll = number(1, 100);
			if(roll > ch->skills[SKILL_CLIMB].learned) {
				slip_in_climb(ch, dir, exitp->to_room);
				LearnFromMistake(ch, SKILL_CLIMB, 0, 95);
			}
			else {
				was_in = ch->in_room;

				char_from_room(ch);
				char_to_room(ch, exitp->to_room);
				do_look(ch, "", 15);
				DisplayMove(ch, dir, was_in, 1);
				if(!check_falling(ch)) {
					if(IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
							GetMaxLevel(ch) < IMMORTALE) {
						NailThisSucker(ch);
						return;
					}

				}
				WAIT_STATE(ch, PULSE_VIOLENCE*3); // climb
				GET_MOVE(ch) -= 10;
				alter_move(ch,0);
				return;
			}
		}
		else {
			send_to_char("You just don't know the nuances of climbing.\n\r", ch);
			slip_in_climb(ch, dir, exitp->to_room);
			return;
		}
	}
	else {
		send_to_char("You're just a goofy mob.\n\r", ch);
		return;
	}
}

void slip_in_climb(struct char_data* ch, int dir, int room) {
	int i;

	i = number(1, 6);

	if(dir != DOWN) {
		act("$n falls down and goes splut.", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char("You fall.\n\r", ch);
	}
	else {
		act("$n loses $s grip and falls further down.", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char("You slip and start to fall.\n\r", ch);
		i += number(1, 6);
		char_from_room(ch);
		char_to_room(ch, room);
		do_look(ch, "", 15);
	}

	GET_POS(ch) = POSITION_SITTING;
	if(i > GET_HIT(ch)) {
		GET_HIT(ch) = 1;
		alter_hit(ch,0);
	}
	else {
		GET_HIT(ch) -= i;
		alter_hit(ch,0);
	}
}


ACTION_FUNC(do_tan) {
	struct obj_data* j=0;
	struct obj_data* hide;
	char itemname[80],itemtype[80],hidetype[80],buf[MAX_STRING_LENGTH];
	int percent=0;
	int i =0 ;

	int acapply=0;
	int acbonus=0;
	int char_bonus=0;
	int total_bonus=0;
	int lev=0;
	int r_num=0;
	int special=0;
	int apply = 0;
	int app_val = 0;

	if(IS_NPC(ch)) {
		return;
	}

	if(!ch->skills) {
		return;
	}
	if(MOUNTED(ch)) {
		send_to_char("Not from this mount you cannot!\n\r",ch);
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
		if(!HasClass(ch,CLASS_BARBARIAN|CLASS_WARRIOR|CLASS_RANGER|CLASS_DRUID)) {
			send_to_char("What do you think you are, A tanner?\n\r",ch);
			return;
		}
	}

	arg = one_argument(arg,itemname);
	arg = one_argument(arg,itemtype);

	if(!*itemname) {
		send_to_char("Tan what?\n\r",ch);
		return;
	}

	if(!*itemtype) {
		send_to_char("I see that, but what do you wanna make?\n\r",ch);
		return;
	}

	if(!(j = get_obj_in_list_vis(ch, itemname,
								 real_roomp(ch->in_room)->contents))) {
		send_to_char("Where did that carcuss go?\n\r",ch);
		return;
	}
	else {
		/* affect[0] == race of corpse, affect[1] == level of corpse */
		if(j->affected[0].modifier !=0 && j->affected[1].modifier !=0) {
			percent = number(1,101); /* 101% is a complete failure */

			if(ch->skills && ch->skills[ SKILL_TAN ].learned &&
					GET_POS(ch) > POSITION_SLEEPING) {
				if(percent > ch->skills[SKILL_TAN].learned) {
					/* FAILURE! */
					j->affected[1].modifier=0; /* make corpse unusable for another tan */

					sprintf(buf,
							"You hack at %s but manage to only destroy the hide.\n\r",
							j->short_description);
					send_to_char(buf,ch);

					sprintf(buf, "%s tries to skins %s for it's hide, but destroys it.",
							GET_NAME(ch),j->short_description);
					act(buf,TRUE, ch, 0, 0, TO_ROOM);
					LearnFromMistake(ch, SKILL_TAN, 0, 95);
					WAIT_STATE(ch, PULSE_VIOLENCE * 3);
					return;
				}


				/* item not a corpse if v3 = 0 */
				if(!j->obj_flags.value[ 3 ]) {
					send_to_char("Sorry, this is not a carcuss.\n\r",ch);
					return;
				}

				lev = j->affected[1].modifier/10 ;
				if(j->affected[1].modifier > 50) {
					lev ++ ;
				}
				char_bonus = GetMaxLevel(ch)/10  ;

				if(HasClass(ch,CLASS_WARRIOR)) {
					char_bonus ++ ;
				}
				if(HasClass(ch,CLASS_BARBARIAN)) {
					char_bonus +=2 ;
				}
				if(HasClass(ch,CLASS_RANGER)) {
					char_bonus +=3;
				}
				if(HasClass(ch,CLASS_DRUID)) {
					char_bonus +=4;
				}
				if(GET_DEX(ch) > 18) {
					char_bonus ++;
				}
				if(GET_DEX(ch) > 19) {
					char_bonus ++;
				}
				if(GET_RACE(ch) == RACE_TROLL) {
					char_bonus -= 2;
				}
				else if(GET_RACE(ch) == RACE_GOLD_ELF) {
					char_bonus += 2;
				}
				else if(GET_RACE(ch) == RACE_DARK_ELF) {
					char_bonus ++;
				}
				else if(GET_RACE(ch) == RACE_WILD_ELF) {
					char_bonus ++;
				}
				else if(GET_RACE(ch) == RACE_SEA_ELF) {
					char_bonus ++;
				}
				else if(GET_RACE(ch) == RACE_DWARF) {
					char_bonus += 3;
				}

				char_bonus -= HowManyClasses(ch);


				switch(j->affected[0].modifier) {
				/* We could use a array using the race as a pointer */
				/* but this way makes it more visable and easier to handle */
				/* however it is ugly. */
				/* Uso questo per definire le caratteristiche di base dell'
				   oggetto */

				case RACE_HALFBREED :
					sprintf(hidetype,"halfbreed leather");
					acapply=-1;
					break;
				case RACE_HUMAN     :
					sprintf(hidetype,"human leather");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_ELVEN     :
					sprintf(hidetype,"elf hide");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_DWARF     :
					sprintf(hidetype,"dwarf hide");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_HALFLING  :
					sprintf(hidetype,"halfing hide");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_GNOME     :
					sprintf(hidetype,"gnome hide");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_REPTILE  :
					sprintf(hidetype,"reptile hide");
					acapply=3;
					break;
				case RACE_SPECIAL  :
				case RACE_LYCANTH  :
					sprintf(hidetype,"hairy leather");
					acapply=2;
					break;
				case RACE_DRAGON   :
					sprintf(hidetype,"dragon hide");
					acapply=6;
					break;
				case RACE_UNDEAD   :
				case RACE_UNDEAD_VAMPIRE :
				case RACE_UNDEAD_LICH    :
				case RACE_UNDEAD_WIGHT   :
				case RACE_UNDEAD_GHAST   :
				case RACE_UNDEAD_SPECTRE :
				case RACE_UNDEAD_ZOMBIE  :
				case RACE_UNDEAD_SKELETON :
				case RACE_UNDEAD_GHOUL    :
					sprintf(hidetype,"rotting hide");
					acapply=-2;
					lev=(int)lev/2;
					break;
				case RACE_ORC      :
					sprintf(hidetype,"orc hide");
					acapply=0;
					lev=(int)lev/2;
					break;
				case RACE_INSECT   :
					sprintf(hidetype,"insectiod hide");
					acapply=2;
					break;
				case RACE_ARACHNID :
					sprintf(hidetype,"hairy leather");
					acapply=1;
					lev=(int)lev/2;
					break;
				case RACE_DINOSAUR :
					sprintf(hidetype,"thick leather");
					acapply=3;
					break;
				case RACE_FISH     :
					sprintf(hidetype,"fishy hide");
					acapply=3;
					break;
				case RACE_BIRD     :
					sprintf(hidetype,"feathery hide");
					acapply=0;
					lev=(int)lev/2;
					break;
				case RACE_GIANT    :
					sprintf(hidetype,"giantish hide");
					acapply=3;
					break;
				case RACE_PREDATOR :
					sprintf(hidetype,"elastic leather");
					acapply=1;
					break;
				case RACE_PARASITE :
					sprintf(hidetype,"purulent leather");
					acapply=1;
					break;
				case RACE_SLIME    :
					sprintf(hidetype,"transparent leather");
					acapply=0;
					lev=(int)lev/2;
					break;
				case RACE_DEMON    :
					sprintf(hidetype,"demon hide");
					acapply=4;
					break;
				case RACE_SNAKE    :
					sprintf(hidetype,"snake hide");
					acapply=3;
					break;
				case RACE_HERBIV   :
					sprintf(hidetype,"green leather");
					acapply=1;
					break;
				case RACE_TREE     :
					sprintf(hidetype,"bark hide");
					acapply=5;
					break;
				case RACE_VEGGIE   :
					sprintf(hidetype,"green hide");
					acapply=2;
					break;
				case RACE_ELEMENT  :
					sprintf(hidetype,"colored leather");
					acapply=1;
					break;
				case RACE_PLANAR   :
					sprintf(hidetype,"planar leather");
					acapply=1;
					break;
				case RACE_DEVIL    :
					sprintf(hidetype,"devil hide");
					acapply=4;
					break;
				case RACE_GHOST    :
					sprintf(hidetype,"ghostly hide");
					acapply=1;
					break;
				case RACE_GOBLIN   :
					sprintf(hidetype,"goblin hide");
					acapply=0;
					lev=(int)lev/2;
					break;
				case RACE_TROLL    :
					sprintf(hidetype,"troll leather");
					acapply=2;
					break;
				case RACE_GNOLL    :
					sprintf(hidetype,"gnoll leather");
					acapply=2;
					break;
				case RACE_VEGMAN   :
					sprintf(hidetype,"green hide");
					acapply=2;
					break;
				case RACE_MFLAYER  :
					sprintf(hidetype,"mindflayer hide");
					acapply=1;
					break;
				case RACE_PRIMATE  :
					sprintf(hidetype,"monkey leather");
					acapply=0;
					break;
				case RACE_ENFAN    :
					sprintf(hidetype,"enfan hide");
					acapply=-1;
					lev=(int)lev/2;
					break;
				case RACE_DARK_ELF     :
					sprintf(hidetype,"drow hide");
					acapply=-2;
					lev=(int)lev/2;
					break;
				case RACE_GOLEM    :
					sprintf(hidetype,"golem hide");
					acapply=1;
					break;
				case RACE_SKEXIE   :
					sprintf(hidetype,"skexie");
					acapply=1;
					break;
				case RACE_LIZARDMAN:
					sprintf(hidetype,"lizard hide");
					acapply=1;
					break;
				case RACE_TROGMAN  :
				case RACE_PATRYN   :
				case RACE_LABRAT   :
				case RACE_SARTAN   :
					sprintf(hidetype,"black leather");
					acapply=1;
					break;
				case RACE_TYTAN   :
					sprintf(hidetype,"tytan hide");
					acapply=3;
					break;
				case RACE_SMURF    :
					sprintf(hidetype,"smurf hide");
					acapply=0;
					break;
				case RACE_ROO      :
					sprintf(hidetype,"roo hide");
					acapply=1;
					break;
				case RACE_HORSE    :
				case RACE_DRAAGDIM :
					sprintf(hidetype,"leather");
					acapply=2;
					break;
				case RACE_ASTRAL   :
					sprintf(hidetype,"strange hide");
					acapply=2;
					break;
				case RACE_GOD      :
					sprintf(hidetype,"god hide");
					acapply=3;
					break;
				case RACE_GIANT_HILL   :
					sprintf(hidetype,"hill giant hide");
					acapply=2;
					break;
				case RACE_GIANT_FROST  :
					sprintf(hidetype,"frost giant hide");
					acapply=2;
					break;
				case RACE_GIANT_FIRE   :
					sprintf(hidetype,"fire giant hide");
					acapply=2;
					break;
				case RACE_GIANT_CLOUD  :
					sprintf(hidetype,"cloud giant hide");
					acapply=1;
					break;
				case RACE_GIANT_STORM  :
					sprintf(hidetype,"storm giant hide");
					acapply=2;
					break;
				case RACE_GIANT_STONE  :
					sprintf(hidetype,"stone giant hide");
					acapply=3;
					break;
				case RACE_DRAGON_RED   :
					sprintf(hidetype,"red dragon hide");
					acapply = 6;
					break;
				case RACE_DRAGON_BLACK :
					sprintf(hidetype,"black dragon hide");
					acapply = 5;
					break;
				case RACE_DRAGON_GREEN :
					sprintf(hidetype,"green dragon hide");
					acapply = 5;
					break;
				case RACE_DRAGON_WHITE :
					sprintf(hidetype,"white dragon hide");
					acapply = 5;
					break;
				case RACE_DRAGON_BLUE  :
					sprintf(hidetype,"blue dragon hide");
					acapply = 6;
					break;
				case RACE_DRAGON_SILVER:
					sprintf(hidetype,"silver dragon hide");
					acapply = 6;
					break;
				case RACE_DRAGON_GOLD  :
					sprintf(hidetype,"gold dragon hide");
					acapply = 7;
					break;
				case RACE_DRAGON_BRONZE:
					sprintf(hidetype,"bronze dragon hide");
					acapply = 6;
					break;
				case RACE_DRAGON_COPPER:
					sprintf(hidetype,"copper dragon hide");
					acapply = 6;
					break;
				case RACE_DRAGON_BRASS :
					sprintf(hidetype,"brass dragon hide");
					acapply = 7;
					break;
				default:
					sprintf(hidetype,"leather");
					acapply = 1;
					break;

				} /* end switch race of carcuss */

				/* figure out what type of armor it is and make it. */

				acapply += ((int)lev/3 + (int)char_bonus/3);  /* 1-6 */
				acbonus += ((int)lev/2 + (int)char_bonus/2);  /* 1-6 */


				if(acapply<0) {
					acapply=0;
				}

				if(!strcmp(itemtype,"shield")) {
					if((r_num = real_object(TAN_SHIELD)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply++;
					acbonus+=3;
					strcat(hidetype," shield");

					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 26 */

					switch(j->affected[0].modifier) {
					case RACE_ARACHNID :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_DEMON    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_SNAKE    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_POISON ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_POISON ;
						}
						break ;
					case RACE_TREE     :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_SLASH ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_FROST  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_GIANT_FIRE   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_CLOUD  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_GIANT_STORM  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
                    case RACE_UNDEAD   :
                    case RACE_UNDEAD_VAMPIRE :
                    case RACE_UNDEAD_LICH    :
                    case RACE_UNDEAD_WIGHT   :
                    case RACE_UNDEAD_GHAST   :
                    case RACE_UNDEAD_SPECTRE :
                    case RACE_UNDEAD_ZOMBIE  :
                    case RACE_UNDEAD_SKELETON :
                    case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						break ;
                    case RACE_DRAGON_RED   :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_DRAGON_BLACK :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_DRAGON_GREEN :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_WHITE :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_DRAGON_BLUE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
					case RACE_DRAGON_SILVER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_DRAGON_GOLD  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_CHARM ;
						}
						break ;
					case RACE_DRAGON_BRONZE:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DRAGON_COPPER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_BRASS :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_BLUNT ;
						}
						break ;
					case RACE_TROLL :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 2 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SUSC ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_SLIME      :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_ORC :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_CHR ;
							app_val = -1 ;
						}
						break ;
					case RACE_GOLEM      :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITROLL ;
							app_val = 2 ;
						}
						break ;
					case RACE_LIZARDMAN :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_TYTAN :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_AC ;
							app_val = -30 ;
						}
						break ;
					case RACE_GNOLL :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITNDAM ;
							app_val = 1 ;
						}
						break ;
					default:
						break;
					}


				}
				else if(!strcmp(itemtype,"armor")) {
					if((r_num = real_object(TAN_ARMOR)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply++;
					acbonus+=(int)((lev+char_bonus)/2);
					strcat(hidetype," armor ");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 26 */
					switch(j->affected[0].modifier) {
					case RACE_ARACHNID :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_DEMON    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_SNAKE    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_POISON ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_POISON ;
						}
						break ;
					case RACE_TREE     :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_SLASH ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_FROST  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_GIANT_FIRE   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_CLOUD  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_GIANT_STORM  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
                    case RACE_UNDEAD   :
                    case RACE_UNDEAD_VAMPIRE :
                    case RACE_UNDEAD_LICH    :
                    case RACE_UNDEAD_WIGHT   :
                    case RACE_UNDEAD_GHAST   :
                    case RACE_UNDEAD_SPECTRE :
                    case RACE_UNDEAD_ZOMBIE  :
                    case RACE_UNDEAD_SKELETON :
                    case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						break ;
					case RACE_DRAGON_RED   :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_DRAGON_BLACK :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_DRAGON_GREEN :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_WHITE :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_DRAGON_BLUE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
					case RACE_DRAGON_SILVER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_DRAGON_GOLD  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_CHARM ;
						}
						break ;
					case RACE_DRAGON_BRONZE:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DRAGON_COPPER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_BRASS :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_INSECT   :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITROLL ;
							app_val = 2 ;
						}
						break ;
					case RACE_DINOSAUR :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_MOVE ;
							app_val = 20 ;
						}
						break ;
					case RACE_GIANT    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HIT ;
							app_val = 10 ;
						}
						break ;
					case RACE_GHOST    :
						if(total_bonus > 20) {
							special = 1 ;
							apply = APPLY_HIDE ;
							app_val = 5 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA ;
							app_val = 15 ;
						}
						break ;
					/* added by REQUIEM 2018 */

					case RACE_VEGGIE      :
					case RACE_VEGMAN      :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_HIT ;
							app_val = 20 ;
						}
						break ;
					case RACE_DARK_ELF     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_INT ;
							app_val = 4 ;
						}
						break ;
					case RACE_TROLL :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SUSC ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_SKEXIE      :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_MANA_REGEN ;
							app_val = 5 ;
						}
						break ;
					case RACE_TYTAN      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 2 ;
						}
						break ;
					default:
						break;
					}
				}
				else if(!strcmp(itemtype,"jacket")) {
					if((r_num = real_object(TAN_JACKET)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply++;
					acbonus+=(lev+char_bonus);
					strcat(hidetype," jacket");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 26 */
					switch(j->affected[0].modifier) {
					case RACE_ARACHNID :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_DEMON    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_SNAKE    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_POISON ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_POISON ;
						}
						break ;
					case RACE_TREE     :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_SLASH ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_FROST  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_GIANT_FIRE   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_GIANT_CLOUD  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_GIANT_STORM  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
                    case RACE_UNDEAD   :
                    case RACE_UNDEAD_VAMPIRE :
                    case RACE_UNDEAD_LICH    :
                    case RACE_UNDEAD_WIGHT   :
                    case RACE_UNDEAD_GHAST   :
                    case RACE_UNDEAD_SPECTRE :
                    case RACE_UNDEAD_ZOMBIE  :
                    case RACE_UNDEAD_SKELETON :
                    case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						break ;
					case RACE_DRAGON_RED   :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_DRAGON_BLACK :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_DRAGON_GREEN :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_WHITE :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_COLD ;
						}
						break ;
					case RACE_DRAGON_BLUE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ELEC ;
						}
						break ;
					case RACE_DRAGON_SILVER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_DRAGON_GOLD  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_CHARM ;
						}
						break ;
					case RACE_DRAGON_BRONZE:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_PIERCE ;
						}
						break ;
					case RACE_DRAGON_COPPER:
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_DRAGON_BRASS :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ENERGY ;
						}
						break ;
					case RACE_INSECT   :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITROLL ;
							app_val = 2 ;
						}
						break ;
					case RACE_DINOSAUR :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_MOVE ;
							app_val = 20 ;
						}
						break ;
					case RACE_GIANT    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HIT ;
							app_val = 10 ;
						}
						break ;
					case RACE_GHOST    :
						if(total_bonus > 20) {
							special = 1 ;
							apply = APPLY_HIDE ;
							app_val = 5 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA ;
							app_val = 15 ;
						}
						break ;
					case RACE_VEGGIE      :
					case RACE_VEGMAN     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_HIT ;
							app_val = 20 ;
						}
						break ;
					case RACE_ENFAN     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_SNEAK ;
							app_val = 50 ;
						}
						break ;
					case RACE_TROLL     :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 2 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SUSC ;
							app_val = IMM_FIRE ;
						}
						break ;
					case RACE_SLIME     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_GOLEM     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_CON ;
							app_val = 3 ;
						}
						break ;
					case RACE_LIZARDMAN     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_HOLD ;
						}
						break ;
					case RACE_TYTAN     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_STR ;
							app_val = 3 ;
						}
						break ;
					case RACE_GNOLL     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_HITNDAM ;
							app_val = 1 ;
						}
						break ;
					default:
						break;
					}
				}
				else if(!strcmp(itemtype,"boots")) {
					if((r_num = real_object(TAN_BOOTS)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply--;
					if(acapply <0) {
						acapply=0;
					}
					acbonus--;
					if(acbonus <0) {
						acbonus=0;
					}
					strcat(hidetype," pair of boots");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 30 */
					switch(j->affected[0].modifier) {
					case RACE_HALFBREED :
					case RACE_HUMAN     :
					case RACE_ELVEN     :
					case RACE_DWARF     :
					case RACE_HALFLING  :
					case RACE_GNOME     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX ;
							app_val = 1 ;
						}
						break ;
					case RACE_REPTILE  :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_KICK ;
							app_val = 5 ;
						}
						break ;
					case RACE_INSECT   :
					case RACE_ARACHNID :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_BIRD     :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MOVE_REGEN ;
							app_val = 10 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_MANA ;
							app_val = 10 ;
						}
						break ;
					case RACE_VEGGIE      :
					case RACE_VEGMAN     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_HIT_REGEN ;
							app_val = 10 ;
						}
						break ;
					case RACE_DARK_ELF     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_DEX ;
							app_val = 4 ;
						}
						break ;
					case RACE_PRIMATE     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_MOVE_REGEN ;
							app_val = 15 ;
						}
						break ;
					case RACE_GOLEM     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_CHAR_HEIGHT ;
							app_val = 50 ;
						}
						break ;
                    case RACE_UNDEAD   :
                    case RACE_UNDEAD_VAMPIRE :
                    case RACE_UNDEAD_LICH    :
                    case RACE_UNDEAD_WIGHT   :
                    case RACE_UNDEAD_GHAST   :
                    case RACE_UNDEAD_SPECTRE :
                    case RACE_UNDEAD_ZOMBIE  :
                    case RACE_UNDEAD_SKELETON :
                    case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						else if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_IMMUNE ;
							app_val = IMM_DRAIN ;
						}
						break ;
					case RACE_TYTAN     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_CHAR_HEIGHT ;
							app_val = 200 ;
						}
						break ;
					case RACE_GNOLL     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_CHAR_HEIGHT ;
							app_val = 100 ;
						}
						break ;
					default:
						break;
					}
				}
				else if(!strcmp(itemtype,"gloves")) {
					if((r_num = real_object(TAN_GLOVES)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}

					acapply--;
					if(acapply<0) {
						acapply=0;
					}
					acbonus--;
					if(acbonus<0) {
						acbonus=0;
					}
					strcat(hidetype," pair of gloves");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 30 */
					switch(j->affected[0].modifier) {
					case RACE_HALFBREED :
					case RACE_HUMAN     :
					case RACE_ELVEN     :
					case RACE_DWARF     :
					case RACE_HALFLING  :
					case RACE_GNOME     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITNDAM ;
							app_val = 1 ;
						}
						break ;
					case RACE_REPTILE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_STR ;
							app_val = 1 ;
						}
						break ;
					case RACE_LYCANTH  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_STR ;
							app_val = 2 ;
						}
						break ;
					case RACE_DRAGON   :
					case RACE_DRAGON_RED   :
					case RACE_DRAGON_BLACK :
					case RACE_DRAGON_GREEN :
					case RACE_DRAGON_WHITE :
					case RACE_DRAGON_BLUE  :
					case RACE_DRAGON_SILVER:
					case RACE_DRAGON_GOLD  :
					case RACE_DRAGON_BRONZE:
					case RACE_DRAGON_COPPER:
					case RACE_DRAGON_BRASS :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_HITNDAM;
							app_val = 1 ;
						}
						break ;
					case RACE_UNDEAD   :
					case RACE_UNDEAD_VAMPIRE :
					case RACE_UNDEAD_LICH    :
					case RACE_UNDEAD_WIGHT   :
					case RACE_UNDEAD_GHAST   :
					case RACE_UNDEAD_SPECTRE :
					case RACE_UNDEAD_ZOMBIE  :
					case RACE_UNDEAD_SKELETON :
					case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -10 ;
						}
						break ;
					case RACE_INSECT   :
					case RACE_ARACHNID :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX;
							app_val = 1 ;
						}
						break ;
					case RACE_FISH     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_STEAL;
							app_val = 5 ;
						}
						break ;
					case RACE_ELEMENT  :
					case RACE_PLANAR   :
					case RACE_MFLAYER  :
					case RACE_ASTRAL   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 5 ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 10 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA;
							app_val = 20 ;
						}
						break ;
					case RACE_ENFAN     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_STEAL ;
							app_val = 30 ;
						}
						break ;
					case RACE_DARK_ELF      :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_MANA ;
							app_val = 25 ;
						}
						break ;
					case RACE_GNOLL     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_STR ;
							app_val = 2 ;
						}
						break ;
					default:
						break;
					}
				}
				else if(!strcmp(itemtype,"leggings")) {
					if((r_num = real_object(TAN_LEGGINGS)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply++;
					acbonus++;
					strcat(hidetype," set of leggings");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 30 */
					switch(j->affected[0].modifier) {
					case RACE_HALFBREED :
					case RACE_HUMAN     :
					case RACE_ELVEN     :
					case RACE_DWARF     :
					case RACE_HALFLING  :
					case RACE_GNOME     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX ;
							app_val = 1 ;
						}
						break ;
					case RACE_REPTILE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_LYCANTH  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 2 ;
						}
						break ;
					case RACE_DRAGON   :
					case RACE_DRAGON_RED   :
					case RACE_DRAGON_BLACK :
					case RACE_DRAGON_GREEN :
					case RACE_DRAGON_WHITE :
					case RACE_DRAGON_BLUE  :
					case RACE_DRAGON_SILVER:
					case RACE_DRAGON_GOLD  :
					case RACE_DRAGON_BRONZE:
					case RACE_DRAGON_COPPER:
					case RACE_DRAGON_BRASS :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_STR;
							app_val = 2 ;
						}
						break ;
					case RACE_UNDEAD   :
					case RACE_UNDEAD_VAMPIRE :
					case RACE_UNDEAD_LICH    :
					case RACE_UNDEAD_WIGHT   :
					case RACE_UNDEAD_GHAST   :
					case RACE_UNDEAD_SPECTRE :
					case RACE_UNDEAD_ZOMBIE  :
					case RACE_UNDEAD_SKELETON :
					case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -10 ;
						}
						break ;
					case RACE_INSECT   :
					case RACE_ARACHNID :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX;
							app_val = 1 ;
						}
						break ;
					case RACE_FISH     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SNEAK;
							app_val = 5 ;
						}
						break ;
					case RACE_ELEMENT  :
					case RACE_PLANAR   :
					case RACE_MFLAYER  :
					case RACE_ASTRAL   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 5 ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA;
							app_val = 10 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -10 ;
						}
						break ;
					/* added by REQUIEM 2018 */

					case RACE_TROLL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HIT_REGEN;
							app_val = 30 ;
						}
						break ;
					case RACE_GOLEM    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITROLL;
							app_val = 3 ;
						}
						break ;
					case RACE_SKEXIE    :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 10 ;
						}
						break ;
					default:
						break;
					}
				}
				else if(!strcmp(itemtype,"sleeves")) {
					if((r_num = real_object(TAN_SLEEVES)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply++;
					acbonus++;
					strcat(hidetype," set of sleeves");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 30 */
					switch(j->affected[0].modifier) {
					case RACE_HALFBREED :
					case RACE_HUMAN     :
					case RACE_ELVEN     :
					case RACE_DWARF     :
					case RACE_HALFLING  :
					case RACE_GNOME     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX ;
							app_val = 1 ;
						}
						break ;
					case RACE_REPTILE  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HITROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_LYCANTH  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_DRAGON   :
					case RACE_DRAGON_RED   :
					case RACE_DRAGON_BLACK :
					case RACE_DRAGON_GREEN :
					case RACE_DRAGON_WHITE :
					case RACE_DRAGON_BLUE  :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HITNDAM;
							app_val = 1 ;
						}
						break ;
					case RACE_DRAGON_SILVER:
					case RACE_DRAGON_GOLD  :
					case RACE_DRAGON_BRONZE:
					case RACE_DRAGON_COPPER:
					case RACE_DRAGON_BRASS :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_STR;
							app_val = 2 ;
						}
						break ;
					case RACE_UNDEAD   :
					case RACE_UNDEAD_VAMPIRE :
					case RACE_UNDEAD_LICH    :
					case RACE_UNDEAD_WIGHT   :
					case RACE_UNDEAD_GHAST   :
					case RACE_UNDEAD_SPECTRE :
					case RACE_UNDEAD_ZOMBIE  :
					case RACE_UNDEAD_SKELETON :
					case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA;
							app_val = 20 ;
						}
						break ;
					case RACE_INSECT   :
					case RACE_ARACHNID :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX;
							app_val = 1 ;
						}
						break ;
					case RACE_FISH     :
						if(total_bonus > 22) {
							special = 1 ;
							apply = APPLY_STEAL;
							app_val = 5 ;
						}
						break ;
					case RACE_ELEMENT  :
					case RACE_PLANAR   :
					case RACE_MFLAYER  :
					case RACE_ASTRAL   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 5 ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -15 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HITROLL;
							app_val = 2 ;
						}
						break ;

					/* added by REQUIEM 2018 */

					case RACE_DARK_ELF    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_INT;
							app_val = 4 ;
						}
						break ;
					case RACE_TROLL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HIT_REGEN;
							app_val = 30 ;
						}
						break ;
					case RACE_PRIMATE    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MOVE_REGEN;
							app_val = 25 ;
						}
						break ;
					case RACE_SKEXIE    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -15 ;
						}
						break ;
					case RACE_TYTAN    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DAMROLL;
							app_val = 2 ;
						}
						break ;
					default:
						break;
					}
				}
				else  if(!strcmp(itemtype,"helmet")) {
					if((r_num = real_object(TAN_HELMET)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					acapply--;
					if(acapply<0) {
						acapply=0;
					}
					acbonus--;
					if(acbonus<0) {
						acbonus=0;
					}
					strcat(hidetype," helmet");
					total_bonus=char_bonus+lev+number(1,12); /* A value between 1 and 30 */
					switch(j->affected[0].modifier) {
					case RACE_HALFBREED :
					case RACE_HUMAN     :
					case RACE_ELVEN     :
					case RACE_DWARF     :
					case RACE_HALFLING  :
					case RACE_GNOME     :
						if(total_bonus < 15) {
							special = 1 ;
							apply = APPLY_CHR ;
							app_val = -2 ;
						}
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_M_IMMUNE ;
							app_val = IMM_CHARM ;
						}
						break ;
					case RACE_LYCANTH  :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						break ;
					case RACE_DRAGON   :
					case RACE_DRAGON_RED   :
					case RACE_DRAGON_BLACK :
					case RACE_DRAGON_GREEN :
					case RACE_DRAGON_WHITE :
					case RACE_DRAGON_BLUE  :
					case RACE_DRAGON_SILVER:
					case RACE_DRAGON_GOLD  :
					case RACE_DRAGON_BRONZE:
					case RACE_DRAGON_COPPER:
					case RACE_DRAGON_BRASS :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_WIS;
							app_val = 3 ;
						}
						break ;
					case RACE_UNDEAD   :
					case RACE_UNDEAD_VAMPIRE :
					case RACE_UNDEAD_LICH    :
					case RACE_UNDEAD_WIGHT   :
					case RACE_UNDEAD_GHAST   :
					case RACE_UNDEAD_SPECTRE :
					case RACE_UNDEAD_ZOMBIE  :
					case RACE_UNDEAD_SKELETON :
					case RACE_UNDEAD_GHOUL    :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_SPELLFAIL;
							app_val = -15 ;
						}
						break ;
					case RACE_INSECT   :
					case RACE_ARACHNID :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_CON;
							app_val = 1 ;
						}
						break ;
					case RACE_FISH     :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_DEX;
							app_val = 1 ;
						}
						break ;
					case RACE_ELEMENT  :
					case RACE_PLANAR   :
					case RACE_MFLAYER  :
					case RACE_ASTRAL   :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_INT;
							app_val = 2 ;
						}
						break ;
					case RACE_DEVIL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_MANA_REGEN;
							app_val = 8 ;
						}
						break ;
					case RACE_GOD      :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HITROLL;
							app_val = 2 ;
						}
						break ;

					/* added by REQUIEM 2018 */

					case RACE_VEGGIE    :
					case RACE_VEGMAN    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_HIT_REGEN;
							app_val = 10 ;
						}
						break ;
					case RACE_DARK_ELF    :
						if(total_bonus > 25) {
							special = 1 ;
							apply = APPLY_FIND_TRAPS ;
							app_val = 100 ;
						}
						else if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_FIND_TRAPS ;
							app_val = 60 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_FIND_TRAPS ;
							app_val = 30 ;
						}
						break ;
					case RACE_TROLL    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_DAMROLL ;
							app_val = 1 ;
						}
						else if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_SUSC ;
							app_val = IMM_ACID ;
						}
						break ;
					case RACE_SKEXIE    :
						if(total_bonus > 23) {
							special = 1 ;
							apply = APPLY_MANA_REGEN ;
							app_val = 5 ;
						}
						break ;
					case RACE_TYTAN    :
						if(total_bonus > 24) {
							special = 1 ;
							apply = APPLY_HITNDAM ;
							app_val = 1 ;
						}
						break ;
					default:
						break;
					}
				}
				else  if(!strcmp(itemtype,"bag")) {
					if((r_num = real_object(TAN_BAG)) >= 0) {
						hide = read_object(r_num, REAL);
						obj_to_char(hide,ch);
					}
					strcat(hidetype," bag");
				}
				else {
					send_to_char("Illegal type of equipment!\n\r",ch);
					return;
				}

				sprintf(buf,"%s name %s",itemtype,hidetype);
				do_ooedit(ch,buf,0);

				sprintf(buf,"%s ldesc A %s lies here",itemtype,hidetype);
				do_ooedit(ch,buf,0);

				sprintf(buf,"%s sdesc a %s made by %s",itemtype, hidetype, GET_NAME(ch));
				do_ooedit(ch,buf,0);

				/* we do not mess with vX if the thing is a bag */
				if(strcmp(itemtype,"bag")) {
					sprintf(buf,"%s v0 %d",itemtype,acapply);
					do_ooedit(ch,buf,0);
					/* I think v1 is how many times it can be hit, so lev of
					 * corpse /10 times */
					sprintf(buf,"%s v1 %d",itemtype,(int)lev/10);
					do_ooedit(ch,buf,0);
					/* add in AC bonus here */
					sprintf(buf,"%s aff1 %d 17",itemtype,0-acbonus);
					do_ooedit(ch,buf,0);
				}                 /* was not a bag ^ */
				if(special) {
					i = getFreeAffSlot(hide);
					hide->affected[i].location = apply;
					hide->affected[i].modifier = app_val;
				}

				j->affected[1].modifier=0; /* make corpse unusable for another tan */

				sprintf(buf,"You hack at the %s and finally make the %s.\n\r",
						j->short_description,itemtype);
				send_to_char(buf,ch);

				sprintf(buf,"%s skins %s for it's hide.",GET_NAME(ch),
						j->short_description);
				act(buf,TRUE, ch, 0, 0, TO_ROOM);
				WAIT_STATE(ch, PULSE_VIOLENCE*((int)lev/2));
				return;
			}
		}
		else {
			send_to_char("Sorry, nothing left of the carcuss to make a item with.\n\r",ch);
			return;
		}
	}
}

ACTION_FUNC(do_find_food) {
	int r_num,percent=0;
	struct obj_data* obj;
	struct room_data* pRoom;

	if(!ch->skills) {
		return;
	}

	if(ch->skills[SKILL_FIND_FOOD].learned <=0) {
		send_to_char("Cerchi qualcosa da mangiare, ma non ne sei capace.\n\r",ch);
		return;
	}

	if(!OUTSIDE(ch)) {
		send_to_char("Devi essere all'aperto.\n\r",ch);
		return;
	}

	/*ACIDUS 2003, controllo il tipo di settore associato alla stanza del pg*/
	pRoom =  real_roomp(ch->in_room);

	if(pRoom->sector_type == SECT_AIR) {
		act("Stai fluttuando nell'aria, non riuscirai a trovare niente da mangiare!",
			TRUE, ch, 0, 0, TO_CHAR);
		return;
	}

	if(pRoom->sector_type == SECT_TREE) {
		act("Sei dentro un albero, non riuscirai a trovare niente da mangiare!",
			TRUE, ch, 0, 0, TO_CHAR);
		return;
	}

	percent = number(1,101); /* 101% is a complete failure */

	if(ch->skills && ch->skills[SKILL_FIND_FOOD].learned &&
			GET_POS(ch) > POSITION_SITTING) {
		if(percent > ch->skills[SKILL_FIND_FOOD].learned) {
			/* failed */
			act("Cerchi qualcosa da mangiare ma non riesci a trovare nulla.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n cerca qualcosa da mangiare ma non riesce a trovare nulla.",
				TRUE, ch, 0, 0, TO_ROOM);
		}
		else {
			/* made it */
			act("Cerchi da mangiare e trovi qualcosa di commestibile.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n cerca da mangiare e trova qualcosa di commestibile.",
				TRUE, ch, 0, 0, TO_ROOM);


			switch(pRoom->sector_type) {
			case SECT_UNDERWATER:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD1);
				}
				else {
					percent = number(1,100);
					if(percent <= 6) {
						r_num = real_object(FOUND_FOOD0);
					}
					else if(percent > 6 && percent <= 36) {
						r_num = real_object(FOUND_FOOD1);
					}
					else if(percent > 36 && percent <= 45) {
						r_num = real_object(FOUND_FOOD2);
					}
					else if(percent > 45 && percent <= 54) {
						r_num = real_object(FOUND_FOOD3);
					}
					else if(percent > 54 && percent <= 79) {
						r_num = real_object(FOUND_FOOD4);
					}
					else if(percent > 79 && percent <= 100) {
						r_num = real_object(FOUND_FOOD5);
					}
				}
				break ;
			case SECT_FOREST:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD39);
				}
				else {
					percent = number(1,100);
					if(percent <= 18) {
						r_num = real_object(FOUND_FOOD39);
					}
					else if(percent > 18 && percent <= 25) {
						r_num = real_object(FOUND_FOOD40);
					}
					else if(percent > 25 && percent <= 32) {
						r_num = real_object(FOUND_FOOD41);
					}
					else if(percent > 32 && percent <= 34) {
						r_num = real_object(FOUND_FOOD42);
					}
					else if(percent > 34 && percent <= 38) {
						r_num = real_object(FOUND_FOOD43);
					}
					else if(percent > 38 && percent <= 42) {
						r_num = real_object(FOUND_FOOD44);
					}
					else if(percent > 42 && percent <= 46) {
						r_num = real_object(FOUND_FOOD45);
					}
					else if(percent > 46 && percent <= 48) {
						r_num = real_object(FOUND_FOOD46);
					}
					else if(percent > 48 && percent <= 54) {
						r_num = real_object(FOUND_FOOD47);
					}
					else if(percent > 54 && percent <= 58) {
						r_num = real_object(FOUND_FOOD48);
					}
					else if(percent > 58 && percent <= 60) {
						r_num = real_object(FOUND_FOOD49);
					}
					else if(percent > 60 && percent <= 74) {
						r_num = real_object(FOUND_FOOD50);
					}
					else if(percent > 74 && percent <= 86) {
						r_num = real_object(FOUND_FOOD51);
					}
					else if(percent > 86 && percent <= 93) {
						r_num = real_object(FOUND_FOOD52);
					}
					else if(percent > 93 && percent <= 100) {
						r_num = real_object(FOUND_FOOD53);
					}
				}
				break ;
			case SECT_INSIDE:
			case SECT_CITY:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD63);
				}
				else {
					percent = number(1,100);
					if(percent <= 90) {
						r_num = real_object(FOUND_FOOD63);
					}
					else if(percent > 90 && percent <= 100) {
						r_num = real_object(FOUND_FOOD64);
					}
				}
				break ;
			case SECT_DARKCITY:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD54);
				}
				else {
					percent = number(1,100);
					if(percent <= 82) {
						r_num = real_object(FOUND_FOOD54);
					}
					else if(percent > 82 && percent <= 91) {
						r_num = real_object(FOUND_FOOD55);
					}
					else if(percent > 91 && percent <= 100) {
						r_num = real_object(FOUND_FOOD56);
					}
				}
				break ;
			case SECT_MOUNTAIN:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD7);
				}
				else {
					percent = number(1,100);
					if(percent <= 5) {
						r_num = real_object(FOUND_FOOD6);
					}
					else if(percent > 5 && percent <= 29) {
						r_num = real_object(FOUND_FOOD7);
					}
					else if(percent > 29 && percent <= 32) {
						r_num = real_object(FOUND_FOOD8);
					}
					else if(percent > 32 && percent <= 42) {
						r_num = real_object(FOUND_FOOD9);
					}
					else if(percent > 42 && percent <= 47) {
						r_num = real_object(FOUND_FOOD10);
					}
					else if(percent > 47 && percent <= 52) {
						r_num = real_object(FOUND_FOOD11);
					}
					else if(percent > 52 && percent <= 57) {
						r_num = real_object(FOUND_FOOD12);
					}
					else if(percent > 57 && percent <= 62) {
						r_num = real_object(FOUND_FOOD13);
					}
					else if(percent > 62 && percent <= 80) {
						r_num = real_object(FOUND_FOOD14);
					}
					else if(percent > 80 && percent <= 90) {
						r_num = real_object(FOUND_FOOD15);
					}
					else if(percent > 90 && percent <= 100) {
						r_num = real_object(FOUND_FOOD16);
					}
				}
				break ;
			case SECT_FIELD:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD19);
				}
				else {
					percent = number(1,100);
					if(percent <= 2) {
						r_num = real_object(FOUND_FOOD17);
					}
					else if(percent > 2 && percent <= 6) {
						r_num = real_object(FOUND_FOOD18);
					}
					else if(percent > 6 && percent <= 26) {
						r_num = real_object(FOUND_FOOD19);
					}
					else if(percent > 26 && percent <= 31) {
						r_num = real_object(FOUND_FOOD20);
					}
					else if(percent > 31 && percent <= 38) {
						r_num = real_object(FOUND_FOOD21);
					}
					else if(percent > 38 && percent <= 45) {
						r_num = real_object(FOUND_FOOD22);
					}
					else if(percent > 45 && percent <= 48) {
						r_num = real_object(FOUND_FOOD23);
					}
					else if(percent > 48 && percent <= 55) {
						r_num = real_object(FOUND_FOOD24);
					}
					else if(percent > 55 && percent <= 64) {
						r_num = real_object(FOUND_FOOD25);
					}
					else if(percent > 64 && percent <= 74) {
						r_num = real_object(FOUND_FOOD26);
					}
					else if(percent > 74 && percent <= 82) {
						r_num = real_object(FOUND_FOOD27);
					}
					else if(percent > 82 && percent <= 91) {
						r_num = real_object(FOUND_FOOD28);
					}
					else if(percent > 91 && percent <= 100) {
						r_num = real_object(FOUND_FOOD29);
					}
				}
				break ;
			case SECT_HILLS:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD30);
				}
				else {
					percent = number(1,100);
					if(percent <= 25) {
						r_num = real_object(FOUND_FOOD30);
					}
					else if(percent > 25 && percent <= 36) {
						r_num = real_object(FOUND_FOOD31);
					}
					else if(percent > 36 && percent <= 44) {
						r_num = real_object(FOUND_FOOD32);
					}
					else if(percent > 44 && percent <= 52) {
						r_num = real_object(FOUND_FOOD33);
					}
					else if(percent > 52 && percent <= 55) {
						r_num = real_object(FOUND_FOOD34);
					}
					else if(percent > 55 && percent <= 61) {
						r_num = real_object(FOUND_FOOD35);
					}
					else if(percent > 61 && percent <= 70) {
						r_num = real_object(FOUND_FOOD36);
					}
					else if(percent > 70 && percent <= 89) {
						r_num = real_object(FOUND_FOOD37);
					}
					else if(percent > 89 && percent <= 100) {
						r_num = real_object(FOUND_FOOD38);
					}
				}
				break ;
			case SECT_DESERT:

				/*il draagdim nel deserto segue una logica diversa, \E8 sul suo terreno*/
				if(GET_RACE(ch) == RACE_DRAAGDIM) {
					percent = number(1,100);
					if(GetMaxLevel(ch)<=10) {
						if(percent <= 95) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					else if(GetMaxLevel(ch)<=20) {
						if(percent <= 90) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					else if(GetMaxLevel(ch)<=30) {
						if(percent <= 85) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					else if(GetMaxLevel(ch)<=45) {
						if(percent <= 80) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					else if(GetMaxLevel(ch)<=50) {
						if(percent <= 75) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					else {
						if(percent <= 70) {
							r_num = real_object(FOUND_FOOD57);
						}
						else {
							r_num = real_object(FOUND_FOOD60);
						}
					}
					break ;
				}

				/* le altre razze hanno condizioni diverse dal Draagdim */
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD57);
				}
				else {
					percent = number(1,100);
					if(percent <= 45) {
						r_num = real_object(FOUND_FOOD57);
					}
					else if(percent > 45 && percent <= 57) {
						r_num = real_object(FOUND_FOOD58);
					}
					else if(percent > 57 && percent <= 62) {
						r_num = real_object(FOUND_FOOD59);
					}
					else if(percent > 62 && percent <= 67) {
						r_num = real_object(FOUND_FOOD60);
					}
					else if(percent > 67 && percent <= 82) {
						r_num = real_object(FOUND_FOOD61);
					}
					else if(percent > 82 && percent <= 100) {
						r_num = real_object(FOUND_FOOD62);
					}
				}
				break ;
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
				if(GetMaxLevel(ch)<20 ||
						!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER|CLASS_DRUID)) {
					r_num = real_object(FOUND_FOOD65);
				}
				else {
					percent = number(1,100);
					if(percent <= 39) {
						r_num = real_object(FOUND_FOOD65);
					}
					else if(percent > 39 && percent <= 44) {
						r_num = real_object(FOUND_FOOD66);
					}
					else if(percent > 44 && percent <= 56) {
						r_num = real_object(FOUND_FOOD67);
					}
					else if(percent > 56 && percent <= 84) {
						r_num = real_object(FOUND_FOOD68);
					}
					else if(percent > 84 && percent <= 100) {
						r_num = real_object(FOUND_FOOD69);
					}
				}
				break ;

			default:
				r_num = real_object(FOUND_FOOD);
				break;
			}


			if(r_num >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj,ch);
			}
			/*else
				{
			  obj = read_object(real_object(FOUND_FOOD), REAL);
			  obj_to_char(obj,ch);
				}*/

		}
		/*ACIDUS 2003 il Draagdim non ha lag quando usa il find food*/
		if(GET_RACE(ch) == RACE_DRAAGDIM) {
			WAIT_STATE(ch, PULSE_VIOLENCE*1);
		}
		else {
			WAIT_STATE(ch, PULSE_VIOLENCE*3);
		}

	} /* ^ had the skill */
	else { /* didn't have the skill... */
		act("You search around for some edibles but failed to find anything.",
			TRUE, ch, 0, 0, TO_CHAR);
		act("$n searches and searches for something to eat but comes up empty.",
			TRUE, ch, 0, 0, TO_ROOM);
	}
}

ACTION_FUNC(do_find_food_old) {
	int r_num,percent=0;
	struct obj_data* obj;

	if(!ch->skills) {
		return;
	}

	if(ch->skills[SKILL_FIND_FOOD].learned <=0) {
		send_to_char("You search blindly for anything, but fail.\n\r.",ch);
		return;
	}

	if(!OUTSIDE(ch)) {
		send_to_char("You need to be outside.\n\r",ch);
		return;
	}

	percent = number(1,101); /* 101% is a complete failure */

	if(ch->skills && ch->skills[SKILL_FIND_FOOD].learned &&
			GET_POS(ch) > POSITION_SITTING) {
		if(percent > ch->skills[SKILL_FIND_FOOD].learned) {
			/* failed */
			act("You search around for some edibles but failed to find anything.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n searches and searches for something to eat but comes up empty.",
				TRUE, ch, 0, 0, TO_ROOM);
		}
		else {
			/* made it */
			act("You search around for some edibles and managed to find some roots and berries.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n searches the area for something to eat and manages to find something.",
				TRUE, ch, 0, 0, TO_ROOM);
			if((r_num = real_object(FOUND_FOOD)) >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj,ch);
			}
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	} /* ^ had the skill */
	else { /* didn't have the skill... */
		act("You search around for some edibles but failed to find anything.",
			TRUE, ch, 0, 0, TO_CHAR);
		act("$n searches and searches for something to eat but comes up empty.",
			TRUE, ch, 0, 0, TO_ROOM);
	}
}



ACTION_FUNC(do_find_water) {
	int r_num,percent=0;
	struct obj_data* obj;

	if(!ch->skills) {
		return;
	}

	if(ch->skills[SKILL_FIND_WATER].learned <=0) {
		send_to_char("You search blindly for anything, but fail.\n\r.",ch);
		return;
	}


	if(!OUTSIDE(ch)) {
		send_to_char("You need to be outside.\n\r",ch);
		return;
	}

	percent = number(1,101); /* 101% is a complete failure */

	if(ch->skills && ch->skills[SKILL_FIND_WATER].learned &&
			GET_POS(ch) > POSITION_SITTING) {
		if(percent > ch->skills[SKILL_FIND_WATER].learned) {
			/* failed */
			act("You search around for stream or puddle of water but failed to find anything.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n searches and searches for something to drink but comes up empty.",
				TRUE, ch, 0, 0, TO_ROOM);
		}
		else {
			/* made it */
			act("You search around and find enough water to fill a water cup.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n searches the area for something to drink and manages to find a small amount of water.",
				TRUE, ch, 0, 0, TO_ROOM);
			if((r_num = real_object(FOUND_WATER)) >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj,ch);
			}
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	} /* ^ had the skill */
	else { /* didn't have the skill... */
		act("You search around for stream or puddle of water but failed to find anything.",
			TRUE, ch, 0, 0, TO_CHAR);
		act("$n searches and searches for something to drink but comes up empty.",
			TRUE, ch, 0, 0, TO_ROOM);
	}
}

ACTION_FUNC(do_find_traps) {
	if(!ch->skills) {
		return;
	}

	if(!IS_PC(ch)) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_THIEF)) {
			send_to_char("What do you think you are?!?\n\r",ch);
			return;
		}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	send_to_char("You are already on the look out for those silly.\n\r",ch);
}


ACTION_FUNC(do_find) {
	char findwhat[30];

	if(!ch->skills) {
		return;
	}

	arg = one_argument(arg,findwhat); /* ACK! find water, call that function! */

	if(!strcmp(findwhat,"water")) {
		do_find_water(ch,arg,cmd);
	}
	else if(!strcmp(findwhat,"food")) {
		do_find_food(ch,arg,cmd);
	}
	else if(!strcmp(findwhat,"traps")) {
		do_find_traps(ch,arg,cmd);
	}
	else {
		send_to_char("Find what?!?!?\n\r",ch);
	}
}

ACTION_FUNC(do_bellow) {
	struct char_data* vict,*tmp;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_BARBARIAN) && !HasClass(ch,CLASS_WARRIOR)) {
			send_to_char("What do you think you are, a warrior!\n\r",ch);
			return;
		}

	if(check_peaceful(ch,
					  "You feel too peaceful to contemplate violence.\n\r")) {
		return;
	}

	if(check_soundproof(ch)) {
		send_to_char("You cannot seem to break the barrier of silence here.\n\r",ch);
		return;
	}

	if(GET_MANA(ch) < 15)  {
		send_to_char("You just cannot get enough energy together for a bellow.\n\r",ch);
		return;
	}

	if(ch->skills && ch->skills[SKILL_BELLOW].learned &&
			(number(1,101) < ch->skills[SKILL_BELLOW].learned)) {

		GET_MANA(ch)-=15;
		alter_mana(ch,0);
		send_to_char("You let out a bellow that rattles your bones!\n\r",ch);
		act("$n lets out a bellow that rattles your bones.",FALSE,ch,0,0,TO_ROOM);

		for(vict=character_list; vict; vict= tmp) {
			tmp=vict->next;
			if(ch->in_room == vict->in_room && ch!=vict) {
				if(!in_group(ch,vict) && !IS_IMMORTAL(vict)) {
					if(GetMaxLevel(vict)-3 <= GetMaxLevel(ch)) {
						if(!saves_spell(vict, SAVING_PARA)) {
							/* they did not save here */
							if((GetMaxLevel(ch)+number(1,40)) > 70) {
								act("You stunned $N!",TRUE,ch,0,vict,TO_CHAR);
								act("$n stuns $N with a loud bellow!",FALSE,ch,0,vict,TO_ROOM);
								GET_POS(vict) = POSITION_STUNNED;
								AddFeared(vict,ch);
							}
							else {
								act("You scared $N to death with your bellow!",TRUE,ch,0,vict,TO_CHAR);
								act("$n scared $N with a loud bellow!",FALSE,ch,0,vict,TO_ROOM);
								do_flee(vict,"",0);
								AddFeared(vict,ch);
							}

						}
						else {
							/* they saved */
							AddHated(vict,ch);
							set_fighting(vict,ch);
						}
					}                                 /* ^ level was greater or equal to mob */

					else {                                 /* V-- level was lower than mobs */
						/* nothing happens */
						AddHated(vict,ch);
						set_fighting(vict,ch);
					}

				} /*  group/immo */
			} /* inroom */
		} /* end for */
	}
	else {                                           /* failed skill check */
		send_to_char("You let out a squalk!\n\r",ch);
		act("$n lets out a squalk of a bellow then blushes.",FALSE,ch,0,0,TO_ROOM);
		LearnFromMistake(ch, SKILL_BELLOW, 0, 95);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

/* ranger skill */
ACTION_FUNC(do_carve) {
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct obj_data* corpse;
	struct obj_data* food;
	int i,r_num;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_RANGER)) {
			send_to_char("Hum, you wonder how you would do this...\n\r",ch);
			return;
		}

	if(!ch->skills[SKILL_RATION].learned)    {
		send_to_char("Best leave the carving to the skilled.\n\r",ch);
		return;
	}

	half_chop(arg,arg1,arg2,sizeof arg1 -1,sizeof arg2 -1);
	corpse=get_obj_in_list_vis(ch,arg1,(real_roomp(ch->in_room)->contents));

	if(!corpse)  {
		send_to_char("That's not here.\n\r",ch);
		return;
	}

	if(!IS_CORPSE(corpse)) {
		send_to_char("You can't carve that!\n\r",ch);
		return;
	}

	if(corpse->obj_flags.weight<70)    {
		send_to_char("There is no good meat left on it.\n\r",ch);
		return;
	}


	if((GET_MANA(ch) < 10) && GetMaxLevel(ch) < IMMORTALE)    {
		send_to_char("You don't have the concentration to do this.\n\r",ch);
		return;
	}


	if(ch->skills[SKILL_RATION].learned < dice(1,101)) {
		send_to_char("You can't seem to locate the choicest parts of the corpse.\n\r",ch);
		GET_MANA(ch) -= 5;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_RATION, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}

	act("$n carves up the $p and creates a healthy ration.",FALSE,ch,corpse,0,TO_ROOM);
	send_to_char("You carve up a fat ration.\n\r",ch);

	if((r_num = real_object(FOUND_FOOD)) >= 0) {
		food = read_object(r_num, REAL);
		food->name= (char*)strdup("ration slice filet food");
		sprintf(buffer,"a Ration%s",corpse->short_description+10);
		food->short_description= (char*)strdup(buffer);
		food->action_description= (char*)strdup(buffer);
		sprintf(arg2,"%s is lying on the ground.",buffer);
		food->description= (char*)strdup(arg2);
		corpse->obj_flags.weight=corpse->obj_flags.weight-50;
		i=number(1,6);
		if(i==6) {
			food->obj_flags.value[3]=1;
		}
		obj_to_room(food,ch->in_room);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	} /* we got the numerb of the item... */

}

ACTION_FUNC(do_doorway) {
	char target_name[140];
	struct char_data* target;
	int location;
	struct room_data* rp;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI) && !IS_IMMORTAL(ch) && !IS_PRINCE(ch)) {
			send_to_char("Your mind is not developed enough to do this\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!ch->skills[SKILL_DOORWAY].learned) {
		send_to_char("You have not trained your mind to do this\n\r",ch);
		return;
	}

	only_argument(arg,target_name);
	if(!(target=get_char_vis_world(ch,target_name,NULL))) {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}

	location = target->in_room;
	rp = real_roomp(location);

	if(GetMaxLevel(target) > MAX_MORT || !rp ||
			IS_SET(rp->room_flags,  PRIVATE | NO_SUM | NO_MAGIC)) {
		send_to_char("Your mind is not yet strong enough.\n\r", ch);
		return;
	}

	if(IS_SET(SystemFlags,SYS_NOPORTAL)) {
		send_to_char("The planes are fuzzy, you cannot portal!\n",ch);
		return;
	}

	if(!IsOnPmp(ch->in_room)) {
		send_to_char("You're on an extra-dimensional plane!\n\r", ch);
		return;
	}

	if(!IsOnPmp(target->in_room)) {
		send_to_char("They're on an extra-dimensional plane!\n\r", ch);
		return;
	}

	if(GetMaxLevel(target)>=IMMORTALE) {
		send_to_char("You mind does not have the power to doorway to this person\n\r",ch);
		return;
	}
	/* Added for pkillers Gaia 2001 */
	if(IS_AFFECTED2((IS_POLY(ch)) ? ch->desc->original : ch, AFF2_PKILLER) &&  // SALVO controllo sul pkiller
			rp->room_flags&PEACEFUL)  {
		send_to_char("Your mind get confused and cannot envision your target any more. \n\r", ch);
		return;
	}
	if((GET_MANA(ch) < 20) && GetMaxLevel(ch) <= PRINCIPE) {  // SALVO mana negativa solo agli immortali
		send_to_char("You have a headache. Better rest before you try this again.\n\r",ch);
		return;
	}
	else if(dice(1,101) > ch->skills[SKILL_DOORWAY].learned) {
		send_to_char("You cannot open a portal at this time.\n\r", ch);
		act("$n seems to briefly disappear, then returns!",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=10;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_DOORWAY, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}
	else {
		GET_MANA(ch)-=20;
		alter_mana(ch,0);
		send_to_char("You close your eyes and open a portal and quickly step through.\n\r",ch);
		act("$n closes $s eyes and a shimmering portal appears!\n\r",FALSE,ch,0,0,TO_ROOM);
		act("$n steps through the portal and the portal dissapears!\n\r",FALSE,ch,0,0,TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, location);
		act("A portal appears before you and $n steps through!",FALSE,ch,0,0,TO_ROOM);
		do_look(ch, "",15);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	}
}


ACTION_FUNC(do_psi_portal) {
	char target_name[140];
	struct char_data* target;
	struct char_data* follower;
	struct char_data* leader;
	struct follow_type* f_list;
	int location;
	int check=0;
	struct room_data* rp;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI) && !IS_IMMORTAL(ch)) {
			send_to_char("Your mind is not developed enough to do this.\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!ch->skills[SKILL_PORTAL].learned) {
		send_to_char("You have not trained your mind to do this.\n\r",ch);
		return;
	}

	only_argument(arg,target_name);
	if(!(target=get_char_vis_world(ch,target_name,NULL))) {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}

	location = target->in_room;
	rp = real_roomp(location);
	if(GetMaxLevel(target) > MAX_MORT ||       !rp ||
			IS_SET(rp->room_flags,  PRIVATE | NO_SUM | NO_MAGIC)) {
		send_to_char("You cannot penetrate the auras surrounding that person.\n\r", ch);
		return;
	}

	if(IS_SET(SystemFlags,SYS_NOPORTAL)) {
		send_to_char("The planes are fuzzy, you cannot portal!\n",ch);
		return;
	}

	if(!IsOnPmp(ch->in_room)) {
		send_to_char("You're on an extra-dimensional plane!\n\r", ch);
		return;
	}

	if(!IsOnPmp(target->in_room)) {
		send_to_char("They're on an extra-dimensional plane!\n\r", ch);
		return;
	}
	/* Added for Pkillers Gaia 2001 */
	if(IS_AFFECTED2((IS_POLY(ch)) ? ch->desc->original : ch, AFF2_PKILLER) &&  // SALVO controllo pkiller
			IS_SET(rp->room_flags, PEACEFUL))  {
		send_to_char("Your mind get confused and cannot envision your target anymore. \n\r", ch);
		return;
	}

	if((GET_MANA(ch) < 75) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You have a headache. Better rest before you try this again.\n\r",ch);
		return;
	}
	else if(dice(1,101) > ch->skills[SKILL_PORTAL].learned) {
		send_to_char("You fail to open a portal at this time.\n\r", ch);
		act("$n briefly summons a portal, then curses as it disappears.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=37;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_PORTAL, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
	}
	else {
		GET_MANA(ch)-=75;
		alter_mana(ch,0);
		send_to_char("You close your eyes and open a portal and quickly step through.\n\r",ch);
		act("$n closes their eyes and a shimmering portal appears!",FALSE,ch,0,0,TO_ROOM);
		act("A portal appears before you!",FALSE,target,0,0,TO_ROOM);
		leader=ch->master;
		if(!leader) {
			leader=ch;
			check=1;
		}
		/* leader goes first, otherwise we miss them */
		if(leader!=ch &&
				(!leader->specials.fighting) &&
				(IS_AFFECTED(leader,AFF_GROUP)) &&
				(IS_PC(leader) || IS_SET(leader->specials.act,ACT_POLYSELF))) {
			act("$n steps through the portal and disappears!",FALSE,leader,0,0,TO_ROOM);
			send_to_char("You step through the shimmering portal.\n\r",leader);
			char_from_room(leader);
			char_to_room(leader,location);
			act("$n steps out of a portal before you!",FALSE,leader,0,0,TO_ROOM);
			do_look(leader,"",15);
		}

		for(f_list = leader->followers; f_list ; f_list = f_list->next) {
			if(!f_list) {
				mudlog(LOG_SYSERR, "logic error in portal follower loop");
				return;
			}
			follower = f_list->follower;
			if(!follower) {
				mudlog(LOG_SYSERR, "pointer error in portal follower loop");
				return;
			}

			if((follower)&&
					(follower->in_room==ch->in_room)&&
					(follower!=ch) &&
					(!follower->specials.fighting) &&
					(IS_PC(follower)||IS_SET(follower->specials.act,ACT_POLYSELF)) &&
					IS_AFFECTED(follower,AFF_GROUP) &&
					/* Added for Pkillers Gaia 2001 */
					!(IS_AFFECTED2((IS_POLY(ch)) ? ch->desc->original : ch, AFF2_PKILLER) &&  // SALVO controllo pkiller
					  IS_SET(rp->room_flags, PEACEFUL))) {
				act("$n steps through the portal and disappears!",FALSE,follower,0,0,TO_ROOM);
				send_to_char("You step through the shimmering portal.\n\r",follower);
				char_from_room(follower);
				char_to_room(follower,location);
				act("$n steps out of a portal before you!",FALSE,follower,0,0,TO_ROOM);
				do_look(follower,"",15);
			}

		}                       /* end follower list.. */

		if(check==1) { /* ch was leader */
			send_to_char("Now that all your comrades are through, you follow them and close the portal.\n\r",ch);
		}
		act("$n steps into the portal just before it disappears.",FALSE,ch,0,0,TO_ROOM);
		char_from_room(ch);
		char_to_room(ch,location);
		do_look(ch, "",15);
		act("$n appears out of the portal as it disappears!",FALSE,ch,0,0,TO_ROOM);

		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	}

}





ACTION_FUNC(do_mindsummon) {
	char target_name[140];
	struct char_data* target;
	int location;
	struct room_data* rp;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI) && !IS_IMMORTAL(ch)) {
			send_to_char("Your mind is not developed enough to do this\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!ch->skills[SKILL_SUMMON].learned) {
		send_to_char("You have not trained your mind to do this\n\r",ch);
		return;
	}

	only_argument(arg,target_name);
	if(!(target=get_char_vis_world(ch,target_name,NULL)))   {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}
	if(target==ch)   {
		send_to_char("You're already in the room with yourself!\n\r",ch);
		return;
	}

	location = target->in_room;
	rp = real_roomp(location);
	if(!rp || IS_SET(rp->room_flags,  PRIVATE | NO_SUM | NO_MAGIC)) {
		send_to_char("Your mind cannot seem to locate this individual.\n\r", ch);
		return;
	}

	if(IS_SET(SystemFlags,SYS_NOSUMMON)) {
		send_to_char("A mistical fog blocks your attemps!\n",ch);
		return;
	}

	if(!IsOnPmp(target->in_room)) {
		send_to_char("They're on an extra-dimensional plane!\n\r", ch);
		return;
	}

	location = target->in_room;
	rp = real_roomp(location);
	if(!rp || rp->sector_type == SECT_AIR ||
			rp->sector_type == SECT_WATER_SWIM) {
		send_to_char("You cannot seem to focus on the target.\n\r",ch);
		return;
	}


	location = ch->in_room;
	rp = real_roomp(location);
	if(!rp || IS_SET(rp->room_flags,  PRIVATE | NO_SUM | NO_MAGIC)) {
		send_to_char("Arcane magics prevent you from summoning here.\n\r", ch);
		return;
	}

	location = ch->in_room;
	rp = real_roomp(location);
	if(!rp || rp->sector_type == SECT_AIR
			|| rp->sector_type == SECT_WATER_SWIM) {
		send_to_char("You cannot seem to focus correctly here.\n\r",ch);
		return;
	}

	/* we check hps on mobs summons */

	if(!IS_SET(target->specials.act,ACT_POLYSELF) && !IS_PC(target))  {
		if(GetMaxLevel(target) > MAX_MORT || GET_MAX_HIT(target) > GET_HIT(ch)) {
			send_to_char("Your mind is not yet strong enough to summon this individual.\n\r", ch);
			return;
		}
	}
	else                           /* pc's we summon without HPS check */
		if(GetMaxLevel(target) > MAX_MORT) {
			send_to_char("Your mind is not yet strong enough to summon this individual.\n\r", ch);
			return;
		}

	if(CanFightEachOther(ch,target))
		if(saves_spell(target, SAVING_SPELL) &&
				(GetMaxLevel(ch)-GetMaxLevel(target)+number(1,100))>20) {
			act("You failed to summon $N!",FALSE,ch,0,target,TO_CHAR);
			act("$n tried to summon you!",FALSE,ch,0,target,TO_VICT);
			return;
		}


	if((GET_MANA(ch) < 30) && GetMaxLevel(ch) < IMMORTALE)    {
		send_to_char("You have a headache. Better rest before you try this again.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_SUMMON].learned < dice(1,101) || target->specials.fighting)   {
		send_to_char("You have failed to open the portal to summon this individual.\n\r",ch);
		act("$n seems to think really hard then gasps in anger.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch) -= 15;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_SUMMON, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}


	GET_MANA(ch) -=30;
	alter_mana(ch,0);

	if(saves_spell(target,SAVING_SPELL) && IS_NPC(target)) {
		act("$N resists your attempt to summon!",FALSE,ch,0,target,TO_CHAR);
		return;
	}

	act("You open a portal and bring forth $N!",FALSE,ch,0,target,TO_CHAR);
	if(GetMaxLevel(target) < GetMaxLevel(ch)+2 && !IS_PC(target)) {
		send_to_char("Their head is reeling. Give them a moment to recover.\n\r",ch);
	}
	act("$n disappears in a shimmering wave of light!",TRUE,target,0,0,TO_ROOM);

	if(IS_PC(target)) {
		act("You are summoned by $n!",TRUE,ch,0,target,TO_VICT);
	}

	char_from_room(target);
	char_to_room(target,ch->in_room);

	act("$n summons $N from nowhere!",TRUE,ch,0,target,TO_NOTVICT);

	if(GetMaxLevel(target) < GetMaxLevel(ch)+2 && !IS_PC(target))  {
		act("$N is lying on the ground stunned!",TRUE,ch,0,target,TO_ROOM);
		target->specials.position = POSITION_STUNNED;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*4);
}

/**
* Flyp 20180128 --> immolation, ovvero canibaliaze modificato per i demoni
**/
ACTION_FUNC(do_immolation) {
	long hit_points,mana_points;  /* hit_points has to be long for storage */
	char number[80];  /* NOTE: the argument function returns FULL argument */
	/* if u just allocate 10 char it will overrun! */
	/* if the argument returns something > 10 char */
	int count;
	bool num_found=TRUE;

	if(GET_RACE(ch) != RACE_DEMON || !ch->skills[SKILL_IMMOLATION].learned) {
		send_to_char("Ma se non hai neanche le corna....", ch);
		return;
	}

	/**
	* Per ora commento, ma potrebbe servire il controllo sulla classe
	if (!HasClass(ch,CLASS_MAGIC_USER))
	{
	  send_to_char ("You don't have any kind of control over your body like that!\n\r",ch);
	  return;
	}
	**/

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	only_argument(arg,number);

	/* polax version of number validation */
	/* NOTE: i changed num_found to be initially TRUE */

	for(count=0; num_found && (count < 9) && (number[count] != '\0'); count++) {
		if((number[count] < '0') || (number[count] > '9')) {  /* leading zero is ok */
			num_found = FALSE;
		}
	}

	/* polax modification ends */

#if 0
	for(count=0; (!num_found) && (count<9); count++)
		if((number[count]>='1') && (number[count]<='9')) {
			num_found=TRUE;
		}
#endif

	if(!num_found) {
		send_to_char("How many life do you want to immolate?\n\r",ch);
		return;
	}
	else {
		number[count] = '\0';    /* forced the string to be proper length */
	}

	sscanf(number,"%ld",&hit_points);   /* long int conversion */
	mudlog(LOG_SYSERR, "read [%i] hit points to sacrifice", hit_points);

	if((hit_points <1) || (hit_points > 65535)) {
		/* bug fix? */
		send_to_char("You cannot immolate such amount of life!.\n\r",ch);
		return;
	}

	// Check position: only standing or fighting.
	// demons has leech, so don't be too generous with them!
	switch(GET_POS(ch)) {
	case POSITION_STANDING:
		mana_points = hit_points;
		break;
	case POSITION_FIGHTING:
		mana_points = hit_points/2;
		break;
	default:
		send_to_char("You need to be standing and maybe fighting to do this!",ch);
		break;
	}

	mudlog(LOG_SYSERR, "I'll try to conbert to [%i] mana", mana_points);

	if(mana_points <0) {
		send_to_char("You can't do that, You Knob!\n\r",ch);
	}

	if((int)ch->points.hit < (hit_points+5)) {
		send_to_char("You don't have enough physical stamina to immolate.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)+mana_points) > (GET_MAX_MANA(ch))) {
		send_to_char("Your mind cannot handle that much extra energy.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_IMMOLATION].learned < dice(1,101)) {
		send_to_char("You try to immolate your stamina but the energy escapes before you can harness it.\n\r",ch);
		act("$n yelps in pain.",FALSE,ch,0,0,TO_ROOM);
		GET_HIT(ch) -= hit_points;
		alter_hit(ch,0);
		update_pos(ch);
		if(GET_POS(ch)==POSITION_DEAD) {
			die(ch,SKILL_IMMOLATION, NULL);
		}
		LearnFromMistake(ch, SKILL_IMMOLATION, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
	}

	send_to_char("You sucessfully convert your stamina to Mental power.\n\r",ch);
	act("$n briefly is surrounded by a red aura.",FALSE,ch,0,0,TO_ROOM);
	GET_HIT(ch) -= hit_points;
	alter_hit(ch,0);
	GET_MANA(ch) += mana_points;
	alter_mana(ch,0);

	update_pos(ch);
	if(GET_POS(ch)==POSITION_DEAD) {
		die(ch,SKILL_IMMOLATION, NULL);
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*1);
}


/** Flyp **/


ACTION_FUNC(do_canibalize) {
	long hit_points,mana_points;  /* hit_points has to be long for storage */
	char number[80];  /* NOTE: the argument function returns FULL argument */
	/* if u just allocate 10 char it will overrun! */
	/* if the argument returns something > 10 char */
	int count;
	bool num_found=TRUE;

	if(!ch->skills) {
		return;
	}


	if(!HasClass(ch,CLASS_PSI) ||!ch->skills[SKILL_CANIBALIZE].learned) {
		send_to_char("You don't have any kind of control over your body like that!\n\r",ch);
		return;
	}


	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	only_argument(arg,number);

	/* polax version of number validation */
	/* NOTE: i changed num_found to be initially TRUE */

	for(count=0; num_found && (count < 9) && (number[count] != '\0'); count++) {
		if((number[count] < '0') || (number[count] > '9')) {  /* leading zero is ok */
			num_found = FALSE;
		}
	}

	/* polax modification ends */

#if 0
	for(count=0; (!num_found) && (count<9); count++)
		if((number[count]>='1') && (number[count]<='9')) {
			num_found=TRUE;
		}
#endif

	if(!num_found) {
		send_to_char("Please include a number after the command.\n\r",ch);
		return;
	}
	else {
		number[count] = '\0';    /* forced the string to be proper length */
	}

	sscanf(number,"%ld",&hit_points);   /* long int conversion */

	if((hit_points <1) || (hit_points > 65535)) {
		/* bug fix? */
		send_to_char("Invalid number to canibalize.\n\r",ch);
		return;
	}

	mana_points = (hit_points * 2);

	/* Added by GAIA for CANIBALIZE of PRiNCES  */
	if(IS_PRINCE(ch)) {
		mana_points = (hit_points * 3);
	}
	if(mana_points <0) {
		send_to_char("You can't do that, You Knob!\n\r",ch);
	}

	if((int)ch->points.hit < (hit_points+5)) {
		send_to_char("You don't have enough physical stamina to canibalize.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)+mana_points) > (GET_MAX_MANA(ch))) {
		send_to_char("Your mind cannot handle that much extra energy.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_CANIBALIZE].learned < dice(1,101)) {
		send_to_char("You try to canibalize your stamina but the energy escapes before you can harness it.\n\r",ch);
		act("$n yelps in pain.",FALSE,ch,0,0,TO_ROOM);
		GET_HIT(ch) -= hit_points;
		alter_hit(ch,0);
		update_pos(ch);
		if(GET_POS(ch)==POSITION_DEAD) {
			die(ch,SKILL_CANIBALIZE, NULL);
		}
		LearnFromMistake(ch, SKILL_CANIBALIZE, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}

	send_to_char("You sucessfully convert your stamina to Mental power.\n\r",ch);
	act("$n briefly is surrounded by a red aura.",FALSE,ch,0,0,TO_ROOM);
	GET_HIT(ch) -= hit_points;
	alter_hit(ch,0);
	GET_MANA(ch) += mana_points;
	alter_mana(ch,0);

	update_pos(ch);
	if(GET_POS(ch)==POSITION_DEAD) {
		die(ch,SKILL_CANIBALIZE, NULL);
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

ACTION_FUNC(do_flame_shroud) {
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You couldn't even light a match!\n\r",ch);
			return;
		}


	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_FLAME_SHROUD].learned)) {
		send_to_char("You haven't studied your psycokinetics.\n\r",ch);
		return;
	}
	if(affected_by_spell(ch,SPELL_FIRESHIELD)) {
		send_to_char("You're already surrounded with flames.\n\r",ch);
		return;
	}
	if(GET_MANA(ch) < 40 && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You'll need more psycic energy to attempt this.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_FLAME_SHROUD].learned < dice(1,101)) {
		send_to_char("You failed and barely avoided buring yourself.\n\r",ch);
		act("$n pats at a small flame on $s arm.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch) -= 20;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_FLAME_SHROUD, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
	}

	send_to_char("You summon a flaming aura to deter attackers.\n\r",ch);
	act("$n summons a flaming aura that surrounds $mself.",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch) -= 40;
	alter_mana(ch,0);

	/* I do not use spell_fireshield because I want psi's shield to last longer */

	af.type       = SPELL_FIRESHIELD;
	af.duration   = GET_LEVEL(ch,PSI_LEVEL_IND)/5+10;
	af.modifier   = 0;
	af.location   = 0;
	af.bitvector  = AFF_FIRESHIELD;
	affect_to_char(ch,&af);

	WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

ACTION_FUNC(do_aura_sight) {
	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You better find a mage or cleric.\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_AURA_SIGHT].learned)) {
		send_to_char("You haven't leanred how to detect auras yet.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SPELL_DETECT_EVIL|SPELL_DETECT_MAGIC)) {
		send_to_char("You already have partial aura sight.\n\r",ch);
		return;
	}

	if(GET_MANA(ch) < 40)   {
		send_to_char("You lack the energy to convert auras to visible light.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_AURA_SIGHT].learned < dice(1,101))   {
		send_to_char("You try to detect the auras around you but you fail.\n\r",ch);
		act("$n blinks $s eyes then sighs.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch) -= 20;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_AURA_SIGHT, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}

	GET_MANA(ch) -= 40;
	alter_mana(ch,0);

	spell_detect_evil(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);
	spell_detect_magic(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);

	WAIT_STATE(ch, PULSE_VIOLENCE*3);
}



ACTION_FUNC(do_great_sight) {
	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You need a cleric or mage for better sight.\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_GREAT_SIGHT].learned)) {
		send_to_char("You haven't learned to enhance your sight yet.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SPELL_DETECT_INVISIBLE | SPELL_SENSE_LIFE | SPELL_TRUE_SIGHT)) {
		send_to_char("You already have partial great sight.\n\r",ch);
		return;
	}

	if(GET_MANA(ch) < 50)   {
		send_to_char("You haven't got the mental strength to try this.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_GREAT_SIGHT].learned < dice(1,101))   {
		send_to_char("You fail to enhance your sight.\n\r",ch);
		act("$n's eyes flash.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch) -= 25;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_GREAT_SIGHT, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}

	GET_MANA(ch) -= 50;
	alter_mana(ch,0);
	spell_detect_invisibility(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);
	spell_sense_life(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);
	spell_true_seeing(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);
	send_to_char("You succede in enhancing your vision.\n\rThere's so much you've missed.\n\r",ch);

	WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

ACTION_FUNC(do_blast) {
	struct char_data* victim;
	char name[240];
	int potency,level,dam = 0;
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	only_argument(arg,name);

	if(IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("Non hai il potere della mente!\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Eh ? Cos'e` ?\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		victim = ch->specials.fighting;
	}
	else {
		victim = get_char_room_vis(ch, name);
		if(!victim) {
			send_to_char("Chi e` che vuoi far esplodere esattamente?\n\r",ch);
			return;
		}
	}

	if(victim == ch) {
		send_to_char("Vorresti esplodere? Qualcuno potrebbe dispiacersi!\n\r",ch);
		return;
	}

	if(check_peaceful(ch, "C'e` troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(GetMaxLevel(victim) >= IMMORTALE || IS_IMMORTAL(victim)) {
		send_to_char("Egli ignora la tua battuta di spirito!\n\r",ch);
		return;
	}

	if(GET_MANA(ch) < 25 && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("La tua mente non ha abbastanza energia al momento.\n\r",ch);
		return;
	}

	if(number(1, 101) > ch->skills[ SKILL_PSIONIC_BLAST ].learned) {
		GET_MANA(ch) -= 12;
		alter_mana(ch,0);
		send_to_char("Provi a focalizzare la tua energia, ma ottieni solo "
					 "qualche scintilla!\n\r",ch);
		LearnFromMistake(ch, SKILL_PSIONIC_BLAST, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);   // blast
		return;
	}

	if(check_nomind(ch, "Non riesci a concentrarti abbastanza in questo "
					"posto",
					"$n cerca invano di concentrarsi")) {
		return;
	}

	if(!IS_IMMORTAL(victim)) {
		act("$n focalizza la mente su quella di $N.", TRUE, ch, 0, victim,
			TO_ROOM);
		act("$n strapazza la tua mente come un uovo.", TRUE, ch, 0, victim,
			TO_VICT);
		act("Colpisci la mente di $n con un'esplosione di enegia psionica!",
			FALSE, victim, 0, ch, TO_VICT);
		GET_MANA(ch) -= 25;
		alter_mana(ch,0);
		level = GET_LEVEL(ch, PSI_LEVEL_IND);
		potency = 0;
		if(level>0) {
			potency=1;
		}
		if(level > 1) {
			potency++;
		}
		if(level > 4) {
			potency++;
		}
		if(level > 7) {
			potency++;
		}
		if(level > 10) {
			potency++;
		}
		if(level > 20) {
			potency += 2;
		}
		if(level > 30) {
			potency += 2;
		}
		if(level > 40) {
			potency += 2;
		}
		if(level > 49) {
			potency += 2;
		}
		if(level > 50) {
			potency++ ;
		}
		if(GetMaxLevel(ch) > 57) {
			potency = 17;
		}

		if((potency < 14) && (number(1, 50) < GetMaxLevel(victim))) {
			potency-- ;
		}
		if((potency < 15) && (number(1, 52) < GetMaxLevel(victim))) {
			potency -= 2 ;
		}

		switch(potency) {
		case 0:
			dam=1;
			break;
		case 1:
			dam=number(1,4);
			break;
		case 2:
			dam=number(2,10);
			break;
		case 3:
			dam=number(3,12);
			break;
		case 4:
			dam=number(4,16);
			break;
		case 5:
			dam = 20;
			if(!IS_AFFECTED(ch, AFF_BLIND)) {
				af.type = SPELL_BLINDNESS;
				af.duration = 5;
				af.modifier = -4;
				af.location = APPLY_HITROLL;
				af.bitvector = AFF_BLIND;
				affect_to_char(victim, &af);
				af.location = APPLY_AC;
				af.modifier = 20;
				affect_to_char(victim, &af);
			}
			break;
		case 6:
			dam = 20;
			break;
		case 7:
			dam = 35;
			if(!IS_AFFECTED(ch, AFF_BLIND)) {
				af.type = SPELL_BLINDNESS;
				af.duration = 5;
				af.modifier = -4;
				af.location = APPLY_HITROLL;
				af.bitvector = AFF_BLIND;
				affect_to_char(victim, &af);
				af.location = APPLY_AC;
				af.modifier = 20;
				affect_to_char(victim, &af);
			}
			if(GET_POS(victim) > POSITION_STUNNED) {
				GET_POS(victim) = POSITION_STUNNED;
			}
			break;
		case 8:
			dam = 50;
			break;
		case 9:
			dam = 70;
			if(GET_POS(victim) > POSITION_STUNNED) {
				GET_POS(victim) = POSITION_STUNNED;
			}
			if(GET_HITROLL(victim) > -50) {
				af.type = SKILL_PSIONIC_BLAST;
				af.duration = 5;
				af.modifier = -5;
				af.location = APPLY_HITROLL;
				af.bitvector = 0;
				affect_join(victim, &af, FALSE, FALSE);
			}
			break;
		case 10:
			dam=75;
			break;
		case 11:
			dam=100;
			if(GET_POS(victim) > POSITION_STUNNED) {
				GET_POS(victim) = POSITION_STUNNED;
			}
			if(GET_HITROLL(victim) > -50) {
				af.type = SKILL_PSIONIC_BLAST;
				af.duration = 5;
				af.modifier = -10;
				af.location = APPLY_HITROLL;
				af.bitvector = 0;
				affect_join(victim, &af, FALSE, FALSE);
			}
			break;
		case 12:
			dam = 100;
			if(GET_HITROLL(victim) > -50) {
				af.type = SKILL_PSIONIC_BLAST;
				af.duration = 5;
				af.modifier = -5;
				af.location = APPLY_HITROLL;
				af.bitvector = 0;
				affect_join(victim, &af, FALSE, FALSE);
			}
			break;
		case 13:
			dam = 150;
			if(GET_POS(victim) > POSITION_STUNNED) {
				GET_POS(victim)=POSITION_STUNNED;
			}
			if((!IsImmune(victim, IMM_HOLD)) &&
					(!IS_AFFECTED(victim, AFF_PARALYSIS))) {
				af.type = SPELL_PARALYSIS;
				af.duration = level;
				af.modifier = 0;
				af.location = APPLY_NONE;
				af.bitvector = AFF_PARALYSIS;
				affect_join(victim, &af, FALSE, FALSE);
			}
			break;
		case 14:
		case 15:
		case 16:
		case 17:
			if(GET_POS(victim) > POSITION_STUNNED) {
				GET_POS(victim) = POSITION_STUNNED;
			}
			af.type = SPELL_PARALYSIS;
			af.duration = 100;
			af.modifier = 0;
			af.location = APPLY_NONE;
			af.bitvector = AFF_PARALYSIS;
			affect_join(victim, &af, FALSE, FALSE);
			send_to_char("Il tuo cervello e` stato tramutato in gelatina!\n\r",
						 victim);
			act("Hai tramutato il cervello di $N in gelatina!", FALSE, ch, 0,
				victim, TO_CHAR);
			break;
		}
	}
	if(damage(ch, victim, dam, SKILL_PSIONIC_BLAST, 5) != SubjectDead) {
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);    // blast
	}
}

ACTION_FUNC(do_hypnosis) {
	char target_name[140];
	struct char_data* victim;
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("Non ne sei capace.\n\r", ch);
			return;
		}


	if(affected_by_spell(ch, SPELL_FEEBLEMIND)) {
		send_to_char("Ehmm, cos'e` ?\n\r", ch);
		return;
	}

	if(!ch->skills[ SKILL_HYPNOSIS ].learned) {
		send_to_char("Non hai ancora imparato la tecnica giusta.\n\r", ch);
		return;
	}

	if(check_peaceful(ch, "C'e` troppa pace qui per essere violenti.\n\r")) {
		return;
	}


	only_argument(arg, target_name);
	victim = get_char_room_vis(ch, target_name);

	if(!victim) {
		send_to_char("Non c'e` nessuno con quel nome.\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Tu fai tutto quello che dirai.\n\r", ch);
		return;
	}

	if(IS_IMMORTAL(victim)) {
		send_to_char("Pah! Non penserai che questa sia una buona idea ?\n\r",
					 ch);
		return;
	}

	if((GET_MANA(ch) < 25)  && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("Hai bisogno di riposo.\n\r", ch);
		return;
	}

	if(circle_follow(victim, ch)) {
		send_to_char("Non puoi farti seguire da chi ti sta gia` seguendo.\n\r",
					 ch);
		return;
	}

	if(victim->tmpabilities.intel < 8) {
		send_to_char("Non sprecare il tuo tempo con questa creatura cosi` "
					 "stupida.\n",ch);
		return;
	}

	if(ch->skills[SKILL_HYPNOSIS].learned < number(1,101)) {
		send_to_char("Il tuo tentativo di ipnosi e` ridicolo.\n\r",ch);
		act("$n guarda negli occhi di $N, $n sembra addormentarsi!", FALSE, ch, 0,
			victim,TO_ROOM);
		GET_MANA(ch) -= 12;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_HYPNOSIS, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		return;
	}

	/* Steve's easy level check addition */
	if(GetMaxLevel(victim) > GetMaxLevel(ch)) {
		send_to_char("Probabilmente otterrai solo un gran mal di testa.\n\r", ch);
		GET_MANA(ch) -= 12;
		alter_mana(ch,0);
		return;
	}

	if(IS_IMMORTAL(victim)) {
		send_to_char("Non puoi ipnotizzare quella persona.\n\r", ch);
		return;
	}

	if(saves_spell(victim, SAVING_SPELL) ||
			(IS_AFFECTED(victim,AFF_CHARM) && !IS_AFFECTED(ch,AFF_CHARM))) {
		send_to_char("Non riesci ad ipnotizzare quella persona.\n\r", ch);
		GET_MANA(ch) -= 25;
		alter_mana(ch,0);
		FailCharm(victim, ch);
		return;
	}

	GET_MANA(ch) -= 25;
	alter_mana(ch,0);

	act("$n ipnotizza $N!", TRUE, ch, 0, victim, TO_ROOM);
	act("Tu hai ipnotizzato $N!", TRUE, ch, 0, victim, TO_CHAR);
	if(IS_PC(victim)) {
		act("$n ti ha ipnotizzato!", TRUE, ch, 0, victim, TO_VICT);
	}

	add_follower(victim, ch);
	if(IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
		REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
	}
	if(!IS_SET(victim->specials.act, ACT_SENTINEL)) {
		SET_BIT(victim->specials.act, ACT_SENTINEL);
	}

	af.type = SPELL_CHARM_MONSTER;
	af.duration = 36;
	af.modifier = 0;
	af.location = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(victim, &af);

	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}






ACTION_FUNC(do_scry) {
	char target_name[140];
	struct char_data* target;
	int location,old_location;
	struct room_data* rp;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI) && !IS_IMMORTAL(ch)) {
			send_to_char("Your mind is not developed enough to do this\n\r",ch);
			return;
		}


	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!ch->skills[SKILL_SCRY].learned) {
		send_to_char("You have not trained your mind to do this\n\r",ch);
		return;
	}

	only_argument(arg,target_name);
	if(!(target=get_char_vis_world(ch,target_name,NULL))) {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}

	old_location = ch->in_room;
	location = target->in_room;
	rp = real_roomp(location);

	if(IS_IMMORTAL(target) ||  !rp ||
			IS_SET(rp->room_flags,  PRIVATE | NO_MAGIC)) {
		send_to_char("Your mind is not yet strong enough.\n\r", ch);
		return;
	}

	if((GET_MANA(ch) < 20) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You have a headache. Better rest before you try this again.\n\r",ch);
		return;
	}
	else if(dice(1,101) > ch->skills[SKILL_SCRY].learned) {
		send_to_char("You cannot open a window at this time.\n\r", ch);
		GET_MANA(ch)-=10;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_SCRY, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
	}
	else {
		GET_MANA(ch)-=20;
		alter_mana(ch,0);
		send_to_char("You close your eyes and envision your target.\n\r",ch);
		char_from_room(ch);
		char_to_room(ch, location);
		do_look(ch, "",15);
		char_from_room(ch);
		char_to_room(ch, old_location);
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
	}
}




ACTION_FUNC(do_invisibililty) {
	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("Get a mage if you want to go Invisible!\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_INVIS].learned)) {
		send_to_char("You are unable to bend light.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)<10) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You don't have enough mental power to hide yourself.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SPELL_INVISIBLE))   {
		send_to_char("You're already invisible.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_INVIS].learned<number(1,101))   {
		send_to_char("You cannot seem to bend light right now.\n\r",ch);
		act("$n fades from view briefly.",FALSE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=5;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_INVIS, 0, 95);
		WAIT_STATE(ch,PULSE_VIOLENCE*2);
		return;
	}

	GET_MANA(ch) -=10;
	alter_mana(ch,0);
	spell_invisibility(GET_LEVEL(ch,PSI_LEVEL_IND),ch,ch,0);
	WAIT_STATE(ch,PULSE_VIOLENCE*2);
}




ACTION_FUNC(do_adrenalize) {
	char target_name[140];
	struct char_data* target;
	struct affected_type af;
	char strength;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You're no psionicist!\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_ADRENALIZE].learned)) {
		send_to_char("You don't know how to energize people.\n\r",ch);
		return;
	}

	only_argument(arg,target_name);
	if(!(target=get_char_room_vis(ch,target_name))) {
		send_to_char("You can't seem to find that person anywhere.\n\r",ch);
		return;
	}

	if(GET_MANA(ch) < 15 && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You don't have the mental power to do this.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_ADRENALIZE].learned < dice(1,101)) {
		send_to_char("You've falied your attempt.\n\r",ch);
		act("$n touches $N's head lightly, then sighs.",FALSE,ch,0,target,TO_ROOM);
		GET_MANA(ch) -= 7;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_ADRENALIZE, 0, 95);
		WAIT_STATE(ch,PULSE_VIOLENCE*2);
		return;
	}

	if(affected_by_spell(target,SKILL_ADRENALIZE)) {
		send_to_char("This person was already adrenalized!\n\r",ch);
		GET_MANA(ch) -= 15;
		alter_mana(ch,0);
		return;
	}

	strength = 1 + (GET_LEVEL(ch,PSI_LEVEL_IND)/10);
	if(strength>4) {
		strength = 4;
	}

	af.type       = SKILL_ADRENALIZE;
	af.location   = APPLY_HITROLL;
	af.modifier   = -strength;
	af.duration   = 5;
	af.bitvector  = 0;
	affect_to_char(target,&af);

	af.location   = APPLY_DAMROLL;
	af.modifier   = strength;
	affect_to_char(target,&af);

	af.location   = APPLY_AC;
	af.modifier   = 20;
	affect_to_char(target,&af);

	GET_MANA(ch) -= 15;
	alter_mana(ch,0);
	act("You excite the chemicals in $N's body!",TRUE,ch,0,target,TO_CHAR);
	act("$n touches $N lightly on the forehead.",TRUE,ch,0,target,TO_NOTVICT);
	act("$N suddenly gets a wild look in $m eyes!",TRUE,ch,0,target,TO_NOTVICT);
	act("$n touches you on the forehead lightly, you feel energy ulimited!",TRUE,ch,0,target,TO_VICT);
}

ACTION_FUNC(do_meditate) {
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You can't stand sitting down and waiting like this.\n\r",ch);
			return;
		}


	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_MEDITATE].learned)) {
		send_to_char("You haven't yet learned to clear your mind.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_MEDITATE].learned < dice(1,101)) {
		send_to_char("You can't clear your mind at this time.\n\r",ch);
		LearnFromMistake(ch, SKILL_MEDITATE, 0, 95);
		return;
	}

	if(ch->specials.conditions[FULL] == 0     /*hungry or*/
			or ch->specials.conditions[THIRST] == 0  /*thirsty or*/
			or ch->specials.conditions[DRUNK] > 0) {   /*alcohol in blood*/
		send_to_char("Your body has certain needs that have to be met before you can meditate.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SKILL_MEDITATE)) {
		send_to_char("Your mind is already prepared to meditate, so rest and become one with nature.\n\r", ch);
		return;
	}

	ch->specials.position=POSITION_RESTING; /* is meditating */
	send_to_char("You sit down and start resting and clear your mind of all thoughts.\n\r",ch);
	act("$n sits down and begins humming,'Oooommmm... Ooooommmm.'",TRUE,ch,0,0,TO_ROOM);
	af.type = SKILL_MEDITATE;
	af.location = 0;
	af.modifier = 0;
	af.duration = 2;
	af.bitvector = 0;
	affect_to_char(ch,&af);
}


int IS_FOLLOWING(struct char_data* tch, struct char_data* person) {
	if(person->master) {
		person = person->master;
	}
	if(tch->master) {
		tch= tch->master;
	}
	return (person == tch && IS_AFFECTED(person,AFF_GROUP) && IS_AFFECTED(tch,AFF_GROUP));
}

ACTION_FUNC(do_heroic_rescue) {
	struct char_data* dude, *enemy;
	int grp = 0, first = 1, rescue, rescued = 0,torescue = 0;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))
		if(!HasClass(ch, CLASS_PALADIN)) {
			send_to_char("Pensi davvero di essere un Paladino?\n\r", ch);
			return;
		}

	if(check_peaceful(ch, "E chi potrebbe mai aver bisogno di aiuto in questa "
					  "pace?\n\r")) {
		return;
	}

	for(dude = real_roomp(ch->in_room)->people; dude;
			dude = dude->next_in_room) {
		if(dude->specials.fighting && dude != ch && IS_PC(dude) &&
				dude->attackers > 0) {
			torescue = 1;
			if(is_same_group(dude, ch)) {
				grp = 1;
			}
		}
	}

	if(!torescue) {
		send_to_char("Non c'e' nessuno da aiutare !\n\r", ch);
		return;
	}

	if(ch->skills[ SKILL_HEROIC_RESCUE ].learned < number(1, 101)) {
		send_to_char("Cerchi, invano, di portati sul fronte della battaglia.\n\r",
					 ch);
		LearnFromMistake(ch, SKILL_HEROIC_RESCUE, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE);   // rescue
		return;
	}

	for(dude = real_roomp(ch->in_room)->people; dude;
			dude = dude->next_in_room) {
		if(dude->specials.fighting && dude->specials.fighting != ch &&
				ch->specials.fighting != dude && dude != ch && IS_PC(dude) &&
				dude->attackers > 0) {
			rescue = 0;
			if(grp) {
				if(is_same_group(dude, ch)) {
					rescue = 1;
				}
			}
			else {
				rescue = 1;
			}
			if(rescue) {
				if(first) {
					act("$n si lancia con furia mistica sul fronte della battaglia!",
						FALSE, ch, 0, 0, TO_ROOM);
					send_to_char("$c0015'GLI DEI SONO CON ME!!' Urli saltando sul "
								 "fronte della battaglia accecato da furia "
								 "mistica!\n\r", ch);
					first = 0;
				}
				act("$n interviene in tuo soccorso, pervaso da furia mistica! "
					"Sei frastornato!", TRUE, ch, 0, dude, TO_VICT);
				act("Intervieni in soccorso di $N!", TRUE, ch, 0, dude, TO_CHAR);
				act("$n interviene in soccorso di $N!", TRUE, ch, 0, dude, TO_NOTVICT);
				rescued = 1;

				stop_fighting(dude);
				WAIT_STATE(dude, 2 * PULSE_VIOLENCE);   // rescue
				if(GET_ALIGNMENT(dude) >= 350) {
					GET_ALIGNMENT(ch) += 10;
				}
				if(GET_ALIGNMENT(dude) >= 950) {
					GET_ALIGNMENT(ch) += 10;
				}


				for(enemy = real_roomp(ch->in_room)->people; enemy;
						enemy = enemy->next_in_room) {
					if(enemy->specials.fighting == dude) {
						stop_fighting(enemy);
						set_fighting(enemy, ch);

						if(GET_ALIGNMENT(dude) > -350 &&
								GET_ALIGNMENT(enemy) < -350)
							GET_ALIGNMENT(ch) += MIN((GET_ALIGNMENT(dude) -
													  GET_ALIGNMENT(enemy)) / 300,
													 3);
					}
				}
			}
		}
	}

	if(!rescued) {
		send_to_char("Sembra che non ci sia nessuno da aiutare!\n\r", ch);
	}

}



ACTION_FUNC(do_blessing) {
	int rating, factor, level;
	struct char_data* test, *dude;
	struct affected_type af;
	char dude_name[140];

	if(!ch->skills) {
		return;
	}

	only_argument(arg,dude_name);

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PALADIN)) {
			send_to_char("I bet you think you are a paladin, don't you?\n\r",ch);
			return;
		}

	if(GET_MANA(ch)<GET_LEVEL(ch,PALADIN_LEVEL_IND)*2) {
		send_to_char("You haven't the spiritual resources to do that now.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SKILL_BLESSING)) {
		send_to_char("You can only request a blessing from your diety once every 3 days.\n\r",ch);
		return;
	}

	if(number(1,101)>ch->skills[SKILL_BLESSING].learned) {
		send_to_char("You fail in the bestow your gods blessing.\n\r",ch);
		GET_MANA(ch) -= GET_LEVEL(ch,PALADIN_LEVEL_IND);
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_BLESSING, 0, 95);
		return;
	}

	if(!(dude=get_char_room_vis(ch,dude_name))) {
		send_to_char("WHO do you wish to bless?\n\r",ch);
		return;
	}

	GET_MANA(ch) -= GET_LEVEL(ch,PALADIN_LEVEL_IND)*2;
	alter_mana(ch,0);
	factor=0;
	if(ch==dude) {
		factor++;
	}
	if(dude->specials.alignment > 350) {
		factor++;
	}
	if(dude->specials.alignment == 1000) {
		factor++;
	}
	level = GET_LEVEL(ch,PALADIN_LEVEL_IND);
	rating = (int)((level)*(GET_ALIGNMENT(ch))/1000)+factor;
	factor=0;
	for(test=real_roomp(ch->in_room)->people; test; test=test->next) {
		if(test!=ch) {
			if(ch->master) {
				if(circle_follow(ch->master,test)) {
					factor++;
				}
			}
			else if(circle_follow(ch,test)) {
				factor++;
			}
		}

	}
	rating += MIN(factor,3);
	if(rating<0) {
		send_to_char("You are so despised by your god that he punishes you!\n\r",ch);
		spell_blindness(level,ch,ch,0);
		spell_paralyze(level,ch,ch,0);
		return;
	}
	if(rating==0) {
		send_to_char("There's no one in your group to bless",ch);
		return;
	}
	if(!(affected_by_spell(dude,SPELL_BLESS))) {
		spell_bless(level,ch,dude,0);
	}
	if(rating>1) {
		if(!(affected_by_spell(dude,SPELL_ARMOR))) {
			spell_armor(level,ch,dude,0);
		}
	}

	if(rating>4)
		if(!(affected_by_spell(dude,SPELL_STRENGTH))) {
			spell_strength(level,ch,dude,0);
		}
	if(rating>6) {
		spell_second_wind(level,ch,dude,0);
	}
	if(rating>9)
		if(!(affected_by_spell(dude,SPELL_SENSE_LIFE))) {
			spell_sense_life(level,ch,dude,0);
		}
	if(rating>14)
		if(!(affected_by_spell(dude,SPELL_TRUE_SIGHT))) {
			spell_true_seeing(level,ch,dude,0);
		}
	if(rating>19) {
		spell_cure_critic(level,ch,dude,0);
	}
	if(rating>24)
		if(!(affected_by_spell(dude,SPELL_SANCTUARY))) {
			spell_sanctuary(level,ch,dude,0);
		}
	if(rating>29) {
		spell_heal(level,ch,dude,0);
	}
	if(rating>34) {
		spell_remove_poison(level,ch,dude,0);
		spell_remove_paralysis(level,ch,dude,0);
	}
	if(rating>39) {
		spell_heal(level,ch,dude,0);
	}
	if(rating>44) {
		if(dude->specials.conditions[FULL] != -1) {
			dude->specials.conditions[FULL] = 24;
		}
		if(dude->specials.conditions[THIRST] != -1) {
			dude->specials.conditions[THIRST] = 24;
		}
	}
	if(rating>54) {
		spell_heal(level,ch,dude,0);
		send_to_char("An awesome feeling of holy power overcomes you!\n\r",dude);
	}

	act("$n asks $s deity to bless $N!",TRUE,ch,0,dude,TO_NOTVICT);
	act("You pray for a blessing on $N!",TRUE,ch,0,dude,TO_CHAR);
	act("$n's deity blesses you!",TRUE,ch,0,dude,TO_VICT);

	af.type = SKILL_BLESSING;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	af.duration  = 24*3;    /* once every three days */
	affect_to_char(ch, &af);
	WAIT_STATE(ch,PULSE_VIOLENCE*2);
}



ACTION_FUNC(do_lay_on_hands) {
	struct char_data* victim;
	struct affected_type af;
	int wounds, healing;
	char victim_name[240];

	if(!ch->skills) {
		return;
	}

	only_argument(arg, victim_name);

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PALADIN)) {
			send_to_char("You are not a holy warrior!\n\r",ch);
			return;
		}

	if(!(victim=get_char_room_vis(ch,victim_name)))     {
		send_to_char("Your hands cannot reach that person\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SKILL_LAY_ON_HANDS)) {
		send_to_char("You have already healed once today.\n\r",ch);
		return;
	}

	wounds=GET_MAX_HIT(victim)-GET_HIT(victim);
	if(!wounds)     {
		send_to_char("Don't try to heal what ain't hurt!\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_LAY_ON_HANDS].learned<number(1,101))     {
		send_to_char("You cannot seem to call on your deity right now.\n\r",ch);
		LearnFromMistake(ch, SKILL_LAY_ON_HANDS, 0, 95);
		WAIT_STATE(ch,PULSE_VIOLENCE);
		return;
	}

	act("$n lays hands on $N.",FALSE,ch,0,victim, TO_NOTVICT);
	act("You lay hands on $N.",FALSE,ch,0,victim,TO_CHAR);
	act("$n lays hands on you.",FALSE,ch,0,victim,TO_VICT);

	if(GET_ALIGNMENT(victim)<0)   {
		act("You are too evil to benefit from this treatment.",FALSE,ch,0,victim,TO_VICT);
		act("$n is too evil to benefit from this treatment.",FALSE,victim,0,ch,TO_ROOM);
		return;
	}

	if(GET_ALIGNMENT(victim)<350) { /* should never be since they get converted */
		healing = GET_LEVEL(ch,PALADIN_LEVEL_IND);    /* after 349 */
	}
	else {
		healing = GET_LEVEL(ch,PALADIN_LEVEL_IND)*2;
	}

	if(healing>wounds) {
		GET_HIT(victim) = GET_MAX_HIT(victim);
	}
	else {
		GET_HIT(victim) +=healing;
	}
	alter_hit(victim,0);

	af.type = SKILL_LAY_ON_HANDS;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	af.duration  = 24;
	affect_to_char(ch, &af);
	WAIT_STATE(ch,PULSE_VIOLENCE);
}


ACTION_FUNC(do_holy_warcry) {
	char name[140];
	int dam, dif,level;
	struct char_data* dude;

	if(!ch->skills) {
		return;
	}

	only_argument(arg,name);

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PALADIN) && !IS_PRINCE(ch)) {
			send_to_char("Your feeble attempt at a war cry makes your victim laugh at you.\n\r",ch);
			return;
		}

	if(GET_ALIGNMENT(ch) < 350)  {
		send_to_char("You're too ashamed of your behavior to warcry.\n\r",ch);
		return;
	}

	if(check_peaceful(ch,"You warcry is completely silenced by the tranquility of this room.\n\r")) {
		return;
	}

	if(ch->specials.fighting) {
		dude = ch->specials.fighting;
	}
	else if(!(dude=get_char_room_vis(ch,name)))    {
		send_to_char("You bellow at the top of your lungs, to bad your victim wasn't here to hear it.\n\r",ch);
		return;
	}

	if(IS_IMMORTAL(dude))  {
		send_to_char("The gods are not impressed by people shouting at them.\n",   ch);
		return;
	}

	if(ch->skills[SKILL_HOLY_WARCRY].learned<number(1,101))  {
		send_to_char("Your mighty warcry emerges from your throat as a tiny squeak.\n\r",ch);
		LearnFromMistake(ch, SKILL_HOLY_WARCRY, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
		set_fighting(dude,ch);  /* make'em fight even if he fails */
	}
	else {
		if(IS_PC(dude))    {
			act("$n surprises you with a painful warcry!",FALSE,ch,0,dude,TO_VICT);
		}

		dif=(level=GET_LEVEL(ch,PALADIN_LEVEL_IND)-GetMaxLevel(dude));
		if(IS_PRINCE(ch)) {
			dif=(level=GetMaxLevel(ch)-GetMaxLevel(dude));
		}

		if(dif>19) {
			spell_paralyze(0,ch,dude,0);
			dam = (int)(level*2.5);
		}
		else if(dif>14) {
			dam = (int)(level*2.5);
		}
		else if(dif>10) {
			dam = (int)(level*2);
		}
		else if(dif>6) {
			dam = (int)(level*1.5);
		}
		else if(dif>-6) {
			dam = (int)(level);
		}
		else if(dif>-11) {
			dam = (int)(level*.5);
		}
		else {
			dam = 0;
		}
		/* Added by GAIA for WARCRY of PRiNCES  */
		if((dif>1) && IS_PRINCE(ch)) {
			spell_fear(GetMaxLevel(ch),ch,dude,0);
			dam = dam + (int)(level*2.5);
		}
		if(saves_spell(dude, SAVING_SPELL)) {
			dam /= 2;
		}
		act("You are attacked by $n who shouts a heroic warcry!", TRUE, ch, 0,
			dude,TO_VICT);
		act("$n screams a warcry at $N with a tremendous fury!", TRUE, ch, 0,
			dude,TO_ROOM);
		act("You fly into battle $N with a holy warcry!", TRUE, ch, 0, dude,
			TO_CHAR);
		if(damage(ch, dude, dam, SKILL_HOLY_WARCRY, 5) != VictimDead) {
			if(!ch->specials.fighting) {
				set_fighting(ch, dude);
			}
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 3);
	}
}



ACTION_FUNC(do_psi_shield) {
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You do not have the mental power to bring forth a shield!\n\r",ch);
			return;
		}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_PSI_SHIELD].learned)) {
		send_to_char("You are unable to use this skill.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)<10) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You don't have enough mental power to protect yourself.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SKILL_PSI_SHIELD)) {
		send_to_char("You're already protected.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_PSI_SHIELD].learned<number(1,101)) {
		send_to_char("You failed to bring forth the protective shield.\n\r",ch);
		GET_MANA(ch)-=5;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_PSI_SHIELD, 0, 95);
		WAIT_STATE(ch,PULSE_VIOLENCE*2);
		return;
	}

	GET_MANA(ch) -=10;
	alter_mana(ch,0);
	act("$n summons a protective shield about $s body!",FALSE,ch,0,0,TO_ROOM);
	act("You errect a protective shield about your body.",FALSE,ch,0,0,TO_CHAR);
	af.type       = SKILL_PSI_SHIELD;
	af.location   = APPLY_AC;
	af.modifier   = ((int)GetMaxLevel(ch)/10)*-10;
	af.duration   = GetMaxLevel(ch);
	af.bitvector  = 0;
	affect_to_char(ch,&af);
	WAIT_STATE(ch,PULSE_VIOLENCE*2);

}

ACTION_FUNC(do_esp) {
	struct affected_type af;

	if(!ch->skills) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch,CLASS_PSI)) {
			send_to_char("You do not have the mental power to do this!\n\r",ch);
			return;
		}


	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SKILL_ESP].learned)) {
		send_to_char("You are unable to use this skill.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)<10) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You don't have enough mental power to do that.\n\r",ch);
		return;
	}

	if(affected_by_spell(ch,SKILL_ESP)) {
		send_to_char("You're already listening to others thoughts.\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_ESP].learned<number(1,101)) {
		send_to_char("You failed open you mind to read others thoughts.\n\r",ch);
		GET_MANA(ch)-=5;
		alter_mana(ch,0);
		LearnFromMistake(ch, SKILL_ESP, 0, 95);
		return;
	}

	GET_MANA(ch) -=10;
	alter_mana(ch,0);
	act("You open your mind to read others thoughts.",FALSE,ch,0,0,TO_CHAR);
	af.type       = SKILL_ESP;
	af.location   = APPLY_NONE;
	af.modifier   = 0;
	af.duration   = (int)GetMaxLevel(ch)/2;
	af.bitvector  = 0;
	affect_to_char(ch,&af);
}

ACTION_FUNC(do_sending) {
	struct char_data* target;
	int skill_check=0;
	char target_name[140],buf[1024], message[MAX_INPUT_LENGTH+20];

	if(!ch->skills) {
		return;
	}

	if(affected_by_spell(ch,SPELL_FEEBLEMIND)) {
		send_to_char("Der, what is that ?\n\r",ch);
		return;
	}

	if(!(ch->skills[SPELL_SENDING].learned) &&
			!(ch->skills[SPELL_MESSENGER].learned)) {
		send_to_char("You are unable to use this skill.\n\r",ch);
		return;
	}

	if((GET_MANA(ch)<5) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("You don't have the power to do that.\n\r",ch);
		return;
	}

	if(ch->skills[SPELL_SENDING].learned >
			ch->skills[SPELL_MESSENGER].learned) {
		skill_check=ch->skills[SPELL_SENDING].learned;
	}
	else {
		skill_check=ch->skills[SPELL_MESSENGER].learned;
	}

	if(skill_check<number(1,101)) {
		send_to_char("You fumble and screw up the spell.\n\r",ch);
		if(GetMaxLevel(ch)< IMMORTALE) {
			GET_MANA(ch)-=3;
			alter_mana(ch,0);
		}
		if(ch->skills[SPELL_SENDING].learned >
				ch->skills[SPELL_MESSENGER].learned) {
			LearnFromMistake(ch, SPELL_SENDING, 0, 95);
		}
		else {
			LearnFromMistake(ch, SPELL_MESSENGER, 0, 95);
		}
		return;
	}

	if(GetMaxLevel(ch)< IMMORTALE) {
		GET_MANA(ch) -=5;
		alter_mana(ch,0);
	}
	half_chop(arg,target_name,message,sizeof target_name -1,sizeof message -1);
	if(!(target=get_char_vis_world(ch,target_name,NULL))) {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}

	if(!IS_PC(target)) {
		send_to_char("You can't sense that person anywhere.\n\r",ch);
		return;
	}

	if(check_soundproof(target)) {
		send_to_char("Non riesce a sentire nemmeno la sua voce, li` dentro.\n\r",
					 ch);
		return;
	}


	if(IS_SET(target->specials.act,PLR_NOTELL)) {
		send_to_char("They are ignoring messages at this time.\n\r",ch);
		return;
	}

	if(IS_LINKDEAD(target)) {
		send_to_char("Non puo` ricevere il tuo messaggio. "
					 "Ha perso il senso della realta`.\n\r", ch);
		return;
	}

	sprintf(buf, "$c0013[$c0015$n$c0013] ti manda il messaggio '%s'", message);
	act(buf, TRUE, ch, 0, target, TO_VICT);
	sprintf(buf, "$c0013Mandi a $N%s il messaggio '%s'",
			(IS_AFFECTED2(target,AFF2_AFK)?" (che e` AFK)":""), message);
	act(buf, TRUE, ch, 0, target, TO_CHAR);
}

ACTION_FUNC(do_brew) {
	if(!ch->skills) {
		return;
	}

	send_to_char("Not implemented yet.\n\r",ch);
}


/* ACIDUS 2003, skill miner */

#define MAX_MINIERE 9  /* numero massimo di righe nella tabella delle miniere */
int in_miniera(struct char_data* ch) {
	struct range_vnum_type {
		int da_vnum;
		int a_vnum;
	};

	struct range_vnum_type lista_miniere[MAX_MINIERE]= {
		{4010,4073}, /* moria, le cave al primo livello - Requiem 2018 */
		{4100,4125}, /* moria, le cave al terzo livello - Requiem 2018 */
		{4435,4445},
		{4447,4447},
		{4449,4456},
		{4459,4459},
		{6542,6553}, /* luoghi profondi, gorrdar - Requiem 2018 */
		{16051,16059}, /* mineshaft sulle high mountains - Requiem 2018 */
		{37375,37382} /* miniere di mordor - Requiem 2018 */
	};

	int X=MAX_MINIERE, found=FALSE;

	X--;  //la numerazione \E8 0...MAX-1
	while((!found) && (X >= 0)) {
		if(
			((lista_miniere[X].da_vnum) <= (ch->in_room))
			&& ((lista_miniere[X].a_vnum) >= (ch->in_room))
		) {
			found = TRUE;
		}
		else {
			X--;
		}
	}

	return(found);
}

void do_miner(struct char_data* ch) {
	int r_num,percent=0,blocco;
	struct obj_data* obj;
	struct char_data* pMob;

	if(!ch->skills) {
		return;
	}

	if(!(canDig(ch))) {
		send_to_char("Forse usando l'attrezzo adatto...\n\r",ch);
		return;
	}

	if(ch->skills[SKILL_MINER].learned <=0) {
		send_to_char("Non sei addestrato a scavare.\n\r",ch);
		return;
	}

	if(!in_miniera(ch)) {
		send_to_char("Qui non puoi scavare.\n\r",ch);
		return;
	}

	switch(GET_RACE(ch)) {
	case RACE_DWARF:
	case RACE_DARK_DWARF:
		break;
	default:
		send_to_char("Lascia stare: scavare in una miniera non fa per te!\n\r",ch);
		return;
		break;
	}

	if(GetMaxLevel(ch)<20) {
		send_to_char("Sei ancora troppo piccolo per scavare in miniera!\n\r",ch);
		return;
	}

	percent = number(1,100); /* 101% si rompe il piccone */

	if(ch->skills && ch->skills[SKILL_MINER].learned &&
			GET_POS(ch) > POSITION_SITTING) {
		if(GET_MOVE(ch) < 10) {
			send_to_char("Sei troppo stanco, e' meglio se ti riposi un po'.\n\r",ch);
			return;
		}

		GET_MOVE(ch) -= 10;
		alter_move(ch,0);

		if(percent > ch->skills[SKILL_MINER].learned) {
			act("Fai una mossa maldestra e non riesci a scavare.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n fa una mossa maldestra e non riesce a scavare.",
				TRUE, ch, 0, 0, TO_ROOM);
			LearnFromMistake(ch, SKILL_MINER, 0, 95);

			//3% di probabilit\E0 che si rompa l'attrezzo
			if(percent >= 98) {
				struct obj_data* pObj = ch->equipment[ HOLD ];
				if(pObj && IS_SET(pObj->obj_flags.extra_flags, ITEM_DIG)
						&& !IS_SET(pObj->obj_flags.extra_flags, ITEM_IMMUNE)
				  ) {
					MakeScrap(ch, 0, pObj);
				}
				else if((pObj = ch->equipment[ WIELD ])
						&&  IS_SET(pObj->obj_flags.extra_flags, ITEM_DIG)
						&&  !IS_SET(pObj->obj_flags.extra_flags, ITEM_IMMUNE)
					   ) {
					MakeScrap(ch, 0, pObj);
				}
			}
		}
		else {
			//testo il livello di scavabilit\E0
			if(real_roomp(ch->in_room)->dig >=10) {
				send_to_char("Qui non si riesce piu' a scavare, il filone sembra esaurito!\n\r",ch);
				return;
			}
			else {
				(real_roomp(ch->in_room)->dig) = (real_roomp(ch->in_room)->dig) +1;
			}


			act("Ti dai da fare e scavando a fondo trovi qualcosa.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n si da da fare e scavando a fondo trova qualcosa.",
				TRUE, ch, 0, 0, TO_ROOM);

			//Testo se esce un metallo (e semmai quale), una pietra preziosa o un mob
			percent = number(1,100);
			if(percent <= 30) {
				r_num = real_object(19500);    //rame
			}
			else if(percent > 30 && percent <= 55) {
				r_num = real_object(19501);    //piombo
			}
			else if(percent > 55 && percent <= 70) {
				r_num = real_object(19502);    //ferro
			}
			else if(percent > 70 && percent <= 80) {
				r_num = real_object(19503);    //carbone
			}
			else if(percent > 80 && percent <= 90) {
				r_num = real_object(19504);    //stagno
			}
			else if(percent > 90 && percent <= 95) {
				r_num = real_object(19505);    //oro
			}
			else if(percent > 95 && percent <= 96) {
				r_num = real_object(19506);    //platino
			}
			else if(percent > 96 && percent <= 97) {
				r_num = real_object(19507);    //mithril
			}
			else if(percent > 97 && percent <= 98) {
				r_num = real_object(19508);    //adamantite
			}

			//in questo caso esce il mob
			if(percent == 99) {
				act("ACCIDENTI!!! Qualcosa si muove nel punto in cui hai scavato!!",
					TRUE, ch, 0, 0, TO_CHAR);
				act("ACCIDENTI!!! Qualcosa si muove nel punto in cui $n ha scavato!!",
					TRUE, ch, 0, 0, TO_ROOM);
				pMob = read_mobile(real_mobile(19500), REAL);
				if(pMob) {
					char_to_room(pMob, ch->in_room);
				}
				return;
			}

			//in questo caso cerco tra le tabelle di pietre preziose
			if(percent == 100) {
				percent = number(1,100);
				if(percent <= 50) {
					blocco = 1;
				}
				else if(percent > 50 && percent <= 70) {
					blocco = 2;
				}
				else if(percent > 70 && percent <= 85) {
					blocco = 3;
				}
				else if(percent > 85 && percent <= 95) {
					blocco = 4;
				}
				else if(percent > 96) {
					blocco = 5;
				}

				percent = number(1,100);
				switch(blocco) {
				case 1:
					if(percent <= 10) {
						r_num = real_object(19509);    //quarzo comune
					}
					else if(percent > 10 && percent <= 20) {
						r_num = real_object(19510);    //ossidiana
					}
					else if(percent > 20 && percent <= 30) {
						r_num = real_object(19511);    //opale
					}
					else if(percent > 30 && percent <= 40) {
						r_num = real_object(19512);    //turchese
					}
					else if(percent > 40 && percent <= 50) {
						r_num = real_object(19513);    //zircone
					}
					else if(percent > 50 && percent <= 60) {
						r_num = real_object(19514);    //lapislazzuli
					}
					else if(percent > 60 && percent <= 70) {
						r_num = real_object(19515);    //onice
					}
					else if(percent > 70 && percent <= 80) {
						r_num = real_object(19516);    //malachite
					}
					else if(percent > 80 && percent <= 90) {
						r_num = real_object(19517);    //ematite
					}
					else if(percent > 90) {
						r_num = real_object(19518);    //giada
					}
					break;
				case 2:
					if(percent <= 20) {
						r_num = real_object(19519);    //resina fossile
					}
					else if(percent > 20 && percent <= 40) {
						r_num = real_object(19520);    //crisoberillo
					}
					else if(percent > 40 && percent <= 60) {
						r_num = real_object(19521);    //spinello blu
					}
					else if(percent > 60 && percent <= 80) {
						r_num = real_object(19522);    //tormalina
					}
					else if(percent > 80) {
						r_num = real_object(19523);    //quarzo comune, clone
					}
					break;
				case 3:
					if(percent <= 20) {
						r_num = real_object(19524);    //quarzo rosa
					}
					else if(percent > 20 && percent <= 40) {
						r_num = real_object(19525);    //agata
					}
					else if(percent > 40 && percent <= 60) {
						r_num = real_object(19526);    //acquamarina
					}
					else if(percent > 60 && percent <= 80) {
						r_num = real_object(19527);    //berillo
					}
					else if(percent > 80) {
						r_num = real_object(19528);    //topazio
					}
					break;
				case 4:
					if(percent <= 20) {
						r_num = real_object(19529);    //spinello nero
					}
					else if(percent > 20 && percent <= 40) {
						r_num = real_object(19530);    //fluorite
					}
					else if(percent > 40 && percent <= 60) {
						r_num = real_object(19531);    //ametista
					}
					else if(percent > 60 && percent <= 80) {
						r_num = real_object(19532);    //corindone
					}
					else if(percent > 80) {
						r_num = real_object(19533);    //granato
					}
					break;
				case 5:
					if(percent <= 20) {
						r_num = real_object(19534);    //zaffiro
					}
					else if(percent > 20 && percent <= 40) {
						r_num = real_object(19535);    //smeraldo
					}
					else if(percent > 40 && percent <= 60) {
						r_num = real_object(19536);    //rubino
					}
					else if(percent > 60 && percent <= 80) {
						r_num = real_object(19537);    //diamante
					}
					else if(percent > 80) {
						act("Che gli DEI ti salvino!! Hai risvegliato un BALROG!!",
							TRUE, ch, 0, 0, TO_CHAR);
						act("Che gli DEI ti salvino!! $n ha risvegliato un BALROG!!",
							TRUE, ch, 0, 0, TO_ROOM);
						pMob = read_mobile(real_mobile(19501), REAL);
						if(pMob) {
							char_to_room(pMob, ch->in_room);
						}
						return;
					}
					break;
				default:
					break;
				}

			}

			if(r_num >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj,ch);
			}

		}

		/*setto lag per razze*/
		switch(GET_RACE(ch)) {
		case RACE_GIANT_STONE:
			WAIT_STATE(ch, PULSE_VIOLENCE*1);
			break;
		case RACE_GIANT_FROST:
		case RACE_GIANT_FIRE:
			WAIT_STATE(ch, PULSE_VIOLENCE*2);
			break;
		case RACE_GIANT_HILL:
			WAIT_STATE(ch, PULSE_VIOLENCE*3);
			break;
		case RACE_DWARF:
		case RACE_DARK_DWARF:
			WAIT_STATE(ch, PULSE_VIOLENCE*4);
			break;
		case RACE_GOBLIN:
		case RACE_ORC:
		case RACE_HALF_OGRE:
			WAIT_STATE(ch, PULSE_VIOLENCE*5);
			break;
		case RACE_GNOME:
		case RACE_DEEP_GNOME:
			WAIT_STATE(ch, PULSE_VIOLENCE*6);
			break;
		case RACE_HUMAN:
		case RACE_GNOLL:
		case RACE_HALFLING:
			WAIT_STATE(ch, PULSE_VIOLENCE*7);
			break;
		default:
			WAIT_STATE(ch, PULSE_VIOLENCE*8);
			break;
		}
	}
}

/* ACIDUS 2003, skill forge */
#define ARMA_BASE   19550

void ForgeString(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;

	if(type != 1)
		if(!*arg || (*arg == '\n')) {
			return;
		}

	obj=ch->specials.objedit;
	if(type != 1) {

		sprintf(buf,"%s, %s",(char*) strdup(arg), obj->short_description);
		if(obj->short_description) {
			free(obj->short_description);
		}
		obj->short_description= (char*)strdup(buf);

		sprintf(buf,"%s %s",(char*) strdup(arg), obj->name);
		if(obj->name) {
			free(obj->name);
		}
		obj->name= (char*)strdup(buf);


		ch->desc->connected = CON_PLYNG;
		send_to_char("\n\r\n\r", ch);
		act("Lavori intensamente e alla fine riesci a forgiare quello che volevi.",
			TRUE, ch, 0, 0, TO_CHAR);
		act("$n lavora intensamente e alla fine riesce a forgiare quello che voleva.",
			TRUE, ch, 0, 0, TO_ROOM);
		return;
	}

	send_to_char("\n\rGli dei ti concedono di battezzare questo splendido oggetto!!", ch);
	send_to_char("\n\rInserisci il nome dell'oggetto forgiato: ", ch);
	ch->desc->connected = CON_OBJ_FORGING;

	return;
}

void ForgeGraphic(struct char_data* ch, int urka) {
	int percent,i;
	struct char_data* vict;
	char buf[250];

	i = 1;
	send_to_char("\n\r\n\r",ch);
	while(i<=10) {
		percent = number(1,40);
		switch(percent) {
		case 1:
			send_to_char("\n\rPrendi un martello, lo impugni bene e picchi violentemente!\n\r",ch);
			break;
		case 2:
			send_to_char("\n\rTi asciughi la fronte grondante di sudore.\n\r",ch);
			break;
		case 3:
			send_to_char("\n\rPer poco non facevi raffreddare il metallo!\n\r",ch);
			break;
		case 4:
			send_to_char("\n\rGuardi l'arma prendere pian piano la forma voluta.\n\r",ch);
			break;
		case 5:
			send_to_char("\n\rUfff...che caldo qui. Mandi giu' un boccalone di birra nanesca!\n\r",ch);
			break;
		case 6:
			send_to_char("\n\rBene, bene, stai lavorando con grande maestria.\n\r",ch);
			break;
		case 7:
			send_to_char("\n\rIl rumore violento degli attrezzi ti rimbomba nel cervello.\n\r",ch);
			break;
		case 8:
			send_to_char("\n\rLavori di precisione sul manico dell'arma.\n\r",ch);
			break;
		case 9:
			send_to_char("\n\rTi siedi un attimo per riposarti...ma proprio un attimo solo!\n\r",ch);
			break;
		case 10:
			send_to_char("\n\rCon una pinza pieghi una parte del metallo incandescente.\n\r",ch);
			break;
		case 11:
			send_to_char("\n\rTi gasi sempre di piu', sara' una buona arma, lo senti!\n\r",ch);
			break;
		case 12:
			send_to_char("\n\rOsservi il Custode della Fiamma che alimenta la Forgia ignorandoti.\n\r",ch);
			break;
		case 13:
			send_to_char("\n\rInfili l'arma nel fuoco vivo della Forgia.\n\r",ch);
			break;
		case 14:
			send_to_char("\n\rDai un colpo violento e l'arma vibra sull'incudine.\n\r",ch);
			break;
		case 15:
			send_to_char("\n\rDosi sapientemente la forza e la precisione.\n\r",ch);
			break;
		case 16:
			send_to_char("\n\rTi fermi un attimo a pensare al prossimo passo.\n\r",ch);
			break;
		case 17:
			send_to_char("\n\rPrendi un martello, lo impugni bene e picchi violentemente!\n\r",ch);
			break;
		case 18:
			send_to_char("\n\rPrendi un martello, lo impugni bene e picchi violentemente!\n\r",ch);
			break;
		case 19:
			send_to_char("\n\rPrendi un martello, lo impugni bene e picchi violentemente!\n\r",ch);
			break;
		case 20:
			send_to_char("\n\rTi pulisci le mani sporche sul grembiule.\n\r",ch);
			break;
		case 21:
			send_to_char("\n\rLa fuliggine ti entra negli occhi ma resisti!\n\r",ch);
			break;
		case 22:
			send_to_char("\n\rSenti l'odore del metallo fuso...e l'aspiri con avidita'!\n\r",ch);
			break;
		case 23:
			send_to_char("\n\rMetti l'arma dentro una morsa e ne pieghi una parte.\n\r",ch);
			break;
		case 24:
			send_to_char("\n\rMetti l'arma dentro una morsa e ne pieghi una parte.\n\r",ch);
			break;
		case 25:
			send_to_char("\n\rSogni di tuffarti in una rissa con la tua nuova arma!\n\r",ch);
			break;
		case 26:
			send_to_char("\n\rMetti insieme due pezzi fondendoli insieme.\n\r",ch);
			break;
		case 27:
			send_to_char("\n\rMetti insieme due pezzi fondendoli insieme.\n\r",ch);
			break;
		case 28:
			send_to_char("\n\rPrendi la mira e dai un colpo secco.\n\r",ch);
			break;
		case 29:
			send_to_char("\n\rPrendi la mira e dai un colpo secco.\n\r",ch);
			break;
		case 30:
			send_to_char("\n\rPrendi la mira e dai un colpo secco.\n\r",ch);
			break;
		case 31:
			send_to_char("\n\rOsservi l'arma prendere la forma desiderata.\n\r",ch);
			break;
		case 32:
			send_to_char("\n\rHai molta sete, e ti scoli un boccale di birra.\n\r",ch);
			break;
		case 33:
			send_to_char("\n\rEhm, a forza di bere birra sei alticcio...ma ti aumenta l'ispirazione!\n\r",ch);
			break;
		case 34:
			send_to_char("\n\rTrattieni il respiro e fai un lavoro di fino.\n\r",ch);
			break;
		case 35:
			send_to_char("\n\rTi fai una risatina goduriosa...\n\r",ch);
			break;
		case 36:
			send_to_char("\n\rMetti a fondere un lingotto.\n\r",ch);
			break;
		case 37:
			send_to_char("\n\rSoppesi il semilavorato per studiarne il bilanciamento.\n\r",ch);
			break;
		case 38:
			send_to_char("\n\rSoppesi il semilavorato per studiarne il bilanciamento.\n\r",ch);
			break;
		case 39:
			send_to_char("\n\rOh porc...per poco non ti davi una martellata su un dito!!\n\r",ch);
			break;
		case 40:
			send_to_char("\n\rAhia!! Ti cade una tenaglia su un piede!!\n\r",ch);
			break;
		default:
			break;
		}
		i = i+1;
	}

	//azioni aggiuntive per arma con massimo danno
	if(urka) {
		send_to_char("\n\r\n\rNon hai sbagliato niente..questa e' un'arma eccezionale!!\n\r\n\r",ch);
		if((vict = get_char_vis(ch, "custode"))) {
			command_interpreter(vict, "gasp");
		}
		send_to_char("\n\r\n\r",ch);
		if((vict = get_char_vis(ch, "custode"))) {
			command_interpreter(vict, "tell durin Grande forgiatura Re Durin");
		}
		send_to_char("\n\r\n\r",ch);
		if((vict = get_char_vis(ch, "durin"))) {
			sprintf(buf, "Onore a %s costruttore di un'arma orgoglio dei nani!!", GET_NAME(ch));
			command_interpreter(vict, buf);
			send_to_char("\n\r\n\r",ch);
		}
		if((vict = get_char_vis(ch, "durin"))) {
			command_interpreter(vict, "shout Nessuno ha mai fatto di meglio!!");
		}
		send_to_char("\n\r\n\r",ch);
	}
}

ACTION_FUNC(do_forge) {
	int r_num,percent,bonus,class_bonus,cdb,cdd,dex_malus,urka=0;
	int numdice,sizedice,peso,div_peso,peso_old,damtype,valore,rent,hold,nling,vling;
	int VNum,i,exp;
	long exflags,wflags;
	struct obj_data* obj;
	char itemname[25];
	char itemmetal[25];
	char itemdesc[80];
	char buf[MAX_STRING_LENGTH];

	if(!ch->skills) {
		return;
	}

	if(ch->skills[SKILL_FORGE].learned <=0) {
		send_to_char("Pensi di essere un fabbro?\n\r.",ch);
		return;
	}

	switch(GET_RACE(ch)) {
	case RACE_DWARF:
		break;
	default:
		send_to_char("Non sei mica un nano!\n\r",ch);
		return;
		break;
	}

	if(!((ch->in_room) == 4432)) {
		send_to_char("Qui non hai gli attrezzi adatti.\n\r",ch);
		return;
	}

	if(GetMaxLevel(ch) < 30) {
		send_to_char("Non sei abbastanza maturo per forgiare oggetti.\n\r",ch);
		return;
	}

	arg = one_argument(arg,itemname);
	arg = one_argument(arg,itemmetal);

	if(!*itemname) {
		send_to_char("Forgiare cosa?\n\r",ch);
		return;
	}

	if(!*itemmetal) {
		send_to_char("Con che materiale?\n\r",ch);
		return;
	}

	if(GET_MOVE(ch) < 10) {
		send_to_char("Sei troppo stanco, e' meglio se ti riposi un po'.\n\r",ch);
		return;
	}

	GET_MOVE(ch) -= 10;
	alter_move(ch,0);

	percent = number(1,101); /* 101% is a complete failure */

	if(ch->skills && ch->skills[SKILL_FORGE].learned &&
			GET_POS(ch) > POSITION_SITTING) {
		if(percent > ch->skills[SKILL_FORGE].learned) {
			/* failed */
			act("Fai una mossa maldestra e rovini il lavoro fatto.",
				TRUE, ch, 0, 0, TO_CHAR);
			act("$n fa una mossa maldestra e rovina il lavoro fatto.",
				TRUE, ch, 0, 0, TO_ROOM);
			LearnFromMistake(ch, SKILL_FORGE, 0, 90);
		}
		else {
			/* made it */

			bonus = (int)(ch->skills[SKILL_FORGE].learned / 10);

			bonus += (int)(GetMaxLevel(ch) / 10);
			if(IS_PRINCE(ch)) {
				bonus +=1;
			}

			//Calcolo bonus per la classe, i multiclasse dividono
			class_bonus = 0;
			if(HasClass(ch,CLASS_WARRIOR)) {
				class_bonus += 4 ;
			}
			if(HasClass(ch,CLASS_PALADIN)) {
				class_bonus += 3 ;
			}
			if(HasClass(ch,CLASS_CLERIC))  {
				class_bonus += 1;
			}
			class_bonus = (int)(class_bonus / HowManyClasses(ch));

			//Malus per dex bassa
			dex_malus = MIN(17-GET_DEX(ch),4);
			dex_malus = MAX(dex_malus,0);

			cdb = MAX(bonus + class_bonus - dex_malus,1);

			//Calcolo il cdd partendo dal cdb e mettendo un fattore di casualit\E0
			percent = number(1,100);

			if(percent <= 25) {
				cdd = cdb+1;
			}
			else if(percent > 25 && percent <= 45) {
				cdd = cdb+2;
			}
			else if(percent > 45 && percent <= 60) {
				cdd = cdb+3;
			}
			else if(percent > 60 && percent <= 70) {
				cdd = cdb+4;
			}
			else if(percent > 70 && percent <= 80) {
				cdd = cdb+5;
			}
			else if(percent > 80 && percent <= 85) {
				cdd = cdb+6;
			}
			else if(percent > 85 && percent <= 90) {
				cdd = cdb+7;
			}
			else if(percent > 90 && percent <= 94) {
				cdd = cdb+8;
			}
			else if(percent > 94 && percent <= 97) {
				cdd = cdb+9;
			}
			else if(percent > 97 && percent <= 99) {
				cdd = cdb+10;
			}
			else {
				cdd = cdb+11;
			}

			//if (!strcmp(itemname,"spada")) urka = TRUE;

			//Modifico il cdd in base al tipo di oggetto, calcolo il divisore per il peso e
			//il tipo di danno
			div_peso = 10; //da dividere per 10 successivamente
			damtype = 0;
			if(!strcmp(itemname,"pugnale")) {
				cdd -= 5;
				div_peso=50;
				damtype=1;
			}
			else if(!strcmp(itemname,"martello")) {
				cdd -= 4;
				div_peso=20;
				damtype=6;
			}
			else if(!strcmp(itemname,"piccone")) {
				cdd -= 4;
				div_peso=20;
				damtype=11;
			}
			else if(!strcmp(itemname,"mazza")) {
				cdd -= 3;
				div_peso=15;
				damtype=0;
			}
			else if(!strcmp(itemname,"mazzafrusto")) {
				cdd -= 2;
				damtype=4;
			}
			else if(!strcmp(itemname,"spada")) {
				cdd -= 1;
				div_peso=15;
				damtype=3;
			}
			else if(!strcmp(itemname,"ascia")) {
				cdd -= 0;
				damtype=5;
			}
			else {
				send_to_char("Non e' un oggetto che sai costruire.\n\r",ch);
				return;
			}

			//Setto il numero di lingotti necessari
			if(!strcmp(itemname,"pugnale")) {
				nling=1;
			}
			else if(!strcmp(itemname,"martello")) {
				nling=2;
			}
			else if(!strcmp(itemname,"piccone")) {
				nling=2;
			}
			else if(!strcmp(itemname,"mazza")) {
				nling=3;
			}
			else if(!strcmp(itemname,"mazzafrusto")) {
				nling=4;
			}
			else if(!strcmp(itemname,"spada")) {
				nling=3;
			}
			else if(!strcmp(itemname,"ascia")) {
				nling=4;
			}


			//Modifico il cdd in base al tipo di materiale, calcolo il modificatore per il peso,
			//e il vnum dei lingotti necessari
			peso = 0;
			if(!strcmp(itemmetal,"oro")) {
				cdd -= 1;
				peso = 8;
				vling=19541;
			}
			else if(!strcmp(itemmetal,"platino")) {
				peso = 10;
				vling=19540;
			}
			else if(!strcmp(itemmetal,"mithril")) {
				peso = -6;
				vling=19539;
			}
			else if(!strcmp(itemmetal,"adamantite")) {
				peso = -2;
				vling=19538;
			}
			else if(!strcmp(itemmetal,"argento")) {
				cdd -= 2;
				peso = 4;
				vling=19546;
			}
			else if(!strcmp(itemmetal,"acciaio")) {
				cdd -= 3;
				peso = -2;
				vling=19548;
			}
			else if(!strcmp(itemmetal,"ferro")) {
				cdd -= 4;
				peso = -2;
				vling=19547;
			}
			else if(!strcmp(itemmetal,"stagno")) {
				cdd -= 5;
				peso = -4;
				vling=19542;
			}
			else if(!strcmp(itemmetal,"piombo")) {
				cdd -= 6;
				peso = 6;
				vling=19545;
			}
			else if(!strcmp(itemmetal,"bronzo")) {
				cdd -= 7;
				vling=19544;
			}
			else if(!strcmp(itemmetal,"rame")) {
				cdd -= 8;
				peso = 2;
				vling=19543;
			}
			else {
				send_to_char("Non e' un materiale adatto.\n\r",ch);
				return;
			}
			cdd = (int) MAX(cdd,1);

			//Consumo i lingotti, se non ce ne sono abbastanza mi fermo
			obj = ch->carrying;
			i = 0;
			while((obj) && (i<nling)) {
				VNum = (obj->item_number >= 0) ?
					   obj_index[obj->item_number].iVNum : 0;

				if(VNum == vling) {
					i = i+1;
				}

				obj = obj->next_content;
			}
			if((i<nling) && (GetMaxLevel(ch) < DIO)) {
				send_to_char("\n\rNon hai il materiale sufficiente, procuratelo!\n\r",ch);
				return;
			}
			obj = ch->carrying;
			i = 0;
			while((obj) && (i<nling)) {
				VNum = (obj->item_number >= 0) ?
					   obj_index[obj->item_number].iVNum : 0;

				if(VNum == vling) {
					obj_from_char(obj);
					extract_obj(obj);
					obj = ch->carrying; //riparto da capo
					i = i+1;
				}
				else {
					obj = obj->next_content;
				}
			}

			//Controllo se ho uno string
			if(!strcmp(itemname,"pugnale") && (cdd >= 25)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"martello") && (cdd >= 26)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"piccone") && (cdd >= 26)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"mazza") && (cdd >= 27)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"mazzafrusto") && (cdd >= 28)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"spada") && (cdd >= 29)) {
				cdd -= 1;
				urka = TRUE;
			}
			else if(!strcmp(itemname,"ascia") && (cdd >= 30)) {
				cdd -= 1;
				urka = TRUE;
			}

			//Setto una parte della descrizione
			if(!strcmp(itemname,"pugnale")) {
				if(urka) {
					sprintf(itemdesc,"il pugnale ");
				}
				else {
					sprintf(itemdesc,"Un pugnale ");
				}
			}
			else if(!strcmp(itemname,"martello")) {
				if(urka) {
					sprintf(itemdesc,"il martello ");
				}
				else {
					sprintf(itemdesc,"Un martello ");
				}
			}
			else if(!strcmp(itemname,"piccone")) {
				if(urka) {
					sprintf(itemdesc,"il piccone ");
				}
				else {
					sprintf(itemdesc,"Un piccone ");
				}
			}
			else if(!strcmp(itemname,"mazza")) {
				if(urka) {
					sprintf(itemdesc,"la mazza ");
				}
				else {
					sprintf(itemdesc,"Una mazza ");
				}
			}
			else if(!strcmp(itemname,"mazzafrusto")) {
				if(urka) {
					sprintf(itemdesc,"il mazzafrusto ");
				}
				else {
					sprintf(itemdesc,"Un mazzafrusto ");
				}
			}
			else if(!strcmp(itemname,"spada")) {
				if(urka) {
					sprintf(itemdesc,"la spada ");
				}
				else {
					sprintf(itemdesc,"Una spada ");
				}
			}
			else if(!strcmp(itemname,"ascia")) {
				if(urka) {
					sprintf(itemdesc,"l'ascia ");
				}
				else {
					sprintf(itemdesc,"Un'ascia ");
				}
			}

			//Setto un'altra parte della desc
			if(!strcmp(itemmetal,"oro")) {
				if(!urka) {
					strcat(itemdesc,"d'oro ");
				}
			}
			else if(!strcmp(itemmetal,"platino")) {
				if(!urka) {
					strcat(itemdesc,"di platino ");
				}
			}
			else if(!strcmp(itemmetal,"mithril")) {
				if(!urka) {
					strcat(itemdesc,"di mithril ");
				}
			}
			else if(!strcmp(itemmetal,"adamantite")) {
				if(!urka) {
					strcat(itemdesc,"d'adamantite ");
				}
			}
			else if(!strcmp(itemmetal,"argento")) {
				if(!urka) {
					strcat(itemdesc,"d'argento ");
				}
			}
			else if(!strcmp(itemmetal,"acciaio")) {
				if(!urka) {
					strcat(itemdesc,"d'acciaio ");
				}
			}
			else if(!strcmp(itemmetal,"ferro")) {
				if(!urka) {
					strcat(itemdesc,"di ferro ");
				}
			}
			else if(!strcmp(itemmetal,"stagno")) {
				if(!urka) {
					strcat(itemdesc,"di stagno ");
				}
			}
			else if(!strcmp(itemmetal,"piombo")) {
				if(!urka) {
					strcat(itemdesc,"di piombo ");
				}
			}
			else if(!strcmp(itemmetal,"bronzo")) {
				if(!urka) {
					strcat(itemdesc,"di bronzo ");
				}
			}
			else if(!strcmp(itemmetal,"rame")) {
				if(!urka) {
					strcat(itemdesc,"di rame ");
				}
			}



			//Determino il numero e la dimensione del dado dell'arma
			switch(cdd) {
			case 1:
				numdice = 1;
				sizedice = 1;
				break;
			case 2:
				numdice = 1;
				sizedice = 2;
				break;
			case 3:
				percent = number(1,2);
				if(percent == 1) {
					numdice = 1;
					sizedice = 3;
				}
				else {
					numdice = 2;
					sizedice = 1;
				}
				break;
			case 4:
				numdice = 1;
				sizedice = 4;
				break;
			case 5:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 5;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 2;
				}
				else {
					numdice = 3;
					sizedice = 1;
				}
				break;
			case 6:
				numdice = 1;
				sizedice = 6;
				break;
			case 7:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 7;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 3;
				}
				else {
					numdice = 4;
					sizedice = 1;
				}
				break;
			case 8:
				percent = number(1,2);
				if(percent == 1) {
					numdice = 1;
					sizedice = 8;
				}
				else {
					numdice = 3;
					sizedice = 2;
				}
				break;
			case 9:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 9;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 4;
				}
				else {
					numdice = 5;
					sizedice = 1;
				}
				break;
			case 10:
				numdice = 1;
				sizedice = 10;
				break;
			case 11:
				percent = number(1,5);
				if(percent == 1) {
					numdice = 1;
					sizedice = 11;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 5;
				}
				else if(percent == 3) {
					numdice = 3;
					sizedice = 3;
				}
				else if(percent == 4) {
					numdice = 4;
					sizedice = 2;
				}
				else {
					numdice = 6;
					sizedice = 1;
				}
				break;
			case 12:
				numdice = 1;
				sizedice = 12;
				break;
			case 13:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 13;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 6;
				}
				else {
					numdice = 7;
					sizedice = 1;
				}
				break;
			case 14:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 14;
				}
				else if(percent == 2) {
					numdice = 3;
					sizedice = 4;
				}
				else {
					numdice = 5;
					sizedice = 2;
				}
				break;
			case 15:
				percent = number(1,4);
				if(percent == 1) {
					numdice = 1;
					sizedice = 15;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 7;
				}
				else if(percent == 3) {
					numdice = 4;
					sizedice = 3;
				}
				else {
					numdice = 8;
					sizedice = 1;
				}
				break;
			case 16:
				numdice = 1;
				sizedice = 16;
				break;
			case 17:
				percent = number(1,4);
				if(percent == 1) {
					numdice = 1;
					sizedice = 17;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 8;
				}
				else if(percent == 3) {
					numdice = 3;
					sizedice = 5;
				}
				else {
					numdice = 9;
					sizedice = 1;
				}
				break;
			case 18:
				numdice = 1;
				sizedice = 18;
				break;
			case 19:
				percent = number(1,5);
				if(percent == 1) {
					numdice = 1;
					sizedice = 19;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 9;
				}
				else if(percent == 3) {
					numdice = 4;
					sizedice = 4;
				}
				else if(percent == 4) {
					numdice = 5;
					sizedice = 3;
				}
				else {
					numdice = 10;
					sizedice = 1;
				}
				break;
			case 20:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 20;
				}
				else if(percent == 2) {
					numdice = 3;
					sizedice = 6;
				}
				else {
					numdice = 7;
					sizedice = 2;
				}
				break;
			case 21:
				percent = number(1,2);
				if(percent == 1) {
					numdice = 1;
					sizedice = 21;
				}
				else {
					numdice = 2;
					sizedice = 10;
				}
				break;
			case 22:
				numdice = 1;
				sizedice = 22;
				break;
			case 23:
				percent = number(1,6);
				if(percent == 1) {
					numdice = 1;
					sizedice = 23;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 11;
				}
				else if(percent == 3) {
					numdice = 3;
					sizedice = 7;
				}
				else if(percent == 4) {
					numdice = 4;
					sizedice = 5;
				}
				else if(percent == 5) {
					numdice = 6;
					sizedice = 3;
				}
				else {
					numdice = 8;
					sizedice = 2;
				}
				break;
			case 24:
				percent = number(1,2);
				if(percent == 1) {
					numdice = 1;
					sizedice = 24;
				}
				else {
					numdice = 5;
					sizedice = 4;
				}
				break;
			case 25:
				percent = number(1,2);
				if(percent == 1) {
					numdice = 1;
					sizedice = 25;
				}
				else {
					numdice = 2;
					sizedice = 12;
				}
				break;
			case 26:
				percent = number(1,3);
				if(percent == 1) {
					numdice = 1;
					sizedice = 26;
				}
				else if(percent == 2) {
					numdice = 3;
					sizedice = 8;
				}
				else {
					numdice = 9;
					sizedice = 2;
				}
				break;
			case 27:
				percent = number(1,4);
				if(percent == 1) {
					numdice = 1;
					sizedice = 27;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 13;
				}
				else if(percent == 3) {
					numdice = 4;
					sizedice = 6;
				}
				else {
					numdice = 7;
					sizedice = 3;
				}
				break;
			case 28:
				numdice = 1;
				sizedice = 28;
				break;
			case 29:
			case 30:
				percent = number(1,6);
				if(percent == 1) {
					numdice = 1;
					sizedice = 29;
				}
				else if(percent == 2) {
					numdice = 2;
					sizedice = 14;
				}
				else if(percent == 3) {
					numdice = 3;
					sizedice = 9;
				}
				else if(percent == 4) {
					numdice = 5;
					sizedice = 5;
				}
				else if(percent == 5) {
					numdice = 6;
					sizedice = 4;
				}
				else {
					numdice = 10;
					sizedice = 2;
				}
				break;
			default:
				mudlog(LOG_SYSERR, "errore nel calcolo del CDD del forge");
				return;
				break;
			}

			//Calcolo il peso dell'oggetto inserendo anche un fattore casuale
			peso = (int)((cdd + peso) / (div_peso/10));
			percent = number(1,100);
			if(percent <= 5)  {
				peso += 3;
			}
			else if(percent > 6 && percent <= 15) {
				peso += 2;
			}
			else if(percent > 15 && percent <= 35) {
				peso += 1;
			}
			else if(percent > 65 && percent <= 85) {
				peso -= 1;
			}
			else if(percent > 85 && percent <= 95) {
				peso -= 2;
			}
			else {
				peso -= 3;
			}
			peso = MAX(peso,1);

			//Calcolo il valore dell'oggetto in base al materiale e al cdd
			if(!strcmp(itemmetal,"oro")) {
				valore = 8;
			}
			else if(!strcmp(itemmetal,"platino")) {
				valore = 11;
			}
			else if(!strcmp(itemmetal,"mithril")) {
				valore = 11;
			}
			else if(!strcmp(itemmetal,"adamantite")) {
				valore = 11;
			}
			else if(!strcmp(itemmetal,"argento")) {
				valore = 7;
			}
			else if(!strcmp(itemmetal,"acciaio")) {
				valore = 6;
			}
			else if(!strcmp(itemmetal,"ferro")) {
				valore = 5;
			}
			else if(!strcmp(itemmetal,"stagno")) {
				valore = 4;
			}
			else if(!strcmp(itemmetal,"piombo")) {
				valore = 3;
			}
			else if(!strcmp(itemmetal,"bronzo")) {
				valore = 2;
			}
			else if(!strcmp(itemmetal,"rame")) {
				valore = 1;
			}
			else {
				valore = 1;
			}
			valore = valore * cdd * 100;

			//Calcolo il rent dell'oggetto in base al suo valore
			rent = (valore/10) * 4;
			if(rent < 5000) {
				rent = 0;
			}

			//Setto i wear flags
			hold = FALSE;
			wflags = ITEM_TAKE + ITEM_WIELD;
			if((cdd < 20) && (peso < 6)) {
				hold = TRUE;
				wflags = wflags + ITEM_HOLD;
			}

			//Setto gli extra flags
			exflags = ITEM_METAL + ITEM_ANTI_MONK;
			//if (!strcmp(itemmetal,"adamantite")) exflags = exflags + ITEM_ANTI_SUN;
			if(!strcmp(itemname,"pugnale")) {
				exflags = exflags + ITEM_ANTI_CLERIC + ITEM_SCYTHE;
				if(peso > 10) {
					exflags = exflags + ITEM_ANTI_DRUID;
				}
				if(!hold) {
					exflags = exflags + ITEM_ANTI_MAGE;
				}
			}
			else if(!strcmp(itemname,"martello")) {
				exflags = exflags + ITEM_ANTI_THIEF + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			else if(!strcmp(itemname,"piccone")) {
				exflags = exflags + ITEM_DIG + ITEM_ANTI_CLERIC + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			else if(!strcmp(itemname,"mazza")) {
				exflags = exflags + ITEM_ANTI_THIEF + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			else if(!strcmp(itemname,"mazzafrusto")) {
				exflags = exflags + ITEM_ANTI_THIEF + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			else if(!strcmp(itemname,"spada")) {
				exflags = exflags + ITEM_ANTI_THIEF + ITEM_ANTI_CLERIC + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			else if(!strcmp(itemname,"ascia")) {
				exflags = exflags + ITEM_ANTI_THIEF + ITEM_ANTI_CLERIC + ITEM_ANTI_DRUID + ITEM_ANTI_MAGE;
			}
			if(!hold) {
				exflags = exflags + ITEM_ANTI_PSI;
			}

			//Calcolo la fragilit\E0/resistenza dell'oggetto
			if(!strcmp(itemmetal,"oro") && (number(1,100)<10)) {
				exflags = exflags + ITEM_BRITTLE;
			}
			else if(!strcmp(itemmetal,"platino") && (number(1,100)<40)) {
				exflags = exflags + ITEM_RESISTANT;
			}
			else if(!strcmp(itemmetal,"mithril") && (number(1,100)<70)) {
				exflags = exflags + ITEM_RESISTANT;
			}
			else if(!strcmp(itemmetal,"acciaio") && (number(1,100)<20)) {
				exflags = exflags + ITEM_RESISTANT;
			}
			else if(!strcmp(itemmetal,"adamantite")) {
				exflags = exflags + ITEM_RESISTANT;
			}
			else if(!strcmp(itemmetal,"stagno")) {
				exflags = exflags + ITEM_BRITTLE;
			}
			else if(!strcmp(itemmetal,"piombo") && (number(1,100)<40)) {
				exflags = exflags + ITEM_BRITTLE;
			}
			else if(!strcmp(itemmetal,"rame") && (number(1,100)<70)) {
				exflags = exflags + ITEM_BRITTLE;
			}




			//Carico l'oggetto e lo edito
			if((r_num = real_object(ARMA_BASE)) >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj,ch);
			}

			if(GetMaxLevel(ch) < DIO) {
				ForgeGraphic(ch, urka);
			}

			if(!urka) {
				send_to_char("\n\r\n\r",ch);
				act("Lavori intensamente e alla fine riesci a forgiare quello che volevi.",
					TRUE, ch, 0, 0, TO_CHAR);
				act("$n lavora intensamente e alla fine riesce a forgiare quello che voleva.",
					TRUE, ch, 0, 0, TO_ROOM);
			}

			//Guadagno xp per skill riuscita
			if(!IS_IMMORTAL(ch)) {

				if(cdd <= 3) {
					exp = 10000;
				}
				else if(cdd > 3 && cdd <= 6) {
					exp = 15000;
				}
				else if(cdd > 6 && cdd <= 8) {
					exp = 20000;
				}
				else if(cdd > 8 && cdd <= 10) {
					exp = 25000;
				}
				else if(cdd > 10 && cdd <= 12) {
					exp = 30000;
				}
				else if(cdd > 12 && cdd <= 14) {
					exp = 50000;
				}
				else if(cdd > 14 && cdd <= 16) {
					exp = 70000;
				}
				else if(cdd == 17) {
					exp = 90000;
				}
				else if(cdd == 18) {
					exp = 100000;
				}
				else if(cdd == 19) {
					exp = 120000;
				}
				else if(cdd == 20) {
					exp = 150000;
				}
				else if(cdd == 21) {
					exp = 200000;
				}
				else if(cdd == 22) {
					exp = 300000;
				}
				else if(cdd == 23) {
					exp = 450000;
				}
				else if(cdd == 24) {
					exp = 500000;
				}
				else if(cdd == 25) {
					exp = 600000;
				}
				else if(cdd == 26) {
					exp = 750000;
				}
				else if(cdd == 27) {
					exp = 1000000;
				}
				else if(cdd == 28) {
					exp = 1500000;
				}
				else if(cdd == 29) {
					exp = 2000000;
				}

				if(urka) {
					exp = 3000000;
				}


				sprintf(buf,"La tua esperienza e` aumentata di %d punti.", exp);
				act(buf, FALSE, ch, 0, 0, TO_CHAR);
				gain_exp(ch, exp);
			}

			peso_old = GET_OBJ_WEIGHT(obj);
			sprintf(buf,"arma name %s %s",itemname,itemmetal);
			do_ooedit(ch,buf,0);
			strcat(itemdesc,"di fattura nanesca");
			sprintf(buf,"%s sdesc %s",itemname,itemdesc);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s ldesc %s giace qui a terra.",itemname,itemdesc);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s v1 %d",itemname,numdice);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s v2 %d",itemname,sizedice);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s v3 %d",itemname,damtype);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s weight %d",itemname,peso);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s value %d",itemname,valore);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s cost %d",itemname,rent);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s exflags %ld",itemname,exflags);
			do_ooedit(ch,buf,0);
			sprintf(buf,"%s wflags %ld",itemname,wflags);
			do_ooedit(ch,buf,0);

			//Il peso \E8 cambiato, quindi lo ricalcolo
			IS_CARRYING_W(obj->carried_by) -= peso_old;
			IS_CARRYING_W(obj->carried_by) += GET_OBJ_WEIGHT(obj);


			if(urka) {
				ch->specials.objedit=obj;
				ch->specials.oedit = 1;
				ForgeString(ch, "", 1);
				return;
			}

		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3);
	}

}
} // namespace Alarmud

