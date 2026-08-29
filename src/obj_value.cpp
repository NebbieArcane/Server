/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "obj_value.hpp"

#include <algorithm>
#include <cmath>
#include <map>
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
#include "object_instance.hpp"
#include "multiclass.hpp"
#include "config.hpp"
#include "flags.hpp"
#include "procarea.hpp"
#if USE_MYSQL
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>
#endif

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

[[nodiscard]] std::map<int, long> SumAffectsByLocation(const struct obj_data* obj) {
	std::map<int, long> out;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		if(loc == APPLY_NONE || loc == APPLY_SKIP || mod == 0) {
			continue;
		}
		out[loc] += mod;
	}
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

	const auto a = SumAffectsByLocation(edited);
	const auto b = SumAffectsByLocation(original);
	auto ia = a.begin();
	auto ib = b.begin();
	while(ia != a.end() || ib != b.end()) {
		if(ib == b.end() || (ia != a.end() && ia->first < ib->first)) {
			AppendLine(out, "+ " + TypeName(ia->first, apply_types) + " by " +
								std::to_string(ia->second));
			++ia;
		}
		else if(ia == a.end() || (ib != b.end() && ib->first < ia->first)) {
			AppendLine(out, "- " + TypeName(ib->first, apply_types) + " by " +
								std::to_string(ib->second));
			++ib;
		}
		else {
			const long delta = ia->second - ib->second;
			if(delta > 0) {
				AppendLine(out, "+ " + TypeName(ia->first, apply_types) + " by " +
									std::to_string(delta));
			}
			else if(delta < 0) {
				AppendLine(out, "- " + TypeName(ia->first, apply_types) + " by " +
									std::to_string(-delta));
			}
			++ia;
			++ib;
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

[[nodiscard]] std::string ResolveEditOwnerName(const struct obj_data* obj) {
	if(obj && obj->personal_owner[0] != '\0') {
		return obj->personal_owner;
	}
	return object_instance_extract_ed_owner(obj ? obj->name : nullptr);
}

[[nodiscard]] int CountClassesInFileU(const struct char_file_u& st) {
	int tot = 0;
	for(int i = 0; i < MAX_CLASS; ++i) {
		if(st.level[i]) {
			++tot;
		}
	}
	return tot;
}

[[nodiscard]] int CountOwnerClassesMysql(const std::string& name) {
#if !USE_MYSQL
	(void)name;
	return 0;
#else
	if(name.empty()) {
		return 0;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == name);
		if(!pg || !pg->id) {
			return 0;
		}
		DB* db = Sql::getMysql();
		if(!db) {
			return 0;
		}
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT COUNT(*) FROM character_classes WHERE toon_id=" << pg->id
			<< " AND level > 0";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const int n = (row && row[0]) ? static_cast<int>(strtol(row[0], nullptr, 10)) : 0;
		mysql_free_result(res);
		return n;
	}
	catch(...) {
		return 0;
	}
#endif
}

[[nodiscard]] int ResolveOwnerClassCount(const std::string& owner_name) {
	if(owner_name.empty()) {
		return 0;
	}
	struct char_data* online = get_char(owner_name.c_str());
	if(online && !IS_NPC(online)) {
		return HowManyClasses(online);
	}
	const int from_mysql = CountOwnerClassesMysql(owner_name);
	if(from_mysql > 0) {
		return from_mysql;
	}
	char_file_u st {};
	if(load_char_mysql(owner_name.c_str(), &st)) {
		const int n = CountClassesInFileU(st);
		if(n > 0) {
			return n;
		}
	}
	if(load_char(owner_name.c_str(), &st)) {
		return CountClassesInFileU(st);
	}
	return 0;
}

[[nodiscard]] double ClassMultFromCount(int class_count) noexcept {
	if(class_count >= 3) {
		return kObjValueClassMultTri;
	}
	if(class_count == 2) {
		return kObjValueClassMultBi;
	}
	return 1.0;
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
			/* Listino: Mana [10] = 15M → 1.5M/unità → 150 raw (mega = raw/100). */
			valore += SignedAffectCost(mod, 150, 300);
			break;

		case APPLY_HIT:
			/* Listino: Hit [10] = 30M → 3M/unità → 300 raw. */
			valore += SignedAffectCost(mod, 300, 600);
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
			/* Listino: Mana/Hit Regen [5] = 15M → 3M/unità → 300 raw. */
			valore += SignedAffectCost(mod, 300, 600);
			break;

		case APPLY_MOVE_REGEN:
			/* Listino: Move Regen [5] = 10M → 2M/unità → 200 raw. */
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

ObjEditAnalysis AnalyzeObjEditAgainst(struct obj_data* obj, const struct obj_data* baseline,
									  bool staff_incremental_absolute) {
	ObjEditAnalysis report;
	if(obj == nullptr || baseline == nullptr) {
		return report;
	}

	const ExpValue edited = CheckValueObj(obj);
	const ExpValue base_val = CheckValueObj(baseline);
	report.diff = DiffFromRaw(edited, base_val);

	if(staff_incremental_absolute) {
		report.absolute.valore = ClampNonNegative(edited.valore - base_val.valore);
		report.absolute.derent = ClampNonNegative(edited.derent - base_val.derent);
		report.absolute.rune =
			static_cast<int>(ClampNonNegative(static_cast<long>(edited.rune - base_val.rune)));
	}
	else {
		report.absolute = edited;
	}

	report.owner_name = ResolveEditOwnerName(obj);
	report.owner_classes = ResolveOwnerClassCount(report.owner_name);
	report.class_mult = ClassMultFromCount(report.owner_classes);
	if(report.class_mult != 1.0 && report.diff.valore > 0) {
		report.diff.valore =
			static_cast<long>(std::llround(static_cast<double>(report.diff.valore) *
										   report.class_mult));
	}

	if(IS_OBJ_STAT(obj, ITEM_IMMUNE) && !IS_OBJ_STAT(baseline, ITEM_IMMUNE) &&
	   report.diff.valore > 0) {
		report.diff.valore = (report.diff.valore * 3) / 2;
	}

	report.changes = DescribeStructuralDiff(obj, baseline);
	report.has_edit = IsMarkedEdited(obj) || DiffHasValue(report.diff) ||
					  !report.changes.empty();
	return report;
}

ObjEditAnalysis AnalyzeProcareaStaffEdit(struct obj_data* obj) {
	ObjEditAnalysis report;
	if(obj == nullptr || !procarea_obj_is_reward(obj)) {
		return report;
	}
#if USE_MYSQL
	if(obj->db_instance_id == 0) {
		return report;
	}
	struct obj_data* baseline =
		object_instance_materialize_create_baseline(obj->db_instance_id);
	if(baseline == nullptr) {
		return report;
	}
	report = AnalyzeObjEditAgainst(obj, baseline, true);
	extract_obj(baseline);
#else
	(void)obj;
#endif
	return report;
}

ObjEditAnalysis AnalyzeObjEdit(struct obj_data* obj) {
	ObjEditAnalysis report;
	if(obj == nullptr) {
		return report;
	}

	const int iVNum = ResolvePrototypeVnum(obj);
	const int rNum = real_object(iVNum);
	struct obj_data* original = nullptr;
	if(rNum >= 0) {
		original = read_object(rNum, REAL);
	}

	if(original == nullptr) {
		report.absolute = CheckValueObj(obj);
		report.diff = DiffFromRaw(report.absolute, ExpValue{});
		if(IsMarkedEdited(obj) || DiffHasValue(report.diff)) {
			report.changes = "  (prototipo non disponibile)\n\r";
			report.has_edit = true;
		}
		return report;
	}

	report = AnalyzeObjEditAgainst(obj, original, false);
	extract_obj(original);
	return report;
}

} // namespace Alarmud
