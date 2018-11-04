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
class OSql
#if USE_MYSQL
	: public SqlMysql
#endif
#if USE_SQLITE
	  : public SqlSqlite
#endif

{

public:
	const static OSql &getInstance() {
		thread_local static OSql instance;
		return instance;
	}
	virtual ~OSql();
protected:
	int index;
	ostringstream _query;
	OSql();
public:
	SqlStatement query(const string queryString);
	OSql* prepare();
	const string query();
	template<typename T>
	T getField(string name);
	template<typename T>
	void push(SqlStatement stm,T data);
	template<typename T, typename... Args>
	OSql* execute(SqlStatement stm, T first, Args... args);

};
OSql* getSql();

} // namespace Alarmud


#endif /* __SQL_HPP_ */

