/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * SqlMysql.cpp
 *
 *  Created on: 25 feb 2018
 *      Author: giovanni
 */
/***************************  System  include ************************************/

/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "boost/date_time.hpp"
/***************************  Local    include ************************************/
#ifdef MYSQL_VERSION
#include "SqlMysql.hpp"
namespace Alarmud {
constexpr int RETRY=5;
SqlMysql::SqlMysql() : driver(nullptr),db(nullptr) {
}

SqlMysql::~SqlMysql() {
	// TODO Auto-generated destructor stub
}
SqlStatement SqlMysql::prepare(const std::string &query) {
	int retry=5;
	try {
		if(!db) {
			while(--retry) {
				db=driver->connect(MYSQL_HOST,MYSQL_USER,MYSQL_PASSWORD);
			}
		}
		else {
			if(!db->isValid() or db->isClosed()) {
				db->reconnect();
			}
		}
	}
	catch(sql::SQLException &e) {
		mudlog(LOG_ERROR,"Error connecting to db (%d): %s",(RETRY-retry),e.what());
	}
	if(!retry) {
		mudlog(LOG_SYSERR,"Gave up attempting to connect to dabatase");
	}
	try {
		return cache[query];
	}
	catch(std::out_of_range &e) {
		cache[query]=SqlStatement(db->prepareStatement(query));
	}
	return cache[query];
}

ResultSet* SqlMysql::execute() {
}

ResultSet* SqlMysql::execute(const PreparedStatement* stm) {
}
void SqlMysql::pushString(SqlStatement stm,const int index,const std::string &data) {
	stm->setString(index, data);
}
void SqlMysql::pushUInt64(SqlStatement stm,const int index,const uint64_t &data) {
	stm->setUInt64(index, data);
}
void SqlMysql::pushInt64(SqlStatement stm,const int index,const int64_t &data) {
	stm->setInt64(index, data);
}
void SqlMysql::pushUInt(SqlStatement stm,const int index,const uint32_t &data) {
	stm->setUInt(index, data);
}
void SqlMysql::pushInt(SqlStatement stm,const int index,const int32_t &data) {
	stm->setInt(index, data);
}
void SqlMysql::pushDouble(SqlStatement stm,const int index,const double &data) {
	stm->setDouble(index, data);
}
void SqlMysql::push(SqlStatement stm,const int index,const boost::posix_time::ptime &data) {
	if(data.is_not_a_date_time()) {
		try {
			stm->setNull(index, 0);
		}
		catch(sql::SQLException &e) {
			stm->setInt(index, 0);
		}
	}
	boost::posix_time::ptime start(boost::gregorian::date(1970,1,1));
	if(data > start) {
		stm->setUInt64(index,(data-start).total_seconds());
	}
	else {
		// This is so far in the past we are ignoring timezone
		stm->setString(index, boost::posix_time::to_simple_string(data));
	}
}
} // namespace Alarmud
#endif

