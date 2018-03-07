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
/***************************  Local    include ************************************/
#include "SqlMysql.hpp"
#ifdef MYSQL_VERSION
namespace Alarmud {
SqlMysql::SqlMysql() {
}

SqlMysql::~SqlMysql() {
	// TODO Auto-generated destructor stub
}
#endif
} // namespace Alarmud

