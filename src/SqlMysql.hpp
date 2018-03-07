/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SQLMYSQL_HPP_
#define __SQLMYSQL_HPP_
/***************************  System  include ************************************/
/*
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
*/
namespace sql { class Connection; }
namespace sql { class Driver; }
namespace sql { class PreparedStatement; }
namespace sql { class ResultSet; }
namespace sql { class exception; }
using namespace sql;
/***************************  Local    include ************************************/
namespace Alarmud {
class SqlMysql {
public:
    Driver* driver;
    Connection* db;
	SqlMysql();
	virtual ~SqlMysql();
	const PreparedStatement * prepare(const string query);
	   ResultSet* execute();
	ResultSet* execute(const PreparedStatement* stm);

};
} // namespace Alarmud
#endif /* __SQLMYSQL_HPP_ */

