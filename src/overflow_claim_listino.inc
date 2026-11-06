/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/**
 * Listino stima valore per OverflowClaim (mercato in coin).
 * Base: listino edit Nebbie (mega XP) con 1 mega = kOverflowCoinPerMega coin.
 * Voci assenti dal listino: stima staff (commentata).
 *
 * Modificare qui i numeri; ricalcolo fee = max(min, market / divisor) resta nel .cpp.
 */
#ifndef __OVERFLOW_CLAIM_LISTINO_INC
#define __OVERFLOW_CLAIM_LISTINO_INC

/* Incluso dentro namespace Alarmud (dopo autoenums/spells). */
namespace OvListino {

/** Coin di mercato per 1 mega del listino edit. */
constexpr long kCoinPerMega = 1000;

/** Livello di lancio di riferimento per pozioni/scroll (scala lineare). */
constexpr int kSpellRefCastLevel = 25;

/**
 * Affect: costo in decimi di mega per `batch` unita' di |modifier|.
 * Esempio mana listino 22.5 mega / 10 → mega_x10=225, batch=10.
 */
struct AffectRate {
	int location;
	int mega_x10;
	int batch;
};

constexpr AffectRate kAffectRates[] = {
	/* Stats — listino 15 mega / pt */
	{ APPLY_STR, 150, 1 },
	{ APPLY_DEX, 150, 1 },
	{ APPLY_INT, 150, 1 },
	{ APPLY_WIS, 150, 1 },
	{ APPLY_CON, 150, 1 },
	{ APPLY_CHR, 150, 1 },
	/* Pool — listino */
	{ APPLY_MANA, 225, 10 },      /* 22.5 / 10 */
	{ APPLY_HIT, 450, 10 },       /* 45 / 10 */
	{ APPLY_MOVE, 150, 10 },      /* 15 / 10 */
	{ APPLY_MANA_REGEN, 225, 5 }, /* 22.5 / 5 */
	{ APPLY_HIT_REGEN, 225, 5 },
	{ APPLY_MOVE_REGEN, 150, 5 }, /* 15 / 5 */
	/* Combat — listino */
	{ APPLY_AC, 100, 10 },        /* Armor -10 → 10 mega */
	{ APPLY_HITROLL, 450, 1 },    /* 45 */
	{ APPLY_DAMROLL, 1000, 1 },   /* 100 */
	{ APPLY_SPELLPOWER, 1000, 1 },
	{ APPLY_HITNDAM, 1450, 1 },   /* hit+dam stima */
	{ APPLY_HITNSP, 1450, 1 },
	/* Save — stima (non in listino): save_all 40M/pt, singoli 15M */
	{ APPLY_SAVE_ALL, 400, 1 },
	{ APPLY_SAVING_PARA, 150, 1 },
	{ APPLY_SAVING_ROD, 150, 1 },
	{ APPLY_SAVING_PETRI, 150, 1 },
	{ APPLY_SAVING_BREATH, 150, 1 },
	{ APPLY_SAVING_SPELL, 150, 1 },
	{ APPLY_SPELLFAIL, 80, 1 }, /* stima ~8M/pt */
};

/**
 * Resist / immune bit (APPLY_IMMUNE / APPLY_M_IMMUNE / APPLY_SUSC).
 * Resi = listino resistenze; Immu = listino immunita' (o stima).
 */
struct ImmBitMega {
	unsigned long bit;
	int resist_mega; /* APPLY_IMMUNE */
	int immune_mega; /* APPLY_M_IMMUNE */
};

constexpr ImmBitMega kImmBitRates[] = {
	{ IMM_FIRE, 100, 120 },
	{ IMM_COLD, 75, 90 },
	{ IMM_ELEC, 150, 180 },
	{ IMM_ENERGY, 150, 180 },
	{ IMM_ACID, 75, 90 },
	{ IMM_POISON, 30, 100 },
	{ IMM_DRAIN, 30, 100 },
	{ IMM_HOLD, 75, 90 },
	{ IMM_CHARM, 60, 60 },
	{ IMM_SLEEP, 40, 60 }, /* stima */
	{ IMM_SLASH, 150, 180 },
	{ IMM_PIERCE, 150, 180 },
	{ IMM_BLUNT, 300, 360 },
	{ IMM_NONMAG, 80, 100 }, /* stima */
};

/** Mega default per bit resi/immu non in tabella. */
constexpr int kImmBitResistDefaultMega = 50;
constexpr int kImmBitImmuneDefaultMega = 80;

/**
 * Spell consumabili / APPLY_SPELL / weapon spell: mega a livello ref.
 * Listino edit "Spell editabili" + "Spell di attacco" + stime combat/utility.
 */
struct SpellMega {
	int spell;
	int mega;
};

constexpr SpellMega kSpellRates[] = {
	/* Listino spell editabili (permanenti → stessa ancora per 1 cast) */
	{ SPELL_INVISIBLE, 30 },
	{ SPELL_PROTECT_FROM_EVIL, 50 },
	{ SPELL_SENSE_LIFE, 50 },
	{ SPELL_WATER_BREATH, 50 },
	{ SPELL_FLY, 50 },
	{ SPELL_TRUE_SIGHT, 50 },
	/* Listino spell di attacco su arma */
	{ SPELL_FAERIE_FIRE, 20 },
	{ SPELL_CURSE, 20 },
	{ SPELL_WARP_WEAPON, 20 },
	{ SPELL_CAUSE_LIGHT, 40 },
	{ SPELL_CHILL_TOUCH, 100 },
	{ SPELL_SUNRAY, 100 },
	{ SPELL_CAUSE_SERIOUS, 100 },
	{ SPELL_BLINDNESS, 150 },
	{ SPELL_CAUSE_CRITICAL, 150 },
	/* Utility / detect — stima */
	{ SPELL_DETECT_MAGIC, 10 },
	{ SPELL_DETECT_INVISIBLE, 15 },
	{ SPELL_DETECT_EVIL, 10 },
	{ SPELL_DETECT_POISON, 10 },
	{ SPELL_INFRAVISION, 20 },
	{ SPELL_LIGHT, 10 },
	{ SPELL_CONT_LIGHT, 15 },
	{ SPELL_IDENTIFY, 25 },
	{ SPELL_KNOW_ALIGNMENT, 15 },
	/* Cure / buff comuni — stima */
	{ SPELL_CURE_LIGHT, 15 },
	{ SPELL_CURE_SERIOUS, 35 },
	{ SPELL_CURE_CRITIC, 55 },
	{ SPELL_CURE_BLIND, 25 },
	{ SPELL_REMOVE_POISON, 30 },
	{ SPELL_REMOVE_CURSE, 35 },
	{ SPELL_REMOVE_PARALYSIS, 40 },
	{ SPELL_ARMOR, 20 },
	{ SPELL_BLESS, 20 },
	{ SPELL_STRENGTH, 35 },
	{ SPELL_SHIELD, 35 },
	{ SPELL_STONE_SKIN, 90 },
	{ SPELL_BARKSKIN, 40 },
	{ SPELL_REFRESH, 25 },
	{ SPELL_SECOND_WIND, 30 },
	{ SPELL_AID, 40 },
	{ SPELL_HASTE, 80 },
	{ SPELL_SLOW, 50 },
	/* Combat big — stima */
	{ SPELL_HEAL, 100 },
	{ SPELL_MINOR_HEAL, 40 },
	{ SPELL_SANCTUARY, 120 },
	{ SPELL_FIRESHIELD, 100 },
	{ SPELL_FLY_GROUP, 80 },
	{ SPELL_WORD_OF_RECALL, 40 },
	{ SPELL_SUCCOR, 45 },
	{ SPELL_TELEPORT, 50 },
	{ SPELL_ASTRAL_WALK, 60 },
	{ SPELL_SUMMON, 70 },
	{ SPELL_RESURRECTION, 200 },
	{ SPELL_MANA, 50 },
	/* Offense — stima da listino attacco + potenza */
	{ SPELL_MAGIC_MISSILE, 25 },
	{ SPELL_BURNING_HANDS, 35 },
	{ SPELL_SHOCKING_GRASP, 40 },
	{ SPELL_LIGHTNING_BOLT, 70 },
	{ SPELL_FIREBALL, 100 },
	{ SPELL_COLOUR_SPRAY, 60 },
	{ SPELL_ENERGY_DRAIN, 90 },
	{ SPELL_HARM, 120 },
	{ SPELL_ACID_BLAST, 110 },
	{ SPELL_CONE_OF_COLD, 100 },
	{ SPELL_ICE_STORM, 110 },
	{ SPELL_METEOR_SWARM, 150 },
	{ SPELL_CHAIN_LIGHTNING, 130 },
	{ SPELL_FLAMESTRIKE, 90 },
	{ SPELL_CALL_LIGHTNING, 80 },
	{ SPELL_EARTHQUAKE, 70 },
	{ SPELL_DISPEL_MAGIC, 60 },
	{ SPELL_DISPEL_EVIL, 50 },
	{ SPELL_DISPEL_GOOD, 50 },
	{ SPELL_SLEEP, 80 },
	{ SPELL_CHARM_PERSON, 100 },
	{ SPELL_PARALYSIS, 120 },
	{ SPELL_FEAR, 70 },
	{ SPELL_WEB, 60 },
	{ SPELL_SILENCE, 70 },
	{ SPELL_POISON, 50 },
	{ SPELL_WEAKNESS, 40 },
	{ SPELL_FIRESTORM, 140 },
	{ SPELL_HOLY_WORD, 130 },
	{ SPELL_UNHOLY_WORD, 130 },
};

/** Spell non in tabella: mega = clamp(learn_level, min, max) * questo fattore. */
constexpr int kSpellUnknownMegaMin = 15;
constexpr int kSpellUnknownMegaMax = 120;
constexpr int kSpellUnknownMegaPerLearnLevel = 2;

/** AC sul proto armatura (value0): mega per punto AC — stima. */
constexpr int kArmorAcMegaPerPoint = 5;

/** Flag ARTIFACT in game (= ITEM_IMMUNE extra): mega listino Artifact. */
constexpr int kArtifactFlagMega = 100;

/**
 * Derivati dig (pepite / pietre / lingotti): mercato claim in coin.
 * Scala: rarita' dig + utilita' listino incastonazione, poi ×3 (+200%).
 * Ancore: diamante Artifact = 600k; zircone (resistent) = 120k (×2 su pietre).
 * Gemme non in tabella: proto.cost * kMineGemCostMult (7500→600k).
 */
constexpr long kMineGemCostMult = 80;

struct MineMarket {
	int vnum;
	long market_coin;
};

/** Pepite / minerali ITEM_M_MINERAL. */
constexpr MineMarket kMineMinerals[] = {
	{ 19500, 10000 },  /* rame */
	{ 19501, 10000 },  /* piombo */
	{ 19502, 12000 },  /* ferro */
	{ 19503, 8000 },   /* carbone */
	{ 19504, 45000 },  /* stagno */
	{ 19505, 75000 },  /* oro */
	{ 19506, 180000 }, /* platino */
	{ 19507, 180000 }, /* mithril */
	{ 19508, 180000 }, /* adamantite */
};

/**
 * Pietre ITEM_M_GEM (19509–19537 + cloni), valori ×2.
 * Eccezioni: opale (spy/2), resina (save), ecc. tarate a utilita'.
 */
constexpr MineMarket kMineGems[] = {
	{ 19509, 120000 },  /* quarzo comune */
	{ 19510, 120000 },  /* ossidiana (+1 str, qty 2 in listino) */
	{ 19511, 450000 },  /* opale (spy) */
	{ 19512, 300000 },  /* turchese (prot evil) */
	{ 19513, 120000 },  /* zircone (resistent) */
	{ 19514, 135000 },  /* lapislazzuli (+5 mana regen) */
	{ 19515, 90000 },   /* onice (+1 chr) */
	{ 19516, 90000 },   /* malachite (+1 int) */
	{ 19517, 90000 },   /* ematite (+1 cos) */
	{ 19518, 90000 },   /* giada (+1 wis) */
	{ 19519, 240000 },  /* resina / ambra (−1 save all) */
	{ 19520, 135000 },  /* crisoberillo (+5 move regen) */
	{ 19521, 96000 },   /* spinello blu (−2 spellfail) */
	{ 19522, 135000 },  /* tormalina */
	{ 19523, 120000 },  /* quarzo comune clone */
	{ 19524, 100000 },  /* quarzo rosa (sense life /3) */
	{ 19525, 600000 },  /* agata (immu poison) */
	{ 19526, 300000 },  /* acquamarina (waterbreath) */
	{ 19527, 90000 },   /* berillo (+1 dex) */
	{ 19528, 900000 },  /* topazio (resi elec) */
	{ 19529, 450000 },  /* spinello nero (resi hold) */
	{ 19530, 180000 },  /* fluorite (invis) */
	{ 19531, 900000 },  /* ametista (resi energy) */
	{ 19532, 300000 },  /* corindone (true sight) */
	{ 19533, 870000 },  /* granato (+1 hit-n-dam) */
	{ 19534, 450000 },  /* zaffiro (resi cold) */
	{ 19535, 450000 },  /* smeraldo (resi acid) */
	{ 19536, 600000 },  /* rubino (resi fire) */
	{ 19537, 600000 },  /* diamante (artifact) */
};

/** Lingotti / barre ITEM_BAR (forge): ~×3 rispetto alla pepita corrispondente. */
constexpr MineMarket kMineBars[] = {
	{ 19538, 540000 }, /* adamantite */
	{ 19539, 540000 }, /* mithril */
	{ 19540, 540000 }, /* platino */
	{ 19541, 225000 }, /* oro */
	{ 19542, 135000 }, /* stagno */
	{ 19543, 30000 },  /* rame */
	{ 19544, 50000 },  /* bronzo */
	{ 19545, 30000 },  /* piombo */
	{ 19546, 200000 }, /* argento */
	{ 19547, 36000 },  /* ferro */
	{ 19548, 50000 },  /* acciaio */
};

} // namespace OvListino

#endif /* __OVERFLOW_CLAIM_LISTINO_INC */
