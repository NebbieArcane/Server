/*
 * registered.hxx
 *
 *  Created on: 22 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_ODB_ACCOUNT_HXX_
#define SRC_ODB_ACCOUNT_HXX_
//#include <boost/smart_ptr.hpp>
#include <iostream>
#include <boost/date_time.hpp>
//#include <odb/boost/date-time/exceptions.hxx>
//#include <odb/boost/date-time/mysql/posix-time-mapping.hxx>
#include <odb/core.hxx>
#include <odb/boost/lazy-ptr.hxx>
#include <odb/nullable.hxx>
#include <string>
#include "../utility.hpp"

namespace Alarmud {
using std::string;
class toonExtra;
class toonRent;
class user;
class legacy;
class toon;
class toonExtra {
public:
	unsigned long id;
	string field;
	string value;
};
class toonRent {
public:
	unsigned long id;
	unsigned long vnum;
	string effects;
	bool bank;
};
class toon {
public:
	toon() :id(0),name(""),password(""),title(""),lastlogin(boost::posix_time::not_a_date_time),owner_id(0) {};
	toon(const char* name,const char* password="invalid", const char* title="") :id(0),name(name),password(password),title(title),lastlogin(boost::posix_time::not_a_date_time),owner_id(0) {}
	unsigned long id;
	string name;
	string password;
	string title;
	boost::posix_time::ptime lastlogin;
	unsigned long long owner_id;
};
class user {
public:
	unsigned long long id;
	string nickname;
	string email;
	boost::posix_time::ptime registered;
	string password;
	unsigned short level;
	odb::nullable<string> backup_email;
	bool ptr;
	string choosen;
	bool authorized;
	user() {};
	user(const string &nickname,const string &email,const string &password=""):
		id(0),
		nickname(nickname),
		email(email),
		registered(boost::posix_time::from_time_t(time(nullptr))),
		password(password),
		level(0),
		backup_email(),
		ptr(false),
		choosen(""),
		authorized(false)
	{}
};
#ifdef ODB_COMPILER
#pragma db view object(user)
#endif
class userCount {
public:
#ifdef ODB_COMPILER
#pragma db column("count(" + user::id + ")")
#endif
	std::size_t count;
	virtual ~userCount() {
		std::cout << "Destroyed usercount" << std::endl;
	}
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
#pragma db member(toon::id) id auto
#pragma db member(toon::name) type("varchar(32)")
#pragma db member(toon::name) unique
#pragma db member(toon::password) type("varchar(16)")
#pragma db member(toon::title) type("varchar(128)")
#pragma db member(toon::lastlogin) type("DATETIME") null
#pragma db member(toon::owner_id) index

#pragma db object(toonExtra)
#pragma db member(toonExtra::id) id auto
#pragma db member(toonExtra::field) type("varchar(32)")
#pragma db member(toonExtra::value) type("varchar(1024)")
#pragma db  index(toonExtra::"idfield") members(id,field)


#pragma db object(toonRent)
#pragma db member(toonRent::id) id auto
#pragma db member(toonRent::effects) type("varchar(10240)")

//login,nickname,email,registered,password,level

#pragma db object(user)
#pragma db member(user::id) id auto
#pragma db member(user::nickname) type("VARCHAR(255)") not_null
#pragma db member(user::email) type("VARCHAR(255)") not_null unique
#pragma db member(user::registered) type("DATETIME") null
#pragma db member(user::password) type("VARCHAR(128)") not_null
#pragma db member(user::level) not_null default(0)
#pragma db member(user::backup_email) type("VARCHAR(255)") null
#pragma db member(user::ptr) not_null default(0)
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
