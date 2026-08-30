/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "procarea_legacy_drop.hpp"

#include "autoenums.hpp"
#include "structs.hpp"

namespace Alarmud {

namespace {

struct LegacyDropAffect {
	short loc;
	int mod;
};

struct LegacyDropPhoto {
	unsigned edit_vnum;
	unsigned extra_flags;
	unsigned extra_flags2;
	LegacyDropAffect aff[MAX_OBJ_AFFECT];
};

/* Chiave: vnum file objects/34k. Stats: drop originale (non proto vuoto, non 34k).
 * Nomi/wear/cost restano quelli del proto 651xx. */
constexpr LegacyDropPhoto kPhotos[] = {
	{34868,
	 ITEM_RESISTANT,
	 ITEM2_ONLY_PRINCE,
	 {{APPLY_HITROLL, 4},
	  {APPLY_SPELLPOWER, 2},
	  {APPLY_HIT, 17},
	  {APPLY_SAVE_ALL, -4},
	  {APPLY_SPELLFAIL, -15}}},
	{34869,
	 ITEM_RESISTANT,
	 ITEM2_ONLY_PRINCE,
	 {{APPLY_HIT, 11},
	  {APPLY_DAMROLL, 2},
	  {APPLY_HITROLL, 4},
	  {APPLY_SAVE_ALL, -5},
	  {APPLY_HIT_REGEN, 10}}},
};

const LegacyDropPhoto* find_photo(unsigned edit_vnum) {
	for(const auto& p : kPhotos) {
		if(p.edit_vnum == edit_vnum) {
			return &p;
		}
	}
	return nullptr;
}

} // namespace

bool procarea_legacy_drop_has(unsigned legacy_edit_vnum) {
	return find_photo(legacy_edit_vnum) != nullptr;
}

bool procarea_legacy_drop_apply_to_proto(struct obj_data* proto, unsigned legacy_edit_vnum) {
	if(!proto) {
		return false;
	}
	const LegacyDropPhoto* photo = find_photo(legacy_edit_vnum);
	if(!photo) {
		return false;
	}
	proto->obj_flags.extra_flags = photo->extra_flags;
	proto->obj_flags.extra_flags2 = photo->extra_flags2;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		proto->affected[i].location = photo->aff[i].loc;
		proto->affected[i].modifier = photo->aff[i].mod;
	}
	return true;
}

} // namespace Alarmud
