/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* object_instance.cpp - CRUD MySQL per eq editato come istanza. */
#include "object_instance.hpp"

#if USE_MYSQL

#include "logging.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "autoenums.hpp"
#include "db.hpp"
#include "Sql.hpp"
#include "odb/account-odb.hxx"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdlib>
#include <cstring>
#include <string>

namespace Alarmud {

namespace {

const char* safe_cstr(const char* s) {
	return s ? s : "";
}

void fill_instance_from_obj(object_instance& row, const struct obj_data* obj, int base_vnum) {
	row.base_vnum = static_cast<unsigned int>(base_vnum);
	if(obj->char_vnum > 0) {
		row.char_vnum = static_cast<unsigned int>(obj->char_vnum);
	}
	else {
		row.char_vnum = odb::nullable<unsigned int>();
	}
	row.type_flag = obj->obj_flags.type_flag;
	row.wear_flags = obj->obj_flags.wear_flags;
	row.extra_flags = static_cast<int>(obj->obj_flags.extra_flags);
	row.extra_flags2 = static_cast<int>(obj->obj_flags.extra_flags2);
	row.weight = obj->obj_flags.weight;
	row.cost = obj->obj_flags.cost;
	row.cost_per_day = obj->obj_flags.cost_per_day;
	row.timer = obj->obj_flags.timer;
	row.bitvector = obj->obj_flags.bitvector;
	row.value0 = obj->obj_flags.value[0];
	row.value1 = obj->obj_flags.value[1];
	row.value2 = obj->obj_flags.value[2];
	row.value3 = obj->obj_flags.value[3];
	row.obj_name = safe_cstr(obj->name);
	row.short_desc = safe_cstr(obj->short_description);
	row.description = safe_cstr(obj->description);
	if(obj->action_description && *obj->action_description) {
		row.action_desc = std::string(obj->action_description);
	}
	else {
		row.action_desc = odb::nullable<std::string>();
	}
	const int cur_vnum =
		(obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	if(cur_vnum >= LOW_EDITED_ITEMS && cur_vnum <= HIGH_EDITED_ITEMS) {
		row.legacy_edit_vnum = static_cast<unsigned int>(cur_vnum);
	}
	const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	if(row.id == 0) {
		row.created_at = now;
	}
	row.updated_at = now;
}

bool replace_instance_affects_tx(DB* db, unsigned long long instance_id,
								 const struct obj_data* obj) {
	using AffQ = odb::query<object_instance_affect>;
	db->erase_query<object_instance_affect>(AffQ::key.instance_id == instance_id);
	for(unsigned char slot = 0; slot < MAX_OBJ_AFFECT; ++slot) {
		const auto& af = obj->affected[slot];
		if(af.location == 0 && af.modifier == 0) {
			continue;
		}
		object_instance_affect row;
		row.key.instance_id = instance_id;
		row.key.affect_slot = slot;
		row.location = af.location;
		row.modifier = af.modifier;
		db->persist(row);
	}
	return true;
}

void apply_strings(struct obj_data* obj, const std::string& name, const std::string& sd,
				   const std::string& desc, const odb::nullable<std::string>& action) {
	if(obj->name) {
		free(obj->name);
	}
	if(obj->short_description) {
		free(obj->short_description);
	}
	if(obj->description) {
		free(obj->description);
	}
	if(obj->action_description) {
		free(obj->action_description);
		obj->action_description = nullptr;
	}
	obj->name = static_cast<char*>(malloc(name.size() + 1));
	obj->short_description = static_cast<char*>(malloc(sd.size() + 1));
	obj->description = static_cast<char*>(malloc(desc.size() + 1));
	std::strcpy(obj->name, name.c_str());
	std::strcpy(obj->short_description, sd.c_str());
	std::strcpy(obj->description, desc.c_str());
	if(!action.null() && !action.get().empty()) {
		const std::string& a = action.get();
		obj->action_description = static_cast<char*>(malloc(a.size() + 1));
		std::strcpy(obj->action_description, a.c_str());
	}
}

} // namespace

int object_instance_resolve_base_vnum(const struct obj_data* obj) {
	if(!obj) {
		return 0;
	}
	const int iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	if(obj->char_vnum > 0) {
		const int cv = obj->char_vnum;
		if(cv < LOW_EDITED_ITEMS || cv > HIGH_EDITED_ITEMS) {
			return cv;
		}
	}
	if(iVNum > 0 && (iVNum < LOW_EDITED_ITEMS || iVNum > HIGH_EDITED_ITEMS)) {
		return iVNum;
	}
	return 0;
}

unsigned long long object_instance_persist(struct obj_data* obj, int base_vnum,
										   unsigned long long update_id) {
	if(!obj || base_vnum <= 0) {
		return 0;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		mudlog(LOG_SYSERR, "object_instance_persist: no database");
		return 0;
	}

	unsigned long long id = update_id ? update_id : obj->db_instance_id;
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		object_instance row {};
		if(id != 0) {
			try {
				db->load<object_instance>(id, row);
			}
			catch(const odb::exception&) {
				id = 0;
				row = object_instance {};
			}
		}
		fill_instance_from_obj(row, obj, base_vnum);
		if(id == 0) {
			row.id = 0;
			db->persist(row);
			id = row.id;
		}
		else {
			row.id = id;
			db->update(row);
		}
		replace_instance_affects_tx(db, id, obj);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_persist: %s", e.what());
		return 0;
	}

	obj->db_instance_id = id;
	return id;
}

bool object_instance_apply(struct obj_data* obj, unsigned long long instance_id) {
	if(!obj || instance_id == 0) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		object_instance row;
		db->load<object_instance>(instance_id, row);

		obj->obj_flags.type_flag = row.type_flag;
		obj->obj_flags.wear_flags = row.wear_flags;
		obj->obj_flags.extra_flags = static_cast<unsigned int>(row.extra_flags);
		obj->obj_flags.extra_flags2 = static_cast<unsigned int>(row.extra_flags2);
		obj->obj_flags.weight = row.weight;
		obj->obj_flags.cost = row.cost;
		obj->obj_flags.cost_per_day = row.cost_per_day;
		obj->obj_flags.timer = row.timer;
		obj->obj_flags.bitvector = row.bitvector;
		obj->obj_flags.value[0] = row.value0;
		obj->obj_flags.value[1] = row.value1;
		obj->obj_flags.value[2] = row.value2;
		obj->obj_flags.value[3] = row.value3;
		apply_strings(obj, row.obj_name, row.short_desc, row.description, row.action_desc);
		if(!row.char_vnum.null()) {
			obj->char_vnum = static_cast<int>(row.char_vnum.get());
		}
		else if(row.base_vnum > 0) {
			obj->char_vnum = static_cast<int>(row.base_vnum);
		}
		SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);

		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			obj->affected[i].location = 0;
			obj->affected[i].modifier = 0;
		}
		using AffQ = odb::query<object_instance_affect>;
		for(const auto& af :
			db->query<object_instance_affect>(AffQ::key.instance_id == instance_id)) {
			if(af.key.affect_slot >= MAX_OBJ_AFFECT) {
				continue;
			}
			obj->affected[af.key.affect_slot].location = af.location;
			obj->affected[af.key.affect_slot].modifier = af.modifier;
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_apply(%llu): %s",
			   static_cast<unsigned long long>(instance_id), e.what());
		return false;
	}
	obj->db_instance_id = instance_id;
	return true;
}

bool object_instance_sync(struct obj_data* obj) {
	if(!obj || obj->db_instance_id == 0) {
		return false;
	}
	int base = object_instance_resolve_base_vnum(obj);
	if(base <= 0) {
		base = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	}
	if(base <= 0) {
		return false;
	}
	return object_instance_persist(obj, base, obj->db_instance_id) != 0;
}

} // namespace Alarmud

#endif /* USE_MYSQL */
