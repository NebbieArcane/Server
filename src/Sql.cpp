/*
 * Sql.cpp
 *
 *  Created on: 25 gen 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#include "Sql.hpp"
#include "config.hpp"
#include <string>

#include "structs.hpp"
#include "utility.hpp"
namespace nebbie {

Sql sql_instance;
Sql* getSql() {
	return (Sql*) &sql_instance;
}

Sql::Sql() : disabled(false) {
#if MYSQL_VERSION
	mysqlConn= mysql_init(NULL);
	MYSQL_RES* mysqlRes;
	MYSQL_ROW mysqlRow;


	if (mysqlConn == NULL) {
		mudlog( LOG_CHECK, "0-----------------> %s\n", mysql_error(mysqlConn));
		disabled=true;
		return;
		//exit(1);
	}

	/* Connect to database */
	if (!mysql_real_connect(mysqlConn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, 0, NULL, 0)) {
		mudlog( LOG_CHECK, "1--------------------> %s\n", mysql_error(mysqlConn));

		//exit(1);
	}
	else {
		mudlog( LOG_CHECK, "CONNESSO");
		/* send SQL query */
		if (mysql_query(mysqlConn, "show tables")) {
			mudlog( LOG_CHECK, "############################### %s\n", mysql_error(mysqlConn));
			//exit(1);
		}

		if(mysqlRes != NULL) {
			mysqlRes = mysql_use_result(mysqlConn);

			/* output table name */
			mudlog( LOG_CHECK, "MySQL Tables in mysql database: [%s]\n", MYSQL_DB);

			while ((mysqlRow = mysql_fetch_row(mysqlRes)) != NULL) {
				mudlog( LOG_CHECK, "%s \n", mysqlRow[0]);
			}
		}
		mysql_free_result(mysqlRes);
		mysql_close(mysqlConn);
	}

#endif
}

Sql::~Sql() {
	// TODO Auto-generated destructor stub
}
} /* namespace nebbie */
