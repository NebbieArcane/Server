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
namespace Alarmud {
//	MYSQL or sqlite3
class Sql {

public:
	Sql();
	virtual ~Sql();
};

} /* namespace nebbie */

#endif /* SQL_HPP_ */
