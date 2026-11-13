/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __VT_SCREEN_HPP
#define __VT_SCREEN_HPP
/***************************  System  include ************************************/
#include <string>
/***************************  Local    include ************************************/
namespace Alarmud {

struct char_data;
struct descriptor_data;

/*
 * Layout dello schermo VT100 (display 1 e 2), R = ch->size, W = ch->width:
 *   display 1: righe 1..R-5 scorrono, R-4 separatore, R-3..R-1 barra
 *   display 2: righe 1..P pannello stanza (nome, descrizione, uscite),
 *              P+1 separatore, P+2..R-5 scorrono, poi la barra come sopra.
 * Se la finestra e' troppo bassa per il pannello, display 2 si comporta
 * come display 1.
 */

/** Campi della barra di stato */
enum VtBarField { VT_BAR_HP, VT_BAR_MOVE, VT_BAR_MANA, VT_BAR_GOLD, VT_BAR_EXP };

/** Righe del pannello stanza (0 = nessun pannello: display 0/1 o finestra bassa). */
int vt_pane_rows(const struct char_data* ch);
/** true se il PG e' in display 2 e il pannello sta nella finestra. */
bool vt_pane_active(const struct char_data* ch);
/** Prima e ultima riga della regione che scorre. */
int vt_scroll_top(const struct char_data* ch);
int vt_scroll_bottom(const struct char_data* ch);
/** Righe per pagina del paginatore in VT100 (tiene conto del pannello). */
int vt_page_lines(const struct char_data* ch);

/** Separatore "-====-" largo quanto lo schermo meno una colonna. */
std::string vt_separator(const struct char_data* ch);
/** Etichetta e valore di un campo della barra (per InitScreen). */
std::string vt_bar_field_init(const struct char_data* ch, VtBarField field, const std::string& value);
/** Aggiornamento di un campo: salva cursore, pulisce, scrive, ripristina. */
std::string vt_bar_field_update(const struct char_data* ch, VtBarField field, const std::string& value);

/** Ricostruisce il contenuto del pannello dalla stanza attuale e lo
 *  segna da ridisegnare (buio, cecita' e sonno compresi). */
void vt_pane_refresh(struct char_data* ch);
/** Sequenza che disegna pannello e separatore sotto (con salva/ripristina cursore). */
std::string vt_pane_sequence(struct char_data* ch);

/** Chiamata da do_look per il look della stanza: display 1 -> se la
 *  stanza e' cambiata pulisce lo scroll; display 2 -> aggiorna il pannello. */
/* look della stanza: display 1 pulisce lo scroll e la stanza riparte
 * dall'alto, display 2 aggiorna il pannello */
void vt_room_look(struct char_data* ch);
/** true se in display 2 il look non deve ripetere descrizione e uscite
 *  nello scroll (sono gia' nel pannello). */
bool vt_room_text_in_pane(const struct char_data* ch);

/** process_output: testo da mandare PRIMA dell'output accodato (pulizia
 *  dello scroll al cambio stanza in display 1). Vuoto se niente. */
std::string vt_take_output_prefix(struct descriptor_data* d);
/** Prima del prompt: in display 2 ridisegna il pannello se la stanza e'
 *  cambiata (anche senza look: teleport, summon...) o se e' sporco. */
void vt_before_prompt(struct descriptor_data* d);
/** Ingresso in un editor: in display 2 la regione di scroll torna quella
 *  di display 1 (gli editor disegnano dall'alto); all'uscita InitScreen. */
void vt_enter_editor(struct descriptor_data* d);

} // namespace Alarmud
#endif // __VT_SCREEN_HPP
