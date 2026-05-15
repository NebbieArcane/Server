/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * DaleMUD v2.0        Released 2/1994
 * See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
 * $Id: act.move.c,v 2.1 2002/03/24 21:36:40 Thunder Exp $
 */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <array>
#include <vector>
#include <sstream>
#include <string>
#include <string_view>
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
#include "act.move.hpp"
#include "act.info.hpp"
#include "act.off.hpp"       // for clearpath
#include "act.other.hpp"
#include "cmdid.hpp"         // for CMD_DIG, CMD_SCYTHE, CMD_LIFT, CMD_PULL
#include "comm.hpp"
#include "config.hpp"        // for FALSE, TRUE
#include "db.hpp"            // for index_data
#include "fight.hpp"
#include "handler.hpp"       // for fname, generic_find, char_from_room, cha...
#include "interpreter.hpp"   // for one_argument, only_argument, search_block
#include "multiclass.hpp"
#include "regen.hpp"
#include "signals.hpp"       // for SetStatus
#include "skills.hpp"
#include "spell_parser.hpp"  // for SpellWearOff, stop_follower, add_follower
#include "spells.hpp"        // for SKILL_MEDITATE, SKILL_MEMORIZE, SKILL_PI...
#include "trap.hpp"          // for CheckForMoveTrap
#include "utility.hpp"       // for exit_ok, FallOffMount, LearnFromMistake

namespace Alarmud {

namespace {

const char* move_actor_label(struct char_data* ch) {
	if(ch == nullptr) {
		return "?";
	}
	if(IS_NPC(ch)) {
		if(ch->player.short_descr != nullptr && ch->player.short_descr[0] != '\0') {
			return ch->player.short_descr;
		}
		return (GET_NAME(ch) != nullptr) ? GET_NAME(ch) : "?";
	}
	return (GET_NAME(ch) != nullptr) ? GET_NAME(ch) : "?";
}

/** Per messaggi di gruppo: toglie articolo iniziale da short_descr ("Un goblin", "A troll" -> "goblin"/"troll"). */
std::string move_strip_leading_article(const char* label) {
	if(label == nullptr || label[0] == '\0') {
		return "?";
	}
	std::string_view sv(label);
	static constexpr std::string_view kArticles[] = {
	    "uno ", "una ", "un ", "il ", "lo ", "la ", "i ", "gli ", "le ",
	    "an ", "a ", "the ",
	};
	for(const std::string_view art : kArticles) {
		if(sv.size() <= art.size()) {
			continue;
		}
		bool match = true;
		for(std::size_t i = 0; i < art.size(); ++i) {
			if(std::tolower(static_cast<unsigned char>(sv[i])) != art[i]) {
				match = false;
				break;
			}
		}
		if(match) {
			sv.remove_prefix(art.size());
			break;
		}
	}
	return std::string(sv);
}

const char* move_italian_count_word(int count) {
	switch(count) {
	case 2:
		return "due";
	case 3:
		return "tre";
	case 4:
		return "quattro";
	case 5:
		return "cinque";
	case 6:
		return "sei";
	case 7:
		return "sette";
	case 8:
		return "otto";
	case 9:
		return "nove";
	case 10:
		return "dieci";
	default:
		return nullptr;
	}
}

std::string move_to_lower_ascii(std::string word) {
	for(char& c : word) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return word;
}

bool move_is_article_word(const std::string& word) {
	const std::string lower = move_to_lower_ascii(word);
	return lower == "il" || lower == "lo" || lower == "la" || lower == "i" || lower == "gli" ||
	       lower == "le" || lower == "uno" || lower == "una" || lower == "un" || lower == "the" ||
	       lower == "a" || lower == "an";
}

std::string move_pluralize_article(const std::string& word) {
	const std::string lower = move_to_lower_ascii(word);
	if(lower == "il" || lower == "lo" || lower == "un") {
		return "i";
	}
	if(lower == "la" || lower == "una") {
		return "le";
	}
	if(lower == "uno") {
		return word;
	}
	return word;
}

/** Nomi propri brevi in -e (Niobe, Sara): non "Niobe" -> "Niobi". Non blocca "Avventuriero" -> "avventurieri". */
bool move_is_likely_given_name(const std::string& word) {
	if(word.size() < 3 || word.size() > 7) {
		return false;
	}
	if(word[0] < 'A' || word[0] > 'Z') {
		return false;
	}
	if(move_is_article_word(word)) {
		return false;
	}
	const char last = static_cast<char>(std::tolower(static_cast<unsigned char>(word.back())));
	return last == 'e';
}

bool move_pluralize_e_ending(const std::string& word) {
	if(word.size() < 6) {
		return false;
	}
	const std::string lower = move_to_lower_ascii(word);
	return lower.size() >= 4 &&
	       (lower.compare(lower.size() - 4, 4, "ente") == 0 || lower.compare(lower.size() - 4, 4, "ante") == 0 ||
	        lower.compare(lower.size() - 3, 3, "ale") == 0 || lower.compare(lower.size() - 3, 3, "ile") == 0 ||
	        lower.compare(lower.size() - 3, 3, "oso") == 0 || lower.compare(lower.size() - 4, 4, "iere") == 0);
}

/** Plurale inglese sull'ultima parola del mob ("devil" -> "devils", "bone devil" -> "bone devils"). */
std::string move_pluralize_english_s(std::string word) {
	if(word.size() < 2 || word.find('-') != std::string::npos) {
		return move_to_lower_ascii(word);
	}
	const std::string lower = move_to_lower_ascii(word);
	const char last = lower.back();
	const char prev = lower.size() >= 2 ? lower[lower.size() - 2] : '\0';
	const auto endsWith = [&lower](const char* suffix) {
		const std::size_t n = std::strlen(suffix);
		return lower.size() >= n && lower.compare(lower.size() - n, n, suffix) == 0;
	};

	if(last == 'y' && prev != '\0' && std::strchr("aeiou", prev) == nullptr) {
		word.pop_back();
		word += "ies";
	}
	else if(endsWith("ss") || endsWith("sh") || endsWith("ch") || last == 'x' || last == 'z' ||
	        last == 's') {
		word += "es";
	}
	else {
		word += 's';
	}
	return move_to_lower_ascii(word);
}

std::string move_pluralize_word(std::string word, bool englishNounTail, bool italianLastEnding) {
	if(word.size() < 2) {
		return word;
	}
	if(move_is_article_word(word)) {
		return move_pluralize_article(word);
	}
	if(move_is_likely_given_name(word)) {
		return word;
	}
	const auto lower = [&word](std::size_t i) {
		return static_cast<char>(std::tolower(static_cast<unsigned char>(word[i])));
	};
	const char last = lower(word.size() - 1);
	const char prev = lower(word.size() - 2);

	if(word.size() >= 3 && lower(word.size() - 3) == 'c' && prev == 'i' && last == 'a') {
		word.back() = 'e';
	}
	else if(last == 'a' && prev == 'c') {
		word.pop_back();
		word += "he";
	}
	else if(last == 'o' && prev == 'c') {
		if(word.size() >= 3 && lower(word.size() - 3) == 'i') {
			word.back() = 'i';
		}
		else {
			word.pop_back();
			word += "hi";
		}
	}
	else if(last == 'o' && prev == 'g') {
		word.pop_back();
		word += "hi";
	}
	else if(word.size() >= 4 && lower(word.size() - 3) == 'a' && lower(word.size() - 2) == 'l' &&
	        last == 'e') {
		word[word.size() - 2] = 'i';
		word.pop_back();
	}
	else if(last == 'o' && (!englishNounTail || italianLastEnding)) {
		word.back() = 'i';
	}
	else if(last == 'a' && (!englishNounTail || italianLastEnding)) {
		word.back() = 'e';
	}
	else if(last == 'e' && move_pluralize_e_ending(word)) {
		word.back() = 'i';
	}
	else if(englishNounTail) {
		return move_pluralize_english_s(word);
	}
	return move_to_lower_ascii(word);
}

std::string move_pluralize_phrase(const std::string& phrase) {
	std::istringstream words(phrase);
	std::vector<std::string> tokens;
	std::string token;
	while(words >> token) {
		tokens.push_back(token);
	}
	if(tokens.empty()) {
		return phrase;
	}
	bool priorItalian = false;
	std::ostringstream os;
	for(std::size_t i = 0; i < tokens.size(); ++i) {
		if(i > 0) {
			os << ' ';
		}
		const bool lastToken = (i + 1 == tokens.size());
		const std::string inLower = move_to_lower_ascii(tokens[i]);
		const bool italianLastEnding =
		    priorItalian ||
		    (lastToken && tokens.size() == 1 &&
		     inLower.back() == 'o');
		const std::string out = move_pluralize_word(tokens[i], lastToken, italianLastEnding);
		if(!lastToken && out != inLower) {
			priorItalian = true;
		}
		os << out;
	}
	return os.str();
}

/** Es. total=4, "un avventuriero perduto" -> "Quattro avventurieri perduti". */
std::string move_group_counted_label(struct char_data* ch, int total) {
	const std::string base = move_strip_leading_article(move_actor_label(ch));
	if(total <= 1) {
		return base;
	}
	std::ostringstream os;
	if(const char* const word = move_italian_count_word(total)) {
		os << word;
	}
	else {
		os << total;
	}
	os << ' ' << move_pluralize_phrase(base);
	std::string out = os.str();
	if(!out.empty()) {
		out[0] = static_cast<char>(UPPER(out[0]));
	}
	return out;
}

/** NPC visibile: "Un hill giant" invece di "A hill giant" (usa UNUNA + short_descr senza articolo). */
std::string move_npc_subject_label(struct char_data* ch) {
	if(ch == nullptr) {
		return "?";
	}
	if(!IS_NPC(ch)) {
		return (ch->player.name != nullptr) ? ch->player.name : "?";
	}
	std::ostringstream os;
	os << UNUNA(ch) << ' ' << move_strip_leading_article(move_actor_label(ch));
	std::string out = os.str();
	if(!out.empty()) {
		out[0] = static_cast<char>(UPPER(out[0]));
	}
	return out;
}

std::string move_npc_pers_label(struct char_data* ch, struct char_data* observer) {
	if(ch == nullptr) {
		return "?";
	}
	if(observer != nullptr && !CAN_SEE(observer, ch)) {
		return "qualcuno";
	}
	if(!IS_NPC(ch)) {
		return (ch->player.name != nullptr) ? ch->player.name : "?";
	}
	return move_npc_subject_label(ch);
}

void move_capitalize_message(std::string& msg) {
	if(msg.size() > 1 && msg[1] == '$') {
		if(msg.size() > 7) {
			msg[7] = static_cast<char>(UPPER(msg[7]));
		}
	}
	else if(!msg.empty() && msg[0] == '$') {
		if(msg.size() > 6) {
			msg[6] = static_cast<char>(UPPER(msg[6]));
		}
	}
	else if(!msg.empty()) {
		msg[0] = static_cast<char>(UPPER(msg[0]));
	}
}

std::string move_format_departure(struct char_data* ch, int dir, int total) {
	const char* const dirTxt = dirsTo[dir];
	std::ostringstream os;

	if(total > 1) {
		const std::string groupLabel = move_group_counted_label(ch, total);
		if(IS_AFFECTED(ch, AFF_FLYING)) {
			os << groupLabel << " volano " << dirTxt << ".\r\n";
		}
		else {
			os << groupLabel << " vanno " << dirTxt << ".\r\n";
		}
		return os.str();
	}

	const std::string actor = IS_NPC(ch) ? move_npc_subject_label(ch) : std::string(move_actor_label(ch));
	if(IS_NPC(ch)) {
		if(MOUNTED(ch)) {
			os << actor << " va " << dirTxt << ", cavalcando " << MOUNTED(ch)->player.short_descr
			   << "\r\n";
		}
		else if(IS_AFFECTED(ch, AFF_FLYING)) {
			os << actor << " vola " << dirTxt << ".\r\n";
		}
		else {
			os << actor << " va " << dirTxt << ".\r\n";
		}
	}
	else {
		if(MOUNTED(ch)) {
			os << actor << " va " << dirTxt << ", cavalcando " << MOUNTED(ch)->player.short_descr
			   << ".\r\n";
		}
		else if(IS_AFFECTED(ch, AFF_FLYING)) {
			os << actor << " vola " << dirTxt << ".\r\n";
		}
		else {
			os << actor << " va " << dirTxt << ".\r\n";
		}
	}
	return os.str();
}

std::string move_format_arrival(struct char_data* ch, int dir, int total, struct char_data* observer) {
	std::ostringstream os;
	if(dir < 6) {
		const char* const fromDir = dirsFrom[rev_dir[dir]];
		if(total == 1) {
			if(MOUNTED(ch)) {
				os << move_npc_pers_label(ch, observer) << " arriva " << fromDir << ", cavalcando "
				   << move_npc_pers_label(MOUNTED(ch), observer) << ".\r\n";
			}
			else if(IS_AFFECTED(ch, AFF_FLYING)) {
				os << move_npc_pers_label(ch, observer) << " arriva volando " << fromDir << ".\r\n";
			}
			else {
				os << move_npc_pers_label(ch, observer) << " arriva " << fromDir << ".\r\n";
			}
		}
		else {
			const std::string groupLabel =
			    IS_NPC(ch) ? move_group_counted_label(ch, total) : std::string(PERS(ch, observer));
			if(IS_AFFECTED(ch, AFF_FLYING)) {
				os << groupLabel << " arrivano volando " << fromDir << ".\r\n";
			}
			else {
				os << groupLabel << " arrivano " << fromDir << ".\r\n";
			}
		}
	}
	else {
		if(total == 1) {
			if(MOUNTED(ch)) {
				os << "Arriva " << move_npc_pers_label(ch, observer) << " cavalcando "
				   << move_npc_pers_label(MOUNTED(ch), observer) << ".\r\n";
			}
			else if(IS_AFFECTED(ch, AFF_FLYING)) {
				os << "Arriva " << move_npc_pers_label(ch, observer) << " volando.\r\n";
			}
			else {
				os << "Arriva " << move_npc_pers_label(ch, observer) << ".\r\n";
			}
		}
		else {
			const std::string groupLabel =
			    IS_NPC(ch) ? move_group_counted_label(ch, total) : std::string(PERS(ch, observer));
			if(IS_AFFECTED(ch, AFF_FLYING)) {
				os << "Arrivano " << groupLabel << " volando.\r\n";
			}
			else {
				os << "Arrivano " << groupLabel << ".\r\n";
			}
		}
	}
	return os.str();
}

void move_show_look_and_exits(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	do_look(ch, "\0", 15);
	if(IS_SET(ch->player.user_flags, SHOW_EXITS)) {
		act("$c0015-----------------", FALSE, ch, nullptr, nullptr, TO_CHAR);
		do_exits(ch, "", 8);
		act("$c0015-----------------", FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
}

bool move_observer_sees_departure(struct char_data* mover, struct char_data* observer) {
	return mover != nullptr && observer != nullptr && mover != observer && AWAKE(observer) &&
	       CAN_SEE(observer, mover) &&
	       (!IS_AFFECTED(mover, AFF_SNEAK) || IS_IMMORTAL(observer));
}

bool move_observer_sees_arrival(struct char_data* mover, struct char_data* observer) {
	return mover != nullptr && observer != nullptr && mover != observer && AWAKE(observer) &&
	       CAN_SEE(observer, mover) &&
	       (!IS_AFFECTED(mover, AFF_SNEAK) || IS_IMMORTAL(observer));
}

bool move_should_announce(struct char_data* mover) {
	return !IS_AFFECTED(mover, AFF_SILENCE) || number(0, 2);
}

void move_display_group_batch(struct char_data* heap[kMoveGroupHeapMax], int top, int totals[kMoveGroupHeapMax],
                              int dir, int was_in) {
	for(int i = 0; i < top; ++i) {
		if(heap[i] != nullptr) {
			DisplayGroupMove(heap[i], dir, was_in, totals[i]);
		}
	}
}

char door_gender_vowel(struct room_direction_data* exitp) {
	return (exitp != nullptr && IS_SET(exitp->exit_info, EX_MALE)) ? 'o' : 'a';
}

const char* door_art_indef(struct room_direction_data* exitp) {
	return (exitp != nullptr && IS_SET(exitp->exit_info, EX_MALE)) ? "un" : "una";
}

const char* door_art_def(struct room_direction_data* exitp) {
	return (exitp != nullptr && IS_SET(exitp->exit_info, EX_MALE)) ? "il" : "la";
}

const char* door_art_def_cap(struct room_direction_data* exitp) {
	return (exitp != nullptr && IS_SET(exitp->exit_info, EX_MALE)) ? "Il" : "La";
}

const char* door_fname_or(struct room_direction_data* exitp, const char* fallback) {
	if(exitp != nullptr && exitp->keyword != nullptr) {
		return fname(exitp->keyword);
	}
	return fallback;
}

bool door_type_matches_exit(const char* type, struct room_direction_data* exitp) {
	if(type == nullptr || exitp == nullptr || !IS_SET(exitp->exit_info, EX_ISDOOR)) {
		return false;
	}
	if(exitp->keyword != nullptr) {
		return isname(type, exitp->keyword) != 0;
	}
	return !IS_SET(exitp->exit_info, EX_SECRET) || std::string_view(type) == "secret";
}

struct DoorCmdNames {
	const char* thirdPerson = nullptr;
	const char* secondPerson = nullptr;
	const char* infinitive = nullptr;
};

DoorCmdNames door_cmd_names(int cmd) {
	switch(cmd) {
	case CMD_PULL:
		return {"tira", "tiri", "tirare"};
	case CMD_TWIST:
		return {"svita", "sviti", "svitare"};
	case CMD_TURN:
		return {"gira", "giri", "girare"};
	case CMD_LIFT:
		return {"solleva", "sollevi", "sollevare"};
	case CMD_PUSH:
		return {"spinge", "spingi", "spingere"};
	case CMD_DIG:
		return {"scava", "scavi", "scavare"};
	case CMD_SCYTHE:
		return {"taglia", "tagli", "tagliare"};
	default:
		return {};
	}
}

void door_open_other_side(struct char_data* ch, struct room_direction_data* exitp, int dir,
                          bool rawVariant) {
	if(ch == nullptr || exitp == nullptr) {
		return;
	}
	struct room_data* rp = nullptr;
	if(!exit_ok(exitp, &rp) || rp == nullptr) {
		return;
	}
	struct room_direction_data* const back = rp->dir_option[rev_dir[dir]];
	if(back == nullptr || back->to_room != ch->in_room) {
		return;
	}
	REMOVE_BIT(back->exit_info, EX_CLOSED);
	if(IS_SET(back->exit_info, EX_SECRET)) {
		send_to_room(rawVariant ? "Un passaggio e' stato aperto dall'altro lato.\n\r"
		                        : "Un passaggio segreto e' stato aperto dall'altro lato.\n\r",
		             exitp->to_room);
		return;
	}
	if(back->keyword != nullptr) {
		std::ostringstream os;
		os << door_art_def_cap(back) << ' ' << fname(back->keyword) << " e' apert"
		   << door_gender_vowel(back) << " dall'altro lato.\n\r";
		send_to_room(os.str().c_str(), exitp->to_room);
	}
	else {
		send_to_room("La porta e' aperta dall'altro lato.\n\r", exitp->to_room);
	}
}

void door_close_other_side(struct char_data* ch, struct room_direction_data* exitp, int door) {
	if(ch == nullptr || exitp == nullptr) {
		return;
	}
	struct room_data* rp = nullptr;
	if(!exit_ok(exitp, &rp) || rp == nullptr) {
		return;
	}
	struct room_direction_data* const back = rp->dir_option[rev_dir[door]];
	if(back == nullptr || back->to_room != ch->in_room) {
		return;
	}
	SET_BIT(back->exit_info, EX_CLOSED);
	if(back->keyword != nullptr && !IS_SET(back->exit_info, EX_SECRET)) {
		std::ostringstream os;
		os << door_art_def_cap(back) << ' ' << back->keyword << ' ' << dirsTo[rev_dir[door]]
		   << " si chiude silenziosamente.\n\r";
		send_to_room(os.str().c_str(), exitp->to_room);
	}
	else if(back->keyword == nullptr) {
		std::ostringstream os;
		os << "La porta " << dirsTo[rev_dir[door]] << " si chiude silenziosamente.\n\r";
		send_to_room(os.str().c_str(), exitp->to_room);
	}
}

bool char_has_item_extra_flag(struct char_data* ch, unsigned long flag) {
	if(ch == nullptr) {
		return false;
	}
	struct obj_data* const held = ch->equipment[HOLD];
	if(held != nullptr && IS_SET(held->obj_flags.extra_flags, flag)) {
		return true;
	}
	struct obj_data* const wielded = ch->equipment[WIELD];
	return wielded != nullptr && IS_SET(wielded->obj_flags.extra_flags, flag);
}

} // namespace

void NotLegalMove(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	send_to_char("Non puoi andare da quella parte...\n\r", ch);
}

int ValidMove(struct char_data* ch, int cmd) {
	struct room_direction_data* exitp;

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in ValidMove (act.move.cpp)");
		return FALSE;
	}

	exitp = EXIT(ch, cmd);

	if(affected_by_spell(ch, SPELL_WEB)) {
		if(!saves_spell(ch, SAVING_PARA)) {
			send_to_char("Sei intrappolato in una ragnatela appiccicosa!\n\r",
						 ch);
			send_to_char("I tuoi sforzi non fanno altro che intrappolarti "
						 "maggiormente!\n\r", ch);
			WAIT_STATE(ch, PULSE_VIOLENCE*5); // web
			if(!IS_PC(ch))
				/* Deve essere azzerato. Quindi non va bene chiamare
				 * alter_move(ch,0);
				 * */
			{
				GET_MOVE(ch) = 0;
			}
			return (FALSE);
		}
		else {
			WAIT_STATE(ch, PULSE_VIOLENCE); // web
			GET_MOVE(ch) -= 50;
			alter_move(ch, 0);
			send_to_char(
				"Ti liberi facilmente dalla ragnatela appiccicosa!\n\r",
				ch);
		}
	}

	if(MOUNTED(ch)) {
		if(GET_POS(MOUNTED(ch)) < POSITION_FIGHTING) {
			send_to_char("La tua cavalcatura deve stare in piedi.\n\r", ch);
			return (FALSE);
		}
		if(ch->in_room != MOUNTED(ch)->in_room) {
			Dismount(ch, MOUNTED(ch), POSITION_STANDING);
		}
	}
#if 0
	if(RIDDEN(ch)) {
		if(ch->in_room != RIDDEN(ch)->in_room) {
			Dismount(RIDDEN(ch), ch, POSITION_STANDING);
		}
	}
#endif

	if(!exit_ok(exitp, nullptr)) {
		/* Se il numero di stanza in quella direzione e' 0 e c'e' una descrizione
		 * per l'uscita, allora mostra la descrizione.
		 * */
		if(exitp && exitp->to_room == 0 && exitp->general_description
				&& *exitp->general_description) {
			send_to_char(exitp->general_description, ch);
		}
		else {
			NotLegalMove(ch);
		}
		return (FALSE);
	}
	else if(IS_SET(exitp->exit_info, EX_CLOSED)) {
		if(exitp->keyword) {
			if(!IS_SET(exitp->exit_info, EX_SECRET)
					&& (std::string_view(fname(exitp->keyword)) != "secret")) {
				std::ostringstream closedMsg;
				closedMsg << "La " << fname(exitp->keyword) << " e' chius"
				          << (IS_SET(exitp->exit_info, EX_MALE) ? 'o' : 'a') << ".\n\r";
				send_to_char(closedMsg.str().c_str(), ch);
				return (FALSE);
			}
			else {
				NotLegalMove(ch);
				return (FALSE);
			}
		}
		else {
			NotLegalMove(ch);
			return (FALSE);
		}
	}
	else if(IS_SET(exitp->exit_info, EX_CLIMB) &&
			!IS_AFFECTED(ch, AFF_FLYING)) {
		send_to_char(
			"Mi spiace, ma puoi andare da quella parte solo volando od "
			"arrampicandoti!\n\r", ch);
		return FALSE;
	}
	else {
		struct room_data* const rp = real_roomp(exitp->to_room);
		if(rp == nullptr) {
			mudlog(LOG_SYSERR, "ValidMove: real_roomp null per stanza %d", exitp->to_room);
			NotLegalMove(ch);
			return FALSE;
		}
		if(IS_SET(rp->room_flags, TUNNEL)) {
			if(MobCountInRoom(rp->people) >= rp->moblim && !IS_IMMORTAL(ch)) {
				send_to_char("Mi spiace, ma non c'e' abbastanza spazio.\n\r",
							 ch);
				return (FALSE);
			}
		}
		if(IS_SET(rp->room_flags, PRIVATE)) {
			if(MobCountInRoom(rp->people) > 2 && !IS_IMMORTAL(ch)) {
				send_to_char("Mi spiace, ma il luogo e' privato.\n\r", ch);
				return (FALSE);
			}
		}
		if(IS_SET(rp->room_flags, INDOORS)) {
			if(MOUNTED(ch)) {
				send_to_char(
					"La tua cavalcatura si rifiuta di entrare in un luogo "
					"chiuso.\n\r", ch);
				return (FALSE);
			}
		}
		/* Added for Pkillers, they cannot enter peaceful rooms
		 Gaia 2001  */
		if(IS_AFFECTED2((IS_POLY(ch)) ? ch->desc->original : ch,
						AFF2_PKILLER) && // SALVO controllo se polato
				IS_SET(rp->room_flags, PEACEFUL)) {
			send_to_char(
				"Una forza arcana ti impedisce di andare in quella direzione. \n\r",
				ch);
			return (FALSE);
		}
		if(IS_SET(rp->room_flags, DEATH)) {
			if(MOUNTED(ch)) {
				send_to_char(
					"La tua cavalcatura si rifiuta di andare da quella "
					"parte.\n\r", ch);
				return (FALSE);
			}
		}
		return (TRUE);
	}
}

int RawMove(struct char_data* ch, int dir, int bCheckSpecial) {
	int need_movement, new_r;
	struct obj_data* obj;
	bool has_boat;
	struct room_data* from_here, *to_here;

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in RawMove (act.move.cpp)");
		return FALSE;
	}

	SetStatus("RawMove started.", nullptr, ch);
	if(bCheckSpecial) {
		SetStatus("Calling special in RawMove");
		if(special(ch, dir + 1, ""))
			/* Check for special routines(North is 1)*/
		{
			SetStatus("return 1 in RawMove");

			return (FALSE);
		}
	}

	SetStatus("Calling ValidMove in RawMove");
	if(!ValidMove(ch, dir)) {
		SetStatus("return 2 in RawMove");
		return (FALSE);
	}

	SetStatus("Checking charmed in RawMove");

	if(IS_AFFECTED(ch, AFF_CHARM) && ch->master != nullptr &&
	   ch->in_room == ch->master->in_room) {
		act("$n scoppia a piangere.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		act("Scoppi a piangere al pensiero di lasciare $N.", FALSE, ch, nullptr, ch->master,
		    TO_CHAR);

		SetStatus("return 3 in RawMove");

		return (FALSE);
	}

	SetStatus("Find rooms in RawMove");
	from_here = real_roomp(ch->in_room);
	if(from_here == nullptr) {
		mudlog(LOG_SYSERR, "RawMove: from_here null per %s", GET_NAME_DESC(ch));
		return FALSE;
	}
	struct room_direction_data* const dirExit = from_here->dir_option[dir];
	if(dirExit == nullptr) {
		mudlog(LOG_SYSERR, "RawMove: dir_option null dir=%d per %s", dir, GET_NAME_DESC(ch));
		return FALSE;
	}
	new_r = dirExit->to_room;
	to_here = real_roomp(new_r);

	if(to_here == nullptr) {
		SetStatus("Moving in room 0 in RawMove");
		char_from_room(ch);
		char_to_room(ch, 0);

		send_to_char(
		    "Il terreno si scioglie sotto i tuoi piedi come tu cadi nel "
		    "turbinante caos.\n\r",
		    ch);
		move_show_look_and_exits(ch);
		SetStatus("return 4 in RawMove");
		return TRUE;
	}

	if(IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE) && (!MOUNTED(ch))) {
		SetStatus("Moving immortal in RawMove");
		char_from_room(ch);
		char_to_room(ch, new_r);
		move_show_look_and_exits(ch);
		SetStatus("return 5 in RawMove");
		return (TRUE);
	}

	/*
	 nail the unlucky with traps. It seems that traps
	 could sometimes crash the mud... I eliminate traps
	 effects for NPC'S Gaia 2001  */

	if(!MOUNTED(ch)) {
		SetStatus("CheckingForMovetrap in RawMove");
		/* Only PCs are affected by traps Gaia 2001
		 if (CheckForMoveTrap(ch, dir))  */

		if(!IS_NPC(ch) && CheckForMoveTrap(ch, dir)) {
			SetStatus("return 6 in RawMove");
			return (FALSE);
		}
	}
	else {
		SetStatus("CheckForMoveTrap while mounted in RawMove");

		/* Only PCs are affected by traps Gaia 2001
		 if (CheckForMoveTrap(MOUNTED(ch), dir))  */

		if(!IS_NPC(ch) && CheckForMoveTrap(MOUNTED(ch), dir)) {
			SetStatus("return 7 in RawMove");
			return (FALSE);
		}
	}

	SetStatus("Find need_movement in RawMove");
	if(IS_AFFECTED(ch, AFF_FLYING)) {
		need_movement = 1;
		if(IS_SET(to_here->room_flags, INDOORS)) {
			need_movement += 2;
		}
	}
	else if(IS_AFFECTED(ch, AFF_TRAVELLING) &&
			!IS_SET(from_here->room_flags, INDOORS)) {
		need_movement = 1;
	}
	else {
		need_movement = (movement_loss[from_here->sector_type]
						 + movement_loss[to_here->sector_type]) / 2;
	}


	/*
	 **   Movement in water_nowswim
	 */

	if((from_here->sector_type == SECT_WATER_NOSWIM)
			|| (to_here->sector_type == SECT_WATER_NOSWIM)) {

		if(!IS_AFFECTED(ch, AFF_FLYING)) {
			if(MOUNTED(ch)) {
				if(!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH) &&
						!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) {
					send_to_char(
						"La tua cavalcatura ha bisogno di saper volare o "
						"nuotare per andare\n\rin quella direzione.\n\r",
						ch);
					SetStatus("return 8 in RawMove");
					return (FALSE);
				}
			}
			else {
				has_boat = FALSE;
				/* See if char is carrying a boat */
				for(obj = ch->carrying; obj; obj = obj->next_content)
					if(obj->obj_flags.type_flag == ITEM_BOAT) {
						has_boat = TRUE;
					}
				if(IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE)) {
					has_boat = TRUE;
				}
				if(!has_boat && !IS_AFFECTED(ch, AFF_WATERBREATH)) {
					send_to_char(
						"Hai bisogno di una barca per andare in quella "
						"direzione.\n\r", ch);
					SetStatus("return 9 in RawMove");
					return (FALSE);
				}
				if(has_boat) {
					need_movement = 1;
				}
			}
		}

	}

	/*
	 * Movement in SECT_AIR
	 */
	if((from_here->sector_type == SECT_AIR)
			|| (to_here->sector_type == SECT_AIR)) {
		if(!IS_AFFECTED(ch, AFF_FLYING)) {
			if((!MOUNTED(ch) || !IS_AFFECTED(MOUNTED(ch), AFF_FLYING))) {
				send_to_char(
					"Hai bisogno di volare per andare da quella parte!\n\r",
					ch);
				SetStatus("return 10 in RawMove");
				return (FALSE);
			}
		}
	}

	/*
	 * Movement in SECT_UNDERWATER
	 */
	if(from_here->sector_type == SECT_UNDERWATER
			|| to_here->sector_type == SECT_UNDERWATER) {
		if(!IS_AFFECTED(ch, AFF_WATERBREATH)) {
			send_to_char("Dovresti avere le branchie per andare da quella "
						 "parte!\n\r", ch);
			SetStatus("return 11 in RawMove");
			return (FALSE);
		}

		if(MOUNTED(ch)) {
			if(!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH)) {
				send_to_char(
					"La tua cavalcatura dovrebbe avere le branchie per "
					"andare da quella parte!\n\r", ch);
				SetStatus("return 12 in RawMove");
				return (FALSE);
			}
		}
	}

	if((from_here->sector_type == SECT_TREE)
			|| (to_here->sector_type == SECT_TREE)) {
		if(!IS_AFFECTED(ch, AFF_TREE_TRAVEL)) {
			send_to_char("Vuoi camminare attraverso agli alberi?\n\r", ch);
			SetStatus("return 13 in RawMove");
			return (FALSE);
		}
		if(MOUNTED(ch)) {
			if(!IS_AFFECTED(MOUNTED(ch), AFF_TREE_TRAVEL)) {
				send_to_char(
					"La tua cavalcatura non puo' camminare attraverso gli "
					"alberi!\n\r", ch);
				SetStatus("return 14 in RawMove");
				return (FALSE);
			}
		}
	}

	if(!MOUNTED(ch)) {
		if(GET_MOVE(ch) < need_movement) {
			act("Sei esaust$b.", FALSE, ch, nullptr, ch, TO_CHAR);
			SetStatus("return 15 in RawMove");
			return (FALSE);
		}
	}
	else if(GET_MOVE(MOUNTED(ch)) < need_movement) {
		send_to_char("La tua cavalcatura e' esausta.\n\r", ch);
		SetStatus("return 16 in RawMove");
		return (FALSE);
	}

	SetStatus("Loosing movement in RawMove");
	if(!IS_IMMORTAL(ch) || MOUNTED(ch)) {
		if(IS_NPC(ch)) {
			GET_MOVE(ch) -= 1;
			alter_move(ch, 0);
		}
		else {
			if(MOUNTED(ch)) {
				GET_MOVE(MOUNTED(ch)) -= need_movement;
				alter_move(ch, 0);
			}
			else {
				GET_MOVE(ch) -= need_movement;
				alter_move(ch, 0);
			}
		}
	}

	if(MOUNTED(ch)) {
		SetStatus("Moving while mounted in RawMove");
		char_from_room(ch);
		char_to_room(ch, new_r);
		char_from_room(MOUNTED(ch));
		char_to_room(MOUNTED(ch), new_r);
	}
	else {
		SetStatus("Moving in RawMove");
		char_from_room(ch);
		char_to_room(ch, new_r);
	}
	SetStatus("Looking in RawMove");
	move_show_look_and_exits(ch);

	if(IS_SET(to_here->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
		SetStatus("Death room in RawMove");
		if(MOUNTED(ch)) {
			NailThisSucker(MOUNTED(ch));
		}
		NailThisSucker(ch);

		SetStatus("return 17 in RawMove");
		return (FALSE);
	}

	/*
	 **  do something with track
	 */

	SetStatus("Checking for tracking in RawMove");
	if(IS_NPC(ch)) {
		if(ch->specials.hunting) {
			if(IS_SET(ch->specials.act, ACT_HUNTING) && ch->desc) {
				WAIT_STATE(ch, PULSE_VIOLENCE);
			} // NPC track
		}
	}
	else {
		if(ch->specials.hunting) {
			if(IS_SET(ch->specials.act, PLR_HUNTING)) {
				send_to_char("Stai cercando un traccia\n\r", ch);
				WAIT_STATE(ch, PULSE_VIOLENCE);
			}
		}
	}
	SetStatus("last return in RawMove");
	return (TRUE);

}
int MoveOne(struct char_data* ch, int dir, int bCheckSpecial) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in MoveOne (act.move.cpp)");
		return FALSE;
	}

	const int was_in = ch->in_room;
	if(!RawMove(ch, dir, bCheckSpecial)) {
		return FALSE;
	}
	DisplayOneMove(ch, dir, was_in);
	return TRUE;
}
void MoveGroup(struct char_data* ch, int dir) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in MoveGroup (act.move.cpp)");
		return;
	}

	const int was_in = ch->in_room;
	if(!RawMove(ch, dir, TRUE)) {
		SetStatus("return in MoveGroup");
		return;
	}

	struct char_data* heap[kMoveGroupHeapMax];
	int totals[kMoveGroupHeapMax];
	int top = 0;

	if(!AddToCharHeap(heap, &top, totals, ch)) {
		DisplayOneMove(ch, dir, was_in);
	}
	SetStatus("Checking followers in MoveGroup");

	for(struct follow_type* k = ch->followers; k != nullptr;) {
		struct follow_type* const next_dude = k->next;
		struct char_data* const follower = k->follower;
		k = next_dude;

		if(follower == nullptr) {
			continue;
		}
		if(was_in != follower->in_room || GET_POS(follower) < POSITION_STANDING) {
			continue;
		}

		act("Segui $N.", FALSE, follower, nullptr, ch, TO_CHAR);
		if(follower->followers != nullptr) {
			MoveGroup(follower, dir);
		}
		else if(RawMove(follower, dir, TRUE)) {
			if(!AddToCharHeap(heap, &top, totals, follower)) {
				DisplayOneMove(follower, dir, was_in);
			}
		}
	}

	move_display_group_batch(heap, top, totals, dir, was_in);
	SetStatus("return in MoveGroup");
}

void DisplayOneMove(struct char_data* ch, int dir, int was_in) {
	if(ch == nullptr) {
		return;
	}
	DisplayMove(ch, dir, was_in, 1);
}

void DisplayGroupMove(struct char_data* ch, int dir, int was_in, int total) {
	if(ch == nullptr) {
		return;
	}
	DisplayMove(ch, dir, was_in, total);
}

ACTION_FUNC(do_move) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_move (act.move.cpp)");
		return;
	}

	SetStatus("do_move started", GET_NAME_DESC(ch), ch);

	if(RIDDEN(ch)) {
		struct char_data* const mount = RIDDEN(ch);
		if(RideCheck(mount, 0)) {
			do_move(mount, arg, cmd);
			SetStatus("Returning from move 1", nullptr);
			return;
		}
		FallOffMount(mount, ch);
		Dismount(mount, ch, POSITION_SITTING);
	}

	const int dir = cmd - 1;

	if(ch->attackers > 1) {
		send_to_char("C'e' troppa gente intorno, non hai spazio per fuggire!\n\r", ch);
		SetStatus("Returning from move 2", nullptr);
		return;
	}

	if(ch->followers == nullptr) {
		MoveOne(ch, dir, TRUE);
	}
	else {
		MoveGroup(ch, dir);
	}
	SetStatus("Returning from move 3", nullptr);
}

/*
 * MoveOne and MoveGroup print messages.  Raw move sends success or failure.
 */

void DisplayMove(struct char_data* ch, int dir, int was_in, int total) {
	if(ch == nullptr || ch->nMagicNumber != CHAR_VALID_MAGIC) {
		return;
	}

	struct room_data* const wasRoom = real_roomp(was_in);
	struct room_data* const hereRoom = real_roomp(ch->in_room);
	if(wasRoom == nullptr || hereRoom == nullptr) {
		mudlog(LOG_SYSERR, "DisplayMove: stanza nulla was=%d here=%d", was_in, ch->in_room);
		return;
	}

	const std::string departureMsg = move_format_departure(ch, dir, total);
	for(struct char_data* observer = wasRoom->people; observer != nullptr;
	    observer = observer->next_in_room) {
		if(!move_observer_sees_departure(ch, observer) || !move_should_announce(ch)) {
			continue;
		}
		std::string msg = departureMsg;
		move_capitalize_message(msg);
		send_to_char(msg.c_str(), observer);
	}

	for(struct char_data* observer = hereRoom->people; observer != nullptr;
	    observer = observer->next_in_room) {
		if(!move_observer_sees_arrival(ch, observer) || !move_should_announce(ch)) {
			continue;
		}
		std::string msg = move_format_arrival(ch, dir, total, observer);
		move_capitalize_message(msg);
		send_to_char(msg.c_str(), observer);
	}
}

int AddToCharHeap(struct char_data* heap[kMoveGroupHeapMax], int* top, int totals[kMoveGroupHeapMax],
				  struct char_data* k) {
	if(top == nullptr || k == nullptr) {
		return FALSE;
	}
	if(*top >= kMoveGroupHeapMax) {
		mudlog(LOG_SYSERR, "AddToCharHeap: heap pieno (%d)", kMoveGroupHeapMax);
		return FALSE;
	}

	if(IS_NPC(k)) {
		for(int i = 0; i < *top; ++i) {
			struct char_data* const rep = heap[i];
			if(rep == nullptr || !IS_NPC(rep) || k->nr != rep->nr) {
				continue;
			}
			if(k->player.short_descr == nullptr || rep->player.short_descr == nullptr) {
				continue;
			}
			if(std::string_view(k->player.short_descr) == rep->player.short_descr) {
				totals[i] += 1;
				return TRUE;
			}
		}
	}

	heap[*top] = k;
	totals[*top] = 1;
	*top += 1;
	return TRUE;
}

int find_door(struct char_data* ch, char* type, char* dir) {
	if(ch == nullptr || type == nullptr || dir == nullptr) {
		return -1;
	}

	struct room_direction_data* exitp = nullptr;

	if(*dir != '\0') {
		const int door = search_block(dir, dirs, FALSE);
		if(door == -1) {
			std::ostringstream os;
			os << '\'' << dir << "' non e' una direzione.\n\r";
			send_to_char(os.str().c_str(), ch);
			return -1;
		}
		exitp = EXIT(ch, door);
		if(door_type_matches_exit(type, exitp)) {
			return door;
		}
		std::ostringstream os;
		os << "Non vedo " << type << ' ' << dirsTo[door] << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		return -1;
	}

	const int doorByDir = search_block(type, dirs, FALSE);
	if(doorByDir != -1) {
		exitp = EXIT(ch, doorByDir);
		if(exitp != nullptr && IS_SET(exitp->exit_info, EX_ISDOOR) &&
		   !IS_SET(exitp->exit_info, EX_SECRET)) {
			return doorByDir;
		}
		std::ostringstream os;
		os << "Non vedi porte " << dirsTo[doorByDir] << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		return -1;
	}

	for(int door = 0; door <= 5; door++) {
		exitp = EXIT(ch, door);
		if(door_type_matches_exit(type, exitp)) {
			return door;
		}
	}

	std::ostringstream os;
	os << "Non vedo " << type << " qui.\n\r";
	send_to_char(os.str().c_str(), ch);
	return -1;
}

/* remove all necessary bits and send messages */
void open_door(struct char_data* ch, int dir) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in open_door (act.move.cpp)");
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "open_door: rp null per %s", GET_NAME_DESC(ch));
		return;
	}

	struct room_direction_data* const exitp = rp->dir_option[dir];
	if(exitp == nullptr) {
		mudlog(LOG_SYSERR, "open_door: exit null dir=%d per %s", dir, GET_NAME_DESC(ch));
		return;
	}

	REMOVE_BIT(exitp->exit_info, EX_CLOSED);
	if(exitp->keyword != nullptr) {
		if(!IS_SET(exitp->exit_info, EX_SECRET)) {
			std::ostringstream os;
			os << "$n apre " << door_art_indef(exitp) << ' ' << fname(exitp->keyword) << '.';
			act(os.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
		}
		else {
			act("$n rivela un passaggio segreto!", FALSE, ch, nullptr, nullptr, TO_ROOM);
		}
	}
	else {
		act("$n apre la porta.", FALSE, ch, nullptr, nullptr, TO_ROOM);
	}

	door_open_other_side(ch, exitp, dir, false);
}

void raw_open_door(struct char_data* ch, int dir) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in raw_open_door (act.move.cpp)");
		return;
	}

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "raw_open_door: rp null per %s", GET_NAME_DESC(ch));
		return;
	}

	struct room_direction_data* const exitp = rp->dir_option[dir];
	if(exitp == nullptr) {
		mudlog(LOG_SYSERR, "raw_open_door: exit null dir=%d per %s", dir, GET_NAME_DESC(ch));
		return;
	}

	REMOVE_BIT(exitp->exit_info, EX_CLOSED);
	door_open_other_side(ch, exitp, dir, true);
}

int canDig(struct char_data* ch) {
	return char_has_item_extra_flag(ch, ITEM_DIG) ? TRUE : FALSE;
}

int canScythe(struct char_data* ch) {
	return char_has_item_extra_flag(ch, ITEM_SCYTHE) ? TRUE : FALSE;
}

ACTION_FUNC(do_open_exit) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_open_exit (act.move.cpp)");
		return;
	}

	const DoorCmdNames names = door_cmd_names(cmd);
	if(names.infinitive == nullptr) {
		mudlog(LOG_ERROR, "Invalid cmd number (%d) in do_open_exit (act.move.cpp)", cmd);
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	argument_interpreter(arg, type.data(), dir.data());

	if(type[0] == '\0') {
		if(cmd == CMD_DIG) {
			do_miner(ch);
			return;
		}
		std::string prompt = std::string("Che cosa vuoi ") + names.infinitive + "?\r\n";
		move_capitalize_message(prompt);
		send_to_char(prompt.c_str(), ch);
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		return;
	}

	struct room_direction_data* const exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Ho paura che questo sia impossibile.\n\r", ch);
		return;
	}
	if(exitp->open_cmd == -1 || cmd != exitp->open_cmd) {
		send_to_char("No-no! Fallo da qualche altra parte, per favore...\n\r", ch);
		return;
	}

	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		if(cmd == CMD_DIG || cmd == CMD_SCYTHE) {
			switch(cmd) {
			case CMD_DIG:
				act("E' inutile scavare ancora.", FALSE, ch, nullptr, nullptr, TO_CHAR);
				break;
			case CMD_SCYTHE:
				act("Non c'e' piu' nulla da tagliare.", FALSE, ch, nullptr, nullptr, TO_CHAR);
				break;
			default:
				mudlog(LOG_ERROR, "Impossible error with cmd = %d in do_open_exit", cmd);
				break;
			}
			return;
		}

		SET_BIT(exitp->exit_info, EX_CLOSED);
		if(IS_SET(exitp->exit_info, EX_SECRET)) {
			std::ostringstream roomOs;
			roomOs << "$n chiude un passaggio segreto " << dirsTo[door] << '.';
			act(roomOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
		}
		else {
			std::ostringstream roomOs;
			roomOs << "$n " << names.thirdPerson << ' ' << door_art_def(exitp)
			       << " $F e chiude il passaggio " << dirsTo[door] << '.';
			act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
		}
		std::ostringstream selfOs;
		selfOs << names.secondPerson << ' ' << door_art_def(exitp) << ' '
		       << door_fname_or(exitp, "passaggio") << " e chiudi il passaggio " << dirsTo[door]
		       << ".\r\n";
		std::string selfMsg = selfOs.str();
		move_capitalize_message(selfMsg);
		send_to_char(selfMsg.c_str(), ch);

		struct room_data* rp = nullptr;
		if(exit_ok(exitp, &rp) && rp != nullptr) {
			struct room_direction_data* const back = rp->dir_option[rev_dir[door]];
			if(back != nullptr && back->to_room == ch->in_room) {
				SET_BIT(back->exit_info, EX_CLOSED);
			}
		}
		return;
	}

	const bool canOperate = (cmd == CMD_DIG && canDig(ch)) || (cmd == CMD_SCYTHE && canScythe(ch)) ||
	                        (cmd != CMD_DIG && cmd != CMD_SCYTHE);
	if(!canOperate) {
		act("Forse usando l'attrezzo adatto...", FALSE, ch, nullptr, nullptr, TO_CHAR);
		if(!IS_SET(exitp->exit_info, EX_SECRET)) {
			std::ostringstream roomOs;
			roomOs << "$n prova a " << names.infinitive << ' ' << door_art_indef(exitp)
			       << " $F a mani nude.";
			act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
		}
		return;
	}

	raw_open_door(ch, door);
	if(IS_SET(exitp->exit_info, EX_SECRET)) {
		std::ostringstream roomOs;
		roomOs << "$n apre un passaggio " << dirsTo[door] << '.';
		act(roomOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	}
	else {
		std::ostringstream roomOs;
		roomOs << "$n " << names.thirdPerson << ' ' << door_art_def(exitp)
		       << " $F ed apre un passaggio " << dirsTo[door] << '.';
		act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
	}
	std::ostringstream selfOs;
	selfOs << names.secondPerson << ' ' << door_art_def(exitp) << ' '
	       << door_fname_or(exitp, "passaggio") << " ed apri un passaggio " << dirsTo[door]
	       << ".\r\n";
	std::string selfMsg = selfOs.str();
	move_capitalize_message(selfMsg);
	send_to_char(selfMsg.c_str(), ch);
}

ACTION_FUNC(do_open) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_open (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	struct obj_data* obj = nullptr;
	struct char_data* victim = nullptr;
	struct room_direction_data* exitp = nullptr;

	argument_interpreter(arg, type.data(), dir.data());

	if(type[0] == '\0') {
		send_to_char("Cosa vuoi aprire?\n\r", ch);
		return;
	}

	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
		if(obj->obj_flags.type_flag != ITEM_CONTAINER) {
			send_to_char("Non e' un contenitore.\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
			send_to_char("Di nuovo?!?\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
			send_to_char("Non puoi farlo.\n\r", ch);
		}
		else if(IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
			send_to_char("Sembra chiuso a chiave.\n\r", ch);
		}
		else {
			REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
			act("Apri $p.", FALSE, ch, obj, nullptr, TO_CHAR);
			act("$n apre $p.", FALSE, ch, obj, nullptr, TO_ROOM);
		}
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		return;
	}

	exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Ho paura che questo sia impossibile.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		std::ostringstream os;
		os << "E' gia' apert" << door_gender_vowel(exitp) << "!\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	if(IS_SET(exitp->exit_info, EX_LOCKED)) {
		std::ostringstream os;
		os << "Sembra chius" << door_gender_vowel(exitp) << " a chiave!\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	if(exitp->open_cmd == -1 || exitp->open_cmd == cmd) {
		open_door(ch, door);
		std::ostringstream os;
		os << "Apri " << door_art_indef(exitp) << ' ' << door_fname_or(exitp, "porta") << '.';
		send_to_char(os.str().c_str(), ch);
		return;
	}
	{
		const char cap = IS_SET(exitp->exit_info, EX_MALE) ? 'O' : 'A';
		std::ostringstream os;
		os << "Non puoi APRIRL" << cap << "!\n\r";
		send_to_char(os.str().c_str(), ch);
	}
}

ACTION_FUNC(do_close) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_close (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	struct obj_data* obj = nullptr;
	struct char_data* victim = nullptr;

	argument_interpreter(arg, type.data(), dir.data());

	if(type[0] == '\0') {
		send_to_char("Cosa vuoi chiudere?\n\r", ch);
		return;
	}

	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
		if(obj->obj_flags.type_flag != ITEM_CONTAINER) {
			send_to_char("Non e' un contenitore.\n\r", ch);
		}
		else if(IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
			send_to_char("Di nuovo?!?!\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
			send_to_char("Questo e' impossibile.\n\r", ch);
		}
		else {
			SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
			send_to_char("Ok.\n\r", ch);
			act("$n chiude $p.", FALSE, ch, obj, nullptr, TO_ROOM);
		}
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		send_to_char("Non posso chiudere niente del genere.\n\r", ch);
		return;
	}

	struct room_direction_data* const exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Questo e' assurdo.\n\r", ch);
		return;
	}
	if(IS_SET(exitp->exit_info, EX_CLOSED)) {
		std::ostringstream os;
		os << "E' gia' chius" << door_gender_vowel(exitp) << "!\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	if(exitp->open_cmd != -1) {
		const char cap = IS_SET(exitp->exit_info, EX_MALE) ? 'O' : 'A';
		std::ostringstream os;
		os << "Non puoi CHIUDERL" << cap << "!\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	SET_BIT(exitp->exit_info, EX_CLOSED);
	if(exitp->keyword != nullptr) {
		if(!IS_SET(exitp->exit_info, EX_SECRET)) {
			std::ostringstream roomOs;
			roomOs << "$n chiude " << door_art_indef(exitp) << ' ' << fname(exitp->keyword)
			       << ' ' << dirsTo[door] << '.';
			act(roomOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
		}
		else {
			act("$n chiude un passaggio segreto.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		}
	}
	else {
		std::ostringstream roomOs;
		roomOs << "$n chiude la porta " << dirsTo[door] << '.';
		act(roomOs.str().c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	}
	{
		std::ostringstream selfOs;
		selfOs << "Chiudi " << door_art_indef(exitp) << ' ' << door_fname_or(exitp, "porta") << '.';
		send_to_char(selfOs.str().c_str(), ch);
	}
	door_close_other_side(ch, exitp, door);
}

int has_key(struct char_data* ch, int key) {
	if(ch == nullptr) {
		return 0;
	}
	for(struct obj_data* o = ch->carrying; o != nullptr; o = o->next_content) {
		if(o->item_number >= 0 && obj_index[o->item_number].iVNum == key) {
			return 1;
		}
	}
	if(ch->equipment[HOLD] != nullptr && ch->equipment[HOLD]->item_number >= 0 &&
	   obj_index[ch->equipment[HOLD]->item_number].iVNum == key) {
		return 1;
	}
	return 0;
}

void raw_unlock_door(struct char_data* ch, struct room_direction_data* exitp, int door) {
	if(ch == nullptr || exitp == nullptr) {
		return;
	}
	REMOVE_BIT(exitp->exit_info, EX_LOCKED);
	struct room_data* const rp = real_roomp(exitp->to_room);
	if(rp == nullptr) {
		mudlog(LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", ch->in_room,
		       exitp->to_room);
		return;
	}
	struct room_direction_data* const back = rp->dir_option[rev_dir[door]];
	if(back != nullptr && back->to_room == ch->in_room) {
		REMOVE_BIT(back->exit_info, EX_LOCKED);
	}
	else {
		mudlog(LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", ch->in_room,
		       exitp->to_room);
	}
}

void raw_lock_door(struct char_data* ch, struct room_direction_data* exitp, int door) {
	if(ch == nullptr || exitp == nullptr) {
		return;
	}
	SET_BIT(exitp->exit_info, EX_LOCKED);
	struct room_data* const rp = real_roomp(exitp->to_room);
	if(rp == nullptr) {
		mudlog(LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", ch->in_room,
		       exitp->to_room);
		return;
	}
	struct room_direction_data* const back = rp->dir_option[rev_dir[door]];
	if(back != nullptr && back->to_room == ch->in_room) {
		SET_BIT(back->exit_info, EX_LOCKED);
	}
	else {
		mudlog(LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", ch->in_room,
		       exitp->to_room);
	}
}

ACTION_FUNC(do_lock) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_lock (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	struct room_direction_data* exitp = nullptr;
	struct obj_data* obj = nullptr;
	struct char_data* victim = nullptr;

	argument_interpreter(arg, type.data(), dir.data());

	if(type[0] == '\0') {
		send_to_char("Cosa vuoi chiudere a chiave?\n\r", ch);
		return;
	}

	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
		if(obj->obj_flags.type_flag != ITEM_CONTAINER) {
			send_to_char("Non e' un contenitore.\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
			std::ostringstream os;
			os << "Devi prima chiudere " << obj->short_description << "!\n\r";
			send_to_char(os.str().c_str(), ch);
		}
		else if(obj->obj_flags.value[2] < 0) {
			send_to_char("Questa cosa non puo' essere chiusa a chiave.\n\r", ch);
		}
		else if(!has_key(ch, obj->obj_flags.value[2])) {
			send_to_char("Sembra che tu non abbia la chiave giusta.\n\r", ch);
		}
		else if(IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
			send_to_char("Ancora? Quante mandate di chiave vuoi dare?!?\n\r", ch);
		}
		else {
			SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			send_to_char("*Cluck*\n\r", ch);
			act("$n chiude a chiave $p.", FALSE, ch, obj, nullptr, TO_ROOM);
		}
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		return;
	}

	exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Questo e' assurdo.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		std::ostringstream os;
		os << "Prima devi chiudere " << door_art_def(exitp) << ' ';
		if(exitp->keyword != nullptr) {
			os << exitp->keyword;
		}
		os << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	if(exitp->key < 0) {
		send_to_char("Non sembra esserci nessuna toppa per la chiave.\n\r", ch);
		return;
	}
	if(!has_key(ch, exitp->key)) {
		send_to_char("Non hai la chiave giusta.\n\r", ch);
		return;
	}
	if(IS_SET(exitp->exit_info, EX_LOCKED)) {
		std::ostringstream os;
		os << (exitp->keyword != nullptr ? exitp->keyword : "La porta") << " e' gia' chius"
		   << door_gender_vowel(exitp) << " a chiave.\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(exitp->keyword != nullptr && std::string_view(fname(exitp->keyword)) != "secret") {
		std::ostringstream roomOs;
		roomOs << "$n chiude a chiave " << door_art_def(exitp) << " $F.";
		act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
	}
	else {
		act("$n chiude a chiave la porta.", FALSE, ch, nullptr, nullptr, TO_ROOM);
	}
	send_to_char("*Click*\n\r", ch);
	raw_lock_door(ch, exitp, door);
}

ACTION_FUNC(do_unlock) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_unlock (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	struct room_direction_data* exitp = nullptr;
	struct obj_data* obj = nullptr;
	struct char_data* victim = nullptr;

	argument_interpreter(arg, type.data(), dir.data());

	if(type[0] == '\0') {
		send_to_char("Che cosa vuoi aprire?\n\r", ch);
		return;
	}

	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
		if(obj->obj_flags.type_flag != ITEM_CONTAINER) {
			send_to_char("Non e' un contenitore.\n\r", ch);
		}
		else if(obj->obj_flags.value[2] < 0) {
			send_to_char("Non ti sembra di trovare un buco per la chiave.\n\r", ch);
		}
		else if(!has_key(ch, obj->obj_flags.value[2])) {
			send_to_char("Non hai la chiave giusta.\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
			std::ostringstream os;
			os << "Per aprire " << obj->short_description << " non serve nessuna chiave.\n\r";
			send_to_char(os.str().c_str(), ch);
		}
		else {
			REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			send_to_char("*Click*\n\r", ch);
			act("$n apre $p.", FALSE, ch, obj, nullptr, TO_ROOM);
		}
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		return;
	}

	exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Questo e' assurdo.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		send_to_char("Ma se non e' nemmeno chiusa!\n\r", ch);
		return;
	}
	if(exitp->key < 0) {
		send_to_char("Non ti sembra di vedere buchi per la chiave.\n\r", ch);
		return;
	}
	if(!has_key(ch, exitp->key)) {
		send_to_char("Non hai la chiave giusta.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_LOCKED)) {
		std::ostringstream os;
		os << (exitp->keyword != nullptr ? exitp->keyword : "La porta") << " non e' chius"
		   << door_gender_vowel(exitp) << " a chiave, dopotutto...\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(exitp->keyword != nullptr && std::string_view(fname(exitp->keyword)) != "secret") {
		std::ostringstream roomOs;
		roomOs << "$n apre " << door_art_def(exitp) << " $F.";
		act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
	}
	else {
		act("$n sblocca la porta.", FALSE, ch, nullptr, nullptr, TO_ROOM);
	}
	send_to_char("*Click*\n\r", ch);
	raw_unlock_door(ch, exitp, door);
}

ACTION_FUNC(do_pick) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_pick (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> type{};
	std::array<char, MAX_INPUT_LENGTH> dir{};
	struct room_direction_data* exitp = nullptr;
	struct obj_data* obj = nullptr;
	struct char_data* victim = nullptr;

	argument_interpreter(arg, type.data(), dir.data());

	const byte percent = number(1, 101);

	if(ch->skills == nullptr) {
		act("Non sei riuscit$b a forzare la serratura.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if(!HasClass(ch, CLASS_THIEF) && !HasClass(ch, CLASS_MONK)) {
		std::ostringstream os;
		os << "Non sei un" << (GET_SEX(ch) == SEX_FEMALE ? "a" : "") << " ladr"
		   << (GET_SEX(ch) == SEX_FEMALE ? "a" : "o") << "!\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(percent > MIN(100, ch->skills[SKILL_PICK_LOCK].learned)) {
		act("Non sei riuscit$b a forzare la serratura.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		LearnFromMistake(ch, SKILL_PICK_LOCK, 0, 90);
		WAIT_STATE(ch, PULSE_VIOLENCE * 4);
		return;
	}

	if(type[0] == '\0') {
		send_to_char("Che cosa vuoi forzare?\n\r", ch);
		return;
	}

	if(generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
		if(obj->obj_flags.type_flag != ITEM_CONTAINER) {
			send_to_char("Non e' un contenitore.\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
			send_to_char("Questo contenitore non e' chiuso!\n\r", ch);
		}
		else if(obj->obj_flags.value[2] < 0) {
			send_to_char("Non sembra esserci una serratura.\n\r", ch);
		}
		else if(!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
			std::ostringstream os;
			os << "Non serve a niente scassinare " << obj->short_description << "!\n\r";
			send_to_char(os.str().c_str(), ch);
		}
		else if(IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
			send_to_char("Non mi pare che tu possa forzare QUESTA serratura.\n\r", ch);
		}
		else {
			REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			send_to_char("*Click*\n\r", ch);
			act("$n ha ragione di $p.", FALSE, ch, obj, nullptr, TO_ROOM);
		}
		return;
	}

	const int door = find_door(ch, type.data(), dir.data());
	if(door < 0) {
		return;
	}

	exitp = EXIT(ch, door);
	if(exitp == nullptr) {
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_ISDOOR)) {
		send_to_char("Questo e' assurdo.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_CLOSED)) {
		send_to_char("Hai appena realizzato che la porta e' aperta.\n\r", ch);
		return;
	}
	if(exitp->key < 0) {
		send_to_char("Non ci sono serrature da forzare.\n\r", ch);
		return;
	}
	if(!IS_SET(exitp->exit_info, EX_LOCKED)) {
		send_to_char("Oh... non e' chiusa a chiave.\n\r", ch);
		return;
	}
	if(IS_SET(exitp->exit_info, EX_PICKPROOF)) {
		send_to_char("Non mi pare che tu possa forzare QUESTA serratura.\n\r", ch);
		return;
	}

	if(exitp->keyword != nullptr) {
		std::ostringstream roomOs;
		roomOs << "$n forza abilmente la serratura di " << door_art_indef(exitp) << " $F.";
		act(roomOs.str().c_str(), FALSE, ch, nullptr, exitp->keyword, TO_ROOM);
	}
	else {
		act("$n forza la serratura.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	}
	send_to_char("La serratura cede velocemente alla tua abilita'.\n\r", ch);
	raw_unlock_door(ch, exitp, door);
}

ACTION_FUNC(do_enter) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_enter (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());

	if(buf[0] != '\0') {
		for(int door = 0; door <= 5; ++door) {
			struct room_direction_data* const exitp = EXIT(ch, door);
			if(exit_ok(exitp, nullptr) && exitp->keyword != nullptr &&
			   isname(buf.data(), exitp->keyword) != 0) {
				do_move(ch, "", ++door);
				return;
			}
		}
		std::ostringstream os;
		os << "Non vedo " << buf.data() << " qui.\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
		send_to_char("Sei gia' all'interno.\n\r", ch);
		return;
	}

	for(int door = 0; door <= 5; ++door) {
		struct room_data* rp = nullptr;
		struct room_direction_data* const exitp = EXIT(ch, door);
		if(exit_ok(exitp, &rp) && !IS_SET(exitp->exit_info, EX_CLOSED) &&
		   IS_SET(rp->room_flags, INDOORS)) {
			do_move(ch, "", ++door);
			return;
		}
	}
	send_to_char("Non c'e' nulla in cui entrare.\n\r", ch);
}

ACTION_FUNC(do_leave) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_leave (act.move.cpp)");
		return;
	}

	if(!IS_SET(RM_FLAGS(ch->in_room), INDOORS)) {
		send_to_char("Sei gia' all'aperto, dove vuoi andare?\n\r", ch);
		return;
	}

	for(int door = 0; door <= 5; ++door) {
		struct room_data* rp = nullptr;
		struct room_direction_data* const exitp = EXIT(ch, door);
		if(exit_ok(exitp, &rp) && !IS_SET(exitp->exit_info, EX_CLOSED) &&
		   !IS_SET(rp->room_flags, INDOORS)) {
			do_move(ch, "", ++door);
			return;
		}
	}
	send_to_char("Non vedo uscite verso l'esterno.\n\r", ch);
}

ACTION_FUNC(do_stand) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_stand (act.move.cpp)");
		return;
	}

	if(affected_by_spell(ch, SKILL_MEMORIZE)) {
		affect_from_char(ch, SKILL_MEMORIZE);
		SpellWearOff(SKILL_MEMORIZE, ch);
	}
	if(affected_by_spell(ch, SKILL_MEDITATE)) {
		affect_from_char(ch, SKILL_MEDITATE);
		SpellWearOff(SKILL_MEDITATE, ch);
	}

	switch(GET_POS(ch)) {
	case POSITION_STANDING:
		act("Sei gia' in piedi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_SITTING:
		act("Ti alzi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n si alza in piedi.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_STANDING;
		break;
	case POSITION_RESTING:
		act("Smetti di riposare e ti alzi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n smette di riposare e si alza in piedi.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_STANDING;
		break;
	case POSITION_SLEEPING:
		act("Dovresti svegliarti, prima!", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_FIGHTING:
		act("Pensavi di poter combattere da sedut$b?", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_MOUNTED:
		send_to_char("Non mentre cavalchi!\n\r", ch);
		break;
	default:
		act("Smetti di fluttare e posi i piedi in terra.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n smette di fluttuare e posa i piedi in terra.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		break;
	}
}

ACTION_FUNC(do_sit) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_sit (act.move.cpp)");
		return;
	}

	switch(GET_POS(ch)) {
	case POSITION_STANDING:
		act("Ti siedi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n si siede.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_SITTING;
		break;
	case POSITION_SITTING:
		act("Sei gia' sedut$b.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_RESTING:
		act("Smetti di riposare e ti siedi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n smette di riposare.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_SITTING;
		break;
	case POSITION_SLEEPING:
		act("Non credi che dovresti svegliarti, prima?", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_FIGHTING:
		act("Sederti mentre combatti ? Ma sei matt$b?", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_MOUNTED:
		send_to_char("Non mentre cavalchi!\n\r", ch);
		break;
	default:
		act("Smetti di fluttuare e ti siedi.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n smette di fluttuare e si siede.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_SITTING;
		break;
	}
}

ACTION_FUNC(do_rest) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_rest (act.move.cpp)");
		return;
	}

	switch(GET_POS(ch)) {
	case POSITION_STANDING:
		act("Ti fermi a riposare le stanche membra.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n si ferma a riposare.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_RESTING;
		break;
	case POSITION_SITTING:
		act("Ti fermi a riposare le stanche membra.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n si ferma a riposare.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_RESTING;
		break;
	case POSITION_RESTING:
		act("Stai gia' riposando.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_SLEEPING:
		act("Prima dovresti svegliarti.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_FIGHTING:
		act("Riposarti mentre combatti? Ma sei matt$b?", FALSE, ch, nullptr, nullptr, TO_CHAR);
		break;
	case POSITION_MOUNTED:
		send_to_char("Non mentre monti!\n\r", ch);
		break;
	default:
		act("Smetti di fluttuare e ti fermi a riposare le stanche membra.", FALSE, ch, nullptr, nullptr,
		    TO_CHAR);
		act("$n smette di fluttuare e si ferma a riposare.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_RESTING;
		break;
	}
}

ACTION_FUNC(do_sleep) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_sleep (act.move.cpp)");
		return;
	}

	switch(GET_POS(ch)) {
	case POSITION_STANDING:
	case POSITION_SITTING:
	case POSITION_RESTING:
		send_to_char("Ti metti a dormire.\n\r", ch);
		act("$n si sdraia e si addormenta.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_SLEEPING;
		break;
	case POSITION_SLEEPING:
		send_to_char("Stai gia' dormendo.\n\r", ch);
		break;
	case POSITION_FIGHTING:
		send_to_char("Pensi che dormire risolva i tuoi problemi?\n\r", ch);
		break;
	case POSITION_MOUNTED:
		send_to_char("Non mentre cavalchi!\n\r", ch);
		break;
	default:
		act("Smetti di fluttuare e ti addormenti.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		act("$n smette di fluttuare e si addormenta.", TRUE, ch, nullptr, nullptr, TO_ROOM);
		GET_POS(ch) = POSITION_SLEEPING;
		break;
	}
}

ACTION_FUNC(do_wake) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wake (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> target{};
	one_argument(arg, target.data());

	if(target[0] != '\0') {
		if(GET_POS(ch) == POSITION_SLEEPING) {
			act("Non puoi svegliare la gente se tu stess$b stai dormendo!", FALSE, ch, nullptr, nullptr,
			    TO_CHAR);
			return;
		}

		struct char_data* const victim = get_char_room_vis(ch, target.data());
		if(victim == nullptr) {
			send_to_char("Non mi sembra che quella persona sia qui.\n\r", ch);
			return;
		}
		if(victim == ch) {
			act("Se vuoi svegliare te stess$b scrivi semplicemente 'wake'", FALSE, ch, nullptr, nullptr,
			    TO_CHAR);
			return;
		}

		if(GET_POS(victim) == POSITION_SLEEPING) {
			if(IS_AFFECTED(victim, AFF_SLEEP)) {
				act("Non puoi svegliare $N!", FALSE, ch, nullptr, victim, TO_CHAR);
			}
			else {
				act("Hai svegliato $N.", FALSE, ch, nullptr, victim, TO_CHAR);
				GET_POS(victim) = POSITION_SITTING;
				act("$n ti sveglia.", FALSE, ch, nullptr, victim, TO_VICT);
				act("$n ha svegliato $N.", TRUE, ch, nullptr, victim, TO_NOTVICT);
			}
			return;
		}
		if(GET_POS(victim) == POSITION_STUNNED) {
			act("$N e' svenut$B.", FALSE, ch, nullptr, victim, TO_CHAR);
			return;
		}
		if(GET_POS(victim) < POSITION_STUNNED) {
			act("$N e' ridott$B proprio male. Non sembra cosciente.", FALSE, ch, nullptr, victim, TO_CHAR);
			return;
		}
		act("$N e' gia' svegli$B.", FALSE, ch, nullptr, victim, TO_CHAR);
		return;
	}

	if(IS_AFFECTED(ch, AFF_SLEEP)) {
		send_to_char("Non riesci a svegliarti!\n\r", ch);
		return;
	}
	if(GET_POS(ch) > POSITION_SLEEPING) {
		act("Sei gia' svegli$b...", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}
	act("Ti sei svegliat$b.", FALSE, ch, nullptr, nullptr, TO_CHAR);
	act("$n si e' svegliat$b.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	GET_POS(ch) = POSITION_SITTING;
}

ACTION_FUNC(do_follow) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_follow (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> name{};
	only_argument(arg, name.data());

	if(name[0] == '\0') {
		send_to_char("Chi e' che vuoi seguire?\n\r", ch);
		return;
	}

	struct char_data* const leader = get_char_room_vis(ch, name.data());
	if(leader == nullptr) {
		send_to_char("Non vedo nessuno con quel nome qui!\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM) && ch->master != nullptr) {
		act("Il tuo unico desiderio e' seguire $N!", FALSE, ch, nullptr, ch->master, TO_CHAR);
		return;
	}

	if(leader == ch) {
		if(ch->master == nullptr) {
			send_to_char("Non stai seguendo nessuno.\n\r", ch);
			return;
		}
		stop_follower(ch);
		return;
	}

	if(circle_follow(ch, leader)) {
		act("Mi spiace, ma quello segue quell'altro, quell'altro segue te...", FALSE, ch, nullptr, nullptr,
		    TO_CHAR);
		return;
	}
	if(ch->master != nullptr) {
		stop_follower(ch);
	}
	if(IS_AFFECTED(ch, AFF_GROUP)) {
		REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
	}
	add_follower(ch, leader);
}

ACTION_FUNC(do_run) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_run (act.move.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buff{};
	only_argument(arg, buff.data());

	if(buff[0] == '\0') {
		send_to_char("In quale direzione vuoi correre?\n\r", ch);
		return;
	}

	const int keyno = search_block(buff.data(), exitKeywords, FALSE);
	if(keyno == -1) {
		act("Mi spiace ma quella non e' una direzione in cui correre.", FALSE, ch, nullptr, nullptr,
		    TO_CHAR);
		return;
	}

	if(GET_MOVE(ch) <= 20) {
		act("Sei troppo stanc$b per correre in questo momento.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM) && ch->master != nullptr && ch->in_room == ch->master->in_room) {
		act("$n scoppia a piangere.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		act("Scoppi a piangere al pensare di corrare via da $N.", FALSE, ch, nullptr, ch->master, TO_CHAR);
		return;
	}

	if(!CAN_GO(ch, keyno)) {
		send_to_char("Non ti pare il caso di correre in quella direzione.\n\r", ch);
		return;
	}
	if(!clearpath(ch, ch->in_room, keyno)) {
		send_to_char("Correre in quella direzione e' inutile.\n\r", ch);
		return;
	}

	struct room_direction_data* const exitdata = real_roomp(ch->in_room)->dir_option[keyno];
	if(exitdata == nullptr || exitdata->to_room == ch->in_room) {
		send_to_char("Correre in quella direzione e' inutile.\n\r", ch);
		return;
	}

	struct char_data* const pkCheck = IS_POLY(ch) ? ch->desc->original : ch;
	if(IS_AFFECTED2(pkCheck, AFF2_PKILLER) && IS_SET(EXIT(ch, keyno)->to_room, PEACEFUL)) {
		send_to_char("Una forza arcana ti impedisce di correre verso quella direzione.", ch);
		return;
	}

	send_to_char("Alzi i tacchi e corri piu' forte che puoi!\n\r", ch);
	act("Improvvisamente $n alza i tacchi e corre piu' veloce che puo'!", FALSE, ch, nullptr, nullptr,
	    TO_ROOM);

	int was_in = ch->in_room;
	while(CAN_GO(ch, keyno) && GET_MOVE(ch) > 20 && RawMove(ch, keyno, TRUE)) {
		DisplayOneMove(ch, keyno, was_in);
		GET_MOVE(ch) -= 1;
		was_in = ch->in_room;
	}
	alter_move(ch, 0);

	if(GET_MOVE(ch) > 25) {
		if(ch->nMagicNumber == CHAR_VALID_MAGIC) {
			act("$n rallenta fino a fermarsi, con il fiatone per la corsa.", FALSE, ch, nullptr, nullptr,
			    TO_ROOM);
		}
		send_to_char("Mi spiace, ma non riesci a correre ulteriormente.\n\r", ch);
	}
	else {
		if(ch->nMagicNumber == CHAR_VALID_MAGIC) {
			act("$n rallenta fino a fermarsi, esaust$b per la corsa.", FALSE, ch, nullptr, nullptr,
			    TO_ROOM);
		}
		act("Sei troppo esaust$b per correre ancora.", FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
}
} // namespace Alarmud

