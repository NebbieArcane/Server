/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Resistenze innate (razza / classe) — unica sorgente dati.
 *
 * OGGI: ancora bitfield (immune / M_immune / susc), come il runtime.
 * FUTURO: valori signed per tipo in -100..+100, con clamp dedicato ai PC
 *         in SyncImmunities (o SyncResists). A quel punto queste righe
 *         diventano profili numerici; la meccanica di rebuild resta qui.
 *
 * Convenzione bit attuale:
 *   immune   = resistenza
 *   M_immune = immunita' / meta
 *   susc     = vulnerabilita'
 */
#ifndef __IMMUNITY_TABLES_INC
#define __IMMUNITY_TABLES_INC

struct ImmunityBits {
	unsigned immune = 0;
	unsigned M_immune = 0;
	unsigned susc = 0;
};

struct RaceImmunityRow {
	int race;
	unsigned immune;
	unsigned M_immune;
	unsigned susc;
};

/** min_level inclusivo (>=). only_class: richiede OnlyClass(). */
struct ClassImmunityRow {
	unsigned class_bits;
	int level_ind;
	int min_level;
	bool only_class;
	unsigned immune;
	unsigned M_immune;
	unsigned susc;
};

static constexpr RaceImmunityRow kRaceImmunities[] = {
	{RACE_SEA_ELF, IMM_CHARM, 0, 0},
	{RACE_ELVEN, IMM_CHARM, 0, 0},
	{RACE_DARK_ELF, IMM_CHARM, 0, 0},
	{RACE_GOLD_ELF, IMM_CHARM, 0, 0},
	{RACE_WILD_ELF, IMM_CHARM, 0, 0},

	{RACE_DEMON, 0, IMM_FIRE | IMM_POISON, 0},
	{RACE_LYCANTH, 0, IMM_NONMAG, 0},

	{RACE_GIANT_FROST, 0, IMM_COLD, IMM_FIRE},
	{RACE_GIANT_FIRE, 0, IMM_FIRE, IMM_COLD},
	{RACE_GIANT_CLOUD, 0, IMM_SLEEP, IMM_ACID}, /* sleep: no IMM_GAS */
	{RACE_GIANT_STORM, 0, IMM_ELEC, 0},
	{RACE_GIANT_STONE, 0, IMM_PIERCE, 0},

	{RACE_UNDEAD, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_VAMPIRE, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_LICH, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_WIGHT, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_GHAST, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_GHOUL, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_SPECTRE, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_ZOMBIE, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	{RACE_UNDEAD_SKELETON, 0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},

	{RACE_DRAGON_RED, 0, IMM_FIRE, IMM_COLD},
	{RACE_DRAGON_BLACK, 0, IMM_ACID, 0},
	{RACE_DRAGON_GREEN, 0, IMM_SLEEP, 0},
	{RACE_DRAGON_WHITE, 0, IMM_COLD, IMM_FIRE},
	{RACE_DRAGON_BLUE, 0, IMM_ELEC, 0},
	{RACE_DRAGON_SILVER, 0, IMM_ENERGY, 0},
	{RACE_DRAGON_GOLD, 0, IMM_SLEEP | IMM_ENERGY, 0},
	{RACE_DRAGON_BRONZE, 0, IMM_COLD | IMM_ACID, 0},
	{RACE_DRAGON_COPPER, 0, IMM_FIRE, 0},
	{RACE_DRAGON_BRASS, 0, IMM_ELEC, 0},
};

/*
 * Soglie storiche: monk usava >N (quindi min = N+1), druido >=N.
 * Precedenza: se ha CLASS_MONK si applicano solo le righe monk (come prima).
 */
static constexpr ClassImmunityRow kClassImmunities[] = {
	{CLASS_MONK, MONK_LEVEL_IND, 11, false, 0, IMM_HOLD, 0},
	{CLASS_MONK, MONK_LEVEL_IND, 19, false, IMM_CHARM, 0, 0},
	{CLASS_MONK, MONK_LEVEL_IND, 23, false, 0, IMM_POISON, 0},
	{CLASS_MONK, MONK_LEVEL_IND, 50, true, 0, IMM_DRAIN, 0},

	{CLASS_DRUID, DRUID_LEVEL_IND, 14, false, IMM_CHARM, 0, 0},
	{CLASS_DRUID, DRUID_LEVEL_IND, 32, false, 0, IMM_POISON, 0},
};

[[nodiscard]] inline ImmunityBits race_immunity_bits(int race) {
	for(const auto& row : kRaceImmunities) {
		if(row.race == race) {
			return {row.immune, row.M_immune, row.susc};
		}
	}
	return {};
}

#endif /* __IMMUNITY_TABLES_INC */
