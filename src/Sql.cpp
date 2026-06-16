/*
 * Sql.cpp
 *
 *  Created on: 24 mar 2018
 *      Author: giovanni
 */
#include "config.hpp"

/* Expand MYSQL_PORT from config.hpp before libmysql redefines it as int. */
namespace {
const char myst_compile_mysql_port_default[] = MYSQL_PORT;
}

#if USE_MYSQL
#include "odb/account-enum-sync-mysql.hxx"
#endif
#include "Sql.hpp"
#include "autoenums.hpp"
#include "logging.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
using std::cout;
using std::endl;
using std::string;
namespace Alarmud {
bool forceDbInit = false;
odb::session odbSession;
#if USE_MYSQL
namespace {
const char *mysql_cfg(const char *env_key, const char *compile_default) {
  const char *v = std::getenv(env_key);
  if (v && v[0]) {
    return v;
  }
  return compile_default;
}

const char *mysql_cfg_db() {
  const char *v = std::getenv("MYSQL_DB");
  if (v && v[0]) {
    return v;
  }
  v = std::getenv("MYSQL_DATABASE");
  if (v && v[0]) {
    return v;
  }
  return MYSQL_DB;
}

/* libmysql: "localhost" = Unix socket; myst in Docker runs as vagrant (EACCES
 * on mysqld.sock). */
const char *mysql_connect_host() {
  const char *host = mysql_cfg("MYSQL_HOST", MYSQL_HOST);
  if (!host || !host[0] || std::strcmp(host, "localhost") == 0) {
    return "127.0.0.1";
  }
  return host;
}

unsigned int mysql_connect_port() {
  const char *port = mysql_cfg("MYSQL_PORT", myst_compile_mysql_port_default);
  return static_cast<unsigned int>(std::strtoul(port, nullptr, 10));
}
} // namespace
odb::database *Sql::getMysql() {
  thread_local static odb::database *db(new odb::mysql::database(
      mysql_cfg("MYSQL_USER", MYSQL_USER), mysql_cfg("MYSQL_PASSWORD", MYSQL_PASSWORD),
      mysql_cfg_db(), mysql_connect_host(), mysql_connect_port()));
  return db;
}
#endif
#if USE_SQLITE
odb::database *Sql::getSqlite() {
  thread_local static odb::database *db(
      new odb::sqlite::database(MYSQL_DB ".db"));
  return db;
}
#endif
void Sql::dbUpdate() {
  /**
   * generates mysql schema
   */
#if USE_MYSQL
  {
    try {
      DB *db = Sql::getMysql();
      odb::schema_version v = db->schema_version("account");
      odb::schema_version bv(odb::schema_catalog::base_version(*db, "account"));
      odb::schema_version cv(
          odb::schema_catalog::current_version(*db, "account"));
      mudlog(LOG_ALWAYS, "Schema version: %d/%d/%d", v, bv, cv);
      if (v == 0) {
        try {
          odb::transaction t(db->begin());
          t.tracer(odb::stderr_full_tracer);
          odb::schema_catalog::create_schema(*db, "account");
          t.commit();
        } catch (std::exception &e) {
          mudlog(LOG_SYSERR, "DB error: %s", e.what());
          ;
        }
        try {
          odb::transaction t(db->begin());
          t.tracer(odb::stderr_full_tracer);
          db->execute("INSERT INTO legacy SELECT name,realname,email1,email2 "
                      "FROM registered;");
          t.commit();
        } catch (std::exception &e) {
          mudlog(LOG_SYSERR, "DB error: %s", e.what());
        }

        forceDbInit = true; // Initial di loading

      } else if (v >= bv and v < cv) {
        try {
          odb::transaction t(db->begin());
          t.tracer(odb::stderr_full_tracer);
          odb::schema_catalog::migrate(*db, cv, "account");
          t.commit();
        } catch (std::exception &e) {
          mudlog(LOG_SYSERR, "DB error: %s", e.what());
        }
      }
      try {
        odb::transaction t(db->begin());
        t.tracer(odb::stderr_full_tracer);
        odb_enum_sync::sync_mysql_enums(*db);
        t.commit();
      } catch (std::exception &e) {
        mudlog(LOG_SYSERR, "DB enum sync error: %s", e.what());
      }
    } catch (std::exception &e) {
      mudlog(LOG_SYSERR, "DB error: %s", e.what());
      std::cerr << "FATAL: cannot initialize MySQL/ODB schema: " << e.what()
                << std::endl;
      std::exit(1);
    }
  }
#endif
}

void sqlTrace::execute(odb::connection &c, const char *statement) {
  (void)c;
  /* SQL letterale: non passare statement come format-string (% nei .dat/.aux)
   */
  if (statement) {
    mudlog(LOG_QUERY, "%s", statement);
  }
}
sqlTrace logTracer;

Sql::~Sql() {}

Sql::Sql() {}
} /* namespace Alarmud */
