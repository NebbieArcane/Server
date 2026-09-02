/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* Incastonazione: listino pietre da miniera + spec proc assegnabile a un mob.
 *
 * Il PG tiene oggetto e pietre con se': il mob lavora sul banco, senza
 * prenderli in consegna. Comando: incastona <oggetto> <pietra> [pietra ...]
 * Ask <mob> aiuto | listino
 */
#include <functional>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
#include "spells.hpp"
#include "utility.hpp"

namespace Alarmud {

constexpr int kGemVnumMin = 19509;
constexpr int kGemVnumMax = 19537;
constexpr int kMaxSlots = MAX_OBJ_AFFECT;
constexpr int kMaxStones = kMaxSlots * 3;
constexpr int kMaxStonesPerSlot = 3;

enum class GemExtra {
	None,
	Resistant,
	Artefact,
	Invisible
};

struct GemCatalogEntry {
	int vnum{};
	std::string_view material;
	int qty{};
	int unit_value{};
	bool weapon_ok{};
	bool zircone_special{};
	int loc_weapon{};
	int mod_weapon{};
	GemExtra extra_weapon{GemExtra::None};
	int loc_other{};
	int mod_other{};
	GemExtra extra_other{GemExtra::None};
	std::string_view desc_weapon;
	std::string_view desc_other;
};

using ColorPalette = std::array<int, kMaxSlots>;
using ColorWords = std::pair<std::string, std::string>;

namespace {

void set_obj_cstr(char*& field, const std::string& value) {
	if(field) {
		free(field);
	}
	field = strdup(value.c_str());
}

[[nodiscard]] std::string_view obj_short_name(const obj_data* obj,
											  std::string_view fallback = "un oggetto") {
	if(obj != nullptr && obj->short_description != nullptr) {
		return obj->short_description;
	}
	return fallback;
}

[[nodiscard]] std::string color_token(int color) {
	std::string token = "$c00";
	if(color <= 9) {
		token += '0';
	}
	token += std::to_string(color);
	return token;
}

[[nodiscard]] std::string pad_field(std::string_view text, std::size_t width) {
	std::string out(text);
	if(out.size() >= width) {
		return out.substr(0, width);
	}
	out.append(width - out.size(), ' ');
	return out;
}

[[nodiscard]] std::string format_listino_row(int qty, int value, std::string_view material,
											 std::string_view weapons, std::string_view other) {
	std::string line = "  ";
	if(qty < 10) {
		line += ' ';
	}
	line += std::to_string(qty);
	line += "  ";
	const std::string val = std::to_string(value);
	line += std::string(val.size() < 4 ? 4 - val.size() : 0, ' ');
	line += val;
	line += "  ";
	line += pad_field(material, 22);
	line += ' ';
	line += pad_field(weapons, 20);
	line += ' ';
	line += other;
	line += "\n\r";
	return line;
}

void tell_from_jeweler(char_data* ch, char_data* jeweler, std::string_view msg) {
	if(jeweler) {
		const std::string buf = std::string("$N ti dice '") + std::string(msg) + "'";
		act(buf.c_str(), FALSE, ch, 0, jeweler, TO_CHAR);
	}
	else {
		send_to_char((std::string(msg) + "\n\r").c_str(), ch);
	}
}

} // namespace

/* Effetti e vnum: copia di do_insert + testi del listino pubblico. */
constexpr std::array<GemCatalogEntry, 29> kGems = {{
	{ 19509, "quarzo comune", 1, 1500, true, false,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GemExtra::None,
	  "infravision", "infravision" },
	{ 19523, "quarzo comune", 1, 1500, true, false,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_INFRAVISION), GemExtra::None,
	  "infravision", "infravision" },
	{ 19510, "ossidiana", 2, 1500, true, false,
	  APPLY_STR, 1, GemExtra::None,
	  APPLY_STR, 1, GemExtra::None,
	  "+1 str", "+1 str" },
	{ 19511, "opale", 2, 1500, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_SCRYING), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_SCRYING), GemExtra::None,
	  "-", "spy" },
	{ 19512, "turchese", 1, 1500, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_PROTECT_FROM_EVIL), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_PROTECT_FROM_EVIL), GemExtra::None,
	  "-", "protection from evil" },
	{ 19513, "zircone", 1, 1500, false, true,
	  APPLY_NONE, 0, GemExtra::Resistant,
	  APPLY_NONE, 0, GemExtra::Resistant,
	  "-", "resistent / artifact (x3)" },
	{ 19514, "lapislazzuli", 1, 1500, false, false,
	  APPLY_MANA_REGEN, 5, GemExtra::None,
	  APPLY_MANA_REGEN, 5, GemExtra::None,
	  "-", "+5 mana regain" },
	{ 19515, "onice", 1, 1500, false, false,
	  APPLY_CHR, 1, GemExtra::None,
	  APPLY_CHR, 1, GemExtra::None,
	  "-", "+1 chr" },
	{ 19516, "malachite", 1, 1500, false, false,
	  APPLY_INT, 1, GemExtra::None,
	  APPLY_INT, 1, GemExtra::None,
	  "-", "+1 int" },
	{ 19517, "ematite", 1, 1500, false, false,
	  APPLY_CON, 1, GemExtra::None,
	  APPLY_CON, 1, GemExtra::None,
	  "-", "+1 cos" },
	{ 19518, "giada", 1, 1500, false, false,
	  APPLY_WIS, 1, GemExtra::None,
	  APPLY_WIS, 1, GemExtra::None,
	  "-", "+1 wis" },
	{ 19519, "resina fossilizzata", 1, 1500, false, false,
	  APPLY_SAVE_ALL, -1, GemExtra::None,
	  APPLY_SAVE_ALL, -1, GemExtra::None,
	  "-", "-1 save all" },
	{ 19520, "crisoberillo", 1, 1500, false, false,
	  APPLY_MOVE_REGEN, 5, GemExtra::None,
	  APPLY_MOVE_REGEN, 5, GemExtra::None,
	  "-", "+5 move regain" },
	{ 19521, "spinello blu", 1, 1500, false, false,
	  APPLY_SPELLFAIL, -2, GemExtra::None,
	  APPLY_SPELLFAIL, -2, GemExtra::None,
	  "-", "-2 spellfail" },
	{ 19522, "tormalina", 1, 1500, true, false,
	  APPLY_HIT, 2, GemExtra::None,
	  APPLY_HIT_REGEN, 5, GemExtra::None,
	  "+2 hp", "+5 hp regain" },
	{ 19524, "quarzo rosa", 3, 2250, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_SENSE_LIFE), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_SENSE_LIFE), GemExtra::None,
	  "-", "sense life" },
	{ 19525, "agata", 1, 2250, true, false,
	  APPLY_WEAPON_SPELL, SPELL_POISON, GemExtra::None,
	  APPLY_M_IMMUNE, static_cast<int>(IMM_POISON), GemExtra::None,
	  "wps poison", "immu poison" },
	{ 19526, "acquamarina", 1, 2250, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_WATERBREATH), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_WATERBREATH), GemExtra::None,
	  "-", "water breath" },
	{ 19527, "berillo", 1, 2250, false, false,
	  APPLY_DEX, 1, GemExtra::None,
	  APPLY_DEX, 1, GemExtra::None,
	  "-", "+1 dex" },
	{ 19528, "topazio", 1, 2250, true, false,
	  APPLY_WEAPON_SPELL, SPELL_SHOCKING_GRASP, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_ELEC), GemExtra::None,
	  "wps shocking grasp", "resi electricity" },
	{ 19529, "spinello nero", 1, 3000, true, false,
	  APPLY_WEAPON_SPELL, SPELL_SLEEP, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_HOLD), GemExtra::None,
	  "wps sleep", "resi hold" },
	{ 19530, "fluorite", 1, 3000, true, false,
	  APPLY_NONE, 0, GemExtra::Invisible,
	  APPLY_SPELL, static_cast<int>(AFF_INVISIBLE), GemExtra::None,
	  "invisible (flag)", "invisibility" },
	{ 19531, "ametista", 1, 3000, true, false,
	  APPLY_WEAPON_SPELL, SPELL_MAGIC_MISSILE, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_ENERGY), GemExtra::None,
	  "wps magic missile", "resi energy" },
	{ 19532, "corindone", 1, 3000, false, false,
	  APPLY_SPELL, static_cast<int>(AFF_TRUE_SIGHT), GemExtra::None,
	  APPLY_SPELL, static_cast<int>(AFF_TRUE_SIGHT), GemExtra::None,
	  "-", "true sight" },
	{ 19533, "granato", 1, 3000, true, false,
	  APPLY_HITNDAM, 1, GemExtra::None,
	  APPLY_AC, -10, GemExtra::None,
	  "+1 hit-n-dam", "-10 armor" },
	{ 19534, "zaffiro", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_CHILL_TOUCH, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_COLD), GemExtra::None,
	  "wps chill touch", "resi cold" },
	{ 19535, "smeraldo", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_ACID_BLAST, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_ACID), GemExtra::None,
	  "wps acid", "resi acid" },
	{ 19536, "rubino", 1, 7500, true, false,
	  APPLY_WEAPON_SPELL, SPELL_BURNING_HANDS, GemExtra::None,
	  APPLY_IMMUNE, static_cast<int>(IMM_FIRE), GemExtra::None,
	  "wps burning hands", "resi fire" },
	{ 19537, "diamante", 1, 7500, true, false,
	  APPLY_NONE, 0, GemExtra::Artefact,
	  APPLY_NONE, 0, GemExtra::Artefact,
	  "artifact", "artifact" },
}};

[[nodiscard]] std::optional<std::reference_wrapper<const GemCatalogEntry>> find_gem(int vnum) {
	for(const auto& g : kGems) {
		if(g.vnum == vnum) {
			return std::cref(g);
		}
	}
	return std::nullopt;
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

[[nodiscard]] std::optional<std::string_view> item_type_reject_reason(const obj_data& obj) {
	switch(GET_ITEM_TYPE(&obj)) {
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
		return std::nullopt;
	case ITEM_SCROLL:
		return "Non si possono incastonare pergamene.";
	case ITEM_POTION:
		return "Non si possono incastonare pozioni.";
	case ITEM_WORN:
		return "Quest'oggetto e' troppo logorato.";
	case ITEM_TRASH:
		return "Spazzatura, non si incastona.";
	case ITEM_TRAP:
		return "Non si puo' incastonare una trappola.";
	case ITEM_NOTE:
		return "Questo tipo di oggetto e' fatto per scriverci sopra.";
	case ITEM_FOOD:
		return "Proprio quello che ci voleva, un panino al diamante.";
	default:
		return "Non si puo' incastonare questo tipo di oggetto.";
	}
}

[[nodiscard]] bool already_reserved(obj_data* obj, const std::vector<obj_data*>& reserved) {
	return std::find(reserved.begin(), reserved.end(), obj) != reserved.end();
}

obj_data* find_inv_by_keyword(char_data* ch, const char* keyword,
							  const std::vector<obj_data*>& reserved) {
	for(obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved)) {
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

obj_data* find_inv_by_vnum(char_data* ch, int vnum, const std::vector<obj_data*>& reserved) {
	for(obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved)) {
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

int count_inv_vnum(char_data* ch, int vnum, const std::vector<obj_data*>& reserved) {
	int n = 0;
	for(obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(already_reserved(obj, reserved)) {
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

[[nodiscard]] unsigned long extra_to_flag(GemExtra extra) {
	switch(extra) {
	case GemExtra::Resistant:
		return ITEM_RESISTANT;
	case GemExtra::Artefact:
		return ITEM_IMMUNE;
	case GemExtra::Invisible:
		return ITEM_INVISIBLE;
	case GemExtra::None:
		return 0;
	}
	return 0;
}

void show_usage(char_data* ch, char_data* jeweler) {
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

void show_listino(char_data* ch, char_data* jeweler) {
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
		if(g.zircone_special) {
			send_to_char(format_listino_row(1, 1500, g.material, "-", "resistent").c_str(), ch);
			send_to_char(format_listino_row(3, 1500, g.material, "-", "artifact").c_str(), ch);
			continue;
		}
		send_to_char(format_listino_row(g.qty, g.unit_value, g.material, g.desc_weapon,
										g.desc_other)
						 .c_str(),
					 ch);
	}
}

bool object_can_be_mounted(char_data* ch, char_data* jeweler, obj_data* obj) {
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
	if(TANNED(obj)) {
		tell_from_jeweler(ch, jeweler, "Non incastono armature conciate.");
		return false;
	}
	if(const auto reject = item_type_reject_reason(*obj)) {
		tell_from_jeweler(ch, jeweler, *reject);
		return false;
	}
	if(count_used_slots(obj) >= kMaxSlots) {
		tell_from_jeweler(ch, jeweler, "Non c'e' piu' spazio: ha gia' 5 effetti.");
		return false;
	}
	return true;
}

void apply_extra_flag(obj_data* obj, GemExtra extra) {
	const unsigned long bit = extra_to_flag(extra);
	if(bit) {
		SET_BIT(obj->obj_flags.extra_flags, bit);
	}
}

int value_for_slot(const GemCatalogEntry& g, int consumed) {
	return g.unit_value * consumed;
}

[[nodiscard]] ColorWords build_color_words(int aff, int val_avg, const ColorPalette& colore) {
	const bool pietra = (val_avg <= 1500)
						|| (val_avg > 3000 && val_avg <= 4500)
						|| (val_avg > 6000 && val_avg < 7500);
	const char* singular_noun = pietra ? "pietra" : "gemma";
	const char* plural_noun = pietra ? "pietre" : "gemme";

	if(aff <= 0) {
		return {singular_noun, plural_noun};
	}
	if(aff == 1) {
		return {color_token(colore[0]) + singular_noun + "$c0007",
				color_token(colore[0]) + plural_noun + "$c0007"};
	}
	if(aff == 2) {
		return {color_token(colore[0]) + (pietra ? "pie" : "gem") + color_token(colore[1])
					+ (pietra ? "tra" : "ma") + "$c0007",
				color_token(colore[0]) + (pietra ? "pie" : "gem") + color_token(colore[1])
					+ (pietra ? "tre" : "me") + "$c0007"};
	}
	if(aff == 3) {
		return {color_token(colore[0]) + (pietra ? "pi" : "ge") + color_token(colore[1])
					+ (pietra ? "et" : "m") + color_token(colore[2]) + (pietra ? "ra" : "ma")
					+ "$c0007",
				color_token(colore[0]) + (pietra ? "pi" : "ge") + color_token(colore[1])
					+ (pietra ? "et" : "m") + color_token(colore[2]) + (pietra ? "re" : "me")
					+ "$c0007"};
	}
	if(aff == 4) {
		return {color_token(colore[0]) + (pietra ? "pi" : "g") + color_token(colore[1])
					+ (pietra ? "e" : "em") + color_token(colore[2]) + (pietra ? "t" : "m")
					+ color_token(colore[3]) + (pietra ? "ra" : "a") + "$c0007",
				color_token(colore[0]) + (pietra ? "pi" : "g") + color_token(colore[1])
					+ (pietra ? "e" : "em") + color_token(colore[2]) + (pietra ? "t" : "m")
					+ color_token(colore[3]) + (pietra ? "re" : "e") + "$c0007"};
	}
	if(pietra) {
		return {color_token(colore[0]) + "p" + color_token(colore[1]) + "i" + color_token(colore[2])
					+ "e" + color_token(colore[3]) + "t" + color_token(colore[4]) + "ra" + "$c0007",
				color_token(colore[0]) + "p" + color_token(colore[1]) + "i" + color_token(colore[2])
					+ "e" + color_token(colore[3]) + "t" + color_token(colore[4]) + "re" + "$c0007"};
	}
	return {color_token(colore[0]) + "g" + color_token(colore[1]) + "e" + color_token(colore[2])
				+ "m" + color_token(colore[3]) + "m" + color_token(colore[4]) + "a" + "$c0007",
			color_token(colore[0]) + "g" + color_token(colore[1]) + "e" + color_token(colore[2])
				+ "m" + color_token(colore[3]) + "m" + color_token(colore[4]) + "e" + "$c0007"};
}

void rename_mounted_item(obj_data* obj, int aff, int val_orig, const ColorPalette& colore) {
	if(aff <= 0) {
		return;
	}
	const int added = obj->obj_flags.cost - val_orig;
	const int val_avg = added / aff;
	const auto [color1, color2] = build_color_words(aff, val_avg, colore);
	const std::string base = std::string(obj_short_name(obj));

	std::string short_desc;
	if(val_avg <= 1500) {
		short_desc = (aff == 1)
			? base + " con una " + color1 + " incastrata brutalmente"
			: base + " con " + color2 + " incastrate brutalmente";
	}
	else if(val_avg <= 3000) {
		short_desc = (aff == 1)
			? base + " con incastonata una " + color1 + " preziosa"
			: base + " con incastonate alcune " + color2 + " preziose";
	}
	else if(val_avg <= 4500) {
		short_desc = (aff == 1)
			? base + " con una " + color1 + " preziosa cesellata finemente"
			: base + " con delle " + color2 + " preziose cesellate finemente";
	}
	else if(val_avg <= 6000) {
		short_desc = (aff == 1)
			? base + " con una grande " + color1 + " incastonata elegantemente"
			: base + " con delle grandi " + color2 + " incastonate elegantemente";
	}
	else if(val_avg < 7500) {
		short_desc = (aff == 1)
			? base + " con una rarissima " + color1 + " preziosa sapientemente incastonata"
			: base + " con rarissime " + color2 + " preziose sapientemente incastonate";
	}
	else {
		short_desc = (aff == 1)
			? base + " con una " + color1 + " unica cesellata ad arte"
			: base + " con alcune " + color2 + " uniche cesellate ad arte";
	}

	set_obj_cstr(obj->short_description, short_desc);
	set_obj_cstr(obj->description, short_desc + " e' qui per terra.");
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
	const GemCatalogEntry* def{nullptr};
	int consumed{};
	int loc{};
	int mod{};
	GemExtra extra{GemExtra::None};
	int color{};
	int value{};
	std::array<obj_data*, kMaxStonesPerSlot> stones{};
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
	std::vector<obj_data*> reserved;
	reserved.reserve(kMaxStones);
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

		obj_data* gem = find_inv_by_keyword(ch, gemma, reserved);
		if(!gem) {
			tell_from_jeweler(ch, jeweler,
							  "Non hai niente che si chiami '" + std::string(gemma) + "' con te.");
			return;
		}
		if(gem == obj) {
			tell_from_jeweler(ch, jeweler, "Quello e' l'oggetto da incastonare, non una pietra.");
			return;
		}
		const int vnum = obj_vnum(gem);
		const auto gem_entry = find_gem(vnum);
		if(!gem_entry || vnum < kGemVnumMin || vnum > kGemVnumMax) {
			tell_from_jeweler(ch, jeweler, "Quello non e' una pietra da incastonare.");
			return;
		}
		const GemCatalogEntry& def = gem_entry->get();
		if(is_weapon_item(obj) && !def.weapon_ok) {
			tell_from_jeweler(ch, jeweler,
							  "La pietra '" + std::string(def.material) + "' non si incastona sulle armi.");
			return;
		}

		int need = def.qty;
		GemExtra extra = is_weapon_item(obj) ? def.extra_weapon : def.extra_other;
		int loc = is_weapon_item(obj) ? def.loc_weapon : def.loc_other;
		int mod = is_weapon_item(obj) ? def.mod_weapon : def.mod_other;

		if(def.zircone_special) {
			const int have = count_inv_vnum(ch, def.vnum, reserved);
			if(have >= 3) {
				need = 3;
				extra = GemExtra::Artefact;
				loc = APPLY_NONE;
				mod = 0;
			}
			else if(have == 1) {
				need = 1;
				extra = GemExtra::Resistant;
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
		plan.def = &def;
		plan.consumed = need;
		plan.loc = loc;
		plan.mod = mod;
		plan.extra = extra;
		plan.color = pick_color(def);
		plan.value = value_for_slot(def, need);
		plan.stones = {};

		for(int s = 0; s < need; s++) {
			obj_data* stone = (s == 0)
				? gem
				: find_inv_by_vnum(ch, def.vnum, reserved);
			if(!stone) {
				tell_from_jeweler(ch, jeweler,
							  "Non hai abbastanza pietre di " + std::string(def.material)
							  + " (ne servono " + std::to_string(need) + ").");
				return;
			}
			if(s == 0 && already_reserved(stone, reserved)) {
				stone = find_inv_by_vnum(ch, def.vnum, reserved);
				if(!stone) {
					tell_from_jeweler(ch, jeweler, "Non hai abbastanza pietre.");
					return;
				}
			}
			plan.stones[static_cast<std::size_t>(s)] = stone;
			reserved.push_back(stone);
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
		tell_from_jeweler(ch, jeweler,
						  "Su questo pezzo restano solo " + std::to_string(nslots)
						  + " incavi liberi: le altre pietre restano nella tua borsa.");
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
			const std::string stone_name = std::string(obj_short_name(stone, "una pietra"));
			if(jeweler) {
				const std::string room_msg = "$n incastona $c0015" + stone_name + "$c0007 su $c0015$p$c0007.";
				act(room_msg.c_str(), TRUE, actor, obj, 0, TO_ROOM);
				act(rand_reaction[number(10, nRandReac + 10)], TRUE, actor, obj, 0, TO_ROOM);
			}
			else {
				send_to_char(("Incastoni $c0015" + stone_name + "$c0007 su $c0015"
							  + std::string(obj_short_name(obj, "l'oggetto")) + "$c0007.\n\r").c_str(), ch);
				act(rand_reaction[number(0, nRandReac)], TRUE, ch, obj, 0, TO_CHAR);
				const std::string room_msg = "$n incastona $c0015" + stone_name + "$c0007 su $c0015$p$c0007.";
				act(room_msg.c_str(), TRUE, ch, obj, 0, TO_ROOM);
				act(rand_reaction[number(10, nRandReac + 10)], TRUE, ch, obj, 0, TO_ROOM);
			}
			obj_from_char(stone);
			extract_obj(stone);
		}
	}

	const int val_orig = obj->obj_flags.cost;
	int aff = 0;
	ColorPalette colore{};

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
		colore[static_cast<std::size_t>(aff)] = plan.color;
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
					std::string& rest) {
	char who[MAX_INPUT_LENGTH];
	const char* p = one_argument(arg, who);
	if(!*who) {
		return false;
	}
	struct char_data* vict = get_char_room_vis(ch, who);
	if(vict != mob) {
		return false;
	}
	rest.clear();
	if(p) {
		while(*p == ' ') {
			p++;
		}
		if(*p) {
			rest.assign(p);
		}
	}
	return true;
}


ACTION_FUNC(do_incastona) {
	if(!ch) {
		return;
	}
	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		send_to_char("Chi ti pensi di essere? Un gioielliere? Sei solo uno stupido mob!\n\r", ch);
		return;
	}
	send_to_char("Non c'e' nessun incastonatore in grado di aiutarti qui.\n\r", ch);
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
		std::string rest;
		if(!ask_is_for_mob(ch, arg, mob, rest)) {
			return FALSE;
		}
		char topic[MAX_INPUT_LENGTH];
		const char* p = one_argument(rest.c_str(), topic);
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
