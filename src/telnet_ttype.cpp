/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Riconoscimento del client via telnet TTYPE (RFC 1091) e MTTS
 * (https://tintin.mudhalla.net/protocols/mtts/).
 *
 * Alla connessione il server invia IAC DO TTYPE. Se il client risponde
 * IAC WILL TTYPE gli si chiede IAC SB TTYPE SEND IAC SE fino a 3 volte:
 *   1a risposta: nome del client     (es. "MUDLET", "XTERM-256COLOR")
 *   2a risposta: tipo di terminale   (es. "ANSI-TRUECOLOR")
 *   3a risposta: "MTTS <bitvector>"  (solo client MTTS)
 * Ci si ferma quando una risposta si ripete (client non MTTS) o arriva MTTS.
 * Con il risultato si sceglie il display: client MUD -> lineare (0),
 * terminale vero -> VT100 (1). Senza risposta non si tocca nulla.
 *
 * NAWS (RFC 1073): con IAC DO NAWS il client manda IAC SB NAWS <w16> <h16>
 * IAC SE alla connessione e a ogni resize della finestra. Le dimensioni
 * servono solo in VT100 (barra di stato, regione di scroll): in modo
 * lineare si memorizzano e basta.
 */
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "telnet_ttype.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "vt100c.hpp"

namespace Alarmud {

namespace {

/* Byte telnet (RFC 854/1091). Nomi propri per non includere <arpa/telnet.h>,
 * che definisce macro corte (IP, DM, EL, ...) */
constexpr unsigned char TN_SE = 240;
constexpr unsigned char TN_SB = 250;
constexpr unsigned char TN_WILL = 251;
constexpr unsigned char TN_WONT = 252;
constexpr unsigned char TN_DO = 253;
constexpr unsigned char TN_DONT = 254;
constexpr unsigned char TN_IAC = 255;
constexpr unsigned char TN_OPT_TTYPE = 24;
constexpr unsigned char TN_OPT_NAWS = 31;
constexpr unsigned char TN_TTYPE_IS = 0;
constexpr unsigned char TN_TTYPE_SEND = 1;

/* Stati del parser IAC */
constexpr unsigned char PS_DATA = 0;
constexpr unsigned char PS_IAC = 1;    /* letto IAC */
constexpr unsigned char PS_OPT = 2;    /* letto IAC WILL/WONT/DO/DONT, manca l'opzione */
constexpr unsigned char PS_SB_OPT = 3; /* letto IAC SB, manca l'opzione */
constexpr unsigned char PS_SB = 4;     /* dentro la subnegoziazione */
constexpr unsigned char PS_SB_IAC = 5; /* IAC dentro la subnegoziazione */

constexpr std::size_t SB_MAX = 128;        /* oltre si scarta (client ostile) */
constexpr std::size_t TTYPE_VALUE_MAX = 40;
constexpr int TTYPE_MAX_REQUESTS = 3;

/* Limiti delle dimensioni NAWS usate in VT100 */
constexpr int SCREEN_MIN_COLS = 40;  /* sotto 80 la barra va in formato compatto */
constexpr int SCREEN_MAX_COLS = 250;
constexpr int SCREEN_MIN_ROWS = 20;
constexpr int SCREEN_MAX_ROWS = 100;
constexpr int SCREEN_DEFAULT_COLS = 80;

/* Nomi (sottostringhe, maiuscole) di client MUD noti */
constexpr std::array<std::string_view, 30> kMudClients{
	"MUDLET", "MUSHCLIENT", "TINTIN", "ZMUD", "CMUD", "BLOWTORCH",
	"MUDRAMMER", "AXMUD", "KILDCLIENT", "POWWOW", "ATLANTIS", "GMUD",
	"MUDMASTER", "MUDPORTAL", "PROXIWEB", "BEIP", "POTATO", "LYNTIN",
	"KBTIN", "SIMPLEMU", "JMC", "MM2K", "MUDLETBOT", "FMUD", "GRAPEVINE",
	"DECAFMUD", "WINTIN", "MUDJS", "MUDMAGIC", "PUEBLO",
};

/* Prefissi (maiuscoli) di terminali veri */
constexpr std::array<std::string_view, 24> kTerminals{
	"XTERM", "VT1", "VT2", "VT3", "VT4", "VT5", "DEC-VT", "ANSI", "LINUX",
	"SCREEN", "TMUX", "RXVT", "PUTTY", "KONSOLE", "GNOME", "ALACRITTY",
	"KITTY", "ITERM", "WEZTERM", "FOOT", "CYGWIN", "CONS25", "SUN", "ETERM",
};

std::string to_upper(const std::string& s) {
	std::string u(s);
	for(char& c : u) {
		c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}
	return u;
}

bool contains_mud_client(const std::string& upper) {
	for(const std::string_view name : kMudClients) {
		if(upper.find(name) != std::string::npos) {
			return true;
		}
	}
	return false;
}

bool is_real_terminal(const std::string& upper) {
	for(const std::string_view prefix : kTerminals) {
		if(upper.compare(0, prefix.size(), prefix) == 0) {
			return true;
		}
	}
	return false;
}

void send_raw(struct descriptor_data* d, std::initializer_list<unsigned char> bytes) {
	/* nessuna sequenza usata qui contiene 0: write_to_descriptor usa strlen */
	std::string buf;
	buf.reserve(bytes.size());
	for(const unsigned char b : bytes) {
		buf += static_cast<char>(b);
	}
	write_to_descriptor(d->descriptor, buf.c_str());
}

void send_ttype_request(struct descriptor_data* d) {
	send_raw(d, {TN_IAC, TN_SB, TN_OPT_TTYPE, TN_TTYPE_SEND, TN_IAC, TN_SE});
	d->telnet.ttype_requests++;
}

const char* display_name(int mode) {
	switch(mode) {
	case 0:
		return "lineare";
	case 1:
		return "VT100";
	default:
		return "invariato";
	}
}

void ttype_finish(struct descriptor_data* d) {
	telnet_negotiation& t = d->telnet;
	t.ttype_state = TTYPE_DONE;
	if(!t.ttype_replies.empty()) {
		t.client = t.ttype_replies[0];
		t.terminal = t.ttype_replies.size() > 1 ? t.ttype_replies[1] : t.ttype_replies[0];
		if(t.terminal.compare(0, 5, "MTTS ") == 0) {
			t.terminal = t.client;
		}
	}
	mudlog(LOG_CONNECT, "TTYPE [HOST:%s]: client '%s' terminale '%s' MTTS %d -> display %s",
		   d->host, t.client.c_str(), t.terminal.c_str(), t.mtts,
		   display_name(telnet_detect_display(d)));
	/* di solito si e' ancora al login; se il PG e' gia' in gioco si applica ora */
	if(d->connected == CON_PLYNG) {
		telnet_apply_auto_display(d);
	}
}

void ttype_on_reply(struct descriptor_data* d, const std::string& raw) {
	telnet_negotiation& t = d->telnet;
	if(t.ttype_state != TTYPE_NEGOTIATING) {
		return;
	}
	std::string value;
	for(char c : raw) {
		const unsigned char uc = static_cast<unsigned char>(c);
		if(uc >= 32 && uc < 127 && value.size() < TTYPE_VALUE_MAX) {
			value += c;
		}
	}
	const std::string upper = to_upper(value);
	/* ripetizione: client non MTTS (stessa risposta o lista ricominciata) */
	if(!t.ttype_replies.empty() &&
			(upper == to_upper(t.ttype_replies.back()) ||
			 upper == to_upper(t.ttype_replies.front()))) {
		ttype_finish(d);
		return;
	}
	t.ttype_replies.push_back(value);
	if(upper.compare(0, 5, "MTTS ") == 0) {
		char* end = nullptr;
		const long bits = std::strtol(upper.c_str() + 5, &end, 10);
		if(end != upper.c_str() + 5 && bits >= 0) {
			t.mtts = static_cast<int>(bits);
		}
		ttype_finish(d);
		return;
	}
	if(t.ttype_requests >= TTYPE_MAX_REQUESTS) {
		ttype_finish(d);
		return;
	}
	send_ttype_request(d);
}

void handle_option(struct descriptor_data* d, unsigned char cmd, unsigned char opt) {
	if(opt != TN_OPT_TTYPE) {
		/* altre opzioni (ECHO, SGA, NAWS, GMCP...): ignorate come prima */
		return;
	}
	telnet_negotiation& t = d->telnet;
	if(cmd == TN_WILL) {
		if(t.ttype_state == TTYPE_ASKED) {
			t.ttype_state = TTYPE_NEGOTIATING;
			send_ttype_request(d);
		}
	}
	else if(cmd == TN_WONT) {
		if(t.ttype_state == TTYPE_ASKED || t.ttype_state == TTYPE_NEGOTIATING) {
			if(!t.ttype_replies.empty()) {
				ttype_finish(d);
			}
			else {
				t.ttype_state = TTYPE_REFUSED;
			}
		}
	}
	/* DO/DONT TTYPE non hanno senso lato server: ignorati */
}

void naws_on_size(struct descriptor_data* d, int cols, int rows) {
	telnet_negotiation& t = d->telnet;
	const int old_cols = t.naws_cols;
	const int old_rows = t.naws_rows;
	/* 0 = dimensione ignota per il client: si tiene la precedente */
	if(cols > 0) {
		t.naws_cols = cols;
	}
	if(rows > 0) {
		t.naws_rows = rows;
	}
	/* log solo alla prima dimensione, non a ogni resize */
	if(old_cols <= 0 && old_rows <= 0) {
		mudlog(LOG_CONNECT, "NAWS [HOST:%s]: %dx%d", d->host, cols, rows);
	}
	/* si confrontano i valori ricevuti, non quelli limitati: sotto i
	 * limiti il terminale rimanda comunque a capo le righe vecchie */
	const bool changed = (t.naws_cols != old_cols || t.naws_rows != old_rows);
	struct char_data* ch = d->character;
	if(ch == nullptr || IS_NPC(ch) || ch->term != VT100) {
		return; /* modo lineare o login: solo memorizzato */
	}
	if(d->connected != CON_PLYNG) {
		/* dentro oedit/medit/redit lo schermo e' dell'editor */
		t.naws_pending = true;
		return;
	}
	if(changed) {
		InitScreen(ch); /* nuova regione di scroll, barra in fondo, pannello */
	}
}

void handle_subneg(struct descriptor_data* d) {
	telnet_negotiation& t = d->telnet;
	if(t.sb_opt == TN_OPT_TTYPE && !t.sb_buf.empty() &&
			static_cast<unsigned char>(t.sb_buf[0]) == TN_TTYPE_IS) {
		ttype_on_reply(d, t.sb_buf.substr(1));
	}
	else if(t.sb_opt == TN_OPT_NAWS && t.sb_buf.size() >= 4) {
		/* IAC IAC (byte 255 nelle dimensioni) e' gia' ridotto a 255 dal parser */
		const auto b = [&t](std::size_t i) {
			return static_cast<int>(static_cast<unsigned char>(t.sb_buf[i]));
		};
		naws_on_size(d, (b(0) << 8) | b(1), (b(2) << 8) | b(3));
	}
	t.sb_buf.clear();
}

} // namespace

void telnet_start_negotiation(struct descriptor_data* d) {
	if(d == nullptr) {
		return;
	}
	d->telnet.ttype_state = TTYPE_ASKED;
	send_raw(d, {TN_IAC, TN_DO, TN_OPT_TTYPE, TN_IAC, TN_DO, TN_OPT_NAWS});
}

void telnet_sync_screen_size(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	if(ch->width < SCREEN_MIN_COLS) {
		ch->width = SCREEN_DEFAULT_COLS;
	}
	if(ch->desc == nullptr) {
		return;
	}
	telnet_negotiation& t = ch->desc->telnet;
	t.naws_pending = false;
	if(t.naws_cols > 0) {
		ch->width = std::clamp(t.naws_cols, SCREEN_MIN_COLS, SCREEN_MAX_COLS);
	}
	if(t.naws_rows > 0 && !ch->size_manual) {
		ch->size = std::clamp(t.naws_rows, SCREEN_MIN_ROWS, SCREEN_MAX_ROWS);
	}
}

void telnet_on_enter_playing(struct descriptor_data* d, bool from_editor) {
	if(d == nullptr) {
		return;
	}
	telnet_apply_auto_display(d);
	struct char_data* ch = d->character;
	/* L'editor cancella lo schermo e usa tutte le righe: in uscita (o dopo
	 * un resize arrivato mentre si editava) si rimettono regione di scroll e
	 * barra di stato, con le dimensioni NAWS correnti. */
	/* from_editor/naws_pending come sopra; !initialized: riconnessione di un
	 * PG gia' in VT100 su un terminale nuovo, che non ha barra ne' regione */
	if((from_editor || d->telnet.naws_pending || !d->screen.initialized) && ch != nullptr &&
			!IS_NPC(ch) && ch->term == VT100) {
		InitScreen(ch);
	}
	d->telnet.naws_pending = false;
}

std::string telnet_screen_label(const struct descriptor_data* d) {
	if(d == nullptr || (d->telnet.naws_cols <= 0 && d->telnet.naws_rows <= 0)) {
		return "";
	}
	return std::to_string(d->telnet.naws_cols) + "x" + std::to_string(d->telnet.naws_rows);
}

std::size_t telnet_filter_input(struct descriptor_data* d, const char* in,
								std::size_t len, char* out) {
	telnet_negotiation& t = d->telnet;
	std::size_t n = 0;
	for(std::size_t i = 0; i < len; ++i) {
		const unsigned char c = static_cast<unsigned char>(in[i]);
		switch(t.iac_state) {
		case PS_DATA:
			if(c == TN_IAC) {
				t.iac_state = PS_IAC;
			}
			else if(c != 0) { /* CR NUL dei client telnet: il NUL si scarta */
				out[n++] = static_cast<char>(c);
			}
			break;
		case PS_IAC:
			if(c == TN_WILL || c == TN_WONT || c == TN_DO || c == TN_DONT) {
				t.iac_cmd = c;
				t.iac_state = PS_OPT;
			}
			else if(c == TN_SB) {
				t.iac_state = PS_SB_OPT;
			}
			else {
				/* IAC IAC (byte 255, non ASCII: l'input lo scarterebbe comunque),
				 * GA, NOP, AYT, SE spaiato...: ignorati */
				t.iac_state = PS_DATA;
			}
			break;
		case PS_OPT:
			t.iac_state = PS_DATA;
			handle_option(d, t.iac_cmd, c);
			break;
		case PS_SB_OPT:
			t.sb_opt = c;
			t.sb_buf.clear();
			t.iac_state = PS_SB;
			break;
		case PS_SB:
			if(c == TN_IAC) {
				t.iac_state = PS_SB_IAC;
			}
			else if(t.sb_buf.size() < SB_MAX) {
				t.sb_buf += static_cast<char>(c);
			}
			break;
		case PS_SB_IAC:
			if(c == TN_SE) {
				t.iac_state = PS_DATA;
				handle_subneg(d);
			}
			else if(c == TN_IAC) {
				if(t.sb_buf.size() < SB_MAX) {
					t.sb_buf += static_cast<char>(c);
				}
				t.iac_state = PS_SB;
			}
			else {
				/* subnegoziazione malformata: la si chiude e si torna ai dati */
				t.sb_buf.clear();
				t.iac_state = PS_DATA;
			}
			break;
		default:
			t.iac_state = PS_DATA;
			break;
		}
	}
	return n;
}

int telnet_detect_display(const struct descriptor_data* d) {
	if(d == nullptr) {
		return -1;
	}
	const telnet_negotiation& t = d->telnet;
	if(t.ttype_state != TTYPE_DONE || t.ttype_replies.empty()) {
		return -1;
	}
	/* MTTS lo implementano solo i client MUD (screen reader compresi) */
	if(t.mtts >= 0) {
		return 0;
	}
	for(const std::string& reply : t.ttype_replies) {
		if(contains_mud_client(to_upper(reply))) {
			return 0;
		}
	}
	if(is_real_terminal(to_upper(t.client))) {
		return 1;
	}
	return -1;
}

void telnet_apply_auto_display(struct descriptor_data* d) {
	if(d == nullptr || d->telnet.display_applied) {
		return;
	}
	struct char_data* ch = d->character;
	if(ch == nullptr || IS_NPC(ch)) {
		return;
	}
	if(d->telnet.ttype_state == TTYPE_ASKED || d->telnet.ttype_state == TTYPE_NEGOTIATING) {
		return; /* risposta non ancora arrivata: ci riprova ttype_finish */
	}
	d->telnet.display_applied = true;
	if(ch->term_manual) {
		return; /* scelta esplicita col comando display: si rispetta */
	}
	const int mode = telnet_detect_display(d);
	if(mode == 1 && ch->term != VT100) {
		if(ch->size < 10) {
			ch->size = 25;
		}
		ch->term = VT100;
		ch->term_pane = false;
		InitScreen(ch);
		send_to_char("Terminale rilevato: modo VT100 attivo (display 0 per disattivarlo).\n\r", ch);
	}
	else if(mode == 0 && ch->term == VT100) {
		ch->term = 0;
		ch->term_pane = false;
		ScreenOff(ch);
		if(!ch->size_manual) {
			ch->size = 25; /* l'altezza NAWS vale solo in VT100 */
		}
		send_to_char("Client MUD rilevato: modo VT100 disattivato (display 1 per riattivarlo).\n\r", ch);
	}
}

std::string telnet_client_label(const struct descriptor_data* d) {
	if(d == nullptr) {
		return "?";
	}
	const telnet_negotiation& t = d->telnet;
	switch(t.ttype_state) {
	case TTYPE_ASKED:
		return "nessuna risposta";
	case TTYPE_NEGOTIATING:
		return "in corso";
	case TTYPE_REFUSED:
		return "nessun TTYPE";
	case TTYPE_DONE:
		break;
	default:
		return "?";
	}
	std::string label = t.client.empty() ? "?" : t.client;
	if(!t.terminal.empty() && t.terminal != t.client) {
		label += " / " + t.terminal;
	}
	if(t.mtts >= 0) {
		label += ", MTTS " + std::to_string(t.mtts);
	}
	return label;
}

} // namespace Alarmud
