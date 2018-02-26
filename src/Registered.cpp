/*
 * Registered.cpp
 *
 *  Created on: 25 gen 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#include "Registered.hpp"
#include "utility.hpp"
namespace Alarmud {
Registered::Registered(char* pgname):code(),god() {
	// TODO Auto-generated constructor stub
	LOG_INFO(::boost::format("P4 %s %s %s %s") % "P1" % "P2" % "P3" %"P4");
	mudlog(LOG_CHECK,"P0");
	//mudlog(LOG_CHECK,"P1 %s","P1");
	//mudlog(LOG_CHECK,"P2 %s %s","P1","P2");
	//mudlog(LOG_CHECK,"P3 %s %s %s","P1","P2","P3");
	//mudlog(LOG_CHECK,"P4 %s %s %s %s","P1","P2","P3","P4");

}

Registered::~Registered() {
	// TODO Auto-generated destructor stub
}
}
