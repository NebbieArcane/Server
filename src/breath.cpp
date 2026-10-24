/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/****************************************************************************
 * AlarMUD. Server per MUD.
 *
 * Files: breath.c breath.h
 * Contengono le funzioni per la gestione dei soffi dei draghi.
 *
 * $Id: breath.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdlib>
#include <cstdio>
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
#include "breath.hpp"
#include "spec_dragon.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "modify.hpp"
#include "regen.hpp"
#include "snew.hpp"
#include "spells.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "structs.hpp"
namespace Alarmud {


struct breath_victim* choose_victims(struct char_data* ch,struct char_data* first_victim) {
	/* this is goofy, dopey extraordinaire */
	struct char_data* cons;
	struct breath_victim* head = NULL, *temp=NULL;

	for(cons = real_roomp(ch->in_room)->people; cons;
			cons = cons->next_in_room) {
		temp = (struct breath_victim*)malloc(sizeof(*temp));
		temp->ch = cons;
		temp->next = head;
		head = temp;
		if(first_victim == cons) {
			temp->yesno = 1;
		}
		else if(ch == cons) {
			temp->yesno = 0;
		}
		else if((in_group(first_victim, cons) ||
				 cons == first_victim->master ||
				 cons->master == first_victim) &&
				(temp->yesno = (3 != dice(1,5)))) {
			/* group members will get hit 4/5 times */
		}
		else if(cons->specials.fighting == ch) {
			/* people fighting the dragon get hit 4/5 times */
			temp->yesno = (3 != dice(1,5));
		}
		else { /* bystanders get his 2/5 times */
			temp->yesno = (dice(1,5)<3);
		}
	}
	return head;
}

void free_victims(struct breath_victim* head) {
	struct  breath_victim* temp;

	while(head) {
		temp = head->next;
		free(head);
		head = temp;
	}
}

void breath_weapon(struct char_data* ch, struct char_data* target,int mana_cost, breath_func func) {
	struct breath_victim* hitlist, *scan;
	struct char_data* tmp;
	int        victim;

	act("$n inarca la schiena e prende un gran respiro.",1,ch,0,ch->specials.fighting,TO_ROOM);
	victim=0;
	bool area_skipped = false; /* Blocco minimo: intoccabili saltati */

	for(tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
		if(area_target_untouchable(ch, tmp)) {
			area_skipped = true;
			continue;
		}
		if(tmp == ch || IS_IMMORTAL(tmp)) {
			continue;
		}
		else {
			victim=1;
			cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tmp, 0);
		}
	}

	if(!victim && area_skipped) { /* solo intoccabili: niente soffio, niente mana */
		area_skip_notice(ch, true);
		area_no_target_notice(ch);
		return;
	}

	hitlist = choose_victims(ch, target);

	if(func!=NULL && victim && hitlist) {
		act("$n soffia con violenza...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

		for(scan = hitlist; scan; scan = scan->next) {
			if(!scan->yesno ||
					IS_IMMORTAL(scan->ch) ||
					area_target_untouchable(ch, scan->ch) ||
					scan->ch->in_room != ch->in_room /* this could happen if
                                              someone fled, I guess */
			  ) {
				continue;
			}
			func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
		}
		area_skip_notice(ch, area_skipped);
		GET_MANA(ch) -=mana_cost;
		alter_mana(ch,0);
	}
	else {
		act("$n prova a soffiare... ma tossisce e sputacchia.",
			1, ch, 0, ch->specials.fighting, TO_ROOM);
		do_flee(ch, "", 0);
	}

	free_victims(hitlist);
}

void use_breath_weapon(struct char_data* ch, struct char_data* target,int cost, breath_func func) {
	if(GET_MANA(ch)>=0) {
		breath_weapon(ch, target, cost, func);
	}
	else if((GET_HIT(ch) < GET_MAX_HIT(ch)/2) && (GET_MANA(ch) >= -cost)) {
		breath_weapon(ch, target, cost, func);
	}
	else if((GET_HIT(ch) < GET_MAX_HIT(ch)/4) && (GET_MANA(ch) >= -2*cost)) {
		breath_weapon(ch, target, cost, func);
	}
	else if(GET_MANA(ch)<=-3*cost) {
		breath_weapon(ch, target, 0, NULL); /* sputter */
	}
}


breath_func bweapons[] = {
	cast_geyser,
	cast_fire_breath,
	cast_gas_breath,
	cast_frost_breath,
	cast_acid_breath,
	cast_lightning_breath
};

ACTION_FUNC(do_breath) {
	char        name[MAX_STRING_LENGTH];

	if(check_peaceful(ch,"Non sarebbe per niente carino.\n\r")) {
		return;
	}

	only_argument(arg, name);

	BreathWeapon(ch, CMD_BREATH, "",ch,0);

	WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

} // namespace Alarmud

