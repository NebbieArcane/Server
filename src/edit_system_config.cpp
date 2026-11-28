/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "edit_system_config.hpp"

#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cerrno>
#include <filesystem>
#include <system_error>

#include "../contrib/slacking/json.hpp"
#include "autoenums.hpp"
#include "edit_pool.hpp"
#include "logging.hpp"

namespace Alarmud {
namespace {

using Json = nlohmann::json;

struct Entry {
	std::string id;
	std::string kind;
	std::string pool_field;
	unsigned damage_type = 0;
	int location = 0;
	EditSystemTarget target = EditSystemTarget::Object;
	bool enabled = true;
	std::string label;
};

	std::mutex g_mutex;
	std::vector<Entry> g_entries;
	std::string g_config_path;
	bool g_loaded = false;

	struct PortalTypeDef {
		const char* slug;
		const char* label;
		bool default_enabled;
	};

	static const PortalTypeDef kPortalTypeDefs[] = {
		{"light", "Light", false},
		{"scroll", "Scroll", false},
		{"wand", "Wand", false},
		{"staff", "Staff", false},
		{"weapon", "Weapon", true},
		{"fireweapon", "Fire weapon", false},
		{"missile", "Missile", false},
		{"treasure", "Treasure", true},
		{"armor", "Armor", true},
		{"potion", "Potion", false},
		{"worn", "Worn (medaglie, amuleti, bracciali)", true},
		{"other", "Other", false},
		{"trash", "Trash", false},
		{"trap", "Trap", false},
		{"container", "Container", false},
		{"note", "Note", false},
		{"drinkcon", "Drink container", false},
		{"key", "Key", false},
		{"food", "Food", false},
		{"money", "Money", false},
		{"pen", "Pen", false},
		{"boat", "Boat", false},
		{"audio", "Audio", false},
		{"board", "Board", false},
		{"tree", "Tree", false},
		{"rock", "Rock", false},
		{"m_gem", "Mineral gem", false},
		{"m_mineral", "Mineral", false},
		{"bar", "Bar", false},
		{"jewel", "Jewel", true},
		{"clan_symbol", "Clan symbol", false},
	};

	std::unordered_map<std::string, bool> g_portal_type_enabled;

	struct CurrencyDef {
		std::string slug;
		std::string label;
		bool enabled = false;
		bool visible = false;
		bool pays_listino = false;
	};

	std::vector<CurrencyDef> g_currencies;

	[[nodiscard]] std::vector<CurrencyDef> default_currencies() {
		return {
			{"mxp", "MXP", true, true, true},
			{"rune", "Rune degli eroi", true, true, true},
			{"gold", "Gold", false, false, false},
			{"token", "Token", false, false, false},
			{"credit", "Credito edit", false, false, false},
		};
	}

	void currencies_reset_defaults() {
		g_currencies = default_currencies();
	}

	void portal_type_reset_defaults() {
		g_portal_type_enabled.clear();
		for(const auto& def : kPortalTypeDefs) {
			g_portal_type_enabled[def.slug] = def.default_enabled;
		}
	}

[[nodiscard]] EditSystemTarget parse_target(const std::string& s) {
	if(s == "character") {
		return EditSystemTarget::Character;
	}
	return EditSystemTarget::Object;
}

[[nodiscard]] const char* target_str(EditSystemTarget t) {
	return t == EditSystemTarget::Character ? "character" : "object";
}

void build_defaults() {
	portal_type_reset_defaults();
	currencies_reset_defaults();
	g_entries.clear();
	const char* pool_fields[] = {"hit", "mana", "move", "hit_regen", "mana_regen", "move_regen"};
	const char* pool_labels[] = {"Hit points", "Mana", "Move", "Hit regen", "Mana regen", "Move regen"};
	for(int i = 0; i < 6; ++i) {
		Entry e;
		e.id = std::string("pool.") + pool_fields[i];
		e.kind = "pool";
		e.pool_field = pool_fields[i];
		e.target = EditSystemTarget::Character;
		e.enabled = true;
		e.label = pool_labels[i];
		g_entries.push_back(e);
	}

	static const struct {
		unsigned bit;
		const char* slug;
		const char* label;
	} resist[] = {
		{IMM_FIRE, "fire", "Fire"},
		{IMM_COLD, "cold", "Cold"},
		{IMM_ELEC, "elec", "Electricity"},
		{IMM_ENERGY, "energy", "Energy"},
		{IMM_BLUNT, "blunt", "Blunt"},
		{IMM_PIERCE, "pierce", "Pierce"},
		{IMM_SLASH, "slash", "Slash"},
		{IMM_ACID, "acid", "Acid"},
		{IMM_POISON, "poison", "Poison"},
		{IMM_DRAIN, "drain", "Drain"},
		{IMM_SLEEP, "sleep", "Sleep"},
		{IMM_CHARM, "charm", "Charm"},
		{IMM_HOLD, "hold", "Hold"},
		{IMM_NONMAG, "nonmag", "Non-magic"},
		/* Weapon +N (IMM_PLUS*): resistenze mob, non listino toon — disabilitate. */
		{IMM_PLUS1, "plus1", "Weapon +1 (solo mob)"},
		{IMM_PLUS2, "plus2", "Weapon +2 (solo mob)"},
		{IMM_PLUS3, "plus3", "Weapon +3 (solo mob)"},
		{IMM_PLUS4, "plus4", "Weapon +4 (solo mob)"},
	};
	for(const auto& r : resist) {
		Entry e;
		e.id = std::string("resistance.") + r.slug;
		e.kind = "resistance";
		e.damage_type = r.bit;
		e.target = EditSystemTarget::Object;
		/* PLUS* non sono edit PG; restano in config staff ma off di default. */
		e.enabled = !(r.bit == IMM_PLUS1 || r.bit == IMM_PLUS2 || r.bit == IMM_PLUS3 ||
					  r.bit == IMM_PLUS4);
		e.label = r.label;
		g_entries.push_back(e);
	}
}

[[nodiscard]] std::string myst_lib_config_path() {
	namespace fs = std::filesystem;
	try {
		if(fs::exists("/proc/self/exe")) {
			const fs::path exe = fs::read_symlink("/proc/self/exe");
			const fs::path mudroot = exe.parent_path();
			return (mudroot / "lib" / "edit_system.json").string();
		}
	}
	catch(...) {
	}
	return "edit_system.json";
}

[[nodiscard]] std::string resolve_config_path() {
	const char* env = std::getenv("EDIT_SYSTEM_CONFIG");
	if(env && *env) {
		return std::string(env);
	}
	return myst_lib_config_path();
}

[[nodiscard]] bool read_file_text(const std::string& path, std::string& out) {
	std::ifstream in(path);
	if(!in) {
		return false;
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	out = ss.str();
	return true;
}

[[nodiscard]] bool write_file_text(const std::string& path, const std::string& text) {
	namespace fs = std::filesystem;
	try {
		const fs::path p(path);
		if(p.has_parent_path()) {
			std::error_code ec;
			fs::create_directories(p.parent_path(), ec);
		}
	}
	catch(...) {
	}
	std::ofstream out(path, std::ios::trunc);
	if(!out) {
		return false;
	}
	out << text;
	return out.good();
}

void parse_portal_categories(const Json& root) {
	portal_type_reset_defaults();
	if(root.find("object_portal") == root.end() || !root["object_portal"].is_object()) {
		return;
	}
	const Json& p = root["object_portal"];
	if(p.find("types") != p.end() && p["types"].is_object()) {
		const Json& types = p["types"];
		for(auto it = types.begin(); it != types.end(); ++it) {
			if(it->is_boolean()) {
				g_portal_type_enabled[it.key()] = it->get<bool>();
			}
		}
	}
	// Legacy flat keys (armor/weapon)
	if(p.find("armor") != p.end()) {
		g_portal_type_enabled["armor"] = p.value("armor", true);
	}
	if(p.find("weapon") != p.end()) {
		g_portal_type_enabled["weapon"] = p.value("weapon", true);
	}
}

void parse_currencies(const Json& root) {
	currencies_reset_defaults();
	if(root.find("currencies") == root.end() || !root["currencies"].is_object()) {
		return;
	}
	const Json& c = root["currencies"];
	if(c.find("catalog") == c.end() || !c["catalog"].is_array()) {
		return;
	}
	std::unordered_map<std::string, size_t> index;
	for(size_t i = 0; i < g_currencies.size(); ++i) {
		index[g_currencies[i].slug] = i;
	}
	for(const auto& item : c["catalog"]) {
		if(!item.is_object()) {
			continue;
		}
		const std::string slug = item.value("slug", "");
		if(slug.empty()) {
			continue;
		}
		CurrencyDef row;
		row.slug = slug;
		row.label = item.value("label", slug);
		row.enabled = item.value("enabled", false);
		row.visible = item.value("visible", false);
		row.pays_listino = item.value("pays_listino", false);
		const auto it = index.find(slug);
		if(it != index.end()) {
			g_currencies[it->second] = row;
		}
		else {
			index[slug] = g_currencies.size();
			g_currencies.push_back(row);
		}
	}
}

void parse_entries_json(const Json& root) {
	g_entries.clear();
	parse_portal_categories(root);
	parse_currencies(root);
	if(root.find("entries") == root.end() || !root["entries"].is_array()) {
		/* Defaults entries, ma conserva object_portal/currencies gia' parsati. */
		const auto portal_types = g_portal_type_enabled;
		const auto currencies = g_currencies;
		build_defaults();
		g_portal_type_enabled = portal_types;
		g_currencies = currencies;
		return;
	}
	for(const auto& item : root["entries"]) {
		if(!item.is_object()) {
			continue;
		}
		Entry e;
		e.id = item.value("id", "");
		e.kind = item.value("kind", "");
		e.pool_field = item.value("pool_field", "");
		e.damage_type = static_cast<unsigned>(item.value("damage_type", 0));
		e.location = item.value("location", 0);
		e.target = parse_target(item.value("target", "object"));
		e.enabled = item.value("enabled", true);
		e.label = item.value("label", "");
		if(e.id.empty() || e.kind.empty()) {
			continue;
		}
		g_entries.push_back(e);
	}
	if(g_entries.empty()) {
		build_defaults();
	}
}

[[nodiscard]] Json entries_to_json() {
	Json root;
	root["version"] = 1;
	Json arr = Json::array();
	for(const auto& e : g_entries) {
		Json item;
		item["id"] = e.id;
		item["kind"] = e.kind;
		if(!e.pool_field.empty()) {
			item["pool_field"] = e.pool_field;
		}
		if(e.damage_type != 0) {
			item["damage_type"] = e.damage_type;
		}
		if(e.location != 0) {
			item["location"] = e.location;
		}
		item["target"] = target_str(e.target);
		item["enabled"] = e.enabled;
		if(!e.label.empty()) {
			item["label"] = e.label;
		}
		arr.push_back(item);
	}
	root["entries"] = arr;
	Json portal;
	Json types = Json::object();
	for(const auto& def : kPortalTypeDefs) {
		const auto it = g_portal_type_enabled.find(def.slug);
		types[def.slug] = it != g_portal_type_enabled.end() ? it->second : def.default_enabled;
	}
	portal["types"] = types;
	Json catalog = Json::array();
	for(const auto& def : kPortalTypeDefs) {
		Json row;
		row["slug"] = def.slug;
		row["label"] = def.label;
		const auto it = g_portal_type_enabled.find(def.slug);
		row["enabled"] =
			it != g_portal_type_enabled.end() ? it->second : def.default_enabled;
		catalog.push_back(row);
	}
	portal["type_catalog"] = catalog;
	portal["comment"] =
		"types: slug ITEM_* — spunta = visibile in inventario e editabile "
		"(primo edit). Senza spunta: nascosto, salvo pezzi gia' personalizzati "
		"(EDIT / instance / owner) che restano visibili per ri-edit. "
		"Sempre esclusi RARO/TAN/HAS-GEMS/simbolo. Edit OK anche indossati "
		"(PG offline).";
	root["object_portal"] = portal;
	Json currencies;
	Json currency_catalog = Json::array();
	for(const auto& c : g_currencies) {
		Json row;
		row["slug"] = c.slug;
		row["label"] = c.label;
		row["enabled"] = c.enabled;
		row["visible"] = c.visible;
		row["pays_listino"] = c.pays_listino;
		currency_catalog.push_back(row);
	}
	currencies["catalog"] = currency_catalog;
	currencies["comment"] =
		"Valute portale. visible=false nasconde ai player. "
		"pays_listino oggi solo mxp/rune.";
	root["currencies"] = currencies;
	return root;
}

struct PoolLookup {
	bool found = false;
	bool enabled = true;
	EditSystemTarget target = EditSystemTarget::Character;
};

struct ResistanceLookup {
	bool found = false;
	bool enabled = true;
	EditSystemTarget target = EditSystemTarget::Object;
};

[[nodiscard]] PoolLookup lookup_pool(const char* pool_field) {
	PoolLookup out;
	if(!pool_field || !*pool_field) {
		return out;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	for(const auto& e : g_entries) {
		if(e.kind == "pool" && e.pool_field == pool_field) {
			out.found = true;
			out.enabled = e.enabled;
			out.target = e.target;
			return out;
		}
	}
	return out;
}

[[nodiscard]] ResistanceLookup lookup_resistance(unsigned damage_type) {
	ResistanceLookup out;
	if(damage_type == 0) {
		return out;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	for(const auto& e : g_entries) {
		if(e.kind == "resistance" && e.damage_type == damage_type) {
			out.found = true;
			out.enabled = e.enabled;
			out.target = e.target;
			return out;
		}
	}
	return out;
}

struct AffectLookup {
	bool found = false;
	bool enabled = true;
	EditSystemTarget target = EditSystemTarget::Object;
};

[[nodiscard]] AffectLookup lookup_affect(int location) {
	AffectLookup out;
	if(location == 0) {
		return out;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	for(const auto& e : g_entries) {
		if(e.kind == "affect" && e.location == location) {
			out.found = true;
			out.enabled = e.enabled;
			out.target = e.target;
			return out;
		}
	}
	return out;
}

[[nodiscard]] bool resistance_bits_blocked_on_object(unsigned bits) {
	if(bits == 0) {
		return false;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	for(const auto& e : g_entries) {
		if(e.kind != "resistance" || !e.enabled) {
			continue;
		}
		if((bits & e.damage_type) && e.target == EditSystemTarget::Character) {
			return true;
		}
	}
	return false;
}

void load_from_disk() {
	std::lock_guard<std::mutex> lock(g_mutex);
	g_config_path = resolve_config_path();
	std::string text;
	if(read_file_text(g_config_path, text)) {
		try {
			const Json root = Json::parse(text);
			const bool had_portal =
				root.find("object_portal") != root.end() && root["object_portal"].is_object();
			parse_entries_json(root);
			g_loaded = true;
			if(!had_portal) {
				Json upgraded = root;
				Json portal;
				Json types = Json::object();
				for(const auto& def : kPortalTypeDefs) {
					const auto it = g_portal_type_enabled.find(def.slug);
					types[def.slug] =
						it != g_portal_type_enabled.end() ? it->second : def.default_enabled;
				}
				portal["types"] = types;
				portal["comment"] =
					"types: slug ITEM_* — spunta = visibile e editabile nel portale.";
				upgraded["object_portal"] = portal;
				if(write_file_text(g_config_path, upgraded.dump(2))) {
					mudlog(LOG_CHECK,
						   "edit_system_config: added missing object_portal to %s",
						   g_config_path.c_str());
				}
				else {
					mudlog(LOG_SYSERR,
						   "edit_system_config: could not write object_portal to %s",
						   g_config_path.c_str());
				}
			}
			mudlog(LOG_CHECK, "edit_system_config: loaded %zu entries from %s",
				   g_entries.size(), g_config_path.c_str());
			return;
		}
		catch(const std::exception& e) {
			mudlog(LOG_SYSERR, "edit_system_config: parse error %s — using defaults",
				   e.what());
		}
	}
	build_defaults();
	g_loaded = true;
	mudlog(LOG_CHECK, "edit_system_config: using built-in defaults (%zu entries)",
		   g_entries.size());
}

} // namespace

void edit_system_config_init() {
	load_from_disk();
}

bool edit_system_config_reload() {
	load_from_disk();
	return g_loaded;
}

std::string edit_system_config_to_json() {
	std::lock_guard<std::mutex> lock(g_mutex);
	return entries_to_json().dump(2);
}

bool edit_system_config_save_json(const std::string& json_text, std::string& err) {
	try {
		const Json parsed = Json::parse(json_text);
		if(parsed.find("entries") == parsed.end() || !parsed["entries"].is_array()) {
			err = "JSON deve contenere array entries";
			return false;
		}
		const std::string path = resolve_config_path();
		const std::string pretty = parsed.dump(2);
		if(!write_file_text(path, pretty)) {
			err = std::string("impossibile scrivere ") + path + ": " + std::strerror(errno);
			return false;
		}
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			parse_entries_json(parsed);
			g_config_path = path;
			g_loaded = true;
		}
		mudlog(LOG_CHECK, "edit_system_config: saved %zu entries to %s",
			   g_entries.size(), path.c_str());
		return true;
	}
	catch(const std::exception& e) {
		err = e.what();
		return false;
	}
}

EditSystemTarget edit_system_pool_target(const char* pool_field) noexcept {
	const PoolLookup lk = lookup_pool(pool_field);
	if(!lk.found || !lk.enabled) {
		return EditSystemTarget::Character;
	}
	return lk.target;
}

bool edit_system_pool_enabled(const char* pool_field) noexcept {
	const PoolLookup lk = lookup_pool(pool_field);
	return lk.found && lk.enabled;
}

EditSystemTarget edit_system_resistance_target(unsigned damage_type) noexcept {
	const ResistanceLookup lk = lookup_resistance(damage_type);
	if(!lk.found || !lk.enabled) {
		return EditSystemTarget::Object;
	}
	return lk.target;
}

bool edit_system_resistance_enabled(unsigned damage_type) noexcept {
	const ResistanceLookup lk = lookup_resistance(damage_type);
	return lk.found && lk.enabled;
}

EditSystemTarget edit_system_affect_target(int location) noexcept {
	const AffectLookup lk = lookup_affect(location);
	if(!lk.found || !lk.enabled) {
		return EditSystemTarget::Object;
	}
	return lk.target;
}

bool edit_system_affect_enabled(int location) noexcept {
	const AffectLookup lk = lookup_affect(location);
	/* Se non in config, resta editabile sull'oggetto se nel listino. */
	if(!lk.found) {
		return true;
	}
	return lk.enabled;
}

bool edit_system_blocked_on_object(int location, int modifier) noexcept {
	if(edit_pool_is_pool_apply(location)) {
		const char* field = nullptr;
		switch(location) {
		case APPLY_HIT:
			field = "hit";
			break;
		case APPLY_MANA:
			field = "mana";
			break;
		case APPLY_MOVE:
			field = "move";
			break;
		case APPLY_HIT_REGEN:
			field = "hit_regen";
			break;
		case APPLY_MANA_REGEN:
			field = "mana_regen";
			break;
		case APPLY_MOVE_REGEN:
			field = "move_regen";
			break;
		default:
			break;
		}
		if(field) {
			return edit_system_pool_target(field) == EditSystemTarget::Character;
		}
		return true;
	}

	if(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE || location == APPLY_SUSC) {
		return resistance_bits_blocked_on_object(static_cast<unsigned>(modifier));
	}
	if(edit_system_affect_enabled(location) &&
	   edit_system_affect_target(location) == EditSystemTarget::Character) {
		return true;
	}
	return false;
}

bool edit_system_portal_type_always_hidden(const char* category) noexcept {
	(void)category;
	return false;
}

bool edit_system_portal_category_enabled(const char* category) noexcept {
	if(!category || !*category) {
		return false;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_portal_type_enabled.find(category);
	if(it == g_portal_type_enabled.end()) {
		return false;
	}
	return it->second;
}

const char* edit_system_config_path() {
	if(g_config_path.empty()) {
		return resolve_config_path().c_str();
	}
	return g_config_path.c_str();
}

} // namespace Alarmud
