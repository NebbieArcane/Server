/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * Sql.cpp
 *
 *  Created on: 25 gen 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */
/***************************  System  include ************************************/
#include "OSql.hpp"

#include <string>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "utility.hpp"
namespace Alarmud {
typedef std::vector<string> tfields;

OSql::OSql() : index(0),_query(),
#if USE_MYSQL
	SqlMysql()
#endif
#if USE_SQLITE
	SqlSqlite()
#endif
		{

}
OSql::~OSql() {
	// TODO Auto-generated destructor stub
}

OSql* OSql::where(const string field, const string op, const string value) {
	_query << " WHERE " << field << " " << op << " " << value;
	return this;
}

OSql* OSql::where(const string glue, const string field, const string op,
				const string value) {
	_query << " " << glue << " " << field << " " << op << value;
	return this;
}

OSql* OSql::from(const string table) {
	_query << " FROM " << table;
	return this;
}

OSql* OSql::select(const string field) {
	_query.str(string());
	_query << "SELECT " << field;
	return this;
}

OSql* OSql::select(const tfields fields) {
	_query.str(string());
	_query << "SELECT ";
	for (tfields::const_iterator it=fields.begin();it!=fields.end();++it) {
		if (it!=fields.begin()) {
			_query << ",";
		}
		_query << it->data();
	}
	return this;
}

OSql* OSql::limit(const unsigned long limit) {
	_query << " LIMIT " << std::dec << limit;
	return this;
}

OSql *OSql::prepare() {
}

const string OSql::query() {
	return _query.str();
}

OSql* OSql::openParens() {
	_query << "(";
	return this;
}
OSql* OSql::closeParens() {
	_query << ")";
	return this;
}
template<typename T>
void OSql::push(SqlStatement stm,T data) {
	if (boost::is_arithmetic<T>()) {
		if (boost::is_signed<T>())  {
			if (boost::is_convertible<T,int32_t>()) {
				stm->setInt(index,data);
			}
			else {
				stm->setInt64(index,data);
			}
		}
		else {
			if (boost::is_convertible<T,uint32_t>()) {
				stm->setUInt(index,data);
			}
			else {
				stm->setUInt64(index,data);
			}

		}
		return;
	}
}
/*
template<typename T, typename ... Args>
	OSql* OSql::execute(SqlStatement stm, T first, Args ... args) {
	push(stm,index,first);
	execute(stm,index,args...);
	return this;
}
*/
OSql* getOSql() {
	return &OSql::getInstance();
}

} // namespace Alarmud

