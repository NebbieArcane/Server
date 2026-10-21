/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cctype>
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
#include "spec_bank.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "utility.hpp"
#include "reception.hpp"
#include "object_instance.hpp"
#include "cmdid.hpp"
#include "clan_symbol.hpp"

#include <algorithm>
#include <charconv>
#include <limits>
#include <map>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#if USE_MYSQL
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include <odb/database.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/transaction.hxx>
#include <mysql/mysql.h>
#endif
namespace Alarmud {

/* Strumenti condivisi con i comandi wiz (vedi spec_bank.hpp). */
using namespace vaultapi;

namespace {
constexpr long long kMaxCoins = std::numeric_limits<int>::max(); /* gold e bankgold sono int */
constexpr long long kMinBankOp = 1000; /* importo minimo per deposito/prelievo */
constexpr int kMonkBankLevel = 10; /* sotto questo livello i monaci non usano la banca */
constexpr long long kBankCapPerLevel = 100000; /* tetto conto per livello, sotto il 40 */
constexpr long long kTaxPct = 5; /* tassa di deposito, arrotondata per eccesso */

/* Colori della banca monete (stile cassette). */
constexpr std::string_view kBankGold = "$c0011"; /* oro: giallo */
constexpr std::string_view kBankAmt = "$c0015";  /* importi: bianco brillante */
constexpr std::string_view kBankName = "$c0014"; /* nomi: ciano */

/* Riga d'errore rossa. */
void bank_err(struct char_data* ch, std::string_view text) {
	send_str(ch, paint(kBankErr, text) + "\n\r");
}

/* "N monete d'oro" con importo bianco e oro giallo. */
[[nodiscard]] std::string gold_str(long long n, std::string_view what = "monete d'oro") {
	return paint(kBankAmt, group(n)) + " " + paint(kBankGold, what);
}

/* Righe "etichetta:  importo resto" allineate: etichette a sinistra,
 * importi a destra, larghezze calcolate come in send_table. */
struct BankLine {
	std::string label;
	long long amount;
	std::string tail;
};

void send_bank_lines(struct char_data* ch, const std::vector<BankLine>& lines) {
	size_t wl = 0;
	size_t wa = 0;
	for(const BankLine& l : lines) {
		wl = std::max(wl, l.label.size() + 1);
		wa = std::max(wa, group(l.amount).size());
	}
	for(const BankLine& l : lines) {
		std::string label = l.label + ":";
		label.append(wl - label.size(), ' ');
		std::string amt = group(l.amount);
		amt.insert(0, wa - amt.size(), ' ');
		send_str(ch, label + "  " + paint(kBankAmt, amt) + " " + paint(kBankGold, l.tail) + "\n\r");
	}
}

/* Importo in monete: "1500", "40.781.250", "2k", "1.5k", "2,25m" (k = 1.000,
 * m = 1.000.000). Senza suffisso il punto separa le migliaia (parse_amount);
 * decimali solo con suffisso e non oltre la sua precisione (k: 3, m: 6).
 * nullopt se vuoto, malformato o oltre kMaxCoins. */
[[nodiscard]] std::optional<long long> parse_coin_amount(std::string_view s) {
	long long mult = 1;
	int max_dec = 0;
	if(!s.empty() && (s.back() == 'k' || s.back() == 'K')) {
		mult = 1000;
		max_dec = 3;
		s.remove_suffix(1);
	}
	else if(!s.empty() && (s.back() == 'm' || s.back() == 'M')) {
		mult = 1000000;
		max_dec = 6;
		s.remove_suffix(1);
	}
	if(mult == 1) {
		const std::optional<long long> plain = parse_amount(s);
		if(!plain || *plain > kMaxCoins) {
			return std::nullopt;
		}
		return plain;
	}
	const size_t dot = s.find_first_of(".,");
	const std::string_view ip = s.substr(0, dot);
	const std::string_view fp = (dot == std::string_view::npos) ? std::string_view() : s.substr(dot + 1);
	if(ip.empty() || (dot != std::string_view::npos && fp.empty()) ||
	   static_cast<int>(fp.size()) > max_dec) {
		return std::nullopt;
	}
	const auto digits = [](std::string_view d) {
		return std::all_of(d.begin(), d.end(), [](char c) {
			return std::isdigit(static_cast<unsigned char>(c)) != 0;
		});
	};
	if(!digits(ip) || !digits(fp)) {
		return std::nullopt;
	}
	long long whole = 0;
	const auto [p1, e1] = std::from_chars(ip.data(), ip.data() + ip.size(), whole);
	if(e1 != std::errc() || p1 != ip.data() + ip.size() || whole > kMaxCoins / mult) {
		return std::nullopt;
	}
	long long frac = 0;
	if(!fp.empty()) {
		const auto [p2, e2] = std::from_chars(fp.data(), fp.data() + fp.size(), frac);
		if(e2 != std::errc() || p2 != fp.data() + fp.size()) {
			return std::nullopt;
		}
		long long scale = mult; /* frac * mult / 10^cifre, esatto in interi */
		for(size_t i = 0; i < fp.size(); ++i) {
			scale /= 10;
		}
		frac *= scale;
	}
	const long long total = whole * mult + frac;
	if(total > kMaxCoins) {
		return std::nullopt;
	}
	return total;
}
}

/* Banca monete (deposit/withdraw/balance). */

ROOMSPECIAL_FUNC(bank) {

	long long money = 0;
	long long tassa = 0;
	bool is_all = false;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(!IS_NPC(ch) && (cmd == CMD_DEPOSIT || cmd == CMD_WITHDRAW)) {
		std::string_view a = arg ? arg : "";
		while(!a.empty() && a.front() == ' ') {
			a.remove_prefix(1);
		}
		while(!a.empty() && (a.back() == ' ' || a.back() == '\r' || a.back() == '\n')) {
			a.remove_suffix(1);
		}
		if(a.size() == 3 && std::tolower(static_cast<unsigned char>(a[0])) == 'a' &&
		   std::tolower(static_cast<unsigned char>(a[1])) == 'l' &&
		   std::tolower(static_cast<unsigned char>(a[2])) == 'l') {
			/* "all": tutto l'oro in mano, o tutto il conto (senza superare kMaxCoins in mano). */
			is_all = true;
			money = (cmd == CMD_DEPOSIT) ? GET_GOLD(ch)
			                             : std::min<long long>(GET_BANK(ch), kMaxCoins - GET_GOLD(ch));
		}
		else if(!a.empty()) {
			const std::optional<long long> n = parse_coin_amount(a);
			if(!n) {
				bank_err(ch, "Il banchiere aggrotta la fronte: 'Quante monete, di preciso?' "
				             "Scrivi un numero, ad esempio 5000, 5.000, 5k, 1.5m oppure all.");
				return(TRUE);
			}
			money = *n;
		}
	}

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	/* Tetto per livello (solo sotto il 40): blocca i depositi, mai i prelievi;
	 * nessuno spostamento automatico dell'eccedenza. */
	const long long bank_cap = static_cast<long long>(GetMaxLevel(ch)) * kBankCapPerLevel;
	const bool capped = GetMaxLevel(ch) < 40;
	const std::string monk_msg = "I tuoi voti di poverta' ti impediscono di accumulare ricchezze. "
	                             "Torna quando avrai raggiunto il decimo livello.";
	const std::string usage_msg = "Il banchiere ti guarda in attesa. "
	                              "Uso: deposit <importo> / withdraw <importo> / balance.";
	const std::string min_msg = "Il banchiere sbuffa: 'Non scomodo il forziere per meno di " +
	                            group(kMinBankOp) + " monete d'oro.'";
	const std::string room_name = std::string(kBankName) + "$n" + std::string("$c0007");

	/*deposit*/
	if(cmd==CMD_DEPOSIT) {
		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < kMonkBankLevel)) {
			bank_err(ch, monk_msg);
			return(TRUE);
		}

		if(money > GET_GOLD(ch)) {
			bank_err(ch, "Frughi nella borsa, ma non hai tutte quelle monete.");
			return(TRUE);
		}
		else if(money <= 0) {
			bank_err(ch, is_all ? "Non hai nulla da depositare." : usage_msg);
			return(TRUE);
		}
		if(money < kMinBankOp) {
			bank_err(ch, min_msg);
			return(TRUE);
		}
		/* inizio procedura calcolo interessi operazione bancaria */
		tassa = (money * kTaxPct + 99) / 100;
		/* termina procedura calcolo interessi operazione bancaria */
		if(capped && (money - tassa + GET_BANK(ch) > bank_cap)) {
			bank_err(ch, "Il banchiere alza una mano: 'Le leggi della gilda mi permettono di custodire "
			             "al massimo " + group(bank_cap) + " monete per uno del tuo rango (" +
			             group(kBankCapPerLevel) + " per livello).'");
			return(TRUE);
		}
		else if(money - tassa + GET_BANK(ch) > kMaxCoins) {
			bank_err(ch, "Il banchiere impallidisce: 'Il forziere non puo' contenere una simile fortuna!'");
			return(TRUE);
		}
		else {
			const long long gross = money;
			GET_GOLD(ch) = static_cast<int>(GET_GOLD(ch) - money);
			money -= tassa;
			GET_BANK(ch) = static_cast<int>(GET_BANK(ch) + money);
			save_char(ch, AUTO_RENT, 0); /* salva dopo la modifica */
			send_str(ch, "Il banchiere conta le tue monete con dita esperte.\n\r");
			std::vector<BankLine> lines{{"Depositi", gross, "monete d'oro"}};
			if(tassa > 0) {
				lines.push_back({"Tassa (" + std::to_string(kTaxPct) + "%)", tassa, "monete d'oro"});
			}
			lines.push_back({"Accreditate", money, "monete d'oro"});
			send_bank_lines(ch, lines);
			send_str(ch, "Nel forziere ora custodisci " + gold_str(GET_BANK(ch)) + ".\n\r");
			act((room_name + " posa un sacchetto tintinnante sul bancone e il banchiere lo fa "
			     "sparire con un inchino.").c_str(), TRUE, ch, nullptr, nullptr, TO_ROOM);
			return(TRUE);
		}
		/*withdraw*/
	}
	else if(cmd==CMD_WITHDRAW) {

		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < kMonkBankLevel)) {
			bank_err(ch, monk_msg);
			return(TRUE);
		}

		const std::string heavy_msg = std::string("Sei gia' caric") +
		                              (GET_SEX(ch) == SEX_FEMALE ? "a" : "o") +
		                              " d'oro: non riusciresti a portarne altro. Preleva una somma minore.";
		if(money > GET_BANK(ch)) {
			bank_err(ch, "Il banchiere scuote la testa: 'Nel tuo forziere non ce ne sono abbastanza.'");
			return(TRUE);
		}
		else if(money <= 0) {
			bank_err(ch, !is_all ? usage_msg
			                     : (GET_BANK(ch) <= 0 ? std::string("Il tuo forziere e' vuoto.") : heavy_msg));
			return(TRUE);
		}
		else if(money < kMinBankOp) {
			bank_err(ch, min_msg);
			return(TRUE);
		}
		else if(money + GET_GOLD(ch) > kMaxCoins) {
			bank_err(ch, heavy_msg);
			return(TRUE);
		}
		else {
			GET_BANK(ch) = static_cast<int>(GET_BANK(ch) - money);
			/* inizio procedura calcolo interessi operazione bancaria */
			/*tassa = (int)(money*tasso_bancario);*/
			/*money -= tassa;*/
			/*sprintf(buf,"Tassa applicata alla sua operazione: %d monete d'oro.\n\r",tassa);*/
			/*send_to_char(buf, ch);*/
			/* termina procedura calcolo interessi operazione bancaria */
			GET_GOLD(ch) = static_cast<int>(GET_GOLD(ch) + money);
			save_char(ch, AUTO_RENT, 0); /* salva dopo la modifica */
			send_str(ch, "Il banchiere apre il forziere e ti consegna " + gold_str(money) + ".\n\r");
			send_str(ch, "Nel forziere restano " + gold_str(GET_BANK(ch)) + ".\n\r");
			act((std::string("Il banchiere conta delle monete d'oro e le spinge verso ") + room_name +
			     ".").c_str(), TRUE, ch, nullptr, nullptr, TO_ROOM);
			return(TRUE);
		}
		/* Balance */
	}
	else if(cmd == CMD_BALANCE) {
		send_str(ch, "Il banchiere sfoglia il suo grosso registro.\n\r");
		std::vector<BankLine> lines{{"Oro con te", GET_GOLD(ch), "monete"},
		                            {"Nel forziere", GET_BANK(ch), "monete"}};
		if(capped) {
			lines.push_back({"Limite del forziere", bank_cap,
			                 "monete (" + group(kBankCapPerLevel) + " per livello)"});
			lines.push_back({"Puoi ancora depositare", std::max(0LL, bank_cap - GET_BANK(ch)), "monete"});
		}
		send_bank_lines(ch, lines);
		if(!capped) {
			send_str(ch, "Il tuo forziere non ha limiti.\n\r");
		}
		else if(GET_BANK(ch) > bank_cap) {
			bank_err(ch, "Il tuo forziere trabocca: superi il limite di " +
			             group(GET_BANK(ch) - bank_cap) +
			             " monete. Puoi prelevare, ma non depositare finche' non scendi sotto il limite.");
		}
		return(TRUE);
	}
	return(FALSE);
}

/* ======================================================================
 * Cassette di sicurezza con piani (personale, account, clan).
 *
 * Schema (heal_v15, odb_schema_heal.cpp):
 *   vault_plan  (scope, tier) -> capienza e prezzi; seed da spec_bank_vault.inc.
 *   vault       una riga per (scope, owner_id): tier, pay_mode, paid_until.
 *   vault_item  saldo per (vault_id, item_number).
 *   vault_log   movimenti e pagamenti (toon_id = chi ha agito).
 *
 * Ogni comando che modifica dati gira in UNA transazione: la riga vault e'
 * bloccata con SELECT ... FOR UPDATE (serializza account/clan condivisi), i
 * prelievi usano quantity >= N. La memoria (oggetti, monete) cambia solo dopo
 * il commit: un errore DB lascia PG e cassetta come prima.
 *
 * Canoni: nessun job in background. Alla prima operazione dopo la scadenza
 * (paid_until) gli arretrati vengono pagati con le monete che il PG ha con se';
 * deposit/withdraw/buy prima mostrano l'importo senza addebitarlo; il PG
 * conferma ripetendo il comando con "paga" in fondo e solo allora arretrati
 * e operazione vengono pagati nella stessa transazione. Senza monete a
 * sufficienza la cassetta resta bloccata. Gli oggetti non vengono mai
 * cancellati per mancato pagamento.
 *
 * Rune: le "rune degli Dei" del PG (GET_RUNEDEI, character_stats.p_rune_dei).
 * Cancellazione: vault.toon_id (personale e clan) e vault.user_id (account)
 * hanno FK ON DELETE CASCADE su toon/user; vault_item e vault_log seguono.
 */

#if USE_MYSQL

namespace vaultapi {

#include "spec_bank_vault.inc"

/* parse_count: "all" / "all.nome" = tutti quelli che corrispondono. */
constexpr int kWantAll = std::numeric_limits<int>::max();
constexpr long long kVaultPeriodDays = 30;
constexpr long long kVaultPeriodSecs = kVaultPeriodDays * 24 * 3600;

/* ---------------------------------------------------------------- testi */

/* Le etichette sono letterali: .data() e' terminato da '\0'. */
[[nodiscard]] std::string_view tier_name(int tier) {
	constexpr std::string_view names[] = {"?", "Bronzo", "Argento", "Oro", "Platino", "Mithril"};
	return (tier >= 1 && tier <= kVaultTiers) ? names[tier] : names[0];
}

[[nodiscard]] std::string_view mode_name(int mode) {
	switch(mode) {
	case VAULT_PAY_ONCE:
		return "acquisto unico";
	case VAULT_PAY_RENT:
		return "affitto mensile";
	case VAULT_PAY_BUYFEE:
		return "acquisto con canone mensile";
	case VAULT_PAY_DEPFEE:
		return "tassa per deposito";
	default:
		return "?";
	}
}

[[nodiscard]] std::string_view scope_label(int scope) {
	switch(scope) {
	case VAULT_SCOPE_ACCOUNT:
		return "di account";
	case VAULT_SCOPE_CLAN:
		return "del clan";
	default:
		return "personale";
	}
}

/* Prefisso del comando per i suggerimenti ("", "account ", "clan "). */
[[nodiscard]] constexpr std::string_view scope_cmd(int scope) {
	switch(scope) {
	case VAULT_SCOPE_ACCOUNT:
		return "account ";
	case VAULT_SCOPE_CLAN:
		return "clan ";
	default:
		return "";
	}
}

void send_str(struct char_data* ch, const std::string& s) {
	send_to_char(s.c_str(), ch);
}

/* Confronto case-insensitive ASCII su string_view (niente '\0' richiesto). */
[[nodiscard]] constexpr char ascii_lower(char x) {
	return (x >= 'A' && x <= 'Z') ? static_cast<char>(x - 'A' + 'a') : x;
}

[[nodiscard]] bool iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() &&
	       std::equal(a.begin(), a.end(), b.begin(),
	                  [](char x, char y) { return ascii_lower(x) == ascii_lower(y); });
}

/* ------------------------------------------------------------ tabelle */

/* Colori ($cNNNN come nel resto del MUD) e larghezza massima delle righe. */
constexpr std::string_view kColRule = "$c0008";
constexpr std::string_view kColReset = "$c0007";

/* Testo colorato; il padding va fatto PRIMA (i codici colore non occupano
 * colonne a schermo). */
[[nodiscard]] std::string paint(std::string_view color, std::string_view text) {
	std::string out(color);
	out.append(text.data(), text.size());
	out.append(kColReset.data(), kColReset.size());
	return out;
}

/* Allinea a destra (numeri) o a sinistra (testi) su w colonne. */
[[nodiscard]] std::string pad(std::string_view text, size_t w, bool right) {
	std::string out(text);
	if(out.size() < w) {
		out.insert(right ? 0 : out.size(), w - out.size(), ' ');
	}
	return out;
}

/* 1234567 -> "1.234.567" (vedi Alarmud::format_thousands). */
[[nodiscard]] std::string group(long long n) {
	return format_thousands(n);
}

/* Quantita' nei messaggi: separatore delle migliaia e verde come nelle tabelle. */
[[nodiscard]] std::string num(long long n) {
	return paint(kColNum, group(n));
}

[[nodiscard]] std::string num(size_t n) {
	return num(static_cast<long long>(n));
}

[[nodiscard]] std::string num(int n) {
	return num(static_cast<long long>(n));
}

[[nodiscard]] std::string rule_line(size_t w) {
	return paint(kColRule, std::string(w, '-')) + "\n\r";
}

/* Tabella con larghezze calcolate: ogni colonna e' larga quanto il suo
 * testo piu' lungo (intestazione compresa). Le celle arrivano come testo
 * semplice, il colore si aggiunge dopo il padding. Riga vuota = separatore.
 * L'ultima colonna allineata a sinistra non viene riempita di spazi. */
constexpr size_t kMaxRule = 78;

void send_table(struct char_data* ch, const std::vector<TableCol>& cols,
                const std::vector<TableRow>& rows, std::string_view gap) {
	const size_t n = cols.size();
	std::vector<size_t> w(n, 0);
	for(size_t i = 0; i < n; ++i) {
		w[i] = cols[i].head.size();
	}
	for(const TableRow& r : rows) {
		for(size_t i = 0; i < n && i < r.size(); ++i) {
			w[i] = std::max(w[i], r[i].size());
		}
	}
	size_t total = n > 0 ? gap.size() * (n - 1) : 0;
	for(const size_t x : w) {
		total += x;
	}
	const std::string rule = rule_line(std::min(total, kMaxRule));
	const auto cell = [&](size_t i, std::string_view text) {
		const bool last_free = (i + 1 == n) && !cols[i].right;
		return last_free ? std::string(text) : pad(text, w[i], cols[i].right);
	};
	std::string head;
	for(size_t i = 0; i < n; ++i) {
		if(i > 0) {
			head.append(gap.data(), gap.size());
		}
		head += cell(i, cols[i].head);
	}
	send_str(ch, paint(kColHead, head) + "\n\r");
	send_str(ch, rule);
	for(const TableRow& r : rows) {
		if(r.empty()) {
			send_str(ch, rule);
			continue;
		}
		std::string line;
		for(size_t i = 0; i < n; ++i) {
			if(i > 0) {
				line.append(gap.data(), gap.size());
			}
			const std::string text = i < r.size() ? r[i] : std::string();
			line += paint(text == "-" ? kColRule : cols[i].color, cell(i, text));
		}
		send_str(ch, line + "\n\r");
	}
	send_str(ch, rule);
}

/* ------------------------------------------------------------------ SQL */

/* Come strtoll: spazi iniziali saltati, prefisso numerico, 0 se non numerico. */
[[nodiscard]] long long to_ll(const std::string& s) {
	std::string_view v = s;
	while(!v.empty() && std::isspace(static_cast<unsigned char>(v.front()))) {
		v.remove_prefix(1);
	}
	if(!v.empty() && v.front() == '+') {
		v.remove_prefix(1);
	}
	long long n = 0;
	const auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), n);
	(void)ptr;
	return ec == std::errc() ? n : 0;
}

/* SELECT sulla connessione della transazione corrente. */
[[nodiscard]] bool vsel(const std::string& sql, SqlRows& rows) {
	rows.clear();
	auto& mc = static_cast<odb::mysql::connection&>(odb::transaction::current().connection());
	MYSQL* h = mc.handle();
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "vault sql: %s", mysql_error(h));
		return false;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		mudlog(LOG_SYSERR, "vault sql: no result set");
		return false;
	}
	const unsigned nf = mysql_num_fields(res);
	MYSQL_ROW r;
	while((r = mysql_fetch_row(res)) != nullptr) {
		std::vector<std::string> v;
		v.reserve(nf);
		for(unsigned i = 0; i < nf; ++i) {
			v.emplace_back(r[i] ? r[i] : "");
		}
		rows.push_back(std::move(v));
	}
	mysql_free_result(res);
	return true;
}

unsigned long long vexec(const std::string& sql) {
	return Sql::getMysql()->execute(sql.c_str());
}

/* Piano dal listino; nullopt se la riga non esiste. Errore SQL = eccezione. */
[[nodiscard]] std::optional<VaultPlan> load_plan(int scope, int tier) {
	SqlRows r;
	if(!vsel("SELECT capacity, price_once, rent_month, price_buy, fee_month, runes_once, "
	         "runes_buy, deposit_fee FROM vault_plan WHERE scope = " + std::to_string(scope) +
	         " AND tier = " + std::to_string(tier), r)) {
		throw std::runtime_error("vault: plan query failed");
	}
	if(r.empty()) {
		return std::nullopt;
	}
	VaultPlan p;
	p.capacity = static_cast<int>(to_ll(r[0][0]));
	p.price_once = to_ll(r[0][1]);
	p.rent_month = to_ll(r[0][2]);
	p.price_buy = to_ll(r[0][3]);
	p.fee_month = to_ll(r[0][4]);
	p.runes_once = static_cast<int>(to_ll(r[0][5]));
	p.runes_buy = static_cast<int>(to_ll(r[0][6]));
	p.deposit_fee = to_ll(r[0][7]);
	return p;
}

/* Cassetta del contesto; nullopt se non esiste. Errore SQL = eccezione. */
[[nodiscard]] std::optional<VaultRow> load_vault(const VaultCtx& c, bool lock) {
	SqlRows r;
	std::string sql =
		"SELECT id, tier, pay_mode, "
		"IF(paid_until IS NOT NULL AND paid_until <= NOW(), "
		"TIMESTAMPDIFF(SECOND, paid_until, NOW()), -1), "
		"IFNULL(DATE_FORMAT(paid_until, '%d/%m/%Y'), '') "
		"FROM vault WHERE scope = " + std::to_string(c.scope) +
		" AND owner_id = " + std::to_string(c.owner_id);
	if(lock) {
		sql += " FOR UPDATE";
	}
	if(!vsel(sql, r)) {
		throw std::runtime_error("vault: load failed");
	}
	if(r.empty()) {
		return std::nullopt;
	}
	VaultRow v;
	v.id = static_cast<unsigned long long>(to_ll(r[0][0]));
	v.tier = static_cast<int>(to_ll(r[0][1]));
	v.mode = static_cast<int>(to_ll(r[0][2]));
	v.overdue_secs = to_ll(r[0][3]);
	v.paid_until = r[0][4];
	return v;
}

[[nodiscard]] long long monthly_due(const VaultRow& v, const VaultPlan& p) {
	if(v.mode == VAULT_PAY_RENT) {
		return p.rent_month;
	}
	if(v.mode == VAULT_PAY_BUYFEE) {
		return p.fee_month;
	}
	return 0;
}

[[nodiscard]] long long months_owed(const VaultRow& v) {
	return v.overdue_secs < 0 ? 0 : v.overdue_secs / kVaultPeriodSecs + 1;
}

[[nodiscard]] long long stored_count(unsigned long long vault_id) {
	SqlRows r;
	if(!vsel("SELECT COALESCE(SUM(quantity),0) FROM vault_item WHERE vault_id = " +
	         std::to_string(vault_id), r) || r.empty()) {
		throw std::runtime_error("vault: stored_count failed");
	}
	return to_ll(r[0][0]);
}

void log_row(unsigned long long vault_id, unsigned long long actor, const char* action,
             unsigned item_number, long long quantity, long long amount) {
	vexec("INSERT INTO vault_log (vault_id, toon_id, item_number, quantity, action, amount, at) "
	      "VALUES (" + std::to_string(vault_id) + "," + std::to_string(actor) + "," +
	      std::to_string(item_number) + "," + std::to_string(quantity) + ",'" + action + "'," +
	      std::to_string(amount) + ",NOW())");
}

void write_gold(unsigned long long actor, long long gold) {
	vexec("UPDATE character_stats SET gold = " + std::to_string(gold) +
	      " WHERE toon_id = " + std::to_string(actor));
}

/* Prima riga: stato e importo dovuto. */
[[nodiscard]] std::string arrears_msg(int scope, long long due, long long months) {
	return fmt("La cassetta %s e' bloccata: devi %s monete di canone (%lld mes%s).\n\r",
	           scope_label(scope), paint(kColNum, group(due)), months, months == 1 ? "e" : "i");
}

/* Come pagare, con il prefisso giusto (account/clan). */
[[nodiscard]] std::string arrears_howto(int scope) {
	const std::string pre(scope_cmd(scope));
	return "Per pagare e sbloccarla ripeti il comando con 'paga' in fondo, ad esempio:\n\r  " +
	       paint(kColCmd, pre + "deposit <oggetto> paga") + "   oppure   " +
	       paint(kColCmd, pre + "withdraw <oggetto> paga") + "\n\r";
}

/* Arretrati: senza c.confirm li mostra e non addebita nulla; con conferma li
 * paga con gold (solo righe DB + gold locale). true = si puo' procedere;
 * false = messaggio gia' inviato, il chiamante esce senza commit. */
[[nodiscard]] bool settle(struct char_data* ch, const VaultCtx& c, VaultRow& v, const VaultPlan& p,
            long long& gold, long long& paid) {
	paid = 0;
	const long long m = months_owed(v);
	if(m <= 0) {
		return true;
	}
	const long long monthly = monthly_due(v, p);
	if(monthly <= 0) {
		/* Canone azzerato nel listino: niente da pagare. */
		vexec("UPDATE vault SET paid_until = NULL WHERE id = " + std::to_string(v.id));
		v.overdue_secs = -1;
		return true;
	}
	const long long due = m * monthly;
	if(!c.confirm) {
		send_str(ch, arrears_msg(c.scope, due, m));
		send_to_char("Per ora non ti e' stato addebitato nulla.\n\r", ch);
		send_str(ch, arrears_howto(c.scope));
		return false;
	}
	if(gold < due) {
		send_str(ch, fmt("Per pagare il canone servono %s monete con te, ma ne hai solo %s: "
		                 "la cassetta %s resta bloccata.\n\r", paint(kColNum, group(due)),
		                 paint(kColNum, group(gold)), scope_label(c.scope)));
		return false;
	}
	vexec("UPDATE vault SET paid_until = DATE_ADD(paid_until, INTERVAL " +
	      std::to_string(m * kVaultPeriodDays) + " DAY) WHERE id = " + std::to_string(v.id));
	gold -= due;
	paid = due;
	log_row(v.id, c.actor_id, "rent", 0, m, due);
	v.overdue_secs = -1;
	return true;
}


[[nodiscard]] std::string no_vault_msg(int scope) {
	return fmt("Non c'e' ancora una cassetta di sicurezza %s. Usa '%sbuy' per vedere i piani.\n\r",
	           scope_label(scope), scope_cmd(scope));
}

/* --------------------------------------------------------- oggetti/nomi */

/* Intero decimale senza segno, tutta la stringa: nullopt se vuota, non
 * numerica o fuori intervallo (niente 0 silenzioso come atoi). */
[[nodiscard]] std::optional<long long> parse_ll(std::string_view s) {
	long long v = 0;
	const char* const first = s.data();
	const char* const last = first + s.size();
	if(s.empty()) {
		return std::nullopt;
	}
	const auto [ptr, ec] = std::from_chars(first, last, v);
	if(ec != std::errc() || ptr != last || v < 0) {
		return std::nullopt;
	}
	return v;
}

[[nodiscard]] bool all_digits(std::string_view s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), [](char ch) {
		return std::isdigit(static_cast<unsigned char>(ch)) != 0;
	});
}

/* one_argument (legacy) con risultato in stringhe: primo token e resto. */
[[nodiscard]] ArgSplit split_arg(const std::string& s) {
	std::string tok(MAX_INPUT_LENGTH, '\0');
	const char* rest = one_argument(s.c_str(), tok.data());
	tok.resize(std::char_traits<char>::length(tok.c_str()));
	return {std::move(tok), rest ? std::string(rest) : std::string()};
}

struct CountArg {
	int want = 1;     /* >0 quantita', -N N-esimo, kWantAll tutti */
	std::string name; /* vuoto: niente da cercare */
};

/* Parsa "N*nome" (quantita'), "N.nome" (N-esimo), "all.nome", "all" o "nome".
 * >0 quantita', -N N-esimo (1-based), 1 senza prefisso. */
[[nodiscard]] CountArg parse_count(const char* argument) {
	std::string_view a = argument ? argument : "";
	while(!a.empty() && a.front() == ' ') {
		a.remove_prefix(1);
	}
	while(!a.empty() && (a.back() == ' ' || a.back() == '\r' || a.back() == '\n')) {
		a.remove_suffix(1);
	}
	/* "all.nome" come getall(): primo token, almeno 5 caratteri, "all." + nome. */
	const std::string_view tok = a.substr(0, std::min(a.find_first_of(" \t\r\n"), a.size()));
	if(tok.size() >= 5 && tok.substr(0, 4) == "all.") {
		return {kWantAll, std::string(tok.substr(4))};
	}
	if(iequals(a, "all")) {
		return {kWantAll, "all"};
	}
	const size_t sep = a.find_first_of("*.");
	if(sep != std::string_view::npos && sep > 0 && all_digits(a.substr(0, sep))) {
		/* Fuori intervallo: valore massimo (poi "ce ne sono solo ..."). */
		const long long n = parse_ll(a.substr(0, sep)).value_or(kWantAll - 1);
		const int k = static_cast<int>(std::min<long long>(n, kWantAll - 1));
		std::string name(a.substr(sep + 1));
		if(a[sep] == '.') {
			return {(k > 0) ? -k : -1, std::move(name)};
		}
		return {(k > 0) ? k : 1, std::move(name)};
	}
	return {1, std::string(a)};
}

[[nodiscard]] const struct obj_data* proto_of(unsigned vnum) {
	const int rnum = real_object(static_cast<int>(vnum));
	if(rnum < 0 || !obj_index[rnum].data) {
		return nullptr;
	}
	return static_cast<const obj_data*>(obj_index[rnum].data);
}

[[nodiscard]] std::string proto_short_desc(unsigned vnum) {
	const obj_data* proto = proto_of(vnum);
	if(proto && proto->short_description && *proto->short_description) {
		return proto->short_description;
	}
	if(real_object(static_cast<int>(vnum)) >= 0) {
		struct obj_data* loaded = read_object(static_cast<int>(vnum), VIRTUAL);
		if(loaded) {
			std::string s = loaded->short_description ? loaded->short_description : "";
			extract_obj(loaded); /* riallinea il contatore di read_object */
			if(!s.empty()) {
				return s;
			}
		}
	}
	return fmt("oggetto #%u", vnum);
}

[[nodiscard]] std::string proto_name(unsigned vnum) {
	const obj_data* proto = proto_of(vnum);
	if(proto && proto->name) {
		return proto->name;
	}
	if(real_object(static_cast<int>(vnum)) >= 0) {
		struct obj_data* loaded = read_object(static_cast<int>(vnum), VIRTUAL);
		if(loaded) {
			std::string s = loaded->name ? loaded->name : "";
			extract_obj(loaded);
			return s;
		}
	}
	return "";
}

/* Solo oggetti ITEM2_VAULT "foglia", non editati/instance: la cassetta
 * conserva solo il VNUM e li restituisce come prototipo. */
[[nodiscard]] bool deposit_check(struct char_data* ch, struct obj_data* obj, bool quiet = false) {
	if(obj->contains) {
		if(quiet) {
			return false;
		}
		send_to_char("Non puoi riporre nella cassetta un contenitore "
		             "con oggetti dentro: svuotalo prima.\n\r", ch);
		return false;
	}
	if(obj->item_number < 0 || obj->db_instance_id != 0 ||
	   object_is_zone_limit_exempt(obj) || !obj_vault_allowed(obj)) {
		if(quiet) {
			return false;
		}
		send_str(ch, fmt("Non puoi riporre %s nella cassetta di sicurezza.\n\r",
		                 obj->short_description ? obj->short_description : "questo oggetto"));
		return false;
	}
	return true;
}

[[nodiscard]] bool proto_vault_allowed(unsigned vnum) {
	if(real_object(static_cast<int>(vnum)) < 0) {
		return false;
	}
	struct obj_data* obj = read_object(static_cast<int>(vnum), VIRTUAL);
	if(obj == nullptr) {
		return false;
	}
	/* Come deposit_check su un oggetto appena creato (niente contenuto, niente
	 * instance); extract_obj riallinea il contatore di read_object. */
	const bool ok = obj->item_number >= 0 && !object_is_zone_limit_exempt(obj) &&
	                obj_vault_allowed(obj);
	extract_obj(obj);
	return ok;
}

[[nodiscard]] unsigned obj_vnum(const struct obj_data* obj) {
	return static_cast<unsigned>(obj_index[obj->item_number].iVNum);
}

/* Estrae oggetti creati ma non consegnati (errori/rollback). */
struct MadeGuard {
	std::vector<struct obj_data*> objs;
	bool released = false;
	~MadeGuard() {
		if(!released) {
			for(struct obj_data* o : objs) {
				extract_obj(o); /* riallinea anche il contatore di read_object */
			}
		}
	}
};

/* ------------------------------------------------------------ contesto */

/* Banca del clan: stanza "vault" con flag IS_CLAN. */
[[nodiscard]] bool vault_room_is_clan(struct char_data* ch) {
	const struct room_data* rp = real_roomp(ch->in_room);
	return rp != nullptr && IS_SET(rp->room_flags, IS_CLAN);
}

/* Cache nome -> (toon.id, toon.owner_id): evita la SELECT su toon a ogni
 * comando. Gli id non cambiano finche' il toon esiste; la cancellazione
 * (con_delete_me) chiama vault_forget_toon. */
struct ToonIds {
	unsigned long long id = 0;
	unsigned long long owner_id = 0;
};

[[nodiscard]] std::unordered_map<std::string, ToonIds>& toon_cache() {
	static std::unordered_map<std::string, ToonIds> cache;
	return cache;
}

[[nodiscard]] std::string name_key(std::string_view name) {
	std::string k(name);
	for(char& ch : k) {
		ch = ascii_lower(ch);
	}
	return k;
}

[[nodiscard]] std::optional<ToonIds> toon_ids(std::string_view name) {
	if(name.empty()) {
		return std::nullopt;
	}
	auto& cache = toon_cache();
	const std::string key = name_key(name);
	if(const auto it = cache.find(key); it != cache.end()) {
		return it->second;
	}
	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return std::nullopt;
	}
	const ToonIds ids {pg->id, pg->owner_id};
	cache.emplace(key, ids);
	const char* who = key.c_str();
	const unsigned long long tid = ids.id;
	mudlog(LOG_CHECK, "vault: toon id cache miss %s -> %llu", who, tid);
	return ids;
}

/* Contesto del comando; nullopt = messaggio gia' inviato. */
[[nodiscard]] std::optional<VaultCtx> resolve_ctx(struct char_data* ch, int scope) {
	VaultCtx c;
	c.scope = scope;
	const std::optional<ToonIds> pg = toon_ids(GET_NAME(ch) ? GET_NAME(ch) : "");
	if(!pg) {
		send_to_char("Non riesco a trovare il tuo personaggio nel database.\n\r", ch);
		return std::nullopt;
	}
	c.actor_id = pg->id;
	if(scope == VAULT_SCOPE_PERSONAL) {
		c.owner_id = pg->id;
		return c;
	}
	if(scope == VAULT_SCOPE_ACCOUNT) {
		if(pg->owner_id == 0) {
			send_to_char("Il tuo personaggio non e' collegato a nessun account.\n\r", ch);
			return std::nullopt;
		}
		c.owner_id = pg->owner_id;
		return c;
	}
	std::string prince;
	bool leader = false;
	if(!clan_vault_identity(ch, prince, leader)) {
		send_to_char("Non fai parte di nessun clan.\n\r", ch);
		return std::nullopt;
	}
	const std::optional<ToonIds> pp = toon_ids(prince);
	if(!pp) {
		send_to_char("Non trovo il principe del tuo clan nel database.\n\r", ch);
		return std::nullopt;
	}
	c.owner_id = pp->id;
	c.is_leader = leader;
	return c;
}

/* -------------------------------------------------------------- comandi */

void cmd_list(struct char_data* ch, const VaultCtx& c) {
	std::optional<VaultRow> vo;
	VaultPlan p;
	SqlRows items;
	long long stored = 0;
	{
		odb::transaction t(Sql::getMysql()->begin());
		vo = load_vault(c, false);
		if(!vo) {
			t.commit();
			send_str(ch, no_vault_msg(c.scope));
			return;
		}
		p = load_plan(c.scope, vo->tier).value_or(VaultPlan {});
		if(!vsel("SELECT item_number, quantity FROM vault_item WHERE vault_id = " +
		         std::to_string(vo->id) + " AND quantity > 0 ORDER BY item_number", items)) {
			throw std::runtime_error("vault: list failed");
		}
		t.commit();
	}
	const VaultRow& v = *vo;
	for(const auto& r : items) {
		stored += to_ll(r[1]);
	}
	send_str(ch, paint(kColTitle, fmt("Cassetta di sicurezza %s", scope_label(c.scope))) + "\n\r");
	send_str(ch, fmt("Piano: %s (%s)   Oggetti: %s / %s\n\r", paint(kColTier, tier_name(v.tier)),
	                 mode_name(v.mode), paint(kColHead, group(stored)),
	                 group(p.capacity)));
	const long long monthly = monthly_due(v, p);
	if(monthly > 0 && months_owed(v) > 0) {
		send_str(ch, arrears_msg(c.scope, months_owed(v) * monthly, months_owed(v)));
		send_str(ch, arrears_howto(c.scope));
	}
	else if(monthly > 0 && !v.paid_until.empty()) {
		send_str(ch, fmt("Canone: %s monete ogni 30 giorni, pagato fino al %s.\n\r",
		                 paint(kColNum, group(monthly)), v.paid_until));
	}
	else if(v.mode == VAULT_PAY_DEPFEE) {
		send_str(ch, fmt("Tassa di deposito: %s monete per oggetto.\n\r",
		                 paint(kColNum, group(p.deposit_fee))));
	}
	if(items.empty()) {
		send_to_char("La cassetta e' vuota.\n\r", ch);
		return;
	}
	std::vector<TableRow> table;
	for(const auto& r : items) {
		table.push_back({group(to_ll(r[1])), proto_short_desc(static_cast<unsigned>(to_ll(r[0])))});
	}
	send_table(ch, {{"Qta", true, kColHead}, {"Oggetto", false, kColReset}}, table);
}

void cmd_history(struct char_data* ch, const VaultCtx& c, std::string_view action) {
	SqlRows rows;
	{
		odb::transaction t(Sql::getMysql()->begin());
		const std::optional<VaultRow> v = load_vault(c, false);
		if(!v) {
			t.commit();
			send_str(ch, no_vault_msg(c.scope));
			return;
		}
		if(!vsel("SELECT DATE_FORMAT(l.at, '%Y-%m-%d %H:%i'), l.item_number, l.quantity, "
		         "IFNULL(t.name, '?') FROM vault_log l LEFT JOIN toon t ON t.id = l.toon_id "
		         "WHERE l.vault_id = " + std::to_string(v->id) + " AND l.action = '" + std::string(action) +
		         "' AND l.at >= (NOW() - INTERVAL " + std::to_string(VAULT_TX_RETENTION_HOURS) +
		         " HOUR) ORDER BY l.at DESC, l.item_number ASC", rows)) {
			throw std::runtime_error("vault: history failed");
		}
		t.commit();
	}
	if(rows.empty()) {
		send_str(ch, fmt("Nessun movimento registrato negli ultimi %d giorni.\n\r",
		                 VAULT_TX_RETENTION_HOURS / 24));
		return;
	}
	const std::string_view kind = (action == "deposit") ? "depositi" : "prelievi";
	send_str(ch, paint(kColTitle, fmt("Storico %s, cassetta %s (ultimi %d giorni)", kind,
	                                  scope_label(c.scope), VAULT_TX_RETENTION_HOURS / 24)) +
	             "\n\r");
	const bool shared = (c.scope != VAULT_SCOPE_PERSONAL);
	std::vector<TableCol> cols = {{"Data", false, kColRule}};
	if(shared) {
		cols.push_back({"Chi", false, kColTier});
	}
	cols.push_back({"Qta", true, kColHead});
	cols.push_back({"Oggetto", false, kColReset});
	std::vector<TableRow> table;
	for(const auto& r : rows) {
		TableRow row = {r[0]};
		if(shared) {
			row.push_back(r[3]);
		}
		row.push_back(group(to_ll(r[2])));
		row.push_back(proto_short_desc(static_cast<unsigned>(to_ll(r[1]))));
		table.push_back(std::move(row));
	}
	send_table(ch, cols, table);
}

/* Riepilogo per tipo: "     15 x una pozione rossa" (colori come le tabelle). */
void send_moved_list(struct char_data* ch, const std::map<unsigned, long long>& moved) {
	size_t w = 0;
	for(const auto& [vnum, qty] : moved) {
		(void)vnum;
		w = std::max(w, group(qty).size());
	}
	for(const auto& [vnum, qty] : moved) {
		send_str(ch, fmt("  %s x %s\n\r", paint(kColHead, pad(group(qty), w, true)),
		                 proto_short_desc(vnum)));
	}
}

/* Metallo del piano, colorato ($cNNNN come act(); reset $c0007), con la
 * preposizione: "di bronzo", "d'argento", "d'oro", "di platino", "di mithril". */
[[nodiscard]] std::string tier_metal(int tier) {
	switch(tier) {
	case 1:
		return "di $c0003bronzo$c0007";
	case 2:
		return "d'$c0015argento$c0007";
	case 3:
		return "d'$c0011oro$c0007";
	case 4:
		return "di $c0014platino$c0007";
	case 5:
		return "di $c0012mithril$c0007";
	default:
		return "di ferro";
	}
}

/* Messaggio agli altri presenti nella stanza (deposit/withdraw). */
void act_room(struct char_data* ch, int scope, int tier) {
	const std::string m = tier_metal(tier);
	std::string msg = "$n gira una chiave " + m + " e fruga nella sua cassetta di sicurezza.";
	if(scope == VAULT_SCOPE_ACCOUNT) {
		msg = "$n sfiora le rune di un antico forziere di famiglia " + m +
		      ", che si apre con un sussurro.";
	}
	else if(scope == VAULT_SCOPE_CLAN) {
		msg = "$n posa la mano sul sigillo del clan e il forziere " + m + " si schiude.";
	}
	act(msg.c_str(), TRUE, ch, nullptr, nullptr, TO_ROOM);
}

/* Acquisto/cambio piano: una riga di colore per chi compra e una per la stanza. */
void act_buy(struct char_data* ch, int scope, int tier) {
	const std::string m = tier_metal(tier);
	std::string to_char = "Il custode scioglie la ceralacca sul contratto e ti porge una "
	                      "chiave " + m + ".";
	std::string to_room = "$n appone il proprio sigillo di ceralacca sul contratto di una "
	                      "cassetta " + m + ".";
	if(scope == VAULT_SCOPE_ACCOUNT) {
		to_char = "Il custode incide il nome della tua stirpe in rune su un forziere " + m + ".";
		to_room = "Il custode incide rune su un forziere " + m + " mentre $n osserva in "
		          "silenzio.";
	}
	else if(scope == VAULT_SCOPE_CLAN) {
		to_char = "Imprimi il sigillo del clan su un forziere " + m +
		          ": ora risponde solo ai tuoi.";
		to_room = "$n imprime il sigillo del suo clan su un grande forziere " + m + ".";
	}
	act(to_char.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	act(to_room.c_str(), TRUE, ch, nullptr, nullptr, TO_ROOM);
}

/* ------------------------------------------------------------------ log */

/* Nome del PG per i log (mai nullptr). */
[[nodiscard]] const char* log_who(struct char_data* ch) {
	const char* const n = GET_NAME(ch);
	return n ? n : "?";
}

/* Una riga per tipo di oggetto spostato: chi, cassetta, piano, vnum, quantita'. */
void log_vault_moves(struct char_data* ch, const char* action, int scope, int tier,
                     const std::map<unsigned, long long>& moved) {
	const char* const who = log_who(ch);
	const char* const sc = scope_label(scope).data();
	const char* const tn = tier_name(tier).data();
	for(const auto& [vnum, qty] : moved) {
		const unsigned v = vnum;
		const long long q = qty;
		mudlog(LOG_PLAYERS, "vault %s: %s, cassetta %s (%s), vnum %u, qta %lld", action, who,
		       sc, tn, v, q);
	}
}

/* Pagamenti (tassa, arretrati): chi, cassetta, piano, monete, rune. */
void log_vault_pay(struct char_data* ch, const char* what, int scope, int tier, long long coins,
                   int runes) {
	const char* const who = log_who(ch);
	const char* const sc = scope_label(scope).data();
	const char* const tn = tier_name(tier).data();
	mudlog(LOG_PLAYERS, "vault %s: %s, cassetta %s (%s), monete %lld, rune %d", what, who, sc,
	       tn, coins, runes);
}

/* Acquisto/cambio piano. */
void log_vault_buy(struct char_data* ch, int scope, int tier, int mode, long long coins,
                   int runes) {
	const char* const who = log_who(ch);
	const char* const sc = scope_label(scope).data();
	const char* const tn = tier_name(tier).data();
	const char* const mn = mode_name(mode).data();
	mudlog(LOG_PLAYERS, "vault acquisto: %s, cassetta %s, piano %s, formula %s, monete %lld, rune %d",
	       who, sc, tn, mn, coins, runes);
}

void cmd_deposit(struct char_data* ch, const VaultCtx& c, const char* arg) {
	const CountArg parsed = parse_count(arg);
	const int want = parsed.want;
	const char* const name = parsed.name.c_str();
	if(!name[0]) {
		send_to_char("Che cosa vuoi depositare nella cassetta?\n\r", ch);
		return;
	}

	/* Fase 1: scelta oggetti, senza toglierli al giocatore. Come
	 * get_obj_in_list_vis: prima nome esatto, poi abbreviazione. */
	const bool every = (want == kWantAll && iequals(name, "all")); /* "all" da solo */
	std::vector<struct obj_data*> matches;
	for(const bool prefix : {false, true}) {
		for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
			if(CAN_SEE_OBJ(ch, obj) && (every || obj_keyword_or_owner_match(obj, name, prefix))) {
				matches.push_back(obj);
			}
		}
		if(!matches.empty()) {
			break;
		}
	}
	if(matches.empty()) {
		send_str(ch, every ? std::string("Non hai niente con te.\n\r")
		                   : fmt("Non hai nessun oggetto chiamato '%s'.\n\r", name));
		return;
	}
	std::vector<struct obj_data*> picked;
	if(want < 0) {
		if(static_cast<int>(matches.size()) < -want) {
			send_str(ch, fmt("Non hai %s oggetti chiamati '%s'.\n\r", num(-want), name));
			return;
		}
		picked.push_back(matches[static_cast<size_t>(-want - 1)]);
	}
	else {
		for(struct obj_data* obj : matches) {
			if(static_cast<int>(picked.size()) >= want) {
				break;
			}
			picked.push_back(obj);
		}
		if(want != kWantAll && static_cast<int>(picked.size()) < want) {
			send_str(ch, fmt("Ne hai chiesti %s, ma con te ne hai solo %s.\n\r", num(want),
			                 num(picked.size())));
		}
	}
	const size_t requested = picked.size();
	if(want == kWantAll) {
		/* all / all.nome: si saltano gli oggetti non ammessi e si riassume. */
		std::vector<struct obj_data*> allowed;
		for(struct obj_data* obj : picked) {
			if(deposit_check(ch, obj, true)) {
				allowed.push_back(obj);
			}
		}
		const size_t skipped = picked.size() - allowed.size();
		picked.swap(allowed);
		if(picked.empty()) {
			send_to_char("Nessuno di questi oggetti puo' entrare nella cassetta.\n\r", ch);
			return;
		}
		if(skipped > 0) {
			send_str(ch, fmt("Lasci con te %s oggett%s non ammess%s nella cassetta "
			                 "(contenitori non vuoti, oggetti unici o non adatti).\n\r",
			                 num(skipped), skipped == 1 ? "o" : "i", skipped == 1 ? "o" : "i"));
		}
	}
	else {
		/* Ogni oggetto deve poter entrare in cassetta: ci si ferma al primo
		 * non ammesso (deposit_check spiega il motivo). */
		size_t ok = 0;
		while(ok < picked.size() && deposit_check(ch, picked[ok])) {
			++ok;
		}
		picked.resize(ok);
		if(picked.empty()) {
			return;
		}
		if(picked.size() < requested) {
			send_str(ch, fmt("Ripongo solo i primi %s: gli altri non sono ammessi.\n\r",
			                 num(picked.size())));
		}
	}
	std::map<unsigned, long long> moved;
	for(struct obj_data* obj : picked) {
		moved[obj_vnum(obj)]++;
	}
	const long long n = static_cast<long long>(picked.size());

	/* Fase 2: una transazione (saldo, inventario PG, monete, storico). */
	int vault_tier = 0;
	long long gold = GET_GOLD(ch);
	long long paid = 0;
	long long fee = 0;
	{
		odb::transaction t(Sql::getMysql()->begin());
		t.tracer(logTracer);
		std::optional<VaultRow> vo = load_vault(c, true);
		if(!vo) {
			send_str(ch, no_vault_msg(c.scope));
			return; /* rollback */
		}
		VaultRow& v = *vo;
		vault_tier = v.tier;
		const std::optional<VaultPlan> po = load_plan(c.scope, v.tier);
		if(!po) {
			throw std::runtime_error("vault: plan missing");
		}
		const VaultPlan& p = *po;
		if(!settle(ch, c, v, p, gold, paid)) {
			return;
		}
		const long long stored = stored_count(v.id);
		if(stored + n > p.capacity) {
			const long long room_left = std::max(0LL, p.capacity - stored);
			send_str(ch, fmt("La cassetta %s non ha abbastanza posto: contiene %s oggetti su %s, "
			                 "ne entrano ancora %s.\n\r", scope_label(c.scope), num(stored),
			                 num(p.capacity), num(room_left)));
			return;
		}
		if(v.mode == VAULT_PAY_DEPFEE) {
			fee = n * p.deposit_fee;
		}
		if(gold < fee) {
			send_str(ch, fmt("Non hai abbastanza monete per la tassa di deposito: servono "
			                 "%s monete (%s per oggetto).\n\r", num(fee), num(p.deposit_fee)));
			return;
		}
		std::vector<unsigned long long> inv_ids;
		for(struct obj_data* obj : picked) {
			if(obj->db_inventory_id != 0) {
				inv_ids.push_back(obj->db_inventory_id);
			}
		}
		for(const auto& [vnum, qty] : moved) {
			vexec("INSERT INTO vault_item (vault_id, item_number, quantity) VALUES (" +
			      std::to_string(v.id) + "," + std::to_string(vnum) + "," +
			      std::to_string(qty) + ") ON DUPLICATE KEY UPDATE quantity = quantity + " +
			      std::to_string(qty));
			log_row(v.id, c.actor_id, "deposit", vnum, qty, 0);
		}
		vault_inventory_remove_rows_tx(std::to_string(c.actor_id), inv_ids);
		gold -= fee;
		if(fee > 0) {
			log_row(v.id, c.actor_id, "fee", 0, n, fee);
		}
		if(gold != GET_GOLD(ch)) {
			write_gold(c.actor_id, gold);
		}
		t.commit();
	}

	/* Fase 3: DB confermato, ora la memoria. obj_to_store = percorso di rent
	 * (conteggio limited + extract_obj). */
	static obj_file_u scratch;
	for(struct obj_data* obj : picked) {
		obj_from_char(obj);
		scratch.number = 0;
		obj_to_store(obj, &scratch, ch, 1);
	}
	GET_GOLD(ch) = static_cast<int>(gold);
	log_vault_moves(ch, "deposito", c.scope, vault_tier, moved);
	if(fee > 0) {
		log_vault_pay(ch, "tassa deposito", c.scope, vault_tier, fee, 0);
	}
	if(paid > 0) {
		log_vault_pay(ch, "arretrati", c.scope, vault_tier, paid, 0);
		send_str(ch, fmt("Hai pagato %s monete di canone arretrato.\n\r", num(paid)));
	}
	send_str(ch, fmt("Hai riposto %s oggett%s nella cassetta %s:\n\r", num(n),
	                 n == 1 ? "o" : "i", scope_label(c.scope)));
	send_moved_list(ch, moved);
	if(!moved.empty()) {
		act_room(ch, c.scope, vault_tier); /* solo dopo commit riuscito */
	}
	if(fee > 0) {
		send_str(ch, fmt("Tassa di deposito pagata: %s monete.\n\r", num(fee)));
	}
}

/* "carico/carica" concordato col sesso del personaggio. */
[[nodiscard]] std::string overweight_msg(struct char_data* ch) {
	return std::string("Sei gia' troppo caric") + (GET_SEX(ch) == SEX_FEMALE ? "a" : "o") +
	       ": non riesci a portare altro peso.\n\r";
}

void cmd_withdraw(struct char_data* ch, const VaultCtx& c, const char* arg) {
	const CountArg parsed = parse_count(arg);
	const int want = parsed.want;
	const char* const name = parsed.name.c_str();
	if(!name[0]) {
		send_to_char("Che cosa vuoi prelevare dalla cassetta?\n\r", ch);
		return;
	}

	const bool every = (want == kWantAll && iequals(name, "all")); /* "all" da solo */
	int vault_tier = 0;
	long long gold = GET_GOLD(ch);
	long long paid = 0;
	MadeGuard made;
	std::vector<unsigned> out;
	std::map<unsigned, long long> moved;
	std::vector<unsigned long long> inv_ids;
	{
		odb::transaction t(Sql::getMysql()->begin());
		t.tracer(logTracer);
		std::optional<VaultRow> vo = load_vault(c, true);
		if(!vo) {
			send_str(ch, no_vault_msg(c.scope));
			return;
		}
		VaultRow& v = *vo;
		vault_tier = v.tier;
		const std::optional<VaultPlan> po = load_plan(c.scope, v.tier);
		if(!po) {
			throw std::runtime_error("vault: plan missing");
		}
		const VaultPlan& p = *po;
		if(!settle(ch, c, v, p, gold, paid)) {
			return;
		}
		SqlRows items;
		if(!vsel("SELECT item_number, quantity FROM vault_item WHERE vault_id = " +
		         std::to_string(v.id) + " AND quantity > 0 ORDER BY item_number", items)) {
			throw std::runtime_error("vault: items failed");
		}
		if(items.empty()) {
			send_str(ch, fmt("La cassetta %s e' vuota.\n\r", scope_label(c.scope)));
			return;
		}
		/* Scelta per nome (keyword del prototipo): prima esatto, poi
		 * abbreviazione, come get_obj_in_list_vis. */
		for(const bool prefix : {false, true}) {
			long long seen = 0;
			for(const auto& r : items) {
				const unsigned vnum = static_cast<unsigned>(to_ll(r[0]));
				const long long qty = to_ll(r[1]);
				const std::string pname = proto_name(vnum);
				if(!every && !isname(name, pname.c_str()) &&
				   !(prefix && isname2(name, pname.c_str()))) {
					continue;
				}
				if(want < 0) {
					if(seen + qty >= -want) {
						out.push_back(vnum);
						break;
					}
					seen += qty;
				}
				else {
					for(long long k = 0; k < qty && static_cast<int>(out.size()) < want; ++k) {
						out.push_back(vnum);
					}
					if(static_cast<int>(out.size()) >= want) {
						break;
					}
				}
			}
			if(!out.empty()) {
				break;
			}
		}
		if(out.empty()) {
			send_str(ch, want < 0 ?
			             fmt("Nella cassetta non ci sono %s oggetti chiamati '%s'.\n\r", num(-want), name) :
			             fmt("Nella cassetta non c'e' nessun oggetto chiamato '%s'.\n\r", name));
			return;
		}
		if(want > 0 && want != kWantAll && static_cast<int>(out.size()) < want) {
			send_str(ch, fmt("Ne hai chiesti %s, ma nella cassetta ce ne sono solo %s.\n\r",
			                 num(want), num(out.size())));
		}
		/* Quanti ne porta, per numero e per peso (come get: totale <= massimo). */
		{
			const long long can_n = std::max(0, CAN_CARRY_N(ch) - IS_CARRYING_N(ch));
			long long w = IS_CARRYING_W(ch);
			long long can_w = 0;
			for(unsigned vnum : out) {
				const obj_data* proto = proto_of(vnum);
				w += proto ? proto->obj_flags.weight : 0;
				if(w > CAN_CARRY_W(ch)) {
					break;
				}
				++can_w;
			}
			const long long asked = static_cast<long long>(out.size());
			if(asked > can_n && std::min(can_n, can_w) == 0) {
				send_str(ch, can_n == 0 ? std::string("Hai le mani piene: non riesci a portare altri oggetti.\n\r")
				                        : overweight_msg(ch));
				return;
			}
			if(asked > can_w && can_w == 0) {
				send_str(ch, overweight_msg(ch));
				return;
			}
			if(asked > can_n) {
				send_str(ch, fmt("Non puoi trasportare %s oggett%s in piu': al massimo %s.\n\r",
				                 num(asked), asked == 1 ? "o" : "i", num(std::min(can_n, can_w))));
				return;
			}
			if(asked > can_w) {
				send_str(ch, fmt("Non riesci a portare tutto quel peso: puoi prelevarne al massimo %s.\n\r",
				                 num(can_w)));
				return;
			}
		}
		/* Oggetti dal prototipo (statistiche complete), non ancora consegnati. */
		for(unsigned vnum : out) {
			struct obj_data* obj =
				real_object(static_cast<int>(vnum)) >= 0 ? read_object(static_cast<int>(vnum), VIRTUAL)
				                                         : nullptr;
			if(!obj) {
				mudlog(LOG_SYSERR, "vault: read_object(%u) fallito per %s", vnum, GET_NAME(ch));
				send_to_char("Qualcosa nella cassetta non esiste piu'. Avvisa un Dio.\n\r", ch);
				return; /* MadeGuard estrae, rollback */
			}
			made.objs.push_back(obj);
		}
		for(unsigned vnum : out) {
			moved[vnum]++;
		}
		for(const auto& [vnum, qty] : moved) {
			if(vexec("UPDATE vault_item SET quantity = quantity - " + std::to_string(qty) +
			         " WHERE vault_id = " + std::to_string(v.id) + " AND item_number = " +
			         std::to_string(vnum) + " AND quantity >= " + std::to_string(qty)) != 1) {
				send_to_char("Nella cassetta non c'e' piu' quello che cerchi.\n\r", ch);
				return;
			}
			log_row(v.id, c.actor_id, "withdraw", vnum, qty, 0);
		}
		vexec("DELETE FROM vault_item WHERE vault_id = " + std::to_string(v.id) +
		      " AND quantity <= 0");
		if(!vault_inventory_add_rows_tx(ch, std::to_string(c.actor_id), made.objs, inv_ids) ||
		   inv_ids.size() != made.objs.size()) {
			throw std::runtime_error("vault: inventory rows failed");
		}
		if(gold != GET_GOLD(ch)) {
			write_gold(c.actor_id, gold);
		}
		t.commit();
	}

	/* DB confermato: correzione limited e consegna. */
	made.released = true;
	for(size_t k = 0; k < made.objs.size(); ++k) {
		struct obj_data* obj = made.objs[k];
#if LIMITED_ITEMS
		/* Come obj_store_to_char: il raro era gia' contato al deposito
		 * (obj_to_store), read_object lo ha contato di nuovo. */
		if(obj->item_number >= 0 && obj->obj_flags.cost >= LIM_ITEM_COST_MIN &&
		   !object_is_zone_limit_exempt(obj)) {
			obj_index[obj->item_number].number--;
		}
#endif
		obj->db_inventory_id = inv_ids[k];
		obj_to_char(obj, ch);
	}
	GET_GOLD(ch) = static_cast<int>(gold);
	log_vault_moves(ch, "prelievo", c.scope, vault_tier, moved);
	if(paid > 0) {
		log_vault_pay(ch, "arretrati", c.scope, vault_tier, paid, 0);
		send_str(ch, fmt("Hai pagato %s monete di canone arretrato.\n\r", num(paid)));
	}
	const size_t n = made.objs.size();
	send_str(ch, fmt("Hai prelevato %s oggett%s dalla cassetta %s:\n\r", num(n), n == 1 ? "o" : "i",
	                 scope_label(c.scope)));
	send_moved_list(ch, moved);
	if(!moved.empty()) {
		act_room(ch, c.scope, vault_tier); /* solo dopo commit riuscito */
	}
}

[[nodiscard]] std::string price_cell(long long coins, int runes) {
	if(coins <= 0) {
		return "-";
	}
	return runes > 0 ? group(coins) + "+" + std::to_string(runes) + "r" : group(coins);
}

void show_price_list(struct char_data* ch, const VaultCtx& c) {
	SqlRows rows;
	std::optional<VaultRow> v;
	{
		odb::transaction t(Sql::getMysql()->begin());
		v = load_vault(c, false);
		if(!vsel("SELECT tier, capacity, price_once, rent_month, price_buy, fee_month, "
		         "runes_once, runes_buy, deposit_fee FROM vault_plan WHERE scope = " +
		         std::to_string(c.scope) + " ORDER BY tier", rows)) {
			throw std::runtime_error("vault: price list failed");
		}
		t.commit();
	}
	const bool personal = (c.scope == VAULT_SCOPE_PERSONAL);
	std::vector<TableCol> cols = {
		{"Piano", false, kColTier},  {"Ogg.", true, kColHead},     {"Unico", true, kColNum},
		{"Affitto", true, kColNum},  {"Acquisto", true, kColNum},  {"Canone", true, kColNum},
	};
	if(personal) {
		cols.push_back({"Tassa", true, kColNum});
	}
	std::vector<TableRow> table;
	for(const auto& r : rows) {
		const long long buy = to_ll(r[4]);
		TableRow row = {
			std::string(tier_name(static_cast<int>(to_ll(r[0])))),
			group(to_ll(r[1])),
			price_cell(to_ll(r[2]), static_cast<int>(to_ll(r[6]))),
			price_cell(to_ll(r[3]), 0),
			price_cell(buy, static_cast<int>(to_ll(r[7]))),
			price_cell(buy > 0 ? to_ll(r[5]) : 0, 0),
		};
		if(personal) {
			row.push_back(price_cell(to_ll(r[8]), 0));
		}
		table.push_back(std::move(row));
	}
	send_str(ch, paint(kColTitle, fmt("Listino cassette di sicurezza %s", scope_label(c.scope))) +
	             "\n\r");
	send_table(ch, cols, table);
	send_to_char("Prezzi in monete. $c0010r$c0007 = rune degli Dei, $c0008-$c0007 = non "
	             "disponibile.\n\r"
	             "Affitto e canone si pagano ogni 30 giorni; il canone segue l'acquisto.\n\r",
	             ch);
	send_str(ch, fmt("Uso: %s\n\r",
	                 paint(kColCmd, fmt("%sbuy <piano> <unico|affitto|canone%s>",
	                                    scope_cmd(c.scope), personal ? "|deposito" : ""))));
	send_to_char("Si puo' solo salire di piano. Da unico a unico, o da canone a canone,\n\r"
	             "si paga la differenza; negli altri casi il nuovo piano per intero.\n\r",
	             ch);
	if(v) {
		send_str(ch, fmt("Piano attuale: %s (%s).\n\r", paint(kColTier, tier_name(v->tier)),
		                 mode_name(v->mode)));
	}
}

[[nodiscard]] int parse_tier(const std::string& s) {
	if(s.empty()) {
		return 0;
	}
	if(std::isdigit(static_cast<unsigned char>(s[0]))) {
		const std::optional<long long> n = parse_ll(s);
		return (n && *n >= 1 && *n <= kVaultTiers) ? static_cast<int>(*n) : 0;
	}
	for(int t = 1; t <= kVaultTiers; ++t) {
		if(is_abbrev(s.c_str(), tier_name(t).data())) {
			return t;
		}
	}
	return 0;
}

[[nodiscard]] int parse_mode(const std::string& s) {
	if(s.empty()) {
		return 0;
	}
	if(is_abbrev(s.c_str(), "unico") || is_abbrev(s.c_str(), "unica")) {
		return VAULT_PAY_ONCE;
	}
	if(is_abbrev(s.c_str(), "affitto")) {
		return VAULT_PAY_RENT;
	}
	if(is_abbrev(s.c_str(), "canone")) {
		return VAULT_PAY_BUYFEE;
	}
	if(is_abbrev(s.c_str(), "deposito")) {
		return VAULT_PAY_DEPFEE;
	}
	return 0;
}

void cmd_buy(struct char_data* ch, const VaultCtx& c, const char* arg) {
	if(c.scope == VAULT_SCOPE_CLAN && !c.is_leader) {
		send_to_char("Solo il principe del clan puo' acquistare la cassetta del clan.\n\r", ch);
		return;
	}
	const ArgSplit t = split_arg(arg ? arg : "");
	const std::string tier_tok = t.first;
	const std::string mode_tok = split_arg(t.rest).first;
	if(!tier_tok[0]) {
		show_price_list(ch, c);
		return;
	}
	const int tier = parse_tier(tier_tok);
	const int mode = parse_mode(mode_tok);
	if(tier == 0 && std::isdigit(static_cast<unsigned char>(tier_tok[0]))) {
		send_str(ch, fmt("Livello '%s' non valido: usa un numero da 1 a %d o un nome "
		                 "(bronzo, argento, oro, platino, mithril).\n\r", tier_tok, kVaultTiers));
		return;
	}
	if(tier == 0 || mode == 0) {
		send_str(ch, fmt("Uso: %sbuy <bronzo|argento|oro|platino|mithril> "
		                 "<unico|affitto|canone%s>\n\r", scope_cmd(c.scope),
		                 c.scope == VAULT_SCOPE_PERSONAL ? "|deposito" : ""));
		return;
	}
	if(mode == VAULT_PAY_DEPFEE && c.scope != VAULT_SCOPE_PERSONAL) {
		send_to_char("Il pagamento a deposito esiste solo per la cassetta personale.\n\r", ch);
		return;
	}

	long long cost = 0;
	int runes = 0;
	long long gold = GET_GOLD(ch);
	long long paid = 0;
	int paid_tier = tier; /* piano su cui maturano gli arretrati (quello vecchio) */
	{
		odb::transaction t(Sql::getMysql()->begin());
		t.tracer(logTracer);
		std::optional<VaultRow> cur = load_vault(c, true);
		const std::optional<VaultPlan> np = load_plan(c.scope, tier);
		std::optional<VaultPlan> op;
		if(cur) {
			op = load_plan(c.scope, cur->tier);
			if(!op) {
				throw std::runtime_error("vault: buy old plan missing");
			}
		}
		const bool available =
			np && ((mode == VAULT_PAY_ONCE && np->price_once > 0) ||
			       (mode == VAULT_PAY_RENT && np->rent_month > 0) ||
			       (mode == VAULT_PAY_BUYFEE && np->price_buy > 0) ||
			       (mode == VAULT_PAY_DEPFEE && np->deposit_fee > 0));
		if(!available) {
			send_str(ch, fmt("La formula %s non e' disponibile per il piano %s.\n\r",
			                 mode_name(mode), tier_name(tier)));
			return;
		}
		bool keep_paid_until = false;
		if(cur) {
			VaultRow& v = *cur;
			if(tier < v.tier) {
				send_to_char("Non puoi passare a un piano inferiore.\n\r", ch);
				return;
			}
			if(tier == v.tier && mode == v.mode) {
				send_to_char("Hai gia' questo piano.\n\r", ch);
				return;
			}
			/* Arretrati: solo con 'paga' (rollback se l'acquisto poi non va a
			 * buon fine). */
			if(!settle(ch, c, v, *op, gold, paid)) {
				return;
			}
			paid_tier = v.tier;
		}
		switch(mode) {
		case VAULT_PAY_ONCE:
			cost = np->price_once;
			runes = np->runes_once;
			if(cur && cur->mode == VAULT_PAY_ONCE) {
				cost = std::max(0LL, np->price_once - op->price_once);
				runes = std::max(0, np->runes_once - op->runes_once);
			}
			break;
		case VAULT_PAY_BUYFEE:
			cost = np->price_buy;
			runes = np->runes_buy;
			if(cur && cur->mode == VAULT_PAY_BUYFEE) {
				cost = std::max(0LL, np->price_buy - op->price_buy);
				runes = std::max(0, np->runes_buy - op->runes_buy);
				keep_paid_until = true;
			}
			break;
		case VAULT_PAY_RENT:
			cost = np->rent_month; /* primo mese anticipato */
			break;
		default:
			cost = 0;
			break;
		}
		if(runes > 0 && static_cast<int>(GET_RUNEDEI(ch)) < runes) {
			send_str(ch, fmt("Ti servono %s rune degli Dei (ne hai %s).\n\r", num(runes),
			                 num(static_cast<int>(GET_RUNEDEI(ch)))));
			return;
		}
		if(gold < cost) {
			send_str(ch, fmt("Ti servono %s monete con te (ne hai %s).\n\r", num(cost), num(gold)));
			return;
		}
		const bool monthly = (mode == VAULT_PAY_RENT || mode == VAULT_PAY_BUYFEE);
		const std::string next = "DATE_ADD(NOW(), INTERVAL " + std::to_string(kVaultPeriodDays) + " DAY)";
		const std::string ins_paid = monthly ? next : "NULL";
		const std::string upd_paid = keep_paid_until ? "paid_until" : ins_paid;
		/* toon_id/user_id: FK ON DELETE CASCADE (cancellazione PG / account). */
		const std::string owner = std::to_string(c.owner_id);
		const bool by_user = (c.scope == VAULT_SCOPE_ACCOUNT);
		vexec("INSERT INTO vault (scope, owner_id, toon_id, user_id, tier, pay_mode, paid_until) "
		      "VALUES (" + std::to_string(c.scope) + "," + owner + "," +
		      (by_user ? std::string("NULL") : owner) + "," +
		      (by_user ? owner : std::string("NULL")) + "," +
		      std::to_string(tier) + "," + std::to_string(mode) + "," + ins_paid +
		      ") ON DUPLICATE KEY UPDATE tier = " + std::to_string(tier) + ", pay_mode = " +
		      std::to_string(mode) + ", paid_until = " + upd_paid);
		const std::optional<VaultRow> nv = load_vault(c, false);
		if(!nv) {
			throw std::runtime_error("vault: buy reload failed");
		}
		if(runes > 0) {
			vexec("UPDATE character_stats SET p_rune_dei = " +
			      std::to_string(static_cast<int>(GET_RUNEDEI(ch)) - runes) +
			      " WHERE toon_id = " + std::to_string(c.actor_id));
		}
		gold -= cost;
		if(cost > 0 || paid > 0) {
			write_gold(c.actor_id, gold);
		}
		log_row(nv->id, c.actor_id, "buy", 0, tier * 10 + mode, cost);
		t.commit();
	}

	GET_GOLD(ch) = static_cast<int>(gold);
	if(runes > 0) {
		GET_RUNEDEI(ch) = static_cast<ush_int>(static_cast<int>(GET_RUNEDEI(ch)) - runes);
	}
	if(paid > 0) {
		log_vault_pay(ch, "arretrati", c.scope, paid_tier, paid, 0);
		send_str(ch, fmt("Hai pagato %s monete di canone arretrato.\n\r", num(paid)));
	}
	log_vault_buy(ch, c.scope, tier, mode, cost, runes);
	act_buy(ch, c.scope, tier);
	send_str(ch, fmt("Ora hai una cassetta di sicurezza %s, piano %s (%s).\n\r",
	                 scope_label(c.scope), tier_name(tier), mode_name(mode)));
	if(cost > 0 || runes > 0) {
		const std::string rune_part = runes > 0 ? fmt(" e %s rune degli Dei", num(runes)) : "";
		send_str(ch, fmt("Hai pagato %s monete%s.\n\r", num(cost), rune_part));
	}
}

/* "paga" come ultima parola = conferma degli arretrati; out = argomento senza. */
[[nodiscard]] bool strip_confirm(std::string_view in, std::string& out) {
	while(!in.empty() && std::isspace(static_cast<unsigned char>(in.back()))) {
		in.remove_suffix(1);
	}
	const size_t sp = in.find_last_of(" \t");
	const std::string_view last = (sp == std::string_view::npos) ? in : in.substr(sp + 1);
	const bool confirm = iequals(last, "paga");
	if(confirm) {
		in = in.substr(0, sp == std::string_view::npos ? 0 : sp);
	}
	out.assign(in.data(), in.size());
	return confirm;
}

/* "depo", "with"...: abbreviazione (non la parola intera) di deposit/withdraw. */
[[nodiscard]] bool is_partial_hist(std::string_view w) {
	const auto partial = [w](std::string_view full) {
		return !w.empty() && w.size() < full.size() && iequals(w, full.substr(0, w.size()));
	};
	return partial("deposit") || partial("withdraw");
}

[[nodiscard]] std::string spell_out_hist_msg(int scope) {
	return fmt("Scrivi per intero: %sbalance deposit oppure %sbalance withdraw.\n\r",
	           scope_cmd(scope), scope_cmd(scope));
}

/* Dispatch comune per account/clan: sub = deposit|withdraw|balance|buy. */
[[nodiscard]] bool dispatch_scope(struct char_data* ch, int scope, std::string_view sub,
                                  const char* rest) {
	/* Abbreviazioni: "d"/"w" bastano; "b" e' ambiguo (balance/buy), servono
	 * almeno "ba" / "bu". Nessun sottocomando di do_clan inizia con d/w/b. */
	const auto abbrev = [sub](std::string_view full, size_t min_len) {
		return sub.size() >= min_len && sub.size() <= full.size() &&
		       iequals(sub, full.substr(0, sub.size()));
	};
	const bool is_dep = abbrev("deposit", 1);
	const bool is_wd = abbrev("withdraw", 1);
	const bool is_bal = abbrev("balance", 2);
	const bool is_buy = abbrev("buy", 2);
	if(!is_dep && !is_wd && !is_bal && !is_buy) {
		return false;
	}
	if(scope == VAULT_SCOPE_CLAN && !vault_room_is_clan(ch)) {
		send_to_char("La cassetta del clan si usa solo nella banca del clan.\n\r", ch);
		return true;
	}
	std::optional<VaultCtx> ctx = resolve_ctx(ch, scope);
	if(!ctx) {
		return true;
	}
	VaultCtx& c = *ctx;
	std::string args;
	c.confirm = strip_confirm(rest ? rest : "", args);
	if(is_dep) {
		cmd_deposit(ch, c, args.c_str());
	}
	else if(is_wd) {
		cmd_withdraw(ch, c, args.c_str());
	}
	else if(is_buy) {
		cmd_buy(ch, c, args.c_str());
	}
	else {
		/* Clan: balance (contenuto) per tutti i membri; lo storico
		 * balance deposit|withdraw solo per il principe. */
		const std::string hist = split_arg(rest ? rest : "").first;
		const std::string_view h = hist;
		if(is_partial_hist(h)) {
			send_str(ch, spell_out_hist_msg(scope));
			return true;
		}
		if(iequals(h, "deposit") || iequals(h, "withdraw")) {
			if(scope == VAULT_SCOPE_CLAN && !c.is_leader) {
				send_to_char("Solo il principe puo' vedere i movimenti della cassetta del "
				             "clan.\n\r", ch);
				return true;
			}
			cmd_history(ch, c, iequals(h, "deposit") ? "deposit" : "withdraw");
		}
		else {
			cmd_list(ch, c);
		}
	}
	return true;
}

[[nodiscard]] int handle(struct char_data* ch, int cmd, const char* arg) {
	const char* a = arg ? arg : "";
	const ArgSplit split = split_arg(a);
	const std::string& first = split.first;
	const char* const rest = split.rest.c_str();

	if(cmd == CMD_ACCOUNT) {
		if(!dispatch_scope(ch, VAULT_SCOPE_ACCOUNT, first, rest)) {
			send_to_char("Uso: account <deposit|withdraw|balance|buy> [argomenti]\n\r", ch);
		}
		return TRUE;
	}
	if(cmd == CMD_CLAN) {
		return dispatch_scope(ch, VAULT_SCOPE_CLAN, first, rest) ? TRUE : FALSE;
	}
	const std::string_view fw = first;
	if(cmd == CMD_BALANCE && is_partial_hist(fw)) {
		send_str(ch, spell_out_hist_msg(VAULT_SCOPE_PERSONAL));
		return TRUE;
	}
	if(cmd == CMD_BALANCE && !fw.empty() && !iequals(fw, "deposit") && !iequals(fw, "withdraw")) {
		/* Nessun passaggio alla banca nella stanza cassette. */
		send_to_char("Uso: balance [deposit|withdraw]  (account balance / clan balance per le "
		             "altre cassette)\n\r", ch);
		return TRUE;
	}
	if(cmd != CMD_VAULT && cmd != CMD_DEPOSIT && cmd != CMD_WITHDRAW && cmd != CMD_BALANCE &&
	   cmd != CMD_BUY) {
		return FALSE;
	}
	std::optional<VaultCtx> ctx = resolve_ctx(ch, VAULT_SCOPE_PERSONAL);
	if(!ctx) {
		return TRUE;
	}
	VaultCtx& c = *ctx;
	std::string args;
	c.confirm = strip_confirm(a, args);
	switch(cmd) {
	case CMD_DEPOSIT:
		cmd_deposit(ch, c, args.c_str());
		break;
	case CMD_WITHDRAW:
		cmd_withdraw(ch, c, args.c_str());
		break;
	case CMD_BUY:
		cmd_buy(ch, c, args.c_str());
		break;
	case CMD_BALANCE:
		if(first[0]) {
			cmd_history(ch, c, iequals(fw, "deposit") ? "deposit" : "withdraw");
			break;
		}
		cmd_list(ch, c);
		break;
	default:
		cmd_list(ch, c);
		break;
	}
	return TRUE;
}

} // namespace vaultapi

int vault_room_command(struct char_data* ch, int cmd, const char* arg) {
	if(ch == nullptr || IS_NPC(ch)) {
		return FALSE;
	}
	try {
		return handle(ch, cmd, arg);
	}
	catch(const std::exception& e) {
		const char* who = GET_NAME(ch) ? GET_NAME(ch) : "?";
		const char* what = e.what();
		mudlog(LOG_SYSERR, "vault (%s): %s", who, what);
		send_to_char("Errore della cassetta di sicurezza. Avvisa un Dio.\n\r", ch);
		return TRUE;
	}
}

void vault_plans_seed(::odb::database* db) {
	if(db == nullptr) {
		return;
	}
	for(const VaultPlanSeed& s : kVaultPlanSeeds) {
		db->execute(("INSERT IGNORE INTO vault_plan (scope, tier, capacity, price_once, "
		             "rent_month, price_buy, fee_month, runes_once, runes_buy, deposit_fee) "
		             "VALUES (" + std::to_string(s.scope) + "," + std::to_string(s.tier) + "," +
		             std::to_string(s.capacity) + "," + std::to_string(s.price_once) + "," +
		             std::to_string(s.rent_month) + "," + std::to_string(s.price_buy) + "," +
		             std::to_string(s.fee_month) + "," + std::to_string(s.runes_once) + "," +
		             std::to_string(s.runes_buy) + "," + std::to_string(s.deposit_fee) + ")")
		                .c_str());
	}
}

/* ------------------------------------------------- cancellazione PG */

void vault_forget_toon(const char* name) {
	if(name && *name) {
		toon_cache().erase(name_key(name));
	}
}

VaultItemList vault_collect_toon_items_tx(unsigned long long toon_id) {
	VaultItemList out;
	SqlRows rows;
	if(!vsel("SELECT vi.item_number, SUM(vi.quantity) FROM vault_item vi "
	         "INNER JOIN vault v ON v.id = vi.vault_id WHERE v.toon_id = " +
	         std::to_string(toon_id) + " AND vi.quantity > 0 GROUP BY vi.item_number",
	         rows)) {
		throw std::runtime_error("vault: collect items failed");
	}
	for(const auto& r : rows) {
		out.emplace_back(static_cast<int>(to_ll(r[0])), to_ll(r[1]));
	}
	return out;
}

VaultClanInfo vault_clan_info_tx(unsigned long long toon_id) {
	VaultClanInfo out;
	SqlRows rows;
	/* Cassetta del clan di cui il toon e' il principe (owner_id = toon.id). */
	if(!vsel("SELECT v.id, COALESCE(SUM(vi.quantity), 0) FROM vault v "
	         "LEFT JOIN vault_item vi ON vi.vault_id = v.id AND vi.quantity > 0 "
	         "WHERE v.scope = " + std::to_string(static_cast<unsigned int>(VAULT_SCOPE_CLAN)) + " AND v.owner_id = " +
	         std::to_string(toon_id) + " GROUP BY v.id", rows)) {
		throw std::runtime_error("vault: clan info failed");
	}
	if(!rows.empty()) {
		out.exists = true;
		out.items = to_ll(rows[0][1]);
	}
	return out;
}

void vault_release_limited(const VaultItemList& items) {
#if LIMITED_ITEMS
	/* Come il prelievo: i rari in cassetta sono contati (boot e deposito). */
	for(const auto& [vnum, qty] : items) {
		const struct obj_data* proto = proto_of(static_cast<unsigned>(vnum));
		if(proto == nullptr || proto->obj_flags.cost < LIM_ITEM_COST_MIN ||
		   object_is_zone_limit_exempt(proto)) {
			continue;
		}
		const int rnum = real_object(vnum);
		if(rnum >= 0) {
			obj_index[rnum].number =
				static_cast<int>(std::max(0LL, static_cast<long long>(obj_index[rnum].number) - qty));
		}
	}
#else
	(void)items;
#endif
}

void vault_acquire_limited(const VaultItemList& items) {
#if LIMITED_ITEMS
	/* Stessa condizione del boot (CountLimitedItemsMysql) e di
	 * vault_release_limited: i rari in cassetta contano nel mondo. */
	for(const auto& [vnum, qty] : items) {
		const struct obj_data* proto = proto_of(static_cast<unsigned>(vnum));
		if(proto == nullptr || proto->obj_flags.cost < LIM_ITEM_COST_MIN ||
		   object_is_zone_limit_exempt(proto)) {
			continue;
		}
		const int rnum = real_object(vnum);
		if(rnum >= 0) {
			obj_index[rnum].number =
				static_cast<int>(static_cast<long long>(obj_index[rnum].number) + qty);
		}
	}
#else
	(void)items;
#endif
}

#else /* !USE_MYSQL */

int vault_room_command(struct char_data* ch, int cmd, const char* arg) {
	(void)ch;
	(void)cmd;
	(void)arg;
	return FALSE;
}

void vault_plans_seed(::odb::database* db) {
	(void)db;
}

VaultItemList vault_collect_toon_items_tx(unsigned long long toon_id) {
	(void)toon_id;
	return {};
}

void vault_forget_toon(const char* name) {
	(void)name;
}

VaultClanInfo vault_clan_info_tx(unsigned long long toon_id) {
	(void)toon_id;
	return {};
}

void vault_release_limited(const VaultItemList& items) {
	(void)items;
}

void vault_acquire_limited(const VaultItemList& items) {
	(void)items;
}

#endif /* USE_MYSQL */

#if USE_MYSQL
namespace {

/* true se argument e' un intero puro > 0 (deposito/prelievo monete). */
bool vault_arg_is_number(const char* argument) {
	while(argument && *argument == ' ') {
		++argument;
	}
	const std::string_view a = argument ? argument : "";
	if(!all_digits(a)) {
		return false;
	}
	/* Fuori intervallo resta un importo (lo gestisce la banca monete). */
	const std::optional<long long> n = parse_ll(a);
	return !n || *n > 0;
}

} // namespace
#endif

/* Stanza cassette di sicurezza: solo oggetti e piani (piu' sopra). L'oro
 * non passa mai alla banca da qui. */
ROOMSPECIAL_FUNC(vault) {
#if !USE_MYSQL
	(void)ch;
	(void)cmd;
	(void)arg;
	(void)room;
	(void)type;
	return FALSE;
#else
	if(type != EVENT_COMMAND || IS_NPC(ch)) {
		return FALSE;
	}
	(void)room;
	if((cmd == CMD_DEPOSIT || cmd == CMD_WITHDRAW) && vault_arg_is_number(arg)) {
		send_to_char("Qui ci sono le cassette di sicurezza: per l'oro vai in banca.\n\r", ch);
		return TRUE;
	}
	return vault_room_command(ch, cmd, arg);
#endif
}

} // namespace Alarmud
