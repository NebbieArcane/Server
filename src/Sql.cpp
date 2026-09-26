/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* Sql.cpp - accesso MySQL/SQLite via ODB.
 * Created on: 24 mar 2018
 * Author: giovanni
 * */
#include "config.hpp"

/* Expand MYSQL_PORT from config.hpp before libmysql redefines it as int. */
namespace {
const char myst_compile_mysql_port_default[] = MYSQL_PORT;
}

#include "Sql.hpp"
#if USE_MYSQL
#include "odb/account-enum-sync-mysql.hxx"
#include "odb_schema_heal.hpp"
#include "character_item_loss.hpp"
#include <odb/mysql/connection-factory.hxx>
#include <odb/mysql/exceptions.hxx>
#include <odb/details/shared-ptr/base.hxx>
#include <mysql/mysql.h>
#include <new>
#endif
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

unsigned int mysql_timeout_sec(const char *env_key, unsigned int fallback) {
  const char *v = std::getenv(env_key);
  if (!v || !v[0]) {
    return fallback;
  }
  char *end = nullptr;
  const unsigned long parsed = std::strtoul(v, &end, 10);
  if (end == v || parsed == 0 || parsed > 600) {
    return fallback;
  }
  return static_cast<unsigned int>(parsed);
}

/* Client-side timeouts so a stuck MySQL call cannot freeze the game loop for
 * minutes (see Gogeta save hang). Defaults: connect 10s, read/write 30s.
 * Override with MYSQL_CONNECT_TIMEOUT_SEC / MYSQL_READ_TIMEOUT_SEC /
 * MYSQL_WRITE_TIMEOUT_SEC. */
class timeout_connection_factory : public odb::mysql::connection_pool_factory {
public:
  timeout_connection_factory()
      : odb::mysql::connection_pool_factory(/*max*/ 0, /*min*/ 0, /*ping*/ true) {}

protected:
  pooled_connection_ptr create() override {
    MYSQL *h = ::mysql_init(nullptr);
    if (!h) {
      throw std::bad_alloc();
    }

    const unsigned int connect_to =
        mysql_timeout_sec("MYSQL_CONNECT_TIMEOUT_SEC", 10);
    const unsigned int read_to =
        mysql_timeout_sec("MYSQL_READ_TIMEOUT_SEC", 30);
    const unsigned int write_to =
        mysql_timeout_sec("MYSQL_WRITE_TIMEOUT_SEC", 30);
    ::mysql_options(h, MYSQL_OPT_CONNECT_TIMEOUT, &connect_to);
    ::mysql_options(h, MYSQL_OPT_READ_TIMEOUT, &read_to);
    ::mysql_options(h, MYSQL_OPT_WRITE_TIMEOUT, &write_to);

    odb::mysql::database &db = odb::mysql::connection_factory::database();
    if (db.charset() && db.charset()[0] != '\0') {
      ::mysql_options(h, MYSQL_SET_CHARSET_NAME, db.charset());
    }

    if (::mysql_real_connect(h, db.host(), db.user(), db.password(), db.db(),
                             db.port(), db.socket(),
                             db.client_flags() | CLIENT_FOUND_ROWS) ==
        nullptr) {
      const unsigned int err = ::mysql_errno(h);
      const std::string sqlstate =
          ::mysql_sqlstate(h) ? ::mysql_sqlstate(h) : "?????";
      const std::string message =
          ::mysql_error(h) ? ::mysql_error(h) : "mysql_real_connect failed";
      ::mysql_close(h);
      throw odb::mysql::database_exception(err, sqlstate, message);
    }

    return pooled_connection_ptr(
        new (odb::details::shared) pooled_connection(*this, h));
  }
};

odb::mysql::database *make_mysql_database() {
  const unsigned int connect_to =
      mysql_timeout_sec("MYSQL_CONNECT_TIMEOUT_SEC", 10);
  const unsigned int read_to = mysql_timeout_sec("MYSQL_READ_TIMEOUT_SEC", 30);
  const unsigned int write_to =
      mysql_timeout_sec("MYSQL_WRITE_TIMEOUT_SEC", 30);
  mudlog(LOG_ALWAYS,
         "MySQL client timeouts: connect=%us read=%us write=%us", connect_to,
         read_to, write_to);
  std::unique_ptr<odb::mysql::connection_factory> factory(
      new timeout_connection_factory());
  return new odb::mysql::database(
      mysql_cfg("MYSQL_USER", MYSQL_USER),
      mysql_cfg("MYSQL_PASSWORD", MYSQL_PASSWORD), mysql_cfg_db(),
      mysql_connect_host(), mysql_connect_port(),
      /*socket*/ static_cast<const char *>(nullptr),
      /*charset*/ static_cast<const char *>(nullptr),
      /*client_flags*/ 0ul, std::move(factory));
}
} // namespace
odb::database *Sql::getMysql() {
  thread_local static odb::database *db(make_mysql_database());
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
          mudlog(LOG_ALWAYS,
                 "schema migrate failed — running idempotent heal toward %llu",
                 static_cast<unsigned long long>(cv));
        }
      }
      /* Catch-up idempotente anche se migrate e' ok o version gia' == cv
       * (es. contatore avanti ma tabella assente, o DDL a meta'). */
      if (v > 0) {
        try {
          const odb::schema_version healed = account_schema_heal(db, cv);
          if (healed != v) {
            mudlog(LOG_ALWAYS, "Schema version after heal: %llu (was %llu, target %llu)",
                   static_cast<unsigned long long>(healed),
                   static_cast<unsigned long long>(v),
                   static_cast<unsigned long long>(cv));
          }
        } catch (std::exception &e) {
          mudlog(LOG_SYSERR, "schema heal error: %s", e.what());
          std::cerr << "FATAL: cannot heal MySQL/ODB schema: " << e.what()
                    << std::endl;
          std::exit(1);
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
      character_item_loss_purge_old();
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
