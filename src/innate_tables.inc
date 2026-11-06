/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/*
 * Tratti innati razza / classe — unica sorgente dati.
 *
 * Immunita' razza: array completo indicizzato da e_races (anche profili a 0).
 * Affect AFF: solo righe presenti (lista sparsa).
 * SyncImmunities / SyncInnateAffects restano rebuild separati.
 *
 * FUTURO: immune/M_immune/susc -> profili signed -100..+100 (SyncResists).
 *
 * Convenzione bit attuale:
 *   immune   = resistenza
 *   M_immune = immunita' / meta
 *   susc     = vulnerabilita'
 *
 * Se aggiungi una razza in e_races, aggiungi la riga qui: static_assert sul size.
 */
#ifndef __INNATE_TABLES_INC
#define __INNATE_TABLES_INC

struct ImmunityBits {
	unsigned immune = 0;
	unsigned M_immune = 0;
	unsigned susc = 0;
};

/** Affect AFF di razza (INNATE_RACE_*). Una riga per tratto. */
struct RaceInnateAffectRow {
	int race;
	sh_int affect_type;
	long bitvector;
};

/** Soglie immunita' di classe. min_level inclusivo (>=). only_class: OnlyClass(). */
struct ClassImmunityRow {
	unsigned class_bits;
	int level_ind;
	int min_level;
	bool only_class;
	unsigned immune;
	unsigned M_immune;
	unsigned susc;
};

/**
 * Affect AFF di classe (INNATE_CLASS_*).
 * min_alignment: GET_ALIGNMENT >= valore.
 */
struct ClassInnateAffectRow {
	unsigned class_bits;
	int min_alignment;
	bool pc_only;
	sh_int affect_type;
	long bitvector;
};

/* Indice = valore e_races (0 .. E_RACES_COUNT-1). Commento = nome razza. */
static constexpr ImmunityBits kRaceImmunities[E_RACES_COUNT] = {
	/*  0 RACE_HALFBREED */ {},
	/*  1 RACE_HUMAN */ {},
	/*  2 RACE_ELVEN */ {IMM_CHARM, 0, 0},
	/*  3 RACE_DWARF */ {},
	/*  4 RACE_HALFLING */ {},
	/*  5 RACE_GNOME */ {},
	/*  6 RACE_REPTILE */ {},
	/*  7 RACE_SPECIAL */ {},
	/*  8 RACE_LYCANTH */ {0, IMM_NONMAG, 0},
	/*  9 RACE_DRAGON */ {},
	/* 10 RACE_UNDEAD */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 11 RACE_ORC */ {},
	/* 12 RACE_INSECT */ {},
	/* 13 RACE_ARACHNID */ {},
	/* 14 RACE_DINOSAUR */ {},
	/* 15 RACE_FISH */ {},
	/* 16 RACE_BIRD */ {},
	/* 17 RACE_GIANT */ {},
	/* 18 RACE_PREDATOR */ {},
	/* 19 RACE_PARASITE */ {},
	/* 20 RACE_SLIME */ {},
	/* 21 RACE_DEMON */ {0, IMM_FIRE | IMM_POISON, 0},
	/* 22 RACE_SNAKE */ {},
	/* 23 RACE_HERBIV */ {},
	/* 24 RACE_TREE */ {},
	/* 25 RACE_VEGGIE */ {},
	/* 26 RACE_ELEMENT */ {},
	/* 27 RACE_PLANAR */ {},
	/* 28 RACE_DEVIL */ {},
	/* 29 RACE_GHOST */ {},
	/* 30 RACE_GOBLIN */ {},
	/* 31 RACE_TROLL */ {},
	/* 32 RACE_VEGMAN */ {},
	/* 33 RACE_MFLAYER */ {},
	/* 34 RACE_PRIMATE */ {},
	/* 35 RACE_ENFAN */ {},
	/* 36 RACE_DARK_ELF */ {IMM_CHARM, 0, 0},
	/* 37 RACE_GOLEM */ {},
	/* 38 RACE_SKEXIE */ {},
	/* 39 RACE_TROGMAN */ {},
	/* 40 RACE_PATRYN */ {},
	/* 41 RACE_LABRAT */ {},
	/* 42 RACE_SARTAN */ {},
	/* 43 RACE_TYTAN */ {},
	/* 44 RACE_SMURF */ {},
	/* 45 RACE_ROO */ {},
	/* 46 RACE_HORSE */ {},
	/* 47 RACE_DRAAGDIM */ {},
	/* 48 RACE_ASTRAL */ {},
	/* 49 RACE_GOD */ {},
	/* 50 RACE_GIANT_HILL */ {},
	/* 51 RACE_GIANT_FROST */ {0, IMM_COLD, IMM_FIRE},
	/* 52 RACE_GIANT_FIRE */ {0, IMM_FIRE, IMM_COLD},
	/* 53 RACE_GIANT_CLOUD */ {0, IMM_SLEEP, IMM_ACID}, /* sleep: no IMM_GAS */
	/* 54 RACE_GIANT_STORM */ {0, IMM_ELEC, 0},
	/* 55 RACE_GIANT_STONE */ {0, IMM_PIERCE, 0},
	/* 56 RACE_DRAGON_RED */ {0, IMM_FIRE, IMM_COLD},
	/* 57 RACE_DRAGON_BLACK */ {0, IMM_ACID, 0},
	/* 58 RACE_DRAGON_GREEN */ {0, IMM_SLEEP, 0},
	/* 59 RACE_DRAGON_WHITE */ {0, IMM_COLD, IMM_FIRE},
	/* 60 RACE_DRAGON_BLUE */ {0, IMM_ELEC, 0},
	/* 61 RACE_DRAGON_SILVER */ {0, IMM_ENERGY, 0},
	/* 62 RACE_DRAGON_GOLD */ {0, IMM_SLEEP | IMM_ENERGY, 0},
	/* 63 RACE_DRAGON_BRONZE */ {0, IMM_COLD | IMM_ACID, 0},
	/* 64 RACE_DRAGON_COPPER */ {0, IMM_FIRE, 0},
	/* 65 RACE_DRAGON_BRASS */ {0, IMM_ELEC, 0},
	/* 66 RACE_UNDEAD_VAMPIRE */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 67 RACE_UNDEAD_LICH */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 68 RACE_UNDEAD_WIGHT */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 69 RACE_UNDEAD_GHAST */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 70 RACE_UNDEAD_SPECTRE */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 71 RACE_UNDEAD_ZOMBIE */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 72 RACE_UNDEAD_SKELETON */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 73 RACE_UNDEAD_GHOUL */ {0, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_CHARM, 0},
	/* 74 RACE_HALF_ELVEN */ {},
	/* 75 RACE_HALF_OGRE */ {},
	/* 76 RACE_HALF_ORC */ {},
	/* 77 RACE_HALF_GIANT */ {},
	/* 78 RACE_LIZARDMAN */ {},
	/* 79 RACE_DARK_DWARF */ {},
	/* 80 RACE_DEEP_GNOME */ {},
	/* 81 RACE_GNOLL */ {},
	/* 82 RACE_GOLD_ELF */ {IMM_CHARM, 0, 0},
	/* 83 RACE_WILD_ELF */ {IMM_CHARM, 0, 0},
	/* 84 RACE_SEA_ELF */ {IMM_CHARM, 0, 0},
};

static_assert(sizeof(kRaceImmunities) / sizeof(kRaceImmunities[0]) == E_RACES_COUNT,
			  "kRaceImmunities size must match E_RACES_COUNT");

static constexpr RaceInnateAffectRow kRaceInnateAffects[] = {
	{RACE_BIRD, INNATE_RACE_FLY, AFF_FLYING},
	{RACE_DEMON, INNATE_RACE_FLY, AFF_FLYING},

	{RACE_FISH, INNATE_RACE_WATERBREATH, AFF_WATERBREATH},
	{RACE_SEA_ELF, INNATE_RACE_WATERBREATH, AFF_WATERBREATH},

	{RACE_ELVEN, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_DARK_ELF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_GOLD_ELF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_WILD_ELF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_SEA_ELF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_DWARF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_DARK_DWARF, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_DEEP_GNOME, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_GNOME, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_MFLAYER, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_TROLL, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_ORC, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_GOBLIN, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_HALFLING, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_GNOLL, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_DEMON, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_VAMPIRE, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_LICH, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_WIGHT, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_GHAST, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_GHOUL, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_SPECTRE, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_ZOMBIE, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_UNDEAD_SKELETON, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_HALF_ELVEN, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_HALF_OGRE, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_HALF_ORC, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
	{RACE_HALF_GIANT, INNATE_RACE_INFRAVISION, AFF_INFRAVISION},
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

/* Aura paladino: PC + align >= 350 (stesso vincolo di prima). */
static constexpr ClassInnateAffectRow kClassInnateAffects[] = {
	{CLASS_PALADIN, 350, true, INNATE_CLASS_DETECT_EVIL, AFF_DETECT_EVIL},
	{CLASS_PALADIN, 350, true, INNATE_CLASS_PROT_EVIL, AFF_PROTECT_FROM_EVIL},
};

/** Tipi AFF di razza noti: sync anche per rimuovere se la razza non li ha piu'. */
static constexpr sh_int kRaceInnateAffectTypes[] = {
	INNATE_RACE_FLY,
	INNATE_RACE_WATERBREATH,
	INNATE_RACE_INFRAVISION,
};

[[nodiscard]] inline ImmunityBits race_immunity_bits(int race) {
	if(race < 0 || race >= E_RACES_COUNT) {
		return {};
	}
	return kRaceImmunities[race];
}

[[nodiscard]] inline bool race_has_innate_affect(int race, sh_int affect_type,
												 long* bitvector_out) {
	for(const auto& row : kRaceInnateAffects) {
		if(row.race == race && row.affect_type == affect_type) {
			if(bitvector_out != nullptr) {
				*bitvector_out = row.bitvector;
			}
			return true;
		}
	}
	return false;
}

#endif /* __INNATE_TABLES_INC */
