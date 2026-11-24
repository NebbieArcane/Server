/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __CONFIGURE_HPP
#define __CONFIGURE_HPP
#include "typedefs.hpp"

namespace Alarmud {

/**
 * Dispatcher wiz per i topic di config:
 *   configure                  - elenco topic
 *   configure dimensione ...   - densita/premi/livelli/kit/scorta (DB)
 *   configure dispel ...       - spell_balance (DB; ex dispelconf)
 *   configure vault ...        - vault_plan (DB; ex vaultplan)
 *   configure reboot ...       - orari REBOOT.SCHEDULE (file; ex reboottime)
 *
 * I vecchi comandi restano alias.
 */
ACTION_FUNC(do_configure);

} // namespace Alarmud

#endif
