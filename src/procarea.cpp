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
#include "act.move.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "procarea.hpp"
#include "fight.hpp"
#include "snew.hpp"
#include "utility.hpp"
#include "maximums.hpp"
#include "procarea_band_stats.inc"
#include "procarea_mob_themes.inc"
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_set>
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
	std::string owner_name;
	std::vector<std::string> member_names;
	std::vector<long> room_vnums;
	std::vector<long> treasure_vnums;
	std::unordered_set<long> treasure_claimed;
	bool boss_key_dropped = false;
};

static std::vector<ProcAreaInstance> g_instances;
static int g_next_instance_id = 1;

[[nodiscard]] static ProcAreaInstance* procarea_find_instance(int instance_id);
[[nodiscard]] static ProcAreaInstance* procarea_find_instance_by_vnum(long vnum);

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
static constexpr int kProcTreasureKeyObj = PROCAREA_TREASURE_KEY_OBJ;
static constexpr int kProcTreasureHoardObj = PROCAREA_TREASURE_HOARD_OBJ;
static constexpr int kProcSentinelChancePct = 20;
static constexpr int kProcAggressiveChanceByBand[PROCAREA_TEMPLATE_BANDS] = {
	20,  /* band 0: eq < 2500 */
	40,  /* band 1 */
	60,  /* band 2 */
	80,  /* band 3 */
	100, /* band 4+ */
	100,
};

enum class ProcMobKind { Normal, Boss, Trap };
enum class ProcMobClassContext { Corridor, Treasure };

struct ProcClassChancePct {
	int warrior;
	int cleric;
	int mage;
};

static constexpr ProcClassChancePct kProcCorridorClassByBand[PROCAREA_TEMPLATE_BANDS] = {
	{70, 22, 8},  /* band 0 */
	{65, 25, 10},
	{60, 28, 12},
	{55, 30, 15},
	{50, 30, 20},
	{50, 30, 20},
};

static constexpr ProcClassChancePct kProcTreasureClassByBand[PROCAREA_TEMPLATE_BANDS] = {
	{65, 25, 10}, /* band 0: corridor -5 W, +3 C, +2 M */
	{60, 28, 12},
	{55, 30, 15},
	{50, 33, 17},
	{45, 33, 22},
	{45, 33, 22},
};

static_assert(std::size(kProcCorridorClassByBand) == PROCAREA_TEMPLATE_BANDS,
			  "procarea corridor class table must match PROCAREA_TEMPLATE_BANDS");
static_assert(std::size(kProcTreasureClassByBand) == PROCAREA_TEMPLATE_BANDS,
			  "procarea treasure class table must match PROCAREA_TEMPLATE_BANDS");

static char_data* procarea_create_mob(int archetype_index, float eq_index, int template_band,
									  ProcMobKind kind, bool follow_anchor_sentinel = false,
									  int add_slot = -1,
									  ProcMobClassContext class_ctx = ProcMobClassContext::Corridor);
static char_data* procarea_spawn_scaled_mob(long room_vnum, int template_band, float eq_index,
											  ProcMobKind kind, int theme_id,
											  bool follow_anchor_sentinel = false,
											  int add_slot = -1,
											  ProcMobClassContext class_ctx = ProcMobClassContext::Corridor);
static void procarea_link_boss_add(char_data* add, char_data* boss);
static void procarea_link_anchor_add(char_data* add, char_data* anchor);

#include "procarea_mob_desc.inc"
#include "procarea_themes.inc"

static_assert(std::size(kProcMobArchetypeTexts) == PROCAREA_ARCHETYPE_COUNT,
			  "procarea mob text table must match PROCAREA_ARCHETYPE_COUNT");

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

[[nodiscard]] static float procarea_eq_factor(float eq_index) {
	return std::clamp((eq_index - PROCAREA_EQ_SCALE_MIN) /
						  (PROCAREA_EQ_SCALE_MAX - PROCAREA_EQ_SCALE_MIN),
					  0.0f, 1.0f);
}

[[nodiscard]] static float procarea_lerp_float(float factor, float lo, float hi) {
	return lo + (hi - lo) * factor;
}

[[nodiscard]] static int procarea_lerp_int(float factor, int lo, int hi) {
	return lo + static_cast<int>((hi - lo) * factor + 0.5f);
}

[[nodiscard]] static int procarea_template_band_from_eq(float eq_index) {
	if(eq_index < 2500.0f) {
		return 0;
	}
	if(eq_index < 4500.0f) {
		return 1;
	}
	if(eq_index < 6500.0f) {
		return 2;
	}
	if(eq_index < 8500.0f) {
		return 3;
	}
	if(eq_index < 10500.0f) {
		return 4;
	}
	return 5;
}

[[nodiscard]] static int procarea_archetype_vnum(int archetype_index, int template_band) {
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(archetype_index < 0 || archetype_index >= PROCAREA_ARCHETYPE_COUNT) {
		return 0;
	}
	return PROCAREA_MOB_VNUM_BASE + band * PROCAREA_ARCHETYPE_COUNT + archetype_index;
}

[[nodiscard]] static ProcAreaDifficulty procarea_difficulty_from_eq(float eq_index) {
	const float factor = procarea_eq_factor(eq_index);
	ProcAreaDifficulty diff{};
	diff.eq_index = eq_index;
	diff.factor = factor;
	diff.template_band = procarea_template_band_from_eq(eq_index);
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

[[nodiscard]] static bool procarea_archetype_fits_theme(unsigned long long mask, int theme_id) {
	if(mask == 0) {
		return false;
	}
	if(theme_id < 0 || theme_id >= PROCAREA_THEME_COUNT) {
		return false;
	}
	return (mask & (1ull << theme_id)) != 0;
}

[[nodiscard]] static int procarea_pick_archetype_index(int theme_id, int pool_start, int pool_size) {
	if(pool_size <= 0) {
		return pool_start;
	}
	int themed[PROCAREA_ARCHETYPE_COUNT];
	int themed_count = 0;
	int universal[PROCAREA_ARCHETYPE_COUNT];
	int universal_count = 0;
	for(int i = 0; i < pool_size; ++i) {
		const int idx = pool_start + i;
		if(idx < 0 || idx >= PROCAREA_ARCHETYPE_COUNT) {
			continue;
		}
		const unsigned long long mask = kProcArchetypeThemeMask[idx];
		if(mask == 0) {
			universal[universal_count++] = idx;
		} else if(procarea_archetype_fits_theme(mask, theme_id)) {
			themed[themed_count++] = idx;
		}
	}
	if(themed_count > 0) {
		return themed[number(0, themed_count - 1)];
	}
	if(universal_count > 0) {
		return universal[number(0, universal_count - 1)];
	}
	return pool_start + number(0, pool_size - 1);
}

[[nodiscard]] static int procarea_pick_mob_archetype(int theme_id, bool trap) {
	if(trap) {
		return procarea_pick_archetype_index(
			theme_id, PROCAREA_BOSS_COUNT + PROCAREA_MOB_POOL_SIZE, PROCAREA_TRAP_POOL_SIZE);
	}
	return procarea_pick_archetype_index(theme_id, PROCAREA_BOSS_COUNT, PROCAREA_MOB_POOL_SIZE);
}

[[nodiscard]] static int procarea_pick_boss_archetype(int theme_id) {
	return procarea_pick_archetype_index(theme_id, 0, PROCAREA_BOSS_COUNT);
}

[[nodiscard]] static bool procarea_short_has_article(std::string_view text) {
	static constexpr std::string_view kArticles[] = {
		"Un ", "Una ", "Il ", "Lo ", "La ", "L'", "Uno ", "Un' ",
		"un ", "una ", "il ", "lo ", "la ", "uno ", "un'",
	};
	for(const std::string_view prefix : kArticles) {
		if(text.size() >= prefix.size() &&
		   text.compare(0, prefix.size(), prefix) == 0) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] static char procarea_lower_first(char c) {
	return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

[[nodiscard]] static std::string procarea_lower_first(std::string_view text) {
	if(text.empty()) {
		return {};
	}
	std::string out(text);
	out[0] = procarea_lower_first(out[0]);
	return out;
}

[[nodiscard]] static std::string procarea_article_short(std::string_view title) {
	std::string s(title);
	if(s.empty() || procarea_short_has_article(s)) {
		return s;
	}
	const std::string first = s.substr(0, s.find(' '));
	std::string lower_first = first;
	for(char& c : lower_first) {
		c = procarea_lower_first(c);
	}
	const std::string body = procarea_lower_first(s);

	static const std::unordered_set<std::string> kLoFirst = {
		"gnomo", "scheletro", "spettro",
	};
	static const std::unordered_set<std::string> kLApostropheFirst = {
		"alfa", "esecutore", "ermite", "oracolo", "araldo", "augusto", "antico", "avatar",
		"arpione", "ombra", "augure", "arconte", "archivista", "erinia",
	};
	static const std::unordered_set<std::string> kUnaFirst = {
		"regina", "dama", "matriarca", "larva", "rana", "anguilla", "voce", "divinita",
		"arpia", "sentinella",
	};

	if(kLoFirst.count(lower_first) != 0) {
		return "Lo " + body;
	}
	if(kLApostropheFirst.count(lower_first) != 0 ||
	   (!lower_first.empty() && lower_first[0] == 'a')) {
		return "L'" + body;
	}
	if(kUnaFirst.count(lower_first) != 0 ||
	   (lower_first.size() > 1 && lower_first.back() == 'a' && lower_first != "arma")) {
		return "Una " + body;
	}
	if(lower_first.size() >= 2 && lower_first[0] == 's' &&
	   lower_first[1] != 'a' && lower_first[1] != 'e' && lower_first[1] != 'i' &&
	   lower_first[1] != 'o' && lower_first[1] != 'u' && lower_first[1] != 'h') {
		return "Uno " + body;
	}
	if(!lower_first.empty() &&
	   (lower_first[0] == 'z' || lower_first.rfind("gn", 0) == 0 ||
		lower_first.rfind("ps", 0) == 0 || lower_first.rfind("pn", 0) == 0)) {
		return "Uno " + body;
	}
	return "Un " + body;
}

[[nodiscard]] static char* procarea_dup_text(const char* text, bool trailing_crlf) {
	if(text == nullptr) {
		return strdup("");
	}
	if(!trailing_crlf) {
		return strdup(text);
	}
	const std::size_t len = std::strlen(text);
	if(len >= 2 && text[len - 2] == '\n' && text[len - 1] == '\r') {
		return strdup(text);
	}
	const std::string wrapped = std::string(text) + "\n\r";
	return strdup(wrapped.c_str());
}

static void procarea_apply_band_combat(char_data* mob, int template_band, int archetype_index) {
	if(mob == nullptr) {
		return;
	}
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int archetype =
		std::clamp(archetype_index, 0, PROCAREA_ARCHETYPE_COUNT - 1);
	const ProcArchetypeCombat& combat = kProcBandCombat[band][archetype];

	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = combat.level;
	mob->points.hitroll = static_cast<sbyte>(combat.hitroll);
	mob->points.armor = combat.armor;
	mob->points.max_hit = std::max(1, dice(combat.level, 8) + combat.hp_bonus);
	mob->points.hit = mob->points.max_hit;
	mob->specials.damnodice = static_cast<ubyte>(combat.dam_n);
	mob->specials.damsizedice = static_cast<ubyte>(combat.dam_s);
	mob->points.damroll = static_cast<sbyte>(combat.dam_plus);
	mob->points.gold = combat.gold;
	mob->specials.position = POSITION_STANDING;
	mob->specials.default_pos = POSITION_STANDING;
	mob->player.sex = static_cast<ubyte>(std::clamp(combat.sex, 0, 2));
	mob->immune = combat.immune;
	mob->M_immune = combat.m_immune;
	mob->susc = combat.susc;
	SET_BIT(mob->M_immune, IMM_CHARM);
	mob->susc &= ~IMM_CHARM;
	mob->specials.act = combat.act;
	SET_BIT(mob->specials.act, ACT_ISNPC);
	mob->specials.affected_by = combat.affected_by;
	mob->mult_att = combat.mult_att;
}

static constexpr unsigned long kProcClassActMask =
	ACT_MAGIC_USER | ACT_WARRIOR | ACT_CLERIC | ACT_THIEF | ACT_DRUID | ACT_MONK |
	ACT_BARBARIAN | ACT_PALADIN | ACT_RANGER | ACT_PSI | ACT_ARCHER;

static void procarea_apply_standalone_class(char_data* mob, int template_band,
											ProcMobClassContext class_ctx) {
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const ProcClassChancePct& pct = class_ctx == ProcMobClassContext::Treasure ?
									  kProcTreasureClassByBand[band] :
									  kProcCorridorClassByBand[band];
	const int roll = number(0, 99);
	if(roll < pct.warrior) {
		SET_BIT(mob->specials.act, ACT_WARRIOR);
		mob->specials.spellfail = 101;
		return;
	}
	if(roll < pct.warrior + pct.cleric) {
		SET_BIT(mob->specials.act, ACT_CLERIC);
		mob->specials.spellfail = 0;
		return;
	}
	SET_BIT(mob->specials.act, ACT_MAGIC_USER);
	mob->specials.spellfail = 0;
}

static void procarea_apply_class_role(char_data* mob, ProcMobKind kind, int add_slot,
									  int template_band, ProcMobClassContext class_ctx) {
	if(mob == nullptr) {
		return;
	}
	mob->specials.act &= ~kProcClassActMask;
	if(kind == ProcMobKind::Boss || kind == ProcMobKind::Trap) {
		SET_BIT(mob->specials.act, ACT_MAGIC_USER | ACT_CLERIC);
		mob->specials.spellfail = 0;
		return;
	}
	if(kind != ProcMobKind::Normal) {
		return;
	}
	if(add_slot >= 0) {
		static constexpr unsigned long kAddClassOrder[] = {
			ACT_WARRIOR,
			ACT_CLERIC,
			ACT_MAGIC_USER,
		};
		const int idx = add_slot % static_cast<int>(std::size(kAddClassOrder));
		SET_BIT(mob->specials.act, kAddClassOrder[static_cast<size_t>(idx)]);
		mob->specials.spellfail = (idx == 0) ? 101 : 0;
		return;
	}
	procarea_apply_standalone_class(mob, template_band, class_ctx);
}

static void procarea_apply_sentinel(char_data* mob, ProcMobKind kind, bool follow_anchor_sentinel,
									ProcMobClassContext class_ctx) {
	if(mob == nullptr) {
		return;
	}
	const bool sentinel = kind == ProcMobKind::Boss || kind == ProcMobKind::Trap ||
						  follow_anchor_sentinel ||
						  class_ctx == ProcMobClassContext::Treasure ||
						  number(0, 99) < kProcSentinelChancePct;
	REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
	if(sentinel) {
		SET_BIT(mob->specials.act, ACT_SENTINEL);
	}
}

static void procarea_apply_aggressive(char_data* mob, int template_band, ProcMobKind kind) {
	if(mob == nullptr) {
		return;
	}
	REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
	if(kind == ProcMobKind::Boss || kind == ProcMobKind::Trap) {
		SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
		return;
	}
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int chance = kProcAggressiveChanceByBand[band];
	if(chance >= 100 || number(0, 99) < chance) {
		SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
	}
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

[[nodiscard]] static unsigned procarea_elemental_mask() {
	return IMM_FIRE | IMM_COLD | IMM_ELEC | IMM_ENERGY | IMM_ACID;
}

static void procarea_boss_elemental_defenses(unsigned& m_immune, unsigned& immune_resist) {
	static constexpr unsigned kElemental[] = {
		IMM_FIRE,
		IMM_COLD,
		IMM_ELEC,
		IMM_ENERGY,
		IMM_ACID,
	};
	constexpr int kElementalCount = static_cast<int>(std::size(kElemental));
	const int resist_a = number(0, kElementalCount - 1);
	int resist_b = number(0, kElementalCount - 1);
	while(resist_b == resist_a) {
		resist_b = number(0, kElementalCount - 1);
	}

	m_immune = 0;
	immune_resist = 0;
	for(int i = 0; i < kElementalCount; ++i) {
		if(i == resist_a || i == resist_b) {
			immune_resist |= kElemental[i];
		} else {
			m_immune |= kElemental[i];
		}
	}
}

static void procarea_apply_boss_traits(char_data* mob) {
	if(mob == nullptr) {
		return;
	}

	SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
	SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);

	unsigned elemental_m_immune = 0;
	unsigned elemental_resist = 0;
	procarea_boss_elemental_defenses(elemental_m_immune, elemental_resist);

	const unsigned elemental_mask = procarea_elemental_mask();
	mob->M_immune &= ~elemental_mask;
	mob->M_immune |= IMM_CHARM | elemental_m_immune;
	mob->immune &= ~elemental_mask;
	SET_BIT(mob->immune, IMM_BLUNT | IMM_PIERCE | IMM_SLASH);
	mob->immune |= elemental_resist;
	mob->susc &= ~elemental_mask;
}

static void procarea_scale_mob(char_data* mob, float eq_index, int /*template_band*/,
							   ProcMobKind kind) {
	if(mob == nullptr) {
		return;
	}

	const float factor = procarea_eq_factor(eq_index);
	float ratio = procarea_lerp_float(factor, 0.65f, 2.5f);
	if(kind == ProcMobKind::Boss) {
		ratio *= 1.12f;
		ratio = std::min(ratio, 3.0f);
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

	const float dice_ratio = std::sqrt(ratio);
	if(mob->specials.damnodice > 0) {
		mob->specials.damnodice = static_cast<ubyte>(std::clamp(
			static_cast<int>(mob->specials.damnodice * dice_ratio), 1, 50));
	}
	if(mob->specials.damsizedice > 0) {
		mob->specials.damsizedice = static_cast<ubyte>(std::clamp(
			static_cast<int>(mob->specials.damsizedice * dice_ratio), 1, 127));
	}
	mob->points.hitroll = static_cast<sbyte>(std::clamp(
		static_cast<int>(mob->points.hitroll * dice_ratio), -50, 50));

	if(kind == ProcMobKind::Boss) {
		procarea_apply_boss_traits(mob);
	} else if(kind != ProcMobKind::Trap) {
		if(factor >= 0.50f) {
			SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
			if(factor >= 0.75f && number(0, 99) < 30) {
				SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
			}
		}
	}
}

static char_data* procarea_spawn_scaled_mob(long room_vnum, int template_band, float eq_index,
											ProcMobKind kind, int theme_id,
											bool follow_anchor_sentinel,
											int add_slot,
											ProcMobClassContext class_ctx) {
	const int archetype = procarea_pick_mob_archetype(theme_id, kind == ProcMobKind::Trap);
	char_data* mob = procarea_create_mob(archetype, eq_index, template_band, kind,
										 follow_anchor_sentinel, add_slot, class_ctx);
	if(mob == nullptr) {
		mudlog(LOG_ERROR, "procarea: scaled spawn failed room %ld", room_vnum);
		return nullptr;
	}
	char_to_room(mob, room_vnum);
	return mob;
}

static void procarea_link_boss_add(char_data* add, char_data* boss) {
	procarea_link_anchor_add(add, boss);
}

static void procarea_link_anchor_add(char_data* add, char_data* anchor) {
	if(add == nullptr || anchor == nullptr || add->master != nullptr) {
		return;
	}
	add->master = anchor;
	follow_type* link = nullptr;
	CREATE(link, follow_type, 1);
	link->follower = add;
	link->next = anchor->followers;
	anchor->followers = link;
	SET_BIT(add->specials.affected_by, AFF_GROUP);
}

[[nodiscard]] static char_data* procarea_loot_recipient(char_data* victim) {
	if(victim == nullptr) {
		return nullptr;
	}
	char_data* killer = victim->specials.fighting;
	if(killer != nullptr && IS_NPC(killer) && killer->master != nullptr &&
	   IS_PC(killer->master)) {
		killer = killer->master;
	}
	if(killer != nullptr && IS_PC(killer)) {
		return killer;
	}
	struct room_data* rp = real_roomp(victim->in_room);
	if(rp == nullptr) {
		return nullptr;
	}
	for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
		if(IS_PC(ch)) {
			return ch;
		}
	}
	return nullptr;
}

[[nodiscard]] static bool procarea_is_treasure_key(const struct obj_data* obj) {
	return obj != nullptr && obj->item_number == -1 &&
		   obj->char_vnum == kProcTreasureKeyObj;
}

[[nodiscard]] static bool procarea_is_treasure_hoard(const struct obj_data* obj) {
	return obj != nullptr && obj->item_number == -1 &&
		   obj->char_vnum == kProcTreasureHoardObj;
}

static struct obj_data* procarea_create_runtime_obj(int logical_vnum) {
	struct obj_data* obj = nullptr;
	CREATE(obj, struct obj_data, 1);
	if(obj == nullptr) {
		return nullptr;
	}
	clear_object(obj);
	obj->char_vnum = logical_vnum;
	obj->item_number = -1;
	obj->next = object_list;
	object_list = obj;
	return obj;
}

static struct obj_data* procarea_create_treasure_key(int instance_id) {
	struct obj_data* key = procarea_create_runtime_obj(kProcTreasureKeyObj);
	if(key == nullptr) {
		return nullptr;
	}
	key->name = strdup("chiave tesoro effimero dimensione cumulo forziere");
	key->short_description = strdup("la chiave del tesoro effimero");
	key->description =
		strdup("Una chiave gelida e' qui: rune pallide la legano al guardiano finale.");
	key->obj_flags.type_flag = ITEM_KEY;
	key->obj_flags.wear_flags = ITEM_TAKE;
	key->obj_flags.extra_flags = ITEM_NODROP;
	key->obj_flags.value[0] = instance_id;
	key->obj_flags.weight = 1;
	key->obj_flags.cost = 0;
	return key;
}

static struct obj_data* procarea_create_treasure_hoard() {
	struct obj_data* hoard = procarea_create_runtime_obj(kProcTreasureHoardObj);
	if(hoard == nullptr) {
		return nullptr;
	}
	hoard->name = strdup("cumulo forziere tesoro sigillo reliquie");
	hoard->short_description = strdup("un cumulo sigillato del tesoro");
	hoard->description = strdup(
		"Detriti e metallo contorto nascondono un forziere sigillato da rune effimere.");
	hoard->obj_flags.type_flag = ITEM_CONTAINER;
	hoard->obj_flags.wear_flags = 0;
	hoard->obj_flags.value[0] = 10000;
	hoard->obj_flags.value[1] = CONT_CLOSEABLE | CONT_CLOSED | CONT_LOCKED;
	hoard->obj_flags.value[2] = kProcTreasureKeyObj;
	hoard->obj_flags.weight = 1;
	hoard->obj_flags.cost = 0;
	return hoard;
}

[[nodiscard]] static struct obj_data* procarea_find_treasure_key(char_data* ch, int instance_id) {
	if(ch == nullptr) {
		return nullptr;
	}
	auto matches = [instance_id](struct obj_data* obj) {
		return procarea_is_treasure_key(obj) && obj->obj_flags.value[0] == instance_id;
	};
	for(struct obj_data* obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
		if(matches(obj)) {
			return obj;
		}
	}
	if(matches(ch->equipment[HOLD])) {
		return ch->equipment[HOLD];
	}
	return nullptr;
}

[[nodiscard]] static struct obj_data* procarea_find_treasure_hoard(long room_vnum) {
	struct room_data* rp = real_roomp(room_vnum);
	if(rp == nullptr) {
		return nullptr;
	}
	for(struct obj_data* obj = rp->contents; obj != nullptr; obj = obj->next_content) {
		if(procarea_is_treasure_hoard(obj)) {
			return obj;
		}
	}
	return nullptr;
}

static void procarea_strip_treasure_keys(char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	struct obj_data* next = nullptr;
	for(struct obj_data* obj = ch->carrying; obj != nullptr; obj = next) {
		next = obj->next_content;
		if(procarea_is_treasure_key(obj)) {
			obj_from_char(obj);
			extract_obj(obj);
		}
	}
	if(ch->equipment[HOLD] != nullptr && procarea_is_treasure_key(ch->equipment[HOLD])) {
		struct obj_data* obj = unequip_char(ch, HOLD);
		if(obj != nullptr) {
			extract_obj(obj);
		}
	}
}

[[nodiscard]] static int procarea_treasure_gold_amount(const ProcAreaInstance& inst) {
	const float factor = procarea_eq_factor(inst.group_eq_index);
	const int band = std::clamp(inst.template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int base = 250 + band * 200;
	const float jitter = 0.85f + static_cast<float>(number(0, 30)) / 100.0f;
	return std::max(50, static_cast<int>(base * (1.5f + factor * 2.0f) * jitter));
}

static bool procarea_plant_treasure_hoard(long room_vnum) {
	struct obj_data* hoard = procarea_create_treasure_hoard();
	if(hoard == nullptr) {
		mudlog(LOG_ERROR, "procarea: create treasure hoard failed room %ld", room_vnum);
		return false;
	}
	obj_to_room(hoard, room_vnum);
	return true;
}

static void procarea_drop_boss_treasure_key(ProcAreaInstance& inst, char_data* victim) {
	if(inst.boss_key_dropped || inst.treasure_vnums.empty()) {
		return;
	}
	char_data* recipient = procarea_loot_recipient(victim);
	if(recipient == nullptr) {
		return;
	}
	struct obj_data* key = procarea_create_treasure_key(inst.id);
	if(key == nullptr) {
		mudlog(LOG_ERROR, "procarea: create treasure key failed instance %d", inst.id);
		return;
	}
	obj_to_char(key, recipient);
	inst.boss_key_dropped = true;
	act("$n strappa $p dal cadavere del capo!", TRUE, recipient, key, nullptr, TO_ROOM);
	act("Strappi $p dal cadavere del capo!", FALSE, recipient, key, nullptr, TO_CHAR);
	send_to_room(
		"\n\r$c0014La chiave del tesoro effimero e' tua: cerca i cumuli sigillati nei rami laterali.$c0007\n\r",
		victim->in_room);
}

static void procarea_grant_treasure_loot(char_data* ch, ProcAreaInstance& inst, long room_vnum) {
	if(ch == nullptr || inst.treasure_claimed.count(room_vnum) != 0) {
		return;
	}
	inst.treasure_claimed.insert(room_vnum);

	const int gold = procarea_treasure_gold_amount(inst);
	if(gold > 0) {
		struct obj_data* money = create_money(gold);
		if(money != nullptr) {
			obj_to_room(money, room_vnum);
		}
	}

	struct obj_data* hoard = procarea_find_treasure_hoard(room_vnum);
	if(hoard != nullptr) {
		REMOVE_BIT(hoard->obj_flags.value[1], CONT_LOCKED);
	}

	act("$n apre il cumulo sigillato con la chiave del capo!", TRUE, ch, nullptr, nullptr, TO_ROOM);
	act("Apri il cumulo sigillato: oro e reliquie effimere fuoriescono dalle rune!", FALSE, ch,
		nullptr, nullptr, TO_CHAR);
	send_to_room(
		"\n\r$c0013Il sigillo cede: il tesoro della dimensione giace ora a terra.$c0007\n\r",
		room_vnum);
}

static bool procarea_try_open_treasure(struct char_data* ch, struct room_data* room,
									   std::string_view target) {
	if(ch == nullptr || room == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!procarea_cmd_is(target, { "cumulo", "forziere", "tesoro", "sigillo", "hoard" })) {
		return false;
	}

	ProcAreaInstance* inst = procarea_find_instance_by_vnum(room->number);
	if(inst == nullptr) {
		return false;
	}
	if(procarea_find_treasure_hoard(room->number) == nullptr) {
		send_to_char("Non c'e' nulla da aprire qui.\n\r", ch);
		return true;
	}
	if(inst->treasure_claimed.count(room->number) != 0) {
		send_to_char("Il cumulo e' gia' stato saccheggiato.\n\r", ch);
		return true;
	}
	if(!inst->boss_key_dropped) {
		send_to_char("Il sigillo resiste: ti serve la chiave del capo finale.\n\r", ch);
		return true;
	}
	if(procarea_find_treasure_key(ch, inst->id) == nullptr) {
		send_to_char("Non hai la chiave del tesoro effimero.\n\r", ch);
		return true;
	}

	procarea_grant_treasure_loot(ch, *inst, room->number);
	return true;
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

static struct reset_com* procarea_empty_zone_reset() {
	struct reset_com* cmd = nullptr;
	CREATE(cmd, struct reset_com, 1);
	cmd->command = 'S';
	return cmd;
}

static void procarea_init_boot_zone(int idx, int num, const char* name, int top, int lifespan,
									int reset_mode) {
	struct zone_data* z = &zone_table[idx];
	z->num = num;
	z->name = strdup(name);
	z->top = top;
	z->lifespan = lifespan;
	z->reset_mode = reset_mode;
	z->bottom = -1;
	z->age = 0;
	z->start = 0;
	z->cmd = procarea_empty_zone_reset();
	memset(z->races, 0, sizeof(z->races));
	z->CurrTotGold = 0.0f;
	z->LastTotGold = 0.0f;
	z->CurrToPCGold = 0.0f;
	z->LastToPCGold = 0.0f;
}

static void procarea_boot_zone_impl() {
	if(zone_table == nullptr || top_of_zone_table < 0) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: zone table missing");
		return;
	}

	for(int i = 0; i <= top_of_zone_table; ++i) {
		if(zone_table[i].num == PROCAREA_ZONE_NUM && zone_table[i].top == PROCAREA_ZONE_TOP) {
			mudlog(LOG_CHECK, "procarea_boot_zone: zone #%d already installed", PROCAREA_ZONE_NUM);
			return;
		}
	}

	int buffer_idx = -1;
	for(int i = top_of_zone_table; i >= 0; --i) {
		if(zone_table[i].num == PROCAREA_LAST_AREA_ZONE_NUM) {
			buffer_idx = i;
			break;
		}
	}
	if(buffer_idx < 0) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: zone #%d (last_area) not found",
			   PROCAREA_LAST_AREA_ZONE_NUM);
		return;
	}
	if(buffer_idx != top_of_zone_table) {
		mudlog(LOG_SYSERR,
			   "procarea_boot_zone: last_area at index %d, expected last index %d", buffer_idx,
			   top_of_zone_table);
		return;
	}
	if(zone_table[buffer_idx].top < PROCAREA_ZONE_BUFFER_TOP) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: last_area top %d too low (need >= %d)",
			   zone_table[buffer_idx].top, PROCAREA_ZONE_BUFFER_TOP);
		return;
	}

	zone_table[buffer_idx].top = PROCAREA_ZONE_BUFFER_TOP;

	const int antro_idx = buffer_idx + 1;
	const int pin_idx = buffer_idx + 2;
	const int new_capacity = pin_idx + 10;
	zone_table = static_cast<struct zone_data*>(
		realloc(zone_table, static_cast<size_t>(new_capacity) * sizeof(struct zone_data)));
	if(zone_table == nullptr) {
		mudlog(LOG_SYSERR, "procarea_boot_zone: realloc failed");
		return;
	}

	procarea_init_boot_zone(antro_idx, PROCAREA_ZONE_NUM, "Dimensione Effimera", PROCAREA_ZONE_TOP, 60,
							ZONE_ASTRAL);
	procarea_init_boot_zone(pin_idx, PROCAREA_LAST_AREA_ZONE_NUM, "last_area", PROCAREA_ZONE_PIN, 60,
							ZONE_NEVER);

	top_of_zone_table = pin_idx;

	mudlog(LOG_CHECK,
		   "procarea_boot_zone: #%d buffer top=%d, #%d antro %ld-%d (astral), #%d pin=%d",
		   PROCAREA_LAST_AREA_ZONE_NUM, PROCAREA_ZONE_BUFFER_TOP, PROCAREA_ZONE_NUM,
		   PROCAREA_VNUM_BASE, PROCAREA_ZONE_TOP, PROCAREA_LAST_AREA_ZONE_NUM, PROCAREA_ZONE_PIN);
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

static char_data* procarea_create_mob(int archetype_index, float eq_index, int template_band,
									  ProcMobKind kind, bool follow_anchor_sentinel,
									  int add_slot, ProcMobClassContext class_ctx) {
	if(archetype_index < 0 || archetype_index >= PROCAREA_ARCHETYPE_COUNT) {
		mudlog(LOG_ERROR, "procarea: invalid archetype index %d", archetype_index);
		return nullptr;
	}

	const ProcMobArchetypeText& text = kProcMobArchetypeTexts[archetype_index];
	char_data* mob = nullptr;
	CREATE(mob, char_data, 1);
	if(mob == nullptr) {
		mudlog(LOG_ERROR, "procarea: CREATE mob failed for archetype %d", archetype_index);
		return nullptr;
	}

	clear_char(mob);
	mob->specials.last_direction = -1;
	mob->mult_att = 1.0f;
	mob->specials.spellfail = 101;
	mob->specials.mobtype = 'L';

	const std::string short_desc = procarea_article_short(text.short_title);
	mob->player.name = strdup(text.keywords != nullptr ? text.keywords : "");
	mob->player.short_descr = strdup(short_desc.c_str());
	mob->player.long_descr = procarea_dup_text(text.long_desc, true);
	mob->player.description = procarea_dup_text(text.look, true);
	mob->player.sounds = procarea_dup_text(text.agg, false);
	mob->player.distant_snds = procarea_dup_text(text.sound, true);
	mob->player.title = nullptr;

	SET_BIT(mob->specials.act, ACT_ISNPC);
	mob->player.iClass = 0;
	mob->player.time.birth = time(nullptr);
	mob->player.time.played = 0;
	mob->player.time.logon = time(nullptr);
	mob->player.weight = 200;
	mob->player.height = 198;
	for(int i = 0; i < 3; ++i) {
		GET_COND(mob, i) = -1;
	}
	for(int i = 0; i < MAX_WEAR; ++i) {
		mob->equipment[i] = nullptr;
	}

	procarea_apply_band_combat(mob, template_band, archetype_index);
	procarea_apply_aggressive(mob, template_band, kind);
	procarea_apply_class_role(mob, kind, add_slot, template_band, class_ctx);
	procarea_apply_sentinel(mob, kind, follow_anchor_sentinel, class_ctx);
	procarea_scale_mob(mob, eq_index, template_band, kind);

	const int level = GET_LEVEL(mob, WARRIOR_LEVEL_IND);
	mob->abilities.str = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->abilities.intel = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->abilities.wis = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->abilities.dex = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->abilities.con = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->abilities.chr = static_cast<ubyte>(MIN(10 + number(0, MAX(1, level / 5)), 18));
	mob->tmpabilities = mob->abilities;
	mob->points.max_mana = 100;
	mob->points.max_move = NewMobMov(mob);
	for(int i = 0; i < 5; ++i) {
		mob->specials.apply_saving_throw[i] =
			static_cast<sbyte>(MAX(20 - level, 2));
	}

	GET_EXP(mob) = DetermineExp(mob, 0) + mob->points.gold;
	if(IS_SET(mob->specials.act, ACT_WIMPY)) {
		GET_EXP(mob) -= GET_EXP(mob) / 10;
	}
	if(IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		GET_EXP(mob) += GET_EXP(mob) / 10;
		if(!IS_SET(mob->specials.act, ACT_WIMPY) ||
		   IS_SET(mob->specials.act, ACT_META_AGG)) {
			GET_EXP(mob) += GET_EXP(mob) / 2;
		}
	}

	mob->nr = -1;
	mob->generic = procarea_archetype_vnum(archetype_index, template_band);
	mob->commandp = static_cast<int>(kind);
	GET_RACE(mob) = text.race;
	SetRacialStuff(mob);
	mob->points.mana = mana_limit(mob);
	mob->points.move = move_limit(mob);
	mob->specials.tick = mob_tick_count++;
	if(mob_tick_count == TICK_WRAP_COUNT) {
		mob_tick_count = 0;
	}

	mob->next = character_list;
	character_list = mob;
	mob_count++;

	return mob;
}

static void procarea_populate_room(const ProcAreaDifficulty& diff, long room_vnum,
								   ProcArchetype type, int depth, int max_depth, int theme_id) {
	const int depth_bonus = procarea_depth_spawn_bonus(diff, depth, max_depth);

	switch(type) {
	case ProcArchetype::Entrance:
		break;
	case ProcArchetype::Corridor: {
		const int chance = std::clamp(diff.corridor_spawn_pct + depth_bonus, 0, 95);
		if(number(0, 99) < chance) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal, theme_id);
		}
		if(depth >= std::max(2, max_depth / 3) &&
		   number(0, 99) < std::clamp(25 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal, theme_id);
		}
		break;
	}
	case ProcArchetype::Treasure: {
		const int chance = std::clamp(diff.treasure_spawn_pct + depth_bonus, 0, 98);
		if(number(0, 99) < chance) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal, theme_id, false, -1,
									  ProcMobClassContext::Treasure);
		}
		if(number(0, 99) < std::clamp(40 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal, theme_id, false, -1,
									  ProcMobClassContext::Treasure);
		}
		procarea_plant_treasure_hoard(room_vnum);
		break;
	}
	case ProcArchetype::Trap: {
		char_data* trap =
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Trap, theme_id);
		int add_slot = 0;
		char_data* add =
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  ProcMobKind::Normal, theme_id, true, add_slot++);
		if(trap != nullptr && add != nullptr) {
			procarea_link_anchor_add(add, trap);
		}
		if(number(0, 99) < std::clamp(50 + depth_bonus, 0, 99)) {
			add = procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
											ProcMobKind::Normal, theme_id, true, add_slot++);
			if(trap != nullptr && add != nullptr) {
				procarea_link_anchor_add(add, trap);
			}
		}
		break;
	}
	case ProcArchetype::Boss: {
		const int boss_idx = procarea_pick_boss_archetype(theme_id);
		char_data* boss = procarea_create_mob(boss_idx, diff.eq_index, diff.template_band,
											  ProcMobKind::Boss);
		if(boss == nullptr) {
			break;
		}
		char_to_room(boss, room_vnum);
		for(int i = 0; i < diff.boss_adds; ++i) {
			char_data* add =
				procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
										  ProcMobKind::Normal, theme_id, true, i);
			if(add != nullptr) {
				procarea_link_boss_add(add, boss);
			}
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

[[nodiscard]] static const char* procarea_instance_owner_key(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return nullptr;
	}
	if(ch->master != nullptr && IS_PC(ch->master)) {
		return GET_NAME(ch->master);
	}
	return GET_NAME(ch);
}

[[nodiscard]] static bool procarea_instance_has_member(const ProcAreaInstance& inst,
													   const char* name) {
	if(name == nullptr || *name == '\0') {
		return false;
	}
	for(const std::string& member : inst.member_names) {
		if(member == name) {
			return true;
		}
	}
	return false;
}

static void procarea_record_instance_members(ProcAreaInstance& inst,
											 const std::vector<char_data*>& group) {
	for(char_data* member : group) {
		if(member == nullptr || !IS_PC(member)) {
			continue;
		}
		const char* name = GET_NAME(member);
		if(name == nullptr || *name == '\0') {
			continue;
		}
		if(!procarea_instance_has_member(inst, name)) {
			inst.member_names.emplace_back(name);
		}
	}
}

[[nodiscard]] static ProcAreaInstance* procarea_find_instance_for_ch(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return nullptr;
	}
	const char* const self = GET_NAME(ch);
	const char* const owner_key = procarea_instance_owner_key(ch);
	for(ProcAreaInstance& inst : g_instances) {
		if(owner_key != nullptr && inst.owner_name == owner_key) {
			return &inst;
		}
		if(procarea_instance_has_member(inst, self)) {
			return &inst;
		}
	}
	return nullptr;
}

static int procarea_create_instance(float group_eq_index, long return_room, long& entrance_vnum,
									const char* owner_name) {
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
	if(owner_name != nullptr && *owner_name != '\0') {
		inst.owner_name = owner_name;
	}
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
		procarea_populate_room(diff, vnum, layout[i].type, depth, max_depth, inst.theme_id);

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
		if(layout[i].type == ProcArchetype::Treasure) {
			inst.treasure_vnums.push_back(vnum);
			if(room_data* treasure_room = real_roomp(vnum); treasure_room != nullptr) {
				treasure_room->funct = procarea_treasure;
				treasure_room->specname = "procarea_treasure";
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

	if(ch->specials.fighting) {
		stop_fighting(ch);
	}

	char_from_room(ch);
	char_to_room(ch, dest);
	act("$n emerge dalla nebbia.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	do_look(ch, "", CMD_LOOK);
}

static void procarea_teleport_group(char_data* ch, long dest) {
	const bool leaving_instance =
		ch != nullptr && procarea_is_generated_room(ch->in_room) &&
		!procarea_is_generated_room(dest);
	const std::vector<char_data*> group = procarea_entering_group(ch);
	for(size_t i = 0; i < group.size(); ++i) {
		if(leaving_instance) {
			procarea_strip_treasure_keys(group[i]);
		}
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
			"lucida e immobile... come se la nebbia non fosse mai passata.$c0007\n\r");
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
		"Una figura eterea si stacca dalla schiuma...\n\r"
		"$c0014DarkStar Luce Oscura$c0007 ti fissa un istante,\n\r"
		"poi viene risucchiata verso l'alto.\n\r",
		ch);
	act("$n tira con forza dalla Fontana della Vita.", TRUE, ch, nullptr, nullptr, TO_ROOM);

	procarea_send_fountain_plaza(
		"\n\r$c0010La Fontana della Vita vibra:\n\r"
		"un'aura $c0014luce-oscura$c0010 si stacca dal centro del bacino,\n\r"
		"sospesa tra cielo e acqua.$c0007\n\r");
	procarea_send_fountain_plaza(
		"$c0010Per un attimo la $c0014Dea DarkStar$c0010 sembra volersi opporre...\n\r"
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
			"Allontana prima lo spirito della Dea con $c0014tirando via lafontana$c0007.\n\r",
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
		"Un rantolo sommerso risponde al tuo spinta...\n\r"
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
		send_to_char("Sei gia' dentro una Dimensione Effimera.\n\r", ch);
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
			"$c0014tirando via la fontana$c0007.\n\r",
			ch);
		return;
	}

	if(!g_fountain_veil.active) {
		send_to_char(
			"Non c'e' alcun velo di nebbia da attraversare.\n\r"
			"Evocalo, ti basta spingere la fontana per farlo apparire.\n\r",
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

	if(procarea_find_instance_for_ch(ch) != nullptr) {
		send_to_char(
			"Hai gia' una Dimensione Effimera aperta"
			" (o ne fai ancora parte).\n\r"
			"Usa $c0014pray darkstar aiuto$c0007 per rientrare,\n\r"
			"oppure attendi che si dissolva da sola.\n\r",
			ch);
		return;
	}

	long entrance = 0;
	const char* owner = group.empty() ? GET_NAME(ch) : GET_NAME(group[0]);
	const int instance_id =
		procarea_create_instance(group_eq_index, PROCAREA_FOUNTAIN_ROOM, entrance, owner);
	if(instance_id < 0 || entrance <= 0) {
		send_to_char(
			"La bruma trema, poi si spezza:\n\r"
			"l'aldila' rifiuta di aprirsi. Riprova tra poco.\n\r",
			ch);
		return;
	}

	if(ProcAreaInstance* inst = procarea_find_instance(instance_id); inst != nullptr) {
		procarea_record_instance_members(*inst, group);
	}

	procarea_send_fountain_plaza(
		"\n\r$c0010Il vortice $c0014si contrae$c0010 su se stesso\n\r"
		"con un suono di vetro sott'acqua.$c0007\n\r");

	for(char_data* member : group) {
		send_to_char(
			"Fai un passo... un solo passo...\n\r"
			"e la nebbia ti ruba il peso del corpo, la voce, persino il nome.\n\r",
			member);
		send_to_char(
			"Senti scale di ghiaccio sotto i piedi che non esistono,\n\r"
			"corridoi che si piegano come alghe al fondo di un mare dimenticato.\n\r",
			member);
		std::ostringstream enter_msg;
		enter_msg << "$c0010Oltre il velo attende una Dimensione Effimera$c0007 (eq medio gruppo "
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
		send_to_char("Non sei in una Dimensione Effimera.\n\r", ch);
		return;
	}

	if(!inst->exit_portal_open) {
		send_to_char(
			"Devi ancora ripulire la Dimensione Effimera: il portale di ritorno non e' aperto.\n\r",
			ch);
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
				"La Fontana della Vita scintilla al sole, indifferente...\n\r"
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
	if(ch->in_room == PROCAREA_FOUNTAIN_ROOM) {
		return true;
	}
	if(ch->in_room == PROCAREA_DARKSTAR_TEMPLE && procarea_find_instance_for_ch(ch) != nullptr) {
		return true;
	}
	return false;
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
			"dalle Dimensioni Effimere o dalla Piazza delle Nebbie.\n\r",
			ch);
		return true;
	}
	if(real_roomp(PROCAREA_DARKSTAR_TEMPLE) == nullptr) {
		send_to_char("Il tempio di DarkStar non risponde.\n\r", ch);
		return true;
	}

	if(ch->specials.fighting) {
		send_to_char("Non puoi invocare DarkStar mentre combatti.\n\r", ch);
		return true;
	}

	const std::vector<char_data*> group = procarea_entering_group(ch);
	if(group.empty()) {
		return true;
	}

	ProcAreaInstance* const in_inst = procarea_find_instance_by_vnum(ch->in_room);
	if(in_inst != nullptr) {
		for(size_t i = 0; i < group.size(); ++i) {
			send_to_char(
				"$c0014DarkStar Luce Oscura$c0007 distende un velo di nebbia argentea:\n\r"
				"la foresta del suo tempio ti accoglie.\n\r",
				group[i]);
		}
		if(!group.empty()) {
			act("$n invoca DarkStar: la nebbia li avvolge e li conduce al tempio.", TRUE, group[0],
				nullptr, nullptr, TO_ROOM);
		}
		procarea_teleport_group(ch, PROCAREA_DARKSTAR_TEMPLE);
		procarea_touch_instance(in_inst->id);
		return true;
	}

	ProcAreaInstance* const return_inst = procarea_find_instance_for_ch(ch);
	if(return_inst != nullptr &&
	   (ch->in_room == PROCAREA_FOUNTAIN_ROOM || ch->in_room == PROCAREA_DARKSTAR_TEMPLE)) {
		if(return_inst->entrance_vnum <= 0 ||
		   real_roomp(return_inst->entrance_vnum) == nullptr) {
			send_to_char("La Dimensione Effimera non risponde piu' alla preghiera.\n\r", ch);
			return true;
		}
		procarea_record_instance_members(*return_inst, group);
		for(size_t i = 0; i < group.size(); ++i) {
			send_to_char(
				"$c0014DarkStar Luce Oscura$c0007 apre un varco nella nebbia:\n\r"
				"ti richiama dentro la Dimensione Effimera.\n\r",
				group[i]);
		}
		if(!group.empty()) {
			act("$n invoca DarkStar: la nebbia li risucchia di nuovo nella Dimensione Effimera.", TRUE,
				group[0],
				nullptr, nullptr, TO_ROOM);
		}
		procarea_teleport_group(ch, return_inst->entrance_vnum);
		procarea_touch_instance(return_inst->id);
		return true;
	}

	for(size_t i = 0; i < group.size(); ++i) {
		send_to_char(
			"$c0014DarkStar Luce Oscura$c0007 distende un velo di nebbia argentea:\n\r"
			"la foresta del suo tempio ti accoglie.\n\r",
			group[i]);
	}
	if(!group.empty()) {
		act("$n invoca DarkStar: la nebbia li avvolge e li conduce al tempio.", TRUE, group[0],
			nullptr, nullptr, TO_ROOM);
	}
	procarea_teleport_group(ch, PROCAREA_DARKSTAR_TEMPLE);
	return true;
}

static void procarea_on_mob_death_impl(struct char_data* victim) {
	if(victim == nullptr || !IS_NPC(victim)) {
		return;
	}

	ProcAreaInstance* inst = procarea_find_instance_by_vnum(victim->in_room);
	if(inst == nullptr) {
		return;
	}

	if(victim->commandp == static_cast<int>(ProcMobKind::Boss)) {
		procarea_drop_boss_treasure_key(*inst, victim);
	}

	if(inst->exit_portal_open) {
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
			   "procarea: DarkStar temple %ld already exists (world file?); skipping boot create",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}
	if(real_roomp(PROCAREA_FOUNTAIN_ROOM) == nullptr) {
		mudlog(LOG_SYSERR,
			   "procarea: cannot create DarkStar temple: fountain square %ld missing",
			   PROCAREA_FOUNTAIN_ROOM);
		return;
	}

	allocate_room(PROCAREA_DARKSTAR_TEMPLE);
	struct room_data* rp = real_roomp(PROCAREA_DARKSTAR_TEMPLE);
	if(rp == nullptr) {
		mudlog(LOG_SYSERR, "procarea: allocate_room(%ld) failed for DarkStar temple",
			   PROCAREA_DARKSTAR_TEMPLE);
		return;
	}

	memset(rp, 0, sizeof(*rp));
	rp->number = PROCAREA_DARKSTAR_TEMPLE;
	rp->zone = procarea_assign_zone(PROCAREA_DARKSTAR_TEMPLE);
	rp->sector_type = SECT_FOREST;
	rp->room_flags = static_cast<long>(NO_MOB | PEACEFUL | INDOORS | BRIGHT);
	rp->light = 1;
	rp->name = strdup("Il Tempio di DarkStar");
	rp->description = strdup(
		"Il sentiero si perde tra cipressi e nebbia fitta; lungo il cammino\n"
		"torce d'argento fissate ai tronchi gettano una luce fredda e costante.\n"
		"La radura circolare accoglie un tempio di pietra scura e argento ornato\n"
		"da simboli della Dea DarkStar; bracieri e torce ai quattro angoli\n"
		"tengono l'oscurita' lontana anche quando la notte avvolge la foresta.\n"
		"L'incenso e l'umido della foresta si mescolano.\n"
		"Qui la Dea accoglie i caduti oltre il velo delle Dimensioni Effimere.\n"
		"Verso est, oltre gli alberi, scorge la luce della citta'.\n");

	procarea_link_rooms_one_way(PROCAREA_DARKSTAR_TEMPLE, 1, PROCAREA_FOUNTAIN_ROOM,
								"La Piazza delle Nebbie",
								"Un varco tra i tronchi lascia intravedere la piazza e la fontana.\n");

	mudlog(LOG_CHECK, "procarea: created DarkStar temple (vnum %ld) east exit to square %ld",
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

void procarea_boot_zone() {
	procarea_boot_zone_impl();
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
		"$c0014DarkStar$c0007 raccoglie il tuo corpo e lo depone nel Tempio nella foresta.\n\r",
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
			"devi tirare la fontana per allontanare lo spirito di DarkStar,\n\r"
			"poi con tutta la tua forza devi spingere la fontana ed infine $c0014entrare nel velo della nebbia$c0007.\n\r",
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
			send_to_char("Non sei in una Dimensione Effimera.\n\r", ch);
			return;
		}
		const char* theme_label = procarea_theme_set(inst->theme_id).label;
		std::ostringstream info_msg;
		info_msg << "Dimensione Effimera #" << inst->id << " (eq medio gruppo "
				 << std::fixed << std::setprecision(0) << inst->group_eq_index << ", tema "
				 << theme_label << "), stanze " << inst->room_vnums.size() << ". ";
		if(inst->exit_portal_open) {
			info_msg << "Il portale verso Myst e' aperto: 'enter portale' nella sala finale.\n\r";
		}
		else {
			info_msg << "Uccidi tutti i nemici per aprire il portale di ritorno.\n\r";
		}
		if(!inst->treasure_vnums.empty()) {
			info_msg << "Stanze tesoro: " << inst->treasure_vnums.size()
					 << ". La chiave cade dal capo finale; apri il cumulo con "
					 << "'apri cumulo'.\n\r";
			if(inst->boss_key_dropped) {
				info_msg << "Hai gia' ottenuto la chiave del tesoro.\n\r";
			}
		}
		send_to_char(info_msg.str().c_str(), ch);
		return;
	}

	send_to_char(
		"Uso: pull fontana + push fontana + enter nebbia (Piazza) | dimensione info | enter portale (sala finale)\n\r",
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

ROOMSPECIAL_FUNC(procarea_treasure) {
	if(type != EVENT_COMMAND) {
		return FALSE;
	}
	if(cmd != CMD_OPEN && cmd != CMD_UNLOCK) {
		return FALSE;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	one_argument(arg, buf.data());
	return procarea_try_open_treasure(ch, room, buf.data()) ? TRUE : FALSE;
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

int procarea_mob_iVNum(const char_data* mob) {
	if(mob == nullptr || !IS_NPC(mob)) {
		return 0;
	}
	if(mob->nr >= 0 && mob->nr <= top_of_mobt) {
		return mob_index[mob->nr].iVNum;
	}
	return mob->generic;
}

} // namespace Alarmud
