/*
 * Sql.cpp
 *
 *  Created on: 24 mar 2018
 *      Author: giovanni
 */
#include "Sql.hpp"
#include <iostream>
#include "autoenums.hpp"
#include "logging.hpp"
#include <mutex>
#include <memory>
#include <cstring>
#include <cstdlib>
using std::string;
using std::endl;
using std::cout;
namespace Alarmud {
bool forceDbInit=false;
odb::session odbSession;
#if USE_MYSQL
namespace {
/* libmysql: "localhost" = Unix socket; myst in Docker runs as vagrant (EACCES on mysqld.sock). */
const char* mysql_connect_host() {
	const char* host = MYSQL_HOST;
	if(!host || !host[0] || std::strcmp(host, "localhost") == 0) {
		return "127.0.0.1";
	}
	return host;
}
}
odb::database* Sql::getMysql() {
	thread_local static odb::database* db(
		new odb::mysql::database(MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, mysql_connect_host()));
	return db;
}
#endif
#if USE_SQLITE
odb::database* Sql::getSqlite() {
	thread_local static odb::database* db(new odb::sqlite::database(MYSQL_DB ".db"));
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
			DB* db=Sql::getMysql();
			odb::schema_version v = db->schema_version("account");
			odb::schema_version bv(odb::schema_catalog::base_version(*db,"account"));
			odb::schema_version cv(odb::schema_catalog::current_version(*db,"account"));
			mudlog(LOG_ALWAYS,"Schema version: %d/%d/%d",v,bv,cv);
			if(v==0) {
				try {
					odb::transaction t(db->begin());
					t.tracer(odb::stderr_full_tracer);
					odb::schema_catalog::create_schema(*db,"account");
					t.commit();
				}
				catch(std::exception &e) {
					mudlog(LOG_SYSERR,"DB error: %s",e.what());;
				}
				try {
					odb::transaction t(db->begin());
					t.tracer(odb::stderr_full_tracer);
					db->execute("INSERT INTO legacy SELECT name,realname,email1,email2 FROM registered;");
					t.commit();
				}
				catch(std::exception &e) {
					mudlog(LOG_SYSERR,"DB error: %s",e.what());
				}

				forceDbInit=true; // Initial di loading

			}
			else if(v>=bv and v < cv) {
				try {
					odb::transaction t(db->begin());
					t.tracer(odb::stderr_full_tracer);
					odb::schema_catalog::migrate(*db,cv,"account");
					t.commit();
				}
				catch(std::exception &e) {
					mudlog(LOG_SYSERR,"DB error: %s",e.what());
				}
			}
		}
		catch(std::exception &e) {
			mudlog(LOG_SYSERR,"DB error: %s",e.what());
			std::cerr << "FATAL: cannot initialize MySQL/ODB schema: " << e.what() << std::endl;
			std::exit(1);
		}
	}
#endif
}

void sqlTrace::execute(odb::connection &c, const char* statement) {
	mudlog(LOG_QUERY,statement);
}
sqlTrace logTracer;

Sql::~Sql() {
}

Sql::Sql() {
}
} /* namespace Alarmud */
