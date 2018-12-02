/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * AlarMUD v1.0   Released 12/1994
 * See license.doc for distribution terms.   AlarMUD is based on DIKUMUD
 * Tradotto in Italiano da Emanuele Benedetti
 * Modificato per Nebbie2000 da Ryltar
 * $Id: act.comm.c,v 2.1 2002/05/06 09:50:00 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
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
#include "act.comm.hpp"
#include "auction.hpp"
#include "cmdid.hpp"      // for CMD_GTELL, CMD_THINK_SUPERNI, CMD_WHISPER
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "regen.hpp"
#include "signals.hpp"    // for PopStatus, PushStatus
#include "spells.hpp"
#include "utility.hpp"    // for apply_soundproof, check_soundproof, number
namespace Alarmud {

char* scrambler(struct char_data* ch,const char* message) {
	static char buf[MAX_STRING_LENGTH];
	int i;
	int percent=0;
	percent=GET_INT(ch);
	strncat(buf,message,MAX_STRING_LENGTH-1);
	buf[MAX_STRING_LENGTH-1]='\0';
	for(i=0; buf[i]; i++) {
		if(number(0,15) > percent) {
			buf[i]=RandomChar();
		}
	}
	return buf;
}

#define scramble(ch,msg) (msg)
ACTION_FUNC(do_say) {
	int i;
	char buf[MAX_INPUT_LENGTH+40];

	if(apply_soundproof(ch)) {
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	for(i = 0; *(arg + i) == ' '; i++);

	if(!*(arg + i)) {
		send_to_char("Si, ma COSA vuoi dire ?\n\r", ch);
	}
	else {
		snprintf(buf,MAX_INPUT_LENGTH+39,"$c0015[$c0005$n$c0015] dice '%s'", scramble(ch,arg + i));
		act(buf,FALSE,ch,0,0,TO_ROOM);
		if(IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
			snprintf(buf,MAX_INPUT_LENGTH+39,"Tu dici '%s'\n\r", arg + i);
			send_to_char(buf, ch);
		}
	}
}

ACTION_FUNC(do_report) {
	char buf[100];


	if(apply_soundproof(ch)) {
		return;
	}

	if(IS_NPC(ch)) {
		return;
	}

	if(GET_HIT(ch) > GET_MAX_HIT(ch) ||                 /* bug fix */
			GET_MANA(ch) > GET_MAX_MANA(ch) ||
			GET_MOVE(ch) > GET_MAX_MOVE(ch)) {
		send_to_char("Mi spiace, ma non puoi farlo ora.\n\r",ch);
		return;
	}

	snprintf(buf,99,"$c0014[$c0015$n$c0014] dichiara "
			 "'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'",
			 ((float)GET_HIT(ch) / (int)GET_MAX_HIT(ch)) * 100.0 + 0.5,
			 ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
			 ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);
	act(buf,FALSE,ch,0,0,TO_ROOM);
	snprintf(buf,99,"$c0014Dichiari 'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'",
			 ((float)GET_HIT(ch)  / (int)GET_MAX_HIT(ch))  * 100.0 + 0.5,
			 ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
			 ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);

	act(buf,FALSE, ch,0,0,TO_CHAR);

}



ACTION_FUNC(do_shout) {
	char buf1[MAX_INPUT_LENGTH+40];
	struct descriptor_data* i;

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi urlare!!\n\r", ch);
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi urlare in queste condizioni.\n\r", ch);
        return;
    }

	if(IS_NPC(ch) &&
			(Silence == 1) &&
			(IS_SET(ch->specials.act, ACT_POLYSELF))) {
		send_to_char("Polymorphed shouting has been banned.\n\r", ch);
		send_to_char("It may return after a bit.\n\r", ch);
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	for(; *arg == ' '; arg++);

	if(ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
		if(!IS_IMMORTAL(ch->master)) {
			send_to_char("Non credo proprio :-)", ch->master);
			return;
		}
	}

	if((GET_MOVE(ch)<10 || GET_MANA(ch)<10) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("Non hai abbastanza forza per gridare !\n\r",ch);
		return;
	}

	if(!(*arg)) {
		send_to_char("Vuoi urlare ? Ottimo ! Ma COSA ??\n\r", ch);
	}
	else {
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			snprintf(buf1,MAX_INPUT_LENGTH+39,"$c0009Tu gridi '%s'", arg);
			act(buf1,FALSE, ch,0,0,TO_CHAR);
		}
		snprintf(buf1,MAX_INPUT_LENGTH+39, "$c0009[$c0015$n$c0009] grida '%s'", scramble(ch,arg));

		act("$c0009[$c0015$n$c0009] alza la testa e grida forte", FALSE, ch, 0, 0,
			TO_ROOM);

		if(GetMaxLevel(ch)<IMMORTALE) {
			GET_MOVE(ch) -=10;
			alter_move(ch,0);
			GET_MANA(ch) -=10;
			alter_mana(ch,0);
		}

		for(i = descriptor_list; i; i = i->next) {
			if(i->character != ch && !i->connected &&
					(IS_NPC(i->character) ||
					 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
					  !IS_SET(i->character->specials.act, PLR_DEAF))) &&
					!check_soundproof(i->character)) {
				act(buf1, 0, ch, 0, i->character, TO_VICT);
			}
		}
	}
}


ACTION_FUNC(do_gossip) {
	char buf1[MAX_INPUT_LENGTH+40];
	struct descriptor_data* i;

	int IsRoomDistanceInRange(int nFirstRoom, int nSecondRoom, int nRange);

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi gridare, parlare od annunciare.\n\r", ch);
		return;
	}

	if(IS_NPC(ch) &&
			(Silence == 1) &&
			(IS_SET(ch->specials.act, ACT_POLYSELF))) {
		send_to_char("Polymorphed gossiping has been banned.\n\r", ch);
		send_to_char("It may return after a bit.\n\r", ch);
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	if(apply_soundproof(ch)) {
		return;
	}

	for(; *arg == ' '; arg++);

	if(ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
		if(!IS_IMMORTAL(ch->master)) {
			send_to_char("Non credo proprio :-)", ch->master);
			return;
		}
	}

	if(!(*arg)) {
		send_to_char("Parlare ? Ma di COSA !\n\r", ch);
	}
	else {
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			snprintf(buf1,MAX_INPUT_LENGTH+39,"$c0011Tu dici '%s'", arg);
			act(buf1, FALSE, ch, 0, 0, TO_CHAR);
		}
		snprintf(buf1,MAX_INPUT_LENGTH+39, "$c0011[$c0015$n$c0011] vi dice '%s'", scramble(ch,arg));
		for(i = descriptor_list; i; i = i->next) {
			if(i->character != ch && !i->connected &&
					(IS_NPC(i->character) ||
					 !IS_SET(i->character->specials.act, PLR_NOGOSSIP)) &&
					!check_soundproof(i->character)) {

				if(i->character->in_room != NOWHERE) {
					if(real_roomp(ch->in_room)->zone ==
							real_roomp(i->character->in_room)->zone ||
							GetMaxLevel(i->character) >= IMMORTALE ||
							GetMaxLevel(ch) >= IMMORTALE) {
						act(buf1, 0, ch, 0, i->character, TO_VICT);
					}
				}
			}
		} /* end for */
	}
}


ACTION_FUNC(do_auction) {

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi gridare, parlare od annunciare.\n\r", ch);
		return;
	}
	if(IS_NPC(ch) &&
			(Silence == 1) &&
			(IS_SET(ch->specials.act, ACT_POLYSELF))) {
		send_to_char("Polymorphed auctioning has been banned.\n\r", ch);
		send_to_char("It may return after a bit.\n\r", ch);
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi farlo.\n\r", ch);
        return;
    }

	if(apply_soundproof(ch)) {
		return;
	}

	for(; *arg == ' '; arg++);

	if(ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
		if(!IS_IMMORTAL(ch->master)) {
			send_to_char("Non credo proprio :-)", ch->master);
			return;
		}
	}
	do_auction_int(ch,arg,cmd);
}

void talk_auction(const char* arg) {
	char buf1[MAX_INPUT_LENGTH+40];
	struct descriptor_data* i;

	snprintf(buf1,MAX_INPUT_LENGTH+39, "$c0010[$c0015AUCTION$c0010] '%s'", arg);
	for(i = descriptor_list; i; i = i->next) {
		if(!i->connected &&
				(IS_NPC(i->character) ||
				 !IS_SET(i->character->specials.act, PLR_NOGOSSIP)) &&
				!check_soundproof(i->character)) {

			if(i->character->in_room != NOWHERE) {
				act(buf1, 0, (auction->seller) ? auction->seller : i->character, 0, i->character, TO_VICT); // SALVO se non c'era venditore, act deve essere visualizzato
			}
		}
	}
}





ACTION_FUNC(do_commune) {
	static char buf1[MAX_INPUT_LENGTH];
	struct descriptor_data* i;
	int livello;
	char sep[3];
	if(cmd==CMD_THINK_SUPERNI) {
		livello=CREATORE;
		strcpy(sep,"||");
	}
	else {
		livello=IMMORTALE;
		strcpy(sep,"::");
	}
	for(; *arg == ' '; arg++);

	if(!(*arg)) {
		send_to_char("Comunicare fra gli Dei e` ottimo, ma COSA ?\n\r",ch);
	}
	else {
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			snprintf(buf1,MAX_INPUT_LENGTH-1,"$c0014Tu pensi %s '%s'", sep,arg);
			act(buf1,FALSE, ch,0,0,TO_CHAR);
		}
		snprintf(buf1, MAX_INPUT_LENGTH, "$c0014%s$c0015$n$c0014%s '%s'", sep,sep,arg);

		for(i = descriptor_list; i; i = i->next)
			if(i->character != ch && !i->connected && !IS_NPC(i->character) &&
					!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
					(GetMaxLevel(i->character) >= livello)) {
				act(buf1, 0, ch, 0, i->character, TO_VICT);
			}
	}
}


ACTION_FUNC(do_tell) {
	struct char_data* vict;
	char name[100], message[MAX_INPUT_LENGTH+20],
		 buf[MAX_INPUT_LENGTH+60];


	if(apply_soundproof(ch)) {
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg,name,message,sizeof name -1,sizeof message -1);

	if(!*name || !*message) {
		send_to_char("A chi e` che vuoi parlare ?\n\r", ch);
		return;
	}
	else if(!(vict = get_char_vis(ch, name))) {
		send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
		return;
	}
	else if(ch == vict) {
		send_to_char("Parlare a se stessi puo` essere utile, a volte...\n\r", ch);
		return;
	}
	else if(GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
		act("Sta dormendo, shhh.",FALSE,ch,0,vict,TO_CHAR);
		return;
	}
	else if(!(GetMaxLevel(ch) >= IMMORTALE) &&
			!IS_NPC(vict) && IS_SET(vict->specials.act,PLR_NOTELL)) {
		act("$N non sta ascoltando adesso.",FALSE,ch,0,vict,TO_CHAR);
		return;
	}
	else if((GetMaxLevel(vict) >= IMMORTALE) &&
			(GetMaxLevel(ch) >= IMMORTALE) &&
			(GetMaxLevel(ch) < GetMaxLevel(vict)) &&
			!IS_NPC(vict) && IS_SET(vict->specials.act, PLR_NOTELL)) {
		act("$N non sta ascoltando adesso.",FALSE,ch,0,vict,TO_CHAR);
		return;
	}
	else if(IS_LINKDEAD(vict)) {
		send_to_char("Non puo` sentirti. Ha perso il senso della realta`.\n\r",
					 ch);
		return;
	}

	if(check_soundproof(vict)) {
		send_to_char("Non riesce a sentire nemmeno la sua voce, li` dentro.\n\r",
					 ch);
		return;
	}

#if ZONE_COMM_ONLY
	if(real_roomp(ch->in_room)->zone !=
			real_roomp(vict->in_room)->zone
			&& GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
		return;
	}
#endif

	snprintf(buf,MAX_INPUT_LENGTH+59,"$c0013[$c0015%s$c0013] ti dice '%s'",
			 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
			 scramble(ch,message));
	act(buf,FALSE, vict,0,0,TO_CHAR);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		snprintf(buf,MAX_INPUT_LENGTH+59,"$c0013Tu dici a %s %s'%s'",
				 (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), (IS_AFFECTED2(vict, AFF2_AFK) ? "(che e` AFK) " : ""), message);
		act(buf,FALSE, ch,0,0,TO_CHAR);
	}
	act("$c0013$n dice qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	thief_listen(ch,vict,message,cmd);
}



ACTION_FUNC(do_whisper) {
	struct char_data* vict;
	char name[100], message[MAX_INPUT_LENGTH],
		 buf[MAX_INPUT_LENGTH];

	if(apply_soundproof(ch)) {
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg,name,message,sizeof name -1, sizeof message -1);

	if(!*name || !*message) {
		send_to_char("A chi vuoi sussurrare ? e cosa ?\n\r", ch);
	}
	else if(!(vict = get_char_room_vis(ch, name))) {
		send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
	}
	else if(vict == ch) {
		act("$n sussurra silenziosamente a se stesso.", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char("Sembra che le tue orecchie non siano abbastanza vicine "
					 "alla bocca...\n\r", ch);
	}
	else if(IS_LINKDEAD(vict)) {
		send_to_char("Non puo` sentirti. Ha perso il senso della realta`.\n\r",
					 ch);
		return;
	}
	else {
		if(check_soundproof(vict)) {
			return;
		}

		snprintf(buf,MAX_INPUT_LENGTH-1,"$c0005[$c0015$n$c0005] ti sussurra '%s'",scramble(ch,message));
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		if(IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
			snprintf(buf,MAX_INPUT_LENGTH-1,"$c0005Tu sussurri a %s%s, '%s'",
					 (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), (IS_AFFECTED2(vict, AFF2_AFK) ? " (che e` AFK)" : ""), message);
			act(buf,FALSE, ch,0,0,TO_CHAR);
		}
		act("$c0005$n sussurra qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
		thief_listen(ch,vict,message,cmd);
	}
}


ACTION_FUNC(do_ask) {
	struct char_data* vict;
	char name[100], message[MAX_INPUT_LENGTH],
		 buf[MAX_INPUT_LENGTH];

	if(apply_soundproof(ch)) {
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg,name,message,sizeof name -1,sizeof message -1);

	if(!*name || !*message) {
		send_to_char("A chi vuoi chiedere... e cosa ?\n\r", ch);
	}
	else if(!(vict = get_char_room_vis(ch, name))) {
		send_to_char("Non vedi nessuno con quel nome qui...\n\r", ch);
	}
	else if(vict == ch) {
		act("$c0006[$c0015$n$c0006] si chiede qualcosa... trovera` la soluzione ?",
			FALSE,ch,0,0,TO_ROOM);
		act("$c0006Oltre alla domanda, conosci anche la risposta ?", FALSE, ch, 0,
			0, TO_CHAR);
	}
	else if(IS_LINKDEAD(vict)) {
		send_to_char("Non puo` sentirti. Ha perso il senso della realta`.\n\r",
					 ch);
		return;
	}
	else {
		if(check_soundproof(vict)) {
			return;
		}

		snprintf(buf,MAX_INPUT_LENGTH-1,"$c0006[$c0015$n$c0006] ti chiede '%s'",scramble(ch,message));
		act(buf, FALSE, ch, 0, vict, TO_VICT);

		if(IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
			snprintf(buf, MAX_INPUT_LENGTH-1,"$c0006Tu chiedi a %s%s, '%s'",
					 (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), (IS_AFFECTED2(vict, AFF2_AFK) ? " (che e` AFK)" : ""), message);
			act(buf,FALSE, ch,0,0,TO_CHAR);
		}
		act("$c0006$n fa una domanda a $N.",FALSE,ch,0,vict,TO_NOTVICT);
        
    /* indizi per le quest */
    if(!IS_PC(vict) && affected_by_spell(ch,STATUS_QUEST) && (isname2("ladro",GET_NAME(vict)) || isname2("cacciatore",GET_NAME(vict)) || isname2("spia",GET_NAME(vict)) || isname2("shop_keeper",mob_index[vict->nr].specname)))    {
        
        if(ch->specials.quest_ref == NULL)    {
            sprintf(buf,"%s Cio' che cerchi appartiene al passato.",
                    GET_NAME(ch));
            do_tell(vict,buf,CMD_TELL);
            return;
        }
        
        if(strstr(message, "indizio") != NULL && IsHumanoid(vict)) {
            
            if(ch->specials.quest_ref && !(ch->specials.quest_ref = get_char_vis_world(ch, ch->specials.quest_ref->player.name, NULL))) {
                sprintf(buf,"%s Mi spiace, ma non ho informazioni al riguardo...",
                        GET_NAME(ch));
                do_tell(vict,buf,CMD_TELL);
                return;
            }
            else {
                sprintf(buf, "%s %s? Ho sentito che l'ultima volta e' stato vist%s a %s.",GET_NAME(ch), ch->specials.quest_ref->player.name,SSLF(ch->specials.quest_ref), real_roomp(ch->specials.quest_ref->in_room)->name);
                do_tell(vict,buf,CMD_TELL);
                
                if(number(0,1) == 1) {
                    struct affected_type* af;
                    
                    for(af = ch->affected; af; af = af->next) {
                        if(af->type == STATUS_QUEST) {
                            af->duration = af->duration/2;
                        }
                    }
                    
                    for(af = ch->specials.quest_ref->affected; af; af = af->next) {
                        if(af->type == STATUS_QUEST) {
                            af->duration = af->duration/2;
                        }
                    }
                    
                    act("\n\r$c0014Voci arrivano alla Gilda dei Mercenari.\nIl tempo per la tua missione viene dimezzato.$c0007\n", FALSE, ch, 0, ch, TO_CHAR);
                }
                return;
            }
            
        } else {
            sprintf(buf,"%s Se vuoi un indizio chiedimelo chiaramente... ma ci sono orecchie ovunque, e se la Gilda dei Mercenari sapra' che hai chiesto aiuto ti costera'!",
                    GET_NAME(ch));
            do_tell(vict,buf,CMD_TELL);
            return;
        }
     }
    /* end indizi quest*/
        
	}
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

ACTION_FUNC(do_write) {
	struct obj_data* paper = 0, *pen = 0;
	char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
		 buf[MAX_STRING_LENGTH];

	argument_interpreter(arg, papername, penname);

	if(!ch->desc) {
		return;
	}

	if(!*papername) {  /* nothing was delivered */
		send_to_char("write (on) papername (with) penname.\n\r", ch);
		return;
	}

	if(!*penname) {
		send_to_char("write (on) papername (with) penname.\n\r", ch);
		return;
	}
	if(!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
		snprintf(buf,MAX_STRING_LENGTH-1, "Tu non hai nessun %s.\n\r", papername);
		send_to_char(buf, ch);
		return;
	}
	if(!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
		snprintf(buf,MAX_STRING_LENGTH-1, "Tu non hai nessun %s.\n\r", papername);
		send_to_char(buf, ch);
		return;
	}

	/* ok.. now let's see what kind of stuff we've found */
	if(pen->obj_flags.type_flag != ITEM_PEN) {
		act("Non puoi scrivere con $p.", FALSE, ch, pen, 0, TO_CHAR);
	}
	else if(paper->obj_flags.type_flag != ITEM_NOTE) {
		act("Non puoi scrivere su $p.", FALSE, ch, paper, 0, TO_CHAR);
	}
	else if(paper->action_description && *paper->action_description) {
		send_to_char("C'e` gia` scritto sopra qualcosa.\n\r", ch);
		return;
	}
	else {
		/* we can write - hooray! */
		send_to_char
		("Ok... puoi scrivere... finisci la nota con un @.\n\r", ch);
		act("$n comincia a scrivere qualcosa.", TRUE, ch, 0,0,TO_ROOM);
#if 0
		if(paper->action_description) {
			free(paper->action_description);
		}
		paper->action_description = NULL;
#endif
		ch->desc->str = &paper->action_description;
		ch->desc->max_str = MAX_NOTE_LENGTH;
	}
}



ACTION_FUNC(do_sign) {
	int i;
	char buf[MAX_INPUT_LENGTH+40];
	char buf2[MAX_INPUT_LENGTH];
	char* p;
	int diff;
	struct char_data* t;
	struct room_data* rp;

	for(i = 0; *(arg + i) == ' '; i++);

	if(!*(arg + i)) {
		send_to_char("Daccordo, ma cosa vuoi dire ?\n\r", ch);
	}
	else {

		rp = real_roomp(ch->in_room);
		if(!rp) {
			return;
		}

		if(!HasHands(ch)) {
			send_to_char("Si giusto... ma con QUALI MANI ?????????\n\r", ch);
			return;
		}

		strcpy(buf, arg+i);
		buf2[0] = '\0';
		/*
		  work through the arg, word by word.  if you fail your
		  skill roll, the word comes out garbled.
		  */
		p = strtok(buf, " ");  /* first word */

		diff = strlen(buf);

		while(p) {
			if(ch->skills and number(1,75+strlen(p))<ch->skills[SKILL_SIGN].learned) {
				strcat(buf2, p);
			}
			else {
				strcat(buf2, RandomWord());
			}
			strcat(buf2, " ");
			diff -= 1;
			p = strtok(0, " ");  /* next word */
		}
		/*
		  if a recipient fails a roll, a word comes out garbled.
		  */

		/*
		  buf2 is now the "corrected" string.
		  */

		snprintf(buf,MAX_INPUT_LENGTH+39,"$c0015[$c0005$n$c0015], con i segni,  dice '%s'", buf2);

		for(t = rp->people; t; t=t->next_in_room) {
			if(t != ch) {
				if(t->skills && number(1,diff) < t->skills[SKILL_SIGN].learned) {
					act(buf, FALSE, ch, 0, t, TO_VICT);
				}
				else {
					act("$n muove le mani in modo molto buffo.",
						FALSE, ch, 0, t, TO_VICT);
				}
			}
		}

		if(IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
			snprintf(buf,MAX_INPUT_LENGTH+39,"Tu hai detto '%s'\n\r", arg + i);
			send_to_char(buf, ch);
		}
	}
}

/* speak elvish, speak dwarvish, etc...                    */
ACTION_FUNC(do_speak) {
	char buf[255];
	int i;

#define MAX_LANGS 8

	const char* lang_list[MAX_LANGS] = {
		"common",
		"elvish",
		"halfling",
		"dwarvish",
		"orcish",
		"giantish",
		"ogre",
		"gnomish"
	};

	only_argument(arg,buf);

	if(buf[0] == '\0') {
		send_to_char("In quale lingua vuoi parlare ?\n\r",ch);
		return;
	}

	if(strstr(buf,"common")) {
		i = SPEAK_COMMON;
	}
	else if(strstr(buf, "elvish")) {
		i = SPEAK_ELVISH;
	}
	else if(strstr(buf, "halfling")) {
		i = SPEAK_HALFLING;
	}
	else if(strstr(buf, "dwarvish")) {
		i = SPEAK_DWARVISH;
	}
	else if(strstr(buf, "orcish")) {
		i = SPEAK_ORCISH;
	}
	else if(strstr(buf, "giantish")) {
		i = SPEAK_GIANTISH;
	}
	else if(strstr(buf, "ogre")) {
		i = SPEAK_OGRE;
	}
	else if(strstr(buf, "gnomish")) {
		i = SPEAK_GNOMISH;
	}
	else {
		i = -1;
	}

	if(i == -1) {
		send_to_char("Non e` un linguaggio molto conosciuto.\n\r",ch);
		return;
	}

	/* set language that we're gonna speak */
	ch->player.speaks = i;
	snprintf(buf,254,"Sei concentrato nel parlare %s.\n\r",lang_list[i-1]);
	send_to_char(buf,ch);
}

void thief_listen(struct char_data* ch,struct char_data* victim, const char* frase,int cmd) {
	char* p=NULL;
	char* np=NULL;
	struct char_data* t;
	struct room_data* rp;
	int malus=0;
	int percent =0;
	rp = real_roomp(ch->in_room);
	if(cmd==CMD_WHISPER) {
		malus=5;
	}
	if(cmd==CMD_GTELL) {
		malus=10;
	}
	if(!IS_SINGLE(ch)) {
		malus+=20;
	}
	if(!rp) {
		return;
	}
	PushStatus("Thief_listen");
	for(t = rp->people; t; t=t->next_in_room) {
		if(!IS_DIO(ch) && !IS_DIO(victim))
			if((t != ch) && (t !=victim)) {
				if(HasClass(t,CLASS_THIEF) && t->skills &&
						t->skills[SKILL_TSPY].learned>0 &&
						affected_by_spell(t,SKILL_TSPY)) {
					percent=GetMaxLevel(ch)-GetMaxLevel(t);
					for(p=strdup(frase),np=p; *p; p++) {
						if((percent+number(20,120)+malus)>t->skills[SKILL_TSPY].learned) {
							if((malus+number(1,40)-GET_INT(t))>(0)) {
								if(number(0,20)) {
									PushStatus("Random");
									if(number(1,101)>100 &&
											t->skills[SKILL_TSPY].learned <100) {
										t->skills[SKILL_TSPY].learned++;
									}
									*p=RandomChar();
									PopStatus();
								}

							}
						}
					}
					act("$c0013Riesci ad origliare: '$T'", FALSE,
						t, 0, np, TO_CHAR);
					free(np);
				}
			}
	}
	PopStatus();

}

/* this is where we do the language says */
ACTION_FUNC(do_new_say) {
	int i, learned, skill_num;
	char buf[MAX_INPUT_LENGTH+40];
	char buf2[MAX_INPUT_LENGTH];
	char buf3[MAX_INPUT_LENGTH+40];
	char* p;
	int diff;
	struct char_data* t;
	struct room_data* rp;

	if(!arg) {
		return;
	}

	for(i = 0; *(arg + i) == ' '; i++);

	if(!arg[i]) {
		send_to_char("Ok, ma cosa hai da dire ?\n\r", ch);
	}
	else {

		if(apply_soundproof(ch)) {
			return;
		}
        
        if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        {
            send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
            return;
        }

		rp = real_roomp(ch->in_room);
		if(!rp) {
			return;
		}


		if(!ch->skills) {
			learned = 0;
			skill_num = LANG_COMMON;
		}
		else {
			/* find the language we are speaking */

			switch(ch->player.speaks) {
			case SPEAK_COMMON:
				learned = ch->skills[LANG_COMMON].learned;
				skill_num=LANG_COMMON;
				break;
			case SPEAK_ELVISH:
				learned = ch->skills[LANG_ELVISH].learned;
				skill_num=LANG_ELVISH;
				break;
			case SPEAK_HALFLING:
				learned = ch->skills[LANG_HALFLING].learned;
				skill_num=LANG_HALFLING;
				break;
			case SPEAK_DWARVISH:
				learned = ch->skills[LANG_DWARVISH].learned;
				skill_num=LANG_DWARVISH;
				break;
			case SPEAK_ORCISH:
				learned = ch->skills[LANG_ORCISH].learned;
				skill_num=LANG_ORCISH;
				break;
			case SPEAK_GIANTISH:
				learned = ch->skills[LANG_GIANTISH].learned;
				skill_num=LANG_GIANTISH;
				break;
			case SPEAK_OGRE:
				learned = ch->skills[LANG_OGRE].learned;
				skill_num=LANG_OGRE;
				break;
			case SPEAK_GNOMISH:
				learned = ch->skills[LANG_GNOMISH].learned;
				skill_num=LANG_GNOMISH;
				break;

			default:
				learned = ch->skills[LANG_COMMON].learned;
				skill_num = LANG_COMMON;
				break;
			} /* end switch */
		}
		/* end finding language */


		strcpy(buf, arg+i);
		buf2[0] = '\0';

		/* we use this for ESP and immortals and comprehend lang */
		snprintf(buf3,MAX_INPUT_LENGTH+39,"$c0015[$c0005$n$c0015] dice '%s'",buf);

		/*
		  work through the arg, word by word.  if you fail your
		  skill roll, the word comes out garbled.
		  */
		p = strtok(buf, " ");  /* first word */

		diff = strlen(buf);


		while(p) {
			if((number(1,75+strlen(p))<learned) or (GetMaxLevel(ch) >= IMMORTALE)) {
				strcat(buf2, p);
			}
			else {
				/* add case statement here to use random words from clips of elvish */
				/* dwarvish etc so the words look like they came from that language */
				strcat(buf2, RandomWord());
			}
			strcat(buf2, " ");
			diff -= 1;
			p = strtok(0, " ");  /* next word */
		}
		/*
		  if a recipient fails a roll, a word comes out garbled.
		  */

		/*
		  buf2 is now the "corrected" string.
		  */
		if(!*buf2) {
			send_to_char("OK, ma cosa hai da dire ?\n\r", ch);
			return;
		}

		snprintf(buf,MAX_INPUT_LENGTH+39,"$c0015[$c0005$n$c0015] dice '%s'", buf2);

		for(t = rp->people; t; t=t->next_in_room) {
			if(t != ch) {
				if((t->skills && number(1,diff) < t->skills[skill_num].learned) ||
						GetMaxLevel(t) >= IMMORTALE || IS_NPC(t) ||
						affected_by_spell(t, SKILL_ESP) ||
						affected_by_spell(t, SPELL_COMP_LANGUAGES) ||
						GetMaxLevel(ch) >= IMMORTALE) {

					/* these guys always understand */
					if(GetMaxLevel(t) >= IMMORTALE ||
							affected_by_spell(t,SKILL_ESP) ||
							affected_by_spell(t,SPELL_COMP_LANGUAGES) || IS_NPC(t)) {
						act(buf3, FALSE,ch,0,t,TO_VICT);
					}
					else
						/* otherwise */

					{
						act(buf, FALSE, ch, 0, t, TO_VICT);
					}
				}
				else {
					act("$c0010$n parla una lingua che non riesci a capire.", FALSE,
						ch, 0, t, TO_VICT);
				}
			}
		}

		if(IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
			snprintf(buf,MAX_INPUT_LENGTH+39,"$c0015Tu dici '%s'", arg + i);
			act(buf,FALSE, ch,0,0,TO_CHAR);
		}
	}
}



ACTION_FUNC(do_gtell) {
	int i;
	struct char_data* k;
	struct follow_type* f;
	char buf[MAX_STRING_LENGTH];

	if(apply_soundproof(ch)) {
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	for(i = 0; *(arg + i) == ' '; i++);

	if(!*(arg+i)) {
		send_to_char("Cosa vuoi dire al gruppo ?\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		send_to_char("Forse dovresti unirti ad un gruppo, prima.\n\r", ch);
		return;
	}
	else {
		thief_listen(ch,ch,arg+i,cmd);
		if(ch->master) {
			k = ch->master;
		}
		else {
			k = ch;
		}

		for(f=k->followers; f; f=f->next) {
			if(IS_AFFECTED(f->follower, AFF_GROUP)) {
				if(!f->follower->desc) {
					/* link dead */
				}
				else if(ch == f->follower) {
					/* can't tell yourself! */
				}
				else if(!check_soundproof(f->follower)) {
					snprintf(buf,MAX_STRING_LENGTH-1,"$c0012[$c0015%s$c0012] dice al gruppo '%s'",
							 (IS_NPC(ch) ? ch->player.short_descr :
							  GET_NAME(ch)), arg+i);
					act(buf, FALSE,f->follower,0,0,TO_CHAR);
				} /* !soundproof */
			}
		} /* end for loop */

		/* send to master now */
		if(ch->master) {
			if(IS_AFFECTED(ch->master, AFF_GROUP)) {
				if(!ch->master->desc) {
					/* link dead */
				}
				else if(ch == ch->master) {
					/* can't tell yourself! */
				}
				else if(!check_soundproof(ch->master)) {
					snprintf(buf,MAX_STRING_LENGTH-1,"$c0012[$c0015%s$c0012] dice al gruppo '%s'",
							 (IS_NPC(ch) ? ch->player.short_descr :
							  GET_NAME(ch)), arg+i);
					act(buf, FALSE,ch->master,0,0,TO_CHAR);
				} /* !soundproof */
			}
		}         /* end master send */

		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			snprintf(buf,MAX_STRING_LENGTH-1,"$c0012Tu dici al gruppo '%s'",arg+i);
			act(buf, FALSE,ch,0,0,TO_CHAR);
		} /* if echo */

	} /* they where grouped... */
} /* end of gtel */

/*
 * 'Split' originally by Gnort, God of Chaos. I stole it from Merc
 * and changed it to work with mine :) Heh msw
 */

ACTION_FUNC(do_split) {

	char buf[MAX_STRING_LENGTH];
	char tmp[MAX_INPUT_LENGTH];
	struct char_data* gch;
	int members, amount, share, extra;

	one_argument(arg, tmp);

	if(tmp[0] == '\0') {
		send_to_char("Quanto vuoi dividere ?\n\r", ch);
		return;
	}

	amount = atoi(tmp);

	if(amount < 0) {
		send_to_char("Non cercare di fare il furbo con il tuo gruppo.\n\r", ch);
		return;
	}

	if(amount == 0) {
		send_to_char("Hai diviso zero monete, ma nessuno lo ha notato.\n\r", ch);
		return;
	}

	if(ch->points.gold < amount) {
		send_to_char("Non hai tutto quell'oro.\n\r", ch);
		return;
	}

	members = 0;
	for(gch = real_roomp(ch->in_room)->people;
			gch != NULL; gch = gch->next_in_room) {
		if(is_same_group(gch, ch)) {
			members++;
		}
	}

	if(members < 2) {
		send_to_char("Ma cosa vuoi dividere che sei solo.\n\r", ch);
		return;
	}

	share = amount / members;
	extra = amount % members;

	if(share == 0) {
		send_to_char("C'e` poco da dividere, siete in troppi.\n\r", ch);
		return;
	}

	ch->points.gold -= amount;
	ch->points.gold += share + extra;

	snprintf(buf,MAX_STRING_LENGTH-1,
			 "Hai diviso %d monete d'oro. La tua parte e` di %d monete.\n\r",
			 amount, share + extra);
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH-1,
			 "$n divide %d monete d'oro. La tua parte e` di %d monete.",
			 amount, share);

	for(gch = real_roomp(ch->in_room)->people;
			gch != NULL; gch = gch->next_in_room) {
		if(gch != ch && is_same_group(gch, ch)) {
			act(buf,FALSE,ch, NULL, gch, TO_VICT);
			gch->points.gold += share;
		}
	}

	return;
}


ACTION_FUNC(do_pray) {
	struct affected_type af;
	char buf1[MAX_INPUT_LENGTH * 2];
	char GodName[MAX_INPUT_LENGTH];
	struct descriptor_data* i;
	int ii=0;
	int durata=6;
	if(!IS_PC(ch)) {
		return;
	}

	if(affected_by_spell(ch, SPELL_PRAYER)) {
		send_to_char("Hai gia` pregato oggi.\n\r", ch);
		return;
	}

	for(; *arg == ' '; arg++);
	if(!(*arg))
		send_to_char("Vuoi pregare. Ottimo, ma chi ? "
					 "(pray <NomeDio> <preghiera>)\n\r", ch);
	else {
		ii = (GetMaxLevel(ch) * 1.5  + 20);
		one_argument(arg,GodName);
		mudlog(LOG_CHECK,"%s ha pregato %s",GET_NAME(ch),GodName);
		if(HasClass(ch, CLASS_CLERIC | CLASS_DRUID)) {
			ii +=10;    /* clerics get a 10% bonus :) */
		}

		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			snprintf(buf1,(MAX_INPUT_LENGTH*2)-1,"Tu preghi '%s'\n\r", arg);
			send_to_char(buf1, ch);
		}
		if (ch->desc->AccountData.authorized) {
			ii+=60;
		}
		buf1[160]=0;
		if(ii > number(1, 101)) {

			for(i = descriptor_list; i; i = i->next) {
				if(i->character != ch && ! i->connected && ! IS_NPC(i->character) &&
						!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
						(GetMaxLevel(i->character) >= IMMORTALE)) {
					if(!str_cmp2(GodName,GET_NAME(i->character)))
						snprintf(buf1, (MAX_INPUT_LENGTH*2)-1,
								 "$c0013[$c0015$n$c0013] TI PREGA: '%s'", arg);
					else
						snprintf(buf1, (MAX_INPUT_LENGTH*2)-1,
								 "$c0014[$c0015$n$c0014] prega :'%s'", arg);
					act(buf1, 0, ch, 0, i->character, TO_VICT);
				}

			} /* end for */
			durata=12;
		} /* failed prayer */

		af.type = SPELL_PRAYER;
		af.duration = durata;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(ch, &af);
		return;
	}
}

/* modified by Aarcerak */
bool is_same_group(struct char_data* ach, struct char_data* bch) {
	if(!IS_AFFECTED(ach, AFF_GROUP) || !IS_AFFECTED(bch, AFF_GROUP)) {
		return 0;
	}
	if(ach->master != NULL) {
		ach = ach->master;
	}
	if(bch->master != NULL) {
		bch = bch->master;
	}
	return(ach == bch);
}


ACTION_FUNC(do_telepathy) {
	struct char_data* vict;
	char name[100], message[MAX_INPUT_LENGTH+20],
		 buf[MAX_INPUT_LENGTH+60];

	half_chop(arg, name, message,sizeof name -1,sizeof message -1);

	if(!HasClass(ch, CLASS_PSI) && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
		send_to_char("Cosa pensi di essere? Un telepate?\n\r", ch);
		return;
	}
    
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi farlo in queste condizioni.\n\r", ch);
        return;
    }

	if(GET_MANA(ch) < 5 && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
		send_to_char("Non hai la potenza mentale sufficiente.\n\r",ch);
		return;
	}

	if(!*name || !*message) {
		send_to_char("A chi vuoi mandare il tuo pensiero ?\n\r", ch);
		return;
	}
	else if(!(vict = get_char_vis(ch, name))) {
		send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
		return;
	}
	else if(ch == vict) {
		send_to_char("Nella tua mente risuona il tuo pensiero...\n\r", ch);
		return;
	}
	else if(GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
		act("Sta dormendo, shhh.",FALSE,ch,0,vict,TO_CHAR);
		return;
	}
	else if(IS_NPC(vict) && !(vict->desc)) {
		send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
		return;
	}
	else if(!(GetMaxLevel(ch) >= IMMORTALE) &&
			IS_SET(vict->specials.act, PLR_NOTELL)) {
		act("$N non sta ascoltando adesso.", FALSE, ch, 0, vict, TO_CHAR);
		return;
	}
	else if((GetMaxLevel(vict) >= IMMORTALE) &&
			(GetMaxLevel(ch) >= IMMORTALE) &&
			(GetMaxLevel(ch) < GetMaxLevel(vict)) &&
			IS_SET(vict->specials.act, PLR_NOTELL)) {
		act("La mente di $N e` chiusa in questo momento !", FALSE, ch, 0, vict,
			TO_CHAR);
		return;
	}
	else if(!vict->desc) {
		send_to_char("Non puo` sentirti. Gli e` caduta la linea (link dead).\n\r",
					 ch);
		return;
	}

	/*
	  if (check_soundproof(vict)) {
	        send_to_char("In a silenced room, try again later.\n\r",ch);
	        return;
	  }
	*/

	if(!IS_AFFECTED(ch, AFF_TELEPATHY)) {
		GET_MANA(ch) -=5;
		alter_mana(ch,0);
	}

	snprintf(buf,MAX_INPUT_LENGTH+59,
			 "$c0013[$c0015%s$c0013] ti manda il pensiero '%s'",
			 (IS_NPC(ch) ? ch->player.short_descr :
			  GET_NAME(ch)), message);
	act(buf, FALSE, vict, 0, 0, TO_CHAR);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		snprintf(buf, MAX_INPUT_LENGTH+59,
				 "$c0013Tu mandi a %s il pensiero '%s'",
				 (IS_NPC(vict) ? vict->player.short_descr :
				  GET_NAME(vict)), message);
		act(buf,FALSE, ch,0,0,TO_CHAR);
	}
}

ACTION_FUNC(do_eavesdrop) {
	char buf[MAX_STRING_LENGTH];
	struct room_direction_data* exitp;
	int dir;
	one_argument(arg, buf);


	if(!*buf) {
		send_to_char("In quale direzione vuoi ascoltare?\r\n", ch);
		return;
	}
	if((dir = search_block(buf, dirs, FALSE)) < 0) {
		send_to_char("Che direzione sarebbe?\r\n", ch);
		return;
	}
	if(ch->skills[SKILL_EAVESDROP].learned<number(1,101)) {
		send_to_char("Resti immobile concentrandoti sui rumori che provengono da quella parte.\r\n", ch);
		WAIT_STATE(ch, PULSE_VIOLENCE); // eavesdrop
		return;
	}
	exitp = EXIT(ch, dir); // SALVO se la stanza e' -1 crash
	if(exitp && exitp->to_room > 0) {
		if(IS_SET(exitp->exit_info, EX_CLOSED) && exitp->keyword) {
			snprintf(buf, MAX_STRING_LENGTH-1, "The %s is closed.\r\n", fname(exitp->keyword));
			send_to_char(buf, ch);
		}
		else {
			ch->next_listener = real_roomp(exitp->to_room)->listeners;
			real_roomp(exitp->to_room)->listeners = ch;
			ch->listening_to = exitp->to_room;
			send_to_char("Resti immobile contentrandoti sui rumori che provengono da quella parte.", ch);
			WAIT_STATE(ch, PULSE_VIOLENCE); // eavesdrop
		}
	}
	else {
		send_to_char("Non c'e' nulla da quella parte...\r\n", ch);
	}
}


} // namespace Alarmud

