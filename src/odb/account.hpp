/*
 * registered.hxx
 *
 *  Created on: 22 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_ODB_ACCOUNT_HXX_
#define SRC_ODB_ACCOUNT_HXX_
//#include <boost/smart_ptr.hpp>
#include <boost/date_time.hpp>
//#include <odb/boost/date-time/exceptions.hxx>
//#include <odb/boost/date-time/mysql/posix-time-mapping.hxx>
#include <odb/core.hxx>
#include <odb/vector.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/boost/lazy-ptr.hxx>
#include <odb/nullable.hxx>
#include <string>
#include "../utility.hpp"

namespace Alarmud {
using std::string;
class toonExtra;
class toonRent;
class toonBank;
class user;
class legacy;
class toon;
class toonExtra {
public:
	string name;
	string classes;
};
class toonRent {
public:
	string name;
	unsigned long vnum;
};
class toonBank {
public:
	string name;
	unsigned long vnum;
};
class toon {
public:
	typedef odb::boost::lazy_shared_ptr<toonExtra> extraPtr;
	typedef odb::boost::lazy_shared_ptr<toonRent> rentPtr;
	typedef odb::vector<rentPtr> rentVector;
	typedef odb::boost::lazy_shared_ptr<toonBank> bankPtr;
	typedef odb::vector<bankPtr> bankVector;
	toon(){};
	toon(const char* name,const char* password) :name(name),password(password),title(""),lastlogin(boost::posix_time::not_a_date_time),owner_id(0){}
	string name;
	string password;
	string title;
	boost::posix_time::ptime lastlogin;
	unsigned long long owner_id;
	extraPtr data;
	rentVector rentItems;
	bankVector bankItems;
};
class user {
public:
	typedef odb::boost::lazy_shared_ptr<toon> toonPtr;
	typedef odb::vector<toonPtr> toonVector;
	unsigned long long id;
	string login;
	string nickname;
	string email;
	boost::posix_time::ptime registered;
	string password;
	unsigned short level;
	odb::nullable<string> backup_email;
	toonVector toons;
	string choosen;
	bool authorized;
	user(){};
	user(const string &login,const string &nickname,const string& email,const string& password=""):
		login(login),
		nickname(nickname),
		email(email),
		registered(boost::posix_time::from_time_t(time(nullptr))),
		password(password),
		level(0),
		backup_email(),
		choosen(""),
		authorized(false)
	{}
};
class legacy {
public:
	string name;
	string realname;
	string email1;
	string email2;
};
#ifdef ODB_COMPILER
#pragma db model version(1,1,open)

#pragma db object(toon)
#pragma db member(toon::name) id type("varchar(32)")
#pragma db member(toon::password) type("varchar(128)")
#pragma db member(toon::title) type("varchar(128)")
#pragma db member(toon::lastlogin) type("DATETIME") null
#pragma db member(toon::owner_id) index
#pragma db member(toon::data) value_not_null inverse(name)
#pragma db member(toon::rentItems) value_not_null inverse(name)
#pragma db member(toon::bankItems) value_not_null inverse(name)

#pragma db object(toonExtra)
#pragma db member(toonExtra::name) id type("varchar(32)")

#pragma db object(toonBank)
#pragma db member(toonBank::name) id type("varchar(32)")

#pragma db object(toonRent)
#pragma db member(toonRent::name) id type("varchar(32)")

//login,nickname,email,registered,password,level

#pragma db object(user)
#pragma db member(user::id) id auto
#pragma db member(user::login) type("VARCHAR(255)") not_null
#pragma db member(user::nickname) type("VARCHAR(255)") not_null
#pragma db member(user::email) type("VARCHAR(255)") not_null unique
#pragma db member(user::registered) type("DATETIME") null
#pragma db member(user::password) type("VARCHAR(128)") not_null
#pragma db member(user::level) not_null default(0)
#pragma db member(user::backup_email) type("VARCHAR(255)") null
#pragma db member(user::toons) value_not_null inverse(owner_id)
#pragma db member(user::choosen) transient
#pragma db member(user::authorized) transient

#pragma db object(legacy)
#pragma db member(legacy::name) id type("varchar(32)")
#pragma db member(legacy::realname) type("varchar(255)")
#pragma db member(legacy::email1) type("varchar(255)")
#pragma db member(legacy::email2) type("varchar(255)")
#endif

} /* namespace Alarmud */



#endif /* SRC_ODB_USERS_HXX_ */
