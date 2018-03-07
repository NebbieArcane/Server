/* GENERATED FILE DO NOT TOUCH
 * Generated from  by code_generator.php */
/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "boost/algorithm/string.hpp"
#include "autoenums.hpp"
namespace Alarmud {
namespace G {
e_affected_by encode_e_affected_by(std::string s,e_affected_by fallback) {
	boost::algorithm::to_upper(s);
	if (s == "AFF_NONE") return AFF_NONE;
	if (s == "AFF_BLIND") return AFF_BLIND;
	if (s == "AFF_INVISIBLE") return AFF_INVISIBLE;
	if (s == "AFF_DETECT_EVIL") return AFF_DETECT_EVIL;
	if (s == "AFF_DETECT_INVISIBLE") return AFF_DETECT_INVISIBLE;
	if (s == "AFF_DETECT_MAGIC") return AFF_DETECT_MAGIC;
	if (s == "AFF_SENSE_LIFE") return AFF_SENSE_LIFE;
	if (s == "AFF_LIFE_PROT") return AFF_LIFE_PROT;
	if (s == "AFF_SANCTUARY") return AFF_SANCTUARY;
	if (s == "AFF_DRAGON_RIDE") return AFF_DRAGON_RIDE;
	if (s == "AFF_GROWTH") return AFF_GROWTH;
	if (s == "AFF_CURSE") return AFF_CURSE;
	if (s == "AFF_FLYING") return AFF_FLYING;
	if (s == "AFF_POISON") return AFF_POISON;
	if (s == "AFF_TREE_TRAVEL") return AFF_TREE_TRAVEL;
	if (s == "AFF_PARALYSIS") return AFF_PARALYSIS;
	if (s == "AFF_INFRAVISION") return AFF_INFRAVISION;
	if (s == "AFF_WATERBREATH") return AFF_WATERBREATH;
	if (s == "AFF_SLEEP") return AFF_SLEEP;
	if (s == "AFF_TRAVELLING") return AFF_TRAVELLING;
	if (s == "AFF_SNEAK") return AFF_SNEAK;
	if (s == "AFF_HIDE") return AFF_HIDE;
	if (s == "AFF_SILENCE") return AFF_SILENCE;
	if (s == "AFF_CHARM") return AFF_CHARM;
	if (s == "AFF_FOLLOW") return AFF_FOLLOW;
	if (s == "AFF_PROTECT_FROM_EVIL") return AFF_PROTECT_FROM_EVIL;
	if (s == "AFF_TRUE_SIGHT") return AFF_TRUE_SIGHT;
	if (s == "AFF_SCRYING") return AFF_SCRYING;
	if (s == "AFF_FIRESHIELD") return AFF_FIRESHIELD;
	if (s == "AFF_GROUP") return AFF_GROUP;
	if (s == "AFF_TELEPATHY") return AFF_TELEPATHY;
	if (s == "AFF_GLOBE_DARKNESS") return AFF_GLOBE_DARKNESS;
	if (s == "AFF_UNDEF_AFF_1") return AFF_UNDEF_AFF_1;
	return fallback;
}

std::string translate(const e_affected_by e) {
	switch(e) {
	case AFF_NONE:
		return "AFF_NONE";
	case AFF_BLIND:
		return "AFF_BLIND";
	case AFF_INVISIBLE:
		return "AFF_INVISIBLE";
	case AFF_DETECT_EVIL:
		return "AFF_DETECT_EVIL";
	case AFF_DETECT_INVISIBLE:
		return "AFF_DETECT_INVISIBLE";
	case AFF_DETECT_MAGIC:
		return "AFF_DETECT_MAGIC";
	case AFF_SENSE_LIFE:
		return "AFF_SENSE_LIFE";
	case AFF_LIFE_PROT:
		return "AFF_LIFE_PROT";
	case AFF_SANCTUARY:
		return "AFF_SANCTUARY";
	case AFF_DRAGON_RIDE:
		return "AFF_DRAGON_RIDE";
	case AFF_GROWTH:
		return "AFF_GROWTH";
	case AFF_CURSE:
		return "AFF_CURSE";
	case AFF_FLYING:
		return "AFF_FLYING";
	case AFF_POISON:
		return "AFF_POISON";
	case AFF_TREE_TRAVEL:
		return "AFF_TREE_TRAVEL";
	case AFF_PARALYSIS:
		return "AFF_PARALYSIS";
	case AFF_INFRAVISION:
		return "AFF_INFRAVISION";
	case AFF_WATERBREATH:
		return "AFF_WATERBREATH";
	case AFF_SLEEP:
		return "AFF_SLEEP";
	case AFF_TRAVELLING:
		return "AFF_TRAVELLING";
	case AFF_SNEAK:
		return "AFF_SNEAK";
	case AFF_HIDE:
		return "AFF_HIDE";
	case AFF_SILENCE:
		return "AFF_SILENCE";
	case AFF_CHARM:
		return "AFF_CHARM";
	case AFF_FOLLOW:
		return "AFF_FOLLOW";
	case AFF_PROTECT_FROM_EVIL:
		return "AFF_PROTECT_FROM_EVIL";
	case AFF_TRUE_SIGHT:
		return "AFF_TRUE_SIGHT";
	case AFF_SCRYING:
		return "AFF_SCRYING";
	case AFF_FIRESHIELD:
		return "AFF_FIRESHIELD";
	case AFF_GROUP:
		return "AFF_GROUP";
	case AFF_TELEPATHY:
		return "AFF_TELEPATHY";
	case AFF_GLOBE_DARKNESS:
		return "AFF_GLOBE_DARKNESS";
	case AFF_UNDEF_AFF_1:
		return "AFF_UNDEF_AFF_1";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_affected_by value) {
	switch(value) {
		case AFF_NONE:
		case AFF_BLIND:
		case AFF_INVISIBLE:
		case AFF_DETECT_EVIL:
		case AFF_DETECT_INVISIBLE:
		case AFF_DETECT_MAGIC:
		case AFF_SENSE_LIFE:
		case AFF_LIFE_PROT:
		case AFF_SANCTUARY:
		case AFF_DRAGON_RIDE:
		case AFF_GROWTH:
		case AFF_CURSE:
		case AFF_FLYING:
		case AFF_POISON:
		case AFF_TREE_TRAVEL:
		case AFF_PARALYSIS:
		case AFF_INFRAVISION:
		case AFF_WATERBREATH:
		case AFF_SLEEP:
		case AFF_TRAVELLING:
		case AFF_SNEAK:
		case AFF_HIDE:
		case AFF_SILENCE:
		case AFF_CHARM:
		case AFF_FOLLOW:
		case AFF_PROTECT_FROM_EVIL:
		case AFF_TRUE_SIGHT:
		case AFF_SCRYING:
		case AFF_FIRESHIELD:
		case AFF_GROUP:
		case AFF_TELEPATHY:
		case AFF_GLOBE_DARKNESS:
		case AFF_UNDEF_AFF_1:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_affected_by value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_affected_by2 encode_e_affected_by2(std::string s,e_affected_by2 fallback) {
	boost::algorithm::to_upper(s);
	if (s == "AFF2_ANIMAL_INVIS") return AFF2_ANIMAL_INVIS;
	if (s == "AFF2_HEAT_STUFF") return AFF2_HEAT_STUFF;
	if (s == "AFF2_LOG_ME") return AFF2_LOG_ME;
	if (s == "AFF2_BERSERK") return AFF2_BERSERK;
	if (s == "AFF2_PARRY") return AFF2_PARRY;
	if (s == "AFF2_CON_ORDER") return AFF2_CON_ORDER;
	if (s == "AFF2_AFK") return AFF2_AFK;
	if (s == "AFF2_PKILLER") return AFF2_PKILLER;
	return fallback;
}

std::string translate(const e_affected_by2 e) {
	switch(e) {
	case AFF2_ANIMAL_INVIS:
		return "AFF2_ANIMAL_INVIS";
	case AFF2_HEAT_STUFF:
		return "AFF2_HEAT_STUFF";
	case AFF2_LOG_ME:
		return "AFF2_LOG_ME";
	case AFF2_BERSERK:
		return "AFF2_BERSERK";
	case AFF2_PARRY:
		return "AFF2_PARRY";
	case AFF2_CON_ORDER:
		return "AFF2_CON_ORDER";
	case AFF2_AFK:
		return "AFF2_AFK";
	case AFF2_PKILLER:
		return "AFF2_PKILLER";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_affected_by2 value) {
	switch(value) {
		case AFF2_ANIMAL_INVIS:
		case AFF2_HEAT_STUFF:
		case AFF2_LOG_ME:
		case AFF2_BERSERK:
		case AFF2_PARRY:
		case AFF2_CON_ORDER:
		case AFF2_AFK:
		case AFF2_PKILLER:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_affected_by2 value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_apply encode_e_apply(std::string s,e_apply fallback) {
	boost::algorithm::to_upper(s);
	if (s == "APPLY_NONE") return APPLY_NONE;
	if (s == "APPLY_STR") return APPLY_STR;
	if (s == "APPLY_DEX") return APPLY_DEX;
	if (s == "APPLY_INT") return APPLY_INT;
	if (s == "APPLY_WIS") return APPLY_WIS;
	if (s == "APPLY_CON") return APPLY_CON;
	if (s == "APPLY_CHR") return APPLY_CHR;
	if (s == "APPLY_SEX") return APPLY_SEX;
	if (s == "APPLY_LEVEL") return APPLY_LEVEL;
	if (s == "APPLY_AGE") return APPLY_AGE;
	if (s == "APPLY_CHAR_WEIGHT") return APPLY_CHAR_WEIGHT;
	if (s == "APPLY_CHAR_HEIGHT") return APPLY_CHAR_HEIGHT;
	if (s == "APPLY_MANA") return APPLY_MANA;
	if (s == "APPLY_HIT") return APPLY_HIT;
	if (s == "APPLY_MOVE") return APPLY_MOVE;
	if (s == "APPLY_GOLD") return APPLY_GOLD;
	if (s == "APPLY_EXP") return APPLY_EXP;
	if (s == "APPLY_AC") return APPLY_AC;
	if (s == "APPLY_HITROLL") return APPLY_HITROLL;
	if (s == "APPLY_DAMROLL") return APPLY_DAMROLL;
	if (s == "APPLY_SAVING_PARA") return APPLY_SAVING_PARA;
	if (s == "APPLY_SAVING_ROD") return APPLY_SAVING_ROD;
	if (s == "APPLY_SAVING_PETRI") return APPLY_SAVING_PETRI;
	if (s == "APPLY_SAVING_BREATH") return APPLY_SAVING_BREATH;
	if (s == "APPLY_SAVING_SPELL") return APPLY_SAVING_SPELL;
	if (s == "APPLY_SAVE_ALL") return APPLY_SAVE_ALL;
	if (s == "APPLY_IMMUNE") return APPLY_IMMUNE;
	if (s == "APPLY_SUSC") return APPLY_SUSC;
	if (s == "APPLY_M_IMMUNE") return APPLY_M_IMMUNE;
	if (s == "APPLY_SPELL") return APPLY_SPELL;
	if (s == "APPLY_WEAPON_SPELL") return APPLY_WEAPON_SPELL;
	if (s == "APPLY_EAT_SPELL") return APPLY_EAT_SPELL;
	if (s == "APPLY_BACKSTAB") return APPLY_BACKSTAB;
	if (s == "APPLY_KICK") return APPLY_KICK;
	if (s == "APPLY_SNEAK") return APPLY_SNEAK;
	if (s == "APPLY_HIDE") return APPLY_HIDE;
	if (s == "APPLY_BASH") return APPLY_BASH;
	if (s == "APPLY_PICK") return APPLY_PICK;
	if (s == "APPLY_STEAL") return APPLY_STEAL;
	if (s == "APPLY_TRACK") return APPLY_TRACK;
	if (s == "APPLY_HITNDAM") return APPLY_HITNDAM;
	if (s == "APPLY_SPELLFAIL") return APPLY_SPELLFAIL;
	if (s == "APPLY_ATTACKS") return APPLY_ATTACKS;
	if (s == "APPLY_HASTE") return APPLY_HASTE;
	if (s == "APPLY_SLOW") return APPLY_SLOW;
	if (s == "APPLY_AFF2") return APPLY_AFF2;
	if (s == "APPLY_FIND_TRAPS") return APPLY_FIND_TRAPS;
	if (s == "APPLY_RIDE") return APPLY_RIDE;
	if (s == "APPLY_RACE_SLAYER") return APPLY_RACE_SLAYER;
	if (s == "APPLY_ALIGN_SLAYER") return APPLY_ALIGN_SLAYER;
	if (s == "APPLY_MANA_REGEN") return APPLY_MANA_REGEN;
	if (s == "APPLY_HIT_REGEN") return APPLY_HIT_REGEN;
	if (s == "APPLY_MOVE_REGEN") return APPLY_MOVE_REGEN;
	if (s == "APPLY_MOD_THIRST") return APPLY_MOD_THIRST;
	if (s == "APPLY_MOD_HUNGER") return APPLY_MOD_HUNGER;
	if (s == "APPLY_MOD_DRUNK") return APPLY_MOD_DRUNK;
	if (s == "APPLY_T_STR") return APPLY_T_STR;
	if (s == "APPLY_T_INT") return APPLY_T_INT;
	if (s == "APPLY_T_DEX") return APPLY_T_DEX;
	if (s == "APPLY_T_WIS") return APPLY_T_WIS;
	if (s == "APPLY_T_CON") return APPLY_T_CON;
	if (s == "APPLY_T_CHR") return APPLY_T_CHR;
	if (s == "APPLY_T_HPS") return APPLY_T_HPS;
	if (s == "APPLY_T_MOVE") return APPLY_T_MOVE;
	if (s == "APPLY_T_MANA") return APPLY_T_MANA;
	if (s == "APPLY_SKIP") return APPLY_SKIP;
	return fallback;
}

std::string translate(const e_apply e) {
	switch(e) {
	case APPLY_NONE:
		return "APPLY_NONE";
	case APPLY_STR:
		return "APPLY_STR";
	case APPLY_DEX:
		return "APPLY_DEX";
	case APPLY_INT:
		return "APPLY_INT";
	case APPLY_WIS:
		return "APPLY_WIS";
	case APPLY_CON:
		return "APPLY_CON";
	case APPLY_CHR:
		return "APPLY_CHR";
	case APPLY_SEX:
		return "APPLY_SEX";
	case APPLY_LEVEL:
		return "APPLY_LEVEL";
	case APPLY_AGE:
		return "APPLY_AGE";
	case APPLY_CHAR_WEIGHT:
		return "APPLY_CHAR_WEIGHT";
	case APPLY_CHAR_HEIGHT:
		return "APPLY_CHAR_HEIGHT";
	case APPLY_MANA:
		return "APPLY_MANA";
	case APPLY_HIT:
		return "APPLY_HIT";
	case APPLY_MOVE:
		return "APPLY_MOVE";
	case APPLY_GOLD:
		return "APPLY_GOLD";
	case APPLY_EXP:
		return "APPLY_EXP";
	case APPLY_AC:
		return "APPLY_AC";
	case APPLY_HITROLL:
		return "APPLY_HITROLL";
	case APPLY_DAMROLL:
		return "APPLY_DAMROLL";
	case APPLY_SAVING_PARA:
		return "APPLY_SAVING_PARA";
	case APPLY_SAVING_ROD:
		return "APPLY_SAVING_ROD";
	case APPLY_SAVING_PETRI:
		return "APPLY_SAVING_PETRI";
	case APPLY_SAVING_BREATH:
		return "APPLY_SAVING_BREATH";
	case APPLY_SAVING_SPELL:
		return "APPLY_SAVING_SPELL";
	case APPLY_SAVE_ALL:
		return "APPLY_SAVE_ALL";
	case APPLY_IMMUNE:
		return "APPLY_IMMUNE";
	case APPLY_SUSC:
		return "APPLY_SUSC";
	case APPLY_M_IMMUNE:
		return "APPLY_M_IMMUNE";
	case APPLY_SPELL:
		return "APPLY_SPELL";
	case APPLY_WEAPON_SPELL:
		return "APPLY_WEAPON_SPELL";
	case APPLY_EAT_SPELL:
		return "APPLY_EAT_SPELL";
	case APPLY_BACKSTAB:
		return "APPLY_BACKSTAB";
	case APPLY_KICK:
		return "APPLY_KICK";
	case APPLY_SNEAK:
		return "APPLY_SNEAK";
	case APPLY_HIDE:
		return "APPLY_HIDE";
	case APPLY_BASH:
		return "APPLY_BASH";
	case APPLY_PICK:
		return "APPLY_PICK";
	case APPLY_STEAL:
		return "APPLY_STEAL";
	case APPLY_TRACK:
		return "APPLY_TRACK";
	case APPLY_HITNDAM:
		return "APPLY_HITNDAM";
	case APPLY_SPELLFAIL:
		return "APPLY_SPELLFAIL";
	case APPLY_ATTACKS:
		return "APPLY_ATTACKS";
	case APPLY_HASTE:
		return "APPLY_HASTE";
	case APPLY_SLOW:
		return "APPLY_SLOW";
	case APPLY_AFF2:
		return "APPLY_AFF2";
	case APPLY_FIND_TRAPS:
		return "APPLY_FIND_TRAPS";
	case APPLY_RIDE:
		return "APPLY_RIDE";
	case APPLY_RACE_SLAYER:
		return "APPLY_RACE_SLAYER";
	case APPLY_ALIGN_SLAYER:
		return "APPLY_ALIGN_SLAYER";
	case APPLY_MANA_REGEN:
		return "APPLY_MANA_REGEN";
	case APPLY_HIT_REGEN:
		return "APPLY_HIT_REGEN";
	case APPLY_MOVE_REGEN:
		return "APPLY_MOVE_REGEN";
	case APPLY_MOD_THIRST:
		return "APPLY_MOD_THIRST";
	case APPLY_MOD_HUNGER:
		return "APPLY_MOD_HUNGER";
	case APPLY_MOD_DRUNK:
		return "APPLY_MOD_DRUNK";
	case APPLY_T_STR:
		return "APPLY_T_STR";
	case APPLY_T_INT:
		return "APPLY_T_INT";
	case APPLY_T_DEX:
		return "APPLY_T_DEX";
	case APPLY_T_WIS:
		return "APPLY_T_WIS";
	case APPLY_T_CON:
		return "APPLY_T_CON";
	case APPLY_T_CHR:
		return "APPLY_T_CHR";
	case APPLY_T_HPS:
		return "APPLY_T_HPS";
	case APPLY_T_MOVE:
		return "APPLY_T_MOVE";
	case APPLY_T_MANA:
		return "APPLY_T_MANA";
	case APPLY_SKIP:
		return "APPLY_SKIP";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_apply value) {
	switch(value) {
		case APPLY_NONE:
		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_INT:
		case APPLY_WIS:
		case APPLY_CON:
		case APPLY_CHR:
		case APPLY_SEX:
		case APPLY_LEVEL:
		case APPLY_AGE:
		case APPLY_CHAR_WEIGHT:
		case APPLY_CHAR_HEIGHT:
		case APPLY_MANA:
		case APPLY_HIT:
		case APPLY_MOVE:
		case APPLY_GOLD:
		case APPLY_EXP:
		case APPLY_AC:
		case APPLY_HITROLL:
		case APPLY_DAMROLL:
		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		case APPLY_SAVE_ALL:
		case APPLY_IMMUNE:
		case APPLY_SUSC:
		case APPLY_M_IMMUNE:
		case APPLY_SPELL:
		case APPLY_WEAPON_SPELL:
		case APPLY_EAT_SPELL:
		case APPLY_BACKSTAB:
		case APPLY_KICK:
		case APPLY_SNEAK:
		case APPLY_HIDE:
		case APPLY_BASH:
		case APPLY_PICK:
		case APPLY_STEAL:
		case APPLY_TRACK:
		case APPLY_HITNDAM:
		case APPLY_SPELLFAIL:
		case APPLY_ATTACKS:
		case APPLY_HASTE:
		case APPLY_SLOW:
		case APPLY_AFF2:
		case APPLY_FIND_TRAPS:
		case APPLY_RIDE:
		case APPLY_RACE_SLAYER:
		case APPLY_ALIGN_SLAYER:
		case APPLY_MANA_REGEN:
		case APPLY_HIT_REGEN:
		case APPLY_MOVE_REGEN:
		case APPLY_MOD_THIRST:
		case APPLY_MOD_HUNGER:
		case APPLY_MOD_DRUNK:
		case APPLY_T_STR:
		case APPLY_T_INT:
		case APPLY_T_DEX:
		case APPLY_T_WIS:
		case APPLY_T_CON:
		case APPLY_T_CHR:
		case APPLY_T_HPS:
		case APPLY_T_MOVE:
		case APPLY_T_MANA:
		case APPLY_SKIP:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_apply value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_class_index encode_e_class_index(std::string s,e_class_index fallback) {
	boost::algorithm::to_upper(s);
	if (s == "MAGE_LEVEL_IND") return MAGE_LEVEL_IND;
	if (s == "CLERIC_LEVEL_IND") return CLERIC_LEVEL_IND;
	if (s == "WARRIOR_LEVEL_IND") return WARRIOR_LEVEL_IND;
	if (s == "THIEF_LEVEL_IND") return THIEF_LEVEL_IND;
	if (s == "DRUID_LEVEL_IND") return DRUID_LEVEL_IND;
	if (s == "MONK_LEVEL_IND") return MONK_LEVEL_IND;
	if (s == "BARBARIAN_LEVEL_IND") return BARBARIAN_LEVEL_IND;
	if (s == "SORCERER_LEVEL_IND") return SORCERER_LEVEL_IND;
	if (s == "PALADIN_LEVEL_IND") return PALADIN_LEVEL_IND;
	if (s == "RANGER_LEVEL_IND") return RANGER_LEVEL_IND;
	if (s == "PSI_LEVEL_IND") return PSI_LEVEL_IND;
	return fallback;
}

std::string translate(const e_class_index e) {
	switch(e) {
	case MAGE_LEVEL_IND:
		return "MAGE_LEVEL_IND";
	case CLERIC_LEVEL_IND:
		return "CLERIC_LEVEL_IND";
	case WARRIOR_LEVEL_IND:
		return "WARRIOR_LEVEL_IND";
	case THIEF_LEVEL_IND:
		return "THIEF_LEVEL_IND";
	case DRUID_LEVEL_IND:
		return "DRUID_LEVEL_IND";
	case MONK_LEVEL_IND:
		return "MONK_LEVEL_IND";
	case BARBARIAN_LEVEL_IND:
		return "BARBARIAN_LEVEL_IND";
	case SORCERER_LEVEL_IND:
		return "SORCERER_LEVEL_IND";
	case PALADIN_LEVEL_IND:
		return "PALADIN_LEVEL_IND";
	case RANGER_LEVEL_IND:
		return "RANGER_LEVEL_IND";
	case PSI_LEVEL_IND:
		return "PSI_LEVEL_IND";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_class_index value) {
	switch(value) {
		case MAGE_LEVEL_IND:
		case CLERIC_LEVEL_IND:
		case WARRIOR_LEVEL_IND:
		case THIEF_LEVEL_IND:
		case DRUID_LEVEL_IND:
		case MONK_LEVEL_IND:
		case BARBARIAN_LEVEL_IND:
		case SORCERER_LEVEL_IND:
		case PALADIN_LEVEL_IND:
		case RANGER_LEVEL_IND:
		case PSI_LEVEL_IND:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_class_index value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_classes encode_e_classes(std::string s,e_classes fallback) {
	boost::algorithm::to_upper(s);
	if (s == "CLASS_MAGIC_USER") return CLASS_MAGIC_USER;
	if (s == "CLASS_CLERIC") return CLASS_CLERIC;
	if (s == "CLASS_WARRIOR") return CLASS_WARRIOR;
	if (s == "CLASS_THIEF") return CLASS_THIEF;
	if (s == "CLASS_DRUID") return CLASS_DRUID;
	if (s == "CLASS_MONK") return CLASS_MONK;
	if (s == "CLASS_BARBARIAN") return CLASS_BARBARIAN;
	if (s == "CLASS_SORCERER") return CLASS_SORCERER;
	if (s == "CLASS_PALADIN") return CLASS_PALADIN;
	if (s == "CLASS_RANGER") return CLASS_RANGER;
	if (s == "CLASS_PSI") return CLASS_PSI;
	return fallback;
}

std::string translate(const e_classes e) {
	switch(e) {
	case CLASS_MAGIC_USER:
		return "CLASS_MAGIC_USER";
	case CLASS_CLERIC:
		return "CLASS_CLERIC";
	case CLASS_WARRIOR:
		return "CLASS_WARRIOR";
	case CLASS_THIEF:
		return "CLASS_THIEF";
	case CLASS_DRUID:
		return "CLASS_DRUID";
	case CLASS_MONK:
		return "CLASS_MONK";
	case CLASS_BARBARIAN:
		return "CLASS_BARBARIAN";
	case CLASS_SORCERER:
		return "CLASS_SORCERER";
	case CLASS_PALADIN:
		return "CLASS_PALADIN";
	case CLASS_RANGER:
		return "CLASS_RANGER";
	case CLASS_PSI:
		return "CLASS_PSI";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_classes value) {
	switch(value) {
		case CLASS_MAGIC_USER:
		case CLASS_CLERIC:
		case CLASS_WARRIOR:
		case CLASS_THIEF:
		case CLASS_DRUID:
		case CLASS_MONK:
		case CLASS_BARBARIAN:
		case CLASS_SORCERER:
		case CLASS_PALADIN:
		case CLASS_RANGER:
		case CLASS_PSI:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_classes value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_conditions encode_e_conditions(std::string s,e_conditions fallback) {
	boost::algorithm::to_upper(s);
	if (s == "DRUNK") return DRUNK;
	if (s == "FULL") return FULL;
	if (s == "THIRST") return THIRST;
	return fallback;
}

std::string translate(const e_conditions e) {
	switch(e) {
	case DRUNK:
		return "DRUNK";
	case FULL:
		return "FULL";
	case THIRST:
		return "THIRST";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_conditions value) {
	switch(value) {
		case DRUNK:
		case FULL:
		case THIRST:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_conditions value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_connection_types encode_e_connection_types(std::string s,e_connection_types fallback) {
	boost::algorithm::to_upper(s);
	if (s == "CON_PLYNG") return CON_PLYNG;
	if (s == "CON_NME") return CON_NME;
	if (s == "CON_NMECNF") return CON_NMECNF;
	if (s == "CON_PWDNRM") return CON_PWDNRM;
	if (s == "CON_PWDGET") return CON_PWDGET;
	if (s == "CON_PWDCNF") return CON_PWDCNF;
	if (s == "CON_QSEX") return CON_QSEX;
	if (s == "CON_RMOTD") return CON_RMOTD;
	if (s == "CON_SLCT") return CON_SLCT;
	if (s == "CON_EXDSCR") return CON_EXDSCR;
	if (s == "CON_QCLASS") return CON_QCLASS;
	if (s == "CON_LDEAD") return CON_LDEAD;
	if (s == "CON_PWDNEW") return CON_PWDNEW;
	if (s == "CON_PWDNCNF") return CON_PWDNCNF;
	if (s == "CON_WIZLOCK") return CON_WIZLOCK;
	if (s == "CON_QRACE") return CON_QRACE;
	if (s == "CON_RACPAR") return CON_RACPAR;
	if (s == "CON_AUTH") return CON_AUTH;
	if (s == "CON_CITY_CHOICE") return CON_CITY_CHOICE;
	if (s == "CON_STAT_LIST") return CON_STAT_LIST;
	if (s == "CON_QDELETE") return CON_QDELETE;
	if (s == "CON_QDELETE2") return CON_QDELETE2;
	if (s == "CON_STAT_LISTV") return CON_STAT_LISTV;
	if (s == "CON_WMOTD") return CON_WMOTD;
	if (s == "CON_EDITING") return CON_EDITING;
	if (s == "CON_DELETE_ME") return CON_DELETE_ME;
	if (s == "CON_CHECK_MAGE_TYPE") return CON_CHECK_MAGE_TYPE;
	if (s == "CON_OBJ_EDITING") return CON_OBJ_EDITING;
	if (s == "CON_MOB_EDITING") return CON_MOB_EDITING;
	if (s == "CON_RNEWD") return CON_RNEWD;
	if (s == "CON_HELPRACE") return CON_HELPRACE;
	if (s == "CON_ENDHELPRACE") return CON_ENDHELPRACE;
	if (s == "CON_HELPCLASS") return CON_HELPCLASS;
	if (s == "CON_ENDHELPCLASS") return CON_ENDHELPCLASS;
	if (s == "CON_HELPROLL") return CON_HELPROLL;
	if (s == "CON_QROLL") return CON_QROLL;
	if (s == "CON_CONF_ROLL") return CON_CONF_ROLL;
	if (s == "CON_EXTRA2") return CON_EXTRA2;
	if (s == "CON_OBJ_FORGING") return CON_OBJ_FORGING;
	return fallback;
}

std::string translate(const e_connection_types e) {
	switch(e) {
	case CON_PLYNG:
		return "CON_PLYNG";
	case CON_NME:
		return "CON_NME";
	case CON_NMECNF:
		return "CON_NMECNF";
	case CON_PWDNRM:
		return "CON_PWDNRM";
	case CON_PWDGET:
		return "CON_PWDGET";
	case CON_PWDCNF:
		return "CON_PWDCNF";
	case CON_QSEX:
		return "CON_QSEX";
	case CON_RMOTD:
		return "CON_RMOTD";
	case CON_SLCT:
		return "CON_SLCT";
	case CON_EXDSCR:
		return "CON_EXDSCR";
	case CON_QCLASS:
		return "CON_QCLASS";
	case CON_LDEAD:
		return "CON_LDEAD";
	case CON_PWDNEW:
		return "CON_PWDNEW";
	case CON_PWDNCNF:
		return "CON_PWDNCNF";
	case CON_WIZLOCK:
		return "CON_WIZLOCK";
	case CON_QRACE:
		return "CON_QRACE";
	case CON_RACPAR:
		return "CON_RACPAR";
	case CON_AUTH:
		return "CON_AUTH";
	case CON_CITY_CHOICE:
		return "CON_CITY_CHOICE";
	case CON_STAT_LIST:
		return "CON_STAT_LIST";
	case CON_QDELETE:
		return "CON_QDELETE";
	case CON_QDELETE2:
		return "CON_QDELETE2";
	case CON_STAT_LISTV:
		return "CON_STAT_LISTV";
	case CON_WMOTD:
		return "CON_WMOTD";
	case CON_EDITING:
		return "CON_EDITING";
	case CON_DELETE_ME:
		return "CON_DELETE_ME";
	case CON_CHECK_MAGE_TYPE:
		return "CON_CHECK_MAGE_TYPE";
	case CON_OBJ_EDITING:
		return "CON_OBJ_EDITING";
	case CON_MOB_EDITING:
		return "CON_MOB_EDITING";
	case CON_RNEWD:
		return "CON_RNEWD";
	case CON_HELPRACE:
		return "CON_HELPRACE";
	case CON_ENDHELPRACE:
		return "CON_ENDHELPRACE";
	case CON_HELPCLASS:
		return "CON_HELPCLASS";
	case CON_ENDHELPCLASS:
		return "CON_ENDHELPCLASS";
	case CON_HELPROLL:
		return "CON_HELPROLL";
	case CON_QROLL:
		return "CON_QROLL";
	case CON_CONF_ROLL:
		return "CON_CONF_ROLL";
	case CON_EXTRA2:
		return "CON_EXTRA2";
	case CON_OBJ_FORGING:
		return "CON_OBJ_FORGING";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_connection_types value) {
	switch(value) {
		case CON_PLYNG:
		case CON_NME:
		case CON_NMECNF:
		case CON_PWDNRM:
		case CON_PWDGET:
		case CON_PWDCNF:
		case CON_QSEX:
		case CON_RMOTD:
		case CON_SLCT:
		case CON_EXDSCR:
		case CON_QCLASS:
		case CON_LDEAD:
		case CON_PWDNEW:
		case CON_PWDNCNF:
		case CON_WIZLOCK:
		case CON_QRACE:
		case CON_RACPAR:
		case CON_AUTH:
		case CON_CITY_CHOICE:
		case CON_STAT_LIST:
		case CON_QDELETE:
		case CON_QDELETE2:
		case CON_STAT_LISTV:
		case CON_WMOTD:
		case CON_EDITING:
		case CON_DELETE_ME:
		case CON_CHECK_MAGE_TYPE:
		case CON_OBJ_EDITING:
		case CON_MOB_EDITING:
		case CON_RNEWD:
		case CON_HELPRACE:
		case CON_ENDHELPRACE:
		case CON_HELPCLASS:
		case CON_ENDHELPCLASS:
		case CON_HELPROLL:
		case CON_QROLL:
		case CON_CONF_ROLL:
		case CON_EXTRA2:
		case CON_OBJ_FORGING:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_connection_types value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_damage_type encode_e_damage_type(std::string s,e_damage_type fallback) {
	boost::algorithm::to_upper(s);
	if (s == "FIRE_DAMAGE") return FIRE_DAMAGE;
	if (s == "COLD_DAMAGE") return COLD_DAMAGE;
	if (s == "ELEC_DAMAGE") return ELEC_DAMAGE;
	if (s == "BLOW_DAMAGE") return BLOW_DAMAGE;
	if (s == "ACID_DAMAGE") return ACID_DAMAGE;
	return fallback;
}

std::string translate(const e_damage_type e) {
	switch(e) {
	case FIRE_DAMAGE:
		return "FIRE_DAMAGE";
	case COLD_DAMAGE:
		return "COLD_DAMAGE";
	case ELEC_DAMAGE:
		return "ELEC_DAMAGE";
	case BLOW_DAMAGE:
		return "BLOW_DAMAGE";
	case ACID_DAMAGE:
		return "ACID_DAMAGE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_damage_type value) {
	switch(value) {
		case FIRE_DAMAGE:
		case COLD_DAMAGE:
		case ELEC_DAMAGE:
		case BLOW_DAMAGE:
		case ACID_DAMAGE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_damage_type value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_events encode_e_events(std::string s,e_events fallback) {
	boost::algorithm::to_upper(s);
	if (s == "EVENT_COMMAND") return EVENT_COMMAND;
	if (s == "EVENT_TICK") return EVENT_TICK;
	if (s == "EVENT_DEATH") return EVENT_DEATH;
	if (s == "EVENT_SUMMER") return EVENT_SUMMER;
	if (s == "EVENT_SPRING") return EVENT_SPRING;
	if (s == "EVENT_FALL") return EVENT_FALL;
	if (s == "EVENT_WINTER") return EVENT_WINTER;
	if (s == "EVENT_GATHER") return EVENT_GATHER;
	if (s == "EVENT_ATTACK") return EVENT_ATTACK;
	if (s == "EVENT_FOLLOW") return EVENT_FOLLOW;
	if (s == "EVENT_MONTH") return EVENT_MONTH;
	if (s == "EVENT_BIRTH") return EVENT_BIRTH;
	if (s == "EVENT_FAMINE") return EVENT_FAMINE;
	if (s == "EVENT_DWARVES_STRIKE") return EVENT_DWARVES_STRIKE;
	if (s == "EVENT_END_STRIKE") return EVENT_END_STRIKE;
	if (s == "EVENT_END_FAMINE") return EVENT_END_FAMINE;
	if (s == "EVENT_WEEK") return EVENT_WEEK;
	if (s == "EVENT_GOBLIN_RAID") return EVENT_GOBLIN_RAID;
	if (s == "EVENT_END_GOB_RAID") return EVENT_END_GOB_RAID;
	if (s == "EVENT_FIGHTING") return EVENT_FIGHTING;
	return fallback;
}

std::string translate(const e_events e) {
	switch(e) {
	case EVENT_COMMAND:
		return "EVENT_COMMAND";
	case EVENT_TICK:
		return "EVENT_TICK";
	case EVENT_DEATH:
		return "EVENT_DEATH";
	case EVENT_SUMMER:
		return "EVENT_SUMMER";
	case EVENT_SPRING:
		return "EVENT_SPRING";
	case EVENT_FALL:
		return "EVENT_FALL";
	case EVENT_WINTER:
		return "EVENT_WINTER";
	case EVENT_GATHER:
		return "EVENT_GATHER";
	case EVENT_ATTACK:
		return "EVENT_ATTACK";
	case EVENT_FOLLOW:
		return "EVENT_FOLLOW";
	case EVENT_MONTH:
		return "EVENT_MONTH";
	case EVENT_BIRTH:
		return "EVENT_BIRTH";
	case EVENT_FAMINE:
		return "EVENT_FAMINE";
	case EVENT_DWARVES_STRIKE:
		return "EVENT_DWARVES_STRIKE";
	case EVENT_END_STRIKE:
		return "EVENT_END_STRIKE";
	case EVENT_END_FAMINE:
		return "EVENT_END_FAMINE";
	case EVENT_WEEK:
		return "EVENT_WEEK";
	case EVENT_GOBLIN_RAID:
		return "EVENT_GOBLIN_RAID";
	case EVENT_END_GOB_RAID:
		return "EVENT_END_GOB_RAID";
	case EVENT_FIGHTING:
		return "EVENT_FIGHTING";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_events value) {
	switch(value) {
		case EVENT_COMMAND:
		case EVENT_TICK:
		case EVENT_DEATH:
		case EVENT_SUMMER:
		case EVENT_SPRING:
		case EVENT_FALL:
		case EVENT_WINTER:
		case EVENT_GATHER:
		case EVENT_ATTACK:
		case EVENT_FOLLOW:
		case EVENT_MONTH:
		case EVENT_BIRTH:
		case EVENT_FAMINE:
		case EVENT_DWARVES_STRIKE:
		case EVENT_END_STRIKE:
		case EVENT_END_FAMINE:
		case EVENT_WEEK:
		case EVENT_GOBLIN_RAID:
		case EVENT_END_GOB_RAID:
		case EVENT_FIGHTING:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_events value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_exit_dir encode_e_exit_dir(std::string s,e_exit_dir fallback) {
	boost::algorithm::to_upper(s);
	if (s == "NORTH") return NORTH;
	if (s == "EAST") return EAST;
	if (s == "SOUTH") return SOUTH;
	if (s == "WEST") return WEST;
	if (s == "UP") return UP;
	if (s == "DOWN") return DOWN;
	return fallback;
}

std::string translate(const e_exit_dir e) {
	switch(e) {
	case NORTH:
		return "NORTH";
	case EAST:
		return "EAST";
	case SOUTH:
		return "SOUTH";
	case WEST:
		return "WEST";
	case UP:
		return "UP";
	case DOWN:
		return "DOWN";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_exit_dir value) {
	switch(value) {
		case NORTH:
		case EAST:
		case SOUTH:
		case WEST:
		case UP:
		case DOWN:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_exit_dir value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_exit_types encode_e_exit_types(std::string s,e_exit_types fallback) {
	boost::algorithm::to_upper(s);
	if (s == "EX_ISDOOR") return EX_ISDOOR;
	if (s == "EX_CLOSED") return EX_CLOSED;
	if (s == "EX_LOCKED") return EX_LOCKED;
	if (s == "EX_SECRET") return EX_SECRET;
	if (s == "EX_NOTBASH") return EX_NOTBASH;
	if (s == "EX_PICKPROOF") return EX_PICKPROOF;
	if (s == "EX_CLIMB") return EX_CLIMB;
	if (s == "EX_MALE") return EX_MALE;
	if (s == "EX_NOLOOKT") return EX_NOLOOKT;
	return fallback;
}

std::string translate(const e_exit_types e) {
	switch(e) {
	case EX_ISDOOR:
		return "EX_ISDOOR";
	case EX_CLOSED:
		return "EX_CLOSED";
	case EX_LOCKED:
		return "EX_LOCKED";
	case EX_SECRET:
		return "EX_SECRET";
	case EX_NOTBASH:
		return "EX_NOTBASH";
	case EX_PICKPROOF:
		return "EX_PICKPROOF";
	case EX_CLIMB:
		return "EX_CLIMB";
	case EX_MALE:
		return "EX_MALE";
	case EX_NOLOOKT:
		return "EX_NOLOOKT";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_exit_types value) {
	switch(value) {
		case EX_ISDOOR:
		case EX_CLOSED:
		case EX_LOCKED:
		case EX_SECRET:
		case EX_NOTBASH:
		case EX_PICKPROOF:
		case EX_CLIMB:
		case EX_MALE:
		case EX_NOLOOKT:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_exit_types value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_fear_type encode_e_fear_type(std::string s,e_fear_type fallback) {
	boost::algorithm::to_upper(s);
	if (s == "FEAR_SEX") return FEAR_SEX;
	if (s == "FEAR_RACE") return FEAR_RACE;
	if (s == "FEAR_CHAR") return FEAR_CHAR;
	if (s == "FEAR_CLASS") return FEAR_CLASS;
	if (s == "FEAR_EVIL") return FEAR_EVIL;
	if (s == "FEAR_GOOD") return FEAR_GOOD;
	if (s == "FEAR_VNUM") return FEAR_VNUM;
	return fallback;
}

std::string translate(const e_fear_type e) {
	switch(e) {
	case FEAR_SEX:
		return "FEAR_SEX";
	case FEAR_RACE:
		return "FEAR_RACE";
	case FEAR_CHAR:
		return "FEAR_CHAR";
	case FEAR_CLASS:
		return "FEAR_CLASS";
	case FEAR_EVIL:
		return "FEAR_EVIL";
	case FEAR_GOOD:
		return "FEAR_GOOD";
	case FEAR_VNUM:
		return "FEAR_VNUM";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_fear_type value) {
	switch(value) {
		case FEAR_SEX:
		case FEAR_RACE:
		case FEAR_CHAR:
		case FEAR_CLASS:
		case FEAR_EVIL:
		case FEAR_GOOD:
		case FEAR_VNUM:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_fear_type value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_find_result encode_e_find_result(std::string s,e_find_result fallback) {
	boost::algorithm::to_upper(s);
	if (s == "FOUND_WATER") return FOUND_WATER;
	if (s == "FOUND_FOOD") return FOUND_FOOD;
	if (s == "FOUND_FOOD0") return FOUND_FOOD0;
	if (s == "FOUND_FOOD1") return FOUND_FOOD1;
	if (s == "FOUND_FOOD2") return FOUND_FOOD2;
	if (s == "FOUND_FOOD3") return FOUND_FOOD3;
	if (s == "FOUND_FOOD4") return FOUND_FOOD4;
	if (s == "FOUND_FOOD5") return FOUND_FOOD5;
	if (s == "FOUND_FOOD6") return FOUND_FOOD6;
	if (s == "FOUND_FOOD7") return FOUND_FOOD7;
	if (s == "FOUND_FOOD8") return FOUND_FOOD8;
	if (s == "FOUND_FOOD9") return FOUND_FOOD9;
	if (s == "FOUND_FOOD10") return FOUND_FOOD10;
	if (s == "FOUND_FOOD11") return FOUND_FOOD11;
	if (s == "FOUND_FOOD12") return FOUND_FOOD12;
	if (s == "FOUND_FOOD13") return FOUND_FOOD13;
	if (s == "FOUND_FOOD14") return FOUND_FOOD14;
	if (s == "FOUND_FOOD15") return FOUND_FOOD15;
	if (s == "FOUND_FOOD16") return FOUND_FOOD16;
	if (s == "FOUND_FOOD17") return FOUND_FOOD17;
	if (s == "FOUND_FOOD18") return FOUND_FOOD18;
	if (s == "FOUND_FOOD19") return FOUND_FOOD19;
	if (s == "FOUND_FOOD20") return FOUND_FOOD20;
	if (s == "FOUND_FOOD21") return FOUND_FOOD21;
	if (s == "FOUND_FOOD22") return FOUND_FOOD22;
	if (s == "FOUND_FOOD23") return FOUND_FOOD23;
	if (s == "FOUND_FOOD24") return FOUND_FOOD24;
	if (s == "FOUND_FOOD25") return FOUND_FOOD25;
	if (s == "FOUND_FOOD26") return FOUND_FOOD26;
	if (s == "FOUND_FOOD27") return FOUND_FOOD27;
	if (s == "FOUND_FOOD28") return FOUND_FOOD28;
	if (s == "FOUND_FOOD29") return FOUND_FOOD29;
	if (s == "FOUND_FOOD30") return FOUND_FOOD30;
	if (s == "FOUND_FOOD31") return FOUND_FOOD31;
	if (s == "FOUND_FOOD32") return FOUND_FOOD32;
	if (s == "FOUND_FOOD33") return FOUND_FOOD33;
	if (s == "FOUND_FOOD34") return FOUND_FOOD34;
	if (s == "FOUND_FOOD35") return FOUND_FOOD35;
	if (s == "FOUND_FOOD36") return FOUND_FOOD36;
	if (s == "FOUND_FOOD37") return FOUND_FOOD37;
	if (s == "FOUND_FOOD38") return FOUND_FOOD38;
	if (s == "FOUND_FOOD39") return FOUND_FOOD39;
	if (s == "FOUND_FOOD40") return FOUND_FOOD40;
	if (s == "FOUND_FOOD41") return FOUND_FOOD41;
	if (s == "FOUND_FOOD42") return FOUND_FOOD42;
	if (s == "FOUND_FOOD43") return FOUND_FOOD43;
	if (s == "FOUND_FOOD44") return FOUND_FOOD44;
	if (s == "FOUND_FOOD45") return FOUND_FOOD45;
	if (s == "FOUND_FOOD46") return FOUND_FOOD46;
	if (s == "FOUND_FOOD47") return FOUND_FOOD47;
	if (s == "FOUND_FOOD48") return FOUND_FOOD48;
	if (s == "FOUND_FOOD49") return FOUND_FOOD49;
	if (s == "FOUND_FOOD50") return FOUND_FOOD50;
	if (s == "FOUND_FOOD51") return FOUND_FOOD51;
	if (s == "FOUND_FOOD52") return FOUND_FOOD52;
	if (s == "FOUND_FOOD53") return FOUND_FOOD53;
	if (s == "FOUND_FOOD54") return FOUND_FOOD54;
	if (s == "FOUND_FOOD55") return FOUND_FOOD55;
	if (s == "FOUND_FOOD56") return FOUND_FOOD56;
	if (s == "FOUND_FOOD57") return FOUND_FOOD57;
	if (s == "FOUND_FOOD58") return FOUND_FOOD58;
	if (s == "FOUND_FOOD59") return FOUND_FOOD59;
	if (s == "FOUND_FOOD60") return FOUND_FOOD60;
	if (s == "FOUND_FOOD61") return FOUND_FOOD61;
	if (s == "FOUND_FOOD62") return FOUND_FOOD62;
	if (s == "FOUND_FOOD63") return FOUND_FOOD63;
	if (s == "FOUND_FOOD64") return FOUND_FOOD64;
	if (s == "FOUND_FOOD65") return FOUND_FOOD65;
	if (s == "FOUND_FOOD66") return FOUND_FOOD66;
	if (s == "FOUND_FOOD67") return FOUND_FOOD67;
	if (s == "FOUND_FOOD68") return FOUND_FOOD68;
	if (s == "FOUND_FOOD69") return FOUND_FOOD69;
	return fallback;
}

std::string translate(const e_find_result e) {
	switch(e) {
	case FOUND_WATER:
		return "FOUND_WATER";
	case FOUND_FOOD:
		return "FOUND_FOOD";
	case FOUND_FOOD0:
		return "FOUND_FOOD0";
	case FOUND_FOOD1:
		return "FOUND_FOOD1";
	case FOUND_FOOD2:
		return "FOUND_FOOD2";
	case FOUND_FOOD3:
		return "FOUND_FOOD3";
	case FOUND_FOOD4:
		return "FOUND_FOOD4";
	case FOUND_FOOD5:
		return "FOUND_FOOD5";
	case FOUND_FOOD6:
		return "FOUND_FOOD6";
	case FOUND_FOOD7:
		return "FOUND_FOOD7";
	case FOUND_FOOD8:
		return "FOUND_FOOD8";
	case FOUND_FOOD9:
		return "FOUND_FOOD9";
	case FOUND_FOOD10:
		return "FOUND_FOOD10";
	case FOUND_FOOD11:
		return "FOUND_FOOD11";
	case FOUND_FOOD12:
		return "FOUND_FOOD12";
	case FOUND_FOOD13:
		return "FOUND_FOOD13";
	case FOUND_FOOD14:
		return "FOUND_FOOD14";
	case FOUND_FOOD15:
		return "FOUND_FOOD15";
	case FOUND_FOOD16:
		return "FOUND_FOOD16";
	case FOUND_FOOD17:
		return "FOUND_FOOD17";
	case FOUND_FOOD18:
		return "FOUND_FOOD18";
	case FOUND_FOOD19:
		return "FOUND_FOOD19";
	case FOUND_FOOD20:
		return "FOUND_FOOD20";
	case FOUND_FOOD21:
		return "FOUND_FOOD21";
	case FOUND_FOOD22:
		return "FOUND_FOOD22";
	case FOUND_FOOD23:
		return "FOUND_FOOD23";
	case FOUND_FOOD24:
		return "FOUND_FOOD24";
	case FOUND_FOOD25:
		return "FOUND_FOOD25";
	case FOUND_FOOD26:
		return "FOUND_FOOD26";
	case FOUND_FOOD27:
		return "FOUND_FOOD27";
	case FOUND_FOOD28:
		return "FOUND_FOOD28";
	case FOUND_FOOD29:
		return "FOUND_FOOD29";
	case FOUND_FOOD30:
		return "FOUND_FOOD30";
	case FOUND_FOOD31:
		return "FOUND_FOOD31";
	case FOUND_FOOD32:
		return "FOUND_FOOD32";
	case FOUND_FOOD33:
		return "FOUND_FOOD33";
	case FOUND_FOOD34:
		return "FOUND_FOOD34";
	case FOUND_FOOD35:
		return "FOUND_FOOD35";
	case FOUND_FOOD36:
		return "FOUND_FOOD36";
	case FOUND_FOOD37:
		return "FOUND_FOOD37";
	case FOUND_FOOD38:
		return "FOUND_FOOD38";
	case FOUND_FOOD39:
		return "FOUND_FOOD39";
	case FOUND_FOOD40:
		return "FOUND_FOOD40";
	case FOUND_FOOD41:
		return "FOUND_FOOD41";
	case FOUND_FOOD42:
		return "FOUND_FOOD42";
	case FOUND_FOOD43:
		return "FOUND_FOOD43";
	case FOUND_FOOD44:
		return "FOUND_FOOD44";
	case FOUND_FOOD45:
		return "FOUND_FOOD45";
	case FOUND_FOOD46:
		return "FOUND_FOOD46";
	case FOUND_FOOD47:
		return "FOUND_FOOD47";
	case FOUND_FOOD48:
		return "FOUND_FOOD48";
	case FOUND_FOOD49:
		return "FOUND_FOOD49";
	case FOUND_FOOD50:
		return "FOUND_FOOD50";
	case FOUND_FOOD51:
		return "FOUND_FOOD51";
	case FOUND_FOOD52:
		return "FOUND_FOOD52";
	case FOUND_FOOD53:
		return "FOUND_FOOD53";
	case FOUND_FOOD54:
		return "FOUND_FOOD54";
	case FOUND_FOOD55:
		return "FOUND_FOOD55";
	case FOUND_FOOD56:
		return "FOUND_FOOD56";
	case FOUND_FOOD57:
		return "FOUND_FOOD57";
	case FOUND_FOOD58:
		return "FOUND_FOOD58";
	case FOUND_FOOD59:
		return "FOUND_FOOD59";
	case FOUND_FOOD60:
		return "FOUND_FOOD60";
	case FOUND_FOOD61:
		return "FOUND_FOOD61";
	case FOUND_FOOD62:
		return "FOUND_FOOD62";
	case FOUND_FOOD63:
		return "FOUND_FOOD63";
	case FOUND_FOOD64:
		return "FOUND_FOOD64";
	case FOUND_FOOD65:
		return "FOUND_FOOD65";
	case FOUND_FOOD66:
		return "FOUND_FOOD66";
	case FOUND_FOOD67:
		return "FOUND_FOOD67";
	case FOUND_FOOD68:
		return "FOUND_FOOD68";
	case FOUND_FOOD69:
		return "FOUND_FOOD69";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_find_result value) {
	switch(value) {
		case FOUND_WATER:
		case FOUND_FOOD:
		case FOUND_FOOD0:
		case FOUND_FOOD1:
		case FOUND_FOOD2:
		case FOUND_FOOD3:
		case FOUND_FOOD4:
		case FOUND_FOOD5:
		case FOUND_FOOD6:
		case FOUND_FOOD7:
		case FOUND_FOOD8:
		case FOUND_FOOD9:
		case FOUND_FOOD10:
		case FOUND_FOOD11:
		case FOUND_FOOD12:
		case FOUND_FOOD13:
		case FOUND_FOOD14:
		case FOUND_FOOD15:
		case FOUND_FOOD16:
		case FOUND_FOOD17:
		case FOUND_FOOD18:
		case FOUND_FOOD19:
		case FOUND_FOOD20:
		case FOUND_FOOD21:
		case FOUND_FOOD22:
		case FOUND_FOOD23:
		case FOUND_FOOD24:
		case FOUND_FOOD25:
		case FOUND_FOOD26:
		case FOUND_FOOD27:
		case FOUND_FOOD28:
		case FOUND_FOOD29:
		case FOUND_FOOD30:
		case FOUND_FOOD31:
		case FOUND_FOOD32:
		case FOUND_FOOD33:
		case FOUND_FOOD34:
		case FOUND_FOOD35:
		case FOUND_FOOD36:
		case FOUND_FOOD37:
		case FOUND_FOOD38:
		case FOUND_FOOD39:
		case FOUND_FOOD40:
		case FOUND_FOOD41:
		case FOUND_FOOD42:
		case FOUND_FOOD43:
		case FOUND_FOOD44:
		case FOUND_FOOD45:
		case FOUND_FOOD46:
		case FOUND_FOOD47:
		case FOUND_FOOD48:
		case FOUND_FOOD49:
		case FOUND_FOOD50:
		case FOUND_FOOD51:
		case FOUND_FOOD52:
		case FOUND_FOOD53:
		case FOUND_FOOD54:
		case FOUND_FOOD55:
		case FOUND_FOOD56:
		case FOUND_FOOD57:
		case FOUND_FOOD58:
		case FOUND_FOOD59:
		case FOUND_FOOD60:
		case FOUND_FOOD61:
		case FOUND_FOOD62:
		case FOUND_FOOD63:
		case FOUND_FOOD64:
		case FOUND_FOOD65:
		case FOUND_FOOD66:
		case FOUND_FOOD67:
		case FOUND_FOOD68:
		case FOUND_FOOD69:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_find_result value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_hate_type encode_e_hate_type(std::string s,e_hate_type fallback) {
	boost::algorithm::to_upper(s);
	if (s == "HATE_SEX") return HATE_SEX;
	if (s == "HATE_RACE") return HATE_RACE;
	if (s == "HATE_CHAR") return HATE_CHAR;
	if (s == "HATE_CLASS") return HATE_CLASS;
	if (s == "HATE_EVIL") return HATE_EVIL;
	if (s == "HATE_GOOD") return HATE_GOOD;
	if (s == "HATE_VNUM") return HATE_VNUM;
	return fallback;
}

std::string translate(const e_hate_type e) {
	switch(e) {
	case HATE_SEX:
		return "HATE_SEX";
	case HATE_RACE:
		return "HATE_RACE";
	case HATE_CHAR:
		return "HATE_CHAR";
	case HATE_CLASS:
		return "HATE_CLASS";
	case HATE_EVIL:
		return "HATE_EVIL";
	case HATE_GOOD:
		return "HATE_GOOD";
	case HATE_VNUM:
		return "HATE_VNUM";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_hate_type value) {
	switch(value) {
		case HATE_SEX:
		case HATE_RACE:
		case HATE_CHAR:
		case HATE_CLASS:
		case HATE_EVIL:
		case HATE_GOOD:
		case HATE_VNUM:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_hate_type value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_immunity_type encode_e_immunity_type(std::string s,e_immunity_type fallback) {
	boost::algorithm::to_upper(s);
	if (s == "IMM_FIRE") return IMM_FIRE;
	if (s == "IMM_COLD") return IMM_COLD;
	if (s == "IMM_ELEC") return IMM_ELEC;
	if (s == "IMM_ENERGY") return IMM_ENERGY;
	if (s == "IMM_BLUNT") return IMM_BLUNT;
	if (s == "IMM_PIERCE") return IMM_PIERCE;
	if (s == "IMM_SLASH") return IMM_SLASH;
	if (s == "IMM_ACID") return IMM_ACID;
	if (s == "IMM_POISON") return IMM_POISON;
	if (s == "IMM_DRAIN") return IMM_DRAIN;
	if (s == "IMM_SLEEP") return IMM_SLEEP;
	if (s == "IMM_CHARM") return IMM_CHARM;
	if (s == "IMM_HOLD") return IMM_HOLD;
	if (s == "IMM_NONMAG") return IMM_NONMAG;
	if (s == "IMM_PLUS1") return IMM_PLUS1;
	if (s == "IMM_PLUS2") return IMM_PLUS2;
	if (s == "IMM_PLUS3") return IMM_PLUS3;
	if (s == "IMM_PLUS4") return IMM_PLUS4;
	return fallback;
}

std::string translate(const e_immunity_type e) {
	switch(e) {
	case IMM_FIRE:
		return "IMM_FIRE";
	case IMM_COLD:
		return "IMM_COLD";
	case IMM_ELEC:
		return "IMM_ELEC";
	case IMM_ENERGY:
		return "IMM_ENERGY";
	case IMM_BLUNT:
		return "IMM_BLUNT";
	case IMM_PIERCE:
		return "IMM_PIERCE";
	case IMM_SLASH:
		return "IMM_SLASH";
	case IMM_ACID:
		return "IMM_ACID";
	case IMM_POISON:
		return "IMM_POISON";
	case IMM_DRAIN:
		return "IMM_DRAIN";
	case IMM_SLEEP:
		return "IMM_SLEEP";
	case IMM_CHARM:
		return "IMM_CHARM";
	case IMM_HOLD:
		return "IMM_HOLD";
	case IMM_NONMAG:
		return "IMM_NONMAG";
	case IMM_PLUS1:
		return "IMM_PLUS1";
	case IMM_PLUS2:
		return "IMM_PLUS2";
	case IMM_PLUS3:
		return "IMM_PLUS3";
	case IMM_PLUS4:
		return "IMM_PLUS4";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_immunity_type value) {
	switch(value) {
		case IMM_FIRE:
		case IMM_COLD:
		case IMM_ELEC:
		case IMM_ENERGY:
		case IMM_BLUNT:
		case IMM_PIERCE:
		case IMM_SLASH:
		case IMM_ACID:
		case IMM_POISON:
		case IMM_DRAIN:
		case IMM_SLEEP:
		case IMM_CHARM:
		case IMM_HOLD:
		case IMM_NONMAG:
		case IMM_PLUS1:
		case IMM_PLUS2:
		case IMM_PLUS3:
		case IMM_PLUS4:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_immunity_type value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_item_flag encode_e_item_flag(std::string s,e_item_flag fallback) {
	boost::algorithm::to_upper(s);
	if (s == "ITEM_GLOW") return ITEM_GLOW;
	if (s == "ITEM_HUM") return ITEM_HUM;
	if (s == "ITEM_METAL") return ITEM_METAL;
	if (s == "ITEM_MINERAL") return ITEM_MINERAL;
	if (s == "ITEM_ORGANIC") return ITEM_ORGANIC;
	if (s == "ITEM_INVISIBLE") return ITEM_INVISIBLE;
	if (s == "ITEM_MAGIC") return ITEM_MAGIC;
	if (s == "ITEM_NODROP") return ITEM_NODROP;
	if (s == "ITEM_BLESS") return ITEM_BLESS;
	if (s == "ITEM_ANTI_GOOD") return ITEM_ANTI_GOOD;
	if (s == "ITEM_ANTI_EVIL") return ITEM_ANTI_EVIL;
	if (s == "ITEM_ANTI_NEUTRAL") return ITEM_ANTI_NEUTRAL;
	if (s == "ITEM_ANTI_CLERIC") return ITEM_ANTI_CLERIC;
	if (s == "ITEM_ANTI_MAGE") return ITEM_ANTI_MAGE;
	if (s == "ITEM_ANTI_THIEF") return ITEM_ANTI_THIEF;
	if (s == "ITEM_ANTI_FIGHTER") return ITEM_ANTI_FIGHTER;
	if (s == "ITEM_BRITTLE") return ITEM_BRITTLE;
	if (s == "ITEM_RESISTANT") return ITEM_RESISTANT;
	if (s == "ITEM_IMMUNE") return ITEM_IMMUNE;
	if (s == "ITEM_ANTI_MEN") return ITEM_ANTI_MEN;
	if (s == "ITEM_ANTI_WOMEN") return ITEM_ANTI_WOMEN;
	if (s == "ITEM_ANTI_SUN") return ITEM_ANTI_SUN;
	if (s == "ITEM_ANTI_BARBARIAN") return ITEM_ANTI_BARBARIAN;
	if (s == "ITEM_ANTI_RANGER") return ITEM_ANTI_RANGER;
	if (s == "ITEM_ANTI_PALADIN") return ITEM_ANTI_PALADIN;
	if (s == "ITEM_ANTI_PSI") return ITEM_ANTI_PSI;
	if (s == "ITEM_ANTI_MONK") return ITEM_ANTI_MONK;
	if (s == "ITEM_ANTI_DRUID") return ITEM_ANTI_DRUID;
	if (s == "ITEM_ONLY_CLASS") return ITEM_ONLY_CLASS;
	if (s == "ITEM_DIG") return ITEM_DIG;
	if (s == "ITEM_SCYTHE") return ITEM_SCYTHE;
	if (s == "ITEM_ANTI_SORCERER") return ITEM_ANTI_SORCERER;
	return fallback;
}

std::string translate(const e_item_flag e) {
	switch(e) {
	case ITEM_GLOW:
		return "ITEM_GLOW";
	case ITEM_HUM:
		return "ITEM_HUM";
	case ITEM_METAL:
		return "ITEM_METAL";
	case ITEM_MINERAL:
		return "ITEM_MINERAL";
	case ITEM_ORGANIC:
		return "ITEM_ORGANIC";
	case ITEM_INVISIBLE:
		return "ITEM_INVISIBLE";
	case ITEM_MAGIC:
		return "ITEM_MAGIC";
	case ITEM_NODROP:
		return "ITEM_NODROP";
	case ITEM_BLESS:
		return "ITEM_BLESS";
	case ITEM_ANTI_GOOD:
		return "ITEM_ANTI_GOOD";
	case ITEM_ANTI_EVIL:
		return "ITEM_ANTI_EVIL";
	case ITEM_ANTI_NEUTRAL:
		return "ITEM_ANTI_NEUTRAL";
	case ITEM_ANTI_CLERIC:
		return "ITEM_ANTI_CLERIC";
	case ITEM_ANTI_MAGE:
		return "ITEM_ANTI_MAGE";
	case ITEM_ANTI_THIEF:
		return "ITEM_ANTI_THIEF";
	case ITEM_ANTI_FIGHTER:
		return "ITEM_ANTI_FIGHTER";
	case ITEM_BRITTLE:
		return "ITEM_BRITTLE";
	case ITEM_RESISTANT:
		return "ITEM_RESISTANT";
	case ITEM_IMMUNE:
		return "ITEM_IMMUNE";
	case ITEM_ANTI_MEN:
		return "ITEM_ANTI_MEN";
	case ITEM_ANTI_WOMEN:
		return "ITEM_ANTI_WOMEN";
	case ITEM_ANTI_SUN:
		return "ITEM_ANTI_SUN";
	case ITEM_ANTI_BARBARIAN:
		return "ITEM_ANTI_BARBARIAN";
	case ITEM_ANTI_RANGER:
		return "ITEM_ANTI_RANGER";
	case ITEM_ANTI_PALADIN:
		return "ITEM_ANTI_PALADIN";
	case ITEM_ANTI_PSI:
		return "ITEM_ANTI_PSI";
	case ITEM_ANTI_MONK:
		return "ITEM_ANTI_MONK";
	case ITEM_ANTI_DRUID:
		return "ITEM_ANTI_DRUID";
	case ITEM_ONLY_CLASS:
		return "ITEM_ONLY_CLASS";
	case ITEM_DIG:
		return "ITEM_DIG";
	case ITEM_SCYTHE:
		return "ITEM_SCYTHE";
	case ITEM_ANTI_SORCERER:
		return "ITEM_ANTI_SORCERER";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_item_flag value) {
	switch(value) {
		case ITEM_GLOW:
		case ITEM_HUM:
		case ITEM_METAL:
		case ITEM_MINERAL:
		case ITEM_ORGANIC:
		case ITEM_INVISIBLE:
		case ITEM_MAGIC:
		case ITEM_NODROP:
		case ITEM_BLESS:
		case ITEM_ANTI_GOOD:
		case ITEM_ANTI_EVIL:
		case ITEM_ANTI_NEUTRAL:
		case ITEM_ANTI_CLERIC:
		case ITEM_ANTI_MAGE:
		case ITEM_ANTI_THIEF:
		case ITEM_ANTI_FIGHTER:
		case ITEM_BRITTLE:
		case ITEM_RESISTANT:
		case ITEM_IMMUNE:
		case ITEM_ANTI_MEN:
		case ITEM_ANTI_WOMEN:
		case ITEM_ANTI_SUN:
		case ITEM_ANTI_BARBARIAN:
		case ITEM_ANTI_RANGER:
		case ITEM_ANTI_PALADIN:
		case ITEM_ANTI_PSI:
		case ITEM_ANTI_MONK:
		case ITEM_ANTI_DRUID:
		case ITEM_ONLY_CLASS:
		case ITEM_DIG:
		case ITEM_SCYTHE:
		case ITEM_ANTI_SORCERER:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_item_flag value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_item_type encode_e_item_type(std::string s,e_item_type fallback) {
	boost::algorithm::to_upper(s);
	if (s == "ITEM_LIGHT") return ITEM_LIGHT;
	if (s == "ITEM_SCROLL") return ITEM_SCROLL;
	if (s == "ITEM_WAND") return ITEM_WAND;
	if (s == "ITEM_STAFF") return ITEM_STAFF;
	if (s == "ITEM_WEAPON") return ITEM_WEAPON;
	if (s == "ITEM_FIREWEAPON") return ITEM_FIREWEAPON;
	if (s == "ITEM_MISSILE") return ITEM_MISSILE;
	if (s == "ITEM_TREASURE") return ITEM_TREASURE;
	if (s == "ITEM_ARMOR") return ITEM_ARMOR;
	if (s == "ITEM_POTION") return ITEM_POTION;
	if (s == "ITEM_WORN") return ITEM_WORN;
	if (s == "ITEM_OTHER") return ITEM_OTHER;
	if (s == "ITEM_TRASH") return ITEM_TRASH;
	if (s == "ITEM_TRAP") return ITEM_TRAP;
	if (s == "ITEM_CONTAINER") return ITEM_CONTAINER;
	if (s == "ITEM_NOTE") return ITEM_NOTE;
	if (s == "ITEM_DRINKCON") return ITEM_DRINKCON;
	if (s == "ITEM_KEY") return ITEM_KEY;
	if (s == "ITEM_FOOD") return ITEM_FOOD;
	if (s == "ITEM_MONEY") return ITEM_MONEY;
	if (s == "ITEM_PEN") return ITEM_PEN;
	if (s == "ITEM_BOAT") return ITEM_BOAT;
	if (s == "ITEM_AUDIO") return ITEM_AUDIO;
	if (s == "ITEM_BOARD") return ITEM_BOARD;
	if (s == "ITEM_TREE") return ITEM_TREE;
	if (s == "ITEM_ROCK") return ITEM_ROCK;
	return fallback;
}

std::string translate(const e_item_type e) {
	switch(e) {
	case ITEM_LIGHT:
		return "ITEM_LIGHT";
	case ITEM_SCROLL:
		return "ITEM_SCROLL";
	case ITEM_WAND:
		return "ITEM_WAND";
	case ITEM_STAFF:
		return "ITEM_STAFF";
	case ITEM_WEAPON:
		return "ITEM_WEAPON";
	case ITEM_FIREWEAPON:
		return "ITEM_FIREWEAPON";
	case ITEM_MISSILE:
		return "ITEM_MISSILE";
	case ITEM_TREASURE:
		return "ITEM_TREASURE";
	case ITEM_ARMOR:
		return "ITEM_ARMOR";
	case ITEM_POTION:
		return "ITEM_POTION";
	case ITEM_WORN:
		return "ITEM_WORN";
	case ITEM_OTHER:
		return "ITEM_OTHER";
	case ITEM_TRASH:
		return "ITEM_TRASH";
	case ITEM_TRAP:
		return "ITEM_TRAP";
	case ITEM_CONTAINER:
		return "ITEM_CONTAINER";
	case ITEM_NOTE:
		return "ITEM_NOTE";
	case ITEM_DRINKCON:
		return "ITEM_DRINKCON";
	case ITEM_KEY:
		return "ITEM_KEY";
	case ITEM_FOOD:
		return "ITEM_FOOD";
	case ITEM_MONEY:
		return "ITEM_MONEY";
	case ITEM_PEN:
		return "ITEM_PEN";
	case ITEM_BOAT:
		return "ITEM_BOAT";
	case ITEM_AUDIO:
		return "ITEM_AUDIO";
	case ITEM_BOARD:
		return "ITEM_BOARD";
	case ITEM_TREE:
		return "ITEM_TREE";
	case ITEM_ROCK:
		return "ITEM_ROCK";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_item_type value) {
	switch(value) {
		case ITEM_LIGHT:
		case ITEM_SCROLL:
		case ITEM_WAND:
		case ITEM_STAFF:
		case ITEM_WEAPON:
		case ITEM_FIREWEAPON:
		case ITEM_MISSILE:
		case ITEM_TREASURE:
		case ITEM_ARMOR:
		case ITEM_POTION:
		case ITEM_WORN:
		case ITEM_OTHER:
		case ITEM_TRASH:
		case ITEM_TRAP:
		case ITEM_CONTAINER:
		case ITEM_NOTE:
		case ITEM_DRINKCON:
		case ITEM_KEY:
		case ITEM_FOOD:
		case ITEM_MONEY:
		case ITEM_PEN:
		case ITEM_BOAT:
		case ITEM_AUDIO:
		case ITEM_BOARD:
		case ITEM_TREE:
		case ITEM_ROCK:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_item_type value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_item_wear encode_e_item_wear(std::string s,e_item_wear fallback) {
	boost::algorithm::to_upper(s);
	if (s == "ITEM_TAKE") return ITEM_TAKE;
	if (s == "ITEM_WEAR_FINGER") return ITEM_WEAR_FINGER;
	if (s == "ITEM_WEAR_NECK") return ITEM_WEAR_NECK;
	if (s == "ITEM_WEAR_BODY") return ITEM_WEAR_BODY;
	if (s == "ITEM_WEAR_HEAD") return ITEM_WEAR_HEAD;
	if (s == "ITEM_WEAR_LEGS") return ITEM_WEAR_LEGS;
	if (s == "ITEM_WEAR_FEET") return ITEM_WEAR_FEET;
	if (s == "ITEM_WEAR_HANDS") return ITEM_WEAR_HANDS;
	if (s == "ITEM_WEAR_ARMS") return ITEM_WEAR_ARMS;
	if (s == "ITEM_WEAR_SHIELD") return ITEM_WEAR_SHIELD;
	if (s == "ITEM_WEAR_ABOUT") return ITEM_WEAR_ABOUT;
	if (s == "ITEM_WEAR_WAISTE") return ITEM_WEAR_WAISTE;
	if (s == "ITEM_WEAR_WRIST") return ITEM_WEAR_WRIST;
	if (s == "ITEM_WIELD") return ITEM_WIELD;
	if (s == "ITEM_HOLD") return ITEM_HOLD;
	if (s == "ITEM_THROW") return ITEM_THROW;
	if (s == "ITEM_LIGHT_SOURCE") return ITEM_LIGHT_SOURCE;
	if (s == "ITEM_WEAR_BACK") return ITEM_WEAR_BACK;
	if (s == "ITEM_WEAR_EAR") return ITEM_WEAR_EAR;
	if (s == "ITEM_WEAR_EYE") return ITEM_WEAR_EYE;
	return fallback;
}

std::string translate(const e_item_wear e) {
	switch(e) {
	case ITEM_TAKE:
		return "ITEM_TAKE";
	case ITEM_WEAR_FINGER:
		return "ITEM_WEAR_FINGER";
	case ITEM_WEAR_NECK:
		return "ITEM_WEAR_NECK";
	case ITEM_WEAR_BODY:
		return "ITEM_WEAR_BODY";
	case ITEM_WEAR_HEAD:
		return "ITEM_WEAR_HEAD";
	case ITEM_WEAR_LEGS:
		return "ITEM_WEAR_LEGS";
	case ITEM_WEAR_FEET:
		return "ITEM_WEAR_FEET";
	case ITEM_WEAR_HANDS:
		return "ITEM_WEAR_HANDS";
	case ITEM_WEAR_ARMS:
		return "ITEM_WEAR_ARMS";
	case ITEM_WEAR_SHIELD:
		return "ITEM_WEAR_SHIELD";
	case ITEM_WEAR_ABOUT:
		return "ITEM_WEAR_ABOUT";
	case ITEM_WEAR_WAISTE:
		return "ITEM_WEAR_WAISTE";
	case ITEM_WEAR_WRIST:
		return "ITEM_WEAR_WRIST";
	case ITEM_WIELD:
		return "ITEM_WIELD";
	case ITEM_HOLD:
		return "ITEM_HOLD";
	case ITEM_THROW:
		return "ITEM_THROW";
	case ITEM_LIGHT_SOURCE:
		return "ITEM_LIGHT_SOURCE";
	case ITEM_WEAR_BACK:
		return "ITEM_WEAR_BACK";
	case ITEM_WEAR_EAR:
		return "ITEM_WEAR_EAR";
	case ITEM_WEAR_EYE:
		return "ITEM_WEAR_EYE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_item_wear value) {
	switch(value) {
		case ITEM_TAKE:
		case ITEM_WEAR_FINGER:
		case ITEM_WEAR_NECK:
		case ITEM_WEAR_BODY:
		case ITEM_WEAR_HEAD:
		case ITEM_WEAR_LEGS:
		case ITEM_WEAR_FEET:
		case ITEM_WEAR_HANDS:
		case ITEM_WEAR_ARMS:
		case ITEM_WEAR_SHIELD:
		case ITEM_WEAR_ABOUT:
		case ITEM_WEAR_WAISTE:
		case ITEM_WEAR_WRIST:
		case ITEM_WIELD:
		case ITEM_HOLD:
		case ITEM_THROW:
		case ITEM_LIGHT_SOURCE:
		case ITEM_WEAR_BACK:
		case ITEM_WEAR_EAR:
		case ITEM_WEAR_EYE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_item_wear value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_large_flags encode_e_large_flags(std::string s,e_large_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "LARGE_NONE") return LARGE_NONE;
	if (s == "LARGE_WATER") return LARGE_WATER;
	if (s == "LARGE_AIR") return LARGE_AIR;
	if (s == "LARGE_IMPASS") return LARGE_IMPASS;
	return fallback;
}

std::string translate(const e_large_flags e) {
	switch(e) {
	case LARGE_NONE:
		return "LARGE_NONE";
	case LARGE_WATER:
		return "LARGE_WATER";
	case LARGE_AIR:
		return "LARGE_AIR";
	case LARGE_IMPASS:
		return "LARGE_IMPASS";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_large_flags value) {
	switch(value) {
		case LARGE_NONE:
		case LARGE_WATER:
		case LARGE_AIR:
		case LARGE_IMPASS:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_large_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_liquids encode_e_liquids(std::string s,e_liquids fallback) {
	boost::algorithm::to_upper(s);
	if (s == "LIQ_WATER") return LIQ_WATER;
	if (s == "LIQ_BEER") return LIQ_BEER;
	if (s == "LIQ_WINE") return LIQ_WINE;
	if (s == "LIQ_ALE") return LIQ_ALE;
	if (s == "LIQ_DARKALE") return LIQ_DARKALE;
	if (s == "LIQ_WHISKY") return LIQ_WHISKY;
	if (s == "LIQ_LEMONADE") return LIQ_LEMONADE;
	if (s == "LIQ_FIREBRT") return LIQ_FIREBRT;
	if (s == "LIQ_LOCALSPC") return LIQ_LOCALSPC;
	if (s == "LIQ_SLIME") return LIQ_SLIME;
	if (s == "LIQ_MILK") return LIQ_MILK;
	if (s == "LIQ_TEA") return LIQ_TEA;
	if (s == "LIQ_COFFE") return LIQ_COFFE;
	if (s == "LIQ_BLOOD") return LIQ_BLOOD;
	if (s == "LIQ_SALTWATER") return LIQ_SALTWATER;
	if (s == "LIQ_COKE") return LIQ_COKE;
	return fallback;
}

std::string translate(const e_liquids e) {
	switch(e) {
	case LIQ_WATER:
		return "LIQ_WATER";
	case LIQ_BEER:
		return "LIQ_BEER";
	case LIQ_WINE:
		return "LIQ_WINE";
	case LIQ_ALE:
		return "LIQ_ALE";
	case LIQ_DARKALE:
		return "LIQ_DARKALE";
	case LIQ_WHISKY:
		return "LIQ_WHISKY";
	case LIQ_LEMONADE:
		return "LIQ_LEMONADE";
	case LIQ_FIREBRT:
		return "LIQ_FIREBRT";
	case LIQ_LOCALSPC:
		return "LIQ_LOCALSPC";
	case LIQ_SLIME:
		return "LIQ_SLIME";
	case LIQ_MILK:
		return "LIQ_MILK";
	case LIQ_TEA:
		return "LIQ_TEA";
	case LIQ_COFFE:
		return "LIQ_COFFE";
	case LIQ_BLOOD:
		return "LIQ_BLOOD";
	case LIQ_SALTWATER:
		return "LIQ_SALTWATER";
	case LIQ_COKE:
		return "LIQ_COKE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_liquids value) {
	switch(value) {
		case LIQ_WATER:
		case LIQ_BEER:
		case LIQ_WINE:
		case LIQ_ALE:
		case LIQ_DARKALE:
		case LIQ_WHISKY:
		case LIQ_LEMONADE:
		case LIQ_FIREBRT:
		case LIQ_LOCALSPC:
		case LIQ_SLIME:
		case LIQ_MILK:
		case LIQ_TEA:
		case LIQ_COFFE:
		case LIQ_BLOOD:
		case LIQ_SALTWATER:
		case LIQ_COKE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_liquids value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_log_levels encode_e_log_levels(std::string s,e_log_levels fallback) {
	boost::algorithm::to_upper(s);
	if (s == "LOG_SYSERR") return LOG_SYSERR;
	if (s == "LOG_CHECK") return LOG_CHECK;
	if (s == "LOG_PLAYERS") return LOG_PLAYERS;
	if (s == "LOG_MOBILES") return LOG_MOBILES;
	if (s == "LOG_CONNECT") return LOG_CONNECT;
	if (s == "LOG_ERROR") return LOG_ERROR;
	if (s == "LOG_WHO") return LOG_WHO;
	if (s == "LOG_SAVE") return LOG_SAVE;
	if (s == "LOG_MAIL") return LOG_MAIL;
	if (s == "LOG_RANK") return LOG_RANK;
	if (s == "LOG_SILENT") return LOG_SILENT;
	return fallback;
}

std::string translate(const e_log_levels e) {
	switch(e) {
	case LOG_SYSERR:
		return "LOG_SYSERR";
	case LOG_CHECK:
		return "LOG_CHECK";
	case LOG_PLAYERS:
		return "LOG_PLAYERS";
	case LOG_MOBILES:
		return "LOG_MOBILES";
	case LOG_CONNECT:
		return "LOG_CONNECT";
	case LOG_ERROR:
		return "LOG_ERROR";
	case LOG_WHO:
		return "LOG_WHO";
	case LOG_SAVE:
		return "LOG_SAVE";
	case LOG_MAIL:
		return "LOG_MAIL";
	case LOG_RANK:
		return "LOG_RANK";
	case LOG_SILENT:
		return "LOG_SILENT";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_log_levels value) {
	switch(value) {
		case LOG_SYSERR:
		case LOG_CHECK:
		case LOG_PLAYERS:
		case LOG_MOBILES:
		case LOG_CONNECT:
		case LOG_ERROR:
		case LOG_WHO:
		case LOG_SAVE:
		case LOG_MAIL:
		case LOG_RANK:
		case LOG_SILENT:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_log_levels value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_mob_flags encode_e_mob_flags(std::string s,e_mob_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "ACT_SPEC") return ACT_SPEC;
	if (s == "ACT_SENTINEL") return ACT_SENTINEL;
	if (s == "ACT_SCAVENGER") return ACT_SCAVENGER;
	if (s == "ACT_ISNPC") return ACT_ISNPC;
	if (s == "ACT_NICE_THIEF") return ACT_NICE_THIEF;
	if (s == "ACT_AGGRESSIVE") return ACT_AGGRESSIVE;
	if (s == "ACT_STAY_ZONE") return ACT_STAY_ZONE;
	if (s == "ACT_WIMPY") return ACT_WIMPY;
	if (s == "ACT_ANNOYING") return ACT_ANNOYING;
	if (s == "ACT_HATEFUL") return ACT_HATEFUL;
	if (s == "ACT_AFRAID") return ACT_AFRAID;
	if (s == "ACT_IMMORTAL") return ACT_IMMORTAL;
	if (s == "ACT_HUNTING") return ACT_HUNTING;
	if (s == "ACT_DEADLY") return ACT_DEADLY;
	if (s == "ACT_POLYSELF") return ACT_POLYSELF;
	if (s == "ACT_META_AGG") return ACT_META_AGG;
	if (s == "ACT_GUARDIAN") return ACT_GUARDIAN;
	if (s == "ACT_ILLUSION") return ACT_ILLUSION;
	if (s == "ACT_HUGE") return ACT_HUGE;
	if (s == "ACT_SCRIPT") return ACT_SCRIPT;
	if (s == "ACT_GREET") return ACT_GREET;
	if (s == "ACT_MAGIC_USER") return ACT_MAGIC_USER;
	if (s == "ACT_WARRIOR") return ACT_WARRIOR;
	if (s == "ACT_CLERIC") return ACT_CLERIC;
	if (s == "ACT_THIEF") return ACT_THIEF;
	if (s == "ACT_DRUID") return ACT_DRUID;
	if (s == "ACT_MONK") return ACT_MONK;
	if (s == "ACT_BARBARIAN") return ACT_BARBARIAN;
	if (s == "ACT_PALADIN") return ACT_PALADIN;
	if (s == "ACT_RANGER") return ACT_RANGER;
	if (s == "ACT_PSI") return ACT_PSI;
	if (s == "ACT_ARCHER") return ACT_ARCHER;
	return fallback;
}

std::string translate(const e_mob_flags e) {
	switch(e) {
	case ACT_SPEC:
		return "ACT_SPEC";
	case ACT_SENTINEL:
		return "ACT_SENTINEL";
	case ACT_SCAVENGER:
		return "ACT_SCAVENGER";
	case ACT_ISNPC:
		return "ACT_ISNPC";
	case ACT_NICE_THIEF:
		return "ACT_NICE_THIEF";
	case ACT_AGGRESSIVE:
		return "ACT_AGGRESSIVE";
	case ACT_STAY_ZONE:
		return "ACT_STAY_ZONE";
	case ACT_WIMPY:
		return "ACT_WIMPY";
	case ACT_ANNOYING:
		return "ACT_ANNOYING";
	case ACT_HATEFUL:
		return "ACT_HATEFUL";
	case ACT_AFRAID:
		return "ACT_AFRAID";
	case ACT_IMMORTAL:
		return "ACT_IMMORTAL";
	case ACT_HUNTING:
		return "ACT_HUNTING";
	case ACT_DEADLY:
		return "ACT_DEADLY";
	case ACT_POLYSELF:
		return "ACT_POLYSELF";
	case ACT_META_AGG:
		return "ACT_META_AGG";
	case ACT_GUARDIAN:
		return "ACT_GUARDIAN";
	case ACT_ILLUSION:
		return "ACT_ILLUSION";
	case ACT_HUGE:
		return "ACT_HUGE";
	case ACT_SCRIPT:
		return "ACT_SCRIPT";
	case ACT_GREET:
		return "ACT_GREET";
	case ACT_MAGIC_USER:
		return "ACT_MAGIC_USER";
	case ACT_WARRIOR:
		return "ACT_WARRIOR";
	case ACT_CLERIC:
		return "ACT_CLERIC";
	case ACT_THIEF:
		return "ACT_THIEF";
	case ACT_DRUID:
		return "ACT_DRUID";
	case ACT_MONK:
		return "ACT_MONK";
	case ACT_BARBARIAN:
		return "ACT_BARBARIAN";
	case ACT_PALADIN:
		return "ACT_PALADIN";
	case ACT_RANGER:
		return "ACT_RANGER";
	case ACT_PSI:
		return "ACT_PSI";
	case ACT_ARCHER:
		return "ACT_ARCHER";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_mob_flags value) {
	switch(value) {
		case ACT_SPEC:
		case ACT_SENTINEL:
		case ACT_SCAVENGER:
		case ACT_ISNPC:
		case ACT_NICE_THIEF:
		case ACT_AGGRESSIVE:
		case ACT_STAY_ZONE:
		case ACT_WIMPY:
		case ACT_ANNOYING:
		case ACT_HATEFUL:
		case ACT_AFRAID:
		case ACT_IMMORTAL:
		case ACT_HUNTING:
		case ACT_DEADLY:
		case ACT_POLYSELF:
		case ACT_META_AGG:
		case ACT_GUARDIAN:
		case ACT_ILLUSION:
		case ACT_HUGE:
		case ACT_SCRIPT:
		case ACT_GREET:
		case ACT_MAGIC_USER:
		case ACT_WARRIOR:
		case ACT_CLERIC:
		case ACT_THIEF:
		case ACT_DRUID:
		case ACT_MONK:
		case ACT_BARBARIAN:
		case ACT_PALADIN:
		case ACT_RANGER:
		case ACT_PSI:
		case ACT_ARCHER:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_mob_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_opinion_op encode_e_opinion_op(std::string s,e_opinion_op fallback) {
	boost::algorithm::to_upper(s);
	if (s == "OP_SEX") return OP_SEX;
	if (s == "OP_RACE") return OP_RACE;
	if (s == "OP_CHAR") return OP_CHAR;
	if (s == "OP_CLASS") return OP_CLASS;
	if (s == "OP_EVIL") return OP_EVIL;
	if (s == "OP_GOOD") return OP_GOOD;
	if (s == "OP_VNUM") return OP_VNUM;
	return fallback;
}

std::string translate(const e_opinion_op e) {
	switch(e) {
	case OP_SEX:
		return "OP_SEX";
	case OP_RACE:
		return "OP_RACE";
	case OP_CHAR:
		return "OP_CHAR";
	case OP_CLASS:
		return "OP_CLASS";
	case OP_EVIL:
		return "OP_EVIL";
	case OP_GOOD:
		return "OP_GOOD";
	case OP_VNUM:
		return "OP_VNUM";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_opinion_op value) {
	switch(value) {
		case OP_SEX:
		case OP_RACE:
		case OP_CHAR:
		case OP_CLASS:
		case OP_EVIL:
		case OP_GOOD:
		case OP_VNUM:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_opinion_op value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_player_flags encode_e_player_flags(std::string s,e_player_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "PLR_BRIEF") return PLR_BRIEF;
	if (s == "PLR_UNUSED") return PLR_UNUSED;
	if (s == "PLR_COMPACT") return PLR_COMPACT;
	if (s == "PLR_DONTSET") return PLR_DONTSET;
	if (s == "PLR_WIMPY") return PLR_WIMPY;
	if (s == "PLR_NOHASSLE") return PLR_NOHASSLE;
	if (s == "PLR_STEALTH") return PLR_STEALTH;
	if (s == "PLR_HUNTING") return PLR_HUNTING;
	if (s == "PLR_DEAF") return PLR_DEAF;
	if (s == "PLR_ECHO") return PLR_ECHO;
	if (s == "PLR_NOGOSSIP") return PLR_NOGOSSIP;
	if (s == "PLR_NOAUCTION") return PLR_NOAUCTION;
	if (s == "PLR_NOTHING") return PLR_NOTHING;
	if (s == "PLR_NOTHING2") return PLR_NOTHING2;
	if (s == "PLR_NOSHOUT") return PLR_NOSHOUT;
	if (s == "PLR_FREEZE") return PLR_FREEZE;
	if (s == "PLR_NOTELL") return PLR_NOTELL;
	if (s == "PLR_MAILING") return PLR_MAILING;
	if (s == "PLR_EMPTY4") return PLR_EMPTY4;
	if (s == "PLR_EMPTY5") return PLR_EMPTY5;
	if (s == "PLR_NOBEEP") return PLR_NOBEEP;
	return fallback;
}

std::string translate(const e_player_flags e) {
	switch(e) {
	case PLR_BRIEF:
		return "PLR_BRIEF";
	case PLR_UNUSED:
		return "PLR_UNUSED";
	case PLR_COMPACT:
		return "PLR_COMPACT";
	case PLR_DONTSET:
		return "PLR_DONTSET";
	case PLR_WIMPY:
		return "PLR_WIMPY";
	case PLR_NOHASSLE:
		return "PLR_NOHASSLE";
	case PLR_STEALTH:
		return "PLR_STEALTH";
	case PLR_HUNTING:
		return "PLR_HUNTING";
	case PLR_DEAF:
		return "PLR_DEAF";
	case PLR_ECHO:
		return "PLR_ECHO";
	case PLR_NOGOSSIP:
		return "PLR_NOGOSSIP";
	case PLR_NOAUCTION:
		return "PLR_NOAUCTION";
	case PLR_NOTHING:
		return "PLR_NOTHING";
	case PLR_NOTHING2:
		return "PLR_NOTHING2";
	case PLR_NOSHOUT:
		return "PLR_NOSHOUT";
	case PLR_FREEZE:
		return "PLR_FREEZE";
	case PLR_NOTELL:
		return "PLR_NOTELL";
	case PLR_MAILING:
		return "PLR_MAILING";
	case PLR_EMPTY4:
		return "PLR_EMPTY4";
	case PLR_EMPTY5:
		return "PLR_EMPTY5";
	case PLR_NOBEEP:
		return "PLR_NOBEEP";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_player_flags value) {
	switch(value) {
		case PLR_BRIEF:
		case PLR_UNUSED:
		case PLR_COMPACT:
		case PLR_DONTSET:
		case PLR_WIMPY:
		case PLR_NOHASSLE:
		case PLR_STEALTH:
		case PLR_HUNTING:
		case PLR_DEAF:
		case PLR_ECHO:
		case PLR_NOGOSSIP:
		case PLR_NOAUCTION:
		case PLR_NOTHING:
		case PLR_NOTHING2:
		case PLR_NOSHOUT:
		case PLR_FREEZE:
		case PLR_NOTELL:
		case PLR_MAILING:
		case PLR_EMPTY4:
		case PLR_EMPTY5:
		case PLR_NOBEEP:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_player_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_positions encode_e_positions(std::string s,e_positions fallback) {
	boost::algorithm::to_upper(s);
	if (s == "POSITION_DEAD") return POSITION_DEAD;
	if (s == "POSITION_MORTALLYW") return POSITION_MORTALLYW;
	if (s == "POSITION_INCAP") return POSITION_INCAP;
	if (s == "POSITION_STUNNED") return POSITION_STUNNED;
	if (s == "POSITION_SLEEPING") return POSITION_SLEEPING;
	if (s == "POSITION_RESTING") return POSITION_RESTING;
	if (s == "POSITION_SITTING") return POSITION_SITTING;
	if (s == "POSITION_FIGHTING") return POSITION_FIGHTING;
	if (s == "POSITION_STANDING") return POSITION_STANDING;
	if (s == "POSITION_MOUNTED") return POSITION_MOUNTED;
	return fallback;
}

std::string translate(const e_positions e) {
	switch(e) {
	case POSITION_DEAD:
		return "POSITION_DEAD";
	case POSITION_MORTALLYW:
		return "POSITION_MORTALLYW";
	case POSITION_INCAP:
		return "POSITION_INCAP";
	case POSITION_STUNNED:
		return "POSITION_STUNNED";
	case POSITION_SLEEPING:
		return "POSITION_SLEEPING";
	case POSITION_RESTING:
		return "POSITION_RESTING";
	case POSITION_SITTING:
		return "POSITION_SITTING";
	case POSITION_FIGHTING:
		return "POSITION_FIGHTING";
	case POSITION_STANDING:
		return "POSITION_STANDING";
	case POSITION_MOUNTED:
		return "POSITION_MOUNTED";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_positions value) {
	switch(value) {
		case POSITION_DEAD:
		case POSITION_MORTALLYW:
		case POSITION_INCAP:
		case POSITION_STUNNED:
		case POSITION_SLEEPING:
		case POSITION_RESTING:
		case POSITION_SITTING:
		case POSITION_FIGHTING:
		case POSITION_STANDING:
		case POSITION_MOUNTED:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_positions value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_races encode_e_races(std::string s,e_races fallback) {
	boost::algorithm::to_upper(s);
	if (s == "RACE_HALFBREED") return RACE_HALFBREED;
	if (s == "RACE_HUMAN") return RACE_HUMAN;
	if (s == "RACE_ELVEN") return RACE_ELVEN;
	if (s == "RACE_DWARF") return RACE_DWARF;
	if (s == "RACE_HALFLING") return RACE_HALFLING;
	if (s == "RACE_GNOME") return RACE_GNOME;
	if (s == "RACE_REPTILE") return RACE_REPTILE;
	if (s == "RACE_SPECIAL") return RACE_SPECIAL;
	if (s == "RACE_LYCANTH") return RACE_LYCANTH;
	if (s == "RACE_DRAGON") return RACE_DRAGON;
	if (s == "RACE_UNDEAD") return RACE_UNDEAD;
	if (s == "RACE_ORC") return RACE_ORC;
	if (s == "RACE_INSECT") return RACE_INSECT;
	if (s == "RACE_ARACHNID") return RACE_ARACHNID;
	if (s == "RACE_DINOSAUR") return RACE_DINOSAUR;
	if (s == "RACE_FISH") return RACE_FISH;
	if (s == "RACE_BIRD") return RACE_BIRD;
	if (s == "RACE_GIANT") return RACE_GIANT;
	if (s == "RACE_PREDATOR") return RACE_PREDATOR;
	if (s == "RACE_PARASITE") return RACE_PARASITE;
	if (s == "RACE_SLIME") return RACE_SLIME;
	if (s == "RACE_DEMON") return RACE_DEMON;
	if (s == "RACE_SNAKE") return RACE_SNAKE;
	if (s == "RACE_HERBIV") return RACE_HERBIV;
	if (s == "RACE_TREE") return RACE_TREE;
	if (s == "RACE_VEGGIE") return RACE_VEGGIE;
	if (s == "RACE_ELEMENT") return RACE_ELEMENT;
	if (s == "RACE_PLANAR") return RACE_PLANAR;
	if (s == "RACE_DEVIL") return RACE_DEVIL;
	if (s == "RACE_GHOST") return RACE_GHOST;
	if (s == "RACE_GOBLIN") return RACE_GOBLIN;
	if (s == "RACE_TROLL") return RACE_TROLL;
	if (s == "RACE_VEGMAN") return RACE_VEGMAN;
	if (s == "RACE_MFLAYER") return RACE_MFLAYER;
	if (s == "RACE_PRIMATE") return RACE_PRIMATE;
	if (s == "RACE_ENFAN") return RACE_ENFAN;
	if (s == "RACE_DARK_ELF") return RACE_DARK_ELF;
	if (s == "RACE_GOLEM") return RACE_GOLEM;
	if (s == "RACE_SKEXIE") return RACE_SKEXIE;
	if (s == "RACE_TROGMAN") return RACE_TROGMAN;
	if (s == "RACE_PATRYN") return RACE_PATRYN;
	if (s == "RACE_LABRAT") return RACE_LABRAT;
	if (s == "RACE_SARTAN") return RACE_SARTAN;
	if (s == "RACE_TYTAN") return RACE_TYTAN;
	if (s == "RACE_SMURF") return RACE_SMURF;
	if (s == "RACE_ROO") return RACE_ROO;
	if (s == "RACE_HORSE") return RACE_HORSE;
	if (s == "RACE_DRAAGDIM") return RACE_DRAAGDIM;
	if (s == "RACE_ASTRAL") return RACE_ASTRAL;
	if (s == "RACE_GOD") return RACE_GOD;
	if (s == "RACE_GIANT_HILL") return RACE_GIANT_HILL;
	if (s == "RACE_GIANT_FROST") return RACE_GIANT_FROST;
	if (s == "RACE_GIANT_FIRE") return RACE_GIANT_FIRE;
	if (s == "RACE_GIANT_CLOUD") return RACE_GIANT_CLOUD;
	if (s == "RACE_GIANT_STORM") return RACE_GIANT_STORM;
	if (s == "RACE_GIANT_STONE") return RACE_GIANT_STONE;
	if (s == "RACE_DRAGON_RED") return RACE_DRAGON_RED;
	if (s == "RACE_DRAGON_BLACK") return RACE_DRAGON_BLACK;
	if (s == "RACE_DRAGON_GREEN") return RACE_DRAGON_GREEN;
	if (s == "RACE_DRAGON_WHITE") return RACE_DRAGON_WHITE;
	if (s == "RACE_DRAGON_BLUE") return RACE_DRAGON_BLUE;
	if (s == "RACE_DRAGON_SILVER") return RACE_DRAGON_SILVER;
	if (s == "RACE_DRAGON_GOLD") return RACE_DRAGON_GOLD;
	if (s == "RACE_DRAGON_BRONZE") return RACE_DRAGON_BRONZE;
	if (s == "RACE_DRAGON_COPPER") return RACE_DRAGON_COPPER;
	if (s == "RACE_DRAGON_BRASS") return RACE_DRAGON_BRASS;
	if (s == "RACE_UNDEAD_VAMPIRE") return RACE_UNDEAD_VAMPIRE;
	if (s == "RACE_UNDEAD_LICH") return RACE_UNDEAD_LICH;
	if (s == "RACE_UNDEAD_WIGHT") return RACE_UNDEAD_WIGHT;
	if (s == "RACE_UNDEAD_GHAST") return RACE_UNDEAD_GHAST;
	if (s == "RACE_UNDEAD_SPECTRE") return RACE_UNDEAD_SPECTRE;
	if (s == "RACE_UNDEAD_ZOMBIE") return RACE_UNDEAD_ZOMBIE;
	if (s == "RACE_UNDEAD_SKELETON") return RACE_UNDEAD_SKELETON;
	if (s == "RACE_UNDEAD_GHOUL") return RACE_UNDEAD_GHOUL;
	if (s == "RACE_HALF_ELVEN") return RACE_HALF_ELVEN;
	if (s == "RACE_HALF_OGRE") return RACE_HALF_OGRE;
	if (s == "RACE_HALF_ORC") return RACE_HALF_ORC;
	if (s == "RACE_HALF_GIANT") return RACE_HALF_GIANT;
	if (s == "RACE_LIZARDMAN") return RACE_LIZARDMAN;
	if (s == "RACE_DARK_DWARF") return RACE_DARK_DWARF;
	if (s == "RACE_DEEP_GNOME") return RACE_DEEP_GNOME;
	if (s == "RACE_GNOLL") return RACE_GNOLL;
	if (s == "RACE_GOLD_ELF") return RACE_GOLD_ELF;
	if (s == "RACE_WILD_ELF") return RACE_WILD_ELF;
	if (s == "RACE_SEA_ELF") return RACE_SEA_ELF;
	return fallback;
}

std::string translate(const e_races e) {
	switch(e) {
	case RACE_HALFBREED:
		return "RACE_HALFBREED";
	case RACE_HUMAN:
		return "RACE_HUMAN";
	case RACE_ELVEN:
		return "RACE_ELVEN";
	case RACE_DWARF:
		return "RACE_DWARF";
	case RACE_HALFLING:
		return "RACE_HALFLING";
	case RACE_GNOME:
		return "RACE_GNOME";
	case RACE_REPTILE:
		return "RACE_REPTILE";
	case RACE_SPECIAL:
		return "RACE_SPECIAL";
	case RACE_LYCANTH:
		return "RACE_LYCANTH";
	case RACE_DRAGON:
		return "RACE_DRAGON";
	case RACE_UNDEAD:
		return "RACE_UNDEAD";
	case RACE_ORC:
		return "RACE_ORC";
	case RACE_INSECT:
		return "RACE_INSECT";
	case RACE_ARACHNID:
		return "RACE_ARACHNID";
	case RACE_DINOSAUR:
		return "RACE_DINOSAUR";
	case RACE_FISH:
		return "RACE_FISH";
	case RACE_BIRD:
		return "RACE_BIRD";
	case RACE_GIANT:
		return "RACE_GIANT";
	case RACE_PREDATOR:
		return "RACE_PREDATOR";
	case RACE_PARASITE:
		return "RACE_PARASITE";
	case RACE_SLIME:
		return "RACE_SLIME";
	case RACE_DEMON:
		return "RACE_DEMON";
	case RACE_SNAKE:
		return "RACE_SNAKE";
	case RACE_HERBIV:
		return "RACE_HERBIV";
	case RACE_TREE:
		return "RACE_TREE";
	case RACE_VEGGIE:
		return "RACE_VEGGIE";
	case RACE_ELEMENT:
		return "RACE_ELEMENT";
	case RACE_PLANAR:
		return "RACE_PLANAR";
	case RACE_DEVIL:
		return "RACE_DEVIL";
	case RACE_GHOST:
		return "RACE_GHOST";
	case RACE_GOBLIN:
		return "RACE_GOBLIN";
	case RACE_TROLL:
		return "RACE_TROLL";
	case RACE_VEGMAN:
		return "RACE_VEGMAN";
	case RACE_MFLAYER:
		return "RACE_MFLAYER";
	case RACE_PRIMATE:
		return "RACE_PRIMATE";
	case RACE_ENFAN:
		return "RACE_ENFAN";
	case RACE_DARK_ELF:
		return "RACE_DARK_ELF";
	case RACE_GOLEM:
		return "RACE_GOLEM";
	case RACE_SKEXIE:
		return "RACE_SKEXIE";
	case RACE_TROGMAN:
		return "RACE_TROGMAN";
	case RACE_PATRYN:
		return "RACE_PATRYN";
	case RACE_LABRAT:
		return "RACE_LABRAT";
	case RACE_SARTAN:
		return "RACE_SARTAN";
	case RACE_TYTAN:
		return "RACE_TYTAN";
	case RACE_SMURF:
		return "RACE_SMURF";
	case RACE_ROO:
		return "RACE_ROO";
	case RACE_HORSE:
		return "RACE_HORSE";
	case RACE_DRAAGDIM:
		return "RACE_DRAAGDIM";
	case RACE_ASTRAL:
		return "RACE_ASTRAL";
	case RACE_GOD:
		return "RACE_GOD";
	case RACE_GIANT_HILL:
		return "RACE_GIANT_HILL";
	case RACE_GIANT_FROST:
		return "RACE_GIANT_FROST";
	case RACE_GIANT_FIRE:
		return "RACE_GIANT_FIRE";
	case RACE_GIANT_CLOUD:
		return "RACE_GIANT_CLOUD";
	case RACE_GIANT_STORM:
		return "RACE_GIANT_STORM";
	case RACE_GIANT_STONE:
		return "RACE_GIANT_STONE";
	case RACE_DRAGON_RED:
		return "RACE_DRAGON_RED";
	case RACE_DRAGON_BLACK:
		return "RACE_DRAGON_BLACK";
	case RACE_DRAGON_GREEN:
		return "RACE_DRAGON_GREEN";
	case RACE_DRAGON_WHITE:
		return "RACE_DRAGON_WHITE";
	case RACE_DRAGON_BLUE:
		return "RACE_DRAGON_BLUE";
	case RACE_DRAGON_SILVER:
		return "RACE_DRAGON_SILVER";
	case RACE_DRAGON_GOLD:
		return "RACE_DRAGON_GOLD";
	case RACE_DRAGON_BRONZE:
		return "RACE_DRAGON_BRONZE";
	case RACE_DRAGON_COPPER:
		return "RACE_DRAGON_COPPER";
	case RACE_DRAGON_BRASS:
		return "RACE_DRAGON_BRASS";
	case RACE_UNDEAD_VAMPIRE:
		return "RACE_UNDEAD_VAMPIRE";
	case RACE_UNDEAD_LICH:
		return "RACE_UNDEAD_LICH";
	case RACE_UNDEAD_WIGHT:
		return "RACE_UNDEAD_WIGHT";
	case RACE_UNDEAD_GHAST:
		return "RACE_UNDEAD_GHAST";
	case RACE_UNDEAD_SPECTRE:
		return "RACE_UNDEAD_SPECTRE";
	case RACE_UNDEAD_ZOMBIE:
		return "RACE_UNDEAD_ZOMBIE";
	case RACE_UNDEAD_SKELETON:
		return "RACE_UNDEAD_SKELETON";
	case RACE_UNDEAD_GHOUL:
		return "RACE_UNDEAD_GHOUL";
	case RACE_HALF_ELVEN:
		return "RACE_HALF_ELVEN";
	case RACE_HALF_OGRE:
		return "RACE_HALF_OGRE";
	case RACE_HALF_ORC:
		return "RACE_HALF_ORC";
	case RACE_HALF_GIANT:
		return "RACE_HALF_GIANT";
	case RACE_LIZARDMAN:
		return "RACE_LIZARDMAN";
	case RACE_DARK_DWARF:
		return "RACE_DARK_DWARF";
	case RACE_DEEP_GNOME:
		return "RACE_DEEP_GNOME";
	case RACE_GNOLL:
		return "RACE_GNOLL";
	case RACE_GOLD_ELF:
		return "RACE_GOLD_ELF";
	case RACE_WILD_ELF:
		return "RACE_WILD_ELF";
	case RACE_SEA_ELF:
		return "RACE_SEA_ELF";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_races value) {
	switch(value) {
		case RACE_HALFBREED:
		case RACE_HUMAN:
		case RACE_ELVEN:
		case RACE_DWARF:
		case RACE_HALFLING:
		case RACE_GNOME:
		case RACE_REPTILE:
		case RACE_SPECIAL:
		case RACE_LYCANTH:
		case RACE_DRAGON:
		case RACE_UNDEAD:
		case RACE_ORC:
		case RACE_INSECT:
		case RACE_ARACHNID:
		case RACE_DINOSAUR:
		case RACE_FISH:
		case RACE_BIRD:
		case RACE_GIANT:
		case RACE_PREDATOR:
		case RACE_PARASITE:
		case RACE_SLIME:
		case RACE_DEMON:
		case RACE_SNAKE:
		case RACE_HERBIV:
		case RACE_TREE:
		case RACE_VEGGIE:
		case RACE_ELEMENT:
		case RACE_PLANAR:
		case RACE_DEVIL:
		case RACE_GHOST:
		case RACE_GOBLIN:
		case RACE_TROLL:
		case RACE_VEGMAN:
		case RACE_MFLAYER:
		case RACE_PRIMATE:
		case RACE_ENFAN:
		case RACE_DARK_ELF:
		case RACE_GOLEM:
		case RACE_SKEXIE:
		case RACE_TROGMAN:
		case RACE_PATRYN:
		case RACE_LABRAT:
		case RACE_SARTAN:
		case RACE_TYTAN:
		case RACE_SMURF:
		case RACE_ROO:
		case RACE_HORSE:
		case RACE_DRAAGDIM:
		case RACE_ASTRAL:
		case RACE_GOD:
		case RACE_GIANT_HILL:
		case RACE_GIANT_FROST:
		case RACE_GIANT_FIRE:
		case RACE_GIANT_CLOUD:
		case RACE_GIANT_STORM:
		case RACE_GIANT_STONE:
		case RACE_DRAGON_RED:
		case RACE_DRAGON_BLACK:
		case RACE_DRAGON_GREEN:
		case RACE_DRAGON_WHITE:
		case RACE_DRAGON_BLUE:
		case RACE_DRAGON_SILVER:
		case RACE_DRAGON_GOLD:
		case RACE_DRAGON_BRONZE:
		case RACE_DRAGON_COPPER:
		case RACE_DRAGON_BRASS:
		case RACE_UNDEAD_VAMPIRE:
		case RACE_UNDEAD_LICH:
		case RACE_UNDEAD_WIGHT:
		case RACE_UNDEAD_GHAST:
		case RACE_UNDEAD_SPECTRE:
		case RACE_UNDEAD_ZOMBIE:
		case RACE_UNDEAD_SKELETON:
		case RACE_UNDEAD_GHOUL:
		case RACE_HALF_ELVEN:
		case RACE_HALF_OGRE:
		case RACE_HALF_ORC:
		case RACE_HALF_GIANT:
		case RACE_LIZARDMAN:
		case RACE_DARK_DWARF:
		case RACE_DEEP_GNOME:
		case RACE_GNOLL:
		case RACE_GOLD_ELF:
		case RACE_WILD_ELF:
		case RACE_SEA_ELF:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_races value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_room_flags encode_e_room_flags(std::string s,e_room_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "DARK") return DARK;
	if (s == "DEATH") return DEATH;
	if (s == "NO_MOB") return NO_MOB;
	if (s == "INDOORS") return INDOORS;
	if (s == "PEACEFUL") return PEACEFUL;
	if (s == "NOSTEAL") return NOSTEAL;
	if (s == "NO_SUM") return NO_SUM;
	if (s == "NO_MAGIC") return NO_MAGIC;
	if (s == "TUNNEL") return TUNNEL;
	if (s == "PRIVATE") return PRIVATE;
	if (s == "SILENCE") return SILENCE;
	if (s == "LARGE") return LARGE;
	if (s == "NO_DEATH") return NO_DEATH;
	if (s == "SAVE_ROOM") return SAVE_ROOM;
	if (s == "NO_TRACK") return NO_TRACK;
	if (s == "NO_MIND") return NO_MIND;
	if (s == "DESERTIC") return DESERTIC;
	if (s == "ARTIC") return ARTIC;
	if (s == "UNDERGROUND") return UNDERGROUND;
	if (s == "HOT") return HOT;
	if (s == "WET") return WET;
	if (s == "COLD") return COLD;
	if (s == "DRY") return DRY;
	if (s == "BRIGHT") return BRIGHT;
	if (s == "NO_ASTRAL") return NO_ASTRAL;
	if (s == "NO_REGAIN") return NO_REGAIN;
	if (s == "RM_1") return RM_1;
	if (s == "RM_2") return RM_2;
	if (s == "RM_3") return RM_3;
	if (s == "RM_4") return RM_4;
	if (s == "RM_5") return RM_5;
	return fallback;
}

std::string translate(const e_room_flags e) {
	switch(e) {
	case DARK:
		return "DARK";
	case DEATH:
		return "DEATH";
	case NO_MOB:
		return "NO_MOB";
	case INDOORS:
		return "INDOORS";
	case PEACEFUL:
		return "PEACEFUL";
	case NOSTEAL:
		return "NOSTEAL";
	case NO_SUM:
		return "NO_SUM";
	case NO_MAGIC:
		return "NO_MAGIC";
	case TUNNEL:
		return "TUNNEL";
	case PRIVATE:
		return "PRIVATE";
	case SILENCE:
		return "SILENCE";
	case LARGE:
		return "LARGE";
	case NO_DEATH:
		return "NO_DEATH";
	case SAVE_ROOM:
		return "SAVE_ROOM";
	case NO_TRACK:
		return "NO_TRACK";
	case NO_MIND:
		return "NO_MIND";
	case DESERTIC:
		return "DESERTIC";
	case ARTIC:
		return "ARTIC";
	case UNDERGROUND:
		return "UNDERGROUND";
	case HOT:
		return "HOT";
	case WET:
		return "WET";
	case COLD:
		return "COLD";
	case DRY:
		return "DRY";
	case BRIGHT:
		return "BRIGHT";
	case NO_ASTRAL:
		return "NO_ASTRAL";
	case NO_REGAIN:
		return "NO_REGAIN";
	case RM_1:
		return "RM_1";
	case RM_2:
		return "RM_2";
	case RM_3:
		return "RM_3";
	case RM_4:
		return "RM_4";
	case RM_5:
		return "RM_5";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_room_flags value) {
	switch(value) {
		case DARK:
		case DEATH:
		case NO_MOB:
		case INDOORS:
		case PEACEFUL:
		case NOSTEAL:
		case NO_SUM:
		case NO_MAGIC:
		case TUNNEL:
		case PRIVATE:
		case SILENCE:
		case LARGE:
		case NO_DEATH:
		case SAVE_ROOM:
		case NO_TRACK:
		case NO_MIND:
		case DESERTIC:
		case ARTIC:
		case UNDERGROUND:
		case HOT:
		case WET:
		case COLD:
		case DRY:
		case BRIGHT:
		case NO_ASTRAL:
		case NO_REGAIN:
		case RM_1:
		case RM_2:
		case RM_3:
		case RM_4:
		case RM_5:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_room_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_seasons encode_e_seasons(std::string s,e_seasons fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SEASON_WINTER") return SEASON_WINTER;
	if (s == "SEASON_SPRING") return SEASON_SPRING;
	if (s == "SEASON_SUMMER") return SEASON_SUMMER;
	if (s == "SEASON_FALL") return SEASON_FALL;
	return fallback;
}

std::string translate(const e_seasons e) {
	switch(e) {
	case SEASON_WINTER:
		return "SEASON_WINTER";
	case SEASON_SPRING:
		return "SEASON_SPRING";
	case SEASON_SUMMER:
		return "SEASON_SUMMER";
	case SEASON_FALL:
		return "SEASON_FALL";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_seasons value) {
	switch(value) {
		case SEASON_WINTER:
		case SEASON_SPRING:
		case SEASON_SUMMER:
		case SEASON_FALL:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_seasons value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_sector_types encode_e_sector_types(std::string s,e_sector_types fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SECT_INSIDE") return SECT_INSIDE;
	if (s == "SECT_CITY") return SECT_CITY;
	if (s == "SECT_FIELD") return SECT_FIELD;
	if (s == "SECT_FOREST") return SECT_FOREST;
	if (s == "SECT_HILLS") return SECT_HILLS;
	if (s == "SECT_MOUNTAIN") return SECT_MOUNTAIN;
	if (s == "SECT_WATER_SWIM") return SECT_WATER_SWIM;
	if (s == "SECT_WATER_NOSWIM") return SECT_WATER_NOSWIM;
	if (s == "SECT_AIR") return SECT_AIR;
	if (s == "SECT_UNDERWATER") return SECT_UNDERWATER;
	if (s == "SECT_DESERT") return SECT_DESERT;
	if (s == "SECT_TREE") return SECT_TREE;
	if (s == "SECT_DARKCITY") return SECT_DARKCITY;
	return fallback;
}

std::string translate(const e_sector_types e) {
	switch(e) {
	case SECT_INSIDE:
		return "SECT_INSIDE";
	case SECT_CITY:
		return "SECT_CITY";
	case SECT_FIELD:
		return "SECT_FIELD";
	case SECT_FOREST:
		return "SECT_FOREST";
	case SECT_HILLS:
		return "SECT_HILLS";
	case SECT_MOUNTAIN:
		return "SECT_MOUNTAIN";
	case SECT_WATER_SWIM:
		return "SECT_WATER_SWIM";
	case SECT_WATER_NOSWIM:
		return "SECT_WATER_NOSWIM";
	case SECT_AIR:
		return "SECT_AIR";
	case SECT_UNDERWATER:
		return "SECT_UNDERWATER";
	case SECT_DESERT:
		return "SECT_DESERT";
	case SECT_TREE:
		return "SECT_TREE";
	case SECT_DARKCITY:
		return "SECT_DARKCITY";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_sector_types value) {
	switch(value) {
		case SECT_INSIDE:
		case SECT_CITY:
		case SECT_FIELD:
		case SECT_FOREST:
		case SECT_HILLS:
		case SECT_MOUNTAIN:
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		case SECT_AIR:
		case SECT_UNDERWATER:
		case SECT_DESERT:
		case SECT_TREE:
		case SECT_DARKCITY:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_sector_types value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_sex encode_e_sex(std::string s,e_sex fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SEX_NEUTRAL") return SEX_NEUTRAL;
	if (s == "SEX_MALE") return SEX_MALE;
	if (s == "SEX_FEMALE") return SEX_FEMALE;
	return fallback;
}

std::string translate(const e_sex e) {
	switch(e) {
	case SEX_NEUTRAL:
		return "SEX_NEUTRAL";
	case SEX_MALE:
		return "SEX_MALE";
	case SEX_FEMALE:
		return "SEX_FEMALE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_sex value) {
	switch(value) {
		case SEX_NEUTRAL:
		case SEX_MALE:
		case SEX_FEMALE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_sex value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_skill_known encode_e_skill_known(std::string s,e_skill_known fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SKILL_KNOWN") return SKILL_KNOWN;
	if (s == "SKILL_KNOWN_CLERIC") return SKILL_KNOWN_CLERIC;
	if (s == "SKILL_KNOWN_MAGE") return SKILL_KNOWN_MAGE;
	if (s == "SKILL_KNOWN_SORCERER") return SKILL_KNOWN_SORCERER;
	if (s == "SKILL_KNOWN_THIEF") return SKILL_KNOWN_THIEF;
	if (s == "SKILL_KNOWN_MONK") return SKILL_KNOWN_MONK;
	if (s == "SKILL_KNOWN_DRUID") return SKILL_KNOWN_DRUID;
	if (s == "SKILL_KNOWN_WARRIOR") return SKILL_KNOWN_WARRIOR;
	if (s == "SKILL_KNOWN_BARBARIAN") return SKILL_KNOWN_BARBARIAN;
	if (s == "SKILL_KNOWN_PALADIN") return SKILL_KNOWN_PALADIN;
	if (s == "SKILL_KNOWN_RANGER") return SKILL_KNOWN_RANGER;
	if (s == "SKILL_KNOWN_PSI") return SKILL_KNOWN_PSI;
	return fallback;
}

std::string translate(const e_skill_known e) {
	switch(e) {
	case SKILL_KNOWN:
		return "SKILL_KNOWN";
	case SKILL_KNOWN_CLERIC:
		return "SKILL_KNOWN_CLERIC";
	case SKILL_KNOWN_MAGE:
		return "SKILL_KNOWN_MAGE";
	case SKILL_KNOWN_SORCERER:
		return "SKILL_KNOWN_SORCERER";
	case SKILL_KNOWN_THIEF:
		return "SKILL_KNOWN_THIEF";
	case SKILL_KNOWN_MONK:
		return "SKILL_KNOWN_MONK";
	case SKILL_KNOWN_DRUID:
		return "SKILL_KNOWN_DRUID";
	case SKILL_KNOWN_WARRIOR:
		return "SKILL_KNOWN_WARRIOR";
	case SKILL_KNOWN_BARBARIAN:
		return "SKILL_KNOWN_BARBARIAN";
	case SKILL_KNOWN_PALADIN:
		return "SKILL_KNOWN_PALADIN";
	case SKILL_KNOWN_RANGER:
		return "SKILL_KNOWN_RANGER";
	case SKILL_KNOWN_PSI:
		return "SKILL_KNOWN_PSI";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_skill_known value) {
	switch(value) {
		case SKILL_KNOWN:
		case SKILL_KNOWN_CLERIC:
		case SKILL_KNOWN_MAGE:
		case SKILL_KNOWN_SORCERER:
		case SKILL_KNOWN_THIEF:
		case SKILL_KNOWN_MONK:
		case SKILL_KNOWN_DRUID:
		case SKILL_KNOWN_WARRIOR:
		case SKILL_KNOWN_BARBARIAN:
		case SKILL_KNOWN_PALADIN:
		case SKILL_KNOWN_RANGER:
		case SKILL_KNOWN_PSI:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_skill_known value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_sky encode_e_sky(std::string s,e_sky fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SKY_CLOUDLESS") return SKY_CLOUDLESS;
	if (s == "SKY_CLOUDY") return SKY_CLOUDY;
	if (s == "SKY_RAINING") return SKY_RAINING;
	if (s == "SKY_LIGHTNING") return SKY_LIGHTNING;
	return fallback;
}

std::string translate(const e_sky e) {
	switch(e) {
	case SKY_CLOUDLESS:
		return "SKY_CLOUDLESS";
	case SKY_CLOUDY:
		return "SKY_CLOUDY";
	case SKY_RAINING:
		return "SKY_RAINING";
	case SKY_LIGHTNING:
		return "SKY_LIGHTNING";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_sky value) {
	switch(value) {
		case SKY_CLOUDLESS:
		case SKY_CLOUDY:
		case SKY_RAINING:
		case SKY_LIGHTNING:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_sky value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_speaks encode_e_speaks(std::string s,e_speaks fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SPEAK_COMMON") return SPEAK_COMMON;
	if (s == "SPEAK_ELVISH") return SPEAK_ELVISH;
	if (s == "SPEAK_HALFLING") return SPEAK_HALFLING;
	if (s == "SPEAK_DWARVISH") return SPEAK_DWARVISH;
	if (s == "SPEAK_ORCISH") return SPEAK_ORCISH;
	if (s == "SPEAK_GIANTISH") return SPEAK_GIANTISH;
	if (s == "SPEAK_OGRE") return SPEAK_OGRE;
	if (s == "SPEAK_GNOMISH") return SPEAK_GNOMISH;
	return fallback;
}

std::string translate(const e_speaks e) {
	switch(e) {
	case SPEAK_COMMON:
		return "SPEAK_COMMON";
	case SPEAK_ELVISH:
		return "SPEAK_ELVISH";
	case SPEAK_HALFLING:
		return "SPEAK_HALFLING";
	case SPEAK_DWARVISH:
		return "SPEAK_DWARVISH";
	case SPEAK_ORCISH:
		return "SPEAK_ORCISH";
	case SPEAK_GIANTISH:
		return "SPEAK_GIANTISH";
	case SPEAK_OGRE:
		return "SPEAK_OGRE";
	case SPEAK_GNOMISH:
		return "SPEAK_GNOMISH";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_speaks value) {
	switch(value) {
		case SPEAK_COMMON:
		case SPEAK_ELVISH:
		case SPEAK_HALFLING:
		case SPEAK_DWARVISH:
		case SPEAK_ORCISH:
		case SPEAK_GIANTISH:
		case SPEAK_OGRE:
		case SPEAK_GNOMISH:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_speaks value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_special_events encode_e_special_events(std::string s,e_special_events fallback) {
	boost::algorithm::to_upper(s);
	if (s == "DWARVES_STRIKE") return DWARVES_STRIKE;
	if (s == "FAMINE") return FAMINE;
	return fallback;
}

std::string translate(const e_special_events e) {
	switch(e) {
	case DWARVES_STRIKE:
		return "DWARVES_STRIKE";
	case FAMINE:
		return "FAMINE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_special_events value) {
	switch(value) {
		case DWARVES_STRIKE:
		case FAMINE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_special_events value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_sunlight encode_e_sunlight(std::string s,e_sunlight fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SUN_DARK") return SUN_DARK;
	if (s == "SUN_RISE") return SUN_RISE;
	if (s == "SUN_LIGHT") return SUN_LIGHT;
	if (s == "SUN_SET") return SUN_SET;
	if (s == "MOON_SET") return MOON_SET;
	if (s == "MOON_RISE") return MOON_RISE;
	return fallback;
}

std::string translate(const e_sunlight e) {
	switch(e) {
	case SUN_DARK:
		return "SUN_DARK";
	case SUN_RISE:
		return "SUN_RISE";
	case SUN_LIGHT:
		return "SUN_LIGHT";
	case SUN_SET:
		return "SUN_SET";
	case MOON_SET:
		return "MOON_SET";
	case MOON_RISE:
		return "MOON_RISE";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_sunlight value) {
	switch(value) {
		case SUN_DARK:
		case SUN_RISE:
		case SUN_LIGHT:
		case SUN_SET:
		case MOON_SET:
		case MOON_RISE:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_sunlight value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_sys_flags encode_e_sys_flags(std::string s,e_sys_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "SYS_NOPORTAL") return SYS_NOPORTAL;
	if (s == "SYS_NOASTRAL") return SYS_NOASTRAL;
	if (s == "SYS_NOSUMMON") return SYS_NOSUMMON;
	if (s == "SYS_NOKILL") return SYS_NOKILL;
	if (s == "SYS_LOGALL") return SYS_LOGALL;
	if (s == "SYS_ECLIPS") return SYS_ECLIPS;
	if (s == "SYS_SKIPDNS") return SYS_SKIPDNS;
	if (s == "SYS_REQAPPROVE") return SYS_REQAPPROVE;
	if (s == "SYS_NOANSI") return SYS_NOANSI;
	if (s == "SYS_LOGMOB") return SYS_LOGMOB;
	return fallback;
}

std::string translate(const e_sys_flags e) {
	switch(e) {
	case SYS_NOPORTAL:
		return "SYS_NOPORTAL";
	case SYS_NOASTRAL:
		return "SYS_NOASTRAL";
	case SYS_NOSUMMON:
		return "SYS_NOSUMMON";
	case SYS_NOKILL:
		return "SYS_NOKILL";
	case SYS_LOGALL:
		return "SYS_LOGALL";
	case SYS_ECLIPS:
		return "SYS_ECLIPS";
	case SYS_SKIPDNS:
		return "SYS_SKIPDNS";
	case SYS_REQAPPROVE:
		return "SYS_REQAPPROVE";
	case SYS_NOANSI:
		return "SYS_NOANSI";
	case SYS_LOGMOB:
		return "SYS_LOGMOB";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_sys_flags value) {
	switch(value) {
		case SYS_NOPORTAL:
		case SYS_NOASTRAL:
		case SYS_NOSUMMON:
		case SYS_NOKILL:
		case SYS_LOGALL:
		case SYS_ECLIPS:
		case SYS_SKIPDNS:
		case SYS_REQAPPROVE:
		case SYS_NOANSI:
		case SYS_LOGMOB:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_sys_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_tan_result encode_e_tan_result(std::string s,e_tan_result fallback) {
	boost::algorithm::to_upper(s);
	if (s == "TAN_SHIELD") return TAN_SHIELD;
	if (s == "TAN_JACKET") return TAN_JACKET;
	if (s == "TAN_BOOTS") return TAN_BOOTS;
	if (s == "TAN_GLOVES") return TAN_GLOVES;
	if (s == "TAN_LEGGINGS") return TAN_LEGGINGS;
	if (s == "TAN_SLEEVES") return TAN_SLEEVES;
	if (s == "TAN_HELMET") return TAN_HELMET;
	if (s == "TAN_BAG") return TAN_BAG;
	if (s == "TAN_ARMOR") return TAN_ARMOR;
	return fallback;
}

std::string translate(const e_tan_result e) {
	switch(e) {
	case TAN_SHIELD:
		return "TAN_SHIELD";
	case TAN_JACKET:
		return "TAN_JACKET";
	case TAN_BOOTS:
		return "TAN_BOOTS";
	case TAN_GLOVES:
		return "TAN_GLOVES";
	case TAN_LEGGINGS:
		return "TAN_LEGGINGS";
	case TAN_SLEEVES:
		return "TAN_SLEEVES";
	case TAN_HELMET:
		return "TAN_HELMET";
	case TAN_BAG:
		return "TAN_BAG";
	case TAN_ARMOR:
		return "TAN_ARMOR";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_tan_result value) {
	switch(value) {
		case TAN_SHIELD:
		case TAN_JACKET:
		case TAN_BOOTS:
		case TAN_GLOVES:
		case TAN_LEGGINGS:
		case TAN_SLEEVES:
		case TAN_HELMET:
		case TAN_BAG:
		case TAN_ARMOR:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_tan_result value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_teleports encode_e_teleports(std::string s,e_teleports fallback) {
	boost::algorithm::to_upper(s);
	if (s == "TELE_LOOK") return TELE_LOOK;
	if (s == "TELE_COUNT") return TELE_COUNT;
	if (s == "TELE_RANDOM") return TELE_RANDOM;
	if (s == "TELE_SPIN") return TELE_SPIN;
	return fallback;
}

std::string translate(const e_teleports e) {
	switch(e) {
	case TELE_LOOK:
		return "TELE_LOOK";
	case TELE_COUNT:
		return "TELE_COUNT";
	case TELE_RANDOM:
		return "TELE_RANDOM";
	case TELE_SPIN:
		return "TELE_SPIN";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_teleports value) {
	switch(value) {
		case TELE_LOOK:
		case TELE_COUNT:
		case TELE_RANDOM:
		case TELE_SPIN:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_teleports value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_user_flags encode_e_user_flags(std::string s,e_user_flags fallback) {
	boost::algorithm::to_upper(s);
	if (s == "NO_DELETE") return NO_DELETE;
	if (s == "USE_ANSI") return USE_ANSI;
	if (s == "RACE_WAR") return RACE_WAR;
	if (s == "SHOW_CLASSES") return SHOW_CLASSES;
	if (s == "SHOW_EXITS") return SHOW_EXITS;
	if (s == "MURDER_1") return MURDER_1;
	if (s == "STOLE_1") return STOLE_1;
	if (s == "MURDER_2") return MURDER_2;
	if (s == "STOLE_2") return STOLE_2;
	if (s == "STOLE_X") return STOLE_X;
	if (s == "MURDER_X") return MURDER_X;
	if (s == "USE_PAGING") return USE_PAGING;
	if (s == "CAN_OBJ_EDIT") return CAN_OBJ_EDIT;
	if (s == "CAN_MOB_EDIT") return CAN_MOB_EDIT;
	return fallback;
}

std::string translate(const e_user_flags e) {
	switch(e) {
	case NO_DELETE:
		return "NO_DELETE";
	case USE_ANSI:
		return "USE_ANSI";
	case RACE_WAR:
		return "RACE_WAR";
	case SHOW_CLASSES:
		return "SHOW_CLASSES";
	case SHOW_EXITS:
		return "SHOW_EXITS";
	case MURDER_1:
		return "MURDER_1";
	case STOLE_1:
		return "STOLE_1";
	case MURDER_2:
		return "MURDER_2";
	case STOLE_2:
		return "STOLE_2";
	case STOLE_X:
		return "STOLE_X";
	case MURDER_X:
		return "MURDER_X";
	case USE_PAGING:
		return "USE_PAGING";
	case CAN_OBJ_EDIT:
		return "CAN_OBJ_EDIT";
	case CAN_MOB_EDIT:
		return "CAN_MOB_EDIT";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_user_flags value) {
	switch(value) {
		case NO_DELETE:
		case USE_ANSI:
		case RACE_WAR:
		case SHOW_CLASSES:
		case SHOW_EXITS:
		case MURDER_1:
		case STOLE_1:
		case MURDER_2:
		case STOLE_2:
		case STOLE_X:
		case MURDER_X:
		case USE_PAGING:
		case CAN_OBJ_EDIT:
		case CAN_MOB_EDIT:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_user_flags value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
e_wear encode_e_wear(std::string s,e_wear fallback) {
	boost::algorithm::to_upper(s);
	if (s == "WEAR_LIGHT") return WEAR_LIGHT;
	if (s == "WEAR_FINGER_R") return WEAR_FINGER_R;
	if (s == "WEAR_FINGER_L") return WEAR_FINGER_L;
	if (s == "WEAR_NECK_1") return WEAR_NECK_1;
	if (s == "WEAR_NECK_2") return WEAR_NECK_2;
	if (s == "WEAR_BODY") return WEAR_BODY;
	if (s == "WEAR_HEAD") return WEAR_HEAD;
	if (s == "WEAR_LEGS") return WEAR_LEGS;
	if (s == "WEAR_FEET") return WEAR_FEET;
	if (s == "WEAR_HANDS") return WEAR_HANDS;
	if (s == "WEAR_ARMS") return WEAR_ARMS;
	if (s == "WEAR_SHIELD") return WEAR_SHIELD;
	if (s == "WEAR_ABOUT") return WEAR_ABOUT;
	if (s == "WEAR_WAISTE") return WEAR_WAISTE;
	if (s == "WEAR_WRIST_R") return WEAR_WRIST_R;
	if (s == "WEAR_WRIST_L") return WEAR_WRIST_L;
	if (s == "WIELD") return WIELD;
	if (s == "HOLD") return HOLD;
	if (s == "WEAR_BACK") return WEAR_BACK;
	if (s == "WEAR_EAR_R") return WEAR_EAR_R;
	if (s == "WEAR_EAR_L") return WEAR_EAR_L;
	if (s == "WEAR_EYES") return WEAR_EYES;
	if (s == "LOADED_WEAPON") return LOADED_WEAPON;
	return fallback;
}

std::string translate(const e_wear e) {
	switch(e) {
	case WEAR_LIGHT:
		return "WEAR_LIGHT";
	case WEAR_FINGER_R:
		return "WEAR_FINGER_R";
	case WEAR_FINGER_L:
		return "WEAR_FINGER_L";
	case WEAR_NECK_1:
		return "WEAR_NECK_1";
	case WEAR_NECK_2:
		return "WEAR_NECK_2";
	case WEAR_BODY:
		return "WEAR_BODY";
	case WEAR_HEAD:
		return "WEAR_HEAD";
	case WEAR_LEGS:
		return "WEAR_LEGS";
	case WEAR_FEET:
		return "WEAR_FEET";
	case WEAR_HANDS:
		return "WEAR_HANDS";
	case WEAR_ARMS:
		return "WEAR_ARMS";
	case WEAR_SHIELD:
		return "WEAR_SHIELD";
	case WEAR_ABOUT:
		return "WEAR_ABOUT";
	case WEAR_WAISTE:
		return "WEAR_WAISTE";
	case WEAR_WRIST_R:
		return "WEAR_WRIST_R";
	case WEAR_WRIST_L:
		return "WEAR_WRIST_L";
	case WIELD:
		return "WIELD";
	case HOLD:
		return "HOLD";
	case WEAR_BACK:
		return "WEAR_BACK";
	case WEAR_EAR_R:
		return "WEAR_EAR_R";
	case WEAR_EAR_L:
		return "WEAR_EAR_L";
	case WEAR_EYES:
		return "WEAR_EYES";
	case LOADED_WEAPON:
		return "LOADED_WEAPON";
	default:
		return "UNKNOWN";
	}
}
bool enum_validate(const e_wear value) {
	switch(value) {
		case WEAR_LIGHT:
		case WEAR_FINGER_R:
		case WEAR_FINGER_L:
		case WEAR_NECK_1:
		case WEAR_NECK_2:
		case WEAR_BODY:
		case WEAR_HEAD:
		case WEAR_LEGS:
		case WEAR_FEET:
		case WEAR_HANDS:
		case WEAR_ARMS:
		case WEAR_SHIELD:
		case WEAR_ABOUT:
		case WEAR_WAISTE:
		case WEAR_WRIST_R:
		case WEAR_WRIST_L:
		case WIELD:
		case HOLD:
		case WEAR_BACK:
		case WEAR_EAR_R:
		case WEAR_EAR_L:
		case WEAR_EYES:
		case LOADED_WEAPON:
		return true;
		default:
		return false;
	}
}
std::ostream & operator<<(std::ostream &out,const e_wear value) {
	out << value << "(" << G::translate(value) << ")";
	return out;
}
} // namespace G
}//namespace gates

