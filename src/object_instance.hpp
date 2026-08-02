/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* object_instance.hpp - persistenza eq editato in MySQL (no vnum 34k).
 * See docs/object-instance-plan.md.
 * */
#ifndef SRC_OBJECT_INSTANCE_HPP_
#define SRC_OBJECT_INSTANCE_HPP_

#include "config.hpp"
#include "flags.hpp"

#include <cstddef>
#include <string>

namespace Alarmud {

struct obj_data;
struct char_data;

#if USE_MYSQL
/** Prototipo mondo da usare come base_vnum (char_vnum / non-34k). 0 se sconosciuto. */
int object_instance_resolve_base_vnum(const obj_data* obj);

/** Estrae owner da keyword EDnome (senza il prefisso ED). Vuoto se assente. */
std::string object_instance_extract_ed_owner(const char* keywords);

/** Rimuove token EDxxx dalle keyword. */
std::string object_instance_strip_ed_tokens(const char* keywords);

/**
 * Crea o aggiorna object_instance (+ affect) da obj.
 * actor = wiz/PG che salva (created/updated_by). owner da personal_owner / ED / PERSONAL.
 * write_event: true per osave/personalize; false per sync inventorio (no flood audit).
 * Non aggiorna istanze soft-deleted: ne crea una nuova.
 */
unsigned long long object_instance_persist(obj_data* obj, int base_vnum,
										   unsigned long long update_id = 0,
										   char_data* actor = nullptr,
										   bool write_event = true);

/** Overlay stats/affect/name da object_instance su obj gia' read_object(base). */
bool object_instance_apply(obj_data* obj, unsigned long long instance_id);

/** Sync rapido: se obj->db_instance_id, UPDATE istanza da obj live. */
bool object_instance_sync(obj_data* obj, char_data* actor = nullptr);

/** read_object(base) + apply; caller fa obj_to_char / extract. nullptr se manca/deleted. */
obj_data* object_instance_materialize(unsigned long long instance_id);

/**
 * Elenco wiz. deleted_list=false: solo attivi con numeri densi 1..N.
 * deleted_list=true: solo soft-deleted con numeri densi 1..M.
 * filter vuoto=tutti della lista; numerico=numero lista; altrimenti substring.
 */
void object_instance_show_list(char_data* ch, const char* filter,
							   bool deleted_list = false);

/** Storico event (create/update/delete) per PK interno. */
void object_instance_show_history(char_data* ch, unsigned long long instance_id);

/**
 * Risolve un'istanza: numero di lista (denso nella lista scelta) oppure substring
 * short/name/owner. 0 = nessuna / ambigua (messaggi gia' inviati a ch se ch != nullptr).
 * Restituisce il PK MySQL stabile.
 */
unsigned long long object_instance_resolve_id(char_data* ch, const char* filter,
											 bool deleted_list = false);

/**
 * Soft-delete: marca deleted, tiene affect+event, aggiunge event "delete",
 * azzera instance_id in inventori e db_instance_id online.
 */
bool object_instance_delete(unsigned long long instance_id, char_data* actor = nullptr);

/** Riepilogo per conferma delete (false se id assente o gia' deleted). */
bool object_instance_send_summary(char_data* ch, unsigned long long instance_id);

/** where db: online + inventori MySQL con quel instance_id (PK). */
void object_instance_where(char_data* ch, unsigned long long instance_id);

/** Ultimo event (at DESC) per istanza; scrive su buf. false se nessuno. */
bool object_instance_latest_event_line(unsigned long long instance_id, char* buf,
									   size_t buflen);

/**
 * Numero denso nella lista attiva (1..N) per PK; 0 se manca o e' soft-deleted.
 */
unsigned object_instance_active_list_num(unsigned long long instance_id);

/**
 * Numero denso nella lista cancelled (1..M) per PK; 0 se manca o e' attiva.
 */
unsigned object_instance_deleted_list_num(unsigned long long instance_id);
#else
inline int object_instance_resolve_base_vnum(const obj_data*) {
	return 0;
}
inline std::string object_instance_extract_ed_owner(const char*) {
	return {};
}
inline std::string object_instance_strip_ed_tokens(const char* keywords) {
	return keywords ? std::string(keywords) : std::string();
}
inline unsigned long long object_instance_persist(obj_data*, int, unsigned long long = 0,
												 char_data* = nullptr, bool = true) {
	return 0;
}
inline bool object_instance_apply(obj_data*, unsigned long long) {
	return false;
}
inline bool object_instance_sync(obj_data*, char_data* = nullptr) {
	return false;
}
inline obj_data* object_instance_materialize(unsigned long long) {
	return nullptr;
}
inline void object_instance_show_list(char_data*, const char*, bool = false) {}
inline void object_instance_show_history(char_data*, unsigned long long) {}
inline unsigned long long object_instance_resolve_id(char_data*, const char*,
													bool = false) {
	return 0;
}
inline bool object_instance_delete(unsigned long long, char_data* = nullptr) {
	return false;
}
inline bool object_instance_send_summary(char_data*, unsigned long long) {
	return false;
}
inline void object_instance_where(char_data*, unsigned long long) {}
inline bool object_instance_latest_event_line(unsigned long long, char*, size_t) {
	return false;
}
inline unsigned object_instance_active_list_num(unsigned long long) {
	return 0;
}
inline unsigned object_instance_deleted_list_num(unsigned long long) {
	return 0;
}
#endif

} // namespace Alarmud
#endif
