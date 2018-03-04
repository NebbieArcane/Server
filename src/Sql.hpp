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
#include "config.hpp"
#include "general.hpp"
#include <vector>
#include <strstream>
#include <boost/type_traits.hpp>

namespace Alarmud {
//	MYSQL or sqlite3
class Sql {

public:
	static Sql& getInstance() {
		static Sql instance;
		return instance;
	}
private:
	Sql():_query(""),_where(""),_select(""),_from(""),_fields(""),_limit(""){}
	virtual ~Sql();
protected:
	std::strstream _query;
	string _where;
	string _select;
	string _from;
	string _fields;
	string _limit;
public:
virtual	Sql* where(const string field,const string op,const string value);
virtual	Sql* where(const string glue,const string field,const string op,const string value);
virtual	Sql* from(const string table);
virtual	Sql* select(const string field);
virtual	Sql* select(const std::vector<string> fields);
virtual	Sql* limit(const unsigned long limit);
template<typename T>
virtual string escape(T value);
};

} /* namespace nebbie */

template<typename T>
inline string Alarmud::Sql::escape(T value) {
}

#endif /* SQL_HPP_ */
