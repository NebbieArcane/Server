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
#include <vector>
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
#define odbTypes(nome) \
	typedef odb::result<nome> nome##Result; \
	typedef odb::query<nome> nome##Query; \
    typedef boost::shared_ptr<nome> nome##Ptr; \
	typedef std::vector<nome> nome##Rows

namespace Alarmud {
odbTypes(user);
odbTypes(legacy);
odbTypes(toon);
odbTypes(toonExtra);
odbTypes(toonRent);
typedef odb::database DB;
} /*namespace Alarmud */
#include "../Sql.hpp"
#endif /* SRC_ODB_ODB_HPP_ */
