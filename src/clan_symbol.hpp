/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_CLAN_SYMBOL_HPP_
#define SRC_CLAN_SYMBOL_HPP_

#include "config.hpp"
#include "flags.hpp"
#include "typedefs.hpp"

#include <string>
#include <vector>

namespace Alarmud {

struct char_data;
struct obj_data;

#if USE_MYSQL
/**
 * Boot: ensure tabella clan_symbol, seed, resolve prince_toon_id,
 * materializza template in object_instance (base fuori 34k), relink inventori,
 * archivia objects/<vnum> 34k. Idempotente.
 */
void clan_symbol_boot_migrate();

/**
 * true se il vnum e' un simbolo clan registrato (seed storico e/o riga
 * attiva in MySQL clan_symbol - es. creato con clan registra).
 */
bool clan_symbol_is_listed_vnum(unsigned vnum);

/** true se ch puo' indossare questo simbolo (principe / vassallo / imm). */
bool clan_symbol_can_wear(struct char_data* ch, const struct obj_data* obj);

/** true se obj e' un simbolo del clan (type o wear flag). */
bool clan_symbol_is_obj(const struct obj_data* obj);

/**
 * Ricarica gli affect dell'istanza MySQL sull'oggetto (anche se indossato).
 * Evita affect "fantasma" dopo morte/save con affected[] corrotti in RAM.
 */
void clan_symbol_refresh_affects_from_instance(struct obj_data* obj);

/** true se ch ha gia' un qualunque simbolo del clan (eq / inv / 1 livello contenitori). */
bool clan_symbol_char_holds_any(struct char_data* ch);

/**
 * true se ch puo' ricevere obj. Se obj e' un simbolo e ch ne ha gia' un altro,
 * false (e messaggio se !silent). Immortali ok. obj gia' in possesso di ch ok.
 */
bool clan_symbol_can_receive(struct char_data* ch, const struct obj_data* obj,
							 bool silent);

/**
 * Al login: se ch ha piu' di un simbolo, ne lascia uno (preferisce quello
 * indossato) e ritira/distrugge gli altri. Poi prova a indossare quello tenuto.
 */
void clan_symbol_enforce_single(struct char_data* ch);

/** Se obj e' un simbolo in inventario e lo slot e' libero, lo indossa. */
void clan_symbol_try_auto_wear(struct char_data* ch, struct obj_data* obj);

/**
 * Rimuove e distrugge (o scollega dal template condiviso) i simboli del clan
 * del principe nominato dall'inventario/eq di ch. Slot liberato per riassegnare.
 */
void clan_symbol_strip_from_char(struct char_data* ch, const char* prince_name);

/**
 * Cassetta del clan: clan di ch (nome del principe) e se ch ne e' il capo.
 * false se ch non fa parte di nessun clan. Poly: usa il PC reale.
 */
bool clan_vault_identity(struct char_data* ch, std::string& prince_name, bool& is_leader);

/** Nomi dei vassalli di un principe (online, MySQL, .aux). Da leggere PRIMA
 *  di cancellare il toon del principe. */
[[nodiscard]] std::vector<std::string> clan_vassal_names(const std::string& prince_name);

/**
 * Il principe si e' cancellato: lettera dello Scriba (posta di gioco) a ogni
 * vassallo, anche offline. vault_exists/vault_items descrivono la cassetta del
 * clan al momento della cancellazione (persa con il toon). No-op senza vassalli.
 */
void clan_mail_prince_left(const std::string& prince_name,
						   const std::vector<std::string>& vassals,
						   bool vault_exists, long long vault_items);
#else
inline void clan_symbol_boot_migrate() {}
inline bool clan_symbol_is_listed_vnum(unsigned) {
	return false;
}
inline bool clan_symbol_can_wear(struct char_data*, const struct obj_data*) {
	return true;
}
inline bool clan_symbol_is_obj(const struct obj_data*) {
	return false;
}
inline void clan_symbol_refresh_affects_from_instance(struct obj_data*) {}
inline bool clan_symbol_char_holds_any(struct char_data*) {
	return false;
}
inline bool clan_symbol_can_receive(struct char_data*, const struct obj_data*, bool) {
	return true;
}
inline void clan_symbol_enforce_single(struct char_data*) {}
inline void clan_symbol_try_auto_wear(struct char_data*, struct obj_data*) {}
inline void clan_symbol_strip_from_char(struct char_data*, const char*) {}
inline std::vector<std::string> clan_vassal_names(const std::string&) {
	return {};
}
inline void clan_mail_prince_left(const std::string&, const std::vector<std::string>&, bool,
								  long long) {}
#endif

/** Comando clan: info / vassalli / simboli / assegna / associa / ripudia / quota. */
ACTION_FUNC(do_clan);

} // namespace Alarmud

#endif /* SRC_CLAN_SYMBOL_HPP_ */
