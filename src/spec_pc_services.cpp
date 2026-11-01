/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
#include "spec_pc_services.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "inventory_overflow.hpp"
#include "utility.hpp"

namespace Alarmud {

namespace {

bool overflow_claim_can_inspect_other(struct char_data* ch) {
	return ch && !IS_NPC(ch) && IS_QUESTMASTER(ch);
}

const char* overflow_claim_ctx_get(struct char_data* ch) {
	if(!ch || ch->specials.overflow_claim_ctx[0] == '\0') {
		return nullptr;
	}
	return ch->specials.overflow_claim_ctx;
}

void overflow_claim_ctx_clear(struct char_data* ch) {
	if(ch) {
		ch->specials.overflow_claim_ctx[0] = '\0';
	}
}

void overflow_claim_ctx_set(struct char_data* ch, const char* name) {
	if(!ch) {
		return;
	}
	if(!name || name[0] == '\0') {
		overflow_claim_ctx_clear(ch);
		return;
	}
	std::strncpy(ch->specials.overflow_claim_ctx, name,
				 sizeof(ch->specials.overflow_claim_ctx) - 1);
	ch->specials.overflow_claim_ctx[sizeof(ch->specials.overflow_claim_ctx) - 1] = '\0';
}

void overflow_claim_show_list(struct char_data* ch, struct char_data* mob, const char* owner_name) {
	const bool staff = overflow_claim_can_inspect_other(ch);
	const bool other = owner_name && owner_name[0];

	std::string resolved;
	std::vector<InventoryOverflowListItem> items;
	const bool listed =
		inventory_overflow_list(ch, items, owner_name, other ? &resolved : nullptr);
	const char* display =
		(other && !resolved.empty()) ? resolved.c_str() : owner_name;

	if(staff) {
		if(other && display && display[0]) {
			overflow_claim_ctx_set(ch, display);
		}
		else {
			overflow_claim_ctx_clear(ch);
		}
	}

	if(!listed || items.empty()) {
		if(other) {
			act("$N ti dice 'Non ho nulla in deposito a quel nome.'", FALSE, ch, nullptr, mob,
				TO_CHAR);
			if(staff && display && display[0]) {
				char note[192];
				std::snprintf(note, sizeof(note),
							  "$c0008Contesto staff: deposito di $c0015%s$c0008. "
							  "buy/value <num> agiscono su quel deposito "
							  "(list senza nome = tuo).$c0007\n\r",
							  display);
				send_to_char(note, ch);
			}
		}
		else {
			act("$N ti dice 'Non ho nulla in deposito a tuo nome.'", FALSE, ch, nullptr, mob,
				TO_CHAR);
			if(staff) {
				send_to_char(
					"$c0008Contesto staff: il tuo deposito "
					"(list <nome> per ispezionare un altro PG).$c0007\n\r",
					ch);
			}
		}
		return;
	}
	if(other) {
		char head[128];
		std::snprintf(head, sizeof(head),
					  "$N ti mostra il deposito di $c0015%s$c0007:", display);
		act(head, FALSE, ch, nullptr, mob, TO_CHAR);
	}
	else {
		act("$N ti mostra il registro del deposito:", FALSE, ch, nullptr, mob, TO_CHAR);
	}
	send_to_char(
		"$c0014Num$c0007  $c0015Pz$c0007  $c0011   Prezzo$c0007  $c0007Descrizione\n\r"
		"$c0008---- ---- --------- ------------------------------$c0007\n\r",
		ch);

	long total_price = 0;
	int total_pieces = 0;
	char line[384];
	for(size_t i = 0; i < items.size(); ++i) {
		const InventoryOverflowListItem& it = items[i];
		total_price += it.price * static_cast<long>(it.item_count);
		total_pieces += it.item_count;
		std::string desc = it.short_desc;
		if(it.is_limited) {
			desc += " $c0001[RARO]$c0007";
		}
		if(desc.size() > 96) {
			desc.resize(96);
		}
		std::snprintf(line, sizeof(line),
					  "$c0014%3zu$c0007 $c0015%4d$c0007 $c0011%9ld$c0007  %s\n\r", i + 1,
					  it.item_count, it.price, desc.c_str());
		send_to_char(line, ch);
	}
	if(staff && other) {
		std::snprintf(line, sizeof(line),
					  "$c0008----$c0007\n\r"
					  "$c0007Lotti: $c0014%zu$c0007  Pezzi: $c0015%d$c0007  Totale ritiro: "
					  "$c0011%ld$c0007 monete\n\r"
					  "$c0008Contesto staff: deposito di $c0015%s$c0008.\n\r"
					  "Usa: buy <num>  |  buy <num>*<qty>  |  value <num>  "
					  "(su quel deposito)\n\r"
					  "Override: buy/value <nome> <num>  |  list / list all.$c0007\n\r",
					  items.size(), total_pieces, total_price, display);
	}
	else if(staff) {
		std::snprintf(line, sizeof(line),
					  "$c0008----$c0007\n\r"
					  "$c0007Lotti: $c0014%zu$c0007  Pezzi: $c0015%d$c0007  Totale ritiro: "
					  "$c0011%ld$c0007 monete\n\r"
					  "$c0008Usa: buy <num>  |  buy <num>*<qty>  |  value <num>\n\r"
					  "Staff: list all  |  list <nome> (contesto) → buy/value <num>.$c0007\n\r",
					  items.size(), total_pieces, total_price);
	}
	else {
		std::snprintf(line, sizeof(line),
					  "$c0008----$c0007\n\r"
					  "$c0007Lotti: $c0014%zu$c0007  Pezzi: $c0015%d$c0007  Totale ritiro: "
					  "$c0011%ld$c0007 monete\n\r"
					  "$c0008Usa: buy <num>  |  buy <num>*<qty>  |  value <num>$c0007\n\r",
					  items.size(), total_pieces, total_price);
	}
	send_to_char(line, ch);
}

/** true se token e' interamente numerico (indice listino). */
bool overflow_token_is_index(const char* tok) {
	if(!tok || !*tok) {
		return false;
	}
	for(const char* p = tok; *p; ++p) {
		if(!std::isdigit(static_cast<unsigned char>(*p))) {
			return false;
		}
	}
	return true;
}

/**
 * Parse: [nome] <num>[*<qty>]
 * owner_out vuoto = proprio deposito.
 */
bool overflow_claim_parse_args(struct char_data* ch, const char* arg, std::string& owner_out,
							   size_t& index_out, int& qty_out) {
	owner_out.clear();
	index_out = 0;
	qty_out = 1;
	if(!ch || !arg) {
		return false;
	}
	char buf[MAX_INPUT_LENGTH];
	const char* rest = one_argument(arg, buf);
	if(buf[0] == '\0') {
		return false;
	}

	char index_tok[MAX_INPUT_LENGTH];
	if(overflow_token_is_index(buf) || std::strchr(buf, '*') != nullptr) {
		std::strncpy(index_tok, buf, sizeof(index_tok) - 1);
		index_tok[sizeof(index_tok) - 1] = '\0';
	}
	else {
		if(!overflow_claim_can_inspect_other(ch)) {
			return false;
		}
		owner_out = buf;
		rest = one_argument(rest, buf);
		if(buf[0] == '\0') {
			return false;
		}
		std::strncpy(index_tok, buf, sizeof(index_tok) - 1);
		index_tok[sizeof(index_tok) - 1] = '\0';
	}

	char* star = std::strchr(index_tok, '*');
	if(star) {
		*star = '\0';
		const char* qty_s = star + 1;
		if(index_tok[0] == '\0' || qty_s[0] == '\0' || !overflow_token_is_index(index_tok) ||
		   !overflow_token_is_index(qty_s)) {
			return false;
		}
		index_out = static_cast<size_t>(std::strtoull(index_tok, nullptr, 10));
		qty_out = static_cast<int>(std::strtol(qty_s, nullptr, 10));
	}
	else {
		if(!overflow_token_is_index(index_tok)) {
			return false;
		}
		index_out = static_cast<size_t>(std::strtoull(index_tok, nullptr, 10));
		qty_out = 1;
	}
	if(index_out == 0 || qty_out <= 0) {
		return false;
	}
	return true;
}

/** Solo nome (list <nome>) oppure vuoto. */
bool overflow_claim_parse_list_owner(struct char_data* ch, const char* arg, std::string& owner_out) {
	owner_out.clear();
	if(!arg) {
		return true;
	}
	char buf[MAX_INPUT_LENGTH];
	one_argument(arg, buf);
	if(buf[0] == '\0') {
		return true;
	}
	if(!overflow_claim_can_inspect_other(ch)) {
		return false;
	}
	owner_out = buf;
	return true;
}

void overflow_claim_show_owners(struct char_data* ch, struct char_data* mob) {
	std::vector<InventoryOverflowOwnerCount> owners;
	if(!inventory_overflow_list_owners(ch, owners)) {
		act("$N ti dice 'Non riesco a consultare i depositi.'", FALSE, ch, nullptr, mob, TO_CHAR);
		return;
	}
	if(owners.empty()) {
		act("$N ti dice 'Nessun deposito overflow attivo.'", FALSE, ch, nullptr, mob, TO_CHAR);
		send_to_char(
			"$c0008Staff: list <nome> per un PG; list senza nome = tuo.$c0007\n\r", ch);
		return;
	}
	act("$N ti mostra chi ha pezzi in deposito:", FALSE, ch, nullptr, mob, TO_CHAR);
	send_to_char(
		"$c0015Nome$c0007                             $c0014Pezzi$c0007\n\r"
		"$c0008-------------------------------- ----$c0007\n\r",
		ch);
	char line[160];
	int total = 0;
	for(const InventoryOverflowOwnerCount& o : owners) {
		total += o.item_count;
		std::snprintf(line, sizeof(line), "$c0015%-32s$c0007 $c0014%4d$c0007\n\r", o.name.c_str(),
					  o.item_count);
		send_to_char(line, ch);
	}
	std::snprintf(line, sizeof(line),
				  "$c0008----$c0007\n\r"
				  "$c0007PG: $c0015%zu$c0007  Pezzi: $c0014%d$c0007\n\r"
				  "$c0008Poi: list <nome> (imposta contesto) → buy/value <num>.$c0007\n\r",
				  owners.size(), total);
	send_to_char(line, ch);
}

} // namespace

MOBSPECIAL_FUNC(OverflowClaim) {
	if(type != EVENT_COMMAND || ch == nullptr || IS_NPC(ch)) {
		return FALSE;
	}
	if(cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_VALUE) {
		return FALSE;
	}
	if(!AWAKE(ch) || check_soundproof(ch)) {
		return FALSE;
	}

	struct char_data* clerk = mob;
	if(clerk == nullptr) {
		return FALSE;
	}

	if(cmd == CMD_LIST) {
		std::string owner;
		if(!overflow_claim_parse_list_owner(ch, arg, owner)) {
			act("$N ti dice 'Usa <list>.'", FALSE, ch, nullptr, clerk, TO_CHAR);
			return TRUE;
		}
		if(!owner.empty() && str_cmp(owner.c_str(), "all") == 0) {
			overflow_claim_show_owners(ch, clerk);
			return TRUE;
		}
		overflow_claim_show_list(ch, clerk, owner.empty() ? nullptr : owner.c_str());
		return TRUE;
	}

	std::string owner;
	size_t index = 0;
	int qty = 1;
	if(!overflow_claim_parse_args(ch, arg, owner, index, qty)) {
		if(cmd == CMD_VALUE) {
			act("$N ti dice 'Cosa vuoi valutare? Usa value <numero>.'", FALSE, ch, nullptr, clerk,
				TO_CHAR);
		}
		else {
			act("$N ti dice 'Cosa vuoi ritirare? Usa buy <numero> o buy <numero>*<qty>.'", FALSE,
				ch, nullptr, clerk, TO_CHAR);
		}
		return TRUE;
	}
	/* Staff: buy/value senza nome usano l'ultimo list <nome>. */
	if(overflow_claim_can_inspect_other(ch) && owner.empty()) {
		if(const char* ctx = overflow_claim_ctx_get(ch)) {
			owner = ctx;
		}
	}
	const char* owner_arg = owner.empty() ? nullptr : owner.c_str();

	std::string resolved;
	std::vector<InventoryOverflowListItem> items;
	if(!inventory_overflow_list(ch, items, owner_arg,
								owner_arg ? &resolved : nullptr) ||
	   items.empty()) {
		act("$N ti dice 'Non trovo quel pezzo nel deposito.'", FALSE, ch, nullptr, clerk, TO_CHAR);
		return TRUE;
	}
	if(overflow_claim_can_inspect_other(ch) && !resolved.empty()) {
		owner = resolved;
		overflow_claim_ctx_set(ch, owner.c_str());
		owner_arg = owner.c_str();
	}
	if(index < 1 || index > items.size()) {
		act("$N ti dice 'Non trovo quel pezzo nel deposito.'", FALSE, ch, nullptr, clerk, TO_CHAR);
		return TRUE;
	}
	const InventoryOverflowListItem& chosen = items[index - 1];
	if(chosen.overflow_ids.empty() || chosen.price <= 0) {
		act("$N ti dice 'Non trovo quel pezzo nel deposito.'", FALSE, ch, nullptr, clerk, TO_CHAR);
		return TRUE;
	}
	const std::string desc_raw =
		chosen.short_desc.empty() ? std::string("quel pezzo") : chosen.short_desc;
	/* Mid-frase: stessa regola di $p in act (Una → una). */
	const char* const desc = act_lower_leading_article(desc_raw.c_str());
	if(qty > chosen.item_count) {
		char buf[320];
		std::snprintf(buf, sizeof(buf),
					  "$N ti dice 'Di %s ne ho solo $c0015%d$c0007, non %d.'", desc,
					  chosen.item_count, qty);
		act(buf, FALSE, ch, nullptr, clerk, TO_CHAR);
		return TRUE;
	}
	const long unit = chosen.price;
	const long total = unit * static_cast<long>(qty);
	const bool staff_other = owner_arg != nullptr;

	if(cmd == CMD_VALUE) {
		char buf[256];
		if(qty > 1) {
			std::snprintf(buf, sizeof(buf),
						  "$N ti dice 'Il ritiro di %d x %s costa $c0011%ld$c0007 monete d'oro "
						  "($c0011%ld$c0007 cadauno).'",
						  qty, desc, total, unit);
		}
		else {
			std::snprintf(buf, sizeof(buf),
						  "$N ti dice 'Il ritiro di %s costa $c0011%ld$c0007 monete d'oro.'",
						  desc, unit);
		}
		act(buf, FALSE, ch, nullptr, clerk, TO_CHAR);
		return TRUE;
	}

	std::vector<unsigned long long> ids;
	ids.assign(chosen.overflow_ids.begin(),
			   chosen.overflow_ids.begin() + static_cast<std::ptrdiff_t>(qty));
	if(!inventory_overflow_claim(ch, ids, owner_arg)) {
		return TRUE;
	}

	char paid[256];
	if(staff_other) {
		if(qty > 1) {
			std::snprintf(paid, sizeof(paid),
						  "Ritiri $c0015%d$c0007 x %s dal deposito di $c0015%s$c0007 (staff).", qty,
						  desc, owner.c_str());
		}
		else {
			std::snprintf(paid, sizeof(paid),
						  "Ritiri %s dal deposito di $c0015%s$c0007 (staff).", desc,
						  owner.c_str());
		}
	}
	else if(qty > 1) {
		std::snprintf(paid, sizeof(paid),
					  "Dai $c0011%ld$c0007 monete d'oro a $N e ritiri $c0015%d$c0007 x %s dal "
					  "deposito.",
					  total, qty, desc);
	}
	else {
		std::snprintf(paid, sizeof(paid),
					  "Dai $c0011%ld$c0007 monete d'oro a $N e ritiri %s dal deposito.", total,
					  desc);
	}
	act(paid, FALSE, ch, nullptr, clerk, TO_CHAR);
	char room[256];
	std::snprintf(room, sizeof(room), "$n da' alcune monete a $N e ritira qualcosa dal deposito.");
	act(room, FALSE, ch, nullptr, clerk, TO_NOTVICT);
	return TRUE;
}

} // namespace Alarmud
