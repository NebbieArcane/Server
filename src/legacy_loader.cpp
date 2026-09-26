/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* legacy_loader.cpp - READ-ONLY historic player file loaders (see legacy_loader.hpp).
 * */
#include "legacy_loader.hpp"

#include "autoenums.hpp"
#include "db.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace Alarmud {
namespace {

int legacy_min(int a, int b) {
	return a < b ? a : b;
}

void legacy_copy_lower_name(const char* name, char* out, std::size_t outlen) {
	if(!name || !out || outlen == 0) {
		return;
	}
	std::snprintf(out, outlen, "%s", name);
	for(char* p = out; *p; ++p) {
		if(*p >= 'A' && *p <= 'Z') {
			*p = static_cast<char>(*p + ('a' - 'A'));
		}
	}
}

void legacy_trim_trailing_crlf(char* s) {
	if(!s) {
		return;
	}
	std::size_t n = std::strlen(s);
	while(n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
		s[--n] = '\0';
	}
}

bool legacy_read_char_blob(FILE* fl, const char* path, char_file_u& out) {
	struct stat fileinfo {};
	long filesize = 0;

	if(fstat(fileno(fl), &fileinfo) == 0) {
		filesize = fileinfo.st_size;
	}
	else {
		filesize = 999999;
	}

	std::memset(&out, 0, sizeof(out));
	out.agemod = 0;

	const std::size_t to_read = static_cast<std::size_t>(
		legacy_min(static_cast<int>(filesize), static_cast<int>(sizeof(char_file_u))));
	if(std::fread(&out, to_read, 1, fl) != 1) {
		return false;
	}

	/* Same kludge as load_char() in db.cpp */
	if(MAX_TOUNGE > 2) {
		out.talks[2] = false;
	}

	(void)path;
	return true;
}

bool legacy_read_rent_new(FILE* fl, obj_file_u& st) {
	std::memset(&st, 0, sizeof(st));

	if(std::fread(st.owner, sizeof(st.owner), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&st.gold_left, sizeof(st.gold_left), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&st.total_cost, sizeof(st.total_cost), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&st.last_update, sizeof(st.last_update), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&st.minimum_stay, sizeof(st.minimum_stay), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&st.number, sizeof(st.number), 1, fl) != 1) {
		return false;
	}
	if(st.number < 0 || st.number > MAX_OBJ_SAVE) {
		return false;
	}
	for(int i = 0; i < st.number; ++i) {
		if(std::fread(&st.objects[i], sizeof(struct obj_file_elem), 1, fl) != 1) {
			return false;
		}
	}
	return true;
}

bool legacy_read_rent_old(FILE* fl, old_obj_file_u& old_st) {
	std::memset(&old_st, 0, sizeof(old_st));

	if(std::fread(old_st.owner, sizeof(old_st.owner), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&old_st.gold_left, sizeof(old_st.gold_left), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&old_st.total_cost, sizeof(old_st.total_cost), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&old_st.last_update, sizeof(old_st.last_update), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&old_st.minimum_stay, sizeof(old_st.minimum_stay), 1, fl) != 1) {
		return false;
	}
	if(std::fread(&old_st.number, sizeof(old_st.number), 1, fl) != 1) {
		return false;
	}
	if(old_st.number < 0 || old_st.number > MAX_OBJ_SAVE) {
		return false;
	}
	for(int i = 0; i < old_st.number; ++i) {
		if(std::fread(&old_st.objects[i], sizeof(struct old_obj_file_elem), 1, fl) != 1) {
			return false;
		}
	}
	return true;
}

void legacy_old_rent_to_new(const old_obj_file_u& old_st, obj_file_u& st) {
	std::memset(&st, 0, sizeof(st));
	std::strncpy(st.owner, old_st.owner, sizeof(st.owner) - 1);
	st.gold_left = old_st.gold_left;
	st.total_cost = old_st.total_cost;
	st.last_update = old_st.last_update;
	st.minimum_stay = old_st.minimum_stay;
	st.number = old_st.number;

	for(int i = 0; i < old_st.number && i < MAX_OBJ_SAVE; ++i) {
		const old_obj_file_elem& o = old_st.objects[i];
		obj_file_elem& d = st.objects[i];

		d.item_number = o.item_number;
		for(int v = 0; v < 4; ++v) {
			d.value[v] = o.value[v];
		}
		d.extra_flags = o.extra_flags;
		d.extra_flags2 = 0;
		d.weight = o.weight;
		d.timer = o.timer;
		d.bitvector = o.bitvector;
		std::strncpy(d.name, o.name, sizeof(d.name) - 1);
		std::strncpy(d.sd, o.sd, sizeof(d.sd) - 1);
		std::strncpy(d.desc, o.desc, sizeof(d.desc) - 1);
		d.wearpos = o.wearpos;
		d.depth = o.depth;
		for(int j = 0; j < MAX_OBJ_AFFECT; ++j) {
			d.affected[j] = o.affected[j];
		}
	}
}

} /* anonymous — re-open after helpers that must stay file-local */

void legacy_convert_old_rent_to_new(const old_obj_file_u& old_st, obj_file_u& st) {
	legacy_old_rent_to_new(old_st, st);
}

namespace {

/** Header: owner[20] + gold/total/last/minstay/number (5 ints). */
constexpr std::size_t kRentFileHeaderBytes = 20 + 5 * sizeof(int);

bool legacy_rent_header_ok(int number, long filesize) {
	if(number < 0 || number > MAX_OBJ_SAVE) {
		return false;
	}
	const long min_old =
		static_cast<long>(kRentFileHeaderBytes +
						  static_cast<std::size_t>(number) * sizeof(old_obj_file_elem));
	return filesize >= min_old;
}

bool prefer_old_rent_by_size(int number, long filesize) {
	const long size_old =
		static_cast<long>(kRentFileHeaderBytes +
						  static_cast<std::size_t>(number) * sizeof(old_obj_file_elem));
	const long size_new =
		static_cast<long>(kRentFileHeaderBytes +
						  static_cast<std::size_t>(number) * sizeof(obj_file_elem));
	if(filesize == size_old) {
		return true;
	}
	if(filesize >= size_old && filesize < size_new) {
		return true;
	}
	return false;
}

void legacy_apply_resistance_mod(unsigned& target, int mod, bool add) {
	if(add) {
		target |= static_cast<unsigned>(mod);
	}
	else {
		target &= ~static_cast<unsigned>(mod);
	}
}

static const unsigned kImmunityBits[] = {
	IMM_FIRE,	 IMM_COLD,	  IMM_ELEC,	   IMM_ENERGY,	IMM_BLUNT,
	IMM_PIERCE,	 IMM_SLASH,	  IMM_ACID,	   IMM_POISON,	IMM_DRAIN,
	IMM_SLEEP,	 IMM_CHARM,	  IMM_HOLD,	   IMM_NONMAG,	IMM_PLUS1,
	IMM_PLUS2,	 IMM_PLUS3,	  IMM_PLUS4,
};

} /* namespace */

bool legacy_load_char_file(const char* name, char_file_u& out) {
	char path[256];
	char lowered[64];

	if(!name || !*name) {
		return false;
	}
	legacy_copy_lower_name(name, lowered, sizeof(lowered));
	std::snprintf(path, sizeof(path), "%s/%s.dat", PLAYERS_DIR, lowered);
	return legacy_load_char_file_path(path, out);
}

bool legacy_load_char_file_path(const char* path, char_file_u& out) {
	if(!path || !*path) {
		return false;
	}

	FILE* fl = std::fopen(path, "rb");
	if(!fl) {
		return false;
	}

	const bool ok = legacy_read_char_blob(fl, path, out);
	std::fclose(fl);
	return ok;
}

bool legacy_load_char_aux(const char* name, LegacyCharAux& out) {
	char path[256];
	char lowered[64];
	char line[260];

	out.entries.clear();
	if(!name || !*name) {
		return false;
	}

	legacy_copy_lower_name(name, lowered, sizeof(lowered));
	std::snprintf(path, sizeof(path), "%s/%s.aux", RENT_DIR, lowered);

	FILE* fp = std::fopen(path, "r");
	if(!fp) {
		std::snprintf(path, sizeof(path), "%s/%s.aux", RENT_DIR, name);
		fp = std::fopen(path, "r");
	}
	if(!fp) {
		return false;
	}

	while(std::fgets(line, sizeof(line), fp)) {
		legacy_trim_trailing_crlf(line);
		if(!line[0]) {
			continue;
		}

		char* colon = std::strchr(line, ':');
		if(!colon) {
			continue;
		}

		*colon = '\0';
		char* value = colon + 1;
		while(*value == ' ' || *value == '\t') {
			++value;
		}

		LegacyAuxEntry entry;
		entry.tag = line;
		entry.value = value;
		out.entries.push_back(std::move(entry));
	}

	std::fclose(fp);
	return true;
}

bool legacy_rent_inventory_looks_sane(const obj_file_u& st) {
	if(st.number < 0 || st.number > MAX_OBJ_SAVE) {
		return false;
	}
	int nonempty = 0;
	int with_vnum = 0;
	int orphan_text = 0; /* name/sd presenti ma item_number == 0 (tipico misalign) */
	int bad_wear = 0;
	int bad_depth = 0;
	for(int i = 0; i < st.number; ++i) {
		const obj_file_elem& o = st.objects[i];
		const bool has_text = (o.name[0] != '\0' || o.sd[0] != '\0');
		const bool has_vnum = (o.item_number > 0);
		if(!has_text && !has_vnum) {
			continue;
		}
		++nonempty;
		if(has_vnum) {
			++with_vnum;
		}
		if(has_text && !has_vnum) {
			++orphan_text;
		}
		if(o.wearpos > MAX_WEAR) {
			++bad_wear;
		}
		if(o.depth > 64) {
			++bad_depth;
		}
	}
	if(nonempty == 0) {
		return true;
	}
	/*
	 * Misaligned 600-on-592 reads: few real vnums, many string fragments without
	 * item_number, and illegal wearpos/depth from ascii bleed.
	 */
	if(orphan_text * 4 >= nonempty) {
		return false;
	}
	if(with_vnum * 4 < nonempty * 3) { /* meno del ~75% degli slot ha un vnum */
		return false;
	}
	if(bad_wear * 3 >= nonempty) {
		return false;
	}
	if(bad_depth * 3 >= nonempty) {
		return false;
	}
	return true;
}

bool legacy_load_rent_file_path(const char* path, obj_file_u& out,
								LegacyRentFormat* detected_format) {
	if(detected_format) {
		*detected_format = LegacyRentFormat::None;
	}
	if(!path || !*path) {
		return false;
	}

	FILE* fl = std::fopen(path, "rb");
	if(!fl) {
		return false;
	}

	struct stat fileinfo {};
	long filesize = 0;
	if(fstat(fileno(fl), &fileinfo) == 0) {
		filesize = fileinfo.st_size;
	}

	auto finish_new = [&](bool ok) -> bool {
		if(!ok) {
			return false;
		}
		if(detected_format) {
			*detected_format = LegacyRentFormat::New;
		}
		std::fclose(fl);
		return true;
	};
	auto finish_old = [&](const old_obj_file_u& old_st) -> bool {
		legacy_old_rent_to_new(old_st, out);
		if(detected_format) {
			*detected_format = LegacyRentFormat::Old;
		}
		std::fclose(fl);
		return true;
	};

	/* Peek number from header for size-based preference. */
	int peeked_number = -1;
	if(filesize >= static_cast<long>(kRentFileHeaderBytes)) {
		unsigned char hdr[kRentFileHeaderBytes];
		if(std::fread(hdr, sizeof(hdr), 1, fl) == 1) {
			std::memcpy(&peeked_number, hdr + 20 + 4 * sizeof(int), sizeof(int));
		}
		std::rewind(fl);
	}

	const bool size_says_old =
		(peeked_number >= 0) && prefer_old_rent_by_size(peeked_number, filesize);

	if(size_says_old) {
		old_obj_file_u old_st;
		if(legacy_read_rent_old(fl, old_st) &&
				legacy_rent_header_ok(old_st.number, filesize)) {
			obj_file_u converted {};
			legacy_old_rent_to_new(old_st, converted);
			if(legacy_rent_inventory_looks_sane(converted)) {
				out = converted;
				return finish_old(old_st);
			}
		}
		std::rewind(fl);
	}

	obj_file_u as_new {};
	if(legacy_read_rent_new(fl, as_new) && legacy_rent_inventory_looks_sane(as_new)) {
		out = as_new;
		return finish_new(true);
	}

	std::rewind(fl);
	old_obj_file_u old_st;
	if(legacy_read_rent_old(fl, old_st)) {
		obj_file_u converted {};
		legacy_old_rent_to_new(old_st, converted);
		if(legacy_rent_inventory_looks_sane(converted) ||
				!legacy_rent_inventory_looks_sane(as_new)) {
			/* Prefer old when new was insane (padded 600-size / 592-body files). */
			out = converted;
			return finish_old(old_st);
		}
	}

	/* Last resort: keep new parse if fread succeeded even if sketchy. */
	if(as_new.number >= 0 && as_new.number <= MAX_OBJ_SAVE && as_new.owner[0] != '\0') {
		out = as_new;
		return finish_new(true);
	}

	std::fclose(fl);
	return false;
}

bool legacy_load_rent_file(const char* name, obj_file_u& out, LegacyRentFormat* detected_format) {
	char path[256];
	char lowered[64];

	if(detected_format) {
		*detected_format = LegacyRentFormat::None;
	}
	if(!name || !*name) {
		return false;
	}

	legacy_copy_lower_name(name, lowered, sizeof(lowered));
	std::snprintf(path, sizeof(path), "%s/%s", RENT_DIR, lowered);

	if(legacy_load_rent_file_path(path, out, detected_format)) {
		return true;
	}

	std::snprintf(path, sizeof(path), "%s/%s", RENT_DIR, name);
	return legacy_load_rent_file_path(path, out, detected_format);
}

void legacy_derive_resistance_from_file(const char_file_u& st, unsigned& immune,
										unsigned& m_immune, unsigned& susc) {
	immune = 0;
	m_immune = 0;
	susc = 0;

	for(int i = 0; i < MAX_AFFECT; ++i) {
		if(!st.affected[i].type) {
			continue;
		}
		const int loc = st.affected[i].location;
		const int mod = st.affected[i].modifier;
		if(loc == APPLY_IMMUNE) {
			legacy_apply_resistance_mod(immune, mod, true);
		}
		else if(loc == APPLY_SUSC) {
			legacy_apply_resistance_mod(susc, mod, true);
		}
		else if(loc == APPLY_M_IMMUNE) {
			legacy_apply_resistance_mod(m_immune, mod, true);
		}
	}
}

void legacy_resistance_rows_from_bitvectors(unsigned immune, unsigned m_immune,
											unsigned susc,
											std::vector<LegacyResistanceRow>& out) {
	out.clear();
	for(unsigned bit : kImmunityBits) {
		short value = 0;
		if((immune & bit) || (m_immune & bit)) {
			value = 100;
		}
		else if(susc & bit) {
			value = -100;
		}
		LegacyResistanceRow row {};
		row.damage_type = bit;
		row.value = value;
		out.push_back(row);
	}
}

} /* namespace Alarmud */
