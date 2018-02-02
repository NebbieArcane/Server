/*
 * Sql.hpp
 *
 *  Created on: 25 gen 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SQL_HPP_
#define SQL_HPP_
#if MYSQL_VERSION
#include <mysql/mysql.h>
#else if SQLITE_VERSION
#include <sqlite3.h>
#endif


namespace nebbie {

class Sql {
private:
    bool disabled;
#if MYSQL_VERSION
    MYSQL *myConn;
#else if SQLITE_VERSION
    sqlite3 *myConn;
#endif

public:
	Sql();
	virtual ~Sql();
};

} /* namespace nebbie */

#endif /* SQL_HPP_ */
