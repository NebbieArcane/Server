/*
 * SqlMysql.cpp
 *
 *  Created on: 25 feb 2018
 *      Author: giovanni
 */
#include "config.hpp"
#ifdef MYSQL_VERSION
#include "SqlMysql.hpp"
namespace Alarmud {
SqlMysql::SqlMysql() {
	// TODO Auto-generated constructor stub
	myConn= mysql_init(NULL);
	MYSQL_RES* mysqlRes;
	MYSQL_ROW mysqlRow;


	if (myConn == NULL) {
		LOG_INFO("0-----------------> " <<  mysql_error(myConn));
		return;
		//exit(1);
	}

	/* Connect to database */
	if (!mysql_real_connect(myConn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, 0, NULL, 0)) {
		LOG_INFO( "1--------------------> " << mysql_error(myConn));

		//exit(1);
	}
	else {
		LOG_INFO("CONNESSO");
		/* send SQL query */
		if (mysql_query(myConn, "show tables")) {
			LOG_INFO( "############################### " << mysql_error(myConn));
			//exit(1);
		}

		if(mysqlRes != NULL) {
			mysqlRes = mysql_use_result(myConn);

			/* output table name */
			LOG_INFO("MySQL Tables in mysql database: [" << MYSQL_DB << "]");

			while ((mysqlRow = mysql_fetch_row(mysqlRes)) != NULL) {
				LOG_INFO(mysqlRow[0]);
			}
		}
		mysql_free_result(mysqlRes);
		mysql_close(myConn);
	}

}

SqlMysql::~SqlMysql() {
	// TODO Auto-generated destructor stub
}
}
#endif
