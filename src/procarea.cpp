/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "hash.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "procarea.hpp"
#include "snew.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace Alarmud {

namespace {

enum class ProcArchetype {
	Entrance = 0,
	Corridor,
	Treasure,
	Trap,
	Boss,
	Count
};

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
	int theme_id;
	long base_vnum;
	long entrance_vnum;
	long boss_vnum;
	long return_room;
	bool exit_portal_open;
	time_t created_at;
	time_t last_activity;
	std::vector<long> room_vnums;
};

static std::vector<ProcAreaInstance> g_instances;
static int g_next_instance_id = 1;

struct ProcFountainVeil {
	bool spirit_dismissed = false;
	time_t spirit_dismissed_at = 0;
	bool active = false;
	time_t opened_at = 0;
	time_t expires_at = 0;
};

static ProcFountainVeil g_fountain_veil {};
static constexpr int kFountainVeilLifetimeSec = 180;

static constexpr long kProcIsolationFlags = static_cast<long>(NO_ASTRAL | NO_SUM);
static constexpr int kProcExitPortalObj = 9071;

enum class ProcMobKind { Normal, Boss, Trap };

static void procarea_spawn_mob(long room_vnum, int mob_vnum, float eq_index, int template_band,
							   ProcMobKind kind);


#include "procarea_themes.inc"

static constexpr std::size_t kThemeSetCount = std::size(kThemeSets);

[[nodiscard]] static const ProcRoomTemplate& procarea_pick_from_list(
	const ProcThemeRoomList& list) {
	if(list.items == nullptr || list.count == 0) {
		return kTheme0Corridors[0];
	}
	return list.items[number(0, static_cast<int>(list.count) - 1)];
}

[[nodiscard]] static const ProcThemeSet& procarea_theme_set(int theme_id) {
	if(theme_id < 0 || static_cast<std::size_t>(theme_id) >= kThemeSetCount) {
		return kThemeSets[0];
	}
	return kThemeSets[static_cast<std::size_t>(theme_id)];
}

[[nodiscard]] static int procarea_pick_theme_id() {
	return number(0, static_cast<int>(kThemeSetCount) - 1);
}

[[nodiscard]] static const ProcRoomTemplate& pick_template(const ProcThemeSet& theme,
															 ProcArchetype type) {
	switch(type) {
	case ProcArchetype::Entrance:
		return procarea_pick_from_list(theme.entrance);
	case ProcArchetype::Corridor:
		return procarea_pick_from_list(theme.corridor);
	case ProcArchetype::Treasure:
		return procarea_pick_from_list(theme.treasure);
	case ProcArchetype::Trap:
		return procarea_pick_from_list(theme.trap);
	case ProcArchetype::Boss:
		return theme.boss != nullptr ? *theme.boss : kTheme0Boss;
	default:
		return procarea_pick_from_list(theme.corridor);
	}
}

struct ProcAreaDifficulty {
	float eq_index;
	float factor;
	int template_band;
	int rooms_min;
	int rooms_max;
	int max_branches;
	int branch_chance;
	int corridor_spawn_pct;
	int treasure_spawn_pct;
	int boss_adds;
	int depth_extra_pct;
};

static constexpr std::array<float, PROCAREA_TEMPLATE_BANDS> kTemplateEqAnchor = {
	100.0f, 280.0f, 480.0f, 720.0f, 1000.0f,
};

[[nodiscard]] static float procarea_eq_factor(float eq_index) {
	return std::clamp((eq_index - PROCAREA_EQ_SCALE_MIN) /
						  (PROCAREA_EQ_SCALE_MAX - PROCAREA_EQ_SCALE_MIN),
					  0.0f, 1.0f);
}

[[nodiscard]] static int procarea_lerp_int(float factor, int lo, int hi) {
	return lo + static_cast<int>((hi - lo) * factor + 0.5f);
}

[[nodiscard]] static ProcAreaDifficulty procarea_difficulty_from_eq(float eq_index) {
	const float factor = procarea_eq_factor(eq_index);
	ProcAreaDifficulty diff{};
	diff.eq_index = eq_index;
	diff.factor = factor;
	diff.template_band = std::min(
		PROCAREA_TEMPLATE_BANDS - 1,
		static_cast<int>(factor * static_cast<float>(PROCAREA_TEMPLATE_BANDS)));
	diff.rooms_min = procarea_lerp_int(factor, 12, 70);
	diff.rooms_max = procarea_lerp_int(factor, 20, PROCAREA_ROOMS_MAX);
	if(diff.rooms_max < diff.rooms_min) {
		diff.rooms_max = diff.rooms_min;
	}
	diff.max_branches = procarea_lerp_int(factor, 2, 6);
	diff.branch_chance = procarea_lerp_int(factor, 30, 50);
	diff.corridor_spawn_pct = procarea_lerp_int(factor, 60, 82);
	diff.treasure_spawn_pct = procarea_lerp_int(factor, 75, 90);
	diff.boss_adds = procarea_lerp_int(factor, 1, 3);
	diff.depth_extra_pct = procarea_lerp_int(factor, 4, 12);
	return diff;
}

[[nodiscard]] static int procarea_pick_mob_vnum(int template_band, bool trap) {
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(trap) {
		return PROCAREA_MOB_VNUM_BASE + band * PROCAREA_MOBS_PER_BAND +
			   (PROCAREA_MOBS_PER_BAND - 1);
	}
	const int base = PROCAREA_MOB_VNUM_BASE + band * PROCAREA_MOBS_PER_BAND;
	return base + number(0, PROCAREA_MOB_POOL_SIZE - 1);
}

[[nodiscard]] static int procarea_pick_boss_vnum(int template_band) {
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int base = PROCAREA_BOSS_VNUM_BASE + band * PROCAREA_BOSSES_PER_BAND;
	return base + number(0, PROCAREA_BOSSES_PER_BAND - 1);
}

[[nodiscard]] static bool procarea_cmd_is(std::string_view token,
										  std::initializer_list<std::string_view> aliases) {
	for(const std::string_view alias : aliases) {
		if(alias == token) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] static std::vector<char_data*> procarea_entering_group(char_data* ch) {
	std::vector<char_data*> group;
	if(ch == nullptr || !IS_PC(ch)) {
		return group;
	}

	group.push_back(ch);
	const long source_room = ch->in_room;
	for(follow_type* fol = ch->followers; fol != nullptr; fol = fol->next) {
		char_data* follower = fol->follower;
		if(follower == nullptr || follower == ch || !IS_PC(follower)) {
			continue;
		}
		if(follower->in_room != source_room) {
			continue;
		}
		if(GET_POS(follower) < POSITION_STANDING) {
			continue;
		}
		if(follower->specials.fighting != nullptr) {
			continue;
		}
		group.push_back(follower);
	}
	return group;
}

[[nodiscard]] static float procarea_group_equipment_index(
	const std::vector<char_data*>& group) {
	if(group.empty()) {
		return 0.0f;
	}
	float sum = 0.0f;
	for(char_data* member : group) {
		sum += GetCharBonusIndex(member);
	}
	return sum / static_cast<float>(group.size());
}

[[nodiscard]] static bool procarea_group_can_enter(const std::vector<char_data*>& group) {
	if(group.empty()) {
		return false;
	}
	for(char_data* member : group) {
		if(GetMaxLevel(member) < PROCAREA_MIN_LEVEL) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static bool procarea_resolve_entry(char_data* ch, float& group_eq_index) {
	group_eq_index = 0.0f;
	const std::vector<char_data*> group = procarea_entering_group(ch);
	if(!procarea_group_can_enter(group)) {
		return false;
	}
	group_eq_index = procarea_group_equipment_index(group);
	return true;
}

static void procarea_scale_mob(char_data* mob, float eq_index, int template_band,
							   ProcMobKind kind) {
	if(mob == nullptr) {
		return;
	}

	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const float anchor = kTemplateEqAnchor[static_cast<std::size_t>(band)];
	float ratio = std::clamp(eq_index / anchor, 0.65f, 1.45f);
	if(kind == ProcMobKind::Boss) {
		ratio *= 1.12f;
	} else if(kind == ProcMobKind::Trap) {
		ratio *= 0.92f;
	}

	const int level = GET_LEVEL(mob, WARRIOR_LEVEL_IND);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) =
		std::clamp(static_cast<int>(level * ratio), 1, 60);

	mob->points.max_hit = std::max(1, static_cast<int>(mob->points.max_hit * ratio));
	mob->points.hit = mob->points.max_hit;

	mob->points.damroll = static_cast<sbyte>(std::clamp(
		static_cast<int>(mob->points.damroll * ratio), -30, 40));
	mob->points.armor = static_cast<int>(mob->points.armor * ratio);

	const float factor = procarea_eq_factor(eq_index);
	if(kind != ProcMobKind::Trap) {
		if(kind == ProcMobKind::Boss) {
			if(factor >= 0.35f) {
				SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
			}
			if(factor >= 0.55f) {
				SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
			}
		} else if(factor >= 0.50f) {
			SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
			if(factor >= 0.75f && number(0, 99) < 30) {
				SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
			}
		}
	}
}

static void procarea_spawn_scaled_mob(long room_vnum, int template_band, float eq_index,
									  ProcMobKind kind) {
	const int mob_vnum = procarea_pick_mob_vnum(template_band, kind == ProcMobKind::Trap);
	if(mob_vnum > 0) {
		procarea_spawn_mob(room_vnum, mob_vnum, eq_index, template_band, kind);
	}
}

static int procarea_depth_spawn_bonus(const ProcAreaDifficulty& diff, int depth, int max_depth) {
	if(max_depth <= 0 || depth <= 0) {
		return 0;
	}
	const int mid = std::max(1, max_depth / 2);
	if(depth < mid) {
		return 0;
	}
	return (depth - mid + 1) * diff.depth_extra_pct;
}

[[nodiscard]] static long procarea_instance_base_vnum(int instance_id) {
	return PROCAREA_VNUM_BASE +
		   static_cast<long>(instance_id - 1) *
			   static_cast<long>(PROCAREA_SLOTS_PER_INSTANCE);
}

[[nodiscard]] static bool procarea_instance_fits_in_world(int instance_id, int room_count) {
	if(instance_id < 1 || instance_id > PROCAREA_MAX_ACTIVE || room_count <= 0) {
		return false;
	}

	const long base = procarea_instance_base_vnum(instance_id);
	if(base < PROCAREA_VNUM_BASE) {
		return false;
	}

	const long world_limit = static_cast<long>(WORLD_SIZE);
	const long room_span = static_cast<long>(room_count);
	if(room_span > world_limit || base > world_limit - room_span) {
		return false;
	}
	return true;
}

[[nodiscard]] static long procarea_local_vnum(int instance_id, int local_index) {
	return procarea_instance_base_vnum(instance_id) + local_index;
}

[[nodiscard]] static ProcAreaInstance* procarea_find_instance(int instance_id) {
	for(ProcAreaInstance& inst : g_instances) {
		if(inst.id == instance_id) {
			return &inst;
		}
	}
	return nullptr;
}

[[nodiscard]] static ProcAreaInstance* procarea_find_instance_by_vnum(long vnum) {
	const long instance_id = procarea_vnum_to_instance(vnum);
	if(instance_id < 0) {
		return nullptr;
	}
	return procarea_find_instance(static_cast<int>(instance_id));
}

static int procarea_assign_zone(long vnum) {
	if(top_of_zone_table < 0) {
		return 0;
	}

	int zone = 0;
	for(; vnum > zone_table[zone].top && zone <= top_of_zone_table; ++zone) {
		;
	}
	if(zone > top_of_zone_table) {
		zone = top_of_zone_table;
	}
	return zone;
}

static void procarea_link_rooms_one_way(long from_vnum, int dir, long to_vnum,
										const char* keyword, const char* description) {
	struct room_data* const from = real_roomp(from_vnum);
	if(from == nullptr || dir < 0 || dir > 5) {
		return;
	}

	if(!from->dir_option[dir]) {
		CREATE(from->dir_option[dir], struct room_direction_data, 1);
		from->dir_option[dir]->general_description =
			strdup(description != nullptr ? description : "");
		from->dir_option[dir]->keyword = strdup(keyword != nullptr ? keyword : "");
		from->dir_option[dir]->exit_info = 0;
		from->dir_option[dir]->key = -1;
		from->dir_option[dir]->open_cmd = -1;
	}
	from->dir_option[dir]->to_room = to_vnum;
}

static void procarea_link_rooms(long from_vnum, int dir, long to_vnum) {
	struct room_data* const from = real_roomp(from_vnum);
	struct room_data* const to = real_roomp(to_vnum);
	if(from == nullptr || to == nullptr) {
		return;
	}

	if(dir < 0 || dir > 5) {
		return;
	}

	const int back = rev_dir[dir];
	if(back < 0 || back > 5) {
		return;
	}

	if(!from->dir_option[dir]) {
		CREATE(from->dir_option[dir], struct room_direction_data, 1);
		from->dir_option[dir]->general_description = strdup("");
		from->dir_option[dir]->keyword = strdup("");
		from->dir_option[dir]->exit_info = 0;
		from->dir_option[dir]->key = -1;
		from->dir_option[dir]->open_cmd = -1;
	}
	from->dir_option[dir]->to_room = to_vnum;

	if(!to->dir_option[back]) {
		CREATE(to->dir_option[back], struct room_direction_data, 1);
		to->dir_option[back]->general_description = strdup("");
		to->dir_option[back]->keyword = strdup("");
		to->dir_option[back]->exit_info = 0;
		to->dir_option[back]->key = -1;
		to->dir_option[back]->open_cmd = -1;
	}
	to->dir_option[back]->to_room = from_vnum;
}

static void procarea_clear_world_links(const ProcAreaInstance& inst) {
	if(inst.return_room <= 0) {
		return;
	}
	struct room_data* const world = real_roomp(inst.return_room);
	if(world == nullptr) {
		return;
	}

	for(int dir = 0; dir <= 5; ++dir) {
		struct room_direction_data* const exit = world->dir_option[dir];
		if(exit == nullptr) {
			continue;
		}
		const auto linked = std::find(inst.room_vnums.begin(), inst.room_vnums.end(),
									  exit->to_room);
		if(linked == inst.room_vnums.end()) {
			continue;
		}
		free(exit->general_description);
		free(exit->keyword);
		free(exit);
		world->dir_option[dir] = nullptr;
	}
}

static int procarea_pick_direction(const std::array<bool, 6>& used) {
	std::vector<int> free_dirs;
	free_dirs.reserve(6);
	for(int dir = 0; dir <= 5; ++dir) {
		if(!used[static_cast<size_t>(dir)]) {
			free_dirs.push_back(dir);
		}
	}
	if(free_dirs.empty()) {
		return -1;
	}
	return free_dirs[static_cast<size_t>(number(0, static_cast<int>(free_dirs.size()) - 1))];
}

static bool procarea_assign_directions(const std::vector<ProcLayoutRoom>& layout,
									   std::vector<ProcLayoutEdge>& edges) {
	std::vector<std::array<bool, 6>> used_dirs(layout.size());
	for(size_t i = 0; i < layout.size(); ++i) {
		used_dirs[i].fill(false);
	}

	std::vector<int> queue;
	std::vector<bool> visited(layout.size(), false);
	queue.push_back(0);
	visited[0] = true;

	for(size_t qi = 0; qi < queue.size(); ++qi) {
		const int node = queue[qi];
		for(int neighbor : layout[static_cast<size_t>(node)].neighbors) {
			if(neighbor < 0 || neighbor >= static_cast<int>(layout.size())) {
				continue;
			}

			const bool already_linked =
				std::any_of(edges.begin(), edges.end(), [node, neighbor](const ProcLayoutEdge& edge) {
					return (edge.from == node && edge.to == neighbor) ||
						   (edge.from == neighbor && edge.to == node);
				});
			if(already_linked) {
				if(!visited[static_cast<size_t>(neighbor)]) {
					visited[static_cast<size_t>(neighbor)] = true;
					queue.push_back(neighbor);
				}
				continue;
			}

			const int dir = procarea_pick_direction(used_dirs[static_cast<size_t>(node)]);
			if(dir < 0) {
				return false;
			}
			const int back = rev_dir[dir];
			if(back < 0 || used_dirs[static_cast<size_t>(neighbor)][static_cast<size_t>(back)]) {
				return false;
			}

			used_dirs[static_cast<size_t>(node)][static_cast<size_t>(dir)] = true;
			used_dirs[static_cast<size_t>(neighbor)][static_cast<size_t>(back)] = true;
			edges.push_back({ node, neighbor, dir });

			if(!visited[static_cast<size_t>(neighbor)]) {
				visited[static_cast<size_t>(neighbor)] = true;
				queue.push_back(neighbor);
			}
		}
	}

	for(size_t i = 0; i < visited.size(); ++i) {
		if(!visited[i]) {
			return false;
		}
	}
	return true;
}

static bool procarea_generate_layout(int target_rooms, int max_branches, int branch_chance,
									   std::vector<ProcLayoutRoom>& layout) {
	layout.clear();
	if(target_rooms < 8) {
		return false;
	}

	layout.push_back({ ProcArchetype::Entrance, {} });
	int spine_end = 0;
	int branch_count = 0;

	while(static_cast<int>(layout.size()) < target_rooms - 1) {
		const int new_idx = static_cast<int>(layout.size());
		layout.push_back({ ProcArchetype::Corridor, {} });
		layout[static_cast<size_t>(spine_end)].neighbors.push_back(new_idx);
		layout[static_cast<size_t>(new_idx)].neighbors.push_back(spine_end);

		if(branch_count < max_branches && static_cast<int>(layout.size()) < target_rooms - 2 &&
		   number(0, 99) < branch_chance) {
			const ProcArchetype branch_type =
				(number(0, 1) == 0) ? ProcArchetype::Treasure : ProcArchetype::Trap;
			const int branch_idx = static_cast<int>(layout.size());
			layout.push_back({ branch_type, {} });
			layout[static_cast<size_t>(new_idx)].neighbors.push_back(branch_idx);
			layout[static_cast<size_t>(branch_idx)].neighbors.push_back(new_idx);
			++branch_count;
		}

		spine_end = new_idx;
	}

	const int boss_idx = static_cast<int>(layout.size());
	layout.push_back({ ProcArchetype::Boss, {} });
	layout[static_cast<size_t>(spine_end)].neighbors.push_back(boss_idx);
	layout[static_cast<size_t>(boss_idx)].neighbors.push_back(spine_end);

	return static_cast<int>(layout.size()) <= PROCAREA_SLOTS_PER_INSTANCE;
}

static struct room_data* procarea_create_room(long vnum, const ProcRoomTemplate& tmpl) {
	allocate_room(vnum);
	struct room_data* rp = real_roomp(vnum);
	if(rp == nullptr) {
		return nullptr;
	}

	memset(rp, 0, sizeof(*rp));
	rp->number = vnum;
	rp->zone = procarea_assign_zone(vnum);
	rp->sector_type = tmpl.sector_type;
	long flags = tmpl.room_flags | kProcIsolationFlags;
	if(number(0, 99) < 10) {
		flags |= NO_TRACK;
	}
	rp->room_flags = flags;
	rp->light = 0;
	rp->name = strdup(tmpl.name);
	rp->description = strdup(tmpl.description);
	return rp;
}

static void procarea_spawn_mob(long room_vnum, int mob_vnum, float eq_index, int template_band,
							   ProcMobKind kind) {
	struct char_data* mob = read_mobile(mob_vnum, VIRTUAL);
	if(mob == nullptr) {
		mudlog(LOG_ERROR, "procarea: read_mobile(%d) failed for room %ld", mob_vnum,
			   room_vnum);
		return;
	}
	procarea_scale_mob(mob, eq_index, template_band, kind);
	char_to_room(mob, room_vnum);
}

static void procarea_populate_room(const ProcAreaDifficulty& diff, long room_vnum,
								   ProcArchetype type, int depth, int max_depth) {
	const int depth_bonus = procarea_depth_spawn_bonus(diff, depth, max_depth);

	switch(type) {
	case ProcArchetype::Entrance:
		break;
	case ProcArchetype::Corridor: {
		const int chance = std::clamp(diff.corridor_spawn_pct + depth_bonus, 0, 95);
		if(number(0, 99) < chance) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		if(depth >= std::max(2, max_depth / 3) &&
		   number(0, 99) < std::clamp(25 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		break;
	}
	case ProcArchetype::Treasure: {
		const int chance = std::clamp(diff.treasure_spawn_pct + depth_bonus, 0, 98);
		if(number(0, 99) < chance) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		if(number(0, 99) < std::clamp(40 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		break;
	}
	case ProcArchetype::Trap:
		procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index, ProcMobKind::Trap);
		procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
								  ProcMobKind::Normal);
		if(number(0, 99) < std::clamp(50 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		break;
	case ProcArchetype::Boss: {
		const int boss = procarea_pick_boss_vnum(diff.template_band);
		if(boss > 0) {
			procarea_spawn_mob(room_vnum, boss, diff.eq_index, diff.template_band,
							   ProcMobKind::Boss);
		}
		for(int i = 0; i < diff.boss_adds; ++i) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal);
		}
		break;
	}
	default:
		break;
	}
}

static int procarea_count_mobs(const ProcAreaInstance& inst) {
	int count = 0;
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(struct char_data* mob = rp->people; mob != nullptr; mob = mob->next_in_room) {
			if(IS_NPC(mob) && !IS_SET(mob->specials.act, ACT_POLYSELF)) {
				++count;
			}
		}
	}
	return count;
}

static void procarea_open_exit_portal(ProcAreaInstance& inst) {
	if(inst.exit_portal_open || inst.boss_vnum <= 0) {
		return;
	}

	inst.exit_portal_open = true;
	send_to_room(
		"\n\r$c0015Una luce fredda squarcia l'oscurita': si apre un portale verso la fontana di Myst!\n\r",
		inst.boss_vnum);

	const int portal_rnum = real_object(kProcExitPortalObj);
	if(portal_rnum < 0) {
		mudlog(LOG_ERROR, "procarea: exit portal obj %d not found", kProcExitPortalObj);
		return;
	}

	struct obj_data* portale = read_object(portal_rnum, REAL);
	if(portale == nullptr) {
		mudlog(LOG_ERROR, "procarea: read_object(%d) failed", kProcExitPortalObj);
		return;
	}
	obj_to_room(portale, inst.boss_vnum);
}

static int procarea_room_depth(const std::vector<ProcLayoutRoom>& layout, int node) {
	std::vector<int> depth(layout.size(), -1);
	std::vector<int> queue;
	queue.push_back(0);
	depth[0] = 0;
	for(size_t qi = 0; qi < queue.size(); ++qi) {
		const int current = queue[qi];
		for(int neighbor : layout[static_cast<size_t>(current)].neighbors) {
			if(neighbor < 0 || neighbor >= static_cast<int>(layout.size())) {
				continue;
			}
			if(depth[static_cast<size_t>(neighbor)] >= 0) {
				continue;
			}
			depth[static_cast<size_t>(neighbor)] = depth[static_cast<size_t>(current)] + 1;
			queue.push_back(neighbor);
		}
	}
	return depth[static_cast<size_t>(node)];
}

static void procarea_clear_room_contents(struct room_data* rp) {
	if(rp == nullptr) {
		return;
	}

	while(rp->people != nullptr) {
		struct char_data* ch = rp->people;
		char_from_room(ch);
		if(IS_PC(ch)) {
			char_to_room(ch, 3001);
		}
		else {
			extract_char(ch);
		}
	}

	while(rp->contents != nullptr) {
		struct obj_data* obj = rp->contents;
		obj_from_room(obj);
		extract_obj(obj);
	}
}

static void procarea_destroy_room(long vnum) {
	struct room_data* rp = real_roomp(vnum);
	if(rp == nullptr) {
		return;
	}

	procarea_clear_room_contents(rp);
	cleanout_room(rp);
#if HASH
	hash_remove(&Alarmud::room_db, vnum);
	free(rp);
#else
	room_remove(Alarmud::room_db, vnum);
#endif
}

static void procarea_destroy_rooms(const std::vector<long>& room_vnums) {
	for(long vnum : room_vnums) {
		procarea_destroy_room(vnum);
	}
}

static void procarea_destroy_instance(int instance_id) {
	ProcAreaInstance* inst = procarea_find_instance(instance_id);
	if(inst == nullptr) {
		return;
	}

	const std::vector<long> rooms = inst->room_vnums;
	procarea_clear_world_links(*inst);
	procarea_destroy_rooms(rooms);

	const auto it = std::find_if(g_instances.begin(), g_instances.end(),
								 [instance_id](const ProcAreaInstance& candidate) {
									 return candidate.id == instance_id;
								 });
	if(it != g_instances.end()) {
		g_instances.erase(it);
	}

	mudlog(LOG_CHECK, "procarea: destroyed instance %d", instance_id);
}

static bool procarea_instance_has_players(const ProcAreaInstance& inst) {
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(struct char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(IS_PC(ch)) {
				return true;
			}
		}
	}
	return false;
}

static void procarea_touch_instance(int instance_id) {
	ProcAreaInstance* inst = procarea_find_instance(instance_id);
	if(inst != nullptr) {
		inst->last_activity = time(nullptr);
	}
}

static int procarea_create_instance(float group_eq_index, long return_room, long& entrance_vnum) {
	const ProcAreaDifficulty diff = procarea_difficulty_from_eq(group_eq_index);
	if(static_cast<int>(g_instances.size()) >= PROCAREA_MAX_ACTIVE) {
		mudlog(LOG_SYSERR, "procarea: too many active instances");
		return -1;
	}

	std::vector<ProcLayoutRoom> layout;
	std::vector<ProcLayoutEdge> edges;
	bool layout_ok = false;
	for(int attempt = 0; attempt < 12; ++attempt) {
		const int target_rooms = number(diff.rooms_min, diff.rooms_max);
		layout.clear();
		edges.clear();
		if(!procarea_generate_layout(target_rooms, diff.max_branches, diff.branch_chance,
									 layout)) {
			continue;
		}
		if(!procarea_assign_directions(layout, edges)) {
			continue;
		}
		if(!procarea_instance_fits_in_world(g_next_instance_id,
											static_cast<int>(layout.size()))) {
			mudlog(LOG_SYSERR, "procarea: instance %d vnums exceed WORLD_SIZE",
				   g_next_instance_id);
			return -1;
		}
		layout_ok = true;
		break;
	}
	if(!layout_ok) {
		mudlog(LOG_SYSERR, "procarea: layout/direction generation failed (eq %.0f)",
			   group_eq_index);
		return -1;
	}

	const int instance_id = g_next_instance_id++;

	ProcAreaInstance inst{};
	inst.id = instance_id;
	inst.group_eq_index = group_eq_index;
	inst.template_band = diff.template_band;
	inst.theme_id = procarea_pick_theme_id();
	inst.base_vnum = procarea_instance_base_vnum(instance_id);
	inst.return_room = return_room;
	inst.exit_portal_open = false;
	inst.created_at = time(nullptr);
	inst.last_activity = inst.created_at;
	inst.room_vnums.reserve(layout.size());

	const int max_depth = procarea_room_depth(layout, static_cast<int>(layout.size()) - 1);
	const ProcThemeSet& theme = procarea_theme_set(inst.theme_id);

	for(size_t i = 0; i < layout.size(); ++i) {
		const long vnum = procarea_local_vnum(instance_id, static_cast<int>(i));
		const ProcRoomTemplate& tmpl = pick_template(theme, layout[i].type);
		if(procarea_create_room(vnum, tmpl) == nullptr) {
			mudlog(LOG_SYSERR, "procarea: create_room failed vnum=%ld (WORLD_SIZE=%d)",
				   vnum, WORLD_SIZE);
			procarea_destroy_rooms(inst.room_vnums);
			return -1;
		}
		inst.room_vnums.push_back(vnum);

		const int depth = procarea_room_depth(layout, static_cast<int>(i));
		procarea_populate_room(diff, vnum, layout[i].type, depth, max_depth);

		if(layout[i].type == ProcArchetype::Entrance) {
			inst.entrance_vnum = vnum;
		}
		if(layout[i].type == ProcArchetype::Boss) {
			inst.boss_vnum = vnum;
			if(room_data* boss_room = real_roomp(vnum); boss_room != nullptr) {
				boss_room->funct = procarea_boss_exit;
				boss_room->specname = "procarea_boss_exit";
			}
		}
	}

	for(const ProcLayoutEdge& edge : edges) {
		const long from_vnum =
			procarea_local_vnum(instance_id, edge.from);
		const long to_vnum =
			procarea_local_vnum(instance_id, edge.to);
		procarea_link_rooms(from_vnum, edge.dir, to_vnum);
	}

	g_instances.push_back(inst);
	entrance_vnum = inst.entrance_vnum;
	mudlog(LOG_CHECK,
		   "procarea: created instance %d theme '%s' eq %.0f (factor %.2f) with %zu rooms (entrance %ld, boss %ld)",
		   instance_id, theme.label, group_eq_index, diff.factor, layout.size(),
		   inst.entrance_vnum, inst.boss_vnum);
	return instance_id;
}

static void procarea_teleport_char(struct char_data* ch, long dest, bool announce_departure) {
	if(ch == nullptr || real_roomp(dest) == nullptr) {
		return;
	}

	if(announce_departure) {
		act("$n scompare in un vortice di nebbia.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	}

	char_from_room(ch);
	char_to_room(ch, dest);
	act("$n emerge dalla nebbia.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	do_look(ch, "", CMD_LOOK);
}

static void procarea_teleport_group(char_data* ch, long dest) {
	const std::vector<char_data*> group = procarea_entering_group(ch);
	for(size_t i = 0; i < group.size(); ++i) {
		procarea_teleport_char(group[i], dest, i == 0);
	}
}

static void procarea_send_fountain_plaza(const char* msg) {
	struct room_data* plaza = real_roomp(PROCAREA_FOUNTAIN_ROOM);
	if(plaza == nullptr || msg == nullptr) {
		return;
	}
	for(char_data* person = plaza->people; person != nullptr; person = person->next_in_room) {
		if(IS_PC(person) && person->desc != nullptr) {
			send_to_char(msg, person);
		}
	}
}

static void procarea_restore_fountain_spirit(bool announce) {
	if(!g_fountain_veil.spirit_dismissed) {
		return;
	}
	g_fountain_veil.spirit_dismissed = false;
	g_fountain_veil.spirit_dismissed_at = 0;

	if(!announce) {
		return;
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Un filo di luce livida ricade sul bordo della fontana:\n\r"
		"lo spirito della $c0014Dea DarkStar$c0010 riprende a vegliare sul bacino,\n\r"
		"muto ed immobile.$c0007\n\r\n\r");
}

static void procarea_dissolve_fountain_veil(bool announce) {
	if(!g_fountain_veil.active) {
		return;
	}
	g_fountain_veil.active = false;
	g_fountain_veil.opened_at = 0;
	g_fountain_veil.expires_at = 0;

	if(announce) {
		procarea_send_fountain_plaza(
			"\n\r$c0010Un sibilo lontano attraversa la piazza:\n\r"
			"la bruma viene risucchiata verso il centro, come marea al rovescio.$c0007\n\r");
		procarea_send_fountain_plaza(
			"$c0010Il vortice $c0014implode$c0010 in un battito\n\r"
			"d'ali d'acqua silenzioso.$c0007\n\r");
		procarea_send_fountain_plaza(
			"$c0010La Fontana della Vita ricompare al suo posto,\n\r"
			"lucida e immobile — come se la nebbia non fosse mai passata.$c0007\n\r");
	}

	procarea_restore_fountain_spirit(announce);
}

static void procarea_dismiss_fountain_spirit(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	if(procarea_find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' oltre il velo del mondo.\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi disturbare la fontana mentre combatti.\n\r", ch);
		return;
	}

	if(g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"Non c'e' piu' nulla da strappare al bacino:\n\r"
			"lo spirito della Dea e' gia' lontano.\n\r",
			ch);
		return;
	}

	if(g_fountain_veil.active) {
		send_to_char("La nebbia turbinante rende impossibile raggiungere lo spirito.\n\r", ch);
		return;
	}

	const time_t now = time(nullptr);
	g_fountain_veil.spirit_dismissed = true;
	g_fountain_veil.spirit_dismissed_at = now;

	send_to_char(
		"Ti aggrappi al bordo scivoloso e $c0014tira$c0007:\n\r"
		"l'acqua si incupisce, come se qualcuno respirasse sotto la pietra.\n\r",
		ch);
	send_to_char(
		"Una figura eterea si stacca dalla schiuma —\n\r"
		"$c0014DarkStar Luce Oscura$c0007 ti fissa un istante,\n\r"
		"poi viene risucchiata verso l'alto.\n\r",
		ch);
	act("$n tira con forza dalla Fontana della Vita.", TRUE, ch, nullptr, nullptr, TO_ROOM);

	procarea_send_fountain_plaza(
		"\n\r$c0010La Fontana della Vita vibra:\n\r"
		"un'aura $c0014luce-oscura$c0010 si stacca dal centro del bacino,\n\r"
		"sospesa tra cielo e acqua.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010Per un attimo la $c0014Dea DarkStar$c0010 sembra volersi opporre —\n\r"
		"poi lo spirito svanisce nel cielo di Myst,\n\r"
		"lasciando l'acqua limpida e muta.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0015Il presidio divino e' caduto.$c0010\n\r"
		"Ora la fontana obbedisce solo a chi osa spingerla.$c0007\n\r");
}

static void procarea_invoke_fountain_veil(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	if(procarea_find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' oltre il velo del mondo.\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi convocare la nebbia mentre combatti.\n\r", ch);
		return;
	}

	if(!g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"L'acqua resiste, ancora protetta da un residuo divino.\n\r"
			"Allontana prima lo spirito della Dea con $c0014pull fontana$c0007.\n\r",
			ch);
		return;
	}

	const time_t now = time(nullptr);
	if(g_fountain_veil.active) {
		g_fountain_veil.expires_at = now + kFountainVeilLifetimeSec;
		send_to_char(
			"La nebbia risponde al tuo tocco e si infittisce,\n\r"
			"turbinando piu' bassa intorno al bacino.\n\r",
			ch);
		act("$n sfiora ancora la fontana: il velo di bruma si addensa.", TRUE, ch, nullptr, nullptr,
			TO_ROOM);
		return;
	}

	g_fountain_veil.active = true;
	g_fountain_veil.opened_at = now;
	g_fountain_veil.expires_at = now + kFountainVeilLifetimeSec;

	send_to_char(
		"Appoggi le mani sulla pietra fredda:\n\r"
		"sotto l'acqua qualcosa $c0014trema$c0007,\n\r"
		"come un respiro trattenuto da secoli.\n\r",
		ch);
	send_to_char(
		"Un rantolo sommerso risponde al tuo spinta —\n\r"
		"e il mondo sembra inclinarsi verso il basso.\n\r",
		ch);
	act("$n preme contro la Fontana della Vita.", TRUE, ch, nullptr, nullptr, TO_ROOM);

	procarea_send_fountain_plaza(
		"\n\r$c0010Gocce si staccano dal bordo della fontana\n\r"
		"e svaniscono in vapore prima di toccare il selciato.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010$c0014Un velo di nebbia argentata$c0010 si alza dal bacino,\n\r"
		"lambisce le statue e ammorbidisce i contorni dei palazzi.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010Per un istante la fontana sembra $c0014dissolversi$c0010 nel nulla;\n\r"
		"al suo posto danza un turbine di bruma silente,\n\r"
		"largo quanto la piazza.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0015I rumori di Myst si affievoliscono.$c0010\n\r"
		"Resta solo un sussurro d'acqua\n\r"
		"ed un gelido invito ad entrare nella $c0014nebbia$c0010.$c0007\n\r");
}

static void procarea_enter_via_veil(struct char_data* ch) {
	if(!IS_PC(ch)) {
		return;
	}

	if(procarea_find_instance_by_vnum(ch->in_room) != nullptr) {
		send_to_char("Sei gia' dentro un'area effimera.\n\r", ch);
		return;
	}

	if(ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		send_to_char(
			"Il velo si apre solo davanti alla Fontana della Vita,\n\r"
			"in Piazza delle Nebbie.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.spirit_dismissed) {
		send_to_char(
			"Una luce gelida ancora custodisce il bacino:\n\r"
			"devi prima allontanare lo spirito della Dea DarkStar\n\r"
			"con $c0014pull fontana$c0007.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.active) {
		send_to_char(
			"Non c'e' alcun velo di nebbia da attraversare.\n\r"
			"Convocalo con $c0014push fontana$c0007.\n\r",
			ch);
		return;
	}

	float group_eq_index = 0.0f;
	if(!procarea_resolve_entry(ch, group_eq_index)) {
		send_to_char(
			"La nebbia si rifiuta di riconoscere il gruppo:\n\r"
			"qualcuno non e' pronto, o non e' adatto.\n\r",
			ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi entrare nella nebbia mentre combatti.\n\r", ch);
		return;
	}

	const std::vector<char_data*> group = procarea_entering_group(ch);

	long entrance = 0;
	const int instance_id =
		procarea_create_instance(group_eq_index, PROCAREA_FOUNTAIN_ROOM, entrance);
	if(instance_id < 0 || entrance <= 0) {
		send_to_char(
			"La bruma trema, poi si spezza:\n\r"
			"l'aldila' rifiuta di aprirsi. Riprova tra poco.\n\r",
			ch);
		return;
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Il vortice $c0014si contrae$c0010 su se stesso\n\r"
		"con un suono di vetro sott'acqua.$c0007\n\r");

	for(char_data* member : group) {
		send_to_char(
			"Fai un passo — un solo passo —\n\r"
			"e la nebbia ti ruba il peso del corpo, la voce, persino il nome.\n\r",
			member);
		send_to_char(
			"Senti scale di ghiaccio sotto i piedi che non esistono,\n\r"
			"corridoi che si piegano come alghe al fondo di un mare dimenticato.\n\r",
			member);
		std::ostringstream enter_msg;
		enter_msg << "$c0010Oltre il velo attende un regno effimero$c0007 (eq medio gruppo "
				  << std::fixed << std::setprecision(0) << group_eq_index << ").\n\r";
		send_to_char(enter_msg.str().c_str(), member);
	}

	if(!group.empty()) {
		act("$n e il gruppo vengono risucchiat$i nel vortice di nebbia.", TRUE, group[0], nullptr,
			nullptr, TO_ROOM);
	}

	procarea_dissolve_fountain_veil(true);
	procarea_teleport_group(ch, entrance);
	procarea_touch_instance(instance_id);
}

static void procarea_tick_fountain_veil() {
	const time_t now = time(nullptr);
	if(g_fountain_veil.active && now > g_fountain_veil.expires_at) {
		procarea_send_fountain_plaza(
			"\n\r$c0010La nebbia si stanca:\n\r"
			"filamenti di bruma si distaccano dal vortice\n\r"
			"e svaniscono nel cielo di Myst.$c0007\n\r");
		procarea_dissolve_fountain_veil(true);
		return;
	}

	if(g_fountain_veil.spirit_dismissed && !g_fountain_veil.active &&
	   g_fountain_veil.spirit_dismissed_at > 0 &&
	   now > g_fountain_veil.spirit_dismissed_at + kFountainVeilLifetimeSec) {
		procarea_restore_fountain_spirit(true);
	}
}

static void procarea_leave_via_portal(struct char_data* ch) {
	ProcAreaInstance* inst = procarea_find_instance_by_vnum(ch->in_room);
	if(inst == nullptr) {
		send_to_char("Non sei in un'area effimera.\n\r", ch);
		return;
	}

	if(!inst->exit_portal_open) {
		send_to_char("Devi ancora ripulire l'area: il portale di ritorno non e' aperto.\n\r", ch);
		return;
	}

	if(ch->in_room != inst->boss_vnum) {
		send_to_char("Devi raggiungere la sala finale dove si e' aperto il portale.\n\r", ch);
		return;
	}

	const long dest =
		inst->return_room > 0 ? inst->return_room : PROCAREA_FOUNTAIN_ROOM;
	const std::vector<char_data*> group = procarea_entering_group(ch);
	for(char_data* member : group) {
		send_to_char(
			"Attraversi il portale: un soffio gelido\n\r"
			"ti restituisce alla Piazza delle Nebbie.\n\r",
			member);
	}
	if(dest == PROCAREA_FOUNTAIN_ROOM) {
		for(char_data* member : group) {
			send_to_char(
				"La Fontana della Vita scintilla al sole, indifferente —\n\r"
				"come se non avesse mai chiuso occhio.\n\r",
				member);
		}
	}
	procarea_teleport_group(ch, dest);
	procarea_touch_instance(inst->id);
	procarea_maybe_destroy(inst->id);
}

[[nodiscard]] static bool procarea_prayer_requests_aid(const char* prayer) {
	if(prayer == nullptr || *prayer == '\0') {
		return false;
	}
	std::array<char, MAX_INPUT_LENGTH> lowered{};
	strncpy(lowered.data(), prayer, lowered.size() - 1);
	lowered[lowered.size() - 1] = '\0';
	for(char& c : lowered) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return strstr(lowered.data(), "aiuto") != nullptr;
}

[[nodiscard]] static bool procarea_can_request_darkstar_aid(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(procarea_find_instance_by_vnum(ch->in_room) != nullptr) {
		return true;
	}
	return ch->in_room == PROCAREA_FOUNTAIN_ROOM;
}

static bool procarea_darkstar_aid_impl(struct char_data* ch, const char* prayer) {
	if(ch == nullptr || prayer == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!procarea_prayer_requests_aid(prayer)) {
		return false;
	}
	if(!procarea_can_request_darkstar_aid(ch)) {
		send_to_char(
			"La preghiera si perde nel vento: DarkStar ti ascolta solo\n\r"
			"dalle aree effimere o dalla Piazza delle Nebbie.\n\r",
			ch);
		return true;
	}
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) == nullptr) {
		send_to_char("Il tempio di DarkStar non risponde.\n\r", ch);
		return true;
	}

	const std::vector<char_data*> group = procarea_entering_group(ch);
	if(group.empty()) {
		return true;
	}

	for(size_t i = 0; i < group.size(); ++i) {
		send_to_char(
			"$c0014DarkStar Luce Oscura$c0007 distende un velo di nebbia argentea:\n\r"
			"la foresta del suo tempio ti accoglie.\n\r",
			group[i]);
	}
	if(!group.empty()) {
		act("$n invoca DarkStar: la nebbia li avvolge e li conduce al tempio.",
			TRUE, group[0], nullptr, nullptr, TO_ROOM);
	}
	procarea_teleport_group(ch, PROCAREA_DARKSTAR_TEMPLE);
	return true;
}

static void procarea_on_mob_death_impl(struct char_data* victim) {
	if(victim == nullptr || !IS_NPC(victim)) {
		return;
	}

	ProcAreaInstance* inst = procarea_find_instance_by_vnum(victim->in_room);
	if(inst == nullptr || inst->exit_portal_open) {
		return;
	}

	if(procarea_count_mobs(*inst) != 1) {
		return;
	}

	procarea_open_exit_portal(*inst);
}

static void procarea_boot_darkstar_temple_impl() {
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) != nullptr) {
		mudlog(LOG_CHECK,
			   "procarea: tempio DarkStar %ld gia' presente (world file?); skip boot create",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}
	if(real_roomp(PROCAREA_FOUNTAIN_ROOM) == nullptr) {
		mudlog(LOG_SYSERR,
			   "procarea: impossibile creare tempio DarkStar: piazza %ld assente",
			   PROCAREA_FOUNTAIN_ROOM);
		return;
	}

	allocate_room(PROCAREA_DARKSTAR_TEMPLE);
	struct room_data* rp = real_roomp(PROCAREA_DARKSTAR_TEMPLE);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "procarea: allocate_room(%ld) fallito per tempio DarkStar",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}

	memset(rp, 0, sizeof(*rp));
	rp->number = PROCAREA_DARKSTAR_TEMPLE;
	rp->zone = procarea_assign_zone(PROCAREA_DARKSTAR_TEMPLE);
	rp->sector_type = SECT_FOREST;
	rp->room_flags = static_cast<long>(NO_MOB | PEACEFUL);
	rp->light = 0;
	rp->name = strdup("Il Tempio di DarkStar");
	rp->description = strdup(
		"Il sentiero si perde tra cipressi e nebbia fitta fino a una radura\n"
		"circolare. Al centro sorge un tempio di pietra scura e argento ornato\n"
		"da simboli della Dea DarkStar; una luce livida filtra tra i rami\n"
		"senza scaldare l'aria. L'incenso e l'umido della foresta si mescolano.\n"
		"Qui la Dea accoglie i caduti oltre il velo delle aree effimere.\n"
		"Verso est, oltre gli alberi, scorge la luce della citta'.\n");

	procarea_link_rooms_one_way(PROCAREA_DARKSTAR_TEMPLE, 1, PROCAREA_FOUNTAIN_ROOM,
								"La Piazza delle Nebbie",
								"Un varco tra i tronchi lascia intravedere la piazza e la fontana.");

	mudlog(LOG_CHECK, "procarea: creato tempio DarkStar (vnum %ld) → est piazza %ld",
		   PROCAREA_DARKSTAR_TEMPLE, PROCAREA_FOUNTAIN_ROOM);
}

} // namespace

long procarea_vnum_to_instance(long vnum) {
	if(vnum < PROCAREA_VNUM_BASE) {
		return -1;
	}
	const long offset = vnum - PROCAREA_VNUM_BASE;
	const long instance_id = offset / PROCAREA_SLOTS_PER_INSTANCE + 1;
	const long slot = offset % PROCAREA_SLOTS_PER_INSTANCE;
	if(instance_id < 1 || slot < 0 || slot >= PROCAREA_SLOTS_PER_INSTANCE) {
		return -1;
	}
	return instance_id;
}

bool procarea_is_generated_room(long vnum) {
	return procarea_find_instance_by_vnum(vnum) != nullptr;
}

void procarea_boot_darkstar_temple() {
	procarea_boot_darkstar_temple_impl();
}

void procarea_relocate_pc_corpse_to_temple(struct char_data* ch, struct obj_data* corpse) {
	if(ch == nullptr || corpse == nullptr || IS_NPC(ch)) {
		return;
	}
	if(!procarea_is_generated_room(ch->in_room)) {
		return;
	}
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) == nullptr) {
		return;
	}
	obj_from_room(corpse);
	obj_to_room(corpse, PROCAREA_DARKSTAR_TEMPLE);
	send_to_char(
		"$c0014DarkStar$c0007 raccoglie il tuo corpo e lo depone nel Tempio tra la foresta.\n\r",
		ch);
}

bool procarea_try_darkstar_aid(struct char_data* ch, const char* prayer) {
	return procarea_darkstar_aid_impl(ch, prayer);
}

void procarea_on_mob_death(struct char_data* victim) {
	procarea_on_mob_death_impl(victim);
}

void procarea_maybe_destroy(long instance_id) {
	ProcAreaInstance* inst = procarea_find_instance(static_cast<int>(instance_id));
	if(inst == nullptr) {
		return;
	}
	if(!procarea_instance_has_players(*inst)) {
		procarea_destroy_instance(static_cast<int>(instance_id));
	}
}

void procarea_tick_cleanup() {
	const time_t now = time(nullptr);
	std::vector<int> stale;
	stale.reserve(g_instances.size());

	for(const ProcAreaInstance& inst : g_instances) {
		if(!procarea_instance_has_players(inst) &&
		   (now - inst.last_activity) > 600) {
			stale.push_back(inst.id);
		}
	}

	for(int instance_id : stale) {
		procarea_destroy_instance(instance_id);
	}
	procarea_tick_fountain_veil();
}

ACTION_FUNC(do_antro) {
	if(!IS_PC(ch)) {
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	const std::string_view subcmd = buf.data();

	if(subcmd.empty() || procarea_cmd_is(subcmd, { "entra", "enter" })) {
		send_to_char(
			"In Piazza delle Nebbie:\n\r"
			"$c0014pull fontana$c0007 (allontana lo spirito di DarkStar),\n\r"
			"poi $c0014push fontana$c0007 e infine $c0014enter nebbia$c0007.\n\r",
			ch);
		return;
	}

	if(procarea_cmd_is(subcmd, { "esci", "exit", "leave" })) {
		procarea_leave_via_portal(ch);
		return;
	}

	if(procarea_cmd_is(subcmd, { "info", "status" })) {
		ProcAreaInstance* inst = procarea_find_instance_by_vnum(ch->in_room);
		if(inst == nullptr) {
			send_to_char("Non sei in un'area effimera.\n\r", ch);
			return;
		}
		const char* theme_label = procarea_theme_set(inst->theme_id).label;
		std::ostringstream info_msg;
		info_msg << "Area Effimera #" << inst->id << " (eq medio gruppo "
				 << std::fixed << std::setprecision(0) << inst->group_eq_index << ", tema "
				 << theme_label << "), stanze " << inst->room_vnums.size() << ". ";
		if(inst->exit_portal_open) {
			info_msg << "Il portale verso Myst e' aperto: 'enter portale' nella sala finale.\n\r";
		}
		else {
			info_msg << "Uccidi tutti i nemici per aprire il portale di ritorno.\n\r";
		}
		send_to_char(info_msg.str().c_str(), ch);
		return;
	}

	send_to_char(
		"Uso: pull fontana + push fontana + enter nebbia (Piazza) | antro info | enter portale (sala finale)\n\r",
		ch);
}

ROOMSPECIAL_FUNC(procarea_portal) {
	if(type != EVENT_COMMAND || cmd != CMD_ENTER) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	return procarea_try_enter_nebbia(ch, buf.data()) ? TRUE : FALSE;
}

ROOMSPECIAL_FUNC(procarea_boss_exit) {
	if(type != EVENT_COMMAND || cmd != CMD_ENTER) {
		return FALSE;
	}

	ProcAreaInstance* inst = procarea_find_instance_by_vnum(room->number);
	if(inst == nullptr || !inst->exit_portal_open) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	const std::string_view target = buf.data();
	if(!target.empty() && !procarea_cmd_is(target, { "portale", "commiato" })) {
		return FALSE;
	}

	procarea_leave_via_portal(ch);
	return TRUE;
}

ROOMSPECIAL_FUNC(procarea_t1_portal) {
	return procarea_portal(ch, cmd, arg, room, type);
}

ROOMSPECIAL_FUNC(procarea_t1_exit) {
	return procarea_boss_exit(ch, cmd, arg, room, type);
}

[[nodiscard]] static bool procarea_is_fountain_target(const char* token) {
	return token != nullptr && token[0] != '\0' &&
		   procarea_cmd_is(token, { "fontana", "fountain", "vita" });
}

bool procarea_try_pull_fountain(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(!procarea_is_fountain_target(target.data())) {
		return false;
	}

	procarea_dismiss_fountain_spirit(ch);
	return true;
}

bool procarea_try_push_fountain(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(target[0] == '\0') {
		return false;
	}
	if(!procarea_is_fountain_target(target.data())) {
		return false;
	}

	procarea_invoke_fountain_veil(ch);
	return true;
}

bool procarea_try_enter_nebbia(struct char_data* ch, const char* arg) {
	if(ch == nullptr || arg == nullptr || ch->in_room != PROCAREA_FOUNTAIN_ROOM) {
		return false;
	}

	std::array<char, MAX_INPUT_LENGTH> target {};
	one_argument(arg, target.data());
	if(target[0] == '\0' ||
	   !procarea_cmd_is(target.data(), { "nebbia", "bruma", "vortice", "velo" })) {
		return false;
	}

	procarea_enter_via_veil(ch);
	return true;
}

} // namespace Alarmud
