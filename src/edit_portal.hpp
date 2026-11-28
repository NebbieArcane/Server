/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_EDIT_PORTAL_HPP_
#define SRC_EDIT_PORTAL_HPP_

namespace Alarmud {

/** Avvia thread HTTP + worker API (porta EDIT_API_PORT). Chiamare dopo boot_db. */
void edit_portal_init();

/**
 * Hook sul pulse: aggiorna lo snapshot dei PG online (se l'API e' accesa).
 * Le job API le elabora il worker dedicato. Se la coda non e' vuota e il
 * worker non e' partito, logga emergenza.
 */
void edit_portal_process_pending();

inline constexpr int kEditPortalStaffLevel = 57;
inline constexpr int kEditPortalLimitedLevel = 51;
/** Soglia Lab test kit (XP/oggetti/clone): sottoinsieme dello staff. */
inline constexpr int kEditPortalLabLevel = 58;
/** Incrementare quando cambia l'API HTTP interna del portale edit. */
inline constexpr int kEditPortalApiVersion = 62;
inline constexpr const char kEditPortalApiVersionTag[] = "portal_api_version";

} // namespace Alarmud

#endif
