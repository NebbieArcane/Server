/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPEC_BANK_HPP
#define __SPEC_BANK_HPP
/***************************  System  include ************************************/
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
/***************************  Local    include ************************************/
#include "config.hpp"
#include "flags.hpp"
#include "typedefs.hpp"
#include "logging.hpp"

namespace odb {
class database;
}

namespace Alarmud {

struct char_data;
struct obj_data;
struct room_data;

/* Banca monete (deposit/withdraw/balance). */
ROOMSPECIAL_FUNC(bank);
/* Stanza cassette di sicurezza: monete alla banca, oggetti e piani al vault. */
ROOMSPECIAL_FUNC(vault);

/* ------------------------------------------------ cassette di sicurezza */

/** Proprietario della cassetta: toon, account (toon.owner_id), clan (toon del principe). */
enum VaultScope {
	VAULT_SCOPE_PERSONAL = 0,
	VAULT_SCOPE_ACCOUNT = 1,
	VAULT_SCOPE_CLAN = 2
};

/** Formula di pagamento della cassetta. */
enum VaultPayMode {
	VAULT_PAY_ONCE = 1,    /* acquisto unico */
	VAULT_PAY_RENT = 2,    /* affitto mensile, nessun acquisto */
	VAULT_PAY_BUYFEE = 3,  /* acquisto ridotto + canone mensile */
	VAULT_PAY_DEPFEE = 4   /* nessun acquisto, tassa per oggetto (solo personale) */
};

/** Riga del listino base (spec_bank_vault.inc). */
struct VaultPlanSeed {
	int scope;
	int tier;
	int capacity;
	long long price_once;
	long long rent_month;
	long long price_buy;
	long long fee_month;
	int runes_once;
	int runes_buy;
	long long deposit_fee;
};

/**
 * Comandi della stanza cassette (special proc "vault"): deposit, withdraw,
 * balance, vault, buy (personale), account <...>, clan <deposit|withdraw|
 * balance|buy> (solo con flag stanza IS_CLAN). TRUE se il comando e' gestito.
 */
[[nodiscard]] int vault_room_command(struct char_data* ch, int cmd, const char* arg);

/** Heal: inserisce in vault_plan le righe mancanti del listino base (INSERT IGNORE). */
void vault_plans_seed(::odb::database* db);

/** (vnum, quantita') degli oggetti nelle cassette di un toon. */
using VaultItemList = std::vector<std::pair<int, long long>>;

/** Nella transazione corrente: oggetti delle cassette personale e di clan del
 *  toon (vault.toon_id), da chiamare prima di cancellare il toon. */
[[nodiscard]] VaultItemList vault_collect_toon_items_tx(unsigned long long toon_id);

/** Cassetta del clan di cui il toon e' il principe (vault.owner_id). */
struct VaultClanInfo {
	bool exists = false;   /* riga vault di scope clan presente */
	long long items = 0;   /* oggetti custoditi */
};

/** Nella transazione corrente: stato della cassetta del clan del principe
 *  `toon_id`. Con la cancellazione del toon la cascade la elimina con gli oggetti. */
[[nodiscard]] VaultClanInfo vault_clan_info_tx(unsigned long long toon_id);

/** Toon cancellato: toglie nome -> id dalla cache delle cassette. */
void vault_forget_toon(const char* name);

/** Dopo il commit della cancellazione: scala i contatori dei limited. */
void vault_release_limited(const VaultItemList& items);

/** Contrario di vault_release_limited: oggetti che entrano in cassetta senza
 *  passare dal deposito (comando wiz give). Aggiunge ai contatori dei limited
 *  la quantita' (come il conteggio del boot). Da chiamare dopo il commit. */
void vault_acquire_limited(const VaultItemList& items);

/* ------------------------------------------------------------------
 * Strumenti condivisi tra le cassette (spec_bank.cpp) e i comandi wiz
 * vaultplan/vaultinfo (act.wizard.cpp). Le definizioni sono in spec_bank.cpp
 * (solo con USE_MYSQL); i comandi wiz le usano solo sotto #if USE_MYSQL.
 * ------------------------------------------------------------------ */
namespace vaultapi {

using SqlRows = std::vector<std::vector<std::string>>;

/** Contesto di una cassetta: tipo, proprietario (toon / account / principe), attore. */
struct VaultCtx {
	int scope = VAULT_SCOPE_PERSONAL;
	unsigned long long owner_id = 0;
	unsigned long long actor_id = 0;
	bool is_leader = false;
	bool confirm = false; /* comando ripetuto con "paga": addebita gli arretrati */
};

/** Riga del listino (tabella vault_plan). */
struct VaultPlan {
	int capacity = 0;
	long long price_once = 0;
	long long rent_month = 0;
	long long price_buy = 0;
	long long fee_month = 0;
	int runes_once = 0;
	int runes_buy = 0;
	long long deposit_fee = 0;
};

/** Riga della tabella vault (con scadenza gia' calcolata). */
struct VaultRow {
	unsigned long long id = 0;
	int tier = 0;
	int mode = 0;
	long long overdue_secs = -1; /* >= 0: canone scaduto da tanti secondi */
	std::string paid_until;       /* dd/mm/yyyy o vuoto */
};

/** one_argument (legacy) con risultato in stringhe: primo token e resto. */
struct ArgSplit {
	std::string first;
	std::string rest;
};

/** Colonna di send_table; le celle sono std::vector<std::string> (TableRow). */
struct TableCol {
	std::string_view head;
	bool right;
	std::string_view color;
};
using TableRow = std::vector<std::string>;

inline constexpr int kVaultTiers = 5;

/* Colori ($cNNNN come nel resto del MUD). */
inline constexpr std::string_view kColTitle = "$c0005";
inline constexpr std::string_view kColHead = "$c0015";
inline constexpr std::string_view kColTier = "$c0011";
inline constexpr std::string_view kColNum = "$c0010";
inline constexpr std::string_view kColCmd = "$c0014";
inline constexpr std::string_view kBankErr = "$c0009"; /* errori: rosso */

/** boost::format (my_fmt, come mudlog). Il fold applica % a un argomento
 *  alla volta: niente problemi di precedenza con ?: o operatori. */
template <typename... Args>
[[nodiscard]] std::string fmt(const std::string& f, const Args&... args) {
	boost::format bf = my_fmt(f);
	(void)(bf % ... % args);
	return bf.str();
}

/* Nomi (letterali: .data() e' terminato da '\0'). */
[[nodiscard]] std::string_view tier_name(int tier);
[[nodiscard]] std::string_view mode_name(int mode);
[[nodiscard]] std::string_view scope_label(int scope);

/* Testo e tabelle. */
void send_str(struct char_data* ch, const std::string& s);
[[nodiscard]] bool iequals(std::string_view a, std::string_view b);
[[nodiscard]] std::string paint(std::string_view color, std::string_view text);
[[nodiscard]] std::string pad(std::string_view text, size_t w, bool right);
[[nodiscard]] std::string group(long long n); /* 1234567 -> "1.234.567" */
[[nodiscard]] std::string num(long long n);
[[nodiscard]] std::string num(size_t n);
[[nodiscard]] std::string num(int n);
void send_table(struct char_data* ch, const std::vector<TableCol>& cols,
                const std::vector<TableRow>& rows, std::string_view gap = "  ");

/* Argomenti. */
[[nodiscard]] ArgSplit split_arg(const std::string& s);
[[nodiscard]] std::optional<long long> parse_ll(std::string_view s);
[[nodiscard]] long long to_ll(const std::string& s);
[[nodiscard]] int parse_tier(const std::string& s);

/* DB: vsel/vexec usano la transazione corrente (Sql::getMysql()). */
[[nodiscard]] bool vsel(const std::string& sql, SqlRows& rows);
unsigned long long vexec(const std::string& sql);
[[nodiscard]] std::optional<VaultPlan> load_plan(int scope, int tier);
[[nodiscard]] std::optional<VaultRow> load_vault(const VaultCtx& c, bool lock);
[[nodiscard]] long long monthly_due(const VaultRow& v, const VaultPlan& p);
[[nodiscard]] long long months_owed(const VaultRow& v);
[[nodiscard]] long long stored_count(unsigned long long vault_id);
/* vault_log: una riga (azione <= 16 caratteri, colonna VARCHAR(16)). */
void log_row(unsigned long long vault_id, unsigned long long actor, const char* action,
             unsigned item_number, long long quantity, long long amount);

/* Oggetti e log. */
[[nodiscard]] const struct obj_data* proto_of(unsigned vnum);
[[nodiscard]] std::string proto_short_desc(unsigned vnum);
/* Stesso criterio del deposito (deposit_check): oggetto "foglia" nuovo con
 * ITEM2_VAULT sul prototipo, non zone-limit-exempt. false se il vnum non esiste. */
[[nodiscard]] bool proto_vault_allowed(unsigned vnum);
[[nodiscard]] const char* log_who(struct char_data* ch);

} // namespace vaultapi

} // namespace Alarmud
#endif // __SPEC_BANK_HPP
