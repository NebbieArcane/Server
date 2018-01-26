/*
 * Registered.hpp
 *
 *  Created on: 25 gen 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef REGISTERED_HPP_
#define REGISTERED_HPP_
#include "structs.h"
namespace Nebbie {
class Registered {
public:
	Registered();
	virtual ~Registered();
	int Count() { return 0;}
	void doReg(char *,char *){};
	bool doDel(char *){return false;};
};
Registered* getRegistered();
}
#endif /* REGISTERED_HPP_ */
