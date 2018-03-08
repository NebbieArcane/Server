/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SQL_HPP_
#define __SQL_HPP_
/***************************  System  include ************************************/
#include <vector>
#include <sstream>
#include <boost/type_traits.hpp>
/***************************  Local    include ************************************/
#include "general.hpp"
#include "config.hpp"
#include "flags.hpp"
#if USE_MYSQL
#if USE_SQLITE
#error You need to choose between Mysql and Sqlite
#endif
#endif
#if USE_MYSQL
#if HAS_MYSQL
#include "SqlMysql.hpp"
#else
#error Mysql requested but not available
#endif
#endif
#if USE_SQLITE
#if HAS_SQLITE
#include "SqlSqlite.hpp"
#else
#error Sqlite3 requested but not available
#endif
#endif
namespace Alarmud {
//	MYSQL or sqlite3

class Sql
#if USE_MYSQL
	: public SqlMysql
#endif
#if USE_SQLITE
	  : public SqlSqlite
#endif

{

public:
	const static Sql &getInstance() {
		static Sql instance;
		return instance;
	}
	virtual ~Sql();
protected:
	stringstream _query;
	string _where;
	string _select;
	string _from;
	string _fields;
	string _limit;
public:
	Sql();
	Sql* where(const string field,const string op,const string value);
	Sql* where(const string glue,const string field,const string op,const string value);
	Sql* from(const string table);
	Sql* select(const string field);
	Sql* select(const std::vector<string> fields);
	Sql* limit(const unsigned long limit);
};

} // namespace Alarmud
#endif /* __SQL_HPP_ */

