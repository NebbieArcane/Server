/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* EditAffectBroker — config trasferimenti (oedit affect -> broker).
 *
 * BrokerXfer::Enable  = si puo' trasferire
 * BrokerXfer::Disable = no (resta in lista config per poterlo riattivare)
 *
 * kBrokerLocXfer  = location numeriche / non-bit (una riga per APPLY_*)
 * kBrokerBitXfer  = un bit per riga (IMMUNE / SUSC / M_IMMUNE / SPELL / AFF2)
 *   cls = insieme di accorpamento (Immune / MImmune / Spell / Susc).
 *   Famiglie diverse possono coesistere su B; dentro la stessa location i bit
 *   Enable si spostano a pacchetti (slot) con regole di accorpamento.
 *
 * Enable allineato a listino-edit pezzo (no hit/mana/move/regen, no saving).
 * SUSC: lasciare Disable finche' non si decide di abilitarle.
 */

enum class BrokerXfer : int { Disable = 0, Enable = 1 };

enum class BrokerBitClass : int {
	Immune = 0,	  /* tutto APPLY_IMMUNE (resi spell + fisici) */
	MImmune = 1,  /* APPLY_M_IMMUNE */
	Spell = 2,	  /* APPLY_SPELL + APPLY_AFF2 */
	Susc = 3,	  /* APPLY_SUSC */
	None = 4
};

struct BrokerLocXferCfg {
	int location;
	BrokerXfer xfer;
};

struct BrokerBitXferCfg {
	int location;
	unsigned long bit;
	BrokerXfer xfer;
	BrokerBitClass cls;
};

/* ========== Location (non bitfield) ========== */
static constexpr BrokerLocXferCfg kBrokerLocXfer[] = {
	/* stats */
	{APPLY_STR, BrokerXfer::Enable},
	{APPLY_DEX, BrokerXfer::Enable},
	{APPLY_INT, BrokerXfer::Enable},
	{APPLY_WIS, BrokerXfer::Enable},
	{APPLY_CON, BrokerXfer::Disable},
	{APPLY_CHR, BrokerXfer::Enable},
	/* misc oedit */
	{APPLY_SEX, BrokerXfer::Disable},
	{APPLY_LEVEL, BrokerXfer::Disable},
	{APPLY_AGE, BrokerXfer::Disable},
	{APPLY_CHAR_WEIGHT, BrokerXfer::Disable},
	{APPLY_CHAR_HEIGHT, BrokerXfer::Disable},
	/* pool PG (non pezzo) */
	{APPLY_MANA, BrokerXfer::Disable},
	{APPLY_HIT, BrokerXfer::Disable},
	{APPLY_MOVE, BrokerXfer::Disable},
	{APPLY_GOLD, BrokerXfer::Disable},
	{APPLY_EXP, BrokerXfer::Disable},
	/* combat / armor listino */
	{APPLY_AC, BrokerXfer::Enable},
	{APPLY_HITROLL, BrokerXfer::Enable},
	{APPLY_DAMROLL, BrokerXfer::Enable},
	{APPLY_SPELLPOWER, BrokerXfer::Enable},
	{APPLY_HITNDAM, BrokerXfer::Enable},
	{APPLY_HITNSP, BrokerXfer::Enable},
	/* saving */
	{APPLY_SAVING_PARA, BrokerXfer::Disable},
	{APPLY_SAVING_ROD, BrokerXfer::Disable},
	{APPLY_SAVING_PETRI, BrokerXfer::Disable},
	{APPLY_SAVING_BREATH, BrokerXfer::Disable},
	{APPLY_SAVING_SPELL, BrokerXfer::Disable},
	{APPLY_SAVE_ALL, BrokerXfer::Disable},
	/* staff / speciali */
	{APPLY_WEAPON_SPELL, BrokerXfer::Disable},
	{APPLY_EAT_SPELL, BrokerXfer::Disable},
	{APPLY_RACE_SLAYER, BrokerXfer::Disable},
	{APPLY_ALIGN_SLAYER, BrokerXfer::Disable},
	/* skill % */
	{APPLY_BACKSTAB, BrokerXfer::Disable},
	{APPLY_KICK, BrokerXfer::Disable},
	{APPLY_SNEAK, BrokerXfer::Disable},
	{APPLY_HIDE, BrokerXfer::Disable},
	{APPLY_BASH, BrokerXfer::Disable},
	{APPLY_PICK, BrokerXfer::Disable},
	{APPLY_STEAL, BrokerXfer::Disable},
	{APPLY_TRACK, BrokerXfer::Disable},
	{APPLY_FIND_TRAPS, BrokerXfer::Disable},
	{APPLY_RIDE, BrokerXfer::Disable},
	/* altro */
	{APPLY_SPELLFAIL, BrokerXfer::Disable},
	{APPLY_ATTACKS, BrokerXfer::Disable},
	{APPLY_HASTE, BrokerXfer::Disable},
	{APPLY_SLOW, BrokerXfer::Disable},
	{APPLY_MANA_REGEN, BrokerXfer::Disable},
	{APPLY_HIT_REGEN, BrokerXfer::Disable},
	{APPLY_MOVE_REGEN, BrokerXfer::Disable},
	{APPLY_MOD_THIRST, BrokerXfer::Disable},
	{APPLY_MOD_HUNGER, BrokerXfer::Disable},
	{APPLY_MOD_DRUNK, BrokerXfer::Disable},
	{APPLY_T_STR, BrokerXfer::Disable},
	{APPLY_T_INT, BrokerXfer::Disable},
	{APPLY_T_DEX, BrokerXfer::Disable},
	{APPLY_T_WIS, BrokerXfer::Disable},
	{APPLY_T_CON, BrokerXfer::Disable},
	{APPLY_T_CHR, BrokerXfer::Disable},
	{APPLY_T_HPS, BrokerXfer::Disable},
	{APPLY_T_MOVE, BrokerXfer::Disable},
	{APPLY_T_MANA, BrokerXfer::Disable},
};

/* ========== Bit: APPLY_IMMUNE (resi) ========== */
static constexpr BrokerBitXferCfg kBrokerBitXfer[] = {
	{APPLY_IMMUNE, IMM_FIRE, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_COLD, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_ELEC, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_ENERGY, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_ACID, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_POISON, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_DRAIN, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_HOLD, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_BLUNT, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_PIERCE, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_SLASH, BrokerXfer::Enable, BrokerBitClass::Immune},
	{APPLY_IMMUNE, IMM_SLEEP, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_CHARM, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_NONMAG, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_PLUS1, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_PLUS2, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_PLUS3, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_IMMUNE, IMM_PLUS4, BrokerXfer::Disable, BrokerBitClass::None},

	/* ========== Bit: APPLY_SUSC (Disable finche' non abilitate) ========== */
	{APPLY_SUSC, IMM_FIRE, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_COLD, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_ELEC, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_ENERGY, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_BLUNT, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_PIERCE, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_SLASH, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_ACID, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_POISON, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_DRAIN, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_SLEEP, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_CHARM, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_HOLD, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_NONMAG, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_PLUS1, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_PLUS2, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_PLUS3, BrokerXfer::Disable, BrokerBitClass::Susc},
	{APPLY_SUSC, IMM_PLUS4, BrokerXfer::Disable, BrokerBitClass::Susc},

	/* ========== Bit: APPLY_M_IMMUNE ========== */
	{APPLY_M_IMMUNE, IMM_DRAIN, BrokerXfer::Enable, BrokerBitClass::MImmune},
	{APPLY_M_IMMUNE, IMM_CHARM, BrokerXfer::Enable, BrokerBitClass::MImmune},
	{APPLY_M_IMMUNE, IMM_POISON, BrokerXfer::Enable, BrokerBitClass::MImmune},
	{APPLY_M_IMMUNE, IMM_FIRE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_COLD, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_ELEC, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_ENERGY, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_BLUNT, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_PIERCE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_SLASH, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_ACID, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_SLEEP, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_HOLD, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_NONMAG, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_PLUS1, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_PLUS2, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_PLUS3, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_M_IMMUNE, IMM_PLUS4, BrokerXfer::Disable, BrokerBitClass::None},

	/* ========== Bit: APPLY_SPELL ========== */
	{APPLY_SPELL, AFF_TELEPATHY, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_GLOBE_DARKNESS, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_WATERBREATH, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_TRUE_SIGHT, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_INVISIBLE, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_SENSE_LIFE, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_SCRYING, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_PROTECT_FROM_EVIL, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_FLYING, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_SPELL, AFF_BLIND, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_DETECT_EVIL, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_DETECT_INVISIBLE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_DETECT_MAGIC, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_LIFE_PROT, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_SANCTUARY, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_DRAGON_RIDE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_GROWTH, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_CURSE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_POISON, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_TREE_TRAVEL, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_PARALYSIS, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_INFRAVISION, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_SLEEP, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_TRAVELLING, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_SNEAK, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_HIDE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_SILENCE, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_CHARM, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_FOLLOW, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_FIRESHIELD, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_GROUP, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_SPELL, AFF_UNDEF_AFF_1, BrokerXfer::Disable, BrokerBitClass::None},

	/* ========== Bit: APPLY_AFF2 ========== */
	{APPLY_AFF2, AFF2_DANGER_SENSE, BrokerXfer::Enable, BrokerBitClass::Spell},
	{APPLY_AFF2, AFF2_ANIMAL_INVIS, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_HEAT_STUFF, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_LOG_ME, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_BERSERK, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_PARRY, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_CON_ORDER, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_AFK, BrokerXfer::Disable, BrokerBitClass::None},
	{APPLY_AFF2, AFF2_PKILLER, BrokerXfer::Disable, BrokerBitClass::None},
};
