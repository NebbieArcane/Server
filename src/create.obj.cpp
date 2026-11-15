/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "create.obj.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "handler.hpp"
#include "modify.hpp"
#include "utility.hpp"
#include "vt100c.hpp"
#include "editor_term.hpp"
#include "procarea.hpp"
#include "db.hpp"
#include "object_instance.hpp"
#include "spell_parser.hpp"
#include "edit_pool.hpp"
#include "obj_value.hpp"
#include "vt_screen.hpp"

namespace Alarmud {

#define OBJ_MAIN_MENU         0
#define CHANGE_OBJ_NAME       1
#define CHANGE_OBJ_SHORT      2
#define CHANGE_OBJ_DESC       3
#define CHANGE_OBJ_TYPE       4
#define CHANGE_OBJ_WEAR       5
#define CHANGE_OBJ_FLAGS      6
#define CHANGE_OBJ_WEIGHT     7
#define CHANGE_OBJ_VALUE      8
#define CHANGE_OBJ_COST       9
#define CHANGE_OBJ_AFFECTS   10
#define CHANGE_OBJ_AFFECT1   11
#define CHANGE_OBJ_AFFECT2   12
#define CHANGE_OBJ_AFFECT3   13
#define CHANGE_OBJ_AFFECT4   14
#define CHANGE_AFFECT1_MOD   15
#define CHANGE_AFFECT2_MOD   16
#define CHANGE_AFFECT3_MOD   17
#define CHANGE_AFFECT4_MOD   18
#define CHANGE_OBJ_VALUES    19
#define CHANGE_OBJ_VALUE1    20
#define CHANGE_OBJ_VALUE2    21
#define CHANGE_OBJ_VALUE3    22
#define CHANGE_OBJ_VALUE4    23
#define OBJ_HIT_RETURN       24

#define CHANGE_OBJ_AFFECT5   25
#define CHANGE_AFFECT5_MOD   26
#define CHANGE_OBJ_TYPE2     27
#define CHANGE_OBJ_SPECIAL   28

#define ENTER_CHECK        1

static const char* ObjApplyTypeName(int location) {
	if(location >= APPLY_NONE && location < APPLY_SKIP &&
			apply_types[location] != nullptr && apply_types[location][0] != '\n') {
		return apply_types[location];
	}
	return "UNKNOWN";
}

static void FormatObjAffectSummary(char* buf, std::size_t buflen, int location, int modifier) {
	if(location == APPLY_NONE && modifier == 0) {
		std::snprintf(buf, buflen, "NONE");
		return;
	}
	if(modifier >= 0) {
		std::snprintf(buf, buflen, "%s +%d", ObjApplyTypeName(location), modifier);
	}
	else {
		std::snprintf(buf, buflen, "%s %d", ObjApplyTypeName(location), modifier);
	}
}

static void SendObjAffectSlotList(struct char_data* ch) {
	char line[128];
	char summary[64];
	struct obj_data* obj = ch->specials.objedit;
	for(int i = 0; i < MAX_OBJ_AFFECT; i++) {
		FormatObjAffectSummary(summary, sizeof(summary),
							   obj->affected[i].location,
							   obj->affected[i].modifier);
		std::snprintf(line, sizeof(line), " %d) %s\n\r", i + 1, summary);
		send_to_char(line, ch);
	}
}

static const char* ObjValueLabel(int type_flag, int idx) {
	if(idx < 0 || idx > 3) {
		return "";
	}
	switch(type_flag) {
	case ITEM_LIGHT:
		if(idx == 0) {
			return "colour (not used)";
		}
		if(idx == 1) {
			return "type (not used)";
		}
		if(idx == 2) {
			return "hours (-1 = infinite)";
		}
		return "unused";
	case ITEM_SCROLL:
	case ITEM_POTION:
		if(idx == 0) {
			return "level";
		}
		if(idx == 1) {
			return "spell 1";
		}
		if(idx == 2) {
			return "spell 2";
		}
		return "spell 3";
	case ITEM_WAND:
	case ITEM_STAFF:
		if(idx == 0) {
			return "level";
		}
		if(idx == 1) {
			return "max charges";
		}
		if(idx == 2) {
			return "charges left";
		}
		return "spell";
	case ITEM_WEAPON:
		if(idx == 0) {
			return "reserved";
		}
		if(idx == 1) {
			return "dice count";
		}
		if(idx == 2) {
			return "dice sides";
		}
		return "damage type";
	case ITEM_FIREWEAPON:
		if(idx == 0) {
			return "strength required";
		}
		if(idx == 1) {
			return "max range (not used)";
		}
		if(idx == 2) {
			return "bonus range";
		}
		return "missile type";
	case ITEM_MISSILE:
		if(idx == 0) {
			return "break chance";
		}
		if(idx == 1) {
			return "dice count";
		}
		if(idx == 2) {
			return "dice sides";
		}
		return "missile type";
	case ITEM_ARMOR:
		if(idx == 0) {
			return "AC apply";
		}
		if(idx == 1) {
			return "full strength";
		}
		return "unused";
	case ITEM_TRAP:
		if(idx == 0) {
			return "trigger flags";
		}
		if(idx == 1) {
			return "damage type";
		}
		if(idx == 2) {
			return "level";
		}
		return "charges";
	case ITEM_CONTAINER:
		if(idx == 0) {
			return "max weight";
		}
		if(idx == 1) {
			return "lock flags";
		}
		if(idx == 2) {
			return "key vnum (-1 = none)";
		}
		return "corpse";
	case ITEM_DRINKCON:
		if(idx == 0) {
			return "capacity";
		}
		if(idx == 1) {
			return "fluid left";
		}
		if(idx == 2) {
			return "liquid";
		}
		return "flags (1=poison 2=endless)";
	case ITEM_KEY:
		if(idx == 0) {
			return "keytype (not used)";
		}
		return "unused";
	case ITEM_FOOD:
		if(idx == 0) {
			return "fullness";
		}
		if(idx == 3) {
			return "poisoned";
		}
		return "unused";
	case ITEM_MONEY:
		if(idx == 0) {
			return "coins";
		}
		return "unused";
	case ITEM_AUDIO:
		if(idx == 0) {
			return "sound interval (pulses)";
		}
		return "unused";
	case ITEM_CLAN_SYMBOL:
		if(idx == 0) {
			return "prince toon_id";
		}
		return "unused";
	case ITEM_NONE:
	case ITEM_TREASURE:
	case ITEM_WORN:
	case ITEM_OTHER:
	case ITEM_TRASH:
	case ITEM_NOTE:
	case ITEM_PEN:
	case ITEM_BOAT:
	case ITEM_BOARD:
	case ITEM_TREE:
	case ITEM_ROCK:
	case ITEM_M_GEM:
	case ITEM_M_MINERAL:
	case ITEM_BAR:
	case ITEM_JEWEL:
		return "unused";
	default:
		return "";
	}
}

static const char* ObjLookupTypeName(int type, const char** names) {
	if(names == nullptr || type < 0) {
		return nullptr;
	}
	for(int i = 0; names[i] != nullptr && names[i][0] != '\n'; i++) {
		if(i == type) {
			return names[i];
		}
	}
	return nullptr;
}

static void FormatObjValueDecoded(char* buf, std::size_t buflen, int type_flag, int idx, int value) {
	buf[0] = '\0';
	const char* name = nullptr;
	if(type_flag == ITEM_WEAPON && idx == 3) {
		name = ObjLookupTypeName(value, aszWeaponType);
	}
	else if(type_flag == ITEM_DRINKCON && idx == 2) {
		name = ObjLookupTypeName(value, drinks);
	}
	else if(((type_flag == ITEM_SCROLL || type_flag == ITEM_POTION) && idx >= 1) ||
			((type_flag == ITEM_WAND || type_flag == ITEM_STAFF) && idx == 3)) {
		/* Spell 1-based come in spell_info[] (0 = nessuno): stesso nome di identify. */
		if(value >= 1) {
			const std::string spell = spell_display_name(value);
			std::snprintf(buf, buflen, " (%s)", spell.c_str());
		}
		return;
	}
	if(name != nullptr && name[0] != '\0') {
		std::snprintf(buf, buflen, " (%s)", name);
	}
}



const char* obj_edit_menu = "    1) Name                    2) Short description\n\r"
							"    3) Description             4) Type\n\r"
							"    5) Wear positions          6) Extra flags\n\r"
							"    7) Weight                  8) Value\n\r"
							"    9) Rent cost              10) Extra affects\n\r"
							"   11) Object values          12) Special\n\r"
							"   $c000913) Modifiche vs prototipo e costo edit pool$c0007\n\r\n\r";

void ChangeObjFlags(struct char_data* ch, const char* arg, int type) {
	int i, row, update;
	unsigned long check = 0;
	char buf[255];
	/* extra_bits (32) + extra_bits2 fino a VAULT (11) = 43 voci menu. */
	static constexpr int kExtraFlagMenuCount = 43;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update >= kExtraFlagMenuCount) {
			return;
		}
		check = (update < 32) ? (1UL << update) : (1UL << (update - 32));
        if(update<32)
        {
            if(IS_SET(ch->specials.objedit->obj_flags.extra_flags, check)) {
                REMOVE_BIT(ch->specials.objedit->obj_flags.extra_flags, check);
            }
            else
            {
                SET_BIT(ch->specials.objedit->obj_flags.extra_flags, check);
            }
        }
        else
        {
                if(IS_SET(ch->specials.objedit->obj_flags.extra_flags2, check)) {
                    REMOVE_BIT(ch->specials.objedit->obj_flags.extra_flags2, check);
                }
                else
            {
				if(check == ITEM2_PERSONAL) {
					struct obj_data* o = ch->specials.objedit;
					const bool has_owner =
						(o->personal_owner[0] != '\0') ||
						!object_instance_extract_ed_owner(o->name).empty();
					if(!has_owner) {
						send_to_char(
							"PERSONAL richiede un proprietario. Usa: "
							"personalize <oggetto> <nome_pg>\n\r",
							ch);
					}
					else {
						SET_BIT(ch->specials.objedit->obj_flags.extra_flags2, check);
					}
				}
				else {
					SET_BIT(ch->specials.objedit->obj_flags.extra_flags2, check);
				}
            }
        }
	}

    if(ch->term == VT100)
    {
        EditorClear(ch);
        sprintf(buf, "Object Extra Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < kExtraFlagMenuCount; i++)
        {
            EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }

            check = (i < 32) ? (1UL << i) : (1UL << (i - 32));
            if (i < 32)
            {
                sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.objedit->obj_flags.extra_flags & (check)) ? "X" : " "), extra_bits[i]);
            }
            else
            {
                sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.objedit->obj_flags.extra_flags2 & (check)) ? "X" : " "), extra_bits2[i-32]);
            }

            send_to_char(buf, ch);
        }

        EditorGoto(ch, row + 8, 1);
        EditorPrompt(ch, "Select the number to toggle, <C/R> to return to main menu.\n\r--> ");
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d [%s] %s");
        char buf2[255];
        int x = 0;

        sprintf(buf, "\n\rObject Extra Flags:\n\r\n\r");
        send_to_char(buf, ch);

        for(i = 0; i < kExtraFlagMenuCount; i++)
        {
            check = (i < 32) ? (1UL << i) : (1UL << (i - 32));
            snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i < 32)
            {
                if(i & 1)
                {
                    fmt2 % "" % (i + 1) % ((ch->specials.objedit->obj_flags.extra_flags & (check)) ? "X" : " ") % extra_bits[i];
                    sb.append(fmt2.str().c_str());
                }
                else
                {
                    fmt % (i + 1) % ((ch->specials.objedit->obj_flags.extra_flags & (check)) ? "X" : " ") % extra_bits[i];
                    sb.append(fmt.str().c_str());
                    x = strlen(fmt.str().c_str());
                }
            }
            else
            {
                if(i & 1)
                {
                    fmt2 % "" % (i + 1) % ((ch->specials.objedit->obj_flags.extra_flags2 & (check)) ? "X" : " ") % extra_bits2[i-32];
                    sb.append(fmt2.str().c_str());
                }
                else
                {
                    fmt % (i + 1) % ((ch->specials.objedit->obj_flags.extra_flags2 & (check)) ? "X" : " ") % extra_bits2[i-32];
                    sb.append(fmt.str().c_str());
                    x = strlen(fmt.str().c_str());
                }
            }

            fmt.clear();
            fmt2.clear();
        }

		/* Count dispari: ultima voce in colonna sinistra senza \n\r. */
		if((kExtraFlagMenuCount & 1) != 0) {
			sb.append("\n\r");
		}
		sb.append("\n\r");
        page_string(ch->desc, sb.c_str(), true);
        EditorPrompt(ch, "Select the number to toggle, <C/R> to return to main menu.\n\r--> ");
    }
}



void ChangeObjWear(struct char_data* ch, const char* arg, int type) {
	int i, a, check=0, row, update;
	char buf[255];

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	if(type != ENTER_CHECK) {
		if(update < 0 || update > 19) {
			return;
		}
		i=1;
		if(update>0)
			for(a=1; a<=update; a++) {
				i*=2;
			}

		if(IS_SET(ch->specials.objedit->obj_flags.wear_flags, i)) {
			REMOVE_BIT(ch->specials.objedit->obj_flags.wear_flags, i);
		}
		else {
			SET_BIT(ch->specials.objedit->obj_flags.wear_flags, i);
		}
	}

    if(ch->term == VT100)
    {
        EditorClear(ch);
        sprintf(buf, "Object Wear Flags:");
        send_to_char(buf, ch);

        row = 0;
        for(i = 0; i < 20; i++)
        {
            EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
            if(i & 1)
            {
                row++;
            }

            check=1;
            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }
            sprintf(buf, "%-2d [%s] %s", i + 1, ((ch->specials.objedit->obj_flags.wear_flags & (check)) ? "X" : " "), wear_bits[i]);
            send_to_char(buf, ch);
        }

        EditorGoto(ch, 20, 1);
        EditorPrompt(ch, "Select the number to toggle, <C/R> to return to main menu.\n\r--> ");
    }
    else
    {
        string sb;
        boost::format fmt ("     %-2d [%s] %s");
        char buf2[255];
        int x = 0;

        send_to_char("\n\rObject Wear Flags:\n\r\n\r", ch);

        for(i = 0; i < 20; i++)
        {
            check = 1;
            snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
            strcat(buf2, "s%-2d [%s] %s\n\r");
            boost::format fmt2 (buf2);

            if(i>0)
                for(a=1; a<=i; a++)
                {
                    check*=2;
                }

            if(i & 1)
            {
                fmt2 % "" % (i + 1) % ((ch->specials.objedit->obj_flags.wear_flags & (check)) ? "X" : " ") % wear_bits[i];
                sb.append(fmt2.str().c_str());
            }
            else
            {
                fmt % (i + 1) % ((ch->specials.objedit->obj_flags.wear_flags & (check)) ? "X" : " ") % wear_bits[i];
                sb.append(fmt.str().c_str());
                x = strlen(fmt.str().c_str());
            }

            fmt.clear();
            fmt2.clear();
        }

        sb.append("\n\r");
        page_string(ch->desc, sb.c_str(), true);
        EditorPrompt(ch, "Select the number to toggle, <C/R> to return to main menu.\n\r--> ");
    }
}


namespace {

struct ObjPoolCostRow {
	EditPoolField field;
	const char* label;
};

constexpr ObjPoolCostRow kObjPoolCostRows[] = {
	{EditPoolField::Hp, "hp"},
	{EditPoolField::Mana, "mana"},
	{EditPoolField::Move, "move"},
	{EditPoolField::HpRegen, "hpregen"},
	{EditPoolField::ManaRegen, "manaregen"},
	{EditPoolField::MoveRegen, "moveregen"},
};

int ObjPoolUnits(const struct char_edit_pool_data& d, EditPoolField field) {
	switch(field) {
	case EditPoolField::Hp:
		return d.edit_hp;
	case EditPoolField::Mana:
		return d.edit_mana;
	case EditPoolField::Move:
		return d.edit_move;
	case EditPoolField::HpRegen:
		return d.edit_hp_regen;
	case EditPoolField::ManaRegen:
		return d.edit_mana_regen;
	case EditPoolField::MoveRegen:
		return d.edit_move_regen;
	}
	return 0;
}

bool ObjStrDiffers(const char* a, const char* b) {
	return std::strcmp(a ? a : "", b ? b : "") != 0;
}

/**
 * Stato dell'oggetto all'ingresso in oedit (copia leggera, nessun obj_data
 * in object_list). obj serve solo per il confronto di puntatori: non viene
 * mai dereferenziato.
 */
struct ObjEditSnapshot {
	const struct obj_data* obj = nullptr;
	std::string name;
	std::string short_desc;
	std::string description;
	struct obj_flag_data flags {};
	struct obj_affected_type affected[MAX_OBJ_AFFECT] {};
};

/* Uno snapshot per immortale in oedit; tolto da oedit_snapshot_clear. */
std::unordered_map<const struct char_data*, ObjEditSnapshot> g_oedit_snapshots;

void ObjEditSnapshotTake(const struct char_data* ch, const struct obj_data* obj) {
	ObjEditSnapshot snap;
	snap.obj = obj;
	snap.name = obj->name ? obj->name : "";
	snap.short_desc = obj->short_description ? obj->short_description : "";
	snap.description = obj->description ? obj->description : "";
	snap.flags = obj->obj_flags;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		snap.affected[i] = obj->affected[i];
	}
	g_oedit_snapshots[ch] = std::move(snap);
}

std::map<int, long> ObjAffectsByLocation(const struct obj_affected_type* affs) {
	std::map<int, long> m;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(affs[i].location != APPLY_NONE) {
			m[affs[i].location] += affs[i].modifier;
		}
	}
	return m;
}

void ObjAppendBitDelta(std::string& out, unsigned long now, unsigned long before,
					   const char* names[], const char* label) {
	char bits[MAX_STRING_LENGTH];
	const unsigned long added = now & ~before;
	const unsigned long removed = before & ~now;
	if(added != 0) {
		sprintbit(added, names, bits);
		out += std::string("  + ") + label + ": " + bits + " (aggiunto)\n\r";
	}
	if(removed != 0) {
		sprintbit(removed, names, bits);
		out += std::string("  - ") + label + ": " + bits + " (rimosso)\n\r";
	}
}

/**
 * Peso, costi e value con le etichette del menu di oedit (7 Weight, 8 Value,
 * 9 Rent cost, 11 Object values numerati 1-4 come nel sottomenu, con la
 * decodifica di spell/arma/liquido). Usato da sessione e prototipo.
 */
void ObjAppendScalarDiff(std::string& out, const struct obj_flag_data& a,
						 const struct obj_flag_data& b) {
	char buf[MAX_STRING_LENGTH];
	if(a.weight != b.weight) {
		std::snprintf(buf, sizeof(buf), "  weight: %d -> %d\n\r", a.weight, b.weight);
		out += buf;
	}
	if(a.cost != b.cost) {
		std::snprintf(buf, sizeof(buf), "  value (cost): %d -> %d\n\r", a.cost, b.cost);
		out += buf;
	}
	if(a.cost_per_day != b.cost_per_day) {
		std::snprintf(buf, sizeof(buf), "  rent cost: %d -> %d\n\r", a.cost_per_day,
					  b.cost_per_day);
		out += buf;
	}
	for(int i = 0; i < 4; ++i) {
		if(a.value[i] == b.value[i]) {
			continue;
		}
		char d_old[128];
		char d_new[128];
		FormatObjValueDecoded(d_old, sizeof(d_old), a.type_flag, i, a.value[i]);
		FormatObjValueDecoded(d_new, sizeof(d_new), b.type_flag, i, b.value[i]);
		const char* label = ObjValueLabel(b.type_flag, i);
		std::snprintf(buf, sizeof(buf), "  value %d%s%s%s: %d%s -> %d%s\n\r", i + 1,
					  label[0] ? " (" : "", label, label[0] ? ")" : "",
					  a.value[i], d_old, b.value[i], d_new);
		out += buf;
	}
}

/**
 * Toglie dal testo di DescribeStructuralDiff (condiviso con stat) le righe
 * di peso/costi/value, che il report di oedit rifa' con ObjAppendScalarDiff.
 */
std::string ObjStripScalarLines(const std::string& changes) {
	static const char* const kPrefixes[] = {"  weight:", "  cost:", "  cost_per_day:",
											"  value["};
	std::string out;
	std::size_t pos = 0;
	while(pos < changes.size()) {
		std::size_t end = changes.find("\n\r", pos);
		end = (end == std::string::npos) ? changes.size() : end + 2;
		const std::string line = changes.substr(pos, end - pos);
		bool drop = false;
		for(const char* pre : kPrefixes) {
			if(line.compare(0, std::strlen(pre), pre) == 0) {
				drop = true;
				break;
			}
		}
		if(!drop) {
			out += line;
		}
		pos = end;
	}
	return out;
}

/** Righe "vecchio -> nuovo" tra lo snapshot e l'oggetto attuale (vuoto se uguali). */
std::string ObjEditSessionDiff(const ObjEditSnapshot& snap, const struct obj_data* obj) {
	std::string out;
	char buf[MAX_STRING_LENGTH];
	const struct obj_flag_data& a = snap.flags;
	const struct obj_flag_data& b = obj->obj_flags;

	if(ObjStrDiffers(snap.name.c_str(), obj->name)) {
		out += "  name: " + snap.name + " -> " + (obj->name ? obj->name : "") + "\n\r";
	}
	if(ObjStrDiffers(snap.short_desc.c_str(), obj->short_description)) {
		out += "  short: " + snap.short_desc + " -> " +
			   (obj->short_description ? obj->short_description : "") + "\n\r";
	}
	if(ObjStrDiffers(snap.description.c_str(), obj->description)) {
		out += "  description cambiata\n\r";
	}
	if(a.type_flag != b.type_flag) {
		char t_old[128];
		char t_new[128];
		sprinttype(a.type_flag, item_types, t_old);
		sprinttype(b.type_flag, item_types, t_new);
		out += std::string("  type: ") + t_old + " -> " + t_new + "\n\r";
	}
	ObjAppendBitDelta(out, b.wear_flags, a.wear_flags, wear_bits, "wear");
	ObjAppendBitDelta(out, b.extra_flags, a.extra_flags, extra_bits, "extra");
	/* ITEM2_EDIT lo mette oedit stesso all'ingresso: non e' una modifica. */
	ObjAppendBitDelta(out, b.extra_flags2 & ~static_cast<unsigned>(ITEM2_EDIT),
					  a.extra_flags2 & ~static_cast<unsigned>(ITEM2_EDIT),
					  extra_bits2, "extra2");
	ObjAppendBitDelta(out, b.bitvector, a.bitvector, affected_bits, "affect bits");
	ObjAppendScalarDiff(out, a, b);

	/* Affect per location (non per slot: togliendone uno gli altri scalano). */
	const std::map<int, long> before = ObjAffectsByLocation(snap.affected);
	const std::map<int, long> now = ObjAffectsByLocation(obj->affected);
	for(const auto& [loc, mod] : now) {
		const auto it = before.find(loc);
		if(it == before.end()) {
			std::snprintf(buf, sizeof(buf), "  + %s by %ld (aggiunto)\n\r",
						  ObjApplyTypeName(loc), mod);
			out += buf;
		}
		else if(it->second != mod) {
			std::snprintf(buf, sizeof(buf), "  %s %ld -> %ld\n\r", ObjApplyTypeName(loc),
						  it->second, mod);
			out += buf;
		}
	}
	for(const auto& [loc, mod] : before) {
		if(now.find(loc) == now.end()) {
			std::snprintf(buf, sizeof(buf), "  - %s by %ld (rimosso)\n\r",
						  ObjApplyTypeName(loc), mod);
			out += buf;
		}
	}
	return out;
}

/** Riga "Listino oggetto (come stat)" se c'e' differenza di valore. */
void ObjAppendListinoLine(std::string& out, const ExpValue& diff) {
	if(diff.valore > 0 || diff.rune > 0) {
		out += "$c0005Listino oggetto (come stat)$c0007: $c0014" +
			   format_thousands(diff.valore) + "$c0007 xp, $c0014" +
			   format_thousands(diff.rune) + "$c0007 rune\n\r";
	}
}

/**
 * Costo edit pool (listino editpool, classi dell'owner) per le unita' in units.
 * Positivi: costo xp oppure rune, cap se l'owner e' online. Negativi: mostrati
 * con neg_label, senza rimborso. Appende a out; nulla se tutte le unita' sono 0
 * (lo gestisce il chiamante).
 */
void ObjAppendPoolCost(std::string& out, const struct char_edit_pool_data& units_by_field,
					   const std::string& owner_name, int owner_classes,
					   const char* title, const char* neg_label) {
	char buf[512];
	const int n_classes = std::max(1, owner_classes);
	const double mult = n_classes >= 3   ? kObjValueClassMultTri
						: n_classes == 2 ? kObjValueClassMultBi
										 : 1.0;
	struct char_data* owner =
		owner_name.empty() ? nullptr : get_char(owner_name.c_str());
	if(owner != nullptr && IS_NPC(owner)) {
		owner = nullptr;
	}

	if(owner_name.empty()) {
		std::snprintf(buf, sizeof(buf),
					  "$c0005%s$c0007 (nessun owner ED*/personal, "
					  "classi=$c00141$c0007 mult=$c00141.0x$c0007):\n\r",
					  title);
	}
	else {
		std::snprintf(buf, sizeof(buf),
					  "$c0005%s$c0007 per $c0015%s$c0007 "
					  "(classi=$c0014%d$c0007 mult=$c0014%.1fx$c0007%s):\n\r",
					  title, owner_name.c_str(), n_classes, mult,
					  owner_classes > 0 ? "" : ", classi non risolte");
	}
	out += buf;

	long long tot_xp = 0;
	long long tot_pq = 0;
	for(const auto& row : kObjPoolCostRows) {
		const int units = ObjPoolUnits(units_by_field, row.field);
		if(units == 0) {
			continue;
		}
		if(units < 0) {
			std::snprintf(buf, sizeof(buf), "  %-10s %+5d: %s\n\r", row.label, units,
						  neg_label);
			out += buf;
			continue;
		}
		const auto [xp, pq] = editpool_cost_for_units_n(row.field, units, n_classes);
		tot_xp += xp;
		tot_pq += pq;
		std::snprintf(buf, sizeof(buf), "  %-10s %+5d: $c0014", row.label, units);
		out += buf;
		out += format_thousands(xp) + "$c0007 xp  oppure  $c0014" +
			   format_thousands(pq) + "$c0007 rune\n\r";
		if(owner != nullptr) {
			const int cap = edit_pool_field_cap(row.field);
			int cur = 0;
			switch(row.field) {
			case EditPoolField::Hp:
				cur = owner->edit_pool.edit_hp;
				break;
			case EditPoolField::Mana:
				cur = owner->edit_pool.edit_mana;
				break;
			case EditPoolField::Move:
				cur = owner->edit_pool.edit_move;
				break;
			case EditPoolField::HpRegen:
				cur = owner->edit_pool.edit_hp_regen;
				break;
			case EditPoolField::ManaRegen:
				cur = owner->edit_pool.edit_mana_regen;
				break;
			case EditPoolField::MoveRegen:
				cur = owner->edit_pool.edit_move_regen;
				break;
			}
			const int room = std::max(0, cap - cur);
			if(units > room) {
				std::snprintf(buf, sizeof(buf),
							  "  $c0011Cap$c0007: listino %s a %d/%d, %d unita' oltre il "
							  "cap (finiscono in over, nessun bonus).\n\r",
							  row.label, cur, cap, units - room);
				out += buf;
			}
		}
	}
	out += "  Totale: $c0014" + format_thousands(tot_xp) + "$c0007 xp  oppure  $c0014" +
		   format_thousands(tot_pq) + "$c0007 rune\n\r";
	if(owner != nullptr && GetMaxLevel(owner) >= DIO_MINORE) {
		out += "$c0010Nota$c0007: owner livello >= 53, edit listino gratis.\n\r";
	}
}

/**
 * Costo delle sole modifiche di questa sessione (snapshot -> ora): unita'
 * pool = somma APPLY attuale - somma APPLY allo snapshot (polvere uguale,
 * si annulla); listino come stat con lo snapshot come baseline.
 * La baseline e' un obj_data temporaneo sullo stack: niente object_list,
 * niente stringhe, niente da liberare.
 */
std::string ObjEditSessionCost(const ObjEditSnapshot& snap, struct obj_data* obj) {
	std::string out;
	if(procarea_obj_is_reward(obj)) {
		out += "Costo sessione: premio procarea, escluso dall'edit pool.\n\r";
		return out;
	}

	struct obj_data before {};
	before.item_number = -1;
	before.in_room = NOWHERE;
	before.obj_flags = snap.flags;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		before.affected[i] = snap.affected[i];
	}
	before.dust_hp = obj->dust_hp;
	before.dust_mana = obj->dust_mana;
	before.dust_move = obj->dust_move;
	before.dust_hp_regen = obj->dust_hp_regen;
	before.dust_mana_regen = obj->dust_mana_regen;
	before.dust_move_regen = obj->dust_move_regen;
	before.dust_spellfail = obj->dust_spellfail;

	struct char_edit_pool_data now_u {};
	struct char_edit_pool_data before_u {};
	edit_pool_accumulate_obj_delta(obj, nullptr, &now_u);
	edit_pool_accumulate_obj_delta(&before, nullptr, &before_u);
	struct char_edit_pool_data delta {};
	delta.edit_hp = static_cast<sh_int>(now_u.edit_hp - before_u.edit_hp);
	delta.edit_mana = static_cast<sh_int>(now_u.edit_mana - before_u.edit_mana);
	delta.edit_move = static_cast<sh_int>(now_u.edit_move - before_u.edit_move);
	delta.edit_hp_regen = static_cast<sh_int>(now_u.edit_hp_regen - before_u.edit_hp_regen);
	delta.edit_mana_regen =
		static_cast<sh_int>(now_u.edit_mana_regen - before_u.edit_mana_regen);
	delta.edit_move_regen =
		static_cast<sh_int>(now_u.edit_move_regen - before_u.edit_move_regen);

	bool pool_any = false;
	for(const auto& row : kObjPoolCostRows) {
		if(ObjPoolUnits(delta, row.field) != 0) {
			pool_any = true;
		}
	}

	/* Listino e owner (classi): snapshot come baseline, ITEM2_EDIT neutro. */
	const auto saved_flags2 = obj->obj_flags.extra_flags2;
	if(!IS_SET(before.obj_flags.extra_flags2, ITEM2_EDIT)) {
		REMOVE_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
	}
	const ObjEditAnalysis sess = AnalyzeObjEditAgainst(obj, &before, false);
	obj->obj_flags.extra_flags2 = saved_flags2;

	const bool listino_any = sess.diff.valore > 0 || sess.diff.rune > 0;
	if(!pool_any && !listino_any) {
		out += "Costo sessione: 0.\n\r";
		return out;
	}
	ObjAppendListinoLine(out, sess.diff);
	if(pool_any) {
		ObjAppendPoolCost(out, delta, sess.owner_name, sess.owner_classes,
						  "Costo sessione edit pool", "in meno, nessun rimborso");
	}
	else {
		out += "Costo sessione edit pool: 0.\n\r";
	}
	return out;
}

} // namespace

void oedit_snapshot_clear(struct char_data* ch) {
	if(ch != nullptr) {
		g_oedit_snapshots.erase(ch);
	}
}

/**
 * Modifiche dell'oggetto in oedit rispetto al prototipo e costo che gli APPLY
 * hp/mana/move/regen avrebbero sull'edit pool del PG (stesso listino di
 * editpool). Usato dal menu 13 e all'uscita da oedit.
 */
static void SendObjEditDiffReportBody(struct char_data* ch, struct obj_data* obj) {
	if(ch == nullptr || obj == nullptr) {
		return;
	}
	{
		/* oedit tiene solo il puntatore: se nel frattempo l'oggetto e' stato
		 * estratto (purge, decay) non va toccato. */
		const struct obj_data* o = object_list;
		while(o != nullptr && o != obj) {
			o = o->next;
		}
		if(o == nullptr) {
			send_to_char("L'oggetto non esiste piu': niente confronto.\n\r", ch);
			return;
		}
	}
	{
		/* Sessione: snapshot preso da do_oedit su questo stesso oggetto. */
		const auto it = g_oedit_snapshots.find(ch);
		if(it == g_oedit_snapshots.end() || it->second.obj != obj) {
			send_to_char("$c0005Modifiche in questa sessione$c0007: snapshot non "
						 "disponibile.\n\r",
						 ch);
		}
		else {
			const std::string session = ObjEditSessionDiff(it->second, obj);
			if(session.empty()) {
				send_to_char("Nessuna modifica in questa sessione.\n\r", ch);
			}
			else {
				const std::string sec =
					"$c0005Modifiche in questa sessione$c0007:$c0014\n\r" + session +
					"$c0007";
				send_to_char(sec.c_str(), ch);
			}
			send_to_char(ObjEditSessionCost(it->second, obj).c_str(), ch);
		}
	}
	char buf[512];
	std::string out;

	if(procarea_obj_is_reward(obj)) {
		out += "$c0005Premio procarea$c0007: escluso dall'edit pool, confronto con "
			   "la baseline di creazione.\n\r";
		if(obj->db_instance_id == 0) {
			out += "Nessuna baseline: salva prima con 'osave <obj> db procarea'.\n\r";
		}
		else {
			const ObjEditAnalysis staff = AnalyzeProcareaStaffEdit(obj);
			if(staff.changes.empty()) {
				out += "L'oggetto e' identico alla baseline di creazione.\n\r";
			}
			else {
				out += "$c0005Modifiche vs baseline$c0007:$c0014\n\r";
				out += staff.changes;
				out += "$c0007";
			}
		}
		send_to_char(out.c_str(), ch);
		return;
	}

	struct obj_data* proto = edit_pool_load_proto_tmp(obj);
	ObjEditAnalysis edit;
	struct char_edit_pool_data pool_delta {};
	std::string text_changes;
	int proto_vnum = 0;

	if(proto != nullptr) {
		proto_vnum = (proto->item_number >= 0) ? obj_index[proto->item_number].iVNum : 0;
		/* ITEM2_EDIT lo mette oedit stesso all'ingresso: non e' una modifica. */
		const auto saved_flags2 = obj->obj_flags.extra_flags2;
		if(!IS_SET(proto->obj_flags.extra_flags2, ITEM2_EDIT)) {
			REMOVE_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
		}
		edit = AnalyzeObjEditAgainst(obj, proto, false);
		obj->obj_flags.extra_flags2 = saved_flags2;
		/* peso/costi/value con le stesse etichette della sezione sessione */
		edit.changes = ObjStripScalarLines(edit.changes);
		ObjAppendScalarDiff(text_changes, proto->obj_flags, obj->obj_flags);

		if(ObjStrDiffers(obj->name, proto->name)) {
			text_changes += std::string("  name: ") + (proto->name ? proto->name : "") +
							" -> " + (obj->name ? obj->name : "") + "\n\r";
		}
		if(ObjStrDiffers(obj->short_description, proto->short_description)) {
			text_changes += std::string("  short: ") +
							(proto->short_description ? proto->short_description : "") +
							" -> " +
							(obj->short_description ? obj->short_description : "") +
							"\n\r";
		}
		if(ObjStrDiffers(obj->description, proto->description)) {
			text_changes += "  description cambiata\n\r";
		}
	}
	edit_pool_accumulate_obj_delta(obj, proto, &pool_delta);
	if(proto != nullptr) {
		extract_obj(proto);
		proto = nullptr;
	}

	bool pool_any = false;
	for(const auto& row : kObjPoolCostRows) {
		if(ObjPoolUnits(pool_delta, row.field) != 0) {
			pool_any = true;
		}
	}

	if(proto_vnum <= 0) {
		const int cur_vnum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : -1;
		snprintf(buf, sizeof(buf),
				 "$c0011Prototipo non disponibile$c0007 (vnum %d, char_vnum %d): "
				 "niente confronto.\n\r",
				 cur_vnum, obj->char_vnum);
		out += buf;
		if(pool_any) {
			out += "Edit pool: tutti gli APPLY hp/mana/move/regen contano come edit.\n\r";
		}
	}
	else if(edit.changes.empty() && text_changes.empty() && !pool_any) {
		snprintf(buf, sizeof(buf),
				 "L'oggetto e' identico al prototipo (vnum $c0014%d$c0007).\n\r",
				 proto_vnum);
		send_to_char(buf, ch);
		return;
	}
	else {
		snprintf(buf, sizeof(buf),
				 "$c0005Modifiche vs prototipo$c0007 (vnum $c0014%d$c0007):$c0014\n\r",
				 proto_vnum);
		out += buf;
		out += text_changes;
		out += edit.changes;
		out += "$c0007";
		ObjAppendListinoLine(out, edit.diff);
	}

	if(!pool_any) {
		out += "Edit pool: nessuna modifica a hp/mana/move/regen, costo 0.\n\r";
		send_to_char(out.c_str(), ch);
		return;
	}

	ObjAppendPoolCost(out, pool_delta, edit.owner_name, edit.owner_classes,
					  "Costo edit pool", "sotto il prototipo, nessun rimborso");
	out += "Nota: hp/mana/move/regen stanno sul PG; per addebitare usa "
		   "editpool <nome> <campo> <n>.\n\r";
	send_to_char(out.c_str(), ch);
}

/* Report + 4 righe vuote finali (qualunque ramo), per staccarlo dalla
 * descrizione della stanza che segue all'uscita da oedit. Solo display 0/1:
 * in display 2 la stanza sta nel pannello in alto, non nello scroll. */
static void SendObjEditDiffReport(struct char_data* ch, struct obj_data* obj) {
	if(ch == nullptr) {
		return;
	}
	SendObjEditDiffReportBody(ch, obj);
	if(!vt_pane_active(ch)) {
		send_to_char("\n\r\n\r\n\r\n\r", ch);
	}
}


ACTION_FUNC(do_oedit) {
	char name[MAX_INPUT_LENGTH];
	struct obj_data* obj;
	int i;

	if(IS_NPC(ch)) {
		return;
	}

	if((IS_NPC(ch)) || (GetMaxLevel(ch)<DIO)) {
		return;
	}

	if(!ch->desc) { /* someone is forced to do something. can be bad! */
		return;    /* the ch->desc->str field will cause problems... */
	}

	if(GetMaxLevel(ch) < DIO &&
			!IS_SET(ch->player.user_flags,CAN_OBJ_EDIT)) {
		send_to_char("Mi dispiace, ma non hai l'autorizzazione per modificare gli oggetti.\n\r",ch);
		return;
	}

	for(i = 0; *(arg + i) == ' '; i++);
	if(!*(arg + i)) {
		send_to_char("Quale oggetto vuoi modificare?\n\r", ch);
		return;
	}

	arg = one_argument(arg, name);

	if(!(obj = (struct obj_data*)get_obj_vis_accessible(ch, name)))         {
		send_to_char("Questo oggetto non e' qui!\n\r",ch);
		return;
	}

	{
		if(procarea_obj_is_reward(obj) && obj->db_instance_id == 0) {
			send_to_char(
				"Premio procarea: salva prima con 'osave <obj> db procarea'\n\r"
				"(o 'osave <obj> db <651xx>') per conservare i bonus rolled.\n\r",
				ch);
		}
	}

#if 0
	if(obj_index[obj->item_number].data == NULL) {
		read_object_to_memory(obj_index[obj->item_number].iVNum);
	}

	ch->specials.objedit=obj_index[obj->item_number].data;
#else
	ch->specials.objedit=obj;
#endif
	/* Prima di ITEM2_EDIT: stato di partenza per "Modifiche in questa sessione". */
	ObjEditSnapshotTake(ch, obj);

	ch->specials.oedit = OBJ_MAIN_MENU;
	SET_STATE(ch->desc, CON_OBJ_EDITING);

    if(!IS_SET(ch->specials.objedit->obj_flags.extra_flags2, ITEM2_EDIT))
    {
        SET_BIT(ch->specials.objedit->obj_flags.extra_flags2, ITEM2_EDIT);
    }

	act("$n inizia a $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch)=POSITION_SLEEPING;

	/*    if(GetMaxLevel(ch)<ROLORD)
	      ch->specials.objedit->mortal_can_use=FALSE;
	      */

	UpdateObjMenu(ch);
}


void UpdateObjMenu(struct char_data* ch) {
	char buf[255];
	struct obj_data* obj;

	obj = ch->specials.objedit;

	EditorClear(ch);
	EditorGoto(ch, 1, 1);
	sprintf(buf, "Object Name: %s", obj->name);
	send_to_char(buf, ch);
    if(ch->term != VT100)
    {
        send_to_char("\n\r", ch);
    }
	EditorGoto(ch, 3, 1);
	send_to_char("\n\rMenu:\n\r", ch);
	send_to_char(obj_edit_menu, ch);
	EditorPrompt(ch, "--> ");
}


void ObjEdit(struct char_data* ch, const char* arg) {
	if(ch->specials.oedit == OBJ_MAIN_MENU) {
		if(!*arg || *arg == '\n') {
			SET_STATE(ch->desc, CON_PLYNG);
			act("$n smette di $c0009p$c0010l$c0011a$c0012$c0013s$c0014m$c0009a$c0010r$c0011e$c0007 la materia.", FALSE, ch, 0, 0, TO_ROOM);
			GET_POS(ch)=POSITION_STANDING;
			send_to_char("\n\r", ch);
			SendObjEditDiffReport(ch, ch->specials.objedit);
			oedit_snapshot_clear(ch);
			return;
		}
		switch(atoi(arg)) {
		case 0:
			UpdateObjMenu(ch);
			return;
		case 1:
			ch->specials.oedit = CHANGE_OBJ_NAME;
			ChangeObjName(ch, "", ENTER_CHECK);
			return;
		case 2:
			ch->specials.oedit = CHANGE_OBJ_SHORT;
			ChangeObjShort(ch, "", ENTER_CHECK);
			return;
		case 3:
			ch->specials.oedit = CHANGE_OBJ_DESC;
			ChangeObjDesc(ch, "", ENTER_CHECK);
			return;
		case 4:
			ch->specials.oedit = CHANGE_OBJ_TYPE;
			ChangeObjType(ch, "", ENTER_CHECK);
			return;
		case 5:
			ch->specials.oedit = CHANGE_OBJ_WEAR;
			ChangeObjWear(ch, "", ENTER_CHECK);
			return;
		case 6:
			ch->specials.oedit = CHANGE_OBJ_FLAGS;
			ChangeObjFlags(ch, "", ENTER_CHECK);
			return;
		case 7:
			ch->specials.oedit = CHANGE_OBJ_WEIGHT;
			ChangeObjWeight(ch, "", ENTER_CHECK);
			return;
		case 8:
			ch->specials.oedit = CHANGE_OBJ_VALUE;
			ChangeObjPrice(ch, "", ENTER_CHECK);
			return;
		case 9:
			ch->specials.oedit = CHANGE_OBJ_COST;
			ChangeObjCost(ch, "", ENTER_CHECK);
			return;
		case 10:
			ch->specials.oedit = CHANGE_OBJ_AFFECTS;
			ChangeObjAffects(ch, "", ENTER_CHECK);
			return;
		case 11:
			ch->specials.oedit = CHANGE_OBJ_VALUES;
			ChangeObjValues(ch, "", ENTER_CHECK);
			return;
        case 12:
            ch->specials.oedit = CHANGE_OBJ_SPECIAL;
            ChangeObjSpecial(ch, "", ENTER_CHECK);
            return;
		case 13:
			EditorClear(ch);
			SendObjEditDiffReport(ch, ch->specials.objedit);
			ch->specials.oedit = OBJ_HIT_RETURN;
			ObjHitReturn(ch, "", ENTER_CHECK);
			return;
		default:
			UpdateObjMenu(ch);
			return;
		}
	}

	switch(ch->specials.oedit) {
	case CHANGE_OBJ_NAME:
		ChangeObjName(ch, arg, 0);
		return;
	case CHANGE_OBJ_SHORT:
		ChangeObjShort(ch, arg, 0);
		return;
	case CHANGE_OBJ_DESC:
		ChangeObjDesc(ch, arg, 0);
		return;
	case CHANGE_OBJ_WEAR:
		ChangeObjWear(ch, arg, 0);
		return;
	case CHANGE_OBJ_TYPE:
    case CHANGE_OBJ_TYPE2:
		ChangeObjType(ch, arg, 0);
		return;
	case CHANGE_OBJ_FLAGS:
		ChangeObjFlags(ch, arg, 0);
		return;
	case CHANGE_OBJ_WEIGHT:
		ChangeObjWeight(ch, arg, 0);
		return;
	case CHANGE_OBJ_VALUE:
		ChangeObjPrice(ch, arg, 0);
		return;
	case CHANGE_OBJ_COST:
		ChangeObjCost(ch, arg, 0);
		return;
	case CHANGE_OBJ_AFFECTS:
		ChangeObjAffects(ch, arg, 0);
		return;
	case CHANGE_OBJ_AFFECT1:
	case CHANGE_OBJ_AFFECT2:
	case CHANGE_OBJ_AFFECT3:
	case CHANGE_OBJ_AFFECT4:
	case CHANGE_OBJ_AFFECT5:
		ChangeObjAffect(ch, arg, 0);
		return;
	case CHANGE_AFFECT1_MOD:
	case CHANGE_AFFECT2_MOD:
	case CHANGE_AFFECT3_MOD:
	case CHANGE_AFFECT4_MOD:
	case CHANGE_AFFECT5_MOD:
		ChangeAffectMod(ch,arg,0);
		return;
	case CHANGE_OBJ_VALUES:
		ChangeObjValues(ch, arg, 0);
		return;
	case CHANGE_OBJ_VALUE1:
	case CHANGE_OBJ_VALUE2:
	case CHANGE_OBJ_VALUE3:
	case CHANGE_OBJ_VALUE4:
		ChangeObjValue(ch, arg, 0);
		return;
	case OBJ_HIT_RETURN:
		ObjHitReturn(ch, arg, 0);
		return;
    case CHANGE_OBJ_SPECIAL:
        ChangeObjSpecial(ch, arg, 0);
        return;
	default:
		mudlog(LOG_ERROR, "Got to bad spot in ObjEdit");
		return;
	}
}

void ChangeObjSpecial(struct char_data* ch, const char* arg, int type) {
	char buf[256], proc[256], parms[256];
	struct obj_data* obj;
	struct OtherSpecialProcEntry* op;
	int i;
	int lastotherproc = 0;

	if(type != ENTER_CHECK)
	{
		if(!*arg || (*arg == '\n'))
		{
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}
	}

	obj = ch->specials.objedit;
	if(type != ENTER_CHECK)
	{
		arg = one_argument(arg, proc);
		only_argument(arg, parms);

		if(strstr(proc, "nessuna") || strstr(proc, "none"))
		{
			obj_index[obj->item_number].func         = NULL;
			obj_index[obj->item_number].specname     = NULL;
			obj_index[obj->item_number].specparms    = NULL;
		}
		else
		{
			for(i = 0; strcmp(otherproc[i].nome, "zFineprocedure"); i++)
			{
				lastotherproc++;
			}

			if(!(op = (struct OtherSpecialProcEntry*) bsearch(&proc, otherproc, lastotherproc, sizeof(struct OtherSpecialProcEntry), nomecompare)))
			{
				mudlog(LOG_ERROR, "obj_assign: Obj %d not found in database.", obj_index[obj->item_number].iVNum);
			}
			else
			{
				obj_index[obj->item_number].func         = op->proc;
				obj_index[obj->item_number].specname     = op->nome;
				obj_index[obj->item_number].specparms    = strdup(parms);
			}
		}

		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Obj Special: %s %s", 	obj_index[obj->item_number].specname ?
												obj_index[obj->item_number].specname :
												"nothing",
												obj_index[obj->item_number].specparms ?
												obj_index[obj->item_number].specparms :
												"");
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Obj Special (name, parameters), type 'none' for no-special: ");

	return;
}

void ChangeObjName(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		if(obj->name) {
			free(obj->name);
		}
		obj->name = (char*)strdup(arg);
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Name: %s", obj->name);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Name: ");

	return;
}

void ChangeObjShort(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		if(obj->short_description) {
			free(obj->short_description);
		}
		obj->short_description = (char*)strdup(arg);
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Short Description: %s", obj->short_description);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Short Description: ");

	return;
}

void ChangeObjDesc(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		if(obj->description) {
			free(obj->description);
		}
		obj->description = (char*)strdup(arg);
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Description: %s", obj->description);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Description: ");

	return;
}


void ChangeObjType(struct char_data* ch, const char* arg, int type) {
	int i, row, update;
	char buf[255];
	struct obj_data* obj;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	update = atoi(arg);
	update--;
	obj = ch->specials.objedit;

	if(type != ENTER_CHECK)
	{
		switch(ch->specials.oedit)
		{
			case CHANGE_OBJ_TYPE:
				if(update < 0 || update > E_ITEM_TYPE_MAX)
				{
					return;
				}
				else
				{
					ch->specials.objedit->obj_flags.type_flag = update;
					if(ch->specials.objedit->obj_flags.type_flag == ITEM_AUDIO)
					{
						send_to_char("\n\rCurrent Sound:\n\r", ch);
						send_to_char(ch->specials.objedit->action_description, ch);
						send_to_char("\n\r\n\rNew Sound:\n\r", ch);
						ch->specials.oedit = CHANGE_OBJ_TYPE2;
					}
					else
					{
						ch->specials.oedit = OBJ_MAIN_MENU;
						UpdateObjMenu(ch);
					}
					return;
				}
			case CHANGE_OBJ_TYPE2:
				if(type != ENTER_CHECK)
				{
					if(obj->action_description)
					{
						free(obj->action_description);
					}
				}
				obj->action_description = (char*)strdup(arg);
				send_to_char("\n\r\n\rDone, sound assigned.\n\rRemember to change Object Value1 to a number greater than 0.\n\r", ch);
				ch->specials.oedit = OBJ_MAIN_MENU;
				UpdateObjMenu(ch);
				return;
		}
	}

	if(ch->term == VT100)
	{
		EditorClear(ch);
		sprintf(buf, "Object Type: %s", item_types[(int)(ch->specials.objedit->obj_flags.type_flag) ]);
		send_to_char(buf, ch);

		row = 0;
		for(i = 0; i < E_ITEM_TYPE_COUNT; i++)
		{
			EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
			if(i & 1)
			{
				row++;
			}

			sprintf(buf, "%-2d %s", i + 1, item_types[i]);
			send_to_char(buf, ch);
		}

		EditorGoto(ch, 20, 1);
		EditorPrompt(ch, "Select the number to set to, <C/R> to return to main menu.\n\r--> ");
	}
	else
	{
		string sb;
		boost::format fmt ("     %-2d %s");
		char buf2[255];
		int x = 0;

		sprintf(buf, "\n\rObject Type: %s\n\r\n\r", item_types[(int)(ch->specials.objedit->obj_flags.type_flag) ]);
		send_to_char(buf, ch);

		for(i = 0; i < E_ITEM_TYPE_COUNT; i++)
		{
			snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
			strcat(buf2, "s%-2d %s\n\r");
			boost::format fmt2 (buf2);

			if(i & 1)
			{
				fmt2 % "" % (i + 1) % item_types[i];
				sb.append(fmt2.str().c_str());
			}
			else
			{
				fmt % (i + 1) % item_types[i];
				sb.append(fmt.str().c_str());
				x = strlen(fmt.str().c_str());
			}

			fmt.clear();
			fmt2.clear();
		}

		sb.append("\n\r");
		page_string(ch->desc, sb.c_str(), true);
		EditorPrompt(ch, "Select the number to set to, <C/R> to return to main menu.\n\r--> ");
	}
}

void ChangeObjWeight(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;
	long change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<0) {
			change=0;
		}
		obj->obj_flags.weight = change;
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Weight: %d", obj->obj_flags.weight);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Weight: ");

	return;
}


void ChangeObjCost(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;
	long change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<0) {
			change=0;
		}
		obj->obj_flags.cost_per_day = change;
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Rental Cost Per Day: %d", obj->obj_flags.cost_per_day);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Rental Cost Per Day: ");

	return;
}

void ObjHitReturn(struct char_data* ch, const char* arg, int type) {

	if(type != ENTER_CHECK) {
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorPrompt(ch, "\n\rHit return: ");

	return;
}


void ChangeObjPrice(struct char_data* ch, const char* arg, int type) {
	char buf[255];
	struct obj_data* obj;
	long change;

	if(type != ENTER_CHECK)
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

	obj=ch->specials.objedit;
	if(type != ENTER_CHECK) {
		change=atoi(arg);
		if(change<0) {
			change=0;
		}
		obj->obj_flags.cost = change;
		ch->specials.oedit = OBJ_MAIN_MENU;
		UpdateObjMenu(ch);
		return;
	}

	EditorClear(ch);

	sprintf(buf, "Current Object Value: %d", obj->obj_flags.cost);
	send_to_char(buf, ch);
	EditorPrompt(ch, "\n\r\n\rNew Object Value: ");

	return;
}

void ChangeObjAffects(struct char_data* ch, const char* arg, int type) {
	int update;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

		update = atoi(arg);
		if(update == 0) {
			ChangeObjAffects(ch, "", ENTER_CHECK);
			return;
		}

		switch(update) {
		case 1:
			ch->specials.oedit = CHANGE_OBJ_AFFECT1;
			ChangeObjAffect(ch, "", ENTER_CHECK);
			return;
			break;
		case 2:
			ch->specials.oedit = CHANGE_OBJ_AFFECT2;
			ChangeObjAffect(ch, "", ENTER_CHECK);
			return;
			break;
		case 3:
			ch->specials.oedit = CHANGE_OBJ_AFFECT3;
			ChangeObjAffect(ch, "", ENTER_CHECK);
			return;
			break;
		case 4:
			ch->specials.oedit = CHANGE_OBJ_AFFECT4;
			ChangeObjAffect(ch, "", ENTER_CHECK);
			return;
			break;
		case 5:
			ch->specials.oedit = CHANGE_OBJ_AFFECT5;
			ChangeObjAffect(ch, "", ENTER_CHECK);
			return;
			break;
		default:
			ChangeObjAffects(ch, "", ENTER_CHECK);
			return;
		}

	}


	EditorClear(ch);
	SendObjAffectSlotList(ch);
	EditorPrompt(ch, "\n\rChange object affect #(1-5) --> ");
	return;
}

void ChangeObjAffect(struct char_data* ch, const char* arg, int type) {
	int update,affect = 1, row = 0, i, a = 0, column = 0;
	unsigned check;
	char buf[1024];

	switch(ch->specials.oedit) {
	case CHANGE_OBJ_AFFECT1:
		affect = 1;
		break;
	case CHANGE_OBJ_AFFECT2:
		affect = 2;
		break;
	case CHANGE_OBJ_AFFECT3:
		affect = 3;
		break;
	case CHANGE_OBJ_AFFECT4:
		affect = 4;
		break;
	case CHANGE_OBJ_AFFECT5:
		affect = 5;
		break;
	}

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

		update = atoi(arg) - 1;

		if(update < 0 || update >= APPLY_SKIP - 9)
		{
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

		if(update > APPLY_MOD_DRUNK)
		{
			update += 9;
		}
		ch->specials.objedit->affected[affect-1].location=update;
		ch->specials.objedit->affected[affect-1].modifier=0;

		switch(affect)
		{
			case 1:
				ch->specials.oedit = CHANGE_AFFECT1_MOD;
				break;

			case 2:
				ch->specials.oedit = CHANGE_AFFECT2_MOD;
				break;

			case 3:
				ch->specials.oedit = CHANGE_AFFECT3_MOD;
				break;

			case 4:
				ch->specials.oedit = CHANGE_AFFECT4_MOD;
				break;

			case 5:
				ch->specials.oedit = CHANGE_AFFECT5_MOD;
				break;
		}
		EditorClear(ch);
		switch(update)
		{
			case APPLY_NONE:
			case APPLY_SKIP:
				break;

			case APPLY_AFF2:
				send_to_char("\n\rNote: Modifier should be ADDED together from this "
								"list of affection flags 2.\n\r", ch);
				if(ch->term == VT100)
				{
					row = 0;
					for(i = 0; *affected_bits2[i] != '\n'; i++)
					{
						EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
						if(i & 1)
						{
							row++;
						}

						check = 1;
						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}
						sprintf(buf, "%-10u : %s", check, affected_bits2[i]);
						send_to_char(buf, ch);
					}
					send_to_char("\n\r\n\r", ch);
				}
				else
				{
					string sb;
					boost::format fmt ("    %-10u : %s");
					char buf2[255];
					int x = 0, column = 0;

					send_to_char("\n\r", ch);

					for(i = 0; *affected_bits2[i] != '\n'; i++)
					{
						check = 1;
						snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
						strcat(buf2, "s%-10u : %s\n\r");
						boost::format fmt2 (buf2);

						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}

						if(column & 1)
						{
							fmt2 % "" % check % affected_bits2[i];
							sb.append(fmt2.str().c_str());
							column += 1;
						}
						else
						{
							fmt % check % affected_bits2[i];
							sb.append(fmt.str().c_str());
							x = strlen(fmt.str().c_str());
							column += 1;
						}

						fmt.clear();
						fmt2.clear();
					}
					page_string(ch->desc, sb.c_str(), true);
					send_to_char("\n\r\n\r", ch);
				}
				break;

			case APPLY_STR:
			case APPLY_DEX:
			case APPLY_INT:
			case APPLY_WIS:
			case APPLY_CON:
			case APPLY_CHR:
			case APPLY_ATTACKS:
			case APPLY_LEVEL:
			case APPLY_AGE:
			case APPLY_CHAR_WEIGHT:
			case APPLY_CHAR_HEIGHT:
			case APPLY_MANA:
			case APPLY_HIT:
			case APPLY_MOVE:
			case APPLY_GOLD:
			case APPLY_EXP:
			case APPLY_HITROLL:
			case APPLY_DAMROLL:
			case APPLY_HITNDAM:
			case APPLY_SPELLPOWER:
			case APPLY_HITNSP:
			case APPLY_SPELLFAIL:
				send_to_char("\n\rNote: Modifier will make field go up modifier number of points.\n\r",ch);
				send_to_char("      Positive modifier will make field go up, negative modifier will make\n\r      field go down.\n\r",ch);
				break;

			case APPLY_SEX:
				send_to_char("\n\rNote: Modifier will change characters sex by adding.\n\r      0=neutral, 1=male, 2=female\n\r",ch);
				break;

			case APPLY_SAVING_PARA:
			case APPLY_SAVING_ROD:
			case APPLY_SAVING_PETRI:
			case APPLY_SAVING_BREATH:
			case APPLY_SAVING_SPELL:
			case APPLY_SAVE_ALL:
			case APPLY_AC:
				send_to_char("\n\rNote: Modifier will make field go up modifier number of points.\n\r",ch);
				send_to_char("      Positive modifier will make field go up, negative modifier will make\n\r      field go down.\n\r",ch);
				send_to_char("      $c0015Negative values are BETTER.$c0007", ch);
			break;

			case APPLY_IMMUNE:
			case APPLY_SUSC:
			case APPLY_M_IMMUNE:
				send_to_char("\n\rNote: Modifier should be ADDED together from this list of immunity flags.\n\r",ch);
				if(ch->term == VT100)
				{
					row = 0;
					for(i = 0; i < 18; i++)
					{
						EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
						if(i & 1)
						{
							row++;
						}

						check = 1;
						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}
						sprintf(buf, "%-6u :   %s", check, immunity_names[i]);
						send_to_char(buf, ch);
					}
					EditorGoto(ch, 20, 1);
				}
				else
				{
					string sb;
					boost::format fmt ("    %-6u : %s");
					char buf2[255];
					int x = 0;

					for(i = 0; i < 18; i++)
					{
						check = 1;
						snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
						strcat(buf2, "s%-6u : %s\n\r");
						boost::format fmt2 (buf2);

						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}

						if(i & 1)
						{
							fmt2 % "" % check % immunity_names[i];
							sb.append(fmt2.str().c_str());
						}
						else
						{
							fmt % check % immunity_names[i];
							sb.append(fmt.str().c_str());
							x = strlen(fmt.str().c_str());
						}

						fmt.clear();
						fmt2.clear();
					}
					sb.append("\n\r");
					page_string(ch->desc, sb.c_str(), true);
				}
				break;

			case APPLY_SPELL:
				send_to_char("\n\rNote: Modifier should be ADDED together from this list of affection flags.\n\r",ch);
				if(ch->term == VT100)
				{
					row = 0;
					for(i = 0; *affected_bits[i] != '\n'; i++)
					{
						EditorGoto(ch, row + 4, ((i & 1) ? 45 : 5));
						if(i & 1)
						{
							row++;
						}

						check = 1;
						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}
						sprintf(buf, "%-10u : %s", check, affected_bits2[i]);
						send_to_char(buf, ch);
					}
					send_to_char("\n\r\n\r", ch);
				}
				else
				{
					string sb;
					boost::format fmt ("    %-10u : %s");
					char buf2[255];
					int x = 0, column = 0;

					send_to_char("\n\r", ch);

					for(i = 0; *affected_bits[i] != '\n'; i++)
					{
						check = 1;
						snprintf(buf2, sizeof(buf2), "%%-%d", 45-x);
						strcat(buf2, "s%-10u : %s\n\r");
						boost::format fmt2 (buf2);

						if(i > 0)
						{
							for(a = 1; a <= i; a++)
							{
								check *= 2;
							}
						}

						if(column & 1)
						{
							fmt2 % "" % check % affected_bits[i];
							sb.append(fmt2.str().c_str());
							column += 1;
						}
						else
						{
							fmt % check % affected_bits[i];
							sb.append(fmt.str().c_str());
							x = strlen(fmt.str().c_str());
							column += 1;
						}

						fmt.clear();
						fmt2.clear();
					}
					page_string(ch->desc, sb.c_str(), true);
					send_to_char("\n\r\n\r", ch);
				}
				break;

			case APPLY_WEAPON_SPELL:
			case APPLY_EAT_SPELL:
				send_to_char("\n\rNote: Modifier will be a spell # which you can get from the allspells.\n\r",ch);
				break;

			case APPLY_BACKSTAB:
			case APPLY_KICK:
			case APPLY_SNEAK:
			case APPLY_HIDE:
			case APPLY_BASH:
			case APPLY_PICK:
			case APPLY_STEAL:
			case APPLY_TRACK:
			case APPLY_FIND_TRAPS:
			case APPLY_RIDE:
				send_to_char("\n\rNote: Modifier will affect % learned of skill.\n\r",ch);
				send_to_char("      Positive values will increase the % learned making the char less likely\n\r      to fail while negative numbers will do the opposite.\n\r",ch);
				break;

			case APPLY_HASTE:
				send_to_char("\n\rNota: Valori positivi = num. attacchi guadagnati, i negativi saranno ignorati.\n\r", ch);
				break;

			case APPLY_SLOW:
				send_to_char("\n\rNote: Positive modifier will halve attacks number, negative will double.\n\r", ch);
				break;

			case APPLY_RACE_SLAYER:
				send_to_char("\n\rNote: Modifier is the race number (see help races)\n\r", ch);
				break;

			case APPLY_ALIGN_SLAYER:
				send_to_char("\n\rNote: Modifier should be ADDED together from this list of align flags.\n\r",ch);
				send_to_char("\n\r 1 = GOOD  2 = NEUTRAL  4 = EVIL\n\r.", ch);
				break;

			case APPLY_MANA_REGEN:
			case APPLY_HIT_REGEN:
			case APPLY_MOVE_REGEN:
				send_to_char("\n\rNote: Modifier will add the point regained from char affected by object.\n\r", ch);
				break;

			case APPLY_MOD_THIRST:
			case APPLY_MOD_HUNGER:
			case APPLY_MOD_DRUNK:
				send_to_char("\n\rNote: Set thirst/hunger/drunk of char to modifier.\n\r", ch);
				break;
		}

		{
			char prompt[256];
			std::snprintf(prompt, sizeof(prompt),
						  "\n\r\n\rAffect #%d %s - Enter new Modifier (return for 0): ",
						  affect, ObjApplyTypeName(update));
			EditorPrompt(ch, prompt);
		}
		ChangeAffectMod(ch,"",ENTER_CHECK);
		return;
	}

	if(ch->term == VT100)
	{
		EditorClear(ch);

		for(i = 0; i < APPLY_SKIP; i++)
		{
			if(i >= APPLY_T_STR && i <= APPLY_T_MANA)
			{
				continue;
			}
			a++;
			if(a == 1)
			{
				column=5;
			}
			else if(a == 2)
			{
				column = 30;
			}
			else if(a == 3)
			{
				column = 55;
			}
			EditorGoto(ch, row + 1, column);
			if(a == 3)
			{
				row++;
				a = 0;
			}

			if(i > APPLY_T_MANA)
			{
				sprintf(buf, "%-2d %s", i - 8, apply_types[i]);
			}
			else
			{
				sprintf(buf, "%-2d %s", i + 1, apply_types[i]);
			}
			send_to_char(buf, ch);
		}

		EditorGoto(ch, 21, 1);
	}
	else
	{
		string sb;
		char buf2[255];
		boost::format fmt ("     %-2d %s");
		int x = 0;

		send_to_char("\n\r", ch);

		for(i = 0; i < APPLY_SKIP; i++)
		{
			if(i >= APPLY_T_STR && i <= APPLY_T_MANA)
			{
				continue;
			}
			a++;
			if(a == 1)
			{
				boost::format fmt ("     %-2d %s");
				fmt % (i > APPLY_T_MANA ? i - 8 : i + 1) % apply_types[i];
				sb.append(fmt.str().c_str());
				x = strlen(fmt.str().c_str());
			}
			else if(a == 2)
			{
				snprintf(buf2, sizeof(buf2), "%%-%d", 30-x);
				strcat(buf2, "s%-2d %s");
				boost::format fmt2 (buf2);
				fmt2 % "" % (i > APPLY_T_MANA ? i - 8 : i + 1) % apply_types[i];
				sb.append(fmt2.str().c_str());
				x += strlen(fmt2.str().c_str());
				fmt2.clear();
			}
			else if(a == 3)
			{
				snprintf(buf2, sizeof(buf2), "%%-%d", 55-x);
				strcat(buf2, "s%-2d %s\n\r");
				boost::format fmt (buf2);
				fmt % "" % (i > APPLY_T_MANA ? i - 8 : i + 1) % apply_types[i];
				sb.append(fmt.str().c_str());
				x = 0;
				a = 0;
			}

			fmt.clear();
		}

		sb.append("\n\r");
		page_string(ch->desc, sb.c_str(), true);
	}


	{
		char prompt[256];
		char current[64];
		FormatObjAffectSummary(current, sizeof(current),
							   ch->specials.objedit->affected[affect - 1].location,
							   ch->specials.objedit->affected[affect - 1].modifier);
		std::snprintf(prompt, sizeof(prompt),
					  "Affect #%d (current: %s) - Select the apply number or hit enter for the main menu.\n\r--> ",
					  affect, current);
		EditorPrompt(ch, prompt);
	}
}

void ChangeAffectMod(struct char_data* ch, const char* arg, int type) {
	signed long update;
	int affect = 1;
	bool skill=FALSE;

	switch(ch->specials.oedit) {
	case CHANGE_AFFECT1_MOD:
		affect = 1;
		break;
	case CHANGE_AFFECT2_MOD:
		affect = 2;
		break;
	case CHANGE_AFFECT3_MOD:
		affect = 3;
		break;
	case CHANGE_AFFECT4_MOD:
		affect = 4;
		break;
	case CHANGE_AFFECT5_MOD:
		affect = 5;
		break;
	}

	if(type == ENTER_CHECK) {
		return;
	}

	update = atoi(arg);

#if 0
	if(update>32000) {
		update=0;
	}

	if(ch->specials.objedit->affected[affect-1].location== APPLY_WEAPON_SPELL ||
			ch->specials.objedit->affected[affect-1].location== APPLY_EAT_SPELL) {
		if(update>=45 && update<=52) {
			skill=TRUE;
		}
		if(update>=120 && update<=127) {
			skill=TRUE;
		}
		if(update>=129 && update<=163) {
			skill=TRUE;
		}
		if(update>=180 && update<=187) {
			skill=TRUE;
		}
		if(skill==TRUE) {
			send_to_char("You must use a spell, not a skill!\n\r"
						 "Setting modifier to 1 (armor spell).\n\r",ch);
			update=1;
		}
	}
#endif

	ch->specials.objedit->affected[affect-1].modifier=update;

	if(skill==TRUE) {
		ch->specials.oedit =  OBJ_HIT_RETURN;
		ObjHitReturn(ch, "", ENTER_CHECK);
		return;
	}

	ch->specials.oedit =  CHANGE_OBJ_AFFECTS;
	ChangeObjAffects(ch, "", ENTER_CHECK);
	return;
}


void ChangeObjValues(struct char_data* ch, const char* arg, int type) {
	int update;

	if(type != ENTER_CHECK) {
		if(!*arg || (*arg == '\n')) {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

		update = atoi(arg);
		if(update == 0) {
			ChangeObjValues(ch, "", ENTER_CHECK);
			return;
		}

		switch(update) {
		case 1:
			ch->specials.oedit = CHANGE_OBJ_VALUE1;
			ChangeObjValue(ch, "", ENTER_CHECK);
			return;
			break;
		case 2:
			ch->specials.oedit = CHANGE_OBJ_VALUE2;
			ChangeObjValue(ch, "", ENTER_CHECK);
			return;
			break;
		case 3:
			ch->specials.oedit = CHANGE_OBJ_VALUE3;
			ChangeObjValue(ch, "", ENTER_CHECK);
			return;
			break;
		case 4:
			ch->specials.oedit = CHANGE_OBJ_VALUE4;
			ChangeObjValue(ch, "", ENTER_CHECK);
			return;
			break;
		default:
			ChangeObjValues(ch, "", ENTER_CHECK);
			return;
		}

	}


	EditorClear(ch);
	{
		char line[256];
		char decoded[128];
		const int typ = ch->specials.objedit->obj_flags.type_flag;
		for(int i = 0; i < 4; i++) {
			const int val = ch->specials.objedit->obj_flags.value[i];
			const char* label = ObjValueLabel(typ, i);
			FormatObjValueDecoded(decoded, sizeof(decoded), typ, i, val);
			if(label[0] == '\0' && decoded[0] == '\0') {
				std::snprintf(line, sizeof(line), " %d) %d\n\r", i + 1, val);
			}
			else {
				std::snprintf(line, sizeof(line), " %d) %-8d %s%s\n\r",
							  i + 1, val, label, decoded);
			}
			send_to_char(line, ch);
		}
	}
	EditorPrompt(ch, "\n\rChange object value #(1-4) --> ");
	return;
}


void ChangeObjValue(struct char_data* ch, const char* arg, int type) {
	int value = 0;
	long update;
	char buf[1024];
	bool skill=FALSE;

	switch(ch->specials.oedit) {
	case CHANGE_OBJ_VALUE1:
		value = 0;
		break;
	case CHANGE_OBJ_VALUE2:
		value = 1;
		break;
	case CHANGE_OBJ_VALUE3:
		value = 2;
		break;
	case CHANGE_OBJ_VALUE4:
		value = 3;
		break;
	}

	if(type != ENTER_CHECK) {
		if(!*arg || *arg == '\n') {
			ch->specials.oedit = OBJ_MAIN_MENU;
			UpdateObjMenu(ch);
			return;
		}

		update = atoi(arg);

		if(update < 0) {
			ch->specials.oedit=CHANGE_OBJ_VALUES;
			ChangeObjValues(ch,"",ENTER_CHECK);
			return;
		}

		/* Solo gli slot spell: V2-V4 di scroll/potion, V4 di wand/staff (V1 e' il livello). */
		if(((ch->specials.objedit->obj_flags.type_flag == ITEM_SCROLL ||
				ch->specials.objedit->obj_flags.type_flag == ITEM_POTION) &&
				value >= 1) ||
				((ch->specials.objedit->obj_flags.type_flag == ITEM_WAND ||
				  ch->specials.objedit->obj_flags.type_flag == ITEM_STAFF) &&
				 value == 3)) {
			if(update >= 45 && update <= 52) {
				skill=TRUE;
			}
			if(update>=120 && update<=127) {
				skill=TRUE;
			}
			if(update>=129 && update<=163) {
				skill=TRUE;
			}
			if(update>=180 && update<=187) {
				skill=TRUE;
			}
			if(skill==TRUE) {
				send_to_char("You must use a spell number, not a skill!\n\rSetting modifier to 1 (armor spell).\n\r",ch);
				update=1;
			}
		}

		ch->specials.objedit->obj_flags.value[ value ] = update;

		if(skill==TRUE) {
			ch->specials.oedit =  OBJ_HIT_RETURN;
			ObjHitReturn(ch, "", ENTER_CHECK);
			return;
		}

		ch->specials.oedit=CHANGE_OBJ_VALUES;
		ChangeObjValues(ch,"",ENTER_CHECK);
		return;
	}

	EditorClear(ch);
	switch(ch->specials.objedit->obj_flags.type_flag) {
	case ITEM_LIGHT:
		if(value==0) {
			send_to_char("\n\rValue1 is the colour (not used by the code).",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the type (not used by the code).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the number of hours the light source will last.\n\r"
						 "-1 = infinite (no flame, works underwater), 0 = burned out.",ch);
		}
		else {
			send_to_char("\n\rValue is not used for this item type.",ch);
		}
		break;
	case ITEM_SCROLL:
		if(value==0) {
			send_to_char("\n\rValue1 is the level of casting this scroll casts.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the first spell this scroll casts (0 = none).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the second spell this scroll casts (0 = none).",ch);
		}
		else {
			send_to_char("\n\rValue4 is the third spell this scroll casts (0 = none).",ch);
		}
		break;
	case ITEM_WAND:
		if(value==0) {
			send_to_char("\n\rValue1 is the level of casting this wand casts.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the max number of charges (shop price scales with Value3/Value2).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the number of charges the wand has left.",ch);
		}
		else {
			send_to_char("\n\rValue4 is the spell the wand casts.",ch);
		}
		break;
	case ITEM_STAFF:
		if(value==0) {
			send_to_char("\n\rValue1 is the level of casting this staff casts.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the max number of charges (shop price scales with Value3/Value2).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the number of charges the staff has left.",ch);
		}
		else {
			send_to_char("\n\rValue4 is the spell the staff casts.",ch);
		}
		break;
	case ITEM_WEAPON:
		if(value==0) {
			send_to_char("\n\rValue1 is reserved for now.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the number of times the damage dice is rolled.",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the number of sides the damage dice has.",ch);
		}
		else {
			send_to_char("\n\rValue4 is the type of weapon damage.\n\r\n\r",ch);
			send_to_char("1 - stab        2 - whip\n\r",ch);
			send_to_char("3 - slash       4 - smash\n\r",ch);
			send_to_char("5 - cleave      6 - crush\n\r",ch);
			send_to_char("7 - pound       8 - claw\n\r",ch);
			send_to_char("9 - bite       10 - sting\n\r",ch);
			send_to_char("11 - pierce     12 - impale\n\r",ch);
		}
		break;
	case ITEM_FIREWEAPON:
		if(value==0) {
			send_to_char("\n\rValue1 is the strength required to draw.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the max range [in rooms] (not used by the code:\n\r"
						 "range comes from strength, plus Value3).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the bonus range [in rooms].",ch);
		}
		else
			send_to_char("\n\rValue4 is the type of missile "
						 "(must mach the value4 of missile).", ch);
		break;
	case ITEM_MISSILE:
		if(value==0) {
			send_to_char("\n\rValue1 is the % chance that the missile will break.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the number of times the damage die is rolled.",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the number of sides on the damage die.",ch);
		}
		else
			send_to_char("\n\rValue4 is the type of missile\n\r"
						 "(must match the Value4 of the fire weapon).",ch);
		break;
	case ITEM_ARMOR:
		if(value==0) {
			send_to_char("\n\rValue1 is the Armor Class Apply.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the Full Strengh of the armor.",ch);
		}
		else {
			send_to_char("\n\rValue is not used for this item type.",ch);
		}
		break;
	case ITEM_POTION:
		if(value==0) {
			send_to_char("\n\rValue1 is the level of the spells this potion casts.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the first spell this potion casts (0 = none).",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the second spell this potion casts (0 = none).",ch);
		}
		else {
			send_to_char("\n\rValue4 is the third spell this potion casts (0 = none).",ch);
		}
		break;
	case ITEM_TRAP:
		if(value==0) {
			send_to_char("\n\rValue1 is when the trap triggers (add the flags).\n\r\n\r",ch);
			send_to_char("  1 - on movement       2 - on get or put\n\r",ch);
			send_to_char("  4 - hits whole room   8 - north\n\r",ch);
			send_to_char(" 16 - east             32 - south\n\r",ch);
			send_to_char(" 64 - west            128 - up\n\r",ch);
			send_to_char("256 - down\n\r",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the attack type of the trap.\n\r\n\r",ch);
			send_to_char(" 10 - blasted          26 - seared\n\r",ch);
			send_to_char(" 67 - corroded        327 - frozen\n\r",ch);
			send_to_char("331 - pounded         332 - pierced\n\r",ch);
			send_to_char("333 - sliced           -2 - teleport\n\r",ch);
			send_to_char(" -3 - sleep            -4 - poison\n\r",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the trap level (damage is 3 x level).",ch);
		}
		else {
			send_to_char("\n\rValue4 are the number of trap charges left.",ch);
		}
		break;
	case ITEM_CONTAINER:
		if(value==0) {
			send_to_char("\n\rValue1 is the max weight (container + contents) that can fit.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 are the lock flags (add the flags).\n\r\n\r",ch);
			send_to_char(" 1 - closeable      2 - pickproof\n\r",ch);
			send_to_char(" 4 - closed         8 - locked\n\r",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the vnum of the key that opens it (-1 = no key).",ch);
		}
		else {
			send_to_char("\n\rValue4 determines whether the container is a corpse (1=yes, 0=no).",ch);
		}
		break;
	case ITEM_DRINKCON:
		if(value==0) {
			send_to_char("\n\rValue1 is the amount of fluid the container can hold.",ch);
		}
		else if(value==1) {
			send_to_char("\n\rValue2 is the amount of fluid left.\n\r\n\r",ch);
		}
		else if(value==2) {
			send_to_char("\n\rValue3 is the type of liquid in the container.\n\r\n\r",ch);
			send_to_char(" 0 - water        1 - beer\n\r",ch);
			send_to_char(" 2 - wine         3 - ale\n\r",ch);
			send_to_char(" 4 - dark ale     5 - whisky\n\r",ch);
			send_to_char(" 6 - lemonade     7 - firebreather\n\r",ch);
			send_to_char(" 8 - speciality   9 - slime mold juice\n\r",ch);
			send_to_char("10 - milk        11 - tea\n\r",ch);
			send_to_char("12 - coffee      13 - blood\n\r",ch);
			send_to_char("14 - salt water  15 - coca cola\n\r",ch);
		}
		else {
			send_to_char("\n\rValue4 are the liquid flags (add the flags).\n\r\n\r",ch);
			send_to_char(" 1 - poisoned       2 - endless (never empties)\n\r",ch);
		}
		break;
	case ITEM_NOTE:
		send_to_char("\n\rValue not used for this item type.\n\r",ch);
		break;
	case ITEM_KEY:
		if(value==0) {
			send_to_char("\n\rValue1 is the keytype (not used by the code:\n\r"
						 "doors and containers check the key vnum).",ch);
		}
		else {
			send_to_char("\n\rValue not used for this item type.\n\r",ch);
		}
		break;
	case ITEM_FOOD:
		if(value==0) {
			send_to_char("\n\rValue1 is how full you get from eating the food.",ch);
		}
		else if(value==3) {
			send_to_char("\n\rValue4 is whether the food is poisoned (1=yes, 0=no).\n\r\n\r",ch);
		}
		else {
			send_to_char("\n\rValue not used for this item type.\n\r",ch);
		}
		break;
	case ITEM_MONEY:
		if(value==0) {
			send_to_char("\n\rValue1 is the number of coins.",ch);
		}
		else {
			send_to_char("\n\rValue not used for this item type.\n\r",ch);
		}
		break;
	case ITEM_AUDIO:
		if(value==0) {
			send_to_char("\n\rValue1 is how often the object plays its sound, in pulses\n\r"
						 "(4 pulses = 1 second, checked every 4 seconds, 1 in 3 chance).\n\r"
						 "0 = never. The sound is the action description.",ch);
		}
		else {
			send_to_char("\n\rValue not used for this item type.\n\r",ch);
		}
		break;
	case ITEM_CLAN_SYMBOL:
		if(value==0) {
			send_to_char(
				"\n\rValue1 (V0) is the prince toon_id (MySQL toon.id) for the clan.\n\r",
				ch);
		}
		else {
			send_to_char("\n\rValue not used for clan symbols (leave 0).\n\r", ch);
		}
		break;
	default:
		send_to_char("Value not used for this item type.\n\r",ch);
		break;
	}


	EditorGoto(ch, 21, 1);
	{
		const char* label = ObjValueLabel(ch->specials.objedit->obj_flags.type_flag, value);
		if(label[0] != '\0') {
			sprintf(buf, "Object value #%d (%s) - Enter the new value (last value was %d).\n\r--> ",
					value + 1, label, ch->specials.objedit->obj_flags.value[ value ]);
		}
		else {
			sprintf(buf, "Object value #%d - Enter the new value (last value was %d).\n\r--> ",
					value + 1, ch->specials.objedit->obj_flags.value[ value ]);
		}
	}
	EditorPrompt(ch, buf);
}



} // namespace Alarmud
