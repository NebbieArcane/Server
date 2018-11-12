/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: spec_procs3.c,v 1.4 2002/03/23 17:15:15 Thunder Exp $
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
#include "spec_procs3.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.obj2.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "board.hpp"
#include "comm.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "gilde.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "mail.hpp"
#include "mindskills1.hpp"
#include "mobact.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "skills.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "trap.hpp"
#include "utility.hpp"
namespace Alarmud {


/* chess_game() stuff starts here */
/* Inspiration and original idea by Feith */
/* Implementation by Gecko */
#define WHITE 0
#define BLACK 1

int side = WHITE;  /* to avoid having to pass side with each function call */

#define IS_BLACK(piece) (((piece) >= 1400) && ((piece) <= 1415))
#define IS_WHITE(piece) (((piece) >= 1448) && ((piece) <= 1463))
#define IS_PIECE(piece) ((IS_WHITE(piece)) || (IS_BLACK(piece)))
#define IS_ENEMY(piece) (side?IS_WHITE(piece):IS_BLACK(piece))
#define IS_FRIEND(piece) (side?IS_BLACK(piece):IS_WHITE(piece))
#define ON_BOARD(room) (((room) >= 1400) && ((room) <= 1463))
#define FORWARD (side?2:0)
#define BACK    (side?0:2)
#define LEFT    (side?1:3)
#define RIGHT   (side?3:1)

#define EXIT_ROOM(roomp,dir) ((roomp)?((roomp)->dir_option[dir]):NULL)
#define CAN_GO_ROOM(roomp,dir) (EXIT_ROOM(roomp,dir) &&                                real_roomp(EXIT_ROOM(roomp,dir)->to_room))

/* get pointer to room in the given direction */
#define ROOMP(roomp,dir) ((CAN_GO_ROOM(roomp,dir)) ?                           real_roomp(EXIT_ROOM(roomp,dir)->to_room) : NULL)

struct room_data* forward_square(struct room_data* room) {
	return ROOMP(room, FORWARD);
}

struct room_data* back_square(struct room_data* room) {
	return ROOMP(room, BACK);
}

struct room_data* left_square(struct room_data* room) {
	return ROOMP(room, LEFT);
}

struct room_data* right_square(struct room_data* room) {
	return ROOMP(room, RIGHT);
}

struct room_data* forward_left_square(struct room_data* room) {
	return ROOMP(ROOMP(room, FORWARD), LEFT);
}

struct room_data* forward_right_square(struct room_data* room) {
	return ROOMP(ROOMP(room, FORWARD), RIGHT);
}

struct room_data* back_right_square(struct room_data* room) {
	return ROOMP(ROOMP(room, BACK), RIGHT);
}

struct room_data* back_left_square(struct room_data* room) {
	return ROOMP(ROOMP(room, BACK), LEFT);
}

struct char_data* square_contains_enemy(struct room_data* square) {
	struct char_data* i;

	for(i = square->people; i; i = i->next_in_room)
		if(IS_ENEMY(mob_index[i->nr].iVNum)) {
			return i;
		}

	return NULL;
}

int square_contains_friend(struct room_data* square) {
	struct char_data* i;

	for(i = square->people; i; i = i->next_in_room)
		if(IS_FRIEND(mob_index[i->nr].iVNum)) {
			return TRUE;
		}

	return FALSE;
}

int square_empty(struct room_data* square) {
	struct char_data* i;

	for(i = square->people; i; i = i->next_in_room)
		if(IS_PIECE(mob_index[i->nr].iVNum)) {
			return FALSE;
		}

	return TRUE;
}

MOBSPECIAL_FUNC(chess_game) {
	struct room_data* rp = NULL, *crp = real_roomp(ch->in_room);
	struct char_data* ep = NULL;
	int move_dir = 0, move_amount = 0, move_found = FALSE;
	int c = 0;

	if(cmd || !AWAKE(ch)) {
		return FALSE;
	}

	/* keep original fighter() spec_proc for kings and knights */
	if(ch->specials.fighting)
		switch(mob_index[ch->nr].iVNum) {
		case 1401:
		case 1404:
		case 1406:
		case 1457:
		case 1460:
		case 1462:
			return fighter(ch, cmd, arg, mob, type);
		default:
			return FALSE;
		}

	if(!crp || !ON_BOARD(crp->number)) {
		return FALSE;
	}

	if(side == WHITE && IS_BLACK(mob_index[ch->nr].iVNum)) {
		return FALSE;
	}

	if(side == BLACK && IS_WHITE(mob_index[ch->nr].iVNum)) {
		return FALSE;
	}

	if(number(0,15)) {
		return FALSE;
	}

	switch(mob_index[ch->nr].iVNum) {
	case 1408:
	case 1409:
	case 1410:
	case 1411:  /* black pawns */
	case 1412:
	case 1413:
	case 1414:
	case 1415:
	case 1448:
	case 1449:
	case 1450:
	case 1451:  /* white pawns */
	case 1452:
	case 1453:
	case 1454:
	case 1455:
		move_dir = number(0,3);
		switch(move_dir) {
		case 0:
			rp = forward_left_square(crp);
			break;
		case 1:
			rp = forward_right_square(crp);
			break;
		case 2:
			rp = forward_square(crp);
			break;
		case 3:
			if(real_roomp(ch->in_room) &&
					(real_roomp(ch->in_room)->number == mob_index[ch->nr].iVNum)) {
				rp = forward_square(crp);
				if(rp && square_empty(rp) && ON_BOARD(rp->number)) {
					crp = rp;
					rp = forward_square(crp);
				}
			}
		}
		if(rp && (!square_contains_friend(rp)) && ON_BOARD(rp->number)) {
			ep = square_contains_enemy(rp);
			if(((move_dir <= 1) && ep) || ((move_dir > 1) && !ep)) {
				move_found = TRUE;
			}
		}
		break;

	case 1400:  /* black rooks */
	case 1407:
	case 1456:  /* white rooks */
	case 1463:
		move_dir = number(0,3);
		move_amount = number(1,7);
		for(c = 0; c < move_amount; c++) {
			switch(move_dir) {
			case 0:
				rp = forward_square(crp);
				break;
			case 1:
				rp = back_square(crp);
				break;
			case 2:
				rp = right_square(crp);
				break;
			case 3:
				rp = left_square(crp);
			}
			if(rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
				move_found = TRUE;
				if((ep = square_contains_enemy(rp))) {
					c = move_amount;
				}
				else {
					crp = rp;
				}
			}
			else {
				c = move_amount;
				rp = crp;
			}
		}
		break;

	case 1401:  /* black knights */
	case 1406:
	case 1457:  /* white knights */
	case 1462:
		move_dir = number(0,7);
		switch(move_dir) {
		case 0:
			rp = forward_left_square(forward_square(crp));
			break;
		case 1:
			rp = forward_right_square(forward_square(crp));
			break;
		case 2:
			rp = forward_right_square(right_square(crp));
			break;
		case 3:
			rp = back_right_square(right_square(crp));
			break;
		case 4:
			rp = back_right_square(back_square(crp));
			break;
		case 5:
			rp = back_left_square(back_square(crp));
			break;
		case 6:
			rp = back_left_square(left_square(crp));
			break;
		case 7:
			rp = forward_left_square(left_square(crp));
		}
		if(rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
			move_found = TRUE;
			ep = square_contains_enemy(rp);
		}
		break;

	case 1402:  /* black bishops */
	case 1405:
	case 1458:  /* white bishops */
	case 1461:
		move_dir = number(0,3);
		move_amount = number(1,7);
		for(c = 0; c < move_amount; c++) {
			switch(move_dir) {
			case 0:
				rp = forward_left_square(crp);
				break;
			case 1:
				rp = forward_right_square(crp);
				break;
			case 2:
				rp = back_right_square(crp);
				break;
			case 3:
				rp = back_left_square(crp);
			}
			if(rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
				move_found = TRUE;
				if((ep = square_contains_enemy(rp))) {
					c = move_amount;
				}
				else {
					crp = rp;
				}
			}
			else {
				c = move_amount;
				rp = crp;
			}
		}
		break;

	case 1403:  /* black queen */
	case 1459:  /* white queen */
		move_dir = number(0,7);
		move_amount = number(1,7);
		for(c = 0; c < move_amount; c++) {
			switch(move_dir) {
			case 0:
				rp = forward_left_square(crp);
				break;
			case 1:
				rp = forward_square(crp);
				break;
			case 2:
				rp = forward_right_square(crp);
				break;
			case 3:
				rp = right_square(crp);
				break;
			case 4:
				rp = back_right_square(crp);
				break;
			case 5:
				rp = back_square(crp);
				break;
			case 6:
				rp = back_left_square(crp);
				break;
			case 7:
				rp = left_square(crp);
			}
			if(rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
				move_found = TRUE;
				if((ep = square_contains_enemy(rp))) {
					c = move_amount;
				}
				else {
					crp = rp;
				}
			}
			else {
				c = move_amount;
				rp = crp;
			}
		}
		break;

	case 1404:  /* black king */
	case 1460:  /* white king */
		move_dir = number(0,7);
		switch(move_dir) {
		case 0:
			rp = forward_left_square(crp);
			break;
		case 1:
			rp = forward_square(crp);
			break;
		case 2:
			rp = forward_right_square(crp);
			break;
		case 3:
			rp = right_square(crp);
			break;
		case 4:
			rp = back_right_square(crp);
			break;
		case 5:
			rp = back_square(crp);
			break;
		case 6:
			rp = back_left_square(crp);
			break;
		case 7:
			rp = left_square(crp);
		}
		if(rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
			move_found = TRUE;
			ep = square_contains_enemy(rp);
		}
		break;
	}

	if(move_found && rp) {
		do_emote(ch, "leaves the room.", 0);
		char_from_room(ch);
		char_to_room(ch, rp->number);
		do_emote(ch, "has arrived.", 0);

		if(ep) {
			if(side)
				switch(number(0,3)) {
				case 0:
					do_emote(ch, "grins evilly and says, 'ONLY EVIL shall rule!'", 0);
					break;
				case 1:
					do_emote(ch, "leers cruelly and says, 'You will die now!'", 0);
					break;
				case 2:
					do_emote(ch, "issues a bloodcurdling scream.", 0);
					break;
				case 3:
					do_emote(ch, "glares with black anger.", 0);
				}
			else
				switch(number(0,3)) {
				case 0:
					do_emote(ch, "glows an even brighter pristine white.", 0);
					break;
				case 1:
					do_emote(ch, "chants a prayer and begins battle.", 0);
					break;
				case 2:
					do_emote(ch, "says, 'Black shall lose!", 0);
					break;
				case 3:
					do_emote(ch, "shouts, 'For the Flame! The Flame!'", 0);
				}
			hit(ch, ep, TYPE_UNDEFINED);
		}
		side = (side + 1) % 2;
		return TRUE;
	}
	return FALSE;
}
MOBSPECIAL_FUNC(AcidBlob) {
	struct obj_data* i;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	for(i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
		if(IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && !strncmp(i->name, "corpse", 6)) {
			act("$n destroys some trash.", FALSE, ch, 0, 0, TO_ROOM);

			obj_from_room(i);
			extract_obj(i);
			return(TRUE);
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(death_knight) {

	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(number(0,1)) {
		return(fighter(mob, cmd, arg, mob, type));
	}
	else {
		return(magic_user(mob, cmd, arg, mob, type));
	}
}

MOBSPECIAL_FUNC(acid_monster) {
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("You spit a stream of acid at $N!",FALSE,ch,0,ch->specials.fighting,TO_CHAR);
		act("$n spits a stream of acid at $N!",FALSE,ch,0,ch->specials.fighting,TO_NOTVICT);
		act("$n spits a stream of acid at you!",FALSE,ch,0,ch->specials.fighting,TO_VICT);
		spell_acid_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
		return(TRUE);
	}

	return(FALSE);
}
MOBSPECIAL_FUNC(avatar_celestian) {

	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(number(0,1))   {
		return(fighter(mob, cmd, arg, mob, type));
	}
	else   {
		return(magic_user_imp(mob, cmd, arg, mob, type));
	}

}



MOBSPECIAL_FUNC(baby_bear) {
	struct char_data* t;
	struct room_data* rp;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting)  {
		command_interpreter(ch,"scream");
		rp = real_roomp(ch->in_room);
		if(!rp) {
			return(FALSE);
		}

		for(t=rp->people; t ; t=t->next_in_room) {
			if(GET_POS(t) == POSITION_SLEEPING)
				if(number(0,1)) {
					do_wake(t,"",0);
				}
		}
	}
	return(FALSE);
}

#define TimnusRoom 22602
#define TIMNUSNORTHLIMIT 30
#define TIMNUSWESTLIMIT 9

MOBSPECIAL_FUNC(timnus) {
	/* north = 1 */
	/* west  = 4 */

	struct char_data* vict;
	byte lspell;

	if(cmd) {
		if(cmd == CMD_NORTH && ch->in_room == TimnusRoom) {
			if((TIMNUSNORTHLIMIT < GetMaxLevel(ch)) &&
					(GetMaxLevel(ch) < IMMORTALE)) {
				if(!check_soundproof(ch)) {
					act("$n tells you 'Thou art not pure enough of heart.'", TRUE,
						mob, 0, ch, TO_VICT);
				}
				act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
				return TRUE;
			}
		}
		else if(cmd == CMD_WEST && ch->in_room == TimnusRoom) {
			if((TIMNUSWESTLIMIT < GetMaxLevel(ch)) &&
					(GetMaxLevel(ch) < IMMORTALE)) {
				if(!check_soundproof(ch)) {
					act("$n tells you 'Thou art not pure enough of heart.'", TRUE,
						mob, 0, ch, TO_VICT);
				}
				act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
				return(TRUE);
			}
		}
		return(FALSE);
	}
	else { /* end of if(cmd) */
		if(!IS_PC(ch)) {
			if(ch->specials.fighting) {
				if((GET_POS(ch) < POSITION_FIGHTING) &&
						(GET_POS(ch) > POSITION_STUNNED)) {
					StandUp(ch);
				}
				else {
					if(check_soundproof(ch) || check_nomagic(ch,0,0)) {
						MakeNiftyAttack(ch);
					}
					else {
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
							lspell = 1;
						}

						if(IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
							act("$n utters the words 'Let me see the light!'",
								TRUE, ch, 0, 0, TO_ROOM);
							cast_cure_blind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
							return(FALSE);
						}

						if(GET_MOVE(ch) < 0) {
							act("$n pulls a glass of lemonade out of thin air.  How refreshing.",
								1,ch,0,0,TO_ROOM);
							cast_refresh(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
							return(FALSE);
						}

						if(!(IS_AFFECTED(ch,AFF_FIRESHIELD)) && (lspell > 39)) {
							act("$n utters the words 'FLAME ON!!!'",1,ch,0,0,TO_ROOM);
							cast_fireshield(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
							return(FALSE);
						}

						if(!(IS_AFFECTED(ch,AFF_SANCTUARY)) && (lspell > 25)) {
							act("$n utters the words 'Don't you just hate it when I do this?'",1,ch,0,0,TO_ROOM);
							cast_sanctuary(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
							return(FALSE);
						}

						if(((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 25)) &&
								(GetMaxLevel(ch) >= GetMaxLevel(vict))) {
							act("$n utters the words 'Do unto others as you'd have them "
								"do unto you...'", TRUE, ch, 0, 0, TO_ROOM);
							cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
							return(FALSE);
						}

						if(((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 25)) &&
								(GetMaxLevel(ch) >= GetMaxLevel(vict))) {
							act("$n utters the words 'Do unto others as you'd have them "
								"do unto you...'", TRUE, ch, 0, 0, TO_ROOM);
							cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
							return(FALSE);
						}

						if(lspell > 40) {
							act("$n utters the words 'Here's a penny, go buy a brain, "
								"and give me the change'", TRUE, ch, 0, 0, TO_ROOM);
							cast_feeblemind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
							return(FALSE);
						}
						/* well, spells failed or not needed, let's kick someone :) */

						FighterMove(ch);
						return(FALSE);
					}
				}
			}
		}
	}
	return FALSE;
}


MOBSPECIAL_FUNC(winger) {
	struct char_data* vict;

	if(cmd) {
		return(FALSE);
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	if(check_nomagic(ch, 0, 0))  {
		return(FALSE);
	}

	if(IS_AFFECTED(ch, AFF_PARALYSIS)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		vict = ch->specials.fighting;

		if(!vict) {
			vict = FindVictim(ch);
		}

		if(!vict) {
			return(FALSE);
		}

		if(IS_AFFECTED(vict,AFF_FLYING)) {
			act("$n utters the words 'Instant Wing Remover!'.",1, ch, 0, 0, TO_ROOM);
			cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			return(TRUE);
		}
		else {
			/* As per request of Alfred, if we are fighting and they aren't  */
			/* affected by fly, then eek, we flee! */
			do_flee(ch,"",0);
			return(TRUE);
		}
	}
	return(FALSE);
}

#define STRAHD_ZOMBIE 30000
MOBSPECIAL_FUNC(strahd_zombie) {
	struct char_data* mobtmp;
	char buf[128];

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		if((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED)) {
			StandUp(ch);
		}

		switch(number(0,4)) {
		case 1:
			return(shadow(ch,cmd,arg,mob,type));
		case 2: {
			if(number(0,1)) {
				mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
				char_to_room(mobtmp, ch->in_room);
				act("A body part falls from $n and forms into $N!", FALSE, ch, 0, mobtmp, TO_ROOM);
				/* avoid same name fighting */
				if(GET_NAME(ch)!=GET_NAME(ch->specials.fighting)) {
					sprintf(buf,"kill %s",GET_NAME(ch->specials.fighting));
					command_interpreter(mobtmp,buf);
				}
			}
			return(TRUE);
		}
		break;
		case 3 :
			return(wraith(ch,cmd,arg,mob,type));
			break;
		default:
			return(shadow(ch,cmd,arg,mob,type));
			break;
		} /* end switch */

	} /* end was fighting */

	return(FALSE);
} /* end zombie */


#define SUN_SWORD_RAVENLOFT  30750
#define MAX_SUN_SWORDS             3                /* to limit the number of sun swords */
#define HOLY_ITEM_RAVENLOFT  30751
#define STRAHD_VAMPIRE       30113        /* mob number of strahd */
#define STRAHD_RELOCATE_ROOM 30008      /* room to put second strahd */
MOBSPECIAL_FUNC(strahd_vampire) {
	struct char_data* mobtmp;
	static struct char_data* tmp;
	struct obj_data*  target_obj ;
	char buf[80];
	int i,r_num,iVNum,hasitem=FALSE;
	static int location = -1;
	static int vampnum = 0;

	int sun_loc[] =
	{    30041,    30085,    30088,    30025    };
	int holy_loc[] =
	{     30037,     30015,     30086     };

	/* first time he dies load another strahd at second location */
	/* and force it to hate the person that he was fighting      */

	if(type == EVENT_DEATH) {
		switch(vampnum) {
		case -1:
			vampnum = 0;
			break;
		case 0:
			vampnum++;
			mobtmp = read_mobile(real_mobile(STRAHD_VAMPIRE),REAL);
			char_to_room(mobtmp, STRAHD_RELOCATE_ROOM);
			AddHated(mobtmp,tmp);
			SetHunting(mobtmp,tmp);
			act("$n falls to the ground and crumbles into dust, a faint green "
				"shadow leaves the corpse.", FALSE, ch, 0, 0, TO_ROOM);
			act("$n summons an army of his faithful followers!", FALSE, mobtmp, 0, 0,
				TO_ROOM);
			ch = mobtmp;
			for(i = 0; i < 7; i++) {
				/* summon 7 zombies */
				mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
				char_to_room(mobtmp, ch->in_room);
				add_follower(mobtmp, ch);
				SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
				AddHated(mobtmp,tmp);
			}
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;

		default:
			act("$n falls to the ground and crumbles into dust, a faint sigh of relief seems to fill the castle.",
				FALSE, ch, 0, 0, TO_ROOM);
			vampnum = -1;
			return TRUE;
			break;
		} /* end switch */
	}
	else { /* end event DEATH */
		if(vampnum == -1) { /* fix this if the vamp is loaded and we are not dead */
			vampnum = 0;
		}
	}


	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		tmp = ch->specials.fighting;

		switch(number(0,5)) {
		case 1:
		case 2:
			return(magic_user(mob,cmd,arg,mob,type));
			break;
		case 3:
			/* drain one level, check for sun or holy item here before doing! */

			if(tmp->equipment[WIELD]) {
				iVNum = (tmp->equipment[WIELD]->item_number >= 0) ?
						obj_index[tmp->equipment[WIELD]->item_number].iVNum : 0;
				if(iVNum == SUN_SWORD_RAVENLOFT) {
					hasitem = TRUE;
				}
			} /* end wielding */

			if(tmp->equipment[HOLD]) {
				iVNum = (tmp->equipment[HOLD]->item_number >= 0) ?
						obj_index[tmp->equipment[HOLD]->item_number].iVNum : 0;
				if(iVNum == HOLY_ITEM_RAVENLOFT) {
					hasitem = TRUE;
				}
			} /* end holding */

			if(hasitem) {
				act("$n tries to bite $N, but is repelled by some powerful force!", FALSE, ch, 0, tmp, TO_ROOM);
				return(TRUE);
			}
			/* wel'p they screwed up and didn't have either the sun sword */
			/* or the holy symbol, lets drain'em one level! */

			if(GetMaxLevel(tmp) <=1) {
				mudlog(LOG_CHECK, "Strahd tried to drain char below level 1");
			}
			else {
				act("$N bites you hard, OUCH!!!", FALSE, tmp, 0, ch, TO_CHAR);
				act("$n bites $N who looks drained and weak!", FALSE, ch, 0, tmp,
					TO_ROOM);
				drop_level(tmp,BestClassBIT(tmp),FALSE);
				set_title(tmp);
				do_save(tmp,"",0);
			} /* else GetMaxLevel */
			break;

		/* summon up Strahd Zombie and order it to protect and kill */
		default:
			tmp =ch->specials.fighting;
			mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
			char_to_room(mobtmp, ch->in_room);
			add_follower(mobtmp, ch);
			SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
			act("$n summons one of his faithful followers, $N!", FALSE, ch, 0,
				mobtmp, TO_ROOM);
			do_order(ch, "followers guard on", 0);
			sprintf(buf,"followers kill %s",GET_NAME(tmp));
			do_order(ch,buf,0);
			return(TRUE);
			break;
		} /* end switch */
	} /* end isfighting? */

	if(!ch->specials.fighting) {
		/* is the holy symbol in game? */
		if(location == -1) {
			if((target_obj = get_obj_vis_world(ch, "baabaabowie", NULL)) &&
					target_obj->in_room != NOWHERE) {
				/* items already loaded! */
				location = target_obj->in_room;
				return FALSE;
			}
			else if(vampnum == 0) {
				/* item was not found, lets load up the items and randomise the */
				/* locations!                                                   */

				mudlog(LOG_CHECK, "Loading Ravenloft random location items.");
				/* place sun sword ... */
				/*        if (obj_index[SUN_SWORD_RAVENLOFT].number < MAX_SUN_SWORDS) */

				if((r_num = real_object(SUN_SWORD_RAVENLOFT)) >= 0) {
					target_obj = read_object(r_num, REAL);
					obj_to_room(target_obj,sun_loc[number(0,3)]);
				}

				/* place holy symbol */
				if((r_num = real_object(HOLY_ITEM_RAVENLOFT)) >= 0) {
					target_obj = read_object(r_num, REAL);
					obj_to_room(target_obj,holy_loc[number(0,2)]);
				}

				return(FALSE);
			} /* end loading items random rooms */
		} /* location != -1 */
	} /* end not fighting */
	return(magic_user(mob,cmd,arg,mob,type));
} /* end strahd */

MOBSPECIAL_FUNC(banshee) {

	struct char_data* tmp;

	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		if(IS_DARK(ch->in_room))  {
			for(tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
				if(tmp == ch || IS_IMMORTAL(tmp)) {
					continue;
				}
				else {
					act("$n wails loudly draining the life from your body!", TRUE, ch, 0, 0, TO_ROOM);
					/* do wail damage stuff here */
				}
			} /* end for */


		} /* not dark */   else if(number(0,1))  {
			return(magic_user(mob,cmd,arg,mob,type));
		}
		else  {
			tmp=ch->specials.fighting;
			if(GetMaxLevel(tmp) <=1) {
				mudlog(LOG_CHECK, "Banshee tried to drain char below level 1");
			}
			else { /* if GetmaxLevel */
				act("$n bites $N who looks drained!", FALSE, ch, 0, tmp, TO_ROOM);
				act("$n bites you hard!", FALSE, ch, 0, tmp, TO_CHAR);
				drop_level(tmp,BestClassBIT(tmp),FALSE);
			} /* else GetMaxLevel */
			set_title(tmp);
			do_save(tmp,"",0);
		}

	} /* end she is fighting */

	return(FALSE);
} /* end banshee */



MOBSPECIAL_FUNC(mad_gertruda) {
	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		return(fighter(mob, cmd, arg, mob, type));
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(mad_cyrus) {
	if(cmd) {
		return(FALSE);
	}
	if(!AWAKE(mob)) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		return(fighter(mob, cmd, arg, mob, type));
	}
	return(FALSE);
}

#define IRON_DIR_STOP  EAST
#define IRON_ROOM_STOP 30078
MOBSPECIAL_FUNC(raven_iron_golem) {
	struct char_data* v;

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(cmd == IRON_DIR_STOP && ch->in_room == IRON_ROOM_STOP) {
		return(CheckForBlockedMove(ch,cmd,arg,IRON_ROOM_STOP,IRON_DIR_STOP,0));
	}
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		if(number(0,1)) {

			act("$n belows out a gout of green gas!",TRUE,ch,0,0,TO_ROOM);
			for(v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
				if(ch != v && !IS_IMMORTAL(v)) {
					act("The green gas fills your lungs!.",FALSE,v,0,0,TO_CHAR);
					cast_sleep(GetMaxLevel(ch),v,"",SPELL_TYPE_SPELL, v, 0);
				}
			}            /* end for */
			return(TRUE);
		}
		else {
			return(fighter(mob, cmd, arg, mob, type));
		}
	}

	return(FALSE);
}



#if EGO_BLADE

OBJSPECIAL_FUNC(EvilBlade) {

	struct obj_data* xobj, *blade;
	struct char_data* joe, *holder;
	struct char_data* lowjoe = 0;
	char arg1[128], buf[250];

	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	if(IS_IMMORTAL(ch)) {
		return(FALSE);
	}
	if(!real_roomp(ch->in_room)) {
		return(FALSE);
	}

	for(xobj = real_roomp(ch->in_room)->contents; xobj ; xobj = xobj->next_content) {
		if(xobj->item_number >= 0 &&
				obj_index[xobj->item_number].func == reinterpret_cast<genericspecial_func>(EvilBlade)) {
			/* I am on the floor */
			for(joe = real_roomp(ch->in_room)->people; joe ; joe = joe->next_in_room) {
				if((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
					if(lowjoe) {
						if(GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
							lowjoe = joe;
						}
					}
					else {
						lowjoe = joe;
					}
				}
			}
			if(lowjoe) {
				if(CAN_GET_OBJ(lowjoe, xobj)) {
					obj_from_room(xobj);
					obj_to_char(xobj,lowjoe);

					sprintf(buf,"%s leaps into your hands!\n\r",xobj->short_description);
					send_to_char(buf,lowjoe);

					sprintf(buf,"%s jumps from the floor and leaps into %s's hands!\n\r",
							xobj->short_description,GET_NAME(lowjoe));
					act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);

					if(!EgoBladeSave(lowjoe)) {
						if(!lowjoe->equipment[WIELD]) {
							sprintf(buf,"%s forces you to wield it!\n\r",xobj->short_description);
							send_to_char(buf,lowjoe);
							wear(lowjoe, xobj, 12);
							return(FALSE);
						}
						else {
							sprintf(buf,"You can feel %s attept to make you wield it.\n\r",
									xobj->short_description);
							send_to_char(buf,lowjoe);
							return(FALSE);
						}
					}
				}
			}
		}
	}
	for(holder = real_roomp(ch->in_room)->people; holder ;
			holder = holder->next_in_room) {
		for(xobj = holder->carrying; xobj ; xobj = xobj->next_content) {
			if(xobj->item_number >= 0 && obj_index[xobj->item_number].func &&
					obj_index[xobj->item_number].func != reinterpret_cast<genericspecial_func>(board)) {
				/*held*/
				if(holder->equipment[WIELD]) {
					if((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
						sprintf(buf,"%s gets pissed off that you are wielding another weapon!\n\r",
								xobj->short_description);
						send_to_char(buf,holder);
						sprintf(buf,"%s knocks %s out of your hands!!\n\r",xobj->short_description,
								holder->equipment[WIELD]->short_description);
						send_to_char(buf,holder);
						blade = unequip_char(holder,WIELD);
						if(blade) {
							obj_to_room(blade,holder->in_room);
						}
						if(!holder->equipment[WIELD]) {
							sprintf(buf,"%s forces you to wield it!\n\r",
									xobj->short_description);
							send_to_char(buf,holder);
							wear(holder, xobj, 12);
							return(FALSE);
						}
					}
				}
				if(!EgoBladeSave(holder)) {
					if(!EgoBladeSave(holder)) {
						if(!holder->equipment[WIELD]) {
							sprintf(buf,"%s forces you yto wield it!\n\r",
									xobj->short_description);
							send_to_char(buf,holder);
							wear(holder, xobj, 12);
							return(FALSE);
						}
					}
				}
				if(affected_by_spell(holder,SPELL_CHARM_PERSON)) {
					affect_from_char(holder,SPELL_CHARM_PERSON);
					sprintf(buf,"Due to the %s, you feel less enthused about your master.\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}
			}
		}
		if(holder->equipment[WIELD]) {
			if(holder->equipment[WIELD]->item_number >= 0 &&
					obj_index[holder->equipment[WIELD]->item_number].func &&
					obj_index[holder->equipment[WIELD]->item_number].func != reinterpret_cast<genericspecial_func>(board)) {
				/*YES! I am being held!*/
				xobj = holder->equipment[WIELD];
				if(affected_by_spell(holder,SPELL_CHARM_PERSON)) {
					affect_from_char(holder,SPELL_CHARM_PERSON);
					sprintf(buf,"Due to the %s, you feel less enthused about your master.\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}
				if(holder->specials.fighting) {
					sprintf(buf,"%s almost sings in your hand!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
					sprintf(buf,"You can hear $n's %s almost sing with joy!",xobj->short_description);
					act(buf,FALSE, holder, 0, 0, TO_ROOM);
					if((holder == ch) && (cmd == CMD_FLEE)) {
						if(EgoBladeSave(ch) && EgoBladeSave(ch)) {
							sprintf(buf,"You can feel %s attempt to stay in the fight!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							return(FALSE);
						}
						else {
							sprintf(buf,"%s laughs at your attempt to flee from a fight!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s gives you a little warning...\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s twists around and smacks you!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
									xobj->short_description);
							act(buf,FALSE, ch, 0, 0, TO_ROOM);
							GET_HIT(ch) -= 25;
							alter_hit(ch,0);
							if(GET_HIT(ch) < 0) {
								GET_HIT(ch) = 0;
								alter_hit(ch,0);
								GET_POS(ch) = POSITION_STUNNED;
							}
							return(TRUE);
						}
					}
				}
				if((cmd == CMD_REMOVE) && (holder == ch)) {
					one_argument(arg, arg1);
					if(strcmp(arg1,"all") == 0) {
						if(!EgoBladeSave(ch)) {
							sprintf(buf,"%s laughs at your attempt remove it!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s gives you a little warning...\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s twists around and smacks you!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
									xobj->short_description);
							act(buf,FALSE, ch, 0, 0, TO_ROOM);

							GET_HIT(ch) -= 25;
							alter_hit(ch,0);
							if(GET_HIT(ch) < 0) {
								GET_HIT(ch) = 0;
								alter_hit(ch,0);
								GET_POS(ch) = POSITION_STUNNED;
							}
							return(TRUE);
						}
						else {
							sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							return(FALSE);
						}
					}
					else {
						if(isname(arg1,xobj->name)) {
							if(!EgoBladeSave(ch)) {
								sprintf(buf,"%s laughs at your attempt to remove it!\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"%s gives you a little warning...\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"%s twists around and smacks you!\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
										xobj->short_description);
								act(buf,FALSE, ch, 0, 0, TO_ROOM);

								GET_HIT(ch) -= 25;
								alter_hit(ch,0);
								if(GET_HIT(ch) < 0) {
									GET_HIT(ch) = 0;
									alter_hit(ch,0);
									GET_POS(ch) = POSITION_STUNNED;
								}
								return(TRUE);
							}
							else {
								sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",xobj->short_description);
								send_to_char(buf,ch);
								return(FALSE);
							}
						}
					}
				}
				for(joe = real_roomp(holder->in_room)->people; joe ;
						joe = joe->next_in_room) {
					if((GET_ALIGNMENT(joe) >= 500) &&
							(IS_MOB(joe)) && (CAN_SEE(holder,joe)) && (holder != joe)) {
						if(lowjoe) {
							if(GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
								lowjoe = joe;
							}
						}
						else {
							lowjoe = joe;
						}
					}
				}
				if(lowjoe) {
					if(!EgoBladeSave(holder)) {
						if(GET_POS(holder) != POSITION_STANDING) {
							sprintf(buf,"%s yanks you yo your feet!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							GET_POS(holder) = POSITION_STANDING;
						}
						sprintf(buf,"%s leaps out of control!!\n\r",
								xobj->short_description);
						send_to_char(buf,holder);
						sprintf(buf,"%s jumps for $n's neck!",xobj->short_description);
						act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);
						do_hit(holder,lowjoe->player.name, 0);
						return(TRUE);
					}
					else {
						return(FALSE);
					}
				}
				if((cmd == CMD_HIT) && (holder == ch)) {
					sprintf(buf,"%s almost sings in your hands!!\n\r",
							xobj->short_description);
					send_to_char(buf,ch);
					sprintf(buf,"You can hear $n's %s almost sing with joy!",
							xobj->short_description);

					act(buf,FALSE, ch, 0, 0, TO_ROOM);
					return(FALSE);
				}
			}
		}
	}
	return(FALSE);
}

OBJSPECIAL_FUNC(GoodBlade) {
	struct obj_data* xobj, *blade;
	struct char_data* joe, *holder;
	struct char_data* lowjoe = 0;
	char arg1[128], buf[250];

	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	return(FALSE); /* disabled */

	if(IS_IMMORTAL(ch)) {
		return(FALSE);
	}
	if(!real_roomp(ch->in_room)) {
		return(FALSE);
	}

	for(xobj = real_roomp(ch->in_room)->contents;
			xobj ; xobj = xobj->next_content) {
		if(xobj->item_number >= 0 &&
				obj_index[xobj->item_number].func == reinterpret_cast<genericspecial_func>(GoodBlade)) {
			/* I am on the floor */
			for(joe = real_roomp(ch->in_room)->people; joe ;
					joe = joe->next_in_room) {
				if((GET_ALIGNMENT(joe) >= 350) && (!IS_IMMORTAL(joe))) {
					if(lowjoe) {
						if(GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
							lowjoe = joe;
						}
					}
					else {
						lowjoe = joe;
					}
				}
			}
			if(lowjoe) {
				if(CAN_GET_OBJ(lowjoe, xobj)) {
					obj_from_room(xobj);
					obj_to_char(xobj,lowjoe);

					sprintf(buf,"%s leaps into your hands!\n\r",xobj->short_description);
					send_to_char(buf,lowjoe);

					sprintf(buf,"%s jumps from the floor and leaps into %s's hands!\n\r",
							xobj->short_description,GET_NAME(lowjoe));
					act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);

					if(!EgoBladeSave(lowjoe)) {
						if(!lowjoe->equipment[WIELD]) {
							sprintf(buf,"%s forces you to wield it!\n\r",xobj->short_description);
							send_to_char(buf,lowjoe);
							wear(lowjoe, xobj, 12);
							return(FALSE);
						}
						else {
							sprintf(buf,"You can feel %s attept to make you wield it.\n\r",
									xobj->short_description);
							send_to_char(buf,lowjoe);
							return(FALSE);
						}
					}
				}
			}
		}
	}
	for(holder = real_roomp(ch->in_room)->people; holder ;
			holder = holder->next_in_room) {
		for(xobj = holder->carrying; xobj ; xobj = xobj->next_content) {
			if(xobj->item_number >= 0 && obj_index[xobj->item_number].func &&
					(void*)obj_index[xobj->item_number].func != (void*)board) {
				/*held*/
				if(holder->equipment[WIELD]) {
					if((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
						sprintf(buf,"%s gets pissed off that you are wielding another weapon!\n\r",
								xobj->short_description);
						send_to_char(buf,holder);
						sprintf(buf,"%s knocks %s out of your hands!!\n\r",xobj->short_description,
								holder->equipment[WIELD]->short_description);
						send_to_char(buf,holder);
						blade = unequip_char(holder,WIELD);
						if(blade) {
							obj_to_room(blade,holder->in_room);
						}
						if(!holder->equipment[WIELD]) {
							sprintf(buf,"%s forces you to wield it!\n\r",
									xobj->short_description);
							send_to_char(buf,holder);
							wear(holder, xobj, 12);
							return(FALSE);
						}
					}
				}
				if(!EgoBladeSave(holder)) {
					if(!EgoBladeSave(holder)) {
						if(!holder->equipment[WIELD]) {
							sprintf(buf,"%s forces you yto wield it!\n\r",
									xobj->short_description);
							send_to_char(buf,holder);
							wear(holder, xobj, 12);
							return(FALSE);
						}
					}
				}
				if(affected_by_spell(holder,SPELL_CHARM_PERSON)) {
					affect_from_char(holder,SPELL_CHARM_PERSON);
					sprintf(buf,"Due to the effects of %s, you feel less enthused about your master.\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}
			}
		}
		if(holder->equipment[WIELD]) {
			if(holder->equipment[WIELD]->item_number >= 0 &&
					obj_index[holder->equipment[WIELD]->item_number].func &&
					(void*)obj_index[holder->equipment[WIELD]->item_number].func != (void*)board) {
				/*YES! I am being held!*/
				xobj = holder->equipment[WIELD];

				/* remove charm */
				if(affected_by_spell(holder,SPELL_CHARM_PERSON)) {
					affect_from_char(holder,SPELL_CHARM_PERSON);
					sprintf(buf,"Due to the effects of %s, you feel less enthused about your master.\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}

				/* remove blindness */
				if(affected_by_spell(holder,SPELL_BLINDNESS)) {
					affect_from_char(holder,SPELL_BLINDNESS);
					sprintf(buf,"%s hums in your hands, you can see!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}

				/* remove chill touch */
				if(affected_by_spell(holder,SPELL_CHILL_TOUCH)) {
					affect_from_char(holder,SPELL_CHILL_TOUCH);
					sprintf(buf,"%s hums in your hands, you feel warm again!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}


				/* remove slow */
				if(affected_by_spell(holder,SPELL_SLOW)) {
					affect_from_char(holder,SPELL_SLOW);
					sprintf(buf,"%s hums in your hands, you feel yourself speed back up!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}

				/* remove poison */
				if(affected_by_spell(holder,SPELL_POISON)) {
					affect_from_char(holder,SPELL_POISON);
					sprintf(buf,"%s hums in your hands, the sick feeling fades!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
				}

				/* wielder is hurt, heal them */
				if((number(1,101) > 90) && GET_HIT(holder) < GET_MAX_HIT(holder)/2) {
					act("You get a gentle warm pulse from $p, you feel MUCH better!",FALSE,holder,xobj,0,TO_CHAR);
					act("$n smiles as $p pulses in $s hands!",FALSE,holder,xobj,0,TO_ROOM);
					GET_HIT(holder) = (GET_MAX_HIT(holder)-number(1,10));
					alter_hit(holder,0);
					return(FALSE);
				}

				if(holder->specials.fighting) {
					sprintf(buf,"%s almost sings in your hand!\n\r",
							xobj->short_description);
					send_to_char(buf,holder);
					sprintf(buf,"You can hear %s almost sing with joy in $n's hands!",xobj->short_description);
					act(buf,FALSE, holder, 0, 0, TO_ROOM);
					if((holder == ch) && (cmd == CMD_FLEE)) {
						if(EgoBladeSave(ch) && EgoBladeSave(ch)) {
							sprintf(buf,"You can feel %s attempt to stay in the fight!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							return(FALSE);
						}
						else {
							sprintf(buf,"%s laughs at your attempt to flee from a fight!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s gives you a little warning...\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s twists around and smacks you!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
									xobj->short_description);
							act(buf,FALSE, ch, 0, 0, TO_ROOM);
							GET_HIT(ch) -= 10;
							alter_hit(ch,0);
							if(GET_HIT(ch) < 0) {
								GET_HIT(ch) = 0;
								alter_hit(ch,0);
								GET_POS(ch) = POSITION_STUNNED;
							}
							return(TRUE);
						}
					}
				}
				if((cmd == CMD_REMOVE) && (holder == ch)) {
					one_argument(arg, arg1);
					if(strcmp(arg1,"all") == 0) {
						if(!EgoBladeSave(ch)) {
							sprintf(buf,"%s laughs at your attempt remove it!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s gives you a little warning...\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"%s twists around and smacks you!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
									xobj->short_description);
							act(buf,FALSE, ch, 0, 0, TO_ROOM);

							GET_HIT(ch) -= 10;
							alter_hit(ch,0);
							if(GET_HIT(ch) < 0) {
								GET_HIT(ch) = 0;
								alter_hit(ch,0);
								GET_POS(ch) = POSITION_STUNNED;
							}
							return(TRUE);
						}
						else {
							sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							return(FALSE);
						}
					}
					else {
						if(isname(arg1,xobj->name)) {
							if(!EgoBladeSave(ch)) {
								sprintf(buf,"%s laughs at your attempt to remove it!\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"%s gives you a little warning...\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"%s twists around and smacks you!\n\r",
										xobj->short_description);
								send_to_char(buf,ch);
								sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
										xobj->short_description);
								act(buf,FALSE, ch, 0, 0, TO_ROOM);

								GET_HIT(ch) -= 10;
								alter_hit(ch,0);
								if(GET_HIT(ch) < 0) {
									GET_HIT(ch) = 0;
									alter_hit(ch,0);
									GET_POS(ch) = POSITION_STUNNED;
								}
								return(TRUE);
							}
							else {
								sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",xobj->short_description);
								send_to_char(buf,ch);
								return(FALSE);
							}
						}
					}
				}
				for(joe = real_roomp(holder->in_room)->people; joe ;
						joe = joe->next_in_room) {
					if((GET_ALIGNMENT(joe) <=-350) &&
							(IS_MOB(joe)) && (CAN_SEE(holder,joe)) && (holder != joe)) {
						if(lowjoe) {
							if(GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
								lowjoe = joe;
							}
						}
						else {
							lowjoe = joe;
						}
					}
				}
				if(lowjoe) {
					if(!EgoBladeSave(holder)) {
						if(GET_POS(holder) != POSITION_STANDING) {
							sprintf(buf,"%s yanks you yo your feet!\n\r",
									xobj->short_description);
							send_to_char(buf,ch);
							GET_POS(holder) = POSITION_STANDING;
						}
						sprintf(buf,"%s leaps out of control!!\n\r",
								xobj->short_description);
						send_to_char(buf,holder);
						sprintf(buf,"%s howls out for $n's neck!",xobj->short_description);
						act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);
						do_hit(holder,lowjoe->player.name, 0);
						return(TRUE);
					}
					else {
						return(FALSE);
					}
				}
				if((cmd == CMD_HIT) && (holder == ch)) {
					sprintf(buf,"%s almost sings in your hands!!\n\r",
							xobj->short_description);
					send_to_char(buf,ch);
					sprintf(buf,"You can hear $n's %s almost sing with joy!",
							xobj->short_description);

					act(buf,FALSE, ch, 0, 0, TO_ROOM);
					return(FALSE);
				}
			}
		}
	}

	return(FALSE);
}

OBJSPECIAL_FUNC(NeutralBlade) {
	return(FALSE);
}

#endif

MOBSPECIAL_FUNC(FireBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_fire_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		} /* end for */

		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(FrostBreather) {
	struct char_data* tar_char;
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_frost_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		} /* end for */
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(AcidBreather) {
	struct char_data* tar_char;
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_acid_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(GasBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_gas_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
	}

	return(FALSE);
}


MOBSPECIAL_FUNC(LightningBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_lightning_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(magic_user_imp) {
	struct char_data* vict;
	byte lspell;
	char buf[254];


	if(cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS)) {
		return(FALSE);
	}

	/* might move this somewhere else later... */

	SET_BIT(ch->player.iClass, CLASS_MAGIC_USER);
	ch->player.level[MAGE_LEVEL_IND] = GetMaxLevel(ch);

	/* --- */

	if(!ch->specials.fighting && !IS_PC(ch)) {
		if(GetMaxLevel(ch) < 25) {
			return FALSE;
		}
		else {
			if(!ch->desc) {
				if(Summoner(ch, cmd, arg, mob, type)) {
					return(TRUE);
				}
				else {
					if(NumCharmedFollowersInRoom(ch) < 5 &&
							!too_many_followers(ch) && // SALVO controllo che non puo' superare un limite
							IS_SET(ch->hatefield, HATE_CHAR)) {
						act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
						cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
						do_order(ch, "followers guard on", 0);
						return(TRUE);
					}
				}
			}
			return FALSE;
		}
	}

	if(!ch->specials.fighting) {
		return FALSE;
	}

	if(!IS_PC(ch)) {
		if(GET_POS(ch) > POSITION_STUNNED && GET_POS(ch) < POSITION_FIGHTING) {
			if(GET_HIT(ch) > GET_HIT(ch->specials.fighting) / 2) {
				StandUp(ch);
			}
			else {
				StandUp(ch);
				WAIT_STATE(ch, PULSE_VIOLENCE * 3);
				do_flee(ch, "\0", 0);
			}
			WAIT_STATE(ch, PULSE_VIOLENCE * 3);
			return(TRUE);
		} /* had to stand */
	}

	if(check_nomagic(ch, 0, 0)) {
		return(FALSE);
	}

	if(number(0,1))
		if(UseViolentHeldItem(ch)) {
			return(TRUE);
		}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	/* Find a dude to to evil things upon ! */
	vict = ch->specials.fighting;

	if(!vict) {
		vict = FindVictim(ch);
	}

	if(!vict) {
		return(FALSE);
	}

	lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */
	if(!IS_PC(ch)) {
		lspell+= GetMaxLevel(ch)/5;   /* weight it towards the upper levels of
                                     the mages range */
	}
	lspell = MIN(GetMaxLevel(ch), lspell);

	/*
	**  check your own problems:
	*/

	if(lspell < 1) {
		lspell = 1;
	}

	/* only problem I can see with this new spell casting is if the mobs name
	   is the same as the victim....  */

	if(IS_AFFECTED(ch, AFF_BLIND) && lspell > 15) {
		sprintf(buf,"'remove blind' %s",GET_NAME(ch));
		do_cast(ch,buf,0);
		return TRUE;
	}

	if(IS_AFFECTED(ch, AFF_BLIND)) {
		return(FALSE);
	}

	if(IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 10 &&
			GetMaxLevel(ch) > GetMaxLevel(vict)) {
		sprintf(buf," 'dispel magic' %s",GET_NAME(vict));
		do_cast(ch,buf,0);
		return(FALSE);
	}

	if(IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 10 &&
			GetMaxLevel(ch) > GetMaxLevel(vict)) {
		sprintf(buf," 'dispel magic' %s",GET_NAME(vict));
		do_cast(ch,buf,0);
		return(FALSE);
	}

	if(!IS_PC(ch)) {
		if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 4) && lspell > 28 &&
				!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
			sprintf(buf," 'teleport' %s",GET_NAME(ch));
			do_cast(ch,buf,0);
			return(FALSE);
		}
	}

	if(!IS_PC(ch)) {
		if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 4) && lspell > 15 &&
				!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
			sprintf(buf," 'teleport' %s",GET_NAME(ch));
			do_cast(ch,buf,0);
			return(FALSE);
		}
	}

	if(GET_HIT(ch) > (GET_MAX_HIT(ch) / 2) &&
			!IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
			GetMaxLevel(vict) < GetMaxLevel(ch) && number(0,1)) {
		/*  Non-damaging case: */
		if(((lspell>8) && (lspell<50)) && (number(0,6)==0)) {
			sprintf(buf," 'web' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			return TRUE;
		}

		if(((lspell>5) && (lspell<10)) && (number(0,6)==0)) {
			sprintf(buf," 'weakness' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			return TRUE;
		}

		if(((lspell>5) && (lspell<10)) && (number(0,7)==0)) {
			sprintf(buf," 'armor' %s",GET_NAME(ch));
			do_cast(ch,buf,0);
			return TRUE;
		}

		if(((lspell>12) && (lspell<20)) && (number(0,7)==0)) {
			sprintf(buf," 'curse' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			return TRUE;
		}

		if(((lspell>10) && (lspell < 20)) && (number(0,5)==0)) {
			sprintf(buf," 'blind' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			return TRUE;
		}

		if(lspell>8 && lspell < 40 && number(0,5) == 0 &&
				vict->specials.fighting != ch) {
			sprintf(buf," 'charm monster' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			if(IS_AFFECTED(vict, AFF_CHARM)) {
				if(!vict->specials.fighting) {
					sprintf(buf, "%s kill %s",
							GET_NAME(vict), GET_NAME(ch->specials.fighting));
					do_order(ch, buf, 0);
				}
				else {
					sprintf(buf, "%s remove all", GET_NAME(vict));
					do_order(ch, buf, 0);
				}
			}
		}

		/*  The really nifty case: */
		switch(lspell) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			sprintf(buf," 'monsum one'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 11:
		case 12:
		case 13:
			sprintf(buf," 'monsum two'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 14:
		case 15:
			sprintf(buf," 'monsum three'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 16:
		case 17:
		case 18:
			sprintf(buf," 'monsum four'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 19:
		case 20:
		case 21:
		case 22:
			sprintf(buf," 'monsum five'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 23:
		case 24:
		case 25:
			sprintf(buf," 'monsum six'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		case 26:
		default:
			sprintf(buf," 'monsum seven'");
			do_cast(ch,buf,0);
			do_order(ch, "followers guard on", 0);
			return(TRUE);
			break;
		}
	}
	else {
		switch(lspell) {
		case 1:
		case 2:
			sprintf(buf," 'magic missile' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			break;
		case 3:
		case 4:
		case 5:
			sprintf(buf," 'shocking grasp' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			break;
		case 6:
		case 7:
		case 8:
			if(ch->attackers <= 2) {
				sprintf(buf," 'web' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			else {
				sprintf(buf," 'burning hands' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			break;
		case 9:
		case 10:
			sprintf(buf," 'acid blast' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			break;
		case 11:
		case 12:
		case 13:
			if(ch->attackers <= 2) {
				sprintf(buf," 'lightning bolt' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			else {
				sprintf(buf," 'ice storm' %s", GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			break;
		case 14:
		case 15:
			sprintf(buf," 'teleport' %s",GET_NAME(ch));
			do_cast(ch,buf,0);
			break;
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
			if(ch->attackers <= 2) {
				sprintf(buf," 'colour spray' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			else {
				sprintf(buf," 'cone of cold' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			break;
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
			sprintf(buf," 'fireball' %s",GET_NAME(vict));
			do_cast(ch,buf,0);
			break;
		case 38:
		case 39:
		case 40:
		case 41:
			if(IS_EVIL(ch)) {
				sprintf(buf," 'energy drain' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			break;
		default:
			if(ch->attackers <= 2) {
				sprintf(buf," 'meteor swarm' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			else {
				sprintf(buf," 'fireball' %s",GET_NAME(vict));
				do_cast(ch,buf,0);
			}
			break;
		}
	}
	return TRUE;
}




MOBSPECIAL_FUNC(cleric_imp) {
	struct char_data* vict;
	byte lspell, healperc=0;


	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch)!=POSITION_FIGHTING) {
		if((GET_POS(ch)<POSITION_STANDING) && (GET_POS(ch)>POSITION_STUNNED)) {
			StandUp(ch);
		}
		return FALSE;
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	if(check_nomagic(ch, 0, 0)) {
		return(FALSE);
	}


	if(!ch->specials.fighting) {
		if(GET_HIT(ch) < GET_MAX_HIT(ch)-10) {
			if((lspell = GetMaxLevel(ch)) >= 20) {
				act("$n utters the words 'What a Rush!'.", 1, ch,0,0,TO_ROOM);
				cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
			else if(lspell > 12) {
				act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch,0,0,TO_ROOM);
				cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
			else if(lspell > 8) {
				act("$n utters the words 'I feel much better now!'.", 1, ch,0,0,TO_ROOM);
				cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
			else {
				act("$n utters the words 'I feel good!'.", 1, ch,0,0,TO_ROOM);
				cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
		}
	}


	/* Find a dude to to evil things upon ! */

	if((vict = ch->specials.fighting) == NULL)
		if((vict = FindAHatee(ch)) == NULL)
			if((vict = FindVictim(ch)) == NULL) {
				return FALSE;
			}


	/*
	 *  gen number from 0 to level
	 */

	lspell = number(0,GetMaxLevel(ch));
	lspell+= GetMaxLevel(ch)/5;
	lspell = MIN(GetMaxLevel(ch), lspell);

	if(lspell < 1) {
		lspell = 1;
	}


	if((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 31) &&
			(!IS_SET(ch->specials.act, ACT_AGGRESSIVE))) {
		act("$n utters the words 'Woah! I'm outta here!'",
			1, ch, 0, 0, TO_ROOM);
		vict = FindMobDiffZoneSameRace(ch);
		if(vict) {
			cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			return(TRUE);
		}
		cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		return(FALSE);
	}


	/*
	  first -- hit a foe, or help yourself?
	  */

	if(ch->points.hit < (ch->points.max_hit / 2)) {
		healperc = 7;
	}
	else if(ch->points.hit < (ch->points.max_hit / 4)) {
		healperc = 5;
	}
	else if(ch->points.hit < (ch->points.max_hit / 8)) {
		healperc=3;
	}

	if(number(1,healperc+2)>3) {
		/* do harm */

		/* call lightning */
		if(OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING) && (lspell >= 15) &&
				(number(0,5)==0)) {
			act("$n whistles.",1,ch,0,0,TO_ROOM);
			act("$n utters the words 'Here Lightning!'.",1,ch,0,0,TO_ROOM);
			cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			return(TRUE);
		}

		switch(lspell) {
		case 1:
		case 2:
		case 3:
			act("$n utters the words 'Moo ha ha!'.",1,ch,0,0,TO_ROOM);
			cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 4:
		case 5:
		case 6:
			act("$n utters the words 'Hocus Pocus!'.",1,ch,0,0,TO_ROOM);
			cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 7:
			act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
			cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 8:
			act("$n utters the words 'Urgle Blurg'.",1,ch,0,0,TO_ROOM);
			cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 9:
		case 10:
			act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
			cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 11:
			act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
			cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		case 13:
		case 14:
		case 15:
		case 16: {
			if(!IS_SET(vict->M_immune, IMM_FIRE)) {
				act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
				cast_flamestrike(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			}
			else if(IS_AFFECTED(vict, AFF_SANCTUARY) &&
					(GetMaxLevel(ch) > GetMaxLevel(vict))) {
				act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
				cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			}
			else {
				act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
				cast_cause_critic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL, vict, 0);
			}
			break;
		}
		case 17:
		case 18:
		case 19:
		default:
			act("$n utters the words 'Hurts, doesn't it?!?'.",1,ch,0,0,TO_ROOM);
			cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			break;
		}

		return(TRUE);

	}
	else {
		/* do heal */

		if(IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0,3)==0)) {
			act("$n utters the words 'Praise <Deity Name>, I can SEE!'.", 1, ch,0,0,TO_ROOM);
			cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			return(TRUE);
		}

		if(IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0,6)==0)) {
			act("$n utters the words 'I'm rubber, you're glue.", 1, ch,0,0,TO_ROOM);
			cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			return(TRUE);
		}

		if(IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0,6)==0)) {
			act("$n utters the words 'Praise <Deity Name> I don't feel sick no more!'.", 1, ch,0,0,TO_ROOM);
			cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			return(TRUE);
		}


		switch(lspell) {
		case 1:
		case 2:
			act("$n utters the words 'Abrazak'.",1,ch,0,0,TO_ROOM);
			cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;
		case 3:
		case 4:
		case 5:
			act("$n utters the words 'I feel good!'.", 1, ch,0,0,TO_ROOM);
			cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			act("$n utters the words 'I feel much better now!'.", 1, ch,0,0,TO_ROOM);
			cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch,0,0,TO_ROOM);
			cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;
		case 17:
		case 18: /* heal */
			act("$n utters the words 'What a Rush!'.", 1, ch,0,0,TO_ROOM);
			cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;
		default:
			act("$n utters the words 'Oooh, pretty!'.", 1, ch,0,0,TO_ROOM);
			cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			break;

		}

		return(TRUE);

	}
}

MOBSPECIAL_FUNC(lich_church) {
	if(type == EVENT_DEATH) {
		/* add spec procs here for when he dies */

		return(FALSE);
	}

	if(number(0,1)) {
		return(magic_user_imp(ch,cmd,arg,mob,type));
	}
	else {
		return(shadow(ch,cmd,arg,mob,type));
	}
}
MOBSPECIAL_FUNC(medusa) {
	struct char_data* tar;
	int i;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	for(i=0; i<8; i++) {
		if((tar = FindAHatee(ch)) == NULL) {
			tar = FindVictim(ch);
		}

		if(tar && (tar->in_room == ch->in_room)) {
			if(HitOrMiss(ch, tar, CalcThaco(ch, NULL))) {
				act("$n glares at $N with an evil eye!", 1, ch, 0, tar, TO_NOTVICT);
				act("$n glares at you with and evil eye!", 1, ch, 0, tar, TO_VICT);
				if(!IS_AFFECTED(tar, AFF_PARALYSIS)) {
					cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(Cockatrice) {
	struct char_data* tar;
	int i;

	if(cmd || (type != EVENT_TICK) || !AWAKE(ch)) {
		return(FALSE);
	}

	if(check_peaceful(ch, "")) {
		return FALSE;
	}

	for(i=0; i<8; i++) {
		/* Se � attaccato se la prende con l'avversario, altrimenti cerca qualcuno da beccare */
		if((tar = (ch->specials.fighting)) == NULL) {
			if((tar = FindAHatee(ch)) == NULL) {
				tar = FindVictim(ch);
			}
		}

		if(tar && (tar->in_room == ch->in_room) && !IS_AFFECTED(tar, AFF_PARALYSIS)) {
			if(HitOrMiss(ch, tar, CalcThaco(ch, NULL))) {
				act("Becchi abilmente $N!", 1, ch, 0, tar, TO_CHAR);
				act("$n becca abilmente $N!", 1, ch, 0, tar, TO_NOTVICT);
				act("$n ti becca abilmente e ti viene a mancare il respiro!", 1, ch, 0, tar, TO_VICT);
				cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
				return TRUE;
			}
		}
	}
	return FALSE;
}



MOBSPECIAL_FUNC(goblin_sentry) {
	struct char_data* sentrymob;

	if(!AWAKE(ch)) {
		return(FALSE);
	}


	if(!cmd) {
		if(ch->specials.fighting) {
			fighter(ch, cmd, arg, mob, type);
		}
	}
	else {
		sentrymob = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(goblin_sentry));
		if(cmd >= 1 && cmd <= 6) {
			if(cmd == CMD_SOUTH) {
				return(FALSE);    /* can always go south */
			}
			/* everything else gets ya attacked */
			if(ch->desc && !IS_IMMORTAL(ch)) {
				if(number(0,1) && GET_RACE(ch) != RACE_GOBLIN && CAN_SEE(sentrymob,ch)) {
					act("$N blocks your path and attacks!",FALSE,ch,0,sentrymob, TO_CHAR);
					act("$N growls at $n and attacks!",TRUE, ch, 0, sentrymob, TO_ROOM);
					hit(sentrymob,ch,0);
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}


MOBSPECIAL_FUNC(PostMaster) {

	/*****************************************************************
	** Below is the spec_proc for a postmaster using the above       **
	** routines.  Written by Jeremy Elson (jelson@server.cs.jhu.edu) **
	*****************************************************************/

	if(!ch->desc) {
		return(FALSE);    /* so mobs don't get caught here */
	}

	switch(cmd) {
	case CMD_MAIL: /* mail */
		postmaster_send_mail(ch, cmd, arg);
		return 1;
		break;
	case CMD_CHECK: /* check */
		postmaster_check_mail(ch, cmd, arg);
		return 1;
		break;
	case CMD_RECEIVE: /* receive */
		postmaster_receive_mail(ch, cmd, arg);
		return 1;
		break;
	default:
		return(FALSE);
		break;
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(TreeThrowerMob) {
	struct char_data* vict;

	/*
	**  Throws people in various directions, value:(N=0,E=1,S=2,W=3,U=4,D=5)
	**  for checking a direction, add 1 to value, for throwing use value.
	*/

	if(!cmd) {
		if(AWAKE(ch) && ch->specials.fighting) {
			/*
			**  take this person and throw them
			*/
			vict = ch->specials.fighting;
			switch(ch->in_room) {
			case 13912:  /*Forest of Rhowyn, Bridge Troll*/
				ThrowChar(ch, vict, 1);  /* throw chars to the east */
				return(FALSE);
				break;
			case 6224:   /*Rhyiana, Sentinel Tree*/
				ThrowChar(ch, vict, 2); /*throw chars to the south*/
				return(FALSE);
				break;
			default:
				return(FALSE);
			}
		}
	}
	else {
		switch(ch->in_room) {
		case 13912:  /*Forest of Rhowyn, Bridge Troll*/
			if(cmd == CMD_NORTH) {
				/* north+1 */
				act("$n blocca la tua strada!", FALSE, mob, 0, ch, TO_VICT);
				act("$n blocca la strada di $N.", FALSE, mob, 0, ch, TO_NOTVICT);
				do_say(mob,"Se vuoi passare, dovrai prima prima sconfiggermi!",1);
				do_action(mob,NULL,97); /*evil grin ;) */
				return(TRUE);
			}
			break;
		case 6224:   /*Rhyiana, Sentinel Tree*/
			if((cmd == CMD_NORTH) && (IS_EVIL(ch))) {
				act("Il vento scuote le foglie degli alberi che ti circondano.",FALSE,mob,0,ch,TO_VICT);
				act("Improvvisamente i rami ti raggiungono e ti sbarrano la strada!",
					FALSE,mob,0,ch,TO_VICT);
				act("I rami di un albero gigantesco si muovono improvvisamente, bloccando il cammino di $N!",
					FALSE,mob,0,ch,TO_NOTVICT);
				act("Una voce potente ti risuona nelle orecchie, 'La via a nord e' impedita a coloro che sono impuri di cuore'",FALSE,mob,0,ch,TO_VICT);
				act("Gli alberi sembrano sussurrare parole nel vento.",
					FALSE,mob,0,ch,TO_NOTVICT);
				return(TRUE);
			}
			break;
		default:
			return(FALSE);
		} /*end of switch*/
	}   /*end of else*/
	return(FALSE);
}




/*
 * well, this paladin will give a tough battle..
 * imho paladins just can't be stupid and do _random_ things on yourself
 * and on opponents.. grin

*/

MOBSPECIAL_FUNC(Paladin) {

	struct char_data* vict, *tch;
	char    buf[255];

	if(!ch->skills || GET_LEVEL(ch,PALADIN_LEVEL_IND) <= 0) {
		/* init skills */
		SET_BIT(ch->player.iClass, CLASS_PALADIN);
		if(!ch->skills) {
			SpaceForSkills(ch);
		}
		GET_LEVEL(ch, PALADIN_LEVEL_IND) = GetMaxLevel(ch);     /* needed in do_ */

		/* set skill levels */

		ch->skills[ SKILL_RESCUE ].learned = GetMaxLevel(ch) + 40;
		ch->skills[ SKILL_HOLY_WARCRY ].learned = GetMaxLevel(ch) + 40;
		ch->skills[ SKILL_LAY_ON_HANDS ].learned = GetMaxLevel(ch) + 40;
		ch->skills[ SKILL_BASH ].learned = GetMaxLevel(ch) + 40;
		ch->skills[ SKILL_KICK ].learned = GetMaxLevel(ch) + 40;
		ch->skills[ SKILL_BLESSING ].learned = GetMaxLevel(ch) + 40;

	}     /* done with setting skills */

	if(type == EVENT_COMMAND) {
		/* we will not give free blesses or lay on hands..
		   we will just ignore all commands.. yah. */

		return(FALSE);
	}

	if(affected_by_spell(ch, SPELL_PARALYSIS)) {    /* poor guy.. */
		return(FALSE);
	}

	if(!AWAKE(mob) && !affected_by_spell(mob, SPELL_SLEEP)) {
		/* hey, why you sleeping guy?  STAND AND FIGHT! */
		command_interpreter(mob,"wake");
		command_interpreter(mob,"stand");
		return TRUE;
	}

	if(ch->specials.fighting && ch->specials.fighting != ch) {

		if(GET_POS(ch) == POSITION_SITTING || GET_POS(ch) == POSITION_RESTING) {
			do_stand(ch, "", 0);
			return TRUE;
		}

		vict = ch->specials.fighting;

		if(!vict) {
			mudlog(LOG_SYSERR, "!vict in Paladin");
			return FALSE;
		}


		/* well, if we in battle, do some nice things on ourself..      */
		if(!affected_by_spell(ch, SKILL_BLESSING)) {
			sprintf(buf, "%s", GET_NAME(ch));     /* bless myself */
			do_blessing(ch, buf, 0);
			return TRUE;
		}

		if(!affected_by_spell(ch, SKILL_LAY_ON_HANDS) &&
				GET_HIT(ch) < (GET_MAX_HIT(ch) / 2)) {
			sprintf(buf,"%s",GET_NAME(ch));
			do_lay_on_hands(ch,buf,0);
			return(TRUE);
		}

		switch(number(1, 6)) {
		case 1: /* intellegent kick/bash.. hmm  */
		case 2:
		case 3:
			if(HasClass(vict, CLASS_SORCERER|CLASS_MAGIC_USER) ||
					HasClass(vict, CLASS_CLERIC|CLASS_PSI)) {
				do_bash(ch,"",0);
				return TRUE;
			}
			else {
				do_kick(ch, "", 0);
				return TRUE;
			}
			break;
		case 4:
			do_holy_warcry(ch,"",0);
			return TRUE;
			break;
		default:
			return(fighter(ch, cmd, arg, mob, type));
			break;
		} /* end switch */
	} /* end fighting */
	else {
		/* not FIGHTING */

		/* check our hps */
		if(GET_HIT(ch) < GET_MAX_HIT(ch) / 2 &&
				!affected_by_spell(ch, SKILL_LAY_ON_HANDS)) {
			sprintf(buf,"%s",GET_NAME(ch));
			do_lay_on_hands(ch,buf,0);
			return(TRUE);
		}
		if(GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && GET_MANA(ch) > GET_MANA(ch) / 2 &&
				number(1, 6) > 4) {
			cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			return(TRUE);
		}

		/* lets check some spells on us.. */
		if(IS_AFFECTED(ch, AFF_POISON) && number(0, 6) == 0 &&
				GET_LEVEL(ch, PALADIN_LEVEL_IND) > 10) {
			act("$n asks $s diety to remove poison from $s blood!", 1, ch, 0, 0,
				TO_ROOM);
			if(GET_LEVEL(ch, PALADIN_LEVEL_IND) < 40) {
				cast_slow_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
			else {
				cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
			}
			return(TRUE);
		}

		if(!IS_AFFECTED(ch, AFF_PROTECT_FROM_EVIL) && number(0, 6) == 0) {
			act("$n prayed to $s diety to protect $m from evil.", 1, ch, 0, 0,
				TO_ROOM);
			cast_protection_from_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch,
									  0);
			return(TRUE);
		}

		/* i dont like GreetPeople behaivor.. lets make it this way */

		if(IS_SET(ch->specials.act, ACT_GREET)) {
			for(tch = real_roomp(ch->in_room)->people; tch;
					tch = tch->next_in_room) {
				if(!IS_NPC(tch) && !number(0, 4)) {
					if(GetMaxLevel(tch) > 5 && CAN_SEE(ch, tch)) {
						if(GET_ALIGNMENT(tch) >= 900) {
							sprintf(buf, "bow %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							if(tch->player.sex == SEX_FEMALE) {
								do_say(ch, "Greetings, noble m'lady!",0);
							}
							else {
								do_say(ch, "Greetings, noble sir!",0);
							}
						}
						else if(GET_ALIGNMENT(tch) >= 350) {
							sprintf(buf, "smile %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							do_say(ch, "Greetings, adventurer",0);
						}
						else if(GET_ALIGNMENT(tch) >= -350) {
							sprintf(buf, "wink %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							do_say(ch, "Your doing well on your path of Neutrality", 0);
						}
						else if(GET_ALIGNMENT(tch) >= -750) {
							sprintf(buf, "nod %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							do_say(ch, "May the prophet smile upon you",0);
						}
						else if(GET_ALIGNMENT(tch) >= -900) {
							sprintf(buf, "wink %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							do_say(ch, "You falling in hands of evil, beware!", 0);
						}
						else {
							/* hmm, not nice guy.. */
							sprintf(buf, "glare %s", GET_NAME(tch));
							command_interpreter(ch, buf);
							do_say(ch, "I sense great evil here!", 0);
						}
						SET_BIT(ch->specials.act, ACT_GREET);
						break;
					} /* endif of can_see */
				}
			}
		}
		else if(!number(0, 50)) {
			REMOVE_BIT(ch->specials.act, ACT_GREET);
		}
	}       /* end not fighting */
	return(FALSE);
}


/* psi stuff here */

/* PSI_CAN is a shorthand for me, use ONLY for PSIs*/
#define PSI_CAN(skill,level) (spell_info[(skill)].min_level_psi <= (level) && spell_info[(skill)].min_level_psi > 0) // SALVO controllo spell_info

/* NOTAFF_N_LEARNED more shorthand, this one is general*/
#define NOTAFF_N_LEARNED(ch,spell,skill) (!affected_by_spell((ch),(spell))        && ((ch)->skills[(skill)].learned))

/* SP(el)L_(and target's)N(a)ME :: puts spell name and target's name together*/
#define SPL_NME(tname,spell) (strcat( strcat( strcat( strcpy(buf,"'"),         spells[(spell)]), "' "), (tname)))

/* (Psychic)C(rush)_OR_B(last) if psi can do a crush he does, if not blast!*/
#define C_OR_B(ch,vict) ((ch)->skills[SKILL_PSYCHIC_CRUSH].learned ?         (mind_psychic_crush(PML,(ch),(vict),NULL)) :         (do_blast((ch),(vict)->player.name,1)) )

/* CAST_OR_BLAST checks to see if psi can cast special attack spell, if not
   it will either do a psyhic crush or it will do a psionic blast */
#define CAST_OR_BLAST(ch,vict,spell) ((ch)->skills[(spell)].learned ?         do_cast((ch), SPL_NME((vict)->player.name, (spell)-1),370) :         C_OR_B((ch),(vict)))


/* Bugs: if 2 mobs of same name in room, may cause problems.
         fix add macro to add those "-" between parts and check for number of
         mobs with same name in room and figure out which one you are!!!
       * add teleport, prob travel, and disguise */

MOBSPECIAL_FUNC(Psionist) {
	int PML;          /*psi mob's level*/
	int Qmana;        /*psi mob's 1/4 mana*/
	int cmana;        /*psi mob's current mana level*/
	int hpcan;        /*safe to canibalize/summon*/
	char buf[100];    /*all purpose buffer*/
	struct char_data* targ;  /*who the psi_mob is fighting*/
	int group;        /* does targ have followers or is grouped? */

	if(cmd) {
		return(FALSE);
	}


	if(!AWAKE(mob) && !affected_by_spell(mob,SKILL_MEDITATE))
		/*no sleeping on the job!!!!*/
	{
		/* NOTE: this also prevents mobs who are incap from casting*/
		command_interpreter(mob,"wake");
		command_interpreter(mob,"stand");
		return(TRUE);
	}

	if(!IS_SET(mob->player.iClass, CLASS_PSI)) {
		SET_BIT(mob->player.iClass, CLASS_PSI);
		PML = GET_LEVEL(mob,PSI_LEVEL_IND) = GetMaxLevel(mob);
		SpaceForSkills(mob);
		/* SetSkillLevels first defensive, general spells. */
		/* I set all of these so that gods running quests can use the
		   psi mobs skills, unlike most other special procedure mobs*/
		if(PSI_CAN(SKILL_CANIBALIZE,PML)) {
			mob->skills[SKILL_CANIBALIZE].learned= MIN(95,10+dice(10,(int)PML/2));
		}
		if(PSI_CAN(SKILL_CELL_ADJUSTMENT,PML)) {
			mob->skills[SKILL_CELL_ADJUSTMENT].learned = MIN(95,10+dice(4,PML));
		}
		if(PSI_CAN(SKILL_CHAMELEON,PML)) {
			mob->skills[SKILL_CHAMELEON].learned = MIN(95,50+number(2,PML));
		}
		if(PSI_CAN(SKILL_FLAME_SHROUD,PML)) {
			mob->skills[SKILL_FLAME_SHROUD].learned = MIN(95,50+dice(5,PML));
		}
		if(PSI_CAN(SKILL_GREAT_SIGHT,PML)) {
			mob->skills[SKILL_GREAT_SIGHT].learned = MIN(95,33+dice(4,PML));
		}
		if(PSI_CAN(SKILL_INVIS,PML)) {
			mob->skills[SKILL_INVIS].learned = MIN(95,50+dice(3,PML));
		}
		if(PSI_CAN(SKILL_MEDITATE,PML)) {
			mob->skills[SKILL_MEDITATE].learned = MIN(95,33+dice((int)PML/5,20));
		}
		if(PSI_CAN(SKILL_MINDBLANK,PML)) {
			mob->skills[SKILL_MINDBLANK].learned = MIN(95,50+dice(4,PML));
		}
		if(PSI_CAN(SKILL_PORTAL,PML)) {
			mob->skills[SKILL_PORTAL].learned = MIN(95,40+dice(4,PML));
		}
		if(PSI_CAN(SKILL_PROBABILITY_TRAVEL,PML)) {
			mob->skills[SKILL_PROBABILITY_TRAVEL].learned=MIN(95,35+dice(3,PML));
		}
		if(PSI_CAN(SKILL_PSI_SHIELD,PML)) {
			mob->skills[SKILL_PSI_SHIELD].learned = MIN(95,66+dice(10,(int) PML/3));
		}
		if(PSI_CAN(SKILL_PSI_STRENGTH,PML)) {
			mob->skills[SKILL_PSI_STRENGTH].learned = MIN(95,dice((int)PML/5,20));
		}
		if(PSI_CAN(SKILL_PSYCHIC_IMPERSONATION,PML)) {
			mob->skills[SKILL_PSYCHIC_IMPERSONATION].learned=MIN(95,66+dice(2,PML));
		}
		if(PSI_CAN(SKILL_SUMMON,PML)) {
			mob->skills[SKILL_SUMMON].learned = MIN(95,66+PML);
		}
		if(PSI_CAN(SKILL_TOWER_IRON_WILL,PML)) {
			mob->skills[SKILL_TOWER_IRON_WILL].learned = MIN(95,50+dice(4,PML));
		}
		/*ATTACK -- TYPE SPELLS BEGIN HERE*/
		if(PSI_CAN(SKILL_MIND_BURN,PML)) {
			mob->skills[SKILL_MIND_BURN].learned = MIN(95,33+dice(2,PML));
		}
		if(PSI_CAN(SKILL_DISINTEGRATE,PML)) {
			mob->skills[SKILL_DISINTEGRATE].learned = MIN(95,33+dice(3,PML));
		}
		if(PSI_CAN(SKILL_MIND_WIPE,PML)) {
			mob->skills[SKILL_MIND_WIPE].learned = MIN(95,45+dice(2,PML));
		}
		if(PSI_CAN(SKILL_PSIONIC_BLAST,PML)) {
			mob->skills[SKILL_PSIONIC_BLAST].learned= MIN(99,33+dice((int)PML/10,30));
		}
		if(PSI_CAN(SKILL_PSYCHIC_CRUSH,PML)) {
			mob->skills[SKILL_PSYCHIC_CRUSH].learned = MIN(99,66+dice(2,PML));
		}
		if(PSI_CAN(SKILL_TELEKINESIS,PML)) {
			mob->skills[SKILL_TELEKINESIS].learned = MIN(95,dice((int)PML/10,30));
		}
		if(PSI_CAN(SKILL_PSI_TELEPORT,PML)) {
			mob->skills[SKILL_PSI_TELEPORT].learned = MIN(95,dice((int)PML/10,30));
		}
		if(PSI_CAN(SKILL_ULTRA_BLAST,PML)) {
			mob->skills[SKILL_ULTRA_BLAST].learned = MIN(99,66+dice(3,PML));
		}
		GET_MANA(mob) = 100;
	}

	/* Aarcerak's little bug fix.. sitting mobs can't cast/mind/etc.. */
	if(mob->specials.fighting) {
		if((GET_POS(mob)<POSITION_FIGHTING) && (GET_POS(mob)>POSITION_STUNNED)) {
			StandUp(mob);
			return (TRUE);
		}
	}


	if(affected_by_spell(mob,SPELL_FEEBLEMIND)) {
		if(!IS_AFFECTED(mob,AFF_HIDE)) {
			act("$n waits for $s death blow impatiently.",FALSE,mob,0,0,TO_ROOM);
		}
		return(TRUE);
	}

	if(GET_POS(mob) == POSITION_SITTING ||
			GET_POS(mob) == POSITION_RESTING) {
		do_stand(mob,"",0);
		return(TRUE);
	}

	PML = GET_LEVEL(mob,PSI_LEVEL_IND);

	if(NOTAFF_N_LEARNED(mob,SPELL_FIRESHIELD,SKILL_FLAME_SHROUD)) {
		do_flame_shroud(mob,mob->player.name,1);
		return(TRUE);
	}

	if(affected_by_spell(mob,SKILL_MEDITATE)) {
		if(GET_MANA(mob) <= .75*mob->points.max_mana) {
			return(TRUE);    /*regaining mana*/
		}
		else {
			command_interpreter(mob,"stand");
			return(TRUE);
		}
	}
	if(IS_AFFECTED(mob,AFF_HIDE)) {
		return (FALSE);    /*hiding, break fer flame shrd*/
	}

	Qmana = 51;
	hpcan = (int)(.75 * mob->points.max_hit);
	cmana = GET_MANA(mob);

	if(!mob->specials.fighting) {
		if(PSI_CAN(SKILL_CELL_ADJUSTMENT, PML) && (cmana > Qmana) &&
				(mob->points.hit < hpcan)) {
			do_say(mob,"That was too close for comfort.",1);
			mind_teleport(PML,mob,mob,NULL);
			mind_cell_adjustment(PML, mob, mob, NULL);
			return(TRUE);
		}
		if(cmana <= Qmana) {
			if((mob->points.hit>(hpcan+1))&&(mob->skills[SKILL_CANIBALIZE].learned)) {
				if((cmana + 2*(mob->points.hit - hpcan)) >= mob->points.max_mana) {
					sprintf(buf,"24");    /*Qmana=51>=cm, cm+(2*24) <= 99 */
				}
				else {
					sprintf(buf,"%d",(mob->points.hit - hpcan -1));
				}
				do_canibalize(mob,buf,1);
				return(TRUE);
			}
			else if(mob->skills[SKILL_MEDITATE].learned) {
				do_meditate(mob,mob->player.name,1);
				return(TRUE);
			}
		}
		if(NOTAFF_N_LEARNED(mob,SKILL_PSI_SHIELD,SKILL_PSI_SHIELD)) {
			do_psi_shield(mob,mob->player.name,1);
		}
		else if(NOTAFF_N_LEARNED(mob,SKILL_MINDBLANK,SKILL_MINDBLANK)) {
			mind_mindblank(PML,mob,mob,NULL);
		}
		else if(NOTAFF_N_LEARNED(mob,SKILL_TOWER_IRON_WILL,SKILL_TOWER_IRON_WILL)) {
			mind_tower_iron_will(PML,mob,mob,NULL);
		}
		else if(NOTAFF_N_LEARNED(mob,SPELL_SENSE_LIFE,SKILL_GREAT_SIGHT)) {
			do_great_sight(mob,mob->player.name,1);
		}
		else if(NOTAFF_N_LEARNED(mob,SKILL_PSI_STRENGTH,SKILL_PSI_STRENGTH)) {
			mind_psi_strength(PML, mob, mob, NULL);
		}
		else if(IS_SET(mob->hatefield, HATE_CHAR) && (mob->points.hit>hpcan)) {
			do_say(mob,"It's payback time!",1);
			GET_MANA(mob) = 100;
			if(PSI_CAN(SKILL_PORTAL,PML) || PSI_CAN(SKILL_SUMMON,PML)) {
				return(Summoner(mob,0,NULL,mob,0));
			}
		}
		else if(NOTAFF_N_LEARNED(mob,SPELL_INVISIBLE,SKILL_INVIS)) {
			do_invisibililty(mob,mob->player.name,1);
		}
		else if(mob->skills[SKILL_CHAMELEON].learned) {
			mind_chameleon(PML,mob,mob,NULL);
		}
		return(TRUE);
	} /* end peace time castings */
	else {
		/*they are fighting someone, do something nasty to them!*/
		GET_MANA(mob) = 100; /*some psi combat spells still cost mana,
                             set to max mana start of every round of combat*/
		targ = mob->specials.fighting;
		if((mob->points.max_hit-hpcan) > (1.5 * mob->points.hit)) {
			if(!mob->skills[SKILL_PSI_TELEPORT].learned || (!IsOnPmp(mob->in_room))) {
				act("$n looks around frantically.",0,mob,0,0,TO_ROOM);
				command_interpreter(mob,"flee");
				return(TRUE);
			}
			act("$n screams defiantly, 'I'll get you yet, $N!'",0,mob,0,targ,TO_ROOM);
			mind_teleport(PML,mob,mob,0);
			return(TRUE);
		}

		group = ((targ->followers || targ->master) ? TRUE : FALSE);

		if(group && (dice(1,2)-1)) {
			group = FALSE;
		}
		if(!group) {
			/* not fighting a group, or has selected person fighting, for spec*/
			if(dice(1,2) - 1) {   /* do special attack 50% of time */
				if(IS_SET(targ->player.iClass, CLASS_MAGIC_USER | CLASS_CLERIC)) {
					if(dice(1, 2) - 1) {
						CAST_OR_BLAST(mob, targ, SKILL_TELEKINESIS);
					}
					else {
						CAST_OR_BLAST(mob, targ, SKILL_MIND_WIPE);
					}
				}
				else if(IS_SET(targ->player.iClass, CLASS_PSI | CLASS_SORCERER)) {
					/*special attack for psi & sorc opponents */
					if(affected_by_spell(targ, SPELL_FEEBLEMIND)) {
						CAST_OR_BLAST(mob, targ, SKILL_DISINTEGRATE);
					}
					else {
						CAST_OR_BLAST(mob, targ, SKILL_MIND_WIPE);
					}
				}
				else if(GetMaxLevel(targ) < 20 && (dice(1,2) - 1))
					/*special attack for fighter subtypes & thieves*/
				{
					CAST_OR_BLAST(mob,targ,SKILL_PSI_TELEPORT);
				}
				else {
					CAST_OR_BLAST(mob,targ,SKILL_DISINTEGRATE);
				}
			}
			else {
				C_OR_B(mob,targ);    /* norm attack, psychic crush or psionic blast*/
			}
		}
		else if(mob->skills[SKILL_ULTRA_BLAST].learned) {
			mind_ultra_blast(PML,mob,targ,NULL);
		}
		else if(mob->skills[SKILL_MIND_BURN].learned) {
			mind_burn(PML, mob, targ, NULL);
		}
		else {
			do_blast(mob, targ->player.name, 1);
		}
		return(TRUE);
	} /* end of fighting stuff */
	return(TRUE);
}
/*-------------------------end Psionist-------------------------*/
/* church bell for ators zone in town */
#define PULL        224
ROOMSPECIAL_FUNC(ChurchBell) {
	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(cmd==PULL) {
		char buf[128];
		arg=one_argument(arg,buf); /* buf == object */

		if(!strcmp("rope",buf)) {
			send_to_all("The bells of Shadow Springs's church sound 'GONG! GONG! GONG! GONG!'\n");
			return(TRUE);
		} /* end strcmpi() */
		return(FALSE);
	} /* end pull */

	return(FALSE);
}


/* end church bell */

#define SLAV_DEAD_DONE_ROOM        3497                /* where to tel people when he is killed Gaia 2001 ex 3494 */
#define SLAV_LIVE_ROOM             3496        /* where he SHOULD live */
MOBSPECIAL_FUNC(Slavalis) {

	if(cmd /* || !AWAKE(mob) */) { // Gaia 2001
		return(FALSE);
	}

	if(type == EVENT_DEATH && ch->in_room == SLAV_LIVE_ROOM) {
		struct char_data* v;
		act("$n screams at you horridly 'I will return again!'",FALSE,ch,0,0,TO_ROOM);
		/* move all pc's to different room */
		for(v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
			if(v != ch && !IS_IMMORTAL(v)) {
				act("You get a quezzy feeling as you fall into a swirling mist.\nYou arrive back on your home plane!",FALSE,v,0,ch,TO_CHAR);
				act("$n fades away blinking in curiousity.",FALSE,v,0,ch,TO_ROOM);
				char_from_room(v);
				char_to_room(v,SLAV_DEAD_DONE_ROOM);
				do_look(v,"\0",15);
			}
		} /* end for */


		/* ding bell... */

		send_to_all("The chruch bells of Myst sound off in a chorus of happiness!\n"); // Gaia 2001

		return(TRUE);
	} /* end event dead! */


	if(ch->specials.fighting) {
		if((GET_POS(ch) < POSITION_FIGHTING) &&
				(GET_POS(ch) > POSITION_STUNNED)) {
			StandUp(ch);
		}
		switch(number(1,2)) {
		case 1:
			return(Demon(ch, cmd, arg, mob, type));
			break;
		case 2:
			return(Psionist(ch, cmd, arg, mob, type));
			break;
		default:
			return(Demon(ch, cmd, arg, mob, type));
			break;
		} /* end switch */

	} /* fighting */   else {
		return(magic_user(ch, cmd, arg, mob, type));
	} /* else not fighting */

}

/* berserker sword */
#define BERSERK 329
OBJSPECIAL_FUNC(BerserkerItem) {

	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		/* fighting! berserk! */
		if(!ch->equipment[WIELD]) {  /* nothing wielded */
			return(FALSE);
		}

		if(ch->equipment[WIELD] != obj) {          /* we are not wielded */
			return(FALSE);
		}

		if(!IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
			mudlog(LOG_CHECK, "trying to berserk because of item ");
			/* not berserked go berserk! */
			do_berserk(ch,"",0);
			return(FALSE);
		}
		else {
			/* already berserked */
			if(number(1, 10) == 1) {
				act("$n seems enraged to the point of exploding!", FALSE, ch, 0, 0,
					TO_ROOM);
				act("Your anger whells up inside, you fling yourself at your opponent!",  // Gaia 2001
					FALSE, ch, 0, 0, TO_CHAR);
			}
			return(FALSE);
		}
	}
	return(FALSE);
}

/* end berserker sword */

OBJSPECIAL_FUNC(AntiSunItem) {

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(OUTSIDE(ch) && weather_info.sunlight == SUN_LIGHT &&
			weather_info.sky<= SKY_CLOUDY &&
       !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) &&
       !IS_AFFECTED(ch, AFF_GLOBE_DARKNESS)) {
		/* frag the item! */
		act("The sun strikes $p, causing it to fall appart!", FALSE, ch, obj,
			NULL, TO_CHAR);
		act("The sun strikes $p worn by $n, causing it to fall appart!", FALSE,
			ch, obj, NULL, TO_ROOM);
		MakeScrap(ch, 0, obj);
		return TRUE; /* if not TRUE mud will CRASH! */
	}

	return FALSE;
}

MOBSPECIAL_FUNC(Beholder) {
	int action=0;

	if(type != EVENT_TICK || !AWAKE(mob)) {
		return FALSE;
	}

	if(mob->specials.fighting) {

		if(GET_POS(mob) == POSITION_SITTING) {
			if(ch->specials.fighting) {
				GET_POS(mob) = POSITION_FIGHTING;
			}
			else {
				GET_POS(mob) = POSITION_STANDING;
			}
		}

		action = number(1, 100);

		if(action >= 90) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_fear(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
					  mob->specials.fighting, 0);
			return TRUE;
		}
		else if(action >= 80) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_slow(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
					  mob->specials.fighting, 0);
			return TRUE;
		}
		else if(action >= 75) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_disintegrate(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
							  mob->specials.fighting, 0);
			return TRUE;
		}
		else if(action >= 50) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_dispel_magic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
							  mob->specials.fighting, 0);
			return TRUE;
		}
		else if(action >= 30) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_charm_monster(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
							   mob->specials.fighting, 0);
			return TRUE;
		}
		else if(action >= 10) {
			act("$n turns one of $m eye stalks at $N!", FALSE, mob, NULL,
				mob->specials.fighting, TO_NOTVICT);
			act("$n turns a evil looking eye stalk at you!", FALSE, mob, NULL,
				mob->specials.fighting, TO_VICT);
			cast_harm(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
					  mob->specials.fighting, 0);
			return TRUE;
		}
		else {
			return magic_user(ch, cmd, arg, mob, type);
		}
	} /* end he was fighting */

	return magic_user(ch, cmd, arg, mob, type);
}

/* Generic Archer */

int range_estimate(struct char_data* ch, struct obj_data* o, int type) {
	int r, sz, w = o->obj_flags.weight;

	/* Type 0 = fireweapon 1 = thrown */
	if(w>100) {
		sz = 3;
	}
	else if(w>25) {
		sz = 2;
	}
	else if(w>5) {
		sz = 1;
	}
	else {
		sz = 0;
	}
	r = (((GET_STR(ch)+GET_ADD(ch)/30)-3)/8)+2;
	r /= sz+1;
	if(!type) {
		r += ch->equipment[WIELD]->obj_flags.value[2];
	}
	/* r = r*2/3;*/
	return r;
}

int pick_archer_target(struct char_data* ch, int maxr,
					   struct char_data** targ, int* rn, int* dr) {
	int i, r, rm;
	struct char_data* ptarg;

	if(maxr > 5) {
		maxr=5;
	}
	for(i = 0; i < 6; i++) {
		rm = ch->in_room;
		r = 0;
		while(r <= maxr) {
			if(clearpath(ch, rm, i)) {
				r++;
				rm = real_roomp(rm)->dir_option[i]->to_room;
				for(ptarg = real_roomp(rm)->people; ptarg;
						ptarg = ptarg->next_in_room) {
					if(ptarg != ch && CAN_SEE(ch, ptarg)) {
						/* find PC's to kill if we are aggressive */
						if(IS_PC(ptarg) &&
								!IS_SET(ptarg->specials.act, PLR_NOHASSLE) &&
								(IS_SET(ch->specials.act, ACT_AGGRESSIVE) ||
								 IS_SET(ch->specials.act, ACT_META_AGG)) &&
								!IS_AFFECTED(ptarg, AFF_SNEAK)) {
							*targ = ptarg;
							*rn = r;
							*dr = i;
							return(TRUE);
						}
						/* find people we are hunting */
						else if(IS_SET(ch->specials.act,ACT_HUNTING) &&
								((ch->specials.hunting &&
								  ch->specials.hunting == ptarg) ||
								 (ch->specials.charging &&
								  ch->specials.charging == ptarg))) {
							*targ = ptarg;
							*rn = r;
							*dr = i;
							return(TRUE);
						}
						/* find hated peoples */
						else if(Hates(ch,ptarg)) {
							*targ = ptarg;
							*rn = r;
							*dr = i;
							return(TRUE);
						}
					}
				}
			}
			else {
				r = maxr+1;
			}
		}
	}
	return FALSE;
}

/* added - changed */
int archer_sub(struct char_data* ch) {
	struct obj_data* bow = NULL, *missile = NULL, *thrown = NULL,
						 *spid = NULL, *obj_object = NULL, *next_obj = NULL;
	int r, trng, tdir, found;
	struct char_data* td;


	if(ch->equipment[ WIELD ] &&
			ch->equipment[ WIELD ]->obj_flags.type_flag == ITEM_FIREWEAPON) {
		bow = ch->equipment[WIELD];
		if(ch->equipment[ LOADED_WEAPON ]) {
			missile = ch->equipment[ LOADED_WEAPON ];
		}
		else {
			/* Search inventory for a missile */
			for(spid = ch->carrying; spid; spid = spid->next_content) {
				if(spid->obj_flags.type_flag == ITEM_MISSILE &&
						spid->obj_flags.value[ 3 ] == bow->obj_flags.value[ 3 ]) {
					missile = spid;
				}
				else {
					/* see if they are carrying a quiver full of arrows,
					   if so get an arrow */
					if(GET_ITEM_TYPE(spid) == ITEM_CONTAINER) {
						found = FALSE;
						for(obj_object = spid->contains; obj_object && !found;
								obj_object = next_obj) {
							next_obj = obj_object->next_content;
							if(obj_object->obj_flags.type_flag == ITEM_MISSILE &&
									obj_object->obj_flags.value[ 3 ] ==
									bow->obj_flags.value[ 3 ]) {
								/* gets arrow out of quiver, next round they will load it */
								get(ch, obj_object, spid);
								found = TRUE;
							}
						}
					}
				}
			}
			/* If you found a missile, load it and return */
			if(missile) {
				do_weapon_load(ch, missile->name, 0);
				return TRUE;
			}
		}
		if(missile) {
			/* Pick a target and fire */
			r = range_estimate(ch, missile, 0);
			if(pick_archer_target(ch, r, &td, &trng, &tdir)) {
				missile = unequip_char(ch, LOADED_WEAPON);
				act("Lanci $p verso $N.", TRUE, ch, missile, td, TO_CHAR);
				act("$n lancia $p!", TRUE, ch, missile, 0, TO_ROOM);
				throw_weapon(missile, tdir, td, ch);
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}
	/* Try and find a missile weapon and wield it */
	if(!bow) {
		for(spid = ch->carrying; spid; spid = spid->next_content) {
			if(spid->obj_flags.type_flag == ITEM_FIREWEAPON) {
				bow = spid;
			}
			if(GET_ITEM_TYPE(spid) == ITEM_CONTAINER) {
				found = FALSE;
				for(obj_object = spid->contains; obj_object && !found;
						obj_object = next_obj) {
					next_obj = obj_object->next_content;
					if(obj_object->obj_flags.type_flag == ITEM_FIREWEAPON) {
						/* gets bow out of container */
						get(ch, obj_object, spid);
						found = TRUE;
						bow = obj_object;
					}
				}
			}
		}
		if(bow) {
			if(ch->equipment[ WIELD ]) {
				do_remove(ch, ch->equipment[ WIELD ]->name, 0);
				return TRUE;
			}
			else {
				do_wield(ch, bow->name, 0);
				return TRUE;
			}
		}
	}

	/* No missile weapon or no ammo.  Try a thrown weapon */
	for(spid = ch->carrying; spid; spid = spid->next_content) {
		if(CAN_WEAR(spid, ITEM_THROW)) {
			thrown = spid;
		}
	}
	if(!thrown) {
		struct obj_data* pObj;
		for(pObj = real_roomp(ch->in_room)->contents; pObj;
				pObj = pObj->next_content) {
			if(IS_WEAPON(pObj) && CAN_GET_OBJ(ch, pObj) &&
					CAN_WEAR(pObj, ITEM_THROW)) {
				if(!special(ch, CMD_GET, fname(pObj->name))) {
					obj_from_room(pObj);
					obj_to_char(pObj, ch);
					act("$n raccoglie $p.", TRUE, ch, pObj, NULL, TO_ROOM);
					return TRUE;
				}
			}
		}

		/* Just can't do nothing. */
		return FALSE;
	}
	/* Got a throw weapon, throw it. */
	r = range_estimate(ch, thrown, 1);
	if(pick_archer_target(ch, r, &td, &trng, &tdir)) {
		obj_from_char(thrown);
		act("Lanci $p verso $N.", TRUE, ch, thrown, td, TO_CHAR);
		act("$n lancia $p!", TRUE, ch, thrown, 0, TO_ROOM);
		throw_weapon(thrown, tdir, td, ch);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int archer_hth(struct char_data* ch) {
	struct obj_data* spid, *hth=NULL;

	/* What to do if you are an archer and find yourself in HTH combat */
	/* I. If you are wielding a bow ditch it */
	if(ch->equipment[ WIELD ] &&
			ch->equipment[ WIELD ]->obj_flags.type_flag == ITEM_FIREWEAPON) {
		do_remove(ch, ch->equipment[ WIELD ]->name, 66);
		return TRUE;
	}
	else {
		if(ch->equipment[WIELD]) {
			return FALSE;
		}
		for(spid = ch->carrying; spid; spid = spid->next_content) {
			if(spid->obj_flags.type_flag == ITEM_WEAPON) {
				hth = spid;
			}
		}
		if(hth) {
			do_wield(ch, hth->name, 14);
			return TRUE;
		}
		return FALSE;
	}
}

MOBSPECIAL_FUNC(archer) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_FIGHTING) {
		return(archer_hth(ch));
	}
	else {
		return(archer_sub(ch));
	}

}

MOBSPECIAL_FUNC(fighter_mage) {
	if(number(1,100) >49) {
		return(fighter(ch,cmd,arg,mob,type));
	}
	else {
		return(magic_user(ch,cmd,arg,mob,type));
	}
}

MOBSPECIAL_FUNC(fighter_cleric) {
	if(number(1,100) >49) {
		return(fighter(ch,cmd,arg,mob,type));
	}
	else {
		return(cleric(ch,cmd,arg,mob,type));
	}
}

MOBSPECIAL_FUNC(cleric_mage) {
	if(number(1,100) >49) {
		return(cleric(ch,cmd,arg,mob,type));
	}
	else {
		return(magic_user(ch,cmd,arg,mob,type));
	}
}

MOBSPECIAL_FUNC(Ranger) {
	return(fighter(ch,cmd,arg,mob,type));
}

MOBSPECIAL_FUNC(Barbarian) {
	int percent=0;

	if((ch->specials.fighting) && (GET_POS(ch) == POSITION_FIGHTING)
			&& (!IS_SET(ch->specials.affected_by2,AFF2_BERSERK))) {
		percent = (GET_HIT(ch) / GET_MAX_HIT(ch)) * 100;
		if(
			((percent == 100) && (number(1,100)<=60)) ||
			((percent >= 60) && (percent < 100) && (number(1,100)<=75)) ||
			(percent < 60)
		) {
			do_berserk(ch,"",0);
			return(TRUE);
		}
		else {
			return(fighter(ch,cmd,arg,mob,type));
		}
	}
	else {
		return(fighter(ch,cmd,arg,mob,type));
	}
}

/***************************************************************************
  ObjIsOnGround restituisce TRUE se l'oggetto in arg e` nella stanza del
  carattere.
***************************************************************************/
int ObjIsOnGround(struct char_data* ch, const char* argument) {
	char arg1[ MAX_STRING_LENGTH ];
	char arg2[ MAX_STRING_LENGTH ];

	argument_interpreter(argument, arg1, arg2);
	if(*arg1) {
		if(*arg2) {
			if(str_cmp(arg2, "all") == 0 ||
					(get_obj_in_list_vis(ch, arg2, ch->carrying) == NULL &&
					 get_obj_in_list_vis(ch, arg2,
										 real_roomp(ch->in_room)->contents) != NULL)) {
				return TRUE;
			}
		}
		else { /* !*arg2 */
			if(str_cmp(arg1, "all")) {
				if(getall(arg1, arg2) == TRUE || getabunch(arg1, arg2) != 0) {
					strcpy(arg1, arg2);
				}
				if(get_obj_in_list_vis(ch, arg1,
									   real_roomp(ch->in_room)->contents) != NULL) {
					return TRUE;
				}
			}
			else {
				return TRUE;
			}

		}
	}
	return FALSE;
}

/***************************************************************************
 * ObjectsGuardian implementa un guardiano che non permette di prendere
 * oggetti dalla stanza in cui e` posto.
 *
 * Prima avverte e la seconda volta che qualcuno prova un get, attacca.
 * Dopo due ore virtuali si dimentica chi erano gli avvertiti ed,
 * eventualmente, li avverte di nuovo.
 *
 * Inoltre, se si posa un oggetto che vale piu` di 1000 monete, si viene
 * curati, liberati dalla magia e dal veleno.
 ***************************************************************************/


#define MINVALUETOHEAL 1000
MOBSPECIAL_FUNC(PrimoAlbero) {
	typedef struct tagAvvertiti {
		struct char_data* pAvvertito;
		struct tagAvvertiti* pNext;
		int nTimer;
	} Avvertiti;

	Avvertiti* pCurr;

	if(mob == NULL || ch == NULL) {
		mudlog(LOG_SYSERR,
			   "Grave errore in ObjectsGuardian (spec_procs3). ch or mob == NULL");
		return FALSE;
	}

	if(type == EVENT_COMMAND && (cmd == CMD_GET || cmd == CMD_TAKE) &&
			!mob->specials.fighting && AWAKE(mob) && !IS_IMMORTAL(ch) &&
			CAN_SEE(mob, ch) && ch != mob) {
		if(ObjIsOnGround(ch, arg)) {
			for(pCurr = (Avvertiti*) mob->act_ptr; pCurr; pCurr = pCurr->pNext) {
				/* Se il carattere che ha dato il comando GET e` gia` stato avvertito,
				 * viene attaccato dal mob */
				if(pCurr->pAvvertito == ch) {
					act("$c0015$n$c0009 grida a $N 'Ti avevo avvertito!'", FALSE, mob, 0,
						ch, TO_NOTVICT);
					act("$c0015$n$c0009 grida verso di te 'Ti avevo avvertito!'", FALSE,
						mob, 0, ch, TO_VICT);
					hit(mob, ch, TYPE_UNDEFINED);
					break;
				}
			}
			if(pCurr == NULL) {
				/* Non e` stato avvertito. Lo avverte e lo inserisce nella lista
				 * degli avvertiti. La testa della lista e` in mob->act_ptr */
				CREATE(pCurr, Avvertiti, 1);
				pCurr->pNext = (Avvertiti*)mob->act_ptr;
				mob->act_ptr = pCurr;
				pCurr->pAvvertito = ch;
				pCurr->nTimer = (SECS_PER_MUD_HOUR * 8) / PULSE_MOBILE;
				/* Due ore virtuali */

				act("$c0015$n$c0011 punta un ramo verso $N e dice "
					"'Ti avverto, non toccare nulla!'", FALSE,
					mob, 0, ch, TO_NOTVICT);
				act("$c0015$n$c0011 punta un ramo verso te e dice "
					"'Ti avverto, non toccare nulla!'", FALSE,
					mob, 0, ch, TO_VICT);
			}
			return TRUE;
		}
	}
	else if(type == EVENT_COMMAND && cmd == CMD_DROP && !IS_IMMORTAL(ch) &&
			ch != mob) {
		char szObjName[ MAX_INPUT_LENGTH ], szDummy[ MAX_INPUT_LENGTH ];
		one_argument(arg, szObjName);
		if(getabunch(szObjName, szDummy) <= 1 &&
				str_cmp(szObjName, "all") != 0) {
			struct obj_data* pObject = get_obj_in_list_vis(ch, arg, ch->carrying);
			if(pObject) {
				if(!IS_OBJ_STAT(pObject, ITEM_NODROP) || IS_IMMORTAL(ch)) {
					act("Posi $p.", 1, ch, pObject, 0, TO_CHAR);
					act("$n posa $p.", 1, ch, pObject, 0, TO_ROOM);
					obj_from_char(pObject);
					obj_to_room(pObject,ch->in_room);
					check_falling_obj(pObject, ch->in_room);

					if(pObject->obj_flags.cost >= MINVALUETOHEAL) {
						send_to_room("Dai rami degli alberi di pietra si forma un "
									 "vortice di luci scintillanti.\n\r", ch->in_room);
						act("Il piccolo vortice scintillante, dopo un attimo di "
							"indecisione, avvolge $n", 0, ch, 0, 0, TO_ROOM);
						act("Il piccolo vortice scintillante, dopo un attimo di "
							"indecisione, ti avvolge", 0, ch, 0, 0, TO_CHAR);
						spell_dispel_magic(MAESTRO_DEI_CREATORI, mob, ch, NULL);
						spell_remove_poison(MAESTRO_DEI_CREATORI, mob, ch, NULL);
						spell_heal(MAESTRO_DEI_CREATORI, mob, ch, NULL);
					}
				}
				else {
					if(singular(pObject))
						send_to_char("Non puoi lasciarlo, deve essere MALEDETTO!\n\r",
									 ch);
					else
						send_to_char("Non puoi lasciarli devono essere MALEDETTI!\n\r",
									 ch);
				}
			}
			else {
				send_to_char("Non hai niente del genere.\n\r", ch);
			}
		}
		else {
			act("$c0015$n$c0011 punta un ramo verso $N e dice "
				"'Puoi posare un solo oggetto alla volta.'", FALSE,
				mob, 0, ch, TO_NOTVICT);
			act("$c0015$n$c0011 punta un ramo verso te e dice "
				"'Puoi posare un solo oggetto alla volta.'", FALSE,
				mob, 0, ch, TO_VICT);
		}
		return TRUE;
	}
	else if(type == EVENT_COMMAND && cmd == CMD_FLEE && !NO_HASSLE(ch) &&
			ch != mob) {
		if(number(10, 25) > GET_DEX(ch)) {
			send_to_char("Inciampi nelle grosse radici dell'albero e cadi in "
						 "terra.\n\r", ch);
			act("$n cerca di fuggire ma inciampa nelle grosse radici dell'albero.",
				TRUE, ch, NULL, NULL, TO_ROOM);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			GET_POS(ch) = POSITION_SITTING;
			return TRUE;
		}
	}
	else if(type == EVENT_TICK) {
		/* Ad ogni EVENT_TICK diminuisce il timer di ogni avvertito.
		 * Quando il timer raggiunge lo zero il carattere viene tolto
		 * dalla lista */
		Avvertiti* pNextCurr;
		for(pCurr = (Avvertiti*)mob->act_ptr; pCurr; pCurr = pNextCurr) {
			pNextCurr = pCurr->pNext;
			pCurr->nTimer--;
			if(pCurr->nTimer == 0) {
				if(pCurr == (Avvertiti*)mob->act_ptr) {
					mob->act_ptr = pCurr->pNext;
				}
				else {
					Avvertiti* pTmp;
					for(pTmp = (Avvertiti*)mob->act_ptr; pTmp && pTmp->pNext != pCurr;
							pTmp = pTmp->pNext);
					if(pTmp) {
						pTmp->pNext = pCurr->pNext;
					}
				}
				free(pCurr);
			}
		}

		if(mob->specials.hunting) {
			mob->persist = 0;
			mob->specials.hunting = NULL;
			mob->hunt_dist = 0;
		}

		if(IS_SET(mob->specials.act, ACT_SENTINEL) &&
				mob->lStartRoom > 0 && mob->lStartRoom != mob->in_room) {
			// Torna a casa se non lo e` gia`.
			if(SentinelBackHome(mob)) {
				return TRUE;
			}
		}
	}
	else if(type == EVENT_DEATH) {
		/* In caso di morte del mob, libera la memoria. */
		while(mob->act_ptr != NULL) {
			pCurr = (Avvertiti*)mob->act_ptr;
			mob->act_ptr = pCurr->pNext;
			free(pCurr);
		}
	}
	return FALSE;
}

/***************************************************************************
 * LegionariV viene attaccata ai legionari della V legione.
 * Se attaccati si chiamano fra loro, altrimenti attaccano se qualcuno
 * ha l'insegna della V legione (oggetto #14009)
 ***************************************************************************/
#define OBJ_INSEGNA5LEGIO 14009
#define IsLegionarioV( ch ) ( (ch)->nr == 12014 || (ch)->nr == 12015 ||                               (ch)->nr == 12016 || (ch)->nr == 12017 ||                               (ch)->nr == 12021 || (ch)->nr == 12038 ||                               (ch)->nr == 12045 )

MOBSPECIAL_FUNC(LegionariV) {
	if(type == EVENT_TICK && AWAKE(mob)) {
		if(mob->specials.fighting && !check_soundproof(mob)) {
			if(number(1, 6) > 3) {
				char szBuf[ 180 ];
				struct char_data* pMob;
				int nNumber;

				sprintf(szBuf, "$c0015%s$c0009 grida 'Legionari a me! "
						"Ho bisogno del vostro aiuto'\n\r",
						GET_NAME_DESC(mob));
				CAP(szBuf);
				send_to_zone(szBuf, mob);

				for(pMob = character_list, nNumber = 3; pMob && nNumber;
						pMob = pMob->next) {
					if(IS_NPC(pMob) && IsLegionarioV(pMob) && pMob != mob &&
							!pMob->specials.fighting &&
							!IS_SET(pMob->specials.act, ACT_HUNTING) &&
							number(1, 3) == 1) {
						SetHunting(pMob, mob->specials.fighting);
						nNumber--;
					}
				}
			} /* if( number( 1, 6 ) > 3 ) */
		} /* if( mob->specials.fighting && !check_soundproof( mob ) ) */
		else if(!mob->specials.fighting) {
			for(struct char_data* p = real_roomp(mob->in_room)->people; p;
					p = p->next_in_room) {
				if(CAN_SEE(mob, p)) {
					struct obj_data* pObj;
					for(pObj = p->carrying; pObj; pObj = pObj->next_content) {
						if(CAN_SEE_OBJ(mob, pObj) && pObj->item_number >= 0 &&
								obj_index[ pObj->item_number ].iVNum == OBJ_INSEGNA5LEGIO) {
							act("$c0015$n$c0011 punta il dito verso $N e dice "
								"'Ma quella e` la nostra insegna'.", FALSE, mob, 0, p,
								TO_ROOM);
							hit(mob, p, TYPE_UNDEFINED);
							return TRUE;
						}
					}
				}
			}
		} /* if( !mob->specials.fighting ) */
	} /* if( type == EVENT_TICK && AWAKE( mob ) ) */
	return FALSE;
}


/****************************************************************************
*  Teleporta i presenti se all`interno di certi livelli. Room Procedure
* ****************************************************************************/
ROOMSPECIAL_FUNC(sTeleport) {
	const char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=room->specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if(type == EVENT_COMMAND) {
		if((cmd != ndir) ||
				((GetMaxLevel(ch)>=nlev1) && (GetMaxLevel(ch)<=nlev2))) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}

			sprintf(lev2,"%s\r\n",msg);
			send_to_char(lev2,ch);
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************
*  Blocca il passaggio in una certa direzione. Room Procedure
****************************************************************************/
ROOMSPECIAL_FUNC(BlockWay) {
	const char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=room->specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if(type == EVENT_COMMAND) {
		if((cmd != ndir) || ((GetMaxLevel(ch)>=nlev1) && (GetMaxLevel(ch)<=nlev2))) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}

			sprintf(lev2,"%s\r\n",msg);
			send_to_char(lev2,ch);
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************
*  Blocca il passaggio in una certa direzione. Mob/Obj Procedure
****************************************************************************/
MOBSPECIAL_FUNC(MobBlockWay) {
	const char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=mob_index[mob->nr].specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if(type == EVENT_COMMAND) {
		if((cmd != ndir) ||
				((GetMaxLevel(ch)>=nlev1) && (GetMaxLevel(ch)<=nlev2))) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			sprintf(lev2,"%s\r\n",msg);
			act(msg, FALSE, mob, 0, ch, TO_VICT);
			act("$n dice qualcosa a $N.", FALSE, mob, 0, ch, TO_NOTVICT);
			return TRUE;
		}
	}
	return FALSE;
}
MOBSPECIAL_FUNC(spTest) {
	char msg[256];
	sprintf(msg,"Char=%s Mob=%s Arg=%s evento=%d comando=%d",
			GET_NAME(ch),GET_NAME(mob),arg,type,cmd);
	act(msg, FALSE, mob, 0, ch, TO_ROOM);
	return(FALSE);
}

MOBSPECIAL_FUNC(Esattore) {
	long xp;
	char buf[512];
	if(cmd != CMD_GIVE) {
		return(FALSE);
	}
	/* Questo mob consente di togliersi xp. Viene usato da
	 * superni 58 - 59 per esigere i pagamenti in xp
	 * Per evitare abusi, deve essere il PC stesso che dona gli xp
	 * */
	xp=abs(atoi(arg)/HowManyClasses(ch));
	if(xp > GET_EXP(ch)) {
		do_say(mob,"Non hai abbastanza esperienza",CMD_SAY);
		return(FALSE);
	}
	GET_EXP(ch)-=(xp);
	sprintf(buf,"%s ha donato %ld punti esperienza (per ogni classe) per la gloria del Clan",
			GET_NAME(ch),xp);
	do_shout(mob,buf,CMD_SAY);
	return(TRUE);
}

MOBSPECIAL_FUNC(spGeneric) {
	const char* p;
	char msg[256];
	if(mob == ch) {
		return(FALSE);
	}
	p=mob_index[mob->nr].specparms;
	sprintf(msg,"Char=%s Mob=%s Arg=%s evento=%d comando=%d parms=%s",
			GET_NAME(ch),GET_NAME(mob),arg,type,cmd,p?p:"");
	act(msg, FALSE, mob, 0, ch, TO_ROOM);
	return(FALSE);
}


MOBSPECIAL_FUNC(ForceMobToAction)
/*Specparms:
* ncmd1: comando (numero) trigger
* trg1: 1=si, 0=no(se si, l'oggetto del comando deve essere il mob che ha la
*       special )
* nmob: vnum del mob che deve eseguire l`azione (0 = self)
* cmd2: comando (testo)
* trg2: 1 sul pc, 0 senza oggetto
* msg: testo libero comando da eseguire dopo l`azione principale
* il comando viene costruito cosi`
* primo comando: nmob esegue cmd2 [pc se trg2]
* secondo comando: msg
* */

{
	const char* p;
	char cmd1[256];
	char trg1[256];
	char mobnum[256];
	char cmd2[256];
	char trg2[256];
	char msg[256];
	char actinfo[256];
	char buf[1024];
	int ncmd1,ntrg1,nmob,ntrg2;
	struct char_data* killer;
	p=mob_index[mob->nr].specparms;
	p=one_argument(p,cmd1);
	p=one_argument(p,trg1);
	p=one_argument(p,mobnum);
	p=one_argument(p,cmd2);
	p=one_argument(p,trg2);
	only_argument(p,msg);
	arg=one_argument(arg,actinfo);
	ncmd1=atoi(cmd1);
	ntrg1=atoi(trg1);
	nmob=atoi(mobnum);
	ntrg2=atoi(trg2);
	if(type == EVENT_COMMAND) {
		if((cmd == ncmd1) && (ch!=mob)) {
			if(!ntrg1 || (!strncasecmp(GET_NAME(mob),actinfo,strlen(actinfo)) && strlen(actinfo))) {
				/* Il comando aveva come target il mob della special! */
				killer=FindMobInRoomWithVNum(real_roomp(ch->in_room)->number,
											 nmob);
				if(!killer) {
					return(FALSE);
				}
				if(ntrg2) {
					sprintf(buf,"%s %s",cmd2,GET_NAME(ch));
					command_interpreter(killer,buf);
					sprintf(buf,"%s",msg);
					command_interpreter(killer,buf);
				}
				else {
					sprintf(buf,"%s",msg);
					command_interpreter(killer,buf);
					sprintf(buf,"%s %s",cmd2,GET_NAME(ch));
					command_interpreter(killer,buf);
				}

				return TRUE;
			}

		}
	}
	return FALSE;
}
/* Agganciata ad un oggetto questa procedura permette di ingannare identify.
 * Infatti il danno dell'arma NON E` quello caricato nelle stat dell'oggetto,
 * ma quello restituito da questa special
 * */
OBJSPECIAL_FUNC(TrueDam) {
	int numero=0;
	int faccie=0;
	int bonus=0;
	char buf[50];
	if(type != EVENT_FIGHTING) {
		return(FALSE);
	}
	sscanf(obj_index[cmd].specparms," %dd%d %d",&numero,&faccie,&bonus);
	if(IsTest()) {
		sprintf(buf,"Applicato dam da proc: %dd%d %d\n\r",numero,faccie,bonus);
		act(buf, FALSE, ch, 0, 0, TO_CHAR);
	}

	return (dice(numero,faccie)+bonus);
}

void trap_obj_damage(struct char_data* v, int damtype, int amnt, struct obj_data* t) {
	amnt = SkipImmortals(v, amnt,damtype);
	if(amnt == -1) {
		return;
	}

	if(IS_AFFECTED(v, AFF_SANCTUARY)) {
		amnt = MAX((int)(amnt/2), 0);    /* Max 1/2 damage when sanct'd */
	}

	amnt = PreProcDam(v, damtype, amnt, -1);

	if(saves_spell(v, SAVING_PETRI)) {
		amnt = MAX((int)(amnt/2),0);
	}

	DamageStuff(v, damtype, amnt, 5);

	amnt=MAX(amnt,0);

	GET_HIT(v)-=amnt;
	alter_hit(v,0);

	update_pos(v);

	//TrapDam(v, damtype, amnt, t);

	InformMess(v);
	if(GET_POS(v) == POSITION_DEAD) {
		if(!IS_NPC(v)) {
			if(real_roomp(v->in_room)->name) {

				mudlog(LOG_PLAYERS, "%s killed by an obj_trap at %s",
					   GET_NAME(v), real_roomp(v->in_room)->name);
				/*ALAR: DEATH */
			}

		}
		else {
			mudlog(LOG_CHECK, "%s killed by an obj_trap.", GET_NAME_DESC(v));
		}

		die(v, 0, NULL);
	}
}

OBJSPECIAL_FUNC(trap_obj) {
	struct char_data* v;
	int tipo=0;
	int tick=0;
	int level=0;
	int true_type=0;
	char msg[256];
	const char* p;
	char ctipo[256];
	char ctick[256];
	char clevel[256];

	if(!ch) {
		//funziona solo se droppato
		if(obj->in_room == NOWHERE) {
			return(FALSE);
		}

		p=obj_index[obj->item_number].specparms;
		p=one_argument(p,ctipo);
		p=one_argument(p,ctick);
		p=one_argument(p,clevel);
		only_argument(p,msg);
		tipo=atoi(ctipo);
		tick=atoi(ctick);
		level=atoi(clevel);
		level = level * number(1,4);

		obj->obj_flags.value[0]++;
		if(obj->obj_flags.value[0] == tick) {
			obj->obj_flags.value[0] = 0;

			switch(tipo) {
			case 1:
				true_type = SPELL_FIREBALL;
				break;
			case 2:
				true_type = SPELL_ACID_BLAST;
				break;
			case 3:
				true_type = SPELL_CONE_OF_COLD;
				break;
			case 4:
				true_type = SPELL_LIGHTNING_BOLT;
				break;
			case 5:
				true_type = SPELL_COLOUR_SPRAY;
				break;
			case 6:
				true_type = TYPE_BLUDGEON;
				break;
			case 7:
				true_type = TYPE_PIERCE;
				break;
			case 8:
				true_type = TYPE_SLASH;
				break;
			case 9:
				true_type = 0;
				break;
			case 10:
				true_type = 0;
				break;
			case 11:
				true_type = 0;
				break;
			case 12:
				true_type = 0;
				break;
			case 13:
				true_type = 0;
				break;
			default:
				break;
			}

			for(v = real_roomp(obj->in_room)->people; v; v = v->next_in_room) {
				send_to_char("\n\r",v);
				send_to_char(msg,v);
				send_to_char("\n\r",v);
				trap_obj_damage(v,true_type,level,obj);
			}

		}
	}

	return (FALSE);
}

/* Idem come sorap, ma modifica il to-hit */
OBJSPECIAL_FUNC(ModHit) {
	int bonus=0;
	char buf[50];
	if(type != EVENT_FIGHTING) {
		return(FALSE);
	}
	sscanf(obj_index[cmd].specparms," %d ",&bonus);
	if(IsTest()) {
		sprintf(buf,"Applicato bonus da proc: %d\n\r",bonus);
		act(buf, FALSE, ch, 0, 0, TO_CHAR);
	}
	return (bonus);
}

#define WARLOCK_MOB 18419
#define WARLOCK_ROOM 18463
MOBSPECIAL_FUNC(banshee_lorelai) {
	struct char_data* pWarlock;
	struct char_data* vict;

	if(type == EVENT_DEATH && ch->in_room == WARLOCK_ROOM) {
		if((pWarlock = read_mobile(real_mobile(WARLOCK_MOB), REAL))) {
			char_to_room(pWarlock, WARLOCK_ROOM);
			act("Come $n muore, senti un lamento agghiacciante e il cadavere nella bara, "
				"in preda alle convulsioni, riesce angosciosamente a mettersi "
				"in posizione eretta.", FALSE, mob, NULL, pWarlock, TO_ROOM);
			act("$c0015[$c0013$n$c0015] dice 'Maledetti, avete ucciso la mia Principessa!\n\r"
				"La vostra ricompensa sara` la $c0011MORTE$c0015!'",
				FALSE, pWarlock, NULL, NULL, TO_ROOM);

			vict = FindVictim(pWarlock);
			if(!vict) {
				return(FALSE);
			}

			/*WAIT_STATE(pWarlock, PULSE_VIOLENCE*2); non lagga il mob */
			do_hit(pWarlock, GET_NAME(vict), 0);

		}
		return(TRUE);
	}

	if(type != EVENT_TICK) {
		return(FALSE);
	}

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	return(banshee(ch,cmd,arg,mob,type));

}
    
/* Procedure per Nilmys */
    
#define NILMYS_MOB 8916
#define NILMYS_ROOM 8957
MOBSPECIAL_FUNC(stanislav_spirit) {
	struct char_data* pNilmys;
	struct obj_data* object;
	int r_num;

	if(type == EVENT_DEATH && ch->in_room == NILMYS_ROOM) {
		if((pNilmys = read_mobile(real_mobile(NILMYS_MOB), REAL))) {
			char_to_room(pNilmys, NILMYS_ROOM);
			
			r_num = real_object(8922);
			object = read_object(r_num, REAL);
			obj_to_char(object, pNilmys);
			obj_from_char(object);
			equip_char(pNilmys, object, WEAR_BODY);
			
			r_num = real_object(8933);
			object = read_object(r_num, REAL);
			obj_to_char(object, pNilmys);
			obj_from_char(object);
			equip_char(pNilmys, object, WEAR_ARMS);
			
			r_num = real_object(8917);
			object = read_object(r_num, REAL);
			obj_to_char(object, pNilmys);
			
			act("\n\rAlla morte di $n una tetra ed eterea figura si leva dal suo cadavere,\n"
			    "la sua anima ormai corrotta appartiene ad Arkhat, e brama $c0009sangue$c0015!", FALSE, mob, NULL, pNilmys, TO_ROOM);
			act("$c0015[$c0013$n$c0015] dice 'Non raggiungerete mai Boris\n\r"
				"e i suoi compagni... oggi perirete per mano mia!'",FALSE, pNilmys, NULL, NULL, TO_ROOM);

		}
		return(TRUE);
	}

	if(type != EVENT_TICK) {
		return(FALSE);
	}

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	return(fighter(mob,cmd,arg,mob,type));

}

/*
	Diamo un po' di vita ad alcuni mob! Con questa procedura un mob marchiato come "Interact" reagisce a
	specifici comandi dati da un piggi *** Flyp 20020204 ***
*/

MOBSPECIAL_FUNC(Interact) {
	struct char_data* interattivo;
	char argument[128];

	if(type != EVENT_COMMAND) {
		return TRUE;
	}

	interattivo = FindMobInRoomWithFunction(mob->in_room, reinterpret_cast<genericspecial_func>(Interact));

	if(!interattivo) {
		return TRUE;
	}

	/*
	per ora basta fare cosi', visto che l'unico mobbo utilizzato e'
	Drunky a Myst. Se si usassero anche altri mobbi con altre spec
	Considerare il check anche sul nome *** Flyp 20030204 ***
	*/

	if(cmd==CMD_PRACTICE) {
		return(sailor(ch, cmd, arg, mob, type));
	}


	if(cmd == CMD_ASK) {
		only_argument(arg, argument);

		if(!CAN_SEE(mob,ch)) {
			act("$N ti dice 'Non do retta a chi non vedo...!'.", FALSE, ch, 0, mob, TO_CHAR);
			return TRUE ;
		}

		if(strstr(argument,"info")) {
			if(GetMaxLevel(ch) >= NOVIZIO && GetMaxLevel(ch) < ALLIEVO) {
				act("$N ti dice: 'Perche' non provi a picchiare i burocrati, dovrebbe essere divertente'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= ALLIEVO && GetMaxLevel(ch) < APPRENDISTA) {
				act("$N ti dice: 'ALLIEVO!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= APPRENDISTA && GetMaxLevel(ch) < INIZIATO) {
				act("$N ti dice: 'APPRENDISTA!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= INIZIATO && GetMaxLevel(ch) < ESPERTO) {
				act("$N ti dice: 'INIZIATO!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= ESPERTO && GetMaxLevel(ch) < MAESTRO) {
				act("$N ti dice: 'ESPERTO!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= MAESTRO && GetMaxLevel(ch) < BARONE) {
				act("$N ti dice: 'MAESTRO!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= BARONE && GetMaxLevel(ch) < PRINCIPE) {
				act("$N ti dice: 'BARONE!!'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
			if(GetMaxLevel(ch) >= PRINCIPE) {
				act("$N ti dice: 'Sei abbastanza esperto per sapere dove andare... O almeno lo dovresti essere...'", FALSE, ch, 0, mob, TO_CHAR);
				return TRUE;
			}
		}
		else {
			act("$N ti dice: 'Machecazz' stai a di'?!?'!!", FALSE, ch, 0, mob, TO_CHAR);
			return TRUE;
		}

		if(strstr(argument,"nani")) {
			act("$N ti dice 'Partendo da myst:\n\rSegui  la strada carovaniera dell'est...\n\rquando le montagne ti sbarreranno il passaggio,\n\recco che guardando verso nord potrai intravedere l'opera dei nani...\n\rma sta attento perche' non sono i soli a vivere da quelle parti...'",
				FALSE, ch, 0, mob, TO_CHAR);
			return TRUE;
		}
		else {
			act("$N ti dice: 'Machecazz' stai a di'?!?'!!", FALSE, ch, 0, mob, TO_CHAR);
			return TRUE;
		}

	}
	return FALSE;
}


// ACIDUS 2003 - Fucina, da usare con skill Miner
#define NUM_COMB 10     //Numero di combinazioni esistenti
#define N_TAB 76        //Lunghezza massima della tabella dei componenti
MOBSPECIAL_FUNC(Capo_Fucina) {
	struct obj_data* obj;
	int iVNum, found, i, k, c, r_num, built;
	struct tab_lingotti {
		int comb;                 //numero combinazione
		int vnum;                 //vnum del componente richiesto
		struct obj_data* oggetto; //oggetto richiesto
	};

	//Tabella delle combinazioni possibili
	struct tab_lingotti ctab[N_TAB]= {
		{1,19508,NULL},
		{1,19508,NULL},
		{1,19508,NULL},
		{1,19508,NULL},
		{1,19508,NULL},
		{2,19507,NULL},
		{2,19507,NULL},
		{2,19507,NULL},
		{2,19507,NULL},
		{2,19507,NULL},
		{3,19506,NULL},
		{3,19506,NULL},
		{3,19506,NULL},
		{3,19506,NULL},
		{3,19506,NULL},
		{4,19505,NULL},
		{4,19505,NULL},
		{4,19505,NULL},
		{4,19505,NULL},
		{4,19505,NULL},
		{5,19504,NULL},
		{5,19504,NULL},
		{5,19504,NULL},
		{5,19504,NULL},
		{5,19504,NULL},
		{6,19500,NULL},
		{6,19500,NULL},
		{6,19500,NULL},
		{6,19500,NULL},
		{6,19500,NULL},
		{7,19500,NULL},
		{7,19500,NULL},
		{7,19500,NULL},
		{7,19504,NULL},
		{7,19504,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{8,19501,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{9,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19502,NULL},
		{10,19503,NULL},
		{10,19503,NULL},
	};

	long lingotto[NUM_COMB+1];   //tabella dei lingotti prodotti dalle varie combinazioni
	lingotto[1]=19538;
	lingotto[2]=19539;
	lingotto[3]=19540;
	lingotto[4]=19541;
	lingotto[5]=19542;
	lingotto[6]=19543;
	lingotto[7]=19544;
	lingotto[8]=19546;  //test fatto nel loop per aggiungere gli altri 3 lingotti
	lingotto[9]=19547;
	lingotto[10]=19548;


	if(type != EVENT_COMMAND) {
		return(TRUE);
	}


	if(cmd == CMD_NOD) {
		built = FALSE;
		k = 1;
		while(k <= NUM_COMB) {
			//Testo la combinazione k
			obj = mob->carrying;
			while(obj) {
				iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
				found = FALSE;
				i = 0;
				while(!found && (i<N_TAB)) {
					if((ctab[i].comb == k) && (ctab[i].vnum == iVNum) && !(ctab[i].oggetto)) {
						ctab[i].oggetto = obj;
						found = TRUE;
					}
					i = i+1;
				}
				obj = obj->next_content;
			}
			found = TRUE; //Appena trovo un vnum che non ha l'oggetto associato, faccio fallire
			i = 0;
			while(found && (i<N_TAB)) {
				if((ctab[i].comb == k) && !(ctab[i].oggetto)) {
					found = FALSE;
				}
				i = i+1;
			}
			//Se � tutto ok cancello i componenti e creo il lingotto
			if(found) {
				i = 0;
				while(i<N_TAB) {
					if((ctab[i].comb == k) && (ctab[i].oggetto)) {
						obj_from_char(ctab[i].oggetto);
						extract_obj(ctab[i].oggetto);
					}
					i = i+1;
				}
				if(k == 8) {  //do i 3 lingotti di piombo
					for(c = 0; c < 3; c++) {
						r_num = real_object(19545);
						obj = read_object(r_num, REAL);
						obj_to_char(obj,ch);
					}
				}

				r_num = real_object(lingotto[k]);
				if(r_num > 0) {
					obj = read_object(r_num, REAL);
					obj_to_char(obj,ch);
					act("[$N] ti dice 'Ecco fatto!!'",FALSE,ch,0,mob,TO_CHAR);
					built = TRUE;
				}
			}
			k = k +1;
		} //fine loop sulle combinazioni possibili

		if(!built) act("[$N] ti dice 'Non mi far perdere tempo!! Con questa roba non ci faccio nulla!!'"
						   ,FALSE,ch,0,mob,TO_CHAR);
		return(TRUE);
	}

	return(FALSE);
}

OBJSPECIAL_FUNC(msg_obj) {
	int numero;
	int num_msg, diffusione;  //diffusione: 1=stanza, 2=zona, 3=mondo
	struct room_data* rp;
	struct char_data* tmp, *tmp2;

	//if (type != EVENT_COMMAND) return(FALSE);

	if(obj->iGeneric==0) {

		if(obj->in_room == NOWHERE) {
			return(FALSE);
		}
		rp = real_roomp(obj->in_room);

		for(tmp = rp->people; tmp; tmp = tmp2) {
			tmp2 = tmp->next_in_room;
			if(IS_PC(tmp) || IS_POLY(tmp)) {
				sscanf(obj_index[obj->item_number].specparms,"%d %d",&num_msg,&diffusione);
				numero = room_of_object(obj);
				obj->iGeneric=1;                  //al prossimo giro il msg non compare pi�

				switch(num_msg) {
				case 1:
					send_to_room("\n\r",numero);
					send_to_room("Un innaturale vento si alza, cogliendoti impreparato. Polvere, sabbia e pietre si\n\r",numero);
					send_to_room("fanno pigramente trasportare dalla brezza che porta in se' una voce tetra e terribile...\n\r",numero);
					send_to_room("\n\r",numero);
					send_to_room("'$c0008La Torre del $c0001Sangue$c0008, il cuore dell'impero di Xarah, dove venivano\n\r",numero);
					send_to_room("$c0008forgiati gli uomini d'arme al servizio del Culto di Thion. L'onore e la forza resero\n\r",numero);
					send_to_room("$c0008grande quest'ordine, la superbia e l'arroganza segnarono la sua fine...\n\r",numero);
					send_to_room("\n\r",numero);
					send_to_room("$c0008Osserva Straniero, e sii testimone del destino dei Cavalieri di Xarah...'$c0007\n\r",numero);
					send_to_room("\n\r",numero);
					break;
				case 2:
					send_to_room("\n\r",numero);
					send_to_room("Un gelo improvviso cala sulla citta', provocandoti brividi e tremiti fin dentro le ossa.\n\r",numero);
					send_to_room("Un grido straziato e incomprensibile scoppia in un boato di suoni, accompagnando una\n\r",numero);
					send_to_room("voce suadente...\n\r",numero);
					send_to_room(" \n\r",numero);
					send_to_room("'$c0008Queste sono le rovine di Xarah, l'orgoglio di Thion tramutatosi nella sua miseria. \n\r",numero);
					send_to_room("$c0008Gli abitanti comuni vennero stritolati nella morsa delle rocce, morendo per aver abbracciato \n\r",numero);
					send_to_room("$c0008il culto del Dio dell'Ombra...$c0007' \n\r",numero);
					send_to_room(" \n\r",numero);
					send_to_room("$c0004La voce si fa piu' sottile, quasi un sibilo$c0007\n\r",numero);
					send_to_room(" \n\r",numero);
					send_to_room("'$c0008E fu a loro che tocco' la miglior sorte...$c0007'\n\r",numero);
					send_to_room(" \n\r",numero);
					break;
				case 3:
					send_to_room(" \n\r",numero);
					send_to_room("Un eco lontano, remoto, affiora quando varchi la soglia della Torre dei Sortilegi. Un tono\n\r",numero);
					send_to_room("greve si diffonde nell'aria, recitando queste parole :\n\r",numero);
					send_to_room(" \n\r",numero);
					send_to_room("'$c0008La Torre dei Maghi di Xarah venne colpita dalla maledizione di Alar l'Oscuro fin\n\r",numero);
					send_to_room("$c0008dalle sue fondamenta, appestando la stessa struttura che gli usufruitori di magia credevano\n\r",numero);
					send_to_room("$c0008inaccessibile a qualsiasi sortilegio$c0007'\n\r",numero);
					send_to_room(" \n\r",numero);
					send_to_room("'$c0008La loro stessa illusione fu la punizione decretata dall'Oscuro, quando il contagio si\n\r",numero);
					send_to_room("$c0008diffuse tra i neofiti per passare poi ai Maestri, che videro le proprie capacita' magiche,\n\r",numero);
					send_to_room("$c0008di cui andavano orgogliosi sin dall'alba dei tempi, diminuire fin quasi a scomparire come\n\r",numero);
					send_to_room("$c0008neve al sole...$c0007'\n\r",numero);
					send_to_room(" \n\r",numero);
					break;
				case 4:
					send_to_room("\n\r",numero);
					send_to_room("Fiumi di parole vengono riversate nella tua mente con la stessa intensita' del fragore di un\n\r",numero);
					send_to_room("fulmine che riempie l'aria, costringendoti a portare le mani alle orecchie per non impazzire...\n\r",numero);
					send_to_room("\n\r",numero);
					send_to_room("'$c0001La Torre delle $c0008Ombre$c0001, Ove il peccato venne consumato piu' che in ogni altro\n\r",numero);
					send_to_room("$c0001luogo di questa citta'... Qui ebbero inizio i natali della citta' e della sua antica dinastia,\n\r",numero);
					send_to_room("$c0001e sempre qui il sacrilego popolo di Xarah venne incontro al proprio destino$c0007'\n\r",numero);
					send_to_room("\n\r",numero);
					break;
				case 5:
					send_to_room("\n\r",numero);
					send_to_room("Improvvisamente cala il silenzio, come se la realta' che ti circonda desiderasse prendere parte\n\r",numero);
					send_to_room("a cio' che sta per accadere... sussurri portati dal vento sembrano chiederti se sarai tu colui\n\r",numero);
					send_to_room("che spezzera' il giogo dell'Oscuro.\n\r",numero);
					send_to_room("'$c0006Le Porte del Santuario sono ora aperte$c0007' sibila qualcuno all'interno del tempio.\n\r",numero);
					send_to_room("'$c0006Sono aperte per te...$c0007'\n\r",numero);
					send_to_room("\n\r",numero);
					break;
				default:
					return(FALSE);
				}
				return(FALSE);
			}


		}
	}

	return(FALSE);
}

#define THION_MOB 19719
#define THION_ROOM 19755
OBJSPECIAL_FUNC(thion_loader) {
	int numero, iVnum, percent, r_num, r_num2;
	int x1 = 0;
	int x2 = 0;
	int x3 = 0;
	int x4 = 0;
	int x5 = 0;
	int x6 = 0;
	int x7 = 0;
	struct obj_data* tmp;
	struct obj_data* obj_object;
	struct obj_data* sub_object;
	struct char_data* pThion;


	if(obj->iGeneric==0) {
		for(tmp = obj->contains; tmp; tmp = tmp->next_content) {
			iVnum = (tmp->item_number >= 0) ? obj_index[tmp->item_number].iVNum : 0;
			switch(iVnum) {
			case 19705:
				x1 = 1;
				break;
			case 19706:
				x2 = 1;
				break;
			case 19715:
				x3 = 1;
				break;
			case 19716:
				x4 = 1;
				break;
			case 19722:
				x5 = 1;
				break;
			case 19739:
				x6 = 1;
				break;
			case 19740:
				x7 = 1;
				break;
			default:
				break;
			}
			if(x1+x2+x3+x4+x5+x6+x7 == 7) {
				numero = room_of_object(obj);
				send_to_room(" \n\r",numero);
				send_to_room("Non appena poni l'ultimo pezzo nell'altare le pareti del tempio cominciano a vibrare, mentre il\n\r",numero);
				send_to_room("pavimento pare lentamente sfaldarsi sotto i tuoi piedi. L' $c0008Altare$c0007 sembra animarsi,\n\r",numero);
				send_to_room("colorandosi di un inquietante alone purpureo, mentre il tempio cade vittima dell'abbraccio\n\r",numero);
				send_to_room("gelido e silenzioso delle tenebre...\n\r",numero);
				send_to_room(" \n\r",numero);
				send_to_room("$c0008... E, in lontananza, l'eco di una folle risata rimbomba tra le pareti del Santuario della Luna...$c0007\n\r",numero);
				send_to_room(" \n\r",numero);

				if((pThion = read_mobile(real_mobile(THION_MOB), REAL))) {
					char_to_room(pThion, THION_ROOM);
				}

				//estrazione casuale del premio
				percent = number(1,100);
				if(percent <= 2) {
					r_num = real_object(19755);
				}
				else if(percent > 2 && percent <= 5) {
					r_num = real_object(19756);
				}
				else if(percent > 5 && percent <= 15) {
					r_num = real_object(19757);
				}
				else if(percent > 15 && percent <= 30) {
					r_num = real_object(19758);
				}
				else if(percent > 30 && percent <= 45) {
					r_num = real_object(19759);
				}
				else if(percent > 45 && percent <= 55) {
					r_num = real_object(19760);
				}
				else if(percent > 55 && percent <= 60) {
					r_num = real_object(19761);
				}
				else if(percent > 60 && percent <= 70) {
					r_num = real_object(19763);
				}
				else if(percent > 70 && percent <= 80) {
					r_num = real_object(19766);
				}
				else if(percent > 80 && percent <= 90) {
					r_num = real_object(19767);
				}
				else if(percent > 90 && percent <= 100) {
					r_num = real_object(19768);
				}

				r_num2 = real_object(19773);
				obj_object = read_object(r_num2, REAL);
				sub_object = read_object(r_num, REAL);

				obj_to_char(obj_object,pThion);
				obj_to_obj(sub_object, obj_object);


				obj->iGeneric=1;                  //al prossimo giro non accade piu' nulla
			}

		}

	}

	return(FALSE);
}

/****************************/
/* REQUIEM 2018 Quest fisse */
/****************************/
/* Tenendo conto delle procedure create da salvo per l'assegnazione
 dei premi, creiamo ora delle procedure per la creazione e l'assegnazione
 delle quest dai mob. */

MOBSPECIAL_FUNC(AssignQuest) {
	char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
	struct char_data* questor;
    struct char_data* quest_tgt;
    int x, y, t;
    int quest_type;     /* 0.Caccia 1.salvataggio 2.ricerca 3.Consegna */
    
	questor = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(AssignQuest));

	if(!questor) {
		return(FALSE);
	}

    if(type == EVENT_COMMAND && cmd == CMD_ASK && IS_PC(ch)) {
        
        arg = one_argument(arg, buf);
        if(!*buf || get_char_room_vis(ch, buf) != questor) {
            return(FALSE);
        }
        
        if(IS_AFFECTED(ch, AFF_INVISIBLE) || ch->invis_level >= IMMORTALE) {
            do_say(questor, "eh? chi ha parlato??", CMD_SAY);
            return(FALSE);
        }
        
        if(GetMaxLevel(ch) < ALLIEVO) {
            sprintf(buf,"%s Torna quando cresci.",GET_NAME(ch));
            do_tell(questor, buf, CMD_TELL);
            return(FALSE);
        }
        
        if(strstr(arg, "quest") != NULL) {
            
            if((!affected_by_spell(ch, STATUS_QUEST))) {
            
            switch(GET_RACE(questor)) {

                case RACE_HUMAN     :
                    quest_type = number(0,3);
                    break;
                case RACE_ELVEN     :
                    quest_type = number(1,3);
                    break;
                case RACE_DWARF     :
                    quest_type = number(0,1);
                    break;
                case RACE_HALFLING  :
                    quest_type = number(1,2);
                    break;
                case RACE_LIZARDMAN :
                    quest_type = 0;
                    break;
                case RACE_ORC       :
                    quest_type = 0;
                    break;
                case RACE_GOBLIN    :
                    quest_type = 0;
                    break;
                default:
                    quest_type = number(0,2);
                    break;
            }
                
                /* forzo il tipo di quest a caccia */
                sprintf(buf2, "pensa di affidarti una missione di %s ma poi ci riflette meglio e decide per qualcosa di piu' indicato...", QuestKind[quest_type]);
                act(buf2, FALSE, ch, 0, ch, TO_CHAR);
                quest_type = 0;
                
            switch(quest_type) {
                    
                case 0      :
                    
                    do {
                        x = number(QUEST_ZONE,QUEST_ZONE+99);
                    } while (real_mobile(x) < 0);
                    
                    AssignMob(x, MobCaccia, "MobCaccia","");
                    
                    quest_tgt = read_mobile(real_mobile(x), REAL);
                    
                    char_to_room(quest_tgt, RandomRoomByLevel(GetMaxLevel(ch)));
                    
                    for (y = 0; NameGenStart[y] != NULL; y++);
                    t = number(0,y-2);
                    sprintf(buf2, "%s", NameGenStart[t]);
                    
                    for (y = 0; NameGenMid[y] != NULL; y++);
                    t = number(0,y-2);
                    strcat(buf2, NameGenMid[t]);
                    
                    for (y = 0; NameGenEnd[y] != NULL; y++);
                    t = number(0,y-2);
                    strcat(buf2, NameGenEnd[t]);
                    
                    
                    quest_tgt->player.name = (char*)strdup(buf2);
                    
                    sprintf(buf2, "%s, %s",buf2, quest_tgt->player.short_descr);
                    
                    quest_tgt->player.short_descr = (char*)strdup(buf2);
                    
                    /* creo il link tra preda e cacciatore */
                    ch->specials.quest_ref = quest_tgt;
                    quest_tgt->specials.quest_ref = ch;
                    
                    /* copio nel mob l'EqValueIndex del pg al momento della richiesta */
                    quest_tgt->specials.eq_val_idx = GetCharBonusIndex(ch);
                    
                    spell_quest(GetMaxLevel(ch),quest_tgt,quest_tgt,0);
                    
                    /* adattiamo il mob al questante, questa e' da perfezionare */
                    
                    if(GetMaxLevel(ch) < IMMORTALE) {
                        GET_LEVEL(quest_tgt, WARRIOR_LEVEL_IND) = GetMaxLevel(ch);
                    }
                
                    if(HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
                                CLASS_BARBARIAN | CLASS_MONK)) {
                        quest_tgt->specials.damsizedice = GetMaxLevel(ch)/17;
                        quest_tgt->specials.damnodice = GetMaxLevel(ch)/5;
                    }
                    
                    
                    if(HasClass(ch, CLASS_MONK)) {
                        quest_tgt->points.max_hit = GET_MAX_HIT(ch)*2;
                    } else if(HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
                                       CLASS_BARBARIAN | CLASS_THIEF)) {
                        quest_tgt->points.max_hit = floor(GET_MAX_HIT(ch)*1.5);
                    } else {
                        GET_MAX_HIT(ch);
                    }

                    GET_HIT(quest_tgt) = GET_MAX_HIT(quest_tgt);
                     
                    quest_tgt->points.max_move = NewMobMov(quest_tgt);
                    GET_MOVE(quest_tgt) = GET_MAX_MOVE(quest_tgt);
                    quest_tgt->points.hitroll = (GetMaxLevel(ch)/2)+ch->points.hitroll;
                    quest_tgt->points.damroll = (GetMaxLevel(ch)/10)+ch->points.damroll;
                    
                    break;
                case 1      :
                    break;
                case 2      :
                    break;
                case 3      :
                    break;
                default:
                    break;
            }
            
            spell_quest(GetMaxLevel(ch),ch,ch,0);
                
            sprintf(buf, "%s Pare ci sia una grossa taglia su %s, l'ultima volta e' stato vist%s a %s.",GET_NAME(ch), quest_tgt->player.name,SSLF(quest_tgt), real_roomp(quest_tgt->in_room)->name);
                
            } else {
                
                sprintf(buf, "%s Non puoi sobbarcarti di tutto il lavoro del regno, torna piu' tardi!",GET_NAME(ch));
            }
            
        } else {
            sprintf(buf, "%s Se vuoi ho un lavoretto per te. Chiedimi una quest...",GET_NAME(ch));
            }
        
        do_tell(questor, buf, CMD_TELL);
        
    } else {
        return(FALSE);
    }
    
    return(TRUE);
    
	}
    
/**************************/
/* SALVO 2006 Quest fisse */
/**************************/
/*
R xxxx MobKillInRoom VNum Tipo Premio SideProcedure
M xxxx ItemGiven VNum Tipo Premio SideProcedure
O xxxx ItemPut VNum Tipo Premio SideProcedure
O xxxx TransformObj LIST_BEG LIST_END RESULT
O xxxx SummonMOB LIST_BEG LIST_END RESULT ROOM

TransformObj
dove xxxxx e' un oggetto (ovviamente container)
LIST_BEG e LIST_END sono i VNUM di due oggetti dell'area; se tutti gli
oggetti della lista si trovano all'interno dell'oggetto xxxxx vengono
junkati ed al loro post0 compare l'oggetto di VNUM [RESULT]

SummonMOB
Dove RESULT stavolta e' il VNUM del MOB da evocare e, se ROOM e' specificato
e' la stanza dove il MOB compare, altrimenti il MOB compare nella stessa
stanza.

Allora, a livello di parametri le procedure funzionano circa allo stesso
modo, cambia solo il loro metodo di attivazione:
R MobKillInRoom si attiva quando il mostro VNum muore nella stanza specificata
M ItemGiven si attiva quando il MOB riceve l'oggetto VNum (l'oggetto viene
junkato)
O ItemPut si attiva quando nel contenitore viene posto l'oggetto VNum
(l'oggetto viene junkato)

Tipo e' il tipo di premio; il significato del parametro Premio dipende dal
tipo di quest:
0 il premio viene dato in XP
1 il premio viene dato in Gold
2 il premio viene dato in Rune degli eroi; e' importante non pagare rune ai
PG che hanno piu' livelli del MOB ucciso, in tal modo possiamo creare quest
fisse anche per bassi livelli senza che i principi vi si affollino per
ottenere rune facili
3 il premio e' un oggetto specifico (magari da usare assieme ad una delle
altre)
4 il premio e' un oggetto casuale scelto da una lista (come in Xarah); il
parametro Premio in questo caso contiene un contatore (0 vuol dire che non
ci sono premi) che indica la dimensione della tabella dei premi; la tabella
parte dall'ultimo oggetto dell'area (per convenzione vnum xxx99) e va
indietro per n valori (xxx99-n+1);
5 il premio e' un oggetto casuale generato sul momento; Premio e' il livello
indicativo dell'oggetto

SideProcedure e' una procedura addizionale da attivare alla fine della
quest; si usa senza parametri e si occupa di tutti gli effetti speciali (ad
esempio teleport)

Il premio in XP o Rune viene assegnato al gruppo di cui fa parte il PG che
compie l'azione scatenante (uccide il MOB o consegna l'oggetto). In ogni
caso vengono divisi in parti eque ed eventuali resti vengono distribuiti ai
membri del gruppo in ordine di join (iniziando dal capogruppo); ad esempio
in un gruppo di  4 persone, se il premio e' 6 Rune, sono 1 runa a testa +1
runa addizionale per il  capogruppo e per il secondo che joina.

Per i premi in Gold o Oggetti e' piu' complesso: se la procedura e' legata
alla room l'oggetto (o il tesoro) dovrebbe apparire in terra con un
messaggio; se la procedura e' legata ad un MOB il MOB da il premio al
capogruppo del gruppo che contiene il PG che compie l'azione; se la
procedura e' legata ad un oggetto, il premio verra' caricato al suo interno.

So che le procedure sono complesse, ma possiamo procedere per passi a
realizzarle. Innanzitutto ti direi di cominciare con la MobCaccia ed
implementare solo il tipo di premio 2 (rune degli eroi), senza pero'
implementare l'utilizzo del parametro SideProcedure che puo' aspettare; in
questo modo possiamo subito iniziare ad implementare delle quest fisse anche
se molto semplici tipo uccidi il MOB e prendi le rune.

Poi se hai tempo procedi a creare i tipi 1, 0 e 3 che verranno usate molto
meno. Per la 4 credo che ti basti copiare un po' di codice dalla special che
genera Thion.
*/
    
/* Requiem:
 aggiungiamo un tipo di assegnazione piu' semplice
 M xxxx MobCaccia VNum Tipo Premio -> il mob crepa e se il vnum combacia col quest target
 vengono assegnati i premi.
 */

MOBSPECIAL_FUNC(MobCaccia) {
    struct affected_type* af;
    struct char_data* t;
    struct room_data* rp;
    int premio[3]; /* 0.coin, 1.xp, 2.rune */
    int n,x;
    char buf[MAX_INPUT_LENGTH];
    
    if(!mob->specials.quest_ref) {
        if(real_roomp(mob->in_room)->people) {
            sprintf(buf,"\n\r$c0014%s ha perso il senso della sua esistenza...$c0007\n\r",mob->player.name);
            act(buf, FALSE, mob, 0, 0, TO_ROOM);
        }
        extract_char(mob);
        return FALSE;
    }
    
    t = mob->specials.quest_ref;
    
    switch(type) {
            
    case EVENT_DEATH    :
            
        t->specials.quest_ref = NULL;

        if(t->in_room == ch->in_room && strstr(t->lastmkill, GET_NAME(ch))) {

            for(af = t->affected; af; af = af->next) {
                if(af->type == STATUS_QUEST) {
                    
                    x = GetMaxLevel(t);
                    
                    if(x >= IMMORTALE) {
                        send_to_char("\n\r$c0014La Gilda dei Mercenari non ammette immortali!$c0007\n\r", t);
                        return FALSE;
                    }
                    
                    premio[0] = (x*10000)-(((x-af->duration)+1)*10000);
                    if(IS_PKILLER(t)) {
                        premio[1] = (x*50000)-((x-af->duration)*50000);
                    }
                    if(IS_PRINCE(t) && af->duration >= x-2) {
                        premio[2] = 1;
                    }
                    
                    if(t->followers || t->master) {
                        send_to_char("\n\r$c0014La Gilda dei Mercenari.... si vergogna di te! Non hai avuto il coraggio di affrontarlo in solitaria.$c0007\n\r", t);
                        return FALSE;
                    }
                    
                    sprintf(buf,"\n\r$c0014Completi la tua missione in %d ore, e la Gilda dei Mercenari valuta la tua prestazione in maniera ",af->duration);
                    if(af->duration >= x-2) {
                    strcat(buf,"eccellente! 'Estremamente veloce ed efficiente, complimenti!'.\n\r");
                    } else if(af->duration >= x/2) {
                    strcat(buf,"sufficiente. 'Ti consigliamo di allenarti ulteriormente'.\n\r");
                    } else {
                    strcat(buf,"scarsa. 'Non ci siamo proprio, ti suggeriamo di chiedere dei consigli in futuro... magari ladri e mercanti sapranno indicarti'.\n\r");
                    }
                    send_to_char(buf, t);
                    
                    if((x = GetCharBonusIndex(t) - t->specials.eq_val_idx) > 10) {
                        mudlog(LOG_CHECK, "Eq Value variation of %s = %d",GET_NAME(t), x);
                        send_to_char("$c0011tenendo conto che il tuo potere e' cresciuto molto rispetto a quando ti abbiamo ingaggiato... $c0007\n\r", t);
                        
                        premio[0] -= x*50;
                        premio[1] -= x*200;
                        premio[2] = 0;
                        
                    }
                    
                    if(premio[0]+premio[1]+premio[2] == 0) {
                        send_to_char("\n\r$c0014...non vinci un piffero. Cerca di essere piu' veloce e piu' coerente!$c0007\n\r", t);
                        
                    } else {
                    
                        for(n = 0;n < 3;n++) {
                            if(premio[n] > 0) {
                                
                                switch(n) {
                                    case 0  :
                                        GET_GOLD(t) += premio[0];
                                        sprintf(buf,"\r$c0014Ricevi %d monete d'oro!$c0007\n\r", premio[0]);
                                        break;
                                    case 1  :
                                        GET_EXP(t) += premio[1]/HowManyClasses(t);
                                        sprintf(buf,"$c0014Ottieni %d punti esperienza!$c0007\n\r", premio[1]);
                                        break;
                                    case 2  :
                                        GET_RUNEDEI(t) += premio[2];
                                        sprintf(buf,"$c0014Vieni marchiato con %d rune degli Dei!$c0007\n\r", premio[2]);
                                        break;
                                    default:
                                        break;
                                }
                                send_to_char(buf, t);
                            }
                        }
                    }
                sprintf(buf,"\n\r$c0014%s ha reso un servigio agli dei!$c0007\n\r",GET_NAME(t));
                act(buf, FALSE, t, 0, 0, TO_ROOM);
                return FALSE;
            }
        }
    } else {
        send_to_char("\n\r$c0014Qualcun'altro compie il tuo dovere e finisci senza paga.$c0007\n\r", t);
        affect_from_char(t, STATUS_QUEST);
        SpellWearOff(STATUS_QUEST, t);
        return FALSE;
    }

    break;
            
    case EVENT_COMMAND  :
    
        if(!*arg) {
            return FALSE;
        }
        
        if(cmd == CMD_KILL) {
            arg = one_argument(arg,buf);
            
                if(*buf && get_char_room_vis(mob, buf) == mob) {
                    
                    if(CAN_SEE(mob,ch)) {
                        sprintf(buf,"%s, Non mi arrendero' mai!",GET_NAME(ch));
                    }
                
                do_tell(mob,buf,CMD_TELL);
            
            }
        }
    
    return FALSE;
            
    break;
    
    case EVENT_TICK     :

            rp = real_roomp(mob->in_room);
            
            if(mob->specials.fighting) {
                    for(t = rp->people; t; t=t->next_in_room) {
                        if((t != mob) && IS_PC(t) && t->specials.quest_ref != mob && t->specials.fighting == mob && GetMaxLevel(t) < IMMORTALE) {
                            WAIT_STATE(t, PULSE_VIOLENCE*3);
                            
                            sprintf(buf,"\n\r$c0014%s Si vede alle strette e se la da' a gambe!$c0007\n\r",GET_NAME(mob));
                            send_to_char(buf, t);
                            
                            stop_fighting(mob);
                            char_from_room(mob);
                            char_to_room(mob, RandomRoomByLevel(GetMaxLevel(ch)));
                            
                            do_shout(mob,"Inutile che mandi i tuoi scagnozzi, non mi troverai mai!",CMD_SHOUT);
                            return FALSE;

                        }
                    }
                    
            } else {
                
                if(!affected_by_spell(t,STATUS_QUEST) && t->specials.quest_ref == mob) {
                    if(rp->people) {
                        sprintf(buf,"\n\r$c0014%s si confonde tra la folla e scompare per sempre...$c0007\n\r",mob->player.name);
                        act(buf, FALSE, mob, 0, 0, TO_ROOM);
                    }
                    extract_char(mob);
                    return FALSE;
                }
                
                if(GET_POS(mob) == POSITION_SLEEPING) {
                    do_wake(mob, "", -1);
                }
                
                if(GET_POS(mob) == POSITION_SLEEPING || GET_POS(mob) == POSITION_SITTING) {
                    StandUp(mob);
                }
                
                if(GET_POS(mob) == POSITION_STANDING) {
                    for(t = rp->people; t; t=t->next_in_room) {
                        if((t != mob) && t->specials.quest_ref == mob && CAN_SEE(mob, t)) {
                            sprintf(buf,"%s Dannazione come mi hai trovato? Non mi avrai cosi' facilmente!",GET_NAME(t));
                            do_tell(mob,buf,CMD_TELL);
                            hit(mob, t, 0);
                            return FALSE;
                        }
                    }
                }
            }
    break;
    }

    return FALSE;
}
    
ROOMSPECIAL_FUNC(MobKillInRoom) {
	if(type == EVENT_DEATH && ch->in_room == room->number) {
		const char* p;
		char killed[8];
		int iKilled, iTipo;
		char tipo[3];
		char premio[25];
		char sideprocedure[25];
		p=room->specparms;
		p=one_argument(p,killed);
		iKilled = atoi(killed);
		p=one_argument(p,tipo);
		iTipo = atoi(tipo);
		p=one_argument(p,premio);
		only_argument(p,sideprocedure);
		if(GET_MOB_VNUM(ch) == iKilled) {
            
            
			if(iTipo == 2) {
				char buf[80];
				sprintf(buf,"Complimenti hai vinto %s rune degli eroi.",premio);
				act(buf, FALSE, ch, 0, ch, TO_ROOM);
			}
			else {
				act("ATTENZIONE il tipo di premio implementato e' solo il 2 ovvero in rune.", FALSE, ch, 0, ch, TO_ROOM);
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(ItemGiven) {
	if(cmd == CMD_GIVE) {
		const char* p;
		char oggetto[8];
		int iOggetto, iTipo;
		char tipo[3];
		char premio[25];
		char sideprocedure[25];
		char obj_name[80];
		struct obj_data* obj;

		p=mob_index[mob->nr].specparms;
		p=one_argument(p,oggetto);
		iOggetto = atoi(oggetto);
		p=one_argument(p,tipo);
		iTipo = atoi(tipo);
		p=one_argument(p,premio);
		only_argument(p,sideprocedure);
		arg=one_argument(arg,obj_name);
		if((obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
			char mob_name[80];
			arg=one_argument(arg,mob_name);
			if(GET_OBJ_VNUM(obj) == iOggetto && (mob = get_char_room_vis(ch, mob_name))) {
				if(iTipo == 2) {
					char buf[80];
					sprintf(buf,"Complimenti hai vinto %s rune degli eroi.\n\r",premio);
					send_to_all(buf);
				}
				else {
					send_to_all("ATTENZIONE il tipo di premio implementato e' solo il 2 ovvero in rune.\n\r");
				}
			}
		}
	}
	return FALSE;
}

OBJSPECIAL_FUNC(ItemPut) {
	if(cmd == CMD_PUT) {
		const char* p;
		char oggetto[8];
		int iOggetto, iTipo;
		char tipo[3];
		char premio[25];
		char sideprocedure[25];
		char obj_name1[80];
		struct obj_data* obj1;

		p=obj_index[obj->item_number].specparms;
		p=one_argument(p,oggetto);
		iOggetto = atoi(oggetto);
		p=one_argument(p,tipo);
		iTipo = atoi(tipo);
		p=one_argument(p,premio);
		only_argument(p,sideprocedure);
		arg=one_argument(arg,obj_name1);
		if((obj1 = get_obj_in_list_vis(ch, obj_name1, ch->carrying))) {
			struct obj_data* obj2=nullptr;
			char obj_name2[80];

			arg=one_argument(arg,obj_name2);
			if((GET_OBJ_VNUM(obj1) == iOggetto) && obj2) {
				if(iTipo == 2) {
					char buf[80];
					sprintf(buf,"Complimenti hai vinto %s rune degli eroi.\n\r",premio);
					send_to_all(buf);
				}
				else {
					send_to_all("ATTENZIONE il tipo di premio implementato e' solo il 2 ovvero in rune.\n\r");
				}
			}
		}
	}
	return FALSE;
}
/*******************************/
/* SALVO 2006 fine Quest fisse */
/*******************************/
} // namespace Alarmud

