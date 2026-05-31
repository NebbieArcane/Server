/*
 * legacy_import.hpp — import da file storici (.dat / .aux / rent) verso MySQL character_*.
 * Richiede USE_MYSQL (build con libmysql) e schema S1 applicato.
 */

#ifndef SRC_LEGACY_IMPORT_HPP_
#define SRC_LEGACY_IMPORT_HPP_

#include <cstddef>
#include <string>

namespace odb {
class database;
}

namespace Alarmud {

struct LegacyImportReport {
	bool ok = false;
	std::string message;
	unsigned long long toon_id = 0;
	std::size_t classes = 0;
	std::size_t skills = 0;
	std::size_t affects = 0;
	std::size_t resistances = 0;
	std::size_t prefs = 0;
	std::size_t inventory = 0;
};

/**
 * Legge players/<name>.dat (+ .aux, rent) e scrive tabelle character_*.
 * `file_name`: nome file (minuscolo, es. "montero"); il record toon deve esistere (match su st.name).
 */
bool legacy_import_character_mysql(const char* file_name, LegacyImportReport& report);

/** Rimuove tutte le righe character_* per un toon (prima di cancellare il record toon). */
void legacy_delete_character_rows(odb::database* db, unsigned long long toon_id);

} /* namespace Alarmud */

#endif /* SRC_LEGACY_IMPORT_HPP_ */
