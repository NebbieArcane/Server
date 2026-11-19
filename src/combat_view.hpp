/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* Visualizzazione del combattimento (comando COMBAT), per categoria.
 * Valore salvato in character_core.combat_view (player.combat_view):
 *   4 bit per categoria, shift = categoria * 4
 *     bit 0-1  vista: 0 completa, 1 compatta, 2 nascosta
 *     bit 2    mancati nascosti
 *     bit 3    incantesimi d'arma nascosti
 *   bit 16     mob uguali NON raggruppati ("(Nx)" spento)
 *   bit 17     riepilogo del gruppo a fine round
 * 0 = tutto completo, come prima del comando. I colpi subiti (TAKEN) non si
 * possono nascondere: al massimo compatti. */
#ifndef __COMBAT_VIEW_HPP
#define __COMBAT_VIEW_HPP

#include "autoenums.hpp"

namespace Alarmud {
namespace combat_view {

enum class Category : int { Mine = 0, Taken = 1, Group = 2, Mobs = 3 };
constexpr int kCategories = 4;

enum class Mode : int { Full = 0, Compact = 1, Hidden = 2 };

constexpr int kBitsPerCategory = 4;
constexpr int kModeMask = 0x3;
constexpr int kNoMissBit = 0x4;
constexpr int kNoWeaponSpellBit = 0x8;
constexpr int kCategoryMask = 0xF;
constexpr int kNoStackBit = 1 << 16;
constexpr int kSummaryBit = 1 << 17;

[[nodiscard]] constexpr int shift(Category c) {
	return static_cast<int>(c) * kBitsPerCategory;
}

/* Vista della categoria; i colpi subiti non scendono sotto "compatta". */
[[nodiscard]] constexpr Mode mode(int v, Category c) {
	const int m = (v >> shift(c)) & kModeMask;
	if(m >= static_cast<int>(Mode::Hidden)) {
		return c == Category::Taken ? Mode::Compact : Mode::Hidden;
	}
	return static_cast<Mode>(m);
}

[[nodiscard]] constexpr int with_mode(int v, Category c, Mode m) {
	return (v & ~(kModeMask << shift(c))) | (static_cast<int>(m) << shift(c));
}

[[nodiscard]] constexpr bool no_miss(int v, Category c) {
	return ((v >> shift(c)) & kNoMissBit) != 0;
}

[[nodiscard]] constexpr bool no_weapon_spell(int v, Category c) {
	return ((v >> shift(c)) & kNoWeaponSpellBit) != 0;
}

/* bit = kNoMissBit o kNoWeaponSpellBit */
[[nodiscard]] constexpr int with_flag(int v, Category c, int bit, bool on) {
	return on ? (v | (bit << shift(c))) : (v & ~(bit << shift(c)));
}

[[nodiscard]] constexpr bool stack(int v) {
	return (v & kNoStackBit) == 0;
}

[[nodiscard]] constexpr bool summary(int v) {
	return (v & kSummaryBit) != 0;
}

static_assert(mode(0, Category::Mine) == Mode::Full, "0 = completo");
static_assert(mode(with_mode(0, Category::Taken, Mode::Hidden), Category::Taken) == Mode::Compact,
			  "i colpi subiti non si nascondono");
static_assert(stack(0) && !summary(0), "default: raggruppa, niente riepilogo");

/* Vecchi bit di user_flags (COMBAT_COMPACT, COMBAT_NO_WEAPON_SPELL,
 * COMBAT_NO_MISS): al caricamento diventano le stesse opzioni su tutte e
 * quattro le categorie e spariscono da user_flags. true = convertito. */
inline bool migrate_legacy(long& user_flags, int& v) {
	const unsigned long legacy = COMBAT_COMPACT | COMBAT_NO_WEAPON_SPELL | COMBAT_NO_MISS;
	const unsigned long uf = static_cast<unsigned long>(user_flags);
	if((uf & legacy) == 0) {
		return false;
	}
	for(int i = 0; i < kCategories; ++i) {
		const Category c = static_cast<Category>(i);
		v = with_mode(v, c, (uf & COMBAT_COMPACT) ? Mode::Compact : Mode::Full);
		v = with_flag(v, c, kNoMissBit, (uf & COMBAT_NO_MISS) != 0);
		v = with_flag(v, c, kNoWeaponSpellBit, (uf & COMBAT_NO_WEAPON_SPELL) != 0);
	}
	user_flags = static_cast<long>(uf & ~legacy);
	return true;
}

} // namespace combat_view
} // namespace Alarmud

#endif /* __COMBAT_VIEW_HPP */
