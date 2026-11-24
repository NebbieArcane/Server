/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __CONFIGURE_HPP
#define __CONFIGURE_HPP
#include "typedefs.hpp"
#include <string>
#include <string_view>

namespace Alarmud {

/**
 * Dispatcher wiz per i topic di config:
 *   configure                  - elenco topic
 *   configure dimensione ...   - densita/premi/livelli/kit/scorta (DB)
 *   configure dispel ...       - spell_balance (DB; ex dispelconf)
 *   configure editpool ...     - cap/prezzi/over editpool (DB edit_pool_balance)
 *   configure derent ...       - prezzi RentEditor (DB service_balance)
 *   configure disincanta ...   - prezzi AffectSlotRemover (DB service_balance)
 *   configure vault ...        - vault_plan (DB; ex vaultplan)
 *   configure reboot ...       - orari REBOOT.SCHEDULE (file; ex reboottime)
 *
 * I vecchi comandi restano alias dove esistevano.
 *
 * Valori interi nei set: usare sempre configure_parse_* (parse_amount:
 * punti migliaia, k/K/m/M, segno opzionale). Nei dump: format_thousands
 * e cfg_kv / cfg_k / cfg_v (chiave bianca, valore giallo).
 * I float (bias, moltiplicatori) restano decimali normali.
 */
ACTION_FUNC(do_configure);

[[nodiscard]] bool configure_parse_ll(std::string_view s, long long& out);
[[nodiscard]] bool configure_parse_long(std::string_view s, long& out);
[[nodiscard]] bool configure_parse_int(std::string_view s, int& out);
void configure_tell_bad_amount(struct char_data* ch);

/** Dump configure: chiave $c0015, valore $c0011, sep tra coppie. */
[[nodiscard]] std::string cfg_k(std::string_view key);
[[nodiscard]] std::string cfg_v(std::string_view val);
[[nodiscard]] std::string cfg_v(long long n);
[[nodiscard]] std::string cfg_kv(std::string_view key, std::string_view val);
[[nodiscard]] std::string cfg_kv(std::string_view key, long long n);
inline constexpr const char* cfg_sep = " | ";

} // namespace Alarmud

#endif
