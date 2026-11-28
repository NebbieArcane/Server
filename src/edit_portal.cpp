/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "edit_portal.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <deque>
#include <fstream>
#include <future>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../contrib/slacking/json.hpp"
#include "autoenums.hpp"
#include "clan_symbol.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "comm.hpp"
#include "edit_pool.hpp"
#include "edit_system_config.hpp"
#include "edit_portal_listino.hpp"
#include "handler.hpp"
#include "legacy_import.hpp"
#include "logging.hpp"
#include "multiclass.hpp"
#include "obj_edit_catalog.hpp"
#include "obj_value.hpp"
#include "object_instance.hpp"
#include "parser.hpp"
#include "procarea.hpp"
#include "reception.hpp"
#include "Sql.hpp"
#include "structs.hpp"
#include "utils.hpp"

#if USE_MYSQL
#include <mysql/mysql.h>
#include <odb/mysql/connection.hxx>
#include "odb/account-odb.hxx"
#endif

namespace Alarmud {
namespace {

using Json = nlohmann::json;

struct EditPortalJob {
	std::string path;
	std::string body;
	std::promise<std::string> response;
};

std::mutex g_queue_mutex;
std::condition_variable g_queue_cv;
std::deque<EditPortalJob> g_jobs;
/* true solo se socket/bind/listen sono riusciti e il thread HTTP e' partito. */
std::atomic<bool> g_portal_running {false};
std::atomic<bool> g_worker_started {false};
/* Letto una volta in edit_portal_init (thread di gioco), poi solo in lettura. */
std::string g_api_secret;

/* Snapshot PG online: lo aggiorna solo il game loop (pulse). Il worker legge
 * solo queste due insiemi, sotto mutex, mai character_list / descriptor_list. */
std::mutex g_online_mu;
std::unordered_set<unsigned long long> g_online_ids;
std::unordered_set<std::string> g_online_names; /* lowercase */
/* Cache nome -> id (riempita dal worker quando ha entrambi). Il pulse non fa SQL. */
std::unordered_map<std::string, unsigned long long> g_online_name_to_id;

[[nodiscard]] std::string online_name_key(const std::string& name) {
	std::string out;
	out.reserve(name.size());
	for(unsigned char c : name) {
		out.push_back(static_cast<char>(std::tolower(c)));
	}
	return out;
}

/** Segreto di sviluppo storico: rifiutato, non deve mai aprire l'API. */
constexpr const char kEditPortalDevSecret[] = "nebbie-edit-dev-secret";
constexpr int kEditPortalDefaultPort = 8090;
constexpr const char kEditPortalDefaultBind[] = "127.0.0.1";

[[nodiscard]] std::string trim_http_value(std::string value) {
	while(!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
		value.erase(value.begin());
	}
	while(!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
		value.pop_back();
	}
	return value;
}

/** EDIT_API_SECRET dall'ambiente; vuoto = API spenta. Nessun default nel codice. */
[[nodiscard]] std::string api_secret_from_env() {
	const char* s = std::getenv("EDIT_API_SECRET");
	if(!s || !*s) {
		return {};
	}
	return trim_http_value(std::string(s));
}

/** EDIT_API_PORT (default 8090). -1 se il valore non e' una porta valida. */
[[nodiscard]] int api_port() {
	const char* p = std::getenv("EDIT_API_PORT");
	if(!p || !*p) {
		return kEditPortalDefaultPort;
	}
	char* end = nullptr;
	const long v = std::strtol(p, &end, 10);
	if(end == p || (end && *end != '\0') || v < 1 || v > 65535) {
		return -1;
	}
	return static_cast<int>(v);
}

/** EDIT_API_BIND (default 127.0.0.1). */
[[nodiscard]] std::string api_bind_from_env() {
	const char* b = std::getenv("EDIT_API_BIND");
	if(!b || !*b) {
		return kEditPortalDefaultBind;
	}
	return trim_http_value(std::string(b));
}

/** Confronto a tempo costante (non esce al primo byte diverso). */
[[nodiscard]] bool secret_equals(const std::string& given, const std::string& expected) {
	if(expected.empty()) {
		return false;
	}
	const std::size_t n = std::max(given.size(), expected.size());
	unsigned char diff = static_cast<unsigned char>(given.size() != expected.size());
	for(std::size_t i = 0; i < n; ++i) {
		const unsigned char a =
			i < given.size() ? static_cast<unsigned char>(given[i]) : 0U;
		const unsigned char b =
			i < expected.size() ? static_cast<unsigned char>(expected[i]) : 0U;
		diff = static_cast<unsigned char>(diff | (a ^ b));
	}
	return diff == 0U;
}

[[nodiscard]] bool header_has_secret(const std::string& headers) {
	std::istringstream stream(headers);
	std::string line;
	bool ok = false;
	while(std::getline(stream, line)) {
		if(!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		const auto colon = line.find(':');
		if(colon == std::string::npos) {
			continue;
		}
		std::string name = line.substr(0, colon);
		for(char& c : name) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		if(name != "x-edit-api-secret") {
			continue;
		}
		/* Nessun return anticipato: header duplicati valutati tutti. */
		if(secret_equals(trim_http_value(line.substr(colon + 1)), g_api_secret)) {
			ok = true;
		}
	}
	return ok;
}

[[nodiscard]] size_t header_content_length(const std::string& headers) {
	std::istringstream stream(headers);
	std::string line;
	while(std::getline(stream, line)) {
		if(!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		const auto colon = line.find(':');
		if(colon == std::string::npos) {
			continue;
		}
		std::string name = line.substr(0, colon);
		for(char& c : name) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		if(name != "content-length") {
			continue;
		}
		return static_cast<size_t>(
			std::strtoul(trim_http_value(line.substr(colon + 1)).c_str(), nullptr, 10));
	}
	return 0;
}

[[nodiscard]] std::string read_http_request(int client) {
	std::string raw;
	char buf[4096];
	size_t need = 0;
	while(raw.size() < 65536) {
		if(need > 0 && raw.size() >= need) {
			break;
		}
		const ssize_t n = read(client, buf, sizeof(buf));
		if(n <= 0) {
			break;
		}
		raw.append(buf, static_cast<size_t>(n));
		const auto hdr_end = raw.find("\r\n\r\n");
		if(hdr_end == std::string::npos) {
			continue;
		}
		const size_t cl = header_content_length(raw.substr(0, hdr_end));
		need = hdr_end + 4 + cl;
	}
	return raw;
}

[[nodiscard]] std::string normalize_api_path(std::string path) {
	if(!path.empty() && path.front() != '/') {
		path.insert(path.begin(), '/');
	}
	const auto q = path.find('?');
	if(q != std::string::npos) {
		path = path.substr(0, q);
	}
	while(path.size() > 1 && path.back() == '/') {
		path.pop_back();
	}
	return path;
}

[[nodiscard]] std::string json_error(const char* msg, int code = 400) {
	Json j;
	j["ok"] = false;
	j["error"] = msg ? msg : "errore";
	j["code"] = code;
	j[kEditPortalApiVersionTag] = kEditPortalApiVersion;
	return j.dump();
}

[[nodiscard]] std::string json_ok(const Json& data) {
	Json j;
	j["ok"] = true;
	j["data"] = data;
	j[kEditPortalApiVersionTag] = kEditPortalApiVersion;
	return j.dump();
}

[[nodiscard]] unsigned long long parse_json_ull(const Json& req, const char* key) {
	if(!key || !*key || req.find(key) == req.end() || req[key].is_null()) {
		return 0ULL;
	}
	const Json& v = req[key];
	if(v.is_number_unsigned()) {
		return v.get<unsigned long long>();
	}
	if(v.is_number_integer()) {
		return static_cast<unsigned long long>(v.get<long long>());
	}
	if(v.is_string()) {
		try {
			const std::string id_text = v.get<std::string>();
			if(id_text.empty()) {
				return 0ULL;
			}
			std::size_t parsed = 0;
			const unsigned long long id = std::stoull(id_text, &parsed);
			if(parsed != id_text.size()) {
				return 0ULL;
			}
			return id;
		}
		catch(...) {
			return 0ULL;
		}
	}
	return 0ULL;
}

[[nodiscard]] int parse_json_int(const Json& v, int default_val = 0) {
	if(v.is_number_integer()) {
		return static_cast<int>(v.get<long long>());
	}
	if(v.is_number_unsigned()) {
		return static_cast<int>(v.get<unsigned long long>());
	}
	if(v.is_string()) {
		try {
			const std::string t = v.get<std::string>();
			if(t.empty()) {
				return default_val;
			}
			std::size_t parsed = 0;
			const int n = std::stoi(t, &parsed);
			if(parsed != t.size()) {
				return default_val;
			}
			return n;
		}
		catch(...) {
			return default_val;
		}
	}
	return default_val;
}

[[nodiscard]] int parse_json_int(const Json& req, const char* key, int default_val = 0) {
	if(!key || !*key || req.find(key) == req.end()) {
		return default_val;
	}
	return parse_json_int(req[key], default_val);
}

[[nodiscard]] unsigned long long parse_json_toon_id(const Json& req) {
	return parse_json_ull(req, "toon_id");
}

/**
 * Aggiorna lo snapshot dei PG in gioco. Solo dal thread di gioco (pulse o boot).
 * character_list (anche linkdead ancora in memoria) + descriptor CON_PLYNG.
 * Nessuna query SQL: gli id arrivano dalla cache nome->id.
 */
void refresh_online_snapshot() {
	std::unordered_set<std::string> names;
	auto add_pc = [&](struct char_data* c) {
		if(!c || IS_NPC(c) || !GET_NAME(c) || !*GET_NAME(c)) {
			return;
		}
		names.insert(online_name_key(GET_NAME(c)));
	};
	for(struct char_data* c = character_list; c; c = c->next) {
		add_pc(c);
	}
	for(struct descriptor_data* d = descriptor_list; d; d = d->next) {
		if(d->connected == CON_PLYNG) {
			add_pc(d->character);
			if(d->original) {
				add_pc(d->original); /* switch: anche il corpo originale */
			}
		}
	}

	std::unordered_set<unsigned long long> ids;
	{
		std::lock_guard<std::mutex> lock(g_online_mu);
		for(const auto& n : names) {
			const auto it = g_online_name_to_id.find(n);
			if(it != g_online_name_to_id.end() && it->second != 0ULL) {
				ids.insert(it->second);
			}
		}
		g_online_ids.swap(ids);
		g_online_names.swap(names);
	}
}

/** Solo snapshot (mutex). Mai liste live del mondo. */
[[nodiscard]] bool is_toon_online(unsigned long long toon_id, const std::string& name) {
	const std::string key = name.empty() ? std::string() : online_name_key(name);
	std::lock_guard<std::mutex> lock(g_online_mu);
	if(toon_id != 0ULL && g_online_ids.count(toon_id) != 0) {
		if(!key.empty()) {
			g_online_name_to_id[key] = toon_id;
		}
		return true;
	}
	if(!key.empty() && g_online_names.count(key) != 0) {
		if(toon_id != 0ULL) {
			g_online_name_to_id[key] = toon_id;
			g_online_ids.insert(toon_id);
		}
		return true;
	}
	return false;
}

#if USE_MYSQL
/** Escape for SQL string literals (portal raw mysql path). */
[[nodiscard]] std::string portal_sql_escape(const char* s) {
	if(!s) {
		return {};
	}
	std::string out;
	out.reserve(std::strlen(s) * 2 + 4);
	for(const char* p = s; *p; ++p) {
		if(*p == '\'' || *p == '\\') {
			out.push_back('\\');
		}
		out.push_back(*p);
	}
	return out;
}

[[nodiscard]] std::string portal_sql_literal(const char* s) {
	return "'" + portal_sql_escape(s ? s : "") + "'";
}

/**
 * Execute SQL via libmysql (no odb::transaction required).
 * Prefer this over db->execute / ODB persist on the mud main thread: ODB object
 * ops and database::execute need a live transaction TLS slot that has proven
 * unreliable for portal jobs (not_in_transaction even after begin()).
 */
[[nodiscard]] bool portal_mysql_exec(DB* db, const std::string& sql, std::string& err,
									 const char* where = "portal_mysql_exec") {
	if(!db) {
		err = std::string("[") + where + "] database non disponibile";
		return false;
	}
	/* TLS ODB sporco (has_current ma finalized) → connection() sulla tx esplode
	 * con "operation can only be performed in transaction". Reset e usa il pool. */
	if(odb::transaction::has_current() && odb::transaction::current().finalized()) {
		odb::transaction::reset_current();
	}
	try {
		/* database::connection(): se c'e' tx attiva riusa quella, altrimenti pool. */
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql.c_str()) != 0) {
			err = std::string("[") + where + "] mysql: " +
				  (mysql_error(h) ? mysql_error(h) : "mysql_query failed");
			return false;
		}
		return true;
	}
	catch(const std::exception& e) {
		const bool had_tx = odb::transaction::has_current();
		err = std::string("[") + where + "] odb/conn has_current=" +
			  (had_tx ? "1" : "0") + ": " + e.what();
		mudlog(LOG_SYSERR, "edit_portal: %s", err.c_str());
		return false;
	}
}

/** INSERT via libmysql; returns AUTO_INCREMENT id on the same connection. */
[[nodiscard]] bool portal_mysql_insert(DB* db, const std::string& sql,
									   unsigned long long& out_id, std::string& err,
									   const char* where = "portal_mysql_insert") {
	out_id = 0;
	if(!db) {
		err = std::string("[") + where + "] database non disponibile";
		return false;
	}
	if(odb::transaction::has_current() && odb::transaction::current().finalized()) {
		odb::transaction::reset_current();
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql.c_str()) != 0) {
			err = std::string("[") + where + "] mysql: " +
				  (mysql_error(h) ? mysql_error(h) : "mysql_query failed");
			return false;
		}
		out_id = static_cast<unsigned long long>(mysql_insert_id(h));
		if(out_id == 0) {
			err = std::string("[") + where + "] mysql_insert_id=0 dopo INSERT";
			return false;
		}
		return true;
	}
	catch(const std::exception& e) {
		const bool had_tx = odb::transaction::has_current();
		err = std::string("[") + where + "] odb/conn has_current=" +
			  (had_tx ? "1" : "0") + ": " + e.what();
		mudlog(LOG_SYSERR, "edit_portal: %s", err.c_str());
		return false;
	}
}

constexpr const char* kSqlCreateEditPortalSession =
	"CREATE TABLE IF NOT EXISTS `edit_portal_session` ("
	"`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
	"`session_key` VARCHAR(64) NOT NULL,"
	"`opened_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
	"`closed_at` TIMESTAMP NULL DEFAULT NULL,"
	"`expires_at` TIMESTAMP NULL DEFAULT NULL,"
	"`actor_account_id` BIGINT UNSIGNED NULL,"
	"`actor_email` VARCHAR(255) NULL,"
	"`actor_toon_id` BIGINT UNSIGNED NULL,"
	"`actor_toon_name` VARCHAR(32) NULL,"
	"`actor_role` VARCHAR(16) NULL,"
	"`actor_max_level` INT NULL,"
	"`client_ip` VARCHAR(64) NULL,"
	"`auth_via` VARCHAR(32) NULL,"
	"`status` VARCHAR(16) NOT NULL DEFAULT 'open',"
	"`summary_json` MEDIUMTEXT NULL,"
	"PRIMARY KEY (`id`),"
	"UNIQUE KEY `uq_edit_portal_session_key` (`session_key`),"
	"KEY `idx_edit_portal_session_actor` (`actor_toon_id`),"
	"KEY `idx_edit_portal_session_opened` (`opened_at`),"
	"KEY `idx_edit_portal_session_status_exp` (`status`, `expires_at`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

constexpr const char* kSqlCreateEditPortalEvent =
	"CREATE TABLE IF NOT EXISTS `edit_portal_event` ("
	"`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
	"`at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
	"`session_id` BIGINT UNSIGNED NULL,"
	"`session_key` VARCHAR(64) NULL,"
	"`kind` VARCHAR(48) NOT NULL,"
	"`severity` VARCHAR(8) NOT NULL DEFAULT 'info',"
	"`actor_toon_id` BIGINT UNSIGNED NULL,"
	"`actor_name` VARCHAR(32) NULL,"
	"`target_toon_id` BIGINT UNSIGNED NULL,"
	"`target_name` VARCHAR(32) NULL,"
	"`inventory_id` BIGINT UNSIGNED NULL,"
	"`instance_id` BIGINT UNSIGNED NULL,"
	"`detail_json` MEDIUMTEXT NULL,"
	"`message` VARCHAR(512) NULL,"
	"`telegram_sent` TINYINT NOT NULL DEFAULT 0,"
	"PRIMARY KEY (`id`),"
	"KEY `idx_edit_portal_event_session_at` (`session_key`, `at`),"
	"KEY `idx_edit_portal_event_kind_at` (`kind`, `at`),"
	"KEY `idx_edit_portal_event_telegram` (`telegram_sent`, `severity`, `at`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

constexpr const char* kSqlCreateEditPortalJob =
	"CREATE TABLE IF NOT EXISTS `edit_portal_job` ("
	"`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
	"`session_key` VARCHAR(64) NOT NULL,"
	"`toon_id` BIGINT UNSIGNED NOT NULL,"
	"`kind` VARCHAR(48) NOT NULL,"
	"`status` VARCHAR(16) NOT NULL DEFAULT 'pending',"
	"`payload_json` MEDIUMTEXT NULL,"
	"`result_json` MEDIUMTEXT NULL,"
	"`error_message` VARCHAR(512) NULL,"
	"`created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
	"`started_at` TIMESTAMP NULL DEFAULT NULL,"
	"`finished_at` TIMESTAMP NULL DEFAULT NULL,"
	"PRIMARY KEY (`id`),"
	"KEY `idx_edit_portal_job_status` (`status`, `id`),"
	"KEY `idx_edit_portal_job_toon_status` (`toon_id`, `status`),"
	"KEY `idx_edit_portal_job_session` (`session_key`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

constexpr const char* kSqlCreateEditPortalDraft =
	"CREATE TABLE IF NOT EXISTS `edit_portal_draft` ("
	"`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
	"`session_key` VARCHAR(64) NOT NULL,"
	"`toon_id` BIGINT UNSIGNED NOT NULL,"
	"`base_version` VARCHAR(64) NULL,"
	"`draft_json` MEDIUMTEXT NOT NULL,"
	"`updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
	"PRIMARY KEY (`id`),"
	"UNIQUE KEY `uq_edit_portal_draft_session` (`session_key`),"
	"KEY `idx_edit_portal_draft_toon` (`toon_id`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

void portal_audit_ensure_tables() {
	DB* db = Sql::getMysql();
	if(!db) {
		return;
	}
	std::string err;
	(void)portal_mysql_exec(db, kSqlCreateEditPortalSession, err, "portal:audit_session_ddl");
	(void)portal_mysql_exec(db, kSqlCreateEditPortalEvent, err, "portal:audit_event_ddl");
	(void)portal_mysql_exec(db, kSqlCreateEditPortalJob, err, "portal:job_ddl");
	(void)portal_mysql_exec(db, kSqlCreateEditPortalDraft, err, "portal:draft_ddl");
	(void)portal_mysql_exec(
		db,
		"ALTER TABLE `edit_portal_session` ADD COLUMN `expires_at` TIMESTAMP NULL DEFAULT NULL "
		"AFTER `closed_at`",
		err, "portal:session_expires_at");
}

[[nodiscard]] std::string portal_audit_trunc(std::string s, std::size_t max_len) {
	if(s.size() > max_len) {
		s.resize(max_len);
	}
	return s;
}

[[nodiscard]] std::string portal_sql_nullable_ull(unsigned long long v) {
	if(v == 0ULL) {
		return "NULL";
	}
	return std::to_string(v);
}

[[nodiscard]] std::string portal_sql_nullable_str(const char* s, std::size_t max_len) {
	if(!s || !*s) {
		return "NULL";
	}
	return portal_sql_literal(portal_audit_trunc(s, max_len).c_str());
}

/** File audit dedicato (una riga/azione). Default: /app/edit-portal-audit.log */
[[nodiscard]] const char* portal_audit_log_path() noexcept {
	const char* env = std::getenv("EDIT_PORTAL_AUDIT_LOG");
	if(env != nullptr && env[0] != '\0') {
		return env;
	}
	return "/app/edit-portal-audit.log";
}

void portal_audit_append_file(const std::string& kind, const std::string& sev,
							  const std::string& actor_name,
							  const std::string& target_name,
							  unsigned long long inventory_id,
							  unsigned long long instance_id,
							  const std::string& session_key, const std::string& msg,
							  const Json* detail) {
	static std::mutex audit_file_mu;
	std::lock_guard<std::mutex> lock(audit_file_mu);
	std::ofstream out(portal_audit_log_path(), std::ios::out | std::ios::app);
	if(!out) {
		return;
	}
	using clock = std::chrono::system_clock;
	const auto now = clock::now();
	const std::time_t tt = clock::to_time_t(now);
	std::tm tm {};
#if defined(_WIN32)
	gmtime_s(&tm, &tt);
#else
	gmtime_r(&tt, &tm);
#endif
	out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ") << ' ' << sev << ' ' << kind
		<< " actor=" << (actor_name.empty() ? "-" : actor_name)
		<< " target=" << (target_name.empty() ? "-" : target_name);
	if(inventory_id != 0ULL) {
		out << " inv=" << inventory_id;
	}
	if(instance_id != 0ULL) {
		out << " inst=" << instance_id;
	}
	if(!session_key.empty()) {
		out << " sess=" << session_key.substr(0, 12);
	}
	if(!msg.empty()) {
		out << " msg=" << msg;
	}
	if(detail && !detail->is_null()) {
		try {
			std::string d = detail->dump();
			if(d.size() > 400) {
				d.resize(400);
				d.append("...");
			}
			for(char& c : d) {
				if(c == '\n' || c == '\r') {
					c = ' ';
				}
			}
			out << " detail=" << d;
		}
		catch(...) {
		}
	}
	out << '\n';
}

/**
 * Scrive un evento in edit_portal_event + file audit. Best-effort.
 * session_key / actor_* arrivano dal body Node (opzionali).
 */
void portal_audit_event(const Json& req, const char* kind, const char* severity,
						const char* message, const Json* detail,
						unsigned long long target_toon_id, const char* target_name,
						unsigned long long inventory_id = 0,
						unsigned long long instance_id = 0) {
	if(!kind || !*kind) {
		return;
	}
	const std::string session_key =
		portal_audit_trunc(req.value("session_key", std::string()), 64);
	const unsigned long long actor_toon_id = parse_json_ull(req, "actor_toon_id");
	const std::string actor_name =
		portal_audit_trunc(req.value("actor_name", std::string()), 32);
	const std::string sev =
		portal_audit_trunc(severity && *severity ? severity : "info", 8);
	const std::string kind_s = portal_audit_trunc(kind, 48);
	const std::string msg =
		portal_audit_trunc(message && *message ? message : "", 512);
	const std::string target_s =
		portal_audit_trunc(target_name && *target_name ? target_name : "", 32);

	portal_audit_append_file(kind_s, sev, actor_name, target_s, inventory_id,
							 instance_id, session_key, msg, detail);

	DB* db = Sql::getMysql();
	if(!db) {
		return;
	}
	std::string detail_sql = "NULL";
	if(detail && !detail->is_null()) {
		try {
			detail_sql = portal_sql_literal(detail->dump().c_str());
		}
		catch(...) {
			detail_sql = "NULL";
		}
	}
	std::ostringstream sql;
	sql << "INSERT INTO edit_portal_event "
		   "(session_key, kind, severity, actor_toon_id, actor_name, "
		   "target_toon_id, target_name, inventory_id, instance_id, "
		   "detail_json, message, telegram_sent) VALUES ("
		<< (session_key.empty() ? "NULL" : portal_sql_literal(session_key.c_str()))
		<< ", " << portal_sql_literal(kind_s.c_str()) << ", "
		<< portal_sql_literal(sev.c_str()) << ", " << portal_sql_nullable_ull(actor_toon_id)
		<< ", " << portal_sql_nullable_str(actor_name.c_str(), 32) << ", "
		<< portal_sql_nullable_ull(target_toon_id) << ", "
		<< portal_sql_nullable_str(target_s.c_str(), 32) << ", "
		<< portal_sql_nullable_ull(inventory_id) << ", "
		<< portal_sql_nullable_ull(instance_id) << ", " << detail_sql << ", "
		<< (msg.empty() ? "NULL" : portal_sql_literal(msg.c_str())) << ", 0)";
	std::string err;
	if(!portal_mysql_exec(db, sql.str(), err, "portal:audit_event")) {
		mudlog(LOG_SYSERR, "edit_portal: audit_event fallito: %s", err.c_str());
	}
}

[[nodiscard]] const char* portal_audit_severity_for_code(int code) {
	if(code >= 500) {
		return "crit";
	}
	if(code == 402 || code == 409) {
		return "warn";
	}
	return "info";
}

[[nodiscard]] const char* portal_audit_kind_for_code(int code, const char* ok_kind) {
	if(code == 402) {
		return "pay.fail";
	}
	if(code >= 500) {
		return "apply.error";
	}
	(void)ok_kind;
	return "apply.fail";
}

[[nodiscard]] std::string portal_audit_fail(const Json& req, const char* msg, int code,
											unsigned long long target_toon_id,
											const char* target_name,
											unsigned long long inventory_id = 0,
											const Json* detail = nullptr) {
	portal_audit_event(req, portal_audit_kind_for_code(code, nullptr),
					   portal_audit_severity_for_code(code), msg, detail, target_toon_id,
					   target_name, inventory_id, 0);
	return json_error(msg, code);
}

[[nodiscard]] int max_level_for_toon(unsigned long long toon_id) {
	DB* db = Sql::getMysql();
	if(!db || toon_id == 0) {
		return 0;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT MAX(level) FROM character_classes WHERE toon_id = " +
			std::to_string(toon_id);
		if(mysql_query(h, sql.c_str()) != 0) {
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const int lvl = row && row[0] ? std::atoi(row[0]) : 0;
		mysql_free_result(res);
		return lvl;
	}
	catch(...) {
		return 0;
	}
}

/** Quante classi con level > 0 (allineato a HowManyClasses offline). */
[[nodiscard]] int class_count_for_toon(unsigned long long toon_id) {
	DB* db = Sql::getMysql();
	if(!db || toon_id == 0) {
		return 1;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT COUNT(*) FROM character_classes WHERE toon_id = " +
			std::to_string(toon_id) + " AND level > 0";
		if(mysql_query(h, sql.c_str()) != 0) {
			return 1;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 1;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const int n = row && row[0] ? std::atoi(row[0]) : 0;
		mysql_free_result(res);
		return n > 0 ? n : 1;
	}
	catch(...) {
		return 1;
	}
}

[[nodiscard]] std::string toon_name_by_id(unsigned long long toon_id) {
	if(toon_id == 0) {
		return {};
	}
	const toonPtr pg = Sql::getOne<toon>(toonQuery::id == toon_id);
	if(pg && pg->name[0] != '\0') {
		return pg->name;
	}
#if USE_MYSQL
	DB* db = Sql::getMysql();
	if(!db) {
		return {};
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT name FROM toon WHERE id = " + std::to_string(toon_id) + " LIMIT 1";
		if(mysql_query(h, sql.c_str()) != 0) {
			return {};
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return {};
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const std::string name = row && row[0] ? row[0] : std::string();
		mysql_free_result(res);
		return name;
	}
	catch(...) {
		return {};
	}
#else
	(void)toon_id;
#endif
	return {};
}

[[nodiscard]] bool load_stats_for_toon(unsigned long long toon_id, int& exp,
									   int& rune, int* gold = nullptr,
									   int* bank_gold = nullptr) {
	DB* db = Sql::getMysql();
	if(!db || toon_id == 0) {
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT exp, p_rune_dei, gold, bank_gold FROM character_stats WHERE toon_id = " +
			std::to_string(toon_id) + " LIMIT 1";
		if(mysql_query(h, sql.c_str()) != 0) {
			return false;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return false;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		if(!row) {
			mysql_free_result(res);
			return false;
		}
		exp = row[0] ? std::atoi(row[0]) : 0;
		rune = row[1] ? std::atoi(row[1]) : 0;
		if(gold) {
			*gold = row[2] ? std::atoi(row[2]) : 0;
		}
		if(bank_gold) {
			*bank_gold = row[3] ? std::atoi(row[3]) : 0;
		}
		mysql_free_result(res);
		return true;
	}
	catch(...) {
		return false;
	}
}

[[nodiscard]] bool deduct_payment(unsigned long long toon_id, int xp_cost,
								  int rune_cost, int max_level, std::string& err) {
	int exp = 0;
	int rune = 0;
	if(!load_stats_for_toon(toon_id, exp, rune)) {
		err = "[portal:deduct] character_stats non trovato";
		return false;
	}
	const long long prince_reserve =
		(max_level >= PRINCIPE) ? static_cast<long long>(PRINCEEXP) : 0LL;
	const long long available_xp = static_cast<long long>(exp) - prince_reserve;
	if(xp_cost > 0 && available_xp < static_cast<long long>(xp_cost)) {
		err = "[portal:deduct] XP insufficienti (riserva principi 400M inclusa)";
		return false;
	}
	if(rune_cost > 0 && rune < rune_cost) {
		err = "[portal:deduct] Rune degli Eroi insufficienti";
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		err = "[portal:deduct] database non disponibile";
		return false;
	}
	std::ostringstream sql;
	sql << "UPDATE character_stats SET exp = exp - " << xp_cost
		<< ", p_rune_dei = p_rune_dei - " << rune_cost << " WHERE toon_id = "
		<< toon_id;
	/* mysql_query: evita odb::execute che richiede transaction sul thread mud. */
	return portal_mysql_exec(db, sql.str(), err, "portal:deduct");
}

/** Owner per class_mult listino: personal_owner / ED, altrimenti toon del portale. */
void ensure_portal_listino_owner(struct obj_data* obj, const std::string& toon_name) {
	if(!obj || toon_name.empty() || obj->personal_owner[0] != '\0') {
		return;
	}
	strncpy(obj->personal_owner, toon_name.c_str(), sizeof(obj->personal_owner) - 1);
	obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';
}

/**
 * Rune listino (prezzo alternativo all'XP, come sul sito) per un quote oggetto
 * sulla location: stesso rapporto rune/xp della riga edit_listino, quindi
 * eredita artifact, class_mult e malus 2x gia' nel quote XP.
 * 0 = nessuna riga con rune (resta la conversione 1 MXP = 1 rune).
 */
[[nodiscard]] int object_listino_rune_for_quote(int location, long quote_xp,
											   unsigned long apply_bit = 0) {
	if(quote_xp <= 0 || !portal_listino_ready()) {
		return 0;
	}
	const double scale =
		portal_listino_param("storage_scale", static_cast<double>(kObjValueStorageScale));
	double xp_per_unit = 0.0;
	int rune_num = 0;
	int rune_den = 1;
	if((location == APPLY_IMMUNE || location == APPLY_M_IMMUNE ||
		location == APPLY_SUSC) &&
	   apply_bit != 0UL) {
		long raw_sum = 0;
		long r_num = 0;
		long r_den = 1;
		bool any = false;
		for(unsigned long b = 1UL; b != 0UL; b <<= 1) {
			if((apply_bit & b) == 0UL) {
				continue;
			}
			ObjEditListinoSpec spec {};
			int rn = 0;
			int rd = 1;
			if(!portal_listino_affect_by_bit(location, b, spec) ||
			   spec.positive_unit_raw <= 0 ||
			   !portal_listino_affect_rune_bit(location, b, rn, rd) || rn <= 0 ||
			   rd <= 0) {
				continue;
			}
			raw_sum += spec.positive_unit_raw;
			const long nn = r_num * static_cast<long>(rd) + static_cast<long>(rn) * r_den;
			const long dd = r_den * static_cast<long>(rd);
			long a = nn < 0 ? -nn : nn;
			long d = dd < 0 ? -dd : dd;
			while(d != 0) {
				const long t = a % d;
				a = d;
				d = t;
			}
			const long g = a > 0 ? a : 1;
			r_num = nn / g;
			r_den = dd / g;
			any = true;
		}
		if(!any || raw_sum <= 0 || r_num <= 0 || r_den <= 0) {
			return 0;
		}
		xp_per_unit = static_cast<double>(raw_sum) * scale;
		rune_num = static_cast<int>(r_num);
		rune_den = static_cast<int>(r_den);
	}
	else {
		ObjEditListinoSpec spec {};
		if(!portal_listino_affect_by_location(location, spec) ||
		   spec.positive_unit_raw <= 0) {
			return 0;
		}
		if(!portal_listino_affect_rune(location, rune_num, rune_den) || rune_num <= 0 ||
		   rune_den <= 0) {
			return 0;
		}
		xp_per_unit = static_cast<double>(spec.positive_unit_raw) * scale;
	}
	if(xp_per_unit <= 0.0) {
		return 0;
	}
	const double rune = static_cast<double>(quote_xp) * static_cast<double>(rune_num) /
						(static_cast<double>(rune_den) * xp_per_unit);
	return std::max(0, static_cast<int>(std::ceil(rune - 1e-9)));
}

/**
 * Listino oggetto. quote_pq sono rune "rent" aggiuntive obbligatorie.
 * La parte listino si paga:
 *  - se quote_rune_listino > 0 (riga edit_listino con rune): XP e rune sono
 *    prezzi paralleli come sul sito; ok se pay_xp/quote_xp +
 *    (pay_rune - pq)/quote_rune_listino >= 1 (slider % rune);
 *  - altrimenti XP e/o Rune con 1 MXP = 1 Rune (kObjEditRunePerMegaXp).
 * Non forzare mai max(pay_xp, quote_xp): spezzerebbe il pagamento solo-rune.
 */
[[nodiscard]] bool resolve_object_edit_payment(int pay_xp, int pay_rune, long quote_xp,
											  int quote_pq, int& xp_cost, int& rune_cost,
											  std::string& err, int quote_rune_listino = 0) {
	if(pay_xp < 0 || pay_rune < 0) {
		err = "[portal:pay] pagamento non valido";
		return false;
	}
	const int quote_xp_i = static_cast<int>(std::max(0L, quote_xp));
	const int pq = std::max(0, quote_pq);
	if(pay_rune < pq) {
		err = "[portal:pay] rune componente listino insufficienti";
		return false;
	}
	if(quote_rune_listino > 0 && quote_xp_i > 0) {
		const long double frac_xp =
			static_cast<long double>(pay_xp) / static_cast<long double>(quote_xp_i);
		const long double frac_rune = static_cast<long double>(pay_rune - pq) /
									  static_cast<long double>(quote_rune_listino);
		if(frac_xp + frac_rune < 1.0L - 1e-9L) {
			err = "[portal:pay] copertura XP/Rune insufficiente rispetto al listino";
			return false;
		}
		xp_cost = pay_xp;
		rune_cost = pay_rune;
		return true;
	}
	const long long rune_cover_xp =
		static_cast<long long>(pay_rune - pq) * kObjEditRunePerMegaXp;
	const long long covered =
		static_cast<long long>(pay_xp) + rune_cover_xp;
	if(covered < static_cast<long long>(quote_xp_i)) {
		err = "[portal:pay] copertura XP/Rune insufficiente rispetto al listino";
		return false;
	}
	xp_cost = pay_xp;
	rune_cost = pay_rune;
	return true;
}

/**
 * Pagamento pool PG come editpool_charge_or_refund: XP e rune sono listini
 * paralleli (pq = alternativa completa), non rune additive / conversione 1 MXP.
 * - 100% XP: pay_rune==0, pay_xp >= quote_xp → addebita (quote_xp*100)/100
 * - 100% rune: pay_xp==0, pay_rune >= quote_pq → addebita quote_pq
 * - misto: percentuali che sommano 100; addebita
 *   (quote_xp*pct_xp)/100 e (quote_pq*pct_rune)/100 (divisione intera).
 */
[[nodiscard]] bool resolve_editpool_payment(int pay_xp, int pay_rune, long quote_xp,
											int quote_pq, int& xp_cost, int& rune_cost,
											std::string& err) {
	if(pay_xp < 0 || pay_rune < 0) {
		err = "[portal:pay] pagamento non valido";
		return false;
	}
	const int quote_xp_i = static_cast<int>(std::max(0L, quote_xp));
	const int pq = std::max(0, quote_pq);
	if(quote_xp_i <= 0 && pq <= 0) {
		xp_cost = 0;
		rune_cost = 0;
		return true;
	}

	auto parts_for = [quote_xp_i, pq](int pct_xp, int pct_rune) {
		struct {
			int xp;
			int rune;
		} out {};
		out.xp = static_cast<int>(
			(static_cast<long long>(quote_xp_i) * static_cast<long long>(pct_xp)) /
			100LL);
		out.rune = (pq * pct_rune) / 100;
		return out;
	};

	/* Match esatto alle parti editpool (UI slider / ok xp|rune|misto). */
	for(int pct_rune = 0; pct_rune <= 100; ++pct_rune) {
		const int pct_xp = 100 - pct_rune;
		const auto parts = parts_for(pct_xp, pct_rune);
		if(pay_xp == parts.xp && pay_rune == parts.rune) {
			xp_cost = parts.xp;
			rune_cost = parts.rune;
			return true;
		}
	}

	/*
	 * Solo-rune / solo-XP con eventuale overpay rispetto al listino intero
	 * (come editpool ok rune / ok xp).
	 */
	if(pay_xp == 0 && pay_rune > 0) {
		if(pay_rune < pq) {
			err = "[portal:pay] rune insufficienti per il listino pool (editpool)";
			return false;
		}
		xp_cost = 0;
		rune_cost = pq;
		return true;
	}
	if(pay_rune == 0 && pay_xp >= quote_xp_i) {
		xp_cost = quote_xp_i;
		rune_cost = 0;
		return true;
	}

	err = "[portal:pay] copertura XP/Rune non allineata al listino pool (editpool)";
	return false;
}

[[nodiscard]] struct obj_data* materialize_inventory_row(
	const inventory_mysql_row& row) {
	if(row.elem.item_number <= 0) {
		return nullptr;
	}
	struct obj_data* obj =
		object_instance_load_stored(row.elem.item_number, row.instance_id);
	if(!obj) {
		return nullptr;
	}
	obj->db_inventory_id = row.id;

	/*
	 * Stessa regola di reception (c83c1d7): instance apply, poi overlay
	 * inventorio su values/flags/affect/testi. Simbolo di clan: solo instance.
	 * Cosi' quote/apply portal vedono cio' che il PG ha addosso in gioco.
	 */
	if(obj->db_instance_id != 0 && clan_symbol_is_obj(obj)) {
		return obj;
	}

	obj->obj_flags.value[0] = row.elem.value[0];
	obj->obj_flags.value[1] = row.elem.value[1];
	obj->obj_flags.value[2] = row.elem.value[2];
	obj->obj_flags.value[3] = row.elem.value[3];
	obj->obj_flags.extra_flags = row.elem.extra_flags;
	obj->obj_flags.extra_flags2 = row.elem.extra_flags2;
	obj->obj_flags.weight = row.elem.weight;
	obj->obj_flags.timer = row.elem.timer;
	obj->obj_flags.bitvector = row.elem.bitvector;
	if(obj->name) {
		free(obj->name);
	}
	if(obj->short_description) {
		free(obj->short_description);
	}
	if(obj->description) {
		free(obj->description);
	}
	obj->name = strdup(row.elem.name);
	obj->short_description = strdup(row.elem.sd);
	obj->description = strdup(row.elem.desc);
	hydrate_personal_owner_from_ed(obj, true);
	/* Inventorio e' verita' possesso: copia sempre gli affect della riga
	 * (anche se tutti zero), come in reception. */
	for(int j = 0; j < MAX_OBJ_AFFECT; ++j) {
		obj->affected[j] = row.elem.affected[j];
	}
	return obj;
}

[[nodiscard]] inventory_mysql_row* find_inventory_row(
	std::vector<inventory_mysql_row>& rows, unsigned long long inventory_id) {
	for(auto& r : rows) {
		if(r.id == inventory_id) {
			return &r;
		}
	}
	return nullptr;
}

/**
 * Tetto dam/sp listino (personaggio):
 * pezzi in possesso (inv o indossati) con ITEM2_EDIT e owner del toon
 * → max(0, dam_attuale − dam_prototipo). Simboli di clan esclusi.
 */
[[nodiscard]] int sum_owned_edited_damroll_excluding(
	std::vector<inventory_mysql_row>& rows, unsigned long long exclude_inventory_id,
	const char* toon_name) {
	int total = 0;
	for(auto& r : rows) {
		if(r.id == exclude_inventory_id) {
			continue;
		}
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name)) {
			total += object_edit_damroll_edited_delta(o);
		}
		extract_obj(o);
	}
	return total;
}

[[nodiscard]] int sum_owned_edited_spellpower_excluding(
	std::vector<inventory_mysql_row>& rows, unsigned long long exclude_inventory_id,
	const char* toon_name) {
	int total = 0;
	for(auto& r : rows) {
		if(r.id == exclude_inventory_id) {
			continue;
		}
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name)) {
			total += object_edit_spellpower_edited_delta(o);
		}
		extract_obj(o);
	}
	return total;
}

[[nodiscard]] int sum_owned_edited_hitroll_excluding(
	std::vector<inventory_mysql_row>& rows, unsigned long long exclude_inventory_id,
	const char* toon_name) {
	int total = 0;
	for(auto& r : rows) {
		if(r.id == exclude_inventory_id) {
			continue;
		}
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name)) {
			total += object_edit_hitroll_edited_delta(o);
		}
		extract_obj(o);
	}
	return total;
}

[[nodiscard]] int sum_owned_edited_spellfail_excluding(
	std::vector<inventory_mysql_row>& rows, unsigned long long exclude_inventory_id,
	const char* toon_name) {
	int total = 0;
	for(auto& r : rows) {
		if(r.id == exclude_inventory_id) {
			continue;
		}
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name)) {
			total += object_edit_spellfail_edited_delta(o);
		}
		extract_obj(o);
	}
	return total;
}

/** Elenco pezzi EDIT+owner che contribuiscono al tetto dam (UI). */
[[nodiscard]] Json owned_dam_budget_contributors_json(
	std::vector<inventory_mysql_row>& rows, const char* toon_name) {
	Json list = Json::array();
	for(auto& r : rows) {
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name)) {
			const int cur = object_edit_damroll_total(o);
			const int proto = object_edit_damroll_prototype_total(o);
			const int delta = object_edit_damroll_edited_delta(o);
			if(delta > 0) {
				Json c;
				c["inventory_id"] = r.id;
				c["short_desc"] = r.elem.sd;
				c["current"] = cur;
				c["proto"] = proto;
				c["delta"] = delta;
				c["worn"] = inventory_row_is_worn(r.elem.wearpos);
				c["proto_vnum"] = object_edit_prototype_vnum(o);
				list.push_back(c);
			}
		}
		extract_obj(o);
	}
	return list;
}

[[nodiscard]] bool persist_inventory_affects(unsigned long long inventory_id,
											 const struct obj_data* obj,
											 std::string& err) {
	DB* db = Sql::getMysql();
	if(!db || !obj) {
		err = "[portal:persist_inv] database/obj non disponibile";
		return false;
	}
	const int base_vnum = object_instance_resolve_base_vnum(obj);
	const std::string name_lit =
		portal_sql_literal(obj->name ? obj->name : "");
	const std::string sd_lit =
		portal_sql_literal(obj->short_description ? obj->short_description : "");
	const std::string desc_lit =
		portal_sql_literal(obj->description ? obj->description : "");
	std::ostringstream upd;
	upd << "UPDATE character_inventory SET extra_flags="
		<< static_cast<int>(obj->obj_flags.extra_flags) << ", extra_flags2="
		<< static_cast<int>(obj->obj_flags.extra_flags2) << ", obj_name=" << name_lit
		<< ", short_desc=" << sd_lit << ", description=" << desc_lit;
	if(obj->db_instance_id != 0) {
		upd << ", instance_id=" << obj->db_instance_id;
		if(base_vnum > 0) {
			upd << ", item_number=" << base_vnum;
		}
	}
	upd << " WHERE id = " << inventory_id;
	if(!portal_mysql_exec(db, upd.str(), err, "portal:persist_inv_upd")) {
		return false;
	}
	if(!portal_mysql_exec(db,
						  "DELETE FROM character_inventory_affect WHERE inventory_id = " +
							  std::to_string(inventory_id),
						  err, "portal:persist_inv_del_aff")) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		if(loc == APPLY_NONE || loc == APPLY_SKIP || mod == 0) {
			continue;
		}
		std::ostringstream ins;
		ins << "INSERT INTO character_inventory_affect (inventory_id, affect_slot, "
			   "location, modifier) VALUES ("
			<< inventory_id << ',' << i << ',' << loc << ',' << mod << ')';
		if(!portal_mysql_exec(db, ins.str(), err, "portal:persist_inv_ins_aff")) {
			return false;
		}
	}
	return true;
}

/**
 * Persist edited object_instance (+ affects + event) via raw mysql_query.
 * Creates a new row when obj->db_instance_id == 0 (first portal edit), otherwise
 * UPDATEs. Avoids ODB persist/update which require transaction TLS on mud thread.
 */
[[nodiscard]] bool portal_persist_object_instance_mysql(struct obj_data* obj,
														int base_vnum,
														const char* owner_name,
														std::string& err) {
	DB* db = Sql::getMysql();
	if(!db || !obj || base_vnum <= 0) {
		err = "[portal:persist_inst] parametri non validi";
		return false;
	}

	int shell_weight = obj->obj_flags.weight;
	if(GET_ITEM_TYPE(obj) == ITEM_CONTAINER) {
		shell_weight -= contained_weight(obj);
		if(shell_weight < 0) {
			shell_weight = 0;
		}
	}
	const std::string stripped_name = object_instance_strip_ed_tokens(obj->name);
	const char* name_src =
		!stripped_name.empty() ? stripped_name.c_str()
							   : (obj->name ? obj->name : "");
	const std::string name_lit = portal_sql_literal(name_src);
	const std::string sd_lit =
		portal_sql_literal(obj->short_description ? obj->short_description : "");
	const std::string desc_lit =
		portal_sql_literal(obj->description ? obj->description : "");

	int char_vnum = obj->char_vnum;
	if(char_vnum <= 0 ||
	   (char_vnum >= LOW_EDITED_ITEMS && char_vnum <= HIGH_EDITED_ITEMS)) {
		char_vnum = base_vnum;
	}
	obj->char_vnum = char_vnum;

	std::string owner;
	if(owner_name && *owner_name) {
		owner = owner_name;
	}
	else if(obj->personal_owner[0] != '\0') {
		owner = obj->personal_owner;
	}
	const std::string owner_sql =
		owner.empty() ? "NULL" : portal_sql_literal(owner.c_str());

	const bool is_create = (obj->db_instance_id == 0);
	unsigned long long id = obj->db_instance_id;

	if(is_create) {
		std::ostringstream ins;
		ins << "INSERT INTO object_instance ("
			   "base_vnum, char_vnum, type_flag, wear_flags, extra_flags, "
			   "extra_flags2, weight, cost, cost_per_day, timer, bitvector, "
			   "value0, value1, value2, value3, obj_name, short_desc, description, "
			   "owner_name, source, created_by_name, updated_by_name, deleted, "
			   "created_at, updated_at) VALUES ("
			<< base_vnum << ',' << char_vnum << ','
			<< static_cast<int>(obj->obj_flags.type_flag) << ','
			<< static_cast<int>(obj->obj_flags.wear_flags) << ','
			<< static_cast<int>(obj->obj_flags.extra_flags) << ','
			<< static_cast<int>(obj->obj_flags.extra_flags2) << ','
			<< shell_weight << ',' << obj->obj_flags.cost << ','
			<< obj->obj_flags.cost_per_day << ',' << obj->obj_flags.timer << ','
			<< obj->obj_flags.bitvector << ',' << obj->obj_flags.value[0] << ','
			<< obj->obj_flags.value[1] << ',' << obj->obj_flags.value[2] << ','
			<< obj->obj_flags.value[3] << ',' << name_lit << ',' << sd_lit << ','
			<< desc_lit << ',' << owner_sql << ", '" << kObjInstSourceGodEdit
			<< "', 'edit_portal', 'edit_portal', 0, NOW(), NOW())";
		if(!portal_mysql_insert(db, ins.str(), id, err, "portal:persist_inst_ins")) {
			return false;
		}
		obj->db_instance_id = id;
		if(!owner.empty()) {
			strncpy(obj->personal_owner, owner.c_str(),
					sizeof(obj->personal_owner) - 1);
			obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';
		}
	}
	else {
		std::ostringstream upd;
		upd << "UPDATE object_instance SET base_vnum=" << base_vnum
			<< ", char_vnum=" << char_vnum
			<< ", type_flag=" << static_cast<int>(obj->obj_flags.type_flag)
			<< ", wear_flags=" << static_cast<int>(obj->obj_flags.wear_flags)
			<< ", extra_flags=" << static_cast<int>(obj->obj_flags.extra_flags)
			<< ", extra_flags2=" << static_cast<int>(obj->obj_flags.extra_flags2)
			<< ", weight=" << shell_weight << ", cost=" << obj->obj_flags.cost
			<< ", cost_per_day=" << obj->obj_flags.cost_per_day
			<< ", timer=" << obj->obj_flags.timer
			<< ", bitvector=" << obj->obj_flags.bitvector
			<< ", value0=" << obj->obj_flags.value[0]
			<< ", value1=" << obj->obj_flags.value[1]
			<< ", value2=" << obj->obj_flags.value[2]
			<< ", value3=" << obj->obj_flags.value[3]
			<< ", obj_name=" << name_lit << ", short_desc=" << sd_lit
			<< ", description=" << desc_lit;
		if(!owner.empty()) {
			upd << ", owner_name=" << owner_sql;
		}
		upd << ", updated_by_name='edit_portal', updated_at=NOW()"
			<< " WHERE id=" << id << " AND deleted=0";
		if(!portal_mysql_exec(db, upd.str(), err, "portal:persist_inst_upd")) {
			return false;
		}
	}

	if(!portal_mysql_exec(db,
						  "DELETE FROM object_instance_affect WHERE key_instance_id=" +
							  std::to_string(id),
						  err, "portal:persist_inst_del_aff")) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		/* Non salvare APPLY_X By 0 (occupa slot senza effetto; vedi DAMROLL By 0). */
		if(loc == APPLY_NONE || loc == APPLY_SKIP || mod == 0) {
			continue;
		}
		std::ostringstream aff;
		aff << "INSERT INTO object_instance_affect (key_instance_id, key_affect_slot, "
			   "location, modifier) VALUES ("
			<< id << ',' << i << ',' << loc << ',' << mod << ')';
		if(!portal_mysql_exec(db, aff.str(), err, "portal:persist_inst_ins_aff")) {
			return false;
		}
	}
	{
		std::ostringstream ev;
		ev << "INSERT INTO object_instance_event (instance_id, at, actor_name, kind, "
			  "note) VALUES ("
		   << id << ", NOW(), 'edit_portal', '" << (is_create ? "create" : "update")
		   << "', "
		   << portal_sql_literal(("base=" + std::to_string(base_vnum)).c_str()) << ')';
		if(!portal_mysql_exec(db, ev.str(), err, "portal:persist_inst_event")) {
			return false;
		}
	}
	return true;
}

/**
 * Come do_personalize / SetPersonOnSave: owner sul toon target + flag PERSONAL.
 * Le keyword restano senza ED* (owner in personal_owner / object_instance.owner_name).
 */
void portal_apply_personalize(struct obj_data* obj, const char* owner_name) {
	if(!obj || !owner_name || !*owner_name) {
		return;
	}
	SET_BIT(obj->obj_flags.extra_flags2, ITEM2_PERSONAL);
	strncpy(obj->personal_owner, owner_name, sizeof(obj->personal_owner) - 1);
	obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';
	if(obj->name) {
		const std::string stripped = object_instance_strip_ed_tokens(obj->name);
		if(!stripped.empty() && stripped != obj->name) {
			free(obj->name);
			obj->name = strdup(stripped.c_str());
		}
	}
}

/**
 * Save apply-affect result: create/update object_instance, then refresh
 * character_inventory (+ affects + instance_id link).
 *
 * Premi procarea al primo create: come EditAffectBroker, scrive create-baseline
 * con gli affect pre-edit (reward_create_baseline) poi lo stato post-edit.
 * Se reward_create_baseline e' null, la baseline coincide con after (es. solo
 * flag artifact senza cambio affect).
 */
[[nodiscard]] bool portal_save_edited_inventory_item(
	unsigned long long inventory_id, struct obj_data* after, std::string& err,
	const char* owner_name,
	const obj_affected_type* reward_create_baseline = nullptr,
	bool mark_edit = true) {
	if(!after) {
		err = "[portal:save] obj null";
		return false;
	}
	const int base = object_instance_resolve_base_vnum(after);
	if(base <= 0) {
		err = "[portal:save] base_vnum non risolvibile (item_number/char_vnum)";
		return false;
	}
	if(after->char_vnum == 0 ||
	   (after->char_vnum >= LOW_EDITED_ITEMS &&
		after->char_vnum <= HIGH_EDITED_ITEMS)) {
		after->char_vnum = base;
	}
	/* Solo flag Artifact: niente ITEM2_EDIT (non conta nei tetti dam/sp). */
	if(mark_edit && !IS_OBJ_STAT2(after, ITEM2_EDIT)) {
		SET_BIT(after->obj_flags.extra_flags2, ITEM2_EDIT);
	}
	/* Stesso effetto di «personalize <obj> <toon>»: PERSONAL + owner del target. */
	if(owner_name && *owner_name) {
		portal_apply_personalize(after, owner_name);
	}

	const bool reward_first_create =
		procarea_obj_is_reward(after) && after->db_instance_id == 0;
	if(reward_first_create) {
		SET_BIT(after->obj_flags.extra_flags2, ITEM2_PROCAREA_REWARD);
		obj_affected_type post_affects[MAX_OBJ_AFFECT];
		std::memcpy(post_affects, after->affected, sizeof(post_affects));
		if(reward_create_baseline) {
			std::memcpy(after->affected, reward_create_baseline, sizeof(post_affects));
		}
		const unsigned long long id = object_instance_persist(
			after, base, 0, nullptr, true, "edit_portal");
		if(id == 0) {
			std::memcpy(after->affected, post_affects, sizeof(post_affects));
			err = "[portal:save] create-baseline premio fallita";
			return false;
		}
		after->db_instance_id = id;
		std::memcpy(after->affected, post_affects, sizeof(post_affects));
		if(object_instance_persist(after, base, id, nullptr, true, "edit_portal") ==
		   0) {
			err = "[portal:save] update post-baseline premio fallito";
			return false;
		}
	}
	else if(!portal_persist_object_instance_mysql(after, base, owner_name, err)) {
		return false;
	}
	if(!persist_inventory_affects(inventory_id, after, err)) {
		return false;
	}
	return true;
}

[[nodiscard]] ObjEditAnalysis analyze_portal_object(struct obj_data* obj) {
	if(!obj) {
		return {};
	}
	if(procarea_obj_is_reward(obj)) {
		struct obj_data* baseline = object_edit_load_baseline(obj);
		if(!baseline) {
			return {};
		}
		ObjEditAnalysis report = AnalyzeObjEditAgainst(obj, baseline, true);
		extract_obj(baseline);
		return report;
	}
	return AnalyzeObjEdit(obj);
}

constexpr long long kLabExpCap = 2000000000LL;
constexpr int kLabGiveQtyMax = 20;

[[nodiscard]] bool lab_actor_allowed(const Json& req, std::string& err) {
	const unsigned long long actor_toon_id = parse_json_ull(req, "actor_toon_id");
	if(actor_toon_id == 0ULL) {
		err = "actor_toon_id richiesto per Lab";
		return false;
	}
	const int lvl = max_level_for_toon(actor_toon_id);
	if(lvl < kEditPortalLabLevel) {
		err = "Lab riservato a staff livello 58+";
		return false;
	}
	return true;
}

[[nodiscard]] bool lab_require_offline_target(unsigned long long target_toon_id,
											 std::string& target_name,
											 std::string& err) {
	target_name = toon_name_by_id(target_toon_id);
	if(target_name.empty()) {
		err = "toon target non trovato";
		return false;
	}
	if(is_toon_online(target_toon_id, target_name)) {
		err = "il toon target e collegato al mud";
		return false;
	}
	return true;
}

[[nodiscard]] int lab_next_list_index(DB* db, unsigned long long toon_id) {
	if(!db || toon_id == 0ULL) {
		return -1;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT IFNULL(MAX(list_index) + 1, 0) FROM character_inventory "
			"WHERE toon_id = " +
			std::to_string(toon_id) + " AND (deleted = 0 OR deleted IS NULL)";
		if(mysql_query(h, sql.c_str()) != 0) {
			return -1;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return -1;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const int next = row && row[0] ? std::atoi(row[0]) : 0;
		mysql_free_result(res);
		return next;
	}
	catch(...) {
		return -1;
	}
}

/**
 * Inserisce un oggetto gia' materializzato nell'inventario offline del toon
 * (wear_pos=0 = non indossato). Restituisce inventory id.
 */
[[nodiscard]] bool lab_insert_inventory_obj(unsigned long long toon_id,
											struct obj_data* obj,
											unsigned long long& out_inventory_id,
											std::string& err) {
	out_inventory_id = 0;
	DB* db = Sql::getMysql();
	if(!db || !obj || toon_id == 0ULL) {
		err = "[lab:inv] parametri non validi";
		return false;
	}
	const int list_index = lab_next_list_index(db, toon_id);
	if(list_index < 0) {
		err = "[lab:inv] list_index non calcolabile";
		return false;
	}
	if(list_index >= MAX_OBJ_SAVE) {
		err = "[lab:inv] inventario pieno (MAX_OBJ_SAVE)";
		return false;
	}
	int item_number = object_instance_resolve_base_vnum(obj);
	if(item_number <= 0) {
		item_number = obj->char_vnum > 0 ? obj->char_vnum : 0;
	}
	if(item_number <= 0) {
		err = "[lab:inv] vnum oggetto non valido";
		return false;
	}
	const std::string name_lit =
		portal_sql_literal(obj->name ? obj->name : "");
	const std::string sd_lit =
		portal_sql_literal(obj->short_description ? obj->short_description : "");
	const std::string desc_lit =
		portal_sql_literal(obj->description ? obj->description : "");
	std::ostringstream ins;
	ins << "INSERT INTO character_inventory ("
		   "toon_id, list_index, item_number, value0, value1, value2, value3, "
		   "extra_flags, extra_flags2, weight, timer, bitvector, "
		   "obj_name, short_desc, description, wear_pos, depth, instance_id, "
		   "deleted) VALUES ("
		<< toon_id << ',' << list_index << ',' << item_number << ','
		<< obj->obj_flags.value[0] << ',' << obj->obj_flags.value[1] << ','
		<< obj->obj_flags.value[2] << ',' << obj->obj_flags.value[3] << ','
		<< static_cast<int>(obj->obj_flags.extra_flags) << ','
		<< static_cast<int>(obj->obj_flags.extra_flags2) << ','
		<< obj->obj_flags.weight << ',' << obj->obj_flags.timer << ','
		<< obj->obj_flags.bitvector << ',' << name_lit << ',' << sd_lit << ','
		<< desc_lit << ", 0, 0, "
		<< (obj->db_instance_id != 0 ? std::to_string(obj->db_instance_id) : "0")
		<< ", 0)";
	if(!portal_mysql_insert(db, ins.str(), out_inventory_id, err, "lab:inv_ins")) {
		return false;
	}
	if(!persist_inventory_affects(out_inventory_id, obj, err)) {
		return false;
	}
	return true;
}

[[nodiscard]] Json analyze_to_json(const struct obj_data* obj) {
	const ObjEditAnalysis a =
		analyze_portal_object(const_cast<struct obj_data*>(obj));
	Json j;
	j["has_edit"] = a.has_edit;
	j["owner_name"] = a.owner_name;
	j["owner_classes"] = a.owner_classes;
	j["class_mult"] = a.class_mult;
	j["diff_xp_mega"] = static_cast<long long>(a.diff.valore / 1000000L);
	j["diff_xp_frac"] = static_cast<long long>(
		(a.diff.valore - static_cast<long long>(j["diff_xp_mega"].get<long long>()) *
							 1000000L) /
		10000L);
	j["diff_xp_raw"] = a.diff.valore;
	j["diff_rune"] = a.diff.rune;
	j["diff_derent_mega"] = static_cast<long long>(a.diff.derent / 1000000L);
	j["changes"] = a.changes;
	j["artifact"] = IS_OBJ_STAT(obj, ITEM_IMMUNE) ? 1 : 0;
	j["paid_malus"] = IS_OBJ_STAT2(obj, ITEM2_PAID_MALUS) ? 1 : 0;
	return j;
}

[[nodiscard]] bool portal_text_field_ok(const std::string& s, int max_len,
										const char* label, std::string& err) {
	if(static_cast<int>(s.size()) > max_len) {
		err = std::string(label) + " troppo lungo (" + std::to_string(s.size()) +
			  "/" + std::to_string(max_len) + ")";
		return false;
	}
	return true;
}

void portal_set_obj_string(char*& field, const std::string& value) {
	if(field) {
		free(field);
		field = nullptr;
	}
	field = strdup(value.c_str());
}

/**
 * Opzionale: name/short/long dal body JSON.
 * Solo insieme a un affect pagato (stesso apply): non si salvano da soli dopo.
 */
[[nodiscard]] bool portal_apply_optional_text_fields(
	struct obj_data* obj, const Json& req, bool affect_is_paid, std::string& err,
	bool& text_changed_out) {
	text_changed_out = false;
	if(!obj) {
		err = "oggetto null";
		return false;
	}
	const bool has_name = req.find("obj_name") != req.end() || req.find("name") != req.end();
	const bool has_short =
		req.find("short_desc") != req.end() || req.find("short") != req.end();
	const bool has_long =
		req.find("description") != req.end() || req.find("long_desc") != req.end();
	if(!has_name && !has_short && !has_long) {
		return true;
	}
	const std::string new_name =
		has_name ? std::string(req.value("obj_name", req.value("name", "")))
				 : std::string(obj->name ? obj->name : "");
	const std::string new_short =
		has_short
			? std::string(req.value("short_desc", req.value("short", "")))
			: std::string(obj->short_description ? obj->short_description : "");
	const std::string new_long =
		has_long
			? std::string(req.value("description", req.value("long_desc", "")))
			: std::string(obj->description ? obj->description : "");

	if(!portal_text_field_ok(new_name, kObjEditTextNameMax, "name", err) ||
	   !portal_text_field_ok(new_short, kObjEditTextShortMax, "short", err) ||
	   !portal_text_field_ok(new_long, kObjEditTextLongMax, "long", err)) {
		return false;
	}

	const std::string cur_name = obj->name ? obj->name : "";
	const std::string cur_short =
		obj->short_description ? obj->short_description : "";
	const std::string cur_long = obj->description ? obj->description : "";
	const bool changed =
		(new_name != cur_name) || (new_short != cur_short) || (new_long != cur_long);
	if(!changed) {
		return true;
	}
	if(!affect_is_paid) {
		err = "name/short/long solo insieme al pagamento di un nuovo affect "
			  "(non con Artifact gratis ne' da soli)";
		return false;
	}
	portal_set_obj_string(obj->name, new_name);
	portal_set_obj_string(obj->short_description, new_short);
	portal_set_obj_string(obj->description, new_long);
	text_changed_out = true;
	return true;
}

inline constexpr long kEditPortalPqPerMegaXp = kEditPoolPqPerMegaXp;

[[nodiscard]] Json quote_from_pool(const portal_pool_quote& quote) {
	Json q;
	q["mxp"] = quote.mxp;
	q["mxp_frac"] = quote.mxp_frac;
	q["xp_raw"] = quote.xp_raw;
	q["pq"] = quote.pq;
	return q;
}

[[nodiscard]] Json quote_xp_json(long xp_raw, int pq) {
	portal_pool_quote q;
	q.xp_raw = xp_raw;
	q.pq = pq;
	q.mxp = xp_raw / 1000000L;
	q.mxp_frac = (xp_raw % 1000000L) / 10000L;
	return quote_from_pool(q);
}

[[nodiscard]] sh_int pool_field_current(const char_edit_pool_data& pool,
										EditPoolField field) noexcept {
	switch(field) {
	case EditPoolField::Hp:
		return pool.edit_hp;
	case EditPoolField::Mana:
		return pool.edit_mana;
	case EditPoolField::Move:
		return pool.edit_move;
	case EditPoolField::HpRegen:
		return pool.edit_hp_regen;
	case EditPoolField::ManaRegen:
		return pool.edit_mana_regen;
	case EditPoolField::MoveRegen:
		return pool.edit_move_regen;
	case EditPoolField::Spellfail:
		return 0; /* spellfail PG: character_stats.edit_spellfail, non gestito dal portale */
	}
	return 0;
}

[[nodiscard]] long resistance_bit_xp_raw(unsigned damage_type) noexcept {
	/* MXP listino × 100 raw (× storage → mega): https://www.nebbiearcane.it/listino-edits/ */
	switch(damage_type) {
	case IMM_FIRE:
		return 10000L; /* 100 MXP */
	case IMM_COLD:
	case IMM_ACID:
	case IMM_HOLD:
		return 7500L; /* 75 MXP */
	case IMM_ELEC:
	case IMM_ENERGY:
	case IMM_SLASH:
	case IMM_PIERCE:
		return 15000L; /* 150 MXP */
	case IMM_BLUNT:
		return 30000L; /* 300 MXP */
	case IMM_DRAIN:
	case IMM_POISON:
		return 3000L; /* 30 MXP */
	default:
		return 5000L;
	}
}

[[nodiscard]] Json quote_xp_json(long xp_raw) {
	return quote_xp_json(xp_raw, 0);
}

[[nodiscard]] bool load_edit_pool_for_toon(unsigned long long toon_id,
										   char_edit_pool_data& pool) {
	DB* db = Sql::getMysql();
	if(!db || toon_id == 0) {
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const std::string sql =
			"SELECT edit_hp, edit_mana, edit_move, edit_hp_regen, edit_mana_regen, "
			"edit_move_regen, overedit_hp, overedit_mana, overedit_move, "
			"overedit_hp_regen, overedit_mana_regen, overedit_move_regen, "
			"edit_pool_migrated "
			"FROM character_stats WHERE toon_id = " +
			std::to_string(toon_id) + " LIMIT 1";
		if(mysql_query(h, sql.c_str()) != 0) {
			return false;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return false;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		if(!row) {
			mysql_free_result(res);
			return false;
		}
		pool.edit_hp = static_cast<sh_int>(row[0] ? std::atoi(row[0]) : 0);
		pool.edit_mana = static_cast<sh_int>(row[1] ? std::atoi(row[1]) : 0);
		pool.edit_move = static_cast<sh_int>(row[2] ? std::atoi(row[2]) : 0);
		pool.edit_hp_regen = static_cast<sh_int>(row[3] ? std::atoi(row[3]) : 0);
		pool.edit_mana_regen = static_cast<sh_int>(row[4] ? std::atoi(row[4]) : 0);
		pool.edit_move_regen = static_cast<sh_int>(row[5] ? std::atoi(row[5]) : 0);
		pool.overedit_hp = static_cast<sh_int>(row[6] ? std::atoi(row[6]) : 0);
		pool.overedit_mana = static_cast<sh_int>(row[7] ? std::atoi(row[7]) : 0);
		pool.overedit_move = static_cast<sh_int>(row[8] ? std::atoi(row[8]) : 0);
		pool.overedit_hp_regen = static_cast<sh_int>(row[9] ? std::atoi(row[9]) : 0);
		pool.overedit_mana_regen =
			static_cast<sh_int>(row[10] ? std::atoi(row[10]) : 0);
		pool.overedit_move_regen =
			static_cast<sh_int>(row[11] ? std::atoi(row[11]) : 0);
		pool.migrated = static_cast<ubyte>(row[12] ? std::atoi(row[12]) : 0);
		mysql_free_result(res);
		return true;
	}
	catch(...) {
		return false;
	}
}

[[nodiscard]] Json pool_to_json(const char_edit_pool_data& pool) {
	Json j;
	j["hit"] = pool.edit_hp;
	j["mana"] = pool.edit_mana;
	j["move"] = pool.edit_move;
	j["hit_regen"] = pool.edit_hp_regen;
	j["mana_regen"] = pool.edit_mana_regen;
	j["move_regen"] = pool.edit_move_regen;
	j["over_hit"] = pool.overedit_hp;
	j["over_mana"] = pool.overedit_mana;
	j["over_move"] = pool.overedit_move;
	j["over_hit_regen"] = pool.overedit_hp_regen;
	j["over_mana_regen"] = pool.overedit_mana_regen;
	j["over_move_regen"] = pool.overedit_move_regen;
	j["migrated"] = pool.migrated ? 1 : 0;
	Json caps = Json::object();
	caps["hit"] = kEditPoolMaxHit;
	caps["mana"] = kEditPoolMaxMana;
	caps["move"] = kEditPoolMaxMove;
	caps["hit_regen"] = kEditPoolMaxHitRegen;
	caps["mana_regen"] = kEditPoolMaxManaRegen;
	caps["move_regen"] = kEditPoolMaxMoveRegen;
	j["caps"] = caps;
	return j;
}

[[nodiscard]] const char* immortal_grade_label(int max_level) noexcept {
	if(max_level >= IMMENSO) {
		return "Immenso";
	}
	if(max_level >= MAESTRO_DEI_CREATORI) {
		return "Maestro dei Creatori";
	}
	if(max_level >= MAESTRO_DEL_CREATO) {
		return "Maestro del Creato";
	}
	if(max_level >= QUESTMASTER) {
		return "Maestro del Fato";
	}
	if(max_level >= CREATORE) {
		return "Creatore";
	}
	if(max_level >= MAESTRO_DEGLI_DEI) {
		return "Maestro degli Dei";
	}
	if(max_level >= DIO) {
		return "Dio";
	}
	if(max_level >= DIO_MINORE) {
		return "Dio Minore";
	}
	if(max_level >= IMMORTALE) {
		return "Immortale";
	}
	if(max_level >= PRINCIPE) {
		return "Principe";
	}
	return "Mortale";
}

[[nodiscard]] Json commands_enabled_at_level(int max_level) {
	Json arr = Json::array();
	/* Solo comandi staff (min_level >= Immortale): non elenca i mortali. */
	if(max_level < IMMORTALE) {
		return arr;
	}
	struct CmdRow {
		std::string name;
		int min_level;
	};
	std::vector<CmdRow> rows;
	for(int i = 0; i < 27; ++i) {
		for(NODE* n = radix_head[i].next; n != nullptr; n = n->next) {
			if(!n->name || !*n->name) {
				continue;
			}
			const int ml = static_cast<int>(n->min_level);
			if(ml >= IMMORTALE && ml <= max_level) {
				rows.push_back({n->name, ml});
			}
		}
	}
	std::sort(rows.begin(), rows.end(), [](const CmdRow& a, const CmdRow& b) {
		if(a.min_level != b.min_level) {
			return a.min_level < b.min_level;
		}
		return a.name < b.name;
	});
	for(const auto& r : rows) {
		Json c;
		c["name"] = r.name;
		c["min_level"] = r.min_level;
		c["grade"] = immortal_grade_label(r.min_level);
		arr.push_back(c);
	}
	return arr;
}

[[nodiscard]] bool pool_data_has_values(const char_edit_pool_data& p) noexcept {
	return p.edit_hp != 0 || p.edit_mana != 0 || p.edit_move != 0 ||
		   p.edit_hp_regen != 0 || p.edit_mana_regen != 0 || p.edit_move_regen != 0;
}

[[nodiscard]] Json resist_status_json(bool present, bool from_proto, bool from_edit) {
	Json j;
	j["present"] = present;
	if(!present) {
		j["origin"] = nullptr;
		j["origin_label"] = "";
		return j;
	}
	if(from_proto && from_edit) {
		j["origin"] = "proto+edit";
		j["origin_label"] = "sul proto e anche aggiunta in edit";
	}
	else if(from_proto) {
		j["origin"] = "proto";
		j["origin_label"] = "già sul proto";
	}
	else {
		j["origin"] = "edit";
		j["origin_label"] = "aggiunta in edit";
	}
	return j;
}

struct PoolResidualPiece {
	unsigned long long inventory_id = 0;
	int item_number = 0;
	std::string short_desc;
	int hit = 0;
	int mana = 0;
	int move = 0;
	int hit_regen = 0;
	int mana_regen = 0;
	int move_regen = 0;
};

struct ToonInventoryEditScan {
	int dam_total = 0;
	int sp_total = 0;
	int hr_total = 0;
	int spellfail_total = 0;
	bool res_slash = false;
	bool res_pierce = false;
	bool res_blunt = false;
	bool slash_proto = false;
	bool slash_edit = false;
	bool pierce_proto = false;
	bool pierce_edit = false;
	bool blunt_proto = false;
	bool blunt_edit = false;
	bool has_clan_symbol = false;
	std::string clan_symbol_owner;
	char_edit_pool_data pool_from_eq {};
	int edited_pieces = 0;
	std::vector<PoolResidualPiece> pool_residuals;
};

[[nodiscard]] std::string object_edit_owner_name(const struct obj_data* obj) {
	if(!obj) {
		return {};
	}
	if(obj->personal_owner[0] != '\0') {
		return obj->personal_owner;
	}
	if(obj->name && obj->name[0] != '\0') {
		return object_instance_extract_ed_owner(obj->name);
	}
	return {};
}

/**
 * Analisi inventario completa: budget combat, resistenze (proto vs edit),
 * delta pool vs proto sugli oggetti EDIT del toon.
 */
[[nodiscard]] ToonInventoryEditScan scan_toon_inventory_edits(
	unsigned long long toon_id, const std::string& toon_name) {
	ToonInventoryEditScan s;
	if(toon_name.empty()) {
		return s;
	}
	std::vector<inventory_mysql_row> rows;
	(void)load_character_inventory_mysql(toon_id, rows);

	for(auto& r : rows) {
		struct obj_data* o = materialize_inventory_row(r);
		if(!o) {
			continue;
		}
		const bool is_clan =
			clan_symbol_is_obj(o) || o->obj_flags.type_flag == ITEM_CLAN_SYMBOL;
		if(is_clan) {
			s.has_clan_symbol = true;
			if(s.clan_symbol_owner.empty()) {
				s.clan_symbol_owner = object_edit_owner_name(o);
			}
		}
		if(object_edit_counts_toward_combat_budget(o, toon_name.c_str())) {
			++s.edited_pieces;
			s.dam_total += object_edit_damroll_edited_delta(o);
			s.sp_total += object_edit_spellpower_edited_delta(o);
			s.hr_total += object_edit_hitroll_edited_delta(o);
			s.spellfail_total += object_edit_spellfail_edited_delta(o);

			struct obj_data* proto = nullptr;
			const int pv = object_edit_prototype_vnum(o);
			if(pv > 0) {
				const int rn = real_object(pv);
				if(rn >= 0) {
					proto = read_object(rn, REAL);
				}
			}
			const int bits = object_immune_current_bits(o);
			const int proto_bits = proto ? object_immune_current_bits(proto) : 0;
			auto note_res = [&](int bit, bool& present, bool& from_p, bool& from_e) {
				if((bits & bit) == 0) {
					return;
				}
				present = true;
				if(proto_bits & bit) {
					from_p = true;
				}
				else {
					from_e = true;
				}
			};
			note_res(static_cast<int>(IMM_SLASH), s.res_slash, s.slash_proto,
					 s.slash_edit);
			note_res(static_cast<int>(IMM_PIERCE), s.res_pierce, s.pierce_proto,
					 s.pierce_edit);
			note_res(static_cast<int>(IMM_BLUNT), s.res_blunt, s.blunt_proto,
					 s.blunt_edit);

			const char_edit_pool_data before = s.pool_from_eq;
			edit_pool_accumulate_obj_delta(o, proto, &s.pool_from_eq);
			const int d_hit = s.pool_from_eq.edit_hp - before.edit_hp;
			const int d_mana = s.pool_from_eq.edit_mana - before.edit_mana;
			const int d_move = s.pool_from_eq.edit_move - before.edit_move;
			const int d_hr = s.pool_from_eq.edit_hp_regen - before.edit_hp_regen;
			const int d_mr = s.pool_from_eq.edit_mana_regen - before.edit_mana_regen;
			const int d_vr = s.pool_from_eq.edit_move_regen - before.edit_move_regen;
			if(d_hit != 0 || d_mana != 0 || d_move != 0 || d_hr != 0 || d_mr != 0 ||
			   d_vr != 0) {
				PoolResidualPiece piece;
				piece.inventory_id = r.id;
				piece.item_number = r.elem.item_number;
				piece.short_desc = r.elem.sd[0] ? r.elem.sd : (o->short_description
																  ? o->short_description
																  : "");
				piece.hit = d_hit;
				piece.mana = d_mana;
				piece.move = d_move;
				piece.hit_regen = d_hr;
				piece.mana_regen = d_mr;
				piece.move_regen = d_vr;
				s.pool_residuals.push_back(std::move(piece));
			}
			if(proto) {
				extract_obj(proto);
			}
		}
		extract_obj(o);
	}
	return s;
}

/**
 * Riepilogo edit principali + residui pool per un toon (login overview).
 * Analisi inventario completa; simboli clan esclusi dai tetti.
 */
[[nodiscard]] Json build_toon_edit_summary(unsigned long long toon_id) {
	Json out;
	out["toon_id"] = toon_id;
	const std::string toon_name = toon_name_by_id(toon_id);
	out["name"] = toon_name;
	const int max_level = max_level_for_toon(toon_id);
	out["max_level"] = max_level;
	out["grade"] = immortal_grade_label(max_level);
	if(toon_name.empty()) {
		out["ok"] = false;
		out["error"] = "toon non trovato";
		return out;
	}

	const ToonInventoryEditScan scan = scan_toon_inventory_edits(toon_id, toon_name);
	const int dam_total = scan.dam_total;
	const int sp_total = scan.sp_total;
	const int hr_total = scan.hr_total;
	const int spellfail_total = scan.spellfail_total;
	const bool res_slash = scan.res_slash;
	const bool res_pierce = scan.res_pierce;
	const bool res_blunt = scan.res_blunt;
	const bool slash_proto = scan.slash_proto;
	const bool slash_edit = scan.slash_edit;
	const bool pierce_proto = scan.pierce_proto;
	const bool pierce_edit = scan.pierce_edit;
	const bool blunt_proto = scan.blunt_proto;
	const bool blunt_edit = scan.blunt_edit;
	const bool has_clan_symbol = scan.has_clan_symbol;
	const char_edit_pool_data& pool_from_eq = scan.pool_from_eq;
	const int edited_pieces = scan.edited_pieces;

	char_edit_pool_data pool_db {};
	const bool has_pool_row = load_edit_pool_for_toon(toon_id, pool_db);
	const bool pool_on_character = has_pool_row && pool_db.migrated != 0;
	const bool residual_on_objects = pool_data_has_values(pool_from_eq);
	const char_edit_pool_data& pool_used =
		pool_on_character ? pool_db : pool_from_eq;

	auto pool_field = [&](const char* key, EditPoolField field, int used) {
		const int cap = edit_pool_field_cap(field);
		Json f;
		f["key"] = key;
		f["used"] = std::max(0, used);
		f["cap"] = cap;
		f["remaining"] = std::max(0, cap - std::max(0, used));
		return f;
	};
	Json pool = Json::object();
	/* Sempre "sul PG": e' l'editpool del personaggio (come comando editpool). */
	pool["source"] = "character";
	pool["on_character"] = pool_on_character;
	pool["migrated"] = pool_on_character ? 1 : 0;
	pool["values_from"] = pool_on_character ? "character" : "objects";
	pool["residual_on_objects"] = residual_on_objects;
	{
		Json residual_list = Json::array();
		for(const PoolResidualPiece& p : scan.pool_residuals) {
			Json j;
			j["inventory_id"] = p.inventory_id;
			j["item_number"] = p.item_number;
			j["short_desc"] = p.short_desc;
			j["hit"] = p.hit;
			j["mana"] = p.mana;
			j["move"] = p.move;
			j["hit_regen"] = p.hit_regen;
			j["mana_regen"] = p.mana_regen;
			j["move_regen"] = p.move_regen;
			residual_list.push_back(j);
		}
		pool["residual_objects"] = residual_list;
	}
	if(pool_on_character && residual_on_objects) {
		pool["warning"] =
			"Pool sul PG (migrato) ma restano ancora HIT/MANA/MOVE/regen in "
			"eccesso su oggetti EDIT: stato misto (migrazione incompleta).";
	}
	else if(!pool_on_character && residual_on_objects) {
		pool["warning"] =
			"Pool non ancora migrato sul PG: i valori sotto sono ancora sugli "
			"oggetti EDIT (verranno sul personaggio con la migrazione).";
	}
	Json fields = Json::array();
	fields.push_back(
		pool_field("hit", EditPoolField::Hp, pool_used.edit_hp));
	fields.push_back(
		pool_field("mana", EditPoolField::Mana, pool_used.edit_mana));
	fields.push_back(
		pool_field("move", EditPoolField::Move, pool_used.edit_move));
	fields.push_back(pool_field("hit_regen", EditPoolField::HpRegen,
								pool_used.edit_hp_regen));
	fields.push_back(pool_field("mana_regen", EditPoolField::ManaRegen,
								pool_used.edit_mana_regen));
	fields.push_back(pool_field("move_regen", EditPoolField::MoveRegen,
								pool_used.edit_move_regen));
	pool["fields"] = fields;

	Json main = Json::object();
	main["res_slash"] = resist_status_json(res_slash, slash_proto, slash_edit);
	main["res_pierce"] = resist_status_json(res_pierce, pierce_proto, pierce_edit);
	main["res_blunt"] = resist_status_json(res_blunt, blunt_proto, blunt_edit);
	{
		/* Tetto personaggio unico DAM+SP (non 30 per canale). */
		const int dam_sp_used = dam_total + sp_total;
		const int dam_sp_cap = kObjEditMaxDamSpEditableTotal;
		const int dam_sp_remaining = std::max(0, dam_sp_cap - dam_sp_used);
		Json dam_sp;
		dam_sp["used"] = dam_sp_used;
		dam_sp["cap"] = dam_sp_cap;
		dam_sp["remaining"] = dam_sp_remaining;
		dam_sp["dam_used"] = dam_total;
		dam_sp["sp_used"] = sp_total;
		dam_sp["note"] =
			"Tetto condiviso: dam editato + spellpower editato <= 30. "
			"Sul pezzo possono coesistere: somma delta wiz dam+sp <= 2 "
			"(es. hit-n-sp 1 + dam 1)";
		main["dam_sp"] = dam_sp;
		Json dam;
		dam["used"] = dam_total;
		dam["cap"] = dam_sp_cap;
		dam["remaining"] = dam_sp_remaining;
		dam["shared_used"] = dam_sp_used;
		dam["shared_cap"] = dam_sp_cap;
		dam["note"] =
			"Pool condiviso con spellpower: ancora " +
			std::to_string(dam_sp_remaining) + " su " +
			std::to_string(dam_sp_cap) + " (DAM+SP " +
			std::to_string(dam_sp_used) + "/" + std::to_string(dam_sp_cap) +
			"). Pezzo: dam e sp insieme ok se somma delta wiz <= 2";
		main["dam"] = dam;
		Json sp;
		sp["used"] = sp_total;
		sp["cap"] = dam_sp_cap;
		sp["remaining"] = dam_sp_remaining;
		sp["shared_used"] = dam_sp_used;
		sp["shared_cap"] = dam_sp_cap;
		sp["note"] =
			"Pool condiviso con dam: ancora " + std::to_string(dam_sp_remaining) +
			" su " + std::to_string(dam_sp_cap) + " (DAM+SP " +
			std::to_string(dam_sp_used) + "/" + std::to_string(dam_sp_cap) +
			"). Pezzo: dam e sp insieme ok se somma delta wiz <= 2";
		main["spellpower"] = sp;
		Json hr;
		hr["used"] = hr_total;
		hr["cap"] = kObjEditMaxHitrollEditableTotal;
		hr["remaining"] = std::max(0, kObjEditMaxHitrollEditableTotal - hr_total);
		hr["per_piece_max"] = kObjEditMaxHitrollPerPiece;
		main["hitroll"] = hr;
		Json sf;
		sf["used"] = spellfail_total;
		sf["cap"] = kObjEditMaxSpellfailEditableTotal;
		sf["remaining"] =
			std::max(0, kObjEditMaxSpellfailEditableTotal - spellfail_total);
		sf["step"] = kObjEditSpellfailStep;
		sf["mxp_per_step"] = 20;
		main["spellfail"] = sf;
	}

	int exp = 0;
	int rune = 0;
	int gold = 0;
	int bank_gold = 0;
	const bool has_stats = load_stats_for_toon(toon_id, exp, rune, &gold, &bank_gold);
	const long long prince_reserve =
		(max_level >= PRINCIPE) ? static_cast<long long>(PRINCEEXP) : 0LL;
	const long long available_xp =
		has_stats ? (static_cast<long long>(exp) - prince_reserve) : 0LL;
	const long long avail_clamped = std::max(0LL, available_xp);

	out["ok"] = true;
	out["edited_pieces"] = edited_pieces;
	out["available_mxp"] = avail_clamped / 1000000L;
	out["available_mxp_frac"] = (avail_clamped % 1000000L) / 10000L;
	out["rune"] = rune;
	out["gold"] = gold;
	out["bank_gold"] = bank_gold;
	{
		Json clan;
		clan["present"] = has_clan_symbol;
		const std::string& owner = scan.clan_symbol_owner;
		clan["owner_name"] = owner;
		const bool own =
			has_clan_symbol && !owner.empty() &&
			strcasecmp(owner.c_str(), toon_name.c_str()) == 0;
		clan["own"] = own;
		if(!has_clan_symbol) {
			clan["owner_kind"] = "none";
			clan["label"] = "No";
		}
		else if(own) {
			clan["owner_kind"] = "own";
			clan["label"] = std::string("SI - del clan di ") + toon_name;
		}
		else if(!owner.empty()) {
			clan["owner_kind"] = "other";
			clan["label"] = std::string("SI - del clan di ") + owner;
		}
		else {
			clan["owner_kind"] = "unknown";
			clan["label"] = "SI - del clan di (sconosciuto)";
		}
		out["clan_symbol"] = clan;
	}
	out["main_edits"] = main;
	out["pool"] = pool;
	if(max_level >= IMMORTALE) {
		out["commands"] = commands_enabled_at_level(max_level);
		out["commands_count"] = out["commands"].size();
	}
	return out;
}

[[nodiscard]] std::string handle_internal(const std::string& path,
										  const std::string& body) {
	try {
		const Json req = body.empty() ? Json::object() : Json::parse(body);

		if(path == "/internal/ping") {
			Json d;
			d["service"] = "myst-edit-api";
			d[kEditPortalApiVersionTag] = kEditPortalApiVersion;
			return json_ok(d);
		}

		if(path == "/internal/is-online") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const std::string name = toon_name_by_id(toon_id); /* SQL, non liste live */
			Json d;
			d["toon_id"] = toon_id;
			d["online"] = is_toon_online(toon_id, name);
			return json_ok(d);
		}

		if(path == "/internal/toon-edit-summary") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			if(toon_id == 0) {
				return json_error("toon_id richiesto", 400);
			}
			return json_ok(build_toon_edit_summary(toon_id));
		}

		if(path == "/internal/toons-edit-summary") {
			Json ids = Json::array();
			if(req.find("toon_ids") != req.end() && req["toon_ids"].is_array()) {
				ids = req["toon_ids"];
			}
			Json list = Json::array();
			for(const auto& idv : ids) {
				unsigned long long tid = 0;
				if(idv.is_number_unsigned() || idv.is_number_integer()) {
					tid = idv.get<unsigned long long>();
				}
				else if(idv.is_string()) {
					try {
						tid = std::stoull(idv.get<std::string>());
					}
					catch(...) {
						tid = 0;
					}
				}
				if(tid == 0) {
					continue;
				}
				list.push_back(build_toon_edit_summary(tid));
			}
			Json d;
			d["summaries"] = list;
			return json_ok(d);
		}

		if(path == "/internal/max-level") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			Json d;
			d["toon_id"] = toon_id;
			d["max_level"] = max_level_for_toon(toon_id);
			return json_ok(d);
		}

		if(path == "/internal/get-edit-catalog") {
			Json cfg;
			try {
				cfg = Json::parse(edit_system_config_to_json());
			}
			catch(...) {
				cfg = Json::object();
			}
			Json entries = Json::array();
			if(cfg.find("entries") != cfg.end() && cfg["entries"].is_array()) {
				for(const auto& e : cfg["entries"]) {
					if(!e.value("enabled", true)) {
						continue;
					}
					Json out = e;
					const std::string kind = e.value("kind", "");
					if(kind == "pool") {
						const std::string field = e.value("pool_field", "");
						EditPoolField pf {};
						if(portal_pool_parse_field_key(field.c_str(), pf)) {
							out["step"] = portal_pool_field_step(pf);
							out["cap"] = edit_pool_field_cap(pf);
							/* Step display: mono + artifact (class_mult sul quote reale). */
							const portal_pool_quote step_quote =
								portal_pool_quote_paid(pf, portal_pool_field_step(pf), 1);
							out["mxp_per_step"] = step_quote.mxp;
							out["mxp_frac_per_step"] = step_quote.mxp_frac;
							out["pq_per_step"] = step_quote.pq;
						}
					}
					if(kind == "resistance") {
						out["min"] = -100;
						out["max"] = 100;
						out["step"] = 25;
					}
					if(kind == "affect") {
						const int loc = e.value("location", 0);
						ObjEditListinoSpec spec {};
						if(loc != 0 && obj_edit_listino_spec(loc, spec)) {
							out["step"] = spec.step;
							out["min"] = spec.min_total;
							out["max"] = spec.max_total;
							out["cap"] = spec.max_total;
							out["cap_group"] = portal_listino_affect_cap_group(loc);
							/* Prezzo di uno step su oggetto (mono, artifact 3/2 del listino). */
							const double step_xp =
								static_cast<double>(spec.positive_unit_raw) *
								static_cast<double>(std::abs(spec.step != 0 ? spec.step : 1)) *
								static_cast<double>(kObjValueStorageScale) *
								portal_listino_param("artifact_mult", 1.5);
							int rn = 0;
							int rd = 1;
							int step_pq = 0;
							if(portal_listino_affect_rune(loc, rn, rd) && rn > 0 && rd > 0) {
								step_pq = static_cast<int>(std::ceil(
									static_cast<double>(rn) *
									static_cast<double>(std::abs(spec.step != 0 ? spec.step : 1)) *
									portal_listino_param("artifact_mult", 1.5) /
									static_cast<double>(rd) - 1e-9));
							}
							out["mxp_per_step"] = static_cast<long>(step_xp) / 1000000L;
							out["pq_per_step"] = step_pq;
						}
					}
					entries.push_back(out);
				}
			}
			/* Affect listino presenti in DB ma non ancora in edit_system.json. */
			const int n_aff = portal_listino_affect_count();
			for(int i = 0; i < n_aff; ++i) {
				ObjEditListinoSpec spec {};
				if(!portal_listino_affect_at(i, spec)) {
					continue;
				}
				bool already = false;
				for(const auto& e : entries) {
					if(e.value("kind", "") == "affect" &&
					   e.value("location", 0) == spec.location) {
						already = true;
						break;
					}
				}
				if(already) {
					continue;
				}
				if(!edit_system_affect_enabled(spec.location)) {
					continue;
				}
				Json out;
				out["id"] = std::string("affect.") + (spec.id ? spec.id : "x");
				out["kind"] = "affect";
				out["location"] = spec.location;
				out["label"] = spec.label ? spec.label : "";
				out["target"] = edit_system_affect_target(spec.location) ==
										EditSystemTarget::Character
									? "character"
									: "object";
				out["enabled"] = true;
				out["step"] = spec.step;
				out["min"] = spec.min_total;
				out["max"] = spec.max_total;
				out["cap"] = spec.max_total;
				out["cap_group"] = portal_listino_affect_cap_group(spec.location);
				entries.push_back(out);
			}
			Json d;
			d["entries"] = entries;
			d["pq_per_mega_xp"] = kEditPoolPqPerMegaXp;
			return json_ok(d);
		}

		if(path == "/internal/get-character-state") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			if(toon_id == 0) {
				return json_error("toon_id richiesto", 400);
			}
			int exp = 0;
			int rune = 0;
			const bool has_stats = load_stats_for_toon(toon_id, exp, rune);
			const int max_level = max_level_for_toon(toon_id);
			const long long prince_reserve =
				(max_level >= PRINCIPE) ? static_cast<long long>(PRINCEEXP) : 0LL;
			const long long available_xp =
				has_stats ? (static_cast<long long>(exp) - prince_reserve) : 0LL;

			char_edit_pool_data pool {};
			if(has_stats) {
				(void)load_edit_pool_for_toon(toon_id, pool);
			}

			Json resistances = Json::array();
			DB* db = Sql::getMysql();
			if(db) {
				try {
					odb::connection_ptr cp(db->connection());
					auto& mc = static_cast<odb::mysql::connection&>(*cp);
					MYSQL* h = mc.handle();
					const std::string sql =
						"SELECT damage_type, value FROM character_resistance WHERE toon_id = " +
						std::to_string(toon_id) + " ORDER BY damage_type";
					if(mysql_query(h, sql.c_str()) == 0) {
						MYSQL_RES* res = mysql_store_result(h);
						if(res) {
							MYSQL_ROW row;
							while((row = mysql_fetch_row(res))) {
								Json r;
								r["damage_type"] = row[0] ? std::atoi(row[0]) : 0;
								r["value"] = row[1] ? std::atoi(row[1]) : 0;
								resistances.push_back(r);
							}
							mysql_free_result(res);
						}
					}
				}
				catch(...) {
					/* resistenze opzionali */
				}
			}

			/* Nessun affect permanente sul PG nel Server (vedi apply-char-affect). */
			const Json char_affects = Json::array();

			Json d;
			d["toon_id"] = toon_id;
			d["name"] = toon_name_by_id(toon_id);
			d["max_level"] = max_level;
			d["exp"] = exp;
			d["rune"] = rune;
			d["available_xp"] = available_xp;
			d["available_mxp"] = available_xp / 1000000L;
			d["available_mxp_frac"] = (available_xp % 1000000L) / 10000L;
			d["prince_reserve_mxp"] = prince_reserve / 1000000L;
			d["pool"] = pool_to_json(pool);
			d["resistances"] = resistances;
			d["char_affects"] = char_affects;
			d["stats_missing"] = !has_stats;
			{
				const int n_classes = class_count_for_toon(toon_id);
				d["n_classes"] = n_classes;
				d["class_mult"] = portal_pool_class_mult(n_classes);
			}
			if(!has_stats) {
				d["warning"] =
					"Nessuna riga in character_stats per questo toon (PG non migrato "
					"o mai creato via login/import). Inventario consultabile; "
					"pagamenti edit bloccati finche' non esiste character_stats.";
			}
			return json_ok(d);
		}

		if(path == "/internal/quote-pool") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const std::string field = req.value("field", "");
			EditPoolField pool_field {};
			if(toon_id == 0 || !portal_pool_parse_field_key(field.c_str(), pool_field)) {
				return json_error("toon_id e campo pool valido richiesti", 400);
			}
			if(!edit_system_pool_enabled(field.c_str())) {
				return json_error("campo pool disabilitato in edit_system.json", 400);
			}
			if(edit_system_pool_target(field.c_str()) != EditSystemTarget::Character) {
				return json_error("campo pool non configurato sul personaggio", 400);
			}

			char_edit_pool_data pool {};
			if(!load_edit_pool_for_toon(toon_id, pool)) {
				return json_error("character_stats non trovato", 404);
			}

			const int current = pool_field_current(pool, pool_field);
			const int cap = edit_pool_field_cap(pool_field);
			const int requested = req.find("new_value") != req.end()
									  ? parse_json_int(req, "new_value", current)
									  : current + parse_json_int(req, "delta", 0);
			if(requested < current) {
				return json_error(
					"non puoi ridurre il pool edit (solo aumentare rispetto all'attuale)",
					400);
			}
			/* Come editpool: clamp al cap listino, eccesso scartato (no overedit). */
			const int target = std::min(requested, cap);
			const int delta = target - current;
			const int overflow = std::max(0, requested - target);
			const int n_classes = class_count_for_toon(toon_id);
			const int max_level = max_level_for_toon(toon_id);
			const bool free_edit = max_level >= DIO_MINORE;

			portal_pool_quote quote {};
			if(delta > 0 && !free_edit) {
				quote = portal_pool_quote_paid(pool_field, delta, n_classes);
			}
			Json d = quote_from_pool(quote);
			d["field"] = field;
			d["current"] = current;
			d["target"] = target;
			d["requested"] = requested;
			d["delta"] = delta;
			d["overflow"] = overflow;
			d["cap"] = cap;
			d["n_classes"] = n_classes;
			d["class_mult"] = portal_pool_class_mult(n_classes);
			d["free_edit"] = free_edit ? 1 : 0;
			d["payment_model"] = "editpool";
			if(overflow > 0) {
				d["note"] = "Cap listino: eccesso scartato (come editpool, no overedit)";
			}
			else if(free_edit && delta > 0) {
				d["note"] = "Target livello >= 53: edit pool listino gratis (come editpool)";
			}
			return json_ok(d);
		}

		if(path == "/internal/quote-resistance") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const unsigned damage_type =
				static_cast<unsigned>(parse_json_int(req, "damage_type", 0));
			const int target_value = parse_json_int(req, "value", 0);
			if(toon_id == 0 || damage_type == 0) {
				return json_error("toon_id e damage_type richiesti", 400);
			}
			if(!edit_system_resistance_enabled(damage_type)) {
				return json_error("resistenza disabilitata in edit_system.json", 400);
			}
			if(edit_system_resistance_target(damage_type) != EditSystemTarget::Character) {
				return json_error("resistenza non configurata sul personaggio", 400);
			}

			int current = 0;
			DB* db = Sql::getMysql();
			if(db) {
				try {
					odb::connection_ptr cp(db->connection());
					auto& mc = static_cast<odb::mysql::connection&>(*cp);
					MYSQL* h = mc.handle();
					const std::string sql =
						"SELECT value FROM character_resistance WHERE toon_id = " +
						std::to_string(toon_id) + " AND damage_type = " +
						std::to_string(damage_type) + " LIMIT 1";
					if(mysql_query(h, sql.c_str()) == 0) {
						MYSQL_RES* res = mysql_store_result(h);
						if(res) {
							MYSQL_ROW row = mysql_fetch_row(res);
							if(row && row[0]) {
								current = std::atoi(row[0]);
							}
							mysql_free_result(res);
						}
					}
				}
				catch(...) {
					/* default current 0 */
				}
			}

			const int delta = target_value - current;
			const long xp_raw =
				static_cast<long>(std::abs(delta)) * resistance_bit_xp_raw(damage_type) /
				100L * kObjValueStorageScale;
			Json d = quote_xp_json(xp_raw);
			d["damage_type"] = damage_type;
			d["current"] = current;
			d["target"] = target_value;
			d["delta"] = delta;
			return json_ok(d);
		}

		if(path == "/internal/list-inventory") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			if(toon_id == 0) {
				return json_error("toon_id richiesto", 400);
			}
			const std::string toon_name = toon_name_by_id(toon_id);
			std::vector<inventory_mysql_row> rows;
			if(!load_character_inventory_mysql(toon_id, rows)) {
				return json_error("impossibile leggere character_inventory da MySQL", 500);
			}
			Json items = Json::array();
			int editable_count = 0;
			int hidden_raro = 0;
			int hidden_tan = 0;
			int hidden_category = 0;
			int hidden_other = 0;
			const bool toon_name_ok = !toon_name.empty();
			for(const auto& r : rows) {
				struct obj_data* obj = materialize_inventory_row(r);
				if(obj && !object_portal_show_in_inventory_list(obj, toon_name.c_str())) {
					if(object_is_tanned(obj)) {
						++hidden_tan;
					}
					else if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
						++hidden_raro;
					}
					else {
						const char* slug = object_portal_item_type_slug(ITEM_TYPE(obj));
						if(slug && !edit_system_portal_category_enabled(slug)) {
							++hidden_category;
						}
						else {
							++hidden_other;
						}
					}
					extract_obj(obj);
					continue;
				}

				Json it;
				it["inventory_id"] = r.id;
				it["list_index"] = r.list_index;
				it["parent_inventory_id"] = r.parent_inventory_id;
				it["instance_id"] = r.instance_id;
				it["item_number"] = r.elem.item_number;
				it["short_desc"] = r.elem.sd;
				it["name"] = r.elem.name;
				it["wear_pos"] = r.elem.wearpos;
				it["depth"] = r.elem.depth;
				it["worn"] = inventory_row_is_worn(r.elem.wearpos);

				if(obj) {
					const char* type_slug = object_portal_item_type_slug(ITEM_TYPE(obj));
					if(type_slug) {
						it["item_type"] = type_slug;
					}
					else if(IS_OBJ_STAT2(obj, ITEM2_EDIT)) {
						it["item_type"] = "edited";
					}
				}

				std::string skip_reason;
				bool editable = false;
				if(!toon_name_ok) {
					skip_reason = "PG non risolto in myst (toon_id)";
				}
				else if(!obj) {
					skip_reason = "non materializzabile";
				}
				else if(inventory_row_is_worn(r.elem.wearpos) &&
						!object_portal_allows_worn_edit(obj)) {
					/* Difesa: allows_worn_edit e' sempre true per obj valido. */
					skip_reason = "indossato (non editabile)";
				}
				else if(object_is_tanned(obj)) {
					skip_reason = "conciato (skill tan)";
				}
				else if(object_portal_editable(obj, toon_name.c_str())) {
					editable = true;
					const char* slug = object_portal_item_type_slug(ITEM_TYPE(obj));
					if(slug) {
						it["item_type"] = slug;
						it["portal_category"] = slug;
					}
					else {
						it["item_type"] = "other";
						it["portal_category"] = "edited";
					}
				}
				else {
					skip_reason = object_portal_skip_reason(obj, toon_name.c_str());
				}

				if(obj) {
					extract_obj(obj);
				}

				it["editable"] = editable;
				if(!skip_reason.empty()) {
					it["skip_reason"] = skip_reason;
				}
				if(editable) {
					++editable_count;
				}
				items.push_back(it);
			}
			Json d;
			d["items"] = items;
			d["total"] = items.size();
			d["editable_count"] = editable_count;
			d["loaded_rows"] = rows.size();
			d["hidden_rows"] = static_cast<int>(rows.size()) - static_cast<int>(items.size());
			{
				Json filt;
				filt["raro"] = hidden_raro;
				filt["tan"] = hidden_tan;
				filt["category"] = hidden_category;
				filt["other"] = hidden_other;
				d["hidden_breakdown"] = filt;
			}
			d["toon_id"] = toon_id;
			d["toon_name"] = toon_name;
			d["toon_name_ok"] = toon_name_ok;
			d[kEditPortalApiVersionTag] = kEditPortalApiVersion;
			return json_ok(d);
		}

		if(path == "/internal/get-object-edit-options") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const unsigned long long inventory_id = parse_json_ull(req, "inventory_id");
			const std::string toon_name = toon_name_by_id(toon_id);
			if(toon_name.empty()) {
				return json_error("toon non trovato", 404);
			}

			std::vector<inventory_mysql_row> rows;
			load_character_inventory_mysql(toon_id, rows);
			inventory_mysql_row* row = find_inventory_row(rows, inventory_id);
			if(!row) {
				return json_error("oggetto non trovato nell'inventario del toon", 404);
			}

			struct obj_data* obj = materialize_inventory_row(*row);
			if(!obj) {
				return json_error("impossibile materializzare oggetto", 500);
			}
			if(inventory_row_is_worn(row->elem.wearpos) &&
			   !object_portal_allows_worn_edit(obj)) {
				extract_obj(obj);
				return json_error("oggetto indossato non editabile", 400);
			}
			if(!object_portal_editable(obj, toon_name.c_str())) {
				extract_obj(obj);
				return json_error("oggetto non editabile (RARO, tan, tipo o owner)", 400);
			}
			/* class_mult listino (Artifact / affect) sul PG target, anche se
			 * il pezzo non ha ancora personal_owner / ED. */
			ensure_portal_listino_owner(obj, toon_name);

			const int item_type = ITEM_TYPE(obj);
			const bool is_armor =
				item_type == ITEM_ARMOR || item_type == ITEM_WORN;
			const bool is_weapon =
				item_type == ITEM_WEAPON || item_type == ITEM_FIREWEAPON ||
				item_type == ITEM_MISSILE;
			Json entries = object_edit_catalog_json(obj);
			for(auto& e : entries) {
				const std::string kind = e.value("kind", "");
				if(kind == "scalar") {
					const int loc = e.value("location", 0);
					e["current"] = object_edit_display_current(obj, loc);
					/* Alias UI: stesso floor gia' in catalog (value_floor). */
					if(e.find("value_floor") != e.end()) {
						e["combat_floor"] = e["value_floor"];
					}
					if(loc == APPLY_HITNDAM) {
						e["combat_hr"] =
							object_edit_display_current(obj, APPLY_HITROLL);
						e["combat_dam"] =
							object_edit_display_current(obj, APPLY_DAMROLL);
					}
					else if(loc == APPLY_HITNSP) {
						e["combat_hr"] =
							object_edit_display_current(obj, APPLY_HITROLL);
						e["combat_sp"] =
							object_edit_display_current(obj, APPLY_SPELLPOWER);
					}
				}
				else if(kind == "immune" || kind == "m_immune") {
					const unsigned bit =
						static_cast<unsigned>(e.value("immune_bit", 0U));
					const int loc = e.value("location", APPLY_IMMUNE);
					const int bits = (loc == APPLY_M_IMMUNE)
										 ? object_m_immune_current_bits(obj)
										 : object_immune_current_bits(obj);
					e["current"] = (bits & static_cast<int>(bit)) ? 1 : 0;
				}
				else if(kind == "spell") {
					const int loc = e.value("location", APPLY_SPELL);
					const unsigned long bit = e.value("spell_bit", 0UL);
					const int bits =
						(loc == APPLY_AFF2) ? object_aff2_current_bits(obj)
											: object_spell_current_bits(obj);
					e["current"] = (bits & static_cast<int>(bit)) ? 1 : 0;
				}
				else if(kind == "flag" && e.value("flag", "") == "artifact") {
					e["current"] = IS_OBJ_STAT(obj, ITEM_IMMUNE) ? 1 : 0;
				}
			}

			Json d = analyze_to_json(obj);
			d["entries"] = entries;
			d["affect_slots"] = object_affect_slots_json(obj);
			d["inventory_id"] = inventory_id;
			d["short_desc"] = row->elem.sd;
			d["clan_symbol"] =
				clan_symbol_is_obj(obj) || obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL;
			{
				const int piece_delta = object_edit_damroll_edited_delta(obj);
				const bool piece_worn = inventory_row_is_worn(row->elem.wearpos);
				const bool piece_edit = object_edit_counts_toward_combat_budget(
					obj, toon_name.c_str());
				const int other_dam = sum_owned_edited_damroll_excluding(
					rows, inventory_id, toon_name.c_str());
				const int piece_for_total = piece_edit ? piece_delta : 0;

				const int piece_sp_delta = object_edit_spellpower_edited_delta(obj);
				const int other_sp = sum_owned_edited_spellpower_excluding(
					rows, inventory_id, toon_name.c_str());
				const int piece_sp_for_total = piece_edit ? piece_sp_delta : 0;
				const int char_dam_total = other_dam + piece_for_total;
				const int char_sp_total = other_sp + piece_sp_for_total;
				const int char_dam_sp_total = char_dam_total + char_sp_total;
				const ObjEditCombatWizCaps wiz = object_edit_combat_wiz_caps(obj);
				const int piece_hr_delta = object_edit_hitroll_edited_delta(obj);
				const int other_hr = sum_owned_edited_hitroll_excluding(
					rows, inventory_id, toon_name.c_str());
				const int piece_hr_for_total = piece_edit ? piece_hr_delta : 0;
				const int char_hr_total = other_hr + piece_hr_for_total;

				Json dam_budget;
				dam_budget["piece"] = piece_delta;
				dam_budget["piece_full"] = piece_delta;
				dam_budget["piece_wiz"] = wiz.dam_wiz();
				dam_budget["piece_broker"] = wiz.dam_broker;
				dam_budget["piece_current"] = object_edit_damroll_total(obj);
				dam_budget["piece_proto"] = object_edit_damroll_prototype_total(obj);
				dam_budget["piece_proto_vnum"] = object_edit_prototype_vnum(obj);
				dam_budget["piece_worn"] = piece_worn;
				dam_budget["piece_edit"] = piece_edit;
				dam_budget["piece_max"] = kObjEditMaxDamrollPerPiece;
				dam_budget["other"] = other_dam;
				dam_budget["char_total"] = char_dam_total;
				dam_budget["char_max"] = kObjEditMaxDamSpEditableTotal;
				dam_budget["char_dam_sp_total"] = char_dam_sp_total;
				dam_budget["char_dam_sp_max"] = kObjEditMaxDamSpEditableTotal;
				dam_budget["char_remaining"] =
					std::max(0, kObjEditMaxDamSpEditableTotal - char_dam_sp_total);
				dam_budget["source"] = "owned_edit_delta_vs_baseline";
				dam_budget["contributors"] =
					owned_dam_budget_contributors_json(rows, toon_name.c_str());
				dam_budget["piece_has_dam"] = object_edit_damroll_total(obj) > 0;
				dam_budget["mutex_with_spellpower"] = false;
				dam_budget["piece_dam_sp_max"] = kObjEditMaxDamSpPerPiece;
				dam_budget["piece_hr_max"] = kObjEditMaxHitrollPerPiece;
				d["dam_budget"] = dam_budget;

				Json sp_budget;
				sp_budget["piece"] = piece_sp_delta;
				sp_budget["piece_full"] = piece_sp_delta;
				sp_budget["piece_wiz"] = wiz.sp_wiz();
				sp_budget["piece_broker"] = wiz.sp_broker;
				sp_budget["piece_current"] = object_edit_spellpower_total(obj);
				sp_budget["piece_max"] = kObjEditMaxSpellpowerPerPiece;
				sp_budget["other"] = other_sp;
				sp_budget["char_total"] = char_sp_total;
				sp_budget["char_max"] = kObjEditMaxDamSpEditableTotal;
				sp_budget["char_dam_sp_max"] = kObjEditMaxDamSpEditableTotal;
				sp_budget["char_dam_sp_total"] = char_dam_sp_total;
				sp_budget["char_remaining"] =
					std::max(0, kObjEditMaxDamSpEditableTotal - char_dam_sp_total);
				sp_budget["source"] = "owned_edit_delta_vs_baseline";
				sp_budget["piece_has_spellpower"] =
					object_edit_spellpower_total(obj) > 0;
				sp_budget["mutex_with_dam"] = false;
				sp_budget["piece_dam_sp_max"] = kObjEditMaxDamSpPerPiece;
				d["sp_budget"] = sp_budget;

				Json hr_budget;
				hr_budget["piece"] = piece_hr_delta;
				hr_budget["piece_full"] = piece_hr_delta;
				hr_budget["piece_wiz"] = wiz.hr_wiz();
				hr_budget["piece_broker"] = wiz.hr_broker;
				hr_budget["piece_current"] = object_edit_hitroll_total(obj);
				hr_budget["piece_max"] = kObjEditMaxHitrollPerPiece;
				hr_budget["other"] = other_hr;
				hr_budget["char_total"] = char_hr_total;
				hr_budget["char_max"] = kObjEditMaxHitrollEditableTotal;
				hr_budget["char_remaining"] =
					std::max(0, kObjEditMaxHitrollEditableTotal - char_hr_total);
				hr_budget["piece_edit"] = piece_edit;
				hr_budget["source"] = "owned_edit_delta_vs_baseline";
				d["hr_budget"] = hr_budget;

				const int piece_sf_delta = object_edit_spellfail_edited_delta(obj);
				const int other_sf = sum_owned_edited_spellfail_excluding(
					rows, inventory_id, toon_name.c_str());
				const int piece_sf_for_total = piece_edit ? piece_sf_delta : 0;
				Json sf_budget;
				sf_budget["piece"] = piece_sf_delta;
				sf_budget["piece_current"] = object_edit_spellfail_total(obj);
				sf_budget["piece_proto"] = object_edit_spellfail_prototype_total(obj);
				sf_budget["piece_max"] = -kObjEditSpellfailMinTotal;
				sf_budget["other"] = other_sf;
				sf_budget["char_total"] = other_sf + piece_sf_for_total;
				sf_budget["char_max"] = kObjEditMaxSpellfailEditableTotal;
				sf_budget["step"] = kObjEditSpellfailStep;
				sf_budget["mxp_per_step"] = 20;
				sf_budget["source"] = "owned_edit_delta_vs_baseline";
				d["sf_budget"] = sf_budget;
			}
			{
				/* Sempre digitabili in UI: si salvano solo insieme a un affect pagato. */
				Json text;
				text["can_edit"] = true;
				text["requires_paid_affect"] = true;
				text["name"] = obj->name ? obj->name : "";
				text["short_desc"] =
					obj->short_description ? obj->short_description : "";
				text["description"] = obj->description ? obj->description : "";
				text["name_max"] = kObjEditTextNameMax;
				text["short_max"] = kObjEditTextShortMax;
				text["long_max"] = kObjEditTextLongMax;
				text["hint"] =
					"Name / short / long: gratuiti ma solo insieme al pagamento "
					"di un nuovo affect (stesso salvataggio). Non si salvano da soli.";
				d["text_edit"] = text;
			}
			if(is_armor && is_weapon) {
				d["item_type"] = "armor+weapon";
			}
			else if(is_armor) {
				d["item_type"] = "armor";
			}
			else if(is_weapon) {
				d["item_type"] = "weapon";
			}
			else {
				d["item_type"] = "other";
			}
			extract_obj(obj);
			return json_ok(d);
		}

		if(path == "/internal/quote-object-edit") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const unsigned long long inventory_id = parse_json_ull(req, "inventory_id");
			const int location = parse_json_int(req, "location", 0);
			const int target_modifier =
				req.find("target_modifier") != req.end()
					? parse_json_int(req, "target_modifier", 0)
					: parse_json_int(req, "modifier", 0);
			const bool clear_slot = parse_json_int(req, "clear_slot", 0) != 0;
			const std::string flag = req.value("flag", "");

			const std::string toon_name = toon_name_by_id(toon_id);
			if(toon_name.empty()) {
				return json_error("toon non trovato", 404);
			}

			std::vector<inventory_mysql_row> rows;
			load_character_inventory_mysql(toon_id, rows);
			inventory_mysql_row* row = find_inventory_row(rows, inventory_id);
			if(!row) {
				return json_error("oggetto non trovato nell'inventario del toon", 404);
			}

			struct obj_data* obj = materialize_inventory_row(*row);
			if(!obj) {
				return json_error("impossibile materializzare oggetto", 500);
			}
			if(inventory_row_is_worn(row->elem.wearpos) &&
			   !object_portal_allows_worn_edit(obj)) {
				extract_obj(obj);
				return json_error("oggetto indossato non editabile", 400);
			}
			if(!object_portal_editable(obj, toon_name.c_str())) {
				extract_obj(obj);
				return json_error("oggetto non editabile (RARO, tan, tipo o owner)", 400);
			}
			ensure_portal_listino_owner(obj, toon_name);

			if(flag == "artifact") {
				const int current = IS_OBJ_STAT(obj, ITEM_IMMUNE) ? 1 : 0;
				const int target = target_modifier ? 1 : 0;
				if(current && !target) {
					extract_obj(obj);
					return json_error(
						"Artifact non rimovibile una volta impostato", 400);
				}
				const long xp_raw =
					(target && !current) ? EditArtifactFlagListinoCost(obj) : 0L;
				/* Rune alternative (max della riga 'artifact', oggi 100), senza class_mult.
				 * Non sono un costo extra: lo slider le mixa con gli XP (come rune_listino). */
				const int art_runes =
					(target && !current) ? EditArtifactFlagListinoRunes() : 0;
				Json d = quote_xp_json(xp_raw, 0);
				d["location"] = 0;
				d["flag"] = "artifact";
				d["payment_model"] = "object";
				d["rune_listino"] = art_runes;
				d["target_modifier"] = target;
				d["current"] = current;
				d["target"] = target;
				d["inventory_id"] = inventory_id;
				d["note"] =
					target && !current
						? "Artifact: costo listino (artifact_cost_xp x class_mult), "
						  "oppure fino a 100 rune in alternativa (senza class_mult), "
						  "con lo slider; senza +50% artifact_mult, senza ITEM2_EDIT, "
						  "senza cost 20001"
						: (current ? "Artifact gia' presente"
								   : "Artifact invariato");
				extract_obj(obj);
				return json_ok(d);
			}

			/* Listino: +50% se pezzo gia' Artifact OPPURE Artifact e' in coda
			 * nello stesso edit (pending_artifact dal carrello portal). */
			const bool pending_artifact =
				parse_json_int(req, "pending_artifact", 0) != 0;
			if(pending_artifact || IS_OBJ_STAT(obj, ITEM_IMMUNE)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
			}

			long xp_raw = 0;
			int pq = 0;
			std::string quote_err;
			ObjEditMalusSplit malus_split {};
			const bool touches_dam_sp =
				object_edit_location_affects_dam(location)
				|| object_edit_location_affects_spellpower(location);
			const int other_dam =
				touches_dam_sp ? sum_owned_edited_damroll_excluding(
									 rows, inventory_id, toon_name.c_str())
							   : -1;
			const int other_sp =
				touches_dam_sp ? sum_owned_edited_spellpower_excluding(
									 rows, inventory_id, toon_name.c_str())
							   : -1;
			const int other_sf =
				object_edit_location_affects_spellfail(location)
					? sum_owned_edited_spellfail_excluding(rows, inventory_id,
														  toon_name.c_str())
					: -1;
			const bool allow_combat_merge =
				parse_json_int(req, "allow_combat_merge", 0) != 0;
			if(!object_quote_affect_target(obj, location, target_modifier, xp_raw, pq,
										   quote_err, other_dam, other_sp, clear_slot,
										   other_sf, allow_combat_merge, &malus_split)) {
				extract_obj(obj);
				return json_error(quote_err.c_str(), 400);
			}

			int current = 0;
			int target = target_modifier;
			if(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE ||
			   location == APPLY_SUSC) {
				const int bits = (location == APPLY_M_IMMUNE)
									 ? object_m_immune_current_bits(obj)
									 : (location == APPLY_SUSC)
										   ? object_susc_current_bits(obj)
										   : object_immune_current_bits(obj);
				current = (bits & target_modifier) ? 1 : 0;
				target = clear_slot ? 0 : (current ? 1 : (target_modifier != 0 ? 1 : 0));
			}
			else if(location == APPLY_SPELL || location == APPLY_AFF2) {
				const int bits = (location == APPLY_AFF2)
									 ? object_aff2_current_bits(obj)
									 : object_spell_current_bits(obj);
				current = (bits & target_modifier) ? 1 : 0;
				target = clear_slot ? 0 : (current ? 1 : (target_modifier != 0 ? 1 : 0));
			}
			else {
				current = object_edit_display_current(obj, location);
				if(clear_slot) {
					target = 0;
				}
			}

			Json d = quote_xp_json(xp_raw, pq);
			d["location"] = location;
			/* Prezzo rune alternativo (listino): lo slider % rune lo usa al posto
			 * della conversione 1 MXP = 1 rune. pq resta rent aggiuntivo. */
			d["payment_model"] = "object";
			d["rune_listino"] = object_listino_rune_for_quote(
				location, xp_raw,
				(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE ||
				 location == APPLY_SUSC)
					? static_cast<unsigned long>(target_modifier)
					: 0UL);
			/* Su clear bit (resi/imm/spell) serve il bit in target_modifier. */
			d["target_modifier"] = target_modifier;
			d["clear_slot"] = clear_slot ? 1 : 0;
			d["current"] = current;
			d["target"] = target;
			d["inventory_id"] = inventory_id;
			d["artifact"] = IS_OBJ_STAT(obj, ITEM_IMMUNE) ? 1 : 0;
			d["pending_artifact"] = pending_artifact ? 1 : 0;
			d["paid_malus"] = IS_OBJ_STAT2(obj, ITEM2_PAID_MALUS) ? 1 : 0;
			d["allow_combat_merge"] = allow_combat_merge ? 1 : 0;
			d["will_set_paid_malus"] =
				object_edit_recovers_listino_malus(obj, location, target_modifier,
												   clear_slot)
					? 1
					: 0;
			if(malus_split.active) {
				d["malus_split"] = 1;
				d["malus_part"] = quote_xp_json(malus_split.malus_xp_raw, 0);
				d["malus_part"]["from"] = malus_split.from;
				d["malus_part"]["to"] = 0;
				d["malus_part"]["note"] = "Recupero malus a 0 (tariffa 2x)";
				d["bonus_part"] = quote_xp_json(malus_split.bonus_xp_raw, 0);
				d["bonus_part"]["from"] = 0;
				d["bonus_part"]["to"] = malus_split.to;
				d["bonus_part"]["note"] = "Bonus oltre lo zero (listino 1x)";
			}
			if(clear_slot) {
				d["note"] = "Rimuovi slot: libera lo slot (gratis, listino)";
			}
			else if(malus_split.active) {
				d["note"] =
					"Spezzato: malus→0 a tariffa 2×, poi 0→bonus a listino 1× "
					"(due apply in sequenza; baseline dopo il recupero)";
			}
			else if(d["will_set_paid_malus"] == 1 && xp_raw > 0) {
				d["note"] =
					"Recupero malus a tariffa 2× (ITEM2_PAID_MALUS); include class_mult "
					"e Artifact se presenti";
			}
			else if(IS_OBJ_STAT(obj, ITEM_IMMUNE) && xp_raw > 0) {
				d["note"] = "Include maggiorazione Artifact +50% (listino)";
			}
			extract_obj(obj);
			return json_ok(d);
		}

		if(path == "/internal/quote-item") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			const unsigned long long inventory_id = parse_json_ull(req, "inventory_id");
			std::vector<inventory_mysql_row> rows;
			load_character_inventory_mysql(toon_id, rows);
			inventory_mysql_row* row = find_inventory_row(rows, inventory_id);
			if(!row) {
				return json_error("oggetto non trovato nell'inventario del toon", 404);
			}
			struct obj_data* obj = materialize_inventory_row(*row);
			if(!obj) {
				return json_error("impossibile materializzare oggetto", 500);
			}
			Json d = analyze_to_json(obj);
			extract_obj(obj);
			return json_ok(d);
		}

		if(path == "/internal/apply-affect") {
			const unsigned long long target_toon_id = parse_json_ull(req, "target_toon_id");
			const unsigned long long inventory_id = parse_json_ull(req, "inventory_id");
			const int location = parse_json_int(req, "location", 0);
			const int target_modifier =
				req.find("target_modifier") != req.end()
					? parse_json_int(req, "target_modifier", 0)
					: parse_json_int(req, "modifier", 0);
			const bool clear_slot = parse_json_int(req, "clear_slot", 0) != 0;
			const int pay_xp = parse_json_int(req, "pay_xp", 0);
			const int pay_rune = parse_json_int(req, "pay_rune", 0);
			const std::string flag = req.value("flag", "");

			const std::string target_name = toon_name_by_id(target_toon_id);
			auto fail_affect = [&](const char* msg, int code) -> std::string {
				return portal_audit_fail(req, msg, code, target_toon_id,
										 target_name.c_str(), inventory_id);
			};
			auto ok_affect = [&](Json d, const char* kind) -> std::string {
				unsigned long long instance_id = 0ULL;
				const auto it_inst = d.find("instance_id");
				if(it_inst != d.end() && it_inst->is_number()) {
					instance_id = it_inst->get<unsigned long long>();
				}
				Json detail = d;
				portal_audit_event(req, kind, "info", kind, &detail, target_toon_id,
								   target_name.c_str(), inventory_id, instance_id);
				return json_ok(d);
			};
			if(target_name.empty()) {
				return fail_affect("toon target non trovato", 404);
			}
			if(is_toon_online(target_toon_id, target_name)) {
				return fail_affect("il toon target e collegato al mud", 409);
			}

			/* Pool su eq: bloccato, salvo clear gratis di affect nativi sul pezzo. */
			const bool allow_native_pool_clear =
				clear_slot &&
				(edit_pool_is_pool_apply(location) || location == APPLY_SAVE_ALL);
			if(flag.empty() &&
			   edit_system_blocked_on_object(location, target_modifier) &&
			   !(allow_native_pool_clear)) {
				if(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE ||
				   location == APPLY_SUSC) {
					return fail_affect(
						"questa resistenza e configurata sul personaggio — usa apply-resistance",
						400);
				}
				if(edit_pool_is_pool_apply(location)) {
					return fail_affect(
						"hit/mana/move/regen vanno sul personaggio (edit pool), non sull'oggetto",
						400);
				}
				return fail_affect("campo configurato sul personaggio, non sull'oggetto", 400);
			}

			std::vector<inventory_mysql_row> rows;
			load_character_inventory_mysql(target_toon_id, rows);
			inventory_mysql_row* row = find_inventory_row(rows, inventory_id);
			if(!row) {
				return fail_affect("oggetto non trovato nell'inventario", 404);
			}

			struct obj_data* obj = materialize_inventory_row(*row);
			if(!obj) {
				return fail_affect("impossibile materializzare oggetto", 500);
			}
			if(inventory_row_is_worn(row->elem.wearpos) &&
			   !object_portal_allows_worn_edit(obj)) {
				extract_obj(obj);
				return fail_affect("oggetto indossato non editabile", 400);
			}
			if(!object_portal_editable(obj, target_name.c_str())) {
				extract_obj(obj);
				return fail_affect("oggetto non editabile (RARO, tan, tipo o owner)", 400);
			}
			ensure_portal_listino_owner(obj, target_name);

			if(flag == "artifact") {
				const bool already = IS_OBJ_STAT(obj, ITEM_IMMUNE);
				if(already && !target_modifier) {
					extract_obj(obj);
					return fail_affect(
						"Artifact non rimovibile una volta impostato", 400);
				}
				if(already && target_modifier) {
					/* Gia' artifact: no-op (flag permanente). */
					Json d = analyze_to_json(obj);
					d["paid_xp"] = 0;
					d["paid_rune"] = 0;
					d["saved"] = true;
					d["flag"] = "artifact";
					d["artifact"] = 1;
					d["instance_id"] = obj->db_instance_id;
					d["base_vnum"] = object_instance_resolve_base_vnum(obj);
					extract_obj(obj);
					(void)pay_xp;
					(void)pay_rune;
					return ok_affect(d, "apply.affect");
				}
				if(!target_modifier) {
					extract_obj(obj);
					return fail_affect("Artifact: specificare target_modifier=1 per attivare",
									  400);
				}
				/*
				 * Portale: Artifact da solo = listino flag; in pacchetto con
				 * affect pagati la UI manda waive_artifact_fee (costo 0: il
				 * +50% e' gia' nel listino affect). Broker non vende Artifact.
				 */
				/* Server: il flag si paga sempre quando viene aggiunto (gli affect
				 * costano gia' 3/2 a parte); waive_artifact_fee ignorato. */
				const long quote_xp = EditArtifactFlagListinoCost(obj);
				/* pq=0: niente rune obbligatorie. quote_rune_listino = max alternativo
				 * (100, no class_mult): lo slider paga p rune e (100-p)% degli XP. */
				const int quote_rune_listino = EditArtifactFlagListinoRunes();
				int xp_cost = 0;
				int rune_cost = 0;
				std::string pay_resolve_err;
				if(!resolve_object_edit_payment(pay_xp, pay_rune, quote_xp, 0, xp_cost,
											   rune_cost, pay_resolve_err,
											   quote_rune_listino)) {
					extract_obj(obj);
					return fail_affect(pay_resolve_err.c_str(), 402);
				}
				struct obj_data* after = materialize_inventory_row(*row);
				if(!after) {
					extract_obj(obj);
					return fail_affect("impossibile clonare oggetto", 500);
				}
				extract_obj(obj);
				SET_BIT(after->obj_flags.extra_flags, ITEM_IMMUNE);
				/* Solo ARTIFACT: niente ITEM2_EDIT, niente cost 20001 / [RARO]. */
				const bool affect_is_paid = (quote_xp > 0) || (xp_cost > 0) ||
											(rune_cost > 0);
				bool text_changed = false;
				std::string text_err;
				if(!portal_apply_optional_text_fields(after, req, affect_is_paid, text_err,
													  text_changed)) {
					extract_obj(after);
					return fail_affect(text_err.c_str(), 400);
				}
				const int max_level = max_level_for_toon(target_toon_id);
				std::string pay_err;
				if(!deduct_payment(target_toon_id, xp_cost, rune_cost, max_level, pay_err)) {
					extract_obj(after);
					return fail_affect(pay_err.c_str(), 402);
				}
				std::string save_err;
				const bool saved = portal_save_edited_inventory_item(
					inventory_id, after, save_err, target_name.c_str(), nullptr,
					/*mark_edit=*/false);
				Json d = analyze_to_json(after);
				d["paid_xp"] = xp_cost;
				d["paid_rune"] = rune_cost;
				d["saved"] = saved;
				d["flag"] = "artifact";
				d["artifact"] = IS_OBJ_STAT(after, ITEM_IMMUNE) ? 1 : 0;
				d["text_changed"] = text_changed;
				d["instance_id"] = after->db_instance_id;
				d["base_vnum"] = object_instance_resolve_base_vnum(after);
				extract_obj(after);
				if(!saved) {
					return fail_affect(save_err.empty()
										  ? "[portal:save] pagamento eseguito ma salvataggio "
											"flag artifact fallito"
										  : save_err.c_str(),
									  500);
				}
				return ok_affect(d, "apply.affect");
			}

			/* Stesso pending_artifact del quote: evita sotto-quote in apply. */
			const bool pending_artifact =
				parse_json_int(req, "pending_artifact", 0) != 0;
			if(pending_artifact || IS_OBJ_STAT(obj, ITEM_IMMUNE)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_IMMUNE);
			}

			long quote_xp = 0;
			int quote_pq = 0;
			std::string quote_err;
			const bool touches_dam_sp =
				object_edit_location_affects_dam(location)
				|| object_edit_location_affects_spellpower(location);
			/* Budget combinato DAM+SP: servono sempre entrambi i totali "other". */
			const int other_dam =
				touches_dam_sp ? sum_owned_edited_damroll_excluding(
									 rows, inventory_id, target_name.c_str())
							   : -1;
			const int other_sp =
				touches_dam_sp ? sum_owned_edited_spellpower_excluding(
									 rows, inventory_id, target_name.c_str())
							   : -1;
			const int other_sf =
				object_edit_location_affects_spellfail(location)
					? sum_owned_edited_spellfail_excluding(rows, inventory_id,
														  target_name.c_str())
					: -1;
			const bool allow_combat_merge =
				parse_json_int(req, "allow_combat_merge", 0) != 0;
			ObjEditMalusSplit malus_split {};
			if(!object_quote_affect_target(obj, location, target_modifier, quote_xp,
										   quote_pq, quote_err, other_dam, other_sp,
										   clear_slot, other_sf, allow_combat_merge,
										   &malus_split)) {
				extract_obj(obj);
				return fail_affect(quote_err.c_str(), 400);
			}

			struct obj_data* after = materialize_inventory_row(*row);
			if(!after) {
				extract_obj(obj);
				return fail_affect("impossibile clonare oggetto", 500);
			}
			obj_affected_type reward_baseline_affs[MAX_OBJ_AFFECT];
			const bool need_reward_baseline =
				procarea_obj_is_reward(after) && after->db_instance_id == 0;
			if(need_reward_baseline) {
				std::memcpy(reward_baseline_affs, after->affected,
							sizeof(reward_baseline_affs));
			}
			const int cur_before = object_edit_display_current(after, location);
			const bool recovers_malus = object_edit_recovers_listino_malus(
				after, location, target_modifier, clear_slot);
			const bool malus_then_bonus =
				malus_split.active ||
				(recovers_malus && !clear_slot &&
				 object_edit_malus_crosses_to_bonus(location, cur_before,
													 target_modifier));
			extract_obj(obj);

			/* Pagamento sul totale quote (anche se spezzato in due apply). */
			int xp_cost = 0;
			int rune_cost = 0;
			std::string pay_resolve_err;
			if(!resolve_object_edit_payment(pay_xp, pay_rune, quote_xp, quote_pq, xp_cost,
										   rune_cost, pay_resolve_err,
										   object_listino_rune_for_quote(
											   location, quote_xp,
											   (location == APPLY_IMMUNE ||
												location == APPLY_M_IMMUNE ||
												location == APPLY_SUSC)
												   ? static_cast<unsigned long>(target_modifier)
												   : 0UL))) {
				extract_obj(after);
				return fail_affect(pay_resolve_err.c_str(), 402);
			}
			const int quote_xp_i = static_cast<int>(std::max(0L, quote_xp));
			const bool affect_is_paid = (quote_xp_i > 0) || (quote_pq > 0) ||
										(xp_cost > 0) || (rune_cost > 0);

			bool text_changed = false;
			std::string text_err;
			if(!portal_apply_optional_text_fields(after, req, affect_is_paid, text_err,
												  text_changed)) {
				extract_obj(after);
				return fail_affect(text_err.c_str(), 400);
			}

			const int max_level = max_level_for_toon(target_toon_id);
			std::string pay_err;
			if(!deduct_payment(target_toon_id, xp_cost, rune_cost, max_level, pay_err)) {
				extract_obj(after);
				return fail_affect(pay_err.c_str(), 402);
			}

			std::string apply_err;
			bool paid_malus_baseline = false;
			bool saved = false;
			std::string save_err;

			if(malus_then_bonus) {
				/*
				 * Fase 1: malus→0 (2× gia' nel quote totale), PAID_MALUS +
				 * baseline. Fase 2: 0→bonus (1×) senza riscrivere la baseline
				 * sul bonus (resta edit vs post-malus).
				 */
				if(!object_apply_affect_target(after, location, 0, apply_err, other_dam,
											   other_sp, false, other_sf,
											   allow_combat_merge)) {
					extract_obj(after);
					return fail_affect(apply_err.c_str(), 400);
				}
				if(!IS_OBJ_STAT2(after, ITEM2_EDIT)) {
					SET_BIT(after->obj_flags.extra_flags2, ITEM2_EDIT);
				}
				saved = portal_save_edited_inventory_item(
					inventory_id, after, save_err, target_name.c_str(),
					need_reward_baseline ? reward_baseline_affs : nullptr);
				if(saved && after->db_instance_id != 0) {
					paid_malus_baseline = object_instance_rebase_create_baseline(
						after, "edit_portal_paid_malus");
				}
				if(!saved) {
					extract_obj(after);
					return fail_affect(save_err.empty()
										   ? "[portal:save] malus→0 fallito dopo pagamento"
										   : save_err.c_str(),
									   500);
				}
				apply_err.clear();
				if(!object_apply_affect_target(after, location, target_modifier, apply_err,
											   other_dam, other_sp, false, other_sf,
											   allow_combat_merge)) {
					extract_obj(after);
					return fail_affect(apply_err.c_str(), 400);
				}
				save_err.clear();
				saved = portal_save_edited_inventory_item(
					inventory_id, after, save_err, target_name.c_str(), nullptr);
			}
			else {
				if(!object_apply_affect_target(after, location, target_modifier, apply_err,
											   other_dam, other_sp, clear_slot, other_sf,
											   allow_combat_merge)) {
					extract_obj(after);
					return fail_affect(apply_err.c_str(), 400);
				}
				if(!IS_OBJ_STAT2(after, ITEM2_EDIT)) {
					SET_BIT(after->obj_flags.extra_flags2, ITEM2_EDIT);
				}
				saved = portal_save_edited_inventory_item(
					inventory_id, after, save_err, target_name.c_str(),
					need_reward_baseline ? reward_baseline_affs : nullptr);
				if(saved && recovers_malus && after->db_instance_id != 0) {
					paid_malus_baseline = object_instance_rebase_create_baseline(
						after, "edit_portal_paid_malus");
					if(!paid_malus_baseline) {
						mudlog(LOG_SYSERR,
							   "edit_portal: paid_malus save ok ma rebase baseline fallito "
							   "inv=%llu inst=%llu",
							   static_cast<unsigned long long>(inventory_id),
							   static_cast<unsigned long long>(after->db_instance_id));
					}
				}
			}

			Json d = analyze_to_json(after);
			d["paid_xp"] = xp_cost;
			d["paid_rune"] = rune_cost;
			d["saved"] = saved;
			d["text_changed"] = text_changed;
			d["instance_id"] = after->db_instance_id;
			d["base_vnum"] = object_instance_resolve_base_vnum(after);
			d["paid_malus"] = IS_OBJ_STAT2(after, ITEM2_PAID_MALUS) ? 1 : 0;
			d["paid_malus_baseline"] = paid_malus_baseline ? 1 : 0;
			d["recovered_malus"] = recovers_malus ? 1 : 0;
			d["malus_split_applied"] = malus_then_bonus ? 1 : 0;
			if(!save_err.empty()) {
				d["save_err"] = save_err;
			}
			extract_obj(after);

			if(!saved) {
				return fail_affect(save_err.empty()
									  ? "[portal:save] pagamento eseguito ma salvataggio fallito"
									  : save_err.c_str(),
								  500);
			}
			return ok_affect(d, "apply.affect");
		}

		if(path == "/internal/quote-object-text" ||
		   path == "/internal/apply-object-text") {
			return json_error(
				"name/short/long non si salvano da soli: includili nel pagamento "
				"di un nuovo affect (apply-affect)",
				400);
		}

		if(path == "/internal/quote-char-affect" || path == "/internal/apply-char-affect") {
			/* Affect permanenti sul PG (character_edit_affect di nebbietest) non
			 * esistono nel Server: gli edit PG passano da editpool / apply-pool. */
			return json_error("affect sul personaggio non disponibili in questo MUD "
							  "(usa il pool edit)",
							  501);
		}

		if(path == "/internal/apply-pool") {
			const unsigned long long target_toon_id = parse_json_ull(req, "target_toon_id");
			const std::string field = req.value("field", "");
			const int req_pay_xp = parse_json_int(req, "pay_xp", 0);
			const int req_pay_rune = parse_json_int(req, "pay_rune", 0);

			const std::string target_name = toon_name_by_id(target_toon_id);

			auto fail_apply = [&](const char* msg, int code) -> std::string {
				return portal_audit_fail(req, msg, code, target_toon_id,
										 target_name.c_str(), 0);
			};
			auto ok_apply = [&](Json d) -> std::string {
				portal_audit_event(req, "apply.pool", "info", "apply.pool", &d,
								   target_toon_id, target_name.c_str(), 0, 0);
				return json_ok(d);
			};
			if(target_name.empty()) {
				return fail_apply("toon target non trovato", 404);
			}
			if(is_toon_online(target_toon_id, target_name)) {
				return fail_apply("il toon target e collegato al mud", 409);
			}

			EditPoolField pool_field {};
			if(!portal_pool_parse_field_key(field.c_str(), pool_field)) {
				return fail_apply("campo pool non valido", 400);
			}

			if(!edit_system_pool_enabled(field.c_str())) {
				return fail_apply("campo pool disabilitato in edit_system.json", 400);
			}
			if(edit_system_pool_target(field.c_str()) != EditSystemTarget::Character) {
				return fail_apply(
					"campo pool configurato sull'oggetto — usa apply-affect (APPLY)", 400);
			}

			DB* db = Sql::getMysql();
			if(!db) {
				return fail_apply("database non disponibile", 500);
			}

			char_edit_pool_data pool {};
			if(!load_edit_pool_for_toon(target_toon_id, pool)) {
				return fail_apply("character_stats non trovato", 404);
			}

			/*
			 * Invariante: niente pool misto. Se non migrato e restano delta
			 * HIT/MANA/MOVE/regen sugli oggetti EDIT, non impostare
			 * edit_pool_migrated=1 (nasconderebbe il residuo). Prima va
			 * completata la migrazione login/boot (credit + strip).
			 */
			if(pool.migrated == 0) {
				const ToonInventoryEditScan inv =
					scan_toon_inventory_edits(target_toon_id, target_name);
				if(pool_data_has_values(inv.pool_from_eq)) {
					return fail_apply(
						"Pool ancora sugli oggetti EDIT (migrazione incompleta). "
						"Entra in gioco una volta: la migrazione sposta i valori "
						"sul PG e li toglie dagli oggetti. Poi riprova dal portale.",
						409);
				}
			}

			const int current = pool_field_current(pool, pool_field);
			const int cap = edit_pool_field_cap(pool_field);
			const int requested = req.find("new_value") != req.end()
									  ? parse_json_int(req, "new_value", current)
									  : current + parse_json_int(req, "delta", 0);
			if(requested < current) {
				return fail_apply(
					"non puoi ridurre il pool edit (solo aumentare rispetto all'attuale)",
					400);
			}
			/* Come editpool: solo fino al cap; niente spill in overedit_*. */
			const int target = std::min(requested, cap);
			const int delta = target - current;
			if(delta == 0) {
				return fail_apply(
					requested > current
						? "listino gia' al cap: nessuna unita' applicabile"
						: "nessuna modifica al pool",
					400);
			}

			const int max_level = max_level_for_toon(target_toon_id);
			const int n_classes = class_count_for_toon(target_toon_id);
			const bool free_edit = max_level >= DIO_MINORE;
			const portal_pool_quote quote =
				free_edit ? portal_pool_quote {}
						  : portal_pool_quote_paid(pool_field, delta, n_classes);

			int xp_cost = 0;
			int rune_cost = 0;
			std::string pay_cover_err;
			if(!free_edit) {
				if(!resolve_editpool_payment(req_pay_xp, req_pay_rune, quote.xp_raw,
											 quote.pq, xp_cost, rune_cost,
											 pay_cover_err)) {
					return fail_apply(pay_cover_err.c_str(), 400);
				}
			}

			if(!portal_pool_set_absolute(&pool, pool_field, target)) {
				return fail_apply("delta pool rifiutato (cap listino)", 400);
			}

			std::string pay_err;
			if(!free_edit &&
			   !deduct_payment(target_toon_id, xp_cost, rune_cost, max_level, pay_err)) {
				return fail_apply(pay_err.c_str(), 402);
			}

			try {
				std::ostringstream upd;
				upd << "UPDATE character_stats SET edit_hp=" << pool.edit_hp
					<< ", edit_mana=" << pool.edit_mana << ", edit_move=" << pool.edit_move
					<< ", edit_hp_regen=" << pool.edit_hp_regen
					<< ", edit_mana_regen=" << pool.edit_mana_regen
					<< ", edit_move_regen=" << pool.edit_move_regen
					<< ", overedit_hp=" << pool.overedit_hp
					<< ", overedit_mana=" << pool.overedit_mana
					<< ", overedit_move=" << pool.overedit_move
					<< ", overedit_hp_regen=" << pool.overedit_hp_regen
					<< ", overedit_mana_regen=" << pool.overedit_mana_regen
					<< ", overedit_move_regen=" << pool.overedit_move_regen
					<< ", edit_pool_migrated=1 WHERE toon_id = " << target_toon_id;
				std::string sql_err;
				if(!portal_mysql_exec(db, upd.str(), sql_err, "portal:apply_pool")) {
					return fail_apply(sql_err.c_str(), 500);
				}
			}
			catch(const std::exception& e) {
				const std::string emsg =
					std::string("[portal:apply_pool] ") + e.what();
				return fail_apply(emsg.c_str(), 500);
			}

			Json d;
			d["edit_hp"] = pool.edit_hp;
			d["edit_mana"] = pool.edit_mana;
			d["edit_move"] = pool.edit_move;
			d["edit_hp_regen"] = pool.edit_hp_regen;
			d["edit_mana_regen"] = pool.edit_mana_regen;
			d["edit_move_regen"] = pool.edit_move_regen;
			d["paid_xp"] = xp_cost;
			d["paid_rune"] = rune_cost;
			d["delta"] = delta;
			d["free_edit"] = free_edit ? 1 : 0;
			d["edit_pool_migrated"] = 1;
			return ok_apply(d);
		}

		if(path == "/internal/apply-resistance") {
			const unsigned long long target_toon_id = parse_json_ull(req, "target_toon_id");
			const unsigned damage_type =
				static_cast<unsigned>(parse_json_int(req, "damage_type", 0));
			const int value = parse_json_int(req, "value", 0);
			const int pay_xp = parse_json_int(req, "pay_xp", 0);
			const int pay_rune = parse_json_int(req, "pay_rune", 0);

			const std::string target_name = toon_name_by_id(target_toon_id);

			auto fail_apply = [&](const char* msg, int code) -> std::string {
				return portal_audit_fail(req, msg, code, target_toon_id,
										 target_name.c_str(), 0);
			};
			auto ok_apply = [&](Json d) -> std::string {
				portal_audit_event(req, "apply.resistance", "info", "apply.resistance", &d,
								   target_toon_id, target_name.c_str(), 0, 0);
				return json_ok(d);
			};
			if(target_name.empty()) {
				return fail_apply("toon target non trovato", 404);
			}
			if(is_toon_online(target_toon_id, target_name)) {
				return fail_apply("il toon target e collegato al mud", 409);
			}
			if(damage_type == 0) {
				return fail_apply("damage_type (bit IMM_*) richiesto", 400);
			}
			if(!edit_system_resistance_enabled(damage_type)) {
				return fail_apply("resistenza disabilitata in edit_system.json", 400);
			}
			if(edit_system_resistance_target(damage_type) != EditSystemTarget::Character) {
				return fail_apply(
					"resistenza configurata sull'oggetto — usa apply-affect (APPLY_IMMUNE)", 400);
			}

			DB* db = Sql::getMysql();
			if(!db) {
				return fail_apply("database non disponibile", 500);
			}

			const int max_level = max_level_for_toon(target_toon_id);
			std::string pay_err;
			if(!deduct_payment(target_toon_id, pay_xp, pay_rune, max_level, pay_err)) {
				return fail_apply(pay_err.c_str(), 402);
			}

			std::string persist_err;
			if(!legacy_upsert_character_resistance(db, target_toon_id,
												   damage_type, value, persist_err)) {
				return fail_apply(persist_err.c_str(), 500);
			}

			Json d;
			d["damage_type"] = damage_type;
			d["value"] = value;
			d["paid_xp"] = pay_xp;
			d["paid_rune"] = pay_rune;
			return ok_apply(d);
		}

		if(path == "/internal/list-resistances") {
			const unsigned long long toon_id = parse_json_toon_id(req);
			DB* db = Sql::getMysql();
			if(!db || toon_id == 0) {
				return json_error("toon_id richiesto", 400);
			}
			Json rows = Json::array();
			try {
				odb::connection_ptr cp(db->connection());
				auto& mc = static_cast<odb::mysql::connection&>(*cp);
				MYSQL* h = mc.handle();
				const std::string sql =
					"SELECT damage_type, value FROM character_resistance WHERE toon_id = " +
					std::to_string(toon_id) + " ORDER BY damage_type";
				if(mysql_query(h, sql.c_str()) == 0) {
					MYSQL_RES* res = mysql_store_result(h);
					if(res) {
						MYSQL_ROW row;
						while((row = mysql_fetch_row(res))) {
							Json r;
							r["damage_type"] = row[0] ? std::atoi(row[0]) : 0;
							r["value"] = row[1] ? std::atoi(row[1]) : 0;
							rows.push_back(r);
						}
						mysql_free_result(res);
					}
				}
			}
			catch(...) {
				return json_error("lettura character_resistance fallita", 500);
			}
			Json d;
			d["rows"] = rows;
			return json_ok(d);
		}

		if(path == "/internal/get-system-config") {
			Json d;
			d["path"] = edit_system_config_path();
			try {
				d["config"] = Json::parse(edit_system_config_to_json());
			}
			catch(...) {
				d["config"] = Json::object();
			}
			return json_ok(d);
		}

		if(path == "/internal/set-system-config") {
			if(req.find("config") == req.end()) {
				return json_error("config JSON richiesto", 400);
			}
			std::string err;
			const std::string text = req["config"].dump(2);
			if(!edit_system_config_save_json(text, err)) {
				return json_error(err.c_str(), 400);
			}
			Json d;
			d["path"] = edit_system_config_path();
			d["config"] = req["config"];
			return json_ok(d);
		}

		if(path == "/internal/edit-capabilities" || path == "/internal/listino-status") {
			/* Revisione della cache gia' in RAM (boot o /internal/listino-reload).
			 * Nessun controllo periodico: il ricarico e' solo esplicito. */
			Json d;
			d["reloaded"] = false;
			d["ready"] = portal_listino_ready();
			d["affect_count"] = portal_listino_affect_count();
			d["seed_tag"] = portal_listino_seed_tag();
			d["broker_fee_percent"] =
				portal_listino_param("broker_fee_percent", 25.0);
			d["artifact_mult"] = portal_listino_param("artifact_mult", 1.5);
			d["artifact_cost_raw"] =
				portal_listino_param("artifact_cost_raw", 10000.0);
			d["artifact_cost_xp"] =
				portal_listino_param("artifact_cost_xp", 100000000.0);
			d["artifact_cost_mult"] = "class_mult";
			d["listino_source"] = "edit_listino";
			d["pq_per_mega_xp"] = kEditPoolPqPerMegaXp;
			d["listino_revision"] = portal_listino_seed_tag();

			/* Manifest tender: solo questi possono pagare. Mix sempre valuta+rune. */
			Json tenders = Json::array();
			{
				Json xp;
				xp["id"] = "xp";
				xp["slug"] = "mxp";
				xp["label"] = "MXP";
				xp["api_field"] = "pay_xp";
				xp["is_rune"] = false;
				xp["can_pay"] = true;
				xp["mix_with_rune"] = true;
				/* object: rune coprono XP a 1 MXP = 1 rune (= pq_per_mega_xp raw). */
				xp["object_mix"] = "cover_xp";
				xp["rune_per_mega_xp"] = kEditPoolPqPerMegaXp;
				/* editpool: % sullo listino XP e % sullo listino rune (paralleli). */
				xp["editpool_mix"] = "parallel_pct";
				tenders.push_back(xp);
			}
			{
				Json rn;
				rn["id"] = "rune";
				rn["slug"] = "rune";
				rn["label"] = "Rune degli eroi";
				rn["api_field"] = "pay_rune";
				rn["is_rune"] = true;
				rn["can_pay"] = true;
				rn["mix_with_rune"] = false;
				tenders.push_back(rn);
			}
			d["tenders"] = tenders;

			Json routes = Json::array();
			auto add_route = [&](const char* kind, const char* path_s, const char* pay_model) {
				Json r;
				r["kind"] = kind;
				r["path"] = path_s;
				r["payment_model"] = pay_model;
				routes.push_back(r);
			};
			add_route("pool", "/internal/apply-pool", "editpool");
			add_route("object_affect", "/internal/apply-affect", "object");
			add_route("resistance", "/internal/apply-resistance", "object");
			d["apply_routes"] = routes;
			d["supported_pay_fields"] = Json::array({"pay_xp", "pay_rune"});
			if(path == "/internal/listino-status") {
				/* Supporto MUD per riga: il portale non permette enabled=1 sulle
				 * voci che questo MUD non sa ancora usare. */
				Json rows = Json::object();
				for(const auto& st : portal_listino_row_status()) {
					Json r;
					r["kind"] = st.kind;
					r["enabled"] = st.enabled;
					r["supported"] = st.supported;
					r["active"] = st.active;
					r["reason"] = st.reason;
					rows[st.code] = r;
				}
				d["rows"] = rows;
			}
			return json_ok(d);
		}

		if(path == "/internal/listino-reload") {
			if(!portal_listino_reload()) {
				return json_error("reload listino fallito", 500);
			}
			Json d;
			d["ready"] = portal_listino_ready();
			d["affect_count"] = portal_listino_affect_count();
			d["seed_tag"] = portal_listino_seed_tag();
			return json_ok(d);
		}

		if(path == "/internal/lab-set-stats") {
			std::string lab_err;
			if(!lab_actor_allowed(req, lab_err)) {
				return json_error(lab_err.c_str(), 403);
			}
			const unsigned long long target_toon_id =
				parse_json_ull(req, "target_toon_id");
			std::string target_name;
			if(!lab_require_offline_target(target_toon_id, target_name, lab_err)) {
				const int code =
					lab_err.find("collegato") != std::string::npos ? 409 : 404;
				return portal_audit_fail(req, lab_err.c_str(), code, target_toon_id,
										 target_name.c_str());
			}
			int exp = 0;
			int rune = 0;
			if(!load_stats_for_toon(target_toon_id, exp, rune)) {
				return portal_audit_fail(req, "character_stats non trovato", 404,
										 target_toon_id, target_name.c_str());
			}
			long long new_exp = exp;
			long long new_rune = rune;
			const bool has_exp = req.find("exp") != req.end() && !req["exp"].is_null();
			const bool has_exp_delta =
				req.find("exp_delta") != req.end() && !req["exp_delta"].is_null();
			const bool has_rune =
				req.find("rune") != req.end() && !req["rune"].is_null();
			const bool has_rune_delta =
				req.find("rune_delta") != req.end() && !req["rune_delta"].is_null();
			if(!has_exp && !has_exp_delta && !has_rune && !has_rune_delta) {
				return portal_audit_fail(
					req, "specificare exp/exp_delta e/o rune/rune_delta", 400,
					target_toon_id, target_name.c_str());
			}
			if(has_exp && has_exp_delta) {
				return portal_audit_fail(req, "non usare exp e exp_delta insieme", 400,
										 target_toon_id, target_name.c_str());
			}
			if(has_rune && has_rune_delta) {
				return portal_audit_fail(req, "non usare rune e rune_delta insieme",
										 400, target_toon_id, target_name.c_str());
			}
			if(has_exp) {
				new_exp = parse_json_int(req, "exp", exp);
			}
			else if(has_exp_delta) {
				new_exp = static_cast<long long>(exp) +
						  parse_json_int(req, "exp_delta", 0);
			}
			if(has_rune) {
				new_rune = parse_json_int(req, "rune", rune);
			}
			else if(has_rune_delta) {
				new_rune = static_cast<long long>(rune) +
						   parse_json_int(req, "rune_delta", 0);
			}
			if(new_exp < 0 || new_exp > kLabExpCap) {
				std::ostringstream er;
				er << "exp fuori range: richiesto " << new_exp
				   << ", ammesso 0.." << kLabExpCap
				   << " (massimo " << kLabExpCap << ")";
				const std::string er_msg = er.str();
				return portal_audit_fail(req, er_msg.c_str(), 400,
										 target_toon_id, target_name.c_str());
			}
			if(new_rune < 0 || new_rune > kLabExpCap) {
				std::ostringstream er;
				er << "rune fuori range: richiesto " << new_rune
				   << ", ammesso 0.." << kLabExpCap
				   << " (massimo " << kLabExpCap << ")";
				const std::string er_msg = er.str();
				return portal_audit_fail(req, er_msg.c_str(), 400,
										 target_toon_id, target_name.c_str());
			}
			DB* db = Sql::getMysql();
			if(!db) {
				return portal_audit_fail(req, "database non disponibile", 500,
										 target_toon_id, target_name.c_str());
			}
			std::ostringstream upd;
			upd << "UPDATE character_stats SET exp = " << new_exp
				<< ", p_rune_dei = " << new_rune << " WHERE toon_id = "
				<< target_toon_id;
			std::string sql_err;
			if(!portal_mysql_exec(db, upd.str(), sql_err, "lab:set_stats")) {
				return portal_audit_fail(req, sql_err.c_str(), 500, target_toon_id,
										 target_name.c_str());
			}
			Json d;
			d["exp"] = static_cast<int>(new_exp);
			d["rune"] = static_cast<int>(new_rune);
			d["prev_exp"] = exp;
			d["prev_rune"] = rune;
			d["target_toon_id"] = target_toon_id;
			d["target_name"] = target_name;
			portal_audit_event(req, "lab.set_stats", "info", "lab.set_stats", &d,
							   target_toon_id, target_name.c_str());
			return json_ok(d);
		}

		if(path == "/internal/lab-search-objects") {
			std::string lab_err;
			if(!lab_actor_allowed(req, lab_err)) {
				return json_error(lab_err.c_str(), 403);
			}
			std::string q = req.value("q", "");
			while(!q.empty() && (q.front() == ' ' || q.front() == '\t')) {
				q.erase(q.begin());
			}
			while(!q.empty() && (q.back() == ' ' || q.back() == '\t')) {
				q.pop_back();
			}
			if(q.size() < 2) {
				return json_error("digita almeno 2 caratteri", 400);
			}
			std::string qlow = q;
			for(char& c : qlow) {
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}
			auto contains_ci = [](const char* hay, const std::string& needle) -> bool {
				if(!hay || !*hay || needle.empty()) {
					return false;
				}
				std::string h = hay;
				for(char& c : h) {
					c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				}
				return h.find(needle) != std::string::npos;
			};
			Json results = Json::array();
			constexpr int kLabSearchMax = 40;
			for(int i = 0; i <= top_of_objt && static_cast<int>(results.size()) < kLabSearchMax;
				++i) {
				const char* nm = obj_index[i].name;
				const char* sd = obj_index[i].short_desc;
				if(!contains_ci(nm, qlow) && !contains_ci(sd, qlow)) {
					continue;
				}
				Json row;
				row["vnum"] = obj_index[i].iVNum;
				row["name"] = nm ? nm : "";
				row["short_desc"] = sd ? sd : "";
				results.push_back(row);
			}
			Json d;
			d["q"] = q;
			d["results"] = results;
			return json_ok(d);
		}

		if(path == "/internal/lab-give-vnum") {
			std::string lab_err;
			if(!lab_actor_allowed(req, lab_err)) {
				return json_error(lab_err.c_str(), 403);
			}
			const unsigned long long target_toon_id =
				parse_json_ull(req, "target_toon_id");
			std::string target_name;
			if(!lab_require_offline_target(target_toon_id, target_name, lab_err)) {
				const int code =
					lab_err.find("collegato") != std::string::npos ? 409 : 404;
				return portal_audit_fail(req, lab_err.c_str(), code, target_toon_id,
										 target_name.c_str());
			}
			const int vnum = parse_json_int(req, "vnum", 0);
			int qty = parse_json_int(req, "qty", 1);
			if(vnum <= 0 || real_object(vnum) < 0) {
				return portal_audit_fail(req, "vnum oggetto non valido", 400,
										 target_toon_id, target_name.c_str());
			}
			if(qty < 1) {
				qty = 1;
			}
			if(qty > kLabGiveQtyMax) {
				return portal_audit_fail(req, "qty massimo 20", 400, target_toon_id,
										 target_name.c_str());
			}
			Json created = Json::array();
			for(int i = 0; i < qty; ++i) {
				struct obj_data* obj = read_object(vnum, VIRTUAL);
				if(!obj) {
					return portal_audit_fail(req, "read_object fallito", 500,
											 target_toon_id, target_name.c_str());
				}
				/*
				 * Prototipo grezzo (es. trash/other o RARO) resterebbe nascosto
				 * dalla lista inventario del portale. Come lab-clone: personalizza
				 * sul target, crea object_instance e poi inserisce in inventario.
				 */
				obj->db_instance_id = 0;
				if(obj->char_vnum <= 0) {
					obj->char_vnum = vnum;
				}
				portal_apply_personalize(obj, target_name.c_str());
				if(!IS_OBJ_STAT2(obj, ITEM2_EDIT)) {
					SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
				}
				std::string persist_err;
				if(!portal_persist_object_instance_mysql(obj, vnum,
														 target_name.c_str(),
														 persist_err)) {
					extract_obj(obj);
					return portal_audit_fail(req, persist_err.c_str(), 500,
											 target_toon_id, target_name.c_str());
				}
				unsigned long long inv_id = 0;
				std::string ins_err;
				if(!lab_insert_inventory_obj(target_toon_id, obj, inv_id, ins_err)) {
					extract_obj(obj);
					return portal_audit_fail(req, ins_err.c_str(), 500, target_toon_id,
											 target_name.c_str());
				}
				Json row;
				row["inventory_id"] = inv_id;
				row["vnum"] = vnum;
				row["instance_id"] = obj->db_instance_id;
				row["short_desc"] =
					obj->short_description ? obj->short_description : "";
				created.push_back(row);
				extract_obj(obj);
			}
			Json d;
			d["created"] = created;
			d["qty"] = qty;
			d["vnum"] = vnum;
			d["target_toon_id"] = target_toon_id;
			d["target_name"] = target_name;
			portal_audit_event(req, "lab.give_vnum", "info", "lab.give_vnum", &d,
							   target_toon_id, target_name.c_str());
			return json_ok(d);
		}

		if(path == "/internal/lab-clone-instance") {
			std::string lab_err;
			if(!lab_actor_allowed(req, lab_err)) {
				return json_error(lab_err.c_str(), 403);
			}
			const unsigned long long target_toon_id =
				parse_json_ull(req, "target_toon_id");
			std::string target_name;
			if(!lab_require_offline_target(target_toon_id, target_name, lab_err)) {
				const int code =
					lab_err.find("collegato") != std::string::npos ? 409 : 404;
				return portal_audit_fail(req, lab_err.c_str(), code, target_toon_id,
										 target_name.c_str());
			}
			const unsigned long long source_instance_id =
				parse_json_ull(req, "instance_id");
			if(source_instance_id == 0ULL) {
				return portal_audit_fail(req, "instance_id richiesto", 400,
										 target_toon_id, target_name.c_str());
			}
			struct obj_data* obj = object_instance_materialize(source_instance_id);
			if(!obj) {
				return portal_audit_fail(req, "istanza sorgente non trovata", 404,
										 target_toon_id, target_name.c_str());
			}
			const unsigned long long source_id = obj->db_instance_id;
			const int base = object_instance_resolve_base_vnum(obj);
			obj->db_instance_id = 0;
			if(base > 0) {
				obj->char_vnum = base;
			}
			portal_apply_personalize(obj, target_name.c_str());
			std::string persist_err;
			if(!portal_persist_object_instance_mysql(obj, base > 0 ? base : obj->char_vnum,
													 target_name.c_str(), persist_err)) {
				extract_obj(obj);
				return portal_audit_fail(req, persist_err.c_str(), 500, target_toon_id,
										 target_name.c_str());
			}
			unsigned long long inv_id = 0;
			std::string ins_err;
			if(!lab_insert_inventory_obj(target_toon_id, obj, inv_id, ins_err)) {
				extract_obj(obj);
				return portal_audit_fail(req, ins_err.c_str(), 500, target_toon_id,
										 target_name.c_str());
			}
			Json d;
			d["source_instance_id"] = source_id;
			d["new_instance_id"] = obj->db_instance_id;
			d["inventory_id"] = inv_id;
			d["base_vnum"] = base;
			d["short_desc"] = obj->short_description ? obj->short_description : "";
			d["target_toon_id"] = target_toon_id;
			d["target_name"] = target_name;
			extract_obj(obj);
			portal_audit_event(req, "lab.clone_instance", "info", "lab.clone_instance",
							   &d, target_toon_id, target_name.c_str(), inv_id,
							   d["new_instance_id"].get<unsigned long long>());
			return json_ok(d);
		}

		return json_error("endpoint non trovato", 404);
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "edit_portal: handle_internal exception: %s", e.what());
		return json_error((std::string("[portal:handle] ") + e.what()).c_str(), 500);
	}
}
#else
[[nodiscard]] std::string handle_internal(const std::string&, const std::string&) {
	return json_error("MySQL non attivo in questa build", 503);
}
#endif

[[nodiscard]] std::string submit_job(const std::string& path, const std::string& body) {
	EditPortalJob job;
	job.path = path;
	job.body = body;
	auto fut = job.response.get_future();
	{
		std::lock_guard<std::mutex> lock(g_queue_mutex);
		g_jobs.push_back(std::move(job));
	}
	g_queue_cv.notify_one();
	/* La job la esegue il worker dedicato (fuori dal pulse). */
	if(fut.wait_for(std::chrono::seconds(30)) != std::future_status::ready) {
		return json_error("timeout elaborazione mud", 504);
	}
	return fut.get();
}

void send_all(int client, const std::string& packet) {
	std::size_t off = 0;
	while(off < packet.size()) {
#ifdef MSG_NOSIGNAL
		const ssize_t n = send(client, packet.data() + off, packet.size() - off, MSG_NOSIGNAL);
#else
		const ssize_t n = send(client, packet.data() + off, packet.size() - off, 0);
#endif
		if(n <= 0) {
			return;
		}
		off += static_cast<std::size_t>(n);
	}
}

[[nodiscard]] const char* http_reason(int status) noexcept {
	switch(status) {
	case 200:
		return "OK";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 402:
		return "Payment Required";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 409:
		return "Conflict";
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	case 503:
		return "Service Unavailable";
	case 504:
		return "Gateway Timeout";
	default:
		return "Error";
	}
}

/**
 * Thread HTTP: accetta una connessione alla volta, controlla il segreto e
 * mette la richiesta in coda. Non tocca MAI lo stato di gioco: le job le
 * esegue il worker dedicato. Il socket in ascolto e' gia' aperto da
 * edit_portal_init.
 */
void http_thread_main(int server_fd) {
	while(g_portal_running.load()) {
		const int client = accept(server_fd, nullptr, nullptr);
		if(client < 0) {
			continue;
		}
		/* Un client lento non deve bloccare il thread per sempre. */
		timeval tv {};
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		(void)setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		(void)setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

		const std::string raw = read_http_request(client);
		if(raw.empty()) {
			close(client);
			continue;
		}

		std::string method;
		std::string path;
		{
			std::istringstream ls(raw);
			ls >> method >> path;
		}
		path = normalize_api_path(path);

		const auto hdr_end = raw.find("\r\n\r\n");
		const std::string headers = hdr_end != std::string::npos
										? raw.substr(0, hdr_end)
										: raw;
		std::string body =
			hdr_end != std::string::npos ? raw.substr(hdr_end + 4) : std::string();

		std::string response_body;
		int status = 200;
		if(!header_has_secret(headers)) {
			response_body = json_error("unauthorized", 401);
			status = 401;
		}
		else if(method != "POST") {
			response_body = json_error("solo POST", 405);
			status = 405;
		}
		else {
			response_body = submit_job(path, body);
			if(response_body.find("\"ok\":false") != std::string::npos) {
				const auto cp = response_body.find("\"code\":");
				if(cp != std::string::npos) {
					status = std::atoi(response_body.c_str() + cp + 7);
				}
				if(status < 100 || status > 599) {
					status = 500;
				}
			}
		}

		std::ostringstream out;
		out << "HTTP/1.1 " << status << ' ' << http_reason(status) << "\r\n"
			<< "Content-Type: application/json\r\n"
			<< "Connection: close\r\n"
			<< "Content-Length: " << response_body.size() << "\r\n\r\n"
			<< response_body;
		send_all(client, out.str());
		close(client);
	}
	close(server_fd);
}

/** true se la job puo' aver scritto character_* / inventario (fuori da save_char). */
[[nodiscard]] bool job_may_write_character(const std::string& path) {
	return path.rfind("/internal/apply-", 0) == 0 || path.rfind("/internal/lab-set", 0) == 0 ||
		   path.rfind("/internal/lab-give", 0) == 0 ||
		   path.rfind("/internal/lab-clone", 0) == 0;
}

/**
 * Apre il socket in ascolto (thread di gioco, al boot). -1 se fallisce:
 * l'errore va nel syslog e il MUD parte comunque.
 */
[[nodiscard]] int open_listen_socket(const std::string& bind_addr, int port) {
	sockaddr_in addr {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<uint16_t>(port));
	if(inet_pton(AF_INET, bind_addr.c_str(), &addr.sin_addr) != 1) {
		mudlog(LOG_SYSERR,
			   "edit_portal: EDIT_API_BIND non valido (%s): API portale non avviata",
			   bind_addr.c_str());
		return -1;
	}
	const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0) {
		mudlog(LOG_SYSERR, "edit_portal: socket() fallito (%s): API portale non avviata",
			   std::strerror(errno));
		return -1;
	}
	const int one = 1;
	(void)setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if(bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		mudlog(LOG_SYSERR,
			   "edit_portal: bind(%s:%d) fallito (%s): API portale non avviata",
			   bind_addr.c_str(), port, std::strerror(errno));
		close(server_fd);
		return -1;
	}
	if(listen(server_fd, 16) < 0) {
		mudlog(LOG_SYSERR, "edit_portal: listen(%s:%d) fallito (%s): API portale non avviata",
			   bind_addr.c_str(), port, std::strerror(errno));
		close(server_fd);
		return -1;
	}
	return server_fd;
}

/**
 * Worker dedicato: elabora le richieste HTTP fuori dal pulse di gioco.
 * Una job alla volta (coda seriale). Apply richiede toon offline: il login
 * e' bloccato da edit_portal_session_blocks_login finche' c'e' una sessione
 * o un job in corso.
 */
void portal_worker_main() {
	mudlog(LOG_CHECK, "edit_portal: worker thread avviato (fuori dal pulse)");
	while(g_portal_running) {
		EditPortalJob job;
		{
			std::unique_lock<std::mutex> lock(g_queue_mutex);
			g_queue_cv.wait(lock, [] {
				return !g_jobs.empty() || !g_portal_running.load();
			});
			if(!g_portal_running && g_jobs.empty()) {
				break;
			}
			if(g_jobs.empty()) {
				continue;
			}
			job = std::move(g_jobs.front());
			g_jobs.pop_front();
		}
		std::string response;
		try {
			response = handle_internal(job.path, job.body);
		}
		catch(...) {
			response = json_error("errore interno", 500);
		}
#if USE_MYSQL
		if(job_may_write_character(job.path)) {
			body_digest_invalidate_all(); /* scrive character_* fuori da save_char */
		}
#endif
		try {
			job.response.set_value(std::move(response));
		}
		catch(...) {
			/* promise gia' soddisfatta: niente da fare */
		}
	}
	mudlog(LOG_CHECK, "edit_portal: worker thread terminato");
}

} // namespace

/*
 * Hook sul pulse: non elabora le job (le fa il worker). Se la coda non e'
 * vuota e il worker non e' partito, logga emergenza (wiring errato).
 * Il MUD non chiama mai il portale e non aspetta mai il portale.
 */
void edit_portal_process_pending() {
	if(!g_portal_running.load(std::memory_order_relaxed)) {
		return;
	}
	/* Solo qui (thread di gioco) si leggono le liste live dei PG. */
	refresh_online_snapshot();
	std::size_t pending = 0;
	{
		std::lock_guard<std::mutex> lock(g_queue_mutex);
		pending = g_jobs.size();
	}
	if(pending > 0 && !g_worker_started.load()) {
		mudlog(LOG_SYSERR,
			   "edit_portal: %zu job in coda ma worker non avviato (wiring)",
			   pending);
	}
}

void edit_portal_init() {
#if USE_MYSQL
	g_api_secret = api_secret_from_env();
	if(g_api_secret.empty()) {
		mudlog(LOG_CHECK,
			   "edit_portal: EDIT_API_SECRET non impostato: API portale non avviata "
			   "(il gioco funziona normalmente)");
		return;
	}
	if(g_api_secret == kEditPortalDevSecret) {
		g_api_secret.clear();
		mudlog(LOG_SYSERR,
			   "edit_portal: EDIT_API_SECRET e' il segreto di sviluppo pubblico: "
			   "API portale non avviata (genera un segreto con openssl rand -hex 32)");
		return;
	}
	const int port = api_port();
	if(port < 0) {
		g_api_secret.clear();
		mudlog(LOG_SYSERR, "edit_portal: EDIT_API_PORT non valido: API portale non avviata");
		return;
	}
	const std::string bind_addr = api_bind_from_env();
	const int server_fd = open_listen_socket(bind_addr, port);
	if(server_fd < 0) {
		g_api_secret.clear();
		return;
	}

	edit_system_config_init();
	portal_audit_ensure_tables();
	portal_listino_refresh_revision();

	g_portal_running = true;
	try {
		std::thread(portal_worker_main).detach();
		g_worker_started = true;
		std::thread(http_thread_main, server_fd).detach();
	}
	catch(const std::exception& e) {
		g_portal_running = false;
		g_worker_started = false;
		g_api_secret.clear();
		g_queue_cv.notify_all();
		close(server_fd);
		mudlog(LOG_SYSERR, "edit_portal: thread HTTP/worker non avviato (%s): API portale spenta",
			   e.what());
		return;
	}
	refresh_online_snapshot();
	mudlog(LOG_CHECK, "edit_portal: API in ascolto su %s:%d (%s=%d)", bind_addr.c_str(),
		   port, kEditPortalApiVersionTag, kEditPortalApiVersion);
#else
	mudlog(LOG_CHECK, "edit_portal: disabled (USE_MYSQL=0)");
#endif
}

} // namespace Alarmud
