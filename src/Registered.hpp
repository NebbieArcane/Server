/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef REGISTERED_HPP_
#define REGISTERED_HPP_
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "general.hpp"
namespace Alarmud {
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
	static unsigned long total() {
		return 0UL;   // Returns the total number of registered toons
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

