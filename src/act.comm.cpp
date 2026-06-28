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
#include "procarea.hpp"
#include "procarea_rune_fragments.hpp"
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

void append_report_hp_mana_mv_pct(std::ostringstream& os, double hitPct, double manaPct,
                                  double movePct) {
	os << std::setw(2) << std::fixed << std::setprecision(0) << hitPct
	   << "% MANA:" << std::setw(2) << manaPct
	   << "% MV:" << std::setw(2) << movePct << "%'";
}

/** Seconda riga quando Silence blocca un NPC in polimorfo su canali pubblici. */
constexpr char kPolySilenceFollowup[] =
	"La limitazione e' temporanea e dipende dalla gestione del gioco.\n\r";

/** @return true se bloccato (messaggi gia' inviati a ch). */
bool comm_poly_silence_blocks(struct char_data* ch, const char* channelBlockedMsg) {
	if(!IS_NPC(ch) || Silence != 1 || !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		return false;
	}
	send_to_char(channelBlockedMsg, ch);
	send_to_char(kPolySilenceFollowup, ch);
	return true;
}

/** Charm: schiavo non usa il canale se il master non e' immortale. @return true se bloccato. */
bool comm_charm_master_blocks(struct char_data* ch) {
	if(ch->master == nullptr || !IS_AFFECTED(ch, AFF_CHARM)) {
		return false;
	}
	if(IS_IMMORTAL(ch->master)) {
		return false;
	}
	send_to_char("Non credo proprio :-)", ch->master);
	return true;
}

const char* comm_display_name(struct char_data* who) {
	if(who == nullptr) {
		return "?";
	}
	if(IS_NPC(who)) {
		if(who->player.short_descr != nullptr && who->player.short_descr[0] != '\0') {
			return who->player.short_descr;
		}
		if(GET_NAME(who) != nullptr) {
			return GET_NAME(who);
		}
		return "?";
	}
	return (GET_NAME(who) != nullptr) ? GET_NAME(who) : "?";
}

/** @return true se il bersaglio ha PLR_NOTELL e il mittente non puo' forzare il tell. */
bool tell_vict_refuses(struct char_data* ch, struct char_data* vict) {
	if(IS_NPC(vict) || !IS_SET(vict->specials.act, PLR_NOTELL)) {
		return false;
	}
	if(GetMaxLevel(ch) < IMMORTALE) {
		return true;
	}
	return GetMaxLevel(vict) >= IMMORTALE && GetMaxLevel(ch) < GetMaxLevel(vict);
}

enum class CommDirectScope { VisWorld, RoomOnly };

enum class CommSelfPolicy { TellReject, Whisper, Ask };

struct CommDirectRules {
	CommDirectScope scope = CommDirectScope::VisWorld;
	CommSelfPolicy self_policy = CommSelfPolicy::TellReject;
	bool check_sleeping = false;
	bool check_notell = false;
	bool zone_comm_only = false;
	const char* prompt_empty = nullptr;
	const char* msg_not_found = nullptr;
	/** Se nullptr, soundproof sul bersaglio termina senza messaggio al mittente. */
	const char* msg_vict_soundproof = nullptr;
};

struct CommDirectFormat {
	const char* color_tag = nullptr;
	bool embedded_sender = false;
	const char* victim_phrase = nullptr;
	const char* echo_verb = nullptr;
	bool echo_comma_before_quote = false;
	const char* room_observers = nullptr;
};

struct CommDirectCtx {
	struct char_data* vict = nullptr;
	std::array<char, MAX_INPUT_LENGTH + 20> message{};
};

enum class CommPrepareResult { Ready, Done };

bool comm_direct_sender_ok(struct char_data* ch) {
	if(apply_soundproof(ch)) {
		return false;
	}
	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
		return false;
	}
	return true;
}

bool comm_direct_handle_self(struct char_data* ch, CommSelfPolicy policy) {
	switch(policy) {
	case CommSelfPolicy::TellReject:
		send_to_char("Parlare a se stessi puo' essere utile, a volte...\n\r", ch);
		return true;
	case CommSelfPolicy::Whisper:
		act("$n sussurra silenziosamente a se stesso.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		send_to_char("Sembra che le tue orecchie non siano abbastanza vicine alla bocca...\n\r", ch);
		return true;
	case CommSelfPolicy::Ask:
		act("$c0006[$c0015$n$c0006] si chiede qualcosa... trovera' la soluzione?", FALSE, ch, nullptr,
		    nullptr, TO_ROOM);
		act("$c0006Oltre alla domanda, conosci anche la risposta?", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return true;
	}
	return false;
}

CommPrepareResult comm_direct_prepare(struct char_data* ch, const char* arg, const CommDirectRules& rules,
                                      CommDirectCtx& ctx) {
	ctx.vict = nullptr;
	ctx.message.fill('\0');

	const auto [nameStr, messageStr] =
	    chop_argument(arg, 99, static_cast<std::size_t>(ctx.message.size()) - 1);

	if(nameStr.empty() || messageStr.empty()) {
		if(rules.prompt_empty != nullptr) {
			send_to_char(rules.prompt_empty, ch);
		}
		return CommPrepareResult::Done;
	}

	ctx.message.fill('\0');
	std::strncpy(ctx.message.data(), messageStr.c_str(), ctx.message.size() - 1);

	if(rules.scope == CommDirectScope::VisWorld) {
		ctx.vict = get_char_vis(ch, nameStr.c_str());
	}
	else {
		ctx.vict = get_char_room_vis(ch, nameStr.c_str());
	}

	if(ctx.vict == nullptr) {
		if(rules.msg_not_found != nullptr) {
			send_to_char(rules.msg_not_found, ch);
		}
		return CommPrepareResult::Done;
	}

	if(ctx.vict == ch) {
		comm_direct_handle_self(ch, rules.self_policy);
		return CommPrepareResult::Done;
	}

	if(rules.check_sleeping && GET_POS(ctx.vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
		act("Sta dormendo, shhh.", FALSE, ch, nullptr, ctx.vict, TO_CHAR);
		return CommPrepareResult::Done;
	}

	if(rules.check_notell && tell_vict_refuses(ch, ctx.vict)) {
		act("$N non sta ascoltando adesso.", FALSE, ch, nullptr, ctx.vict, TO_CHAR);
		return CommPrepareResult::Done;
	}

	if(IS_LINKDEAD(ctx.vict)) {
		send_to_char("Non puo' sentirti. Ha perso il senso della realta'.\n\r", ch);
		return CommPrepareResult::Done;
	}

	if(check_soundproof(ctx.vict)) {
		if(rules.msg_vict_soundproof != nullptr) {
			send_to_char(rules.msg_vict_soundproof, ch);
		}
		return CommPrepareResult::Done;
	}

#if ZONE_COMM_ONLY
	if(rules.zone_comm_only) {
		const struct room_data* const chRoom = real_roomp(ch->in_room);
		const struct room_data* const victRoom = real_roomp(ctx.vict->in_room);
		if(chRoom == nullptr || victRoom == nullptr) {
			mudlog(LOG_SYSERR, "comm_direct_prepare: stanza nulla per %s o bersaglio", GET_NAME(ch));
			return CommPrepareResult::Done;
		}
		if(chRoom->zone != victRoom->zone && GetMaxLevel(ch) < IMMORTALE) {
			send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
			return CommPrepareResult::Done;
		}
	}
#endif

	return CommPrepareResult::Ready;
}

void comm_direct_deliver(struct char_data* ch, struct char_data* vict, const char* message, int cmd,
                         const CommDirectFormat& fmt) {
	if(fmt.embedded_sender) {
		std::ostringstream toVictOs;
		toVictOs << fmt.color_tag << "[$c0015" << comm_display_name(ch) << fmt.color_tag << "] "
		         << fmt.victim_phrase << " '" << message << "'";
		act(toVictOs.str().c_str(), FALSE, vict, nullptr, nullptr, TO_CHAR);
	}
	else {
		std::ostringstream toVictOs;
		toVictOs << fmt.color_tag << "[$c0015$n" << fmt.color_tag << "] " << fmt.victim_phrase << " '"
		         << message << "'";
		act(toVictOs.str().c_str(), FALSE, ch, nullptr, vict, TO_VICT);
	}

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream echoOs;
		echoOs << fmt.color_tag << fmt.echo_verb << comm_display_name(vict);
		if(IS_AFFECTED2(vict, AFF2_AFK)) {
			echoOs << " (che e' AFK)";
		}
		if(fmt.echo_comma_before_quote) {
			echoOs << ", '";
		}
		else {
			echoOs << " '";
		}
		echoOs << message << "'";
		act(echoOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}

	if(fmt.room_observers != nullptr) {
		act(fmt.room_observers, FALSE, ch, nullptr, vict, TO_NOTVICT);
	}
	thief_listen(ch, vict, message, cmd);
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
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_say (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_say (act.comm.cpp)");
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
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_report (act.comm.cpp)");
		return;
	}
	if(apply_soundproof(ch)) {
		return;
	}
	if(IS_NPC(ch)) {
		return;
	}

	if(GET_HIT(ch) > GET_MAX_HIT(ch) || GET_MANA(ch) > GET_MAX_MANA(ch) ||
	   GET_MOVE(ch) > GET_MAX_MOVE(ch)) {
		send_to_char("Mi spiace, ma non puoi farlo ora.\n\r", ch);
		return;
	}

	const int maxHit = GET_MAX_HIT(ch);
	const int maxMana = GET_MAX_MANA(ch);
	const int maxMove = GET_MAX_MOVE(ch);
	if(maxHit <= 0 || maxMana <= 0 || maxMove <= 0) {
		send_to_char("Mi spiace, ma non puoi farlo ora.\n\r", ch);
		return;
	}

	const double hitPct =
	    (static_cast<double>(GET_HIT(ch)) / static_cast<double>(maxHit)) * 100.0 + 0.5;
	const double manaPct =
	    (static_cast<double>(GET_MANA(ch)) / static_cast<double>(maxMana)) * 100.0 + 0.5;
	const double movePct =
	    (static_cast<double>(GET_MOVE(ch)) / static_cast<double>(maxMove)) * 100.0 + 0.5;

	std::ostringstream roomReport;
	roomReport << "$c0014[$c0015$n$c0014] dichiara 'HP:";
	append_report_hp_mana_mv_pct(roomReport, hitPct, manaPct, movePct);
	act(roomReport.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);

	std::ostringstream selfReport;
	selfReport << "$c0014Dichiari 'HP:";
	append_report_hp_mana_mv_pct(selfReport, hitPct, manaPct, movePct);
	act(selfReport.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
}



ACTION_FUNC(do_shout) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_shout (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_shout (act.comm.cpp)");
		return;
	}

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi urlare!!\n\r", ch);
		return;
	}

	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi urlare in queste condizioni.\n\r", ch);
		return;
	}

	if(comm_poly_silence_blocks(
	       ch,
	       "Urlare mentre sei in forma polimorfa non e' consentito al momento.\n\r")) {
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	const char* speech = arg;
	while(*speech == ' ') {
		speech++;
	}

	if(comm_charm_master_blocks(ch)) {
		return;
	}

	if((GET_MOVE(ch) < 10 || GET_MANA(ch) < 10) && GetMaxLevel(ch) < IMMORTALE) {
		send_to_char("Non hai abbastanza forza per gridare!\n\r", ch);
		return;
	}

	if(*speech == '\0') {
		send_to_char("Vuoi urlare? Ottimo! Ma COSA?\n\r", ch);
		return;
	}

	std::string shoutLine;
	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::string selfLine = "$c0009Tu gridi '";
		selfLine += speech;
		selfLine += "'";
		act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
	shoutLine = "$c0009[$c0015$n$c0009] grida '";
	shoutLine += scramble(ch, speech);
	shoutLine += "'";

	act("$c0009[$c0015$n$c0009] alza la testa e grida forte", FALSE, ch, nullptr, nullptr,
	    TO_ROOM);

	if(GetMaxLevel(ch) < IMMORTALE) {
		GET_MOVE(ch) -= 10;
		alter_move(ch, 0);
		GET_MANA(ch) -= 10;
		alter_mana(ch, 0);
	}

	for(struct descriptor_data* i = descriptor_list; i != nullptr; i = i->next) {
		if(i->character == nullptr || i->character == ch || i->connected != 0) {
			continue;
		}
		if(!(IS_NPC(i->character) ||
		     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
		      !IS_SET(i->character->specials.act, PLR_DEAF)))) {
			continue;
		}
		if(check_soundproof(i->character)) {
			continue;
		}
		act(shoutLine.c_str(), FALSE, ch, nullptr, i->character, TO_VICT);
	}
}


ACTION_FUNC(do_gossip) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_gossip (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_gossip (act.comm.cpp)");
		return;
	}

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi parlare!\n\r", ch);
		return;
	}

	if(comm_poly_silence_blocks(
	       ch,
	       "Usare il gossip in forma polimorfa non e' consentito al momento.\n\r")) {
		return;
	}

	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	const struct room_data* const chRoom = real_roomp(ch->in_room);
	if(chRoom == nullptr) {
		mudlog(LOG_SYSERR, "do_gossip: real_roomp(ch->in_room) nullo per %s", GET_NAME(ch));
		return;
	}

	const char* speech = arg;
	while(*speech == ' ') {
		speech++;
	}

	if(comm_charm_master_blocks(ch)) {
		return;
	}

	if(*speech == '\0') {
		send_to_char("Parlare? Ma di COSA!\n\r", ch);
		return;
	}

	std::string gossipLine;
	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::string selfLine = "$c0011Tu dici '";
		selfLine += speech;
		selfLine += "'";
		act(selfLine.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
	gossipLine = "$c0011[$c0015$n$c0011] vi dice '";
	gossipLine += scramble(ch, speech);
	gossipLine += "'";

	for(struct descriptor_data* i = descriptor_list; i != nullptr; i = i->next) {
		if(i->character == nullptr || i->character == ch || i->connected != 0) {
			continue;
		}
		if(!(IS_NPC(i->character) ||
		     !IS_SET(i->character->specials.act, PLR_NOGOSSIP))) {
			continue;
		}
		if(check_soundproof(i->character)) {
			continue;
		}
		if(i->character->in_room == NOWHERE) {
			continue;
		}
		const struct room_data* const victRoom = real_roomp(i->character->in_room);
		if(victRoom == nullptr) {
			continue;
		}
		if(chRoom->zone != victRoom->zone && GetMaxLevel(i->character) < IMMORTALE &&
		   GetMaxLevel(ch) < IMMORTALE) {
			continue;
		}
		act(gossipLine.c_str(), FALSE, ch, nullptr, i->character, TO_VICT);
	}
}


ACTION_FUNC(do_auction) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_auction (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_auction (act.comm.cpp)");
		return;
	}

	if(!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("Non puoi annunciare all'asta!\n\r", ch);
		return;
	}

	if(comm_poly_silence_blocks(
	       ch,
	       "Annunciare all'asta in forma polimorfa non e' consentito al momento.\n\r")) {
		return;
	}

	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi farlo.\n\r", ch);
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	for(; *arg == ' '; arg++) {
	}

	if(comm_charm_master_blocks(ch)) {
		return;
	}
	do_auction_int(ch, arg, cmd);
}

void talk_auction(const char* arg) {
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in talk_auction (act.comm.cpp)");
		return;
	}
	if(auction == nullptr) {
		mudlog(LOG_SYSERR, "auction==nullptr in talk_auction (act.comm.cpp)");
		return;
	}

	std::string auctionLine = "$c0010[$c0015Il Banditore$c0010] '";
	auctionLine += arg;
	auctionLine += "'";

	for(struct descriptor_data* i = descriptor_list; i != nullptr; i = i->next) {
		if(i->character == nullptr || i->connected != 0) {
			continue;
		}
		if(!(IS_NPC(i->character) ||
		     !IS_SET(i->character->specials.act, PLR_NOGOSSIP))) {
			continue;
		}
		if(check_soundproof(i->character)) {
			continue;
		}
		if(i->character->in_room == NOWHERE) {
			continue;
		}
		/* SALVO: senza venditore, $n deve comunque risolversi per il messaggio */
		struct char_data* const actor =
		    auction->seller != nullptr ? auction->seller : i->character;
		act(auctionLine.c_str(), FALSE, actor, nullptr, i->character, TO_VICT);
	}
}





ACTION_FUNC(do_commune) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_commune (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_commune (act.comm.cpp)");
		return;
	}

	const int minLevel = (cmd == CMD_THINK_SUPERNI) ? CREATORE : IMMORTALE;
	const char* const sep = (cmd == CMD_THINK_SUPERNI) ? "||" : "::";

	while(*arg == ' ') {
		arg++;
	}

	if(*arg == '\0') {
		send_to_char("Comunicare fra gli Dei e' ottimo, ma COSA?\n\r", ch);
		return;
	}

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream selfOs;
		selfOs << "$c0014Tu pensi " << sep << " '" << arg << "'";
		act(selfOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}

	std::ostringstream communeOs;
	communeOs << "$c0014" << sep << "$c0015$n$c0014" << sep << " '" << arg << "'";
	const std::string communeLine = communeOs.str();

	for(struct descriptor_data* i = descriptor_list; i != nullptr; i = i->next) {
		if(i->character == nullptr || i->character == ch || i->connected != 0) {
			continue;
		}
		if(IS_NPC(i->character)) {
			continue;
		}
		if(IS_SET(i->character->specials.act, PLR_NOSHOUT)) {
			continue;
		}
		if(GetMaxLevel(i->character) < minLevel) {
			continue;
		}
		act(communeLine.c_str(), FALSE, ch, nullptr, i->character, TO_VICT);
	}
}


ACTION_FUNC(do_tell) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_tell (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_tell (act.comm.cpp)");
		return;
	}
	if(!comm_direct_sender_ok(ch)) {
		return;
	}

	static const CommDirectRules kTellRules{
	    CommDirectScope::VisWorld, CommSelfPolicy::TellReject, true,  true,  true,
	    "A chi e' che vuoi parlare?\n\r",
	    "Non c'e' nessuno con quel nome qui...\n\r",
	    "Non riesce a sentire nemmeno la sua voce, li' dentro.\n\r",
	};
	static const CommDirectFormat kTellFmt{
	    "$c0013", true, "ti dice", "Tu dici a ", false, "$c0013$n dice qualcosa a $N.",
	};

	CommDirectCtx ctx;
	if(comm_direct_prepare(ch, arg, kTellRules, ctx) != CommPrepareResult::Ready) {
		return;
	}
	comm_direct_deliver(ch, ctx.vict, ctx.message.data(), cmd, kTellFmt);
}



ACTION_FUNC(do_whisper) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_whisper (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_whisper (act.comm.cpp)");
		return;
	}
	if(!comm_direct_sender_ok(ch)) {
		return;
	}

	static const CommDirectRules kWhisperRules{
	    CommDirectScope::RoomOnly, CommSelfPolicy::Whisper, false, false, false,
	    "A chi vuoi sussurrare? E cosa?\n\r",
	    "Non c'e' nessuno con quel nome qui...\n\r",
	    nullptr,
	};
	static const CommDirectFormat kWhisperFmt{
	    "$c0005", false, "ti sussurra", "Tu sussurri a ", true, "$c0005$n sussurra qualcosa a $N.",
	};

	CommDirectCtx ctx;
	if(comm_direct_prepare(ch, arg, kWhisperRules, ctx) != CommPrepareResult::Ready) {
		return;
	}
	comm_direct_deliver(ch, ctx.vict, ctx.message.data(), cmd, kWhisperFmt);
}

namespace {
bool ask_quest_try_hint_response(struct char_data* ch, struct char_data* vict, const char* message);
} // namespace

ACTION_FUNC(do_ask) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_ask (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_ask (act.comm.cpp)");
		return;
	}
	if(!comm_direct_sender_ok(ch)) {
		return;
	}

	static const CommDirectRules kAskRules{
	    CommDirectScope::RoomOnly, CommSelfPolicy::Ask, false, false, false,
	    "A chi vuoi chiedere... e cosa?\n\r",
	    "Non vedi nessuno con quel nome qui...\n\r",
	    nullptr,
	};
	static const CommDirectFormat kAskFmt{
	    "$c0006", false, "ti chiede", "Tu chiedi a ", true, "$c0006$n fa una domanda a $N.",
	};

	CommDirectCtx ctx;
	if(comm_direct_prepare(ch, arg, kAskRules, ctx) != CommPrepareResult::Ready) {
		return;
	}
	comm_direct_deliver(ch, ctx.vict, ctx.message.data(), cmd, kAskFmt);
	ask_quest_try_hint_response(ch, ctx.vict, ctx.message.data());
}



namespace {
constexpr int kMaxNoteLength = 1000;

std::string sign_build_corrected(struct char_data* ch, const std::string& spokenText) {
	std::ostringstream correctedStream;
	std::istringstream wordStream(spokenText);
	std::string token;
	bool firstWord = true;
	while(wordStream >> token) {
		if(!firstWord) {
			correctedStream << ' ';
		}
		const int learned = (ch->skills != nullptr) ? ch->skills[SKILL_SIGN].learned : 0;
		if(ch->skills != nullptr &&
		   number(1, 75 + static_cast<int>(token.size())) < learned) {
			correctedStream << token;
		}
		else {
			correctedStream << RandomWord();
		}
		firstWord = false;
	}
	return correctedStream.str();
}

/** Match esatto o abbreviazione (es. elv -> elvish), senza strstr su sottostringhe spurie. */
bool speak_language_matches(const char* userTok, const char* languageToken) {
	if(userTok == nullptr || languageToken == nullptr || userTok[0] == '\0') {
		return false;
	}
	if(str_cmp(userTok, languageToken) == 0) {
		return true;
	}
	return is_abbrev(userTok, languageToken) != 0;
}

void sign_broadcast_room(struct char_data* ch, struct room_data* rp, const std::string& spokenText,
                         const std::string& correctedText) {
	const int diff = std::max(1, static_cast<int>(spokenText.size()));
	std::ostringstream signOs;
	signOs << "$c0015[$c0005$n$c0015], con i segni, dice '" << correctedText << "'";
	const std::string signMessage = signOs.str();

	for(struct char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
		if(t == ch) {
			continue;
		}
		const int learned = (t->skills != nullptr) ? t->skills[SKILL_SIGN].learned : 0;
		if(t->skills != nullptr && number(1, diff) < learned) {
			act(signMessage.c_str(), FALSE, ch, nullptr, t, TO_VICT);
		}
		else {
			act("$n muove le mani in modo molto buffo.", FALSE, ch, nullptr, t, TO_VICT);
		}
	}
}

int thief_listen_malus(struct char_data* ch, int cmd) {
	int malus = 0;
	if(cmd == CMD_WHISPER) {
		malus = 5;
	}
	else if(cmd == CMD_GTELL) {
		malus = 10;
	}
	if(!IS_SINGLE(ch)) {
		malus += 20;
	}
	return malus;
}

bool thief_listener_eligible(struct char_data* listener, struct char_data* speaker,
                           struct char_data* victim) {
	if(listener == speaker || listener == victim) {
		return false;
	}
	if(!HasClass(listener, CLASS_THIEF)) {
		return false;
	}
	if(listener->skills == nullptr || listener->skills[SKILL_TSPY].learned <= 0) {
		return false;
	}
	return affected_by_spell(listener, SKILL_TSPY);
}

void say_resolve_language(struct char_data* ch, int& learned, int& skillNum) {
	learned = 0;
	skillNum = LANG_COMMON;
	if(ch->skills == nullptr) {
		return;
	}
	switch(ch->player.speaks) {
	case SPEAK_COMMON:
		learned = ch->skills[LANG_COMMON].learned;
		skillNum = LANG_COMMON;
		break;
	case SPEAK_ELVISH:
		learned = ch->skills[LANG_ELVISH].learned;
		skillNum = LANG_ELVISH;
		break;
	case SPEAK_HALFLING:
		learned = ch->skills[LANG_HALFLING].learned;
		skillNum = LANG_HALFLING;
		break;
	case SPEAK_DWARVISH:
		learned = ch->skills[LANG_DWARVISH].learned;
		skillNum = LANG_DWARVISH;
		break;
	case SPEAK_ORCISH:
		learned = ch->skills[LANG_ORCISH].learned;
		skillNum = LANG_ORCISH;
		break;
	case SPEAK_GIANTISH:
		learned = ch->skills[LANG_GIANTISH].learned;
		skillNum = LANG_GIANTISH;
		break;
	case SPEAK_OGRE:
		learned = ch->skills[LANG_OGRE].learned;
		skillNum = LANG_OGRE;
		break;
	case SPEAK_GNOMISH:
		learned = ch->skills[LANG_GNOMISH].learned;
		skillNum = LANG_GNOMISH;
		break;
	default:
		learned = ch->skills[LANG_COMMON].learned;
		skillNum = LANG_COMMON;
		break;
	}
}

std::string say_build_corrected(struct char_data* ch, const std::string& spokenText, int learned) {
	std::ostringstream correctedStream;
	std::istringstream wordStream(spokenText);
	std::string token;
	bool firstWord = true;
	while(wordStream >> token) {
		if(!firstWord) {
			correctedStream << ' ';
		}
		if(number(1, 75 + static_cast<int>(token.size())) < learned || GetMaxLevel(ch) >= IMMORTALE) {
			correctedStream << token;
		}
		else {
			correctedStream << RandomWord();
		}
		firstWord = false;
	}
	return correctedStream.str();
}

bool say_listener_hears_language(struct char_data* listener, struct char_data* speaker, int skillNum,
                               int diff) {
	if(GetMaxLevel(listener) >= IMMORTALE || GetMaxLevel(speaker) >= IMMORTALE || IS_NPC(listener)) {
		return true;
	}
	if(affected_by_spell(listener, SKILL_ESP) || affected_by_spell(listener, SPELL_COMP_LANGUAGES)) {
		return true;
	}
	return listener->skills != nullptr && number(1, diff) < listener->skills[skillNum].learned;
}

bool say_listener_hears_clear(struct char_data* listener) {
	return GetMaxLevel(listener) >= IMMORTALE || IS_NPC(listener) ||
	       affected_by_spell(listener, SKILL_ESP) ||
	       affected_by_spell(listener, SPELL_COMP_LANGUAGES);
}

void say_broadcast_room(struct char_data* ch, struct room_data* rp, const std::string& spokenText,
                        const std::string& correctedText, int skillNum) {
	const int diff = std::max(1, static_cast<int>(spokenText.size()));
	std::ostringstream understoodOs;
	understoodOs << "$c0015[$c0005$n$c0015] dice '" << spokenText << "'";
	const std::string sayUnderstood = understoodOs.str();

	std::ostringstream garbledOs;
	garbledOs << "$c0015[$c0005$n$c0015] dice '" << correctedText << "'";
	const std::string sayGarbled = garbledOs.str();

	for(struct char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
		if(t == ch) {
			continue;
		}
		if(!say_listener_hears_language(t, ch, skillNum, diff)) {
			act("$c0010$n parla una lingua che non riesci a capire.", FALSE, ch, nullptr, t, TO_VICT);
			continue;
		}
		if(say_listener_hears_clear(t)) {
			act(sayUnderstood.c_str(), FALSE, ch, nullptr, t, TO_VICT);
		}
		else {
			act(sayGarbled.c_str(), FALSE, ch, nullptr, t, TO_VICT);
		}
	}
}

std::string gtell_format_line(struct char_data* speaker, const char* message) {
	std::ostringstream os;
	os << "$c0012[$c0015" << comm_display_name(speaker) << "$c0012] dice al gruppo '" << message
	   << "'";
	return os.str();
}

void gtell_deliver(struct char_data* recipient, struct char_data* speaker, const std::string& line) {
	if(recipient == nullptr || recipient == speaker || recipient->desc == nullptr) {
		return;
	}
	if(check_soundproof(recipient)) {
		return;
	}
	act(line.c_str(), FALSE, recipient, nullptr, nullptr, TO_CHAR);
}

void gtell_broadcast(struct char_data* ch, const char* message) {
	struct char_data* const leader = ch->master != nullptr ? ch->master : ch;
	const std::string line = gtell_format_line(ch, message);

	for(struct follow_type* f = leader->followers; f != nullptr; f = f->next) {
		if(f->follower == nullptr || !IS_AFFECTED(f->follower, AFF_GROUP)) {
			continue;
		}
		gtell_deliver(f->follower, ch, line);
	}

	if(ch->master != nullptr && IS_AFFECTED(ch->master, AFF_GROUP)) {
		gtell_deliver(ch->master, ch, line);
	}
}

std::string split_gold_amount_phrase(int amount) {
	std::string s = std::to_string(amount);
	s += (amount == 1) ? " moneta d'oro" : " monete d'oro";
	return s;
}

std::string split_coin_part_phrase(int amount) {
	std::string s = std::to_string(amount);
	s += (amount == 1) ? " moneta" : " monete";
	return s;
}

int split_count_group_in_room(struct room_data* rp, struct char_data* ch) {
	int members = 0;
	for(struct char_data* gch = rp->people; gch != nullptr; gch = gch->next_in_room) {
		if(is_same_group(gch, ch)) {
			members++;
		}
	}
	return members;
}

bool split_parse_amount(const char* token, int& amountOut) {
	if(token == nullptr || token[0] == '\0') {
		return false;
	}
	char* parseEnd = nullptr;
	errno = 0;
	const long parsed = std::strtol(token, &parseEnd, 10);
	if(parseEnd == token || (parseEnd != nullptr && *parseEnd != '\0')) {
		return false;
	}
	if(errno == ERANGE || parsed > std::numeric_limits<int>::max() ||
	   parsed < std::numeric_limits<int>::min()) {
		return false;
	}
	amountOut = static_cast<int>(parsed);
	return true;
}

int pray_spell_duration_remaining(struct char_data* ch) {
	if(ch == nullptr) {
		return 0;
	}
	for(struct affected_type* aff = ch->affected; aff != nullptr; aff = aff->next) {
		if(aff->type == SPELL_PRAYER) {
			return aff->duration;
		}
	}
	return 0;
}

void pray_send_cooldown_message(struct char_data* ch, int hoursRemaining) {
	std::ostringstream msg;
	msg << "Hai gia' pregato di recente.";
	if(hoursRemaining > 0) {
		msg << " Potrai pregare di nuovo tra circa " << hoursRemaining << " ore di gioco.";
	}
	msg << "\n\r";
	send_to_char(msg.str().c_str(), ch);
}

const char* pray_pick_room_emote(bool isDevoutClass) {
	switch(number(1, 6)) {
	case 1:
		return isDevoutClass ? "$n china il capo e mormora una preghiera sottovoce."
		                     : "$n resta in silenzio, raccogliendosi per un istante.";
	case 2:
		return isDevoutClass ? "Un fremito percorre l'aria mentre $n invoca il suo dio."
		                     : "$n mormora poche parole, con tono misurato.";
	case 3:
		return isDevoutClass ? "$n intreccia le mani e sussurra parole antiche."
		                     : "$n abbassa lo sguardo e riflette in silenzio.";
	case 4:
		return isDevoutClass ? "Per un istante, attorno a $n cala un silenzio solenne."
		                     : "$n chiude gli occhi per un breve momento.";
	case 5:
		return isDevoutClass ? "$n alza lo sguardo al cielo e prega con fervore."
		                     : "$n sussurra una breve invocazione, senza enfasi.";
	default:
		return isDevoutClass ? "Una tenue aura di devozione sembra avvolgere $n."
		                     : "$n si concentra un attimo, poi torna composto.";
	}
}

int pray_success_threshold(struct char_data* ch) {
	int threshold = static_cast<int>(GetMaxLevel(ch) * 1.5 + 20);
	if(HasClass(ch, CLASS_CLERIC | CLASS_DRUID)) {
		threshold += 10;
	}
	if(ch->desc != nullptr && ch->desc->AccountData.authorized) {
		threshold += 60;
	}
	return threshold;
}

void pray_notify_immortals(struct char_data* ch, const char* godName, const char* prayerText) {
	if(ch == nullptr || godName == nullptr || prayerText == nullptr) {
		return;
	}
	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		if(d->character == nullptr || d->character == ch || d->connected != 0) {
			continue;
		}
		struct char_data* imm = d->character;
		if(IS_NPC(imm) || IS_SET(imm->specials.act, PLR_NOSHOUT)) {
			continue;
		}
		if(GetMaxLevel(imm) < IMMORTALE) {
			continue;
		}
		std::string prayerLine;
		if(!str_cmp2(godName, GET_NAME(imm))) {
			prayerLine = "$c0013[$c0015$n$c0013] TI PREGA: '";
		}
		else {
			prayerLine = "$c0014[$c0015$n$c0014] prega :'";
		}
		prayerLine += prayerText;
		prayerLine += "'";
		act(prayerLine.c_str(), FALSE, ch, nullptr, imm, TO_VICT);
	}
}

void telepathy_deliver(struct char_data* ch, struct char_data* vict, const char* message) {
	if(ch == nullptr || vict == nullptr || message == nullptr) {
		return;
	}
	std::ostringstream toVict;
	toVict << "$c0013[$c0015" << comm_display_name(ch) << "$c0013] ti manda il pensiero '"
	       << message << "'";
	act(toVict.str().c_str(), FALSE, vict, nullptr, nullptr, TO_CHAR);

	if(!IS_NPC(ch) && !IS_SET(ch->specials.act, PLR_ECHO)) {
		return;
	}
	std::ostringstream echo;
	echo << "$c0013Tu mandi a " << comm_display_name(vict) << " il pensiero '" << message << "'";
	act(echo.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
}

constexpr CommDirectFormat kQuestNpcTellFmt{
    "$c0013", true, "ti dice", "Tu dici a ", false, "$c0013$n dice qualcosa a $N.",
};

void ask_quest_npc_tell(struct char_data* npc, struct char_data* player, const std::string& line) {
	if(npc == nullptr || player == nullptr || line.empty()) {
		return;
	}
	comm_direct_deliver(npc, player, line.c_str(), CMD_TELL, kQuestNpcTellFmt);
}

std::string ask_quest_prefixed_line(struct char_data* ch, const char* text) {
	std::ostringstream os;
	if(ch != nullptr && GET_NAME(ch) != nullptr) {
		os << GET_NAME(ch) << ' ';
	}
	os << (text != nullptr ? text : "");
	return os.str();
}

bool ask_quest_is_hint_vendor(struct char_data* vict) {
	if(vict == nullptr || IS_PC(vict)) {
		return false;
	}
	const char* const victName = GET_NAME(vict);
	if(victName == nullptr) {
		return false;
	}
	return isname2("ladro", victName) || isname2("cacciatore", victName) ||
	       isname2("spia", victName) ||
	       isname2("shop_keeper", mob_index[vict->nr].specname);
}

bool ask_quest_message_requests_hint(const char* message) {
	return message != nullptr && std::strstr(message, "indizio") != nullptr;
}

bool ask_quest_refresh_target(struct char_data* ch) {
	if(ch == nullptr || ch->specials.quest_ref == nullptr) {
		return false;
	}
	const char* const targetName = ch->specials.quest_ref->player.name;
	if(targetName == nullptr || targetName[0] == '\0') {
		return false;
	}
	struct char_data* const located = get_char_vis_world(ch, targetName, nullptr);
	if(located == nullptr) {
		return false;
	}
	ch->specials.quest_ref = located;
	return true;
}

std::string ask_quest_build_location_hint(struct char_data* ch) {
	if(ch == nullptr) {
		return {};
	}
	struct char_data* const tgt = ch->specials.quest_ref;
	if(tgt == nullptr) {
		return ask_quest_prefixed_line(ch, "Mi spiace, ma non ho informazioni al riguardo...");
	}

	const char* const tgtName = (tgt->player.name != nullptr) ? tgt->player.name : "?";
	const struct room_data* const here = real_roomp(ch->in_room);
	const struct room_data* const there = real_roomp(tgt->in_room);
	if(here == nullptr || there == nullptr) {
		mudlog(LOG_SYSERR, "ask_quest_build_location_hint: stanza nulla per %s",
		       comm_display_name(ch));
		return ask_quest_prefixed_line(ch, "Mi spiace, ma non ho informazioni al riguardo...");
	}

	std::ostringstream os;
	if(GET_NAME(ch) != nullptr) {
		os << GET_NAME(ch) << ' ';
	}
	os << tgtName;
	if(here->zone == there->zone) {
		os << "? Ho sentito che l'ultima volta e' stato vist" << SSLF(tgt) << " a " << there->name;
	}
	else {
		const char* const zoneName = zonename_by_room(tgt->in_room);
		os << "? Pare fosse dirett" << SSLF(tgt) << " verso "
		   << (zoneName != nullptr ? zoneName : "?");
	}
	return os.str();
}

/** @return true se gestita (messaggi gia' inviati). */
bool ask_quest_try_hint_response(struct char_data* ch, struct char_data* vict, const char* message) {
	if(ch == nullptr || vict == nullptr || message == nullptr) {
		return false;
	}
	if(!affected_by_spell(ch, STATUS_QUEST) || !ask_quest_is_hint_vendor(vict)) {
		return false;
	}

	if(ch->specials.quest_ref == nullptr) {
		ask_quest_npc_tell(vict, ch,
		                   ask_quest_prefixed_line(ch, "Cio' che cerchi appartiene al passato."));
		return true;
	}

	if(!ask_quest_message_requests_hint(message) || !IsHumanoid(vict)) {
		ask_quest_npc_tell(
		    vict, ch,
		    ask_quest_prefixed_line(ch,
		                            "Se vuoi un indizio chiedimelo chiaramente... ma ti costera'!"));
		return true;
	}

	if(!ask_quest_refresh_target(ch)) {
		ask_quest_npc_tell(
		    vict, ch,
		    ask_quest_prefixed_line(ch, "Mi spiace, ma non ho informazioni al riguardo..."));
		return true;
	}

	const int price = number(250000, 300000) - (5000 * GET_CHR(ch));
	if(GET_GOLD(ch) < price) {
		ask_quest_npc_tell(
		    vict, ch,
		    ask_quest_prefixed_line(ch, "...ma chi credi di comprare con quegli spiccioli!"));
		return true;
	}

	if(!IS_DIO(ch)) {
		GET_GOLD(ch) -= price;
		std::ostringstream payMsg;
		payMsg << "Paghi " << price << " monete a " << comm_display_name(vict)
		       << " per le sue informazioni. \n\r";
		send_to_char(payMsg.str().c_str(), ch);
	}

	ask_quest_npc_tell(vict, ch, ask_quest_build_location_hint(ch));
	return true;
}

std::string thief_garble_overheard(struct char_data* speaker, struct char_data* listener,
                                   const char* frase, int malus) {
	std::string overheard = (frase != nullptr) ? frase : "";
	const int percent = GetMaxLevel(speaker) - GetMaxLevel(listener);
	const int spyLearned = listener->skills[SKILL_TSPY].learned;

	for(char& c : overheard) {
		if((percent + number(20, 120) + malus) <= spyLearned) {
			continue;
		}
		if((malus + number(1, 40) - GET_INT(listener)) <= 0) {
			continue;
		}
		if(number(0, 20) == 0) {
			continue;
		}
		PushStatus("Random");
		if(number(1, 101) > 100 && spyLearned < 100) {
			listener->skills[SKILL_TSPY].learned++;
		}
		c = RandomChar();
		PopStatus();
	}
	return overheard;
}
} // namespace

ACTION_FUNC(do_write) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_write (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_write (act.comm.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		mudlog(LOG_SYSERR, "ch->desc==nullptr in do_write (act.comm.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> papername{};
	std::array<char, MAX_INPUT_LENGTH> penname{};
	argument_interpreter(arg, papername.data(), penname.data());

	if(papername[0] == '\0' || penname[0] == '\0') {
		send_to_char("write <foglio> <penna>\n\r"
		              "write on <foglio> with <penna>\n\r",
		              ch);
		return;
	}

	OBJ_DATA* const paper = get_obj_in_list_vis(ch, papername.data(), ch->carrying);
	if(paper == nullptr) {
		std::ostringstream os;
		os << "Tu non hai nessun " << papername.data() << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	OBJ_DATA* const pen = get_obj_in_list_vis(ch, penname.data(), ch->carrying);
	if(pen == nullptr) {
		std::ostringstream os;
		os << "Tu non hai nessun " << penname.data() << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(pen->obj_flags.type_flag != ITEM_PEN) {
		act("Non puoi scrivere con $p.", FALSE, ch, pen, nullptr, TO_CHAR);
		return;
	}
	if(paper->obj_flags.type_flag != ITEM_NOTE) {
		act("Non puoi scrivere su $p.", FALSE, ch, paper, nullptr, TO_CHAR);
		return;
	}
	if(paper->action_description != nullptr && paper->action_description[0] != '\0') {
		send_to_char("C'e' gia' scritto sopra qualcosa.\n\r", ch);
		return;
	}

	send_to_char("Ok... puoi scrivere... finisci la nota con un @.\n\r", ch);
	act("$n comincia a scrivere qualcosa.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	ch->desc->str = &paper->action_description;
	ch->desc->max_str = kMaxNoteLength;
}



ACTION_FUNC(do_sign) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_sign (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_sign (act.comm.cpp)");
		return;
	}

	while(*arg == ' ') {
		arg++;
	}

	if(*arg == '\0') {
		send_to_char("D'accordo, ma cosa vuoi dire?\n\r", ch);
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "do_sign: real_roomp null per %s", GET_NAME(ch));
		return;
	}

	if(!HasHands(ch)) {
		send_to_char("Si giusto... ma con QUALI MANI?\n\r", ch);
		return;
	}

	const std::string spokenText = arg;
	const std::string correctedText = sign_build_corrected(ch, spokenText);
	sign_broadcast_room(ch, rp, spokenText, correctedText);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream echoOs;
		echoOs << "Tu hai detto '" << spokenText << "'\n\r";
		send_to_char(echoOs.str().c_str(), ch);
	}
}

/* speak elvish, speak dwarvish, etc... */
ACTION_FUNC(do_speak) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_speak (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_speak (act.comm.cpp)");
		return;
	}

	struct SpeakLanguage {
		int speakValue;
		const char* commandToken;
		const char* displayLabel;
	};

	static constexpr std::array<SpeakLanguage, 8> kSpeakLanguages = {{
	    {SPEAK_COMMON, "common", "la lingua comune"},
	    {SPEAK_ELVISH, "elvish", "elfico"},
	    {SPEAK_HALFLING, "halfling", "la lingua halfling"},
	    {SPEAK_DWARVISH, "dwarvish", "nanico"},
	    {SPEAK_ORCISH, "orcish", "orchesco"},
	    {SPEAK_GIANTISH, "giantish", "la lingua dei giganti"},
	    {SPEAK_OGRE, "ogre", "la lingua degli ogre"},
	    {SPEAK_GNOMISH, "gnomish", "gnomesco"},
	}};

	std::array<char, 255> langTok{};
	only_argument(arg, langTok.data());

	if(langTok[0] == '\0') {
		send_to_char("Sintassi: speak <lingua>\n\r"
		              "         (common, elvish, dwarvish, orcish, halfling, giantish, ogre, gnomish)\n\r",
		              ch);
		return;
	}

	const SpeakLanguage* selected = nullptr;
	for(const auto& language : kSpeakLanguages) {
		if(speak_language_matches(langTok.data(), language.commandToken)) {
			selected = &language;
			break;
		}
	}

	if(selected == nullptr) {
		send_to_char("Non e' un linguaggio molto conosciuto.\n\r", ch);
		return;
	}

	ch->player.speaks = selected->speakValue;

	std::ostringstream os;
	os << "Ti concentri nel parlare " << selected->displayLabel << ".\n\r";
	send_to_char(os.str().c_str(), ch);
}

void thief_listen(struct char_data* ch, struct char_data* victim, const char* frase, int cmd) {
	if(ch == nullptr || victim == nullptr || frase == nullptr) {
		mudlog(LOG_SYSERR, "thief_listen: parametro nullo (act.comm.cpp)");
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		return;
	}
	if(IS_DIO(ch) || IS_DIO(victim)) {
		return;
	}

	const int malus = thief_listen_malus(ch, cmd);
	PushStatus("Thief_listen");
	for(struct char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
		if(!thief_listener_eligible(t, ch, victim)) {
			continue;
		}
		const std::string overheard = thief_garble_overheard(ch, t, frase, malus);
		std::vector<char> overheardBuf(overheard.begin(), overheard.end());
		overheardBuf.push_back('\0');
		act("$c0013Riesci ad origliare: '$T'", FALSE, t, nullptr, overheardBuf.data(), TO_CHAR);
	}
	PopStatus();
}

/* say con lingua (speak) — testo al parlante in chiaro, ascoltatori in base a skill lingua */
ACTION_FUNC(do_new_say) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_new_say (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_new_say (act.comm.cpp)");
		return;
	}

	while(*arg == ' ') {
		arg++;
	}

	if(*arg == '\0') {
		send_to_char("Ok, ma cosa hai da dire?\n\r", ch);
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi parlare in queste condizioni.\n\r", ch);
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "do_new_say: real_roomp null per %s", GET_NAME(ch));
		return;
	}

	int learned = 0;
	int skillNum = LANG_COMMON;
	say_resolve_language(ch, learned, skillNum);

	const std::string spokenText = arg;
	const std::string correctedText = say_build_corrected(ch, spokenText, learned);
	if(correctedText.empty()) {
		send_to_char("Ok, ma cosa hai da dire?\n\r", ch);
		return;
	}

	say_broadcast_room(ch, rp, spokenText, correctedText, skillNum);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream echoOs;
		echoOs << "$c0015Tu dici '" << spokenText << "'";
		act(echoOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
}



ACTION_FUNC(do_gtell) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_gtell (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_gtell (act.comm.cpp)");
		return;
	}
	if(!comm_direct_sender_ok(ch)) {
		return;
	}

	while(*arg == ' ') {
		arg++;
	}

	if(*arg == '\0') {
		send_to_char("Cosa vuoi dire al gruppo?\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(ch, AFF_GROUP)) {
		send_to_char("Forse dovresti unirti ad un gruppo, prima.\n\r", ch);
		return;
	}

	const char* const message = arg;
	thief_listen(ch, ch, message, cmd);
	gtell_broadcast(ch, message);

	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream echoOs;
		echoOs << "$c0012Tu dici al gruppo '" << message << "'";
		act(echoOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
}

/*
 * 'Split' originally by Gnort, God of Chaos. I stole it from Merc
 * and changed it to work with mine :) Heh msw
 */

ACTION_FUNC(do_split) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_split (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_split (act.comm.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> amountTok{};
	const char* rest = one_argument(arg, amountTok.data());
	while(*rest == ' ') {
		++rest;
	}

	if(*rest != '\0') {
		send_to_char("Sintassi: split <quantita'>\n\r", ch);
		return;
	}

	if(amountTok[0] == '\0') {
		send_to_char("Quanto vuoi dividere?\n\r", ch);
		return;
	}

	int amount = 0;
	if(!split_parse_amount(amountTok.data(), amount)) {
		send_to_char("Inserisci un numero valido.\n\r", ch);
		return;
	}

	if(amount < 0) {
		send_to_char("Non cercare di fare il furbo con il tuo gruppo.\n\r", ch);
		return;
	}

	if(amount == 0) {
		send_to_char("Hai diviso zero monete, ma nessuno lo ha notato.\n\r", ch);
		return;
	}

	if(GET_GOLD(ch) < amount) {
		send_to_char("Non hai tutto quell'oro.\n\r", ch);
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "do_split: real_roomp null per %s", GET_NAME(ch));
		return;
	}

	const int members = split_count_group_in_room(rp, ch);
	if(members < 2) {
		send_to_char("Ma cosa vuoi dividere che sei solo.\n\r", ch);
		return;
	}

	const int share = amount / members;
	const int extra = amount % members;
	if(share == 0) {
		send_to_char("C'e' poco da dividere, siete in troppi.\n\r", ch);
		return;
	}

	const int selfShare = share + extra;
	GET_GOLD(ch) -= amount;
	GET_GOLD(ch) += selfShare;

	{
		std::ostringstream selfMsg;
		selfMsg << "Hai diviso " << split_gold_amount_phrase(amount) << ". La tua parte e' di "
		        << split_coin_part_phrase(selfShare) << ".\n\r";
		send_to_char(selfMsg.str().c_str(), ch);
	}

	std::ostringstream groupActOs;
	groupActOs << "$n divide " << split_gold_amount_phrase(amount) << ". La tua parte e' di "
	           << split_coin_part_phrase(share) << '.';
	const std::string groupAct = groupActOs.str();

	for(struct char_data* gch = rp->people; gch != nullptr; gch = gch->next_in_room) {
		if(gch == ch || !is_same_group(gch, ch)) {
			continue;
		}
		act(groupAct.c_str(), FALSE, ch, nullptr, gch, TO_VICT);
		GET_GOLD(gch) += share;
	}
}


ACTION_FUNC(do_pray) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_pray (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_pray (act.comm.cpp)");
		return;
	}
	if(!IS_PC(ch)) {
		return;
	}

	if(affected_by_spell(ch, SPELL_PRAYER)) {
		pray_send_cooldown_message(ch, pray_spell_duration_remaining(ch));
		return;
	}

	while(*arg == ' ') {
		++arg;
	}
	if(*arg == '\0') {
		send_to_char("Vuoi pregare. Ottimo, ma chi? (pray <NomeDio> <preghiera>)\n\r", ch);
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> godName{};
	one_argument(arg, godName.data());
	mudlog(LOG_CHECK, "%s ha pregato %s", GET_NAME(ch), godName.data());

	if(!strcasecmp(godName.data(), "darkstar")) {
		if(procarea_try_convert_rune_fragments(ch, arg)) {
			return;
		}
		if(procarea_try_darkstar_aid(ch, arg)) {
			return;
		}
	}

	const bool isDevoutClass = HasClass(ch, CLASS_CLERIC | CLASS_DRUID);
	if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
		std::ostringstream selfPrayer;
		selfPrayer << "Tu preghi '" << arg << "'\n\r";
		send_to_char(selfPrayer.str().c_str(), ch);
	}

	act(pray_pick_room_emote(isDevoutClass), FALSE, ch, nullptr, nullptr, TO_ROOM);

	int durata = 6;
	if(pray_success_threshold(ch) > number(1, 101)) {
		pray_notify_immortals(ch, godName.data(), arg);
		durata = 12;
	}

	struct affected_type af{};
	af.type = SPELL_PRAYER;
	af.duration = durata;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
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
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_telepathy (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_telepathy (act.comm.cpp)");
		return;
	}

	const auto [nameStr, messageStr] =
	    chop_argument(arg, 99, static_cast<std::size_t>(MAX_INPUT_LENGTH + 19));

	if(!HasClass(ch, CLASS_PSI) && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
		send_to_char("Cosa pensi di essere? Un telepate?\n\r", ch);
		return;
	}

	if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Non puoi farlo in queste condizioni.\n\r", ch);
		return;
	}

	if(GET_MANA(ch) < 5 && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
		send_to_char("Non hai la potenza mentale sufficiente.\n\r", ch);
		return;
	}

	if(nameStr.empty() || messageStr.empty()) {
		send_to_char("A chi vuoi mandare il tuo pensiero?\n\r", ch);
		return;
	}

	struct char_data* vict = get_char_vis(ch, nameStr.c_str());
	if(vict == nullptr) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
		return;
	}
	if(ch == vict) {
		send_to_char("Nella tua mente risuona il tuo pensiero...\n\r", ch);
		return;
	}
	if(GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
		act("Sta dormendo, shhh.", FALSE, ch, nullptr, vict, TO_CHAR);
		return;
	}
	if(IS_NPC(vict) && vict->desc == nullptr) {
		send_to_char("Non c'e' nessuno con quel nome qui...\n\r", ch);
		return;
	}
	if(tell_vict_refuses(ch, vict)) {
		if(GetMaxLevel(vict) >= IMMORTALE && GetMaxLevel(ch) >= IMMORTALE) {
			act("La mente di $N e' chiusa in questo momento!", FALSE, ch, nullptr, vict, TO_CHAR);
		}
		else {
			act("$N non sta ascoltando adesso.", FALSE, ch, nullptr, vict, TO_CHAR);
		}
		return;
	}
	if(vict->desc == nullptr) {
		send_to_char("Non puo' sentirti. Gli e' caduta la linea (link dead).\n\r", ch);
		return;
	}

	if(!IS_AFFECTED(ch, AFF_TELEPATHY)) {
		GET_MANA(ch) -= 5;
		alter_mana(ch, 0);
	}

	telepathy_deliver(ch, vict, messageStr.c_str());
}

ACTION_FUNC(do_eavesdrop) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_eavesdrop (act.comm.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_eavesdrop (act.comm.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> dirTok{};
	one_argument(arg, dirTok.data());

	if(dirTok[0] == '\0') {
		send_to_char("In quale direzione vuoi ascoltare?\r\n", ch);
		return;
	}

	const int dir = search_block(dirTok.data(), dirs, FALSE);
	if(dir < 0) {
		send_to_char("Che direzione sarebbe?\r\n", ch);
		return;
	}

	if(ch->skills[SKILL_EAVESDROP].learned < number(1, 101)) {
		send_to_char("Resti immobile concentrandoti sui rumori che provengono da quella parte.\r\n", ch);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		return;
	}

	struct room_direction_data* const exitp = EXIT(ch, dir);
	if(exitp == nullptr || exitp->to_room <= 0) {
		send_to_char("Non c'e' nulla da quella parte...\r\n", ch);
		return;
	}

	struct room_data* const destRoom = real_roomp(exitp->to_room);
	if(destRoom == nullptr) {
		mudlog(LOG_SYSERR, "do_eavesdrop: real_roomp null per stanza %d", exitp->to_room);
		send_to_char("Non c'e' nulla da quella parte...\r\n", ch);
		return;
	}

	if(IS_SET(exitp->exit_info, EX_CLOSED) && exitp->keyword != nullptr) {
		std::ostringstream closedMsg;
		closedMsg << "The " << fname(exitp->keyword) << " is closed.\r\n";
		send_to_char(closedMsg.str().c_str(), ch);
		return;
	}

	ch->next_listener = destRoom->listeners;
	destRoom->listeners = ch;
	ch->listening_to = exitp->to_room;
	send_to_char("Resti immobile concentrandoti sui rumori che provengono da quella parte.", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);
}


} // namespace Alarmud
