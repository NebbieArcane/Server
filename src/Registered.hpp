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
#include "general.hpp"
namespace Nebbie {
class Registered {
private:
	string code;
	string god;
public:
	Registered(char* pgname);
	virtual ~Registered();
	bool get() {
		return false;
	}
	bool reg(char*) {
		return true;
	}
	bool del() {
		return false;
	}
	/* Static members which operates at top level */
	static int total() {
		return 0;   // Returns the total number of registered toons
	}

	const string &getCode() const {
		return code;
	}

	const string &getGod() const {
		return god;
	}

	void setGod(const string &godName) {
		god = godName;
	}

	static const unsigned short REG_PC		=0;
	static const unsigned short REG_DIO    =1;
	static const unsigned short REG_CODICE =2;
	static const unsigned short REG_NOME   =3;
	static const unsigned short REG_EMAIL  =4;
	static const unsigned short REG_ALTRO  =5;
	static const unsigned short REG_MAX    =6;
	static const unsigned short REG_LAST   =5;
	static const unsigned short REG_CODELEN=6;
	static const unsigned short NEW 		=1;
	static const unsigned short ADD 		=2;
	static const unsigned short CHG 		=3;

};
}
#endif /* REGISTERED_HPP_ */
