/*
 * legacy_import.cpp — file → MySQL character_* (see legacy_import.hpp).
 */

#include "legacy_import.hpp"

#include "config.hpp"
#include "flags.hpp"

#if USE_MYSQL

#include "legacy_loader.hpp"
#include "logging.hpp"
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include "autoenums.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Alarmud {
namespace {

std::string legacy_sql_escape(const char* s) {
	if(!s) {
		return "";
	}
	std::string out;
	out.reserve(std::strlen(s) * 2 + 4);
	for(const char* p = s; *p; ++p) {
		if(*p == '\'' || *p == '\\') {
			out.push_back('\\');
		}
		out.push_back(*p);
	}
	return out;
}

/* Campi char[] in char_file_u spesso non sono '\0'-terminated: non usare strlen. */
std::string legacy_utf8_from_legacy_bytes(const char* s, std::size_t cap) {
	if(!s || cap == 0) {
		return "";
	}
	std::string out;
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	const unsigned char* const end = p + strnlen(s, cap);
	out.reserve(static_cast<std::size_t>(end - p) + 8);
	while(p < end) {
		const unsigned char c = *p;
		if(c < 0x80) {
			if(c >= 0x20 || c == '\n' || c == '\r' || c == '\t') {
				out.push_back(static_cast<char>(c));
			}
			++p;
			continue;
		}
		std::size_t need = 0;
		if((c & 0xE0) == 0xC0) {
			need = 2;
		}
		else if((c & 0xF0) == 0xE0) {
			need = 3;
		}
		else if((c & 0xF8) == 0xF0) {
			need = 4;
		}
		if(need > 1 && p + need <= end) {
			bool ok = true;
			for(std::size_t i = 1; i < need; ++i) {
				if((p[i] & 0xC0) != 0x80) {
					ok = false;
					break;
				}
			}
			if(ok) {
				out.append(reinterpret_cast<const char*>(p), need);
				p += need;
				continue;
			}
		}
		/* ISO-8859-1 (testi MUD storici) → Unicode U+00xx → UTF-8 */
		out.push_back(static_cast<char>(0xC0 | (c >> 6)));
		out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
		++p;
	}
	return out;
}

std::string legacy_sql_string_literal(const char* s, std::size_t cap, bool allow_sql_null) {
	const std::string utf8 = legacy_utf8_from_legacy_bytes(s, cap);
	if(utf8.empty()) {
		return allow_sql_null ? "NULL" : "''";
	}
	return "'" + legacy_sql_escape(utf8.c_str()) + "'";
}

int legacy_condition_for_db(int raw) {
	if(raw < -1) {
		return -1;
	}
	if(raw > 24) {
		return 24;
	}
	return raw;
}

void legacy_delete_character_rows(odb::database* db, unsigned long long toon_id) {
	const std::string id = std::to_string(toon_id);
	db->execute(("DELETE cia FROM character_inventory_affect cia "
				 "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
				 "WHERE ci.toon_id = " +
				 id)
					.c_str());
	const char* tables[] = {"character_inventory", "character_rent",	 "character_prefs",
							"character_aliases", "character_achievements", "character_mercy",
							"character_quest_progress", "character_resistance", "character_affects",
							"character_skills",		"character_classes",	"character_stats",
							"character_core"};
	for(const char* table : tables) {
		std::string sql = "DELETE FROM ";
		sql += table;
		sql += " WHERE toon_id = ";
		sql += id;
		db->execute(sql.c_str());
	}
}

void legacy_insert_core(odb::database* db, unsigned long long toon_id, const char_file_u& st) {
	const int wimpy = std::atoi(st.WimpyLevel);
	std::ostringstream sql;
	sql << "INSERT INTO character_core (toon_id, description, class_primary, sex, race, "
		   "birth, played, last_logon, weight, height, hometown, "
		   "talks_0, talks_1, talks_2, speaks, user_flags, extra_flags, age_modifier, "
		   "authcode, wimpy_level, load_room, start_room, spells_to_learn, alignment, act, "
		   "affected_by, affected_by2, "
		   "condition_drunk, condition_full, condition_thirst, "
		   "save_throw_0, save_throw_1, save_throw_2, save_throw_3, "
		   "save_throw_4, save_throw_5, save_throw_6, save_throw_7) VALUES ("
		<< toon_id << ','
		<< legacy_sql_string_literal(st.description, sizeof(st.description), true) << ','
		<< st.iClass << ','
		<< static_cast<int>(st.sex)
		<< ',' << st.race << ',' << st.birth << ',' << st.played << ',' << st.last_logon << ','
		<< st.weight << ',' << st.height << ',' << st.hometown << ','
		<< (st.talks[0] ? 1 : 0) << ',' << (st.talks[1] ? 1 : 0) << ','
		<< (st.talks[2] ? 1 : 0) << ',' << st.speaks << ',' << st.user_flags << ','
		<< st.extra_flags << ',' << st.agemod << ','
		<< legacy_sql_string_literal(st.authcode, sizeof(st.authcode), false) << ','
		<< wimpy << ',' << st.load_room << ',' << st.startroom << ','
		<< static_cast<int>(st.spells_to_learn) << ',' << st.alignment << ',' << st.act << ','
		<< st.affected_by << ',' << st.affected_by2 << ','
		<< legacy_condition_for_db(st.conditions[0]) << ','
		<< legacy_condition_for_db(st.conditions[1]) << ','
		<< legacy_condition_for_db(st.conditions[2]) << ',';
	for(int i = 0; i < MAX_SAVES; ++i) {
		if(i > 0) {
			sql << ',';
		}
		sql << st.apply_saving_throw[i];
	}
	sql << ')';
	db->execute(sql.str().c_str());
}

void legacy_insert_stats(odb::database* db, unsigned long long toon_id, const char_file_u& st) {
	const char_ability_data& a = st.abilities;
	const char_point_data& p = st.points;
	std::ostringstream sql;
	sql << "INSERT INTO character_stats (toon_id, str, str_add, intel, wis, dex, con, chr, extra, "
		   "extra2, mana, max_mana, mana_gain, hit, max_hit, hit_gain, move, max_move, move_gain, "
		   "p_rune_dei, points_extra1, points_extra2, points_extra3, armor, gold, bank_gold, exp, "
		   "true_exp, extra_dual, hitroll, damroll, libero) VALUES ("
		<< toon_id << ',' << static_cast<int>(a.str) << ',' << static_cast<int>(a.str_add) << ','
		<< static_cast<int>(a.intel) << ',' << static_cast<int>(a.wis) << ','
		<< static_cast<int>(a.dex) << ',' << static_cast<int>(a.con) << ','
		<< static_cast<int>(a.chr) << ',' << static_cast<int>(a.extra) << ','
		<< static_cast<int>(a.extra2) << ',' << p.mana << ',' << p.max_mana << ','
		<< static_cast<int>(p.mana_gain) << ',' << p.hit << ',' << p.max_hit << ','
		<< static_cast<int>(p.hit_gain) << ',' << p.move << ',' << p.max_move << ','
		<< static_cast<int>(p.move_gain) << ',' << p.pRuneDei << ',' << p.extra1 << ',' << p.extra2
		<< ',' << static_cast<int>(p.extra3) << ',' << p.armor << ',' << p.gold << ',' << p.bankgold
		<< ',' << p.exp << ',' << p.true_exp << ',' << p.extra_dual << ','
		<< static_cast<int>(p.hitroll) << ',' << static_cast<int>(p.damroll) << ','
		<< static_cast<int>(p.libero) << ')';
	db->execute(sql.str().c_str());
}

std::size_t legacy_insert_classes(odb::database* db, unsigned long long toon_id,
								  const char_file_u& st) {
	std::size_t n = 0;
	for(int i = 0; i < MAX_CLASS; ++i) {
		if(st.level[i] == 0) {
			continue;
		}
		std::ostringstream sql;
		sql << "INSERT INTO character_classes (toon_id, class_index, level) VALUES ("
			<< toon_id << ',' << i << ',' << static_cast<int>(st.level[i]) << ')';
		db->execute(sql.str().c_str());
		++n;
	}
	return n;
}

std::size_t legacy_insert_skills(odb::database* db, unsigned long long toon_id,
								 const char_file_u& st) {
	std::size_t n = 0;
	for(int i = 0; i < MAX_SKILLS; ++i) {
		const char_skill_data& sk = st.skills[i];
		if(sk.learned == 0 && sk.flags == 0) {
			continue;
		}
		std::ostringstream sql;
		sql << "INSERT INTO character_skills (toon_id, skill_id, learned, flags, special, nummem) "
			   "VALUES ("
			<< toon_id << ',' << i << ',' << static_cast<int>(sk.learned) << ','
			<< static_cast<int>(sk.flags) << ',' << static_cast<int>(sk.special) << ','
			<< static_cast<int>(sk.nummem) << ')';
		db->execute(sql.str().c_str());
		++n;
	}
	return n;
}

std::size_t legacy_insert_affects(odb::database* db, unsigned long long toon_id,
								  const char_file_u& st) {
	std::size_t n = 0;
	for(int i = 0; i < MAX_AFFECT; ++i) {
		if(st.affected[i].type == 0) {
			continue;
		}
		const affected_type_u& af = st.affected[i];
		std::ostringstream sql;
		sql << "INSERT INTO character_affects (toon_id, slot, type, duration, modifier, location, "
			   "bitvector) VALUES ("
			<< toon_id << ',' << i << ',' << af.type << ',' << af.duration << ',' << af.modifier
			<< ',' << af.location << ',' << af.bitvector << ')';
		db->execute(sql.str().c_str());
		++n;
	}
	return n;
}

std::size_t legacy_insert_resistances(odb::database* db, unsigned long long toon_id,
									  const char_file_u& st) {
	unsigned immune = 0, m_immune = 0, susc = 0;
	legacy_derive_resistance_from_file(st, immune, m_immune, susc);
	std::vector<LegacyResistanceRow> rows;
	legacy_resistance_rows_from_bitvectors(immune, m_immune, susc, rows);

	std::size_t n = 0;
	for(const auto& row : rows) {
		if(row.value == 0) {
			continue;
		}
		std::ostringstream sql;
		sql << "INSERT INTO character_resistance (toon_id, damage_type, value) VALUES ("
			<< toon_id << ',' << row.damage_type << ',' << static_cast<int>(row.value) << ')';
		db->execute(sql.str().c_str());
		++n;
	}
	return n;
}

std::size_t legacy_insert_prefs(odb::database* db, unsigned long long toon_id,
								const LegacyCharAux& aux) {
	std::size_t n = 0;
	for(const auto& e : aux.entries) {
		if(e.tag.empty()) {
			continue;
		}
		std::string key = e.tag;
		if(key.size() > 32) {
			key.resize(32);
		}
		std::string val = e.value;
		if(val.size() > 1024) {
			val.resize(1024);
		}
		std::ostringstream sql;
		sql << "INSERT INTO character_prefs (toon_id, pref_key, pref_value) VALUES (" << toon_id
			<< ",'" << legacy_sql_escape(key.c_str()) << "','" << legacy_sql_escape(val.c_str())
			<< "')";
		db->execute(sql.str().c_str());
		++n;
	}
	return n;
}

std::size_t legacy_insert_rent(odb::database* db, unsigned long long toon_id,
							   const obj_file_u& rent) {
	std::ostringstream sql;
	sql << "INSERT INTO character_rent (toon_id, gold_left, total_cost, last_update, minimum_stay, "
		   "object_count) VALUES ("
		<< toon_id << ',' << rent.gold_left << ',' << rent.total_cost << ',' << rent.last_update
		<< ',' << rent.minimum_stay << ',' << rent.number << ')';
	db->execute(sql.str().c_str());

	std::size_t n = 0;
	for(int i = 0; i < rent.number && i < MAX_OBJ_SAVE; ++i) {
		const obj_file_elem& o = rent.objects[i];
		std::ostringstream ins;
		ins << "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, value1, "
			   "value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, obj_name, "
			   "short_desc, description, wear_pos, depth) VALUES ("
			<< toon_id << ',' << i << ',' << o.item_number << ',' << o.value[0] << ',' << o.value[1]
			<< ',' << o.value[2] << ',' << o.value[3] << ',' << o.extra_flags << ','
			<< o.extra_flags2 << ',' << o.weight << ',' << o.timer << ',' << o.bitvector << ","
			<< legacy_sql_string_literal(o.name, sizeof(o.name), false) << ','
			<< legacy_sql_string_literal(o.sd, sizeof(o.sd), false) << ','
			<< legacy_sql_string_literal(o.desc, sizeof(o.desc), false) << ','
			<< static_cast<int>(o.wearpos) << ','
			<< static_cast<int>(o.depth) << ')';
		db->execute(ins.str().c_str());

		for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
			const obj_affected_type& oa = o.affected[a];
			if(oa.location == 0 && oa.modifier == 0) {
				continue;
			}
			std::ostringstream aff;
			aff << "INSERT INTO character_inventory_affect (inventory_id, affect_slot, location, "
				   "modifier) SELECT id, "
				<< a << ',' << oa.location << ',' << oa.modifier
				<< " FROM character_inventory WHERE toon_id = " << toon_id << " AND list_index = "
				<< i;
			db->execute(aff.str().c_str());
		}
		++n;
	}
	return n;
}

} /* anonymous */

bool legacy_import_character_mysql(const char* file_name, LegacyImportReport& report) {
	report = LegacyImportReport {};

	if(!file_name || !*file_name) {
		report.message = "nome file mancante";
		return false;
	}

	char_file_u st {};
	if(!legacy_load_char_file(file_name, st)) {
		report.message = "impossibile leggere .dat";
		return false;
	}

	toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(st.name));
	if(!pg || !pg->id) {
		pg = Sql::getOne<toon>(toonQuery::name == std::string(file_name));
	}
	if(!pg || !pg->id) {
		report.message = "nessun record toon per ";
		report.message += st.name;
		return false;
	}

	try {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);

		legacy_delete_character_rows(db, pg->id);
		legacy_insert_core(db, pg->id, st);
		legacy_insert_stats(db, pg->id, st);
		report.classes = legacy_insert_classes(db, pg->id, st);
		report.skills = legacy_insert_skills(db, pg->id, st);
		report.affects = legacy_insert_affects(db, pg->id, st);
		report.resistances = legacy_insert_resistances(db, pg->id, st);

		LegacyCharAux aux {};
		if(legacy_load_char_aux(file_name, aux)) {
			report.prefs = legacy_insert_prefs(db, pg->id, aux);
		}

		obj_file_u rent {};
		if(legacy_load_rent_file(file_name, rent)) {
			report.inventory = legacy_insert_rent(db, pg->id, rent);
		}

		t.commit();

		report.ok = true;
		report.toon_id = pg->id;
		char buf[256];
		std::snprintf(buf, sizeof(buf),
					  "import OK toon_id=%llu core+stats classi=%zu skill=%zu affect=%zu "
					  "resist=%zu prefs=%zu oggetti=%zu",
					  static_cast<unsigned long long>(pg->id), report.classes, report.skills,
					  report.affects, report.resistances, report.prefs, report.inventory);
		report.message = buf;
		return true;
	}
	catch(const odb::exception& e) {
		report.message = e.what();
		mudlog(LOG_SYSERR, "legacy_import: %s", e.what());
		return false;
	}
	catch(const std::exception& e) {
		report.message = e.what();
		return false;
	}
}

} /* namespace Alarmud */

#else /* !USE_MYSQL */

namespace Alarmud {

bool legacy_import_character_mysql(const char* file_name, LegacyImportReport& report) {
	(void)file_name;
	report = LegacyImportReport {};
	report.message = "MySQL non abilitato in build";
	return false;
}

} /* namespace Alarmud */

#endif /* USE_MYSQL */
