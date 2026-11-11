/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_TOON_RENAME_HPP_
#define SRC_TOON_RENAME_HPP_

#include <string>
#include <vector>

namespace Alarmud {

/**
 * Nomi precedenti (lowercase) di un toon, da toon_name_history.
 * current_name puo' essere il nome attuale o un nome storico.
 * Lista vuota se la tabella manca o non ci sono righe.
 */
std::vector<std::string> toon_previous_names_lower(const std::string& current_name);

/** Aggiorna name e token ED nel .dat estratto da un backup. true se ha scritto. */
bool toon_fix_player_dat_file(const char* path, const char* old_name, const char* new_name);

/** Aggiorna owner e token ED nel file rent estratto da un backup. true se ha scritto. */
bool toon_fix_rent_file(const char* path, const char* old_name, const char* new_name);

} /* namespace Alarmud */

#endif /* SRC_TOON_RENAME_HPP_ */
