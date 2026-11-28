/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "edit_portal_lock.hpp"

#include "config.hpp"
#include "flags.hpp"
#include "logging.hpp"

#if USE_MYSQL

#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>

#include <cctype>
#include <cstring>
#include <string>

#include "Sql.hpp"

namespace Alarmud {
namespace {

std::string sql_escape_literal(const char* s) {
	if(s == nullptr) {
		return "''";
	}
	std::string out = "'";
	for(const char* p = s; *p; ++p) {
		if(*p == '\'' || *p == '\\') {
			out.push_back('\\');
		}
		out.push_back(*p);
	}
	out.push_back('\'');
	return out;
}

std::string sql_lower_name_literal(const char* name) {
	if(name == nullptr || *name == '\0') {
		return "''";
	}
	std::string lower;
	lower.reserve(std::strlen(name));
	for(const char* p = name; *p; ++p) {
		lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(*p))));
	}
	return sql_escape_literal(lower.c_str());
}

bool run_sql_void(odb::database* db, const std::string& sql) {
	const bool in_tx = odb::transaction::has_current();
	try {
		odb::connection_ptr owned;
		odb::connection& conn = in_tx ? odb::transaction::current().connection()
									  : *(owned = db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(conn);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql.c_str()) != 0) {
			mudlog(LOG_SYSERR, "edit_portal_lock: %s", mysql_error(h));
			return false;
		}
		/* consume possible result set */
		if(MYSQL_RES* res = mysql_store_result(h)) {
			mysql_free_result(res);
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_portal_lock odb: %s", e.what());
		return false;
	}
}

unsigned long long run_sql_count(odb::database* db, const std::string& sql) {
	const bool in_tx = odb::transaction::has_current();
	try {
		odb::connection_ptr owned;
		odb::connection& conn = in_tx ? odb::transaction::current().connection()
									  : *(owned = db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(conn);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql.c_str()) != 0) {
			mudlog(LOG_SYSERR, "edit_portal_lock: %s", mysql_error(h));
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		unsigned long long n = 0;
		if(res) {
			if(MYSQL_ROW row = mysql_fetch_row(res)) {
				if(row[0]) {
					n = std::strtoull(row[0], nullptr, 10);
				}
			}
			mysql_free_result(res);
		}
		return n;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_portal_lock odb: %s", e.what());
		return 0;
	}
}

bool table_has_rows(odb::database* db, const std::string& sql) {
	if(odb::transaction::has_current()) {
		return run_sql_count(db, sql) > 0ULL;
	}
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	const unsigned long long n = run_sql_count(db, sql);
	t.commit();
	return n > 0ULL;
}

void expire_stale_sessions(odb::database* db) {
	/* TTL scaduto / legacy NULL; non chiudere se job apply ancora in volo. */
	const char* sql =
		"UPDATE edit_portal_session s "
		"SET s.status = 'closed', s.closed_at = CURRENT_TIMESTAMP, "
		"s.summary_json = '{\"reason\":\"ttl_expired_by_myst\"}' "
		"WHERE s.status = 'open' AND "
		"(s.expires_at IS NULL OR s.expires_at < NOW()) AND "
		"NOT EXISTS (SELECT 1 FROM edit_portal_job j WHERE j.session_key = s.session_key "
		"AND j.status IN ('pending','running')) AND "
		"NOT EXISTS (SELECT 1 FROM edit_portal_job j2 WHERE j2.toon_id = s.actor_toon_id "
		"AND j2.status IN ('pending','running'))";
	if(odb::transaction::has_current()) {
		run_sql_void(db, sql);
		return;
	}
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	run_sql_void(db, sql);
	t.commit();
}

[[nodiscard]] bool jobs_block_login(odb::database* db, unsigned long long toon_id) {
	if(toon_id == 0ULL) {
		return false;
	}
	const std::string sql =
		"SELECT COUNT(*) FROM edit_portal_job WHERE toon_id = " +
		std::to_string(toon_id) + " AND status IN ('pending','running') LIMIT 1";
	return table_has_rows(db, sql);
}

} /* namespace */

bool edit_portal_session_blocks_login(odb::database* db, unsigned long long toon_id,
									  const char* toon_name) {
	if(db == nullptr) {
		return false;
	}
	if(toon_id == 0ULL && (toon_name == nullptr || *toon_name == '\0')) {
		return false;
	}

	expire_stale_sessions(db);

	if(jobs_block_login(db, toon_id)) {
		mudlog(LOG_CONNECT,
			   "edit_portal_lock: login blocked for toon_id=%llu name=%s (job pending)",
			   static_cast<unsigned long long>(toon_id),
			   toon_name ? toon_name : "?");
		return true;
	}

	std::string sql =
		"SELECT COUNT(*) FROM edit_portal_session WHERE status = 'open' "
		"AND expires_at IS NOT NULL AND expires_at > NOW() AND (";
	bool any = false;
	if(toon_id != 0ULL) {
		sql += "actor_toon_id = ";
		sql += std::to_string(toon_id);
		any = true;
	}
	if(toon_name != nullptr && *toon_name != '\0') {
		if(any) {
			sql += " OR ";
		}
		sql += "LOWER(actor_toon_name) = ";
		sql += sql_lower_name_literal(toon_name);
		any = true;
	}
	if(!any) {
		return false;
	}
	sql += ") LIMIT 1";

	const bool blocked = table_has_rows(db, sql);
	if(blocked) {
		mudlog(LOG_CONNECT,
			   "edit_portal_lock: login blocked for toon_id=%llu name=%s (session open)",
			   static_cast<unsigned long long>(toon_id),
			   toon_name ? toon_name : "?");
	}
	return blocked;
}

} /* namespace Alarmud */

#else /* !USE_MYSQL */

namespace Alarmud {

bool edit_portal_session_blocks_login(odb::database* /*db*/, unsigned long long /*toon_id*/,
									  const char* /*toon_name*/) {
	return false;
}

} /* namespace Alarmud */

#endif /* USE_MYSQL */
