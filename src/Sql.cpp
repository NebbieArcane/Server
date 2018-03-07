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
#include "Sql.hpp"
#include "utility.hpp"
namespace Alarmud {

Sql::Sql() :_query(),_where(""),_select(""),_from(""),_fields(""),_limit("") {


}
Sql::~Sql() {
	// TODO Auto-generated destructor stub
}

Sql* Sql::where(const string field, const string op, const string value) {
	_where.append(field).append(" ").append(op).append(" ").append(value);
	return this;
}

Sql* Sql::where(const string glue, const string field, const string op,
		const string value) {
	_where.append(" ").append(glue).append(field).append(" ").append(op).append(" ").append(value);
}

Sql* Sql::from(const string table) {
}

Sql* Sql::select(const string field) {
}

Sql* Sql::select(const std::vector<string> fields) {
}

Sql* Sql::limit(const unsigned long limit) {
}

} // namespace Alarmud

