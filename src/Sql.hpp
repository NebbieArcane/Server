/*
 * Sql.hpp
 *
 *  Created on: 24 mar 2018
 *      Author: giovanni
 */

#ifndef SRC_SQL_HPP_
#define SRC_SQL_HPP_
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
	template <typename T>
	static bool load(T &obj) {
		try {
			DB* db = Sql::getMysql();
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			obj.load();
			t.commit();
		}
		catch (odb::exception &e) {
			return false;
		}
		return true;
	}
	template <typename T>
	static boost::shared_ptr<T> getOne(odb::query<T> key) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		auto datum(db->query_one<T>(key));
		t.commit();
		return datum;
	}
	template <typename T>
	static boost::shared_ptr<odb::result<T>> getAll() {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		auto datum(db->load<T>());
		t.commit();
		return datum;
	}
	template <typename T>
	static boost::shared_ptr<odb::result<T>> getAll(odb::query<T> key) {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		auto datum(db->query<T>(key));
		t.commit();
		return datum;
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
