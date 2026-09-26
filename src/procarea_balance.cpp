/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "logging.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "utility.hpp"
#include "procarea_balance.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <strings.h>
#include <unordered_map>

#if USE_MYSQL
#include "Sql.hpp"
#include "odb/odb.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#endif

namespace Alarmud {

namespace {

ProcDensityConfig g_density{};
ProcRewardsConfig g_rewards{};
ProcLevelConfig g_levels{};

constexpr const char* kCrystalNames[PROCAREA_CRYSTAL_COUNT] = {
	"verde", "blu", "rosso", "arancione", "fucsia",
};

[[nodiscard]] bool is_balance_wiz(char_data* ch) {
	return ch != nullptr && IS_PC(ch) && GetMaxLevel(ch) >= 56;
}

[[nodiscard]] int clamp_int(int v, int lo, int hi) {
	return std::clamp(v, lo, hi);
}

[[nodiscard]] float clamp_float(float v, float lo, float hi) {
	return std::clamp(v, lo, hi);
}

void sanitize_density(ProcDensityConfig& d) {
	d.bias = clamp_float(d.bias, 0.50f, 1.50f);
	d.rooms_min_lo = clamp_int(d.rooms_min_lo, 4, PROCAREA_ROOMS_MAX);
	d.rooms_min_hi = clamp_int(d.rooms_min_hi, 4, PROCAREA_ROOMS_MAX);
	d.rooms_max_lo = clamp_int(d.rooms_max_lo, 4, PROCAREA_ROOMS_MAX);
	d.rooms_max_hi = clamp_int(d.rooms_max_hi, 4, PROCAREA_ROOMS_MAX);
	if(d.rooms_max_lo < d.rooms_min_lo) {
		d.rooms_max_lo = d.rooms_min_lo;
	}
	if(d.rooms_max_hi < d.rooms_min_hi) {
		d.rooms_max_hi = d.rooms_min_hi;
	}
	d.corr_lo = clamp_int(d.corr_lo, 5, 95);
	d.corr_hi = clamp_int(d.corr_hi, 5, 95);
	d.tes_lo = clamp_int(d.tes_lo, 5, 98);
	d.tes_hi = clamp_int(d.tes_hi, 5, 98);
	d.adds_lo = clamp_int(d.adds_lo, 0, 6);
	d.adds_hi = clamp_int(d.adds_hi, 0, 6);
	d.depth_lo = clamp_int(d.depth_lo, 0, 20);
	d.depth_hi = clamp_int(d.depth_hi, 0, 20);
	d.br_lo = clamp_int(d.br_lo, 0, 10);
	d.br_hi = clamp_int(d.br_hi, 0, 10);
	d.bc_lo = clamp_int(d.bc_lo, 0, 90);
	d.bc_hi = clamp_int(d.bc_hi, 0, 90);
	d.sec_corr = clamp_int(d.sec_corr, 0, 95);
	d.sec_tre = clamp_int(d.sec_tre, 0, 95);
	d.sec_trap = clamp_int(d.sec_trap, 0, 95);
	d.solo_rooms_pct = clamp_int(d.solo_rooms_pct, 20, 100);
	d.solo_rooms_min_floor = clamp_int(d.solo_rooms_min_floor, 4, PROCAREA_ROOMS_MAX);
	d.solo_rooms_max_floor = clamp_int(d.solo_rooms_max_floor, 4, PROCAREA_ROOMS_MAX);
	d.solo_branches_delta = clamp_int(d.solo_branches_delta, 0, 8);
	d.solo_branch_chance_delta = clamp_int(d.solo_branch_chance_delta, 0, 40);
	d.solo_branch_chance_floor = clamp_int(d.solo_branch_chance_floor, 0, 90);
	d.solo_corr_pct = clamp_int(d.solo_corr_pct, 20, 100);
	d.solo_corr_floor = clamp_int(d.solo_corr_floor, 5, 95);
	d.solo_tes_delta = clamp_int(d.solo_tes_delta, 0, 40);
	d.solo_tes_floor = clamp_int(d.solo_tes_floor, 5, 98);
	d.solo_adds_delta = clamp_int(d.solo_adds_delta, 0, 4);
	d.solo_depth_delta = clamp_int(d.solo_depth_delta, 0, 12);
	d.solo_depth_floor = clamp_int(d.solo_depth_floor, 0, 20);
}

void sanitize_rewards(ProcRewardsConfig& r) {
	for(int i = 0; i < PROCAREA_FATIGUE_TIER_COUNT; ++i) {
		r.fatigue_gear_base[i] = clamp_int(r.fatigue_gear_base[i], 0, 100);
		r.fatigue_gear_decay[i] = clamp_int(r.fatigue_gear_decay[i], 0, 100);
		r.fatigue_gold_pct[i] = clamp_int(r.fatigue_gold_pct[i], 0, 100);
	}
	for(int i = 0; i < 5; ++i) {
		r.frag_drop_pct[i] = clamp_int(r.frag_drop_pct[i], 0, 100);
		r.frag_qty_mult[i] = clamp_int(r.frag_qty_mult[i], 10, 1000);
	}
	r.frag_qty_bias = clamp_float(r.frag_qty_bias, 0.25f, 3.0f);
	r.fragments_per_rune = clamp_int(r.fragments_per_rune, 100, 100000);
	for(int i = 0; i < PROCAREA_CRYSTAL_COUNT; ++i) {
		r.crystal[i].mob_mult = clamp_float(r.crystal[i].mob_mult, 0.40f, 2.0f);
		r.crystal[i].frag_mult = clamp_float(r.crystal[i].frag_mult, 0.20f, 3.0f);
		r.crystal[i].frag_drop_corridor_pct =
			clamp_int(r.crystal[i].frag_drop_corridor_pct, 0, 100);
		r.crystal[i].frag_drop_treasure_pct =
			clamp_int(r.crystal[i].frag_drop_treasure_pct, 0, 100);
	}
}

void sanitize_levels(ProcLevelConfig& L) {
	for(int i = 0; i < PROCAREA_TEMPLATE_BANDS; ++i) {
		L.fascia[i] = clamp_int(L.fascia[i], 0, 8);
	}
	L.boss_bonus = clamp_int(L.boss_bonus, 0, 10);
	L.trap_bonus_lo = clamp_int(L.trap_bonus_lo, 0, 10);
	L.trap_bonus_hi = clamp_int(L.trap_bonus_hi, 0, 10);
	if(L.trap_bonus_hi < L.trap_bonus_lo) {
		L.trap_bonus_hi = L.trap_bonus_lo;
	}
}

using KeyMap = std::unordered_map<std::string, std::string>;

void put_int(KeyMap& m, const std::string& k, int v) {
	m[k] = std::to_string(v);
}

void put_float(KeyMap& m, const std::string& k, float v) {
	char buf[64];
	std::snprintf(buf, sizeof(buf), "%.4f", static_cast<double>(v));
	m[k] = buf;
}

void export_density(KeyMap& m) {
	put_float(m, "bias", g_density.bias);
	put_int(m, "rooms_min_lo", g_density.rooms_min_lo);
	put_int(m, "rooms_min_hi", g_density.rooms_min_hi);
	put_int(m, "rooms_max_lo", g_density.rooms_max_lo);
	put_int(m, "rooms_max_hi", g_density.rooms_max_hi);
	put_int(m, "corr_lo", g_density.corr_lo);
	put_int(m, "corr_hi", g_density.corr_hi);
	put_int(m, "tes_lo", g_density.tes_lo);
	put_int(m, "tes_hi", g_density.tes_hi);
	put_int(m, "adds_lo", g_density.adds_lo);
	put_int(m, "adds_hi", g_density.adds_hi);
	put_int(m, "depth_lo", g_density.depth_lo);
	put_int(m, "depth_hi", g_density.depth_hi);
	put_int(m, "br_lo", g_density.br_lo);
	put_int(m, "br_hi", g_density.br_hi);
	put_int(m, "bc_lo", g_density.bc_lo);
	put_int(m, "bc_hi", g_density.bc_hi);
	put_int(m, "sec_corr", g_density.sec_corr);
	put_int(m, "sec_tre", g_density.sec_tre);
	put_int(m, "sec_trap", g_density.sec_trap);
	put_int(m, "solo_rooms_pct", g_density.solo_rooms_pct);
	put_int(m, "solo_rooms_min_floor", g_density.solo_rooms_min_floor);
	put_int(m, "solo_rooms_max_floor", g_density.solo_rooms_max_floor);
	put_int(m, "solo_branches_delta", g_density.solo_branches_delta);
	put_int(m, "solo_branch_chance_delta", g_density.solo_branch_chance_delta);
	put_int(m, "solo_branch_chance_floor", g_density.solo_branch_chance_floor);
	put_int(m, "solo_corr_pct", g_density.solo_corr_pct);
	put_int(m, "solo_corr_floor", g_density.solo_corr_floor);
	put_int(m, "solo_tes_delta", g_density.solo_tes_delta);
	put_int(m, "solo_tes_floor", g_density.solo_tes_floor);
	put_int(m, "solo_adds_delta", g_density.solo_adds_delta);
	put_int(m, "solo_depth_delta", g_density.solo_depth_delta);
	put_int(m, "solo_depth_floor", g_density.solo_depth_floor);
}

void export_rewards(KeyMap& m) {
	for(int i = 0; i < PROCAREA_FATIGUE_TIER_COUNT; ++i) {
		put_int(m, "gear_base_" + std::to_string(i), g_rewards.fatigue_gear_base[i]);
		put_int(m, "gear_decay_" + std::to_string(i), g_rewards.fatigue_gear_decay[i]);
		put_int(m, "gold_pct_" + std::to_string(i), g_rewards.fatigue_gold_pct[i]);
	}
	static const char* const kFragKind[] = { "corr", "tes", "add", "trap", "boss" };
	for(int i = 0; i < 5; ++i) {
		put_int(m, std::string("frag_drop_") + kFragKind[i], g_rewards.frag_drop_pct[i]);
		put_int(m, std::string("frag_qty_") + kFragKind[i], g_rewards.frag_qty_mult[i]);
	}
	put_float(m, "frag_qty_bias", g_rewards.frag_qty_bias);
	put_int(m, "frag_per_rune", g_rewards.fragments_per_rune);
	for(int i = 0; i < PROCAREA_CRYSTAL_COUNT; ++i) {
		const std::string p = std::string("crystal_") + kCrystalNames[i] + "_";
		put_float(m, p + "mob", g_rewards.crystal[i].mob_mult);
		put_float(m, p + "frag", g_rewards.crystal[i].frag_mult);
		put_int(m, p + "drop_corr", g_rewards.crystal[i].frag_drop_corridor_pct);
		put_int(m, p + "drop_tes", g_rewards.crystal[i].frag_drop_treasure_pct);
	}
}

void export_levels(KeyMap& m) {
	for(int i = 0; i < PROCAREA_TEMPLATE_BANDS; ++i) {
		put_int(m, "fascia_" + std::to_string(i), g_levels.fascia[i]);
	}
	put_int(m, "boss_bonus", g_levels.boss_bonus);
	put_int(m, "trap_bonus_lo", g_levels.trap_bonus_lo);
	put_int(m, "trap_bonus_hi", g_levels.trap_bonus_hi);
}

[[nodiscard]] bool parse_int_val(const std::string& s, int& out) {
	char* end = nullptr;
	const long v = std::strtol(s.c_str(), &end, 10);
	if(end == s.c_str() || (end != nullptr && *end != '\0')) {
		return false;
	}
	out = static_cast<int>(v);
	return true;
}

[[nodiscard]] bool parse_float_val(const std::string& s, float& out) {
	char* end = nullptr;
	const float v = std::strtof(s.c_str(), &end);
	if(end == s.c_str() || (end != nullptr && *end != '\0')) {
		return false;
	}
	out = v;
	return true;
}

void apply_density_key(const std::string& key, const std::string& val) {
	int iv = 0;
	float fv = 0.0f;
	if(key == "bias" && parse_float_val(val, fv)) {
		g_density.bias = fv;
	} else if(key == "rooms_min_lo" && parse_int_val(val, iv)) {
		g_density.rooms_min_lo = iv;
	} else if(key == "rooms_min_hi" && parse_int_val(val, iv)) {
		g_density.rooms_min_hi = iv;
	} else if(key == "rooms_max_lo" && parse_int_val(val, iv)) {
		g_density.rooms_max_lo = iv;
	} else if(key == "rooms_max_hi" && parse_int_val(val, iv)) {
		g_density.rooms_max_hi = iv;
	} else if(key == "corr_lo" && parse_int_val(val, iv)) {
		g_density.corr_lo = iv;
	} else if(key == "corr_hi" && parse_int_val(val, iv)) {
		g_density.corr_hi = iv;
	} else if(key == "tes_lo" && parse_int_val(val, iv)) {
		g_density.tes_lo = iv;
	} else if(key == "tes_hi" && parse_int_val(val, iv)) {
		g_density.tes_hi = iv;
	} else if(key == "adds_lo" && parse_int_val(val, iv)) {
		g_density.adds_lo = iv;
	} else if(key == "adds_hi" && parse_int_val(val, iv)) {
		g_density.adds_hi = iv;
	} else if(key == "depth_lo" && parse_int_val(val, iv)) {
		g_density.depth_lo = iv;
	} else if(key == "depth_hi" && parse_int_val(val, iv)) {
		g_density.depth_hi = iv;
	} else if(key == "br_lo" && parse_int_val(val, iv)) {
		g_density.br_lo = iv;
	} else if(key == "br_hi" && parse_int_val(val, iv)) {
		g_density.br_hi = iv;
	} else if(key == "bc_lo" && parse_int_val(val, iv)) {
		g_density.bc_lo = iv;
	} else if(key == "bc_hi" && parse_int_val(val, iv)) {
		g_density.bc_hi = iv;
	} else if(key == "sec_corr" && parse_int_val(val, iv)) {
		g_density.sec_corr = iv;
	} else if(key == "sec_tre" && parse_int_val(val, iv)) {
		g_density.sec_tre = iv;
	} else if(key == "sec_trap" && parse_int_val(val, iv)) {
		g_density.sec_trap = iv;
	} else if(key == "solo_rooms_pct" && parse_int_val(val, iv)) {
		g_density.solo_rooms_pct = iv;
	} else if(key == "solo_rooms_min_floor" && parse_int_val(val, iv)) {
		g_density.solo_rooms_min_floor = iv;
	} else if(key == "solo_rooms_max_floor" && parse_int_val(val, iv)) {
		g_density.solo_rooms_max_floor = iv;
	} else if(key == "solo_branches_delta" && parse_int_val(val, iv)) {
		g_density.solo_branches_delta = iv;
	} else if(key == "solo_branch_chance_delta" && parse_int_val(val, iv)) {
		g_density.solo_branch_chance_delta = iv;
	} else if(key == "solo_branch_chance_floor" && parse_int_val(val, iv)) {
		g_density.solo_branch_chance_floor = iv;
	} else if(key == "solo_corr_pct" && parse_int_val(val, iv)) {
		g_density.solo_corr_pct = iv;
	} else if(key == "solo_corr_floor" && parse_int_val(val, iv)) {
		g_density.solo_corr_floor = iv;
	} else if(key == "solo_tes_delta" && parse_int_val(val, iv)) {
		g_density.solo_tes_delta = iv;
	} else if(key == "solo_tes_floor" && parse_int_val(val, iv)) {
		g_density.solo_tes_floor = iv;
	} else if(key == "solo_adds_delta" && parse_int_val(val, iv)) {
		g_density.solo_adds_delta = iv;
	} else if(key == "solo_depth_delta" && parse_int_val(val, iv)) {
		g_density.solo_depth_delta = iv;
	} else if(key == "solo_depth_floor" && parse_int_val(val, iv)) {
		g_density.solo_depth_floor = iv;
	}
}

void apply_rewards_key(const std::string& key, const std::string& val) {
	int iv = 0;
	float fv = 0.0f;
	if(key.rfind("gear_base_", 0) == 0 && parse_int_val(val, iv)) {
		const int idx = std::atoi(key.c_str() + 10);
		if(idx >= 0 && idx < PROCAREA_FATIGUE_TIER_COUNT) {
			g_rewards.fatigue_gear_base[idx] = iv;
		}
	} else if(key.rfind("gear_decay_", 0) == 0 && parse_int_val(val, iv)) {
		const int idx = std::atoi(key.c_str() + 11);
		if(idx >= 0 && idx < PROCAREA_FATIGUE_TIER_COUNT) {
			g_rewards.fatigue_gear_decay[idx] = iv;
		}
	} else if(key.rfind("gold_pct_", 0) == 0 && parse_int_val(val, iv)) {
		const int idx = std::atoi(key.c_str() + 9);
		if(idx >= 0 && idx < PROCAREA_FATIGUE_TIER_COUNT) {
			g_rewards.fatigue_gold_pct[idx] = iv;
		}
	} else if(key == "frag_drop_corr" && parse_int_val(val, iv)) {
		g_rewards.frag_drop_pct[0] = iv;
	} else if(key == "frag_drop_tes" && parse_int_val(val, iv)) {
		g_rewards.frag_drop_pct[1] = iv;
	} else if(key == "frag_drop_add" && parse_int_val(val, iv)) {
		g_rewards.frag_drop_pct[2] = iv;
	} else if(key == "frag_drop_trap" && parse_int_val(val, iv)) {
		g_rewards.frag_drop_pct[3] = iv;
	} else if(key == "frag_drop_boss" && parse_int_val(val, iv)) {
		g_rewards.frag_drop_pct[4] = iv;
	} else if(key == "frag_qty_corr" && parse_int_val(val, iv)) {
		g_rewards.frag_qty_mult[0] = iv;
	} else if(key == "frag_qty_tes" && parse_int_val(val, iv)) {
		g_rewards.frag_qty_mult[1] = iv;
	} else if(key == "frag_qty_add" && parse_int_val(val, iv)) {
		g_rewards.frag_qty_mult[2] = iv;
	} else if(key == "frag_qty_trap" && parse_int_val(val, iv)) {
		g_rewards.frag_qty_mult[3] = iv;
	} else if(key == "frag_qty_boss" && parse_int_val(val, iv)) {
		g_rewards.frag_qty_mult[4] = iv;
	} else if(key == "frag_qty_bias" && parse_float_val(val, fv)) {
		g_rewards.frag_qty_bias = fv;
	} else if(key == "frag_per_rune" && parse_int_val(val, iv)) {
		g_rewards.fragments_per_rune = iv;
	} else {
		for(int i = 0; i < PROCAREA_CRYSTAL_COUNT; ++i) {
			const std::string p = std::string("crystal_") + kCrystalNames[i] + "_";
			if(key == p + "mob" && parse_float_val(val, fv)) {
				g_rewards.crystal[i].mob_mult = fv;
			} else if(key == p + "frag" && parse_float_val(val, fv)) {
				g_rewards.crystal[i].frag_mult = fv;
			} else if(key == p + "drop_corr" && parse_int_val(val, iv)) {
				g_rewards.crystal[i].frag_drop_corridor_pct = iv;
			} else if(key == p + "drop_tes" && parse_int_val(val, iv)) {
				g_rewards.crystal[i].frag_drop_treasure_pct = iv;
			}
		}
	}
}

void apply_levels_key(const std::string& key, const std::string& val) {
	int iv = 0;
	if(!parse_int_val(val, iv)) {
		return;
	}
	if(key.rfind("fascia_", 0) == 0) {
		const int idx = std::atoi(key.c_str() + 7);
		if(idx >= 0 && idx < PROCAREA_TEMPLATE_BANDS) {
			g_levels.fascia[idx] = iv;
		}
	} else if(key == "boss_bonus") {
		g_levels.boss_bonus = iv;
	} else if(key == "trap_bonus_lo") {
		g_levels.trap_bonus_lo = iv;
	} else if(key == "trap_bonus_hi") {
		g_levels.trap_bonus_hi = iv;
	}
}

#if USE_MYSQL
void load_from_db() {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		int loaded = 0;
		for(const auto& row : db->query<procarea_balance>()) {
			const std::string& key = row.conf_key;
			const std::string& val = row.conf_value;
			if(key.rfind("d_", 0) == 0) {
				apply_density_key(key.substr(2), val);
				++loaded;
			} else if(key.rfind("r_", 0) == 0) {
				apply_rewards_key(key.substr(2), val);
				++loaded;
			} else if(key.rfind("l_", 0) == 0) {
				apply_levels_key(key.substr(2), val);
				++loaded;
			}
		}
		t.commit();
		sanitize_density(g_density);
		sanitize_rewards(g_rewards);
		sanitize_levels(g_levels);
		if(loaded > 0) {
			mudlog(LOG_CHECK, "procarea_balance: loaded %d keys from DB", loaded);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "procarea_balance load: %s", e.what());
	}
}

void save_map_to_db(const KeyMap& density_map, const KeyMap& rewards_map,
					const KeyMap& levels_map) {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->erase_query<procarea_balance>(odb::query<procarea_balance>(true));
		const boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
		auto persist_map = [&](const char* prefix, const KeyMap& map) {
			for(const auto& kv : map) {
				procarea_balance row;
				row.conf_key = std::string(prefix) + kv.first;
				row.conf_value = kv.second;
				row.updated_at = now;
				db->persist(row);
			}
		};
		persist_map("d_", density_map);
		persist_map("r_", rewards_map);
		persist_map("l_", levels_map);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "procarea_balance save: %s", e.what());
	}
}
#endif

void dump_density(char_data* ch) {
	const ProcDensityConfig& d = g_density;
	std::ostringstream os;
	os << "$c0014=== Densita' Dimensione (MID runtime) ===$c0007\n\r"
	   << "Vale solo per le $c0010prossime$c0007 istanze (gruppo e solitaria).\n\r"
	   << "bias=" << d.bias << "\n\r"
	   << "rooms_min " << d.rooms_min_lo << ".." << d.rooms_min_hi << " | rooms_max "
	   << d.rooms_max_lo << ".." << d.rooms_max_hi << "\n\r"
	   << "corr " << d.corr_lo << ".." << d.corr_hi << " | tes " << d.tes_lo << ".." << d.tes_hi
	   << " | adds " << d.adds_lo << ".." << d.adds_hi << "\n\r"
	   << "depth " << d.depth_lo << ".." << d.depth_hi << " | branches " << d.br_lo << ".."
	   << d.br_hi << " | branch_chance " << d.bc_lo << ".." << d.bc_hi << "\n\r"
	   << "sec_corr=" << d.sec_corr << " sec_tre=" << d.sec_tre << " sec_trap=" << d.sec_trap
	   << "\n\r"
	   << "solo: rooms_pct=" << d.solo_rooms_pct << " floors " << d.solo_rooms_min_floor << "/"
	   << d.solo_rooms_max_floor << " br_delta=" << d.solo_branches_delta
	   << " bc_delta/floor=" << d.solo_branch_chance_delta << "/" << d.solo_branch_chance_floor
	   << "\n\r"
	   << "solo: corr_pct/floor=" << d.solo_corr_pct << "/" << d.solo_corr_floor
	   << " tes_delta/floor=" << d.solo_tes_delta << "/" << d.solo_tes_floor
	   << " adds_delta=" << d.solo_adds_delta << " depth_delta/floor=" << d.solo_depth_delta << "/"
	   << d.solo_depth_floor << "\n\r"
	   << "Uso: $c0014dimensione densita set <chiave> <val>$c0007 | "
		  "$c0014bias <n>$c0007 | $c0014reset$c0007\n\r";
	send_to_char(os.str().c_str(), ch);
}

void dump_rewards(char_data* ch) {
	const ProcRewardsConfig& r = g_rewards;
	std::ostringstream os;
	os << "$c0014=== Premi Dimensione (runtime) ===$c0007\n\r"
	   << "Vale per loot/frammenti delle $c0010prossime$c0007 uccisioni/istanze.\n\r"
	   << "fatigue gear_base:";
	for(int i = 0; i < PROCAREA_FATIGUE_TIER_COUNT; ++i) {
		os << " t" << i << "=" << r.fatigue_gear_base[i];
	}
	os << "\n\rfatigue gear_decay:";
	for(int i = 0; i < PROCAREA_FATIGUE_TIER_COUNT; ++i) {
		os << " t" << i << "=" << r.fatigue_gear_decay[i];
	}
	os << "\n\rfatigue gold_pct:";
	for(int i = 0; i < PROCAREA_FATIGUE_TIER_COUNT; ++i) {
		os << " t" << i << "=" << r.fatigue_gold_pct[i];
	}
	os << "\n\rfrag_drop corr/tes/add/trap/boss: " << r.frag_drop_pct[0] << "/" << r.frag_drop_pct[1]
	   << "/" << r.frag_drop_pct[2] << "/" << r.frag_drop_pct[3] << "/" << r.frag_drop_pct[4]
	   << "\n\rfrag_qty  corr/tes/add/trap/boss: " << r.frag_qty_mult[0] << "/" << r.frag_qty_mult[1]
	   << "/" << r.frag_qty_mult[2] << "/" << r.frag_qty_mult[3] << "/" << r.frag_qty_mult[4]
	   << "\n\rfrag_qty_bias=" << r.frag_qty_bias << " frag_per_rune=" << r.fragments_per_rune
	   << "\n\r";
	for(int i = 0; i < PROCAREA_CRYSTAL_COUNT; ++i) {
		os << "crystal_" << kCrystalNames[i] << ": mob x" << r.crystal[i].mob_mult << " frag x"
		   << r.crystal[i].frag_mult << " drop " << r.crystal[i].frag_drop_corridor_pct << "/"
		   << r.crystal[i].frag_drop_treasure_pct << "\n\r";
	}
	os << "Uso: $c0014dimensione premi set <chiave> <val>$c0007 | $c0014reset$c0007\n\r"
	   << "Chiavi: gear_base_N, gear_decay_N, gold_pct_N, frag_drop_*, frag_qty_*, "
		  "frag_qty_bias, frag_per_rune, crystal_<colore>_mob/frag/drop_corr/drop_tes\n\r";
	send_to_char(os.str().c_str(), ch);
}

void densita_help(char_data* ch) {
	send_to_char(
		"$c0014dimensione densita$c0007 - mostra config\n\r"
		"$c0014dimensione densita bias <0.5-1.5>$c0007 - moltiplicatore globale\n\r"
		"$c0014dimensione densita set <chiave> <val>$c0007 - ritocca un parametro\n\r"
		"$c0014dimensione densita reset$c0007 - torna al profilo MID di default\n\r"
		"Chiavi: rooms_min_lo/hi, rooms_max_lo/hi, corr_lo/hi, tes_lo/hi, adds_lo/hi,\n\r"
		"depth_lo/hi, br_lo/hi, bc_lo/hi, sec_corr/tre/trap, solo_*\n\r"
		"Effetto: solo istanze $c0010nuove$c0007 (gruppo e touch-fontana).\n\r"
		"Esempi: $c0014wizhelp dimensione densita esempi$c0007\n\r",
		ch);
}

void premi_help(char_data* ch) {
	send_to_char(
		"$c0014dimensione premi$c0007 - mostra config premi numerici\n\r"
		"$c0014dimensione premi set <chiave> <val>$c0007\n\r"
		"$c0014dimensione premi reset$c0007 - default\n\r"
		"No editor oggetti: solo %, moltiplicatori, conversione frammenti.\n\r"
		"\n\r"
		"$c0010Edit loot procarea (staff):$c0007\n\r"
		"  1) osave <obj> db procarea   (subito dopo il drop, con bonus rolled)\n\r"
		"  2) oedit <obj>\n\r"
		"  3) osave <obj> db            (update MySQL)\n\r"
		"  4) personalize <obj> <pg>\n\r"
		"  Non usare osave <obj> 651xx su file (prototipo condiviso).\n\r"
		"Esempi: $c0014wizhelp dimensione premi esempi$c0007\n\r",
		ch);
}

void dump_levels(char_data* ch) {
	const ProcLevelConfig& L = g_levels;
	std::ostringstream os;
	os << "$c0014=== Livelli Dimensione (runtime, default B1.5) ===$c0007\n\r"
	   << "Vale per i mob delle $c0010prossime$c0007 istanze.\n\r"
	   << "normale = (group_max_level - 1) + fascia[band]; trap +lo..hi; boss +bonus.\n\r"
	   << "fascia bande 0..9:";
	for(int i = 0; i < PROCAREA_TEMPLATE_BANDS; ++i) {
		os << " " << L.fascia[i];
	}
	os << "\n\rboss_bonus=" << L.boss_bonus << " trap_bonus=" << L.trap_bonus_lo << ".."
	   << L.trap_bonus_hi << "\n\r"
	   << "Esempio PG 51 (normali):";
	for(int band = 0; band < PROCAREA_TEMPLATE_BANDS; ++band) {
		const int n = (51 - 1) + L.fascia[band];
		os << " b" << band << "=" << n;
	}
	os << "\n\rUso: $c0014dimensione livelli set <chiave> <val>$c0007 | $c0014reset$c0007\n\r"
	   << "Chiavi: fascia_0..fascia_9, boss_bonus, trap_bonus_lo, trap_bonus_hi\n\r";
	send_to_char(os.str().c_str(), ch);
}

void livelli_help(char_data* ch) {
	send_to_char(
		"$c0014dimensione livelli$c0007 - mostra fascia livelli per banda\n\r"
		"$c0014dimensione livelli set <chiave> <val>$c0007\n\r"
		"$c0014dimensione livelli reset$c0007 - torna a B1.5 (1,1,1,1,2,2,2,3,3,4)\n\r"
		"Chiavi: fascia_0..fascia_9 (0-8), boss_bonus, trap_bonus_lo/hi\n\r"
		"Effetto: solo istanze $c0010nuove$c0007.\n\r"
		"Esempi: $c0014wizhelp dimensione livelli esempi$c0007\n\r",
		ch);
}

[[nodiscard]] bool handle_levels_wiz(char_data* ch, const char* rest) {
	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	std::array<char, MAX_INPUT_LENGTH> arg3{};
	const char* p = rest != nullptr ? rest : "";
	p = one_argument(p, arg1.data());
	p = one_argument(p, arg2.data());
	one_argument(p, arg3.data());

	if(arg1[0] == '\0') {
		dump_levels(ch);
		return true;
	}
	if(!strcasecmp(arg1.data(), "help") || !strcasecmp(arg1.data(), "?")) {
		livelli_help(ch);
		return true;
	}
	if(!strcasecmp(arg1.data(), "reset")) {
		procarea_balance_reset_levels();
		procarea_balance_save();
		send_to_char("Livelli ripristinati a B1.5 e salvati.\n\r", ch);
		mudlog(LOG_CHECK, "procarea_balance: %s reset livelli", GET_NAME(ch));
		return true;
	}
	if(!strcasecmp(arg1.data(), "set")) {
		if(arg2[0] == '\0' || arg3[0] == '\0') {
			send_to_char("Uso: dimensione livelli set <chiave> <valore>\n\r", ch);
			return true;
		}
		KeyMap probe;
		export_levels(probe);
		if(probe.find(arg2.data()) == probe.end()) {
			send_to_char("Chiave livelli sconosciuta. Vedi: dimensione livelli help\n\r", ch);
			return true;
		}
		apply_levels_key(arg2.data(), arg3.data());
		sanitize_levels(g_levels);
		procarea_balance_save();
		std::ostringstream os;
		os << "Impostato " << arg2.data() << "=" << arg3.data()
		   << " (clamp/salvato; prossime istanze).\n\r";
		send_to_char(os.str().c_str(), ch);
		mudlog(LOG_CHECK, "procarea_balance: %s livelli set %s=%s", GET_NAME(ch), arg2.data(),
			   arg3.data());
		return true;
	}
	send_to_char("Sotto-comandi: (vuoto) | help | set | reset\n\r", ch);
	return true;
}

[[nodiscard]] bool handle_density_wiz(char_data* ch, const char* rest) {
	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	std::array<char, MAX_INPUT_LENGTH> arg3{};
	const char* p = rest != nullptr ? rest : "";
	p = one_argument(p, arg1.data());
	p = one_argument(p, arg2.data());
	one_argument(p, arg3.data());

	if(arg1[0] == '\0' || !strcasecmp(arg1.data(), "help") || !strcasecmp(arg1.data(), "?")) {
		if(arg1[0] == '\0') {
			dump_density(ch);
		} else {
			densita_help(ch);
		}
		return true;
	}
	if(!strcasecmp(arg1.data(), "reset")) {
		procarea_balance_reset_density();
		procarea_balance_save();
		send_to_char("Densita' ripristinata al profilo MID e salvata.\n\r", ch);
		mudlog(LOG_CHECK, "procarea_balance: %s reset densita", GET_NAME(ch));
		return true;
	}
	if(!strcasecmp(arg1.data(), "bias")) {
		float fv = 0.0f;
		if(!parse_float_val(arg2.data(), fv)) {
			send_to_char("Uso: dimensione densita bias <0.5-1.5>\n\r", ch);
			return true;
		}
		g_density.bias = fv;
		sanitize_density(g_density);
		procarea_balance_save();
		std::ostringstream os;
		os << "bias=" << g_density.bias << " (salvato; vale dalle prossime istanze).\n\r";
		send_to_char(os.str().c_str(), ch);
		mudlog(LOG_CHECK, "procarea_balance: %s densita bias=%.3f", GET_NAME(ch), g_density.bias);
		return true;
	}
	if(!strcasecmp(arg1.data(), "set")) {
		if(arg2[0] == '\0' || arg3[0] == '\0') {
			send_to_char("Uso: dimensione densita set <chiave> <valore>\n\r", ch);
			return true;
		}
		KeyMap probe;
		export_density(probe);
		if(probe.find(arg2.data()) == probe.end()) {
			send_to_char("Chiave densita' sconosciuta. Vedi: dimensione densita help\n\r", ch);
			return true;
		}
		apply_density_key(arg2.data(), arg3.data());
		sanitize_density(g_density);
		procarea_balance_save();
		std::ostringstream os;
		os << "Impostato " << arg2.data() << "=" << arg3.data()
		   << " (clamp/salvato; prossime istanze).\n\r";
		send_to_char(os.str().c_str(), ch);
		mudlog(LOG_CHECK, "procarea_balance: %s densita set %s=%s", GET_NAME(ch), arg2.data(),
			   arg3.data());
		return true;
	}
	send_to_char("Sotto-comandi: (vuoto) | help | bias | set | reset\n\r", ch);
	return true;
}

[[nodiscard]] bool handle_rewards_wiz(char_data* ch, const char* rest) {
	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	std::array<char, MAX_INPUT_LENGTH> arg3{};
	const char* p = rest != nullptr ? rest : "";
	p = one_argument(p, arg1.data());
	p = one_argument(p, arg2.data());
	one_argument(p, arg3.data());

	if(arg1[0] == '\0') {
		dump_rewards(ch);
		return true;
	}
	if(!strcasecmp(arg1.data(), "help") || !strcasecmp(arg1.data(), "?")) {
		premi_help(ch);
		return true;
	}
	if(!strcasecmp(arg1.data(), "reset")) {
		procarea_balance_reset_rewards();
		procarea_balance_save();
		send_to_char("Premi ripristinati ai default e salvati.\n\r", ch);
		mudlog(LOG_CHECK, "procarea_balance: %s reset premi", GET_NAME(ch));
		return true;
	}
	if(!strcasecmp(arg1.data(), "set")) {
		if(arg2[0] == '\0' || arg3[0] == '\0') {
			send_to_char("Uso: dimensione premi set <chiave> <valore>\n\r", ch);
			return true;
		}
		KeyMap probe;
		export_rewards(probe);
		if(probe.find(arg2.data()) == probe.end()) {
			send_to_char("Chiave premi sconosciuta. Vedi: dimensione premi help\n\r", ch);
			return true;
		}
		apply_rewards_key(arg2.data(), arg3.data());
		sanitize_rewards(g_rewards);
		procarea_balance_save();
		std::ostringstream os;
		os << "Impostato " << arg2.data() << "=" << arg3.data()
		   << " (clamp/salvato).\n\r";
		send_to_char(os.str().c_str(), ch);
		mudlog(LOG_CHECK, "procarea_balance: %s premi set %s=%s", GET_NAME(ch), arg2.data(),
			   arg3.data());
		return true;
	}
	send_to_char("Sotto-comandi: (vuoto) | help | set | reset\n\r", ch);
	return true;
}

} // namespace

const ProcDensityConfig& procarea_density_config() {
	return g_density;
}

const ProcRewardsConfig& procarea_rewards_config() {
	return g_rewards;
}

const ProcLevelConfig& procarea_level_config() {
	return g_levels;
}

ProcDensityConfig& procarea_density_config_mut() {
	return g_density;
}

ProcRewardsConfig& procarea_rewards_config_mut() {
	return g_rewards;
}

ProcLevelConfig& procarea_level_config_mut() {
	return g_levels;
}

int procarea_fragments_per_rune() {
	return g_rewards.fragments_per_rune;
}

void procarea_balance_reset_density() {
	g_density = ProcDensityConfig{};
	sanitize_density(g_density);
}

void procarea_balance_reset_rewards() {
	g_rewards = ProcRewardsConfig{};
	sanitize_rewards(g_rewards);
}

void procarea_balance_reset_levels() {
	g_levels = ProcLevelConfig{};
	sanitize_levels(g_levels);
}

void procarea_balance_save() {
	sanitize_density(g_density);
	sanitize_rewards(g_rewards);
	sanitize_levels(g_levels);
#if USE_MYSQL
	KeyMap dmap;
	KeyMap rmap;
	KeyMap lmap;
	export_density(dmap);
	export_rewards(rmap);
	export_levels(lmap);
	save_map_to_db(dmap, rmap, lmap);
#endif
}

void procarea_balance_boot() {
	procarea_balance_reset_density();
	procarea_balance_reset_rewards();
	procarea_balance_reset_levels();
#if USE_MYSQL
	load_from_db();
#endif
	mudlog(LOG_CHECK,
		   "procarea_balance: ready (bias=%.2f rooms_max_hi=%d fascia9=%d)", g_density.bias,
		   g_density.rooms_max_hi, g_levels.fascia[PROCAREA_TEMPLATE_BANDS - 1]);
}

bool procarea_try_balance_wiz_command(char_data* ch, const char* subcmd, const char* rest) {
	if(ch == nullptr || subcmd == nullptr) {
		return false;
	}
	if(!strcasecmp(subcmd, "densita") || !strcasecmp(subcmd, "density")) {
		if(!is_balance_wiz(ch)) {
			send_to_char("Solo gli immortali possono modificare la densita'.\n\r", ch);
			return true;
		}
		return handle_density_wiz(ch, rest);
	}
	if(!strcasecmp(subcmd, "premi") || !strcasecmp(subcmd, "rewards") ||
	   !strcasecmp(subcmd, "loot")) {
		if(!is_balance_wiz(ch)) {
			send_to_char("Solo gli immortali possono modificare i premi.\n\r", ch);
			return true;
		}
		return handle_rewards_wiz(ch, rest);
	}
	if(!strcasecmp(subcmd, "livelli") || !strcasecmp(subcmd, "levels") ||
	   !strcasecmp(subcmd, "level")) {
		if(!is_balance_wiz(ch)) {
			send_to_char("Solo gli immortali possono modificare i livelli.\n\r", ch);
			return true;
		}
		return handle_levels_wiz(ch, rest);
	}
	return false;
}

} // namespace Alarmud
