/*
 * registered.hxx
 *
 *  Created on: 22 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_ODB_RENTS_HXX_
#define SRC_ODB_RENTS_HXX_
#include "../config.hpp"
#include <odb/core.hxx>
#include <odb/boost/smart-ptr/lazy-ptr.hxx>
#include <string>
#include <vector>
#include <memory>
namespace Alarmud {
using std::string;
#pragma db object
class item {
public:
#pragma db index
	uint64_t vnum;
#pragma db id
	string name;
};
#pragma db object
class rents {
public:
	rents(){};
	rents(const string &name)
	: name(name)
	{}
	//registered();
#pragma db id
	string name;
#pragma db value_not_null unordered
	std::vector<boost::shared_ptr<item> > items;
};
}




#endif /* SRC_ODB_RENTS_HXX_ */
