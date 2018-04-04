/*
 * odb.hpp
 *
 *  Created on: 22 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_ODB_ODB_HPP_
#define SRC_ODB_ODB_HPP_
#include "../config.hpp"
#include "../flags.hpp"
#include "../Sql.hpp"
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/session.hxx>
#if USE_MYSQL
#include <odb/mysql/database.hxx>
#endif
#if USE_SQLITE
#include <odb/sqlite/database.hxx>
#endif
#include "account-odb.hxx"
//#include "pg-odb.hxx"
//#include "world-odb.hxx"
#define odbTypes(nome) typedef odb::result<nome> nome##Rows;typedef odb::query<nome> nome##Query
namespace Alarmud {
	odbTypes(user);
	odbTypes(legacy);
	odbTypes(toon);
	odbTypes(toonExtra);
	odbTypes(toonRent);
	odbTypes(toonBank);
	typedef odb::database DB;
} /*namespace Alarmud */
#endif /* SRC_ODB_ODB_HPP_ */
