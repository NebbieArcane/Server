/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "db.hpp"
#include "zone_slots.hpp"

namespace Alarmud {
namespace zone_slots {

namespace {

using Members = std::vector<std::uint64_t>;

/** Mob dei reset ancora in gioco (tolto in free_char): il mob, la riga
 *  (zona, cmd) che lo ha caricato o adottato, e se e' orfano. */
struct LiveEntry {
	char_data* mob;
	int zone;
	int cmd;
	bool orphan;
};

/** serie -> mob dei reset ancora in gioco. */
std::unordered_map<std::uint64_t, LiveEntry> live;
/** Prossimo numero di serie: 0 resta il "nessun posto". */
std::uint64_t next_serial = 1;
/** Mob delle righe capo / C fuori catena, per group_id. */
std::unordered_map<std::uint64_t, Members> line_groups;
/** Prossimo group_id: 0 = nessun gruppo. */
std::uint64_t next_group = 1;
/** Mob delle righe dipendenti: serie del proprietario -> riga -> mob. */
std::unordered_map<std::uint64_t, std::unordered_map<int, Members>> dependents;
/** Orfani per riga dipendente: chiave (zona, cmd) -> mob. */
std::unordered_map<std::uint64_t, Members> orphans;

[[nodiscard]] std::uint64_t line_key(const int zone, const int cmd) {
	return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(zone)) << 32) |
		   static_cast<std::uint64_t>(static_cast<std::uint32_t>(cmd));
}

void bind(char_data& mob, const int zone, const int cmd) {
	if(mob.reset_serial != 0) {
		live.erase(mob.reset_serial);
	}
	mob.reset_serial = next_serial++;
	live[mob.reset_serial] = LiveEntry{&mob, zone, cmd, false};
}

void erase_serial(Members& members, const std::uint64_t serial) {
	members.erase(std::remove(members.begin(), members.end(), serial), members.end());
}

/** Toglie i morti dal gruppo e restituisce i vivi. */
[[nodiscard]] std::vector<char_data*> collect(Members& members) {
	std::vector<char_data*> out;
	out.reserve(members.size());
	members.erase(std::remove_if(members.begin(), members.end(),
								 [&out](const std::uint64_t s) {
									 const auto it = live.find(s);
									 if(it == live.end()) {
										 return true;
									 }
									 out.push_back(it->second.mob);
									 return false;
								 }),
				  members.end());
	return out;
}

[[nodiscard]] int count_alive(const Members& members) {
	return static_cast<int>(std::count_if(members.begin(), members.end(),
										  [](const std::uint64_t s) { return live.count(s) != 0; }));
}

[[nodiscard]] bool contains(const Members& members, const std::uint64_t serial) {
	return std::find(members.begin(), members.end(), serial) != members.end();
}

[[nodiscard]] bool zone_ok(const int zone) {
	return zone_table != nullptr && zone >= 0 && zone <= top_of_zone_table &&
		   zone_table[zone].cmd != nullptr;
}

} // namespace

int owner_of(const reset_com& line) {
	return line.owner_ref - 1;
}

std::vector<char_data*> line_members(const reset_com& line) {
	if(line.group_id == 0) {
		return {};
	}
	return collect(line_groups[line.group_id]);
}

std::vector<char_data*> dependent_members(const char_data& owner, const int cmd) {
	if(owner.reset_serial == 0) {
		return {};
	}
	return collect(dependents[owner.reset_serial][cmd]);
}

void add_line_member(const int zone, const int cmd, char_data& mob) {
	bind(mob, zone, cmd);
	if(!zone_ok(zone)) {
		return;
	}
	const std::uint64_t group = zone_table[zone].cmd[cmd].group_id;
	if(group != 0) {
		line_groups[group].push_back(mob.reset_serial);
	}
}

void add_dependent(const char_data& owner, const int zone, const int cmd, char_data& mob) {
	bind(mob, zone, cmd);
	if(owner.reset_serial != 0) {
		dependents[owner.reset_serial][cmd].push_back(mob.reset_serial);
	}
}

std::vector<char_data*> adopt_orphans(
		const char_data& owner, const int zone, const int cmd, const int wanted,
		bool (*can_join)(const char_data&)) {
	std::vector<char_data*> adopted;
	if(owner.reset_serial == 0 || wanted <= 0) {
		return adopted;
	}
	const auto pool_it = orphans.find(line_key(zone, cmd));
	if(pool_it == orphans.end()) {
		return adopted;
	}
	Members& pool = pool_it->second;
	Members& group = dependents[owner.reset_serial][cmd];
	Members kept;
	for(const std::uint64_t s : pool) {
		const auto it = live.find(s);
		if(it == live.end()) {
			continue; /* morto */
		}
		LiveEntry& entry = it->second;
		if(entry.mob->master != nullptr) {
			entry.orphan = false; /* segue qualcuno: niente adozione */
			continue;
		}
		if(static_cast<int>(adopted.size()) >= wanted || !can_join(*entry.mob)) {
			kept.push_back(s);
			continue;
		}
		entry.orphan = false;
		group.push_back(s);
		adopted.push_back(entry.mob);
	}
	if(kept.empty()) {
		orphans.erase(pool_it);
	}
	else {
		pool = std::move(kept);
	}
	return adopted;
}

void release(char_data& mob) {
	if(mob.reset_serial == 0) {
		return;
	}
	const std::uint64_t serial = mob.reset_serial;
	mob.reset_serial = 0;
	const auto self = live.find(serial);
	if(self != live.end()) {
		if(self->second.orphan) {
			const auto pool = orphans.find(line_key(self->second.zone, self->second.cmd));
			if(pool != orphans.end()) {
				erase_serial(pool->second, serial);
			}
		}
		live.erase(self);
	}
	/* i suoi dipendenti ancora vivi diventano orfani della loro riga */
	const auto deps = dependents.find(serial);
	if(deps == dependents.end()) {
		return;
	}
	for(const auto& [cmd, members] : deps->second) {
		for(const std::uint64_t s : members) {
			const auto it = live.find(s);
			if(it == live.end() || it->second.orphan) {
				continue;
			}
			/* segue gia' qualcun altro (charmato, pet): resta libero */
			if(it->second.mob->master != nullptr && it->second.mob->master != &mob) {
				continue;
			}
			it->second.orphan = true;
			orphans[line_key(it->second.zone, cmd)].push_back(s);
		}
	}
	dependents.erase(deps);
}

bool is_alive(const std::uint64_t serial) {
	return serial != 0 && live.count(serial) != 0;
}

void forget_zone(const int zone) {
	if(!zone_ok(zone)) {
		return;
	}
	reset_com* const cmds = zone_table[zone].cmd;
	for(int i = 0; cmds[i].command != 'S'; ++i) {
		if(cmds[i].group_id != 0) {
			line_groups.erase(cmds[i].group_id);
		}
		const auto pool = orphans.find(line_key(zone, i));
		if(pool != orphans.end()) {
			for(const std::uint64_t s : pool->second) {
				const auto it = live.find(s);
				if(it != live.end()) {
					it->second.orphan = false;
				}
			}
			orphans.erase(pool);
		}
		cmds[i].group_id = 0;
		cmds[i].spawn_max = 0;
		cmds[i].owner_ref = 0;
	}
}

void setup_zone(const int zone) {
	forget_zone(zone);
	if(!zone_ok(zone)) {
		return;
	}
	reset_com* const cmds = zone_table[zone].cmd;
	/* righe M per mob (numero reale): solo le M contano, le C sono seguaci */
	std::unordered_map<int, int> m_lines;
	for(int i = 0; cmds[i].command != 'S'; ++i) {
		if(cmds[i].command == 'M' && cmds[i].arg1 >= 0) {
			++m_lines[cmds[i].arg1];
		}
	}
	int head = -1;     /* riga M capo della catena aperta */
	int last_m = -1;   /* ultima M della catena (capo delle C) */
	int last_mob = -1; /* ultimo mob della catena (G/E/P/... sono sue) */
	for(int i = 0; cmds[i].command != 'S'; ++i) {
		reset_com& line = cmds[i];
		if(!line.if_flag) {
			head = last_m = last_mob = -1;
		}
		if(line.command == 'M') {
			const bool spawn_point =
				line.arg1 >= 0 && line.arg2 > 1 && m_lines[line.arg1] == 1;
			line.spawn_max = spawn_point ? line.arg2 : 1;
			if(head < 0) {
				head = i;
				line.group_id = next_group++;
			}
			else {
				line.owner_ref = head + 1;
			}
			last_m = last_mob = i;
		}
		else if(line.command == 'C') {
			line.spawn_max = 1;
			if(last_m >= 0) {
				line.owner_ref = last_m + 1;
				last_mob = i;
			}
			else {
				line.group_id = next_group++;
			}
		}
		else if(last_mob >= 0) {
			line.owner_ref = last_mob + 1;
		}
	}
}

std::optional<SlotLocation> find(const std::uint64_t serial) {
	if(serial == 0 || zone_table == nullptr) {
		return std::nullopt;
	}
	for(int zone = 0; zone <= top_of_zone_table; ++zone) {
		const reset_com* const cmds = zone_table[zone].cmd;
		if(cmds == nullptr) {
			continue;
		}
		for(int cmd = 0; cmds[cmd].command != 'S'; ++cmd) {
			if(cmds[cmd].group_id == 0) {
				continue;
			}
			const auto it = line_groups.find(cmds[cmd].group_id);
			if(it != line_groups.end() && contains(it->second, serial)) {
				return SlotLocation{zone, cmd, count_alive(it->second), cmds[cmd].spawn_max, 0, false};
			}
		}
	}
	for(const auto& [owner, rows] : dependents) {
		for(const auto& [cmd, members] : rows) {
			if(!contains(members, serial)) {
				continue;
			}
			const std::optional<SlotLocation> master = find(owner);
			const int zone = master ? master->zone : -1;
			const int max = zone_ok(zone) ? zone_table[zone].cmd[cmd].spawn_max : 0;
			return SlotLocation{zone, cmd, count_alive(members), max, owner, false};
		}
	}
	const auto it = live.find(serial);
	if(it != live.end() && it->second.orphan) {
		const LiveEntry& entry = it->second;
		const int max = zone_ok(entry.zone) ? zone_table[entry.zone].cmd[entry.cmd].spawn_max : 0;
		return SlotLocation{entry.zone, entry.cmd, 0, max, 0, true};
	}
	return std::nullopt;
}

} // namespace zone_slots
} // namespace Alarmud
