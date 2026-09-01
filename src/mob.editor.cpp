/*ALARMUD*
 * Incastonazione: listino pietre da miniera + spec proc assegnabile a un mob.
 *
 * Il PG tiene oggetto e pietre con se': il mob lavora sul banco, senza
 * prenderli in consegna. Comando: incastona <oggetto> <pietra> [pietra ...]
 * Ask <mob> aiuto | listino
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"

#include "mob.editor.hpp"
#include "act.other.hpp"
#include "cmdid.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "obj_edit_catalog.hpp"
#include "spells.hpp"
#include "utility.hpp"

namespace Alarmud {

constexpr int kGemVnumMin = 19509;
constexpr int kGemVnumMax = 19537;
constexpr int kMaxSlots = MAX_OBJ_AFFECT;
constexpr int kMaxStones = kMaxSlots * 3;

enum GemExtra {
	GEM_EXTRA_NONE = 0,
	GEM_EXTRA_RESISTANT,
	GEM_EXTRA_ARTEFACT,
	GEM_EXTRA_INVISIBLE
};

struct GemCatalogEntry {
	int vnum;
	const char* material;
	int qty;
	int unit_value;
	bool weapon_ok;
	bool zircone_special;
	int loc_weapon;
	int mod_weapon;
	int extra_weapon;
	int loc_other;
	int mod_other;
	int extra_other;
	const char* desc_weapon;
	const char* desc_other;
};

/* Effetti e vnum: copia di do_insert + testi del listino pubblico. */
const GemCatalogEntry kGems[] = {
	{ 19509, "quarzo comune", 1, 1500, true, false,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GEM_EXTRA_NONE,
	  "infravision", "infravision" },
	{ 19523, "quarzo comune", 1, 1500, true, false,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GEM_EXTRA_NONE,
	  "infravision", "infravision" },
	{ 19510, "ossidiana", 2, 1500, true, false,
	  APPLY_STR, 1, GEM_EXTRA_NONE,
	  APPLY_STR, 1, GEM_EXTRA_NONE,
	  "+1 str", "+1 str" },
	{ 19511, "opale", 2, 1500, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_SCRYING), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_SCRYING), GEM_EXTRA_NONE,
	  "-", "spy" },
	{ 19512, "turchese", 1, 1500, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_PROTECT_FROM_EVIL), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_PROTECT_FROM_EVIL), GEM_EXTRA_NONE,
	  "-", "protection from evil" },
	{ 19513, "zircone", 1, 1500, false, true,
	  APPLY_NONE, 0, GEM_EXTRA_RESISTANT,
	  APPLY_NONE, 0, GEM_EXTRA_RESISTANT,
	  "-", "resistent / artifact (x3)" },
	{ 19514, "lapislazzuli", 1, 1500, false, false,
	  APPLY_MANA_REGEN, 5, GEM_EXTRA_NONE,
	  APPLY_MANA_REGEN, 5, GEM_EXTRA_NONE,
	  "-", "+5 mana regain" },
	{ 19515, "onice", 1, 1500, false, false,
	  APPLY_CHR, 1, GEM_EXTRA_NONE,
	  APPLY_CHR, 1, GEM_EXTRA_NONE,
	  "-", "+1 chr" },
	{ 19516, "malachite", 1, 1500, false, false,
	  APPLY_INT, 1, GEM_EXTRA_NONE,
	  APPLY_INT, 1, GEM_EXTRA_NONE,
	  "-", "+1 int" },
	{ 19517, "ematite", 1, 1500, false, false,
	  APPLY_CON, 1, GEM_EXTRA_NONE,
	  APPLY_CON, 1, GEM_EXTRA_NONE,
	  "-", "+1 cos" },
	{ 19518, "giada", 1, 1500, false, false,
	  APPLY_WIS, 1, GEM_EXTRA_NONE,
	  APPLY_WIS, 1, GEM_EXTRA_NONE,
	  "-", "+1 wis" },
	{ 19519, "resina fossilizzata", 1, 1500, false, false,
	  APPLY_SAVE_ALL, -1, GEM_EXTRA_NONE,
	  APPLY_SAVE_ALL, -1, GEM_EXTRA_NONE,
	  "-", "-1 save all" },
	{ 19520, "crisoberillo", 1, 1500, false, false,
	  APPLY_MOVE_REGEN, 5, GEM_EXTRA_NONE,
	  APPLY_MOVE_REGEN, 5, GEM_EXTRA_NONE,
	  "-", "+5 move regain" },
	{ 19521, "spinello blu", 1, 1500, false, false,
	  APPLY_SPELLFAIL, -2, GEM_EXTRA_NONE,
	  APPLY_SPELLFAIL, -2, GEM_EXTRA_NONE,
	  "-", "-2 spellfail" },
	{ 19522, "tormalina", 1, 1500, true, false,
	  APPLY_HIT, 2, GEM_EXTRA_NONE,
	  APPLY_HIT_REGEN, 5, GEM_EXTRA_NONE,
	  "+2 hp", "+5 hp regain" },
	{ 19524, "quarzo rosa", 3, 2250, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_SENSE_LIFE), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_SENSE_LIFE), GEM_EXTRA_NONE,
	  "-", "sense life" },
	{ 19525, "agata", 1, 2250, true, false,
	  APPLY_WEAPON_SPELL, SPELL_POISON, GEM_EXTRA_NONE,
	  APPLY_M_IMMUNE, static_cast<int>(IMM_POISON), GEM_EXTRA_NONE,
	  "wps poison", "immu poison" },
	{ 19526, "acquamarina", 1, 2250, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_WATERBREATH), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_WATERBREATH), GEM_EXTRA_NONE,
	  "-", "water breath" },
	{ 19527, "berillo", 1, 2250, false, false,
	  APPLY_DEX, 1, GEM_EXTRA_NONE,
	  APPLY_DEX, 1, GEM_EXTRA_NONE,
	  "-", "+1 dex" },
	{ 19528, "topazio", 1, 2250, true, false,
	  APPLY_WEAPON_SPELL, SPELL_SHOCKING_GRASP, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_ELEC), GEM_EXTRA_NONE,
	  "wps shocking grasp", "resi electricity" },
	{ 19529, "spinello nero", 1, 3000, true, false,
	  APPLY_WEAPON_SPELL, SPELL_SLEEP, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_HOLD), GEM_EXTRA_NONE,
	  "wps sleep", "resi hold" },
	{ 19530, "fluorite", 1, 3000, true, false,
	  APPLY_NONE, 0, GEM_EXTRA_INVISIBLE,
	  APPLY_SPELL, static_cast<int>(AFF_INVISIBLE), GEM_EXTRA_NONE,
	  "invisible (flag)", "invisibility" },
	{ 19531, "ametista", 1, 3000, true, false,
	  APPLY_WEAPON_SPELL, SPELL_MAGIC_MISSILE, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_ENERGY), GEM_EXTRA_NONE,
	  "wps magic missile", "resi energy" },
	{ 19532, "corindone", 1, 3000, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_TRUE_SIGHT), GEM_EXTRA_NONE,
	  APPLY_SPELL, static_cast<int>(AFF_TRUE_SIGHT), GEM_EXTRA_NONE,
	  "-", "true sight" },
	{ 19533, "granato", 1, 3000, true, false,
	  APPLY_HITNDAM, 1, GEM_EXTRA_NONE,
	  APPLY_AC, -10, GEM_EXTRA_NONE,
	  "+1 hit-n-dam", "-10 armor" },
	{ 19534, "zaffiro", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_CHILL_TOUCH, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_COLD), GEM_EXTRA_NONE,
	  "wps chill touch", "resi cold" },
	{ 19535, "smeraldo", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_ACID_BLAST, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_ACID), GEM_EXTRA_NONE,
	  "wps acid", "resi acid" },
	{ 19536, "rubino", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_BURNING_HANDS, GEM_EXTRA_NONE,
	  APPLY_IMMUNE, static_cast<int>(IMM_FIRE), GEM_EXTRA_NONE,
	  "wps burning hands", "resi fire" },
	{ 19537, "diamante", 1, 7500, true, false,
	  APPLY_NONE, 0, GEM_EXTRA_ARTEFACT,
	  APPLY_NONE, 0, GEM_EXTRA_ARTEFACT,
	  "artifact", "artifact" },
};

const GemCatalogEntry* find_gem(int vnum) {
	for(const auto& g : kGems) {
		if(g.vnum == vnum) {
			return &g;
		}
	}
	return nullptr;
}

int obj_vnum(const struct obj_data* obj) {
	if(!obj || obj->item_number < 0) {
		return 0;
	}
	return obj_index[obj->item_number].iVNum;
}

bool is_weapon_item(const struct obj_data* obj) {
	return GET_ITEM_TYPE(obj) == ITEM_WEAPON;
}

int count_used_slots(const struct obj_data* obj) {
	int affect = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; i++) {
		if((obj->affected[i].location != APPLY_NONE)
		   && (obj->affected[i].modifier != 0)
		   && (obj->affected[i].location != APPLY_SKIP)) {
			affect++;
		}
	}
	return affect;
}

bool item_type_allowed(const struct obj_data* obj, char* why, std::size_t why_sz) {
	switch(GET_ITEM_TYPE(obj)) {
	case ITEM_LIGHT:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_WEAPON:
	case ITEM_FIREWEAPON:
	case ITEM_OTHER:
	case ITEM_AUDIO:
	case ITEM_ARMOR:
	case ITEM_CONTAINER:
	case ITEM_TREASURE:
		return true;
	case ITEM_SCROLL:
		std::snprintf(why, why_sz, "Non si possono incastonare pergamene.");
		return false;
	case ITEM_POTION:
		std::snprintf(why, why_sz, "Non si possono incastonare pozioni.");
		return false;
	case ITEM_WORN:
		std::snprintf(why, why_sz, "Quest'oggetto e' troppo logorato.");
		return false;
	case ITEM_TRASH:
		std::snprintf(why, why_sz, "Spazzatura, non si incastona.");
		return false;
	case ITEM_TRAP:
		std::snprintf(why, why_sz, "Non si puo' incastonare una trappola.");
		return false;
	case ITEM_NOTE:
		std::snprintf(why, why_sz, "Questo tipo di oggetto e' fatto per scriverci sopra.");
		return false;
	case ITEM_FOOD:
		std::snprintf(why, why_sz, "Proprio quello che ci voleva, un panino al diamante.");
		return false;
	default:
		std::snprintf(why, why_sz, "Non si puo' incastonare questo tipo di oggetto.");
		return false;
	}
}

bool already_reserved(struct obj_data* obj, struct obj_data** reserved, int nres) {
	for(int i = 0; i < nres; i++) {
		if(reserved[i] == obj) {
			return true;
		}
	}
	return false;
}

struct obj_data* find_inv_by_keyword(struct char_data* ch, const char* keyword,
									 struct obj_data** reserved, int nres) {
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved, nres)) {
			continue;
		}
		if(!CAN_SEE_OBJ(ch, obj)) {
			continue;
		}
		if(isname(keyword, obj->name)) {
			return obj;
		}
	}
	return nullptr;
}

struct obj_data* find_inv_by_vnum(struct char_data* ch, int vnum,
								  struct obj_data** reserved, int nres) {
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved, nres)) {
			continue;
		}
		if(!CAN_SEE_OBJ(ch, obj)) {
			continue;
		}
		if(obj_vnum(obj) == vnum) {
			return obj;
		}
	}
	return nullptr;
}

int count_inv_vnum(struct char_data* ch, int vnum, struct obj_data** reserved, int nres) {
	int n = 0;
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved, nres)) {
			continue;
		}
		if(obj_vnum(obj) == vnum) {
			n++;
		}
	}
	return n;
}

int count_weapon_spells(const struct obj_data* obj) {
	int n = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; i++) {
		if(obj->affected[i].location == APPLY_WEAPON_SPELL) {
			n++;
		}
	}
	return n;
}

int pick_color(const GemCatalogEntry& g) {
	switch(g.vnum) {
	case 19509:
	case 19523:
		return 15;
	case 19510:
		return 8;
	case 19511:
		return number(9, 15);
	case 19512:
		return 14;
	case 19513:
		return number(9, 15);
	case 19514:
		return 12;
	case 19515: {
		int c = number(1, 8);
		return (c < 5) ? 1 : 8;
	}
	case 19516:
		return 2;
	case 19517:
		return 7;
	case 19518:
		return 10;
	case 19519: {
		int c = number(3, 11);
		return (c < 7) ? 3 : 11;
	}
	case 19520:
		return 11;
	case 19521:
		return 12;
	case 19522: {
		int c = number(1, 15);
		return (c == 4) ? 6 : c;
	}
	case 19524:
		return 13;
	case 19525: {
		int c = number(1, 15);
		return (c == 4) ? 2 : c;
	}
	case 19526: {
		int c = number(6, 14);
		return (c < 11) ? 6 : 14;
	}
	case 19527:
		return number(11, 15);
	case 19528:
		return 11;
	case 19529:
		return 8;
	case 19530:
		return 10;
	case 19531: {
		int c = number(5, 13);
		return (c < 9) ? 5 : 13;
	}
	case 19532: {
		int c = number(3, 7);
		return (c < 6) ? 3 : 7;
	}
	case 19533:
		return 13;
	case 19534:
		return 12;
	case 19535:
		return 10;
	case 19536:
		return 9;
	case 19537:
		return 15;
	default:
		return 15;
	}
}

unsigned long extra_to_flag(int extra) {
	switch(extra) {
	case GEM_EXTRA_RESISTANT:
		return ITEM_RESISTANT;
	case GEM_EXTRA_ARTEFACT:
		return ITEM_IMMUNE;
	case GEM_EXTRA_INVISIBLE:
		return ITEM_INVISIBLE;
	default:
		return 0;
	}
}

void tell_from_jeweler(struct char_data* ch, struct char_data* jeweler, const char* msg) {
	if(jeweler) {
		char buf[MAX_STRING_LENGTH];
		std::snprintf(buf, sizeof(buf), "$N ti dice '%s'", msg);
		act(buf, FALSE, ch, 0, jeweler, TO_CHAR);
	}
	else {
		std::string line = msg;
		line += "\n\r";
		send_to_char(line.c_str(), ch);
	}
}

void show_usage(struct char_data* ch, struct char_data* jeweler) {
	if(jeweler) {
		act("$c0011$N$c0007 solleva lo sguardo dal banco, con polvere di gemme sulle dita.",
			FALSE, ch, 0, jeweler, TO_CHAR);
		act("$N parla a bassa voce con $n, indicando il banco da lavoro.",
			FALSE, ch, 0, jeweler, TO_NOTVICT);
		tell_from_jeweler(ch, jeweler,
						  "$c0011Posa l'arma o il gioiello sul mio banco, ma non affidarmelo: ci lavoro io, mentre resta tuo.$c0007");
		tell_from_jeweler(ch, jeweler,
						  "$c0011Le pietre restano nella tua borsa. Le prendo io, una a una, quando mi dici quale intarsio vuoi.$c0007");
		tell_from_jeweler(ch, jeweler,
						  "$c0011Quando sei pronto, dimmi: $c0015incastona$c0011 seguito dal nome del pezzo e da quello delle pietre, una per ogni incavo.$c0007");
		tell_from_jeweler(ch, jeweler,
						  "$c0011Al piu' cinque incavi, meno quelli gia' sul pezzo. Opale e ossidiana ne chiedono due, il quarzo rosa tre.$c0007");
		tell_from_jeweler(ch, jeweler,
						  "$c0011Lo zircone: una sola pietra per la resistenza, tre per l'artifact.$c0007");
		tell_from_jeweler(ch, jeweler,
						  "$c0011Se vuoi vedere gli effetti, $c0015chiedimi listino$c0011. Per queste parole, $c0015chiedimi aiuto$c0011.$c0007");
		return;
	}
	send_to_char("$c0015insert$c0007 / $c0015incastona$c0007 <oggetto> <pietra> [pietra ...]\n\r"
				 "Oggetto e pietre nel tuo inventario. Max 5 incavi. Zircone: 1 resistent, 3 artifact.\n\r",
				 ch);
}

void show_listino(struct char_data* ch, struct char_data* jeweler) {
	if(jeweler) {
		act("$N srotola un foglio di pergamena ingiallita, pieno di segni e pietre disegnate.",
			FALSE, ch, 0, jeweler, TO_CHAR);
		act("$N mostra una pergamena a $n.", FALSE, ch, 0, jeweler, TO_NOTVICT);
	}
	send_to_char("$c0015Listino incastonazione$c0007\n\r", ch);
	send_to_char("$c0014qty  val   pietra                 armi                 altro$c0007\n\r", ch);
	send_to_char("$c0008-------------------------------------------------------------$c0007\n\r", ch);
	for(const auto& g : kGems) {
		if(g.vnum == 19523) {
			continue;
		}
		char line[256];
		if(g.zircone_special) {
			std::snprintf(line, sizeof(line),
						  "  1  1500  %-22s %-20s %s\n\r",
						  g.material, "-", "resistent");
			send_to_char(line, ch);
			std::snprintf(line, sizeof(line),
						  "  3  1500  %-22s %-20s %s\n\r",
						  g.material, "-", "artifact");
			send_to_char(line, ch);
			continue;
		}
		std::snprintf(line, sizeof(line),
					  "  %d  %4d  %-22s %-20s %s\n\r",
					  g.qty, g.unit_value, g.material, g.desc_weapon, g.desc_other);
		send_to_char(line, ch);
	}
}

bool object_can_be_mounted(struct char_data* ch, struct char_data* jeweler,
						   struct obj_data* obj) {
	if(IS_OBJ_STAT2(obj, ITEM2_EDIT)) {
		tell_from_jeweler(ch, jeweler, "Quell'oggetto e' stato plasmato dagli Dei, non lo tocco.");
		return false;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_INSERT)) {
		tell_from_jeweler(ch, jeweler, "E' gia' incastonato. Non si incastona due volte.");
		return false;
	}
	if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
		tell_from_jeweler(ch, jeweler, "Non incastono oggetti RARI.");
		return false;
	}
	if(object_is_tanned(obj)) {
		tell_from_jeweler(ch, jeweler, "Non incastono armature conciate.");
		return false;
	}
	char why[128];
	if(!item_type_allowed(obj, why, sizeof(why))) {
		tell_from_jeweler(ch, jeweler, why);
		return false;
	}
	if(count_used_slots(obj) >= kMaxSlots) {
		tell_from_jeweler(ch, jeweler, "Non c'e' piu' spazio: ha gia' 5 effetti.");
		return false;
	}
	return true;
}

void apply_extra_flag(struct obj_data* obj, int extra) {
	const unsigned long bit = extra_to_flag(extra);
	if(bit) {
		SET_BIT(obj->obj_flags.extra_flags, bit);
	}
}

int value_for_slot(const GemCatalogEntry& g, int consumed) {
	return g.unit_value * consumed;
}

void build_color_word(int aff, int val_avg, const int* colore,
					  char* color1, std::size_t c1sz,
					  char* color2, std::size_t c2sz) {
	const bool pietra = (val_avg <= 1500)
						|| (val_avg > 3000 && val_avg <= 4500)
						|| (val_avg > 6000 && val_avg < 7500);
	auto pad = [](int c) {
		return (c > 9) ? "" : "0";
	};
	if(aff <= 0) {
		std::snprintf(color1, c1sz, "%s", pietra ? "pietra" : "gemma");
		std::snprintf(color2, c2sz, "%s", pietra ? "pietre" : "gemme");
		return;
	}
	if(aff == 1) {
		std::snprintf(color1, c1sz, "$c00%s%d%s$c0007", pad(colore[0]), colore[0],
					  pietra ? "pietra" : "gemma");
		std::snprintf(color2, c2sz, "$c00%s%d%s$c0007", pad(colore[0]), colore[0],
					  pietra ? "pietre" : "gemme");
		return;
	}
	if(aff == 2) {
		std::snprintf(color1, c1sz, "$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pie" : "gem",
					  pad(colore[1]), colore[1], pietra ? "tra" : "ma");
		std::snprintf(color2, c2sz, "$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pie" : "gem",
					  pad(colore[1]), colore[1], pietra ? "tre" : "me");
		return;
	}
	if(aff == 3) {
		std::snprintf(color1, c1sz, "$c00%s%d%s$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pi" : "ge",
					  pad(colore[1]), colore[1], pietra ? "et" : "m",
					  pad(colore[2]), colore[2], pietra ? "ra" : "ma");
		std::snprintf(color2, c2sz, "$c00%s%d%s$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pi" : "ge",
					  pad(colore[1]), colore[1], pietra ? "et" : "m",
					  pad(colore[2]), colore[2], pietra ? "re" : "me");
		return;
	}
	if(aff == 4) {
		std::snprintf(color1, c1sz, "$c00%s%d%s$c00%s%d%s$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pi" : "g",
					  pad(colore[1]), colore[1], pietra ? "e" : "em",
					  pad(colore[2]), colore[2], pietra ? "t" : "m",
					  pad(colore[3]), colore[3], pietra ? "ra" : "a");
		std::snprintf(color2, c2sz, "$c00%s%d%s$c00%s%d%s$c00%s%d%s$c00%s%d%s$c0007",
					  pad(colore[0]), colore[0], pietra ? "pi" : "g",
					  pad(colore[1]), colore[1], pietra ? "e" : "em",
					  pad(colore[2]), colore[2], pietra ? "t" : "m",
					  pad(colore[3]), colore[3], pietra ? "re" : "e");
		return;
	}
	std::snprintf(color1, c1sz, "$c00%s%dp$c00%s%di$c00%s%det$c00%s%dr$c00%s%da$c0007",
				  pad(colore[0]), colore[0], pad(colore[1]), colore[1],
				  pad(colore[2]), colore[2], pad(colore[3]), colore[3],
				  pad(colore[4]), colore[4]);
	std::snprintf(color2, c2sz, "$c00%s%dp$c00%s%di$c00%s%det$c00%s%dr$c00%s%de$c0007",
				  pad(colore[0]), colore[0], pad(colore[1]), colore[1],
				  pad(colore[2]), colore[2], pad(colore[3]), colore[3],
				  pad(colore[4]), colore[4]);
	if(!pietra) {
		std::snprintf(color1, c1sz, "$c00%s%dg$c00%s%de$c00%s%dm$c00%s%dm$c00%s%da$c0007",
					  pad(colore[0]), colore[0], pad(colore[1]), colore[1],
					  pad(colore[2]), colore[2], pad(colore[3]), colore[3],
					  pad(colore[4]), colore[4]);
		std::snprintf(color2, c2sz, "$c00%s%dg$c00%s%de$c00%s%dm$c00%s%dm$c00%s%de$c0007",
					  pad(colore[0]), colore[0], pad(colore[1]), colore[1],
					  pad(colore[2]), colore[2], pad(colore[3]), colore[3],
					  pad(colore[4]), colore[4]);
	}
}

void rename_mounted_item(struct obj_data* obj, int aff, int val_orig, const int* colore) {
	if(aff <= 0) {
		return;
	}
	const int added = obj->obj_flags.cost - val_orig;
	const int val_avg = added / aff;
	char color1[80];
	char color2[80];
	build_color_word(aff, val_avg, colore, color1, sizeof(color1), color2, sizeof(color2));

	char buf[MAX_STRING_LENGTH];
	const char* base = obj->short_description ? obj->short_description : "un oggetto";
	if(val_avg <= 1500) {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con una %s incastrata brutalmente", base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con %s incastrate brutalmente", base, color2);
		}
	}
	else if(val_avg <= 3000) {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con incastonata una %s preziosa", base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con incastonate alcune %s preziose", base, color2);
		}
	}
	else if(val_avg <= 4500) {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con una %s preziosa cesellata finemente", base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con delle %s preziose cesellate finemente", base, color2);
		}
	}
	else if(val_avg <= 6000) {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con una grande %s incastonata elegantemente", base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con delle grandi %s incastonate elegantemente", base, color2);
		}
	}
	else if(val_avg < 7500) {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con una rarissima %s preziosa sapientemente incastonata",
						  base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con rarissime %s preziose sapientemente incastonate",
						  base, color2);
		}
	}
	else {
		if(aff == 1) {
			std::snprintf(buf, sizeof(buf), "%s con una %s unica cesellata ad arte", base, color1);
		}
		else {
			std::snprintf(buf, sizeof(buf), "%s con alcune %s uniche cesellate ad arte", base, color2);
		}
	}

	if(obj->short_description) {
		free(obj->short_description);
	}
	obj->short_description = (char*)strdup(buf);
	if(obj->description) {
		free(obj->description);
	}
	std::string obj_desc = obj->short_description;
	obj_desc += " e' qui per terra.";
	obj->description = (char*)strdup(obj_desc.c_str());
}

void consolidate_weapon_hnd(struct obj_data* obj) {
	if(GET_ITEM_TYPE(obj) != ITEM_WEAPON) {
		return;
	}
	int hitroll = 0;
	int damroll = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; i++) {
		if(obj->affected[i].location == APPLY_HITROLL) {
			hitroll += obj->affected[i].modifier;
			obj->affected[i].location = APPLY_NONE;
			obj->affected[i].modifier = 0;
		}
		else if(obj->affected[i].location == APPLY_DAMROLL) {
			damroll += obj->affected[i].modifier;
			obj->affected[i].location = APPLY_NONE;
			obj->affected[i].modifier = 0;
		}
		else if(obj->affected[i].location == APPLY_HITNDAM) {
			hitroll += obj->affected[i].modifier;
			damroll += obj->affected[i].modifier;
			obj->affected[i].location = APPLY_NONE;
			obj->affected[i].modifier = 0;
		}
	}
	if((hitroll + damroll) <= 0) {
		return;
	}
	bool hnd = false;
	bool h = false;
	bool d = false;
	for(int i = 0; i < MAX_OBJ_AFFECT; i++) {
		if(hitroll == damroll && obj->affected[i].location == APPLY_NONE && !hnd) {
			obj->affected[i].location = APPLY_HITNDAM;
			obj->affected[i].modifier = damroll;
			hnd = true;
		}
		else if(hitroll != 0 && obj->affected[i].location == APPLY_NONE && !h && !hnd) {
			obj->affected[i].location = APPLY_HITROLL;
			obj->affected[i].modifier = hitroll;
			h = true;
		}
		else if(damroll != 0 && obj->affected[i].location == APPLY_NONE && !d && !hnd) {
			obj->affected[i].location = APPLY_DAMROLL;
			obj->affected[i].modifier = damroll;
			d = true;
		}
	}
}

struct SlotPlan {
	const GemCatalogEntry* def;
	int consumed;
	int loc;
	int mod;
	int extra;
	int color;
	int value;
	struct obj_data* stones[3];
};

void incastona_execute(struct char_data* ch, struct char_data* jeweler, const char* arg) {
	char objname[MAX_INPUT_LENGTH];
	arg = one_argument(arg, objname);
	if(!*objname) {
		show_usage(ch, jeweler);
		return;
	}
	if(!str_cmp(objname, "listino") || !str_cmp(objname, "aiuto")
	   || !str_cmp(objname, "help")) {
		if(!str_cmp(objname, "listino")) {
			show_listino(ch, jeweler);
		}
		else {
			show_usage(ch, jeweler);
		}
		return;
	}

	struct obj_data* obj = get_obj_in_list_vis(ch, objname, ch->carrying);
	if(!obj) {
		tell_from_jeweler(ch, jeweler,
						  "Non vedo quel pezzo tra le tue cose. Deve essere con te, qui al banco.");
		return;
	}
	if(!object_can_be_mounted(ch, jeweler, obj)) {
		const char* oname = obj->short_description ? obj->short_description : "?";
		mudlog(LOG_PLAYERS, "%s incastona refused on %s", GET_NAME(ch), oname);
		return;
	}

	const int free_slots = kMaxSlots - count_used_slots(obj);
	struct obj_data* reserved[kMaxStones];
	int nres = 0;
	SlotPlan slots[kMaxSlots];
	int nslots = 0;
	int wait = 0;
	int incoming_wps = 0;

	for(int i = 0; i < free_slots; i++) {
		char gemma[MAX_INPUT_LENGTH];
		arg = one_argument(arg, gemma);
		if(!*gemma) {
			if(i == 0) {
				tell_from_jeweler(ch, jeweler, "Quale pietra vuoi incastonare?");
				return;
			}
			break;
		}

		struct obj_data* gem = find_inv_by_keyword(ch, gemma, reserved, nres);
		if(!gem) {
			char buf[256];
			std::snprintf(buf, sizeof(buf), "Non hai niente che si chiami '%s' con te.", gemma);
			tell_from_jeweler(ch, jeweler, buf);
			return;
		}
		if(gem == obj) {
			tell_from_jeweler(ch, jeweler, "Quello e' l'oggetto da incastonare, non una pietra.");
			return;
		}
		const int vnum = obj_vnum(gem);
		const GemCatalogEntry* def = find_gem(vnum);
		if(!def || vnum < kGemVnumMin || vnum > kGemVnumMax) {
			tell_from_jeweler(ch, jeweler, "Quello non e' una pietra da incastonare.");
			return;
		}
		if(is_weapon_item(obj) && !def->weapon_ok) {
			char buf[256];
			std::snprintf(buf, sizeof(buf), "La pietra '%s' non si incastona sulle armi.", def->material);
			tell_from_jeweler(ch, jeweler, buf);
			return;
		}

		int need = def->qty;
		int extra = is_weapon_item(obj) ? def->extra_weapon : def->extra_other;
		int loc = is_weapon_item(obj) ? def->loc_weapon : def->loc_other;
		int mod = is_weapon_item(obj) ? def->mod_weapon : def->mod_other;

		if(def->zircone_special) {
			const int have = count_inv_vnum(ch, def->vnum, reserved, nres);
			if(have >= 3) {
				need = 3;
				extra = GEM_EXTRA_ARTEFACT;
				loc = APPLY_NONE;
				mod = 0;
			}
			else if(have == 1) {
				need = 1;
				extra = GEM_EXTRA_RESISTANT;
				loc = APPLY_NONE;
				mod = 0;
			}
			else {
				tell_from_jeweler(ch, jeweler,
								  "Per lo zircone serve 1 pietra (resistent) oppure 3 (artifact).");
				return;
			}
		}

		if(loc == APPLY_WEAPON_SPELL) {
			incoming_wps++;
		}

		SlotPlan& plan = slots[nslots];
		plan.def = def;
		plan.consumed = need;
		plan.loc = loc;
		plan.mod = mod;
		plan.extra = extra;
		plan.color = pick_color(*def);
		plan.value = value_for_slot(*def, need);
		for(int s = 0; s < 3; s++) {
			plan.stones[s] = nullptr;
		}

		for(int s = 0; s < need; s++) {
			struct obj_data* stone = (s == 0)
				? gem
				: find_inv_by_vnum(ch, def->vnum, reserved, nres);
			if(!stone) {
				char buf[256];
				std::snprintf(buf, sizeof(buf), "Non hai abbastanza pietre di %s (ne servono %d).",
							  def->material, need);
				tell_from_jeweler(ch, jeweler, buf);
				return;
			}
			if(s == 0 && already_reserved(stone, reserved, nres)) {
				stone = find_inv_by_vnum(ch, def->vnum, reserved, nres);
				if(!stone) {
					tell_from_jeweler(ch, jeweler, "Non hai abbastanza pietre.");
					return;
				}
			}
			plan.stones[s] = stone;
			reserved[nres++] = stone;
		}

		if(!jeweler && !IS_DIO_MINORE(ch)) {
			wait += PULSE_VIOLENCE + PULSE_VIOLENCE * i;
			if(need >= 2) {
				wait += PULSE_VIOLENCE;
			}
			if(need >= 3) {
				wait += PULSE_VIOLENCE * 2;
			}
		}
		nslots++;
	}

	char extra_gem[MAX_INPUT_LENGTH];
	one_argument(arg, extra_gem);
	if(*extra_gem && nslots > 0) {
		char buf[256];
		std::snprintf(buf, sizeof(buf),
					  "Su questo pezzo restano solo %d incavi liberi: le altre pietre restano nella tua borsa.",
					  nslots);
		tell_from_jeweler(ch, jeweler, buf);
	}

	if(nslots <= 0) {
		tell_from_jeweler(ch, jeweler, "Quale pietra vuoi incastonare?");
		return;
	}

	if((count_weapon_spells(obj) + incoming_wps) > 1) {
		tell_from_jeweler(ch, jeweler, "Un'arma puo' avere una sola weapon spell.");
		return;
	}

	const char* rand_reaction[] = {
		"Studi meticolosamente $p, poi sorridi tra te e te.",
		"Guardi entusiasta $p pensando 'Ma quanto sono brav$b!'",
		"Esclami: '$c0009SI PUO' FARE!$c0007'",
		"Sorridi compiaciut$b.",
		"Pensi: 'Potevo fare di meglio, ma comunque va MOLTO bene :-)'",
		"Guardi con adorazione $p poi, a voce alta, esclami: '$c0009Il mio tesssssoro!$c0007'",
		"Ti sfreghi le mani con soddisfazione.",
		"Osservi sognante $p, hai fatto un ottimo lavoro!",
		"Molto bene, la gemma e' incastonata perfettamente.",
		"Pensi tra te e te: 'E anche questa e' fatta!'",
		"$n studia meticolosamente $p, poi sorride tra se e se.",
		"$n guarda entusiasta $p.",
		"$n esclama: '$c0009SI PUO' FARE!$c0007'",
		"$n sorride compiaciut$b.",
		"$n annuisce soddisfatto, valutando il taglio.",
		"$n guarda con adorazione $p poi esclama: '$c0009Il mio tesssssoro!$c0007'",
		"$n si sfrega le mani con soddisfazione.",
		"$n osserva sognante $p.",
		"Un ghigno compiaciuto compare sulle labbra di $n.",
		"$n mormora: 'E anche questa e' fatta!'"
	};
	const int nRandReac = 9;

	struct char_data* actor = jeweler ? jeweler : ch;

	if(jeweler) {
		act("$n sistema gli attrezzi sul banco di legno: scalpelli, uncini, pinze, lime.",
			TRUE, actor, obj, 0, TO_ROOM);
		act("$N attira $c0015$p$c0007 sul banco davanti a te, senza sottrartelo, e pesca le pietre dalla tua borsa.",
			FALSE, ch, obj, jeweler, TO_CHAR);
		act("$N attira $c0015$p$c0007 sul banco davanti a $n e pesca le pietre dalla borsa.",
			FALSE, ch, obj, jeweler, TO_NOTVICT);
		act("$n valuta $c0015$p$c0007 e, con mano ferma, si mette all'opera.\n\r",
			TRUE, actor, obj, 0, TO_ROOM);
	}
	else {
		send_to_char("Sistemi gli attrezzi di lavoro sul tuo banco di legno e li controlli con cura: scalpelli, uncini, pinze, lime.\n\r", ch);
		send_to_char("Valuti con cura quali siano i migliori per iniziare, prendi fiato ed inizi a lavorare.\n\r\n\r", ch);
		act("Inizi ad armeggiare con $c0015$p$c0007.\n\r", TRUE, ch, obj, 0, TO_CHAR);
		act("$n tira fuori una serie di utensili da lavoro, controlla sapientemente $c0015$p$c0007 poi,\n\rcon mano ferma, si mette all'opera.\n\r",
			TRUE, ch, obj, 0, TO_ROOM);
	}

	for(int i = 0; i < nslots; i++) {
		for(int s = 0; s < slots[i].consumed; s++) {
			struct obj_data* stone = slots[i].stones[s];
			if(!stone) {
				continue;
			}
			char buf[256];
			if(jeweler) {
				std::snprintf(buf, sizeof(buf),
							  "$n incastona $c0015%s$c0007 su $c0015$p$c0007.",
							  stone->short_description ? stone->short_description : "una pietra");
				act(buf, TRUE, actor, obj, 0, TO_ROOM);
				act(rand_reaction[number(10, nRandReac + 10)], TRUE, actor, obj, 0, TO_ROOM);
			}
			else {
				std::snprintf(buf, sizeof(buf), "Incastoni $c0015%s$c0007 su $c0015%s$c0007.\n\r",
							  stone->short_description ? stone->short_description : "una pietra",
							  obj->short_description ? obj->short_description : "l'oggetto");
				send_to_char(buf, ch);
				act(rand_reaction[number(0, nRandReac)], TRUE, ch, obj, 0, TO_CHAR);
				std::snprintf(buf, sizeof(buf), "$n incastona $c0015%s$c0007 su $c0015$p$c0007.",
							  stone->short_description ? stone->short_description : "una pietra");
				act(buf, TRUE, ch, obj, 0, TO_ROOM);
				act(rand_reaction[number(10, nRandReac + 10)], TRUE, ch, obj, 0, TO_ROOM);
			}
			obj_from_char(stone);
			extract_obj(stone);
		}
	}

	const int val_orig = obj->obj_flags.cost;
	int aff = 0;
	int colore[kMaxSlots] = { 0, 0, 0, 0, 0 };

	for(int i = 0; i < MAX_OBJ_AFFECT && aff < nslots; i++) {
		if((obj->affected[i].location != APPLY_NONE)
		   && (obj->affected[i].modifier != 0)
		   && (obj->affected[i].location != APPLY_SKIP)) {
			continue;
		}
		const SlotPlan& plan = slots[aff];
		obj->affected[i].location = plan.loc;
		obj->affected[i].modifier = plan.mod;
		apply_extra_flag(obj, plan.extra);
		obj->obj_flags.cost += plan.value;
		colore[aff] = plan.color;
		aff++;
	}

	consolidate_weapon_hnd(obj);
	rename_mounted_item(obj, aff, val_orig, colore);
	SET_BIT(obj->obj_flags.extra_flags2, ITEM2_INSERT);
	/* Listino: ogni incastonatura rende l'oggetto raro (cost >= LIM_ITEM_COST_MIN).
	 * insert somma solo il valore delle pietre; se non basta, si porta alla soglia. */
	if(obj->obj_flags.cost < LIM_ITEM_COST_MIN) {
		obj->obj_flags.cost = LIM_ITEM_COST_MIN;
	}

	if(!jeweler && wait > 0 && !IS_DIO_MINORE(ch)) {
		WAIT_STATE(ch, wait);
	}

	if(jeweler) {
		act("$n lascia $c0015$p$c0007 sul banco davanti a te e mette via gli attrezzi, soddisfatt$b.",
			TRUE, actor, obj, ch, TO_VICT);
		act("$n lascia $c0015$p$c0007 sul banco davanti a $N e mette via gli attrezzi, soddisfatt$b.",
			TRUE, actor, obj, ch, TO_NOTVICT);
	}
	else {
		act("\n\rHai terminato il tuo lavoro su $c0015$p$c0007.", TRUE, ch, obj, 0, TO_CHAR);
		act("$n mette via tutti gli attrezzi, e' soddisfatt$b del suo lavoro su $c0015$p$c0007.",
			TRUE, ch, obj, 0, TO_ROOM);
	}

	const char* oname = obj->short_description ? obj->short_description : "?";
	const char* jname = (jeweler && GET_NAME(jeweler)) ? GET_NAME(jeweler) : "self";
	mudlog(LOG_PLAYERS, "%s incastona %d slot su %s (jeweler=%s)",
		   GET_NAME(ch), aff, oname, jname);
	schedule_inventory_save(ch);
}

bool ask_is_for_mob(struct char_data* ch, const char* arg, struct char_data* mob,
					char* rest, std::size_t rest_sz) {
	char who[MAX_INPUT_LENGTH];
	const char* p = one_argument(arg, who);
	if(!*who) {
		return false;
	}
	struct char_data* vict = get_char_room_vis(ch, who);
	if(vict != mob) {
		return false;
	}
	while(p && *p == ' ') {
		p++;
	}
	if(!p) {
		rest[0] = '\0';
		return true;
	}
	std::snprintf(rest, rest_sz, "%s", p);
	return true;
}

void incastona_from_command(struct char_data* ch, const char* arg,
							struct char_data* jeweler) {
	if(!ch) {
		return;
	}
	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		send_to_char("Chi ti pensi di essere? Un gioielliere? Sei solo uno stupido mob!\n\r", ch);
		return;
	}
	incastona_execute(ch, jeweler, arg ? arg : "");
}

MOBSPECIAL_FUNC(Incastonatore) {
	if(!ch || !mob) {
		return FALSE;
	}

	if(type == EVENT_TICK) {
		if(!AWAKE(mob) || mob->specials.fighting) {
			return FALSE;
		}
		if(number(0, 2) != 0) {
			return FALSE;
		}
		struct room_data* rp = real_roomp(mob->in_room);
		if(!rp) {
			return FALSE;
		}
		bool saw_pc = false;
		for(struct char_data* t = rp->people; t; t = t->next_in_room) {
			if(IS_PC(t) && t != mob) {
				saw_pc = true;
				break;
			}
		}
		if(!saw_pc) {
			return FALSE;
		}
		switch(number(0, 3)) {
		case 0:
			act("$n dice '$c0010Se volete un intarsio, posate il pezzo sul mio banco e nominate pietra e foggia. Ci lavoro io: non serve affidarmelo.$c0007'",
				FALSE, mob, 0, 0, TO_ROOM);
			break;
		case 1:
			act("$n dice '$c0010Se non sapete da dove cominciare, $c0015chiedetemi aiuto$c0010: vi spiego il mestiere.$c0007'",
				FALSE, mob, 0, 0, TO_ROOM);
			break;
		case 2:
			act("$n dice '$c0010Volete sapere che potere cela ciascuna pietra? Chiedetemi il $c0015listino$c0010.$c0007'",
				FALSE, mob, 0, 0, TO_ROOM);
			break;
		default:
			act("$n dice '$c0010Pronunciate $c0015incastona$c0010, poi il nome del pezzo e delle pietre. Opale e ossidiana ne vogliono due, il quarzo rosa tre.$c0007'",
				FALSE, mob, 0, 0, TO_ROOM);
			break;
		}
		return FALSE;
	}

	if(type != EVENT_COMMAND) {
		return FALSE;
	}
	if(!AWAKE(mob)) {
		return FALSE;
	}
	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		return FALSE;
	}

	if(cmd == CMD_ASK) {
		char rest[MAX_INPUT_LENGTH];
		if(!ask_is_for_mob(ch, arg, mob, rest, sizeof(rest))) {
			return FALSE;
		}
		char topic[MAX_INPUT_LENGTH];
		const char* p = one_argument(rest, topic);
		if(!*topic || !str_cmp(topic, "aiuto") || !str_cmp(topic, "help")
		   || !str_cmp(topic, "incastona")) {
			if(*topic && !str_cmp(topic, "incastona") && p && *p) {
				while(*p == ' ') {
					p++;
				}
				if(*p) {
					incastona_from_command(ch, p, mob);
					return TRUE;
				}
			}
			show_usage(ch, mob);
			return TRUE;
		}
		if(!str_cmp(topic, "listino") || !str_cmp(topic, "pietre")
		   || !str_cmp(topic, "gemme")) {
			show_listino(ch, mob);
			return TRUE;
		}
		show_usage(ch, mob);
		return TRUE;
	}

	if(cmd == CMD_INCASTONA) {
		if(mob->specials.fighting) {
			tell_from_jeweler(ch, mob, "Non vedi che sto combattendo?");
			return TRUE;
		}
		incastona_from_command(ch, arg, mob);
		return TRUE;
	}

	return FALSE;
}

} // namespace Alarmud
