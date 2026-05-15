/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: auction.c,v 1.3 2002/03/17 16:48:47 Thunder Exp $
 */
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>
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
#include "cmdid.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "modify.hpp"
#include "utility.hpp"

namespace Alarmud {

#define SELLER 0 // SALVO ruolo is_present (venditore)
#define BUYER 1  // SALVO ruolo is_present (compratore)

AUCTION_DATA* auction;

static bool auction_ensure(const char* ctx) {
	if(auction == nullptr) {
		mudlog(LOG_SYSERR, "auction==nullptr in %s (auction.cpp)", ctx);
		return false;
	}
	return true;
}

static void auc_copy_name(char (&dest)[20], const char* src) {
	if(src == nullptr) {
		dest[0] = '\0';
		return;
	}
	std::snprintf(dest, sizeof(dest), "%.*s", static_cast<int>(sizeof(dest) - 1), src);
}

static void auc_set_none(char (&dest)[20]) {
	std::snprintf(dest, sizeof(dest), "(nessuno)");
}

static bool linkdead_room3_ok(struct char_data* c) {
	return c != nullptr && !(IS_LINKDEAD(c) && c->in_room == 3);
}

/** Allinea seller/buyer al PG connesso di nome `nome`; ritorna false se assente. */
static bool is_present(int role, const char* nome) {
	if(!auction_ensure("is_present")) {
		return false;
	}
	if(nome == nullptr) {
		return false;
	}

	if(role == SELLER) {
		if(auction->seller != nullptr && linkdead_room3_ok(auction->seller) &&
		   std::strcmp(GET_NAME(auction->seller), nome) == 0) {
			return true;
		}
	}
	else {
		if(auction->buyer != nullptr && linkdead_room3_ok(auction->buyer) &&
		   std::strcmp(GET_NAME(auction->buyer), nome) == 0) {
			return true;
		}
	}

	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		if(d->character == nullptr || d->character->player.name == nullptr) {
			continue;
		}
		struct char_data* const who = d->original != nullptr ? d->original : d->character;
		if(std::strcmp(GET_NAME(who), nome) != 0) {
			continue;
		}
		if(role == SELLER) {
			auction->seller = d->character;
		}
		else {
			auction->buyer = d->character;
		}
		return true;
	}

	if(role == SELLER) {
		auction->seller = nullptr;
	}
	else {
		auction->buyer = nullptr;
	}
	return false;
}

static const char* auc_short_descr(struct obj_data* o) {
	return (o != nullptr && o->short_description != nullptr) ? o->short_description : "?";
}

static bool auc_reboot_denied(struct char_data* ch) {
	if(IS_IMMORTAL(ch) || !auction_blocked_near_reboot()) {
		return false;
	}
	send_to_char("Non puoi usare l'asta: il reboot e' previsto entro dieci minuti.\n\r", ch);
	return true;
}

static int advatoi(const char* s);
static int parsebet(int currentbet, const char* argument);

ACTION_FUNC(do_auction_int) {
	OBJ_DATA* obj = nullptr;
	std::array<char, MAX_INPUT_LENGTH> arg1{};

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_auction_int (auction.cpp)");
		return;
	}
	if(arg == nullptr) {
		mudlog(LOG_SYSERR, "arg==nullptr in do_auction_int (auction.cpp)");
		return;
	}
	if(!auction_ensure("do_auction_int")) {
		return;
	}

	arg = one_argument(arg, arg1.data());

	if(IS_NPC(ch)) {
		return;
	}

	if(arg1[0] == '\0') {
		if(auction->item != nullptr) {
			std::ostringstream os;
			if(auction->bet > 0) {
				os << "Offerta corrente, fatta da " << auction->real_buyer << ", per "
				   << auc_short_descr(auction->item) << " e' " << auction->bet
				   << " monete.\n\rVenditore: " << auction->real_seller << ".\n\r";
			}
			else {
				os << auc_short_descr(auction->item)
				   << " non ha ricevuto offerte.\n\rVenditore: " << auction->real_seller << ".\n\r";
				if(auction->opening_reserve > 0) {
					os << "Prezzo minimo richiesto per la prima offerta: " << auction->opening_reserve
					   << " monete.\n\r";
				}
			}
			const std::string msg = os.str();
			send_to_char(msg.c_str(), ch);
			if(IS_IMMORTAL(ch)) {
				spell_identify(10, ch, nullptr, auction->item);
			}
			return;
		}
		send_to_char("Mettere all'asta COSA?\n\r"
		              "Sintassi: auction <nome-oggetto> [prezzo-minimo-prima-offerta]\n\r", ch);
		return;
	}

	if(IS_IMMORTAL(ch) &&
	   (!str_cmp(arg1.data(), "purge") || !str_cmp(arg1.data(), "stop") || !str_cmp(arg1.data(), "halt"))) {
		if(auction->item == nullptr) {
			send_to_char("Non c'e' nessuna asta in corso da fermare.\n\r", ch);
			return;
		}
		std::ostringstream os;
		if(!str_cmp(arg1.data(), "stop") || !str_cmp(arg1.data(), "halt")) {
			os << "La vendita di " << auc_short_descr(auction->item)
			   << " e' stata annullata da un Dio. Oggetto confiscato.";
			mudlog(LOG_CHECK, "ASTA: vendita di (%s) annullata dal Dio %s", auction->item->name, GET_NAME(ch));
			obj_to_char(auction->item, ch);
		}
		else {
			os << "Spiacente, la vendita di " << auc_short_descr(auction->item)
			   << " e' stata revocata. Oggetto distrutto.";
			mudlog(LOG_CHECK, "ASTA: vendita di (%s) revocata dal Dio %s", auction->item->name, GET_NAME(ch));
			extract_obj(auction->item);
		}
		const std::string immMsg = os.str();
		const int refundBet = auction->bet;
		is_present(SELLER, auction->real_seller);
		talk_auction(immMsg.c_str());
		if(refundBet > 0 && is_present(BUYER, auction->real_buyer)) {
			GET_GOLD(auction->buyer) += refundBet;
			{
				std::ostringstream payOs;
				payOs << "Il banditore d'asta appare e ti restituisce " << refundBet << " monete.\n\r";
				send_to_char(payOs.str().c_str(), auction->buyer);
			}
			act("Il banditore d'asta appare e restituisce le monete a $n.", FALSE, auction->buyer, nullptr, nullptr,
			    TO_ROOM);
		}
		auction->item = nullptr;
		auction->bet = 0;
		auction->opening_reserve = 0;
		auction->buyer = nullptr;
		auc_set_none(auction->real_buyer);
		auction->seller = nullptr;
		auc_set_none(auction->real_seller);
		return;
	}

	if(cmd == CMD_BID) {
		if(auc_reboot_denied(ch)) {
			return;
		}
		if(auction->item != nullptr) {
			if(arg1[0] == '\0') {
				send_to_char("Quanto vuoi offrire?\n\r", ch);
				return;
			}

			const int newbet = parsebet(auction->bet, arg1.data());

			if(auction->bet > 0 &&
			   (auction->buyer == ch || str_cmp(auction->real_buyer, GET_NAME(ch)) == 0)) {
				send_to_char("Sei gia' il miglior offerente; attendi una controfferta.\n\r", ch);
				return;
			}

			const int minNext = (auction->bet == 0)
			                        ? std::max(100, auction->opening_reserve)
			                        : auction->bet + 100;
			if(newbet <= auction->bet || newbet < minNext) {
				if(auction->bet == 0) {
					std::ostringstream err;
					err << "La prima offerta deve essere di almeno " << minNext << " monete.\n\r";
					send_to_char(err.str().c_str(), ch);
				}
				else {
					send_to_char("Devi impegnare almeno 100 monete oltre l'offerta attuale.\n\r", ch);
				}
				return;
			}

			if(newbet > GET_GOLD(ch)) {
				send_to_char("Non hai abbastanza soldi!\n\r", ch);
				return;
			}

			mudlog(LOG_PLAYERS, "ASTA: %s offre %d monete per (%s)", GET_NAME(ch), newbet, auction->item->name);

			if(is_present(BUYER, auction->real_buyer)) {
				GET_GOLD(auction->buyer) += auction->bet;
			}

			GET_GOLD(ch) -= newbet;
			auction->buyer = ch;
			auc_copy_name(auction->real_buyer, GET_NAME(ch));
			auction->bet = newbet;
			auction->going = 0;
			auction->pulse = AUCTION_WAIT_TICKS;

			std::ostringstream bidOs;
			bidOs << "Un offerta di " << newbet << " monete e' stata ricevuta per "
			      << auc_short_descr(auction->item) << ".\n\r";
			const std::string bidMsg = bidOs.str();
			is_present(SELLER, auction->real_seller);
			talk_auction(bidMsg.c_str());
			return;
		}
		send_to_char("Al momento non c'e' niente in vendita.\n\r", ch);
		return;
	}

	obj = get_obj_in_list(arg1.data(), ch->carrying);

	if(obj == nullptr) {
		send_to_char("Non lo porti.\n\r", ch);
		return;
	}

	if(auction->item == nullptr) {
		if(auc_reboot_denied(ch)) {
			return;
		}
		switch(GET_ITEM_TYPE(obj)) {
		default:
			act("Non puoi vendere all'asta $T.", FALSE, ch, nullptr, obj->short_description, TO_CHAR);
			return;

		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_STAFF:
		case ITEM_WAND:
		case ITEM_SCROLL: {
			std::array<char, MAX_INPUT_LENGTH> priceTok{};
			one_argument(arg, priceTok.data());
			int openingReserve = 0;
			if(priceTok[0] != '\0') {
				char* parseEnd = nullptr;
				errno = 0;
				const long pr = std::strtol(priceTok.data(), &parseEnd, 10);
				if(parseEnd == priceTok.data() || (parseEnd != nullptr && *parseEnd != '\0') ||
				   errno == ERANGE || pr < 0 || pr > static_cast<long>(std::numeric_limits<int>::max())) {
					send_to_char(
					    "Prezzo minimo non valido: dopo il nome oggetto indica un intero non negativo "
					    "(es. 500), o niente per usare il minimo predefinito (100 monete).\n\r",
					    ch);
					return;
				}
				openingReserve = static_cast<int>(pr);
			}

			obj_from_char(obj);
			auction->item = obj;
			auction->bet = 0;
			auction->opening_reserve = openingReserve;
			auction->buyer = nullptr;
			auc_set_none(auction->real_buyer);
			auction->seller = ch;
			auc_copy_name(auction->real_seller, GET_NAME(ch));
			auction->pulse = AUCTION_WAIT_TICKS;
			auction->going = 0;

			{
				std::ostringstream startOs;
				startOs << "Nuovo lotto all'asta: " << auc_short_descr(obj) << ".";
				if(openingReserve > 0) {
					startOs << " Prima offerta da almeno " << openingReserve << " monete.";
				}
				talk_auction(startOs.str().c_str());
			}
			mudlog(LOG_PLAYERS, "ASTA: %s vende (%s)", auction->real_seller, auction->item->name);
			return;
		}
		}
	}

	act("Riprova piu' tardi - attualmente e' in corso l'asta per $p!", FALSE, ch, auction->item, nullptr,
	    TO_CHAR);
}

void auction_update(void) {
	if(!auction_ensure("auction_update")) {
		return;
	}
	if(auction->item == nullptr) {
		return;
	}

	auction->pulse--;
	if(auction->pulse > 0) {
		return;
	}

	auction->pulse = AUCTION_CALL_TICKS;
	switch(++auction->going) {
	case 1:
	case 2: {
		std::ostringstream os;
		const char* const chiamata =
		    (auction->going == 1) ? "Prima chiamata" : "Seconda chiamata";
		if(auction->bet > 0) {
			os << auc_short_descr(auction->item) << " - " << chiamata << ", offerta corrente "
			   << auction->bet << " monete.";
		}
		else {
			os << auc_short_descr(auction->item) << " - " << chiamata << "; nessuna offerta";
			if(auction->opening_reserve > 0) {
				os << " (minimo prima offerta: " << auction->opening_reserve << " monete)";
			}
			os << '.';
		}
		is_present(SELLER, auction->real_seller);
		talk_auction(os.str().c_str());
		break;
	}

	case 3:
		if(auction->bet > 0) {
			if(is_present(BUYER, auction->real_buyer)) {
				std::ostringstream soldOs;
				soldOs << auc_short_descr(auction->item) << " aggiudicata a " << GET_NAME(auction->buyer)
				       << " per " << auction->bet << " monete.";
				is_present(SELLER, auction->real_seller);
				talk_auction(soldOs.str().c_str());
				mudlog(LOG_PLAYERS, "ASTA: %s compra (%s) per %d monete", GET_NAME(auction->buyer),
				       auction->item->name, auction->bet);
				obj_to_char(auction->item, auction->buyer);
				act("Il banditore d'asta appare in un soffio di fumo e ti mette nelle mani $p.", FALSE,
				    auction->buyer, auction->item, nullptr, TO_CHAR);
				act("Il banditore d'asta appare e consegna $p a $n.", FALSE, auction->buyer, auction->item,
				    nullptr, TO_ROOM);
				if(is_present(SELLER, auction->real_seller)) {
					mudlog(LOG_PLAYERS, "ASTA: %s riceve %d per la vendita di (%s)", GET_NAME(auction->seller),
					       auction->bet, auction->item->name);
					GET_GOLD(auction->seller) += auction->bet;
					{
						std::ostringstream payOs;
						payOs << "Il banditore d'asta appare e ti consegna " << auction->bet
						      << " monete per la vendita di " << auc_short_descr(auction->item) << ".\n\r";
						send_to_char(payOs.str().c_str(), auction->seller);
					}
					act("Il banditore d'asta appare e paga $n per la vendita di $p.", FALSE, auction->seller,
					    auction->item, nullptr, TO_ROOM);
				}
				else {
					mudlog(LOG_CHECK, "ASTA: venditore assente, non prende %d monete dalla vendita",
					       auction->bet);
				}
				auction->item = nullptr;
				auction->bet = 0;
				auction->opening_reserve = 0;
			}
			else {
				static constexpr char kInvalid[] = "Asta non valida.";
				if(is_present(SELLER, auction->real_seller)) {
					mudlog(LOG_CHECK, "ASTA: compratore assente, (%s) restituito a %s", auction->item->name,
					       GET_NAME(auction->seller));
					obj_to_char(auction->item, auction->seller);
				}
				else {
					mudlog(LOG_CHECK, "ASTA: compratore e venditore assenti, (%s) rimosso",
					       auction->item->name);
					extract_obj(auction->item);
				}
				auction->item = nullptr;
				auction->bet = 0;
				auction->opening_reserve = 0;
				talk_auction(kInvalid);
			}
		}
		else {
			std::ostringstream endOs;
			endOs << "Asta terminata senza offerte per " << auc_short_descr(auction->item) << ".";
			const std::string endMsg = endOs.str();
			if(is_present(SELLER, auction->real_seller)) {
				mudlog(LOG_PLAYERS, "ASTA: nessuna offerta, (%s) restituito a %s", auction->item->name,
				       GET_NAME(auction->seller));
				obj_to_char(auction->item, auction->seller);
				talk_auction(endMsg.c_str());
				act("Il banditore d'asta appare e ti restituisce $p.", FALSE, auction->seller, auction->item,
				    nullptr, TO_CHAR);
				act("Il banditore d'asta appare e restituisce $p a $n.", FALSE, auction->seller, auction->item,
				    nullptr, TO_ROOM);
			}
			else {
				mudlog(LOG_CHECK, "ASTA: venditore assente e nessuna offerta, (%s) rimosso",
				       auction->item->name);
				extract_obj(auction->item);
				talk_auction(endMsg.c_str());
			}
			auction->item = nullptr;
			auction->bet = 0;
			auction->opening_reserve = 0;
			auction->buyer = nullptr;
			auc_set_none(auction->real_buyer);
			auction->seller = nullptr;
			auc_set_none(auction->real_seller);
		}
		break;

	default:
		break;
	}
}

/*
  advatoi / parsebet: logica originale (numeri con suffissi k/m, +percentuale, moltiplicatore).
*/

static int advatoi(const char* s) {
	if(s == nullptr) {
		return 0;
	}

	std::array<char, MAX_INPUT_LENGTH> buffer{};
	std::strncpy(buffer.data(), s, buffer.size() - 1);
	buffer[buffer.size() - 1] = '\0';

	const char* stringptr = buffer.data();
	int number = 0;
	int multiplier = 0;

	while(std::isdigit(static_cast<unsigned char>(*stringptr)) != 0) {
		number = (number * 10) + (*stringptr - '0');
		stringptr++;
	}

	switch(UPPER(*stringptr)) {
	case 'K':
		multiplier = 1000;
		number *= multiplier;
		stringptr++;
		break;
	case 'M':
		multiplier = 1000000;
		number *= multiplier;
		stringptr++;
		break;
	case '\0':
		break;
	default:
		return 0;
	}

	while(std::isdigit(static_cast<unsigned char>(*stringptr)) != 0 && multiplier > 1) {
		multiplier /= 10;
		number += (*stringptr - '0') * multiplier;
		stringptr++;
	}

	if(*stringptr != '\0' && std::isdigit(static_cast<unsigned char>(*stringptr)) == 0) {
		return 0;
	}

	return number;
}

static int parsebet(int currentbet, const char* argument) {
	int newbet = 0;

	if(argument == nullptr) {
		return 0;
	}

	std::array<char, MAX_INPUT_LENGTH> buffer{};
	std::strncpy(buffer.data(), argument, buffer.size() - 1);
	buffer[buffer.size() - 1] = '\0';
	char* stringptr = buffer.data();

	if(*stringptr == '\0') {
		return 0;
	}

	if(std::isdigit(static_cast<unsigned char>(*stringptr)) != 0) {
		newbet = advatoi(stringptr);
	}
	else if(*stringptr == '+') {
		if(std::strlen(stringptr) == 1) {
			newbet = (currentbet * 125) / 100;
		}
		else {
			newbet = (currentbet * (100 + std::atoi(stringptr + 1))) / 100;
		}
	}
	else if((*stringptr == '*') || (*stringptr == 'x')) {
		if(std::strlen(stringptr) == 1) {
			newbet = currentbet * 2;
		}
		else {
			newbet = currentbet * std::atoi(stringptr + 1);
		}
	}

	return newbet;
}

} // namespace Alarmud
