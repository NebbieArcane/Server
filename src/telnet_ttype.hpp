/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __TELNET_TTYPE_HPP
#define __TELNET_TTYPE_HPP
/***************************  System  include ************************************/
#include <cstddef>
#include <string>
/***************************  Local    include ************************************/
namespace Alarmud {

struct descriptor_data;

/* Stati di telnet_negotiation::ttype_state */
constexpr int TTYPE_NONE = 0;        /* mai chiesto */
constexpr int TTYPE_ASKED = 1;       /* inviato IAC DO TTYPE, attesa WILL/WONT */
constexpr int TTYPE_NEGOTIATING = 2; /* WILL ricevuto, SEND in corso */
constexpr int TTYPE_DONE = 3;        /* client/terminale noti */
constexpr int TTYPE_REFUSED = 4;     /* WONT TTYPE: il client non risponde */
constexpr int TTYPE_NO_REPLY = 5;    /* WILL senza risposta IS all'ingresso in gioco:
                                      * sconosciuto, risposte tardive ignorate */

struct char_data;

/** Nuova connessione: chiede al client il tipo di terminale e le dimensioni
 *  della finestra (IAC DO TTYPE, IAC DO NAWS). */
void telnet_start_negotiation(struct descriptor_data* d);

/** Filtra i comandi telnet (IAC ...) dai byte letti dal socket.
 *  Scrive in out solo i dati utente (max len byte, mai NUL) e ritorna
 *  quanti ne ha scritti. Lo stato del parser resta sul descrittore, per
 *  cui le sequenze spezzate fra due read() vengono ricomposte.
 *  Gestisce TTYPE/MTTS; le altre opzioni vengono ignorate come prima. */
std::size_t telnet_filter_input(struct descriptor_data* d, const char* in,
								std::size_t len, char* out);

/** Display suggerito dal client rilevato: 0 lineare (client MUD),
 *  1 VT100 (terminale vero), -1 se non si sa (nessuna risposta/sconosciuto). */
int telnet_detect_display(const struct descriptor_data* d);

/** Decide il display del PG del descrittore, una volta per PG caricato o
 *  per connessione nuova (term_applied): quello di prima se riconnesso da
 *  un client dello stesso tipo, poi il display salvato sul PG per quel
 *  tipo di client, poi l'ultimo usato sulla connessione, infine quello
 *  rilevato dal client.
 *  Da chiamare quando il PG entra in gioco (e a fine negoziazione se e'
 *  gia' in gioco). */
void telnet_apply_auto_display(struct descriptor_data* d);

/** Tipo di client per il display: 1 terminale vero, 0 client MUD, -1
 *  sconosciuto. *pending vero se la risposta TTYPE e' ancora in corso
 *  (si decide a ttype_finish). */
int telnet_client_type(const struct descriptor_data* d, bool* pending);

/** Posto della preferenza (TERM_PREF_*) per un tipo di client: lo
 *  sconosciuto va con i terminali. */
int telnet_pref_slot(int client_type);

/** Display attuale del PG: 0 lineare, 1 VT100, 2 VT100 con la stanza in alto. */
int telnet_current_display(const struct char_data* ch);

/** Porta il PG al display mode (0/1/2) con ScreenOff/InitScreen, senza
 *  messaggi; non fa niente se e' gia' quello. Non tocca term_manual. */
void telnet_set_display(struct char_data* ch, int mode);

/** Copia le dimensioni NAWS del descrittore su ch->width / ch->size, con i
 *  limiti 40-250 colonne e 20-100 righe. L'altezza non si tocca se il
 *  giocatore l'ha scelta con resize. Senza NAWS restano 80x25 (o resize).
 *  Chiamata da InitScreen: vale solo per il modo VT100. */
void telnet_sync_screen_size(struct char_data* ch);

/** Il descrittore torna/entra in CON_PLYNG: display automatico e, in VT100,
 *  ridisegno di regione di scroll e barra se si esce da oedit/medit/redit
 *  (from_editor) o se un resize NAWS e' arrivato dentro un editor. */
void telnet_on_enter_playing(struct descriptor_data* d, bool from_editor);

/** Dimensioni NAWS "COLSxRIGHE" ricevute dal client, "" se assenti. */
std::string telnet_screen_label(const struct descriptor_data* d);

/** Descrizione breve del client per users/display (es. "MUDLET 4.17 / ANSI-TRUECOLOR, MTTS 2349"). */
std::string telnet_client_label(const struct descriptor_data* d);

} // namespace Alarmud
#endif // __TELNET_TTYPE_HPP
