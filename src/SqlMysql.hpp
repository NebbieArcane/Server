/*
 * SqlMysql.hpp
 *
 *  Created on: 25 feb 2018
 *      Author: giovanni
 */

#ifndef __SQLMYSQL_HPP_
#define __SQLMYSQL_HPP_
#include "config.hpp"
#include "Sql.hpp"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
namespace Alarmud {
class SqlMysql : public Sql {
public:
    ::sql::Driver* driver;
    ::sql::Connection* db;
	SqlMysql();
	virtual ~SqlMysql();
};
}
#endif /* __SQLMYSQL_HPP_ */
