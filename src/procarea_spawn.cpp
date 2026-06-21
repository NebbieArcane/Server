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
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "procarea.hpp"
#include "procarea_internal.hpp"
#include "fight.hpp"
#include "snew.hpp"
#include "utility.hpp"
#include "spells.hpp"
#include "maximums.hpp"
#include "procarea_band_stats.inc"
#include "procarea_mob_themes.inc"
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <sys/stat.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace Alarmud {
namespace procarea_internal {

#include "procarea_reward_gear.inc"
#include "procarea_reward_names.inc"

static constexpr long kProcIsolationFlags = static_cast<long>(NO_ASTRAL | NO_SUM);
static constexpr int kProcExitPortalObj = 9071;
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
									  int group_max_level, ProcMobKind kind,
									  bool follow_anchor_sentinel = false,
									  int add_slot = -1,
									  ProcMobClassContext class_ctx = ProcMobClassContext::Corridor,
									  bool solo_mode = false, float party_power_mult = 1.0f);
static char_data* procarea_spawn_scaled_mob(long room_vnum, int template_band, float eq_index,
											  int group_max_level, ProcMobKind kind, int theme_id,
											  bool follow_anchor_sentinel = false,
											  int add_slot = -1,
											  ProcMobClassContext class_ctx = ProcMobClassContext::Corridor,
											  bool solo_mode = false, float party_power_mult = 1.0f);

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

[[nodiscard]] const ProcThemeSet& theme_set(int theme_id) {
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

[[nodiscard]] static ProcAreaDifficulty procarea_difficulty_apply_solo(ProcAreaDifficulty diff) {
	diff.rooms_min = std::max(10, diff.rooms_min * 45 / 100);
	diff.rooms_max = std::max(15, diff.rooms_max * 45 / 100);
	diff.max_branches = std::max(1, diff.max_branches - 2);
	diff.branch_chance = std::max(15, diff.branch_chance - 12);
	diff.corridor_spawn_pct = std::max(35, diff.corridor_spawn_pct * 65 / 100);
	diff.treasure_spawn_pct = std::max(50, diff.treasure_spawn_pct - 8);
	diff.boss_adds = std::max(0, diff.boss_adds - 1);
	diff.depth_extra_pct = std::max(2, diff.depth_extra_pct - 3);
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
[[nodiscard]] static int procarea_spawn_level(int group_max_level, int template_band,
											  ProcMobKind kind) {
	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const int fascia = band + 1;
	const int mob_level =
		std::clamp((group_max_level - 1) + fascia, 1, PROCAREA_MOB_LEVEL_CAP);
	switch(kind) {
	case ProcMobKind::Boss:
		return std::clamp(mob_level + 3, 1, PROCAREA_MOB_LEVEL_CAP);
	case ProcMobKind::Trap:
		return std::clamp(mob_level + number(1, 2), 1, PROCAREA_MOB_LEVEL_CAP);
	default:
		return mob_level;
	}
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

[[nodiscard]] static float procarea_party_power_mult(int party_size) {
	const int clamped = std::clamp(party_size, 1, PROCAREA_PARTY_SIZE_CAP);
	const int extra = std::max(0, clamped - PROCAREA_PARTY_BASE_SIZE);
	return 1.0f + static_cast<float>(extra) * PROCAREA_PARTY_EXTRA_MULT;
}

[[nodiscard]] static float procarea_solo_kind_mult(ProcMobKind kind) {
	switch(kind) {
	case ProcMobKind::Boss:
		return 0.85f;
	case ProcMobKind::Trap:
		return 0.78f;
	default:
		return 0.70f;
	}
}

static void procarea_apply_scaling_affects(char_data* mob, float eq_index, ProcMobKind kind,
										   ProcMobClassContext class_ctx, bool solo_mode) {
	if(mob == nullptr) {
		return;
	}

	if(kind == ProcMobKind::Boss) {
		procarea_apply_boss_traits(mob);
		return;
	}

	const float factor = procarea_eq_factor(eq_index);
	if(solo_mode) {
		const bool trap_or_treasure = kind == ProcMobKind::Trap ||
									  class_ctx == ProcMobClassContext::Treasure ||
									  class_ctx == ProcMobClassContext::Trap;
		if(trap_or_treasure) {
			SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
			const int fireshield_pct = (kind == ProcMobKind::Trap ||
										class_ctx == ProcMobClassContext::Trap) ?
										   20 :
										   25;
			if(number(0, 99) < fireshield_pct) {
				SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
			}
			return;
		}
		if(number(0, 99) < 60) {
			SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
			if(number(0, 99) < 15) {
				SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
			}
		}
		return;
	}

	if(kind == ProcMobKind::Trap) {
		return;
	}
	if(factor >= 0.50f) {
		SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
		if(factor >= 0.75f && number(0, 99) < 30) {
			SET_BIT(mob->specials.affected_by, AFF_FIRESHIELD);
		}
	}
}

static void procarea_apply_power_delta(char_data* mob, float delta) {
	if(mob == nullptr || delta <= 0.0f) {
		return;
	}

	mob->points.max_hit = std::max(1, static_cast<int>(mob->points.max_hit * delta));
	mob->points.hit = std::max(1, static_cast<int>(mob->points.hit * delta));
	mob->points.damroll = static_cast<sbyte>(std::clamp(
		static_cast<int>(mob->points.damroll * delta), -30, 40));
	mob->points.armor = static_cast<int>(mob->points.armor * delta);

	const float dice_delta = std::sqrt(delta);
	if(mob->specials.damnodice > 0) {
		mob->specials.damnodice = static_cast<ubyte>(std::clamp(
			static_cast<int>(mob->specials.damnodice * dice_delta), 1, 50));
	}
	if(mob->specials.damsizedice > 0) {
		mob->specials.damsizedice = static_cast<ubyte>(std::clamp(
			static_cast<int>(mob->specials.damsizedice * dice_delta), 1, 127));
	}
	mob->points.hitroll = static_cast<sbyte>(std::clamp(
		static_cast<int>(mob->points.hitroll * dice_delta), -50, 50));
}

[[nodiscard]] static bool procarea_is_runtime_mob(const char_data* mob) {
	return mob != nullptr && IS_NPC(mob) && mob->nr == -1 &&
		   !IS_SET(mob->specials.act, ACT_POLYSELF);
}

static void procarea_rescale_instance_mobs(const ProcAreaInstance& inst, float delta) {
	if(delta <= 1.0f) {
		return;
	}
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(char_data* mob = rp->people; mob != nullptr; mob = mob->next_in_room) {
			if(procarea_is_runtime_mob(mob)) {
				procarea_apply_power_delta(mob, delta);
			}
		}
	}
}

[[nodiscard]] static int procarea_count_pcs_in_instance(const ProcAreaInstance& inst) {
	int count = 0;
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(IS_PC(ch)) {
				++count;
			}
		}
	}
	return count;
}

static void procarea_echo_to_instance_pcs(const ProcAreaInstance& inst, const char* msg) {
	if(msg == nullptr) {
		return;
	}
	std::unordered_set<char_data*> sent;
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(IS_PC(ch) && ch->desc != nullptr && sent.insert(ch).second) {
				send_to_char(msg, ch);
			}
		}
	}
}

static void procarea_scale_mob(char_data* mob, float eq_index, int template_band,
							   int group_max_level, ProcMobKind kind,
							   ProcMobClassContext class_ctx, bool solo_mode,
							   float party_power_mult) {
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

	if(solo_mode) {
		ratio *= procarea_solo_kind_mult(kind);
	} else if(party_power_mult > 1.0f) {
		ratio *= party_power_mult;
	}

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

	procarea_apply_scaling_affects(mob, eq_index, kind, class_ctx, solo_mode);

	if(group_max_level > 0) {
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) =
			procarea_spawn_level(group_max_level, template_band, kind);
	}
}

static void procarea_apply_air_room_flight(char_data* mob, long room_vnum) {
	if(mob == nullptr || !IS_NPC(mob)) {
		return;
	}
	const struct room_data* rp = real_roomp(room_vnum);
	if(rp != nullptr && rp->sector_type == SECT_AIR) {
		SET_BIT(mob->specials.affected_by, AFF_FLYING);
	}
}

static char_data* procarea_spawn_scaled_mob(long room_vnum, int template_band, float eq_index,
											int group_max_level, ProcMobKind kind, int theme_id,
											bool follow_anchor_sentinel,
											int add_slot,
											ProcMobClassContext class_ctx,
											bool solo_mode, float party_power_mult) {
	const int archetype = procarea_pick_mob_archetype(theme_id, kind == ProcMobKind::Trap);
	char_data* mob = procarea_create_mob(archetype, eq_index, template_band, group_max_level,
										 kind, follow_anchor_sentinel, add_slot, class_ctx,
										 solo_mode, party_power_mult);
	if(mob == nullptr) {
		mudlog(LOG_ERROR, "procarea: scaled spawn failed room %ld", room_vnum);
		return nullptr;
	}
	char_to_room(mob, room_vnum);
	procarea_apply_air_room_flight(mob, room_vnum);
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

static const char* const kTreasureHoardDescriptions[] = {
	"Detriti e metallo contorto nascondono un forziere sigillato dalle rune effimere.",
	"Tra scorie e lamiere contorte si intravede un forziere chiuso da sigilli runici instabili.",
	"Un cumulo di detriti metallici cela un forziere sigillato da rune che vacillano nel vuoto.",
	"Frammenti d'acciaio piegato e macerie nascondono un forziere ancora sigillato da rune effimere.",
};

static struct obj_data* procarea_create_treasure_hoard() {
	struct obj_data* hoard = procarea_create_runtime_obj(kProcTreasureHoardObj);
	if(hoard == nullptr) {
		return nullptr;
	}
	const int desc_idx = number(0, static_cast<int>(sizeof(kTreasureHoardDescriptions) /
													 sizeof(kTreasureHoardDescriptions[0])) -
										1);
	hoard->name = strdup("cumulo forziere tesoro sigillo reliquie");
	hoard->short_description = strdup("un cumulo sigillato del tesoro");
	hoard->description = strdup(kTreasureHoardDescriptions[desc_idx]);
	hoard->obj_flags.type_flag = ITEM_CONTAINER;
	hoard->obj_flags.wear_flags = 0;
	hoard->obj_flags.value[0] = 10000;
	hoard->obj_flags.value[1] = CONT_CLOSEABLE | CONT_CLOSED | CONT_LOCKED;
	hoard->obj_flags.value[2] = -1;
	hoard->obj_flags.weight = 1;
	hoard->obj_flags.cost = 0;
	return hoard;
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

static void procarea_send_to_instance_members(const ProcAreaInstance& inst, const char* msg) {
	if(msg == nullptr) {
		return;
	}
	std::unordered_set<char_data*> sent;
	auto deliver = [&](char_data* member) {
		if(member == nullptr || !IS_PC(member) || member->desc == nullptr) {
			return;
		}
		if(sent.insert(member).second) {
			send_to_char(msg, member);
		}
	};
	if(!inst.owner_name.empty()) {
		deliver(get_char(inst.owner_name.c_str()));
	}
	for(const std::string& name : inst.member_names) {
		deliver(get_char(name.c_str()));
	}
}

[[nodiscard]] static bool procarea_valid_instance_room(long room_vnum) {
	const struct room_data* rp = real_roomp(room_vnum);
	return rp != nullptr && rp->number == room_vnum;
}

static void procarea_safe_send_to_room(long room_vnum, const char* msg) {
	if(msg == nullptr || !procarea_valid_instance_room(room_vnum)) {
		return;
	}
	send_to_room(msg, static_cast<int>(room_vnum));
}

static void procarea_ensure_treasure_obj_strings(struct obj_data* obj) {
	if(obj == nullptr) {
		return;
	}
	if(obj->name == nullptr || obj->name[0] == '\0') {
		if(obj->name != nullptr) {
			free(obj->name);
		}
		obj->name = strdup("reliquia effimera tesoro");
	}
	if(obj->short_description == nullptr || obj->short_description[0] == '\0') {
		if(obj->short_description != nullptr) {
			free(obj->short_description);
		}
		obj->short_description = strdup("una reliquia effimera");
	}
}

static void procarea_announce_treasure_item(char_data* ch, struct obj_data* item, long room_vnum,
											bool forged) {
	if(item == nullptr) {
		return;
	}
	procarea_ensure_treasure_obj_strings(item);
	if(ch != nullptr) {
		if(forged) {
			act("Dal cumulo emerge $p, forgiato dalle rune della dimensione!", FALSE, ch, item,
				nullptr, TO_CHAR);
		} else {
			act("Dal cumulo emerge $p!", FALSE, ch, item, nullptr, TO_CHAR);
		}
		act("Dal cumulo emerge $p!", TRUE, ch, item, nullptr, TO_ROOM);
		return;
	}

	const char* const label =
		item->short_description != nullptr ? item->short_description : "una reliquia effimera";
	std::ostringstream room_msg;
	room_msg << "Dal cumulo emerge " << label;
	if(forged) {
		room_msg << ", forgiato dalle rune della dimensione";
	}
	room_msg << "!\n\r";
	procarea_safe_send_to_room(room_vnum, room_msg.str().c_str());
}

[[nodiscard]] static std::string procarea_capitalize_label(std::string_view label) {
	std::string s(label);
	if(s.size() > 7 && s[1] == '$') {
		s[7] = UPPER(s[7]);
	} else if(!s.empty()) {
		s[0] = UPPER(s[0]);
	}
	return s;
}

[[nodiscard]] static const char* procarea_mob_display_name(const char_data* mob) {
	if(mob == nullptr) {
		return "il custode della dimensione";
	}
	if(mob->player.short_descr != nullptr && mob->player.short_descr[0] != '\0') {
		return mob->player.short_descr;
	}
	if(mob->player.long_descr != nullptr && mob->player.long_descr[0] != '\0') {
		return mob->player.long_descr;
	}
	if(GET_NAME(mob) != nullptr && GET_NAME(mob)[0] != '\0') {
		return GET_NAME(mob);
	}
	return "il custode della dimensione";
}

[[nodiscard]] static bool procarea_room_in_instance(const ProcAreaInstance& inst, long vnum) {
	return std::find(inst.room_vnums.begin(), inst.room_vnums.end(), vnum) !=
		   inst.room_vnums.end();
}

static char_data* procarea_pick_treasure_opener(const ProcAreaInstance& inst, long prefer_room) {
	if(prefer_room > 0 && procarea_room_in_instance(inst, prefer_room)) {
		if(struct room_data* rp = real_roomp(prefer_room); rp != nullptr) {
			for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
				if(IS_PC(ch)) {
					return ch;
				}
			}
		}
	}
	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			if(IS_PC(ch)) {
				return ch;
			}
		}
	}
	for(const std::string& name : inst.member_names) {
		if(char_data* ch = get_char(name.c_str());
		   ch != nullptr && IS_PC(ch) && procarea_room_in_instance(inst, ch->in_room)) {
			return ch;
		}
	}
	return nullptr;
}

static void procarea_release_all_treasures(ProcAreaInstance& inst, char_data* opener);

void break_treasure_seals(ProcAreaInstance& inst, const char_data* boss) {
	if(inst.boss_key_dropped || inst.treasure_vnums.empty()) {
		return;
	}
	inst.boss_key_dropped = true;
	const long boss_room = boss != nullptr ? boss->in_room : 0;
	char_data* opener = procarea_pick_treasure_opener(inst, boss_room);
	procarea_release_all_treasures(inst, opener);

	const bool boss_female = boss != nullptr && GET_SEX(boss) == SEX_FEMALE;
	std::ostringstream msg;
	msg << "\n\r$c0014" << procarea_capitalize_label(procarea_mob_display_name(boss))
		<< " cade: le rune dei cumuli si incrinano e cedono con "
		<< (boss_female ? "lei" : "lui") << ".$c0007\n\r"
		<< "$c0010I sigilli cedono su ogni cumulo:$c0007 oro e reliquie effimere giacciono nelle stanze del tesoro.\n\r"
		<< "Recati a raccogliere il bottino.\n\r";
	const std::string message = msg.str();
	procarea_echo_to_instance_pcs(inst, message.c_str());
	procarea_send_to_instance_members(inst, message.c_str());
}

[[nodiscard]] static int procarea_treasure_gold_amount(const ProcAreaInstance& inst) {
	const float factor = procarea_eq_factor(inst.group_eq_index);
	const int band = std::clamp(inst.template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	const float eq_part = inst.group_eq_index * (0.55f + factor * 0.45f);
	const int band_part = 100 + band * 250;
	const float jitter = 0.85f + static_cast<float>(number(0, 30)) / 100.0f;
	return std::max(50, static_cast<int>((eq_part + band_part) * jitter * 50.0f));
}

[[nodiscard]] static long procarea_reward_shield_vnum(int template_band) {
	const int band = std::clamp(template_band, 0, PROCAREA_REWARD_SHIELD_COUNT - 1);
	return PROCAREA_REWARD_SHIELD_VNUM_BASE + band;
}

[[nodiscard]] static int procarea_find_free_affect_slot(struct obj_data* obj) {
	if(obj == nullptr) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == APPLY_NONE) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] static unsigned procarea_roll_elemental_resist() {
	static constexpr unsigned kElemental[] = {
		IMM_FIRE,
		IMM_COLD,
		IMM_ELEC,
		IMM_ENERGY,
		IMM_ACID,
	};
	return kElemental[number(0, static_cast<int>(std::size(kElemental)) - 1)];
}

/** 85% pierce, 14.5% slash, 0.5% blunt (su 10000). */
[[nodiscard]] static unsigned procarea_roll_physical_resist() {
	const int roll = number(0, 9999);
	if(roll < 8500) {
		return IMM_PIERCE;
	}
	if(roll < 9950) {
		return IMM_SLASH;
	}
	return IMM_BLUNT;
}

/** 5% resistenza fisica solo se consentita (body), altrimenti elementale. */
[[nodiscard]] static unsigned procarea_roll_gear_resist(bool allow_physical) {
	if(allow_physical && number(0, 99) < 5) {
		return procarea_roll_physical_resist();
	}
	return procarea_roll_elemental_resist();
}

[[nodiscard]] static bool procarea_shield_bonus_used(const struct obj_data* obj, int location) {
	if(obj == nullptr) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == location) {
			return true;
		}
	}
	return false;
}

/** Caster (spellpower) e melee (damroll, hit&dam) sono mutuamente esclusivi. */
[[nodiscard]] static bool procarea_shield_bonus_allowed(const struct obj_data* obj, int location) {
	if(obj == nullptr) {
		return false;
	}
	if(procarea_shield_bonus_used(obj, location)) {
		return false;
	}
	const bool has_spellpower = procarea_shield_bonus_used(obj, APPLY_SPELLPOWER);
	const bool has_melee =
		procarea_shield_bonus_used(obj, APPLY_DAMROLL) ||
		procarea_shield_bonus_used(obj, APPLY_HITNDAM);
	if(has_spellpower &&
	   (location == APPLY_DAMROLL || location == APPLY_HITNDAM)) {
		return false;
	}
	if(has_melee && location == APPLY_SPELLPOWER) {
		return false;
	}
	if(has_melee && !has_spellpower && location == APPLY_SPELLFAIL) {
		return false;
	}
	return true;
}

/** Riferimento band 4 (eq ~8500+): cap massimi bonus scudo premio. */
[[nodiscard]] static float procarea_shield_band_factor(int band) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	return std::min(1.0f, 0.20f + static_cast<float>(b) * 0.20f);
}

[[nodiscard]] static int procarea_shield_bonus_count(int group_max_level) {
	const int level = std::clamp(group_max_level, PROCAREA_MIN_LEVEL, PROCAREA_PC_MAX_LEVEL);
	if(level <= 10) {
		return 1;
	}
	if(level <= 20) {
		return 2;
	}
	if(level <= 35) {
		return 3;
	}
	if(level <= 50) {
		return 4;
	}
	return PROCAREA_REWARD_BONUS_MAX;
}

[[nodiscard]] static int procarea_shield_scaled_roll(int ref_min, int ref_max, float scale) {
	const int lo = std::max(1, static_cast<int>(std::lround(static_cast<float>(ref_min) * scale)));
	const int hi = std::max(lo, static_cast<int>(std::lround(static_cast<float>(ref_max) * scale)));
	return number(lo, hi);
}

[[nodiscard]] static int procarea_roll_shield_bonus_modifier(int location, int band) {
	const float scale = procarea_shield_band_factor(band);

	switch(location) {
	case APPLY_HIT:
		return procarea_shield_scaled_roll(10, 30, scale);
	case APPLY_MANA:
		return procarea_shield_scaled_roll(40, 100, scale);
	case APPLY_HIT_REGEN:
	case APPLY_MANA_REGEN:
		return procarea_shield_scaled_roll(5, 30, scale);
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
	case APPLY_SPELLPOWER:
		return procarea_shield_scaled_roll(1, 4, scale);
	case APPLY_HITNDAM:
		return procarea_shield_scaled_roll(1, 3, scale);
	case APPLY_SAVE_ALL: {
		const int mag_min = std::max(1, static_cast<int>(std::lround(2.0f * scale)));
		const int mag_max = std::max(mag_min, static_cast<int>(std::lround(5.0f * scale)));
		return -number(mag_min, mag_max);
	}
	case APPLY_SPELLFAIL: {
		// Banda 0: −5…−10 → banda 5: −20…−30 (cap).
		static constexpr int kSpellfailAbsMinByBand[PROCAREA_TEMPLATE_BANDS] = {
			5, 8, 11, 14, 17, 20,
		};
		static constexpr int kSpellfailAbsMaxByBand[PROCAREA_TEMPLATE_BANDS] = {
			10, 14, 18, 22, 26, 30,
		};
		const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
		return -number(kSpellfailAbsMinByBand[b], kSpellfailAbsMaxByBand[b]);
	}
	default:
		return 1;
	}
}

enum class ProcShieldRollModel {
	Tank,
	Caster,
	Hybrid,
	Generic,
};

struct ProcShieldBonusWeight {
	int location;
	int weight;
};

/** Prevalenza tank: HP, regen, save, melee. */
static constexpr ProcShieldBonusWeight kShieldWeightsTank[] = {
	{ APPLY_HIT, 28 },
	{ APPLY_HIT_REGEN, 22 },
	{ APPLY_SAVE_ALL, 18 },
	{ APPLY_DAMROLL, 14 },
	{ APPLY_HITROLL, 10 },
	{ APPLY_HITNDAM, 6 },
	{ APPLY_MANA, 1 },
	{ APPLY_MANA_REGEN, 1 },
};

/** Prevalenza caster: mana, spellpower, regen, save, spellfail. */
static constexpr ProcShieldBonusWeight kShieldWeightsCaster[] = {
	{ APPLY_MANA, 26 },
	{ APPLY_SPELLPOWER, 22 },
	{ APPLY_MANA_REGEN, 18 },
	{ APPLY_SPELLFAIL, 14 },
	{ APPLY_SAVE_ALL, 12 },
	{ APPLY_HIT, 6 },
	{ APPLY_HIT_REGEN, 2 },
	{ APPLY_DAMROLL, 1 },
	{ APPLY_HITROLL, 1 },
};

/** Multiclasse magic + fighter: mix equilibrato su entrambi i fronti. */
static constexpr ProcShieldBonusWeight kShieldWeightsHybrid[] = {
	{ APPLY_HIT, 16 },
	{ APPLY_MANA, 14 },
	{ APPLY_SAVE_ALL, 14 },
	{ APPLY_MANA_REGEN, 12 },
	{ APPLY_HIT_REGEN, 12 },
	{ APPLY_SPELLPOWER, 10 },
	{ APPLY_SPELLFAIL, 8 },
	{ APPLY_DAMROLL, 8 },
	{ APPLY_HITROLL, 6 },
	{ APPLY_HITNDAM, 6 },
};

/** Fallback: peso uniforme. */
static constexpr ProcShieldBonusWeight kShieldWeightsGeneric[] = {
	{ APPLY_HIT, 10 },
	{ APPLY_MANA, 10 },
	{ APPLY_MANA_REGEN, 10 },
	{ APPLY_HIT_REGEN, 10 },
	{ APPLY_DAMROLL, 10 },
	{ APPLY_HITROLL, 10 },
	{ APPLY_SPELLPOWER, 10 },
	{ APPLY_HITNDAM, 10 },
	{ APPLY_SAVE_ALL, 10 },
};

/** Peso del modello principale in run solitaria (resto ripartito sugli altri). */
static constexpr int kProcShieldSoloPrimaryWeight = 75;
static constexpr int kProcShieldSoloSecondaryWeight = 8;
static constexpr int kProcShieldSoloGenericWeight = 5;

[[nodiscard]] static int procarea_weighted_pick_index(const int* weights, int count) {
	int total = 0;
	for(int i = 0; i < count; ++i) {
		total += weights[i];
	}
	if(total <= 0) {
		return -1;
	}
	int roll = number(1, total);
	for(int i = 0; i < count; ++i) {
		roll -= weights[i];
		if(roll <= 0) {
			return i;
		}
	}
	return count - 1;
}

[[nodiscard]] static int procarea_shield_model_index(ProcShieldRollModel model) {
	switch(model) {
	case ProcShieldRollModel::Tank:
		return 0;
	case ProcShieldRollModel::Caster:
		return 1;
	case ProcShieldRollModel::Hybrid:
		return 2;
	case ProcShieldRollModel::Generic:
		break;
	}
	return 3;
}

[[nodiscard]] static ProcShieldRollModel procarea_shield_model_from_index(int index) {
	switch(index) {
	case 0:
		return ProcShieldRollModel::Tank;
	case 1:
		return ProcShieldRollModel::Caster;
	case 2:
		return ProcShieldRollModel::Hybrid;
	default:
		return ProcShieldRollModel::Generic;
	}
}

[[nodiscard]] static ProcShieldRollModel procarea_shield_archetype_for_char(char_data* ch) {
	if(ch == nullptr) {
		return ProcShieldRollModel::Generic;
	}
	if(IS_MULTI(ch)) {
		return ProcShieldRollModel::Hybrid;
	}
	if(IS_CASTER_N(ch)) {
		return ProcShieldRollModel::Caster;
	}
	if(IS_MELEE(ch)) {
		return ProcShieldRollModel::Tank;
	}
	if(IS_CASTER(ch) && IS_FIGHTER(ch)) {
		return ProcShieldRollModel::Hybrid;
	}
	if(IS_CASTER(ch)) {
		return ProcShieldRollModel::Caster;
	}
	if(IS_FIGHTER(ch)) {
		return ProcShieldRollModel::Tank;
	}
	return ProcShieldRollModel::Generic;
}

template<typename Fn>
static void procarea_for_each_resolved_member(const ProcAreaInstance& inst, Fn&& fn) {
	std::unordered_set<char_data*> seen;
	auto visit = [&](char_data* member) {
		if(member == nullptr || !IS_PC(member) || !seen.insert(member).second) {
			return;
		}
		fn(member);
	};
	if(!inst.owner_name.empty()) {
		visit(get_char(inst.owner_name.c_str()));
	}
	for(const std::string& name : inst.member_names) {
		visit(get_char(name.c_str()));
	}
}

[[nodiscard]] static bool procarea_instance_has_ranger(const ProcAreaInstance& inst) {
	bool found = false;
	procarea_for_each_resolved_member(inst, [&](char_data* member) {
		if(HasClass(member, CLASS_RANGER)) {
			found = true;
		}
	});
	return found;
}

[[nodiscard]] static ProcShieldRollModel
procarea_pick_shield_roll_model_solo(char_data* solo_ch) {
	const ProcShieldRollModel primary = procarea_shield_archetype_for_char(solo_ch);
	int weights[4] = {
		kProcShieldSoloSecondaryWeight,
		kProcShieldSoloSecondaryWeight,
		kProcShieldSoloSecondaryWeight,
		kProcShieldSoloGenericWeight,
	};
	weights[procarea_shield_model_index(primary)] = kProcShieldSoloPrimaryWeight;

	const int pick = procarea_weighted_pick_index(weights, 4);
	if(pick < 0) {
		return primary;
	}
	return procarea_shield_model_from_index(pick);
}

[[nodiscard]] static ProcShieldRollModel
procarea_pick_shield_roll_model_group(const ProcAreaInstance& inst, char_data* fallback_ch) {
	int weights[4] = { 0, 0, 0, 0 };

	procarea_for_each_resolved_member(inst, [&](char_data* member) {
		const ProcShieldRollModel archetype = procarea_shield_archetype_for_char(member);
		++weights[procarea_shield_model_index(archetype)];
	});

	if(weights[0] > 0 && weights[1] > 0) {
		weights[2] += weights[0] + weights[1];
	}

	const int total = weights[0] + weights[1] + weights[2] + weights[3];
	if(total <= 0) {
		return procarea_shield_archetype_for_char(fallback_ch);
	}

	const int pick = procarea_weighted_pick_index(weights, 4);
	if(pick < 0) {
		return procarea_shield_archetype_for_char(fallback_ch);
	}
	return procarea_shield_model_from_index(pick);
}

[[nodiscard]] static ProcShieldRollModel procarea_pick_shield_roll_model(const ProcAreaInstance& inst,
																		 char_data* fallback_ch) {
	if(inst.solo_mode) {
		char_data* solo_ch = fallback_ch;
		if(solo_ch == nullptr && !inst.owner_name.empty()) {
			solo_ch = get_char(inst.owner_name.c_str());
		}
		if(solo_ch == nullptr && !inst.member_names.empty()) {
			solo_ch = get_char(inst.member_names.front().c_str());
		}
		return procarea_pick_shield_roll_model_solo(solo_ch);
	}
	return procarea_pick_shield_roll_model_group(inst, fallback_ch);
}

[[nodiscard]] static std::pair<const ProcShieldBonusWeight*, std::size_t>
procarea_shield_weight_table(ProcShieldRollModel model) {
	switch(model) {
	case ProcShieldRollModel::Tank:
		return { kShieldWeightsTank, std::size(kShieldWeightsTank) };
	case ProcShieldRollModel::Caster:
		return { kShieldWeightsCaster, std::size(kShieldWeightsCaster) };
	case ProcShieldRollModel::Hybrid:
		return { kShieldWeightsHybrid, std::size(kShieldWeightsHybrid) };
	case ProcShieldRollModel::Generic:
		break;
	}
	return { kShieldWeightsGeneric, std::size(kShieldWeightsGeneric) };
}

static bool procarea_apply_shield_bonus_roll(struct obj_data* obj, int band,
											 ProcShieldRollModel model) {
	if(obj == nullptr) {
		return false;
	}

	const auto [table, table_size] = procarea_shield_weight_table(model);

	static constexpr int kMaxShieldBonusCandidates = 12;
	int candidates[kMaxShieldBonusCandidates];
	int candidate_weights[kMaxShieldBonusCandidates];
	int candidate_count = 0;

	for(std::size_t i = 0; i < table_size; ++i) {
		const int location = table[i].location;
		const int weight = table[i].weight;
		if(weight <= 0 || !procarea_shield_bonus_allowed(obj, location)) {
			continue;
		}
		if(candidate_count >= kMaxShieldBonusCandidates) {
			mudlog(LOG_SYSERR,
				   "procarea: shield bonus candidate overflow (model %d, table %zu)",
				   static_cast<int>(model), table_size);
			break;
		}
		candidates[candidate_count] = location;
		candidate_weights[candidate_count] = weight;
		++candidate_count;
	}
	if(candidate_count <= 0) {
		return false;
	}

	const int pick_idx =
		procarea_weighted_pick_index(candidate_weights, candidate_count);
	if(pick_idx < 0) {
		return false;
	}

	const int pick = candidates[pick_idx];
	const int slot = procarea_find_free_affect_slot(obj);
	if(slot < 0) {
		return false;
	}

	obj->affected[slot].location = pick;
	obj->affected[slot].modifier = procarea_roll_shield_bonus_modifier(pick, band);
	return true;
}

static void procarea_roll_reward_bonuses(const ProcAreaInstance& inst, struct obj_data* obj,
										 char_data* opener, bool allow_physical_immune,
										 bool try_ac_upgrade) {
	if(obj == nullptr) {
		return;
	}

	SET_BIT(obj->obj_flags.extra_flags, ITEM_RESISTANT);

	const int band = std::clamp(inst.template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);

	if(try_ac_upgrade && GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
		const int ac_upgrade_chance = 30 + band * 10;
		if(number(0, 99) < ac_upgrade_chance) {
			const int ac_jitter_max = band >= 4 ? 2 : band >= 3 ? 1 : 0;
			const int ac_jitter = number(0, ac_jitter_max);
			obj->obj_flags.value[0] = std::max(3, obj->obj_flags.value[0] + ac_jitter);
			obj->obj_flags.value[1] = obj->obj_flags.value[0];
		}
	}

	const int bonus_count = procarea_shield_bonus_count(inst.group_max_level);
	const ProcShieldRollModel model = procarea_pick_shield_roll_model(inst, opener);
	for(int i = 0; i < bonus_count; ++i) {
		if(!procarea_apply_shield_bonus_roll(obj, band, model)) {
			break;
		}
	}

	if(bonus_count < PROCAREA_REWARD_BONUS_MAX &&
	   !procarea_shield_bonus_used(obj, APPLY_IMMUNE)) {
		if(const int slot = procarea_find_free_affect_slot(obj); slot >= 0) {
			obj->affected[slot].location = APPLY_IMMUNE;
			obj->affected[slot].modifier =
				static_cast<int>(procarea_roll_gear_resist(allow_physical_immune));
		}
	}

	const int cost_jitter = number(90, 115);
	const long long scaled_cost =
		(static_cast<long long>(obj->obj_flags.cost) * static_cast<long long>(cost_jitter)) / 100LL;
	const long long clamped_cost = std::clamp(scaled_cost, 1LL, static_cast<long long>(INT_MAX));
	obj->obj_flags.cost = static_cast<int>(clamped_cost);
}

static void procarea_roll_reward_shield(const ProcAreaInstance& inst, struct obj_data* obj,
										char_data* opener) {
	procarea_roll_reward_bonuses(inst, obj, opener, false, true);
}

static void procarea_roll_reward_gear_item(const ProcAreaInstance& inst, struct obj_data* obj,
										   char_data* opener, ProcRewardGearSlot slot) {
	if(obj == nullptr) {
		return;
	}
	if(slot == ProcRewardGearSlot::Wield) {
		SET_BIT(obj->obj_flags.extra_flags, ITEM_RESISTANT);
		roll_reward_weapon_impl(obj, inst);
		return;
	}
	const bool allow_physical = slot == ProcRewardGearSlot::Body;
	const bool try_ac = GET_ITEM_TYPE(obj) == ITEM_ARMOR;
	procarea_roll_reward_bonuses(inst, obj, opener, allow_physical, try_ac);
}

[[nodiscard]] static int procarea_treasure_gear_drop_pct(int hoard_count) {
	if(hoard_count <= 1) {
		return 100;
	}
	return std::max(0, 100 - PROCAREA_TREASURE_GEAR_DROP_DECAY_PCT * (hoard_count - 1));
}

[[nodiscard]] static int procarea_pick_gear_sub_variant(ProcRewardGearSlot slot) {
	switch(slot) {
	case ProcRewardGearSlot::Finger:
	case ProcRewardGearSlot::Neck:
	case ProcRewardGearSlot::Wrist:
	case ProcRewardGearSlot::Ear:
		return number(0, 1);
	default:
		return 0;
	}
}

/** true = scudo premio; false = slot gear in @p gear_slot. */
static void procarea_pick_random_treasure_loot(bool& is_shield, ProcRewardGearSlot& gear_slot) {
	const int pick = number(0, static_cast<int>(ProcRewardGearSlot::Count));
	if(pick >= static_cast<int>(ProcRewardGearSlot::Count)) {
		is_shield = true;
		gear_slot = ProcRewardGearSlot::Light;
		return;
	}
	is_shield = false;
	gear_slot = static_cast<ProcRewardGearSlot>(pick);
}

static bool procarea_try_grant_treasure_item(char_data* ch, ProcAreaInstance& inst, long room_vnum) {
	const int hoard_count = static_cast<int>(inst.treasure_vnums.size());
	const int drop_pct = procarea_treasure_gear_drop_pct(hoard_count);
	if(number(0, 99) >= drop_pct) {
		return false;
	}

	bool is_shield = false;
	ProcRewardGearSlot slot = ProcRewardGearSlot::Light;
	procarea_pick_random_treasure_loot(is_shield, slot);

	long vnum = -1;
	if(is_shield) {
		vnum = procarea_reward_shield_vnum(inst.template_band);
	} else {
		const int sub_variant =
			slot == ProcRewardGearSlot::Wield ? number(0, 2) : procarea_pick_gear_sub_variant(slot);
		vnum = reward_gear_vnum(slot, inst.template_band, sub_variant);
	}
	if(vnum < 0) {
		return false;
	}

	const int rnum = real_object(static_cast<int>(vnum));
	if(rnum < 0) {
		mudlog(LOG_SYSERR, "procarea: treasure loot vnum %ld not in object index", vnum);
		return false;
	}

	struct obj_data* item = read_object(rnum, REAL);
	if(item == nullptr) {
		mudlog(LOG_ERROR, "procarea: read_object(%ld) failed for treasure loot", vnum);
		return false;
	}

	if(is_shield) {
		procarea_roll_reward_shield(inst, item, ch);
	} else {
		procarea_roll_reward_gear_item(inst, item, ch, slot);
	}
	if(!procarea_valid_instance_room(room_vnum)) {
		mudlog(LOG_ERROR, "procarea: treasure item room %ld invalid after roll", room_vnum);
		extract_obj(item);
		return false;
	}
	obj_to_room(item, room_vnum);
	procarea_announce_treasure_item(ch, item, room_vnum, is_shield);
	return true;
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

[[nodiscard]] static int procarea_count_unclaimed_treasures(const ProcAreaInstance& inst) {
	int count = 0;
	for(long vnum : inst.treasure_vnums) {
		if(inst.treasure_claimed.count(vnum) == 0) {
			++count;
		}
	}
	return count;
}

static void procarea_grant_treasure_loot(char_data* ch, ProcAreaInstance& inst, long room_vnum) {
	if(inst.treasure_claimed.count(room_vnum) != 0) {
		return;
	}
	if(!procarea_valid_instance_room(room_vnum)) {
		mudlog(LOG_ERROR, "procarea: grant treasure loot invalid room %ld", room_vnum);
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

	const bool item_granted = procarea_try_grant_treasure_item(ch, inst, room_vnum);

	struct obj_data* hoard = procarea_find_treasure_hoard(room_vnum);
	if(hoard != nullptr) {
		REMOVE_BIT(hoard->obj_flags.value[1], CONT_LOCKED);
		REMOVE_BIT(hoard->obj_flags.value[1], CONT_CLOSED);
	}

	if(ch != nullptr) {
		act("$n spezza il sigillo runico del cumulo!", TRUE, ch, nullptr, nullptr, TO_ROOM);
		if(item_granted) {
			act("Apri il cumulo sigillato: oro e un trofeo effimero fuoriescono dalle rune!", FALSE,
				ch, nullptr, nullptr, TO_CHAR);
		} else {
			act("Apri il cumulo sigillato: oro e reliquie effimere fuoriescono dalle rune!", FALSE,
				ch, nullptr, nullptr, TO_CHAR);
		}
	}
	procarea_safe_send_to_room(
		room_vnum,
		"\n\r$c0013Il sigillo cede: il tesoro della dimensione giace ora a terra.$c0007\n\r");

	const int still = procarea_count_unclaimed_treasures(inst);
	if(still > 0 && ch != nullptr) {
		std::ostringstream os;
		os << "Altri sigilli restano attivi: "
		   << still << " cumul" << (still == 1 ? "o" : "i") << " sigillat"
		   << (still == 1 ? "o" : "i") << " resta" << (still == 1 ? "" : "no")
		   << " nella dimensione effimera.\n\r";
		const std::string notice = os.str();
		send_to_char(notice.c_str(), ch);
	}
}

static void procarea_release_all_treasures(ProcAreaInstance& inst, char_data* opener) {
	for(long vnum : inst.treasure_vnums) {
		if(inst.treasure_claimed.count(vnum) != 0) {
			continue;
		}
		if(procarea_find_treasure_hoard(vnum) == nullptr) {
			continue;
		}
		procarea_grant_treasure_loot(opener, inst, vnum);
	}
}

bool try_open_treasure(struct char_data* ch, struct room_data* room,
									   std::string_view target) {
	if(ch == nullptr || room == nullptr || !IS_PC(ch)) {
		return false;
	}
	if(!cmd_is(target, { "cumulo", "forziere", "tesoro", "sigillo", "hoard" })) {
		return false;
	}

	ProcAreaInstance* inst = find_instance_by_vnum(room->number);
	if(inst == nullptr) {
		return false;
	}
	if(procarea_find_treasure_hoard(room->number) == nullptr) {
		send_to_char("Non c'e' nulla da aprire qui.\n\r", ch);
		return true;
	}
	if(inst->treasure_claimed.count(room->number) != 0) {
		send_to_char("Il tesoro di questo cumulo giace gia' a terra.\n\r", ch);
		return true;
	}
	if(!inst->boss_key_dropped) {
		send_to_char(
			"Il sigillo resiste: devi prima sconfiggere il custode della dimensione.\n\r", ch);
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

void clear_world_links(const ProcAreaInstance& inst) {
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

[[nodiscard]] static int procarea_layout_count_type(const std::vector<ProcLayoutRoom>& layout,
													ProcArchetype type) {
	int count = 0;
	for(const ProcLayoutRoom& room : layout) {
		if(room.type == type) {
			++count;
		}
	}
	return count;
}

/** Ogni istanza deve avere almeno un cumulo: trap → tesoro, altrimenti un corridoio dello spine. */
static void procarea_ensure_min_treasure(std::vector<ProcLayoutRoom>& layout) {
	if(procarea_layout_count_type(layout, ProcArchetype::Treasure) > 0) {
		return;
	}
	for(ProcLayoutRoom& room : layout) {
		if(room.type == ProcArchetype::Trap) {
			room.type = ProcArchetype::Treasure;
			return;
		}
	}
	for(size_t i = 1; i + 1 < layout.size(); ++i) {
		if(layout[i].type == ProcArchetype::Corridor) {
			layout[i].type = ProcArchetype::Treasure;
			return;
		}
	}
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

	procarea_ensure_min_treasure(layout);

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
	rp->zone = assign_zone(vnum);
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
									  int group_max_level, ProcMobKind kind,
									  bool follow_anchor_sentinel, int add_slot,
									  ProcMobClassContext class_ctx, bool solo_mode,
									  float party_power_mult) {
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
	procarea_scale_mob(mob, eq_index, template_band, group_max_level, kind, class_ctx, solo_mode,
					   party_power_mult);

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
									  diff.group_max_level, ProcMobKind::Normal, theme_id, false,
									  -1, ProcMobClassContext::Corridor, diff.solo_mode,
									  diff.party_power_mult);
		}
		if(depth >= std::max(2, max_depth / 3) &&
		   number(0, 99) < std::clamp(25 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  diff.group_max_level, ProcMobKind::Normal, theme_id, false,
									  -1, ProcMobClassContext::Corridor, diff.solo_mode,
									  diff.party_power_mult);
		}
		break;
	}
	case ProcArchetype::Treasure: {
		const int chance = std::clamp(diff.treasure_spawn_pct + depth_bonus, 0, 98);
		if(number(0, 99) < chance) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  diff.group_max_level, ProcMobKind::Normal, theme_id, false,
									  -1, ProcMobClassContext::Treasure, diff.solo_mode,
									  diff.party_power_mult);
		}
		if(number(0, 99) < std::clamp(40 + depth_bonus, 0, 99)) {
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  diff.group_max_level, ProcMobKind::Normal, theme_id, false,
									  -1, ProcMobClassContext::Treasure, diff.solo_mode,
									  diff.party_power_mult);
		}
		procarea_plant_treasure_hoard(room_vnum);
		break;
	}
	case ProcArchetype::Trap: {
		char_data* trap =
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  diff.group_max_level, ProcMobKind::Trap, theme_id, false, -1,
									  ProcMobClassContext::Corridor, diff.solo_mode,
									  diff.party_power_mult);
		int add_slot = 0;
		char_data* add =
			procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
									  diff.group_max_level, ProcMobKind::Normal, theme_id, true,
									  add_slot++, ProcMobClassContext::Trap, diff.solo_mode,
									  diff.party_power_mult);
		if(trap != nullptr && add != nullptr) {
			procarea_link_anchor_add(add, trap);
		}
		if(number(0, 99) < std::clamp(50 + depth_bonus, 0, 99)) {
			add = procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
											diff.group_max_level, ProcMobKind::Normal, theme_id,
											true, add_slot++, ProcMobClassContext::Trap,
											diff.solo_mode, diff.party_power_mult);
			if(trap != nullptr && add != nullptr) {
				procarea_link_anchor_add(add, trap);
			}
		}
		break;
	}
	case ProcArchetype::Boss: {
		const int boss_idx = procarea_pick_boss_archetype(theme_id);
		char_data* boss = procarea_create_mob(boss_idx, diff.eq_index, diff.template_band,
											  diff.group_max_level, ProcMobKind::Boss, false, -1,
											  ProcMobClassContext::Corridor, diff.solo_mode,
											  diff.party_power_mult);
		if(boss == nullptr) {
			break;
		}
		char_to_room(boss, room_vnum);
		procarea_apply_air_room_flight(boss, room_vnum);
		for(int i = 0; i < diff.boss_adds; ++i) {
			char_data* add =
				procarea_spawn_scaled_mob(room_vnum, diff.template_band, diff.eq_index,
										  diff.group_max_level, ProcMobKind::Normal, theme_id,
										  true, i, ProcMobClassContext::Corridor, diff.solo_mode,
										  diff.party_power_mult);
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

int count_mobs(const ProcAreaInstance& inst) {
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

void open_exit_portal(ProcAreaInstance& inst) {
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
int create_instance(float group_eq_index, int group_max_level, long return_room,
					long& entrance_vnum, const char* owner_name, bool solo_mode, int party_size) {
	ProcAreaDifficulty diff = procarea_difficulty_from_eq(group_eq_index);
	if(solo_mode) {
		diff = procarea_difficulty_apply_solo(diff);
	}
	diff.group_max_level = group_max_level;
	diff.solo_mode = solo_mode;
	if(solo_mode) {
		diff.party_power_mult = 1.0f;
	} else {
		const int clamped_party = std::clamp(party_size, 1, PROCAREA_PARTY_SIZE_CAP);
		diff.party_power_mult = procarea_party_power_mult(clamped_party);
	}
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
	inst.group_max_level = group_max_level;
	inst.theme_id = procarea_pick_theme_id();
	inst.base_vnum = procarea_instance_base_vnum(instance_id);
	inst.return_room = return_room;
	inst.exit_portal_open = false;
	inst.created_at = time(nullptr);
	inst.last_activity = inst.created_at;
	if(owner_name != nullptr && *owner_name != '\0') {
		inst.owner_name = owner_name;
	}
	inst.solo_mode = solo_mode;
	if(solo_mode) {
		inst.party_size_at_scale = 1;
		inst.party_power_mult = 1.0f;
	} else {
		inst.party_size_at_scale = std::clamp(party_size, 1, PROCAREA_PARTY_SIZE_CAP);
		inst.party_power_mult = diff.party_power_mult;
	}
	inst.room_vnums.reserve(layout.size());

	const int max_depth = procarea_room_depth(layout, static_cast<int>(layout.size()) - 1);
	const ProcThemeSet& theme = theme_set(inst.theme_id);

	for(size_t i = 0; i < layout.size(); ++i) {
		const long vnum = procarea_local_vnum(instance_id, static_cast<int>(i));
		const ProcRoomTemplate& tmpl = pick_template(theme, layout[i].type);
		if(procarea_create_room(vnum, tmpl) == nullptr) {
			mudlog(LOG_SYSERR, "procarea: create_room failed vnum=%ld (WORLD_SIZE=%d)",
				   vnum, WORLD_SIZE);
			destroy_rooms(inst.room_vnums);
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
	const char* const solo_suffix = solo_mode ? ", solo" : "";
	if(solo_mode) {
		mudlog(LOG_CHECK,
			   "procarea: created instance %d theme '%s' eq %.0f (factor %.2f%s) with %zu rooms (entrance %ld, boss %ld)",
			   instance_id, theme.label, group_eq_index, diff.factor, solo_suffix, layout.size(),
			   inst.entrance_vnum, inst.boss_vnum);
	} else {
		mudlog(LOG_CHECK,
			   "procarea: created instance %d theme '%s' eq %.0f (factor %.2f, party x%.2f) with %zu rooms (entrance %ld, boss %ld)",
			   instance_id, theme.label, group_eq_index, diff.factor, diff.party_power_mult,
			   layout.size(), inst.entrance_vnum, inst.boss_vnum);
	}
	return instance_id;
}

void sync_party_power_scale(ProcAreaInstance& inst) {
	if(inst.solo_mode) {
		return;
	}

	const int pc_count = procarea_count_pcs_in_instance(inst);
	const int new_size = std::clamp(pc_count, 1, PROCAREA_PARTY_SIZE_CAP);
	if(new_size <= inst.party_size_at_scale) {
		return;
	}

	const float old_mult = inst.party_power_mult;
	const float new_mult = procarea_party_power_mult(new_size);
	if(new_mult <= old_mult) {
		inst.party_size_at_scale = new_size;
		return;
	}

	const float delta = new_mult / old_mult;
	inst.party_power_mult = new_mult;
	inst.party_size_at_scale = new_size;
	procarea_rescale_instance_mobs(inst, delta);
	procarea_echo_to_instance_pcs(
		inst,
		"\n\r$c0010La bruma effimera trema:\n\r"
		"nuove presenze piegano l'aldilà, e i suoi custodi si risvegliano più feroci.$c0007\n\r");
	mudlog(LOG_CHECK,
		   "procarea: instance %d party scale %d PG (mult %.2f -> %.2f, delta %.3f)",
		   inst.id, new_size, old_mult, new_mult, delta);
}
struct ProcRewardShieldTpl {
	long vnum;
	const char* name;
	const char* short_descr;
	const char* description;
	int ac;
	unsigned int extra_flags;
	unsigned int extra_flags2;
	int cost;
	int apply_ac;
};

static constexpr ProcRewardShieldTpl kProcRewardShields[PROCAREA_REWARD_SHIELD_COUNT] = {
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 0,
		"disco velo bruma opalescente",
		"un disco opalescente del velo brumoso",
		"Un disco opalescente giace a terra: la nebbia lo tiene composto\n"
		"come vetro appena soffiato, pronto a proteggere chi esce dal velo.\n",
		4, ITEM_RESISTANT, 0, 500, 0,
	},
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 1,
		"scudo runa sigillo pulsante",
		"uno scudo dal sigillo pulsante",
		"Runi argentate si accendono e si spengono sulla lamiera;\n"
		"il sigillo regge finche' qualcuno osa ancora reggerlo.\n",
		5, ITEM_RESISTANT, 0, 1200, 0,
	},
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 2,
		"buckler nebbia piazza specchiata",
		"un buckler dalla Piazza specchiata",
		"La superficie riflette la Piazza delle Nebbie anche lontano da Myst:\n"
		"vapori gelidi gli danno forma e lo rendono leggero come un sospiro.\n",
		7, ITEM_RESISTANT, 0, 2200, 0,
	},
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 3,
		"pavise varco croce darkstar",
		"un pavise dalla croce DarkStar",
		"Sul bordo e' incisa la croce obliqua di DarkStar;\n"
		"pare forgiato per chi torna dal tempio nella foresta con le mani ancora fredde.\n",
		9, ITEM_RESISTANT, 0, 3500, 0,
	},
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 4,
		"scudo custode lamiera finale",
		"uno scudo di lamiera finale",
		"Frammenti di sala finale e sigilli spezzati sono fusi nella lamiera:\n"
		"e' un trofeo che non dovrebbe esistere fuori dalla Dimensione Effimera.\n",
		10, ITEM_RESISTANT, 0, 5000, 0,
	},
	{
		PROCAREA_REWARD_SHIELD_VNUM_BASE + 5,
		"scudo eclisse disco assorbente",
		"uno scudo disco assorbente",
		"La lamiera non riflette nulla: assorbe la luce come il varco stesso.\n"
		"Chi lo regge porta seco l'eco della dimensione ancora aperta.\n",
		12, ITEM_RESISTANT, 0, 7500, 0,
	},
};

static_assert(PROCAREA_REWARD_SHIELD_COUNT == PROCAREA_TEMPLATE_BANDS,
			  "procarea reward shields must match template bands");

static bool procarea_write_reward_shield_file(const ProcRewardShieldTpl& tpl) {
	char path[96];
	std::snprintf(path, sizeof(path), "%s/%ld", OBJ_DIR, tpl.vnum);
	FILE* f = std::fopen(path, "wt");
	if(f == nullptr) {
		mudlog(LOG_SYSERR, "procarea: cannot write reward shield %s (%s)", path,
			   std::strerror(errno));
		return false;
	}

	std::fprintf(f, "#%ld\n", tpl.vnum);
	fwrite_string(f, const_cast<char*>(tpl.name));
	fwrite_string(f, const_cast<char*>(tpl.short_descr));
	fwrite_string(f, const_cast<char*>(tpl.description));
	fwrite_string(f, nullptr);
	std::fprintf(f, "%d %u 1|512|131072\n", ITEM_ARMOR, tpl.extra_flags);
	std::fprintf(f, "%d %d 0 0\n", tpl.ac, tpl.ac);
	std::fprintf(f, "5 %d 0\n", tpl.cost);
	if(tpl.apply_ac != 0) {
		std::fprintf(f, "A\n%d %d\n", APPLY_AC, tpl.apply_ac);
	}
	if(tpl.extra_flags2 != 0) {
		std::fprintf(f, "F\n%u\n", tpl.extra_flags2);
	}
	std::fclose(f);
	return true;
}

void boot_reward_shields_impl() {
	for(const ProcRewardShieldTpl& tpl : kProcRewardShields) {
		if(procarea_write_reward_shield_file(tpl)) {
			mudlog(LOG_CHECK, "procarea: wrote reward shield %ld (%s)", tpl.vnum, tpl.short_descr);
		}
	}
}

[[nodiscard]] static int procarea_reward_gear_lerp(int lo, int hi, int band) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(PROCAREA_TEMPLATE_BANDS <= 1) {
		return lo;
	}
	return lo + (hi - lo) * b / (PROCAREA_TEMPLATE_BANDS - 1);
}

[[nodiscard]] static const ProcRewardGearSlotDef*
procarea_reward_gear_slot_def(ProcRewardGearSlot slot) {
	for(const ProcRewardGearSlotDef& def : kProcRewardGearSlots) {
		if(def.slot == slot) {
			return &def;
		}
	}
	return nullptr;
}

[[nodiscard]] static int procarea_reward_gear_variant_index(const ProcRewardGearSlotDef& def, int band,
															int sub_variant) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(def.type_flag == ITEM_WEAPON && def.variant_count == PROCAREA_TEMPLATE_BANDS * 3) {
		const int dtype = std::clamp(sub_variant, 0, 2);
		return b * 3 + dtype;
	}
	if(def.variant_count > PROCAREA_TEMPLATE_BANDS) {
		const int side = sub_variant != 0 ? 1 : 0;
		return b * 2 + side;
	}
	return b;
}

[[nodiscard]] static int procarea_reward_gear_band_from_variant(const ProcRewardGearSlotDef& def,
																int variant) {
	if(def.type_flag == ITEM_WEAPON && def.variant_count == PROCAREA_TEMPLATE_BANDS * 3) {
		return variant / 3;
	}
	if(def.variant_count > PROCAREA_TEMPLATE_BANDS) {
		return variant / 2;
	}
	return variant;
}

[[nodiscard]] static int procarea_reward_gear_weapon_damage_type(const ProcRewardGearSlotDef& def,
																 int variant) {
	if(def.type_flag == ITEM_WEAPON && def.variant_count == PROCAREA_TEMPLATE_BANDS * 3) {
		return variant % 3;
	}
	return 0;
}

struct ProcRewardWeaponDice {
	int num;
	int sides;
};

/** Media danno ~9 (band 0) → ~18 (band 5). */
static constexpr ProcRewardWeaponDice kProcRewardWeaponDiceByBand[PROCAREA_TEMPLATE_BANDS] = {
	{ 2, 8 }, /* 9.0 */
	{ 3, 6 }, /* 10.5 */
	{ 3, 7 }, /* 12.0 */
	{ 3, 8 }, /* 13.5 */
	{ 4, 7 }, /* 16.0 */
	{ 4, 8 }, /* 18.0 */
};

static constexpr int kProcRewardWeaponDamageValue[] = {
	3,  /* slash */
	11, /* pierce */
	6,  /* blunt / crush */
};

[[nodiscard]] static int procarea_weapon_hit_dam_bonus(int band) {
	const int b = std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(b >= 4) {
		return 5;
	}
	if(b >= 2) {
		return 4;
	}
	return 3;
}

long reward_gear_vnum(ProcRewardGearSlot slot, int band, int sub_variant) {
	const ProcRewardGearSlotDef* def = procarea_reward_gear_slot_def(slot);
	if(def == nullptr) {
		return -1;
	}
	const int variant = procarea_reward_gear_variant_index(*def, band, sub_variant);
	if(variant < 0 || variant >= def->variant_count) {
		return -1;
	}

	int offset = 0;
	for(const ProcRewardGearSlotDef& entry : kProcRewardGearSlots) {
		if(entry.slot == slot) {
			return PROCAREA_REWARD_GEAR_VNUM_BASE + offset + variant;
		}
		offset += entry.variant_count;
	}
	return -1;
}

static void procarea_build_reward_gear_text(const ProcRewardGearSlotDef& def, int /*band*/, int variant,
											char* name, std::size_t name_len, char* short_descr,
											std::size_t short_len, char* description,
											std::size_t desc_len) {
	const ProcRewardNameEntry* entry = procarea_reward_name_for(def, variant);
	if(entry == nullptr) {
		std::snprintf(name, name_len, "reliquia dimensione effimera");
		std::snprintf(short_descr, short_len, "una reliquia della dimensione effimera");
		std::snprintf(description, desc_len,
					  "Il metallo sembra trattenuto da rune instabili:\n"
					  "e' un premio nato nella Dimensione Effimera, fuori dal tempo di Myst.\n");
		return;
	}
	std::snprintf(name, name_len, "%s", entry->keywords);
	std::snprintf(short_descr, short_len, "%s", entry->short_descr);
	std::snprintf(description, desc_len, "%s", entry->description);
}

static bool procarea_write_reward_gear_file(long vnum, const ProcRewardGearSlotDef& def, int band,
											int variant) {
	char path[96];
	std::snprintf(path, sizeof(path), "%s/%ld", OBJ_DIR, vnum);
	FILE* f = std::fopen(path, "wt");
	if(f == nullptr) {
		mudlog(LOG_SYSERR, "procarea: cannot write reward gear %s (%s)", path,
			   std::strerror(errno));
		return false;
	}

	char name[160];
	char short_descr[160];
	char description[256];
	procarea_build_reward_gear_text(def, band, variant, name, sizeof(name), short_descr,
									sizeof(short_descr), description, sizeof(description));

	const int cost = kProcRewardGearCosts[std::clamp(band, 0, PROCAREA_TEMPLATE_BANDS - 1)];
	int value0 = 0;
	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	int weight = 1;

	if(def.type_flag == ITEM_ARMOR) {
		const int ac = procarea_reward_gear_lerp(def.ac_min, def.ac_max, band);
		value0 = ac;
		value1 = ac;
		weight = 3 + band;
	} else if(def.type_flag == ITEM_WEAPON) {
		const ProcRewardWeaponDice& dice = kProcRewardWeaponDiceByBand[std::clamp(band, 0,
																				   PROCAREA_TEMPLATE_BANDS -
																					   1)];
		const int dtype =
			std::clamp(procarea_reward_gear_weapon_damage_type(def, variant), 0, 2);
		value0 = 0;
		value1 = dice.num;
		value2 = dice.sides;
		value3 = kProcRewardWeaponDamageValue[dtype];
		weight = 2 + band;
	} else if(def.type_flag == ITEM_LIGHT) {
		value2 = def.light_hours;
		weight = 1;
	}

	std::fprintf(f, "#%ld\n", vnum);
	fwrite_string(f, name);
	fwrite_string(f, short_descr);
	fwrite_string(f, description);
	fwrite_string(f, nullptr);
	std::fprintf(f, "%d %lu %s\n", def.type_flag, ITEM_RESISTANT, def.wear_flags);
	std::fprintf(f, "%d %d %d %d\n", value0, value1, value2, value3);
	std::fprintf(f, "%d %d 0\n", weight, cost);
	if(def.type_flag == ITEM_WEAPON) {
		const int hit_dam = procarea_weapon_hit_dam_bonus(band);
		std::fprintf(f, "A\n%d %d\n", APPLY_HITROLL, hit_dam);
		std::fprintf(f, "A\n%d %d\n", APPLY_DAMROLL, hit_dam);
	}
	std::fclose(f);
	return true;
}

void boot_reward_gear_impl() {
	int offset = 0;
	for(const ProcRewardGearSlotDef& def : kProcRewardGearSlots) {
		for(int variant = 0; variant < def.variant_count; ++variant) {
			const long vnum = PROCAREA_REWARD_GEAR_VNUM_BASE + offset + variant;
			const int band = procarea_reward_gear_band_from_variant(def, variant);
			if(procarea_write_reward_gear_file(vnum, def, band, variant)) {
				mudlog(LOG_CHECK, "procarea: wrote reward gear %ld (%s band %d)", vnum,
					   def.keyword, band);
			}
		}
		offset += def.variant_count;
	}
}

static bool procarea_try_add_weapon_affect(struct obj_data* obj, int location,
										   int modifier) {
	if(obj == nullptr || modifier == 0) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == location) {
			return false;
		}
	}
	const int slot = procarea_find_free_affect_slot(obj);
	if(slot < 0) {
		return false;
	}
	obj->affected[slot].location = location;
	obj->affected[slot].modifier = modifier;
	return true;
}

[[nodiscard]] static int procarea_roll_nonhuman_race() {
	for(int attempt = 0; attempt < 32; ++attempt) {
		const int race = number(2, MAX_RACE - 1);
		if(race != RACE_HUMAN) {
			return race;
		}
	}
	return RACE_ELVEN;
}

[[nodiscard]] static int procarea_roll_align_slayer_flag() {
	switch(number(0, 2)) {
	case 0:
		return SLAYER_GOOD;
	case 1:
		return SLAYER_NEUTRAL;
	default:
		return SLAYER_EVIL;
	}
}

/** Un solo roll su 100000: al massimo una proc speciale per arma (band >= 4). */
static void procarea_roll_reward_weapon_proc(struct obj_data* obj) {
	const int roll = number(0, 99999);
	if(roll < 10) {
		procarea_try_add_weapon_affect(obj, APPLY_WEAPON_SPELL, SPELL_CAUSE_CRITICAL);
		return;
	}
	if(roll < 310) {
		procarea_try_add_weapon_affect(obj, APPLY_WEAPON_SPELL, SPELL_CAUSE_SERIOUS);
		return;
	}
	if(roll < 1310) {
		procarea_try_add_weapon_affect(obj, APPLY_WEAPON_SPELL, SPELL_CAUSE_LIGHT);
		return;
	}
	if(roll < 1360) {
		procarea_try_add_weapon_affect(obj, APPLY_ALIGN_SLAYER, procarea_roll_align_slayer_flag());
		return;
	}
	if(roll < 2360) {
		procarea_try_add_weapon_affect(obj, APPLY_RACE_SLAYER, procarea_roll_nonhuman_race());
	}
}

void roll_reward_weapon_impl(struct obj_data* obj, int template_band,
							 bool instance_has_ranger) {
	if(obj == nullptr || GET_ITEM_TYPE(obj) != ITEM_WEAPON) {
		return;
	}

	if(instance_has_ranger && number(0, 99) < 30) {
		SET_BIT(obj->obj_flags.wear_flags, ITEM_HOLD);
	}

	const int band = std::clamp(template_band, 0, PROCAREA_TEMPLATE_BANDS - 1);
	if(band < 4) {
		return;
	}

	procarea_roll_reward_weapon_proc(obj);
}

bool instance_has_ranger(const ProcAreaInstance& inst) {
	return procarea_instance_has_ranger(inst);
}

void roll_reward_weapon_impl(struct obj_data* obj, const ProcAreaInstance& inst) {
	roll_reward_weapon_impl(obj, inst.template_band, procarea_instance_has_ranger(inst));
}

} // namespace procarea_internal
} // namespace Alarmud
