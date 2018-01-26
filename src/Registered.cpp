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
namespace Nebbie {
Registered registered_instance();
Registered* getRegistered() {
    return &registered_instance;
}

Registered::Registered() {
	// TODO Auto-generated constructor stub

}

Registered::~Registered() {
	// TODO Auto-generated destructor stub
}
}
