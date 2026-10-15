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
 * Ask <mob> aiuto | listino. Preview + si/no/nod; shake annulla in qualsiasi momento.
 *
 * EditAffectBroker (myst.spe: M 3017 EditAffectBroker):
 * Ask <mob> trasferisci - wizard: give partenza, give destinazione, numero effetto, si/no
 * Ask <mob> distruggi - wizard: give oggetto, si/no
 * Ask <mob> rinomina - give, keywords/short/desc, pattern colori opz., si/no
 * (1 runa; proto match; colori: solid/lettere/parole/per-parola; somma 100k+110k+...)
 * Give = gesto (oggetto resta al PG).
 * Ask: aiuto (comandi), info <obj>, dettagli [servizio], trasferisci/distruggi/rinomina.
 * Persist transfer: TX unica A+B + broker_xfer_applied; XP/done via reconcile
 * (stesso path boot + login). Pending wizard: sweep tick + cleanup su extract.
 *
 * Delta combat: differenza sui canali effective (non slot-per-slot).
 * Es. proto HIT-N-DAM+2, edit HR+2 DAM+4 → trasferibile solo DAM+2.
 * Tetti pezzo destinazione (edit e broker separati, sommati una sola volta ciascuno):
 * - HR effective (HITROLL+HITNDAM+HITNSP): edit <= +2, broker <= +2
 * - DAM effective (DAMROLL+HITNDAM):       edit <= +2, broker <= +2
 * - SP effective (SPELLPOWER+HITNSP):      edit <= +2, broker <= +2
 * - STR/DEX/INT/WIS/CHR:                   edit <= +3, broker <= +3
 * - ARMOR: edit <= -40, broker <= -40 (delta sempre negativo; totale fino a -80)
 * Toon-wide: DAM+SP edit <= 30 (HITNDAM→dam, HITNSP→sp). Armi/slayer/eat → staff.
 *
 * PROCAREA-REWARD: baseline = create-snapshot (bonus rolled inclusi), come proto
 * per transfer/distruggi. Se B non ha ancora instance: baseline sospesa in memoria
 * (affect rolled attuali) per i tetti; scrittura DB solo nel commit atomico a
 * conferma (create-baseline + transfer). Annulla/timeout/fail → nessun DB, pezzo
 * invariato.
 *
 * Fee transfer/distruggi = 25% del listino (scale × class_mult × artifact),
 * poi /HowManyClasses su GET_EXP (come editpool / Esattore): biclasse paga meta'.
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
#include "magic.hpp"
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

/** $c M B FG - modifier 0=normale, 5=blink; background 0; FG 00-15. */
[[nodiscard]] std::string color_token_mod(int color, int modifier = 0) {
	std::string token = "$c";
	token += static_cast<char>('0' + std::max(0, std::min(modifier, 9)));
	token += '0';
	if(color <= 9) {
		token += '0';
	}
	token += std::to_string(color);
	return token;
}

[[nodiscard]] std::string color_token(int color) {
	return color_token_mod(color, 0);
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

/* Gesti in stanza per EditAffectBroker (dettagli restano nei tell privati). */
void broker_ambient_talk(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	act("$c0011$N$c0007 volge lo sguardo verso di te.", FALSE, ch, 0, mob, TO_CHAR);
	act("$N parla a bassa voce con $n.", FALSE, ch, 0, mob, TO_NOTVICT);
}

void broker_ambient_examine(char_data* ch, char_data* mob, obj_data* obj) {
	if(!ch || !mob || !obj) {
		return;
	}
	act("$N esamina $p che gli mostri.", FALSE, ch, obj, mob, TO_CHAR);
	act("$N esamina $p che $n gli mostra.", FALSE, ch, obj, mob, TO_NOTVICT);
}

void broker_ambient_work_transfer(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	act("$N concentra un bagliore tenue tra i due pezzi.", FALSE, ch, 0, mob, TO_CHAR);
	act("$N concentra un bagliore tenue davanti a $n.", FALSE, ch, 0, mob, TO_NOTVICT);
}

void broker_ambient_work_destroy(char_data* ch, char_data* mob, obj_data* obj) {
	if(!ch || !mob) {
		return;
	}
	if(obj) {
		act("$N preme le dita su $p finche' il metallo si sgretola.", FALSE, ch, obj, mob,
			TO_CHAR);
		act("$N preme le dita su $p davanti a $n: il pezzo si sgretola.", FALSE, ch, obj,
			mob, TO_NOTVICT);
	}
	else {
		act("$N preme le dita sul pezzo finche' si sgretola.", FALSE, ch, 0, mob, TO_CHAR);
		act("$N preme le dita su un pezzo davanti a $n: si sgretola.", FALSE, ch, 0, mob,
			TO_NOTVICT);
	}
}

void broker_ambient_done(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	act("$N annuisce e abbassa le mani: il lavoro e' fatto.", FALSE, ch, 0, mob,
		TO_CHAR);
	act("$N annuisce a $n e abbassa le mani.", FALSE, ch, 0, mob, TO_NOTVICT);
}

void broker_ambient_stop(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	act("$N scuote appena il capo e si ferma.", FALSE, ch, 0, mob, TO_CHAR);
	act("$N scuote appena il capo verso $n e si ferma.", FALSE, ch, 0, mob, TO_NOTVICT);
}

void broker_ambient_work_rename(char_data* ch, char_data* mob, obj_data* obj) {
	if(!ch || !mob) {
		return;
	}
	if(obj) {
		act("$N passa le dita su $p, come a riscrivere il nome.", FALSE, ch, obj, mob,
			TO_CHAR);
		act("$N passa le dita su $p davanti a $n, come a riscrivere il nome.", FALSE, ch,
			obj, mob, TO_NOTVICT);
	}
	else {
		act("$N passa le dita sul pezzo, come a riscrivere il nome.", FALSE, ch, 0, mob,
			TO_CHAR);
		act("$N passa le dita su un pezzo davanti a $n, come a riscrivere il nome.",
			FALSE, ch, 0, mob, TO_NOTVICT);
	}
}

#include "edit_broker_banned_words.inc"

constexpr int kRenameRuneCost = 1;
constexpr std::size_t kRenameMaxKeywords = 80;
constexpr std::size_t kRenameMaxShort = 120;
constexpr std::size_t kRenameMaxDesc = 240;
constexpr const char* kRenameGroundSuffix = " e' qui per terra.";
/* Colori rinomina: pattern fissi, il PG sceglie solo i colori (niente $c a mano). */
constexpr long kRenameColorBaseCoins = 100000L;
/* Ogni colore distinto costa di piu': 1°=base, 2°=base+10%, 3°=base+20%, ...
 * Totale = somma (es. 2 colori: 100k + 110k = 210k). */
constexpr int kRenameColorExtraPctPerDistinct = 10;
constexpr int kRenameMaxColorSlots = 12; /* parola-per-parola sulla short */
constexpr int kRenameDynamicSlots = -1;
/** Max colori distinti (codici $c, escluso reset 0007) per short e per description. */
constexpr int kRenameMaxDistinctColorsShort = 3;
/** Description: una sola parola colorata → al piu' un colore distinto. */
constexpr int kRenameMaxDistinctColorsDesc = 1;
constexpr const char* kRenameExampleText = "Spada di Mithril";
/** FG "normale": nessun codice finche' non cambia colore. */
constexpr int kRenameFgNormal = 7;

enum class RenamePatternKind {
	Solid,			 /* 1: tutto un colore */
	LetterAlternate, /* 2: lettere alterne */
	WordAlternate,	 /* 2: parole alterne */
	PerWord			 /* N: un colore per parola (short) */
};

struct RenameColorPattern {
	const char* id;
	const char* label;
	RenamePatternKind kind;
	int color_slots; /* fisso, oppure kRenameDynamicSlots */
};

constexpr RenameColorPattern kRenameColorPatterns[] = {
	{"solid", "Un solo colore", RenamePatternKind::Solid, 1},
	{"lettere", "Due colori alternati (lettera)", RenamePatternKind::LetterAlternate, 2},
	{"parole", "Due colori a parola", RenamePatternKind::WordAlternate, 2},
	{"perparola", "Colore parola per parola", RenamePatternKind::PerWord, kRenameDynamicSlots},
};

struct RenameColorChoice {
	int code; /* 1..15 */
	const char* name;
};

constexpr RenameColorChoice kRenameColorChoices[] = {
	{1, "rosso"},	 {2, "verde"},	   {3, "giallo"},	{4, "blu"},
	{5, "magenta"},	 {6, "ciano"},	   {7, "normale"},	{8, "grigio"},
	{9, "scarlatto"}, {10, "lime"},	   {11, "oro"},		{12, "azzurro"},
	{13, "rosa"},	 {14, "acqua"},	   {15, "bianco"},
};

[[nodiscard]] long rename_color_unit_cost(int color_index_zero_based) {
	if(color_index_zero_based < 0) {
		return 0;
	}
	const unsigned long base = static_cast<unsigned long>(kRenameColorBaseCoins);
	const unsigned long pct = static_cast<unsigned long>(kRenameColorExtraPctPerDistinct) *
							  static_cast<unsigned>(color_index_zero_based);
	return static_cast<long>(base + (base * pct) / 100UL);
}

[[nodiscard]] long rename_color_coin_cost(int distinct_colors) {
	if(distinct_colors < 1) {
		return 0;
	}
	unsigned long total = 0;
	for(int i = 0; i < distinct_colors; ++i) {
		total += static_cast<unsigned long>(rename_color_unit_cost(i));
	}
	return static_cast<long>(total);
}

[[nodiscard]] int count_distinct_rename_colors(const int* colors, int n) {
	if(!colors || n <= 0) {
		return 0;
	}
	int uniq[kRenameMaxColorSlots] {};
	int nu = 0;
	for(int i = 0; i < n; ++i) {
		bool seen = false;
		for(int j = 0; j < nu; ++j) {
			if(uniq[j] == colors[i]) {
				seen = true;
				break;
			}
		}
		if(!seen && nu < kRenameMaxColorSlots) {
			uniq[nu++] = colors[i];
		}
	}
	return nu;
}

/** Conta colori distinti in un testo ($cMBFG). Ignora il reset normale 0007. */
[[nodiscard]] int count_distinct_colors_in_text(std::string_view text) {
	int uniq[16] {};
	int nu = 0;
	for(std::size_t i = 0; i + 5 < text.size(); ++i) {
		if(text[i] != '$') {
			continue;
		}
		const char c1 = text[i + 1];
		if(c1 != 'c' && c1 != 'C') {
			continue;
		}
		bool digits = true;
		for(int k = 0; k < 4; ++k) {
			if(!std::isdigit(static_cast<unsigned char>(text[i + 2 + static_cast<std::size_t>(k)]))) {
				digits = false;
				break;
			}
		}
		if(!digits) {
			continue;
		}
		const int mod = text[i + 2] - '0';
		const int bg = text[i + 3] - '0';
		const int fg = (text[i + 4] - '0') * 10 + (text[i + 5] - '0');
		/* Reset standard: non conta come "colore usato". */
		if(mod == 0 && bg == 0 && fg == 7) {
			i += 5;
			continue;
		}
		bool seen = false;
		for(int j = 0; j < nu; ++j) {
			if(uniq[j] == fg) {
				seen = true;
				break;
			}
		}
		if(!seen && nu < 16) {
			uniq[nu++] = fg;
		}
		i += 5;
	}
	return nu;
}

[[nodiscard]] bool rename_field_color_count_ok(std::string_view text, int max_distinct,
											  const char* field_label, std::string& err) {
	const int n = count_distinct_colors_in_text(text);
	if(n > max_distinct) {
		std::ostringstream os;
		os << "Troppi colori distinti nella " << field_label << " (" << n << ", max "
		   << max_distinct << ").";
		err = os.str();
		return false;
	}
	return true;
}

[[nodiscard]] std::vector<std::string> split_rename_words(std::string_view text) {
	std::vector<std::string> words;
	std::string cur;
	for(unsigned char c : text) {
		if(std::isspace(c)) {
			if(!cur.empty()) {
				words.push_back(cur);
				cur.clear();
			}
		}
		else {
			cur.push_back(static_cast<char>(c));
		}
	}
	if(!cur.empty()) {
		words.push_back(cur);
	}
	return words;
}

[[nodiscard]] int rename_pattern_slots_for(const RenameColorPattern& pat,
										  std::string_view short_text) {
	if(pat.color_slots > 0) {
		return pat.color_slots;
	}
	const int n = static_cast<int>(split_rename_words(short_text).size());
	return std::max(1, n);
}

[[nodiscard]] int rename_fg_for_index(RenamePatternKind kind, const int* colors, int nslots,
									  int word_i, int letter_in_text) {
	if(!colors || nslots < 1) {
		return 7;
	}
	const auto at = [&](int idx) -> int {
		return colors[std::min(std::max(idx, 0), nslots - 1)];
	};
	switch(kind) {
	case RenamePatternKind::Solid:
		return at(0);
	case RenamePatternKind::LetterAlternate:
		return at(letter_in_text % 2);
	case RenamePatternKind::WordAlternate:
		return at(word_i % 2);
	case RenamePatternKind::PerWord:
		return at(word_i % nslots);
	}
	return at(0);
}

/**
 * Applica pattern colori: emette $c solo al cambio di FG/blink.
 * FG 7 (normale) = nessun codice finche' non cambia; chiusura $c0007 a fine
 * stringa solo se l'ultimo stato non e' normale.
 * Blink: una lettera (parola/lettera 0-based sulla short).
 */
[[nodiscard]] std::string apply_rename_color_pattern(std::string_view plain,
													RenamePatternKind kind,
													const int* colors, int nslots,
													int blink_word = -1,
													int blink_letter = -1) {
	if(plain.empty()) {
		return {};
	}
	const bool do_blink = (blink_word >= 0 && blink_letter >= 0);
	if((!colors || nslots < 1) && !do_blink) {
		return std::string(plain);
	}

	std::string out;
	int word_i = -1;
	int letter_in_word = 0;
	int letter_in_text = 0;
	bool in_word = false;
	int active_fg = kRenameFgNormal;
	bool active_blink = false;

	const auto emit_state = [&](int fg, bool blink) {
		if(blink) {
			out += color_token_mod(fg == kRenameFgNormal ? kRenameFgNormal : fg, 5);
		}
		else if(fg == kRenameFgNormal) {
			out += "$c0007";
		}
		else {
			out += color_token(fg);
		}
		active_fg = fg;
		active_blink = blink;
	};

	for(unsigned char uch : plain) {
		const char ch = static_cast<char>(uch);
		if(std::isspace(uch)) {
			out.push_back(ch);
			in_word = false;
			continue;
		}
		if(!in_word) {
			in_word = true;
			++word_i;
			letter_in_word = 0;
		}
		const int fg = (colors && nslots > 0)
						   ? rename_fg_for_index(kind, colors, nslots, word_i, letter_in_text)
						   : kRenameFgNormal;
		const bool blink_here =
			do_blink && word_i == blink_word && letter_in_word == blink_letter;
		if(fg != active_fg || blink_here != active_blink) {
			emit_state(fg, blink_here);
		}
		out.push_back(ch);
		++letter_in_word;
		++letter_in_text;
	}
	if(active_fg != kRenameFgNormal || active_blink) {
		out += "$c0007";
	}
	return out;
}

/** Colora una sola parola (0-based); FG 7 = nessuna modifica. */
[[nodiscard]] std::string apply_rename_one_word_color(std::string_view plain, int word_index,
													 int fg) {
	if(plain.empty() || word_index < 0 || fg < 1 || fg == kRenameFgNormal) {
		return std::string(plain);
	}
	std::string out;
	int word_i = -1;
	bool in_word = false;
	bool coloring = false;
	for(unsigned char uch : plain) {
		const char ch = static_cast<char>(uch);
		if(std::isspace(uch)) {
			if(coloring) {
				out += "$c0007";
				coloring = false;
			}
			out.push_back(ch);
			in_word = false;
			continue;
		}
		if(!in_word) {
			in_word = true;
			++word_i;
			if(word_i == word_index) {
				out += color_token(fg);
				coloring = true;
			}
		}
		out.push_back(ch);
	}
	if(coloring) {
		out += "$c0007";
	}
	return out;
}

/** Stima worst-case: pattern applicabile entro il limite DB della short? */
[[nodiscard]] bool rename_pattern_fits_db_short(const RenameColorPattern& pat,
											   std::string_view short_text) {
	if(short_text.empty()) {
		return true;
	}
	const int slots = rename_pattern_slots_for(pat, short_text);
	int trial[kRenameMaxColorSlots]{};
	const int n = std::min(slots, kRenameMaxColorSlots);
	for(int i = 0; i < n; ++i) {
		/* Colori non-7 alternati → massima densita' di cambi (lettere/parole). */
		trial[i] = (i % 2 == 0) ? 1 : 2;
	}
	const std::string colored =
		apply_rename_color_pattern(short_text, pat.kind, trial, n);
	return colored.size() <= kObjInstMaxShort;
}

/** Indici in kRenameColorPatterns compatibili con questa short (ordine menu). */
[[nodiscard]] std::vector<int> rename_patterns_available_for(std::string_view short_text) {
	std::vector<int> out;
	constexpr int npat =
		static_cast<int>(sizeof(kRenameColorPatterns) / sizeof(kRenameColorPatterns[0]));
	out.reserve(static_cast<std::size_t>(npat));
	for(int i = 0; i < npat; ++i) {
		if(rename_pattern_fits_db_short(kRenameColorPatterns[i], short_text)) {
			out.push_back(i);
		}
	}
	return out;
}

[[nodiscard]] std::string rename_pattern_example(const RenameColorPattern& pat) {
	/* Demo: oro / acqua / scarlatto */
	const int demo[3] = {11, 14, 9};
	const int slots = (pat.color_slots > 0) ? pat.color_slots : 3;
	return apply_rename_color_pattern(kRenameExampleText, pat.kind, demo, slots);
}

[[nodiscard]] const RenameColorChoice* find_rename_color_choice(std::string_view token) {
	if(token.empty()) {
		return nullptr;
	}
	bool all_digit = true;
	for(unsigned char c : token) {
		if(!std::isdigit(c)) {
			all_digit = false;
			break;
		}
	}
	if(all_digit) {
		const int code = std::atoi(std::string(token).c_str());
		for(const auto& ch : kRenameColorChoices) {
			if(ch.code == code) {
				return &ch;
			}
		}
		return nullptr;
	}
	std::string low;
	low.reserve(token.size());
	for(unsigned char c : token) {
		low.push_back(static_cast<char>(std::tolower(c)));
	}
	for(const auto& ch : kRenameColorChoices) {
		if(low == ch.name) {
			return &ch;
		}
	}
	return nullptr;
}

void show_rename_color_palette(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	tell_from_jeweler(ch, mob, "Colori disponibili (nome o numero):");
	constexpr std::size_t ncolors =
		sizeof(kRenameColorChoices) / sizeof(kRenameColorChoices[0]);
	const std::size_t mid = (ncolors + 1) / 2;
	for(int row = 0; row < 2; ++row) {
		std::ostringstream os;
		const std::size_t from = (row == 0) ? 0 : mid;
		const std::size_t to = (row == 0) ? mid : ncolors;
		for(std::size_t i = from; i < to; ++i) {
			const auto& c = kRenameColorChoices[i];
			if(i > from) {
				os << "  ";
			}
			os << color_token(c.code) << c.name << "$c0007(" << c.code << ")";
		}
		os << "\n\r";
		send_to_char(os.str().c_str(), ch);
	}
}

void show_rename_pattern_menu(char_data* ch, char_data* mob, std::string_view short_text) {
	if(!ch || !mob) {
		return;
	}
	const auto available = rename_patterns_available_for(short_text);
	tell_from_jeweler(ch, mob,
					  "Scegli un $c0011pattern$c0007 (numero), oppure $c00110$c0007 / "
					  "$c0011nessuno$c0007 senza colori. "
					  "$c0011indietro$c0007 per tornare.");
	{
		std::ostringstream head;
		head << "Esempi su \"$c0015" << kRenameExampleText << "$c0007\":\n\r"
			 << "  $c00110)$c0007 Nessun colore\n\r";
		send_to_char(head.str().c_str(), ch);
	}
	int idx = 1;
	for(int pat_i : available) {
		const auto& pat = kRenameColorPatterns[pat_i];
		std::ostringstream line;
		line << "  $c0011" << idx << ")$c0007 " << pat.label << ": "
			 << rename_pattern_example(pat) << "\n\r";
		send_to_char(line.str().c_str(), ch);
		++idx;
	}
	if(available.empty()) {
		send_to_char(
			"$c0008(Nessun pattern colore sta in questa short: usa 0 / nessuno.)$c0007\n\r",
			ch);
	}
	{
		std::ostringstream cost;
		cost << "Costo: si sommano i colori $c0015distinti$c0007 - 1° $c0011"
			 << rename_color_unit_cost(0) << "$c0007, 2° $c0011" << rename_color_unit_cost(1)
			 << "$c0007 (tot. 2: $c0011" << rename_color_coin_cost(2) << "$c0007).\n\r";
		send_to_char(cost.str().c_str(), ch);
	}
}

[[nodiscard]] std::string trim_rename_text(std::string_view in) {
	while(!in.empty() && std::isspace(static_cast<unsigned char>(in.front()))) {
		in.remove_prefix(1);
	}
	while(!in.empty() && std::isspace(static_cast<unsigned char>(in.back()))) {
		in.remove_suffix(1);
	}
	return std::string(in);
}

[[nodiscard]] bool rename_text_has_color_codes(std::string_view in) {
	for(std::size_t i = 0; i + 1 < in.size(); ++i) {
		if(in[i] == '$' && (in[i + 1] == 'c' || in[i + 1] == 'C')) {
			return true;
		}
	}
	return false;
}

/** Solo lettere a-z; spazi/punteggiatura/apostrofi eliminati (mer da -> merda). */
[[nodiscard]] std::string normalize_ban_text(std::string_view in) {
	std::string out;
	out.reserve(in.size());
	for(unsigned char c : in) {
		if(std::isalpha(c)) {
			out.push_back(static_cast<char>(std::tolower(c)));
		}
	}
	return out;
}

[[nodiscard]] const char* rename_banned_hit(std::string_view text) {
	const std::string norm = normalize_ban_text(text);
	if(norm.empty()) {
		return nullptr;
	}
	for(const char* const* p = kEditBrokerBannedWords; *p; ++p) {
		const std::string bw = normalize_ban_text(*p);
		if(bw.empty()) {
			continue;
		}
		if(norm.find(bw) != std::string::npos) {
			return *p;
		}
	}
	return nullptr;
}

[[nodiscard]] bool cstr_eq_field(const char* a, const char* b) {
	const char* aa = a ? a : "";
	const char* bb = b ? b : "";
	return std::strcmp(aa, bb) == 0;
}

[[nodiscard]] struct obj_data* load_edit_prototype(const struct obj_data* obj);

[[nodiscard]] bool obj_texts_match_prototype(const obj_data* obj, std::string& err) {
	err.clear();
	if(!obj) {
		err = "Oggetto non valido.";
		return false;
	}
	struct obj_data* proto = load_edit_prototype(obj);
	if(!proto) {
		err = "Non riesco a leggere l'essenza originale di quel pezzo.";
		return false;
	}
	const bool same = cstr_eq_field(obj->name, proto->name) &&
					  cstr_eq_field(obj->short_description, proto->short_description) &&
					  cstr_eq_field(obj->description, proto->description);
	extract_obj(proto);
	if(!same) {
		err = "Ricordati che non posso cambiare il nome di un oggetto gia' rinominato.";
		return false;
	}
	return true;
}

/** true se la description e' gia' "a terra" (evita doppio "e' qui" / "per terra"). */
[[nodiscard]] bool rename_desc_already_grounded(std::string_view desc) {
	const std::string norm = normalize_ban_text(desc);
	if(norm.empty()) {
		return false;
	}
	if(norm.find("quiperterra") != std::string::npos) {
		return true;
	}
	const auto ends_with = [&](std::string_view suf) -> bool {
		return norm.size() >= suf.size() &&
			   norm.compare(norm.size() - suf.size(), suf.size(), suf) == 0;
	};
	/* "e' qui" / "e qui" / "è qui" → equi; oppure gia' "...per terra" */
	return ends_with("equi") || ends_with("perterra");
}

[[nodiscard]] std::string finalize_rename_description(std::string desc) {
	desc = trim_rename_text(desc);
	if(!rename_desc_already_grounded(desc)) {
		desc += kRenameGroundSuffix;
	}
	return desc;
}

[[nodiscard]] bool validate_rename_field(std::string_view raw, std::size_t max_len,
										 const char* field_label, std::string& out,
										 std::string& err) {
	out = trim_rename_text(raw);
	err.clear();
	if(out.empty()) {
		err = std::string("Il campo ") + field_label + " non puo' essere vuoto.";
		return false;
	}
	if(out.size() > max_len) {
		std::ostringstream os;
		os << "Il campo " << field_label << " e' troppo lungo (max " << max_len
		   << " caratteri).";
		err = os.str();
		return false;
	}
	if(rename_text_has_color_codes(out)) {
		err = "Non puoi usare codici colore nei nomi.";
		return false;
	}
	if(const char* hit = rename_banned_hit(out)) {
		(void)hit;
		err = "Quel testo contiene parole non ammesse.";
		return false;
	}
	return true;
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
		line << "  $c0012" << mat << "$c0007 x" << plan.consumed << "  -  $c0015"
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
 * EditAffectBroker - trasferimento affect tra due edit / soft-delete edit
 * Aggancio: myst.spe  M 3017 EditAffectBroker
 * ========================================================================= */
namespace {

constexpr long kEditBrokerPrinceFloor = PRINCEEXP;
constexpr int kEditBrokerPercentKeep = 25; /* fee = 25% listino; su GET_EXP poi /classi */

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

/**
 * Canali combat effective (HITNDAM conta in HR+DAM, HITNSP in HR+SP).
 * La differenza edit e' sui totali, non slot-per-slot (vedi Montero / vasoline).
 */
struct CombatEffective {
	long hr{0};
	long dam{0};
	long sp{0};
};

[[nodiscard]] bool is_combat_channel_location(int loc) noexcept {
	return loc == APPLY_HITROLL || loc == APPLY_DAMROLL || loc == APPLY_SPELLPOWER ||
		   loc == APPLY_HITNDAM || loc == APPLY_HITNSP;
}

[[nodiscard]] CombatEffective combat_effective_sums(const struct obj_data* obj) {
	CombatEffective e {};
	if(!obj) {
		return e;
	}
	const long hr = sum_affect_location(obj, APPLY_HITROLL);
	const long dam = sum_affect_location(obj, APPLY_DAMROLL);
	const long sp = sum_affect_location(obj, APPLY_SPELLPOWER);
	const long hd = sum_affect_location(obj, APPLY_HITNDAM);
	const long hs = sum_affect_location(obj, APPLY_HITNSP);
	e.hr = hr + hd + hs;
	e.dam = dam + hd;
	e.sp = sp + hs;
	return e;
}

[[nodiscard]] CombatEffective combat_effective_delta(const struct obj_data* obj,
													 const struct obj_data* proto) {
	const CombatEffective cur = combat_effective_sums(obj);
	const CombatEffective base = combat_effective_sums(proto);
	CombatEffective d {};
	d.hr = std::max(0L, cur.hr - base.hr);
	d.dam = std::max(0L, cur.dam - base.dam);
	d.sp = std::max(0L, cur.sp - base.sp);
	return d;
}

/** Quanto si puo' trasferire da quella location, limitato al delta effective. */
[[nodiscard]] int combat_transferable_amount(const struct obj_data* obj,
											 const struct obj_data* proto, int loc) {
	if(!obj || !proto || !is_combat_channel_location(loc)) {
		return 0;
	}
	const CombatEffective d = combat_effective_delta(obj, proto);
	const long raw_loc = std::max(
		0L, sum_affect_location(obj, loc) - sum_affect_location(proto, loc));
	if(loc == APPLY_HITNDAM) {
		return static_cast<int>(std::min({d.hr, d.dam, raw_loc}));
	}
	if(loc == APPLY_HITNSP) {
		return static_cast<int>(std::min({d.hr, d.sp, raw_loc}));
	}
	if(loc == APPLY_HITROLL) {
		return static_cast<int>(std::min(d.hr, raw_loc));
	}
	if(loc == APPLY_DAMROLL) {
		return static_cast<int>(std::min(d.dam, raw_loc));
	}
	if(loc == APPLY_SPELLPOWER) {
		return static_cast<int>(std::min(d.sp, raw_loc));
	}
	return 0;
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
	const CombatEffective d = combat_effective_delta(obj, proto);
	b.hr_total = static_cast<int>(d.hr);
	b.dam_total = static_cast<int>(d.dam);
	b.sp_total = static_cast<int>(d.sp);
	/* Quota broker per canale: somma history location, clamp al delta effective. */
	const int br_hr = broker_delta_clamped(iid, APPLY_HITROLL, b.hr_total);
	const int br_dam = broker_delta_clamped(iid, APPLY_DAMROLL, b.dam_total);
	const int br_sp = broker_delta_clamped(iid, APPLY_SPELLPOWER, b.sp_total);
	const int br_hd = broker_delta_clamped(iid, APPLY_HITNDAM,
										   std::min(b.hr_total, b.dam_total));
	const int br_hs = broker_delta_clamped(iid, APPLY_HITNSP,
										   std::min(b.hr_total, b.sp_total));
	b.hr_broker = std::min(b.hr_total, br_hr + br_hd + br_hs);
	b.dam_broker = std::min(b.dam_total, br_dam + br_hd);
	b.sp_broker = std::min(b.sp_total, br_sp + br_hs);
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

/** Slot libero: APPLY_NONE/0, oppure qualsiasi location con modifier 0 ("By 0"). */
[[nodiscard]] bool affect_slot_is_empty(const obj_affected_type& aff) noexcept {
	return aff.location == APPLY_NONE || aff.location == 0 || aff.modifier == 0;
}

[[nodiscard]] int find_free_affect_slot(const struct obj_data* obj) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(affect_slot_is_empty(obj->affected[i])) {
			return i;
		}
	}
	return -1;
}

/**
 * Slot con location==loc e modifier!=0 (effetto reale).
 * I "By 0" non contano: sono slot vuoti riusabili.
 */
[[nodiscard]] int find_location_slot(const struct obj_data* obj, int loc) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc && obj->affected[i].modifier != 0) {
			return i;
		}
	}
	return -1;
}

/** Svuota ogni affect "By 0" (location → APPLY_NONE). Usato su B dopo il transfer. */
void clear_zero_modifier_slots(struct obj_data* obj) {
	if(!obj) {
		return;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].modifier == 0) {
			obj->affected[i].location = APPLY_NONE;
		}
	}
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

/**
 * Prototipo per delta broker / distruggi su pezzi coinvolti nel trasferimento.
 * - non-procarea: prototipo mondo (come load_edit_prototype)
 * - procarea con instance: create-baseline (rolled al primo snapshot in DB)
 * - procarea senza instance: baseline sospesa = affect attuali sul pezzo vivo
 *   (niente commit DB; lo snapshot definitivo resta a conferma in execute_transfer)
 */
[[nodiscard]] struct obj_data* load_broker_prototype(const struct obj_data* obj) {
	if(!obj) {
		return nullptr;
	}
	if(procarea_obj_is_reward(obj)) {
#if USE_MYSQL
		if(obj->db_instance_id != 0) {
			return object_instance_materialize_create_baseline(obj->db_instance_id);
		}
#endif
		struct obj_data* proto = load_edit_prototype(obj);
		if(!proto) {
			return nullptr;
		}
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			proto->affected[i].location = obj->affected[i].location;
			proto->affected[i].modifier = obj->affected[i].modifier;
		}
		return proto;
	}
	return load_edit_prototype(obj);
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

	struct obj_data* proto = load_broker_prototype(src);
	if(!proto) {
		err = "Non riesco a leggere l'essenza originale di quel pezzo.";
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
					"Su quel pezzo non trovo quell'effetto da spostare: non e' un'aggiunta "
					"rispetto alla sua essenza originale.");
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
		else if(is_combat_channel_location(matched_loc)) {
			/* Solo la differenza effective (HR/DAM/SP), non lo slot grezzo. */
			delta = combat_transferable_amount(src, proto, matched_loc);
		}
		else if(cur > base) {
			delta = cur - base;
		}
		if(delta == 0) {
			return finish_fail(
				"Su quel pezzo non trovo quell'effetto da spostare: non e' un'aggiunta "
				"rispetto alla sua essenza originale.");
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
					"Su quel pezzo non trovo quell'effetto da spostare: non e' un'aggiunta rispetto alla sua essenza originale.");
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
						"Su quel pezzo non trovo quell'effetto da spostare: non e' "
						"un'aggiunta rispetto alla sua essenza originale.");
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
			"Su quel pezzo non trovo quell'effetto da spostare: non e' un'aggiunta rispetto alla sua essenza originale.");
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
 * Fee = 25% del listino; su GET_EXP si addebita/rimborsa fee/HowManyClasses
 * (mono=/1, bi=/2, tri=/3), come editpool e Esattore. Esempio biclasse
 * listino 450M → fee 112.5M → payment 56.25M sull'unico pool exp.
 */
[[nodiscard]] long toon_xp_share(struct char_data* ch, long fee) {
	if(fee <= 0 || ch == nullptr) {
		return 0;
	}
	const int n = HowManyClasses(ch);
	const int classes = n > 0 ? n : 1;
	return fee / classes;
}

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

/**
 * Dopo il transfer: B diventa PERSONAL+EDIT+ARTIFACT del toon.
 * Cost almeno 20001 → scatta [RARO] (IS_RARE / LIM_ITEM_COST_MIN).
 */
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
	/* Valore listino pezzo: soglia RARO usata in produzione sugli edit. */
	constexpr int kBrokerRareCost = 20001;
	if(obj_b->obj_flags.cost < kBrokerRareCost) {
		obj_b->obj_flags.cost = kBrokerRareCost;
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
	unsigned long long instance_id_b{};
	int item_number_b{-1};
	int char_vnum_b{};

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
			s.instance_id_b = obj_b->db_instance_id;
			s.item_number_b = obj_b->item_number;
			s.char_vnum_b = obj_b->char_vnum;
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
		obj_b->db_instance_id = instance_id_b;
		obj_b->item_number = item_number_b;
		obj_b->char_vnum = char_vnum_b;
	}
};

[[nodiscard]] std::vector<std::string> collect_transferable_delta_labels(struct obj_data* obj) {
	std::vector<std::string> out;
	if(!obj) {
		return out;
	}
	struct obj_data* proto = load_broker_prototype(obj);
	if(!proto) {
		return out;
	}

	for(int loc = 0; apply_types[loc] && apply_types[loc][0] != '\n'; ++loc) {
		if(loc == APPLY_NONE || loc == APPLY_SKIP) {
			continue;
		}
		if(is_combat_channel_location(loc)) {
			/* Gestiti sotto con delta effective HR/DAM/SP. */
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

	/*
	 * Combat: differenza sui canali effective, poi etichette in base agli slot
	 * davvero presenti (HIT-N-DAM se c'e' overlap HR+DAM, altrimenti singoli).
	 */
	{
		CombatEffective d = combat_effective_delta(obj, proto);
		const long hd_raw = std::max(
			0L, sum_affect_location(obj, APPLY_HITNDAM) -
					sum_affect_location(proto, APPLY_HITNDAM));
		const long hs_raw = std::max(
			0L, sum_affect_location(obj, APPLY_HITNSP) -
					sum_affect_location(proto, APPLY_HITNSP));
		const long hr_raw = std::max(
			0L, sum_affect_location(obj, APPLY_HITROLL) -
					sum_affect_location(proto, APPLY_HITROLL));
		const long dam_raw = std::max(
			0L, sum_affect_location(obj, APPLY_DAMROLL) -
					sum_affect_location(proto, APPLY_DAMROLL));
		const long sp_raw = std::max(
			0L, sum_affect_location(obj, APPLY_SPELLPOWER) -
					sum_affect_location(proto, APPLY_SPELLPOWER));

		auto push_by = [&](int loc, long amt) {
			if(amt > 0) {
				out.push_back(apply_display_name(loc) + " by " + std::to_string(amt));
			}
		};

		const long hd = std::min({d.hr, d.dam, hd_raw});
		if(hd > 0) {
			push_by(APPLY_HITNDAM, hd);
			d.hr -= hd;
			d.dam -= hd;
		}
		const long hs = std::min({d.hr, d.sp, hs_raw});
		if(hs > 0) {
			push_by(APPLY_HITNSP, hs);
			d.hr -= hs;
			d.sp -= hs;
		}
		push_by(APPLY_HITROLL, std::min(d.hr, hr_raw));
		push_by(APPLY_DAMROLL, std::min(d.dam, dam_raw));
		push_by(APPLY_SPELLPOWER, std::min(d.sp, sp_raw));
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
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob,
					  "Posso spostare un effetto da un tuo pezzo EDIT a un altro oggetto.");
	tell_from_jeweler(ch, mob, "Posso distruggere un edit, oppure rinominare un pezzo.");
	send_to_char(
		"$c0015Comandi:\n\r"
		"\n\r"
		"  Chiedimi $c0011trasferisci$c0007\n\r"
		"      1) usa $c0011give$c0007 - pezzo da cui togliere l'effetto\n\r"
		"      2) usa $c0011give$c0007 - pezzo su cui metterlo\n\r"
		"      3) digita il numero dell'effetto dalla lista\n\r"
		"      4) conferma con $c0011si$c0007 / $c0011nod$c0007\n\r"
		"\n\r"
		"  Chiedimi $c0011distruggi$c0007\n\r"
		"      usa $c0011give$c0007 come comando, poi conferma.\n\r"
		"\n\r"
		"  Chiedimi $c0011rinomina$c0007\n\r"
		"      usa $c0011give$c0007, poi testi, pattern colori (opz.), conferma.\n\r"
		"\n\r"
		"  Chiedimi $c0011info$c0007 <oggetto>   - identify + effetti trasferibili\n\r"
		"  Chiedimi $c0011aiuto$c0007            - questo elenco\n\r"
		"  Chiedimi $c0011dettagli$c0007         - costi di trasferisci / distruggi / "
		"rinomina\n\r"
		"\n\r"
		"Durante rinomina: $c0011indietro$c0007 / $c0011i$c0007 torna allo step prima; "
		"$c0011shake$c0007 annulla in qualsiasi momento; $c0011no$c0007 annulla "
		"(tranne alla domanda blink).\n\r",
		ch);
	if(GetMaxLevel(ch) >= IMMORTALE) {
		send_to_char(
			"$c0008[wiz] Toon: DAM+SP edit <= 30. Armi: chiedi allo staff.\n\r"
			"$c0008[wiz] PROCAREA-REWARD: baseline = create-snapshot (rolled); senza "
			"instance i tetti usano baseline sospesa; DB solo a commit riuscito.\n\r",
			ch);
	}
	(void)mob;
}

void show_edit_broker_details_transfer(struct char_data* ch, struct char_data* mob) {
	if(!ch) {
		return;
	}
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dettagli sul $c0011trasferimento$c0007:");
	send_to_char(
		"Costa il $c001125%$c0007 del listino di quell'effetto (classi e "
		"$c0011artifact$c0007 sono gia' nel listino), poi diviso per il "
		"numero di classi sul tuo score.\n\r"
		"Puoi spostare un solo effetto alla volta: viene spostato "
		"$c0015tutto$c0007 il delta di quello slot.\n\r"
		"Esempi: se hai editato $c0011+2 damroll$c0007, non puoi trasferirne "
		"solo $c00111$c0007; se sullo stesso slot hai resistenza\n\r"
		"all'impatto e al taglio, partono entrambe.\n\r"
		"Nessun pagamento puo' farti scendere sotto i $c0011400 milioni$c0007 "
		"di esperienza.\n\r"
		"\n\r"
		"$c0015Tetti sul pezzo di destinazione$c0007\n\r"
		"($c0011edit$c0007 e $c0011trasferimento$c0007 sono due budget separati; "
		"la somma non supera $c0011+4$c0007 per canale):\n\r"
		"  $c0014hitroll / dam / spellpower:$c0007 si confrontano i totali "
		"$c0015effective$c0007 (HIT-N-DAM conta in hit e dam, HIT-N-SP in hit e sp), "
		"non i singoli slot. Ciascun canale: +$c00112$c0007 edit + +$c00112$c0007 "
		"trasferimento (max +$c00114$c0007). Hitroll senza tetto 30.\n\r"
		"  Se c'e' uno slot libero puoi aggiungere HITROLL/DAMROLL/SPELLPOWER "
		"anche se c'e' gia' HIT-N-DAM o HIT-N-SP (proto o delta), "
		"purche' il canale non superi +4.\n\r"
		"  $c0014DAM+SP sul personaggio:$c0007 massimo $c001130$c0007 "
		"(HIT-N-DAM conta come dam, HIT-N-SP come spellpower).\n\r"
		"  $c0014forza / destrezza / intelligenza / saggezza / carisma:$c0007 "
		"fino a $c0011+3$c0007 da edit e $c0011+3$c0007 da trasferimento\n\r"
		"  $c0014armor:$c0007 fino a $c0011-40$c0007 da edit e $c0011-40$c0007 "
		"da trasferimento\n\r"
		"\n\r"
		"$c0015Pezzo di partenza:$c0007 deve essere $c0011TUO$c0007, "
		"$c0011EDIT$c0007 e $c0011PERSONAL$c0007.\n\r"
		"$c0015Pezzo di destinazione:$c0007 non puo' essere $c0011PERSONAL$c0007 "
		"di un altro; dopo il trasferimento\n\r"
		"diventa $c0011EDIT$c0007 e $c0011PERSONAL$c0007 tuo, "
		"$c0011ARTIFACT$c0007 se non lo era gia', e valore $c001120001$c0007 "
		"($c0011RARO$c0007) se era piu' basso. Puo' essere gia' un tuo "
		"$c0011EDIT$c0007.\n\r"
		"\n\r"
		"$c0015Premi PROCAREA-REWARD:$c0007 i bonus rolled contano come base del "
		"pezzo (non come edit). Solo a conferma riuscita il pezzo viene legato "
		"alle scritture; se annulli, resta com'era.\n\r",
		ch);
	(void)mob;
}

void show_edit_broker_details_destroy(struct char_data* ch, struct char_data* mob) {
	if(!ch) {
		return;
	}
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dettagli su $c0011distruggi$c0007:");
	send_to_char(
		"Elimini l'edit e ricevi subito il $c001125%$c0007 di quanto e' stato "
		"pagato in piu'\n\r"
		"rispetto al pezzo originale, poi diviso per il numero di classi "
		"(sempre in esperienza).\n\r"
		"Serve un pezzo $c0011EDIT$c0007, $c0011PERSONAL$c0007 e di tua "
		"proprieta'.\n\r",
		ch);
	(void)mob;
}

void show_edit_broker_details_rename(struct char_data* ch, struct char_data* mob) {
	if(!ch) {
		return;
	}
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dettagli su $c0011rinomina$c0007:");
	send_to_char(
		"Costa $c00111 runa$c0007 degli Dei. Solo pezzi nel database "
		"(name / short / description).\n\r"
		"Ricordati che non posso cambiare il nome di un oggetto gia' rinominato.\n\r"
		"Niente codici colore a mano: scegli un $c0011pattern$c0007 e i colori.\n\r"
		"Opzionale: una lettera $c0011lampeggiante$c0007 sulla short "
		"(raddoppia rune e coins).\n\r"
		"Description: puoi colorare $c0011una sola parola$c0007 con un colore a "
		"tua scelta (anche senza pattern sulla short).\n\r"
		"Max $c0011",
		ch);
	{
		std::ostringstream lim;
		lim << kRenameMaxDistinctColorsShort
			<< "$c0007 colori distinti in short; description una parola / "
			<< "$c0011" << kRenameMaxDistinctColorsDesc << "$c0007 colore.\n\r"
			<< "\n\r"
			<< "$c0015Colori:$c0007 base $c0011";
		send_to_char(lim.str().c_str(), ch);
	}
	{
		std::ostringstream os;
		os << kRenameColorBaseCoins << "$c0007 coins il 1° colore; ogni colore diverso "
		   << "successivo +$c0011" << kRenameColorExtraPctPerDistinct
		   << "%$c0007 del base, e si $c0015sommano$c0007.\n\r"
		   << "Esempi: 1=$c0011" << rename_color_coin_cost(1)
		   << "$c0007; 2=$c0011" << rename_color_unit_cost(0) << "$c0007+"
		   << "$c0011" << rename_color_unit_cost(1) << "$c0007=$c0011"
		   << rename_color_coin_cost(2) << "$c0007; 3=$c0011" << rename_color_coin_cost(3)
		   << "$c0007.\n\r"
		   << "Keywords senza colore; short = pattern; description = una parola.\n\r"
		   << "I codici colore si mettono solo al $c0015cambio$c0007; $c00117$c0007 "
		   << "(normale) non scrive nulla finche' non cambi.\n\r"
		   << "Se la short colorata supera $c0011" << kObjInstMaxShort
		   << "$c0007 caratteri (DB), riprova con meno alternanze.\n\r"
		   << "Pattern (es. su \"$c0015" << kRenameExampleText << "$c0007\"):\n\r";
		send_to_char(os.str().c_str(), ch);
	}
	{
		int idx = 1;
		for(const auto& pat : kRenameColorPatterns) {
			std::ostringstream line;
			line << "  $c0011" << idx << ")$c0007 " << pat.label << ": "
				 << rename_pattern_example(pat) << "\n\r";
			send_to_char(line.str().c_str(), ch);
			++idx;
		}
	}
	(void)mob;
}

void show_edit_broker_details(struct char_data* ch, struct char_data* mob,
							  std::string_view which) {
	if(!ch) {
		return;
	}
	const std::string topic = next_arg(which).first;
	if(topic.empty()) {
		broker_ambient_talk(ch, mob);
		tell_from_jeweler(ch, mob,
						  "Posso darti i dettagli di $c0011trasferisci$c0007, "
						  "$c0011distruggi$c0007 o $c0011rinomina$c0007.");
		tell_from_jeweler(ch, mob,
						  "Chiedimi $c0011dettagli trasferisci$c0007, "
						  "$c0011dettagli distruggi$c0007 oppure "
						  "$c0011dettagli rinomina$c0007, a seconda di cosa ti serve.");
		return;
	}
	if(topic == "trasferisci" || topic == "trasferimento" || topic == "transfer") {
		show_edit_broker_details_transfer(ch, mob);
		return;
	}
	if(topic == "distruggi" || topic == "distruggere" || topic == "destroy") {
		show_edit_broker_details_destroy(ch, mob);
		return;
	}
	if(topic == "rinomina" || topic == "rinominare" || topic == "rename") {
		show_edit_broker_details_rename(ch, mob);
		return;
	}
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob,
					  "Non ho dettagli su quello. Prova $c0011dettagli trasferisci$c0007, "
					  "$c0011dettagli distruggi$c0007 o $c0011dettagli rinomina$c0007.");
}

constexpr time_t kEditBrokerPendingTimeoutSec = 120;

enum class EditBrokerOpKind { Transfer, Destroy, Rename };

enum class EditBrokerStep {
	WaitGiveA,		 /* give pezzo di partenza */
	WaitGiveB,		 /* give pezzo di destinazione */
	WaitAffectPick,	 /* numero/nome effetto */
	ConfirmTransfer, /* si/no sul transfer */
	WaitGiveDestroy, /* give oggetto da distruggere */
	ConfirmDestroy,	 /* si/no sulla destroy */
	WaitGiveRename,	 /* give pezzo da rinominare */
	WaitRenameKeywords,
	WaitRenameShort,
	WaitRenameDesc,
	WaitRenamePattern, /* scelta pattern colori (0 = nessuno) */
	WaitRenameColors,  /* scelta colori del pattern */
	WaitRenameDescWord,	   /* quale parola colorare sulla description (1 sola) */
	WaitRenameDescColor,   /* colore a piacere per quella parola */
	WaitRenameBlinkAsk,	   /* si/no lettera blinking (x2 rune+coins) */
	WaitRenameBlinkWord,   /* quale parola della short */
	WaitRenameBlinkLetter, /* quale lettera in quella parola */
	ConfirmRename
};

struct EditBrokerPending {
	char_data* jeweler{};
	EditBrokerOpKind op{};
	EditBrokerStep step{};
	time_t expires_at{};
	std::string aff_name;
	std::string name_a;
	std::string name_b;
	std::string name_c;
	unsigned long long id_a{};
	unsigned long long id_b{};
	unsigned long long id_c{};
	std::vector<std::string> affect_labels;
	std::string rename_keywords;
	std::string rename_short;
	std::string rename_desc;
	int rename_pattern{-1}; /* -1 nessuno; 0..n-1 indice in kRenameColorPatterns */
	int rename_colors[kRenameMaxColorSlots]{};
	int rename_color_count{};
	int rename_colors_needed{};
	int rename_desc_color_word{-1}; /* 0-based sulla description; -1 = nessuna */
	int rename_desc_color_fg{-1};	/* 1..15 scelto dal PG; -1 = nessuno */
	bool rename_blink{};
	int rename_blink_word{-1};	 /* 0-based */
	int rename_blink_letter{-1}; /* 0-based nella parola */
};

std::map<char_data*, EditBrokerPending> g_edit_broker_pending;

void touch_edit_broker_pending(EditBrokerPending& p) {
	p.expires_at = time(nullptr) + kEditBrokerPendingTimeoutSec;
}

[[nodiscard]] std::string obj_first_keyword(const obj_data* obj) {
	if(!obj || !obj->name || !*obj->name) {
		return "?";
	}
	std::string_view n = obj->name;
	auto [kw, rest] = next_arg(n);
	(void)rest;
	return kw.empty() ? std::string("?") : std::string(kw);
}

[[nodiscard]] std::string obj_vis_key(char_data* ch, obj_data* obj) {
	if(!ch || !obj) {
		return "?";
	}
	const std::string kw = obj_first_keyword(obj);
	int n = 1;
	for(obj_data* o = ch->carrying; o; o = o->next_content) {
		if(o == obj) {
			break;
		}
		if(isname(kw.c_str(), o->name)) {
			++n;
		}
	}
	if(n <= 1) {
		return kw;
	}
	return std::to_string(n) + "." + kw;
}

[[nodiscard]] obj_data* find_broker_obj(char_data* ch, const std::string& name,
										 unsigned long long instance_id) {
	if(!ch) {
		return nullptr;
	}
	if(instance_id != 0) {
		for(obj_data* o = ch->carrying; o; o = o->next_content) {
			if(o->db_instance_id == instance_id && CAN_SEE_OBJ(ch, o)) {
				return o;
			}
		}
	}
	if(name.empty()) {
		return nullptr;
	}
	return get_obj_in_list_vis(ch, name.c_str(), ch->carrying);
}

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
		broker_ambient_stop(ch, j);
		tell_from_jeweler(ch, j, "Ok, non faccio niente.");
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
	tell_from_jeweler(ch, mob, "Confermi? $c0011si$c0007 / $c0011nod$c0007.");
	tell_from_jeweler(ch, mob,
					  "Oppure $c0011no$c0007 / $c0011shake$c0007 per annullare "
					  "(anche via say o ask).");
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
		tell_from_jeweler(ch, mob, "Annullo la richiesta precedente.");
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
		tell_from_jeweler(ch, mob, "Mancano pezzi o effetto da trasferire.");
		return false;
	}

	const std::string name_a_str{name_a};
	const std::string name_b_str{name_b};
	struct obj_data* obj_a =
		get_obj_in_list_vis(ch, name_a_str.c_str(), ch->carrying);
	struct obj_data* obj_b =
		get_obj_in_list_vis(ch, name_b_str.c_str(), ch->carrying);
	if(!obj_a) {
		tell_from_jeweler(ch, mob, "Non vedo il pezzo di partenza nel tuo inventario.");
		return false;
	}
	if(!obj_b) {
		tell_from_jeweler(ch, mob, "Non vedo il pezzo di destinazione nel tuo inventario.");
		return false;
	}
	if(obj_a == obj_b) {
		tell_from_jeweler(ch, mob,
						  "Partenza e destinazione devono essere due pezzi distinti.");
		return false;
	}
	if(!obj_is_owned_edit(ch, obj_a)) {
		tell_from_jeweler(ch, mob,
						  "Il pezzo di partenza deve essere EDIT, PERSONAL e di tua "
						  "proprieta'.");
		return false;
	}
	const char* const actor_name = GET_NAME(ch) ? GET_NAME(ch) : "?";
	if(obj_a->db_instance_id == 0) {
		tell_from_jeweler(ch, mob,
						  "Il pezzo di partenza non e' collegato al database edit.");
		tell_from_jeweler(ch, mob, "Contatta uno staffer.");
		std::ostringstream log;
		log << "EditAffectBroker transfer: missing instance_id A owner=" << actor_name;
		editor_mudlog(LOG_ERROR, log.str());
		return false;
	}
	if(obj_personal_owned_by_other(ch, obj_b)) {
		tell_from_jeweler(ch, mob,
						  "Il pezzo di destinazione e' PERSONAL di un altro personaggio.");
		tell_from_jeweler(ch, mob, "Non posso lavorarci.");
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
						  "Questo effetto (arma / slayer / eat-spell) non si puo' "
						  "trasferire qui: rivolgiti a un membro dello staff.");
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
							  "Questo effetto non e' tra quelli trasferibili qui: "
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
			os << "Il pezzo di destinazione ha gia' questo effetto ("
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
							  "Il pezzo di destinazione non ha uno slot libero per questo "
							  "effetto.");
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
							  "Il pezzo di destinazione ha gia' questo effetto: non posso "
							  "trasferirlo.");
			std::ostringstream log;
			log << "EditAffectBroker transfer denied " << actor_name
				<< ": nonstack already on B " << pick.label;
			editor_mudlog(LOG_PLAYERS, log.str());
			return false;
		}
		if(find_free_affect_slot(obj_b) < 0) {
			tell_from_jeweler(ch, mob,
							  "Il pezzo di destinazione non ha uno slot libero per questo "
							  "effetto.");
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
						  "Il pezzo di destinazione non ha gia' lo stesso effetto ne' uno "
						  "slot libero: non posso sommarlo.");
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
		struct obj_data* proto_b = load_broker_prototype(obj_b);
		if(!proto_b) {
			tell_from_jeweler(ch, mob,
							  "Non riesco a leggere l'essenza originale del pezzo di "
							  "destinazione.");
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
				os << "Quel pezzo e' gia' troppo carico di $c0011" << what
				   << "$c0007 rispetto alla sua essenza: +" << edit_now
				   << " da edit (massimo +" << edit_cap
				   << "). Non posso vincolarne ancora, neppure da trasferimento.";
				return refuse_cap(os.str());
			}
			const int room = broker_cap - broker_now;
			take_out = std::min(want, room);
			if(take_out <= 0) {
				std::ostringstream os;
				os << "Quel pezzo non regge altro $c0011" << what
				   << "$c0007: ha gia' +" << edit_now << " da edit e +" << broker_now
				   << " da trasferimenti (massimo +" << edit_cap << " / +" << broker_cap
				   << ", totale +" << (edit_cap + broker_cap)
				   << "). La potenza che vuoi spostare non ci sta.";
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
					"Quel pezzo e' gia' troppo carico di $c0011armor$c0007 rispetto "
					"alla sua essenza (oltre -40 da edit). Non posso vincolarne ancora.");
			}
			const int want = pick.delta < 0 ? -pick.delta : 0;
			const int room = kBrokerPieceArmorCap - broker_improve;
			const int take = std::min(want, room);
			if(take <= 0) {
				extract_obj(proto_b);
				return refuse_cap(
					"Quel pezzo non regge altro $c0011armor$c0007: ha gia' esaurito "
					"il budget da edit e da trasferimento (-40 / -40).");
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
				if(!clamp_channel(cb.hr_total, cb.hr_broker,
								  "hitroll (HITROLL+HIT-N-DAM+HIT-N-SP)", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_DAMROLL) {
				b_prior_edit_delta = std::max(0, cb.dam_total - cb.dam_broker);
				if(!clamp_channel(cb.dam_total, cb.dam_broker,
								  "damroll (DAMROLL+HIT-N-DAM)", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_SPELLPOWER) {
				b_prior_edit_delta = std::max(0, cb.sp_total - cb.sp_broker);
				if(!clamp_channel(cb.sp_total, cb.sp_broker,
								  "spellpower (SPELLPOWER+HIT-N-SP)", take)) {
					extract_obj(proto_b);
					return false;
				}
			}
			else if(pick.location == APPLY_HITNDAM) {
				b_prior_edit_delta = std::max(0, cb.dam_total - cb.dam_broker);
				int t_hr = take;
				int t_dam = take;
				if(!clamp_channel(cb.hr_total, cb.hr_broker,
								  "hitroll (via HIT-N-DAM)", t_hr) ||
				   !clamp_channel(cb.dam_total, cb.dam_broker,
								  "damroll (via HIT-N-DAM)", t_dam)) {
					extract_obj(proto_b);
					return false;
				}
				take = std::min(t_hr, t_dam);
			}
			else if(pick.location == APPLY_HITNSP) {
				b_prior_edit_delta = std::max(0, cb.sp_total - cb.sp_broker);
				int t_hr = take;
				int t_sp = take;
				if(!clamp_channel(cb.hr_total, cb.hr_broker,
								  "hitroll (via HIT-N-SP)", t_hr) ||
				   !clamp_channel(cb.sp_total, cb.sp_broker,
								  "spellpower (via HIT-N-SP)", t_sp)) {
					extract_obj(proto_b);
					return false;
				}
				take = std::min(t_hr, t_sp);
			}
			else {
				extract_obj(proto_b);
				return refuse_cap(
					"Questa essenza numerica non si lascia spostare dalle mie mani.");
			}
			if(take <= 0) {
				extract_obj(proto_b);
				return refuse_cap(
					"Quel pezzo ha gia' assorbito tutta la potenza che puo' reggere "
					"su questo effetto: non c'e' spazio per il trasferimento.");
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
								  "Le scritture dell'ombra non mi rispondono: non posso "
								  "pesare il tuo DAM+SP adesso. Riprova tra poco.");
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
			/* Quanto togliamo da A: delta effective del canale, non lo slot grezzo. */
			struct obj_data* proto_a = load_broker_prototype(obj_a);
			if(proto_a) {
				const CombatEffective dA = combat_effective_delta(obj_a, proto_a);
				if(loc == APPLY_DAMROLL || loc == APPLY_HITNDAM) {
					rem_dam = std::min(static_cast<long long>(pick.delta),
									   static_cast<long long>(dA.dam));
				}
				if(loc == APPLY_SPELLPOWER || loc == APPLY_HITNSP) {
					rem_sp = std::min(static_cast<long long>(pick.delta),
									  static_cast<long long>(dA.sp));
				}
				extract_obj(proto_a);
			}
			const long long before = cur.dam + cur.sp;
			const long long projected =
				before - rem_dam - rem_sp + add_dam + add_sp;
			if(projected > kObjInstListinoMaxDamSp && projected > before) {
				std::ostringstream os;
				os << "Il tuo spirito non reggerebbe tanta potenza: DAM+SP "
				   << "passerebbe a $c0011" << static_cast<long>(projected)
				   << "$c0007 (massimo $c001130$c0007). Hit-n-dam conta come danno, "
				   << "hit-n-sp come spellpower.";
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
				os << "Sei gia' oltre il limite di DAM+SP ($c0011"
				   << static_cast<long>(before)
				   << "$c0007/30): non aggiungo altra potenza di colpo o magia.";
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
	const long payment = toon_xp_share(ch, fee); /* 25% listino / classi */
	const int nclass = HowManyClasses(ch);
	const int classes = nclass > 0 ? nclass : 1;
	if(!can_afford_prince_floor(ch, payment)) {
		std::ostringstream os;
		os << "Esperienza insufficiente: servono " << payment
		   << " XP (minimo 400 milioni).";
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
	   << "  Effetto: $c0011" << plan.pick.label << "$c0007 (trasferimento)\n\r"
	   << "  Da (tolgo): " << obj_shortn(plan.obj_a) << "\n\r"
	   << "  Su (metto): " << obj_shortn(plan.obj_b);
	if(plan.had_stack_cap) {
		if(plan.pick.location == APPLY_AC) {
			os << " (edit gia' presente: " << plan.b_prior_edit_delta
			   << "; tetti edit " << -plan.existing_cap << " / trasferimento "
			   << -plan.transfer_cap << " / totale " << -plan.total_cap << ")";
		}
		else {
			os << " (edit gia' presente: " << std::showpos << plan.b_prior_edit_delta
			   << std::noshowpos << "; tetti edit " << plan.existing_cap
			   << " / trasferimento " << plan.transfer_cap << " / totale "
			   << plan.total_cap << ")";
		}
	}
	os << "\n\r"
	   << "  Costo: " << plan.payment << " esperienza.\n\r"
	   << "  Dopo il pagamento non potrai scendere sotto i 400 milioni di esperienza.";
	tell_from_jeweler(ch, mob, os.str());
	if(GetMaxLevel(ch) >= IMMORTALE && procarea_obj_is_reward(plan.obj_b) &&
	   plan.obj_b->db_instance_id == 0) {
		tell_from_jeweler(ch, mob,
						  "[wiz] Tetti gia' su baseline sospesa (rolled). DB solo se il "
						  "commit del transfer riesce; annulla = zero modifiche.");
	}
	ask_edit_broker_confirm(ch, mob);
}

[[nodiscard]] bool build_destroy_plan(struct char_data* ch, struct char_data* mob,
										std::string_view name_c, DestroyPlan& out) {
	if(!ch || !mob) {
		return false;
	}
	if(name_c.empty()) {
		tell_from_jeweler(ch, mob, "Serve l'oggetto da distruggere.");
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

	ObjEditAnalysis edit;
	if(procarea_obj_is_reward(obj)) {
#if USE_MYSQL
		struct obj_data* baseline =
			object_instance_materialize_create_baseline(obj->db_instance_id);
		if(!baseline) {
			tell_from_jeweler(ch, mob,
							  "Non riesco a ricostruire la baseline del premio procarea. "
							  "Contatta uno staffer.");
			std::ostringstream log;
			log << "EditAffectBroker destroy: create baseline fail inst="
				<< static_cast<unsigned long long>(obj->db_instance_id) << " owner="
				<< (GET_NAME(ch) ? GET_NAME(ch) : "?");
			editor_mudlog(LOG_ERROR, log.str());
			return false;
		}
		edit = AnalyzeObjEditAgainst(obj, baseline, true);
		extract_obj(baseline);
#else
		tell_from_jeweler(ch, mob, "Database non disponibile per i premi procarea.");
		return false;
#endif
	}
	else {
		edit = AnalyzeObjEdit(obj);
	}
	const int nclass = HowManyClasses(ch);
	const int classes = nclass > 0 ? nclass : 1;
	const long fee = percent_of_listino(edit.diff.valore);
	const long refund = toon_xp_share(ch, fee); /* 25% listino / classi */

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
	   << " esperienza (25% del listino di edit / " << plan.classes
	   << " classi).\n\r"
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
	broker_ambient_work_transfer(ch, mob);

	/* Snapshot memoria PRIMA di ogni mutazione: fino al commit riuscito niente DB
	 * su B nuovo; cancel/timeout/disconnect lasciano il pezzo invariato. */
	const BrokerTransferSnap snap = BrokerTransferSnap::capture(plan.obj_a, plan.obj_b);
	const bool b_new_procarea =
		procarea_obj_is_reward(plan.obj_b) && plan.obj_b->db_instance_id == 0;
	const obj_affected_type* const b_create_baseline =
		b_new_procarea ? snap.affects_b : nullptr;

	auto abort_transfer = [&](std::string_view player_msg, const char* log_tag) {
		snap.restore(plan.obj_a, plan.obj_b);
		broker_ambient_stop(ch, mob);
		if(!player_msg.empty()) {
			tell_from_jeweler(ch, mob, player_msg);
		}
		std::ostringstream log;
		log << "EditAffectBroker transfer aborted " << actor_name << ": " << log_tag
			<< " affect=" << plan.pick.label;
		editor_mudlog(LOG_PLAYERS, log.str());
	};

	if(is_bitfield_location(plan.pick.location)) {
		remove_bit_delta(plan.obj_a, plan.pick.location,
						 static_cast<unsigned>(plan.pick.delta));
		if(!add_bit_delta_new_slot(plan.obj_b, plan.pick.location,
								   static_cast<unsigned>(plan.pick.delta))) {
			abort_transfer(
				"Operazione fallita. Sul pezzo di destinazione non c'e' piu' uno slot "
				"libero.",
				"race no slot B bit");
			return;
		}
	}
	else if(is_nonstackable_location(plan.pick.location)) {
		remove_numeric_delta(plan.obj_a, plan.pick.location, plan.pick.delta);
		const int free_slot = find_free_affect_slot(plan.obj_b);
		if(free_slot < 0) {
			abort_transfer(
				"Operazione fallita. Sul pezzo di destinazione non c'e' piu' uno slot "
				"libero.",
				"race no slot B nonstack");
			return;
		}
		plan.obj_b->affected[free_slot].location = static_cast<short>(plan.pick.location);
		plan.obj_b->affected[free_slot].modifier = plan.pick.delta;
	}
	else {
		remove_numeric_delta(plan.obj_a, plan.pick.location, plan.pick.delta);
		add_numeric_delta(plan.obj_b, plan.pick.location, plan.pick.delta);
	}

	/* "HITROLL By 0" / simili: in analisi sono slot liberi; dopo il transfer su B
	 * li normalizziamo a APPLY_NONE cosi' non restano fantasmi. */
	clear_zero_modifier_slots(plan.obj_b);

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
	note_a_os << "trasferimento REMOVE " << plan.pick.label << " -> B_inst="
			  << static_cast<unsigned long long>(plan.obj_b->db_instance_id);
	const std::string note_a = note_a_os.str();

	std::ostringstream note_b_os;
	note_b_os << "trasferimento ADD " << plan.pick.label << " <- A_inst="
			  << static_cast<unsigned long long>(plan.obj_a->db_instance_id) << ' '
			  << caps_part;
	const std::string note_b = note_b_os.str();

	std::ostringstream xfer_os;
	xfer_os << static_cast<unsigned long long>(plan.obj_a->db_instance_id) << '-'
			<< static_cast<unsigned long long>(plan.obj_b->db_instance_id) << '-'
			<< static_cast<long>(time(nullptr)) << '-' << plan.pick.location << '-'
			<< plan.pick.delta;
	const std::string xfer_id = xfer_os.str();

	std::ostringstream detail_os;
	detail_os << "channel=trasferimento kind=broker_transfer affect=" << plan.pick.label
			  << " location=" << plan.pick.location << " broker_delta=" << std::showpos
			  << plan.pick.delta << std::noshowpos << ' ' << caps_part
			  << " A_inst=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			  << " B_inst=" << static_cast<unsigned long long>(plan.obj_b->db_instance_id)
			  << " payment_xp=" << plan.payment << " fee=" << plan.fee
			  << " listino=" << plan.listino << " classes=" << plan.classes
			  << " actor=" << actor_name << " xfer_id=" << xfer_id;
	const std::string detail = detail_os.str();

	if(b_new_procarea) {
		tell_from_jeweler(ch, mob,
						  "Un attimo: prima imprimo l'essenza di questo premio nelle "
						  "scritture dell'ombra...");
	}

	/* Una sola TX: create-baseline (se premio nuovo) + persist A+B + history.
	 * Fallimento → rollback TX + restore memoria; XP non ancora addebitato. */
	const bool ok_commit = object_instance_broker_transfer_commit(
		plan.obj_a, plan.obj_b, ch, note_a.c_str(), note_b.c_str(), detail.c_str(),
		xfer_id.c_str(), b_create_baseline);
	if(!ok_commit) {
		snap.restore(plan.obj_a, plan.obj_b);
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob,
						  "Trasferimento annullato: il legame non ha retto.");
		tell_from_jeweler(ch, mob, "Nessuna esperienza e' stata addebitata.");
		std::ostringstream log;
		log << "EditAffectBroker transfer persist fail+rollback owner=" << actor_name
			<< " A=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			<< " B_was_new=" << b_new_procarea << " xfer=" << xfer_id << ' '
			<< plan.pick.label;
		editor_mudlog(LOG_SYSERR, log.str());
		return;
	}

	/*
	 * XP + broker_xfer_done solo via reconcile (stesso path di boot/login):
	 * evita doppi addebiti se mark_done fallisce dopo aver scalato XP.
	 */
	schedule_inventory_save(ch);
	const unsigned closed = object_instance_broker_reconcile(actor_name, false);
	broker_ambient_done(ch, mob);
	if(closed == 0) {
		tell_from_jeweler(ch, mob, "I pezzi sono aggiornati.");
		tell_from_jeweler(ch, mob,
						  "Non sono riuscito a chiudere il pagamento ora: verra' "
						  "completato al prossimo login o reboot.");
		std::ostringstream log;
		log << "EditAffectBroker transfer commit OK but reconcile pending xfer=" << xfer_id
			<< " actor=" << actor_name << " pay=" << plan.payment;
		editor_mudlog(LOG_SYSERR, log.str());
	}
	else {
		{
			std::ostringstream ok;
			ok << "$c0015Fatto:$c0007 trasferito $c0011" << plan.pick.label
			   << "$c0007.";
			tell_from_jeweler(ch, mob, ok.str());
		}
		{
			std::ostringstream ok;
			ok << "Ti ho addebitato $c0011" << plan.payment
			   << "$c0007 esperienza";
			if(plan.classes >= 2) {
				ok << " per classe";
			}
			ok << ".";
			tell_from_jeweler(ch, mob, ok.str());
		}
	}
	save_char(ch, AUTO_RENT, 0);
	{
		std::ostringstream log;
		log << "EditAffectBroker transfer OK " << actor_name
			<< " affect=" << plan.pick.label << " broker_delta=" << plan.pick.delta
			<< " B_prior_edit_delta=" << plan.b_prior_edit_delta
			<< " A_inst=" << static_cast<unsigned long long>(plan.obj_a->db_instance_id)
			<< " B_inst=" << static_cast<unsigned long long>(plan.obj_b->db_instance_id)
			<< " xfer=" << xfer_id << " pay=" << plan.payment << " fee=" << plan.fee
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
	broker_ambient_work_destroy(ch, mob, plan.obj);

	if(!object_instance_delete(plan.inst, ch)) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "Non sono riuscito a cancellare l'edit nel database.");
		tell_from_jeweler(ch, mob, "Ok, non faccio niente.");
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

	broker_ambient_done(ch, mob);
	{
		std::ostringstream ok;
		ok << "$c0015Fatto:$c0007 ho distrutto $c0011" << plan.shortn << "$c0007.";
		tell_from_jeweler(ch, mob, ok.str());
	}
	{
		std::ostringstream ok;
		ok << "Ti rimborso $c0011" << plan.refund << "$c0007 esperienza";
		if(plan.classes >= 2) {
			ok << " per classe";
		}
		ok << ".";
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
	return word == "trasferisci" || word == "distruggi" || word == "rinomina" ||
		   word == "aiuto" || word == "help" || word == "info" || word == "dettagli" ||
		   word == "regole";
}

[[nodiscard]] bool pending_is_confirm_step(EditBrokerStep step) {
	return step == EditBrokerStep::ConfirmTransfer ||
		   step == EditBrokerStep::ConfirmDestroy || step == EditBrokerStep::ConfirmRename;
}

[[nodiscard]] bool pending_is_rename_text_step(EditBrokerStep step) {
	return step == EditBrokerStep::WaitRenameKeywords ||
		   step == EditBrokerStep::WaitRenameShort ||
		   step == EditBrokerStep::WaitRenameDesc;
}

[[nodiscard]] bool pending_is_rename_style_step(EditBrokerStep step) {
	return step == EditBrokerStep::WaitRenamePattern ||
		   step == EditBrokerStep::WaitRenameColors;
}

[[nodiscard]] bool pending_is_rename_desc_word_step(EditBrokerStep step) {
	return step == EditBrokerStep::WaitRenameDescWord ||
		   step == EditBrokerStep::WaitRenameDescColor;
}

[[nodiscard]] bool pending_is_rename_blink_step(EditBrokerStep step) {
	return step == EditBrokerStep::WaitRenameBlinkAsk ||
		   step == EditBrokerStep::WaitRenameBlinkWord ||
		   step == EditBrokerStep::WaitRenameBlinkLetter;
}

void preview_rename(char_data* ch, char_data* mob, const EditBrokerPending& pending,
					obj_data* obj);
void execute_rename(char_data* ch, char_data* mob, EditBrokerPending& pending);
void show_rename_partial_preview(char_data* ch, char_data* mob,
								 const EditBrokerPending& pending);
void prompt_rename_colors(char_data* ch, char_data* mob, EditBrokerPending& pending);
void begin_rename_desc_word_ask(char_data* ch, char_data* mob, EditBrokerPending& pending);
void show_rename_desc_words(char_data* ch, char_data* mob, const EditBrokerPending& pending);
void begin_rename_desc_color_ask(char_data* ch, char_data* mob, EditBrokerPending& pending);
void prompt_rename_desc_color(char_data* ch, char_data* mob, EditBrokerPending& pending);
void begin_rename_blink_ask(char_data* ch, char_data* mob, EditBrokerPending& pending);
void show_rename_blink_words(char_data* ch, char_data* mob, const EditBrokerPending& pending);
void show_rename_blink_letters(char_data* ch, char_data* mob,
							   const EditBrokerPending& pending);
void begin_rename_confirm(char_data* ch, char_data* mob, EditBrokerPending& pending);
[[nodiscard]] bool build_rename_final_texts(const EditBrokerPending& pending,
											std::string& out_kw, std::string& out_short,
											std::string& out_desc, std::string& err);
void clear_rename_blink(EditBrokerPending& pending);

[[nodiscard]] bool is_broker_shake_cancel(std::string_view text) {
	return next_arg(text).first == "shake";
}

[[nodiscard]] bool is_rename_back_command(std::string_view text) {
	const std::string word = next_arg(text).first;
	return word == "indietro" || word == "i" || word == "back" || word == "b" ||
		   word == "prev" || word == "precedente";
}

[[nodiscard]] bool pending_is_rename_wizard_step(EditBrokerStep step) {
	return pending_is_rename_text_step(step) || pending_is_rename_style_step(step) ||
		   pending_is_rename_desc_word_step(step) || pending_is_rename_blink_step(step) ||
		   step == EditBrokerStep::ConfirmRename || step == EditBrokerStep::WaitGiveRename;
}

void clear_rename_desc_tint(EditBrokerPending& pending) {
	pending.rename_desc_color_word = -1;
	pending.rename_desc_color_fg = -1;
}

void clear_rename_after_keywords(EditBrokerPending& pending) {
	pending.rename_short.clear();
	pending.rename_desc.clear();
	pending.rename_pattern = -1;
	pending.rename_color_count = 0;
	pending.rename_colors_needed = 0;
	clear_rename_desc_tint(pending);
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
}

void clear_rename_after_short(EditBrokerPending& pending) {
	pending.rename_desc.clear();
	pending.rename_pattern = -1;
	pending.rename_color_count = 0;
	pending.rename_colors_needed = 0;
	clear_rename_desc_tint(pending);
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
}

void clear_rename_after_desc(EditBrokerPending& pending) {
	pending.rename_pattern = -1;
	pending.rename_color_count = 0;
	pending.rename_colors_needed = 0;
	clear_rename_desc_tint(pending);
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
}

void clear_rename_style(EditBrokerPending& pending) {
	pending.rename_pattern = -1;
	pending.rename_color_count = 0;
	pending.rename_colors_needed = 0;
	clear_rename_desc_tint(pending);
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
}

void clear_rename_blink(EditBrokerPending& pending) {
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
}

void prompt_rename_current_step(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	if(!ch || !mob) {
		return;
	}
	touch_edit_broker_pending(pending);
	switch(pending.step) {
	case EditBrokerStep::WaitGiveRename:
		tell_from_jeweler(ch, mob, "Dammi il pezzo da rinominare.");
		tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
		break;
	case EditBrokerStep::WaitRenameKeywords:
		show_rename_partial_preview(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Dimmi le $c0011keywords$c0007 (nome per get/give), "
						  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameShort:
		show_rename_partial_preview(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Ora dimmi la $c0011short$c0007 (nome in inventario), "
						  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameDesc:
		show_rename_partial_preview(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Ora dimmi la $c0011description$c0007 (come appare a terra), "
						  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenamePattern:
		show_rename_partial_preview(ch, mob, pending);
		show_rename_pattern_menu(ch, mob, pending.rename_short);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameColors:
		show_rename_partial_preview(ch, mob, pending);
		prompt_rename_colors(ch, mob, pending);
		break;
	case EditBrokerStep::WaitRenameDescWord:
		show_rename_partial_preview(ch, mob, pending);
		show_rename_desc_words(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameDescColor:
		show_rename_partial_preview(ch, mob, pending);
		prompt_rename_desc_color(ch, mob, pending);
		break;
	case EditBrokerStep::WaitRenameBlinkAsk:
		begin_rename_blink_ask(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameBlinkWord:
		show_rename_partial_preview(ch, mob, pending);
		show_rename_blink_words(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::WaitRenameBlinkLetter:
		show_rename_partial_preview(ch, mob, pending);
		show_rename_blink_letters(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	case EditBrokerStep::ConfirmRename:
		begin_rename_confirm(ch, mob, pending);
		tell_from_jeweler(ch, mob,
						  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
		break;
	default:
		break;
	}
}

/** Torna allo step precedente del wizard rinomina. true = gestito. */
bool try_rename_wizard_back(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	if(!ch || !mob || pending.op != EditBrokerOpKind::Rename) {
		return false;
	}
	switch(pending.step) {
	case EditBrokerStep::WaitGiveRename:
		tell_from_jeweler(ch, mob,
						  "Sei gia' all'inizio: $c0011shake$c0007 per annullare.");
		return true;
	case EditBrokerStep::WaitRenameKeywords:
		pending.step = EditBrokerStep::WaitGiveRename;
		pending.name_c.clear();
		pending.id_c = 0;
		clear_rename_after_keywords(pending);
		pending.rename_keywords.clear();
		tell_from_jeweler(ch, mob, "Ok, torniamo indietro.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameShort:
		clear_rename_after_keywords(pending);
		pending.step = EditBrokerStep::WaitRenameKeywords;
		tell_from_jeweler(ch, mob, "Ok, torniamo alle keywords.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameDesc:
		clear_rename_after_short(pending);
		pending.step = EditBrokerStep::WaitRenameShort;
		tell_from_jeweler(ch, mob, "Ok, torniamo alla short.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenamePattern:
		clear_rename_after_desc(pending);
		pending.step = EditBrokerStep::WaitRenameDesc;
		tell_from_jeweler(ch, mob, "Ok, torniamo alla description.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameColors:
		clear_rename_style(pending);
		pending.step = EditBrokerStep::WaitRenamePattern;
		tell_from_jeweler(ch, mob, "Ok, torniamo ai pattern.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameDescWord:
		clear_rename_desc_tint(pending);
		if(pending.rename_pattern >= 0) {
			pending.rename_color_count = 0;
			pending.step = EditBrokerStep::WaitRenameColors;
			tell_from_jeweler(ch, mob, "Ok, torniamo ai colori della short.");
		}
		else {
			pending.step = EditBrokerStep::WaitRenamePattern;
			tell_from_jeweler(ch, mob, "Ok, torniamo ai pattern.");
		}
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameDescColor:
		pending.rename_desc_color_fg = -1;
		pending.step = EditBrokerStep::WaitRenameDescWord;
		tell_from_jeweler(ch, mob, "Ok, torniamo alla parola della description.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameBlinkAsk:
		clear_rename_blink(pending);
		if(pending.rename_desc_color_word >= 0) {
			pending.rename_desc_color_fg = -1;
			pending.step = EditBrokerStep::WaitRenameDescColor;
			tell_from_jeweler(ch, mob, "Ok, torniamo al colore della description.");
		}
		else {
			clear_rename_desc_tint(pending);
			pending.step = EditBrokerStep::WaitRenameDescWord;
			tell_from_jeweler(ch, mob, "Ok, torniamo alla parola della description.");
		}
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameBlinkWord:
		clear_rename_blink(pending);
		pending.step = EditBrokerStep::WaitRenameBlinkAsk;
		tell_from_jeweler(ch, mob, "Ok, torniamo alla scelta blink.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::WaitRenameBlinkLetter:
		pending.rename_blink_letter = -1;
		pending.step = EditBrokerStep::WaitRenameBlinkWord;
		tell_from_jeweler(ch, mob, "Ok, torniamo alla scelta della parola.");
		prompt_rename_current_step(ch, mob, pending);
		return true;
	case EditBrokerStep::ConfirmRename:
		if(pending.rename_blink && pending.rename_blink_letter >= 0) {
			pending.rename_blink_letter = -1;
			pending.step = EditBrokerStep::WaitRenameBlinkLetter;
			tell_from_jeweler(ch, mob, "Ok, torniamo alla lettera lampeggiante.");
		}
		else if(pending.rename_blink) {
			pending.step = EditBrokerStep::WaitRenameBlinkWord;
			tell_from_jeweler(ch, mob, "Ok, torniamo alla parola da far lampeggiare.");
		}
		else {
			pending.step = EditBrokerStep::WaitRenameBlinkAsk;
			tell_from_jeweler(ch, mob, "Ok, torniamo alla scelta blink.");
		}
		prompt_rename_current_step(ch, mob, pending);
		return true;
	default:
		return false;
	}
}

[[nodiscard]] int pending_rename_rune_cost(const EditBrokerPending& pending) {
	return kRenameRuneCost * (pending.rename_blink ? 2 : 1);
}

[[nodiscard]] long pending_rename_coin_cost(const EditBrokerPending& pending) {
	constexpr int kPaletteCap = kRenameMaxColorSlots + 1;
	int palette[kPaletteCap]{};
	int n = 0;
	if(pending.rename_pattern >= 0 && pending.rename_color_count >= 1) {
		n = std::min(pending.rename_color_count, kRenameMaxColorSlots);
		for(int i = 0; i < n; ++i) {
			palette[i] = pending.rename_colors[i];
		}
	}
	if(pending.rename_desc_color_word >= 0 && pending.rename_desc_color_fg >= 1 &&
	   pending.rename_desc_color_fg != kRenameFgNormal && n < kPaletteCap) {
		palette[n++] = pending.rename_desc_color_fg;
	}
	long coins = 0;
	if(n >= 1) {
		coins = rename_color_coin_cost(count_distinct_rename_colors(palette, n));
	}
	if(pending.rename_blink) {
		coins *= 2;
	}
	return coins;
}

void begin_rename_confirm(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	{
		std::string kw;
		std::string sh;
		std::string ds;
		std::string err;
		if(!build_rename_final_texts(pending, kw, sh, ds, err)) {
			tell_from_jeweler(ch, mob, err.empty() ? "Testo non valido." : err);
			begin_rename_blink_ask(ch, mob, pending);
			return;
		}
		if(sh.size() > kObjInstMaxShort) {
			tell_from_jeweler(ch, mob,
							  "Troppe alternanze di colori (o blink): la short non "
							  "sta nel limite consentito.");
			{
				std::ostringstream os;
				os << "Lunghezza $c0011" << sh.size() << "$c0007 / max $c0011"
				   << kObjInstMaxShort << "$c0007.";
				tell_from_jeweler(ch, mob, os.str());
			}
			if(pending.rename_blink) {
				clear_rename_blink(pending);
				begin_rename_blink_ask(ch, mob, pending);
			}
			else if(pending.rename_pattern >= 0) {
				pending.rename_color_count = 0;
				pending.step = EditBrokerStep::WaitRenameColors;
				prompt_rename_colors(ch, mob, pending);
			}
			return;
		}
		if(ds.size() > kObjInstMaxDesc) {
			tell_from_jeweler(ch, mob, "La description colorata e' troppo lunga.");
			{
				std::ostringstream os;
				os << "Lunghezza $c0011" << ds.size() << "$c0007 / max $c0011"
				   << kObjInstMaxDesc << "$c0007.";
				tell_from_jeweler(ch, mob, os.str());
			}
			clear_rename_desc_tint(pending);
			pending.step = EditBrokerStep::WaitRenameDescWord;
			show_rename_desc_words(ch, mob, pending);
			return;
		}
	}
	pending.step = EditBrokerStep::ConfirmRename;
	touch_edit_broker_pending(pending);
	obj_data* obj = find_broker_obj(ch, pending.name_c, pending.id_c);
	if(!obj) {
		cancel_edit_broker_pending(ch, mob, true);
		tell_from_jeweler(ch, mob, "Non trovo piu' l'oggetto nel tuo inventario.");
		return;
	}
	preview_rename(ch, mob, pending, obj);
}

void show_rename_desc_words(char_data* ch, char_data* mob, const EditBrokerPending& pending) {
	const auto words = split_rename_words(pending.rename_desc);
	tell_from_jeweler(ch, mob,
					  "Sulla $c0011description$c0007 puoi colorare $c0015una sola "
					  "parola$c0007, con un colore a tua scelta.");
	tell_from_jeweler(ch, mob,
					  "Scegli il $c0011numero$c0007 della parola, oppure $c00110$c0007 / "
					  "$c0011nessuna$c0007.");
	for(std::size_t i = 0; i < words.size(); ++i) {
		std::ostringstream line;
		line << "  $c0011" << (i + 1) << ")$c0007 " << words[i] << "\n\r";
		send_to_char(line.str().c_str(), ch);
	}
}

void prompt_rename_desc_color(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	if(!ch || !mob) {
		return;
	}
	(void)pending;
	tell_from_jeweler(ch, mob,
					  "Dimmi il $c0011colore$c0007 per quella parola (nome o numero), "
					  "oppure $c00117$c0007 / $c0011normale$c0007 senza tinta.");
	show_rename_color_palette(ch, mob);
	tell_from_jeweler(ch, mob,
					  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
}

void begin_rename_desc_color_ask(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	pending.rename_desc_color_fg = -1;
	pending.step = EditBrokerStep::WaitRenameDescColor;
	touch_edit_broker_pending(pending);
	show_rename_partial_preview(ch, mob, pending);
	prompt_rename_desc_color(ch, mob, pending);
}

void begin_rename_desc_word_ask(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	clear_rename_desc_tint(pending);
	pending.step = EditBrokerStep::WaitRenameDescWord;
	touch_edit_broker_pending(pending);
	show_rename_partial_preview(ch, mob, pending);
	const auto words = split_rename_words(pending.rename_desc);
	if(words.empty()) {
		tell_from_jeweler(ch, mob, "La description non ha parole da colorare.");
		begin_rename_blink_ask(ch, mob, pending);
		return;
	}
	show_rename_desc_words(ch, mob, pending);
}

void begin_rename_blink_ask(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	pending.rename_blink = false;
	pending.rename_blink_word = -1;
	pending.rename_blink_letter = -1;
	pending.step = EditBrokerStep::WaitRenameBlinkAsk;
	touch_edit_broker_pending(pending);
	show_rename_partial_preview(ch, mob, pending);
	tell_from_jeweler(ch, mob,
					  "Vuoi una $c0011lettera lampeggiante$c0007 sulla short? "
					  "($c0011si$c0007 / $c0011no$c0007)");
	tell_from_jeweler(ch, mob, "Costa il $c0015doppio$c0007 di rune e di coins.");
}

void show_rename_blink_words(char_data* ch, char_data* mob, const EditBrokerPending& pending) {
	const auto words = split_rename_words(pending.rename_short);
	tell_from_jeweler(ch, mob, "Scegli la $c0011parola$c0007 (numero):");
	for(std::size_t i = 0; i < words.size(); ++i) {
		std::ostringstream line;
		line << "  $c0011" << (i + 1) << ")$c0007 " << words[i] << "\n\r";
		send_to_char(line.str().c_str(), ch);
	}
}

void show_rename_blink_letters(char_data* ch, char_data* mob,
							   const EditBrokerPending& pending) {
	const auto words = split_rename_words(pending.rename_short);
	if(pending.rename_blink_word < 0 ||
	   static_cast<std::size_t>(pending.rename_blink_word) >= words.size()) {
		return;
	}
	const std::string& w = words[static_cast<std::size_t>(pending.rename_blink_word)];
	{
		std::ostringstream os;
		os << "Parola \"$c0011" << w << "$c0007\": scegli la $c0011lettera$c0007 (numero).";
		tell_from_jeweler(ch, mob, os.str());
	}
	std::ostringstream line;
	for(std::size_t i = 0; i < w.size(); ++i) {
		if(i) {
			line << "  ";
		}
		line << "$c0011" << (i + 1) << ")$c0007" << w[i];
	}
	line << "\n\r";
	send_to_char(line.str().c_str(), ch);
}

[[nodiscard]] bool build_rename_final_texts(const EditBrokerPending& pending,
											std::string& out_kw, std::string& out_short,
											std::string& out_desc, std::string& err) {
	out_kw.clear();
	out_short.clear();
	out_desc.clear();
	err.clear();
	std::string kw;
	std::string sh;
	std::string ds;
	if(!validate_rename_field(pending.rename_keywords, kRenameMaxKeywords, "keywords", kw,
							  err) ||
	   !validate_rename_field(pending.rename_short, kRenameMaxShort, "short", sh, err) ||
	   !validate_rename_field(pending.rename_desc, kRenameMaxDesc, "description", ds,
							  err)) {
		return false;
	}
	out_kw = kw;
	const std::string plain_desc_body = ds;
	ds = finalize_rename_description(ds);

	const int blink_w = pending.rename_blink ? pending.rename_blink_word : -1;
	const int blink_l = pending.rename_blink ? pending.rename_blink_letter : -1;

	if(pending.rename_pattern >= 0) {
		constexpr int npat =
			static_cast<int>(sizeof(kRenameColorPatterns) / sizeof(kRenameColorPatterns[0]));
		if(pending.rename_pattern >= npat) {
			err = "Pattern non valido.";
			return false;
		}
		const auto& pat = kRenameColorPatterns[pending.rename_pattern];
		const int need = rename_pattern_slots_for(pat, sh);
		if(pending.rename_color_count < need) {
			err = "Colori del pattern incompleti.";
			return false;
		}
		out_short = apply_rename_color_pattern(sh, pat.kind, pending.rename_colors,
											   pending.rename_color_count, blink_w, blink_l);
	}
	else if(pending.rename_blink) {
		out_short = apply_rename_color_pattern(sh, RenamePatternKind::Solid, nullptr, 0,
											   blink_w, blink_l);
	}
	else {
		out_short = sh;
	}

	/* Description: al piu' una parola, colore scelto a parte dal pattern short. */
	{
		std::string colored_body = plain_desc_body;
		if(pending.rename_desc_color_word >= 0 && pending.rename_desc_color_fg >= 1) {
			colored_body = apply_rename_one_word_color(
				plain_desc_body, pending.rename_desc_color_word,
				pending.rename_desc_color_fg);
		}
		if(!rename_desc_already_grounded(plain_desc_body)) {
			out_desc = colored_body + "$c0007" + kRenameGroundSuffix;
		}
		else {
			out_desc = colored_body;
		}
	}
	if(!rename_field_color_count_ok(out_short, kRenameMaxDistinctColorsShort, "short", err) ||
	   !rename_field_color_count_ok(out_desc, kRenameMaxDistinctColorsDesc, "description",
									err)) {
		return false;
	}
	return true;
}

/** Anteprima best-effort con i campi gia' scelti (anche colori parziali). */
void show_rename_partial_preview(char_data* ch, char_data* mob,
								 const EditBrokerPending& pending) {
	if(!ch || !mob) {
		return;
	}
	if(pending.rename_keywords.empty() && pending.rename_short.empty() &&
	   pending.rename_desc.empty()) {
		return;
	}
	std::string sh = pending.rename_short;
	std::string ds = pending.rename_desc;
	if(!sh.empty() || !ds.empty()) {
		const int blink_w = pending.rename_blink ? pending.rename_blink_word : -1;
		const int blink_l = pending.rename_blink ? pending.rename_blink_letter : -1;
		const int* colors =
			(pending.rename_color_count > 0) ? pending.rename_colors : nullptr;
		const int nslots = pending.rename_color_count;
		RenamePatternKind kind = RenamePatternKind::Solid;
		if(pending.rename_pattern >= 0) {
			constexpr int npat = static_cast<int>(sizeof(kRenameColorPatterns) /
												  sizeof(kRenameColorPatterns[0]));
			if(pending.rename_pattern < npat) {
				kind = kRenameColorPatterns[pending.rename_pattern].kind;
			}
		}
		if(!sh.empty() && (colors || pending.rename_blink)) {
			sh = apply_rename_color_pattern(sh, kind, colors, nslots, blink_w, blink_l);
		}
		if(!ds.empty()) {
			std::string body = ds;
			if(pending.rename_desc_color_word >= 0 && pending.rename_desc_color_fg >= 1) {
				body = apply_rename_one_word_color(ds, pending.rename_desc_color_word,
												   pending.rename_desc_color_fg);
			}
			if(!rename_desc_already_grounded(ds)) {
				ds = body + "$c0007" + kRenameGroundSuffix;
			}
			else {
				ds = body;
			}
		}
	}
	std::ostringstream os;
	os << "$c0015Anteprima parziale$c0007\n\r";
	if(!pending.rename_keywords.empty()) {
		os << "  Keywords: $c0011" << pending.rename_keywords << "$c0007\n\r";
	}
	if(!sh.empty()) {
		os << "  Short: " << sh << "$c0007\n\r";
	}
	if(!ds.empty()) {
		os << "  Description: " << ds << "$c0007\n\r";
	}
	if(pending.rename_pattern >= 0 && pending.rename_color_count > 0 &&
	   pending.rename_color_count < pending.rename_colors_needed) {
		os << "  ($c0008colori " << pending.rename_color_count << "/"
		   << pending.rename_colors_needed << "$c0007)\n\r";
	}
	send_to_char(os.str().c_str(), ch);
}

void prompt_rename_colors(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	if(!ch || !mob) {
		return;
	}
	const int need = pending.rename_colors_needed;
	if(need < 1) {
		return;
	}
	{
		std::ostringstream os;
		os << "Dimmi i $c0011" << need << "$c0007 color"
		   << (need == 1 ? "e" : "i")
		   << " in $c0015una sola riga$c0007 (nome o numero), nell'ordine.";
		tell_from_jeweler(ch, mob, os.str());
	}
	if(pending.rename_pattern >= 0) {
		const auto& pat = kRenameColorPatterns[pending.rename_pattern];
		if(pat.kind == RenamePatternKind::PerWord) {
			const auto words = split_rename_words(pending.rename_short);
			std::ostringstream ord;
			ord << "Ordine parole:";
			for(std::size_t i = 0; i < words.size(); ++i) {
				ord << " $c0011" << (i + 1) << ")$c0007" << words[i];
			}
			tell_from_jeweler(ch, mob, ord.str());
		}
	}
	{
		/* Esempio: numeri ciclici dalla palette (7 1 7 9 1 ...). */
		static const int kEx[] = {7, 1, 7, 9, 1, 11, 14, 9, 13, 4, 2, 15};
		std::ostringstream ex;
		ex << "Esempio: $c0011";
		for(int i = 0; i < need; ++i) {
			if(i) {
				ex << ' ';
			}
			ex << kEx[i % static_cast<int>(sizeof(kEx) / sizeof(kEx[0]))];
		}
		ex << "$c0007";
		tell_from_jeweler(ch, mob, ex.str());
	}
	show_rename_color_palette(ch, mob);
	tell_from_jeweler(ch, mob,
					  "Oppure $c0011indietro$c0007 / $c0011shake$c0007 per annullare.");
}

/** Avviso mud-mail allo staff dopo string a pagamento (rinomina / colori). */
void notify_staff_paid_rename(char_data* ch, const obj_data* obj,
							  const std::string& old_kw, const std::string& old_short,
							  const std::string& old_desc, const std::string& new_kw,
							  const std::string& new_short, const std::string& new_desc,
							  int rune_cost, long coin_cost) {
	if(!ch || !ch->desc || !GET_NAME(ch) || !obj) {
		return;
	}
	std::ostringstream os;
	os << GET_NAME(ch) << " ha rinominato un pezzo (EditAffectBroker).\n\r"
	   << "Instance: " << static_cast<unsigned long long>(obj->db_instance_id)
	   << "  vnum: " << obj_vnum(obj) << "\n\r"
	   << "Costo: " << rune_cost << " runa/e";
	if(coin_cost > 0) {
		os << ", " << coin_cost << " coins";
	}
	os << ".\n\r"
	   << "Prima:\n\r"
	   << "  keywords: " << old_kw << "\n\r"
	   << "  short: " << old_short << "\n\r"
	   << "  desc: " << old_desc << "\n\r"
	   << "Dopo:\n\r"
	   << "  keywords: " << new_kw << "\n\r"
	   << "  short: " << new_short << "\n\r"
	   << "  desc: " << new_desc << "\n\r";
	const std::string msg = os.str();
	static const char* const kStaff[] = {"Sirio", "Hephaistos", nullptr};
	for(const char* const* p = kStaff; *p; ++p) {
		mail_to_god(ch, *p, msg.c_str());
	}
}

void show_numbered_affect_list(char_data* ch, char_data* mob, const obj_data* obj_a,
							   const std::vector<std::string>& labels, bool choose_prompt) {
	if(!ch || !mob || !obj_a) {
		return;
	}
	const char* shortn = obj_a->short_description ? obj_a->short_description : "oggetto";
	{
		std::ostringstream hdr;
		hdr << "Edit trasferibili da " << shortn << ":";
		tell_from_jeweler(ch, mob, hdr.str());
	}
	for(std::size_t i = 0; i < labels.size(); ++i) {
		std::ostringstream line;
		line << "  $c0015" << (i + 1) << ")$c0007  $c0011" << labels[i] << "$c0007\n\r";
		send_to_char(line.str().c_str(), ch);
	}
	if(choose_prompt) {
		tell_from_jeweler(ch, mob,
						  "Scegli il $c0015numero$c0007 dell'effetto da spostare "
						  "(oppure scrivi il nome).");
		tell_from_jeweler(ch, mob, "Di' $c0011no$c0007 per annullare.");
	}
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
	if(!pending_is_confirm_step(pending.step)) {
		return false;
	}
	if(time(nullptr) > pending.expires_at || !edit_broker_pending_room_ok(ch, pending)) {
		g_edit_broker_pending.erase(it);
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "La richiesta e' scaduta o non e' piu' valida.");
		return true;
	}

	switch(ans) {
	case BrokerConfirmAnswer::Yes: {
		g_edit_broker_pending.erase(it);
		if(pending.op == EditBrokerOpKind::Transfer) {
			obj_data* oa = find_broker_obj(ch, pending.name_a, pending.id_a);
			obj_data* ob = find_broker_obj(ch, pending.name_b, pending.id_b);
			if(!oa || !ob) {
				broker_ambient_stop(ch, mob);
				tell_from_jeweler(ch, mob, "Non trovo piu' i pezzi nel tuo inventario.");
				tell_from_jeweler(ch, mob, "Ok, non faccio niente.");
				return true;
			}
			pending.name_a = obj_vis_key(ch, oa);
			pending.name_b = obj_vis_key(ch, ob);
			TransferPlan plan;
			if(!build_transfer_plan(ch, mob, pending.aff_name, pending.name_a, pending.name_b,
									plan)) {
				broker_ambient_stop(ch, mob);
				tell_from_jeweler(ch, mob, "Non posso piu' completare il trasferimento.");
				tell_from_jeweler(ch, mob, "Le condizioni sono cambiate.");
				return true;
			}
			execute_transfer(ch, mob, plan);
		}
		else if(pending.op == EditBrokerOpKind::Destroy) {
			obj_data* oc = find_broker_obj(ch, pending.name_c, pending.id_c);
			if(!oc) {
				broker_ambient_stop(ch, mob);
				tell_from_jeweler(ch, mob, "Non trovo piu' l'oggetto nel tuo inventario.");
				tell_from_jeweler(ch, mob, "Ok, non faccio niente.");
				return true;
			}
			pending.name_c = obj_vis_key(ch, oc);
			DestroyPlan plan;
			if(!build_destroy_plan(ch, mob, pending.name_c, plan)) {
				broker_ambient_stop(ch, mob);
				tell_from_jeweler(ch, mob, "Non posso piu' completare la distruzione.");
				tell_from_jeweler(ch, mob, "Le condizioni sono cambiate.");
				return true;
			}
			execute_destroy(ch, mob, plan);
		}
		else if(pending.op == EditBrokerOpKind::Rename) {
			execute_rename(ch, mob, pending);
		}
		return true;
	}
	case BrokerConfirmAnswer::No:
		cancel_edit_broker_pending(ch, mob, true);
		return true;
	case BrokerConfirmAnswer::Other:
		ask_edit_broker_confirm(ch, mob);
		return true;
	}
	return true;
}

[[nodiscard]] bool try_pick_affect_from_text(char_data* ch, char_data* mob,
											 EditBrokerPending& pending,
											 std::string_view text) {
	auto [word, rest] = next_arg(text);
	(void)rest;
	if(word.empty()) {
		return false;
	}

	std::string chosen;
	bool all_digits = !word.empty();
	for(unsigned char c : word) {
		if(!std::isdigit(c)) {
			all_digits = false;
			break;
		}
	}
	if(all_digits) {
		const int idx = std::atoi(word.c_str());
		if(idx < 1 || static_cast<std::size_t>(idx) > pending.affect_labels.size()) {
			tell_from_jeweler(ch, mob, "Numero non valido. Riprova, oppure di' no.");
			return true;
		}
		chosen = pending.affect_labels[static_cast<std::size_t>(idx - 1)];
	}
	else {
		const std::string key = normalize_affect_key(word);
		std::string full = std::string(text);
		while(!full.empty() && std::isspace(static_cast<unsigned char>(full.front()))) {
			full.erase(full.begin());
		}
		while(!full.empty() && std::isspace(static_cast<unsigned char>(full.back()))) {
			full.pop_back();
		}
		const std::string full_key = normalize_affect_key(full);
		for(const auto& lab : pending.affect_labels) {
			const std::string lab_key = normalize_affect_key(lab);
			if(lab_key == full_key || lab_key == key) {
				chosen = lab;
				break;
			}
		}
		if(chosen.empty()) {
			tell_from_jeweler(ch, mob, "Non riconosco quell'effetto.");
			tell_from_jeweler(ch, mob, "Usa il numero della lista, oppure di' no.");
			return true;
		}
	}

	TransferPlan plan;
	if(!build_transfer_plan(ch, mob, chosen, pending.name_a, pending.name_b, plan)) {
		cancel_edit_broker_pending(ch, mob, false);
		return true;
	}
	pending.aff_name = chosen;
	pending.step = EditBrokerStep::ConfirmTransfer;
	touch_edit_broker_pending(pending);
	g_edit_broker_pending[ch] = pending;
	preview_transfer(ch, mob, plan);
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
	if(time(nullptr) > it->second.expires_at || !edit_broker_pending_room_ok(ch, it->second)) {
		g_edit_broker_pending.erase(it);
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "La richiesta e' scaduta o non e' piu' valida.");
		return true;
	}

	const EditBrokerStep step = it->second.step;
	const BrokerConfirmAnswer ans = parse_broker_confirm(text);

	/* shake = annulla sempre, anche dove "no" ha altro significato (es. blink). */
	if(is_broker_shake_cancel(text)) {
		cancel_edit_broker_pending(ch, mob, true);
		return true;
	}

	if(it->second.op == EditBrokerOpKind::Rename && is_rename_back_command(text) &&
	   pending_is_rename_wizard_step(step)) {
		return try_rename_wizard_back(ch, mob, it->second);
	}

	/* Blink ask: "no" = niente blink (non annulla il wizard). shake gia' gestito. */
	if(step == EditBrokerStep::WaitRenameBlinkAsk) {
		EditBrokerPending& pending = it->second;
		if(ans == BrokerConfirmAnswer::No) {
			pending.rename_blink = false;
			begin_rename_confirm(ch, mob, pending);
			return true;
		}
		if(ans == BrokerConfirmAnswer::Yes) {
			const auto words = split_rename_words(pending.rename_short);
			if(words.empty()) {
				tell_from_jeweler(ch, mob, "La short non ha parole da far lampeggiare.");
				pending.rename_blink = false;
				begin_rename_confirm(ch, mob, pending);
				return true;
			}
			pending.rename_blink = true;
			pending.step = EditBrokerStep::WaitRenameBlinkWord;
			touch_edit_broker_pending(pending);
			show_rename_blink_words(ch, mob, pending);
			return true;
		}
		tell_from_jeweler(ch, mob, "Rispondi $c0011si$c0007 o $c0011no$c0007.");
		return true;
	}

	if(ans == BrokerConfirmAnswer::No) {
		cancel_edit_broker_pending(ch, mob, true);
		return true;
	}
	if(ans == BrokerConfirmAnswer::Other && allow_passthrough_cmds &&
	   is_edit_broker_command_topic(text)) {
		return false;
	}
	if(pending_is_confirm_step(step)) {
		/* Conferma rinomina: "indietro" gia' gestito sopra; Other ≠ indietro. */
		return apply_edit_broker_answer(ch, mob, ans);
	}
	if(pending_is_rename_text_step(step)) {
		if(ans == BrokerConfirmAnswer::No) {
			cancel_edit_broker_pending(ch, mob, true);
			return true;
		}
		if(ans == BrokerConfirmAnswer::Yes) {
			tell_from_jeweler(ch, mob,
							  "Dimmi il testo richiesto, oppure $c0011shake$c0007 per "
							  "annullare.");
			return true;
		}
		EditBrokerPending& pending = it->second;
		std::string err;
		std::string field;
		if(step == EditBrokerStep::WaitRenameKeywords) {
			if(!validate_rename_field(text, kRenameMaxKeywords, "keywords", field, err)) {
				tell_from_jeweler(ch, mob, err);
				tell_from_jeweler(ch, mob,
								  "Riprova, oppure $c0011indietro$c0007 / $c0011shake$c0007 "
								  "per annullare.");
				return true;
			}
			pending.rename_keywords = field;
			pending.step = EditBrokerStep::WaitRenameShort;
			touch_edit_broker_pending(pending);
			show_rename_partial_preview(ch, mob, pending);
			tell_from_jeweler(ch, mob,
							  "Ora dimmi la $c0011short$c0007 (nome in inventario), "
							  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per "
							  "annullare.");
			return true;
		}
		if(step == EditBrokerStep::WaitRenameShort) {
			if(!validate_rename_field(text, kRenameMaxShort, "short", field, err)) {
				tell_from_jeweler(ch, mob, err);
				tell_from_jeweler(ch, mob,
								  "Riprova, oppure $c0011indietro$c0007 / $c0011shake$c0007 "
								  "per annullare.");
				return true;
			}
			pending.rename_short = field;
			pending.step = EditBrokerStep::WaitRenameDesc;
			touch_edit_broker_pending(pending);
			show_rename_partial_preview(ch, mob, pending);
			tell_from_jeweler(ch, mob,
							  "Ora dimmi la $c0011description$c0007 (come appare a terra), "
							  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per "
							  "annullare.");
			return true;
		}
		/* WaitRenameDesc */
		if(!validate_rename_field(text, kRenameMaxDesc, "description", field, err)) {
			tell_from_jeweler(ch, mob, err);
			tell_from_jeweler(ch, mob,
							  "Riprova, oppure $c0011indietro$c0007 / $c0011shake$c0007 "
							  "per annullare.");
			return true;
		}
		pending.rename_desc = field;
		pending.rename_pattern = -1;
		pending.rename_color_count = 0;
		pending.rename_colors_needed = 0;
		pending.step = EditBrokerStep::WaitRenamePattern;
		touch_edit_broker_pending(pending);
		show_rename_partial_preview(ch, mob, pending);
		show_rename_pattern_menu(ch, mob, pending.rename_short);
		return true;
	}
	if(pending_is_rename_style_step(step)) {
		if(ans == BrokerConfirmAnswer::No) {
			cancel_edit_broker_pending(ch, mob, true);
			return true;
		}
		if(ans == BrokerConfirmAnswer::Yes) {
			if(step == EditBrokerStep::WaitRenameColors) {
				prompt_rename_colors(ch, mob, it->second);
			}
			else if(step == EditBrokerStep::WaitRenamePattern) {
				show_rename_partial_preview(ch, mob, it->second);
				show_rename_pattern_menu(ch, mob, it->second.rename_short);
			}
			else {
				tell_from_jeweler(ch, mob,
								  "Scegli un numero dalla lista, oppure $c0011shake$c0007 "
								  "per annullare.");
			}
			return true;
		}
		EditBrokerPending& pending = it->second;
		if(step == EditBrokerStep::WaitRenamePattern) {
			auto [word, rest] = next_arg(text);
			(void)rest;
			if(word == "nessuno" || word == "0") {
				pending.rename_pattern = -1;
				pending.rename_color_count = 0;
				pending.rename_colors_needed = 0;
				show_rename_partial_preview(ch, mob, pending);
				begin_rename_desc_word_ask(ch, mob, pending);
				return true;
			}
			const auto available = rename_patterns_available_for(pending.rename_short);
			bool digits = !word.empty();
			for(unsigned char c : word) {
				if(!std::isdigit(c)) {
					digits = false;
					break;
				}
			}
			int pat_index = -1;
			if(digits) {
				const int pick = std::atoi(std::string(word).c_str());
				if(pick >= 1 && static_cast<std::size_t>(pick) <= available.size()) {
					pat_index = available[static_cast<std::size_t>(pick - 1)];
				}
			}
			else {
				std::string low(word);
				for(char& c : low) {
					c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				}
				for(int ai : available) {
					if(low == kRenameColorPatterns[ai].id) {
						pat_index = ai;
						break;
					}
				}
			}
			if(pat_index < 0) {
				tell_from_jeweler(ch, mob, "Pattern non valido. Riprova.");
				show_rename_partial_preview(ch, mob, pending);
				show_rename_pattern_menu(ch, mob, pending.rename_short);
				return true;
			}
			{
				const auto& pat = kRenameColorPatterns[pat_index];
				const int need = rename_pattern_slots_for(pat, pending.rename_short);
				if(need > kRenameMaxColorSlots) {
					tell_from_jeweler(ch, mob,
									  "Troppe parole nella short per colorarle una a una.");
					show_rename_partial_preview(ch, mob, pending);
					show_rename_pattern_menu(ch, mob, pending.rename_short);
					return true;
				}
				pending.rename_colors_needed = need;
			}
			pending.rename_pattern = pat_index;
			pending.rename_color_count = 0;
			pending.step = EditBrokerStep::WaitRenameColors;
			touch_edit_broker_pending(pending);
			{
				std::ostringstream os;
				os << "Ok, pattern $c0011"
				   << kRenameColorPatterns[pending.rename_pattern].label << "$c0007.";
				tell_from_jeweler(ch, mob, os.str());
			}
			show_rename_partial_preview(ch, mob, pending);
			prompt_rename_colors(ch, mob, pending);
			return true;
		}
		/* WaitRenameColors: tutti i colori in una sola riga. */
		const int need = pending.rename_colors_needed;
		int parsed[kRenameMaxColorSlots]{};
		int got = 0;
		std::string_view left = text;
		while(true) {
			auto [tok, rest] = next_arg(left);
			left = rest;
			if(tok.empty()) {
				break;
			}
			const RenameColorChoice* choice = find_rename_color_choice(tok);
			if(!choice) {
				{
					std::ostringstream os;
					os << "Colore non riconosciuto: \"$c0011" << tok << "$c0007\".";
					tell_from_jeweler(ch, mob, os.str());
				}
				prompt_rename_colors(ch, mob, pending);
				return true;
			}
			if(got >= kRenameMaxColorSlots) {
				break;
			}
			parsed[got++] = choice->code;
		}
		if(got != need) {
			{
				std::ostringstream os;
				os << "Ne servono $c0011" << need << "$c0007, ne hai detti $c0011" << got
				   << "$c0007. Riprova.";
				tell_from_jeweler(ch, mob, os.str());
			}
			prompt_rename_colors(ch, mob, pending);
			return true;
		}
		const int distinct = count_distinct_rename_colors(parsed, got);
		if(distinct > kRenameMaxDistinctColorsShort) {
			{
				std::ostringstream os;
				os << "Puoi usare al massimo $c0011" << kRenameMaxDistinctColorsShort
				   << "$c0007 colori distinti sulla short (ne hai $c0011" << distinct
				   << "$c0007).";
				tell_from_jeweler(ch, mob, os.str());
			}
			prompt_rename_colors(ch, mob, pending);
			return true;
		}
		{
			const auto& pat = kRenameColorPatterns[pending.rename_pattern];
			const std::string trial = apply_rename_color_pattern(
				pending.rename_short, pat.kind, parsed, got);
			if(trial.size() > kObjInstMaxShort) {
				tell_from_jeweler(ch, mob,
								  "Troppe alternanze di colori: la short colorata non "
								  "sta nel limite consentito.");
				{
					std::ostringstream os;
					os << "Lunghezza $c0011" << trial.size() << "$c0007 / max $c0011"
					   << kObjInstMaxShort << "$c0007. Cambia i colori (usa $c00117$c0007 "
					   << "normale per ridurre i cambi).";
					tell_from_jeweler(ch, mob, os.str());
				}
				prompt_rename_colors(ch, mob, pending);
				return true;
			}
		}
		for(int i = 0; i < got; ++i) {
			pending.rename_colors[i] = parsed[i];
		}
		pending.rename_color_count = got;
		touch_edit_broker_pending(pending);
		show_rename_partial_preview(ch, mob, pending);
		begin_rename_desc_word_ask(ch, mob, pending);
		return true;
	}
	if(step == EditBrokerStep::WaitRenameDescWord) {
		EditBrokerPending& pending = it->second;
		if(ans == BrokerConfirmAnswer::Yes) {
			tell_from_jeweler(ch, mob, "Scegli un numero dalla lista, oppure 0 / nessuna.");
			return true;
		}
		if(ans == BrokerConfirmAnswer::No) {
			cancel_edit_broker_pending(ch, mob, true);
			return true;
		}
		auto [word, rest] = next_arg(text);
		(void)rest;
		if(word == "nessuna" || word == "nessuno" || word == "0") {
			clear_rename_desc_tint(pending);
			show_rename_partial_preview(ch, mob, pending);
			begin_rename_blink_ask(ch, mob, pending);
			return true;
		}
		bool digits = !word.empty();
		for(unsigned char c : word) {
			if(!std::isdigit(c)) {
				digits = false;
				break;
			}
		}
		if(!digits) {
			tell_from_jeweler(ch, mob,
							  "Indica un numero, oppure 0 / nessuna, oppure "
							  "$c0011shake$c0007 per annullare.");
			return true;
		}
		const int pick = std::atoi(std::string(word).c_str());
		const auto words = split_rename_words(pending.rename_desc);
		if(pick < 1 || static_cast<std::size_t>(pick) > words.size()) {
			tell_from_jeweler(ch, mob,
							  "Parola non valida. Riprova, oppure 0 / nessuna, oppure "
							  "$c0011shake$c0007 per annullare.");
			return true;
		}
		pending.rename_desc_color_word = pick - 1;
		pending.rename_desc_color_fg = -1;
		touch_edit_broker_pending(pending);
		begin_rename_desc_color_ask(ch, mob, pending);
		return true;
	}
	if(step == EditBrokerStep::WaitRenameDescColor) {
		EditBrokerPending& pending = it->second;
		if(ans == BrokerConfirmAnswer::Yes) {
			prompt_rename_desc_color(ch, mob, pending);
			return true;
		}
		if(ans == BrokerConfirmAnswer::No) {
			cancel_edit_broker_pending(ch, mob, true);
			return true;
		}
		auto [word, rest] = next_arg(text);
		(void)rest;
		const RenameColorChoice* choice = find_rename_color_choice(word);
		if(!choice) {
			tell_from_jeweler(ch, mob,
							  "Colore non riconosciuto. Riprova, oppure "
							  "$c0011shake$c0007 per annullare.");
			prompt_rename_desc_color(ch, mob, pending);
			return true;
		}
		pending.rename_desc_color_fg = choice->code;
		touch_edit_broker_pending(pending);
		/* Anteprima con tinta; se troppo lunga, torna alla scelta parola. */
		{
			std::string kw;
			std::string sh;
			std::string ds;
			std::string err;
			if(build_rename_final_texts(pending, kw, sh, ds, err) &&
			   ds.size() > kObjInstMaxDesc) {
				tell_from_jeweler(ch, mob, "La description colorata e' troppo lunga.");
				{
					std::ostringstream os;
					os << "Lunghezza $c0011" << ds.size() << "$c0007 / max $c0011"
					   << kObjInstMaxDesc << "$c0007. Scegli un altro colore o parola.";
					tell_from_jeweler(ch, mob, os.str());
				}
				clear_rename_desc_tint(pending);
				pending.step = EditBrokerStep::WaitRenameDescWord;
				show_rename_partial_preview(ch, mob, pending);
				show_rename_desc_words(ch, mob, pending);
				return true;
			}
		}
		/* begin_rename_blink_ask stampa gia' l'anteprima parziale. */
		begin_rename_blink_ask(ch, mob, pending);
		return true;
	}
	if(step == EditBrokerStep::WaitRenameBlinkWord ||
	   step == EditBrokerStep::WaitRenameBlinkLetter) {
		EditBrokerPending& pending = it->second;
		if(ans == BrokerConfirmAnswer::Yes) {
			tell_from_jeweler(ch, mob,
							  "Scegli un numero dalla lista, oppure $c0011shake$c0007 "
							  "per annullare.");
			return true;
		}
		auto [word, rest] = next_arg(text);
		(void)rest;
		bool digits = !word.empty();
		for(unsigned char c : word) {
			if(!std::isdigit(c)) {
				digits = false;
				break;
			}
		}
		if(!digits) {
			tell_from_jeweler(ch, mob,
							  "Indica un numero, oppure $c0011shake$c0007 per annullare.");
			return true;
		}
		const int pick = std::atoi(std::string(word).c_str());
		if(step == EditBrokerStep::WaitRenameBlinkWord) {
			const auto words = split_rename_words(pending.rename_short);
			if(pick < 1 || static_cast<std::size_t>(pick) > words.size()) {
				tell_from_jeweler(ch, mob,
								  "Parola non valida. Riprova, oppure $c0011shake$c0007 "
								  "per annullare.");
				return true;
			}
			pending.rename_blink_word = pick - 1;
			pending.step = EditBrokerStep::WaitRenameBlinkLetter;
			touch_edit_broker_pending(pending);
			show_rename_partial_preview(ch, mob, pending);
			show_rename_blink_letters(ch, mob, pending);
			return true;
		}
		const auto words = split_rename_words(pending.rename_short);
		if(pending.rename_blink_word < 0 ||
		   static_cast<std::size_t>(pending.rename_blink_word) >= words.size()) {
			cancel_edit_broker_pending(ch, mob, true);
			return true;
		}
		const std::string& w = words[static_cast<std::size_t>(pending.rename_blink_word)];
		if(pick < 1 || static_cast<std::size_t>(pick) > w.size()) {
			tell_from_jeweler(ch, mob,
							  "Lettera non valida. Riprova, oppure $c0011shake$c0007 "
							  "per annullare.");
			return true;
		}
		pending.rename_blink_letter = pick - 1;
		show_rename_partial_preview(ch, mob, pending);
		begin_rename_confirm(ch, mob, pending);
		return true;
	}
	if(step == EditBrokerStep::WaitAffectPick) {
		if(ans == BrokerConfirmAnswer::Yes) {
			tell_from_jeweler(ch, mob, "Prima scegli l'effetto dalla lista.");
			return true;
		}
		EditBrokerPending pending_copy = it->second;
		return try_pick_affect_from_text(ch, mob, pending_copy, text);
	}
	if(step == EditBrokerStep::WaitGiveA || step == EditBrokerStep::WaitGiveB ||
	   step == EditBrokerStep::WaitGiveDestroy ||
	   step == EditBrokerStep::WaitGiveRename) {
		if(step == EditBrokerStep::WaitGiveA) {
			tell_from_jeweler(ch, mob, "Dammi il pezzo da cui togliere l'effetto.");
			tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
		}
		else if(step == EditBrokerStep::WaitGiveB) {
			tell_from_jeweler(ch, mob, "Ora dammi il pezzo su cui mettere l'effetto.");
			tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
		}
		else if(step == EditBrokerStep::WaitGiveRename) {
			tell_from_jeweler(ch, mob, "Dammi il pezzo da rinominare.");
			tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
		}
		else {
			tell_from_jeweler(ch, mob, "Dammi l'oggetto da distruggere.");
			tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
		}
		return true;
	}
	return true;
}

void list_transferable_affects(struct char_data* ch, struct char_data* mob,
							   struct obj_data* obj_a) {
	if(!ch || !mob || !obj_a) {
		return;
	}
	if(!obj_is_owned_edit(ch, obj_a)) {
		tell_from_jeweler(ch, mob,
						  "Per trasferire serve un pezzo EDIT, PERSONAL e di tua proprieta'.");
		return;
	}
	const auto labels = collect_transferable_delta_labels(obj_a);
	const char* shortn = obj_a->short_description ? obj_a->short_description : "oggetto";
	if(labels.empty()) {
		std::ostringstream os;
		os << "Su " << shortn
		   << " non ci sono effetti che possa spostare: e' ancora come quando e' stato "
			  "legato.";
		tell_from_jeweler(ch, mob, os.str());
		return;
	}
	show_numbered_affect_list(ch, mob, obj_a, labels, false);
}

/** ask <mob> info <oggetto>: identify + quali edit si possono trasferire. */
void show_edit_broker_object_info(struct char_data* ch, struct char_data* mob,
								  struct obj_data* obj) {
	if(!ch || !mob || !obj) {
		return;
	}
	if(!IS_OBJ_STAT2(obj, ITEM2_EDIT) || !IS_OBJ_STAT2(obj, ITEM2_PERSONAL)) {
		tell_from_jeweler(ch, mob,
						  "Ricordati che posso lavorare solo pezzi $c0011EDIT$c0007 e "
						  "$c0011PERSONAL$c0007.");
		return;
	}
	if(!pers_on(ch, obj)) {
		tell_from_jeweler(ch, mob, "Quel pezzo PERSONAL non e' tuo.");
		return;
	}
	broker_ambient_examine(ch, mob, obj);
	send_to_char("\n\r", ch);
	spell_identify(GET_LEVEL(mob, WARRIOR_LEVEL_IND), ch, mob, obj);
	send_to_char("\n\r", ch);
	list_transferable_affects(ch, mob, obj);
}

void show_edit_broker_help_and_list(struct char_data* ch, struct char_data* mob,
									std::string_view maybe_extra) {
	if(!ch || !mob) {
		return;
	}
	auto [tok, rest] = next_arg(maybe_extra);
	(void)rest;
	/* aiuto = solo elenco comandi. Per gli oggetti: ask <mob> info <oggetto>. */
	if(!tok.empty()) {
		tell_from_jeweler(ch, mob,
						  "Per esaminare un pezzo chiedimi $c0011info$c0007 "
						  "<oggetto>.");
		tell_from_jeweler(ch, mob,
						  "Oppure chiedimi solo $c0011aiuto$c0007 per i comandi.");
		return;
	}
	show_edit_broker_usage(ch, mob);
}

void show_edit_broker_info_object(struct char_data* ch, struct char_data* mob,
								  std::string_view maybe_obj) {
	if(!ch || !mob) {
		return;
	}
	auto [tok, rest] = next_arg(maybe_obj);
	(void)rest;
	if(tok.empty()) {
		tell_from_jeweler(ch, mob,
						  "Di quale pezzo? Esempio: $c0011info$c0007 <oggetto>.");
		return;
	}
	struct obj_data* obj_a = get_obj_in_list_vis(ch, tok.c_str(), ch->carrying);
	if(!obj_a) {
		tell_from_jeweler(ch, mob, "Non vedo quell'oggetto nel tuo inventario.");
		return;
	}
	show_edit_broker_object_info(ch, mob, obj_a);
}

void preview_rename(char_data* ch, char_data* mob, const EditBrokerPending& pending,
					obj_data* obj) {
	if(!ch || !mob || !obj) {
		return;
	}
	std::string kw;
	std::string sh;
	std::string ds;
	std::string err;
	if(!build_rename_final_texts(pending, kw, sh, ds, err)) {
		tell_from_jeweler(ch, mob, err);
		return;
	}
	const int runes = pending_rename_rune_cost(pending);
	const long coins = pending_rename_coin_cost(pending);
	std::ostringstream os;
	os << "$c0015Anteprima rinomina$c0007\n\r"
	   << "  Pezzo: " << obj_shortn(obj) << "\n\r"
	   << "  Keywords: $c0011" << kw << "$c0007\n\r"
	   << "  Short: " << sh << "$c0007\n\r"
	   << "  Description: " << ds << "$c0007\n\r"
	   << "  Costo: $c0011" << runes << "$c0007 runa/e degli Dei";
	if(coins > 0) {
		os << " + $c0011" << coins << "$c0007 coins";
	}
	if(pending.rename_pattern >= 0) {
		os << "\n\r  Pattern: $c0011"
		   << kRenameColorPatterns[pending.rename_pattern].label << "$c0007";
	}
	if(pending.rename_blink) {
		os << "\n\r  Blink: $c0011si$c0007 (x2)";
	}
	os << ".";
	tell_from_jeweler(ch, mob, os.str());
	ask_edit_broker_confirm(ch, mob);
}

void execute_rename(char_data* ch, char_data* mob, EditBrokerPending& pending) {
	if(!ch || !mob) {
		return;
	}
	obj_data* obj = find_broker_obj(ch, pending.name_c, pending.id_c);
	if(!obj) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "Non trovo piu' l'oggetto nel tuo inventario.");
		tell_from_jeweler(ch, mob, "Ok, non faccio niente.");
		return;
	}
	if(obj->db_instance_id == 0) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "L'oggetto non e' nel database edit.");
		return;
	}
	if(GetMaxLevel(ch) < IMMORTALE) {
		if(!(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && pers_on(ch, obj))) {
			broker_ambient_stop(ch, mob);
			tell_from_jeweler(ch, mob, "Il pezzo deve essere PERSONAL e tuo.");
			return;
		}
		std::string perr;
		if(!obj_texts_match_prototype(obj, perr)) {
			broker_ambient_stop(ch, mob);
			tell_from_jeweler(ch, mob, perr);
			return;
		}
	}
	std::string kw;
	std::string sh;
	std::string ds;
	std::string err;
	if(!build_rename_final_texts(pending, kw, sh, ds, err)) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, err);
		return;
	}
	if(sh.size() > kObjInstMaxShort) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob,
						  "La short colorata e' troppo lunga per essere registrata "
						  "(troppe alternanze di colori).");
		return;
	}
	if(ds.size() > kObjInstMaxDesc) {
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "La description colorata e' troppo lunga.");
		{
			std::ostringstream os;
			os << "Lunghezza $c0011" << ds.size() << "$c0007 / max $c0011"
			   << kObjInstMaxDesc << "$c0007.";
			tell_from_jeweler(ch, mob, os.str());
		}
		return;
	}
	const int runes = pending_rename_rune_cost(pending);
	const long coins = pending_rename_coin_cost(pending);
	if(GET_RUNEDEI(ch) < runes) {
		broker_ambient_stop(ch, mob);
		{
			std::ostringstream os;
			os << "Ti servono almeno $c0011" << runes << "$c0007 rune degli Dei.";
			tell_from_jeweler(ch, mob, os.str());
		}
		return;
	}
	if(coins > 0 && GET_GOLD(ch) < coins) {
		broker_ambient_stop(ch, mob);
		{
			std::ostringstream os;
			os << "Ti servono $c0011" << coins << "$c0007 coins.";
			tell_from_jeweler(ch, mob, os.str());
		}
		return;
	}

	broker_ambient_work_rename(ch, mob, obj);
	const std::string old_name = obj->name ? obj->name : "";
	const std::string old_short = obj->short_description ? obj->short_description : "";
	const std::string old_desc = obj->description ? obj->description : "";

	set_obj_cstr(obj->name, kw);
	set_obj_cstr(obj->short_description, sh);
	set_obj_cstr(obj->description, ds);
	SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
	if(!IS_OBJ_STAT2(obj, ITEM2_PERSONAL)) {
		pers_obj(mob, ch, obj, CMD_PERSONALIZE);
	}

	std::ostringstream detail;
	detail << "channel=rinomina kind=rename"
		   << " old_short=" << old_short << " new_short=" << sh
		   << " keywords=" << kw << " rune=" << runes << " coins=" << coins
		   << " blink=" << (pending.rename_blink ? 1 : 0);
	if(pending.rename_pattern >= 0) {
		detail << " pattern=" << kRenameColorPatterns[pending.rename_pattern].id;
	}
	if(!persist_edit_obj(obj, ch, "rename", "rinomina", detail.str().c_str(), true)) {
		set_obj_cstr(obj->name, old_name);
		set_obj_cstr(obj->short_description, old_short);
		set_obj_cstr(obj->description, old_desc);
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob,
						  "Salvataggio fallito: non tocco ne' rune, ne' coins, ne' nomi.");
		return;
	}

	GET_RUNEDEI(ch) = static_cast<ush_int>(GET_RUNEDEI(ch) - runes);
	if(coins > 0) {
		GET_GOLD(ch) -= static_cast<int>(coins);
	}
	schedule_inventory_save(ch);
	save_char(ch, AUTO_RENT, 0);
	broker_ambient_done(ch, mob);
	{
		std::ostringstream ok;
		ok << "$c0015Fatto:$c0007 pezzo rinominato in " << sh << "$c0007.";
		tell_from_jeweler(ch, mob, ok.str());
	}
	{
		std::ostringstream bill;
		bill << "Ti ho addebitato $c0011" << runes << "$c0007 runa/e degli Dei";
		if(coins > 0) {
			bill << " e $c0011" << coins << "$c0007 coins";
		}
		bill << ".";
		tell_from_jeweler(ch, mob, bill.str());
	}
	notify_staff_paid_rename(ch, obj, old_name, old_short, old_desc, kw, sh, ds, runes,
							 coins);
	{
		std::ostringstream log;
		log << "EditAffectBroker rename OK "
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << " inst="
			<< static_cast<unsigned long long>(obj->db_instance_id) << " short=" << sh
			<< " coins=" << coins;
		editor_mudlog(LOG_PLAYERS, log.str());
	}
}

[[nodiscard]] bool rename_object_eligible(char_data* ch, char_data* mob, obj_data* obj) {
	if(!ch || !mob || !obj) {
		return false;
	}
	if(obj->db_instance_id == 0) {
		tell_from_jeweler(ch, mob, "Serve un pezzo gia' nel database edit.");
		return false;
	}
	if(GetMaxLevel(ch) < IMMORTALE) {
		if(!(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && pers_on(ch, obj))) {
			tell_from_jeweler(ch, mob, "Il pezzo deve essere PERSONAL e tuo.");
			return false;
		}
		std::string perr;
		if(!obj_texts_match_prototype(obj, perr)) {
			tell_from_jeweler(ch, mob, perr);
			return false;
		}
	}
	return true;
}

void begin_rename_wizard(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	supersede_edit_broker_pending(ch, mob);
	EditBrokerPending pending {};
	pending.jeweler = mob;
	pending.op = EditBrokerOpKind::Rename;
	pending.step = EditBrokerStep::WaitGiveRename;
	touch_edit_broker_pending(pending);
	g_edit_broker_pending[ch] = std::move(pending);
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dammi il pezzo da rinominare.");
	tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
	tell_from_jeweler(ch, mob,
					  "Costo: $c00111 runa$c0007 (+ coins se scegli un pattern colore).");
}

void begin_transfer_wizard(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	supersede_edit_broker_pending(ch, mob);
	EditBrokerPending pending {};
	pending.jeweler = mob;
	pending.op = EditBrokerOpKind::Transfer;
	pending.step = EditBrokerStep::WaitGiveA;
	touch_edit_broker_pending(pending);
	g_edit_broker_pending[ch] = std::move(pending);
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dammi il pezzo da cui togliere l'effetto.");
	tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
}

void begin_destroy_wizard(char_data* ch, char_data* mob) {
	if(!ch || !mob) {
		return;
	}
	supersede_edit_broker_pending(ch, mob);
	EditBrokerPending pending {};
	pending.jeweler = mob;
	pending.op = EditBrokerOpKind::Destroy;
	pending.step = EditBrokerStep::WaitGiveDestroy;
	touch_edit_broker_pending(pending);
	g_edit_broker_pending[ch] = std::move(pending);
	broker_ambient_talk(ch, mob);
	tell_from_jeweler(ch, mob, "Dammi l'oggetto da distruggere.");
	tell_from_jeweler(ch, mob, "Usa $c0011give$c0007 come comando.");
}

/** Give come gesto: TRUE = consumato (oggetto NON trasferito al mob). */
/**
 * Give verso il broker senza procedura attiva (o nello step sbagliato):
 * l'oggetto resta al PG; il mob lo rifiuta stizzito.
 * @return true se il give era rivolto a questo mob (comando consumato).
 */
[[nodiscard]] bool refuse_stray_give_to_broker(char_data* ch, char_data* mob,
											  const char* arg) {
	if(!ch || !mob || !arg) {
		return false;
	}
	std::string_view av = arg;
	auto [obj_tok, rest1] = next_arg(av);
	auto [vict_tok, rest2] = next_arg(rest1);
	(void)rest2;
	if(obj_tok.empty() || vict_tok.empty()) {
		return false;
	}
	bool obj_is_num = true;
	for(unsigned char c : obj_tok) {
		if(!std::isdigit(c)) {
			obj_is_num = false;
			break;
		}
	}
	if(obj_is_num) {
		return false; /* give N coins: lascia al flusso normale */
	}
	struct char_data* vict = get_char_room_vis(ch, vict_tok.c_str());
	if(!vict || vict != mob) {
		return false;
	}
	struct obj_data* obj = get_obj_in_list_vis(ch, obj_tok.c_str(), ch->carrying);
	if(!obj) {
		send_to_char("Non hai quell'oggetto.\n\r", ch);
		return true;
	}
	act("Mostri $p a $N.", FALSE, ch, obj, mob, TO_CHAR);
	act("$n mostra $p a $N.", FALSE, ch, obj, mob, TO_ROOM);
	act("$N ti respinge $p contro il petto, senza prenderlo.", FALSE, ch, obj, mob,
		TO_CHAR);
	act("$N respinge $p a $n senza prenderlo.", FALSE, ch, obj, mob, TO_ROOM);
	/* Varianti stizzite; a capo dopo la prima frase per schermi stretti. */
	switch(number(0, 3)) {
	case 0:
		tell_from_jeweler(ch, mob,
						  "E tu cosa pensi che me ne faccia, se non mi dici cosa vuoi?");
		tell_from_jeweler(ch, mob,
						  "Non sono un ripostiglio. Prima chiedimi $c0011trasferisci$c0007, "
						  "$c0011distruggi$c0007 o $c0011rinomina$c0007 - poi, magari, "
						  "ti ascolto.");
		break;
	case 1:
		tell_from_jeweler(ch, mob,
						  "Mi ficchi roba in mano senza una parola? Che ci faccio, "
						  "un museo?");
		tell_from_jeweler(ch, mob,
						  "Dimmi $c0011trasferisci$c0007, $c0011distruggi$c0007 o "
						  "$c0011rinomina$c0007 - oppure riprenditelo e non farmi "
						  "perdere tempo.");
		break;
	case 2:
		tell_from_jeweler(ch, mob,
						  "Bello pezzo. Peccato che non mi interessi, se non mi "
						  "spieghi il lavoro.");
		tell_from_jeweler(ch, mob,
						  "Chiedimi $c0011trasferisci$c0007, $c0011distruggi$c0007 o "
						  "$c0011rinomina$c0007 prima di usarmi come magazzino.");
		break;
	default:
		tell_from_jeweler(ch, mob,
						  "Oh, un dono? No. Io non colleziono.");
		tell_from_jeweler(ch, mob,
						  "Se vuoi il mio mestiere, chiedi $c0011trasferisci$c0007, "
						  "$c0011distruggi$c0007 o $c0011rinomina$c0007. Altrimenti "
						  "tieni pure quello che e' tuo.");
		break;
	}
	return true;
}

bool handle_edit_broker_give(char_data* ch, char_data* mob, const char* arg) {
	if(!ch || !mob || !arg) {
		return false;
	}
	auto it = g_edit_broker_pending.find(ch);
	if(it == g_edit_broker_pending.end() || it->second.jeweler != mob) {
		return refuse_stray_give_to_broker(ch, mob, arg);
	}
	EditBrokerPending& pending = it->second;
	if(pending.step != EditBrokerStep::WaitGiveA && pending.step != EditBrokerStep::WaitGiveB &&
	   pending.step != EditBrokerStep::WaitGiveDestroy &&
	   pending.step != EditBrokerStep::WaitGiveRename) {
		return refuse_stray_give_to_broker(ch, mob, arg);
	}
	if(time(nullptr) > pending.expires_at || !edit_broker_pending_room_ok(ch, pending)) {
		g_edit_broker_pending.erase(it);
		broker_ambient_stop(ch, mob);
		tell_from_jeweler(ch, mob, "La richiesta e' scaduta o non e' piu' valida.");
		return true;
	}

	std::string_view av = arg;
	auto [obj_tok, rest1] = next_arg(av);
	auto [vict_tok, rest2] = next_arg(rest1);
	(void)rest2;
	if(obj_tok.empty()) {
		return false;
	}
	/* give <n> coins ... - non e' un gesto oggetto */
	bool obj_is_num = !obj_tok.empty();
	for(unsigned char c : obj_tok) {
		if(!std::isdigit(c)) {
			obj_is_num = false;
			break;
		}
	}
	if(obj_is_num) {
		tell_from_jeweler(ch, mob, "Mi serve un oggetto, non monete.");
		return true;
	}
	if(vict_tok.empty()) {
		return false;
	}
	struct char_data* vict = get_char_room_vis(ch, vict_tok.c_str());
	if(!vict || vict != mob) {
		return false;
	}

	struct obj_data* obj = get_obj_in_list_vis(ch, obj_tok.c_str(), ch->carrying);
	if(!obj) {
		send_to_char("Non hai quell'oggetto.\n\r", ch);
		return true;
	}

	act("Mostri $p a $N.", FALSE, ch, obj, mob, TO_CHAR);
	act("$n mostra $p a $N.", FALSE, ch, obj, mob, TO_ROOM);

	if(pending.step == EditBrokerStep::WaitGiveA) {
		if(!obj_is_owned_edit(ch, obj)) {
			tell_from_jeweler(ch, mob,
							  "Serve un pezzo EDIT, PERSONAL e di tua proprieta'. Riprova.");
			return true;
		}
		if(obj->db_instance_id == 0) {
			tell_from_jeweler(ch, mob, "Questo pezzo non e' collegato al database edit.");
			tell_from_jeweler(ch, mob, "Contatta lo staff.");
			return true;
		}
		const auto labels = collect_transferable_delta_labels(obj);
		if(labels.empty()) {
			tell_from_jeweler(ch, mob,
							  "Su questo pezzo non ci sono effetti trasferibili.");
			tell_from_jeweler(ch, mob,
							  "Dammi un altro oggetto: usa $c0011give$c0007 come comando, "
							  "oppure di' no.");
			return true;
		}
		pending.name_a = obj_vis_key(ch, obj);
		pending.id_a = obj->db_instance_id;
		pending.affect_labels = labels;
		pending.step = EditBrokerStep::WaitGiveB;
		touch_edit_broker_pending(pending);
		broker_ambient_examine(ch, mob, obj);
		{
			std::ostringstream os;
			os << "Ok, tolgo da " << obj_shortn(obj) << ".";
			tell_from_jeweler(ch, mob, os.str());
		}
		tell_from_jeweler(ch, mob,
						  "Ora dammi il pezzo su cui metterlo: usa $c0011give$c0007 come "
						  "comando.");
		return true;
	}

	if(pending.step == EditBrokerStep::WaitGiveB) {
		obj_data* obj_a = find_broker_obj(ch, pending.name_a, pending.id_a);
		if(!obj_a) {
			broker_ambient_stop(ch, mob);
			tell_from_jeweler(ch, mob,
							  "Non trovo piu' il pezzo di partenza nel tuo inventario.");
			tell_from_jeweler(ch, mob, "Ok, non faccio niente.");
			g_edit_broker_pending.erase(it);
			return true;
		}
		if(obj == obj_a) {
			tell_from_jeweler(ch, mob, "Servono due pezzi distinti.");
			tell_from_jeweler(ch, mob,
							  "Dammi il pezzo su cui mettere l'effetto: usa "
							  "$c0011give$c0007 come comando.");
			return true;
		}
		if(obj_personal_owned_by_other(ch, obj)) {
			tell_from_jeweler(ch, mob, "Quel pezzo e' di qualcun altro.");
			return true;
		}
		pending.name_b = obj_vis_key(ch, obj);
		pending.id_b = obj->db_instance_id;
		pending.step = EditBrokerStep::WaitAffectPick;
		touch_edit_broker_pending(pending);
		broker_ambient_examine(ch, mob, obj);
		{
			std::ostringstream os;
			os << "Ok, metto su " << obj_shortn(obj) << ".";
			tell_from_jeweler(ch, mob, os.str());
		}
		tell_from_jeweler(ch, mob, "Ecco gli effetti da spostare:");
		show_numbered_affect_list(ch, mob, obj_a, pending.affect_labels, true);
		return true;
	}

	if(pending.step == EditBrokerStep::WaitGiveRename) {
		if(!rename_object_eligible(ch, mob, obj)) {
			return true;
		}
		pending.name_c = obj_vis_key(ch, obj);
		pending.id_c = obj->db_instance_id;
		pending.step = EditBrokerStep::WaitRenameKeywords;
		touch_edit_broker_pending(pending);
		broker_ambient_examine(ch, mob, obj);
		{
			std::ostringstream os;
			os << "Ok, rinomino " << obj_shortn(obj) << ".";
			tell_from_jeweler(ch, mob, os.str());
		}
		tell_from_jeweler(ch, mob,
						  "Dimmi le $c0011keywords$c0007 (nome per get/give), "
						  "oppure $c0011indietro$c0007 / $c0011shake$c0007 per "
						  "annullare.");
		return true;
	}

	/* WaitGiveDestroy */
	if(!obj_is_owned_edit(ch, obj)) {
		tell_from_jeweler(ch, mob,
						  "Serve un pezzo EDIT, PERSONAL e di tua proprieta'. Riprova.");
		return true;
	}
	pending.name_c = obj_vis_key(ch, obj);
	pending.id_c = obj->db_instance_id;
	DestroyPlan plan;
	if(!build_destroy_plan(ch, mob, pending.name_c, plan)) {
		broker_ambient_stop(ch, mob);
		return true;
	}
	pending.step = EditBrokerStep::ConfirmDestroy;
	pending.op = EditBrokerOpKind::Destroy;
	touch_edit_broker_pending(pending);
	broker_ambient_examine(ch, mob, obj);
	preview_destroy(ch, mob, plan);
	return true;
}

void do_trasferisci(struct char_data* ch, struct char_data* mob, std::string_view args) {
	if(!ch || !mob) {
		return;
	}
	(void)args;
	begin_transfer_wizard(ch, mob);
}

void do_distruggi(struct char_data* ch, struct char_data* mob, std::string_view args) {
	if(!ch || !mob) {
		return;
	}
	(void)args;
	begin_destroy_wizard(ch, mob);
}

void do_rinomina(struct char_data* ch, struct char_data* mob, std::string_view args) {
	if(!ch || !mob) {
		return;
	}
	(void)args;
	begin_rename_wizard(ch, mob);
}

void sweep_edit_broker_pending() {
	const time_t now = time(nullptr);
	for(auto it = g_edit_broker_pending.begin(); it != g_edit_broker_pending.end();) {
		char_data* client = it->first;
		EditBrokerPending& pending = it->second;
		const bool expired = now > pending.expires_at;
		const bool room_bad = !edit_broker_pending_room_ok(client, pending);
		if(!expired && !room_bad) {
			++it;
			continue;
		}
		char_data* j = pending.jeweler;
		it = g_edit_broker_pending.erase(it);
		if(client && j && client->in_room == j->in_room) {
			broker_ambient_stop(client, j);
			tell_from_jeweler(client, j, "La richiesta e' scaduta o non e' piu' valida.");
		}
	}
}

} // namespace

void edit_broker_on_char_extract(struct char_data* ch) {
	if(!ch) {
		return;
	}
	cancel_edit_broker_pending(ch, nullptr, false);
}

void edit_broker_reconcile_boot() {
#if USE_MYSQL
	const unsigned n = object_instance_broker_reconcile(nullptr, false);
	if(n > 0) {
		mudlog(LOG_CHECK, "edit_broker_reconcile_boot: closed %u open transfer(s)", n);
	}
#else
	(void)0;
#endif
}

void edit_broker_reconcile_login(struct char_data* ch) {
	if(!ch || IS_NPC(ch) || !GET_NAME(ch)) {
		return;
	}
#if USE_MYSQL
	const unsigned n = object_instance_broker_reconcile(GET_NAME(ch), true);
	if(n > 0) {
		mudlog(LOG_PLAYERS, "edit_broker_reconcile_login: %s closed %u transfer(s)",
			   GET_NAME(ch), n);
	}
#else
	(void)ch;
#endif
}

MOBSPECIAL_FUNC(EditAffectBroker) {
	if(!ch || !mob) {
		return FALSE;
	}
	if(type == EVENT_TICK) {
		sweep_edit_broker_pending();
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

	if(pc_ok && IS_PC(ch) && !IS_POLY(ch)) {
		if(cmd == CMD_GIVE) {
			/* Con o senza procedura: give al broker e' sempre intercettato. */
			if(handle_edit_broker_give(ch, mob, arg ? arg : "")) {
				return TRUE;
			}
			return FALSE;
		}
		if(cmd == CMD_NOD) {
			auto it = g_edit_broker_pending.find(ch);
			if(it != g_edit_broker_pending.end() && it->second.jeweler == mob &&
			   pending_is_confirm_step(it->second.step)) {
				if(apply_edit_broker_answer(ch, mob, BrokerConfirmAnswer::Yes)) {
					return TRUE;
				}
			}
			return FALSE;
		}
		if(cmd == CMD_SHAKE) {
			auto it = g_edit_broker_pending.find(ch);
			if(it != g_edit_broker_pending.end() && it->second.jeweler == mob) {
				cancel_edit_broker_pending(ch, mob, true);
				return TRUE;
			}
			return FALSE;
		}
		if(cmd == CMD_SAY || cmd == CMD_SAY_APICE) {
			std::string_view speech = arg ? arg : "";
			while(!speech.empty() && std::isspace(static_cast<unsigned char>(speech.front()))) {
				speech.remove_prefix(1);
			}
			auto it = g_edit_broker_pending.find(ch);
			if(it != g_edit_broker_pending.end() && it->second.jeweler == mob) {
				if(try_handle_edit_broker_confirm(ch, mob, speech, false)) {
					return TRUE;
				}
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
	if(topic == "info") {
		show_edit_broker_info_object(ch, mob, after);
		return TRUE;
	}
	if(topic == "dettagli" || topic == "regole") {
		show_edit_broker_details(ch, mob, after);
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
	if(topic == "rinomina") {
		do_rinomina(ch, mob, after);
		return TRUE;
	}

	tell_from_jeweler(ch, mob,
					  "Non ho capito. Chiedimi $c0011aiuto$c0007 per vedere cosa posso "
					  "fare.");
	return TRUE;
}

} // namespace Alarmud
