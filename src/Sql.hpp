/*
 * Sql.hpp
 *
 *  Created on: 24 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_SQL_HPP_
#define SRC_SQL_HPP_
#include "odb/odb.hpp"
namespace Alarmud {
class Sql {
public:
#if USE_MYSQL
	const static odb::database* getMysql();
#endif
#if USE_SQLITE
	const static odb::database* getSqlite();
#endif
	void static dbUpdate();
	virtual ~Sql();
protected:
	Sql();

};

} /* namespace Alarmud */

#endif /* SRC_SQL_HPP_ */
