/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Schermo VT100 (display 1 e 2): barra di stato adattata alla larghezza,
 * pulizia dello scroll al cambio stanza (display 1) e pannello fisso con
 * la stanza attuale (display 2). Le dimensioni vengono da NAWS
 * (telnet_sync_screen_size) o da resize.
 */
/***************************  System  include ************************************/
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "vt_screen.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "utility.hpp"   /* exit_ok */
#include "vt100c.hpp"

namespace Alarmud {

namespace {

constexpr int kMinScrollRows = 8;   /* sotto: display 2 si comporta come display 1 */
constexpr int kMinPaneRows = 4;     /* nome, almeno 2 righe di descrizione, uscite */

int screen_rows(const struct char_data* ch) {
	return (ch != nullptr && ch->size > 0) ? ch->size : 25;
}

int screen_cols(const struct char_data* ch) {
	return (ch != nullptr && ch->width >= 40) ? ch->width : 80;
}

std::string cursor_pos(int row, int col) {
	char buf[32];
	std::snprintf(buf, sizeof(buf), VT_CURSPOS, row, col);
	return buf;
}

/** Lunghezza visibile: i codici colore $cNNNN / $$cNNNN non occupano colonne. */
bool color_code_at(const std::string& s, std::size_t i, std::size_t* skip) {
	if(s[i] != '$') {
		return false;
	}
	if(i + 1 < s.size() && std::toupper(static_cast<unsigned char>(s[i + 1])) == 'C') {
		*skip = 6;
		return true;
	}
	if(i + 2 < s.size() && s[i + 1] == '$' &&
			std::toupper(static_cast<unsigned char>(s[i + 2])) == 'C') {
		*skip = 7;
		return true;
	}
	return false;
}

std::size_t visible_len(const std::string& s) {
	std::size_t n = 0;
	for(std::size_t i = 0; i < s.size();) {
		std::size_t skip = 0;
		if(color_code_at(s, i, &skip)) {
			i += skip;
		}
		else if(s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '$') {
			i += 2;
			++n;
		}
		else {
			++i;
			++n;
		}
	}
	return n;
}

/** Primi n caratteri visibili di s, codici colore compresi. */
std::string truncate_visible(const std::string& s, std::size_t n) {
	std::string out;
	std::size_t seen = 0;
	for(std::size_t i = 0; i < s.size();) {
		std::size_t skip = 0;
		if(color_code_at(s, i, &skip)) {
			out.append(s, i, std::min(skip, s.size() - i));
			i += skip;
			continue;
		}
		if(seen >= n) {
			break;
		}
		if(s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '$') {
			out.append(s, i, 2);
			i += 2;
		}
		else {
			out += s[i++];
		}
		++seen;
	}
	return out;
}

/** Testo rifluito a parole su righe di al massimo width colonne visibili. */
std::vector<std::string> wrap_text(const std::string& text, std::size_t width) {
	std::vector<std::string> lines;
	std::string line;
	std::size_t lineLen = 0;
	std::string word;
	const auto flush_word = [&]() {
		if(word.empty()) {
			return;
		}
		std::size_t wlen = visible_len(word);
		if(wlen > width) {
			word = truncate_visible(word, width);
			wlen = width;
		}
		if(lineLen > 0 && lineLen + 1 + wlen > width) {
			lines.push_back(line);
			line.clear();
			lineLen = 0;
		}
		if(lineLen > 0) {
			line += ' ';
			++lineLen;
		}
		line += word;
		lineLen += wlen;
		word.clear();
	};
	for(char c : text) {
		if(c == ' ' || c == '\n' || c == '\r' || c == '\t') {
			flush_word();
		}
		else {
			word += c;
		}
	}
	flush_word();
	if(!line.empty()) {
		lines.push_back(line);
	}
	return lines;
}

std::string colorize(const struct char_data* ch, const std::string& line) {
	const bool ansi = ch != nullptr && IS_SET(ch->player.user_flags, USE_ANSI);
	return std::string(ParseAnsiColors(ansi, line.c_str()));
}

struct BarSpec {
	int row;
	int label_col;
	int value_col;
	int clear;
	const char* label;
};

/* fuori linea: con le costanti inlined GCC riscrive i confronti e
 * -Werror=strict-overflow fallisce */
__attribute__((noinline)) int clamp_span(int span, int cap) {
	if(span >= cap) {
		return cap;
	}
	return span >= 1 ? span : 1;
}

/* ---------------------------------------------------------------------
 * Minimappa di display 2 (stile wilderness), a destra del testo del
 * pannello. Le stanze non hanno coordinate: BFS sulle uscite N/E/S/O dalla
 * stanza del PG, (dx,dy) per passo; sulla cella vince la prima stanza che
 * ci arriva (i conflitti non si espandono). Porte chiuse o segrete non si
 * attraversano. Un carattere colorato per stanza secondo il sector, celle
 * separate da uno spazio; '@' e' il PG. Stanze con uscita su/giu':
 * simbolo sottolineato. Sotto la griglia una riga "Alto ^ Basso v" per la
 * stanza del PG.
 * ------------------------------------------------------------------- */
constexpr std::size_t kMapMaxRooms = 100;
constexpr int kMapMaxRadiusY = 8;

struct MapGeom {
	int rx;        /* passi a est/ovest */
	int ry;        /* passi a nord/sud */
	int cells_w;   /* celle per riga */
	int grid_rows; /* righe della griglia */
	int col_w;     /* colonne di schermo occupate: griglia + 2 di stacco */
};

/* fuori linea per -Werror=strict-overflow: niente costanti propagate nei
 * confronti. false = niente mappa (finestra stretta o pannello basso). */
__attribute__((noinline)) bool map_geometry(int cols, int pane, MapGeom* g) {
	if(g == nullptr || cols < 80 || pane < 5) {
		return false;
	}
	g->rx = cols >= 100 ? 5 : 4;
	/* righe del testo = pane-2; una va alla riga su/giu'. Il tetto si
	 * decide su pane, senza confrontare un'espressione (strict-overflow) */
	constexpr int kPaneForMaxRadius = kMapMaxRadiusY * 2 + 4;
	g->ry = (pane >= kPaneForMaxRadius) ? kMapMaxRadiusY : (pane - 4) / 2;
	g->cells_w = g->rx * 2 + 1;
	g->grid_rows = g->ry * 2 + 1;
	g->col_w = g->cells_w * 2 + 1;
	return true;
}

/** Indice della cella (dx,dy) nella griglia, -1 se fuori raggio. */
__attribute__((noinline)) int map_cell(int dx, int dy, int rx, int ry) {
	if(dx < -rx || dx > rx || dy < -ry || dy > ry) {
		return -1;
	}
	return (dy + ry) * (rx * 2 + 1) + (dx + rx);
}

struct MapCell {
	int sector = -1;    /* -1 = vuota */
	bool updown = false;
	bool self = false;
};

/** Uscita dir percorribile: esiste, non chiusa ne' segreta, stanza valida. */
bool map_exit_open(struct room_data* rp, int dir, struct room_data** to) {
	struct room_direction_data* const exitp = rp->dir_option[dir];
	if(exitp == nullptr || IS_SET(exitp->exit_info, EX_CLOSED) ||
			IS_SET(exitp->exit_info, EX_SECRET)) {
		return false;
	}
	return exit_ok(exitp, to) != 0;
}

bool map_has_up(struct room_data* rp) {
	struct room_data* to = nullptr;
	return rp != nullptr && map_exit_open(rp, 4, &to);
}

bool map_has_down(struct room_data* rp) {
	struct room_data* to = nullptr;
	return rp != nullptr && map_exit_open(rp, 5, &to);
}

std::vector<MapCell> map_bfs(long start_room, const MapGeom& g) {
	std::vector<MapCell> cells(static_cast<std::size_t>(g.cells_w * g.grid_rows));
	struct room_data* const start = real_roomp(start_room);
	const int center = map_cell(0, 0, g.rx, g.ry);
	if(start == nullptr || center < 0) {
		return cells;
	}
	struct Node {
		long room;
		int dx;
		int dy;
	};
	static const int kDx[4] = {0, 1, 0, -1}; /* nord, est, sud, ovest */
	static const int kDy[4] = {-1, 0, 1, 0};
	std::vector<Node> queue;
	std::unordered_set<long> seen;
	queue.push_back({start_room, 0, 0});
	seen.insert(start_room);
	MapCell& self = cells[static_cast<std::size_t>(center)];
	self.sector = static_cast<int>(start->sector_type);
	self.updown = map_has_up(start) || map_has_down(start);
	self.self = true;

	for(std::size_t head = 0; head < queue.size(); ++head) {
		const Node n = queue[head];
		struct room_data* const rp = real_roomp(n.room);
		if(rp == nullptr) {
			continue;
		}
		for(int dir = 0; dir < 4; ++dir) {
			struct room_data* to = nullptr;
			if(!map_exit_open(rp, dir, &to) || to == nullptr) {
				continue;
			}
			const long toRoom = rp->dir_option[dir]->to_room;
			if(seen.count(toRoom) != 0) {
				continue;
			}
			if(seen.size() >= kMapMaxRooms) {
				return cells;
			}
			const int idx = map_cell(n.dx + kDx[dir], n.dy + kDy[dir], g.rx, g.ry);
			if(idx < 0) {
				continue; /* oltre il raggio: si riprova da un'altra strada */
			}
			seen.insert(toRoom);
			MapCell& cell = cells[static_cast<std::size_t>(idx)];
			if(cell.sector >= 0) {
				continue; /* cella gia' presa: vince la prima, niente espansione */
			}
			cell.sector = static_cast<int>(to->sector_type);
			cell.updown = map_has_up(to) || map_has_down(to);
			queue.push_back({toRoom, n.dx + kDx[dir], n.dy + kDy[dir]});
		}
	}
	return cells;
}

/** Simbolo e colore (2 cifre di primo piano $c) del sector. */
void map_sector_symbol(int sector, char* sym, const char** fg) {
	switch(sector) {
	case SECT_INSIDE:       *sym = '.';  *fg = "07"; break; /* grigio chiaro */
	case SECT_CITY:         *sym = '#';  *fg = "15"; break; /* bianco */
	case SECT_FIELD:        *sym = '"';  *fg = "10"; break; /* verde chiaro */
	case SECT_FOREST:       *sym = '*';  *fg = "02"; break; /* verde */
	case SECT_HILLS:        *sym = 'n';  *fg = "11"; break; /* giallo */
	case SECT_MOUNTAIN:     *sym = '^';  *fg = "03"; break; /* marrone */
	case SECT_WATER_SWIM:   *sym = '~';  *fg = "14"; break; /* ciano chiaro */
	case SECT_WATER_NOSWIM: *sym = '~';  *fg = "12"; break; /* blu chiaro */
	case SECT_AIR:          *sym = ':';  *fg = "06"; break; /* ciano */
	case SECT_UNDERWATER:   *sym = '~';  *fg = "04"; break; /* blu scuro */
	case SECT_DESERT:       *sym = '.';  *fg = "11"; break; /* giallo */
	case SECT_TREE:         *sym = 'T';  *fg = "10"; break; /* verde chiaro */
	case SECT_DARKCITY:     *sym = '#';  *fg = "08"; break; /* grigio scuro */
	default:                *sym = '?';  *fg = "07"; break;
	}
}

/** Una riga della griglia: celle separate da spazi, colore chiuso a fine riga. */
std::string map_row(const std::vector<MapCell>& cells, const MapGeom& g, int row) {
	std::string out;
	for(int cx = 0; cx < g.cells_w; ++cx) {
		if(cx > 0) {
			out += ' ';
		}
		const std::size_t idx = static_cast<std::size_t>(row * g.cells_w + cx);
		const MapCell& c = (idx < cells.size()) ? cells[idx] : MapCell();
		if(c.self) {
			/* '@' rosso chiaro, sottolineato se c'e' su/giu' */
			out += c.updown ? "$c4009@" : "$c0009@";
		}
		else if(c.sector < 0) {
			out += ' ';
		}
		else {
			char sym = '?';
			const char* fg = "07";
			map_sector_symbol(c.sector, &sym, &fg);
			out += c.updown ? "$c40" : "$c00"; /* 4 = sottolineato: ha su/giu' */
			out += fg;
			out += sym;
		}
	}
	out += "$c0007";
	return out;
}

/** Riga sotto la griglia: uscite alto/basso della stanza del PG. */
std::string map_updown_row(long room) {
	struct room_data* const rp = real_roomp(room);
	std::string out;
	if(map_has_up(rp)) {
		out += "$c0014Alto ^";
	}
	if(map_has_down(rp)) {
		if(!out.empty()) {
			out += "$c0007  ";
		}
		out += "$c0011Basso v";
	}
	if(!out.empty()) {
		out += "$c0007";
	}
	return out;
}

BarSpec bar_spec(const struct char_data* ch, VtBarField field) {
	const int R = screen_rows(ch);
	const int W = screen_cols(ch);
	if(W >= 80) {
		/* layout storico a 80 colonne */
		switch(field) {
		case VT_BAR_HP:
			return {R - 3, 1, 13, 10, "Hit Points: "};
		case VT_BAR_MOVE:
			return {R - 3, 40, 58, 10, "Movement Points: "};
		case VT_BAR_MANA:
			return {R - 2, 1, 7, 10, "Mana: "};
		case VT_BAR_GOLD:
			return {R - 2, 40, 47, 16, "Gold: "};
		case VT_BAR_EXP:
		default:
			return {R - 1, 1, 20, 16, "Experience Points: "};
		}
	}
	/* finestra stretta: etichette corte, seconda colonna a meta' */
	const int half = W / 2 + 1;
	const auto room = [](int from, int to, int cap) {
		return clamp_span(to - from, cap);
	};
	switch(field) {
	case VT_BAR_HP:
		return {R - 3, 1, 5, room(5, half - 1, 12), "PF: "};
	case VT_BAR_MOVE:
		return {R - 3, half, half + 5, room(half + 5, W, 12), "Mov: "};
	case VT_BAR_MANA:
		return {R - 2, 1, 7, room(7, half - 1, 12), "Mana: "};
	case VT_BAR_GOLD:
		return {R - 2, half, half + 5, room(half + 5, W, 16), "Oro: "};
	case VT_BAR_EXP:
	default:
		return {R - 1, 1, 6, room(6, W, 16), "Exp: "};
	}
}

} // namespace

int vt_pane_rows(const struct char_data* ch) {
	if(ch == nullptr || ch->term != VT100 || !ch->term_pane) {
		return 0;
	}
	const int R = screen_rows(ch);
	const int pane = std::max(kMinPaneRows, R * 2 / 5);  /* ~40% dell'altezza */
	/* righe di scroll = (R-5) - (pane+2) + 1 = R - pane - 6; nessuna costante
	 * sul lato variabile del confronto (-Werror=strict-overflow) */
	const int avail = R - pane;
	constexpr int kNeeded = kMinScrollRows + 6;
	return avail >= kNeeded ? pane : 0;
}

bool vt_pane_active(const struct char_data* ch) {
	return vt_pane_rows(ch) > 0;
}

int vt_scroll_top(const struct char_data* ch) {
	const int pane = vt_pane_rows(ch);
	return pane > 0 ? pane + 2 : 1;
}

int vt_scroll_bottom(const struct char_data* ch) {
	const int rows = screen_rows(ch);
	return rows >= 6 ? rows - 5 : 1;
}

int vt_page_lines(const struct char_data* ch) {
	if(vt_pane_active(ch)) {
		const int span = vt_scroll_bottom(ch) - vt_scroll_top(ch);
		return span >= 7 ? span - 2 : 5;
	}
	const int rows = screen_rows(ch);
	return rows >= 13 ? rows - 8 : 5;
}

std::string vt_separator(const struct char_data* ch) {
	/* mai largo quanto la finestra: l'ultima colonna resta libera, cosi'
	 * il terminale non va a capo */
	const int cols = screen_cols(ch);
	const int len = cols >= 3 ? cols - 1 : 2;
	std::string sep(static_cast<std::size_t>(len), '=');
	sep.front() = '-';
	sep.back() = '-';
	return sep;
}

std::string vt_bar_field_init(const struct char_data* ch, VtBarField field, const std::string& value) {
	const BarSpec spec = bar_spec(ch, field);
	return cursor_pos(spec.row, spec.label_col) + spec.label +
		   cursor_pos(spec.row, spec.value_col) +
		   value.substr(0, static_cast<std::size_t>(spec.clear));
}

std::string vt_bar_field_update(const struct char_data* ch, VtBarField field, const std::string& value) {
	const BarSpec spec = bar_spec(ch, field);
	std::string out = VT_CURSAVE;
	out += cursor_pos(spec.row, spec.value_col);
	out += std::string(static_cast<std::size_t>(spec.clear), ' ');
	out += cursor_pos(spec.row, spec.value_col);
	out += value.substr(0, static_cast<std::size_t>(spec.clear));
	out += VT_CURREST;
	return out;
}

void vt_pane_refresh(struct char_data* ch) {
	if(ch == nullptr || ch->desc == nullptr) {
		return;
	}
	vt_screen_state& s = ch->desc->screen;
	s.pane_valid = true;
	s.pane_dirty = true;
	s.pane_room = ch->in_room;
	s.pane_name.clear();
	s.pane_desc.clear();
	s.pane_exits.clear();
	s.pane_can_see = false;

	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		s.pane_name = "?";
		return;
	}
	/* stesse condizioni di do_look: chi non vede non legge la stanza */
	if(GET_POS(ch) < POSITION_SLEEPING) {
		s.pane_name = "$c0008Non vedi nient'altro che stelle!";
	}
	else if(GET_POS(ch) == POSITION_SLEEPING) {
		s.pane_name = "$c0008Stai dormendo.";
	}
	else if(IS_AFFECTED(ch, AFF_BLIND)) {
		s.pane_name = "$c0008Sei cieco, non vedi nulla.";
	}
	else if(look_room_too_dark(ch, rp)) {
		s.pane_name = "$c0008E' molto buio qui...";
	}
	else {
		/* la descrizione resta anche in modo breve: nel pannello e' fissa e
		 * non riempie lo scroll, che e' il motivo del modo breve */
		s.pane_name = std::string("$c0015") + (rp->name ? rp->name : "?");
		s.pane_desc = rp->description ? rp->description : "";
		s.pane_exits = exits_in_room_text(ch);
		s.pane_can_see = true; /* buio, cecita', sonno: niente minimappa */
	}
}

std::string vt_pane_sequence(struct char_data* ch) {
	const int pane = vt_pane_rows(ch);
	if(pane <= 0 || ch->desc == nullptr) {
		return "";
	}
	vt_screen_state& s = ch->desc->screen;
	if(!s.pane_valid) {
		vt_pane_refresh(ch);
	}
	const std::size_t width = static_cast<std::size_t>(screen_cols(ch) - 1);
	const std::size_t descRows = static_cast<std::size_t>(pane - 2);

	/* minimappa a destra: la descrizione va a capo prima per lasciarle posto */
	MapGeom geom{};
	const bool showMap = s.pane_can_see && map_geometry(screen_cols(ch), pane, &geom);
	const std::size_t mapCols = showMap ? static_cast<std::size_t>(geom.col_w) : 0;
	const std::size_t textWidth = (width > mapCols) ? width - mapCols : width;

	std::vector<std::string> rows;
	rows.push_back(truncate_visible(s.pane_name, width));
	std::vector<std::string> desc = wrap_text(s.pane_desc, textWidth);
	if(desc.size() > descRows) {
		desc.resize(descRows);
		std::string& last = desc.back();
		last = truncate_visible(last, textWidth > 3 ? textWidth - 3 : 0) + "$c0007...";
	}
	for(const std::string& line : desc) {
		rows.push_back(line);
	}
	while(rows.size() < static_cast<std::size_t>(pane - 1)) {
		rows.push_back("");
	}
	if(!s.pane_can_see) {
		rows.push_back(""); /* buio/cieco/sonno: niente uscite */
	}
	else {
		rows.push_back(truncate_visible("$c0011Uscite:$c0007" +
										(s.pane_exits.empty() ? std::string(" nessuna") : s.pane_exits), width));
	}

	if(showMap) {
		/* righe 1..: griglia, poi la riga su/giu'; il testo a sinistra viene
		 * chiuso col colore normale e allineato alla colonna della mappa */
		const std::vector<MapCell> cells = map_bfs(s.pane_room, geom);
		/* m == grid_rows e' la riga su/giu' (<=: niente "X + 1" nel confronto) */
		for(int m = 0; m <= geom.grid_rows; ++m) {
			const std::size_t r = static_cast<std::size_t>(m) + 1;
			if(r + 1 >= rows.size()) {
				break; /* l'ultima riga e' delle uscite */
			}
			std::string& line = rows[r];
			line += "$c0007";
			const std::size_t vl = visible_len(line);
			if(vl < textWidth) {
				line.append(textWidth - vl, ' ');
			}
			line += "  ";
			line += (m < geom.grid_rows) ? map_row(cells, geom, m) : map_updown_row(s.pane_room);
		}
	}

	std::string out = VT_CURSAVE;
	for(int r = 0; r < pane; ++r) {
		out += cursor_pos(r + 1, 1);
		out += "\033[2K";
		out += colorize(ch, rows[static_cast<std::size_t>(r)]);
	}
	out += cursor_pos(pane + 1, 1);
	out += "\033[2K";
	out += vt_separator(ch);
	out += VT_CURREST;
	s.pane_dirty = false;
	return out;
}

void vt_room_look(struct char_data* ch) {
	if(ch == nullptr || ch->desc == nullptr || ch->term != VT100) {
		return;
	}
	vt_screen_state& s = ch->desc->screen;
	if(vt_pane_active(ch)) {
		vt_pane_refresh(ch);
	}
	/* ogni look della stanza (anche nella stessa) pulisce lo scroll e
	 * riparte dall'alto, sia in display 1 sia sotto il pannello del
	 * display 2. CMD_LOOK e' 15 come i look interni dopo un movimento,
	 * quindi non si distingue il look scritto a mano. */
	s.clear_pending = true;
	s.last_look_room = ch->in_room;
}

bool vt_room_text_in_pane(const struct char_data* ch) {
	return vt_pane_active(ch);
}

std::string vt_take_output_prefix(struct descriptor_data* d) {
	if(d == nullptr || !d->screen.clear_pending) {
		return "";
	}
	d->screen.clear_pending = false;
	struct char_data* const ch = d->character;
	if(ch == nullptr || ch->term != VT100 || d->connected != CON_PLYNG) {
		return "";
	}
	/* pulisce solo le righe che scorrono: pannello, separatori e barra
	 * restano (display 2: lo scroll parte sotto il pannello) */
	std::string out;
	const int top = vt_scroll_top(ch);
	const int bottom = vt_scroll_bottom(ch);
	for(int r = top; r <= bottom; ++r) {
		out += cursor_pos(r, 1);
		out += "\033[2K";
	}
	out += cursor_pos(top, 1);
	return out;
}

void vt_before_prompt(struct descriptor_data* d) {
	if(d == nullptr || d->connected != CON_PLYNG) {
		return;
	}
	struct char_data* const ch = d->character;
	if(!vt_pane_active(ch)) {
		return;
	}
	vt_screen_state& s = d->screen;
	/* stanza cambiata senza look (teleport, summon, trappole...) */
	if(!s.pane_valid || s.pane_room != ch->in_room) {
		vt_pane_refresh(ch);
	}
	if(s.pane_dirty) {
		write_to_descriptor(d->descriptor, vt_pane_sequence(ch).c_str());
	}
}

void vt_enter_editor(struct descriptor_data* d) {
	if(d == nullptr) {
		return;
	}
	struct char_data* const ch = d->character;
	if(!vt_pane_active(ch)) {
		return;
	}
	/* gli editor disegnano dall'alto come in display 1 */
	char buf[32];
	std::snprintf(buf, sizeof(buf), VT_MARGSET, 1, vt_scroll_bottom(ch));
	write_to_descriptor(d->descriptor, buf);
	d->screen.pane_valid = false; /* all'uscita InitScreen lo ridisegna */
}

} // namespace Alarmud
