/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* Spec proc mob editoriali:
 * - Incastonatore: listino pietre + incastona
 * - EditAffectBroker: trasferisci affect tra edit / distruggi edit PERSONAL
 *
 * Incastonatore: il PG tiene oggetto e pietre con se'; il mob lavora sul banco.
 * Comando: incastona <oggetto> <pietra> [pietra ...]
 * Ask <mob> aiuto | listino. Preview + si/no/nod/shake prima di cesellare.
 *
 * EditAffectBroker (myst.spe: M 3017 EditAffectBroker):
 * Ask <mob> trasferisci <effetto> <objA> <objB>  — anteprima, poi ask <mob> si|no
 * Ask <mob> distruggi <objC>
 *
 * Tetti pezzo su B (edit e broker separati, sommati una sola volta ciascuno):
 * - HR effective (HITROLL+HITNDAM+HITNSP): edit <= +2, broker <= +2
 * - DAM effective (DAMROLL+HITNDAM):       edit <= +2, broker <= +2
 * - SP effective (SPELLPOWER+HITNSP):      edit <= +2, broker <= +2
 * - STR/DEX/INT/WIS/CHR:                   edit <= +3, broker <= +3
 * - ARMOR: edit <= -40, broker <= -40 (delta sempre negativo; totale fino a -80)
 * Toon-wide: DAM+SP edit <= 30 (HITNDAM→dam, HITNSP→sp). Armi/slayer/eat → staff.
 *
 * Fee transfer/distruggi = 25% del listino (scale × class_mult × artifact).
 * Stessa unita' di GET_EXP / portale: niente /HowManyClasses sul 25%.
 */
#include <functional>
#include <map>
#include <set>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <optional>
#include <sstream>
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
#include "act.obj_wear.hpp"
#include "act.other.hpp"
#include "clan_symbol.hpp"
#include "cmdid.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "multiclass.hpp"
#include "obj_value.hpp"
#include "object_instance.hpp"
#include "procarea.hpp"
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

/* Come one_argument: lower-case, salta filler (a/the/...), senza buffer stack. */
[[nodiscard]] std::pair<std::string, std::string_view> next_arg(std::string_view input) {
	for(;;) {
		while(!input.empty()
			  && std::isspace(static_cast<unsigned char>(input.front()))) {
			input.remove_prefix(1);
		}
		if(input.empty()) {
			return {"", {}};
		}
		std::size_t len = 0;
		while(len < input.size() && static_cast<unsigned char>(input[len]) > ' ') {
			++len;
		}
		std::string word;
		word.reserve(len);
		for(std::size_t i = 0; i < len; ++i) {
			word.push_back(LOWER(input[i]));
		}
		input.remove_prefix(len);
		if(!fill_word(word.c_str())) {
			return {std::move(word), input};
		}
	}
}

/** Riassegna un campo char* del mud da std::string (libera il precedente).
 *  stringa vuota → nullptr. */
void set_obj_cstr(char*& field, const std::string& value) {
	if(field) {
		free(field);
		field = nullptr;
	}
	if(!value.empty()) {
		field = strdup(value.c_str());
	}
}

/** mudlog e' un macro su token (mudlog_##level): non si puo' passare un int. */
void editor_mudlog(e_log_levels level, const std::string& msg) {
	const char* const s = msg.c_str();
	switch(level) {
	case LOG_SYSERR:
		mudlog(LOG_SYSERR, "%s", s);
		break;
	case LOG_ERROR:
		mudlog(LOG_ERROR, "%s", s);
		break;
	case LOG_PLAYERS:
		mudlog(LOG_PLAYERS, "%s", s);
		break;
	case LOG_CHECK:
		mudlog(LOG_CHECK, "%s", s);
		break;
	default:
		mudlog(LOG_CHECK, "%s", s);
		break;
	}
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
	if(!ch) {
		return;
	}
	if(jeweler) {
		const std::string buf = std::string("$N ti dice '") + std::string(msg) + "'";
		act(buf.c_str(), FALSE, ch, 0, jeweler, TO_CHAR);
	}
	else {
		send_to_char((std::string(msg) + "\n\r").c_str(), ch);
	}
}


constexpr time_t kNameInciseTimeoutSec = 75;
constexpr time_t kAmbientIntervalSec = 60;

struct NameInciseOffer {
	char_data* jeweler{nullptr};
	obj_data* obj{nullptr};
	time_t expires_at{0};
};

std::map<char_data*, NameInciseOffer> g_name_incise_offers;

/* Stato ambient per mob: PC gia' visti in stanza + ultimo rumor. */
struct JewelerAmbientState {
	std::set<char_data*> seen_pcs;
	time_t last_say{0};
};

std::map<char_data*, JewelerAmbientState> g_jeweler_ambient;

void say_jeweler_ambient_line(char_data* mob) {
	if(!mob) {
		return;
	}
	switch(number(0, 3)) {
	case 0:
		say_multiline_to_room(mob, {
			"Posate il pezzo sul banco: ci lavoro io,",
			"senza prenderlo in consegna."
		});
		break;
	case 1:
		say_multiline_to_room(mob, {
			"Se non sapete da dove iniziare,",
			"$c0015chiedetemi aiuto$c0010."
		});
		break;
	case 2:
		say_multiline_to_room(mob, {
			"Volete sapere gli effetti delle pietre?",
			"Chiedetemi il $c0015listino$c0010."
		});
		break;
	default:
		say_multiline_to_room(mob, {
			"Il comando e' $c0015incastona$c0010, poi pezzo e pietre.",
			"Opale e ossidiana ne vogliono due, il quarzo rosa tre."
		});
		break;
	}
}

void say_jeweler_welcome(char_data* mob) {
	if(!mob) {
		return;
	}
	say_multiline_to_room(mob, {
		"Benvenuti al banco.",
		"Per un intarsio $c0015chiedetemi aiuto$c0010 o il $c0015listino$c0010."
	});
}

/* Ingresso (PC nuovo rispetto al tick precedente) + rumor ogni ~60s. */
void incastonatore_ambient_tick(char_data* mob) {
	if(!mob || !AWAKE(mob) || mob->specials.fighting) {
		return;
	}
	struct room_data* rp = real_roomp(mob->in_room);
	if(!rp) {
		return;
	}

	std::set<char_data*> now_pcs;
	for(struct char_data* t = rp->people; t; t = t->next_in_room) {
		if(IS_PC(t) && t != mob) {
			now_pcs.insert(t);
		}
	}

	JewelerAmbientState& st = g_jeweler_ambient[mob];
	if(now_pcs.empty()) {
		st.seen_pcs.clear();
		return;
	}

	bool entered = false;
	for(char_data* pc : now_pcs) {
		if(st.seen_pcs.find(pc) == st.seen_pcs.end()) {
			entered = true;
			break;
		}
	}

	const time_t now = time(nullptr);
	if(entered) {
		say_jeweler_welcome(mob);
		st.last_say = now; /* anti-spam: niente ambient subito dopo il saluto */
	}
	else if(st.last_say == 0 || (now - st.last_say) >= kAmbientIntervalSec) {
		say_jeweler_ambient_line(mob);
		st.last_say = now;
	}

	st.seen_pcs = std::move(now_pcs);
}

[[nodiscard]] bool obj_in_carrying(char_data* ch, obj_data* obj) {
	if(!ch || !obj) {
		return false;
	}
	for(obj_data* o = ch->carrying; o; o = o->next_content) {
		if(o == obj) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] bool name_incise_offer_valid(char_data* ch, const NameInciseOffer& offer) {
	if(!ch || !offer.jeweler || !offer.obj) {
		return false;
	}
	if(ch->in_room != offer.jeweler->in_room) {
		return false;
	}
	if(!obj_in_carrying(ch, offer.obj)) {
		return false;
	}
	if(IS_OBJ_STAT2(offer.obj, ITEM2_PERSONAL)) {
		return false;
	}
	return true;
}

void ask_name_incise_question(char_data* ch, char_data* jeweler) {
	if(!ch) {
		return;
	}
	tell_from_jeweler(ch, jeweler,
					  "Vuoi che incida il tuo nome nell'oggetto? Dimmi si o no, oppure annuisci o scuoti la testa.");
}

void start_name_incise_offer(char_data* ch, char_data* jeweler, obj_data* obj) {
	if(!ch || !jeweler || !obj) {
		return;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_PERSONAL)) {
		return;
	}
	g_name_incise_offers[ch] = NameInciseOffer{jeweler, obj, time(nullptr) + kNameInciseTimeoutSec};
	ask_name_incise_question(ch, jeweler);
}

void cancel_name_incise_offer(char_data* ch, char_data* jeweler, bool notify) {
	if(!ch) {
		return;
	}
	auto it = g_name_incise_offers.find(ch);
	if(it == g_name_incise_offers.end()) {
		return;
	}
	char_data* j = jeweler ? jeweler : it->second.jeweler;
	g_name_incise_offers.erase(it);
	if(notify && j) {
		tell_from_jeweler(ch, j, "Va bene, lascio il pezzo senza il tuo nome.");
	}
}

enum class YesNoAnswer { Yes, No, Other };

[[nodiscard]] YesNoAnswer parse_yes_no(std::string_view text) {
	const std::string word = next_arg(text).first;
	if(word.empty()) {
		return YesNoAnswer::Other;
	}
	if(word == "si" || word == "s" || word == "yes" || word == "y" || word == "nod") {
		return YesNoAnswer::Yes;
	}
	if(word == "no" || word == "n" || word == "shake") {
		return YesNoAnswer::No;
	}
	return YesNoAnswer::Other;
}

/* true = risposta gestita (consuma comando). */
bool try_handle_name_incise_answer(char_data* ch, char_data* mob, std::string_view text) {
	if(!ch || !mob) {
		return false;
	}
	auto it = g_name_incise_offers.find(ch);
	if(it == g_name_incise_offers.end()) {
		return false;
	}
	NameInciseOffer offer = it->second;
	if(offer.jeweler != mob) {
		return false;
	}
	if(time(nullptr) > offer.expires_at || !name_incise_offer_valid(ch, offer)) {
		cancel_name_incise_offer(ch, mob, true);
		return true;
	}
	switch(parse_yes_no(text)) {
	case YesNoAnswer::Yes:
		g_name_incise_offers.erase(it);
		if(!IS_OBJ_STAT2(offer.obj, ITEM2_PERSONAL)) {
			pers_obj(mob, ch, offer.obj, CMD_PERSONALIZE);
		}
		tell_from_jeweler(ch, mob, "Fatto: il tuo nome e' inciso nel pezzo.");
		schedule_inventory_save(ch);
		return true;
	case YesNoAnswer::No:
		cancel_name_incise_offer(ch, mob, true);
		return true;
	case YesNoAnswer::Other:
		ask_name_incise_question(ch, mob);
		return true;
	}
	return true;
}

void sweep_name_incise_offers_for_mob(char_data* mob) {
	if(!mob) {
		return;
	}
	const time_t now = time(nullptr);
	for(auto it = g_name_incise_offers.begin(); it != g_name_incise_offers.end();) {
		char_data* client = it->first;
		const NameInciseOffer& offer = it->second;
		if(offer.jeweler != mob) {
			++it;
			continue;
		}
		if(now > offer.expires_at || !name_incise_offer_valid(client, offer)) {
			char_data* j = offer.jeweler;
			it = g_name_incise_offers.erase(it);
			if(client && j && client->in_room == j->in_room) {
				tell_from_jeweler(client, j, "Va bene, lascio il pezzo senza il tuo nome.");
			}
		}
		else {
			++it;
		}
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
	if(!ch || !keyword || !*keyword) {
		return nullptr;
	}
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
	if(!ch) {
		return nullptr;
	}
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
	if(!ch) {
		return 0;
	}
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
	if(!ch) {
		return;
	}
	if(jeweler) {
		act("$c0011$N$c0007 solleva lo sguardo dal banco, con polvere di gemme sulle dita.",
			FALSE, ch, 0, jeweler, TO_CHAR);
		act("$N parla a bassa voce con $n, indicando il banco da lavoro.",
			FALSE, ch, 0, jeweler, TO_NOTVICT);
		say_multiline_to_char(ch, jeweler, {
			"Posa l'arma o il gioiello sul mio banco,",
			"ma non affidarmelo: ci lavoro io, mentre resta tuo.",
			"Le pietre restano nella tua borsa. Le prendo io, una ad una,",
			"quando mi dici quale intarsio vuoi.",
			"Quando sei pronto, dimmi: $c0015incastona$c0011 seguito dal nome del pezzo",
			"e da quello delle pietre, una per ogni incavo.",
			"Al piu' cinque incavi, meno quelli gia' sul pezzo.",
			"Opale e ossidiana ne chiedono due, il quarzo rosa tre.",
			"Lo zircone: una sola pietra per la resistenza, tre per l'artifact.",
			"Prima di cesellare ti mostro l'intarsio e attendo $c0015si$c0011 o $c0015no$c0011,",
			"oppure un cenno del capo ($c0015nod$c0011) o uno scuotere la testa ($c0015shake$c0011).",
			"Se vuoi vedere gli effetti, $c0015chiedimi listino$c0011.",
			"Per queste parole, $c0015chiedimi aiuto$c0011."
		});
		return;
	}
	send_to_char("$c0015insert$c0007 / $c0015incastona$c0007 <oggetto> <pietra> [pietra ...]\n\r"
				 "Oggetto e pietre nel tuo inventario. Max 5 incavi. Zircone: 1 resistent, 3 artifact.\n\r",
				 ch);
}

void show_listino(char_data* ch, char_data* jeweler) {
	if(!ch) {
		return;
	}
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
	if(!ch || !obj) {
		return false;
	}
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
	if(!obj) {
		return;
	}
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
	if(!obj || aff <= 0) {
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
	if(!obj || GET_ITEM_TYPE(obj) != ITEM_WEAPON) {
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

struct MountOffer {
	char_data* jeweler{nullptr};
	obj_data* obj{nullptr};
	SlotPlan slots[kMaxSlots]{};
	int nslots{};
	int wait{};
	time_t expires_at{};
	std::string leftover;
};

std::map<char_data*, MountOffer> g_mount_offers;

[[nodiscard]] const char* slot_effect_label(const SlotPlan& plan, bool weapon) {
	if(plan.extra == GemExtra::Artefact) {
		return "artifact";
	}
	if(plan.extra == GemExtra::Resistant) {
		return "resistent";
	}
	if(plan.extra == GemExtra::Invisible && weapon) {
		return "invisible (flag)";
	}
	if(!plan.def) {
		return "?";
	}
	return (weapon ? plan.def->desc_weapon : plan.def->desc_other).data();
}

[[nodiscard]] bool mount_offer_valid(char_data* ch, const MountOffer& offer) {
	if(!ch || !offer.jeweler || !offer.obj || offer.nslots <= 0) {
		return false;
	}
	if(ch->in_room != offer.jeweler->in_room) {
		return false;
	}
	if(!obj_in_carrying(ch, offer.obj)) {
		return false;
	}
	if(IS_OBJ_STAT2(offer.obj, ITEM2_INSERT) || IS_OBJ_STAT2(offer.obj, ITEM2_EDIT)
	   || offer.obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
		return false;
	}
	for(int i = 0; i < offer.nslots; i++) {
		for(int s = 0; s < offer.slots[i].consumed; s++) {
			if(!obj_in_carrying(ch, offer.slots[i].stones[static_cast<std::size_t>(s)])) {
				return false;
			}
		}
	}
	return true;
}

void cancel_mount_offer(char_data* ch, char_data* jeweler, bool notify) {
	if(!ch) {
		return;
	}
	auto it = g_mount_offers.find(ch);
	if(it == g_mount_offers.end()) {
		return;
	}
	char_data* j = jeweler ? jeweler : it->second.jeweler;
	g_mount_offers.erase(it);
	if(notify && j) {
		tell_from_jeweler(ch, j, "Va bene, non tocco nulla. Pietre e pezzo restano tuoi.");
	}
}

void show_mount_preview(char_data* ch, char_data* jeweler, const MountOffer& offer) {
	if(!ch || !offer.obj) {
		return;
	}
	const std::string oname(obj_short_name(offer.obj, "il pezzo"));
	const bool weapon = is_weapon_item(offer.obj);
	{
		std::ostringstream os;
		os << "$c0011Ecco l'intarsio che farei su " << oname
		   << ", prima di toccare nulla:$c0007";
		tell_from_jeweler(ch, jeweler, os.str());
	}
	int hnd = 0;
	int added = 0;
	for(int i = 0; i < offer.nslots; i++) {
		const SlotPlan& plan = offer.slots[i];
		added += plan.value;
		if(plan.loc == APPLY_HITNDAM) {
			hnd += plan.mod;
		}
		const char* mat = plan.def ? plan.def->material.data() : "pietra";
		std::ostringstream line;
		line << "  $c0012" << mat << "$c0007 x" << plan.consumed << "  —  $c0015"
			 << slot_effect_label(plan, weapon) << "$c0007\n\r";
		send_to_char(line.str().c_str(), ch);
	}
	if(weapon && hnd > 1) {
		std::ostringstream fuse;
		fuse << "  I bonus hit-n-dam si fondono in un solo $c0015+" << hnd << "/+" << hnd
			 << "$c0007.\n\r";
		send_to_char(fuse.str().c_str(), ch);
	}
	if(!offer.leftover.empty()) {
		tell_from_jeweler(ch, jeweler, offer.leftover);
	}
	const int new_cost = (offer.obj->obj_flags.cost + added < LIM_ITEM_COST_MIN)
		? LIM_ITEM_COST_MIN
		: offer.obj->obj_flags.cost + added;
	{
		std::ostringstream os;
		os << "Il pezzo verra' considerato raro (valore " << new_cost
		   << "). Conferma con $c0015si$c0007 / $c0015nod$c0007, rinuncia con $c0015no$c0007 / "
			  "$c0015shake$c0007.";
		tell_from_jeweler(ch, jeweler, os.str());
	}
}

void incastona_apply(char_data* ch, char_data* jeweler, obj_data* obj,
					 SlotPlan* slots, int nslots, int wait);

void start_mount_offer(char_data* ch, char_data* jeweler, obj_data* obj,
					   const SlotPlan* slots, int nslots, int wait, std::string leftover) {
	if(!ch || !jeweler || !obj || !slots || nslots <= 0) {
		return;
	}
	MountOffer offer{};
	offer.jeweler = jeweler;
	offer.obj = obj;
	offer.nslots = nslots;
	offer.wait = wait;
	offer.expires_at = time(nullptr) + kNameInciseTimeoutSec;
	offer.leftover = std::move(leftover);
	for(int i = 0; i < nslots; i++) {
		offer.slots[i] = slots[i];
	}
	g_mount_offers[ch] = offer;
	show_mount_preview(ch, jeweler, offer);
}

bool try_handle_mount_confirm(char_data* ch, char_data* mob, std::string_view text,
							  bool consume_other) {
	if(!ch || !mob) {
		return false;
	}
	auto it = g_mount_offers.find(ch);
	if(it == g_mount_offers.end()) {
		return false;
	}
	if(it->second.jeweler != mob) {
		return false;
	}
	MountOffer offer = it->second;
	if(time(nullptr) > offer.expires_at || !mount_offer_valid(ch, offer)) {
		cancel_mount_offer(ch, mob, true);
		return true;
	}
	switch(parse_yes_no(text)) {
	case YesNoAnswer::Yes:
		g_mount_offers.erase(it);
		if(!object_can_be_mounted(ch, mob, offer.obj) || !mount_offer_valid(ch, offer)) {
			tell_from_jeweler(ch, mob, "Qualcosa e' cambiato: non posso piu' fare quell'intarsio.");
			return true;
		}
		incastona_apply(ch, mob, offer.obj, offer.slots, offer.nslots, offer.wait);
		return true;
	case YesNoAnswer::No:
		cancel_mount_offer(ch, mob, true);
		return true;
	case YesNoAnswer::Other:
		if(!consume_other) {
			return false;
		}
		tell_from_jeweler(ch, mob, "Attendo un si o un no, un cenno del capo o uno scuotere la testa.");
		show_mount_preview(ch, mob, offer);
		return true;
	}
	return true;
}

void sweep_mount_offers_for_mob(char_data* mob) {
	if(!mob) {
		return;
	}
	const time_t now = time(nullptr);
	for(auto it = g_mount_offers.begin(); it != g_mount_offers.end();) {
		char_data* client = it->first;
		const MountOffer& offer = it->second;
		if(offer.jeweler != mob) {
			++it;
			continue;
		}
		if(now > offer.expires_at || !mount_offer_valid(client, offer)) {
			char_data* j = offer.jeweler;
			it = g_mount_offers.erase(it);
			if(client && j && client->in_room == j->in_room) {
				tell_from_jeweler(client, j, "Va bene, non tocco nulla. Pietre e pezzo restano tuoi.");
			}
		}
		else {
			++it;
		}
	}
}

void incastona_execute(struct char_data* ch, struct char_data* jeweler, const char* arg) {
	if(!ch) {
		return;
	}
	std::string_view rest = arg ? arg : "";
	auto [objname, after_obj] = next_arg(rest);
	rest = after_obj;
	if(objname.empty()) {
		show_usage(ch, jeweler);
		return;
	}
	if(objname == "listino" || objname == "aiuto" || objname == "help") {
		if(objname == "listino") {
			show_listino(ch, jeweler);
		}
		else {
			show_usage(ch, jeweler);
		}
		return;
	}

	struct obj_data* obj = get_obj_in_list_vis(ch, objname.c_str(), ch->carrying);
	if(!obj) {
		tell_from_jeweler(ch, jeweler,
						  "Non vedo quel pezzo tra le tue cose. Deve essere con te, qui al banco.");
		return;
	}
	if(!object_can_be_mounted(ch, jeweler, obj)) {
		std::ostringstream log;
		log << (GET_NAME(ch) ? GET_NAME(ch) : "?") << " incastona refused on "
			<< obj_short_name(obj, "?");
		editor_mudlog(LOG_PLAYERS, log.str());
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
		auto [gemma, after_gem] = next_arg(rest);
		rest = after_gem;
		if(gemma.empty()) {
			if(i == 0) {
				tell_from_jeweler(ch, jeweler, "Quale pietra vuoi incastonare?");
				return;
			}
			break;
		}

		obj_data* gem = find_inv_by_keyword(ch, gemma.c_str(), reserved);
		if(!gem) {
			tell_from_jeweler(ch, jeweler,
							  "Non hai niente che si chiami '" + gemma + "' con te.");
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

	std::string leftover;
	const std::string extra_gem = next_arg(rest).first;
	if(!extra_gem.empty() && nslots > 0) {
		leftover = "Su questo pezzo restano solo " + std::to_string(nslots)
			+ " incavi liberi: le altre pietre restano nella tua borsa.";
	}

	if(nslots <= 0) {
		tell_from_jeweler(ch, jeweler, "Quale pietra vuoi incastonare?");
		return;
	}

	if((count_weapon_spells(obj) + incoming_wps) > 1) {
		tell_from_jeweler(ch, jeweler, "Un'arma puo' avere una sola weapon spell.");
		return;
	}

	if(jeweler) {
		start_mount_offer(ch, jeweler, obj, slots, nslots, wait, leftover);
		return;
	}
	if(!leftover.empty()) {
		tell_from_jeweler(ch, jeweler, leftover);
	}
	incastona_apply(ch, jeweler, obj, slots, nslots, wait);
}

void incastona_apply(char_data* ch, char_data* jeweler, obj_data* obj,
					 SlotPlan* slots, int nslots, int wait) {
	if(!ch || !obj || !slots || nslots <= 0) {
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

	/* Domanda si/no per incidere il nome (solo mob; insert immortale no). */
	if(jeweler && !IS_OBJ_STAT2(obj, ITEM2_PERSONAL)) {
		start_name_incise_offer(ch, jeweler, obj);
	}

	{
		std::ostringstream log;
		log << (GET_NAME(ch) ? GET_NAME(ch) : "?") << " incastona " << aff << " slot su "
			<< obj_short_name(obj, "?") << " (jeweler="
			<< ((jeweler && GET_NAME(jeweler)) ? GET_NAME(jeweler) : "self") << ")";
		editor_mudlog(LOG_PLAYERS, log.str());
	}
	schedule_inventory_save(ch);
}

bool ask_is_for_mob(struct char_data* ch, const char* arg, struct char_data* mob,
					std::string& rest) {
	if(!ch || !mob) {
		return false;
	}
	const auto [who, after] = next_arg(arg ? arg : "");
	if(who.empty()) {
		return false;
	}
	struct char_data* vict = get_char_room_vis(ch, who.c_str());
	if(vict != mob) {
		return false;
	}
	rest.assign(after.begin(), after.end());
	while(!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) {
		rest.erase(rest.begin());
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
		sweep_mount_offers_for_mob(mob);
		sweep_name_incise_offers_for_mob(mob);
		incastonatore_ambient_tick(mob);
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

	/* Se esce dalla stanza mentre aspetta una conferma, annulla. */
	if(cmd >= CMD_NORTH && cmd <= CMD_DOWN) {
		auto mit = g_mount_offers.find(ch);
		if(mit != g_mount_offers.end() && mit->second.jeweler == mob) {
			cancel_mount_offer(ch, mob, true);
		}
		auto it = g_name_incise_offers.find(ch);
		if(it != g_name_incise_offers.end() && it->second.jeweler == mob) {
			cancel_name_incise_offer(ch, mob, true);
		}
		return FALSE;
	}
	if(cmd == CMD_FLEE) {
		auto mit = g_mount_offers.find(ch);
		if(mit != g_mount_offers.end() && mit->second.jeweler == mob) {
			cancel_mount_offer(ch, mob, true);
		}
		auto it = g_name_incise_offers.find(ch);
		if(it != g_name_incise_offers.end() && it->second.jeweler == mob) {
			cancel_name_incise_offer(ch, mob, true);
		}
		return FALSE;
	}

	if(cmd == CMD_NOD || cmd == CMD_SHAKE) {
		const char* answer = (cmd == CMD_NOD) ? "si" : "no";
		if(try_handle_mount_confirm(ch, mob, answer, false)) {
			return TRUE;
		}
		if(try_handle_name_incise_answer(ch, mob, answer)) {
			return TRUE;
		}
		return FALSE;
	}

	if(cmd == CMD_SAY || cmd == CMD_SAY_APICE) {
		std::string_view speech = arg ? arg : "";
		while(!speech.empty() && std::isspace(static_cast<unsigned char>(speech.front()))) {
			speech.remove_prefix(1);
		}
		if(try_handle_mount_confirm(ch, mob, speech, true)) {
			return TRUE;
		}
		if(try_handle_name_incise_answer(ch, mob, speech)) {
			return TRUE;
		}
		return FALSE;
	}

	if(cmd == CMD_ASK) {
		std::string rest;
		if(!ask_is_for_mob(ch, arg, mob, rest)) {
			return FALSE;
		}
		if(try_handle_mount_confirm(ch, mob, rest, false)) {
			return TRUE;
		}
		if(try_handle_name_incise_answer(ch, mob, rest)) {
			return TRUE;
		}
		const auto [topic, after] = next_arg(rest);
		if(topic.empty() || topic == "aiuto" || topic == "help" || topic == "incastona") {
			if(topic == "incastona" && !after.empty()) {
				std::string_view mount_args = after;
				while(!mount_args.empty()
					  && std::isspace(static_cast<unsigned char>(mount_args.front()))) {
					mount_args.remove_prefix(1);
				}
				if(!mount_args.empty()) {
					incastona_from_command(ch, std::string(mount_args).c_str(), mob);
					return TRUE;
				}
			}
			show_usage(ch, mob);
			return TRUE;
		}
		if(topic == "listino" || topic == "pietre" || topic == "gemme") {
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

/* =========================================================================
 * EditAffectBroker — trasferimento affect tra due edit / soft-delete edit
 * Aggancio: myst.spe  M 3017 EditAffectBroker
 * ========================================================================= */
namespace {

constexpr long kEditBrokerPrinceFloor = PRINCEEXP;
constexpr int kEditBrokerPercentKeep = 25; /* pagamento / rimborso = 25% del listino */

struct AffectPick {
	int location{APPLY_NONE};
	int delta{0}; /* modifier numerico oppure bitmask da trasferire */
	std::string label; /* nome umano per messaggi/history */
};

[[nodiscard]] std::string normalize_affect_key(std::string_view in) {
	std::string out;
	out.reserve(in.size());
	for(unsigned char c : in) {
		if(std::isalnum(c)) {
			out.push_back(static_cast<char>(UPPER(c)));
		}
	}
	return out;
}

[[nodiscard]] bool is_bitfield_location(int loc) noexcept {
	return loc == APPLY_AFF2 || loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE ||
		   loc == APPLY_SPELL || loc == APPLY_SUSC;
}

/** Affect che non si sommano: serve uno slot libero sul target. */
[[nodiscard]] bool is_nonstackable_location(int loc) noexcept {
	return is_bitfield_location(loc) || loc == APPLY_WEAPON_SPELL || loc == APPLY_EAT_SPELL ||
		   loc == APPLY_RACE_SLAYER || loc == APPLY_ALIGN_SLAYER;
}

/** Armi / slayer / eat: broker rifiuta, si rivolgono allo staff. */
[[nodiscard]] bool is_staff_only_broker_location(int loc) noexcept {
	return loc == APPLY_WEAPON_SPELL || loc == APPLY_EAT_SPELL ||
		   loc == APPLY_RACE_SLAYER || loc == APPLY_ALIGN_SLAYER;
}

/** Numerici accorpabili entro tetti edit+broker. */
[[nodiscard]] bool is_mergeable_numeric_location(int loc) noexcept {
	return loc == APPLY_HITROLL || loc == APPLY_DAMROLL || loc == APPLY_SPELLPOWER ||
		   loc == APPLY_HITNDAM || loc == APPLY_HITNSP || loc == APPLY_AC ||
		   loc == APPLY_STR || loc == APPLY_DEX || loc == APPLY_INT || loc == APPLY_WIS ||
		   loc == APPLY_CHR;
}

/* Maschere listino (allineate a object_instance show edits). */
constexpr unsigned long kBrokerResiBits =
	IMM_FIRE | IMM_COLD | IMM_ELEC | IMM_ENERGY | IMM_BLUNT | IMM_PIERCE | IMM_SLASH |
	IMM_ACID | IMM_POISON | IMM_DRAIN | IMM_HOLD;
constexpr unsigned long kBrokerImmuneBits = IMM_DRAIN | IMM_CHARM | IMM_POISON;
constexpr unsigned long kBrokerSpellBits =
	AFF_TELEPATHY | AFF_GLOBE_DARKNESS | AFF_WATERBREATH | AFF_TRUE_SIGHT |
	AFF_INVISIBLE | AFF_SENSE_LIFE | AFF_SCRYING | AFF_PROTECT_FROM_EVIL | AFF_FLYING;
constexpr unsigned long kBrokerSpell2Bits = AFF2_DANGER_SENSE;

constexpr int kBrokerPieceCombatCap = 2;
constexpr int kBrokerPieceStatCap = 3;
constexpr int kBrokerPieceArmorCap = 40;

[[nodiscard]] unsigned long broker_allowed_bits_for(int loc) noexcept {
	if(loc == APPLY_IMMUNE) {
		return kBrokerResiBits;
	}
	if(loc == APPLY_M_IMMUNE) {
		return kBrokerImmuneBits;
	}
	if(loc == APPLY_SPELL) {
		return kBrokerSpellBits;
	}
	if(loc == APPLY_AFF2) {
		return kBrokerSpell2Bits;
	}
	return 0ul;
}

[[nodiscard]] long sum_affect_location(const struct obj_data* obj, int loc) {
	long sum = 0;
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			sum += obj->affected[i].modifier;
		}
	}
	return sum;
}

[[nodiscard]] int obj_delta_vs_proto(const struct obj_data* obj, const struct obj_data* proto,
									int loc) {
	if(!obj || !proto) {
		return 0;
	}
	const long cur = sum_affect_location(obj, loc);
	const long base = sum_affect_location(proto, loc);
	if(loc == APPLY_AC) {
		return static_cast<int>(cur - base); /* negativo = miglioramento */
	}
	return static_cast<int>(std::max(0L, cur - base));
}

[[nodiscard]] int broker_delta_clamped(unsigned long long iid, int loc, int total_delta) {
	const long raw = object_instance_sum_broker_delta(iid, loc);
	if(loc == APPLY_AC) {
		/* total_delta e broker sono negativi se miglioramento. */
		if(total_delta >= 0) {
			return 0;
		}
		return static_cast<int>(std::max(static_cast<long>(total_delta),
										 std::min(0L, raw)));
	}
	if(total_delta <= 0) {
		return 0;
	}
	return static_cast<int>(std::max(0L, std::min(static_cast<long>(total_delta), raw)));
}

struct PieceCombatBreakdown {
	int hr_total{};
	int dam_total{};
	int sp_total{};
	int hr_broker{};
	int dam_broker{};
	int sp_broker{};
};

[[nodiscard]] PieceCombatBreakdown piece_combat_breakdown(const struct obj_data* obj,
														  const struct obj_data* proto) {
	PieceCombatBreakdown b {};
	if(!obj || !proto) {
		return b;
	}
	const unsigned long long iid = obj->db_instance_id;
	const int d_hr = obj_delta_vs_proto(obj, proto, APPLY_HITROLL);
	const int d_dam = obj_delta_vs_proto(obj, proto, APPLY_DAMROLL);
	const int d_sp = obj_delta_vs_proto(obj, proto, APPLY_SPELLPOWER);
	const int d_hd = obj_delta_vs_proto(obj, proto, APPLY_HITNDAM);
	const int d_hs = obj_delta_vs_proto(obj, proto, APPLY_HITNSP);
	const int br_hr = broker_delta_clamped(iid, APPLY_HITROLL, d_hr);
	const int br_dam = broker_delta_clamped(iid, APPLY_DAMROLL, d_dam);
	const int br_sp = broker_delta_clamped(iid, APPLY_SPELLPOWER, d_sp);
	const int br_hd = broker_delta_clamped(iid, APPLY_HITNDAM, d_hd);
	const int br_hs = broker_delta_clamped(iid, APPLY_HITNSP, d_hs);
	b.hr_total = d_hr + d_hd + d_hs;
	b.dam_total = d_dam + d_hd;
	b.sp_total = d_sp + d_hs;
	b.hr_broker = br_hr + br_hd + br_hs;
	b.dam_broker = br_dam + br_hd;
	b.sp_broker = br_sp + br_hs;
	return b;
}

[[nodiscard]] unsigned or_affect_bits_on_obj(const struct obj_data* obj, int loc) {
	unsigned bits = 0;
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			bits |= static_cast<unsigned>(obj->affected[i].modifier);
		}
	}
	return bits;
}

[[nodiscard]] bool has_affect_location(const struct obj_data* obj, int loc) {
	if(!obj) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc && obj->affected[i].modifier != 0) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] int find_free_affect_slot(const struct obj_data* obj) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == APPLY_NONE || obj->affected[i].location == 0) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] int find_location_slot(const struct obj_data* obj, int loc) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] struct obj_data* load_edit_prototype(const struct obj_data* obj) {
	if(!obj) {
		return nullptr;
	}
	int iVNum = object_instance_resolve_base_vnum(obj);
	if(iVNum <= 0) {
		iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	}
	const int rNum = real_object(iVNum);
	if(rNum < 0) {
		return nullptr;
	}
	return read_object(rNum, REAL);
}

[[nodiscard]] std::string apply_display_name(int loc) {
	char buf[MAX_STRING_LENGTH];
	sprinttype(loc, apply_types, buf);
	return buf;
}

[[nodiscard]] std::string bit_display_name(int loc, unsigned bits) {
	char buf[MAX_STRING_LENGTH];
	buf[0] = '\0';
	if(loc == APPLY_SPELL) {
		sprintbit(bits, affected_bits, buf);
	}
	else if(loc == APPLY_AFF2) {
		sprintbit(bits, affected_bits2, buf);
	}
	else if(loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE || loc == APPLY_SUSC) {
		sprintbit(bits, immunity_names, buf);
	}
	else {
		return std::to_string(bits);
	}
	return buf;
}

[[nodiscard]] bool match_bit_table(const char* names[], const std::string& key,
								   unsigned& bit_out) {
	if(!names || key.empty()) {
		return false;
	}
	for(int i = 0; names[i] && names[i][0] != '\n'; ++i) {
		if(normalize_affect_key(names[i]) == key) {
			bit_out = (1u << i);
			return true;
		}
	}
	return false;
}

/**
 * Risolve il nome dato dal toon in un affect presente come delta vs prototipo su src.
 * Accetta nomi apply_types (HITROLL, DAMROLL, SPELL AFFECT, ...) oppure bit
 * (DARKNESS, FIRE, ...).
 */
/**
 * Accetta "str", "str by 2", "STR by 2": toglie un eventuale suffisso "by N"
 * (come nei messaggi di aiuto) e restituisce l'ammontare richiesto se presente.
 */
[[nodiscard]] std::string strip_by_amount(std::string_view raw, int* requested_amt) {
	if(requested_amt) {
		*requested_amt = 0;
	}
	std::string cleaned(raw);
	/* trim trailing spaces */
	while(!cleaned.empty() && std::isspace(static_cast<unsigned char>(cleaned.back()))) {
		cleaned.pop_back();
	}
	const auto pos = cleaned.rfind(" by ");
	if(pos == std::string::npos) {
		const auto pos2 = cleaned.rfind(" BY ");
		if(pos2 == std::string::npos) {
			return cleaned;
		}
		const std::string num = cleaned.substr(pos2 + 4);
		char* end = nullptr;
		const long v = std::strtol(num.c_str(), &end, 10);
		if(end != num.c_str() && *end == '\0' && v != 0) {
			if(requested_amt) {
				*requested_amt = static_cast<int>(v);
			}
			return cleaned.substr(0, pos2);
		}
		return cleaned;
	}
	const std::string num = cleaned.substr(pos + 4);
	char* end = nullptr;
	const long v = std::strtol(num.c_str(), &end, 10);
	if(end != num.c_str() && *end == '\0' && v != 0) {
		if(requested_amt) {
			*requested_amt = static_cast<int>(v);
		}
		return cleaned.substr(0, pos);
	}
	return cleaned;
}

[[nodiscard]] bool resolve_affect_pick(struct obj_data* src, const std::string& raw_name,
									   AffectPick& out, std::string& err) {
	out = AffectPick{};
	int requested_amt = 0;
	const std::string name_core = strip_by_amount(raw_name, &requested_amt);
	const std::string key = normalize_affect_key(name_core);
	if(key.empty()) {
		err = "Devi specificare il nome dell'effetto da trasferire.";
		return false;
	}

	struct obj_data* proto = load_edit_prototype(src);
	if(!proto) {
		err = "Non riesco a confrontare l'oggetto con il suo prototipo.";
		return false;
	}

	auto finish_fail = [&](const char* msg) {
		extract_obj(proto);
		err = msg;
		return false;
	};

	/* 1) Match su apply_types (location intera). */
	int matched_loc = APPLY_NONE;
	for(int i = 0; apply_types[i] && apply_types[i][0] != '\n'; ++i) {
		if(normalize_affect_key(apply_types[i]) == key) {
			matched_loc = i;
			break;
		}
	}

	if(matched_loc != APPLY_NONE && matched_loc != APPLY_SKIP) {
		if(is_bitfield_location(matched_loc)) {
			const unsigned added =
				or_affect_bits_on_obj(src, matched_loc) &
				~or_affect_bits_on_obj(proto, matched_loc);
			if(added == 0) {
				return finish_fail(
					"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
			}
			out.location = matched_loc;
			out.delta = static_cast<int>(added);
			out.label = apply_display_name(matched_loc) + " " +
						bit_display_name(matched_loc, added);
			extract_obj(proto);
			return true;
		}

		const long cur = sum_affect_location(src, matched_loc);
		const long base = sum_affect_location(proto, matched_loc);
		long delta = 0;
		if(matched_loc == APPLY_AC) {
			if(cur < base) {
				delta = cur - base; /* negativo = miglioramento */
			}
		}
		else if(cur > base) {
			delta = cur - base;
		}
		if(delta == 0) {
			return finish_fail(
				"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
		}
		out.location = matched_loc;
		out.delta = static_cast<int>(delta);
		if(requested_amt != 0) {
			if(matched_loc == APPLY_AC) {
				const int want =
					requested_amt < 0 ? requested_amt : -std::abs(requested_amt);
				if(out.delta < 0 && want < 0) {
					out.delta = std::max(out.delta, want);
				}
			}
			else if(out.delta > 0 && requested_amt > 0) {
				out.delta = std::min(out.delta, requested_amt);
			}
		}
		out.label = apply_display_name(matched_loc) + " by " + std::to_string(out.delta);
		extract_obj(proto);
		return true;
	}

	/* 1b) Label lista / identify: "SPELL AFFECT INVISIBLE DETECT-EVIL ..." */
	{
		std::vector<std::string> words;
		std::string_view rest = name_core;
		for(;;) {
			auto [w, next] = next_arg(rest);
			if(w.empty()) {
				break;
			}
			words.push_back(std::move(w));
			rest = next;
		}
		const int max_prefix = std::min(3, static_cast<int>(words.size()));
		for(int nprefix = max_prefix; nprefix >= 1; --nprefix) {
			std::string cand;
			for(int i = 0; i < nprefix; ++i) {
				if(i != 0) {
					cand.push_back(' ');
				}
				cand += words[static_cast<std::size_t>(i)];
			}
			const std::string ckey = normalize_affect_key(cand);
			int loc = APPLY_NONE;
			for(int i = 0; apply_types[i] && apply_types[i][0] != '\n'; ++i) {
				if(is_bitfield_location(i) && normalize_affect_key(apply_types[i]) == ckey) {
					loc = i;
					break;
				}
			}
			if(loc == APPLY_NONE) {
				continue;
			}

			const unsigned added =
				or_affect_bits_on_obj(src, loc) & ~or_affect_bits_on_obj(proto, loc);
			if(added == 0) {
				return finish_fail(
					"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
			}

			unsigned want = 0;
			if(static_cast<int>(words.size()) == nprefix) {
				want = added;
			}
			else {
				bool bits_ok = true;
				for(std::size_t wi = static_cast<std::size_t>(nprefix); wi < words.size();
					++wi) {
					unsigned b = 0;
					const std::string bk = normalize_affect_key(words[wi]);
					bool matched = false;
					if(loc == APPLY_SPELL) {
						matched = match_bit_table(affected_bits, bk, b);
					}
					else if(loc == APPLY_AFF2) {
						matched = match_bit_table(affected_bits2, bk, b);
					}
					else {
						matched = match_bit_table(immunity_names, bk, b);
					}
					if(!matched) {
						bits_ok = false;
						break;
					}
					want |= b;
				}
				if(!bits_ok) {
					continue;
				}
				want &= added;
				if(want == 0) {
					return finish_fail(
						"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al "
						"prototipo.");
				}
			}

			out.location = loc;
			out.delta = static_cast<int>(want);
			out.label =
				apply_display_name(loc) + " " + bit_display_name(loc, want);
			extract_obj(proto);
			return true;
		}
	}

	/* 2) Match su singolo bit (SPELL / IMMUNE / AFF2). */
	unsigned bit = 0;
	int bit_loc = APPLY_NONE;
	if(match_bit_table(affected_bits, key, bit)) {
		bit_loc = APPLY_SPELL;
	}
	else if(match_bit_table(affected_bits2, key, bit)) {
		bit_loc = APPLY_AFF2;
	}
	else if(match_bit_table(immunity_names, key, bit)) {
		const unsigned add_imm =
			or_affect_bits_on_obj(src, APPLY_IMMUNE) &
			~or_affect_bits_on_obj(proto, APPLY_IMMUNE);
		const unsigned add_mimm =
			or_affect_bits_on_obj(src, APPLY_M_IMMUNE) &
			~or_affect_bits_on_obj(proto, APPLY_M_IMMUNE);
		const unsigned add_susc =
			or_affect_bits_on_obj(src, APPLY_SUSC) &
			~or_affect_bits_on_obj(proto, APPLY_SUSC);
		if(add_imm & bit) {
			bit_loc = APPLY_IMMUNE;
		}
		else if(add_mimm & bit) {
			bit_loc = APPLY_M_IMMUNE;
		}
		else if(add_susc & bit) {
			bit_loc = APPLY_SUSC;
		}
		else {
			bit_loc = APPLY_IMMUNE;
		}
	}

	if(bit_loc == APPLY_NONE) {
		return finish_fail(
			"Non riconosco quel nome di effetto. Usa il nome come in stat (es. DAMROLL, DARKNESS).");
	}

	const unsigned added =
		or_affect_bits_on_obj(src, bit_loc) & ~or_affect_bits_on_obj(proto, bit_loc);
	if((added & bit) == 0) {
		return finish_fail(
			"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
	}

	out.location = bit_loc;
	out.delta = static_cast<int>(bit);
	out.label = apply_display_name(bit_loc) + " " + bit_display_name(bit_loc, bit);
	extract_obj(proto);
	return true;
}

[[nodiscard]] bool obj_is_owned_edit(struct char_data* ch, struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_EDIT) && IS_OBJ_STAT2(obj, ITEM2_PERSONAL) &&
		   pers_on(ch, obj);
}

/** B PERSONAL di un altro toon: blocco (niente riassegnazione). */
[[nodiscard]] bool obj_personal_owned_by_other(struct char_data* ch, struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && !pers_on(ch, obj);
}

/**
 * Dry-run delle restrizioni wear rilevanti (class/anti/sesso/prince/clan),
 * senza indossare ne' side-effect (no drop barb).
 */
[[nodiscard]] bool toon_can_use_obj(struct char_data* ch, struct obj_data* obj,
									std::string& err) {
	if(!ch || !obj) {
		err = "Oggetto non valido.";
		return false;
	}
	struct char_data* tch = ch;
	if(IS_POLY(ch) && ch->desc && ch->desc->original) {
		tch = ch->desc->original;
	}

	if(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && !pers_on(ch, obj)) {
		err = "Non puoi usare quell'oggetto: non ti appartiene.";
		return false;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_NO_PRINCE) && IS_PRINCE(tch)) {
		err = "Sei troppo potente per usare quell'oggetto.";
		return false;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_ONLY_PRINCE) && !IS_PRINCE(tch)) {
		err = "Quell'oggetto e' troppo potente per te.";
		return false;
	}

	const int bitMask = static_cast<int>(GetItemClassRestrictions(obj));
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ONLY_CLASS)) {
		int mask = bitMask;
		if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
			mask |= CLASS_SORCERER;
		}
		if(!OnlyClass(tch, mask)) {
			err = "Non sei la persona adatta a usare quell'oggetto.";
			return false;
		}
	}
	else if(IsRestricted(obj, tch->player.iClass)) {
		err = "Non riesci a usare quell'oggetto (restrizioni di classe).";
		return false;
	}

	if(anti_barbarian_stuff(obj) && GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
	   GetMaxLevel(ch) < IMMORTALE) {
		err = "Percepisci magia su quell'oggetto: un barbaro non puo' usarlo.";
		return false;
	}
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MEN) && GET_SEX(ch) != SEX_FEMALE) {
		err = "Solo le femmine possono utilizzare quell'oggetto.";
		return false;
	}
	if(IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_WOMEN) && GET_SEX(ch) != SEX_MALE) {
		err = "Solo i maschi possono utilizzare quell'oggetto.";
		return false;
	}
	if(obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL && !clan_symbol_can_wear(ch, obj)) {
		err = "Non puoi usare quel simbolo di casata.";
		return false;
	}
	return true;
}

[[nodiscard]] long percent_of_listino(long listino_cost) {
	if(listino_cost <= 0) {
		return 0;
	}
	/* 25% listino: evita (x*25)/100 che con -Werror=strict-overflow fallisce. */
	static_assert(kEditBrokerPercentKeep == 25, "percent_of_listino assume 25%");
	return listino_cost / 4;
}

/*
 * Listino = scale × class_mult × +50% artifact (EditAffectDeltaListinoCost).
 * Addebito/rimborso = 25% di quel listino, come il portale (stesso campo exp).
 * Non dividere per HowManyClasses: class_mult e' gia' nel listino.
 */

[[nodiscard]] bool can_afford_prince_floor(struct char_data* ch, long cost) {
	if(cost <= 0) {
		return true;
	}
	return (static_cast<long long>(GET_EXP(ch)) - static_cast<long long>(cost)) >=
		   kEditBrokerPrinceFloor;
}

[[nodiscard]] int resolve_obj_base_vnum(struct obj_data* obj) {
	if(!obj) {
		return 0;
	}
	int base = object_instance_resolve_base_vnum(obj);
	if(base <= 0 && obj->item_number >= 0 && obj->item_number <= top_of_objt) {
		base = obj_index[obj->item_number].iVNum;
	}
	return base;
}

/** Equivalente interno di `osave <obj> db procarea` (senza comando wiz). */
[[nodiscard]] bool persist_procarea_reward_snapshot(struct obj_data* obj,
													 struct char_data* actor,
													 std::string& err) {
	if(!obj || !procarea_obj_is_reward(obj)) {
		err = "Non e' un premio procarea.";
		return false;
	}
	if(obj->db_instance_id != 0) {
		return true;
	}
	const int base_vnum = resolve_obj_base_vnum(obj);
	if(base_vnum <= 0 || !procarea_is_reward_vnum(base_vnum)) {
		err = "Non riesco a dedurre il prototipo base del premio procarea.";
		return false;
	}
	const int old_rnum = obj->item_number;
	const bool already_exempt = object_is_zone_limit_exempt(obj);
	const unsigned long long id = object_instance_persist(obj, base_vnum, 0, actor, true);
	if(id == 0) {
		err = "Salvataggio del premio procarea nel database fallito.";
		return false;
	}
	if(obj->char_vnum == 0 ||
	   (obj->char_vnum >= LOW_EDITED_ITEMS && obj->char_vnum <= HIGH_EDITED_ITEMS)) {
		obj->char_vnum = base_vnum;
	}
	SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
	SET_BIT(obj->obj_flags.extra_flags2, ITEM2_PROCAREA_REWARD);
	const int base_rnum = real_object(base_vnum);
	if(base_rnum >= 0) {
		obj->item_number = base_rnum;
	}
	if(!already_exempt) {
		object_exclude_from_zone_limit(obj, old_rnum);
	}
	/* Ternary fuori da mudlog: FORMAT/% mangia ?: */
	{
		std::ostringstream log;
		log << "EditAffectBroker procarea snapshot inst="
			<< static_cast<unsigned long long>(id) << " base=" << base_vnum
			<< " actor=" << ((actor && GET_NAME(actor)) ? GET_NAME(actor) : "?");
		editor_mudlog(LOG_PLAYERS, log.str());
	}
	return true;
}

/** Dopo il transfer: B diventa PERSONAL+EDIT+ARTIFACT del toon (se non lo e' gia'). */
void ensure_b_personal_edit(struct char_data* ch, struct char_data* mob,
							struct obj_data* obj_b) {
	if(!ch || !obj_b) {
		return;
	}
	if(!IS_OBJ_STAT2(obj_b, ITEM2_PERSONAL)) {
		pers_obj(mob ? mob : ch, ch, obj_b, CMD_PERSONALIZE);
	}
	else if(!pers_on(ch, obj_b)) {
		/* Gia' bloccato a monte; non riassegnare. */
		return;
	}
	SET_BIT(obj_b->obj_flags.extra_flags2, ITEM2_EDIT);
	if(!IS_OBJ_STAT(obj_b, ITEM_IMMUNE)) {
		SET_BIT(obj_b->obj_flags.extra_flags, ITEM_IMMUNE); /* ARTIFACT */
	}
	strncpy(obj_b->personal_owner, GET_NAME(ch), sizeof(obj_b->personal_owner) - 1);
	obj_b->personal_owner[sizeof(obj_b->personal_owner) - 1] = '\0';
}

void remove_numeric_delta(struct obj_data* obj, int loc, int delta) {
	if(!obj || delta == 0) {
		return;
	}
	if(delta > 0) {
		int left = delta;
		for(int i = 0; i < MAX_OBJ_AFFECT && left > 0; ++i) {
			if(obj->affected[i].location != loc) {
				continue;
			}
			const int take = std::min(obj->affected[i].modifier, left);
			obj->affected[i].modifier -= take;
			left -= take;
			if(obj->affected[i].modifier == 0) {
				obj->affected[i].location = APPLY_NONE;
			}
		}
		return;
	}
	/* Delta negativo (es. armor): togli miglioramento riportando i modifier verso 0. */
	int left = -delta;
	for(int i = 0; i < MAX_OBJ_AFFECT && left > 0; ++i) {
		if(obj->affected[i].location != loc || obj->affected[i].modifier >= 0) {
			continue;
		}
		const int avail = -obj->affected[i].modifier;
		const int take = std::min(avail, left);
		obj->affected[i].modifier += take;
		left -= take;
		if(obj->affected[i].modifier == 0) {
			obj->affected[i].location = APPLY_NONE;
		}
	}
}

void add_numeric_delta(struct obj_data* obj, int loc, int delta) {
	if(!obj || delta == 0) {
		return;
	}
	int slot = find_location_slot(obj, loc);
	if(slot < 0) {
		slot = find_free_affect_slot(obj);
		if(slot < 0) {
			return;
		}
		obj->affected[slot].location = static_cast<short>(loc);
		obj->affected[slot].modifier = 0;
	}
	obj->affected[slot].modifier += delta;
}

void remove_bit_delta(struct obj_data* obj, int loc, unsigned bits) {
	if(!obj || bits == 0) {
		return;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location != loc) {
			continue;
		}
		obj->affected[i].modifier =
			static_cast<int>(static_cast<unsigned>(obj->affected[i].modifier) & ~bits);
		if(obj->affected[i].modifier == 0) {
			obj->affected[i].location = APPLY_NONE;
		}
	}
}

[[nodiscard]] bool add_bit_delta_new_slot(struct obj_data* obj, int loc, unsigned bits) {
	if(!obj || bits == 0) {
		return false;
	}
	int slot = find_location_slot(obj, loc);
	if(slot >= 0) {
		obj->affected[slot].modifier =
			static_cast<int>(static_cast<unsigned>(obj->affected[slot].modifier) | bits);
		return true;
	}
	slot = find_free_affect_slot(obj);
	if(slot < 0) {
		return false;
	}
	obj->affected[slot].location = static_cast<short>(loc);
	obj->affected[slot].modifier = static_cast<int>(bits);
	return true;
}

[[nodiscard]] bool persist_edit_obj(struct obj_data* obj, struct char_data* actor,
									const char* kind = nullptr, const char* note = nullptr,
									const char* detail = nullptr, bool mark_edit = true) {
	if(!obj) {
		return false;
	}
	const int base = resolve_obj_base_vnum(obj);
	if(base <= 0) {
		return false;
	}
	/* mark_edit=false sul path undo post-restore: non ri-marcare ITEM2_EDIT. */
	if(mark_edit) {
		SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
	}
	const unsigned long long id =
		object_instance_persist(obj, base, obj->db_instance_id, actor, true);
	if(id == 0) {
		return false;
	}
	if(kind && *kind) {
		object_instance_append_event(id, kind, note, detail, nullptr, actor);
	}
	return true;
}

/**
 * Snapshot pre-transfer di A/B (affect + personalizzazione/testo di B)
 * per rollback se il persist fallisce.
 */
struct BrokerTransferSnap {
	obj_affected_type affects_a[MAX_OBJ_AFFECT]{};
	obj_affected_type affects_b[MAX_OBJ_AFFECT]{};
	unsigned int extra_flags_b{};
	unsigned int extra_flags2_b{};
	/* Allineato a obj_data::personal_owner[32] in structs.hpp. */
	char personal_owner_b[32]{};
	std::string name_b;
	std::string short_b;
	std::string desc_b;

	[[nodiscard]] static BrokerTransferSnap capture(const obj_data* obj_a,
													const obj_data* obj_b) {
		BrokerTransferSnap s;
		static_assert(sizeof(s.personal_owner_b) == 32, "personal_owner size mismatch");
		if(obj_a) {
			std::memcpy(s.affects_a, obj_a->affected, sizeof(s.affects_a));
		}
		if(obj_b) {
			std::memcpy(s.affects_b, obj_b->affected, sizeof(s.affects_b));
			s.extra_flags_b = obj_b->obj_flags.extra_flags;
			s.extra_flags2_b = obj_b->obj_flags.extra_flags2;
			std::memcpy(s.personal_owner_b, obj_b->personal_owner,
						sizeof(s.personal_owner_b));
			s.name_b = obj_b->name ? obj_b->name : "";
			s.short_b = obj_b->short_description ? obj_b->short_description : "";
			s.desc_b = obj_b->description ? obj_b->description : "";
		}
		return s;
	}

	void restore(obj_data* obj_a, obj_data* obj_b) const {
		if(obj_a) {
			std::memcpy(obj_a->affected, affects_a, sizeof(affects_a));
		}
		if(!obj_b) {
			return;
		}
		std::memcpy(obj_b->affected, affects_b, sizeof(affects_b));
		obj_b->obj_flags.extra_flags = extra_flags_b;
		obj_b->obj_flags.extra_flags2 = extra_flags2_b;
		std::memcpy(obj_b->personal_owner, personal_owner_b, sizeof(personal_owner_b));
		set_obj_cstr(obj_b->name, name_b);
		set_obj_cstr(obj_b->short_description, short_b);
		set_obj_cstr(obj_b->description, desc_b);
	}
};

[[nodiscard]] std::vector<std::string> collect_transferable_delta_labels(struct obj_data* obj) {
	std::vector<std::string> out;
	if(!obj) {
		return out;
	}
	struct obj_data* proto = load_edit_prototype(obj);
	if(!proto) {
		return out;
	}

	for(int loc = 0; apply_types[loc] && apply_types[loc][0] != '\n'; ++loc) {
		if(loc == APPLY_NONE || loc == APPLY_SKIP) {
			continue;
		}
		if(is_bitfield_location(loc)) {
			const unsigned added =
				or_affect_bits_on_obj(obj, loc) & ~or_affect_bits_on_obj(proto, loc);
			if(added == 0) {
				continue;
			}
			/* Una sola riga per location bitfield (come in identify: tutti i bit
			 * sulla stessa affect). Si puo' comunque trasferire un singolo bit
			 * nominandolo (es. invisible). */
			std::string bits = bit_display_name(loc, added);
			while(!bits.empty() &&
				  (bits.back() == ' ' || bits.back() == '\r' || bits.back() == '\n')) {
				bits.pop_back();
			}
			out.push_back(apply_display_name(loc) + " " + bits);
			continue;
		}
		const long cur = sum_affect_location(obj, loc);
		const long base = sum_affect_location(proto, loc);
		long delta = 0;
		if(loc == APPLY_AC) {
			if(cur < base) {
				delta = cur - base;
			}
		}
		else if(cur > base) {
			delta = cur - base;
		}
		if(delta != 0) {
			out.push_back(apply_display_name(loc) + " by " + std::to_string(delta));
		}
	}

	extract_obj(proto);
	/* Dedup preservando ordine */
	std::vector<std::string> uniq;
	uniq.reserve(out.size());
	for(const auto& s : out) {
		if(std::find(uniq.begin(), uniq.end(), s) == uniq.end()) {
			uniq.push_back(s);
		}
	}
	return uniq;
}

void show_edit_broker_usage(struct char_data* ch, struct char_data* mob) {
	if(!ch) {
		return;
	}
	tell_from_jeweler(ch, mob,
					  "Posso trasferire un effetto da un tuo pezzo EDIT a un altro oggetto, "
					  "oppure distruggere un edit.");
	send_to_char(
		"$c0015Comandi:\n\r"
		"  $c0010ask$c0007 <me> $c0011trasferisci$c0007 <effetto> <oggettoA> <oggettoB>\n\r"
		"  $c0010ask$c0007 <me> $c0011distruggi$c0007 <oggettoC>\n\r"
		"  $c0010ask$c0007 <me> $c0011aiuto$c0007 [<oggettoA>]  — elenco effetti (tutti gli EDIT "
		"PERSONAL in inv, oppure uno solo)\n\r"
		"$c0015A:$c0007 deve essere EDIT, PERSONAL e tuo.\n\r"
		"$c0015B:$c0007 non deve esserlo gia'; dopo il transfer diventa EDIT/PERSONAL tuo.\n\r"
		"$c0015Trasferimento:$c0007 costa il 25% del listino di quell'effetto "
		"(gia' con bonus classi e artifact).\n\r"
		"$c0015Tetti pezzo (edit + broker, una volta ciascuno):$c0007 "
		"HR/DAM/SP effective +2+2 (hit-n-dam / hit-n-sp inclusi); "
		"STR/DEX/INT/WIS/CHR +3+3; armor -40+-40 (sempre negativo).\n\r"
		"$c0015Distruggi:$c0007 elimini l'edit e ricevi il 25% di quanto e' stato pagato "
		"per editare l'intero oggetto (il valore in piu' rispetto al pezzo originale).\n\r"
		"Non puoi scendere sotto i 400 milioni di esperienza.\n\r"
		"Prima di ogni operazione ti mostro un riepilogo: conferma con $c0011si$c0007 o "
		"$c0011nod$c0007, annulla con $c0011no$c0007 o $c0011shake$c0007 "
		"(funzionano anche $c0011say$c0007 e $c0011ask$c0007).\n\r",
		ch);
	if(ch && GetMaxLevel(ch) >= IMMORTALE) {
		send_to_char(
			"$c0008[wiz] Toon: DAM+SP edit <= 30. Armi: chiedi allo staff.\n\r"
			"$c0008[wiz] I premi PROCAREA-REWARD senza instance vengono registrati in "
			"automatico prima del transfer.\n\r",
			ch);
	}
}

constexpr time_t kEditBrokerPendingTimeoutSec = 90;

enum class EditBrokerOpKind { Transfer, Destroy };

struct EditBrokerPending {
	char_data* jeweler{};
	EditBrokerOpKind op{};
	time_t expires_at{};
	std::string aff_name;
	std::string name_a;
	std::string name_b;
	std::string name_c;
};

std::map<char_data*, EditBrokerPending> g_edit_broker_pending;

struct TransferPlan {
	obj_data* obj_a{};
	obj_data* obj_b{};
	AffectPick pick;
	long listino{};
	long fee{};
	long payment{};
	int classes{};
	/** Delta edit gia' su B vs proto prima del broker (AC: negativo se migliorato). */
	int b_prior_edit_delta{};
	int existing_cap{};
	int transfer_cap{};
	int total_cap{};
	bool had_stack_cap{};
};

struct DestroyPlan {
	obj_data* obj{};
	unsigned long long inst{};
	std::string shortn;
	long listino_diff{};
	long fee{};
	long refund{};
	int classes{};
};

[[nodiscard]] const char* obj_shortn(const obj_data* obj) {
	return (obj && obj->short_description) ? obj->short_description : "?";
}

void cancel_edit_broker_pending(char_data* ch, char_data* jeweler, bool notify) {
	if(!ch) {
		return;
	}
	auto it = g_edit_broker_pending.find(ch);
	if(it == g_edit_broker_pending.end()) {
		return;
	}
	char_data* j = jeweler ? jeweler : it->second.jeweler;
	g_edit_broker_pending.erase(it);
	if(notify && j) {
		tell_from_jeweler(ch, j, "Operazione annullata.");
	}
}

[[nodiscard]] bool edit_broker_pending_room_ok(char_data* ch, const EditBrokerPending& p) {
	return ch && p.jeweler && ch->in_room == p.jeweler->in_room;
}

enum class BrokerConfirmAnswer { Yes, No, Other };

[[nodiscard]] BrokerConfirmAnswer parse_broker_confirm(std::string_view text) {
	const std::string word = next_arg(text).first;
	if(word.empty()) {
		return BrokerConfirmAnswer::Other;
	}
	if(word == "si" || word == "s" || word == "yes" || word == "y" || word == "conferma" ||
	   word == "ok" || word == "nod") {
		return BrokerConfirmAnswer::Yes;
	}
	if(word == "no" || word == "n" || word == "annulla" || word == "shake") {
		return BrokerConfirmAnswer::No;
	}
	return BrokerConfirmAnswer::Other;
}

void ask_edit_broker_confirm(char_data* ch, char_data* mob) {
	if(!ch) {
		return;
	}
	tell_from_jeweler(ch, mob,
					  "Confermi? $c0011si$c0007 / $c0011nod$c0007, oppure $c0011no$c0007 / "
					  "$c0011shake$c0007 (anche via say o ask).");
}

void supersede_edit_broker_pending(char_data* ch, char_data* mob) {
	if(!ch) {
		return;
	}
	auto it = g_edit_broker_pending.find(ch);
	if(it == g_edit_broker_pending.end()) {
		return;
	}
	if(it->second.jeweler == mob) {
		tell_from_jeweler(ch, mob, "Annuo la richiesta precedente.");
	}
	g_edit_broker_pending.erase(it);
}

[[nodiscard]] bool build_transfer_plan(struct char_data* ch, struct char_data* mob,
									   std::string_view aff_name, std::string_view name_a,
									   std::string_view name_b, TransferPlan& out) {
	if(!ch || !mob) {
		return false;
	}
	if(aff_name.empty() || name_a.empty() || name_b.empty()) {
		tell_from_jeweler(ch, mob, "Sintassi: trasferisci <effetto> <oggettoA> <oggettoB>.");
		return false;
	}

	const std::string name_a_str{name_a};
	const std::string name_b_str{name_b};
	struct obj_data* obj_a =
		get_obj_in_list_vis(ch, name_a_str.c_str(), ch->carrying);
	struct obj_data* obj_b =
		get_obj_in_list_vis(ch, name_b_str.c_str(), ch->carrying);
	if(!obj_a) {
		tell_from_jeweler(ch, mob, "Non vedo l'oggetto A nel tuo inventario.");
		return false;
	}
	if(!obj_b) {
		tell_from_jeweler(ch, mob, "Non vedo l'oggetto B nel tuo inventario.");
		return false;
	}
	if(obj_a == obj_b) {
		tell_from_jeweler(ch, mob, "Oggetto A e oggetto B devono essere due pezzi distinti.");
		return false;
	}
	if(!obj_is_owned_edit(ch, obj_a)) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto A deve essere EDIT, PERSONAL e di tua proprieta'.");
		return false;
	}
	const char* const actor_name = GET_NAME(ch) ? GET_NAME(ch) : "?";
	if(obj_a->db_instance_id == 0) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto A non e' collegato al database edit. Contatta uno staffer.");
		std::ostringstream log;
		log << "EditAffectBroker transfer: missing instance_id A owner=" << actor_name;
		editor_mudlog(LOG_ERROR, log.str());
		return false;
	}
	if(obj_personal_owned_by_other(ch, obj_b)) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto B e' PERSONAL di un altro personaggio: non posso "
						  "lavorarci.");
		std::ostringstream log;
		log << "EditAffectBroker transfer denied " << actor_name << ": B owned by other";
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	std::string use_err;
	if(!toon_can_use_obj(ch, obj_b, use_err)) {
		tell_from_jeweler(ch, mob, use_err);
		std::ostringstream log;
		log << "EditAffectBroker transfer denied " << actor_name << ": B not usable ("
			<< use_err << ")";
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	AffectPick pick;
	std::string err;
	if(!resolve_affect_pick(obj_a, std::string(aff_name), pick, err)) {
		tell_from_jeweler(ch, mob, err);
		std::ostringstream log;
		log << "EditAffectBroker transfer denied " << actor_name << ": " << err
			<< " (affect=" << aff_name << " A=" << obj_shortn(obj_a) << ")";
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	if(is_staff_only_broker_location(pick.location)) {
		tell_from_jeweler(ch, mob,
						  "Questo effetto (arma / slayer / eat-spell) non si trasferisce "
						  "dal broker: rivolgiti a un membro dello staff.");
		std::ostringstream log;
		log << "EditAffectBroker transfer denied " << actor_name << ": staff-only "
			<< pick.label;
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	/* Bitfield: solo bit listino; bit gia' su B → stop; altrimenti OR nello stesso slot. */
	if(is_bitfield_location(pick.location)) {
		const unsigned long allowed = broker_allowed_bits_for(pick.location);
		const unsigned xfer = static_cast<unsigned>(pick.delta);
		if(allowed == 0ul || (static_cast<unsigned long>(xfer) & ~allowed) != 0ul) {
			tell_from_jeweler(ch, mob,
							  "Questo effetto non e' tra quelli trasferibili dal broker: "
							  "rivolgiti a un membro dello staff.");
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": bit not allowlisted " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
		const unsigned on_b = or_affect_bits_on_obj(obj_b, pick.location);
		const unsigned overlap = on_b & xfer;
		if(overlap != 0) {
			std::ostringstream os;
			os << "L'oggetto B ha gia' questo effetto ("
			   << bit_display_name(pick.location, overlap)
			   << "): non posso trasferirlo.";
			tell_from_jeweler(ch, mob, os.str());
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": bit already on B " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
		if(find_location_slot(obj_b, pick.location) < 0 &&
		   find_free_affect_slot(obj_b) < 0) {
			tell_from_jeweler(ch, mob,
							  "L'oggetto B non ha uno slot libero per questo effetto.");
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": no free slot on B for " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
	}
	else if(is_nonstackable_location(pick.location)) {
		if(has_affect_location(obj_b, pick.location)) {
			tell_from_jeweler(ch, mob,
							  "L'oggetto B ha gia' questo effetto: non posso trasferirlo.");
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": nonstack already on B " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
		if(find_free_affect_slot(obj_b) < 0) {
			tell_from_jeweler(ch, mob,
							  "L'oggetto B non ha uno slot libero per questo effetto.");
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": no free slot on B for " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
	}
	else if(!has_affect_location(obj_b, pick.location) &&
			find_free_affect_slot(obj_b) < 0) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto B non ha gia' lo stesso effetto ne' uno slot libero: "
						  "non posso sommarlo.");
		std::ostringstream log;
		log << "EditAffectBroker transfer denied " << actor_name << ": B no slot for "
			<< pick.label;
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	/*
	 * Tetti pezzo: edit e broker separati (una sola somma ciascuno).
	 * Combat effective come show edits; stats +3/+3; armor -40/-40.
	 */
	int existing_cap = 0;
	int transfer_cap = 0;
	int total_cap = 0;
	int b_prior_edit_delta = 0;
	bool had_stack_cap = false;

	auto refuse_cap = [&](std::string_view msg) {
		tell_from_jeweler(ch, mob, msg);
		std::ostringstream log;
		log << "EditAffectBroker transfer denied "
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": cap " << msg
			<< " affect=" << pick.label;
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	};

	if(is_mergeable_numeric_location(pick.location)) {
		struct obj_data* proto_b = load_edit_prototype(obj_b);
		if(!proto_b) {
			tell_from_jeweler(ch, mob,
							  "Non riesco a confrontare l'oggetto B con il suo prototipo.");
			return false;
		}
		had_stack_cap = true;
		const unsigned long long iid_b = obj_b->db_instance_id;

		auto apply_positive_broker_cap = [&](int edit_cap, int broker_cap, int total_now,
											int broker_now, int want, const char* what,
											int& take_out) -> bool {
			const int edit_now = std::max(0, total_now - broker_now);
			if(edit_now > edit_cap) {
				std::ostringstream os;
				os << "L'oggetto B supera gia' il tetto di edit su " << what
				   << " (massimo " << edit_cap << ").";
				return refuse_cap(os.str());
			}
			const int room = broker_cap - broker_now;
			take_out = std::min(want, room);
			if(take_out <= 0) {
				std::ostringstream os;
				os << "Non posso trasferire altro " << what
				   << ": B e' al tetto brokeraggio (" << edit_cap << " di edit + "
				   << broker_cap << " di broker).";
				return refuse_cap(os.str());
			}
			existing_cap = edit_cap;
			transfer_cap = broker_cap;
			total_cap = edit_cap + broker_cap;
			return true;
		};

		if(pick.location == APPLY_AC) {
			existing_cap = kBrokerPieceArmorCap;
			transfer_cap = kBrokerPieceArmorCap;
			total_cap = kBrokerPieceArmorCap * 2;
			const int total_delta = obj_delta_vs_proto(obj_b, proto_b, APPLY_AC);
			const int broker_delta = broker_delta_clamped(iid_b, APPLY_AC, total_delta);
			const int total_improve = std::max(0, -total_delta);
			const int broker_improve = std::max(0, -broker_delta);
			const int edit_improve = std::max(0, total_improve - broker_improve);
			b_prior_edit_delta = -edit_improve;
			if(edit_improve > kBrokerPieceArmorCap) {
				extract_obj(proto_b);
				return refuse_cap(
					"L'oggetto B supera gia' il tetto di edit su armor (massimo -40).");
			}
			const int want = pick.delta < 0 ? -pick.delta : 0;
			const int room = kBrokerPieceArmorCap - broker_improve;
			const int take = std::min(want, room);
			if(take <= 0) {
				extract_obj(proto_b);
				return refuse_cap(
					"Non posso trasferire altro armor: B e' al tetto brokeraggio "
					"(-40 di edit + -40 di broker).");
			}
			pick.delta = -take;
			pick.label = apply_display_name(pick.location) + " by " +
						 std::to_string(pick.delta);
		}
		else if(pick.location == APPLY_STR || pick.location == APPLY_DEX ||
				pick.location == APPLY_INT || pick.location == APPLY_WIS ||
				pick.location == APPLY_CHR) {
			const int total_now = obj_delta_vs_proto(obj_b, proto_b, pick.location);
			const int broker_now =
				broker_delta_clamped(iid_b, pick.location, total_now);
			b_prior_edit_delta = std::max(0, total_now - broker_now);
			const int want = pick.delta > 0 ? pick.delta : 0;
			int take = 0;
			if(!apply_positive_broker_cap(kBrokerPieceStatCap, kBrokerPieceStatCap,
										 total_now, broker_now, want,
										 apply_display_name(pick.location).c_str(),
										 take)) {
				extract_obj(proto_b);
				return false;
			}
			pick.delta = take;
			pick.label = apply_display_name(pick.location) + " by " +
						 std::to_string(pick.delta);
		}
		else {
			/* Combat: HITROLL / DAMROLL / SPELLPOWER / HITNDAM / HITNSP. */
			const PieceCombatBreakdown cb = piece_combat_breakdown(obj_b, proto_b);
			const int want = pick.delta > 0 ? pick.delta : 0;
			int take = want;
			existing_cap = kBrokerPieceCombatCap;
			transfer_cap = kBrokerPieceCombatCap;
			total_cap = kBrokerPieceCombatCap * 2;

			auto clamp_channel = [&](int total_now, int broker_now, const char* what,
									 int& take_io) -> bool {
				int t = 0;
				if(!apply_positive_broker_cap(kBrokerPieceCombatCap, kBrokerPieceCombatCap,
											 total_now, broker_now, take_io, what, t)) {
					return false;
				}
				take_io = t;
				return true;
			};

			if(pick.location == APPLY_HITROLL) {
				b_prior_edit_delta = std::max(0, cb.hr_total - cb.hr_broker);
				if(!clamp_channel(cb.hr_total, cb.hr_broker, "hitroll", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_DAMROLL) {
				b_prior_edit_delta = std::max(0, cb.dam_total - cb.dam_broker);
				if(!clamp_channel(cb.dam_total, cb.dam_broker, "damroll", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_SPELLPOWER) {
				b_prior_edit_delta = std::max(0, cb.sp_total - cb.sp_broker);
				if(!clamp_channel(cb.sp_total, cb.sp_broker, "spellpower", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_HITNDAM) {
				b_prior_edit_delta = std::max(0, cb.dam_total - cb.dam_broker);
				int t_hr = take;
				int t_dam = take;
				if(!clamp_channel(cb.hr_total, cb.hr_broker, "hitroll (via hit-n-dam)",
								  t_hr) ||
				   !clamp_channel(cb.dam_total, cb.dam_broker, "damroll (via hit-n-dam)",
								  t_dam)) {
					extract_obj(proto_b);
					return false;
				}
				take = std::min(t_hr, t_dam);
			}
			else if(pick.location == APPLY_HITNSP) {
				b_prior_edit_delta = std::max(0, cb.sp_total - cb.sp_broker);
				int t_hr = take;
				int t_sp = take;
				if(!clamp_channel(cb.hr_total, cb.hr_broker, "hitroll (via hit-n-sp)",
								  t_hr) ||
				   !clamp_channel(cb.sp_total, cb.sp_broker, "spellpower (via hit-n-sp)",
								  t_sp)) {
					extract_obj(proto_b);
					return false;
				}
				take = std::min(t_hr, t_sp);
			}
			else {
				extract_obj(proto_b);
				return refuse_cap("Effetto numerico non gestito dal broker.");
			}
			if(take <= 0) {
				extract_obj(proto_b);
				return refuse_cap("Non posso trasferire altro su questo effetto: B e' al "
								  "tetto di brokeraggio.");
			}
			pick.delta = take;
			pick.label = apply_display_name(pick.location) + " by " +
						 std::to_string(pick.delta);
		}
		extract_obj(proto_b);
	}

	/* Tetto toon-wide DAM+SP <= 30 (net -A +B). Fail-closed se DB non risponde. */
	{
		const int loc = pick.location;
		const bool touches_dam =
			loc == APPLY_DAMROLL || loc == APPLY_HITNDAM;
		const bool touches_sp =
			loc == APPLY_SPELLPOWER || loc == APPLY_HITNSP;
		if(touches_dam || touches_sp) {
			ObjInstDamSpTotals cur {};
			const char* owner = GET_NAME(ch);
			if(!object_instance_owner_dam_sp_totals(owner, cur)) {
				tell_from_jeweler(ch, mob,
								  "Non posso verificare il tetto DAM+SP del personaggio "
								  "(database non disponibile). Riprova piu' tardi.");
				std::ostringstream log;
				log << "EditAffectBroker transfer denied "
					<< (GET_NAME(ch) ? GET_NAME(ch) : "?")
					<< ": DAM+SP totals unavailable";
				editor_mudlog(LOG_SYSERR, log.str());
				return false;
			}
			long long add_dam = 0;
			long long add_sp = 0;
			long long rem_dam = 0;
			long long rem_sp = 0;
			if(loc == APPLY_DAMROLL || loc == APPLY_HITNDAM) {
				add_dam = pick.delta;
			}
			if(loc == APPLY_SPELLPOWER || loc == APPLY_HITNSP) {
				add_sp = pick.delta;
			}
			/* Quanto togliamo da A sullo stesso canale. */
			struct obj_data* proto_a = load_edit_prototype(obj_a);
			if(proto_a) {
				if(loc == APPLY_DAMROLL || loc == APPLY_HITNDAM) {
					rem_dam = std::min(static_cast<long long>(pick.delta),
									   static_cast<long long>(
										   obj_delta_vs_proto(obj_a, proto_a, loc)));
				}
				if(loc == APPLY_SPELLPOWER || loc == APPLY_HITNSP) {
					rem_sp = std::min(static_cast<long long>(pick.delta),
									  static_cast<long long>(
										  obj_delta_vs_proto(obj_a, proto_a, loc)));
				}
				extract_obj(proto_a);
			}
			const long long before = cur.dam + cur.sp;
			const long long projected =
				before - rem_dam - rem_sp + add_dam + add_sp;
			if(projected > kObjInstListinoMaxDamSp && projected > before) {
				std::ostringstream os;
				os << "Il personaggio supererebbe il tetto DAM+SP ("
				   << static_cast<long>(projected) << "/30).";
				tell_from_jeweler(ch, mob, os.str());
				std::ostringstream log;
				log << "EditAffectBroker transfer denied "
					<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": DAM+SP toon "
					<< static_cast<long>(before) << "->" << static_cast<long>(projected);
				editor_mudlog(LOG_PLAYERS, log.str());
				return false;
			}
			if(projected > kObjInstListinoMaxDamSp) {
				std::ostringstream os;
				os << "Il personaggio e' gia' oltre il tetto DAM+SP ("
				   << static_cast<long>(before) << "/30): non posso procedere.";
				tell_from_jeweler(ch, mob, os.str());
				std::ostringstream log;
				log << "EditAffectBroker transfer denied "
					<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": DAM+SP already "
					<< static_cast<long>(before);
				editor_mudlog(LOG_PLAYERS, log.str());
				return false;
			}
		}
	}

	const long listino = EditAffectDeltaListinoCost(obj_a, pick.location, pick.delta);
	const long fee = percent_of_listino(listino);
	const long payment = fee; /* 25% listino; niente /classi (come portale / 74c167bb) */
	const int nclass = HowManyClasses(ch);
	const int classes = nclass > 0 ? nclass : 1;
	if(!can_afford_prince_floor(ch, payment)) {
		std::ostringstream os;
		os << "Non hai abbastanza esperienza. Servono " << payment
		   << " XP (restando almeno " << kEditBrokerPrinceFloor << ").";
		tell_from_jeweler(ch, mob, os.str());
		std::ostringstream log;
		log << "EditAffectBroker transfer denied "
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": XP floor (need " << payment
			<< " have " << static_cast<long>(GET_EXP(ch)) << ") affect=" << pick.label;
		editor_mudlog(LOG_PLAYERS, log.str());
		return false;
	}

	out = TransferPlan{};
	out.obj_a = obj_a;
	out.obj_b = obj_b;
	out.pick = pick;
	out.listino = listino;
	out.fee = fee;
	out.payment = payment;
	out.classes = classes;
	out.b_prior_edit_delta = b_prior_edit_delta;
	out.existing_cap = existing_cap;
	out.transfer_cap = transfer_cap;
	out.total_cap = total_cap;
	out.had_stack_cap = had_stack_cap;
	return true;
}

void preview_transfer(struct char_data* ch, struct char_data* mob, const TransferPlan& plan) {
	if(!ch || !mob || !plan.obj_a || !plan.obj_b) {
		return;
	}
	std::ostringstream os;
	os << "$c0015Anteprima trasferimento$c0007\n\r"
	   << "  Effetto: $c0011" << plan.pick.label << "$c0007 (brokeraggio)\n\r"
	   << "  Da: " << obj_shortn(plan.obj_a) << "\n\r"
	   << "  Verso: " << obj_shortn(plan.obj_b);
	if(plan.had_stack_cap) {
		if(plan.pick.location == APPLY_AC) {
			os << " (edit gia' presente: " << plan.b_prior_edit_delta
			   << "; tetti edit " << -plan.existing_cap << " / broker "
			   << -plan.transfer_cap << " / totale " << -plan.total_cap << ")";
		}
		else {
			os << " (edit gia' presente: " << std::showpos << plan.b_prior_edit_delta
			   << std::noshowpos << "; tetti edit " << plan.existing_cap << " / broker "
			   << plan.transfer_cap << " / totale " << plan.total_cap << ")";
		}
	}
	os << "\n\r"
	   << "  Costo: " << plan.payment << " esperienza.\n\r"
	   << "  Dopo il pagamento non potrai scendere sotto i 400 milioni di esperienza.";
	tell_from_jeweler(ch, mob, os.str());
	if(GetMaxLevel(ch) >= IMMORTALE && procarea_obj_is_reward(plan.obj_b) &&
	   plan.obj_b->db_instance_id == 0) {
		tell_from_jeweler(ch, mob,
						  "[wiz] Registrero' il premio PROCAREA di B nel database prima del "
						  "trasferimento.");
	}
	ask_edit_broker_confirm(ch, mob);
}

[[nodiscard]] bool build_destroy_plan(struct char_data* ch, struct char_data* mob,
										std::string_view name_c, DestroyPlan& out) {
	if(!ch || !mob) {
		return false;
	}
	if(name_c.empty()) {
		tell_from_jeweler(ch, mob, "Sintassi: distruggi <oggettoC>.");
		return false;
	}

	const std::string name_c_str{name_c};
	struct obj_data* obj =
		get_obj_in_list_vis(ch, name_c_str.c_str(), ch->carrying);
	if(!obj) {
		tell_from_jeweler(ch, mob, "Non vedo quell'oggetto nel tuo inventario.");
		return false;
	}
	if(!obj_is_owned_edit(ch, obj)) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto deve essere EDIT, PERSONAL e di tua proprieta'.");
		return false;
	}
	if(obj->db_instance_id == 0) {
		tell_from_jeweler(ch, mob,
						  "L'oggetto non e' collegato al database edit. Contatta uno staffer.");
		std::ostringstream log;
		log << "EditAffectBroker destroy: missing instance_id owner="
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?");
		editor_mudlog(LOG_ERROR, log.str());
		return false;
	}

	const ObjEditAnalysis edit = AnalyzeObjEdit(obj);
	const int nclass = HowManyClasses(ch);
	const int classes = nclass > 0 ? nclass : 1;
	const long fee = percent_of_listino(edit.diff.valore);
	const long refund = fee; /* 25% listino; niente /classi */

	out.obj = obj;
	out.inst = obj->db_instance_id;
	out.shortn = obj_shortn(obj);
	out.listino_diff = edit.diff.valore;
	out.fee = fee;
	out.refund = refund;
	out.classes = classes;
	return true;
}

void preview_destroy(struct char_data* ch, struct char_data* mob, const DestroyPlan& plan) {
	if(!ch || !mob || !plan.obj) {
		return;
	}
	std::ostringstream os;
	os << "$c0015Anteprima distruzione$c0007\n\r"
	   << "  Oggetto: " << plan.shortn << "\n\r"
	   << "  Rimborso: " << plan.refund
	   << " esperienza (25% del valore di edit rispetto all'originale).\n\r"
	   << "  L'edit verra' eliminato: non si puo' annullare da qui.";
	tell_from_jeweler(ch, mob, os.str());
	ask_edit_broker_confirm(ch, mob);
}

void execute_transfer(struct char_data* ch, struct char_data* mob, const TransferPlan& plan) {
	if(!ch || !mob || !plan.obj_a || !plan.obj_b) {
		editor_mudlog(LOG_SYSERR,
					  "EditAffectBroker execute_transfer: null ch/mob/obj");
		return;
	}
	const char* const actor_name = GET_NAME(ch) ? GET_NAME(ch) : "?";

	if(procarea_obj_is_reward(plan.obj_b) && plan.obj_b->db_instance_id == 0) {
		std::string perr;
		tell_from_jeweler(ch, mob, "Prima registro il pezzo premio nel database...");
		if(!persist_procarea_reward_snapshot(plan.obj_b, ch, perr)) {
			tell_from_jeweler(ch, mob, perr);
			std::ostringstream log;
			log << "EditAffectBroker procarea snapshot fail " << actor_name << ": "
				<< perr;
			editor_mudlog(LOG_SYSERR, log.str());
			return;
		}
	}

	const BrokerTransferSnap snap = BrokerTransferSnap::capture(plan.obj_a, plan.obj_b);

	if(is_bitfield_location(plan.pick.location)) {
		remove_bit_delta(plan.obj_a, plan.pick.location,
						 static_cast<unsigned>(plan.pick.delta));
		if(!add_bit_delta_new_slot(plan.obj_b, plan.pick.location,
								   static_cast<unsigned>(plan.pick.delta))) {
			snap.restore(plan.obj_a, plan.obj_b);
			tell_from_jeweler(ch, mob, "Operazione fallita: slot su B non piu' disponibile.");
			std::ostringstream log;
			log << "EditAffectBroker transfer race: no slot B for " << actor_name;
			editor_mudlog(LOG_ERROR, log.str());
			return;
		}
	}
	else if(is_nonstackable_location(plan.pick.location)) {
		remove_numeric_delta(plan.obj_a, plan.pick.location, plan.pick.delta);
		const int free_slot = find_free_affect_slot(plan.obj_b);
		if(free_slot < 0) {
			snap.restore(plan.obj_a, plan.obj_b);
			tell_from_jeweler(ch, mob, "Operazione fallita: slot su B non piu' disponibile.");
			std::ostringstream log;
			log << "EditAffectBroker transfer race: no slot B for " << actor_name;
			editor_mudlog(LOG_ERROR, log.str());
			return;
		}
		plan.obj_b->affected[free_slot].location = static_cast<short>(plan.pick.location);
		plan.obj_b->affected[free_slot].modifier = plan.pick.delta;
	}
	else {
		remove_numeric_delta(plan.obj_a, plan.pick.location, plan.pick.delta);
		add_numeric_delta(plan.obj_b, plan.pick.location, plan.pick.delta);
	}

	ensure_b_personal_edit(ch, mob, plan.obj_b);

	std::ostringstream caps;
	if(plan.had_stack_cap) {
		if(plan.pick.location == APPLY_AC) {
			caps << "B_prior_edit_delta=" << plan.b_prior_edit_delta << " caps=edit"
				 << -plan.existing_cap << "/broker" << -plan.transfer_cap << "/total"
				 << -plan.total_cap;
		}
		else {
			caps << "B_prior_edit_delta=" << std::showpos << plan.b_prior_edit_delta
				 << std::noshowpos << " caps=edit" << plan.existing_cap << "/broker"
				 << plan.transfer_cap << "/total" << plan.total_cap;
		}
	}
	else {
		caps << "B_prior_edit_delta=" << std::showpos << plan.b_prior_edit_delta
			 << std::noshowpos << " caps=none";
	}
	const std::string caps_part = caps.str();

	std::ostringstream note_a_os;
	note_a_os << "brokeraggio REMOVE " << plan.pick.label << " -> B_inst="
			  << static_cast<unsigned long long>(plan.obj_b->db_instance_id);
	const std::string note_a = note_a_os.str();

	std::ostringstream note_b_os;
	note_b_os << "brokeraggio ADD " << plan.pick.label << " <- A_inst="
			  << static_cast<unsigned long long>(plan.obj_a->db_instance_id) << ' '
			  << caps_part;
	const std::string note_b = note_b_os.str();

	std::ostringstream detail_os;
	detail_os << "channel=brokeraggio kind=broker_transfer affect=" << plan.pick.label
			  << " location=" << plan.pick.location << " broker_delta=" << std::showpos
			  << plan.pick.delta << std::noshowpos << ' ' << caps_part
			  << " A_inst=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			  << " B_inst=" << static_cast<unsigned long long>(plan.obj_b->db_instance_id)
			  << " payment_xp=" << plan.payment << " fee=" << plan.fee
			  << " listino=" << plan.listino << " classes=" << plan.classes
			  << " actor=" << actor_name;
	const std::string detail = detail_os.str();

	/* kind dedicato (visibile in show history), distinto da create/update/edit_pool. */
	const bool ok_a =
		persist_edit_obj(plan.obj_a, ch, "broker_transfer", note_a.c_str(), detail.c_str());
	const bool ok_b =
		persist_edit_obj(plan.obj_b, ch, "broker_transfer", note_b.c_str(), detail.c_str());
	if(!ok_a || !ok_b) {
		/* Rollback memoria + risincronizza DB (XP non ancora addebitato).
		 * mark_edit=false: restore ha gia' ripristinato i flag; non forzare
		 * ITEM2_EDIT su B che non lo aveva prima del transfer. */
		snap.restore(plan.obj_a, plan.obj_b);
		const bool undo_a = persist_edit_obj(plan.obj_a, ch, nullptr, nullptr, nullptr, false);
		const bool undo_b = persist_edit_obj(plan.obj_b, ch, nullptr, nullptr, nullptr, false);
		tell_from_jeweler(ch, mob,
						  "Trasferimento annullato: salvataggio nel database fallito. "
						  "Nessuna esperienza e' stata addebitata.");
		std::ostringstream log;
		log << "EditAffectBroker transfer persist fail+rollback owner=" << actor_name
			<< " A=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			<< "(ok=" << ok_a << " undo=" << undo_a << ") B="
			<< static_cast<unsigned long long>(plan.obj_b->db_instance_id)
			<< "(ok=" << ok_b << " undo=" << undo_b << ") " << plan.pick.label;
		editor_mudlog(LOG_SYSERR, log.str());
		return;
	}

	/* XP solo dopo persist riuscito su entrambi. */
	if(plan.payment > 0) {
		GET_EXP(ch) = static_cast<int>(static_cast<long long>(GET_EXP(ch)) -
										static_cast<long long>(plan.payment));
	}

	schedule_inventory_save(ch);
	save_char(ch, AUTO_RENT, 0);

	{
		std::ostringstream ok;
		ok << "Fatto: trasferito " << plan.pick.label << ". Ti ho addebitato "
		   << plan.payment << " esperienza.";
		tell_from_jeweler(ch, mob, ok.str());
	}
	{
		std::ostringstream log;
		log << "EditAffectBroker transfer OK " << actor_name
			<< " affect=" << plan.pick.label << " broker_delta=" << plan.pick.delta
			<< " B_prior_edit_delta=" << plan.b_prior_edit_delta
			<< " A_inst=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			<< " B_inst=" << static_cast<unsigned long long>(plan.obj_b->db_instance_id)
			<< " pay=" << plan.payment << " fee=" << plan.fee
			<< " listino=" << plan.listino << " classes=" << plan.classes;
		editor_mudlog(LOG_PLAYERS, log.str());
	}
}

void execute_destroy(struct char_data* ch, struct char_data* mob, const DestroyPlan& plan) {
	if(!ch || !mob || !plan.obj) {
		editor_mudlog(LOG_SYSERR, "EditAffectBroker execute_destroy: null ch/mob/obj");
		return;
	}
	const char* const actor_name = GET_NAME(ch) ? GET_NAME(ch) : "?";

	if(!object_instance_delete(plan.inst, ch)) {
		tell_from_jeweler(ch, mob,
						  "Non sono riuscito a cancellare l'edit nel database. Operazione "
						  "annullata.");
		std::ostringstream log;
		log << "EditAffectBroker destroy delete fail inst="
			<< static_cast<unsigned long long>(plan.inst) << " owner=" << actor_name;
		editor_mudlog(LOG_SYSERR, log.str());
		return;
	}

	obj_from_char(plan.obj);
	extract_obj(plan.obj);

	if(plan.refund > 0) {
		const long long cur = static_cast<long long>(GET_EXP(ch));
		const long long next = std::min(cur + static_cast<long long>(plan.refund),
										static_cast<long long>(MAX_XP));
		GET_EXP(ch) = static_cast<int>(next);
	}

	schedule_inventory_save(ch);
	save_char(ch, AUTO_RENT, 0);

	{
		std::ostringstream ok;
		ok << "Ho distrutto " << plan.shortn << ". Ti rimborso " << plan.refund
		   << " esperienza.";
		tell_from_jeweler(ch, mob, ok.str());
	}
	{
		std::ostringstream log;
		log << "EditAffectBroker destroy OK " << actor_name
			<< " inst=" << static_cast<unsigned long long>(plan.inst)
			<< " refund=" << plan.refund << " fee=" << plan.fee
			<< " listino=" << plan.listino_diff << " classes=" << plan.classes
			<< " short=" << plan.shortn;
		editor_mudlog(LOG_PLAYERS, log.str());
	}
}

[[nodiscard]] bool is_edit_broker_command_topic(std::string_view text) {
	const std::string word = next_arg(text).first;
	return word == "trasferisci" || word == "distruggi" || word == "aiuto" || word == "help";
}

bool apply_edit_broker_answer(char_data* ch, char_data* mob, BrokerConfirmAnswer ans) {
	if(!ch || !mob) {
		return false;
	}
	auto it = g_edit_broker_pending.find(ch);
	if(it == g_edit_broker_pending.end()) {
		return false;
	}
	EditBrokerPending pending = it->second;
	if(pending.jeweler != mob) {
		return false;
	}
	if(time(nullptr) > pending.expires_at || !edit_broker_pending_room_ok(ch, pending)) {
		g_edit_broker_pending.erase(it);
		tell_from_jeweler(ch, mob, "La richiesta e' scaduta o non e' piu' valida.");
		return true;
	}

	switch(ans) {
	case BrokerConfirmAnswer::Yes:
		g_edit_broker_pending.erase(it);
		if(pending.op == EditBrokerOpKind::Transfer) {
			TransferPlan plan;
			if(!build_transfer_plan(ch, mob, pending.aff_name, pending.name_a, pending.name_b,
									plan)) {
				tell_from_jeweler(ch, mob,
								  "Non posso piu' completare il trasferimento: condizioni "
								  "cambiate.");
				return true;
			}
			execute_transfer(ch, mob, plan);
		}
		else {
			DestroyPlan plan;
			if(!build_destroy_plan(ch, mob, pending.name_c, plan)) {
				tell_from_jeweler(ch, mob,
								  "Non posso piu' completare la distruzione: condizioni "
								  "cambiate.");
				return true;
			}
			execute_destroy(ch, mob, plan);
		}
		return true;
	case BrokerConfirmAnswer::No:
		cancel_edit_broker_pending(ch, mob, true);
		return true;
	case BrokerConfirmAnswer::Other:
		ask_edit_broker_confirm(ch, mob);
		return true;
	}
	return true;
}

/* true = risposta gestita (consuma comando). allow_passthrough_cmds: ask puo'
 * lasciare passare trasferisci/distruggi/aiuto senza ri-chiedere conferma. */
bool try_handle_edit_broker_confirm(char_data* ch, char_data* mob, std::string_view text,
									bool allow_passthrough_cmds) {
	if(!ch || !mob) {
		return false;
	}
	auto it = g_edit_broker_pending.find(ch);
	if(it == g_edit_broker_pending.end() || it->second.jeweler != mob) {
		return false;
	}
	const BrokerConfirmAnswer ans = parse_broker_confirm(text);
	if(ans == BrokerConfirmAnswer::Other && allow_passthrough_cmds &&
	   is_edit_broker_command_topic(text)) {
		return false;
	}
	return apply_edit_broker_answer(ch, mob, ans);
}

void list_transferable_affects(struct char_data* ch, struct char_data* mob,
							   struct obj_data* obj_a) {
	if(!ch || !mob || !obj_a) {
		return;
	}
	if(!obj_is_owned_edit(ch, obj_a)) {
		tell_from_jeweler(ch, mob,
						  "Per l'elenco serve un oggetto EDIT, PERSONAL e di tua proprieta'.");
		return;
	}
	const auto labels = collect_transferable_delta_labels(obj_a);
	const char* shortn = obj_a->short_description ? obj_a->short_description : "oggetto";
	if(labels.empty()) {
		std::ostringstream os;
		os << "Su " << shortn << " non ci sono effetti aggiunti rispetto al prototipo.";
		tell_from_jeweler(ch, mob, os.str());
		return;
	}
	{
		std::ostringstream hdr;
		hdr << "Effetti trasferibili su " << shortn
			<< " (usa questi nomi con trasferisci):";
		tell_from_jeweler(ch, mob, hdr.str());
	}
	for(const auto& lab : labels) {
		send_to_char(("  $c0011" + lab + "$c0007\n\r").c_str(), ch);
	}
}

void show_edit_broker_help_and_list(struct char_data* ch, struct char_data* mob,
									std::string_view maybe_obj) {
	if(!ch || !mob) {
		return;
	}
	show_edit_broker_usage(ch, mob);

	auto [tok, rest] = next_arg(maybe_obj);
	(void)rest;
	if(!tok.empty()) {
		struct obj_data* obj_a = get_obj_in_list_vis(ch, tok.c_str(), ch->carrying);
		if(!obj_a) {
			tell_from_jeweler(ch, mob, "Non vedo quell'oggetto nel tuo inventario.");
			return;
		}
		list_transferable_affects(ch, mob, obj_a);
		return;
	}

	int count = 0;
	for(struct obj_data* o = ch->carrying; o; o = o->next_content) {
		if(obj_is_owned_edit(ch, o)) {
			list_transferable_affects(ch, mob, o);
			++count;
		}
	}
	if(count == 0) {
		tell_from_jeweler(ch, mob,
						  "Non hai oggetti EDIT PERSONAL in inventario da cui trasferire.");
	}
}

/** Ultimi due token = oggetti A/B; tutto il resto (anche multi-parola) = effetto. */
[[nodiscard]] bool split_transfer_args(std::string_view args, std::string& aff_name,
										 std::string& name_a, std::string& name_b) {
	std::vector<std::string> toks;
	std::string_view rest = args;
	for(;;) {
		auto [w, next] = next_arg(rest);
		if(w.empty()) {
			break;
		}
		toks.push_back(std::move(w));
		rest = next;
	}
	if(toks.size() < 3) {
		return false;
	}
	name_b = toks.back();
	toks.pop_back();
	name_a = toks.back();
	toks.pop_back();
	aff_name.clear();
	for(std::size_t i = 0; i < toks.size(); ++i) {
		if(i != 0) {
			aff_name.push_back(' ');
		}
		aff_name += toks[i];
	}
	return true;
}

void do_trasferisci(struct char_data* ch, struct char_data* mob, std::string_view args) {
	if(!ch || !mob) {
		return;
	}
	std::string aff_name;
	std::string name_a;
	std::string name_b;
	if(!split_transfer_args(args, aff_name, name_a, name_b)) {
		tell_from_jeweler(ch, mob, "Sintassi: trasferisci <effetto> <oggettoA> <oggettoB>.");
		return;
	}

	supersede_edit_broker_pending(ch, mob);

	TransferPlan plan;
	if(!build_transfer_plan(ch, mob, aff_name, name_a, name_b, plan)) {
		return;
	}

	preview_transfer(ch, mob, plan);
	EditBrokerPending pending {};
	pending.jeweler = mob;
	pending.op = EditBrokerOpKind::Transfer;
	pending.expires_at = time(nullptr) + kEditBrokerPendingTimeoutSec;
	pending.aff_name = aff_name;
	pending.name_a = name_a;
	pending.name_b = name_b;
	g_edit_broker_pending[ch] = std::move(pending);
}

void do_distruggi(struct char_data* ch, struct char_data* mob, std::string_view args) {
	if(!ch || !mob) {
		return;
	}
	auto [name_c, rest] = next_arg(args);
	(void)rest;

	supersede_edit_broker_pending(ch, mob);

	DestroyPlan plan;
	if(!build_destroy_plan(ch, mob, name_c, plan)) {
		return;
	}

	preview_destroy(ch, mob, plan);
	EditBrokerPending pending {};
	pending.jeweler = mob;
	pending.op = EditBrokerOpKind::Destroy;
	pending.expires_at = time(nullptr) + kEditBrokerPendingTimeoutSec;
	pending.name_c = std::string(name_c);
	g_edit_broker_pending[ch] = std::move(pending);
}

} // namespace

MOBSPECIAL_FUNC(EditAffectBroker) {
	if(!ch || !mob) {
		return FALSE;
	}
	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if((cmd >= CMD_NORTH && cmd <= CMD_DOWN) || cmd == CMD_FLEE) {
		auto it = g_edit_broker_pending.find(ch);
		if(it != g_edit_broker_pending.end() && it->second.jeweler == mob) {
			cancel_edit_broker_pending(ch, mob, true);
		}
		return FALSE;
	}

	const bool pc_ok = !(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF));

	/* Conferma in sospeso: say / nod / shake (senza dover rivolgerti al mob). */
	if(pc_ok && IS_PC(ch) && !IS_POLY(ch)) {
		if(cmd == CMD_NOD) {
			if(apply_edit_broker_answer(ch, mob, BrokerConfirmAnswer::Yes)) {
				return TRUE;
			}
			return FALSE;
		}
		if(cmd == CMD_SHAKE) {
			if(apply_edit_broker_answer(ch, mob, BrokerConfirmAnswer::No)) {
				return TRUE;
			}
			return FALSE;
		}
		if(cmd == CMD_SAY || cmd == CMD_SAY_APICE) {
			std::string_view speech = arg ? arg : "";
			while(!speech.empty() && std::isspace(static_cast<unsigned char>(speech.front()))) {
				speech.remove_prefix(1);
			}
			if(try_handle_edit_broker_confirm(ch, mob, speech, false)) {
				return TRUE;
			}
			return FALSE;
		}
	}

	if(cmd != CMD_ASK) {
		return FALSE;
	}

	std::string rest;
	if(!ask_is_for_mob(ch, arg, mob, rest)) {
		return FALSE;
	}

	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		return FALSE;
	}
	if(IS_POLY(ch)) {
		tell_from_jeweler(ch, mob, "Mi dispiace, non puoi farlo in questa forma.");
		return TRUE;
	}
	if(!IS_PC(ch)) {
		tell_from_jeweler(ch, mob, "Mi dispiace, non posso aiutarti.");
		return TRUE;
	}

	if(try_handle_edit_broker_confirm(ch, mob, rest, true)) {
		return TRUE;
	}

	const auto [topic, after] = next_arg(rest);
	if(topic.empty() || topic == "aiuto" || topic == "help") {
		show_edit_broker_help_and_list(ch, mob, after);
		return TRUE;
	}
	if(topic == "trasferisci") {
		do_trasferisci(ch, mob, after);
		return TRUE;
	}
	if(topic == "distruggi") {
		do_distruggi(ch, mob, after);
		return TRUE;
	}
	/* `ask <mob> <oggettoA>`: help + lista delta di quell'oggetto. */
	if(get_obj_in_list_vis(ch, topic.c_str(), ch->carrying)) {
		show_edit_broker_help_and_list(ch, mob, topic);
		return TRUE;
	}

	show_edit_broker_help_and_list(ch, mob, "");
	return TRUE;
}

} // namespace Alarmud
