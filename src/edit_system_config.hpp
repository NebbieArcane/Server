/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_EDIT_SYSTEM_CONFIG_HPP_
#define SRC_EDIT_SYSTEM_CONFIG_HPP_

#include <string>

namespace Alarmud {

enum class EditSystemTarget {
	Object,
	Character
};

/** Carica edit_system.json nella mudlib (-d) o EDIT_SYSTEM_CONFIG. */
void edit_system_config_init();

/** Ricarica da file senza riavvio. */
bool edit_system_config_reload();

/** Serializza la config attiva (per API admin). */
[[nodiscard]] std::string edit_system_config_to_json();

/**
 * Sostituisce config, salva su file e ricarica.
 * Restituisce false se JSON non valido o salvataggio fallito.
 */
bool edit_system_config_save_json(const std::string& json_text, std::string& err);

/**
 * true se un APPLY su oggetto con questa location/modifier e' bloccato
 * (perche' il target configurato e' il personaggio).
 */
[[nodiscard]] bool edit_system_blocked_on_object(int location, int modifier) noexcept;

/** Target configurato per un campo pool (hit, mana, move, ...). */
[[nodiscard]] EditSystemTarget edit_system_pool_target(const char* pool_field) noexcept;

[[nodiscard]] bool edit_system_pool_enabled(const char* pool_field) noexcept;

/** Target per resistenza (damage_type = bit IMM_*). Default object se non in config. */
[[nodiscard]] EditSystemTarget edit_system_resistance_target(unsigned damage_type) noexcept;

[[nodiscard]] bool edit_system_resistance_enabled(unsigned damage_type) noexcept;

/** Target per affect listino (APPLY location). Default object. */
[[nodiscard]] EditSystemTarget edit_system_affect_target(int location) noexcept;

[[nodiscard]] bool edit_system_affect_enabled(int location) noexcept;

/** Categoria portale: slug e_item_type senza ITEM_. */
[[nodiscard]] bool edit_system_portal_category_enabled(const char* category) noexcept;

/** Tipi forzati nascosti: nessuno (tutti gestiti dalle checkbox staff). */
[[nodiscard]] bool edit_system_portal_type_always_hidden(const char* category) noexcept;

/** Percorso effettivo del file di config (per log/admin). */
[[nodiscard]] const char* edit_system_config_path();

} // namespace Alarmud

#endif /* SRC_EDIT_SYSTEM_CONFIG_HPP_ */
