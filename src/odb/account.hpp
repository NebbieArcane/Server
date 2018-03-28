/*
 * registered.hxx
 *
 *  Created on: 22 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_ODB_ACCOUNT_HXX_
#define SRC_ODB_ACCOUNT_HXX_
#pragma db model version(1,1,open)
#include "../act.comm.hpp"
#include <boost/smart_ptr.hpp>
#include <odb/core.hxx>
#include <odb/vector.hxx>
#include <odb/boost/smart-ptr/lazy-ptr.hxx>
#include <odb/nullable.hxx>
#include <string>

namespace Alarmud {
using std::string;
class user;
class legacy;
class toon;
#pragma db object
class toon {
public:
	string name;
	string password;
	boost::shared_ptr<user> owner_id;
#pragma db member(name) id type("varchar(32)")
#pragma db member(password) type("varchar(128)")
#pragma db member(owner_id) index
};

#pragma db object
class user {
public:
	typedef odb::vector<boost::weak_ptr<toon> > toonVector;
	unsigned long long id;
	string nickname;
	string realname;
	string email;
	odb::nullable<string> backup_email;
	string password;
	string token;
	unsigned short level;
	toonVector toons;
#pragma db member(id) id auto
#pragma db member(nickname) type("VARCHAR(255)") not_null
#pragma db member(realname) type("VARCHAR(255)") not_null
#pragma db member(email) type("VARCHAR(255)") not_null unique
#pragma db member(backup_email) type("VARCHAR(255)") null
#pragma db member(token) type("VARCHAR(128)") not_null
#pragma db member(password) type("VARCHAR(128)") not_null
#pragma db member(toons) value_not_null inverse(owner_id)


	void newToken(unsigned short len=6) {
		char temp[len+1];
		for (unsigned short i=0;i<len;++i) {
			temp[i]=std::toupper(RandomChar());
		}
		temp[len]='\0';
		token.assign(temp);

	}
	user(){};
	user(const string &_nickname,const string &_realname,const string& _email)
	: nickname(_nickname),realname(_realname),email(_email),backup_email(""),password(""),token("")
	{
		newToken();
	}
};
#pragma db object
class legacy {
public:
	string name;
	string realname;
	string email1;
	string email2;
#pragma db member(name) id type("varchar(32)")
#pragma db member(realname) type("varchar(255)")
#pragma db member(email1) type("varchar(255)")
#pragma db member(email2) type("varchar(255)")
};

} /* namespace Alarmud */



#endif /* SRC_ODB_USERS_HXX_ */
