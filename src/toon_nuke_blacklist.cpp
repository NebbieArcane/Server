/*
 * toon_nuke_blacklist.cpp — audit nuke + gate login.
 */

#include "toon_nuke_blacklist.hpp"

#include "config.hpp"
#include "flags.hpp"
#include "logging.hpp"

#if USE_MYSQL

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>

#include <cctype>
#include <cstring>
#include <string>

#include "Sql.hpp"

namespace Alarmud {
namespace {

unsigned long long nuke_query_scalar_conn(odb::database* db, const std::string& sql) {
	odb::connection_ptr owned;
	odb::connection& conn = odb::transaction::has_current()
		? odb::transaction::current().connection()
		: *(owned = db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(conn);
	MYSQL* h = mc.handle();
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "toon_nuke_blacklist: %s", mysql_error(h));
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

unsigned long long nuke_query_scalar(odb::database* db, const std::string& sql) {
	if(odb::transaction::has_current()) {
		return nuke_query_scalar_conn(db, sql);
	}
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	const unsigned long long n = nuke_query_scalar_conn(db, sql);
	t.commit();
	return n;
}

std::string sql_escape_literal(const char* s) {
	std::string out = "'";
	if(s) {
		for(const char* p = s; *p; ++p) {
			if(*p == '\'') {
				out += "''";
			}
			else if(*p == '\\') {
				out += "\\\\";
			}
			else {
				out += *p;
			}
		}
	}
	out += "'";
	return out;
}

std::string sql_lower_name(const char* name) {
	std::string s(name ? name : "");
	for(char& c : s) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return s;
}

} /* anonymous */

bool toon_nuke_table_exists(odb::database* db) {
	if(!db) {
		return false;
	}
	try {
		const std::string sql =
			"SELECT COUNT(*) FROM information_schema.TABLES "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'toon_nuke_blacklist'";
		return nuke_query_scalar(db, sql) > 0;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_nuke_table_exists: %s", e.what());
		return false;
	}
}

bool toon_nuke_is_blocked(odb::database* db, unsigned long long toon_id, const char* name) {
	if(!db || !toon_nuke_table_exists(db)) {
		return false;
	}
	if(!toon_id && (!name || !*name)) {
		return false;
	}
	try {
		std::string sql = "SELECT COUNT(*) FROM toon_nuke_blacklist WHERE ";
		bool first = true;
		if(toon_id) {
			sql += "toon_id = " + std::to_string(toon_id);
			first = false;
		}
		if(name && *name) {
			if(!first) {
				sql += " OR ";
			}
			sql += "LOWER(toon_name) = " + sql_escape_literal(sql_lower_name(name).c_str());
		}
		return nuke_query_scalar(db, sql) > 0;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_nuke_is_blocked: %s", e.what());
		return false;
	}
}

bool toon_nuke_blacklist_remove(odb::database* db, unsigned long long toon_id, const char* name) {
	if(!db || !toon_nuke_table_exists(db)) {
		return false;
	}
	if(!toon_id && (!name || !*name)) {
		return false;
	}
	if(!toon_nuke_is_blocked(db, toon_id, name)) {
		return false;
	}
	try {
		std::string sql = "DELETE FROM toon_nuke_blacklist WHERE ";
		bool first = true;
		if(toon_id) {
			sql += "toon_id = " + std::to_string(toon_id);
			first = false;
		}
		if(name && *name) {
			if(!first) {
				sql += " OR ";
			}
			sql += "LOWER(toon_name) = " + sql_escape_literal(sql_lower_name(name).c_str());
		}
		if(odb::transaction::has_current()) {
			db->execute(sql.c_str());
		}
		else {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			db->execute(sql.c_str());
			t.commit();
		}
		{
			const std::string logline = std::string("toon_nuke_blacklist: removed toon_id=") +
										std::to_string(toon_id) + " name=" + (name ? name : "");
			mudlog(LOG_PLAYERS, "%s", logline.c_str());
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_nuke_blacklist_remove: %s", e.what());
		return false;
	}
}

bool toon_nuke_blacklist_add(odb::database* db, unsigned long long toon_id, const char* name,
							 const char* nuked_by) {
	if(!db || !toon_id || !name || !*name || !nuked_by || !*nuked_by) {
		return false;
	}
	if(!toon_nuke_table_exists(db)) {
		mudlog(LOG_SYSERR,
			   "toon_nuke_blacklist_add: table missing (apply docs/schema-toon-nuke-blacklist.sql)");
		return false;
	}
	try {
		std::string sql =
			"INSERT INTO toon_nuke_blacklist (toon_id, toon_name, nuked_at, nuked_by) VALUES (";
		sql += std::to_string(toon_id) + ',';
		sql += sql_escape_literal(name) + ",NOW(),";
		sql += sql_escape_literal(nuked_by) + ')';
		sql += " ON DUPLICATE KEY UPDATE toon_name = VALUES(toon_name), "
			   "nuked_at = NOW(), nuked_by = VALUES(nuked_by)";
		if(odb::transaction::has_current()) {
			db->execute(sql.c_str());
		}
		else {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			db->execute(sql.c_str());
			t.commit();
		}
		{
			const std::string logline = std::string("toon_nuke_blacklist: ") + name + " (id=" +
										std::to_string(toon_id) + ") nuked by " + nuked_by;
			mudlog(LOG_PLAYERS, "%s", logline.c_str());
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_nuke_blacklist_add: %s", e.what());
		return false;
	}
}

} /* namespace Alarmud */

#else /* !USE_MYSQL */

namespace Alarmud {

bool toon_nuke_table_exists(odb::database* db) {
	(void)db;
	return false;
}

bool toon_nuke_is_blocked(odb::database* db, unsigned long long toon_id, const char* name) {
	(void)db;
	(void)toon_id;
	(void)name;
	return false;
}

bool toon_nuke_blacklist_add(odb::database* db, unsigned long long toon_id, const char* name,
							 const char* nuked_by) {
	(void)db;
	(void)toon_id;
	(void)name;
	(void)nuked_by;
	return false;
}

bool toon_nuke_blacklist_remove(odb::database* db, unsigned long long toon_id, const char* name) {
	(void)db;
	(void)toon_id;
	(void)name;
	return false;
}

} /* namespace Alarmud */

#endif /* USE_MYSQL */
