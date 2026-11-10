/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "toon_rename.hpp"

#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"

#include "act.wizard.hpp"
#include "Sql.hpp"
#include "clan_symbol.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "interpreter.hpp"
#include "legacy_loader.hpp"
#include "mail.hpp"
#include "reception.hpp"
#include "toon_nuke_blacklist.hpp"
#include "utility.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#if USE_MYSQL
#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {
namespace fs = std::filesystem;

namespace {

std::string ascii_lower(const std::string& in) {
	std::string s = in;
	for(char& c : s) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return s;
}

std::string canonicalize_pc_name(const std::string& lower_name) {
	std::string s = ascii_lower(lower_name);
	if(!s.empty()) {
		s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
	}
	return s;
}

bool names_equal_ci(const char* a, const char* b) {
	if(!a || !b) {
		return false;
	}
	return strcasecmp(a, b) == 0;
}

} /* anonymous */

bool replace_ed_tokens_in_string(std::string& keywords, const char* old_name, const char* new_name) {
	if(keywords.empty() || !old_name || !*old_name || !new_name || !*new_name) {
		return false;
	}
	const size_t old_len = std::strlen(old_name);
	std::string out;
	out.reserve(keywords.size() + 16);
	bool changed = false;
	bool first = true;
	const char* p = keywords.c_str();
	while(*p) {
		while(*p && std::isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if(!*p) {
			break;
		}
		const char* start = p;
		while(*p && !std::isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		const size_t tok_len = static_cast<size_t>(p - start);
		if(!first) {
			out.push_back(' ');
		}
		first = false;
		if(tok_len > 2 && start[0] == 'E' && start[1] == 'D') {
			const size_t n = tok_len - 2;
			if(n == old_len && strncasecmp(start + 2, old_name, n) == 0) {
				out += "ED";
				out += new_name;
				changed = true;
				continue;
			}
		}
		out.append(start, tok_len);
	}
	if(changed) {
		keywords = std::move(out);
	}
	return changed;
}

bool toon_fix_player_dat_file(const char* path, const char* old_name, const char* new_name) {
	if(!path || !*path || !old_name || !*old_name || !new_name || !*new_name) {
		return false;
	}
	char_file_u st {};
	if(!legacy_load_char_file_path(path, st)) {
		return false;
	}
	bool changed = false;
	if(names_equal_ci(st.name, old_name)) {
		std::strncpy(st.name, new_name, sizeof(st.name) - 1);
		st.name[sizeof(st.name) - 1] = '\0';
		changed = true;
	}
	if(!changed) {
		return false;
	}
	FILE* fp = std::fopen(path, "wb");
	if(!fp) {
		mudlog(LOG_SYSERR, "toon_rename: cannot rewrite %s", path);
		return false;
	}
	const size_t n = std::fwrite(&st, sizeof(st), 1, fp);
	std::fclose(fp);
	if(n != 1) {
		mudlog(LOG_SYSERR, "toon_rename: short write on %s", path);
		return false;
	}
	return true;
}

bool toon_fix_rent_file(const char* path, const char* old_name, const char* new_name) {
	if(!path || !*path || !old_name || !*old_name || !new_name || !*new_name) {
		return false;
	}
	obj_file_u st {};
	if(!legacy_load_rent_file_path(path, st)) {
		return false;
	}
	bool changed = false;
	if(names_equal_ci(st.owner, old_name)) {
		std::strncpy(st.owner, new_name, sizeof(st.owner) - 1);
		st.owner[sizeof(st.owner) - 1] = '\0';
		changed = true;
	}
	for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
		std::string kw = st.objects[i].name;
		if(replace_ed_tokens_in_string(kw, old_name, new_name)) {
			std::strncpy(st.objects[i].name, kw.c_str(), sizeof(st.objects[i].name) - 1);
			st.objects[i].name[sizeof(st.objects[i].name) - 1] = '\0';
			changed = true;
		}
	}
	if(!changed) {
		return false;
	}
	FILE* fp = std::fopen(path, "w+b");
	if(!fp) {
		mudlog(LOG_SYSERR, "toon_rename: cannot rewrite rent %s", path);
		return false;
	}
	WriteObjs(fp, &st);
	std::fclose(fp);
	return true;
}

#if USE_MYSQL

namespace {

std::string sql_quote(MYSQL* h, const std::string& in) {
	std::string esc;
	esc.resize(in.size() * 2 + 1);
	unsigned long n = 0;
	if(h) {
		n = mysql_real_escape_string(h, esc.data(), in.c_str(),
									 static_cast<unsigned long>(in.size()));
	}
	else {
		n = 0;
		for(char c : in) {
			if(c == '\'' || c == '\\') {
				esc[n++] = (c == '\\') ? '\\' : '\'';
			}
			esc[n++] = static_cast<char>(c);
		}
	}
	esc.resize(n);
	return "'" + esc + "'";
}

MYSQL* current_tx_handle() {
	if(!odb::transaction::has_current()) {
		return nullptr;
	}
	odb::connection& conn = odb::transaction::current().connection();
	return static_cast<odb::mysql::connection&>(conn).handle();
}

bool mysql_exec(MYSQL* h, const std::string& sql, const char* what) {
	if(!h) {
		return false;
	}
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "toon_rename: %s: %s [%s]", what, mysql_error(h), sql.c_str());
		return false;
	}
	/* Consume any result so the connection stays usable. */
	if(MYSQL_RES* res = mysql_store_result(h)) {
		mysql_free_result(res);
	}
	return true;
}

bool table_exists_tx(MYSQL* h, const char* table) {
	if(!h || !table) {
		return false;
	}
	std::string sql =
		"SELECT COUNT(*) FROM information_schema.TABLES "
		"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = ";
	sql += sql_quote(h, table);
	if(mysql_query(h, sql.c_str()) != 0) {
		return false;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return false;
	}
	bool ok = false;
	if(MYSQL_ROW row = mysql_fetch_row(res)) {
		ok = row[0] && std::strcmp(row[0], "0") != 0;
	}
	mysql_free_result(res);
	return ok;
}

unsigned long long query_ull(MYSQL* h, const std::string& sql) {
	if(!h || mysql_query(h, sql.c_str()) != 0) {
		return 0;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return 0;
	}
	unsigned long long n = 0;
	if(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0]) {
			n = std::strtoull(row[0], nullptr, 10);
		}
	}
	mysql_free_result(res);
	return n;
}

bool query_string(MYSQL* h, const std::string& sql, std::string& out) {
	out.clear();
	if(!h || mysql_query(h, sql.c_str()) != 0) {
		return false;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return false;
	}
	bool ok = false;
	if(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0]) {
			out = row[0];
			ok = true;
		}
	}
	mysql_free_result(res);
	return ok;
}

void update_name_column(MYSQL* h, const char* table, const char* column, const std::string& old_q,
						const std::string& new_q, int& warnings, std::string& summary) {
	if(!table_exists_tx(h, table)) {
		return;
	}
	std::string sql = "UPDATE `";
	sql += table;
	sql += "` SET `";
	sql += column;
	sql += "` = ";
	sql += new_q;
	sql += " WHERE LOWER(`";
	sql += column;
	sql += "`) = LOWER(";
	sql += old_q;
	sql += ")";
	if(!mysql_exec(h, sql, table)) {
		++warnings;
		return;
	}
	const my_ulonglong n = mysql_affected_rows(h);
	if(n > 0) {
		summary += " ";
		summary += table;
		summary += ".";
		summary += column;
		summary += "=";
		summary += std::to_string(static_cast<unsigned long long>(n));
	}
}

bool update_ed_column(MYSQL* h, const char* table, const char* id_col, const char* name_col,
					  const char* old_name, const char* new_name, int& warnings, std::string& summary) {
	if(!table_exists_tx(h, table)) {
		return true;
	}
	std::string sql = "SELECT `";
	sql += id_col;
	sql += "`, `";
	sql += name_col;
	sql += "` FROM `";
	sql += table;
	sql += "` WHERE `";
	sql += name_col;
	sql += "` LIKE '%ED%'";
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "toon_rename: select ED %s: %s", table, mysql_error(h));
		++warnings;
		return false;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return true;
	}
	struct Row {
		std::string id;
		std::string name;
	};
	std::vector<Row> rows;
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0] && row[1]) {
			rows.push_back(Row{row[0], row[1]});
		}
	}
	mysql_free_result(res);

	unsigned long updated = 0;
	for(Row& r : rows) {
		if(!replace_ed_tokens_in_string(r.name, old_name, new_name)) {
			continue;
		}
		std::string upd = "UPDATE `";
		upd += table;
		upd += "` SET `";
		upd += name_col;
		upd += "` = ";
		upd += sql_quote(h, r.name);
		upd += " WHERE `";
		upd += id_col;
		upd += "` = ";
		upd += r.id;
		if(!mysql_exec(h, upd, "ED update")) {
			++warnings;
			return false;
		}
		++updated;
	}
	if(updated > 0) {
		summary += " ";
		summary += table;
		summary += ".obj_name=";
		summary += std::to_string(updated);
	}
	return true;
}

bool ensure_history_table(odb::database* db) {
	if(!db) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(
			"CREATE TABLE IF NOT EXISTS `toon_name_history` ("
			"  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
			"  `toon_id` BIGINT UNSIGNED NOT NULL,"
			"  `old_name` VARCHAR(32) NOT NULL,"
			"  `new_name` VARCHAR(32) NOT NULL,"
			"  `renamed_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
			"  `renamed_by` VARCHAR(32) NULL,"
			"  PRIMARY KEY (`id`),"
			"  KEY `idx_toon_name_history_toon_id` (`toon_id`),"
			"  KEY `idx_toon_name_history_old_name` (`old_name`)"
			") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
		t.commit();
		return true;
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "toon_rename: CREATE toon_name_history: %s", e.what());
		return false;
	}
}

struct FileMove {
	fs::path from;
	fs::path to;
	const char* label;
};

void consider_file(std::vector<FileMove>& moves, const fs::path& from, const fs::path& to,
				   const char* label, std::string& err) {
	std::error_code ec;
	if(!fs::exists(from, ec) || ec || !fs::is_regular_file(from, ec)) {
		return;
	}
	if(fs::exists(to, ec) && !ec) {
		err = "Esiste gia' il file di destinazione ";
		err += to.string();
		err += " (";
		err += label;
		err += "). Rifiuto per sicurezza.";
		return;
	}
	moves.push_back(FileMove{from, to, label});
}

bool toon_is_online(const char* name) {
	if(!name || !*name) {
		return false;
	}
	for(struct char_data* i = character_list; i; i = i->next) {
		if(!IS_NPC(i) && GET_NAME(i) && names_equal_ci(GET_NAME(i), name)) {
			return true;
		}
	}
	for(struct descriptor_data* d = descriptor_list; d; d = d->next) {
		if(d->character && GET_NAME(d->character) && names_equal_ci(GET_NAME(d->character), name)) {
			return true;
		}
		if(!d->AccountData.choosen.empty() && names_equal_ci(d->AccountData.choosen.c_str(), name)) {
			return true;
		}
	}
	return false;
}

void update_online_vassals(const char* old_name, const char* new_name) {
	if(!old_name || !new_name) {
		return;
	}
	for(struct char_data* i = character_list; i; i = i->next) {
		if(IS_NPC(i) || !GET_PRINCE(i) || !names_equal_ci(GET_PRINCE(i), old_name)) {
			continue;
		}
		char* oldp = i->player.vassallodi;
		i->player.vassallodi = strdup(new_name);
		if(oldp) {
			free(oldp);
		}
	}
}

void update_vassal_aux_files(const char* old_name, const char* new_name, int& files_ok, int& files_warn) {
	std::error_code ec;
	if(!fs::is_directory(RENT_DIR, ec)) {
		return;
	}
	for(const fs::directory_entry& entry : fs::directory_iterator(RENT_DIR, ec)) {
		if(ec || !entry.is_regular_file(ec)) {
			continue;
		}
		const std::string fname = entry.path().filename().string();
		if(fname.size() < 5 || fname.compare(fname.size() - 4, 4, ".aux") != 0) {
			continue;
		}
		const std::string stem = fname.substr(0, fname.size() - 4);
		if(stem.empty() || stem.front() == '.') {
			continue;
		}
		LegacyCharAux aux;
		if(!legacy_load_char_aux(stem.c_str(), aux)) {
			continue;
		}
		bool changed = false;
		for(LegacyAuxEntry& e : aux.entries) {
			if(strcasecmp(e.tag.c_str(), "principe") == 0 && names_equal_ci(e.value.c_str(), old_name)) {
				e.value = new_name;
				changed = true;
			}
		}
		if(!changed) {
			continue;
		}
		FILE* fp = std::fopen(entry.path().c_str(), "w");
		if(!fp) {
			mudlog(LOG_SYSERR, "toon_rename: cannot write vassal aux %s", entry.path().c_str());
			++files_warn;
			continue;
		}
		for(const LegacyAuxEntry& e : aux.entries) {
			std::fprintf(fp, "%s:%s\n", e.tag.c_str(), e.value.c_str());
		}
		std::fclose(fp);
		++files_ok;
	}
}

std::vector<std::string> history_names_for(MYSQL* h, const std::string& needle_lower) {
	std::vector<std::string> out;
	if(!h || needle_lower.empty() || !table_exists_tx(h, "toon_name_history")) {
		return out;
	}
	const std::string q = sql_quote(h, needle_lower);
	std::string sql =
		"SELECT DISTINCT LOWER(h.old_name) FROM toon_name_history h "
		"WHERE h.toon_id IN ("
		"  SELECT DISTINCT toon_id FROM toon_name_history "
		"  WHERE LOWER(old_name) = " + q + " OR LOWER(new_name) = " + q +
		") OR h.toon_id IN (SELECT id FROM toon WHERE LOWER(name) = " + q + ")";
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "toon_rename: history lookup: %s", mysql_error(h));
		return out;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return out;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0] && row[0][0]) {
			const std::string low = ascii_lower(row[0]);
			if(low != needle_lower) {
				out.push_back(low);
			}
		}
	}
	mysql_free_result(res);
	return out;
}

} /* anonymous */

std::vector<std::string> toon_previous_names_lower(const std::string& current_name) {
	std::vector<std::string> out;
	DB* db = Sql::getMysql();
	if(!db || current_name.empty()) {
		return out;
	}
	const std::string needle = ascii_lower(current_name);
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		MYSQL* h = current_tx_handle();
		out = history_names_for(h, needle);
		t.commit();
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "toon_previous_names_lower: %s", e.what());
	}
	return out;
}

ACTION_FUNC(do_rename) {
	(void)cmd;
	if(!ch) {
		return;
	}
	char old_raw[100];
	char new_raw[100];
	argument_interpreter(arg, old_raw, new_raw);
	if(!*old_raw || !*new_raw) {
		send_to_char("Sintassi: rename <nome-vecchio> <nome-nuovo>\n\r", ch);
		send_to_char("Il personaggio deve essere offline. Aggiorna DB, file, mail e storia nomi.\n\r", ch);
		return;
	}

	char old_parsed[100];
	char new_parsed[100];
	if(parse_name(old_raw, old_parsed) != 0ULL) {
		send_to_char("Nome vecchio non valido.\n\r", ch);
		return;
	}
	if(parse_name(new_raw, new_parsed) != 0ULL) {
		send_to_char("Nome nuovo non valido. Solo lettere, massimo 15 caratteri, niente spazi.\n\r", ch);
		return;
	}
	if(names_equal_ci(old_parsed, new_parsed)) {
		send_to_char("Il nome nuovo e' identico al vecchio.\n\r", ch);
		return;
	}
	if(_check_ass_name(new_parsed)) {
		send_to_char("Nome nuovo non ammesso (troppo corto o presente nella lista dei nomi vietati).\n\r", ch);
		return;
	}

	const std::string new_official = canonicalize_pc_name(new_parsed);
	const std::string old_lower = ascii_lower(old_parsed);

	if(toon_is_online(old_parsed)) {
		send_to_char("Il personaggio e' in gioco (anche linkdead o in login). Deve essere completamente offline.\n\r", ch);
		return;
	}
	if(toon_is_online(new_parsed)) {
		send_to_char("C'e' gia' qualcuno collegato con il nome nuovo. Rifiuto.\n\r", ch);
		return;
	}

	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("Database non disponibile.\n\r", ch);
		return;
	}

#if USE_MYSQL
	if(toon_nuke_is_blocked(db, 0, new_official.c_str())) {
		send_to_char("Il nome nuovo e' in blacklist (personaggio nukato) e non puo' essere usato.\n\r", ch);
		return;
	}
#endif

	if(!ensure_history_table(db)) {
		send_to_char("Non riesco a preparare la tabella toon_name_history.\n\r", ch);
		return;
	}

	/* Controllo file prima della transazione. */
	std::vector<FileMove> moves;
	std::string file_err;
	const std::string new_lower = ascii_lower(new_official);
	consider_file(moves, fs::path(PLAYERS_DIR) / (old_lower + ".dat"),
				  fs::path(PLAYERS_DIR) / (new_lower + ".dat"), "players .dat", file_err);
	consider_file(moves, fs::path(PLAYERS_DIR) / (old_lower + ".dead"),
				  fs::path(PLAYERS_DIR) / (new_lower + ".dead"), "players .dead", file_err);
	consider_file(moves, fs::path(PLAYERS_DIR) / (old_lower + ".deaths"),
				  fs::path(PLAYERS_DIR) / (new_lower + ".deaths"), "players .deaths", file_err);
	consider_file(moves, fs::path(RENT_DIR) / old_lower, fs::path(RENT_DIR) / new_lower, "rent",
				  file_err);
	consider_file(moves, fs::path(RENT_DIR) / (old_lower + ".aux"),
				  fs::path(RENT_DIR) / (new_lower + ".aux"), "rent .aux", file_err);
	consider_file(moves, fs::path("security") / old_lower, fs::path("security") / new_lower,
				  "security", file_err);
	if(!file_err.empty()) {
		send_to_char((file_err + "\n\r").c_str(), ch);
		return;
	}

	unsigned long long toon_id = 0;
	std::string old_official;
	std::string summary;
	int warnings = 0;
	const char* by = GET_NAME(ch) ? GET_NAME(ch) : "unknown";

	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		MYSQL* h = current_tx_handle();
		if(!h) {
			t.rollback();
			send_to_char("Connessione MySQL non disponibile.\n\r", ch);
			return;
		}

		const std::string old_q = sql_quote(h, old_lower);
			const unsigned long long nfound = query_ull(
			h, "SELECT COUNT(*) FROM toon WHERE LOWER(name) = " + old_q);
		if(nfound != 1) {
			t.rollback();
			if(nfound == 0) {
				send_to_char("Personaggio non trovato nel database.\n\r", ch);
			}
			else {
				send_to_char("Trovati piu' personaggi con quel nome (collation). Rifiuto.\n\r", ch);
			}
			return;
		}
		if(!query_string(h, "SELECT name FROM toon WHERE LOWER(name) = " + old_q, old_official)) {
			t.rollback();
			send_to_char("Non riesco a leggere il nome attuale dal database.\n\r", ch);
			return;
		}
		toon_id = query_ull(h, "SELECT id FROM toon WHERE LOWER(name) = " + old_q);
		if(!toon_id) {
			t.rollback();
			send_to_char("toon_id assente, rifiuto.\n\r", ch);
			return;
		}

		const std::string new_q = sql_quote(h, new_official);
		const unsigned long long taken =
			query_ull(h, "SELECT COUNT(*) FROM toon WHERE LOWER(name) = " + sql_quote(h, new_lower) +
							 " AND id <> " + std::to_string(toon_id));
		if(taken) {
			t.rollback();
			send_to_char("Esiste gia' un personaggio con il nome nuovo.\n\r", ch);
			return;
		}
		if(table_exists_tx(h, "registered") &&
		   query_ull(h, "SELECT COUNT(*) FROM registered WHERE LOWER(name) = " +
							sql_quote(h, new_lower))) {
			t.rollback();
			send_to_char("Il nome nuovo e' gia' presente nella tabella registered.\n\r", ch);
			return;
		}
		if(table_exists_tx(h, "legacy") &&
		   query_ull(h, "SELECT COUNT(*) FROM legacy WHERE LOWER(name) = " + sql_quote(h, new_lower))) {
			t.rollback();
			send_to_char("Il nome nuovo e' gia' presente nella tabella legacy.\n\r", ch);
			return;
		}

		std::string ins =
			"INSERT INTO toon_name_history (toon_id, old_name, new_name, renamed_by) VALUES (";
		ins += std::to_string(toon_id);
		ins += ",";
		ins += sql_quote(h, old_official);
		ins += ",";
		ins += new_q;
		ins += ",";
		ins += sql_quote(h, by);
		ins += ")";
		if(!mysql_exec(h, ins, "insert history")) {
			t.rollback();
			send_to_char("Errore inserendo la riga di storico.\n\r", ch);
			return;
		}

		if(!mysql_exec(h,
					   "UPDATE toon SET name = " + new_q + " WHERE id = " + std::to_string(toon_id),
					   "update toon.name")) {
			t.rollback();
			send_to_char("Errore aggiornando toon.name.\n\r", ch);
			return;
		}

		update_name_column(h, "object_instance", "owner_name", old_q, new_q, warnings, summary);
		update_name_column(h, "object_instance", "created_by_name", old_q, new_q, warnings, summary);
		update_name_column(h, "object_instance", "updated_by_name", old_q, new_q, warnings, summary);
		update_name_column(h, "object_instance_event", "actor_name", old_q, new_q, warnings, summary);
		update_name_column(h, "character_item_loss", "toon_name", old_q, new_q, warnings, summary);
		update_name_column(h, "clan_symbol", "prince_name", old_q, new_q, warnings, summary);
		update_name_column(h, "toon_nuke_blacklist", "toon_name", old_q, new_q, warnings, summary);
		update_name_column(h, "registered", "name", old_q, new_q, warnings, summary);
		update_name_column(h, "legacy", "name", old_q, new_q, warnings, summary);
		update_name_column(h, "edit_pool_event", "toon_name", old_q, new_q, warnings, summary);
		update_name_column(h, "edit_pool_event", "actor_name", old_q, new_q, warnings, summary);

		if(!update_ed_column(h, "object_instance", "id", "obj_name", old_official.c_str(),
							 new_official.c_str(), warnings, summary)) {
			t.rollback();
			send_to_char("Errore aggiornando i token ED in object_instance.\n\r", ch);
			return;
		}
		if(!update_ed_column(h, "character_inventory", "id", "obj_name", old_official.c_str(),
							 new_official.c_str(), warnings, summary)) {
			t.rollback();
			send_to_char("Errore aggiornando i token ED in character_inventory.\n\r", ch);
			return;
		}
		if(!update_ed_column(h, "character_inventory_overflow", "id", "obj_name",
							 old_official.c_str(), new_official.c_str(), warnings, summary)) {
			t.rollback();
			send_to_char("Errore aggiornando i token ED in character_inventory_overflow.\n\r", ch);
			return;
		}

		t.commit();
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "toon_rename: DB: %s", e.what());
		send_to_char("Errore database durante il rename: transazione annullata, file non toccati.\n\r",
					 ch);
		return;
	}

	int files_ok = 0;
	int files_warn = 0;
	for(const FileMove& m : moves) {
		std::error_code ec;
		fs::rename(m.from, m.to, ec);
		if(ec) {
			mudlog(LOG_SYSERR, "toon_rename: rename %s -> %s failed: %s", m.from.c_str(),
				   m.to.c_str(), ec.message().c_str());
			++files_warn;
			continue;
		}
		++files_ok;
		if(std::strcmp(m.label, "rent") == 0) {
			toon_fix_rent_file(m.to.c_str(), old_official.c_str(), new_official.c_str());
		}
		else if(std::strcmp(m.label, "players .dat") == 0) {
			toon_fix_player_dat_file(m.to.c_str(), old_official.c_str(), new_official.c_str());
		}
	}

	update_vassal_aux_files(old_official.c_str(), new_official.c_str(), files_ok, files_warn);
	update_online_vassals(old_official.c_str(), new_official.c_str());

	int mail_n = 0;
	if(!no_mail) {
		mail_n = mail_rename_player(old_official.c_str(), new_official.c_str());
		if(mail_n < 0) {
			++files_warn;
			mail_n = 0;
		}
	}

	char buf[512];
	std::snprintf(buf, sizeof(buf), "Rename completato: %s -> %s (toon_id=%llu).%s\n\r",
				  old_official.c_str(), new_official.c_str(),
				  static_cast<unsigned long long>(toon_id), summary.c_str());
	send_to_char(buf, ch);
	std::snprintf(buf, sizeof(buf), "File rinominati: %d. Posta aggiornata: %d blocchi. Avvisi: %d.\n\r",
				  files_ok, mail_n, files_warn + warnings);
	send_to_char(buf, ch);
	if(files_warn) {
		send_to_char("Alcuni file non sono stati rinominati: vedi il mudlog. Il DB e' gia' aggiornato.\n\r",
					 ch);
	}

	mudlog(LOG_PLAYERS, "RENAME: %s ha rinominato %s in %s (toon_id=%llu)%s files=%d mail=%d warn=%d",
		   by, old_official.c_str(), new_official.c_str(),
		   static_cast<unsigned long long>(toon_id), summary.c_str(), files_ok, mail_n,
		   files_warn + warnings);
}

#else /* !USE_MYSQL */

std::vector<std::string> toon_previous_names_lower(const std::string& current_name) {
	(void)current_name;
	return {};
}

ACTION_FUNC(do_rename) {
	(void)cmd;
	(void)arg;
	if(ch) {
		send_to_char("Rename richiede MySQL.\n\r", ch);
	}
}

#endif /* USE_MYSQL */

} /* namespace Alarmud */
