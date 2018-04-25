/*
 * Sql.hpp
 *
 *  Created on: 24 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_SQL_HPP_
#define SRC_SQL_HPP_
#include <vector>
#include <boost/make_shared.hpp>
#include "odb/odb.hpp"
#include "autoenums.hpp"
#include "logging.hpp"
namespace Alarmud {
extern bool forceDbInit;

class sqlTrace : public odb::tracer {
public:
	virtual void execute(odb::connection &c, const odb::statement &s) {
		execute(c,s.text());
	}
	virtual void execute(odb::connection &c, const char* statement);
};
extern sqlTrace logTracer;
class Sql {
public:
#if USE_MYSQL
	static odb::database* getMysql();
#endif
#if USE_SQLITE
	static odb::database* getSqlite();
#endif
	static void dbUpdate();
	template <typename T,typename C>
	static boost::shared_ptr<T> getOne(C key) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		auto datum(db->load<T>(key));
		t.commit();
		return datum;
	}
	/* To be used with views only (other objects are not meaningful without a key */
	template <typename T>
	static boost::shared_ptr<T> getOne() {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		/*returned datum wiill be destroyed on exit, so we make a copy of it and return in
		 * a shared pointer. This is also done for consistency with the other getOne which all
		 * return a shared pointer
		 */

		auto  datum=boost::make_shared<T>(db->query_value<T>());

		t.commit();
		return datum;
	}
	template <typename T>
	static boost::shared_ptr<T> getOne(odb::query<T> key) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		boost::shared_ptr<T> datum=db->query_one<T>(key);
		t.commit();
		return datum;
	}
	template <typename T>
	static boost::shared_ptr<T> getOne(odb::query_base key) {
		return getOne(odb::query<T>(key));
	}
	template <typename T>
	static std::vector<boost::shared_ptr<T>> getAll(odb::query<T> key) {
		std::vector<boost::shared_ptr<T>> v({});
		DB* db = Sql::getMysql();
		//odb::session s;
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		auto r(db->query<T> (key));
		if (!r.empty()) {
			v.reserve(r.size());
			for(auto iter: r) {
				v.push_back(boost::make_shared<T>(iter));
			}
		}
		t.commit();
		return v;
	}
	template <typename T>
	static std::vector<boost::shared_ptr<T>> getAll() {
		return getAll(odb::query<T>());
	}
	template <typename T>
	static bool save(T &data,bool upsert=false) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		try {
			db->persist<T>(data);
			t.commit();
			return true;
		}
		catch(odb::object_already_persistent &e) {
			if(upsert) {
				try {
					db->update<T>(data);
					t.commit();
					return true;
				}
				catch(odb::exception &e) {
					mudlog(LOG_SYSERR,"Db exception: %s",e.what());
				}
			}
		}
		return false;

	}
	template <typename T>
	static bool update(T &data,bool upsert=false) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		try {
			db->update<T>(data);
			t.commit();
			return true;
		}
		catch(odb::object_not_persistent &e) {
			if(upsert) {
				try {
					db->persist<T>(data);
					t.commit();
					return true;
				}
				catch(odb::exception &e) {
					mudlog(LOG_SYSERR,"Db exception: %s",e.what());
				}
			}
		}
		return false;

	}
	template <typename T>
	static bool erase(T &data,bool upsert=false) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		try {
			db->erase<T>(data);
			t.commit();
			return true;
		}
		catch(odb::exception &e) {
			mudlog(LOG_SYSERR,"Db exception: %s",e.what());
		}
		return false;

	}
	virtual ~Sql();
protected:
	Sql();

};

} /* namespace Alarmud */

#endif /* SRC_SQL_HPP_ */
