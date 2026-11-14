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
#include <array>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <system_error>
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
#include "maximums.hpp"   /* title_type */
#include "constants.hpp"  /* titles, RacialMax */
#include "multiclass.hpp"  /* HasClass */
#include "modify.hpp"      /* page_string */
#include "str_ci.hpp"      /* str_iequals, str_istarts_with */
#include "spells.hpp"      /* SPELL_*, STATUS_QUEST */
#include "handler.hpp"   /* get_char_room_vis */
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
	return vt_csi_pos(row, col);
}

/** Lunghezza visibile: i codici colore $cNNNN / $$cNNNN non occupano
 *  colonne, e nemmeno le sequenze ANSI gia' espanse (ESC[...m), che
 *  arrivano dal testo catturato per il pannello (oggetti e creature). */
bool color_code_at(const std::string& s, std::size_t i, std::size_t* skip) {
	if(s[i] == '\033') {
		std::size_t j = i + 1;
		if(j < s.size() && s[j] == '[') {
			++j;
			while(j < s.size() &&
					(std::isdigit(static_cast<unsigned char>(s[j])) || s[j] == ';')) {
				++j;
			}
		}
		if(j < s.size()) {
			++j; /* lettera finale (m) o carattere dopo ESC */
		}
		*skip = j - i;
		return true;
	}
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

/* fuori linea: con le costanti inlined GCC riscrive i confronti e
 * -Werror=strict-overflow fallisce */
__attribute__((noinline)) int at_least_one(int v) {
	return v >= 1 ? v : 1;
}

/** avail = R - pane - n: c'e' posto per almeno kMinScrollRows righe di scroll. */
__attribute__((noinline)) bool scroll_room_ok(int avail) {
	constexpr int kNeeded = kMinScrollRows + 2;
	return avail >= kNeeded;
}

/** Righe per pagina del paginatore: 3 righe della regione restano al prompt. */
__attribute__((noinline)) int page_lines_for(int scroll_rows) {
	return scroll_rows >= 10 ? scroll_rows - 3 : 5;
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
	static constexpr std::array<int, 4> kDx{0, 1, 0, -1}; /* nord, est, sud, ovest */
	static constexpr std::array<int, 4> kDy{-1, 0, 1, 0};
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
			const std::size_t d = static_cast<std::size_t>(dir);
			const int idx = map_cell(n.dx + kDx[d], n.dy + kDy[d], g.rx, g.ry);
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
			queue.push_back({toRoom, n.dx + kDx[d], n.dy + kDy[d]});
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

} // namespace

int vt_pane_rows(const struct char_data* ch) {
	if(ch == nullptr || ch->term != VT100 || !ch->term_pane) {
		return 0;
	}
	const int R = screen_rows(ch);
	const int pane = std::max(kMinPaneRows, R * 2 / 5);  /* ~40% dell'altezza */
	/* righe di scroll = (R-n-1) - (pane+2) + 1 = R - pane - n - 2, con n righe
	 * di statusbar; nessuna costante sul lato variabile del confronto
	 * (-Werror=strict-overflow) */
	return scroll_room_ok(R - pane - vt_status_rows(ch)) ? pane : 0;
}

bool vt_pane_active(const struct char_data* ch) {
	return vt_pane_rows(ch) > 0;
}

int vt_scroll_top(const struct char_data* ch) {
	const int pane = vt_pane_rows(ch);
	return pane > 0 ? pane + 2 : 1;
}

int vt_scroll_bottom(const struct char_data* ch) {
	/* sopra il separatore della statusbar */
	return at_least_one(screen_rows(ch) - vt_status_rows(ch) - 1);
}

int vt_separator_row(const struct char_data* ch) {
	return at_least_one(screen_rows(ch) - vt_status_rows(ch));
}

int vt_page_lines(const struct char_data* ch) {
	const int scroll = vt_scroll_bottom(ch) - vt_scroll_top(ch) + 1;
	return page_lines_for(scroll);
}

std::string vt_csi_pos(int row, int col) {
	/* come VT_CURSPOS: ESC[riga;colonnaH */
	return "\033[" + std::to_string(row) + ";" + std::to_string(col) + "H";
}

std::string vt_csi_margins(int top, int bottom) {
	/* come VT_MARGSET: ESC[alto;bassor */
	return "\033[" + std::to_string(top) + ";" + std::to_string(bottom) + "r";
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

/* Cosa il PG vede della stanza (VtPaneView), con le stesse condizioni di do_look. */
static VtPaneView pane_view_state(struct char_data* ch, struct room_data* rp) {
	if(rp == nullptr) {
		return VtPaneView::NoRoom;
	}
	if(GET_POS(ch) < POSITION_SLEEPING) {
		return VtPaneView::Stars;
	}
	if(GET_POS(ch) == POSITION_SLEEPING) {
		return VtPaneView::Sleep;
	}
	if(IS_AFFECTED(ch, AFF_BLIND)) {
		return VtPaneView::Blind;
	}
	if(look_room_too_dark(ch, rp)) {
		return VtPaneView::Dark;
	}
	return VtPaneView::Room;
}

/*
 * Oggetti e creature della stanza con le stesse righe del look (gruppi,
 * visibilita', hide, sense life, colori): l'output di list_obj_in_room e
 * list_char_in_room viene catturato con page_capture. Il testo e' gia'
 * passato da ParseAnsiColors, quindi contiene sequenze ESC[...m (gestite
 * da color_code_at); i '$' rimasti diventano \$ perche' colorize non li
 * riprenda per codici colore. false = cattura non possibile (gia' in corso),
 * out invariato.
 */
static bool pane_capture_contents(struct char_data* ch, std::vector<std::string>* out) {
	struct descriptor_data* const d = ch->desc;
	if(d == nullptr || out == nullptr || d->page_capture != nullptr) {
		return false;
	}
	struct room_data* const rp = real_roomp(ch->in_room);
	out->clear();
	if(rp == nullptr) {
		return true;
	}
	std::string captured;
	/* show_obj_to_char usa page_string: il paginatore del giocatore (se sta
	 * leggendo un testo lungo) va messo da parte e rimesso dopo */
	char* const savedHead = d->showstr_head;
	const char* const savedPoint = d->showstr_point;
	d->showstr_head = nullptr;
	d->showstr_point = nullptr;
	d->page_capture = &captured;
	list_obj_in_room(rp->contents, ch);
	list_char_in_room(rp->people, ch);
	d->page_capture = nullptr;
	if(d->showstr_head != nullptr) {
		free(d->showstr_head);
	}
	d->showstr_head = savedHead;
	d->showstr_point = savedPoint;

	std::string line;
	const auto flush_line = [&]() {
		if(visible_len(line) > 0) {
			out->push_back(line);
		}
		line.clear();
	};
	for(char c : captured) {
		if(c == '\n') {
			flush_line();
		}
		else if(c == '\r') {
			continue;
		}
		else if(c == '$') {
			line += "\\$";
		}
		else {
			line += c;
		}
	}
	flush_line();
	return true;
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
	s.pane_contents.clear();

	struct room_data* const rp = real_roomp(ch->in_room);
	s.pane_view = pane_view_state(ch, rp);
	/* stesse condizioni di do_look: chi non vede non legge la stanza */
	if(s.pane_view == VtPaneView::NoRoom) {
		s.pane_name = "?";
	}
	else if(s.pane_view == VtPaneView::Stars) {
		s.pane_name = "$c0008Non vedi nient'altro che stelle!";
	}
	else if(s.pane_view == VtPaneView::Sleep) {
		s.pane_name = "$c0008Stai dormendo.";
	}
	else if(s.pane_view == VtPaneView::Blind) {
		s.pane_name = "$c0008Sei cieco, non vedi nulla.";
	}
	else if(s.pane_view == VtPaneView::Dark) {
		s.pane_name = "$c0008E' molto buio qui...";
	}
	else {
		/* la descrizione resta anche in modo breve: nel pannello e' fissa e
		 * non riempie lo scroll, che e' il motivo del modo breve */
		s.pane_name = std::string("$c0015") + (rp->name ? rp->name : "?");
		s.pane_desc = rp->description ? rp->description : "";
		s.pane_exits = exits_in_room_text(ch);
		s.pane_can_see = true; /* buio, cecita', sonno: niente minimappa */
		pane_capture_contents(ch, &s.pane_contents);
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
	/* righe fra nome e uscite: descrizione, poi oggetti e creature */
	const std::size_t bodyRows = static_cast<std::size_t>(pane - 2);

	/* minimappa a destra: la descrizione va a capo prima per lasciarle posto */
	MapGeom geom{};
	const bool showMap = s.pane_can_see && map_geometry(screen_cols(ch), pane, &geom);
	const std::size_t mapCols = showMap ? static_cast<std::size_t>(geom.col_w) : 0;
	/* testo al massimo di 80 colonne (le descrizioni sono scritte cosi'):
	 * la minimappa gli sta subito accanto invece che in fondo a destra */
	constexpr std::size_t kPaneTextMax = 80;
	const std::size_t textWidth =
		std::min(kPaneTextMax, (width > mapCols) ? width - mapCols : width);

	std::vector<std::string> rows;
	rows.push_back(truncate_visible(s.pane_name, width));
	std::vector<std::string> desc = wrap_text(s.pane_desc, textWidth);
	const std::vector<std::string>& contents = s.pane_contents;
	/* con oggetti o creature la descrizione lascia loro almeno meta' delle
	 * righe (o quelle che servono, se sono meno); se e' corta, i contenuti
	 * usano anche il resto */
	std::size_t descMax = bodyRows;
	if(!contents.empty()) {
		const std::size_t half = bodyRows - bodyRows / 2;
		descMax = bodyRows - std::min(contents.size() + 1, half);
	}
	if(desc.size() > descMax) {
		desc.resize(descMax);
		if(!desc.empty()) {
			std::string& last = desc.back();
			last = truncate_visible(last, textWidth > 3 ? textWidth - 3 : 0) + "$c0007...";
		}
	}
	for(const std::string& line : desc) {
		rows.push_back(line);
	}
	if(!contents.empty()) {
		std::size_t room = bodyRows - desc.size();
		/* riga vuota di stacco se avanza spazio */
		if(!desc.empty() && room > contents.size()) {
			rows.push_back("");
			--room;
		}
		if(contents.size() <= room) {
			for(const std::string& line : contents) {
				rows.push_back(truncate_visible(line, textWidth));
			}
		}
		else if(room > 0) {
			const std::size_t shown = room - 1;
			for(std::size_t i = 0; i < shown; ++i) {
				rows.push_back(truncate_visible(contents[i], textWidth));
			}
			const std::size_t more = contents.size() - shown;
			rows.push_back("$c0007... " + std::string(shown > 0 ? "e altri " : "") +
						   std::to_string(more) + (shown > 0 ? "" : " fra oggetti e creature"));
		}
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
	/* look senza testo nello scroll (display 2: stanza tutta nel pannello)
	 * non accoda output, quindi la pulizia non e' partita: si fa qui */
	if(d->screen.clear_pending) {
		const std::string clr = vt_take_output_prefix(d);
		if(!clr.empty()) {
			write_to_descriptor(d->descriptor, clr.c_str());
		}
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
	else if(d->str == nullptr) {
		/* stessa stanza: ci si addormenta, si diventa ciechi, si spegne la
		 * luce -> pannello da rifare; altrimenti si ricontrollano solo
		 * oggetti e creature (mob che entrano, escono, muoiono, oggetti
		 * lasciati o presi, chi combatte). Le uscite no: per elfi e nani
		 * le porte segrete si vedono a caso, cambierebbero a ogni prompt. */
		if(pane_view_state(ch, real_roomp(ch->in_room)) != s.pane_view) {
			vt_pane_refresh(ch);
		}
		else if(s.pane_can_see) {
			std::vector<std::string> now;
			if(pane_capture_contents(ch, &now) && now != s.pane_contents) {
				s.pane_contents.swap(now);
				s.pane_dirty = true;
			}
		}
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
	write_to_descriptor(d->descriptor, vt_csi_margins(1, vt_scroll_bottom(ch)).c_str());
	d->screen.pane_valid = false; /* all'uscita InitScreen lo ridisegna */
}

/* ---------------------------------------------------------------------
 * Stato del combattimento sull'ultima riga dello schermo (display 1 e 2):
 * nemico e tank con le condizioni colorate come %C/%c del prompt.
 */
static const char* fight_condition(struct char_data* v) {
	const int maxhp = GET_MAX_HIT(v) > 0 ? GET_MAX_HIT(v) : 1;
	const int i = (100 * GET_HIT(v)) / maxhp;
	const bool f = GET_SEX(v) == SEX_FEMALE;
	if(i >= 100) {
		return "$c0010eccellente$c0007";
	}
	if(i >= 80) {
		return f ? "$c0002graffiata$c0007" : "$c0002graffiato$c0007";
	}
	if(i >= 60) {
		return f ? "$c0011tagliata$c0007" : "$c0011tagliato$c0007";
	}
	if(i >= 40) {
		return f ? "$c0013ferita$c0007" : "$c0013ferito$c0007";
	}
	if(i >= 20) {
		return "$c0009sanguinante$c0007";
	}
	if(i >= 0) {
		return f ? "$c0001squarciata$c0007" : "$c0001squarciato$c0007";
	}
	return "$c0008morente$c0007";
}

/** Nome senza colori: short_descr per i mob, nome per i PG. */
static std::string fight_name(struct char_data* v) {
	const char* n = IS_NPC(v) ? v->player.short_descr : GET_NAME(v);
	return std::string(ParseAnsiColors(FALSE, n != nullptr ? n : "?"));
}

/** Barra della vita: 10 tacche del colore della condizione, il resto grigio. */
static std::string fight_hp_bar(struct char_data* v, const std::string& cond) {
	constexpr int kSlots = 10;
	const int maxhp = GET_MAX_HIT(v) > 0 ? GET_MAX_HIT(v) : 1;
	const int hp = GET_HIT(v) > 0 ? GET_HIT(v) : 0;
	int full = static_cast<int>((static_cast<long>(hp) * kSlots + maxhp - 1) / maxhp);
	full = std::min(full, kSlots);
	std::string out = "$c0007[" + cond.substr(0, 6);
	out.append(static_cast<std::size_t>(full), '#');
	out += "$c0007"; /* grigio normale: il grigio scuro su nero non si vede */
	out.append(static_cast<std::size_t>(kSlots - full), '-');
	out += "$c0007]";
	return out;
}

/** "nome [#####-----] condizione" */
static std::string fight_part(struct char_data* v, std::size_t name_max) {
	const std::string cond = fight_condition(v);
	std::string name = fight_name(v);
	if(name.size() > name_max) {
		name = name.substr(0, name_max);
	}
	return name + " " + fight_hp_bar(v, cond) + " " + cond;
}

/** Nemico mostrato: il proprio o, come il prompt, quello di chi si supporta. */
static struct char_data* fight_enemy(struct char_data* ch) {
	struct char_data* enemy = ch->specials.fighting;
	if(enemy == nullptr && ch->specials.supporting != nullptr) {
		struct char_data* const sup = get_char_room_vis(ch, ch->specials.supporting);
		if(sup != nullptr) {
			enemy = sup->specials.fighting;
		}
	}
	return enemy;
}

std::string vt_fight_text(struct char_data* ch) {
	static const std::string kIdle = "$c0015Nemico:$c0007 -";
	if(ch == nullptr) {
		return kIdle;
	}
	struct char_data* const enemy = fight_enemy(ch);
	if(enemy == nullptr) {
		return kIdle;
	}
	const std::size_t width = static_cast<std::size_t>(screen_cols(ch) - 1);
	/* "Nemico: " 8 + nome + " [##########] " 14 + condizione (max 11) */
	constexpr std::size_t kFixed = 8 + 14 + 11;
	const std::size_t name_max = width > kFixed + 4 ? width - kFixed : 4;
	std::string out = "$c0015Nemico:$c0007 " + fight_part(enemy, name_max);
	struct char_data* const tank = enemy->specials.fighting;
	if(tank != nullptr && tank != ch) {
		const std::string tpart = "   $c0015Tank:$c0007 " + fight_part(tank, 24);
		if(visible_len(out) + visible_len(tpart) <= width) {
			out += tpart;
		}
	}
	return out;
}

/* ---------------------------------------------------------------------
 * Statusbar configurabile (comando statusbar / hud): fino a
 * VT_STATUSBAR_LINES righe in fondo allo schermo, ognuna da un modello con
 * codici tipo prompt. specials.statusbar[i]: nullptr = riga predefinita,
 * "-" = riga nascosta, altrimenti il modello del giocatore.
 */
constexpr const char* kStatusbarHidden = "-"; /* C-string: va in specials.statusbar */

/** Modello salvato che nasconde la riga (vuoto o "-"). */
static bool statusbar_hidden(std::string_view t) {
	return t.empty() || t == kStatusbarHidden;
}

/* lunghezza massima di un modello (salvato nelle preferenze del personaggio) */
static constexpr std::size_t kStatusbarMaxLen = 200;

/** Di chi e' la configurazione: chi e' switchato usa la propria. */
static const struct char_data* statusbar_owner(const struct char_data* ch) {
	if(ch != nullptr && ch->desc != nullptr && ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}

/** Riga predefinita: etichette lunghe da 80 colonne, corte se piu' stretta. */
static const char* statusbar_default(const struct char_data* ch, int idx) {
	static constexpr std::array<const char*, VT_STATUSBAR_LINES> kWide{
		"$c0015Hit Points:$c0007 %h(%H)%|$c0015Movement Points:$c0007 %v(%V)",
		"$c0015Mana:$c0007 %m(%M)%|$c0015Gold:$c0007 %g",
		"$c0015Experience Points:$c0007 %X",
		"%F",
	};
	static constexpr std::array<const char*, VT_STATUSBAR_LINES> kNarrow{
		"$c0015PF:$c0007 %h(%H)%|$c0015Mov:$c0007 %v(%V)",
		"$c0015Mana:$c0007 %m(%M)%|$c0015Oro:$c0007 %g",
		"$c0015Exp:$c0007 %X",
		"%F",
	};
	if(idx < 0 || idx >= VT_STATUSBAR_LINES) {
		return "";
	}
	const std::size_t i = static_cast<std::size_t>(idx);
	return screen_cols(ch) >= 80 ? kWide[i] : kNarrow[i];
}

/** Modello in uso per la riga idx, nullptr se la riga e' nascosta. */
static const char* statusbar_template(const struct char_data* ch, int idx) {
	const struct char_data* const owner = statusbar_owner(ch);
	const char* const t = owner != nullptr ? owner->specials.statusbar[idx] : nullptr;
	if(t == nullptr) {
		return statusbar_default(ch, idx);
	}
	if(statusbar_hidden(t)) {
		return nullptr;
	}
	return t;
}

/** Esperienza che manca al prossimo livello (come %x del prompt). */
static long statusbar_exp_to_level(struct char_data* ch) {
	struct char_data* race_ch = nullptr;
	if(IS_SET(ch->specials.act, ACT_POLYSELF) && ch->desc != nullptr) {
		/* i polimorfati usano i massimi razziali del personaggio originale */
		race_ch = ch->desc->original;
	}
	if(race_ch == nullptr) {
		race_ch = ch;
	}
	constexpr long kNone = 999999999L;
	long best = kNone;
	int bit = 1;
	for(int i = 0; i <= PSI_LEVEL_IND; ++i, bit <<= 1) {
		if(!HasClass(ch, bit)) {
			continue;
		}
		const int lev = GET_LEVEL(ch, i);
		if(lev < RacialMax[GET_RACE(race_ch)][i]) {
			const long need = static_cast<long>(titles[i][lev + 1].exp) - static_cast<long>(GET_EXP(ch));
			best = std::min(best, need);
		}
	}
	if(best == kNone) {
		best = static_cast<long>(PRINCEEXP) - static_cast<long>(GET_EXP(ch));
	}
	return best;
}

/** Incantesimi attivi come %S/%s del prompt: lettera maiuscola se manca
 *  piu' di un'ora, minuscola se sta finendo; parentesi scure col globo di
 *  oscurita'. dashes (%a, come %s del prompt): '-' al posto di quelli
 *  assenti. %A senza niente attivo: '-'. */
static std::string statusbar_affects(struct char_data* ch, bool dashes) {
	struct AffSpec {
		int spell;
		const char* strong;
		const char* weak;
	};
	static constexpr std::array<AffSpec, 11> kAffs{{
		{SPELL_POLY_SELF, "$c0013P$c0007", "$c0005p$c0007"},
		{SPELL_CHANGE_FORM, "$c0010P$c0007", "$c0002p$c0007"},
		{SPELL_TREE, "$c0011P$c0007", "$c0003p$c0007"},
		{SPELL_FIRESHIELD, "$c0009F$c0007", "$c0001f$c0007"},
		{SPELL_SANCTUARY, "$c0015S$c0007", "s"},
		{SPELL_INVISIBLE, "$c0011I$c0007", "$c0003i$c0007"},
		{SPELL_TRUE_SIGHT, "$c0014T$c0007", "$c0006t$c0007"},
		{SPELL_MIRROR_IMAGES, "$c0013M$c0007", "$c0005m$c0007"},
		{SPELL_PROT_ENERGY_DRAIN, "D", "d"},
		{SPELL_ANTI_MAGIC_SHELL, "$c0012A$c0007", "$c0004a$c0007"},
		{STATUS_QUEST, "$c0010Q$c0007", "$c0002q$c0007"},
	}};
	std::string inner;
	bool any = false;
	for(const AffSpec& a : kAffs) {
		const int left = _affected_by_s(ch, a.spell);
		if(left != -1) {
			inner += left > 1 ? a.strong : a.weak;
			any = true;
		}
		else if(dashes) {
			inner += '-';
		}
	}
	const int dark = _affected_by_s(ch, SPELL_GLOBE_DARKNESS);
	if(!any && !dashes && dark == -1) {
		return "-";
	}
	if(dark == -1) {
		return "[" + inner + "]";
	}
	const char* const br_open = dark > 1 ? "$c0008[$c0007" : "$c5008[$c0007";
	const char* const br_close = dark > 1 ? "$c0008]$c0007" : "$c5008]$c0007";
	return br_open + inner + br_close;
}

/** Codici solo per gli dei (IS_DIO come %R e %iI/%iS/%iN del prompt):
 *  %R stanza, %I livello di invisibilita', %Z stealth, %P chi stai
 *  snoopando. Per gli altri '-'. */
static std::string statusbar_immortal_code(struct char_data* ch, char code) {
	struct char_data* const real = (ch->desc != nullptr && ch->desc->original != nullptr)
								   ? ch->desc->original : ch;
	if(!IS_DIO(real)) {
		return "-";
	}
	switch(code) {
	case 'R': {
		struct room_data* const rp = real_roomp(ch->in_room);
		return rp != nullptr ? std::to_string(rp->number) : std::string("-");
	}
	case 'I':
		return std::to_string(real->invis_level);
	case 'Z':
		return IS_SET(real->specials.act, PLR_STEALTH) ? "On" : "Off";
	case 'P':
		if(ch->desc != nullptr && ch->desc->snoop.snooping != nullptr) {
			return fight_name(ch->desc->snoop.snooping);
		}
		return "-";
	default:
		return "-";
	}
}

/** Riga della statusbar dal modello (codici colore $c non ancora espansi). */
static std::string statusbar_render(struct char_data* ch, const char* tmpl) {
	struct char_data* const enemy = fight_enemy(ch);
	struct char_data* const tank = enemy != nullptr ? enemy->specials.fighting : nullptr;
	struct char_data* const sup = ch->specials.supporting != nullptr
								  ? get_char_room_vis(ch, ch->specials.supporting) : nullptr;
	const int W = screen_cols(ch);
	/* seconda colonna: la 40 come la barra storica, a meta' se piu' stretta */
	const std::size_t col2 = W >= 80 ? 40 : static_cast<std::size_t>(W / 2 + 1);
	static const std::string kNone = "-";
	std::string out;
	for(const char* p = tmpl; *p != '\0'; ++p) {
		if(*p != '%') {
			out += *p;
			continue;
		}
		const char c = p[1];
		if(c == '\0') {
			out += '%';
			break;
		}
		++p;
		switch(c) {
		case '%':
			out += '%';
			break;
		case 'h':
			out += std::to_string(GET_HIT(ch));
			break;
		case 'H':
			out += std::to_string(GET_MAX_HIT(ch));
			break;
		case 'm':
			out += std::to_string(GET_MANA(ch));
			break;
		case 'M':
			out += std::to_string(GET_MAX_MANA(ch));
			break;
		case 'v':
			out += std::to_string(GET_MOVE(ch));
			break;
		case 'V':
			out += std::to_string(GET_MAX_MOVE(ch));
			break;
		case 'g':
			out += format_thousands(GET_GOLD(ch));
			break;
		case 'G':
			out += format_thousands(GET_BANK(ch));
			break;
		case 'X':
			out += format_thousands(GET_EXP(ch));
			break;
		case 'x':
			out += format_thousands(statusbar_exp_to_level(ch));
			break;
		case 'n':
			out += fight_name(ch);
			break;
		case 'N':
			out += enemy != nullptr ? fight_name(enemy) : kNone;
			break;
		case 'C':
			out += enemy != nullptr ? std::string(fight_condition(enemy)) : kNone;
			break;
		case 'B':
			out += enemy != nullptr ? fight_hp_bar(enemy, fight_condition(enemy)) : kNone;
			break;
		case 't':
			out += tank != nullptr ? fight_name(tank) : kNone;
			break;
		case 'c':
			out += tank != nullptr ? std::string(fight_condition(tank)) : kNone;
			break;
		case 'b':
			out += tank != nullptr ? fight_hp_bar(tank, fight_condition(tank)) : kNone;
			break;
		case 's':
			if(sup != nullptr) {
				out += fight_name(sup);
			}
			else if(ch->specials.supporting != nullptr) {
				out += ch->specials.supporting; /* non e' qui */
			}
			else {
				out += kNone;
			}
			break;
		case 'S':
			out += sup != nullptr ? fight_hp_bar(sup, fight_condition(sup)) : kNone;
			break;
		case 'F':
			out += vt_fight_text(ch);
			break;
		case 'R':
		case 'I':
		case 'Z':
		case 'P':
			out += statusbar_immortal_code(ch, c);
			break;
		case 'A':
			out += statusbar_affects(ch, false);
			break;
		case 'a':
			out += statusbar_affects(ch, true);
			break;
		case '|': {
			const std::size_t vl = visible_len(out);
			if(vl + 1 < col2) {
				out.append(col2 - 1 - vl, ' ');
			}
			else {
				out += ' ';
			}
			break;
		}
		default:
			out += '%';
			out += c;
			break;
		}
	}
	return out;
}

int vt_status_rows(const struct char_data* ch) {
	int n = 0;
	for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
		if(statusbar_template(ch, i) != nullptr) {
			++n;
		}
	}
	return n;
}

std::string vt_status_sequence(struct char_data* ch) {
	if(ch == nullptr || ch->desc == nullptr) {
		return "";
	}
	vt_screen_state& s = ch->desc->screen;
	std::vector<std::string> lines;
	for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
		const char* const t = statusbar_template(ch, i);
		if(t != nullptr) {
			lines.push_back(statusbar_render(ch, t));
		}
	}
	/* l'ultima colonna resta libera: il terminale non va a capo */
	const std::size_t width = static_cast<std::size_t>(screen_cols(ch) - 1);
	const int first = vt_separator_row(ch) + 1;
	const bool full = s.status_drawn.size() != lines.size();
	if(full) {
		s.status_drawn.assign(lines.size(), std::string());
	}
	std::string out;
	for(std::size_t i = 0; i < lines.size(); ++i) {
		const std::string text = truncate_visible(lines[i], width);
		if(!full && text == s.status_drawn[i]) {
			continue;
		}
		s.status_drawn[i] = text;
		out += cursor_pos(first + static_cast<int>(i), 1);
		out += "\033[2K";
		out += colorize(ch, text);
	}
	if(out.empty()) {
		return out;
	}
	return VT_CURSAVE + out + VT_CURREST;
}

void vt_status_update(struct descriptor_data* d) {
	if(d == nullptr || d->connected != CON_PLYNG || d->character == nullptr) {
		return;
	}
	struct char_data* const ch = d->character;
	if(ch->term != VT100 || (IS_NPC(ch) && d->original == nullptr)) {
		return;
	}
	const std::string out = vt_status_sequence(ch);
	if(!out.empty()) {
		write_to_descriptor(d->descriptor, out.c_str());
	}
}

void vt_statusbar_set(struct char_data* ch, int idx, const char* tmpl) {
	if(ch == nullptr || idx < 0 || idx >= VT_STATUSBAR_LINES) {
		return;
	}
	char*& slot = ch->specials.statusbar[idx];
	if(slot != nullptr) {
		free(slot);
		slot = nullptr;
	}
	if(tmpl != nullptr) {
		slot = strdup(tmpl);
	}
}

/** '$' scritti come \$: send_to_char mostra il modello senza colorarlo. */
static std::string statusbar_escape(const char* t) {
	std::string out;
	for(const char* p = t; *p != '\0'; ++p) {
		if(*p == '$') {
			out += '\\';
		}
		out += *p;
	}
	return out;
}

static void statusbar_show(struct char_data* ch) {
	std::string out = "$c0015Statusbar$c0007 (in fondo allo schermo in display 1 e 2):\n\r";
	for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
		const char* const t = ch->specials.statusbar[i];
		out += "  " + std::to_string(i + 1) + ": ";
		if(t == nullptr) {
			out += statusbar_escape(statusbar_default(ch, i));
			out += "  $c0011(predefinita)$c0007";
		}
		else if(statusbar_hidden(t)) {
			out += "$c0011(nascosta)$c0007";
		}
		else {
			out += statusbar_escape(t);
		}
		out += "\n\r";
	}
	out +=
		"\n\r$c0015Codici$c0007:\n\r"
		"  %h %H  punti vita e massimo     %m %M  mana e massimo\n\r"
		"  %v %V  movimento e massimo      %g %G  oro addosso e in banca\n\r"
		"  %X     esperienza               %x     esperienza al prossimo livello\n\r"
		"  %n     il tuo nome\n\r"
		"  %N %C %B  nemico: nome, condizione, barra della vita\n\r"
		"  %t %c %b  tank (chi il nemico sta colpendo): nome, condizione, barra\n\r"
		"  %s %S     chi supporti: nome, barra della vita\n\r"
		"  %F     combattimento completo (nemico e tank, la riga predefinita)\n\r"
		"  %A     incantesimi attivi su di te, come %S del prompt (es. [FSI])\n\r"
		"  %a     come %A ma con '-' per quelli assenti (come %s del prompt)\n\r"
		"  %|     salta alla seconda colonna   %%  il simbolo %\n\r"
		"  Colori con \\$cNNNN come nel prompt. Fuori combattimento i codici del\n\r"
		"  nemico, del tank e del supporto mostrano '-'.\n\r";
	if(IS_DIO(ch)) {
		out += "  Dei: %R stanza, %I invisibilita', %Z stealth, %P chi snoopi\n\r";
	}
	out +=
		"\n\r$c0015Uso$c0007: statusbar <1-4> <modello> | statusbar off <1-4> | "
		"statusbar default [1-4]\n\r"
		"     statusbar preset (elenco) | statusbar preset <numero|nome>\n\r"
		"Esempio: statusbar 3 \\$c0015Exp:\\$c0007 %X%|\\$c0015Al livello:\\$c0007 %x\n\r";
	if(ch->term != VT100) {
		out += "Nota: la statusbar si vede solo in VT100 (display 1 o display 2).\n\r";
	}
	/* paginato: in VT100 la regione che scorre puo' essere bassa */
	if(ch->desc != nullptr) {
		page_string(ch->desc, out.c_str(), true);
	}
}

/** "1".."4" -> 0..3, -1 se non e' un numero di riga. */
static int statusbar_line_arg(const std::string& w) {
	if(w.size() != 1 || w[0] < '1' || w[0] > '0' + VT_STATUSBAR_LINES) {
		return -1;
	}
	return w[0] - '1';
}

/** Ridisegna lo schermo con la nuova statusbar, o avvisa se non e' VT100. */
static void statusbar_apply(struct char_data* ch, const char* msg) {
	send_to_char(msg, ch);
	if(ch->term == VT100 && ch->desc != nullptr) {
		InitScreen(ch); /* il numero di righe puo' cambiare: regione di scroll nuova */
	}
	else {
		send_to_char("La statusbar si vede solo in VT100 (display 1 o display 2): "
					 "la configurazione e' salvata.\n\r", ch);
	}
}

/* ---------------------------------------------------------------------
 * Preset della statusbar (statusbar preset): righe pronte da caricare.
 * lines[i] vuota -> riga nascosta; use_default -> le righe
 * predefinite (seguono la larghezza: etichette corte sotto 80 colonne).
 */
struct StatusbarPreset {
	int n;
	std::string_view name;
	bool immortal;      /* solo per IS_DIO (come il prompt 40/41), non elencato agli altri */
	bool use_default;
	std::array<std::string_view, VT_STATUSBAR_LINES> lines; /* vuota = riga nascosta */
};

constexpr std::string_view kSbAffects = "$c0015Incantesimi:$c0007 %A";
constexpr std::string_view kSbCompact =
	"$c0015PF$c0007 %h/%H  $c0015Mana$c0007 %m/%M  $c0015Mov$c0007 %v/%V";
/* compatto: PF/Mana/Mov e incantesimi sulla stessa riga */
constexpr std::string_view kSbCompactAffects =
	"$c0015PF$c0007 %h/%H  $c0015Mana$c0007 %m/%M  $c0015Mov$c0007 %v/%V  %A";

constexpr std::array<StatusbarPreset, 6> kStatusbarPresets{{
	{1, "classico", false, true, {{}}},
	{2, "compatto", false, false, {{kSbCompactAffects, "%F", {}, {}}}},
	{3, "combattimento", false, false, {{
			kSbCompact,
			"$c0015Nemico:$c0007 %N %B %C",
			"$c0015Tank:$c0007 %t %b %c",
			kSbAffects
		}}
	},
	{4, "healer", false, false, {{
			kSbCompact,
			"$c0015Supporto:$c0007 %s %S",
			"$c0015Tank:$c0007 %t %b %c%|$c0015Nemico:$c0007 %N %C",
			kSbAffects
		}}
	},
	{5, "minimo", false, false, {{
			"$c0015PF$c0007 %h/%H  $c0015Mana$c0007 %m/%M  %A  %N %B",
			{}, {}, {}
		}}
	},
	{99, "immortale", true, false, {{
			"$c0015Stanza:$c0007 %R  $c0015Invis:$c0007 %I  $c0015Stealth:$c0007 %Z  "
			"$c0015Snoop:$c0007 %P",
			kSbCompact,
			"%F",
			kSbAffects
		}}
	},
}};

static void statusbar_preset_list(struct char_data* ch) {
	const bool imm = IS_DIO(ch);
	std::string out = "$c0015Preset della statusbar$c0007 (statusbar preset <numero|nome>):\n\r";
	for(const StatusbarPreset& p : kStatusbarPresets) {
		if(p.immortal && !imm) {
			continue;
		}
		out += "\n\r$c0011" + std::to_string(p.n) + " ";
		out += p.name;
		out += "$c0007";
		if(p.use_default) {
			out += "  (le righe predefinite)";
		}
		out += "\n\r";
		for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
			const std::string_view t = p.use_default
									   ? std::string_view(statusbar_default(ch, i))
									   : p.lines[static_cast<std::size_t>(i)];
			if(t.empty()) {
				continue;
			}
			out += "    ";
			out += t;
			out += "$c0007\n\r";
		}
	}
	if(ch->desc != nullptr) {
		page_string(ch->desc, out.c_str(), true);
	}
}

static void statusbar_preset_load(struct char_data* ch, const std::string& w) {
	const bool imm = IS_DIO(ch);
	const bool numeric = !w.empty() && w.size() <= 3 &&
						 std::all_of(w.begin(), w.end(), [](char c) {
							 return std::isdigit(static_cast<unsigned char>(c)) != 0;
						 });
	int num = -1;
	if(numeric) {
		/* al massimo 3 cifre: niente overflow */
		if(std::from_chars(w.data(), w.data() + w.size(), num).ec != std::errc()) {
			num = -1;
		}
	}
	const StatusbarPreset* found = nullptr;
	for(const StatusbarPreset& p : kStatusbarPresets) {
		if(numeric) {
			if(p.n == num) {
				found = &p;
				break;
			}
		}
		else if((!p.immortal || imm) && !w.empty() && str_istarts_with(p.name, w)) {
			found = &p;
			break;
		}
	}
	if(found == nullptr) {
		send_to_char("Preset inesistente. Scrivi statusbar preset per l'elenco.\n\r", ch);
		return;
	}
	if(found->immortal && !imm) {
		send_to_char("Eh?\r\n", ch); /* come prompt 40/41 */
		return;
	}
	for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
		if(found->use_default) {
			vt_statusbar_set(ch, i, nullptr);
		}
		else {
			const std::string line(found->lines[static_cast<std::size_t>(i)]);
			vt_statusbar_set(ch, i, line.empty() ? kStatusbarHidden : line.c_str());
		}
	}
	statusbar_apply(ch, ("Preset " + std::to_string(found->n) + " (" + std::string(found->name) +
						 ") caricato.\n\r").c_str());
}

ACTION_FUNC(do_statusbar) {
	if(ch == nullptr || IS_NPC(ch)) {
		return;
	}
	std::string args = arg != nullptr ? arg : "";
	const std::size_t b = args.find_first_not_of(" \t");
	args = (b == std::string::npos) ? std::string() : args.substr(b);
	const std::size_t sp = args.find_first_of(" \t");
	const std::string word = args.substr(0, sp);
	std::string rest;
	if(sp != std::string::npos) {
		const std::size_t r = args.find_first_not_of(" \t", sp);
		if(r != std::string::npos) {
			rest = args.substr(r);
		}
	}
	while(!rest.empty() && (rest.back() == ' ' || rest.back() == '\t' ||
							rest.back() == '\r' || rest.back() == '\n')) {
		rest.pop_back();
	}

	if(word.empty()) {
		statusbar_show(ch);
		return;
	}
	if(str_iequals(word, "default")) {
		if(rest.empty()) {
			for(int i = 0; i < VT_STATUSBAR_LINES; ++i) {
				vt_statusbar_set(ch, i, nullptr);
			}
			statusbar_apply(ch, "Statusbar predefinita ripristinata.\n\r");
			return;
		}
		const int idx = statusbar_line_arg(rest);
		if(idx < 0) {
			send_to_char("Uso: statusbar default [1-4]\n\r", ch);
			return;
		}
		vt_statusbar_set(ch, idx, nullptr);
		statusbar_apply(ch, ("Riga " + std::to_string(idx + 1) +
							 " della statusbar riportata a quella predefinita.\n\r").c_str());
		return;
	}
	if(str_iequals(word, "preset")) {
		if(rest.empty()) {
			statusbar_preset_list(ch);
		}
		else {
			statusbar_preset_load(ch, rest);
		}
		return;
	}
	if(str_iequals(word, "off")) {
		const int idx = statusbar_line_arg(rest);
		if(idx < 0) {
			send_to_char("Uso: statusbar off <1-4>\n\r", ch);
			return;
		}
		vt_statusbar_set(ch, idx, kStatusbarHidden);
		statusbar_apply(ch, ("Riga " + std::to_string(idx + 1) +
							 " della statusbar nascosta.\n\r").c_str());
		return;
	}
	const int idx = statusbar_line_arg(word);
	if(idx < 0) {
		send_to_char("Uso: statusbar <1-4> <modello> | statusbar off <1-4> | "
					 "statusbar default [1-4] | statusbar preset [numero|nome]. "
					 "Scrivi statusbar per i codici.\n\r", ch);
		return;
	}
	if(rest.empty()) {
		send_to_char("Scrivi anche il modello della riga, es. statusbar 1 PF %h(%H). "
					 "Per nasconderla: statusbar off <1-4>.\n\r", ch);
		return;
	}
	if(rest.size() > kStatusbarMaxLen) {
		send_to_char(("Modello troppo lungo (massimo " + std::to_string(kStatusbarMaxLen) +
					  " caratteri).\n\r").c_str(), ch);
		return;
	}
	vt_statusbar_set(ch, idx, rest.c_str());
	statusbar_apply(ch, ("Riga " + std::to_string(idx + 1) + " della statusbar impostata.\n\r").c_str());
}

} // namespace Alarmud

