/*
 * legacy_loader.hpp — READ-ONLY loaders for historic player file formats.
 *
 * Formats covered (do not change struct layouts in structs.hpp):
 *   lib/players/<name>.dat  → char_file_u
 *   lib/rent/<name>.aux     → text tag:value lines
 *   lib/rent/<name>         → obj_file_u (new or old rent binary)
 *
 * No writes. Safe for refund zip import and DB migration scripts.
 * See docs/schema-s1-mapping.md and docs/resistance-bit-to-value.md
 */

#ifndef SRC_LEGACY_LOADER_HPP_
#define SRC_LEGACY_LOADER_HPP_

#include "structs.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace Alarmud {

struct LegacyAuxEntry {
	std::string tag;
	std::string value;
};

struct LegacyCharAux {
	std::vector<LegacyAuxEntry> entries;
};

enum class LegacyRentFormat {
	None,
	New,
	Old,
};

/** Load player .dat by character name (lower-case file name). */
bool legacy_load_char_file(const char* name, char_file_u& out);

/** Load player .dat from an explicit path (import from zip/refund). */
bool legacy_load_char_file_path(const char* path, char_file_u& out);

/** Load rent/<name>.aux if present; returns true when file exists and was read. */
bool legacy_load_char_aux(const char* name, LegacyCharAux& out);

/**
 * Load rent inventory file. Tries new obj_file_u layout first, then old_obj_file_u.
 * On success, `out` is always normalized to obj_file_u.
 */
bool legacy_load_rent_file(const char* name, obj_file_u& out,
						   LegacyRentFormat* detected_format = nullptr);

/**
 * Reconstruct immune / M_immune / susc bitvectors from persisted affects in char_file_u.
 * Uses APPLY_IMMUNE / APPLY_SUSC / APPLY_M_IMMUNE (same semantics as affect_modify).
 */
void legacy_derive_resistance_from_file(const char_file_u& st, unsigned& immune,
										unsigned& m_immune, unsigned& susc);

/** One row for character_resistance migration (damage_type = IMM_* bit value). */
struct LegacyResistanceRow {
	unsigned damage_type;
	short value;
};

/** Expand bitvectors to -100 / 0 / +100 rows per docs/resistance-bit-to-value.md */
void legacy_resistance_rows_from_bitvectors(unsigned immune, unsigned m_immune,
											unsigned susc,
											std::vector<LegacyResistanceRow>& out);

} /* namespace Alarmud */

#endif /* SRC_LEGACY_LOADER_HPP_ */
