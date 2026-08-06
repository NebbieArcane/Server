/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_CLAN_SYMBOL_HPP_
#define SRC_CLAN_SYMBOL_HPP_

#include "config.hpp"
#include "flags.hpp"

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

/** true se il vnum e' nella lista simboli di casata (seed). */
bool clan_symbol_is_listed_vnum(unsigned vnum);

/** true se ch puo' indossare questo simbolo (principe / vassallo / imm). */
bool clan_symbol_can_wear(struct char_data* ch, const struct obj_data* obj);
#else
inline void clan_symbol_boot_migrate() {}
inline bool clan_symbol_is_listed_vnum(unsigned) {
	return false;
}
inline bool clan_symbol_can_wear(struct char_data*, const struct obj_data*) {
	return true;
}
#endif

} // namespace Alarmud

#endif /* SRC_CLAN_SYMBOL_HPP_ */
