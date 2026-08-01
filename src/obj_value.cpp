/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "obj_value.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "autoenums.hpp"
#include "constants.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

[[nodiscard]] constexpr long SignedAffectCost(int modifier, long positiveUnit,
											  long negativeUnit) noexcept {
	if(modifier < 0) {
		return static_cast<long>(modifier) * negativeUnit;
	}
	return static_cast<long>(modifier) * positiveUnit;
}

[[nodiscard]] long ClampNonNegative(long v) noexcept {
	return std::max(0L, v);
}

[[nodiscard]] int ResolvePrototypeVnum(const struct obj_data* obj) {
	int iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;

	const bool useOriginal =
		(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) || IS_OBJ_STAT2(obj, ITEM2_EDIT)) &&
		obj->char_vnum > 0 &&
		static_cast<int>(obj->char_vnum) != iVNum;

	if(useOriginal) {
		return static_cast<int>(obj->char_vnum);
	}
	return iVNum;
}

[[nodiscard]] ExpValue DiffFromRaw(const ExpValue& edited, const ExpValue& original) noexcept {
	ExpValue diff;
	/* Edit: aumento di valore rispetto al prototipo. */
	diff.valore = ClampNonNegative((edited.valore - original.valore) * kObjValueStorageScale);
	/*
	 * Derent: si "spende" per abbassare cost_per_day rispetto al prototipo
	 * (formula ProvaLocale su value_exp_total - value_exp).
	 */
	diff.derent = ClampNonNegative((original.derent - edited.derent) * kObjValueStorageScale);
	diff.rune = static_cast<int>(ClampNonNegative(static_cast<long>(original.rune - edited.rune)));
	return diff;
}

[[nodiscard]] bool DiffHasValue(const ExpValue& d) noexcept {
	return d.valore != 0 || d.derent != 0 || d.rune != 0;
}

using AffPair = std::pair<int, int>; /* location, modifier */

[[nodiscard]] std::vector<AffPair> CollectAffects(const struct obj_data* obj) {
	std::vector<AffPair> out;
	out.reserve(MAX_OBJ_AFFECT);
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		if(loc == APPLY_NONE || loc == APPLY_SKIP || mod == 0) {
			continue;
		}
		out.emplace_back(loc, mod);
	}
	std::sort(out.begin(), out.end());
	return out;
}

[[nodiscard]] std::string TypeName(int type, const char* names[]) {
	char buf[MAX_STRING_LENGTH];
	sprinttype(type, names, buf);
	return buf;
}

[[nodiscard]] std::string BitNames(unsigned long bits, const char* names[]) {
	char buf[MAX_STRING_LENGTH];
	sprintbit(bits, names, buf);
	return buf;
}

void AppendLine(std::ostringstream& out, const std::string& line) {
	out << "  " << line << "\n\r";
}

void AppendFlagDelta(std::ostringstream& out, unsigned long edited, unsigned long original,
					 const char* names[], const char* label) {
	const unsigned long added = edited & ~original;
	const unsigned long removed = original & ~edited;
	if(added != 0) {
		AppendLine(out, std::string("+ ") + label + ": " + BitNames(added, names));
	}
	if(removed != 0) {
		AppendLine(out, std::string("- ") + label + ": " + BitNames(removed, names));
	}
}

[[nodiscard]] std::string DescribeStructuralDiff(const struct obj_data* edited,
												 const struct obj_data* original) {
	std::ostringstream out;

	auto a = CollectAffects(edited);
	auto b = CollectAffects(original);
	std::size_t i = 0;
	std::size_t j = 0;
	while(i < a.size() || j < b.size()) {
		if(j >= b.size() || (i < a.size() && a[i] < b[j])) {
			AppendLine(out, "+ " + TypeName(a[i].first, apply_types) + " by " +
								std::to_string(a[i].second));
			++i;
		}
		else if(i >= a.size() || (j < b.size() && b[j] < a[i])) {
			AppendLine(out, "- " + TypeName(b[j].first, apply_types) + " by " +
								std::to_string(b[j].second));
			++j;
		}
		else {
			++i;
			++j;
		}
	}

	AppendFlagDelta(out, edited->obj_flags.extra_flags, original->obj_flags.extra_flags,
					extra_bits, "extra");
	AppendFlagDelta(out, edited->obj_flags.extra_flags2, original->obj_flags.extra_flags2,
					extra_bits2, "extra2");
	AppendFlagDelta(out, edited->obj_flags.wear_flags, original->obj_flags.wear_flags,
					wear_bits, "wear");
	AppendFlagDelta(out, edited->obj_flags.bitvector, original->obj_flags.bitvector,
					affected_bits, "affect bits");

	if(edited->obj_flags.cost_per_day != original->obj_flags.cost_per_day) {
		AppendLine(out, "cost_per_day: " + std::to_string(original->obj_flags.cost_per_day) +
							" -> " + std::to_string(edited->obj_flags.cost_per_day));
	}
	if(edited->obj_flags.cost != original->obj_flags.cost) {
		AppendLine(out, "cost: " + std::to_string(original->obj_flags.cost) + " -> " +
							std::to_string(edited->obj_flags.cost));
	}
	if(edited->obj_flags.weight != original->obj_flags.weight) {
		AppendLine(out, "weight: " + std::to_string(original->obj_flags.weight) + " -> " +
							std::to_string(edited->obj_flags.weight));
	}
	for(int v = 0; v < 4; ++v) {
		if(edited->obj_flags.value[v] != original->obj_flags.value[v]) {
			AppendLine(out, "value[" + std::to_string(v) + "]: " +
								std::to_string(original->obj_flags.value[v]) + " -> " +
								std::to_string(edited->obj_flags.value[v]));
		}
	}
	if(edited->obj_flags.type_flag != original->obj_flags.type_flag) {
		AppendLine(out, "type: " + TypeName(original->obj_flags.type_flag, item_types) +
							" -> " + TypeName(edited->obj_flags.type_flag, item_types));
	}

	return out.str();
}

[[nodiscard]] bool IsMarkedEdited(const struct obj_data* obj) noexcept {
	return IS_OBJ_STAT2(obj, ITEM2_EDIT) || IS_OBJ_STAT2(obj, ITEM2_PERSONAL);
}

} // namespace

ExpValue ScaleObjExpValue(const ExpValue& raw, long scale) noexcept {
	return ExpValue{
		raw.valore * scale,
		raw.derent * scale,
		raw.rune
	};
}

ExpValue CheckValueObj(const struct obj_data* obj) {
	if(obj == nullptr) {
		return {};
	}

	long valore = 0;

	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const auto& aff = obj->affected[i];
		const int mod = aff.modifier;

		switch(aff.location) {
		case APPLY_NONE:
		case APPLY_SEX:
			break;

		case APPLY_AFF2:
			if(IS_SET(mod, AFF2_DANGER_SENSE)) {
				valore += 15000;
			}
			break;

		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_INT:
		case APPLY_WIS:
		case APPLY_CON:
		case APPLY_CHR:
			valore += SignedAffectCost(mod, 1500, 3000);
			break;

		case APPLY_LEVEL:
		case APPLY_AGE:
		case APPLY_CHAR_WEIGHT:
		case APPLY_CHAR_HEIGHT:
			break;

		case APPLY_MANA:
		case APPLY_HIT:
			valore += SignedAffectCost(mod, 150, 300);
			break;

		case APPLY_MOVE:
			valore += SignedAffectCost(mod, 100, 200);
			break;

		case APPLY_GOLD:
		case APPLY_EXP:
			break;

		case APPLY_AC:
			valore -= static_cast<long>(mod) * 100;
			break;

		case APPLY_HITROLL:
			valore += SignedAffectCost(mod, 4500, 9000);
			break;

		case APPLY_DAMROLL:
		case APPLY_SPELLPOWER:
			/* Su ProvaLocale SPELLPOWER era sotto #if NO_SPELLPOWER; qui e' sempre prezzato. */
			valore += SignedAffectCost(mod, 10000, 20000);
			break;

		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		case APPLY_SAVE_ALL:
			break;

		case APPLY_IMMUNE: {
			if(IS_SET(mod, IMM_ACID)) {
				valore += 7500;
			}
			if(IS_SET(mod, IMM_ELEC)) {
				valore += 15000;
			}
			if(IS_SET(mod, IMM_FIRE)) {
				valore += 10000;
			}
			if(IS_SET(mod, IMM_COLD)) {
				valore += 7500;
			}
			if(IS_SET(mod, IMM_ENERGY)) {
				valore += 15000;
			}
			if(IS_SET(mod, IMM_DRAIN)) {
				valore += 3000;
			}
			if(IS_SET(mod, IMM_HOLD)) {
				valore += 7500;
			}
			if(IS_SET(mod, IMM_POISON)) {
				valore += 3000;
			}
			if(IS_SET(mod, IMM_SLASH)) {
				valore += 15000;
			}
			if(IS_SET(mod, IMM_PIERCE)) {
				valore += 15000;
			}
			if(IS_SET(mod, IMM_BLUNT)) {
				valore += 30000;
			}
			break;
		}

		case APPLY_SUSC:
			break;

		case APPLY_M_IMMUNE: {
			if(IS_SET(mod, IMM_DRAIN)) {
				valore += 10000;
			}
			if(IS_SET(mod, IMM_CHARM)) {
				valore += 6000;
			}
			if(IS_SET(mod, IMM_POISON)) {
				valore += 10000;
			}
			break;
		}

		case APPLY_SPELL: {
			if(IS_SET(mod, AFF_INVISIBLE)) {
				valore += 3000;
			}
			if(IS_SET(mod, AFF_TELEPATHY)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_WATERBREATH)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_TRUE_SIGHT)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_SCRYING)) {
				valore += 15000;
			}
			if(IS_SET(mod, AFF_PROTECT_FROM_EVIL)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_SENSE_LIFE)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_FLYING)) {
				valore += 5000;
			}
			if(IS_SET(mod, AFF_GLOBE_DARKNESS)) {
				valore += 5000;
			}
			break;
		}

		case APPLY_HITNDAM:
		case APPLY_HITNSP:
			valore += SignedAffectCost(mod, 14500, 29000);
			break;

		case APPLY_WEAPON_SPELL:
		case APPLY_EAT_SPELL:
		case APPLY_BACKSTAB:
		case APPLY_KICK:
		case APPLY_SNEAK:
		case APPLY_HIDE:
		case APPLY_BASH:
		case APPLY_PICK:
		case APPLY_STEAL:
		case APPLY_TRACK:
		case APPLY_SPELLFAIL:
		case APPLY_HASTE:
		case APPLY_SLOW:
		case APPLY_ATTACKS:
		case APPLY_FIND_TRAPS:
		case APPLY_RIDE:
		case APPLY_RACE_SLAYER:
		case APPLY_ALIGN_SLAYER:
		case APPLY_MOD_THIRST:
		case APPLY_MOD_HUNGER:
		case APPLY_MOD_DRUNK:
		case APPLY_T_STR:
		case APPLY_T_INT:
		case APPLY_T_DEX:
		case APPLY_T_WIS:
		case APPLY_T_CON:
		case APPLY_T_CHR:
		case APPLY_T_HPS:
		case APPLY_T_MOVE:
		case APPLY_T_MANA:
		case APPLY_SKIP:
			break;

		case APPLY_MANA_REGEN:
		case APPLY_HIT_REGEN:
			valore += SignedAffectCost(mod, 150, 300);
			break;

		case APPLY_MOVE_REGEN:
			valore += SignedAffectCost(mod, 200, 400);
			break;

		default:
			break;
		}
	}

	const long dayUnits = static_cast<long>(
		std::ceil(static_cast<double>(obj->obj_flags.cost_per_day) / 1000.0));
	const long derent = dayUnits * (kObjValuePriceExp / 10000);
	const int rune = static_cast<int>(dayUnits * kObjValuePriceRune);

	return ExpValue{valore, derent, rune};
}

ExpValue CheckDiffValue(struct obj_data* obj) {
	return AnalyzeObjEdit(obj).diff;
}

ObjEditAnalysis AnalyzeObjEdit(struct obj_data* obj) {
	ObjEditAnalysis report;
	if(obj == nullptr) {
		return report;
	}

	report.absolute = CheckValueObj(obj);

	const int iVNum = ResolvePrototypeVnum(obj);
	const int rNum = real_object(iVNum);
	struct obj_data* original = nullptr;
	if(rNum >= 0) {
		original = read_object(rNum, REAL);
	}

	if(original == nullptr) {
		report.diff = DiffFromRaw(report.absolute, ExpValue{});
		if(IsMarkedEdited(obj) || DiffHasValue(report.diff)) {
			report.changes = "  (prototipo non disponibile)\n\r";
			report.has_edit = true;
		}
		return report;
	}

	const ExpValue base = CheckValueObj(original);
	report.diff = DiffFromRaw(report.absolute, base);
	if(IS_OBJ_STAT(obj, ITEM_IMMUNE) && !IS_OBJ_STAT(original, ITEM_IMMUNE) &&
	   report.diff.valore > 0) {
		report.diff.valore = (report.diff.valore * 3) / 2;
	}

	report.changes = DescribeStructuralDiff(obj, original);
	report.has_edit = IsMarkedEdited(obj) || DiffHasValue(report.diff) ||
					  !report.changes.empty();

	extract_obj(original);
	return report;
}

} // namespace Alarmud
