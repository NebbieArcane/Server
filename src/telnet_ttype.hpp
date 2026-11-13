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

/** Applica il display rilevato al PG del descrittore, una volta per
 *  connessione, se il giocatore non ha scelto a mano col comando display.
 *  Da chiamare quando il PG entra in gioco (e a fine negoziazione se e'
 *  gia' in gioco). */
void telnet_apply_auto_display(struct descriptor_data* d);

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
