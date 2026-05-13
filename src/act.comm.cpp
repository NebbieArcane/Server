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
#include <array>
#include <algorithm>
#include <cerrno>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
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

namespace {
/** Lunghezza C-string limitata (portabile: non usare std::strnlen, non e' C++ standard). */
std::size_t mud_capped_strlen(const char* s, std::size_t cap) {
	std::size_t n = 0;
	while(n < cap && s[n] != '\0') {
		++n;
	}
	return n;
}
} // namespace

std::string scrambler(struct char_data* ch, const char* message) {
	if(ch == nullptr || message == nullptr) {
		return {};
	}
	const int percent = GET_INT(ch);
	std::string out;
	out.assign(message, mud_capped_strlen(message, MAX_STRING_LENGTH - 1));
	for(std::size_t i = 0; i < out.size(); i++) {
		if(number(0, 15) > percent) {
			out[i] = RandomChar();
		}
	}
	return out;
}

/* Per canali in chiaro lasciare (msg). Per offuscare in base a INT del PG:
 * #define scramble(ch, msg) Alarmud::scrambler((ch), (msg)) */
#define scramble(ch, msg) (msg)

ACTION_FUNC(do_say) {
	if(ch == nullptr || arg == nullptr) {
		return;
	}
	if(apply_soundproof(ch)) {
		return;
	}
	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
		return;
	}

	const char* speech = arg;
	while(*speech == ' ') {
		speech++;
	}
	if(*speech == '\0') {
		send_to_char("Si, ma COSA vuoi dire ?\n\r", ch);
		return;
	}

	std::string roomLine = "$c0015[$c0005$n$c0015] dice '";
	roomLine += scramble(ch, speech);
	roomLine += "'";
	act(roomLine.c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::string selfLine = "Tu dici '";
		selfLine += speech;
		selfLine += "'\n\r";
		send_to_char(selfLine.c_str(), ch);
	}
}

ACTION_FUNC(do_report) {
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

	const double hitPct = (static_cast<double>(GET_HIT(ch)) / static_cast<double>(GET_MAX_HIT(ch))) * 100.0 + 0.5;
	const double manaPct = (static_cast<double>(GET_MANA(ch)) / static_cast<double>(GET_MAX_MANA(ch))) * 100.0 + 0.5;
	const double movePct = (static_cast<double>(GET_MOVE(ch)) / static_cast<double>(GET_MAX_MOVE(ch))) * 100.0 + 0.5;
	std::ostringstream roomReport;
	roomReport << "$c0014[$c0015$n$c0014] dichiara 'HP:"
			   << std::setw(2) << std::fixed << std::setprecision(0) << hitPct
			   << "% MANA:" << std::setw(2) << manaPct
			   << "% MV:" << std::setw(2) << movePct << "%'";
	act(roomReport.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	std::ostringstream selfReport;
	selfReport << "$c0014Dichiari 'HP:"
			   << std::setw(2) << std::fixed << std::setprecision(0) << hitPct
			   << "% MANA:" << std::setw(2) << manaPct
			   << "% MV:" << std::setw(2) << movePct << "%'";
	act(selfReport.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

}



ACTION_FUNC(do_shout) {
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
		std::string shoutLine;
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			std::string selfLine = "$c0009Tu gridi '";
			selfLine += arg;
			selfLine += "'";
			act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		shoutLine = "$c0009[$c0015$n$c0009] grida '";
		shoutLine += scramble(ch, arg);
		shoutLine += "'";

		act("$c0009[$c0015$n$c0009] alza la testa e grida forte", FALSE, ch, nullptr, nullptr,
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
				act(shoutLine.c_str(), 0, ch, 0, i->character, TO_VICT);
			}
		}
	}
}


ACTION_FUNC(do_gossip) {
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
		std::string gossipLine;
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			std::string selfLine = "$c0011Tu dici '";
			selfLine += arg;
			selfLine += "'";
			act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		gossipLine = "$c0011[$c0015$n$c0011] vi dice '";
		gossipLine += scramble(ch, arg);
		gossipLine += "'";
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
						act(gossipLine.c_str(), 0, ch, 0, i->character, TO_VICT);
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
	struct descriptor_data* i;
	std::string auctionLine = "$c0010[$c0015AUCTION$c0010] '";
	auctionLine += arg;
	auctionLine += "'";

	for(i = descriptor_list; i; i = i->next) {
		if(!i->connected &&
				(IS_NPC(i->character) ||
				 !IS_SET(i->character->specials.act, PLR_NOGOSSIP)) &&
				!check_soundproof(i->character)) {

			if(i->character->in_room != NOWHERE) {
				act(auctionLine.c_str(), 0, (auction->seller) ? auction->seller : i->character, 0, i->character, TO_VICT); // SALVO se non c'era venditore, act deve essere visualizzato
			}
		}
	}
}





ACTION_FUNC(do_commune) {
	struct descriptor_data* i;
	int livello;
	const char* sep = nullptr;
	if(cmd==CMD_THINK_SUPERNI) {
		livello=CREATORE;
		sep = "||";
	}
	else {
		livello=IMMORTALE;
		sep = "::";
	}
	for(; *arg == ' '; arg++);

	if(!(*arg)) {
		send_to_char("Comunicare fra gli Dei e' ottimo, ma COSA?\n\r",ch);
	}
	else {
		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			std::string selfLine = "$c0014Tu pensi ";
			selfLine += sep;
			selfLine += " '";
			selfLine += arg;
			selfLine += "'";
			act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		std::string communeLine = "$c0014";
		communeLine += sep;
		communeLine += "$c0015$n$c0014";
		communeLine += sep;
		communeLine += " '";
		communeLine += arg;
		communeLine += "'";

		for(i = descriptor_list; i; i = i->next)
			if(i->character != ch && !i->connected && !IS_NPC(i->character) &&
					!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
					(GetMaxLevel(i->character) >= livello)) {
				act(communeLine.c_str(), 0, ch, 0, i->character, TO_VICT);
			}
	}
}


ACTION_FUNC(do_tell) {
	struct char_data* vict;
	std::array<char, 100> name{};
	std::array<char, MAX_INPUT_LENGTH + 20> message{};


	if(apply_soundproof(ch)) {
		return;
	}

    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg, name.data(), message.data(), static_cast<int>(name.size()) - 1,
	          static_cast<int>(message.size()) - 1);

	if(name[0] == '\0' || message[0] == '\0') {
		send_to_char("A chi e' che vuoi parlare ?\n\r", ch);
		return;
	}
	else if(!(vict = get_char_vis(ch, name.data()))) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
		return;
	}
	else if(ch == vict) {
		send_to_char("Parlare a se stessi puo' essere utile, a volte...\n\r", ch);
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
		send_to_char("Non puo' sentirti. Ha perso il senso della realta'.\n\r",
					 ch);
		return;
	}

	if(check_soundproof(vict)) {
		send_to_char("Non riesce a sentire nemmeno la sua voce, li' dentro.\n\r",
					 ch);
		return;
	}

#if ZONE_COMM_ONLY
	if(real_roomp(ch->in_room)->zone !=
			real_roomp(vict->in_room)->zone
			&& GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
		return;
	}
#endif

	std::string tellToVict = "$c0013[$c0015";
	tellToVict += (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch));
	tellToVict += "$c0013] ti dice '";
	tellToVict += scramble(ch, message.data());
	tellToVict += "'";
	act(tellToVict.c_str(), FALSE, vict, nullptr, nullptr, TO_CHAR);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::string tellToChar = "$c0013Tu dici a ";
		tellToChar += (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict));
		tellToChar += " ";
		tellToChar += (IS_AFFECTED2(vict, AFF2_AFK) ? "(che e' AFK) " : "");
		tellToChar += "'";
		tellToChar += message.data();
		tellToChar += "'";
		act(tellToChar.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
	act("$c0013$n dice qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	thief_listen(ch, vict, message.data(), cmd);
}



ACTION_FUNC(do_whisper) {
	struct char_data* vict;
	std::array<char, 100> name{};
	std::array<char, MAX_INPUT_LENGTH> message{};

	if(apply_soundproof(ch)) {
		return;
	}

    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg, name.data(), message.data(), static_cast<int>(name.size()) - 1,
	          static_cast<int>(message.size()) - 1);

	if(name[0] == '\0' || message[0] == '\0') {
		send_to_char("A chi vuoi sussurrare ? e cosa ?\n\r", ch);
	}
	else if(!(vict = get_char_room_vis(ch, name.data()))) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
	}
	else if(vict == ch) {
		act("$n sussurra silenziosamente a se stesso.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		send_to_char("Sembra che le tue orecchie non siano abbastanza vicine "
					 "alla bocca...\n\r", ch);
	}
	else if(IS_LINKDEAD(vict)) {
		send_to_char("Non puo' sentirti. Ha perso il senso della realta'.\n\r",
					 ch);
		return;
	}
	else {
		if(check_soundproof(vict)) {
			return;
		}

		std::string whisper_to_vict = "$c0005[$c0015$n$c0005] ti sussurra '";
		whisper_to_vict += scramble(ch, message.data());
		whisper_to_vict += "'";
		act(whisper_to_vict.c_str(), FALSE, ch, 0, vict, TO_VICT);
		if(IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
			std::string whisper_to_char = "$c0005Tu sussurri a ";
			whisper_to_char += (IS_NPC(vict) ? vict->player.name : GET_NAME(vict));
			if(IS_AFFECTED2(vict, AFF2_AFK)) {
				whisper_to_char += " (che e' AFK)";
			}
			whisper_to_char += ", '";
			whisper_to_char += message.data();
			whisper_to_char += "'";
			act(whisper_to_char.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		act("$c0005$n sussurra qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
		thief_listen(ch, vict, message.data(), cmd);
	}
}


ACTION_FUNC(do_ask) {
	struct char_data* vict;
	std::array<char, 100> name{};
	std::array<char, MAX_INPUT_LENGTH> message{};
	std::array<char, MAX_INPUT_LENGTH> buf{};

	if(apply_soundproof(ch)) {
		return;
	}

    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    {
        send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
        return;
    }

	half_chop(arg, name.data(), message.data(), static_cast<int>(name.size()) - 1,
	          static_cast<int>(message.size()) - 1);

	if(name[0] == '\0' || message[0] == '\0') {
		send_to_char("A chi vuoi chiedere... e cosa ?\n\r", ch);
	}
	else if(!(vict = get_char_room_vis(ch, name.data()))) {
		send_to_char("Non vedi nessuno con quel nome qui...\n\r", ch);
	}
	else if(vict == ch) {
		act("$c0006[$c0015$n$c0006] si chiede qualcosa... trovera' la soluzione?",
			FALSE,ch,0,0,TO_ROOM);
		act("$c0006Oltre alla domanda, conosci anche la risposta?", FALSE, ch, 0,
			0, TO_CHAR);
	}
	else if(IS_LINKDEAD(vict)) {
		send_to_char("Non puo' sentirti. Ha perso il senso della realta'.\n\r",
					 ch);
		return;
	}
	else {
		if(check_soundproof(vict)) {
			return;
		}

		std::string ask_to_vict = "$c0006[$c0015$n$c0006] ti chiede '";
		ask_to_vict += scramble(ch, message.data());
		ask_to_vict += "'";
		act(ask_to_vict.c_str(), FALSE, ch, 0, vict, TO_VICT);

		if(IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
			std::string ask_to_char = "$c0006Tu chiedi a ";
			ask_to_char += (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict));
			if(IS_AFFECTED2(vict, AFF2_AFK)) {
				ask_to_char += " (che e' AFK)";
			}
			ask_to_char += ", '";
			ask_to_char += message.data();
			ask_to_char += "'";
			act(ask_to_char.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		act("$c0006$n fa una domanda a $N.",FALSE,ch,0,vict,TO_NOTVICT);

    /* indizi per le quest */
    if(!IS_PC(vict) && affected_by_spell(ch,STATUS_QUEST) && (isname2("ladro",GET_NAME(vict)) || isname2("cacciatore",GET_NAME(vict)) || isname2("spia",GET_NAME(vict)) || isname2("shop_keeper",mob_index[vict->nr].specname)))    {

        if(ch->specials.quest_ref == nullptr)    {
            std::snprintf(buf.data(), buf.size(), "%s Cio' che cerchi appartiene al passato.",
                          GET_NAME(ch));
            do_tell(vict, buf.data(), CMD_TELL);
            return;
        }

        if(strstr(message.data(), "indizio") != nullptr && IsHumanoid(vict)) {

            if(ch->specials.quest_ref && !(ch->specials.quest_ref = get_char_vis_world(ch, ch->specials.quest_ref->player.name, nullptr))) {
                std::snprintf(buf.data(), buf.size(), "%s Mi spiace, ma non ho informazioni al riguardo...",
                              GET_NAME(ch));
                do_tell(vict, buf.data(), CMD_TELL);
                return;
            }
            else {

                int price = number(250000,300000) - (5000*GET_CHR(ch));

                if(GET_GOLD(ch) >= price) {
                    if(!IS_DIO(ch)) {
                        GET_GOLD(ch) -= price;
                        std::snprintf(buf.data(), buf.size(), "Paghi %d monete a %s per le sue informazioni. \n\r",
                                      price, GET_NAME(vict));
                        send_to_char(buf.data(), ch);
                    }

                    if(real_roomp(ch->in_room)->zone == real_roomp(ch->specials.quest_ref->in_room)->zone) {
                        std::snprintf(buf.data(), buf.size(), "%s %s? Ho sentito che l'ultima volta e' stato vist%s a %s.",
                                      GET_NAME(ch), ch->specials.quest_ref->player.name, SSLF(ch->specials.quest_ref),
                                      real_roomp(ch->specials.quest_ref->in_room)->name);
                    } else {
                        std::snprintf(buf.data(), buf.size(), "%s %s? Pare fosse dirett%s verso %s.",
                                      GET_NAME(ch), ch->specials.quest_ref->player.name, SSLF(ch->specials.quest_ref),
                                      zonename_by_room(ch->specials.quest_ref->in_room));
                    }
                    do_tell(vict, buf.data(), CMD_TELL);
                } else {
                    std::snprintf(buf.data(), buf.size(), "%s ...ma chi credi di comprare con quegli spiccioli!",
                                  GET_NAME(ch));
                    do_tell(vict, buf.data(), CMD_TELL);
                }
                return;
            }

        } else {
            std::snprintf(buf.data(), buf.size(), "%s Se vuoi un indizio chiedimelo chiaramente... ma ti costera'!",
                          GET_NAME(ch));
            do_tell(vict, buf.data(), CMD_TELL);
            return;
        }
     }
    /* end indizi quest*/

	}
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

ACTION_FUNC(do_write) {
	struct obj_data* paper = nullptr;
	struct obj_data* pen = nullptr;
	std::array<char, MAX_INPUT_LENGTH> papername{};
	std::array<char, MAX_INPUT_LENGTH> penname{};
	std::array<char, MAX_STRING_LENGTH> buf{};

	argument_interpreter(arg, papername.data(), penname.data());

	if(!ch->desc) {
		return;
	}

	if(papername[0] == '\0') {  /* nothing was delivered */
		send_to_char("write (on) papername (with) penname.\n\r", ch);
		return;
	}

	if(penname[0] == '\0') {
		send_to_char("write (on) papername (with) penname.\n\r", ch);
		return;
	}
	if(!(paper = get_obj_in_list_vis(ch, papername.data(), ch->carrying))) {
		std::snprintf(buf.data(), buf.size(), "Tu non hai nessun %s.\n\r", papername.data());
		send_to_char(buf.data(), ch);
		return;
	}
	if(!(pen = get_obj_in_list_vis(ch, penname.data(), ch->carrying))) {
		std::snprintf(buf.data(), buf.size(), "Tu non hai nessun %s.\n\r", papername.data());
		send_to_char(buf.data(), ch);
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
		send_to_char("C'e' gia' scritto sopra qualcosa.\n\r", ch);
		return;
	}
	else {
		/* we can write - hooray! */
		send_to_char
		("Ok... puoi scrivere... finisci la nota con un @.\n\r", ch);
		act("$n comincia a scrivere qualcosa.", TRUE, ch, nullptr, nullptr, TO_ROOM);
#if 0
		if(paper->action_description) {
			free(paper->action_description);
		}
		paper->action_description = nullptr;
#endif
		ch->desc->str = &paper->action_description;
		ch->desc->max_str = MAX_NOTE_LENGTH;
	}
}



ACTION_FUNC(do_sign) {
	int i;
	int diff;
	struct char_data* t;
	struct room_data* rp;

	for(i = 0; *(arg + i) == ' '; i++);

	if(!*(arg + i)) {
		send_to_char("D'accordo, ma cosa vuoi dire ?\n\r", ch);
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

		const std::string spokenText = arg + i;
		std::ostringstream correctedStream;
		std::istringstream wordStream(spokenText);
		std::string token;
		bool firstWord = true;
		while(wordStream >> token) {
			if(!firstWord) {
				correctedStream << ' ';
			}
			if(ch->skills && number(1, 75 + static_cast<int>(token.size())) < ch->skills[SKILL_SIGN].learned) {
				correctedStream << token;
			}
			else {
				correctedStream << RandomWord();
			}
			firstWord = false;
		}
		const std::string correctedText = correctedStream.str();
		diff = std::max(1, static_cast<int>(spokenText.size()));
		/*
		  if a recipient fails a roll, a word comes out garbled.
		  */

		/*
		  buf2 is now the "corrected" string.
		  */

		std::string sign_message = "$c0015[$c0005$n$c0015], con i segni, dice '";
		sign_message += correctedText;
		sign_message += "'";

		for(t = rp->people; t; t=t->next_in_room) {
			if(t != ch) {
				if(t->skills && number(1,diff) < t->skills[SKILL_SIGN].learned) {
					act(sign_message.c_str(), FALSE, ch, 0, t, TO_VICT);
				}
				else {
					act("$n muove le mani in modo molto buffo.",
						FALSE, ch, 0, t, TO_VICT);
				}
			}
		}

		if(IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
			std::string echo_message = "Tu hai detto '";
			echo_message += (arg + i);
			echo_message += "'\n\r";
			send_to_char(echo_message.c_str(), ch);
		}
	}
}

/* speak elvish, speak dwarvish, etc...                    */
ACTION_FUNC(do_speak) {
	std::array<char, 255> buf{};
	int i = -1;
	const char* selectedLanguageLabel = nullptr;
	struct SpeakLanguage
	{
		int speakValue;
		const char* commandToken;
		const char* displayLabel;
	};
	constexpr std::array<SpeakLanguage, 8> speakLanguages = {{
		{SPEAK_COMMON, "common", "la lingua comune"},
		{SPEAK_ELVISH, "elvish", "elfico"},
		{SPEAK_HALFLING, "halfling", "la lingua halfling"},
		{SPEAK_DWARVISH, "dwarvish", "nanico"},
		{SPEAK_ORCISH, "orcish", "orchesco"},
		{SPEAK_GIANTISH, "giantish", "la lingua dei giganti"},
		{SPEAK_OGRE, "ogre", "la lingua degli ogre"},
		{SPEAK_GNOMISH, "gnomish", "gnomesco"}
	}};

	only_argument(arg, buf.data());

	if(buf[0] == '\0') {
		send_to_char("In quale lingua vuoi parlare?\n\r",ch);
		return;
	}

	for(const auto& language : speakLanguages) {
		if(strstr(buf.data(), language.commandToken)) {
			i = language.speakValue;
			selectedLanguageLabel = language.displayLabel;
			break;
		}
	}

	if(i == -1) {
		send_to_char("Non e' un linguaggio molto conosciuto.\n\r",ch);
		return;
	}

	/* set language that we're gonna speak */
	ch->player.speaks = i;
	if(selectedLanguageLabel == nullptr) {
		send_to_char("Non e' un linguaggio molto conosciuto.\n\r", ch);
		return;
	}
	std::snprintf(buf.data(), buf.size(), "Ti concentri nel parlare %s.\n\r",
	              selectedLanguageLabel);
	send_to_char(buf.data(), ch);
}

void thief_listen(struct char_data* ch,struct char_data* victim, const char* frase,int cmd) {
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
					std::string overheard = frase;
					for(char& c : overheard) {
						if((percent+number(20,120)+malus)>t->skills[SKILL_TSPY].learned) {
							if((malus+number(1,40)-GET_INT(t))>(0)) {
								if(number(0,20)) {
									PushStatus("Random");
									if(number(1,101)>100 &&
											t->skills[SKILL_TSPY].learned <100) {
										t->skills[SKILL_TSPY].learned++;
									}
									c = RandomChar();
									PopStatus();
								}

							}
						}
					}
					std::vector<char> overheardBuffer(overheard.begin(), overheard.end());
					overheardBuffer.push_back('\0');
					act("$c0013Riesci ad origliare: '$T'", FALSE,
						t, nullptr, overheardBuffer.data(), TO_CHAR);
				}
			}
	}
	PopStatus();

}

/* this is where we do the language says */
ACTION_FUNC(do_new_say) {
	int i, learned, skill_num;
	int diff;
	struct char_data* t;
	struct room_data* rp;

	if(!arg) {
		return;
	}

	for(i = 0; *(arg + i) == ' '; i++);

	if(!arg[i]) {
		send_to_char("Ok, ma cosa hai da dire?\n\r", ch);
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


		const std::string spokenText = arg + i;

		/* we use this for ESP and immortals and comprehend lang */
		std::string say_understood = "$c0015[$c0005$n$c0015] dice '";
		say_understood += spokenText;
		say_understood += "'";

		/*
		  work through the arg, word by word.  if you fail your
		  skill roll, the word comes out garbled.
		  */
		std::ostringstream correctedStream;
		std::istringstream wordStream(spokenText);
		std::string token;
		bool firstWord = true;
		while(wordStream >> token) {
			if(!firstWord) {
				correctedStream << ' ';
			}
			if((number(1, 75 + static_cast<int>(token.size())) < learned) || (GetMaxLevel(ch) >= IMMORTALE)) {
				correctedStream << token;
			}
			else {
				/* add case statement here to use random words from clips of elvish */
				/* dwarvish etc so the words look like they came from that language */
				correctedStream << RandomWord();
			}
			firstWord = false;
		}
		const std::string correctedText = correctedStream.str();
		diff = std::max(1, static_cast<int>(spokenText.size()));
		/*
		  if a recipient fails a roll, a word comes out garbled.
		  */

		/*
		  buf2 is now the "corrected" string.
		  */
		if(correctedText.empty()) {
			send_to_char("OK, ma cosa hai da dire?\n\r", ch);
			return;
		}

		std::string say_garbled = "$c0015[$c0005$n$c0015] dice '";
		say_garbled += correctedText;
		say_garbled += "'";

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
						act(say_understood.c_str(), FALSE, ch, nullptr, t, TO_VICT);
					}
					else
						/* otherwise */

					{
						act(say_garbled.c_str(), FALSE, ch, nullptr, t, TO_VICT);
					}
				}
				else {
					act("$c0010$n parla una lingua che non riesci a capire.", FALSE,
						ch, nullptr, t, TO_VICT);
				}
			}
		}

		if(IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
			std::string say_echo = "$c0015Tu dici '";
			say_echo += (arg + i);
			say_echo += "'";
			act(say_echo.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
	}
}



ACTION_FUNC(do_gtell) {
	int i;
	struct char_data* k;
	struct follow_type* f;

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
		send_to_char("Cosa vuoi dire al gruppo?\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		send_to_char("Forse dovresti unirti ad un gruppo, prima.\n\r", ch);
		return;
	}
	else {
		const char* message = arg + i;
		thief_listen(ch, ch, message, cmd);
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
					std::string groupLine = "$c0012[$c0015";
					groupLine += (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch));
					groupLine += "$c0012] dice al gruppo '";
					groupLine += message;
					groupLine += "'";
					act(groupLine.c_str(), FALSE, f->follower, nullptr, nullptr, TO_CHAR);
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
					std::string groupLine = "$c0012[$c0015";
					groupLine += (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch));
					groupLine += "$c0012] dice al gruppo '";
					groupLine += message;
					groupLine += "'";
					act(groupLine.c_str(), FALSE, ch->master, nullptr, nullptr, TO_CHAR);
				} /* !soundproof */
			}
		}         /* end master send */

		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			std::string selfLine = "$c0012Tu dici al gruppo '";
			selfLine += message;
			selfLine += "'";
			act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		} /* if echo */

	} /* they where grouped... */
} /* end of gtel */

/*
 * 'Split' originally by Gnort, God of Chaos. I stole it from Merc
 * and changed it to work with mine :) Heh msw
 */

ACTION_FUNC(do_split) {

	std::array<char, MAX_INPUT_LENGTH> tmp{};
	struct char_data* gch;
	int members, amount, share, extra;

	const char* rest = one_argument(arg, tmp.data());
	while(*rest == ' ') {
		++rest;
	}

	if(*rest != '\0') {
		send_to_char("Sintassi: split <quantita'>\n\r", ch);
		return;
	}

	if(tmp[0] == '\0') {
		send_to_char("Quanto vuoi dividere?\n\r", ch);
		return;
	}

	char* parseEnd = nullptr;
	errno = 0;
	const long parsedAmount = std::strtol(tmp.data(), &parseEnd, 10);
	if(parseEnd == tmp.data() || *parseEnd != '\0') {
		send_to_char("Inserisci un numero valido.\n\r", ch);
		return;
	}
	if(errno == ERANGE || parsedAmount > std::numeric_limits<int>::max() ||
	   parsedAmount < std::numeric_limits<int>::min()) {
		send_to_char("Numero fuori intervallo.\n\r", ch);
		return;
	}
	amount = static_cast<int>(parsedAmount);

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

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		return;
	}

	members = 0;
	for(gch = rp->people; gch != nullptr; gch = gch->next_in_room) {
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
		send_to_char("C'e' poco da dividere, siete in troppi.\n\r", ch);
		return;
	}

	ch->points.gold -= amount;
	ch->points.gold += share + extra;

	const auto goldAmountPhrase = [](int n) {
		std::string s = std::to_string(n);
		s += (n == 1) ? " moneta d'oro" : " monete d'oro";
		return s;
	};
	const auto coinPartPhrase = [](int n) {
		std::string s = std::to_string(n);
		s += (n == 1) ? " moneta" : " monete";
		return s;
	};

	{
		std::ostringstream selfMsg;
		selfMsg << "Hai diviso " << goldAmountPhrase(amount) << ". La tua parte e' di "
		        << coinPartPhrase(share + extra) << ".\n\r";
		send_to_char(selfMsg.str().c_str(), ch);
	}

	std::string groupAct = "$n divide ";
	groupAct += goldAmountPhrase(amount);
	groupAct += ". La tua parte e' di ";
	groupAct += coinPartPhrase(share);
	groupAct += '.';

	for(gch = rp->people; gch != nullptr; gch = gch->next_in_room) {
		if(gch != ch && is_same_group(gch, ch)) {
			act(groupAct.c_str(), FALSE, ch, nullptr, gch, TO_VICT);
			gch->points.gold += share;
		}
	}

	return;
}


ACTION_FUNC(do_pray) {
	struct affected_type af;
	std::array<char, MAX_INPUT_LENGTH> godName{};
	struct descriptor_data* i;
	int ii = 0;
	int durata = 6;
	if(!IS_PC(ch)) {
		return;
	}

	if(affected_by_spell(ch, SPELL_PRAYER)) {
		int ore_rimanenti = 0;
		for(struct affected_type* aff = ch->affected; aff != nullptr;
		    aff = aff->next) {
			if(aff->type == SPELL_PRAYER) {
				ore_rimanenti = aff->duration;
				break;
			}
		}
		std::ostringstream msg;
		msg << "Hai gia' pregato di recente.";
		if(ore_rimanenti > 0) {
			msg << " Potrai pregare di nuovo tra circa " << ore_rimanenti
			    << " ore di gioco.";
		}
		msg << "\n\r";
		send_to_char(msg.str().c_str(), ch);
		return;
	}

	for(; *arg == ' '; arg++);
	if(!(*arg)) {
		send_to_char("Vuoi pregare. Ottimo, ma chi? "
					 "(pray <NomeDio> <preghiera>)\n\r", ch);
	}
	else {
		ii = (GetMaxLevel(ch) * 1.5  + 20);
		one_argument(arg, godName.data());
		mudlog(LOG_CHECK, "%s ha pregato %s", GET_NAME(ch), godName.data());
		const bool isDevoutClass = HasClass(ch, CLASS_CLERIC | CLASS_DRUID);
		if(isDevoutClass) {
			ii +=10;    /* clerics get a 10% bonus :) */
		}

		if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
			std::string selfPrayer = "Tu preghi '";
			selfPrayer += arg;
			selfPrayer += "'\n\r";
			send_to_char(selfPrayer.c_str(), ch);
		}

		const char* roomPrayerEmote = nullptr;
		switch(number(1, 6)) {
		case 1:
			roomPrayerEmote = isDevoutClass
			                  ? "$n china il capo e mormora una preghiera sottovoce."
			                  : "$n resta in silenzio, raccogliendosi per un istante.";
			break;
		case 2:
			roomPrayerEmote = isDevoutClass
			                  ? "Un fremito percorre l'aria mentre $n invoca il suo dio."
			                  : "$n mormora poche parole, con tono misurato.";
			break;
		case 3:
			roomPrayerEmote = isDevoutClass
			                  ? "$n intreccia le mani e sussurra parole antiche."
			                  : "$n abbassa lo sguardo e riflette in silenzio.";
			break;
		case 4:
			roomPrayerEmote = isDevoutClass
			                  ? "Per un istante, attorno a $n cala un silenzio solenne."
			                  : "$n chiude gli occhi per un breve momento.";
			break;
		case 5:
			roomPrayerEmote = isDevoutClass
			                  ? "$n alza lo sguardo al cielo e prega con fervore."
			                  : "$n sussurra una breve invocazione, senza enfasi.";
			break;
		default:
			roomPrayerEmote = isDevoutClass
			                  ? "Una tenue aura di devozione sembra avvolgere $n."
			                  : "$n si concentra un attimo, poi torna composto.";
			break;
		}
		act(roomPrayerEmote, FALSE, ch, nullptr, nullptr, TO_ROOM);

		if(ch->desc != nullptr && ch->desc->AccountData.authorized) {
			ii += 60;
		}
		if(ii > number(1, 101)) {

			for(i = descriptor_list; i; i = i->next) {
				if(i->character != ch && ! i->connected && ! IS_NPC(i->character) &&
						!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
						(GetMaxLevel(i->character) >= IMMORTALE)) {
					std::string prayerLine;
					if(!str_cmp2(godName.data(), GET_NAME(i->character))) {
						prayerLine = "$c0013[$c0015$n$c0013] TI PREGA: '";
					}
					else {
						prayerLine = "$c0014[$c0015$n$c0014] prega :'";
					}
					prayerLine += arg;
					prayerLine += "'";
					act(prayerLine.c_str(), 0, ch, nullptr, i->character, TO_VICT);
				}

			} /* end for */
			durata = 12;
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
	if(ach->master != nullptr) {
		ach = ach->master;
	}
	if(bch->master != nullptr) {
		bch = bch->master;
	}
	return(ach == bch);
}


ACTION_FUNC(do_telepathy) {
	struct char_data* vict;
	char name[100], message[MAX_INPUT_LENGTH+20];

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
		send_to_char("A chi vuoi mandare il tuo pensiero?\n\r", ch);
		return;
	}
	else if(!(vict = get_char_vis(ch, name))) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
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
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
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
		act("La mente di $N e' chiusa in questo momento!", FALSE, ch, 0, vict,
			TO_CHAR);
		return;
	}
	else if(!vict->desc) {
		send_to_char("Non puo' sentirti. Gli e' caduta la linea (link dead).\n\r",
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

	{
		std::string telepathy_to_vict = "$c0013[$c0015";
		telepathy_to_vict += (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch));
		telepathy_to_vict += "$c0013] ti manda il pensiero '";
		telepathy_to_vict += message;
		telepathy_to_vict += "'";
		act(telepathy_to_vict.c_str(), FALSE, vict, nullptr, nullptr, TO_CHAR);
	}

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::string telepathy_echo = "$c0013Tu mandi a ";
		telepathy_echo += (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict));
		telepathy_echo += " il pensiero '";
		telepathy_echo += message;
		telepathy_echo += "'";
		act(telepathy_echo.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
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
	if(exitp && exitp->to_room > 0 && real_roomp(exitp->to_room)) {
		if(IS_SET(exitp->exit_info, EX_CLOSED) && exitp->keyword) {
			snprintf(buf, MAX_STRING_LENGTH-1, "The %s is closed.\r\n", fname(exitp->keyword));
			send_to_char(buf, ch);
		}
		else {
			ch->next_listener = real_roomp(exitp->to_room)->listeners;
			real_roomp(exitp->to_room)->listeners = ch;
			ch->listening_to = exitp->to_room;
			send_to_char("Resti immobile concentrandoti sui rumori che provengono da quella parte.", ch);
			WAIT_STATE(ch, PULSE_VIOLENCE); // eavesdrop
		}
	}
	else {
		send_to_char("Non c'e' nulla da quella parte...\r\n", ch);
	}
}


} // namespace Alarmud
