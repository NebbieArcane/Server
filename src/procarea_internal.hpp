/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_INTERNAL_HPP
#define __PROCAREA_INTERNAL_HPP
#include "procarea.hpp"
#include "typedefs.hpp"
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace Alarmud {

namespace procarea_internal {

enum class ProcArchetype {
	Entrance = 0,
	Corridor,
	Treasure,
	Trap,
	Boss,
	Count
};

enum class ProcMobKind { Normal, Boss, Trap };
enum class ProcMobClassContext { Corridor, Treasure };

struct ProcRoomTemplate {
	const char* name;
	const char* description;
	long sector_type;
	long room_flags;
};

struct ProcLayoutRoom {
	ProcArchetype type;
	std::vector<int> neighbors;
};

struct ProcLayoutEdge {
	int from;
	int to;
	int dir;
};

struct ProcThemeRoomList {
	const ProcRoomTemplate* items;
	std::size_t count;
};

template<typename T, std::size_t N>
constexpr ProcThemeRoomList theme_room_list(const T (&items)[N]) noexcept {
	return { items, N };
}

struct ProcThemeSet {
	const char* label;
	ProcThemeRoomList entrance;
	ProcThemeRoomList corridor;
	ProcThemeRoomList treasure;
	ProcThemeRoomList trap;
	const ProcRoomTemplate* boss;
};

struct ProcAreaInstance {
	int id;
	float group_eq_index;
	int template_band;
	int group_max_level;
	int theme_id;
	long base_vnum;
	long entrance_vnum;
	long boss_vnum;
	long return_room;
	bool exit_portal_open;
	time_t created_at;
	time_t last_activity;
	std::string owner_name;
	std::vector<std::string> member_names;
	std::vector<long> room_vnums;
	std::vector<long> treasure_vnums;
	std::unordered_set<long> treasure_claimed;
	bool boss_key_dropped = false;
	bool reward_shield_granted = false;
	bool solo_mode = false;
	std::unordered_map<std::string, long> member_saved_load_room;
	std::unordered_map<std::string, int> member_saved_start_room;
};

struct ProcAreaDifficulty {
	float eq_index;
	float factor;
	int template_band;
	int group_max_level;
	int rooms_min;
	int rooms_max;
	int max_branches;
	int branch_chance;
	int corridor_spawn_pct;
	int treasure_spawn_pct;
	int boss_adds;
	int depth_extra_pct;
};

extern std::vector<ProcAreaInstance> g_instances;
extern int g_next_instance_id;

[[nodiscard]] inline bool cmd_is(std::string_view token,
								 std::initializer_list<std::string_view> aliases) {
	for(const std::string_view alias : aliases) {
		if(alias == token) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] ProcAreaInstance* find_instance(int instance_id);
[[nodiscard]] ProcAreaInstance* find_instance_by_vnum(long vnum);
int assign_zone(long vnum);
void destroy_rooms(const std::vector<long>& room_vnums);
void clear_world_links(const ProcAreaInstance& inst);

int create_instance(float group_eq_index, int group_max_level, long return_room,
					long& entrance_vnum, const char* owner_name, bool solo_mode = false);

int count_mobs(const ProcAreaInstance& inst);
void open_exit_portal(ProcAreaInstance& inst);
void break_treasure_seals(ProcAreaInstance& inst, const char_data* boss);
bool try_open_treasure(char_data* ch, struct room_data* room, std::string_view target);

void boot_reward_shields_impl();

[[nodiscard]] const ProcThemeSet& theme_set(int theme_id);

} // namespace procarea_internal
} // namespace Alarmud
#endif // __PROCAREA_INTERNAL_HPP
