/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD*/
/* $Id: act.other.c,v 1.5 2002/03/11 11:33:34 Thunder Exp $
 *
 */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
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
#include "act.other.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj2.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "ansi_parser.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "mail.hpp"
#include "maximums.hpp"
#include "modify.hpp"
#include "multiclass.hpp"
#include "nilmys.hpp"
#include "opinion.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "snew.hpp"
#include "snew.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
#include "spell_parser.hpp"
#include "utility.hpp"
namespace Alarmud {

ACTION_FUNC(do_gain) {

}

ACTION_FUNC(do_guard) {
	if(!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
		send_to_char("Sorry. you can't just put your brain on autopilot!\n\r",ch);
		return;
	}

	for(; isspace(*arg); arg++);

	if(!*arg) {
		if(IS_SET(ch->specials.act, ACT_GUARDIAN)) {
			act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
			send_to_char("You relax.\n\r",ch);
			REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
		}
		else {
			SET_BIT(ch->specials.act, ACT_GUARDIAN);
			act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
			act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
			send_to_char("You snap to attention\n\r", ch);
		}
	}
	else {
		if(!str_cmp(arg,"on")) {
			if(!IS_SET(ch->specials.act, ACT_GUARDIAN)) {
				SET_BIT(ch->specials.act, ACT_GUARDIAN);
				act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
				act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
				send_to_char("You snap to attention\n\r", ch);
			}
		}
		else if(!str_cmp(arg,"off")) {
			if(IS_SET(ch->specials.act, ACT_GUARDIAN)) {
				act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
				send_to_char("You relax.\n\r",ch);
				REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
			}
		}
	}

	return;
}


ACTION_FUNC(do_junk) {
	char tmp[100], buf[100], newarg[100];
	struct obj_data* tmp_object;
	int num, p, count, value=0,value2=0;

	/*
	 *   get object name & verify
	 */

	only_argument(arg, tmp);
	if(*tmp) {
		if(getall(tmp,newarg) != 0) {
			num = -1;
			strcpy(tmp,newarg);
		}
		else if((p = getabunch(tmp,newarg)) != 0) {
			num = p;
			strcpy(tmp,newarg);
		}
		else {
			num = 1;
		}
	}
	else {
		send_to_char("Cosa vuoi gettare via?\n\r",ch);
		return;
	}
	count = 0;
	while(num != 0) {
		tmp_object = get_obj_in_list_vis(ch, tmp, ch->carrying);
		if(tmp_object)
        {
            if((IS_OBJ_STAT2(tmp_object,ITEM2_EDIT) || IS_OBJ_STAT2(tmp_object,ITEM2_PERSONAL)) && !IS_DIO(ch))
            {
                act("Non puoi gettare via $p, c'e' inciso il nome di qualcuno!",FALSE, ch, tmp_object, 0, TO_CHAR);
                return ;
            }

			if(IS_OBJ_STAT(tmp_object,ITEM_NODROP)  && !IS_IMMORTAL(ch)) {
				send_to_char
				("Non puoi farlo, una $c0009maledizione$c0007 te lo impedisce!\n\r", ch);
				return;
			}
#if EGO
			/* if it is a limited items check if the PC EGO is strong enough
			   Gaia 2001 */
			if(IS_RARE(tmp_object) && !EgoSave(ch)) {
				act("Gettare via $p??? Non ci pensare nemmeno!",FALSE, ch, tmp_object, 0, TO_CHAR);
				return ;
			}
#endif
			value+=(MIN(1000,MAX(tmp_object->obj_flags.cost/4,1)));
			value2+=(tmp_object->obj_flags.cost>=(LIM_ITEM_COST_MIN+10000)?
					 tmp_object->obj_flags.cost:0);
			obj_from_char(tmp_object);
			extract_obj(tmp_object);
			if(num > 0) {
				num--;
			}
			count++;
		}
		else {
			num = 0;
		}
	}
	if(count > 1) {
		sprintf(buf, "Butti tra i rifiuti %s (%d).\n\r", arg, count);
		act(buf, 1, ch, 0, 0, TO_CHAR);
		sprintf(buf, "$n butta nei rifiuti %s (%d).\n\r", arg, count);
		act(buf, 1, ch, 0, 0, TO_ROOM);
	}
	else if(count == 1) {
		sprintf(buf, "Butti tra i rifiuti %s.\n\r", arg);
		act(buf, 1, ch, 0, 0, TO_CHAR);
		sprintf(buf, "$n butta nei rifiuti %s.\n\r", arg);
		act(buf, 1, ch, 0, 0, TO_ROOM);
	}
	else {
		send_to_char("Non hai niente del genere!\n\r", ch);
	}

	value /= 2;

	if(value) {
		act("Sei stat$b ricompensat$b.",
			FALSE, ch, 0, 0, TO_CHAR);

		if(GetMaxLevel(ch) < 3) {
			gain_exp(ch, MIN(100,value));
		}
		else {
			GET_GOLD(ch) += value;
		}
	}
	if(value2) {
		act("Rinunciare ai beni terreni ti fa progredire nel tuo cammino.",
			FALSE, ch, 0, 0, TO_CHAR);
		gain_exp(ch, (value2*number(10,30))/10);

    // Junk Achievement
        if(IS_POLY(ch))
        {
            ch->desc->original->specials.achievements[OTHER_ACHIE][ACHIE_JUNK] += 1;
            if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
            {
                SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
            }
        }
        else
        {
            ch->specials.achievements[OTHER_ACHIE][ACHIE_JUNK] += 1;
            if(!IS_SET(ch->specials.act,PLR_ACHIE))
            {
                SET_BIT(ch->specials.act, PLR_ACHIE);
            }
        }
        CheckAchie(ch, ACHIE_JUNK, OTHER_ACHIE);
	}
	return;
}

ACTION_FUNC(do_destroy)
{
    char tmp[100], buf[200];
    struct obj_data* tmp_object;
    int value=0,value2=0;

    if(!IS_PC(ch))
    {
        send_to_char("Non puoi farlo, sei solo un mob!\n\r",ch);
        return;
    }

    one_argument(arg, tmp);

    if(*tmp)
    {
        tmp_object = get_obj_in_list_vis(ch, tmp, ch->carrying);

        if(tmp_object)
        {
            char name[25];
            int val = 0;
            bool check = TRUE;

            if(ch->lastpkill)
            {
                strcpy(name, "ED");
                strcat(name,ch->lastpkill);
                val = 1;
                if(isname(name, tmp_object->name))
                {
                    val = 2;
                }
            }
            strcpy(name, "ED");
            strcat(name, GET_NAME(ch));
            if(isname(name, tmp_object->name))
            {
                val = 3;
            }

            if((!IS_OBJ_STAT2(tmp_object,ITEM2_EDIT) && !IS_OBJ_STAT2(tmp_object, ITEM2_PERSONAL)) && !IS_DIO(ch))
            {
                act("Non puoi distruggere $p! Prova a buttarlo via!",FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }

            if(val == 0)
            {
                sprintf(buf, "Non puoi distruggere $p! Non e' di tua proprieta'!");
                check = FALSE;
            }
            if(val == 1)
            {
                sprintf(buf, "Non puoi distruggere $p! Non hai ucciso il suo proprietario!");
                check = FALSE;
            }
            if(val == 2 && !IS_AFFECTED2(ch,AFF2_PKILLER))
            {
                sprintf(buf, "Non puoi distruggere $p! E' passato troppo tempo da quando hai ucciso %s!",ch->lastpkill);
                check = FALSE;
            }

            if(!IS_DIO(ch) && !check)
            {
                act(buf,FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }

            if(IS_OBJ_STAT(tmp_object,ITEM_NODROP) && !IS_IMMORTAL(ch))
            {
                send_to_char("Non puoi farlo, una $c0009maledizione$c0007 te lo impedisce!\n\r", ch);
                return;
            }
#if EGO
            if(IS_RARE(tmp_object) && !EgoSave(ch))
            {
                act("Vuoi distruggere $p??? Non ci pensare nemmeno!",FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }
#endif
            value+=(MIN(100000,MAX(tmp_object->obj_flags.cost/4,1)));
            value2+=(tmp_object->obj_flags.cost>=LIM_ITEM_COST_MIN ? tmp_object->obj_flags.cost : 0);
            mudlog(LOG_PLAYERS,"%s destroy %s [owner was %s]",GET_NAME(ch), tmp_object->short_description, (ch->lastpkill ? ch->lastpkill : "no one"));
            obj_from_char(tmp_object);
            extract_obj(tmp_object);
        }

        if(value > 0)
        {
            sprintf(buf, "Distruggi %s.\n\r", arg);
            act(buf, 1, ch, 0, 0, TO_CHAR);
            sprintf(buf, "$n distrugge %s.\n\r", arg);
            act(buf, 1, ch, 0, 0, TO_ROOM);
        }
        else
        {
            send_to_char("Cosa vuoi distruggere?\n\r",ch);
            return;
        }
    }
    else
    {
        send_to_char("Cosa vuoi distruggere?\n\r",ch);
        return;
    }

    value /= 2;

    if(value)
    {
        act("Sei stat$b ricompensat$b.", FALSE, ch, 0, 0, TO_CHAR);

        if(GetMaxLevel(ch) < 3)
        {
            gain_exp(ch, MIN(100,value));
        }
        else
        {
            GET_GOLD(ch) += value;
        }
    }
    if(value2)
    {
        act("Distruggere l'equipaggiamento dei tuoi avversari ti fa progredire nel tuo cammino!", FALSE, ch, 0, 0, TO_CHAR);
        gain_exp(ch, (value2*number(10,30))/10);
    }
    return;
}

ACTION_FUNC(do_qui) {
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	return;
}

ACTION_FUNC(do_set_prompt) {
	static struct def_prompt {
		int n;
		const char* pr;
	} prompts[] = {
		{1, "Nebbie Arcane>> "},
		{2, "H%h V%v>> "},
		{3, "H%h M%m V%v>> "},
		{4, "H%h/%H V%v/%V>> "},
		{5, "H%h/%H M%m/%M V%v/%V>> "},
		{6, "H%h V%v C%C>> "},
		{7, "H%h M%m V%v C%C>> "},
		{8, "H%h V%v C%C %S>> "},
		{9, "%N H%h M%m V%v X%x %S %c/%C>> "},
		{40,"R%R>> "},
		{41,"R%R [%iS/%iN/%iI]>> "},
		{0,NULL}
	};
	char buf[512];
	int i,n;


	if(IS_NPC(ch)) {
		if(IS_SET(ch->specials.act, ACT_POLYSELF)) {
			send_to_char("Puoi farlo solo nella tua forma originale.\n\r", ch);
		}
		return;
	}


	for(; isspace(*arg); arg++);

	if(*arg) {
		if((n=atoi(arg))!=0) {
			if(n>39 && !IS_IMMORTAL(ch)) {
				send_to_char("Eh?\r\n",ch);
				return;
			}
			for(i=0; prompts[i].pr; i++) {

				if(prompts[i].n==n) {
					if(ch->specials.prompt) {
						free(ch->specials.prompt);
					}
					ch->specials.prompt = strdup(prompts[i].pr);
					return;
				}
			}

			send_to_char("Invalid prompt number\n\r",ch);
		}
		else {
			if(ch->specials.prompt) {
				free(ch->specials.prompt);
			}
			ch->specials.prompt = strdup(arg);
		}
	}
	else {
		sprintf(buf,"Your current prompt is : %s\n\r",ch->specials.prompt);
		send_to_char(buf,ch);
	}

}



ACTION_FUNC(do_title) {
	char buf[512];


	if(IS_NPC(ch) || !ch->desc) {
		return;
	}
	string argument(arg);
	boost::trim_left(argument);
	if(argument.length()>80) {
		send_to_char("Line too long, truncated\n", ch);
		free(ch->player.title);
	}
	if (argument.length()<5) {
		send_to_char("Line too short, title not changed\n", ch);
	}
	else {
		ch->player.title = strdup(argument.substr(0,80).c_str());
		toonPtr pg=Sql::getOne<toon>(toonQuery::name==string(GET_NAME(ch)));
		pg->title=argument.substr(0,80);
		Sql::update(*pg);
	}
	sprintf(buf, "Il tuo titolo adesso e' : <%s>\n\r", ch->player.title);
	send_to_char(buf, ch);


}

ACTION_FUNC(do_quit) {

	if(IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM)) {
		return;
	}

	if(GET_POS(ch) == POSITION_FIGHTING) {
		send_to_char("No way! You are fighting.\n\r", ch);
		return;
	}

	if(GET_POS(ch) < POSITION_STUNNED) {
		send_to_char("You die before your time!\n\r", ch);
		mudlog(LOG_PLAYERS, "%s dies via quit.", GET_NAME(ch));
		die(ch, 0, NULL);
		return;
	}

	act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
	zero_rent(ch);
	extract_char(ch); /* Char is saved in extract char */
}



ACTION_FUNC(do_save) {
	struct obj_cost cost;
	struct char_data* tmp;
	struct obj_data* tl;
	struct obj_data* teq[MAX_WEAR], *o;
	char buf[256];
	int i;

	if(ch == NULL) {
		mudlog(LOG_SYSERR, "ch == NULL in do_save (act.other.c)");
		return;
	}
	if(ch->nMagicNumber != CHAR_VALID_MAGIC) {
		mudlog(LOG_SYSERR, "Invalid character in do_save (act.other.c)");
		return;
	}


	if(IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF))) {
		return;
	}
	arg=one_argument(arg,buf);
	if(IS_MAESTRO_DEL_CREATO(ch) && *buf) {
		do_passwd(ch,buf,CMD_SAVE);
		return;
	}

	if(IS_POLY(ch)) {
		/*
		 * Polymorphed
		 * swap stuff, and equipment
		 */
		tmp=0;
		if(ch->desc) {
			tmp = ch->desc->original;    /* tmp = the original characer */
		}

		mudlog(LOG_SAVE,"E' un poly: chiamato per %s, agisco su %s",
			   GET_NAME(ch),
			   GET_NAME(tmp));
		if(!tmp) {
			mudlog(LOG_SYSERR, "do_save ended (!tmp).");
			return;
		}

		tl = tmp->carrying;
		/*
		 * there is a bug with this:  When you save, the alignment thing is
		 * checked, to see if you are supposed to be wearing what you are.
		 * If your stuff gets kicked off your body, it will end up in room #3, on
		 * the floor, and in the inventory of the polymorphed monster.
		 * This is a "bad" thing.  So, to fix it, each item in the inventory is
		 * checked.  if it is in a room, it is moved from the room, back to the
		 * correct inventory slot.
		 */
		tmp->carrying = ch->carrying;
		for(i = 0; i < MAX_WEAR; i++) {
			/* move all the mobs eq to the ch */
			teq[ i ] = tmp->equipment[ i ];
			tmp->equipment[ i ] = ch->equipment[ i ];
		}
		GET_EXP(tmp) = GET_EXP(ch);
		GET_GOLD(tmp) = GET_GOLD(ch);
		GET_ALIGNMENT(tmp) = GET_ALIGNMENT(ch);
		mudlog(LOG_SAVE,"recep_offer su %s",
			   GET_NAME(tmp));
		recep_offer(tmp, NULL, &cost, 0);
		mudlog(LOG_SAVE,"save_obj su %s",
			   GET_NAME(tmp));
		save_obj(tmp, &cost, 0);
		mudlog(LOG_SAVE,"save_char su %s",
			   GET_NAME(ch));
		/* Fangala, trovato il bug del polymorph!
		 * save_char controlla per i poly, e salva il char originale, su
		 * cui noi abbiamo appena ricaricato tutto l'eq del poly
		 * SENZA applicargli le correzioni.
		 * save_char le rimuove..... ed ecco spiegato il busillis.
		 * SPostando la save_char a dopo il ripristino dell'eq dovrebbe
		 * risolversi il problema.
		 * */
		/*save_char( ch, AUTO_RENT );  */

		tmp->carrying = tl;

		for(i = 0; i < MAX_WEAR; i++) {
			tmp->equipment[ i ] = teq[ i ];
			if(ch->equipment[ i ] && ch->equipment[ i ]->in_room != -1) {
				o = ch->equipment[ i ];
				ch->equipment[ i ] = 0;
				obj_from_room(o);
				equip_char(ch, o, i);    /* equip the correct slot */
			}
		}

		save_char(ch, AUTO_RENT, 0);


	}
	else {
		recep_offer(ch, NULL, &cost, 0);
		save_obj(ch, &cost, 0);
		save_char(ch, AUTO_RENT, 0);
	}
	if(cmd == CMD_SAVE)
		send_to_char("Salvato.\n\r",
					 ch);
	mudlog(LOG_CHECK, "do_save ended.");
}


ACTION_FUNC(do_not_here) {
	send_to_char("Mi dispiace, ma non puoi farlo qui!\n\r",ch);
}


ACTION_FUNC(do_sneak) {
	struct affected_type af;
	byte percent;

	if(IS_AFFECTED(ch, AFF_SNEAK)) {
		affect_from_char(ch, SKILL_SNEAK);
		if(IS_AFFECTED(ch, AFF_HIDE)) {
			REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
		}
		send_to_char("Occhio... ti sentono!.\n\r",ch);
		return;
	}

	if(!ch->skills || !IS_SET(ch->skills[SKILL_SNEAK].flags, SKILL_KNOWN)) {
		send_to_char("You're not trained to walk silently!\n\r", ch);
		return;
	}

	if(HasClass(ch,CLASS_RANGER) && !OUTSIDE(ch)) {
		send_to_char("You must do this outdoors!\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(ch, AFF_SILENCE)) {
		if(EqWBits(ch, ITEM_ANTI_THIEF)) {
			send_to_char("Dura muoversi silenziosamente con tutta quella ferraglia addosso!\n\r", ch);
			return;
		}
		if(HasWBits(ch, ITEM_HUM)) {
			send_to_char("Si, bravo.. ronzi come un calabrone e vorresti muoverti in silenzio?\n\r",
						 ch);
			return;
		}
	}

	send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);

	percent=number(1,101); /* 101% is a complete failure */

	if(!ch->skills) {
		return;
	}

	if(IS_AFFECTED(ch, AFF_SILENCE)) {
		percent = MIN(1, percent-35);    /* much easier when silenced */
	}

	if(percent > MIN(100, ch->skills[SKILL_SNEAK].learned) +
			dex_app_skill[ static_cast<int>(GET_DEX(ch)) ].sneak) {
		LearnFromMistake(ch, SKILL_SNEAK, 1, 90);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		return;
	}

	af.type = SKILL_SNEAK;
	af.duration = GET_LEVEL(ch, BestThiefClass(ch));
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);
	WAIT_STATE(ch, PULSE_VIOLENCE);

}
ACTION_FUNC(do_tspy) {
	struct affected_type af;

	if(affected_by_spell(ch, SKILL_TSPY)) {
		affect_from_char(ch, SKILL_TSPY);
		send_to_char("Smetti di origliare.\n\r",ch);
		return;
	}

	if(!ch->skills || !IS_SET(ch->skills[SKILL_TSPY].flags, SKILL_KNOWN)) {
		send_to_char("Sei un pessimo ascoltatore!\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}

	send_to_char("Cerchi di origliare le conversazioni nella stanza.\n\r", ch);

	af.type = SKILL_TSPY;
	if IS_DIO_MINORE(ch) {
		af.duration = GET_LEVEL(ch,BestThiefClass(ch));
	}
	else {
		af.duration = GET_LEVEL(ch, BestThiefClass(ch))/(IS_SINGLE(ch)?1:10);
	}
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	WAIT_STATE(ch, PULSE_VIOLENCE);

}



ACTION_FUNC(do_hide) {
	byte percent;



	if(IS_AFFECTED(ch, AFF_HIDE)) {
		REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
	}

	if(!HasClass(ch, CLASS_THIEF|CLASS_MONK|CLASS_BARBARIAN|CLASS_RANGER)
			/**** SALVO skills prince
			    vvvvvvvvvvvvvvvvv ****/
			&& !IS_PRINCE(ch)) {
		send_to_char("You're not trained to hide!\n\r", ch);
		return;
	}

	if(!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER)) {
		send_to_char("You attempt to hide in the shadows.\n\r", ch);
	}
	else {
		send_to_char("You attempt to camouflage yourself.\n\r",ch);
	}

	if(HasClass(ch, CLASS_BARBARIAN|CLASS_RANGER) && !OUTSIDE(ch)) {
		send_to_char("You must do this outdoors.\n\r",ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Yeah... right... while mounted\n\r", ch);
		return;
	}


	percent=number(1,101); /* 101% is a complete failure */

	if(!ch->skills) {
		return;
	}

	if(percent > MIN(100, ch->skills[SKILL_HIDE].learned) +
			dex_app_skill[ static_cast<int>(GET_DEX(ch)) ].hide) {
		LearnFromMistake(ch, SKILL_HIDE, 1, 90);
		WAIT_STATE(ch, PULSE_VIOLENCE*1);
		return;
	}

	SET_BIT(ch->specials.affected_by, AFF_HIDE);
	WAIT_STATE(ch, PULSE_VIOLENCE*1);

}


ACTION_FUNC(do_steal) {
	struct char_data* victim;
	struct obj_data* obj;
	char victim_name[240];
	char obj_name[240];
	char buf[240];
	int percent;
	int gold, eq_pos;
	bool ohoh = FALSE;

	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "Non in questa stanza di pace!\n\r") &&
			!IS_DIO(ch)) {
		return;
	}


	arg = one_argument(arg, obj_name);
	only_argument(arg, victim_name);

	if(!HasClass(ch, CLASS_THIEF)) {
		send_to_char("Non sei un ladro!\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Si, si, certo, mentre sei a cavallo...\n\r", ch);
		return;
	}

	if(!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Rubare cosa da chi?\n\r", ch);
		return;
	}
	else if(victim == ch) {
		send_to_char("Dai, su, e' abbastanza stupida la cosa!\n\r", ch);
		return;
	}



	if(IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		send_to_char("Rubare ad un Dio?!?  Ma cosa credi di fare!\n\r", ch);
		mudlog(LOG_PLAYERS, "%s tried to steal from GOD %s", GET_NAME(ch),
			   GET_NAME(victim));
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*2);  /* they're gonna have to wait. */

	if((GetMaxLevel(ch) < CHUMP) && (IS_PC(victim))) {
		send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
		send_to_char("unless you are at least chump level. \n\r", ch);
		return;
	}
	if(!IS_PKILLER(ch) && IS_PC(victim)&& !IS_DIO(ch)) {
		send_to_char("Devi essere un pkiller per rubare ad un altro giocatore\n\r",ch);
		return;
	}

	if((!victim->desc) && (!IS_NPC(victim))) {
		return;
	}

	/* 101% is a complete failure */
	percent  = number(1, 101) - dex_app_skill[static_cast<int>(GET_DEX(ch))].p_pocket;
	percent += dex_app_skill[static_cast<int>(GET_DEX(victim))].p_pocket;
	percent -= GetMaxLevel(ch);
	percent += GET_AVE_LEVEL(victim);

	if(GET_POS(victim) < POSITION_SLEEPING || GetMaxLevel(ch) >=MAESTRO_DEI_CREATORI) {
		percent = -1;    /* ALWAYS SUCCESS */
	}

	if(GetMaxLevel(victim)>MAX_MORT && GetMaxLevel(ch)<MAESTRO_DEI_CREATORI) {
		percent = 101;    /* Failure */
	}

	if(str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {

		if(!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {

			for(eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
				if(victim->equipment[eq_pos] &&
						(isname(obj_name, victim->equipment[eq_pos]->name)) &&
						CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
					obj = victim->equipment[eq_pos];
					break;
				}

			if(!obj) {
				act("$E non ha questo oggetto.",FALSE,ch,0,victim,TO_CHAR);
				return;
			}
			else {   /* It is equipment */
				if((GET_POS(victim) > POSITION_SLEEPING)) {
					send_to_char("Rubare equipaggiamento ora? Impossibile!\n\r", ch);
					return;
				}
				else {
					act("You unequip $p and steal it.",FALSE, ch, obj,0, TO_CHAR);
					act("$n steals $p from $N.",TRUE,ch,obj,victim,TO_NOTVICT);
					obj_to_char(unequip_char(victim, eq_pos), ch);
#if NODUPLICATES
					do_save(ch, "", 0);
					do_save(victim, "", 0);
#endif
					if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
						GET_ALIGNMENT(ch)-=20;
					}

                    if(HasClass(ch, CLASS_THIEF) && IS_PC(ch))
                    {
                        if(IS_POLY(ch))
                        {
                            ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                            if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                                SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                        }
                        else
                        {
                            ch->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                            if(!IS_SET(ch->specials.act,PLR_ACHIE))
                                SET_BIT(ch->specials.act, PLR_ACHIE);
                        }

                        CheckAchie(ch, ACHIE_THIEF_3, CLASS_ACHIE);
                    }

				}
			}
		}
		else {
			/* obj found in inventory */

			if(IS_OBJ_STAT(obj,ITEM_NODROP) && !IS_IMMORTAL(ch)) {
				send_to_char("Non riesci a rubare quella cosa... probabilmente e' maledetta!\n\r", ch);
			}

			if(!CheckEgoGet(ch, obj)) {
				return;
			}
#if EGO

			if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN  && !EgoSave(ch)) {
				act("Provi a rubare $p, ma poi cambi idea.",FALSE, ch, obj,0, TO_CHAR);
				return;
			}
			/* Gli oggetti rari vanno con chi ha piu' carisma! Gaia 2001 */
			if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
				percent += (GET_CHR(victim)- GET_CHR(ch))*5 ;
			}

#endif

			percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

			if(AWAKE(victim) &&
					percent > MIN(100, ch->skills[SKILL_STEAL].learned)) {
				ohoh = TRUE;
				act("Yikes, you fumbled!", FALSE, ch,0,0,TO_CHAR);
				LearnFromMistake(ch, SKILL_STEAL, 0, 90);
				SET_BIT(ch->player.user_flags,STOLE_1);
				act("$n ha tentato di rubarti qualche cosa!",FALSE,ch,0,victim,TO_VICT);
				act("$n ha tentato di rubare qualche cosa a $N.", TRUE, ch, 0, victim, TO_NOTVICT);
			}
			else {
				/* Steal the item */
				if((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						send_to_char("Preso!\n\r", ch);
#if NODUPLICATES
						do_save(ch, "", 0);
						do_save(victim, "", 0);
#endif
						if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
							GET_ALIGNMENT(ch)-=20;
						}

                        if(HasClass(ch, CLASS_THIEF) && IS_PC(ch))
                        {
                            if(IS_POLY(ch))
                            {
                                ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                                if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                                    SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                            }
                            else
                            {
                                ch->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                                if(!IS_SET(ch->specials.act,PLR_ACHIE))
                                    SET_BIT(ch->specials.act, PLR_ACHIE);
                            }

                            CheckAchie(ch, ACHIE_THIEF_3, CLASS_ACHIE);
                        }

					}
					else {
						send_to_char("You cannot carry that much.\n\r", ch);
					}
				}
				else {
					send_to_char("You cannot carry that much.\n\r", ch);
				}
			}
		}
	}
	else {
		/* Steal some coins */
		if(AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
			ohoh = TRUE;
			act("Oops..", FALSE, ch,0,0,TO_CHAR);
			if(ch->skills[SKILL_STEAL].learned < 90) {
				act("Even though you were caught, you realize your mistake and promise to remember.",FALSE, ch, 0, 0, TO_CHAR);
				ch->skills[SKILL_STEAL].learned++;
				if(ch->skills[SKILL_STEAL].learned >= 90) {
					send_to_char("You are now learned in this skill!\n\r", ch);
				}
			}
			act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
			act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
		}
		else {
			/* Steal some gold coins */
			gold = static_cast<int>((GET_GOLD(victim)*number(1,10))/100);
			gold = MIN(number(5000,10000), gold);
			if(gold > 0) {
				GET_GOLD(ch) += gold;
				GET_GOLD(victim) -= gold;
				sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
				send_to_char(buf, ch);
				if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
					GET_ALIGNMENT(ch)-=20;
				}

                if(HasClass(ch, CLASS_THIEF) && IS_PC(ch))
                {
                    if(IS_POLY(ch))
                    {
                        ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                        if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                            SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                    }
                    else
                    {
                        ch->specials.achievements[CLASS_ACHIE][ACHIE_THIEF_3] += 1;
                        if(!IS_SET(ch->specials.act,PLR_ACHIE))
                            SET_BIT(ch->specials.act, PLR_ACHIE);
                    }

                    CheckAchie(ch, ACHIE_THIEF_3, CLASS_ACHIE);
                }
			}
			else {
				send_to_char("You couldn't get any gold...\n\r", ch);
			}
		}
	}

	if(ohoh && IS_NPC(victim) && AWAKE(victim)) {
		if(IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
			sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
			do_shout(victim, buf, 0);
			do_say(victim, "Don't you ever do that again!", 0);
		}
		else {
			if(CAN_SEE(victim, ch)) {
				hit(victim, ch, TYPE_UNDEFINED);
			}
			else if(number(0,1)) {
				hit(victim, ch, TYPE_UNDEFINED);
			}
		}
	}
}

ACTION_FUNC(do_practice) {
	char buf[MAX_STRING_LENGTH*2], buffer[MAX_STRING_LENGTH*2], temp[20];
	int i,max;


	buffer[0] = '\0';

	if(cmd != CMD_PRACTICE) {
		return;
	}

	if(!ch->skills) {
		return;
	}

	for(; isspace(*arg); arg++);

	if(!arg) {
		send_to_char("You need to supply a class for that.",ch);
		return;
	}

	switch(*arg) {
	case 'w':
	case 'W':
	case 'f':
	case 'F': {
		if(!HasClass(ch, CLASS_WARRIOR)) {
			send_to_char("Scommetto che pensi di essere un guerriero.\n\r", ch);
			return;
		}
		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(i = 0; *spells[ i ] != '\n' && i < MAX_SPL_LIST; i++)
			if(CheckPrac(CLASS_WARRIOR,i+1,GetMaxLevel(ch)) && ch->skills[i+1].learned &&  // SALVO uso la nuova funz
					IS_SET(ch->skills[ i + 1 ].flags, SKILL_KNOWN)) {
				sprintf(buf,"%-30s %s",spells[i], how_good(ch->skills[i+1].learned));
				if(IsSpecialized(ch->skills[i+1].special)) {
					strcat(buf," (special)");
				}
				strcat(buf," \n\r");
				if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
					break;
				}
				strcat(buffer, buf);
				strcat(buffer, "\r");
			}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 't':
	case 'T': {

		if(!HasClass(ch, CLASS_THIEF)) {
			send_to_char("Scommetto che pensi di essere un ladro.\n\r", ch);
			return;
		}
		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
			if(CheckPrac(CLASS_THIEF,i+1,GetMaxLevel(ch)) && ch->skills[i+1].learned  // SALVO uso la nuova funz
					&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
				sprintf(buf,"%-30s %s",spells[i],
						how_good(ch->skills[i+1].learned));
				if(IsSpecialized(ch->skills[i+1].special)) {
					strcat(buf," (special)");
				}
				strcat(buf," \n\r");
				if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
					break;
				}
				strcat(buffer, buf);
				strcat(buffer, "\r");
			}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;
	case 'M':
	case 'm': {
		if(!HasClass(ch, CLASS_MAGIC_USER)) {
			send_to_char("Scommetto che pensi di essere un mago.\n\r", ch);
			return;
		}
		send_to_char("Il tuo libro contiene i seguenti incantesimi:\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=1; max<=GET_LEVEL(ch,MAGE_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_magic == max &&
						IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_magic,
							spells[i],how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[ i + 1 ].special)) {
						strcat(buf," (special)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'S':
	case 's': {
		if(!HasClass(ch, CLASS_SORCERER)) {
			send_to_char("Scommetto che pensi di essere uno stregone.\n\r", ch);
			return;
		}
		SET_BIT(ch->player.user_flags,USE_PAGING);
		sprintf(buf, "Puoi memorizzare un incantesimo %d volte, per un totale di %d "
				"incantesimi memorizzati.\n\r",
				MaxCanMemorize(ch,0),TotalMaxCanMem(ch));
		send_to_char(buf,ch);
		sprintf(buf, "Hai attualmente %d incantesimi memorizzati.\n\r",
				TotalMemorized(ch));
		send_to_char(buf,ch);
		send_to_char("Il tuo libro contiene i seguenti incantesimi:\n\r", ch);
		for(max=1; max<=GET_LEVEL(ch,SORCERER_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_sorcerer == max &&
						IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) &&
						IS_SET(ch->skills[i+1].flags,SKILL_KNOWN_SORCERER)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_sorcerer,
							spells[i],how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					if(MEMORIZED(ch,i+1)) {
						sprintf(temp," x%d",ch->skills[i+1].nummem);
						strcat(buf,temp);
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'C':
	case 'c': {
		if(!HasClass(ch, CLASS_CLERIC)) {
			send_to_char("Scommetto che pensi di essere un chierico.\n\r", ch);
			return;
		}
		SET_BIT(ch->player.user_flags,USE_PAGING);
		send_to_char("Puoi tentare i seguenti incantesimi:\n\r", ch);
		for(max=1; max<=GET_LEVEL(ch,CLERIC_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_cleric==max)
						&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_cleric,
							spells[i],how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					if(MEMORIZED(ch,i+1)) {
						strcat(buf," (memorized)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'D':
	case 'd': {
		if(!HasClass(ch, CLASS_DRUID)) {
			send_to_char("Scommetto che pensi di essere un druido.\n\r", ch);
			return;
		}
		SET_BIT(ch->player.user_flags,USE_PAGING);
		send_to_char("Puoi tentare uno dei senguenti incantesimi:\n\r", ch);
		for(max=1; max<=GET_LEVEL(ch,DRUID_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_druid==max)
						&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_druid,
							spells[i],how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'K':
	case 'k': {
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Scommetto che pensi di essere un monaco.\n\r", ch);
			return;
		}

		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++) {
			if(CheckPrac(CLASS_MONK,i+1,GetMaxLevel(ch)) && ch->skills[i+1].learned  // SALVO uso la nuova funz
					&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
				sprintf(buf,"%-30s %-13s",spells[i],
						how_good(ch->skills[i+1].learned));
				if(IsSpecialized(ch->skills[i+1].special)) {
					strcat(buf," (special)");
				}
				strcat(buf," \n\r");
				if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
					break;
				}
				strcat(buffer, buf);
				strcat(buffer, "\r");
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'b':
	case 'B': {
		if(!HasClass(ch, CLASS_BARBARIAN)) {
			send_to_char("Scommetto che pensi di essere un barbaro.\n\r", ch);
			return;
		}
		SET_BIT(ch->player.user_flags,USE_PAGING);
		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++) {
			if(CheckPrac(CLASS_BARBARIAN,i+1,GetMaxLevel(ch)) && ch->skills[i+1].learned  // SALVO uso la nuova funz
					&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
				sprintf(buf,"%-30s %s",spells[i],how_good(ch->skills[i+1].learned));
				if(IsSpecialized(ch->skills[i+1].special)) {
					strcat(buf," (special)");
				}
				strcat(buf," \n\r");
				if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
					break;
				}
				strcat(buffer, buf);
				strcat(buffer, "\r");
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'R':
	case 'r': {
		if(!HasClass(ch, CLASS_RANGER)) {
			send_to_char("Scommetto che pensi di essere un ranger.\n\r", ch);
			return;
		}

		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=1; max<=GET_LEVEL(ch,RANGER_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++) {
				if(ch->skills[i+1].learned && (spell_info[i+1].min_level_ranger==max)
						&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_ranger,
							spells[i],how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'i':
	case 'I': {
		if(!HasClass(ch, CLASS_PSI)) {
			send_to_char("Scommetto che credi di essere uno psionico.\n\r", ch);
			return;
		}

		SET_BIT(ch->player.user_flags,USE_PAGING);
		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		for(max=1; max<=GET_LEVEL(ch,PSI_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++) {
				if(ch->skills[i+1].learned && (spell_info[i+1].min_level_psi==max) &&
						IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30.30s %-13s",
							spell_info[i+1].min_level_psi,
							spells[i], how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'P':
	case 'p': {
		if(!HasClass(ch, CLASS_PALADIN)) {
			send_to_char("Scommetto che credi di essere un paladino.\n\r", ch);
			return;
		}

		send_to_char("Conosci le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=1; max<=GET_LEVEL(ch,PALADIN_LEVEL_IND); max++) { // SALVO corretto max deve partire da 1
			for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++) {
				if(ch->skills[i+1].learned && (spell_info[i+1].min_level_paladin==max)
						&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN)) {
					sprintf(buf,"[%2d] %-30s %-13s",
							spell_info[i+1].min_level_paladin,
							spells[i], how_good(ch->skills[i+1].learned));
					if(IsSpecialized(ch->skills[i+1].special)) {
						strcat(buf," (special)");
					}
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
			if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
				break;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;

	case 'a': // SALVO implemenento il prac A
	case 'A': {
		send_to_char("Conosci anche le seguenti abilita':\n\r", ch);
		for(i = 0; *spells[ i ] != '\n' && i < MAX_SPL_LIST; i++) {
			if(ch->skills[i+1].learned) {
				if(!CheckPrac(0,i+1,GetMaxLevel(ch))) {
					if(IS_SET(ch->player.iClass, CLASS_DRUID) && CheckPrac(CLASS_DRUID,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_WARRIOR) && CheckPrac(CLASS_WARRIOR,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_THIEF) && CheckPrac(CLASS_THIEF,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_BARBARIAN) && CheckPrac(CLASS_BARBARIAN,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_MONK) && CheckPrac(CLASS_MONK,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_MAGIC_USER) && CheckPrac(CLASS_MAGIC_USER,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_SORCERER) && CheckPrac(CLASS_SORCERER,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_CLERIC) && CheckPrac(CLASS_CLERIC,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_PALADIN) && CheckPrac(CLASS_PALADIN,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_RANGER) && CheckPrac(CLASS_RANGER,i+1,GetMaxLevel(ch))) {
						continue;
					}
					if(IS_SET(ch->player.iClass,CLASS_PSI) && CheckPrac(CLASS_PSI,i+1,GetMaxLevel(ch))) {
						continue;
					}
				}
				else if(!IS_PRINCE(ch) && CheckPrac(0,i+1,GetMaxLevel(ch))) {
					continue;
				}
				sprintf(buf,"%-30s %s",spells[i], how_good(ch->skills[i+1].learned));
				if(IsSpecialized(ch->skills[i+1].special)) {
					strcat(buf," (special)");
				}
				strcat(buf," \n\r");
				send_to_char(buf,ch);
			}
		}
		return;
	}
	break;
	default:
		send_to_char("Quale classe??? oppure { a } per le altre conoscenze.\n\r", ch);
	}

	send_to_char("Vai dal tuo guildmaster per imparare quello che non conosci.\n\r", ch);

}







ACTION_FUNC(do_idea) {
	FILE* fl;
	char str[MAX_INPUT_LENGTH+20];

	if(IS_NPC(ch))        {
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
		return;
	}

	/* skip whites */
	for(; isspace(*arg); arg++);

	if(!*arg)        {
		send_to_char
		("That doesn't sound like a good idea to me.. Sorry.\n\r",ch);
		return;
	}
	if(!(fl = fopen(IDEA_FILE, "a")))        {
		mudlog(LOG_ERROR,"%s:%s","do_idea",strerror(errno));
		send_to_char("Could not open the idea-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), arg);

	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n\r", ch);
}


ACTION_FUNC(do_idea_new)
{
    if (IS_NPC(ch))
        send_to_char("Sei un MOSTRO! Ma se vuoi, scrivi il tuo suggerimento su www.nebbiearcane.it/forum/forum/first-forum :-)\n\r", ch);
    else
        send_to_char("Scrivi la tua idea su www.nebbiearcane.it/forum/forum/first-forum \n\rGrazie!\n\r", ch);
}



/* rirpristinato il comando typo, con 'checktypos' è ora possibile vedere il file */
ACTION_FUNC(do_typo)
{
	FILE* fl;
	char str[MAX_INPUT_LENGTH+20];

	if(IS_NPC(ch))
    {
		send_to_char("I mostri non possono farlo!!!\n\r", ch);
		return;
	}

	/* skip whites */
	for(; isspace(*arg); arg++);

	if(!*arg)
    {
		send_to_char("Prego? Cosa volevi dire?\n\r",         ch);
		return;
	}

	if(!(fl = fopen(TYPO_FILE, "a")))
    {
		mudlog(LOG_ERROR,"%s:%s","do_typo",strerror(errno));
		send_to_char("Qualcosa e' andato storto, contatta gli Dei.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
			GET_NAME(ch), ch->in_room, arg);
	fputs(str, fl);
	fclose(fl);

	send_to_char("Ok. Grazie.\n\r", ch);

    mail_to_god(ch, "Tethys", "Ti ha segnalato un typo, digita '$c0009checktypos list$c0007' per vederlo!\n\r\n\r");
    mail_to_god(ch, "Requiem", "Ti ha segnalato un typo, digita '$c0009checktypos list$c0007' per vederlo!\n\r\n\r");
    mail_to_god(ch, "Croneh", "Ti ha segnalato un typo, digita '$c0009checktypos list$c0007' per vederlo!\n\r\n\r");

    mudlog(LOG_PLAYERS,"%s reports a typo to the Gods.", GET_NAME(ch));
}

ACTION_FUNC(do_bug) {
	FILE* fl;
	char str[MAX_INPUT_LENGTH+20];
	/*
	 *       if (IS_NPC(ch))        {
	 *               send_to_char("You are a monster! Bug off!\n\r", ch);
	 *               return;
	 *       }
	 * */
	/* skip whites */
	for(; isspace(*arg); arg++);

	if(!*arg)        {
		send_to_char("Pardon?\n\r",ch);
		return;
	}
	if(!(fl = fopen(BUG_FILE, "a")))        {
		mudlog(LOG_ERROR,"%s:%s","do_bug",strerror(errno));
		send_to_char("Could not open the bug-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
			GET_NAME(ch), ch->in_room, arg);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Non sempre riesco a leggere i bug.\n\rPer cose urgenti scrivetemi a alar@aspide.it. Grazie!", ch);
}

ACTION_FUNC(do_bug_new)
{
    if (IS_NPC(ch))
        send_to_char("Sei un MOSTRO! Ma se vuoi, segnala il problema su www.nebbiearcane.it/forum/forum/bugs :-)\n\r", ch);
    else
        send_to_char("Segnalaci il problema su www.nebbiearcane.it/forum/forum/bugs \n\rGrazie!\n\r", ch);
}

ACTION_FUNC(do_brief) {
	if(IS_NPC(ch)) {
		if(IS_SET(ch->specials.act, ACT_POLYSELF)) {
			send_to_char("Puoi farlo solo nella tua forma originale.\n\r", ch);
		}
		return;
	}

	if(IS_SET(ch->specials.act, PLR_BRIEF)) {
		send_to_char("Brief mode off.\n\r", ch);
		REMOVE_BIT(ch->specials.act, PLR_BRIEF);
	}
	else {
		send_to_char("Brief mode on.\n\r", ch);
		SET_BIT(ch->specials.act, PLR_BRIEF);
	}
}


ACTION_FUNC(do_compact) {
	if(IS_NPC(ch)) {
		return;
	}

	if(IS_SET(ch->specials.act, PLR_COMPACT))        {
		send_to_char("You are now in the uncompacted mode.\n\r", ch);
		REMOVE_BIT(ch->specials.act, PLR_COMPACT);
	}
	else        {
		send_to_char("You are now in compact mode.\n\r", ch);
		SET_BIT(ch->specials.act, PLR_COMPACT);
	}
}


char* Condition(struct char_data* ch) {
	int   c;
	static char buf[100];
	static char* p;
#if 0
	float a, b, t;

	a = (float)GET_HIT(ch);
	b = (float)GET_MAX_HIT(ch);

	t = a / b;
#endif
	c = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);

	strcpy(buf, how_good(c));
	p = buf;
	return(p);

}

char* Tiredness(struct char_data* ch) {
	int   c;
	static char buf[100];
	static char* p;
#if 0
	float a, b, t;

	a = (float)GET_MOVE(ch);
	b = (float)GET_MAX_MOVE(ch);

	t = a / b;
#endif
	c = (100 * GET_MOVE(ch)) / GET_MAX_MOVE(ch);

	strcpy(buf, how_good(c));
	p = buf;
	return(p);

}
ACTION_FUNC(do_group) {
	char name[256], buf[256];
	struct char_data* victim, *k;
	struct follow_type* f;
	bool found;

	const char* rand_groupname[] = {
		"I predatori delle Nebbie",
		"Il clan mcLeod",
		"I camminatori",
		"Gli straccioni di Xannie",
		"I rinnegati di Alar",
		"I seguaci di Darkstar",
		"I cacciatori di Draghi",
        "I defunti di LadyOfPain",
        "I buggati di Requiem",
        "I devoti della Luna di Isildur",
        "Le fotomonelle di Flyp",
        "I fumatori di pipa di Jethro",
        "Gli infortunati di Denethor",
        "I connessi di Ryltar",
        "Le $c0008Ombre$c0007"
	};

	const int nMaxGroupName = 14;

	only_argument(arg, name);

	if(!*name) {
		if(!IS_AFFECTED(ch, AFF_GROUP)) {
			send_to_char("But you are a member of no group?!\n\r", ch);
		}
		else {
			if(ch->specials.group_name) {
				sprintf(buf,"$c0015Your group \"%s\" consists of:", ch->specials.group_name);
			}
			else if(ch->master && ch->master->specials.group_name) {
				sprintf(buf,"$c0015Your group \"%s\" consists of:", ch->master->specials.group_name);
			}
			else {
				sprintf(buf,"$c0015Your group consists of:");
			}
			act(buf,FALSE,ch,0,0,TO_CHAR);
			if(ch->master) {
				k = ch->master;
			}
			else {
				k = ch;
			}

			if(IS_AFFECTED(k, AFF_GROUP) &&
					GET_MAX_HIT(k) >0 &&
					GET_MAX_MANA(k) >0 &&
					GET_MAX_MOVE(k) >0) {
				sprintf(buf, "$c0014    %-15s $c0011(Head of group) $c0006HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
						fname(k->player.name),
						(static_cast<float>(GET_HIT(k)) / static_cast<int>(GET_MAX_HIT(k))) * 100.0+0.5,
						(static_cast<float>(GET_MANA(k))/ static_cast<int>(GET_MAX_MANA(k))) * 100.0+0.5,
						(static_cast<float>(GET_MOVE(k))/ static_cast<int>(GET_MAX_MOVE(k))) * 100.0+0.5
					   );
				act(buf,FALSE,ch, 0, k, TO_CHAR);

			}

			for(f=k->followers; f; f=f->next) {
				if(IS_AFFECTED(f->follower, AFF_GROUP) &&
						GET_MAX_HIT(f->follower) >0 &&
						GET_MAX_MANA(f->follower) >0 &&
						GET_MAX_MOVE(f->follower) >0) {
					sprintf(buf, "$c0014    %-15s             $c0011%s $c0006HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
							fname(f->follower->player.name),
							(IS_AFFECTED2(f->follower,AFF2_CON_ORDER))?"(O)":"   ",
							((float)GET_HIT(f->follower) /(int)GET_MAX_HIT(f->follower)) *100.0+0.5,
							((float)GET_MANA(f->follower)/(int)GET_MAX_MANA(f->follower)) * 100.0+0.5,
							((float)GET_MOVE(f->follower)/(int)GET_MAX_MOVE(f->follower)) * 100.0+0.5);
					act(buf,FALSE,ch, 0, f->follower, TO_CHAR);
				}
			}
		}

		return;
	}

	if(!(victim = get_char_room_vis(ch, name))) {
		send_to_char("No one here by that name.\n\r", ch);
	}
	else {

		if(ch->master) {
			act("You can not enroll group members without being head of a group.",
				FALSE, ch, 0, 0, TO_CHAR);
			return;
		}

		found = FALSE;

		if(victim == ch) {
			found = TRUE;
		}
		else {
			for(f=ch->followers; f; f=f->next) {
				if(f->follower == victim) {
					found = TRUE;
					break;
				}
			}
		}

		if(found) {
			if(IS_AFFECTED(victim, AFF_GROUP)) {
				act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
				act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
				REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
				REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);
			}
			else {
				if(IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
					act("You really don't want $N in your group.", FALSE, ch, 0, victim, TO_CHAR);
					return;
				}
				if(IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
					act("Now now.  That would be CHEATING!",FALSE,ch,0,0,TO_CHAR);
					return;
				}
				act("$n is now a member of $N's group.",
					FALSE, victim, 0, ch, TO_ROOM);
				act("You are now a member of $N's group.",
					FALSE, victim, 0, ch, TO_CHAR);
				SET_BIT(victim->specials.affected_by, AFF_GROUP);
				REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);
				SET_BIT(ch->specials.affected_by, AFF_GROUP);
				REMOVE_BIT(ch->specials.affected_by2, AFF2_CON_ORDER);

				/* set group name if not one */
				if(!ch->master && !ch->specials.group_name && ch->followers) {
					int gnum=number(0, nMaxGroupName);
					ch->specials.group_name = strdup(rand_groupname[ gnum ]);
					sprintf(buf,"You form <%s> adventuring group!",rand_groupname[gnum]);
					act(buf,FALSE,ch,0,0,TO_CHAR);
				}
			}
		}
		else {
			act("$N must follow you, to enter the group",
				FALSE, ch, 0, victim, TO_CHAR);
		}
	}
}

ACTION_FUNC(do_group_name) {
	int count;
	struct follow_type* f;

	/* check to see if this person is the master */
	if(ch->master || !IS_AFFECTED(ch, AFF_GROUP)) {
		send_to_char("You aren't the master of a group.\n\r", ch);
		return;
	}
	/* check to see at least 2 pcs in group      */
	for(count=0,f=ch->followers; f; f=f->next) {
		if(IS_AFFECTED(f->follower, AFF_GROUP) && IS_PC(f->follower)) {
			count++;
		}
	}
	if(count < 1) {
		send_to_char("You can't have a group with just one player!\n\r", ch);
		return;
	}
	/* free the old ch->specials.group_name           */
	if(ch->specials.group_name) {
		free(ch->specials.group_name);
	}
	/* set ch->specials.group_name to the argument    */
	for(; *arg==' '; arg++);
	send_to_char("\n\rSetting your group name to :", ch);
	send_to_char(arg, ch);
	send_to_char("\n\r",ch);
	ch->specials.group_name = strdup(arg);

}

ACTION_FUNC(do_quaff) {
	char buf[MAX_INPUT_LENGTH];
	struct obj_data* temp;
	int i;
	bool equipped;

	equipped = FALSE;

	only_argument(arg,buf);

	if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		temp = ch->equipment[HOLD];
		equipped = TRUE;
		if((temp==0) || !isname(buf, temp->name)) {
			act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
			return;
		}
	}
	if(!IS_IMMORTAL(ch)) {
		if(GET_COND(ch,FULL)>23) {
			act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
			return;
		}
		else {
			GET_COND(ch, FULL)+=1;
		}
	}

	if(temp->obj_flags.type_flag!=ITEM_POTION) {
		act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
		return;
	}


	/*  my stuff */
	if(ch->specials.fighting) {
		if(equipped) {
			if(number(1,20) > ch->abilities.dex) {
				act("$n is jolted and drops $p!  It shatters!",
					TRUE, ch, temp, 0, TO_ROOM);
				act("You arm is jolted and $p flies from your hand, *SMASH*",
					TRUE, ch, temp, 0, TO_CHAR);
				if(equipped) {
					temp = unequip_char(ch, HOLD);
				}
				extract_obj(temp);
				return;
			}
		}
		else {
			if(number(1,20) > ch->abilities.dex - 4) {
				act("$n is jolted and drops $p!  It shatters!",
					TRUE, ch, temp, 0, TO_ROOM);
				act("You arm is jolted and $p flies from your hand, *SMASH*",
					TRUE, ch, temp, 0, TO_CHAR);
				extract_obj(temp);
				return;
			}
		}
	}

	act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
	act("You quaff $p which dissolves.",FALSE, ch, temp,0, TO_CHAR);
	for(i=1; i<4; i++) {
		if(temp->obj_flags.value[i] >= 1) {
			((*spell_info[temp->obj_flags.value[i]].spell_pointer)(static_cast<byte>(temp->obj_flags.value[0]), ch, "", SPELL_TYPE_POTION, ch, temp));
		}
	}
	if(equipped) {
		temp = unequip_char(ch, HOLD);
	}

	extract_obj(temp);

	WAIT_STATE(ch, PULSE_VIOLENCE);

}


ACTION_FUNC(do_recite) {
	char buf[MAX_INPUT_LENGTH];
	struct obj_data* scroll, *obj;
	struct char_data* victim;
	int i, bits;
	bool equipped;

	equipped = FALSE;
	obj = 0;
	victim = 0;

	if(!ch->skills) {
		return;
	}

	arg = one_argument(arg,buf);

	if(!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		scroll = ch->equipment[HOLD];
		equipped = TRUE;
		if((scroll==0) || !isname(buf, scroll->name)) {
			act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
			return;
		}
	}

	if(scroll->obj_flags.type_flag!=ITEM_SCROLL) {
		act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if(*arg) {
		bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
							FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
		if(bits == 0) {
			send_to_char("No such thing around to recite the scroll on.\n\r", ch);
			return;
		}
	}
	else {
		victim = ch;
	}

	if(!HasClass(ch, CLASS_MAGIC_USER) &&
			!HasClass(ch, CLASS_CLERIC) &&
			!HasClass(ch, CLASS_SORCERER)) {
		if(number(1,95) > ch->skills[SKILL_READ_MAGIC].learned ||
				ch->skills[SKILL_READ_MAGIC].learned == 0) {
			WAIT_STATE(ch, PULSE_VIOLENCE*3);
			send_to_char("After several seconds of study, your head hurts trying "
						 "to understand.\n\r", ch);
			return;
		}
	}

	act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
	act("You recite $p which bursts into flame.",FALSE,ch,scroll,0,TO_CHAR);

	for(i = 1; i < 4; i++) {
		if(scroll->obj_flags.value[0] > 0) {
			/* spells for casting */
			if(scroll->obj_flags.value[i] >= 1) {
				if(IS_SET(spell_info[scroll->obj_flags.value[i]].targets,
						  TAR_VIOLENT) &&
						check_peaceful(ch, "Impolite magic is banned here.")) {
					continue;
				}

				if(check_nomagic(ch,"The magic is blocked by unknown forces.\n\r",
								 "The magic dissolves powerlessly")) {
					continue;
				}

				void (*pSpellFunc)(byte, struct char_data*, const char*, int,
								   struct char_data*, struct obj_data*);
				pSpellFunc = spell_info[scroll->obj_flags.value[i]].spell_pointer;

				if(pSpellFunc)
					(*pSpellFunc)((byte)scroll->obj_flags.value[0], ch, "",
								  SPELL_TYPE_SCROLL, victim, obj);
				else
					send_to_char("Questa pergamena non sembra sortire alcun effetto.",
								 ch);
			}
		}
		else {
			/* this is a learning scroll */
			//if (scroll->obj_flags.value[0] < -30)  /* max learning is 30% */
			//  scroll->obj_flags.value[0] = -30;

			if(scroll->obj_flags.value[i] > 0) {
				/* positive learning */
				if(ch->skills) {
					if((ch->skills[scroll->obj_flags.value[i]].learned -
							scroll->obj_flags.value[0]) <= 95)
						ch->skills[scroll->obj_flags.value[i]].learned +=
							(-scroll->obj_flags.value[0]);
				}
			}
			else {
				/* negative learning (cursed */
				if(scroll->obj_flags.value[i] < 0) {
					/* 0 = blank */
					if(ch->skills) {
						if(ch->skills[-scroll->obj_flags.value[i]].learned > 0)
							ch->skills[-scroll->obj_flags.value[i]].learned +=
								scroll->obj_flags.value[0];
						ch->skills[-scroll->obj_flags.value[i]].learned =
							MAX(0, ch->skills[scroll->obj_flags.value[i]].learned);
					}
				}
			}
		}
	}
	if(equipped) {
		scroll = unequip_char(ch, HOLD);
	}

	extract_obj(scroll);

}



ACTION_FUNC(do_use) {
	char buf[MAX_INPUT_LENGTH];
	struct char_data* tmp_char;
	struct obj_data* tmp_object, *stick;

	int bits, vnum = 0;

	arg = one_argument(arg,buf);

	if(ch->equipment[HOLD] == 0 ||
			!isname(buf, ch->equipment[HOLD]->name)) {

		act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);

		return;
	}

#if 0
	if(!IS_PC(ch) && ch->master) {
		act("$n looks confused, and shrugs helplessly", FALSE, ch, 0, 0, TO_ROOM);
		return;
	}
#endif

	if(RIDDEN(ch)) {
		return;
	}

	stick = ch->equipment[HOLD];

	if(stick->obj_flags.type_flag == ITEM_STAFF)  {

		act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
		act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);
		if(stick->obj_flags.value[2] > 0) {   /* Is there any charges left? */
			stick->obj_flags.value[2]--;

			if(check_nomagic(ch,"The magic is blocked by unknown forces.",
							 "The magic is blocked by unknown forces.")) {
				return;
			}

			((*spell_info[stick->obj_flags.value[3]].spell_pointer)
			 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
			WAIT_STATE(ch, PULSE_VIOLENCE);
		}
		else {
			send_to_char("The staff seems powerless.\n\r", ch);
		}
	}
	else if(stick->obj_flags.type_flag == ITEM_WAND) {

		bits = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV |
							FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

		if(bits) {
			struct spell_info_type*        spellp;

			spellp = spell_info + (stick->obj_flags.value[3]);

			if(bits == FIND_CHAR_ROOM) {
				act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
				act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
			}
			else {
				act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
				act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
			}

			if(IS_SET(spellp->targets, TAR_VIOLENT) &&
					check_peaceful(ch, "Impolite magic is banned here.")) {
				return;
			}

			if(stick->obj_flags.value[2] > 0) {
				/* Is there any charges left? */
				stick->obj_flags.value[2]--;

				if(check_nomagic(ch, "The magic is blocked by unknown forces.",
								 "The magic is blocked by unknown forces.")) {
					return;
				}

				((*spellp->spell_pointer)((byte) stick->obj_flags.value[0], ch, "",
										  SPELL_TYPE_WAND, tmp_char, tmp_object));
				WAIT_STATE(ch, PULSE_VIOLENCE);
			}
			else {
				send_to_char("The wand seems powerless.\n\r", ch);
			}
		}
		else {
			send_to_char("What should the wand be pointed at?\n\r", ch);
		}
	}
    else if(stick->obj_flags.type_flag == ITEM_TREASURE && (vnum = (stick->item_number >= 0) ? obj_index[stick->item_number].iVNum : 0) == OBJ_REWARD)
    {
        string sbch, sbroom;
        char name[25], risultato[255];
        int percent, bonus = 1, i;
        bool found = FALSE;

        tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);

        if(tmp_object)
        {
            if(IS_RARE(tmp_object))
            {
                act("Non puoi spargere la polvere su $p.", FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }

            strcpy(name, "ED");
            strcat(name, GET_NAME(ch));
            if(!isname(name, stick->name))
            {
                act("Non puoi spargere $p da nessuna parte, non e' tua!", FALSE, ch, stick, 0, TO_CHAR);
                return;
            }

            if(IS_OBJ_STAT2(tmp_object, ITEM2_PERSONAL) && !pers_on(ch, tmp_object))
            {
                act("Non puoi farlo, $p non ti appartiene!", FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }

            boost::format fmt("%s $p su %s.\n\r");
            fmt % "Spargi" % tmp_object->short_description;
            sbch.append(fmt.str().c_str());
            fmt.clear();
            fmt % "$n sparge" % tmp_object->short_description;
            sbroom.append(fmt.str().c_str());
            fmt.clear();

            percent = number(1, 100);
            if(IS_PRINCE(ch))
            {
                percent += 1;
            }

            if(percent >= 95)
            {
                bonus += 1;
                sprintf(risultato, "Una $c0011luce dorata$c0007 si diffonde tutto intorno per poi venir risucchiata dentro %s.\n\r", tmp_object->short_description);
            }
            else if(percent >= 100)
            {
                bonus += 1;
                sprintf(risultato, "Una $c0015cascata di luce$c0007 scende su %s.\n\r", tmp_object->short_description);
            }
            else
            {
                sprintf(risultato, "Un'intensa $c0015luce$c0007 avvolge %s per un attimo per poi spengersi.\n\r", tmp_object->short_description);
            }

            sbch.append(risultato);
            sbroom.append(risultato);

            for(i = 0; i < MAX_OBJ_AFFECT; i++)
            {
                if(tmp_object->affected[i].location == stick->affected[0].location)
                {
                    if(stick->affected[0].location == APPLY_SPELLFAIL)
                    {
                        tmp_object->affected[i].modifier -= bonus;
                    }
                    else
                    {
                        tmp_object->affected[i].modifier += bonus;
                    }
                    found = TRUE;
                    break;
                }
            }

            if(!found)
            {
                for(i = 0; i < MAX_OBJ_AFFECT; i++)
                {
                    if(tmp_object->affected[i].location == APPLY_NONE)
                    {
                        tmp_object->affected[i].location = stick->affected[0].location;
                        if(stick->affected[0].location == APPLY_SPELLFAIL)
                        {
                            tmp_object->affected[i].modifier -= bonus;
                        }
                        else
                        {
                            tmp_object->affected[i].modifier += bonus;
                        }
                        found = TRUE;
                        break;
                    }
                }
            }

            if(!found)
            {
                act("Non puoi spargere questo tipo di polvere su $p!", FALSE, ch, tmp_object, 0, TO_CHAR);
                return;
            }

            act(sbch.c_str(), FALSE, ch, stick, 0, TO_CHAR);
            act(sbroom.c_str(),TRUE ,ch, stick, 0, TO_ROOM);
            if(!pers_on(ch, tmp_object))
            {
                SetPersonOnSave(ch, tmp_object);
            }
            unequip_char(ch, HOLD);
            extract_obj(stick);
        }
        else
        {
            act("Non hai niente del genere con te! Su cosa vuoi spargere $p?", FALSE, ch, stick, 0, TO_CHAR);
            return;
        }
    }
    else if((vnum = (stick->item_number >= 0) ? obj_index[stick->item_number].iVNum : 0) == NILMYS_FLASK)
    {
        struct obj_data* arkhat_equip;
        struct char_data* Arkhat;
        int r_num = 0, wait_ch = 0, wait_party = 0, rune;

        arg = one_argument(arg, buf);

        if(!strcmp("punta", buf))
        {
            switch(ch->in_room)
            {
                case 9115:
                {
                    if(!IS_SET(stick->iGeneric, NILMYS_PORTAL_ONE))
                    {
                        act("\n\r$c0011Cospargi con cura, ma anche con un certo ribrezzo, il liquido sulla punta rituale,\n\r$c0011cercando di coprire interamente la figura.", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0011$n$c0011 cosparge con cura il liquido di $p$c0011 sulla punta rituale.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        stick->iGeneric += NILMYS_PORTAL_ONE;
                        wait_ch = 3;
                        wait_party = 2;
                    }
                    else
                    {
                        act("\n\r$c0010Ti avvicini punta rituale e ti rendi conto che un liquido viscoso la copre interamente.\n\r", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0010$n$c0011 si avvicina alla punta rituale, ma subito dopo fa un passo indietro.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        wait_ch = 2;
                        wait_party = 1;
												return;
                    }
                }
                    break;

                case 9116:
                {
                    if(!IS_SET(stick->iGeneric, NILMYS_PORTAL_TWO))
                    {
                        act("\n\r$c0011Cospargi con cura, ma anche con un certo ribrezzo, il liquido sulla punta rituale,\n\r$c0011cercando di coprire interamente la figura.", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0011$n$c0011 cosparge con cura il liquido di $p$c0011 sulla punta rituale.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        stick->iGeneric += NILMYS_PORTAL_TWO;
                        wait_ch = 3;
                        wait_party = 2;
                    }
                    else
                    {
                        act("\n\r$c0010Ti avvicini punta rituale e ti rendi conto che un liquido viscoso la copre interamente.\n\r", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0010$n$c0011 si avvicina alla punta rituale, ma subito dopo fa un passo indietro.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        wait_ch = 2;
                        wait_party = 1;
												return;
                    }
                }
                    break;

                case 9117:
                {
                    if(!IS_SET(stick->iGeneric, NILMYS_PORTAL_THREE))
                    {
                        act("\n\r$c0011Cospargi con cura, ma anche con un certo ribrezzo, il liquido sulla punta rituale,\n\r$c0011cercando di coprire interamente la figura.", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0011$n$c0011 cosparge con cura il liquido di $p$c0011 sulla punta rituale.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        stick->iGeneric += NILMYS_PORTAL_THREE;
                        wait_ch = 3;
                        wait_party = 2;
                    }
                    else
                    {
                        act("\n\r$c0010Ti avvicini punta rituale e ti rendi conto che un liquido viscoso la copre interamente.\n\r", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0010$n$c0011 si avvicina alla punta rituale, ma subito dopo fa un passo indietro.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        wait_ch = 2;
                        wait_party = 1;
												return;
                    }
                }
                    break;

                case 9118:
                {
                    if(!IS_SET(stick->iGeneric, NILMYS_PORTAL_FOUR))
                    {
                        act("\n\r$c0011Cospargi con cura, ma anche con un certo ribrezzo, il liquido sulla punta rituale,\n\r$c0011cercando di coprire interamente la figura.", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0011$n$c0011 cosparge con cura il liquido di $p$c0011 sulla punta rituale.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        stick->iGeneric += NILMYS_PORTAL_FOUR;
                        wait_ch = 3;
                        wait_party = 2;
                    }
                    else
                    {
                        act("\n\r$c0010Ti avvicini punta rituale e ti rendi conto che un liquido viscoso la copre interamente.\n\r", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0010$n$c0011 si avvicina alla punta rituale, ma subito dopo fa un passo indietro.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        wait_ch = 2;
                        wait_party = 1;
												return;
                    }
                }
                    break;

                case 9119:
                {
                    if(!IS_SET(stick->iGeneric, NILMYS_PORTAL_FIVE))
                    {
                        act("\n\r$c0011Cospargi con cura, ma anche con un certo ribrezzo, il liquido sulla punta rituale,\n\r$c0011cercando di coprire interamente la figura.", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0011$n$c0011 cosparge con cura il liquido di $p$c0011 sulla punta rituale.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        stick->iGeneric += NILMYS_PORTAL_FIVE;
                        wait_ch = 3;
                        wait_party = 2;
                    }
                    else
                    {
                        act("\n\r$c0010Ti avvicini punta rituale e ti rendi conto che un liquido viscoso la copre interamente.\n\r", FALSE, ch, NULL, NULL, TO_CHAR);
                        act("\n\r$c0010$n$c0011 si avvicina alla punta rituale, ma subito dopo fa un passo indietro.\n\r", FALSE, ch, stick, NULL, TO_ROOM);
                        wait_ch = 2;
                        wait_party = 1;
												return;
                    }
                }
                    break;

                default:
                    act("Non vedi nessuna punta qui.", FALSE, ch, NULL, NULL, TO_CHAR);
                    return;
                    break;
            }

            // lagghiamo chi usa l'ampolla e il party per evitare furbate
            for(tmp_char = real_roomp(ch->in_room)->people; tmp_char; tmp_char = tmp_char->next_in_room)
            {
                if(tmp_char == ch)
                {
                    WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
                }
                else if(tmp_char && is_same_group(ch, tmp_char))
                {
                    WAIT_STATE(tmp_char, PULSE_VIOLENCE * wait_party);
                }
            }

            if(stick->iGeneric == NILMYS_PORTAL_ONE + NILMYS_PORTAL_TWO + NILMYS_PORTAL_THREE + NILMYS_PORTAL_FOUR + NILMYS_PORTAL_FIVE)
            {
                rune = number(1, 3);

                for(tmp_char = real_roomp(ch->in_room)->people; tmp_char; tmp_char = tmp_char->next_in_room)
                {
                    if(IS_PC(tmp_char))
                    {
                        if(tmp_char == ch)
                        {
                            ch->generic = 10;
                            ch->commandp = rune;
                        }
                        else if(tmp_char && (is_same_group(ch, tmp_char) || ch->master == tmp_char->master || ch->master == tmp_char || ch == tmp_char->master))
                        {
                            tmp_char->generic = 10;
                            tmp_char->commandp = rune;
                        }
                    }
                }

                send_to_all("\n\r\n\r");
                send_to_all("$c0008La voce di $c0013Arkhat$c0008, il Dio Divoratore, tuona:\n\r");
                send_to_all("$c0008 'Ho aspettato centinaia di vite mortali per potermi manifestare in questo mondo.$c0007\n\r");
                send_to_all("$c0008  Inutile insetto hai compiuto il tuo ultimo errore.'\n\r");
                send_to_all("\n\r\n\r");

                if((Arkhat = read_mobile(real_mobile(ARKHAT_GOD), REAL)))
                {
                    char_to_room(Arkhat, ARKHAT_GOD_ROOM);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_NECK_1);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_BODY);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_HEAD);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_FEET);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_ARMS);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_ABOUT);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_BACK);

                    r_num = real_object(LOST_SOUL_EQ);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, WEAR_EYES);

                    r_num = real_object(NILNYS_VICTORY_KEY);
                    arkhat_equip = read_object(r_num, REAL);
                    obj_to_char(arkhat_equip, Arkhat);
                    obj_from_char(arkhat_equip);
                    equip_char(Arkhat, arkhat_equip, HOLD);

                    send_to_zone("$c0015[$c0013Arkhat$c0007, il Dio Divoratore$c0015] dice '$c0009ORA CHE SONO LIBERO NESSUNO HA PIU' SPERANZA!$c0015'\n\r\n\r", Arkhat);
                    send_to_zone("$c0015[$c0013Arkhat$c0007, il Dio Divoratore$c0015] dice '$c0009ADESSO MORIRETE!$c0015'\n\r\n\r", Arkhat);
                    return;
                }
            }
        }
        else
        {
            act("Su cosa vuoi usare $p?", FALSE, ch, stick, NULL, TO_CHAR);
            return;
        }
    }
    else if((vnum = (stick->item_number >= 0) ? obj_index[stick->item_number].iVNum : 0) == NILMYS_BLEEDER)
    {
        struct char_data* iskra;

        if(stick->iGeneric == 1)
        {
            act("Hai gia' prelevato abbastanza $c0009sangue$c0007 dalla povera Iskra!", FALSE, ch, NULL, NULL, TO_CHAR);
            return;
        }

        arg = one_argument(arg, buf);

        if(!strcmp("iskra", buf))
        {
            iskra = 0;

            for(tmp_char = real_roomp(ch->in_room)->people; (!iskra) && (tmp_char); tmp_char = tmp_char->next_in_room)
            {
                if(IS_MOB(tmp_char))
                {
                    if(mob_index[tmp_char->nr].iVNum == ISKRA_STANISLAV)
                    {
                        iskra = tmp_char;
                    }
                }
            }

            if(!iskra)
            {
                act("$N non e' qui!", FALSE, ch, NULL, iskra, TO_CHAR);
                return;
            }

            act("$c0008Inserisci lo strano marchingegno nel braccio della giovane Iskra e ne prelevi il $c0009sangue$c0008.\n\r$c0008Lei, coraggiosamente, emette solo un flebile gemito.", FALSE, ch, NULL, NULL, TO_CHAR);
            act("$c0008$n inserisci lo strano marchingegno nel braccio della giovane Iskra prelevandole del $c0009sangue$c0008.\n\r$c0008Lei, coraggiosamente, emette solo un flebile gemito.", FALSE, ch, NULL, iskra, TO_NOTVICT);
            act("$c0008$n inserisce uno strano marchingegno nel tuo braccio ed iniza a prelevare il tuo $c0009sangue$c0008.\n\r$c0008Emetti un flebile gemito per il dolore.", FALSE, ch, NULL, iskra, TO_VICT);

            //  assegno al salassatore il valore di 1 su IGeneric
            stick->iGeneric = 1;
        }
        else
        {
            act("Su chi vuoi usare $p?", FALSE, ch, stick, NULL, TO_CHAR);
            return;
        }
    }
	else {
		send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
	}
}

ACTION_FUNC(do_plr_noshout) {
	char buf[MAX_INPUT_LENGTH];

	if(IS_NPC(ch)) {
		return;
	}

	only_argument(arg, buf);

	if(!*buf) {
		if(IS_SET(ch->specials.act, PLR_DEAF)) {
			send_to_char("You can now hear shouts again.\n\r", ch);
			REMOVE_BIT(ch->specials.act, PLR_DEAF);
		}
		else {
			send_to_char("From now on, you won't hear shouts.\n\r", ch);
			SET_BIT(ch->specials.act, PLR_DEAF);
		}
	}
	else {
		send_to_char("Only the gods can shut up someone else. \n\r",ch);
	}

}

ACTION_FUNC(do_plr_nogossip) {
	char buf[MAX_INPUT_LENGTH];

	if(IS_NPC(ch)) {
		return;
	}

	only_argument(arg, buf);

	if(!*buf) {
		if(IS_SET(ch->specials.act, PLR_NOGOSSIP)) {
			send_to_char("You can now hear gossips again.\n\r", ch);
			REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
		}
		else {
			send_to_char("From now on, you won't hear gossips.\n\r", ch);
			SET_BIT(ch->specials.act, PLR_NOGOSSIP);
		}
	}
	else {
		send_to_char("Only the gods can no gossip someone else. \n\r",ch);
	}

}

ACTION_FUNC(do_plr_noauction) {
	char buf[MAX_INPUT_LENGTH];

	if(IS_NPC(ch)) {
		return;
	}

	only_argument(arg, buf);

	if(!*buf) {
		if(IS_SET(ch->specials.act, PLR_NOAUCTION)) {
			send_to_char("You can now hear auctions again.\n\r", ch);
			REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
		}
		else {
			send_to_char("From now on, you won't hear auctions.\n\r", ch);
			SET_BIT(ch->specials.act, PLR_NOAUCTION);
		}
	}
	else {
		send_to_char("Only the gods can no auction someone else. \n\r",ch);
	}

}

ACTION_FUNC(do_plr_notell) {
	char buf[MAX_INPUT_LENGTH];

	if(IS_NPC(ch)) {
		return;
	}

	only_argument(arg, buf);

	if(!*buf) {
		if(IS_SET(ch->specials.act, PLR_NOTELL)) {
			send_to_char("You can now hear tells again.\n\r", ch);
			REMOVE_BIT(ch->specials.act, PLR_NOTELL);
		}
		else {
			send_to_char("From now on, you won't hear tells.\n\r", ch);
			SET_BIT(ch->specials.act, PLR_NOTELL);
		}
	}
	else {
		send_to_char("Only the gods can notell up someone else. \n\r",ch);
	}

}


ACTION_FUNC(do_alias) {
	char buf[512], buf2[512];
	char* p, *p2;
	int i, num;

	if(cmd == CMD_ALIAS) {
		for(; *arg==' '; arg++);
		if(!*arg) {   /* print list of current aliases */
			if(ch->specials.A_list) {
				for(i=0; i<MAX_ALIAS; i++) {
					if(ch->specials.A_list->com[i]) {
						sprintf(buf,"[%d] %s\n\r",i, ch->specials.A_list->com[i]);
						send_to_char(buf,ch);
					}
				}
			}
			else {
				send_to_char("You have no aliases defined!\n\r", ch);
				return;
			}
		}
		else {    /* assign a particular alias */
			if(!ch->specials.A_list) {
				ch->specials.A_list = (Alias*)malloc(sizeof(Alias));
				for(i=0; i<MAX_ALIAS; i++) {
					ch->specials.A_list->com[i] = (char*)0;
				}
			}
			strcpy(buf, arg);
			p = strtok(buf," ");
			num = atoi(p);
			if(num < 0 || num >= MAX_ALIAS) {
				send_to_char("Numero alias non ammesso \n\r",
							 ch);
				return;
			}
			if(GET_ALIAS(ch,num)) {
				free(GET_ALIAS(ch, num));
				GET_ALIAS(ch, num) = 0;
			}
			/*
			  testing
			*/
			p = strtok(NULL," ");  /* get the command string */
			if(!p) {
				send_to_char("Need to supply a command to alias budee\n\r",ch);
				return;
			}
			p2 = strtok(p," ");  /* check the command, make sure its not an alias
                              */
			if(!p2) {
				send_to_char("Hmmmmm\n\r", ch);
				return;
			}
			if(*p2 >= '0' && *p2 <= '9') {
				send_to_char("Can't use an alias inside an alias\n\r", ch);
				return;
			}
			if(strncmp(p2,"alias",strlen(p2))==0) {
				send_to_char("Can't use the word 'alias' in an alias\n\r", ch);
				return;
			}
			/*
			   verified.. now the copy.
			*/
			if(strlen(p) <= 80) {
				strcpy(buf2, arg);  /* have to rebuild, because buf is full of
                               nulls */
				p = strchr(buf2,' ');
				p++;
				ch->specials.A_list->com[num] = (char*)malloc(strlen(p)+1);
				strcpy(ch->specials.A_list->com[num], p);
			}
			else {
				send_to_char("alias must be less than 80 chars, lost\n\r", ch);
				return;
			}
		}
	}
	else {     /* execute this alias */
		num = cmd - 260;  /* 260 = alias */
		if(num >= 10) {
			num = 0;
		}
		if(ch->specials.A_list) {
			if(GET_ALIAS(ch, num)) {
				strcpy(buf, GET_ALIAS(ch, num));
				if(*arg) {
					sprintf(buf2,"%s %s",buf,arg);
					send_to_char(buf2,ch);
					command_interpreter(ch, buf2);
				}
				else {
					command_interpreter(ch, buf);
				}
			}
		}
	}
}

void Dismount(struct char_data* ch, struct char_data* h, int pos) {

	MOUNTED(ch) = 0;
	RIDDEN(h) = 0;
	GET_POS(ch) = abs(pos);

	check_falling(ch);

}

ACTION_FUNC(do_mount) {
	char name[MAX_INPUT_LENGTH];
	int check;
	struct char_data* horse;


	if(cmd == CMD_MOUNT || cmd == CMD_RIDE) {
		only_argument(arg, name);

		if(!(horse = get_char_room_vis(ch, name))) {
			send_to_char("Mount what?\n\r", ch);
			return;
		}

		if(!IsHumanoid(ch)) {
			send_to_char("You can't ride things!\n\r", ch);
			return;
		}

		if(IsRideable(horse)) {

			if(GET_POS(horse) < POSITION_STANDING) {
				send_to_char("Your mount must be standing\n\r", ch);
				return;
			}

			if(RIDDEN(horse)) {
				send_to_char("Already ridden\n\r", ch);
				return;
			}
			else if(MOUNTED(ch)) {
				send_to_char("Already riding\n\r", ch);
				return;
			}

			check = MountEgoCheck(ch, horse);
			if(check > 5) {
				act("$N snarls and attacks!",
					FALSE, ch, 0, horse, TO_CHAR);
				act("as $n tries to mount $N, $N attacks $n!",
					FALSE, ch, 0, horse, TO_NOTVICT);
				WAIT_STATE(ch, PULSE_VIOLENCE); // mount
				hit(horse, ch, TYPE_UNDEFINED);
				return;
			}
			else if(check > -1) {
				act("$N moves out of the way, you fall on your butt",
					FALSE, ch, 0, horse, TO_CHAR);
				act("as $n tries to mount $N, $N moves out of the way",
					FALSE, ch, 0, horse, TO_NOTVICT);
				WAIT_STATE(ch, PULSE_VIOLENCE); // mount
				GET_POS(ch) = POSITION_SITTING;
				return;
			}


			if(RideCheck(ch, 50)) {
				act("You hop on $N's back", FALSE, ch, 0, horse, TO_CHAR);
				act("$n hops on $N's back", FALSE, ch, 0, horse, TO_NOTVICT);
				act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
				MOUNTED(ch) = horse;
				RIDDEN(horse) = ch;
				GET_POS(ch) = POSITION_MOUNTED;
				REMOVE_BIT(ch->specials.affected_by, AFF_SNEAK);
			}
			else {
				act("You try to ride $N, but falls on $s butt",
					FALSE, ch, 0, horse, TO_CHAR);
				act("$n tries to ride $N, but falls on $s butt",
					FALSE, ch, 0, horse, TO_NOTVICT);
				act("$n tries to ride you, but falls on $s butt",
					FALSE, ch, 0, horse, TO_VICT);
				GET_POS(ch) = POSITION_SITTING;
				WAIT_STATE(ch, PULSE_VIOLENCE*2); // mount
			}
		}
		else {
			send_to_char("You can't ride that!\n\r", ch);
			return;
		}
	}
	else if(cmd == CMD_DISMOUNT) {
		horse = MOUNTED(ch);

		act("You dismount from $N", FALSE, ch, 0, horse, TO_CHAR);
		act("$n dismounts from $N", FALSE, ch, 0, horse, TO_NOTVICT);
		act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
		Dismount(ch, MOUNTED(ch), POSITION_STANDING);
		return;
	}

}

int CanMemorize(struct char_data* ch) {
	int nMax=0;
	nMax = int_app[(int)GET_INT(ch) ].memorize +
		   GET_LEVEL(ch, SORCERER_LEVEL_IND) / 8;
	nMax = (int)(nMax / (1.0 + (HowManyClasses(ch) - 1) / 2.0));
	return(nMax);
}

int CheckContempMemorize(struct char_data* pChar) {
	struct affected_type* pAf;
	int nCount = 0;

	for(pAf = pChar->affected; pAf; pAf = pAf->next)
		if(pAf->type == SKILL_MEMORIZE) {
			nCount+=(1+pAf->bitvector);
		}

	if(nCount && nCount >= CanMemorize(pChar)) {
		return FALSE;
	}
	else {
		return TRUE;
	}

}

ACTION_FUNC(do_memorize) {

	int spl,qend;
	short int duration;
	struct affected_type af;
	const char* argument=arg;

	if(!IS_PC(ch)) {
		return;
	}

	if(!IsHumanoid(ch)) {
		send_to_char("Mi dispiace ma non hai la forma giusta.\n\r",ch);
		return;
	}

	if(!IS_IMMORTAL(ch)) {
		if(BestMagicClass(ch) == WARRIOR_LEVEL_IND ||
				BestMagicClass(ch) == BARBARIAN_LEVEL_IND) {
			send_to_char("Credo che sia meglio che tu combatta...\n\r", ch);
			return;
		}
		else if(BestMagicClass(ch) == THIEF_LEVEL_IND) {
			send_to_char("Credo che sia meglio che tu vada a rubare...\n\r", ch);
			return;
		}
		else if(BestMagicClass(ch) == MONK_LEVEL_IND) {
			send_to_char("Credo che sia meglio che tu vada a meditare...\n\r", ch);
			return;
		}
		else if(BestMagicClass(ch) == MAGE_LEVEL_IND) {
			send_to_char("Questo non e' il tuo modo di lanciare incantesimi...\n\r",
						 ch);
			return;
		}
		else if(BestMagicClass(ch) ==DRUID_LEVEL_IND) {
			send_to_char("Questo non e' il tuo modo di lanciare incantesimi...\n\r",
						 ch);
			return;
		}
		else if(BestMagicClass(ch) ==CLERIC_LEVEL_IND) {
			send_to_char("Questo non e' il tuo modo di lanciare incantesimi...\n\r",
						 ch);
			return;
		}
	}


	argument = skip_spaces(argument);

	if(!(CheckContempMemorize(ch)) && *argument) {
		act("Non riesci a imparare tutti questi incatesimi contemporaneamente.",
			FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	/* If there is no chars in argument */
	if(!(*argument)) {
		char buf[ MAX_STRING_LENGTH * 2 ], temp[ 20 ];
		int i;
		struct string_block sb;

		sprintf(buf, "Puoi memorizzare lo stesso incantesimo fino a $c0011%d"
				"$c0007 volt%c.\n\r"
				"In tutto puoi memorizzare $c0011%d$c0007 incantesimi ("
				"$c0011%d$c0007 per volta).\n\r",
				MaxCanMemorize(ch, 0),
				MaxCanMemorize(ch, 0) == 1 ? 'a' : 'e',
				TotalMaxCanMem(ch),
				CanMemorize(ch));
		send_to_char(buf, ch);
		sprintf(buf, "Attualmente hai $c0011%d$c0007 incantesim%c "
				"memorizzat%c.\n\r\n\r",
				TotalMemorized(ch),
				TotalMemorized(ch) == 1 ? 'o' : 'i',
				TotalMemorized(ch) == 1 ? 'o' : 'i');
		send_to_char(buf, ch);
		send_to_char("Il tuo libro contiene i seguenti incantesimi:\n\r", ch);

		init_string_block(&sb);

		for(i = 0; *spells[ i ] != '\n'; i++) {
			if(spell_info[ i + 1 ].spell_pointer &&
					spell_info[ i + 1 ].min_level_sorcerer <=
					GET_LEVEL(ch, SORCERER_LEVEL_IND) &&
					IS_SET(ch->skills[ i + 1 ].flags, SKILL_KNOWN) &&
					IS_SET(ch->skills[ i + 1 ].flags, SKILL_KNOWN_SORCERER)
					&& MEMORIZED(ch,i+1)) {
				sprintf(buf, "[%3d] %27s %14s",
						spell_info[ i + 1 ].min_level_sorcerer,
						spells[ i ], how_good(ch->skills[ i + 1 ].learned));
				if(MEMORIZED(ch, i + 1)) {
					sprintf(temp, " x%d", ch->skills[ i + 1 ].nummem);
					strcat(buf,temp);
				}
				if(IsSpecialized(ch->skills[ i + 1 ].special)) {
					strcat(buf," (special)");
				}
				strcat(buf, " \n\r");

				append_to_string_block(&sb, buf);
			}
		}
		append_to_string_block(&sb, "\n\r");
		page_string_block(&sb, ch);
		destroy_string_block(&sb);

		return;
	}

	if(GET_POS(ch) > POSITION_SITTING) {
		do_rest(ch,"rest",CMD_REST);

		if(affected_by_spell(ch, SKILL_MEMORIZE)) {
			SpellWearOff(SKILL_MEMORIZE, ch);
			affect_from_char(ch, SKILL_MEMORIZE);
		}
	}

	if(*argument != '\'') {
		send_to_char("Gli incantesimi vanno circondati dal simbolo sacro: '\n\r",
					 ch);
		return;
	}

	for(qend = 1; *(argument + qend) && (*(argument + qend) != '\'') ; qend++);
	//{ *(argument + qend) = LOWER( *(argument + qend) ); }

	if(*(argument + qend) != '\'') {
		send_to_char("Gli incantesimi vanno circondati dal simbolo sacro: '\n\r",
					 ch);
		return;
	}

	spl = old_search_block(argument, 1, qend-1, spells, 0);

	if(!spl || !ch->skills[spl].learned) {  // SALVO metto il controllo sugli allenamenti
		send_to_char("Sfogli il tuo libro ma non trovi questo incantesimo.\n\r",
					 ch);
		return;
	}

	if(!ch->skills) {
		return;
	}

	if(spl > 0 && spl < MAX_SKILLS && spell_info[ spl ].spell_pointer) {
		if(!IS_IMMORTAL(ch)) {
			if(spell_info[ spl ].min_level_sorcerer >
					GET_LEVEL(ch, SORCERER_LEVEL_IND)) {
                act("Non sei cosi' brav$b da poter usare questo incantesimo!", FALSE, ch, 0, 0, TO_CHAR);
				return;
			}
		}

		/* Non-Sorcerer spell, cleric/druid or something else */
		if(spell_info[ spl ].min_level_sorcerer == 0 || !IS_SET(ch->skills[spl].flags, SKILL_KNOWN_SORCERER)) {    // SALVO altro controllo sulla conoscenza
			send_to_char("Non hai le giuste abilita' per usare questo "
						 "incantesimo.\n\r", ch);
			return;
		}

		/* made it, lets memorize the spell! */

		if(ch->skills[ spl ].nummem < 0) {  /* should not happen */
			ch->skills[ spl ].nummem = 0;
		}

		if(spell_info[ spl ].min_level_magic <= 40) {
			duration = 0;    /* Un'ora virtuale */
		}
		else if(spell_info[ spl ].min_level_magic <= 45) {
			duration = 1;    /* Due ore virtuali */
		}
		else {
			duration = 2;    /* Tre ore virtuali */
		}

		if(duration == 2) { // SALVO riduco le ore di memo per tutti max 2 virtuali
			duration =1;
		}

		if(!affected_by_spell(ch, SKILL_MEMORIZE))
			act("$n sfoglia il suo libro ed inizia a leggere e meditare.",
				TRUE, ch, 0, 0, TO_ROOM);

		af.type = SKILL_MEMORIZE;
		af.duration = 0;
		af.modifier = spl;                 /**/
		af.location = APPLY_SKIP;
		af.bitvector = duration;
		affect_to_char(ch, &af);

		send_to_char("Sfogli il tuo libro ed inizi a leggere e meditare.\n\r",
					 ch);
	}

	return;
} /* end memorize */

int TotalMaxCanMem(struct char_data* ch) {
	int i;

	if(OnlyClass(ch, CLASS_SORCERER)) {
		i = GET_LEVEL(ch, SORCERER_LEVEL_IND);
	}
	else        /* Multis get less spells */
		i = (int)(GET_LEVEL(ch, SORCERER_LEVEL_IND) /
				  HowManyClasses(ch) * 0.5);

	i += (int)int_app[(int)GET_INT(ch) ].learn / 2;
	return(i);
}

/* total amount of spells memorized */
int TotalMemorized(struct char_data* ch) {
	int i, ii = 0;
	for(i = 0; i < MAX_SKILLS; i++) {
		if(ch->skills[ i ].nummem &&
				IS_SET(ch->skills[ i ].flags, SKILL_KNOWN_SORCERER)) {
			ii += ch->skills[ i ].nummem;
		}
	}

	return(ii);
}

void check_memorize(struct char_data* ch, struct affected_type* af) {
	if(af->type == SKILL_MEMORIZE) {
		if(ch->skills[ af->modifier ].nummem >= MaxCanMemorize(ch, af->modifier)) {
			send_to_char("Non puoi memorizzare ancora questo incantesimo.\n\r",
						 ch);
			return;
		}

		if(TotalMemorized(ch) >= TotalMaxCanMem(ch)) {
			send_to_char("La tua mente non riesce a memorizzare altri "
						 "incantesimi!\n\r", ch);
			return;
		}
		if(GET_POS_PREV(ch) != POSITION_RESTING &&
				GET_TEMPO_IN(ch,POSITION_RESTING)<(PULSE_TICK/3)) { // SALVO riduco il tempo di timeout del memo
			send_to_char("Non sei stato seduto nemmeno il tempo di aprire il libro...\n\r", ch);
			return;
		}
		ch->skills[ af->modifier ].nummem += 1;
	}
}

ACTION_FUNC(do_set_afk) {
	if(!ch) {
		return;
	}
	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		return;
	}

	act("$c0006$n quietly goes Away From Keyboard.", TRUE, ch, 0, 0, TO_ROOM);
	act("$c0006You quietly go AFK.", TRUE, ch, 0, 0, TO_CHAR);
	SET_BIT(ch->specials.affected_by2, AFF2_AFK);
	return;
}

#define RACE_WAR_MIN_LEVEL 31
/* this is the level a user can turn race war ON */
ACTION_FUNC(do_set_flags) {
	char type[255],field[255];
	if(!ch) {
		return;
	}

	arg = one_argument(arg,type);

	if(!*type) {
		send_to_char("Actually supported:\n\r"
					 "Ansi       \n\r"
					 "Autoexits \n\r"
					 "Color     \n\r"
					 "Display      \n\r"
					 "Email\n\r"
					 "Realname \n\r"
					 "Pkill \n\r"
					 "Who \n\r"
					 "Pwp \n\r"
					 ,ch);
		return;
	}

	arg = OneArgumentNoFill(arg,field);

	if(!strcmp("pwp",type) && (!*field)) {
		send_to_char("Usa 'set pwp on/off'\n\r"
					 "Questo comando ti permette di vedere "
					 "l'esatto ammontare dei danni arrecati "
                     "e ricevuti. Vale solo per i colpi "
                     "inferti personalmente, e gli altri non "
                     "ne visualizzeranno il valore esatto.\n\r", ch);
		return;
	}

	if(!strcmp(type,"pwp")) {
		if(!strcmp("on",field) || !strcmp("enable",field)) {
			SET_BIT(ch->player.user_flags,PWP_MODE);
			send_to_char("Adesso i danni verranno affiancati dal valore esatto.\n\r",ch);
			return;
		}
		else if(!strcmp("off",field) || !strcmp("disable",field)) {
			if(IS_SET(ch->player.user_flags,PWP_MODE)) {
				REMOVE_BIT(ch->player.user_flags,PWP_MODE);
			}
			send_to_char("Adesso il valore esatto di ciascun attacco non verra' mostrato.\n\r",ch);
			return;
		}
		else {
			send_to_char("Uso: set pwp on(enable)/off(disable) \n\r",ch);
			return;
		}
	}

    if(!strcmp("who",type) && (!*field)) {
		send_to_char("Usa 'set who showclasses/hideclasses'\n\r"
					 "Questo comando ti permette di mostrare o nascondere "
					 "le tue classi agli altri giocatori \n\r", ch);
		send_to_char("Se hideclasses e' attivo al momento del gain il title non viene modificato. \n\r",
					 ch);
		return;
	}

	if(!strcmp(type,"who")) {
		if(!strcmp("showclasses",field) || !strcmp("show",field)) {
			SET_BIT(ch->player.user_flags,SHOW_CLASSES);
			send_to_char("Adesso tutti possono leggere le tue classi nel WHO!\n\r",ch);
			return;
		}
		else if(!strcmp("hideclasses",field) || !strcmp("hide",field)) {
			if(IS_SET(ch->player.user_flags,SHOW_CLASSES)) {
				REMOVE_BIT(ch->player.user_flags,SHOW_CLASSES);
			}
			send_to_char("Adesso le tue classi non appaiono nel WHO!\n\r",ch);
			return;
		}
		else {
			send_to_char("Uso: set who show(classes)/hide(classes) \n\r",ch);
			return;
		}
	}

	if(!strcmp("pkill",type) && (!*field)) {
		send_to_char("Usa 'set pkill enable'\n\r"
					 "RICORDA, UNA VOLTA CHE HAI ABILITATO IL PLAYERS KILLING "
					 "NON PUOI PIU' TORNARE INDIETRO!\n\r", ch);
		send_to_char("Assicurati di aver letto l'help sul PLAYERS KILLING.\n\r",
					 ch);
		return;
	}

	if(!*field) {
		send_to_char("Set it to what? (Enable/On,Disable/Off)\n\r",ch);
		return;
	}

	if(!strcmp(type,"pkill") && GetMaxLevel(ch)>=RACE_WAR_MIN_LEVEL) {
		if(!strcmp("enable",field) || !strcmp("on",field)) {
#if 0
			send_to_char("Il PLAYERS KILLING puo' essere attivato solo dagli Dei "
						 "superiori, per il momento.\n\r", ch);
#else
			SET_BIT(ch->player.user_flags,RACE_WAR);
			send_to_char("PUOI ESSERE ATTACCATO DAGLI ALTRI GIOCATORI!\n\r",ch);
#endif
			return;
		}
		else {
			send_to_char("Leggi l'help sul PLAYERS KILLING.\n\r",ch);
		}
		return;
	}

	if(!strcmp(type,"ansi")) {
		/* turn ansi stuff ON/OFF */
		if(strstr(field, "enable") || !strcmp("on",field)) {
			send_to_char("Setting ansi colors enabled.\n\r",ch);
			SET_BIT(ch->player.user_flags,USE_ANSI);
		}
		else {
			act("Setting ansi colors off.",FALSE,ch,0,0,TO_CHAR);
			if(IS_SET(ch->player.user_flags,USE_ANSI)) {
				REMOVE_BIT(ch->player.user_flags,USE_ANSI);
			}
		}
	} /* was ansi */
	else if(!strcmp(type, "color")) {
		/* set current screen color */
		char buf[128];
		sprintf(buf,"%sChanging screen colors!",ansi_parse(field));
		act(buf,FALSE,ch,0,0,TO_CHAR);
	} /* was color*/
	else if(!strcmp(type,"pause")) {
		/* turn page mode ON/OFF */
		if(strstr(field,"enable") || !strcmp("on",field)) {
			send_to_char("Setting page pause mode enabled.\n\r",ch);
			SET_BIT(ch->player.user_flags,USE_PAGING);
		}
		else {
			act("Turning page pause off.",FALSE,ch,0,0,TO_CHAR);
			if(IS_SET(ch->player.user_flags,USE_PAGING)) {
				REMOVE_BIT(ch->player.user_flags,USE_PAGING);
			}
		}
	}
	else if(!strcmp(type,"group")) {
		if(!strcmp(field,"name")) {
			if(arg) {
				do_group_name(ch,arg,0);
			}
		}
		else if(!strcmp(field,"order")) {
			if(IS_SET(ch->specials.affected_by2,AFF2_CON_ORDER)) {
				act("Non accetti piu' ordini da nessuno!!.",FALSE,ch,0,0,TO_CHAR);
				act("$n non accetta piu' ordini dal boss!",FALSE,ch,0,0,TO_ROOM);
				REMOVE_BIT(ch->specials.affected_by2,AFF2_CON_ORDER);
			}
			else if(!ch->master) {
				act("You already can accept orders from YOURSELF",FALSE,ch,0,0,TO_CHAR);
			}
			else {
				act("Mi spiace, ma non mi sembra divertente stare seduto davanti al "
					"video mentre un altro gioca per te!",FALSE,ch,0,0,TO_CHAR);
				act("$N vorrebbe far giocare il boss al suo posto...."
					"ma non e' possibile!", FALSE,ch->master,0,ch,TO_CHAR);
			}
		}  /* end order */
		else {
			send_to_char("Unknown set group command\n",ch);
		}
	} /* end was a group command */
	else if(!strcmp(type,"autoexits")) {
		if(strstr(field,"enable") || !strcmp("on",field)) {
			act("Setting autodisplay exits on.",FALSE,ch,0,0,TO_CHAR);
			if(!IS_SET(ch->player.user_flags,SHOW_EXITS)) {
				SET_BIT(ch->player.user_flags,SHOW_EXITS);
			}
		}
		else {
			act("Setting autodisplay exits off.",FALSE,ch,0,0,TO_CHAR);
			if(IS_SET(ch->player.user_flags,SHOW_EXITS)) {
				REMOVE_BIT(ch->player.user_flags,SHOW_EXITS);
			}
		}
	}
	else if(!strcmp(type,"email")) {
		if(*field) {
			/* set email to field */
			if(ch->specials.email) {
				free(ch->specials.email);
			}
			ch->specials.email = strdup(field);
			send_to_char("Email address set.\n\r",ch);
		}
		else {
			if(ch->specials.email) {
				free(ch->specials.email);
				ch->specials.email = NULL;
				send_to_char("Email address disabled.\n\r",ch);
			}
		}
	}
	else if(!strcmp(type,"realname")) {
		if(*field) {
			/* set realname to field */
			if(ch->specials.realname) {
				free(ch->specials.realname);
			}
			ch->specials.realname = strdup(field);
			send_to_char("Real name set.\n\r",ch);
		}
		else {
			if(ch->specials.realname) {
				free(ch->specials.realname);
				ch->specials.realname = NULL;
				send_to_char("Realname deleted.\n\r",ch);
			}
		}
	}
	else { /* end real name */
		return;
	}
}

ACTION_FUNC(do_whois) {
	char name[128],buf[254];
	struct char_data* finger;

	arg= one_argument(arg,name);

	if(!*name) {
		send_to_char("Finger whom?!?!\n\r",ch);
		return;
	}

	if(!(finger=get_char(name))) {
		send_to_char("No person by that name\n\r",ch);
	}
	else {
		if(CAN_SEE(ch,finger)&& IS_PC(finger)) {

			act("$c0001$N's finger stats:\n\r",FALSE,ch,0,finger,TO_CHAR);
			sprintf(buf,"$c0005Email       : $c0011%s\n\r"
					"$c0005In real life: $c0011%s$c0007",
					finger->specials.email,finger->specials.realname);
			act(buf,FALSE,ch,0,0,TO_CHAR);
		} /* end cansee */
		else {
			send_to_char("No person by that name\n\r",ch);
		}
	} /* end found finger'e */

}

void do_ego_eq_action(struct char_data* ch) {
	struct obj_data* obj ;

	for(int j = 0; j < MAX_WEAR; j++) {
		if(ch->equipment[j] && ch->equipment[j]->item_number>=0) {
			obj = ch->equipment[j];
			parse_eq_action(ch, obj);
		}
	}
}
void parse_eq_action(struct char_data* ch, struct obj_data* obj) {
	if(obj->contains) {
		parse_eq_action(ch, obj->contains);
	}
	if(obj->next_content) {
		parse_eq_action(ch, obj->next_content);
	}
	ck_eq_action(ch, obj);
}

void ck_eq_action(struct char_data* ch, struct obj_data* obj) {
	int keyword=0;
	int gone=0;
	int j = 0 ;
	struct char_data* tmp_ch ;
	struct obj_data* tmp_obj ;
	struct room_data* ch_room ;

	ch_room = real_roomp(ch->in_room);

	if(IS_RARE(obj) && !EgoSave(ch)) {

		if(obj->carried_by) {
			/* Se l'align non e' quello giusto l'oggetto se ne va */
			if(ItemAlignClash(ch, obj)) {
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
				check_falling_obj(obj, ch->in_room);
				gone = 1;
			}
			/* Altrimenti cerca di essere indossato */
			else if(number(0,1)) {
				keyword = -2 ;
				if(CAN_WEAR(obj,ITEM_HOLD)) {
					j = 0 ;
					keyword = 13;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_SHIELD)) {
					j = 11 ;
					keyword = 14;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_FINGER)) {
					j = 1;
					keyword = 1;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_NECK)) {
					j = 3 ;
					keyword = 2;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_WRIST)) {
					j = 14 ;
					keyword = 11;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_WAISTE)) {
					j = 13 ;
					keyword = 10;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_ARMS)) {
					j = 10 ;
					keyword = 8;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_HANDS)) {
					j =  9 ;
					keyword = 7;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_FEET)) {
					j =  8 ;
					keyword = 6;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_LEGS)) {
					j =  7 ;
					keyword = 5;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_ABOUT)) {
					j = 12 ;
					keyword = 9;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_HEAD)) {
					j =  6 ;
					keyword = 4;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_BODY)) {
					j =  5 ;
					keyword = 3;
				}
				if(CAN_WEAR(obj,ITEM_WIELD)) {
					j = 16 ;
					keyword = 12;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_BACK) &&
						obj->obj_flags.type_flag==ITEM_CONTAINER) {
					j = 18 ;
					keyword=15;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_EYE)) {
					j = 21 ;
					keyword = 17;
				}
				if(CAN_WEAR(obj,ITEM_WEAR_EAR)) {
					j = 19 ;
					keyword = 16;
				}

				if(keyword != -2) {
					if(ch->equipment[j]) {
						tmp_obj = unequip_char(ch,j) ;
						if(tmp_obj) {
							obj_to_char(tmp_obj, ch);
						}
						if(number(0,1)) {
							act("Smetti di usare $p.",FALSE,ch,tmp_obj,0,TO_CHAR);
							act("$n smette di usare $p.",TRUE,ch,tmp_obj,0,TO_ROOM);
						}
						else if(number(0,2)) {
							act("Smetti di usare $p e lo getti in terra.",FALSE,ch,tmp_obj,0,TO_CHAR);
							act("$n smette di usare $p e lo getta in terra.",TRUE,ch,tmp_obj,0,TO_ROOM);
							obj_from_char(tmp_obj);
							obj_to_room(tmp_obj, ch->in_room);
							check_falling_obj(tmp_obj, ch->in_room);
						}
						else {
							act("Smetti di usare $p e lo getti via.",FALSE,ch,tmp_obj,0,TO_CHAR);
							act("$n smette di usare $p e lo getta via.",TRUE,ch,tmp_obj,0,TO_ROOM);
							obj_from_char(tmp_obj);
							extract_obj(tmp_obj);
						}
					}
					wear(ch, obj, keyword);
				}
			}
			/* Altrimenti guarda se c'e' qualcuno con un carisma decente in giro */
			else {
				mudlog(LOG_CHECK, "%s is looking for a better guy",
					   obj->name);
				for(tmp_ch = ch_room->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
					if(GET_CHR(tmp_ch) > GET_CHR(ch) &&
							GetMaxLevel(tmp_ch) >= GetMaxLevel(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, tmp_ch);
						mudlog(LOG_CHECK, "%s liked very much %s",
							   obj->name, GET_NAME(tmp_ch));
						gone = 1 ;
					}
				}
				/* Altrimenti se ne va sconsolato... */
				if(!gone && !number(0,2)) {
					obj_from_char(obj);
					obj_to_room(obj, ch->in_room);
					check_falling_obj(obj, ch->in_room);
					mudlog(LOG_CHECK, "%s has gone..",
						   obj->name);
				}
			}
		}

		else if(obj->in_obj) {
			if(!number(0,2)) {
				tmp_obj = obj->in_obj ;
				act("$p improvvisamente si lacera!",TRUE,ch,tmp_obj,0,TO_CHAR);
				MakeScrap(ch, NULL, tmp_obj) ;
			}
		}

		else if(obj->equipped_by) {
			if(GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
				if(GET_POS(ch) == POSITION_SLEEPING) {
					act("Una botta in testa ti sveglia all'improvviso!",FALSE,ch,obj,0,TO_CHAR);
					act("$p improvvisamente sobbalza e colpisce in testa $n.",TRUE,ch,obj,0,TO_ROOM);
					GET_POS(ch) = POSITION_SITTING;
				}
				else if(GET_POS(ch) == POSITION_SITTING || GET_POS(ch) == POSITION_RESTING) {
					act("$p ti punge con impazienza il sedere e ti costringe ad alzarti.",FALSE,ch,obj,0,TO_CHAR);
					act("$p colpisce ripetutamente $n per farlo alzare.",TRUE,ch,obj,0,TO_ROOM);
					GET_POS(ch) = POSITION_STANDING;
				}
				else if(GET_POS(ch) == POSITION_STANDING) {
					for(tmp_ch = ch_room->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
						if(ItemAlignClash(tmp_ch, obj)) {
							act("$p si muove nelle tue mani senza controllo!!",FALSE,ch,obj,0,TO_CHAR);
							act("$p salta alla gola di $N.",TRUE,ch,obj,tmp_ch,TO_ROOM);
							do_hit(ch,tmp_ch->player.name, 0);
						}
					}
				}
				else if(number(0,1) && ch->specials.fighting
						&& ItemAlignClash(ch->specials.fighting, obj)) {
					if(!IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
						mudlog(LOG_CHECK, "trying to berserk because of item ");
						act("BANZAI!!! Morte agli infedeli!!!",FALSE,ch,obj,0,TO_CHAR);
						act("$n sembra preso da una incontrollabile furia omicida!",TRUE,ch,obj,NULL,TO_ROOM);
						SET_BIT(ch->specials.affected_by2,AFF2_BERSERK);
					}
				}
			}
			else if(!number(0,4)) {
				obj_to_char(unequip_char(ch, obj->eq_pos), ch);
			}
		}
		else {
			mudlog(LOG_SYSERR, "Object %s carried by %s but not equip or carried or in bag"
				   , obj->name, GET_NAME(ch));
			act("Dove diavolo e' $p? (Segnala la cosa ad un coder per piacere :-))", FALSE, ch, obj, NULL, TO_CHAR);
		}

	}
}

ACTION_FUNC(do_insert)
{
    int i, l, affect = 0, aff = 0, loc = 0, mod = 0, val = 0, zir = 0, hitroll = 0, damroll = 0, castoni = 0;
    int gems[5] = { 0, 0, 0, 0, 0 }, val_orig, vnum, wait = 0, colore[5] = { 0, 0, 0, 0, 0 };
    bool ok = FALSE;
    char tmp[100], gemma[100], buf[MAX_STRING_LENGTH], color1[50], color2[50];
    struct obj_data* obj, *gem, *gem_tmp;

    const char* rand_reaction[] = {
        "Studi meticolosamente $p, poi sorridi tra te e te.",
        "Guardi entusiasta $p pensando 'Ma quanto sono brav$b!'",
        "Esclami: '$c0009SI PUO' FARE!$c0007'",
        "Sorridi compiaciut$b.",
        "Pensi: 'Potevo fare di meglio, ma comunque va MOLTO bene :-)'",
        "Guardi con adorazione $p poi, a voce alta, esclami: '$c0009Il mio tesssssoro!$c0007'",
        "Ti sfreghi le mani con soddisfazione.",
        "Osservi sognante $p, hai fatto un ottimo lavoro!",
        "Molto bene, la gemma e' incastonata perfettamente.",
        "Pensi tra te e te: 'E anche questa e' fatta!'",
        "$n studia meticolosamente $p, poi sorride tra se e se.",
        "$n guarda entusiasta $p pensando 'Ma quanto sono brav$b!'",
        "$n esclama: '$c0009SI PUO' FARE!$c0007'",
        "$n sorride compiaciut$b.",
        "$n pensa a voce alta: 'Potevo fare di meglio, ma comunque va MOLTO bene :-)'",
        "$n guarda con adorazione $p poi, a voce alta, esclama: '$c0009Il mio tesssssoro!$c0007'",
        "$n si sfrega le mani con soddisfazione.",
        "$n osserva sognante $p.",
        "Un ghigno malefico compare sulle labbra di $n: e' chiaramente soddisfatt$b.",
        "$n pensa ad alta voce: 'E anche questa e' fatta!'"
    };

    const int nRandReac = 9;

    if(IS_NPC(ch))
    {
        send_to_char("Chi ti pensi di essere? Un gioielliere? Sei solo uno stupido mob!\n\r", ch);
        return;
    }

    arg = one_argument(arg, tmp);
    obj = get_obj_in_list_vis(ch, tmp, ch->carrying);

    if(obj)
    {
        for(i = 0; i < MAX_OBJ_AFFECT; i++)
        {
            if((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0) && (obj->affected[i].location != APPLY_SKIP))
                affect++;
        }

        if(IS_OBJ_STAT2(obj, ITEM2_EDIT))
        {
            act("Non puoi farlo, $c0015$p$c0007 e' stato plasmato dagli Dei!", TRUE, ch, obj, 0, TO_CHAR);
            mudlog(LOG_PLAYERS, "%s can't insert gems in %s, it is an Edited Item", GET_NAME(ch), obj->short_description);
            return;
        }
        else if(IS_OBJ_STAT2(obj, ITEM2_INSERT))
        {
            act("Non vedi che alcune gemme gia' adornano $c0015$p$c0007!", TRUE, ch, obj, 0, TO_CHAR);
            mudlog(LOG_PLAYERS, "%s can't insert gems in %s, it was already mounted", GET_NAME(ch), obj->short_description);
            return;
        }
        else if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN)
        {
            send_to_char("Non puoi incastonare oggetti RARI!\n\r", ch);
            mudlog(LOG_PLAYERS, "%s can't insert gems in %s, it is a Rare Item", GET_NAME(ch), obj->short_description);
            return;
        }
        else if((vnum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0) == TAN_BAG || vnum == TAN_SHIELD || vnum == TAN_JACKET || vnum == TAN_BOOTS || vnum == TAN_GLOVES || vnum == TAN_LEGGINGS || vnum == TAN_SLEEVES || vnum == TAN_HELMET || vnum == TAN_ARMOR)
        {
            send_to_char("Non puoi incastonare armature conciate!\n\r", ch);
            mudlog(LOG_PLAYERS, "%s can't insert gems in %s, it is a Tanned Item", GET_NAME(ch), obj->short_description);
            return;
        }

        if(affect == 5)
        {
            sprintf(buf, "Non puoi farlo, $c0015%s$c0007 va bene cosi'!\n\r", obj->short_description);
            send_to_char(buf, ch);
            mudlog(LOG_PLAYERS, "%s try to insert gems in %s but there are already 5 affects", GET_NAME(ch), obj->short_description);
            return;
        }

        switch(GET_ITEM_TYPE(obj))
        {
            case ITEM_LIGHT :
            case ITEM_WAND :
            case ITEM_STAFF :
            case ITEM_WEAPON :
            case ITEM_FIREWEAPON :
            case ITEM_OTHER :
            case ITEM_AUDIO :
            case ITEM_ARMOR :
            case ITEM_CONTAINER :
                ok = TRUE;
                break;
            case ITEM_SCROLL :
                sprintf(buf, "Non puoi incastonare pergamene!\n\r");
                ok = FALSE;
                break;
            case ITEM_POTION :
                sprintf(buf, "Non puoi incastonare pozioni!\n\r");
                ok = FALSE;
                break;
            case ITEM_WORN :
                sprintf(buf, "Quest'oggetto e' troppo logorato per essere incastonato!\n\r");
                ok = FALSE;
                break;
            case ITEM_TRASH :
                sprintf(buf, "Spazzatura, e' solo un mucchio di spazzatura non lo vedi?\n\r");
                ok = FALSE;
                break;
            case ITEM_TRAP :
                sprintf(buf, "Non vedo come tu possa incastonare una trappola!\n\r");
                ok = FALSE;
                break;
            case ITEM_NOTE :
                sprintf(buf, "Questo tipo di oggetti e' fatto per scriverci sopra!\n\r");
                ok = FALSE;
                break;
            case ITEM_FOOD :
                sprintf(buf, "Proprio quello che ci voleva, un panino al diamante!\n\r");
                ok = FALSE;
                break;
            default:
                sprintf(buf, "Non puoi incastonare questo tipo di oggetto!\n\r");
                ok = FALSE;
        }

        if(!ok)
        {
            send_to_char(buf, ch);
            mudlog(LOG_PLAYERS, "%s try to insert gems in %s, it is not a valid type of object", GET_NAME(ch), obj->short_description);
            return;
        }
        else
        {
            val_orig = obj->obj_flags.cost;
            send_to_char("Sistemi gli attrezzi di lavoro sul tuo banco di legno e li controlli con cura: scalpelli, uncini, pinze, lime.\n\r", ch);
            send_to_char("Valuti con cura quali siano i migliori per iniziare, prendi fiato ed inizi a lavorare.\n\r\n\r", ch);
            act("Inizi ad armeggiare con $c0015$p$c0007.\n\r",TRUE, ch, obj, 0, TO_CHAR);
            act("$n tira fuori una serie di utensili da lavoro, controlla sapientemente $c0015$p$c0007 poi,\n\rcon mano ferma, si mette all'opera.\n\r", TRUE, ch, obj, 0, TO_ROOM);
        }

        castoni = 5 - affect;

        for(i = 0; i < castoni; i++)
        {
            arg = one_argument(arg, gemma);
            if(*gemma)
            {
                gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                if(gem)
                {
                    gems[i] = (gem->item_number >= 0) ? obj_index[gem->item_number].iVNum : 0;

                    // controllo se il vnum è una gemma incastonabile
                    if(gems[i] < 19509 || gems[i] > 19537)
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, %s it is not a gem", GET_NAME(ch), obj->short_description, gem->short_description);
                        sprintf(buf, "Mi dispiace ma $c0015%s$c0007 non e' una gemma!\n\r", gem->short_description);
                        send_to_char(buf, ch);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*4;
                            }
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19511 || gems[l] == 19510)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if((gems[l] == 19513 && zir == 3) || gems[l] == 19524)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }

                    // controllo se si può incastonare il tipo di pietro nell'arma
                    if (GET_ITEM_TYPE(obj) == ITEM_WEAPON && (gems[i] == 19511 || gems[i] == 19512 || gems[i] == 19513 || gems[i] == 19514 || gems[i] == 19515 || gems[i] == 19516 || gems[i] == 19517 || gems[i] == 19518 || gems[i] == 19519 || gems[i] == 19520 || gems[i] == 19521 || gems[i] == 19524 || gems[i] == 19526 || gems[i] == 19527 || gems[i] == 19532))
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert %s in %s, it is a weapon", GET_NAME(ch), gem->short_description, obj->short_description);
                        sprintf(buf, "Mi dispiace, non sei riuscit$b ad inserire $c0015%s$c0007 su $c0015$p$c0007.\n\r", gem->short_description);
                        act(buf, TRUE, ch, obj, 0, TO_CHAR);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*4;
                            }
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19511 || gems[l] == 19510)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if((gems[l] == 19513 && zir == 3) || gems[l] == 19524)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }

                    if(!IS_DIO_MINORE(ch))
                    {
                        /* Se il livello è minore del 53 assegno il wait state in proporzione al numero di gemme */
                        wait += PULSE_VIOLENCE + PULSE_VIOLENCE*i;
                    }
                    sprintf(buf, "Incastoni $c0015%s$c0007 su $c0015%s$c0007.\n\r", gem->short_description, obj->short_description);
                    send_to_char(buf, ch);
                    act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                    sprintf(buf, "$n incastona $c0015%s$c0007 su $c0015$p$c0007.", gem->short_description);
                    act(buf, TRUE, ch, obj, 0, TO_ROOM);
                    act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                    if(gems[i] == 19513)
                        zir = 1;
                    obj_from_char(gem);
                    extract_obj(gem);
                }
                else
                {
                    mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                    sprintf(buf, "Non hai niente che si chiami '$c0009%s$c0007' con te.\n\r", gemma);
                    send_to_char(buf, ch);
                    if(i > 0)
                    {
                        sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE*4;
                        }
                        for(l = 0; l < i; l++)
                        {
                            gem_tmp = read_object(real_object(gems[l]), REAL);
                            obj_to_char(gem_tmp, ch);
                            if(gems[l] == 19511 || gems[l] == 19510)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                            }
                            else if((gems[l] == 19513 && zir == 3) || gems[l] == 19524)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                            }
                        }
                    }
                    return;
                }

                if(gems[i] == 19511)        // opale
                {
                    gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                    if(gem)
                    {
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE;
                        }
                        sprintf(buf, "Incastoni la seconda $c0015pietra d'opale$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                        act("$n incastona una seconda $c0015pietra d'opale$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                        obj_from_char(gem);
                        extract_obj(gem);
                    }
                    else
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                        send_to_char("Ti rendi conto solo ora che non hai abbastanza $c0015pietre d'opale$c0007.\n\r", ch);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*4;
                            }
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19510)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if((gems[l] == 19513 && zir == 3) || gems[l] == 19524)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }
                }
                else if(gems[i] == 19510)       // ossidiana
                {
                    gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                    if(gem)
                    {
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE;
                        }
                        sprintf(buf, "Incastoni la seconda $c0015pietra d'ossidiana$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                        act("$n incastona una seconda $c0015pietra d'ossidiana$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                        obj_from_char(gem);
                        extract_obj(gem);
                    }
                    else
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                        send_to_char("Ti rendi conto solo ora che non hai abbastanza $c0015pietre d'ossidiana$c0007.\n\r", ch);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*4;
                            }
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19511)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if((gems[l] == 19513 && zir == 3) || gems[l] == 19524)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }
                }
                else if(gems[i] == 19513)       // zircone
                {
                    gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                    if(gem)
                    {
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE;
                        }
                        sprintf(buf, "Incastoni la seconda $c0015pietra di zircone$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                        act("$n incastona una seconda $c0015pietra di zircone$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                        obj_from_char(gem);
                        extract_obj(gem);
                        zir = 2;
                    }
                    else
                    {
                        continue;
                    }

                    if(zir == 2)
                    {
                        gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                        if(gem)
                        {
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*2;
                            }
                            sprintf(buf, "Incastoni la $c0015terza pietra di zircone$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                            act("$n incastona una terza $c0015pietra di zircone$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                            obj_from_char(gem);
                            extract_obj(gem);
                            zir = 2;
                        }
                        else
                        {
                            mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                            send_to_char("Ti rendi conto solo ora che non hai abbastanza $c0015pietre di zircone$c0007.\n\r", ch);
                            if(i > 0)
                            {
                                sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                                send_to_char(buf, ch);
                                act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                                if(!IS_DIO_MINORE(ch))
                                {
                                    wait += PULSE_VIOLENCE*4;
                                }
                                for(l = 0; l < i; l++)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    if(gems[l] == 19511 || gems[l] == 19510)
                                    {
                                        gem_tmp = read_object(real_object(gems[l]), REAL);
                                        obj_to_char(gem_tmp, ch);
                                    }
                                    else if(gems[l] == 19524)
                                    {
                                        gem_tmp = read_object(real_object(gems[l]), REAL);
                                        obj_to_char(gem_tmp, ch);
                                        gem_tmp = read_object(real_object(gems[l]), REAL);
                                        obj_to_char(gem_tmp, ch);
                                    }
                                }
                            }
                            return;
                        }
                    }
                }
                else if(gems[i] == 19524)       // quarzo rosa
                {
                    gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                    if(gem)
                    {
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE;
                        }
                        sprintf(buf, "Incastoni la seconda $c0015pietra di quarzo rosa$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                        act("$n incastona una seconda $c0015pietra di quarzo rosa$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                        obj_from_char(gem);
                        extract_obj(gem);
                    }
                    else
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                        send_to_char("Ti rendi conto solo ora che che non hai abbastanza $c0015pietre di quarzo rosa$c0007.\n\r", ch);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19511 || gems[l] == 19510)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if(gems[l] == 19513 && zir == 3)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }

                    gem = get_obj_in_list_vis(ch, gemma, ch->carrying);

                    if(gem)
                    {
                        if(!IS_DIO_MINORE(ch))
                        {
                            wait += PULSE_VIOLENCE*2;
                        }
                        sprintf(buf, "Incastoni la terza $c0015pietra di quarzo rosa$c0007 su $c0015%s$c0007.\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        act(rand_reaction[ number(0, nRandReac) ],TRUE, ch, obj, 0, TO_CHAR);
                        act("$n incastona una terza $c0015pietra di quarzo rosa$c0007 su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                        act(rand_reaction[ number(10, (nRandReac+10)) ],TRUE, ch, obj, 0, TO_ROOM);
                        obj_from_char(gem);
                        extract_obj(gem);
                    }
                    else
                    {
                        mudlog(LOG_PLAYERS, "%s fails to insert gems in %s, a gem is missing", GET_NAME(ch), obj->short_description);
                        send_to_char("Ti rendi conto solo ora che non hai abbastanza $c0015pietre di quarzo rosa$c0007.\n\r", ch);
                        if(i > 0)
                        {
                            sprintf(buf, "Rimuovi tutte le gemme incastonate su $c0015%s$c0007.\n\r", obj->short_description);
                            send_to_char(buf, ch);
                            act("$n scuote la testa contrariat$b e rimuove tutte le gemme incastonate su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);
                            if(!IS_DIO_MINORE(ch))
                            {
                                wait += PULSE_VIOLENCE*4;
                            }
                            for(l = 0; l < i; l++)
                            {
                                gem_tmp = read_object(real_object(gems[l]), REAL);
                                obj_to_char(gem_tmp, ch);
                                if(gems[l] == 19511 || gems[l] == 19510 || gems[l] == 19524)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                                else if(gems[l] == 19513 && zir == 3)
                                {
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                    gem_tmp = read_object(real_object(gems[l]), REAL);
                                    obj_to_char(gem_tmp, ch);
                                }
                            }
                        }
                        return;
                    }
                }
            }
            else
            {
                if(i == 0)
                {
                    act("Quale gemma vuoi inserire dentro $p?", FALSE, ch, obj, 0, TO_CHAR);
                    return;
                }
                castoni = castoni - i;
                sprintf(buf, "Finalmente hai terminato il tuo lavoro, ti fermi un attimo \n\re ti rendi conto che potevi incastonare ancora %d gemm%s su $c0015%s$c0007.\n\r", castoni, (castoni > 1 ? "e" : "a"), obj->short_description);
                send_to_char(buf, ch);
                castoni = 5 - castoni;
                break;
            }
        }

        for(i = 0; i < MAX_OBJ_AFFECT; i++)
        {
            while((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0) && (obj->affected[i].location != APPLY_SKIP))
            {
                i++;
            }

            if(aff < castoni)
            {
                switch(gems[aff])
                {
                    case 0:
                        val = 0;
                        loc = 0;
                        mod = 0;
                        colore[aff] = 0;
                        break;
                    case 19509 :        // quarzo comune
                    case 19523 :        // quarzo comune clone
                        val = 1500;
                        loc = APPLY_SPELL;
                        mod = AFF_INFRAVISION;
                        colore[aff] = 15;
                        break;
                    case 19510 :        // ossidiana
                        val = 3000;
                        loc = APPLY_STR;
                        mod = 1;
                        colore[aff] = 8;
                        break;
                    case 19511 :        // opale
                        val = 3000;
                        loc = APPLY_SPELL;
                        mod = AFF_SCRYING;
                        colore[aff] = number(9,15);
                        break;
                    case 19512 :        // turchese
                        val = 1500;
                        loc = APPLY_SPELL;
                        mod = AFF_PROTECT_FROM_EVIL;
                        colore[aff] = 14;
                        break;
                    case 19513 :        // zircone
                        if(zir == 1)
                        {
                            val = 1500;
                            loc = APPLY_NONE;
                            mod = 0;
                            colore[aff] = number(9,15);
                            SET_BIT(obj->obj_flags.extra_flags, ITEM_RESISTANT);
                        }
                        else
                        {
                            val = 4500;
                            loc = APPLY_NONE;
                            mod = 0;
                            colore[aff] = number(9,15);
                            SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
                        }
                        break;
                    case 19514 :        // lapislazzuli
                        val = 1500;
                        loc = APPLY_MANA_REGEN;
                        mod = 5;
                        colore[aff] = 12;
                        break;
                    case 19515 :        // onice
                        val = 1500;
                        loc = APPLY_CHR;
                        mod = 1;
                        colore[aff] = number(1,8);
                        if(colore[aff] < 5)
                            colore[aff] = 1;
                        else
                            colore[aff] = 8;
                        break;
                    case 19516 :        // malachite
                        val = 1500;
                        loc = APPLY_INT;
                        mod = 1;
                        colore[aff] = 2;
                        break;
                    case 19517 :        // ematite
                        val = 1500;
                        loc = APPLY_CON;
                        mod = 1;
                        colore[aff] = 7;
                        break;
                    case 19518 :        // giada
                        val = 1500;
                        loc = APPLY_WIS;
                        mod = 1;
                        colore[aff] = 10;
                        break;
                    case 19519 :        // resina fossile
                        val = 1500;
                        loc = APPLY_SAVE_ALL;
                        mod = -1;
                        colore[aff] = number(3,11);
                        if(colore[aff] < 7)
                            colore[aff] = 3;
                        else
                            colore[aff] = 11;
                        break;
                    case 19520 :        // crisoberillo
                        val = 1500;
                        loc = APPLY_MOVE_REGEN;
                        mod = 5;
                        colore[aff] = 11;
                        break;
                    case 19521 :        // spinello blu
                        val = 1500;
                        loc = APPLY_SPELLFAIL;
                        mod = -2;
                        colore[aff] = 12;
                        break;
                    case 19522 :        // tormalina
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 1500;
                            loc = APPLY_HIT;
                            mod = 2;
                            colore[aff] = number(1,15);
                            if(colore[aff] == 4)
                                colore[aff] = 6;
                        }
                        else
                        {
                            val = 1500;
                            loc = APPLY_HIT_REGEN;
                            mod = 5;
                            colore[aff] = number(1,15);
                            if(colore[aff] == 4)
                                colore[aff] = 6;
                        }
                        break;
                    case 19524 :        // quarzo rosa
                        val = 6750;
                        loc = APPLY_SPELL;
                        mod = AFF_SENSE_LIFE;
                        colore[aff] = 13;
                        break;
                    case 19525 :        // agata
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 2250;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 33;   // poison
                            colore[aff] = number(1,15);
                            if(colore[aff] == 4)
                                colore[aff] = 2;
                        }
                        else
                        {
                            val = 2250;
                            loc = APPLY_M_IMMUNE;
                            mod = IMM_POISON;
                            colore[aff] = number(1,15);
                            if(colore[aff] == 4)
                                colore[aff] = 2;
                        }
                        break;
                    case 19526 :        // acquamarina
                        val = 2250;
                        loc = APPLY_SPELL;
                        mod = AFF_WATERBREATH;
                        colore[aff] = number(6,14);
                        if(colore[aff] < 11)
                            colore[aff] = 6;
                        else
                            colore[aff] = 14;
                        break;
                    case 19527 :        // berillo
                        val = 2250;
                        loc = APPLY_DEX;
                        mod = 1;
                        colore[aff] = number(11,15);
                        break;
                    case 19528 :        // topazio
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 2250;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 37;   // shocking grasp
                            colore[aff] = 11;
                        }
                        else
                        {
                            val = 2250;
                            loc = APPLY_IMMUNE;
                            mod = IMM_ELEC;
                            colore[aff] = 11;
                        }
                        break;
                    case 19529 :        // spinello nero
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 3000;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 38;   // sleep
                            colore[aff] = 8;
                        }
                        else
                        {
                            val = 3000;
                            loc = APPLY_IMMUNE;
                            mod = IMM_HOLD;
                            colore[aff] = 8;
                        }
                        break;
                    case 19530 :        // fluorite
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 3000;
                            loc = APPLY_NONE;
                            mod = 0;
                            colore[aff] = 10;
                            SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
                        }
                        else
                        {
                            val = 3000;
                            loc = APPLY_SPELL;
                            mod = AFF_INVISIBLE;
                            colore[aff] = 10;
                        }
                        break;
                    case 19531 :        // ametista
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 3000;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 32;   // magic missile
                            colore[aff] = number(5,13);
                            if(colore[aff] < 9)
                                colore[aff] = 5;
                            else
                                colore[aff] = 13;
                        }
                        else
                        {
                            val = 3000;
                            loc = APPLY_IMMUNE;
                            mod = IMM_ENERGY;
                            colore[aff] = number(5,13);
                            if(colore[aff] < 9)
                                colore[aff] = 5;
                            else
                                colore[aff] = 13;
                        }
                        break;
                    case 19532 :        // corindone
                        val = 3000;
                        loc = APPLY_SPELL;
                        mod = AFF_TRUE_SIGHT;
                        colore[aff] = number(3,7);
                        if(colore[aff] < 6)
                            colore[aff] = 3;
                        else
                            colore[aff] = 7;
                        break;
                    case 19533 :        // granato
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 3000;
                            loc = APPLY_HITNDAM;
                            mod = 1;
                            colore[aff] = 13;
                        }
                        else
                        {
                            val = 3000;
                            loc = APPLY_AC;
                            mod = -10;
                            colore[aff] = 13;
                        }
                        break;
                    case 19534 :        // zaffiro
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 7500;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 8;    // chill touch
                            colore[aff] = 12;
                        }
                        else
                        {
                            val = 7500;
                            loc = APPLY_IMMUNE;
                            mod = IMM_COLD;
                            colore[aff] = 12;
                        }
                        break;
                    case 19535 :        // smeraldo
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 7500;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 67;   // acid blast
                            colore[aff] = 10;
                        }
                        else
                        {
                            val = 7500;
                            loc = APPLY_IMMUNE;
                            mod = IMM_ACID;
                            colore[aff] = 10;
                        }
                        break;
                    case 19536 :        // rubino
                        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
                        {
                            val = 7500;
                            loc = APPLY_WEAPON_SPELL;
                            mod = 5;    // burning hands
                            colore[aff] = 9;
                        }
                        else
                        {
                            val = 7500;
                            loc = APPLY_IMMUNE;
                            mod = IMM_FIRE;
                            colore[aff] = 9;
                        }
                        break;
                    case 19537 :        // diamante
                        val = 7500;
                        loc = APPLY_NONE;
                        mod = 0;
                        colore[aff] = 15;
                        SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
                        break;

                    default :
                        mudlog(LOG_ERROR, "Do_insert: missing gem");
                        send_to_char("Qualcosa e' andato storto, contatta un immortale...\n\r", ch);
                        return;
                }
            }

            if(val > 0)
            {
                obj->affected[i].location = loc;
                obj->affected[i].modifier = mod;
                obj->obj_flags.cost += val;
                aff += 1;
                val = 0;
                loc = 0;
                mod = 0;
            }

        }

        if(GET_ITEM_TYPE(obj) == ITEM_WEAPON)
        {
            for(i = 0; i < MAX_OBJ_AFFECT; i++)
            {
                if(obj->affected[i].location == APPLY_HITROLL)
                {
                    hitroll += obj->affected[i].modifier;
                    obj->affected[i].location = APPLY_NONE;
                    obj->affected[i].modifier = 0;
                }
                else if(obj->affected[i].location == APPLY_DAMROLL)
                {
                    damroll += obj->affected[i].modifier;
                    obj->affected[i].location = APPLY_NONE;
                    obj->affected[i].modifier = 0;
                }
                else if(obj->affected[i].location == APPLY_HITNDAM)
                {
                    hitroll += obj->affected[i].modifier;
                    damroll += obj->affected[i].modifier;
                    obj->affected[i].location = APPLY_NONE;
                    obj->affected[i].modifier = 0;
                }
            }
        }

        switch(aff)
        {
            case 0:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500)
                {
                    sprintf(color1, "pietra");
                    sprintf(color2, "pietre");
                }
                else
                {
                    sprintf(color1, "gemma");
                    sprintf(color2, "gemme");
                }
                break;

            case 1:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500 || (((obj->obj_flags.cost - val_orig)/aff) > 3000 && ((obj->obj_flags.cost - val_orig)/aff) <= 4500) || (((obj->obj_flags.cost - val_orig)/aff) > 6000 && ((obj->obj_flags.cost - val_orig)/aff) < 7500))
                {
                    sprintf(color1, "$c00%s%dpietra$c0007", (colore[0] > 9 ? "" : "0"), colore[0]);
                    sprintf(color2, "$c00%s%dpietre$c0007", (colore[0] > 9 ? "" : "0"), colore[0]);
                }
                else
                {
                    sprintf(color1, "$c00%s%dgemma$c0007", (colore[0] > 9 ? "" : "0"), colore[0]);
                    sprintf(color2, "$c00%s%dgemme$c0007", (colore[0] > 9 ? "" : "0"), colore[0]);
                }
                break;

            case 2:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500 || (((obj->obj_flags.cost - val_orig)/aff) > 3000 && ((obj->obj_flags.cost - val_orig)/aff) <= 4500) || (((obj->obj_flags.cost - val_orig)/aff) > 6000 && ((obj->obj_flags.cost - val_orig)/aff) < 7500))
                {
                    sprintf(color1, "$c00%s%dpie$c00%s%dtra$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1]);
                    sprintf(color2, "$c00%s%dpie$c00%s%dtre$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1]);
                }
                else
                {
                    sprintf(color1, "$c00%s%dgem$c00%s%dma$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1]);
                    sprintf(color2, "$c00%s%dgem$c00%s%dme$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1]);
                }
                break;

            case 3:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500 || (((obj->obj_flags.cost - val_orig)/aff) > 3000 && ((obj->obj_flags.cost - val_orig)/aff) <= 4500) || (((obj->obj_flags.cost - val_orig)/aff) > 6000 && ((obj->obj_flags.cost - val_orig)/aff) < 7500))
                {
                    sprintf(color1, "$c00%s%dpi$c00%s%det$c00%s%dra$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2]);
                    sprintf(color2, "$c00%s%dpi$c00%s%det$c00%s%dre$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2]);
                }
                else
                {
                    sprintf(color1, "$c00%s%dge$c00%s%dm$c00%s%dma$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2]);
                    sprintf(color2, "$c00%s%dge$c00%s%dm$c00%s%dme$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2]);
                }
                break;

            case 4:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500 || (((obj->obj_flags.cost - val_orig)/aff) > 3000 && ((obj->obj_flags.cost - val_orig)/aff) <= 4500) || (((obj->obj_flags.cost - val_orig)/aff) > 6000 && ((obj->obj_flags.cost - val_orig)/aff) < 7500))
                {
                    sprintf(color1, "$c00%s%dpi$c00%s%de$c00%s%dt$c00%s%dra$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3]);
                    sprintf(color2, "$c00%s%dpi$c00%s%de$c00%s%dt$c00%s%dre$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3]);
                }
                else
                {
                    sprintf(color1, "$c00%s%dg$c00%s%dem$c00%s%dm$c00%s%da$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3]);
                    sprintf(color2, "$c00%s%dg$c00%s%dem$c00%s%dm$c00%s%de$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3]);
                }
                break;

            case 5:
                if(((obj->obj_flags.cost - val_orig)/aff) <= 1500 || (((obj->obj_flags.cost - val_orig)/aff) > 3000 && ((obj->obj_flags.cost - val_orig)/aff) <= 4500) || (((obj->obj_flags.cost - val_orig)/aff) > 6000 && ((obj->obj_flags.cost - val_orig)/aff) < 7500))
                {
                    sprintf(color1, "$c00%s%dp$c00%s%di$c00%s%det$c00%s%dr$c00%s%da$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3], (colore[4] > 9 ? "" : "0"), colore[4]);
                    sprintf(color2, "$c00%s%dp$c00%s%di$c00%s%det$c00%s%dr$c00%s%de$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3], (colore[4] > 9 ? "" : "0"), colore[4]);
                }
                else
                {
                    sprintf(color1, "$c00%s%dg$c00%s%de$c00%s%dm$c00%s%dm$c00%s%da$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3], (colore[4] > 9 ? "" : "0"), colore[4]);
                    sprintf(color2, "$c00%s%dg$c00%s%de$c00%s%dm$c00%s%dm$c00%s%de$c0007", (colore[0] > 9 ? "" : "0"), colore[0], (colore[1] > 9 ? "" : "0"), colore[1], (colore[2] > 9 ? "" : "0"), colore[2], (colore[3] > 9 ? "" : "0"), colore[3], (colore[4] > 9 ? "" : "0"), colore[4]);
                }
                break;

            default:
                mudlog(LOG_CHECK, "Do_insert: something going wrong with number of gems[%d]", aff);
                send_to_char("Qualcosa e' andato storto, contatta un immortale per cortesia", ch);
                return;
        }

        if((hitroll + damroll) > 0 && GET_ITEM_TYPE(obj) == ITEM_WEAPON)
        {
            bool hnd = FALSE, h = FALSE, d = FALSE;

            for(i = 0; i < MAX_OBJ_AFFECT; i++)
            {
                if(hitroll == damroll && obj->affected[i].location == APPLY_NONE && !hnd)
                {
                    obj->affected[i].location = APPLY_HITNDAM;
                    obj->affected[i].modifier = damroll;
                    hnd = TRUE;
                }
                else if(hitroll != 0 && obj->affected[i].location == APPLY_NONE && !h && !hnd)
                {
                    obj->affected[i].location = APPLY_HITROLL;
                    obj->affected[i].modifier = hitroll;
                    h = TRUE;
                }
                else if(damroll != 0 && obj->affected[i].location == APPLY_NONE && !d && !hnd)
                {
                    obj->affected[i].location = APPLY_DAMROLL;
                    obj->affected[i].modifier = damroll;
                    d = TRUE;
                }
            }
        }

        mudlog(LOG_PLAYERS, "%s insert with success some gems in %s", GET_NAME(ch), obj->short_description);
        WAIT_STATE(ch, wait);
        act("\n\rHai terminato il tuo lavoro su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_CHAR);
        act("\n\r", TRUE, ch, obj, 0, TO_ROOM);
        act("$n mette via tutti gli attrezzi, e' soddisfatt$b del suo lavoro su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_ROOM);

        if(((obj->obj_flags.cost - val_orig)/aff) <= 1500)
        {
            if(aff == 1)
                sprintf(buf, "%s con una %s incastrata brutalmente", obj->short_description, color1);
            else
                sprintf(buf, "%s con %s incastrate brutalmente", obj->short_description, color2);
        }
        else if(((obj->obj_flags.cost - val_orig)/aff) <= 3000)
        {
            if(aff == 1)
                sprintf(buf, "%s con incastonata una %s preziosa", obj->short_description, color1);
            else
                sprintf(buf, "%s con incastonate alcune %s preziose", obj->short_description, color2);
        }
        else if(((obj->obj_flags.cost - val_orig)/aff) <= 4500)
        {
            if(aff == 1)
                sprintf(buf, "%s con una %s preziosa cesellata finemente", obj->short_description, color1);
            else
                sprintf(buf, "%s con delle %s preziose cesellate finemente", obj->short_description, color2);
        }
        else if(((obj->obj_flags.cost - val_orig)/aff) <= 6000)
        {
            if(aff == 1)
                sprintf(buf, "%s con una grande %s incastonata elegantemente", obj->short_description, color1);
            else
                sprintf(buf, "%s con delle grandi %s incastonate elegantemente", obj->short_description, color2);
        }
        else if(((obj->obj_flags.cost - val_orig)/aff) < 7500)
        {
            if(aff == 1)
                sprintf(buf, "%s con una rarissima %s preziosa sapientemente incastonata", obj->short_description, color1);
            else
                sprintf(buf, "%s con rarissime %s preziose sapientemente incastonate", obj->short_description, color2);
        }
        else
        {
            if(aff == 1)
                sprintf(buf, "%s con una %s unica cesellata ad arte", obj->short_description, color1);
            else
                sprintf(buf, "%s con alcune %s uniche cesellate ad arte", obj->short_description, color2);
        }

        free(obj->short_description);
        obj->short_description = (char*)strdup(buf);
        free(obj->description);
        sprintf(buf, "%s e' qui per terra.", buf);
        obj->description = (char*)strdup(buf);
        SET_BIT(obj->obj_flags.extra_flags2, ITEM2_INSERT);
    }
    else
    {
        send_to_char("Cosa vuoi incastonare?\n\r",ch);
        return;
    }

}

} // namespace Alarmud
