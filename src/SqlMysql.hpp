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
#include <mysql/mysql.h>
namespace Alarmud {
class SqlMysql : public Sql {
public:
	MYSQL* myConn;
	SqlMysql();
	virtual ~SqlMysql();
};
}
#endif /* __SQLMYSQL_HPP_ */
