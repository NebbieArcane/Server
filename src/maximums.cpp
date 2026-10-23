/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: maximums.c,v 1.2 2002/02/13 12:30:58 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <vector>
#include <string_view>
#include <numeric>
#include <optional>
#include <utility>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "utility.hpp"
/***************************  Local    include ************************************/
#include "maximums.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

/* struct room_data *real_roomp(int); */

/*ALAR modificato. Invecchiando il
 * mana gain diventa SEMPRE +
 * alto. In compenso ho peggiorato
 * ancora sia il max, che il
 * gain di hp
 * */

char* ClassTitles(struct char_data* ch) {
	unsigned int i, count=0;
	static char buf[256];

	for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
		int nLev = GET_LEVEL(ch, i);
		if(nLev) {
			count++;
			if(count > 1) {
				sprintf(buf + strlen(buf), "/%s", GET_CLASS_TITLE(ch, i, nLev));
			}
			else {
				sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, nLev));
			}
		}
	}
	return(buf);
}


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6) {

	if(age < 15) {
		return(p0);    /* < 15   */
	}
	else if(age <= 29) {
		return (int)(p1+(((age-15)*(p2-p1))/15));    /* 15..29 */
	}
	else if(age <= 44) {
		return (int)(p2+(((age-30)*(p3-p2))/15));    /* 30..44 */
	}
	else if(age <= 59) {
		return (int)(p3+(((age-45)*(p4-p3))/15));    /* 45..59 */
	}
	else if(age <= 79) {
		return (int)(p4+(((age-60)*(p5-p4))/20));    /* 60..79 */
	}
	else {
		return(p6);    /* >= 80 */
	}
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data* ch) {
	int max, tmp, chClasses;

	max = 0;

	if(!IS_PC(ch)) {
		return(100);
	}
	if(HasClass(ch, CLASS_MAGIC_USER)) {
		max += 100;
		max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
	}

	/* actually this is worthless as Sorcerer's do not */
	/* use mana at all.... */
	if(HasClass(ch, CLASS_SORCERER)) {
		max += 100;
		max += GET_LEVEL(ch, SORCERER_LEVEL_IND) * 5;
	}

	if(HasClass(ch, CLASS_PSI)) {
		max += 100;
		max += GET_LEVEL(ch, PSI_LEVEL_IND) * 5;
	}


	if(HasClass(ch, CLASS_PALADIN)) {
		max += 100;
		max += (GET_LEVEL(ch, PALADIN_LEVEL_IND)/4) * 5;
	}

	if(HasClass(ch, CLASS_RANGER)) {
		max += 100;
		max += (GET_LEVEL(ch, RANGER_LEVEL_IND)/4) * 5;
	}

	if(HasClass(ch, CLASS_CLERIC)) {
		max += 100;
		max += (GET_LEVEL(ch, CLERIC_LEVEL_IND)/3) * 5;
	}

	if(HasClass(ch, CLASS_DRUID)) {
		max += 100;
		max += (GET_LEVEL(ch, DRUID_LEVEL_IND)/3) * 5;
	}

	if(HasClass(ch, CLASS_THIEF)) {
		max += 100;
	}

	if(HasClass(ch, CLASS_WARRIOR)) {
		max += 100;
	}

	if(HasClass(ch, CLASS_MONK)) {
		max += 100;
	}

	if(HasClass(ch, CLASS_BARBARIAN)) {
		max += 100;
	}

	chClasses = HowManyClasses(ch);

	if(chClasses) {
		max /= HowManyClasses(ch);
	}
	else {
		mudlog(LOG_ERROR, "Character with no levels! Name [%s]", ch->player.name);
	}


	/*
	 * new classes should be inserted here.
	 */

	tmp = 0;


	tmp = GET_INT(ch)/3;
	tmp += 2;
	tmp = tmp*3;

	max += tmp;
	max += ch->points.max_mana;
	/* bonus mana. In questo campo vanno gli affect  */
	max += ch->edit_pool.edit_mana;

	struct time_info_data ma;
	age2(ch, &ma);
	max += (graf(ma.year, 6,8,12,16,20,24,28))-10;

	/*
	 * Add class mana maximums here...
	 */

//	rimosso il limite di 100 mana ai barbari
//	if(OnlyClass(ch,CLASS_BARBARIAN)) {  /* 100 mana max for barbs */
//		max=100;  /* barbarians only get 100 mana... */
//	}

	return(max);
}


int hit_limit(struct char_data* ch) {
	int max;

	if(IS_PC(ch)) {
		struct time_info_data ma;
		age2(ch, &ma);
		max = (ch->points.max_hit) + (graf(ma.year, 2,4,17,14,8,-5,-15));
		max += ch->edit_pool.edit_hp;

	}
	else {
		max = (ch->points.max_hit);
	}

	/* Class/Level calculations */

	/* Skill/Spell calculations */

	return (max);
}


int move_limit(struct char_data* ch) {
	int max;

	if(IS_PC(ch)) {
		max = 100 ;
	}
	else {
		max = ch->points.max_move;
		if(IsRideable(ch)) {
			max *= 2;
		}
		return(max);
	}
	struct time_info_data ma;
	age2(ch, &ma);
	max -= (graf(ma.year, -4,0,2,6,10,14,18));



	if(GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_GNOME) {
		max -= 35;
	}
	else if(GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_DARK_ELF  ||
			GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
			GET_RACE(ch) == RACE_SEA_ELF ||
			GET_RACE(ch)== RACE_HALF_ELVEN) {
		max += 20;
	}
	else if(GET_CLASS(ch) == CLASS_BARBARIAN) {
		max +=45;    /* barbs get more move ... */
	}
	else if(GET_RACE(ch) == RACE_HALFLING) {
		max -= 45 ;
	}
	else if(GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL) {
		max +=60;
	}
	else if(GET_RACE(ch) == RACE_HALF_OGRE) {
		max +=50        ;
	}

	max += ch->points.max_move;  /* move bonus */
	max += ch->edit_pool.edit_move;

	return (max);
}


/* manapoint gain pr. game hour */
int mana_gain(struct char_data* ch) {
	int gain;

	if(!IS_PC(ch)) {
		/* Neat and fast */
		gain = 8;
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
#ifdef NEW_GAIN
		gain = graf(ma.year, 3,9,12,16,20,24,30);
#else
		gain = graf(ma.year, 2,4,6,8,10,16,2);
#endif
	}

	/* Position calculations    */
	/*switch (GET_POS_PREV(ch))*/
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += gain;
		break;
	case POSITION_RESTING:
		gain+= (gain>>1);  /* Divide by 2 */
		break;
	case POSITION_SITTING:
		gain += (gain>>2); /* Divide by 4 */
		break;
	}

	gain += gain;

	gain += wis_app[(int)GET_WIS(ch) ].bonus*2;

	gain += ch->points.mana_gain;
	gain += ch->eq_regen.mana;
	gain += ch->edit_pool.edit_mana_regen;

	if(IS_AFFECTED(ch, AFF_POISON)) {
		gain >>= 2;
	}

	if(GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0) {
		gain >>= 2;
	}

	if(GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_GNOME ||
			GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
			GET_RACE(ch) == RACE_SEA_ELF ||
			GET_RACE(ch) == RACE_DEMON ||
			GET_RACE(ch) == RACE_DARK_ELF || GET_RACE(ch) == RACE_HALF_ELVEN) {
		gain+=2;
	}

	if(GET_COND(ch, DRUNK) > 10) {
		gain += (gain >> 1);
	}
	else if(GET_COND(ch, DRUNK) > 0) {
		gain += (gain >> 2);
	}

	/* Class calculations */

	/* magic type people get quicker mana re-gen, fighter/paladin/rangers get it slower */
	if(!HasClass(ch, CLASS_MAGIC_USER | CLASS_SORCERER | CLASS_CLERIC |
				 CLASS_DRUID | CLASS_PSI)) {
		gain -= 2;
	}
	/* these guys get mana even slower */
	if(HasClass(ch, CLASS_BARBARIAN)) {
		gain -= 2;
	}

	/* Skill/Spell calculations */

	if(affected_by_spell(ch, SKILL_MEDITATE)) {
		gain += 3*GET_LEVEL(ch, PSI_LEVEL_IND)/10 ;
	}

	return(gain);
}

int hit_gain(struct char_data* ch) // Gaia 2001
/* Hitpoint gain pr. game hour */
{

	int gain, dam, i;

	if(IS_NPC(ch)) {
		gain = 8;
		/* Neat and fast */
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
#ifdef NEW_GAIN

		gain = graf(ma.year, 3,9,12,16,12,6,0);
#else
		gain = graf(ma.year, 2,4,6,8,6,3,1);
#endif
		/*if (GET_POS_PREV(ch) == POSITION_FIGHTING)*/
		if(GET_POS(ch) == POSITION_FIGHTING) {
#if defined( ALAR )
			gain = ((GET_RACE(ch) == RACE_TROLL)?gain:0);
#else
			gain = 0;
#endif
		}
	}


	/* Position calculations    */

	/*switch (GET_POS_PREV(ch))*/
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += gain>>2;
		break;
	case POSITION_RESTING:
		gain+= gain>>3;
		break;
	case POSITION_SITTING:
		gain += gain>>4;
		break;
	}


	if(GET_RACE(ch) == RACE_DWARF) {
		gain += 2;
	}

	if(GET_RACE(ch) == RACE_HALFLING) {
		gain += 1;
	}
#if defined (ALAR)
	if(GET_RACE(ch) == RACE_TROLL) {
		gain += 5;
	}
#endif
	if(GET_RACE(ch) == RACE_HALF_GIANT) { /* faster hps regen */
		gain+=3;
	}
	if(GET_RACE(ch) == RACE_HALF_OGRE) {
		gain +=2;
	}
	if(GET_RACE(ch) == RACE_HALF_ORC) {
		gain +=1;
	}

	if(GET_CLASS(ch) == CLASS_BARBARIAN) {
		gain += 4;    /* barbs gain hits faster... */
	}

	gain += con_app[(int)GET_CON(ch) ].hitp/2;

	gain += ch->points.hit_gain;
	gain += ch->eq_regen.hit;
	gain += ch->edit_pool.edit_hp_regen;

	if(GET_COND(ch,FULL) == 0 || GET_COND(ch, THIRST) == 0) {
		gain >>= 4;
	}

	if(GET_COND(ch, DRUNK)>10) {
		gain += (gain >> 1);
	}
	else if(GET_COND(ch, DRUNK)>0) {
		gain += (gain >> 2);
	}

	/* Class/Level calculations */
	/* non-warrior types get slower hps re-gen */
	if(IS_PC(ch) &&
			!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN |
					  CLASS_RANGER | CLASS_BARBARIAN) &&
			gain>2) {
		gain -=2;
	}

	if(IS_AFFECTED(ch, AFF_POISON)) {
		gain = 0;
		dam = number(10,32);
		if(GET_RACE(ch) == RACE_HALFLING) {
			dam = number(1,20);
		}
		/*I nani sono i piu' coriacei...*/
		if(GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_DARK_DWARF) {
			dam = number(1,10);
		}
		if(affected_by_spell(ch, SPELL_SLOW_POISON)) {
			dam /= 4;
		}
		/*  if( damage( ch, ch, dam, SPELL_POISON, location ) == VictimDead )
		   	return 0;   Elimino la chiamata a damage e lo metto come modificatore
		                sull HP-regen in modo che scali in continuo
		                inoltre ho potenziato lo spell con un *5  Gaia 2001 */
		gain -= dam*5 ;
	}

	if(IS_AFFECTED2(ch, AFF2_HEAT_STUFF)) {
		dam = 0;
		/*
		count items in eq
		*/
		for(i=0; i<=HOLD; i++) {
			if(ch->equipment[i]) {
				dam += 15;   /* Potenziato, utile per un pkill.. Gaia 2001 */
			}
		}

		/*  if( damage( ch, ch, dam, SPELL_HEAT_STUFF, location ) == VictimDead )
		   	return 0;   Elimino la chiamata a damage e lo metto come modificatore
		                sull HP-regen in modo che scali in continuo Gaia 2001 */
		gain -= dam ;
	}

	/*  if( gain < 0 && !ch->specials.fighting )
	  damage( ch, ch, gain * -1, TYPE_SUFFERING, location );
	  lo commento perche' il damage me lo vado a vedere solo se faccio
	  un regen event, anche se e' vero che devo stare attento alle no regen.
	  Gaia 2001 */
	/* Skill/Spell calculations */
	if(affected_by_spell(ch, SKILL_MEDITATE)) {
		gain += 3*GET_LEVEL(ch, PSI_LEVEL_IND)/10 ;
	}

	return(gain);
}



int move_gain(struct char_data* ch)
/* move gain pr. game hour */
{
	int gain;

	if(IS_NPC(ch)) {
		gain = 22;
		if(IsRideable(ch)) {
			gain += gain/2;
		}

		/* Neat and fast */
	}
	else {
		struct time_info_data ma;
		age2(ch, &ma);
		if(GET_POS(ch) != POSITION_FIGHTING)
#ifdef NEW_GAIN
			gain = graf(ma.year, 15,21,25,28,20,7,1);
#else
			gain = graf(ma.year, 10,15,20,22,15,7,1);
#endif

		else {
			gain = 0;
		}
	}

	/* Position calculations    */
	/*switch (GET_POS_PREV(ch))*/
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING:
		gain += (gain>>2); /* Divide by 4 */
		break;
	case POSITION_RESTING:
		gain+= (gain>>3);  /* Divide by 8 */
		break;
	case POSITION_SITTING:
		gain += (gain>>4); /* Divide by 16 */
		break;
	}


	if(GET_RACE(ch) == RACE_DWARF) {
		gain += 4;
	}

	if(GET_RACE(ch) == RACE_HALF_GIANT) {
		gain +=6;
	}
	if(GET_RACE(ch) == RACE_HALF_OGRE) {
		gain +=5;
	}
	if(GET_RACE(ch) == RACE_HALF_ORC) {
		gain +=4;
	}

	gain += ch->points.move_gain;
	gain += ch->eq_regen.move;
	gain += ch->edit_pool.edit_move_regen;

	if(IS_AFFECTED(ch,AFF_POISON)) {
		gain >>= 5;
	}

	if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0)) {
		gain >>= 3;
	}

	/* Class specific stuff */

	/* non-thief/monks types regen move slower */
	if(!HasClass(ch,CLASS_THIEF|CLASS_MONK)) {
		gain -= 2;
	}

	return (gain);
}

namespace {

/* Dado HP per livello di una classe: sotto `threshold` [low_min, low_max], poi [high_min, high_max]. */
struct HpBand {
	int threshold;
	int low_min;
	int low_max;
	int high_min;
	int high_max;
};

[[nodiscard]] std::optional<HpBand> hp_band_for(int class_index) {
	switch(class_index) {
	case MAGE_LEVEL_IND:
	case SORCERER_LEVEL_IND:
		return HpBand{12, 2, 6, 1, 2};
	case CLERIC_LEVEL_IND:
		return HpBand{12, 3, 15, 1, 6};
	case THIEF_LEVEL_IND:
	case PSI_LEVEL_IND:
		return HpBand{12, 2, 10, 1, 4};
	case WARRIOR_LEVEL_IND:
	case PALADIN_LEVEL_IND:
		return HpBand{10, 3, 16, 2, 8};
	case RANGER_LEVEL_IND:
		return HpBand{11, 3, 13, 2, 8};
	case BARBARIAN_LEVEL_IND:
		return HpBand{15, 3, 22, 3, 12};
	case DRUID_LEVEL_IND:
		return HpBand{15, 2, 14, 1, 6};
	case MONK_LEVEL_IND:
		return HpBand{17, 2, 10, 1, 5};
	default:
		return std::nullopt;
	}
}

[[nodiscard]] bool full_con_bonus_class(int class_index) {
	return class_index == WARRIOR_LEVEL_IND || class_index == BARBARIAN_LEVEL_IND ||
		   class_index == PALADIN_LEVEL_IND || class_index == RANGER_LEVEL_IND;
}

/* Bonus costituzione per livello: pieno per i combattenti, al massimo 3 per gli altri. */
[[nodiscard]] int con_hp_bonus(int con, int class_index) {
	const int hitp = con_app[std::clamp(con, 0, 25)].hitp;
	return full_con_bonus_class(class_index) ? hitp : std::min(hitp, 3);
}

/* PRINCIPE arriva da check_prince_promotion senza HP: mai una riga per quel livello. */
[[nodiscard]] bool valid_level_hp_cell(int class_index, int level) {
	return class_index >= 0 && class_index < MAX_CLASS && level > 0 && level < ABS_MAX_LVL && level != PRINCIPE;
}

/* Livello piu' alto con una riga di storico: PRINCIPE vale come BARONE. */
[[nodiscard]] int level_hp_top_row(int level) {
	return std::min({level, static_cast<int>(BARONE), ABS_MAX_LVL - 1});
}

/* Livello ancora posseduto: le righe sopra il livello attuale sono livelli persi. */
[[nodiscard]] bool level_hp_held(const struct char_data* ch, int class_index, int level) {
	return level <= GET_LEVEL(ch, class_index);
}

/* Media attesa di GetHpGain per (classe, livello), almeno 1: peso della baseline. */
[[nodiscard]] int expected_level_gain(int class_index, int level, int con, int classes) {
	int twice = 2 * con_hp_bonus(con, class_index); /* doppio per non perdere i mezzi punti */
	if(const auto band = hp_band_for(class_index)) {
		twice += level < band->threshold ? band->low_min + band->low_max
										 : band->high_min + band->high_max;
	}
	return std::max(1, twice / (2 * classes));
}

/* HP iniziali medi di roll_abilities: classi*10 + razza + bonus di classe, non divisi (arrotondati). */
[[nodiscard]] int estimated_start_hp(const int* levels, int classes, int race) {
	const auto has = [levels](int class_index) { return levels[class_index] > 0; };
	int tenths = (classes * 10 + creation_race_hp_bonus(race)) * 10;
	if(has(BARBARIAN_LEVEL_IND)) {
		tenths += 100;
	}
	if(has(MAGE_LEVEL_IND)) {
		tenths += 25;
	}
	if(has(SORCERER_LEVEL_IND)) {
		tenths += 25;
	}
	if(has(CLERIC_LEVEL_IND)) {
		tenths += 45;
	}
	if(has(DRUID_LEVEL_IND)) {
		tenths += 45;
	}
	if(has(WARRIOR_LEVEL_IND) || has(BARBARIAN_LEVEL_IND) || has(PALADIN_LEVEL_IND) ||
	   has(RANGER_LEVEL_IND)) {
		tenths += 55;
	}
	if(has(THIEF_LEVEL_IND) || has(PSI_LEVEL_IND)) {
		tenths += 35;
	}
	if(has(MONK_LEVEL_IND)) {
		tenths += 35;
	}
	return (tenths + 5) / 10;
}

void apply_max_hit_delta(struct char_data* ch, int delta) {
	ch->points.max_hit = static_cast<sh_int>(
		std::clamp(ch->points.max_hit + delta, 1, static_cast<int>(std::numeric_limits<sh_int>::max())));
	if(GET_HIT(ch) > hit_limit(ch)) {
		GET_HIT(ch) = static_cast<sh_int>(hit_limit(ch));
	}
}

/* Vecchio calcolo della perdita di livello: si usa solo se manca lo storico. */
struct LegacyDropRule {
	unsigned long class_bit;
	int class_index;
	int threshold;
	int dice_max;
	int flat;
	int minimum;
};

constexpr std::array<LegacyDropRule, 11> kLegacyDropRules{{
	{CLASS_MAGIC_USER, MAGE_LEVEL_IND, 12, 8, 2, 3},
	{CLASS_SORCERER, SORCERER_LEVEL_IND, 12, 8, 2, 3},
	{CLASS_CLERIC, CLERIC_LEVEL_IND, 12, 12, 5, 5},
	{CLASS_THIEF, THIEF_LEVEL_IND, 12, 10, 4, 4},
	{CLASS_PSI, PSI_LEVEL_IND, 12, 10, 4, 4},
	{CLASS_WARRIOR, WARRIOR_LEVEL_IND, 10, 16, 6, 6},
	{CLASS_PALADIN, PALADIN_LEVEL_IND, 10, 16, 6, 6},
	{CLASS_RANGER, RANGER_LEVEL_IND, 11, 13, 6, 5},
	{CLASS_DRUID, DRUID_LEVEL_IND, 15, 12, 5, 5},
	{CLASS_MONK, MONK_LEVEL_IND, 17, 10, 4, 4},
	{CLASS_BARBARIAN, BARBARIAN_LEVEL_IND, 15, 14, 8, 8},
}};

[[nodiscard]] const LegacyDropRule* legacy_drop_rule(int class_bit) {
	for(const LegacyDropRule& rule : kLegacyDropRules) {
		if(rule.class_bit == static_cast<unsigned long>(class_bit)) {
			return &rule;
		}
	}
	return nullptr;
}

[[nodiscard]] int legacy_drop_hp_loss(struct char_data* ch, const LegacyDropRule& rule, int level) {
	int lost = con_app[std::clamp(static_cast<int>(GET_RCON(ch)), 0, 25)].hitp;
	lost += level < rule.threshold ? number(2, rule.dice_max) : rule.flat;
	lost = std::max(lost, rule.minimum) / HowManyClasses(ch);
	return lost <= 2 ? 3 : lost;
}

} // namespace

int creation_race_hp_bonus(int race) {
	constexpr int kBigRaceHpBonus = 15; /* mezzigiganti e troll */
	return (race == RACE_HALF_GIANT || race == RACE_TROLL) ? kBigRaceHpBonus : 0;
}

int GetHpGain(struct char_data* ch, int iClass, int livello, int compat, int check) {
	(void)check;
	int add_hp = con_hp_bonus(compat ? 18 : static_cast<int>(GET_RCON(ch)), iClass);
	if(const auto band = hp_band_for(iClass)) {
		add_hp += livello < band->threshold
					  ? number(compat ? band->low_max : band->low_min, band->low_max)
					  : number(compat ? band->high_max : band->high_min, band->high_max);
	}
	const int classes = HowManyClasses(ch);
	add_hp /= classes;
	if(compat) {
		add_hp += number(0, classes);
	}
	return add_hp;
}

LevelHpExpected level_hp_expected(const int* levels, int con, int race) {
	LevelHpExpected out;
	if(levels == nullptr) {
		return out;
	}
	int classes = 0;
	for(int i = 0; i < MAX_CLASS; ++i) {
		if(levels[i] >= IMMORTALE) {
			return out;
		}
		if(levels[i] > 0) {
			++classes;
		}
	}
	if(classes == 0) {
		return out;
	}
	out.start = estimated_start_hp(levels, classes, race);
	long long total = out.start;
	for(int class_index = 0; class_index < MAX_CLASS; ++class_index) {
		const int top = level_hp_top_row(levels[class_index]);
		for(int level = 1; level <= top; ++level) {
			const int gain = expected_level_gain(class_index, level, con, classes);
			out.cells.push_back(LevelHpCell{class_index, level, gain});
			total += gain;
		}
	}
	out.target = static_cast<int>(std::min<long long>(total, 32767));
	return out;
}

std::vector<LevelHpCell> level_hp_baseline(const int* levels, int con, int max_hit, int race) {
	return level_hp_baseline_missing(levels, con, max_hit, race, {}).cells;
}

LevelHpFill level_hp_baseline_missing(const int* levels, int con, int max_hit, int race,
									  const std::vector<LevelHpCell>& existing) {
	LevelHpFill fill;
	if(levels == nullptr) {
		return fill;
	}
	int classes = 0;
	for(int i = 0; i < MAX_CLASS; ++i) {
		if(levels[i] >= IMMORTALE) {
			return fill; /* immortali: niente storico */
		}
		if(levels[i] > 0) {
			++classes;
		}
	}
	if(classes == 0) {
		return fill;
	}
	std::array<std::array<bool, ABS_MAX_LVL>, MAX_CLASS> present {};
	long long held_sum = 0;
	for(const LevelHpCell& e : existing) {
		if(!valid_level_hp_cell(e.class_index, e.level)) {
			continue;
		}
		present[static_cast<std::size_t>(e.class_index)][static_cast<std::size_t>(e.level)] = true;
		if(e.level <= level_hp_top_row(levels[e.class_index]) && e.hp > 0) {
			held_sum += e.hp;
		}
	}
	std::vector<LevelHpCell>& cells = fill.cells;
	std::vector<long long> weights;
	for(int class_index = 0; class_index < MAX_CLASS; ++class_index) {
		const int top = level_hp_top_row(levels[class_index]);
		for(int level = 1; level <= top; ++level) {
			if(present[static_cast<std::size_t>(class_index)][static_cast<std::size_t>(level)]) {
				continue;
			}
			cells.push_back(LevelHpCell{class_index, level, 1});
			weights.push_back(expected_level_gain(class_index, level, con, classes));
		}
	}
	if(cells.empty()) {
		return fill;
	}
	const auto count = static_cast<long long>(cells.size());
	const long long remaining =
		static_cast<long long>(max_hit) - estimated_start_hp(levels, classes, race) - held_sum;
	fill.clamped = remaining < count;
	const long long spare = std::max(remaining, count) - count;
	const long long weight_sum = std::accumulate(weights.begin(), weights.end(), 0LL);
	if(spare == 0 || weight_sum <= 0) {
		return fill;
	}
	long long given = 0;
	std::vector<std::pair<long long, std::size_t>> remainders;
	for(std::size_t i = 0; i < cells.size(); ++i) {
		const long long share = spare * weights[i];
		cells[i].hp += static_cast<int>(share / weight_sum);
		given += share / weight_sum;
		remainders.emplace_back(share % weight_sum, i);
	}
	std::stable_sort(remainders.begin(), remainders.end(),
					 [](const auto& a, const auto& b) { return a.first > b.first; });
	for(std::size_t k = 0; given < spare && k < remainders.size(); ++k, ++given) {
		++cells[remainders[k].second].hp;
	}
	return fill;
}

std::optional<int> hp_for_level_loss(struct char_data* ch, int class_index, int level) {
	if(ch == nullptr || IS_NPC(ch) || !valid_level_hp_cell(class_index, level)) {
		return std::nullopt;
	}
	return level_hp_db_get(ch, class_index, level);
}

bool level_hp_matches_base(struct char_data* ch) {
	if(ch == nullptr || IS_NPC(ch)) {
		return true;
	}
	return !level_hp_db_any_differs(ch).value_or(false);
}

std::optional<int> level_hp_reroll(struct char_data* ch) {
	if(ch == nullptr || IS_NPC(ch)) {
		return std::nullopt;
	}
	const auto rows = level_hp_db_rows(ch, false);
	if(!rows || rows->empty()) {
		return std::nullopt;
	}
	int delta = 0;
	std::vector<LevelHpRow> changed;
	for(LevelHpRow row : *rows) {
		if(!valid_level_hp_cell(row.class_index, row.level) || row.hp <= 0) {
			continue;
		}
		const int fresh = std::max(1, GetHpGain(ch, row.class_index, row.level, 0));
		if(fresh == row.hp) {
			continue;
		}
		if(level_hp_held(ch, row.class_index, row.level)) {
			delta += fresh - row.hp;
		}
		row.hp = fresh;
		changed.push_back(row);
	}
	if(!level_hp_db_update_hp(ch, changed)) {
		return std::nullopt;
	}
	apply_max_hit_delta(ch, delta);
	return delta;
}

std::optional<int> level_hp_restore_base(struct char_data* ch) {
	if(ch == nullptr || IS_NPC(ch)) {
		return std::nullopt;
	}
	const auto rows = level_hp_db_rows(ch, true);
	if(!rows) {
		return std::nullopt;
	}
	int delta = 0;
	std::vector<LevelHpRow> changed;
	for(LevelHpRow row : *rows) {
		if(!valid_level_hp_cell(row.class_index, row.level) || row.base_hp <= 0 || row.hp == row.base_hp) {
			continue;
		}
		if(level_hp_held(ch, row.class_index, row.level)) {
			delta += row.base_hp - row.hp;
		}
		row.hp = row.base_hp;
		changed.push_back(row);
	}
	if(changed.empty() || !level_hp_db_update_hp(ch, changed)) {
		return std::nullopt;
	}
	apply_max_hit_delta(ch, delta);
	return delta;
}

int GetExtimatedHp(struct char_data* ch) {
	int i=0;
	int j=0;
	int ext_hp=0;
	if(GetMaxLevel(ch) < IMMORTALE) {

		for(i=0; i<MAX_CLASS; i++) {

			if(GET_LEVEL(ch,i) > 0) {
				for(j=1; j<=GET_LEVEL(ch,i); j++) {
					ext_hp+=GetHpGain(ch,i,j,1,1);
				}
				/* for */
			}

		}
	}
	else {
		ext_hp=hit_limit(ch)*12/10;
	}
	return(ext_hp);
}


void advance_level(struct char_data* ch, int iClass) {
	advance_level(ch, iClass, LevelHpRecord::Store);
}

void advance_level(struct char_data* ch, int iClass, LevelHpRecord record) {
	if(iClass < 0 || iClass >= MAX_CLASS) {
		mudlog(LOG_SYSERR, "Bad advance class.. no such class");
		return;
	}

	if(GET_LEVEL(ch, iClass) > 0 &&
			GET_EXP(ch) < titles[iClass][GET_LEVEL(ch, iClass) + 1].exp) {
		/*  they can't advance here */
		mudlog(LOG_ERROR, "Bad advance_level, can't advance in this class.");
		return;
	}

	GET_LEVEL(ch, iClass) += 1;
	const int level = GET_LEVEL(ch, iClass);

	/* PRINCIPE: come la promozione naturale, niente HP e niente riga.
	 * Livello gia' avuto e poi perso: stessi hp di allora, niente tiro. */
	int gain = level == PRINCIPE ? 0 : hp_for_level_loss(ch, iClass, level).value_or(0);
	if(gain <= 0 && level != PRINCIPE) {
		gain = std::max(1, GetHpGain(ch, iClass, level, 0));
		if(record == LevelHpRecord::Store && !IS_NPC(ch)) {
			level_hp_db_insert(ch, iClass, level, gain); /* errori: log, si va avanti */
		}
	}
	apply_max_hit_delta(ch, gain);

	if(ch->specials.spells_to_learn < 100) {
		ch->specials.spells_to_learn = static_cast<byte>(
			ch->specials.spells_to_learn + std::max(2, static_cast<int>(wis_app[static_cast<int>(GET_RWIS(ch))].bonus)));
	}
	else {
		send_to_char("Stai perdendo le sessioni di allenamento!", ch);
	}

	ClassSpecificStuff(ch);

	if(GetMaxLevel(ch) >= IMMORTALE) {
		for(int i = 0; i < 3; ++i) {
			ch->specials.conditions[i] = -1;
		}
	}

	/* start regening new points */
	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);
}

/* Lose in various points */

/*
** Damn tricky for multi-class...
** HP persi = quelli registrati per quel livello (la riga resta, come livello perso).
** Senza storico: errore nel log e vecchio calcolo a dadi.
*/
void drop_level(struct char_data* ch, int iClass, int goddrain) {
	if(!goddrain && GetMaxLevel(ch) >= IMMORTALE) {
		return;
	}
	if(GetMaxLevel(ch) == 1) {
		return;
	}

	const LegacyDropRule* rule = legacy_drop_rule(iClass);
	if(rule == nullptr) {
		mudlog(LOG_SYSERR, "Classe %d non valida in drop_level (limits.c).", iClass);
		return;
	}
	const int lin_class = rule->class_index;
	const int level = GET_LEVEL(ch, lin_class);
	if(level <= 1) {
		/* Prima il livello tornava a 1 e max_hit scendeva a 20: ora non si toglie nulla. */
		mudlog(LOG_CHECK, "drop_level: %s classe %d gia' al livello %d, nessuna perdita",
			   GET_NAME(ch), lin_class, level);
		return;
	}

	/* PRINCIPE -> BARONE: il livello non aveva HP ne' riga, non si toglie nulla. */
	int lost_hp = 0;
	if(level != PRINCIPE) {
		if(const auto stored = hp_for_level_loss(ch, lin_class, level)) {
			lost_hp = std::max(1, *stored);
		}
		else {
			mudlog(LOG_SYSERR, "drop_level: %s senza storico HP (classe %d livello %d), uso il vecchio calcolo",
				   GET_NAME(ch), lin_class, level);
			lost_hp = std::max(1, legacy_drop_hp_loss(ch, *rule, level));
		}
	}

	GET_LEVEL(ch, lin_class) -= 1;
	ch->points.max_hit = static_cast<sh_int>(std::max(1, ch->points.max_hit - lost_hp));

	ch->specials.spells_to_learn = static_cast<byte>(std::max(
		0, ch->specials.spells_to_learn - std::max(2, static_cast<int>(wis_app[static_cast<int>(GET_RWIS(ch))].bonus))));

	const int exp_floor = titles[lin_class][GET_LEVEL(ch, lin_class)].exp;
	if(ch->points.exp > exp_floor) {
		ch->points.exp = exp_floor;
	}
	if(ch->points.exp < 0) {
		ch->points.exp = 0;
	}

	send_to_char("Perdi un livello.\n\r", ch);
}



void set_title(struct char_data* ch) {

	char buf[256];

	if(IS_SET(ch->player.user_flags,SHOW_CLASSES)) {
		sprintf(buf,
				"the %s %s", RaceName[ch->race], ClassTitles(ch));
	}
	else {
		sprintf(buf, "the %s", RaceName[ch->race]);
	}

	if(GET_TITLE(ch)) {
		free(GET_TITLE(ch));
		CREATE(GET_TITLE(ch),char,strlen(buf)+1);
	}
	else {
		CREATE(GET_TITLE(ch),char,strlen(buf)+1);
	}

	strcpy(GET_TITLE(ch), buf);

}
namespace {
/** Aneddoti mostrati quando gli xp toccano il limite degli int. */
constexpr std::array<std::string_view, 10> kExpLimitLines{
	"Un vecchio nano tesoriere di Khazad borbotta: 'Neanche io riuscirei a ficcare un altro ricordo in quella testa.'",
	"Un saggio gnomo chiude il suo taccuino: 'Hai imparato tutto quello che c'era da imparare. Per oggi niente sconti.'",
	"Si racconta di un nano che non volle mai insegnare niente a nessuno, per paura di restare senza sapere. Tu non hai piu' spazio nemmeno per quello.",
	"Un halfling ti guarda con invidia: 'Pure la mia memoria ha un fondo, ma la tua l'ha toccato.'",
	"Un goblin usuraio sghignazza: 'Ti darei in prestito un po' di saggezza, ma non sapresti dove metterla.'",
	"Un antico proverbio nanico dice: 'Chi vuole sapere tutto della montagna finisce sepolto sotto la montagna.'",
	"Un drago in letargo brontola nel sonno: 'Basta lezioni, anche la mia pazienza ha un limite.'",
	"Un archivista elfo chiude il libro mastro: 'Le pagine della tua storia sono finite, e gli elfi non sprecano carta.'",
	"Un nano delle profondita' ride sotto la barba: 'Persino le nostre miniere hanno un fondo, ragazzo. E tu ci sei arrivato.'",
	"Un vecchio mago halfling ripone la pergamena: 'Il tuo calice e' colmo, una goccia in piu' e trabocca.'"
};

/**
 * Spezza text in righe di al massimo width caratteri visibili (a capo sugli
 * spazi), ognuna con color davanti e "\n\r" in fondo.
 */
[[nodiscard]] std::string wrap_colored(std::string_view text, std::string_view color,
									   std::size_t width = 79) {
	std::string out;
	std::string line;
	std::size_t pos = 0;
	while(pos < text.size()) {
		const std::size_t space = text.find(' ', pos);
		const std::size_t end = (space == std::string_view::npos) ? text.size() : space;
		const std::string_view word = text.substr(pos, end - pos);
		if(!line.empty() && line.size() + 1 + word.size() > width) {
			out.append(color).append(line).append("\n\r");
			line.clear();
		}
		if(!line.empty()) {
			line += ' ';
		}
		line.append(word);
		pos = (space == std::string_view::npos) ? text.size() : space + 1;
	}
	if(!line.empty()) {
		out.append(color).append(line).append("\n\r");
	}
	return out;
}

/** Corpo a cui appartengono gli xp: l'originale se ch e' un poly, altrimenti ch. */
[[nodiscard]] char_data* xp_owner(char_data* ch) {
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}
} // namespace

void send_exp_limit_message(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	const std::string_view anecdote =
		kExpLimitLines[static_cast<std::size_t>(number(0, static_cast<int>(kExpLimitLines.size()) - 1))];
	const std::string limit_msg =
		wrap_colored(anecdote, "$c0014") +
		wrap_colored("La tua esperienza ha toccato il limite: non puoi accumularne altra.", "$c0014") +
		"$c0011E' ora di editare.\n\r";
	send_to_char(limit_msg.c_str(), ch);
}

long long exp_guild_cap(const struct char_data* ch) {
	long long cap = static_cast<long long>(std::numeric_limits<int>::max());
	if(ch == nullptr || !IS_PC(ch)) {
		return cap;
	}
	/* in poly vale la razza del pg, come in gain_exp */
	const int race = (ch->desc != nullptr && ch->desc->original != nullptr)
						 ? static_cast<int>(ch->desc->original->race)
						 : static_cast<int>(GET_RACE(ch));
	for(int i = MAGE_LEVEL_IND; i < MAX_CLASS; ++i) {
		const int level = GET_LEVEL(ch, i);
		if(level > 0 && level < RacialMax[race][i]) {
			cap = std::min(cap, static_cast<long long>(titles[i][level + 2].exp) - 1);
		}
	}
	return cap;
}

namespace {
/* Promozione gia' applicata, messaggi rimandati a fine pulse (dopo le righe xp). */
struct PendingPromotion {
	const char_data* ch; /* solo confronto con i desc: mai dereferenziato se sparito */
	std::string name;
	bool female;
};

[[nodiscard]] std::vector<PendingPromotion>& pending_promotions() {
	static std::vector<PendingPromotion> pending;
	return pending;
}
} // namespace

void flush_prince_promotions() {
	auto& pending = pending_promotions();
	if(pending.empty()) {
		return;
	}
	const std::vector<PendingPromotion> batch = std::move(pending);
	pending.clear();
	for(const PendingPromotion& promo : batch) {
		char_data* target = nullptr;
		for(descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
			if(d->character == promo.ch) {
				target = d->character;
				break;
			}
		}
		const std::string announce =
			"$c0015Squillano le trombe in tutte le Nebbie: $c0011" + promo.name +
			(promo.female ? std::string("$c0015 e' stata incoronata Principessa.\n\r")
						  : std::string("$c0015 e' stato incoronato Principe.\n\r"));
		if(target != nullptr) {
			const std::string personal =
				std::string("$c0015Le voci delle tue imprese sono arrivate fino a corte: da oggi\n\r") +
				(promo.female ? "$c0015sei una Principessa. Porta la corona con onore.\n\r"
							  : "$c0015sei un Principe. Porta la corona con onore.\n\r");
			send_to_char(personal.c_str(), target);
			send_to_all_not_ch(target, announce.c_str());
		}
		else {
			send_to_all(announce.c_str());
		}
	}
}

void check_prince_promotion(struct char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	/* in poly i livelli del mob sono una copia: la promozione va anche sul pg */
	char_data* const owner = xp_owner(ch);
	bool promoted = false;
	for(int i = MAGE_LEVEL_IND; i < MAX_CLASS; ++i) {
		if(GET_LEVEL(ch, i) == BARONE && GET_EXP(ch) > PRINCEEXP) {
			GET_LEVEL(ch, i) = PRINCIPE;
			if(owner != ch && GET_LEVEL(owner, i) == BARONE) {
				GET_LEVEL(owner, i) = PRINCIPE;
			}
			promoted = true;
		}
	}
	if(!promoted) {
		return;
	}
	/* in poly il desc e' sul mob (ch): il messaggio andra' li' (flush_prince_promotions),
	 * dopo la riga xp del chiamante; annuncio e log col nome del pg */
	pending_promotions().push_back({ch, GET_NAME(owner), GET_SEX(owner) == SEX_FEMALE});
	mudlog(LOG_PLAYERS, "%s e' diventato Principe (%d xp).", GET_NAME(owner), GET_EXP(ch));
}

int add_exp_checked(struct char_data* ch, int amount) {
	if(ch == nullptr || amount <= 0) {
		return 0;
	}
	const long long have = static_cast<long long>(GET_EXP(ch));
	const long long int_room = static_cast<long long>(std::numeric_limits<int>::max()) - have;
	const long long guild_room = exp_guild_cap(ch) - have;
	const long long room = std::max(0LL, std::min(int_room, guild_room));
	const int added = static_cast<int>(std::min(static_cast<long long>(amount), room));
	if(added < amount) {
		if(guild_room < int_room) {
			if(IS_PC(ch)) {
				send_to_char("Devi passare alla tua gilda prima di guadagnare ulteriore esperienza.\n\r", ch);
			}
		}
		else {
			/* limite di un int: entra solo quel che ci sta */
			mudlog(LOG_PLAYERS, "WARNING: %s EXP OVERFLOW!", GET_NAME(ch));
			if(IS_PC(ch)) {
				send_exp_limit_message(ch);
			}
		}
	}
	GET_EXP(ch) += added;
	if(added > 0) {
		check_prince_promotion(ch);
	}
	return added;
}

int gain_corretto(struct char_data* ch,int gain) {
	if(!IS_PC(ch) || gain <= 0) {
		return(gain);    // Gaia 2001
	}
#if NEW_EQ_GAIN // Gaia 2001
	float eqindex=0.0;
	float eqratio=0.0;
	float tmp=0.0;
	int newgain=0;
	char buf[255];
	eqindex=(float)GetCharBonusIndex(ch);
	eqratio=eqindex-EQINDEX;
	if(abs((int)eqratio)<100) {
		return(gain);
	}
	tmp=eqratio/20;
	if(tmp < -30) {
		tmp = -30 ;
	}
	if(tmp > 30) {
		tmp = 30 ;
	}
	tmp=gain*tmp/100;
	newgain=(int)(gain-tmp);
	buglog(LOG_CHECK,"Eq: %s I: %f R:%f gain da %d %d",
		   GET_NAME(ch),eqindex,eqratio,gain,newgain);

	sprintf(buf, "Il tuo equipaggiamento modifica il guadagno di esperienza in  %d",
			newgain);
	act(buf, 0, ch, 0, ch, TO_CHAR);
	gain = newgain ;
	/*#else
	   sprintf( buf, "La qualita' del tuo equipaggiamento modificherebbe il tuo guadagno di esperienza in  %d",
		   newgain);
	   act( buf, 0, ch, 0, ch, TO_CHAR );*/
#endif

	return (gain);
}

void gain_exp_rev(struct char_data* ch, int gain) {
	/* gain completo se positivo, diviso per le classi se negativo */
	int i;
	char buf[256];
	short chrace;
	int nClassiNonMaxxate=0;
	mark_body_save_pending(ch); /* salvato a breve da body_save_pulse */
	gain=gain_corretto(ch,gain);
	if(!IS_PC(ch) && ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
		if(ch->master->in_room == ch->in_room) {
			if(gain > 1) {
				gain /= 2;
				sprintf(buf, "Guadagni i %d punti di esperienza di $N", gain);
				act(buf, 0, ch->master, 0, ch, TO_CHAR);
				gain_exp(ch->master, gain);
			}
		}
		return;
	}

	if(!IS_IMMORTAL(ch)) {
		if(gain > 0) {

			if(GetMaxLevel(ch) == 1) {
				gain *= 2;
			}

			if(IS_PC(ch)) {
				if(ch->desc && ch->desc->original) {
					chrace = ch->desc->original->race;
				}
				else {
					chrace = GET_RACE(ch);
				}
				for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
					/*ALAR:check sui limiti razziali per i  livelli */
					if(GET_LEVEL(ch, i) &&
							GET_LEVEL(ch, i) < RacialMax[ chrace ][ i ]) {
						nClassiNonMaxxate++;
						if(GET_EXP(ch) >=
								titles[ i ][ GET_LEVEL(ch, i) + 1 ].exp) {
							/* do nothing..this is cool */
						}
						else {
							const int class_level = GET_LEVEL(ch, i);
							const long long projected_exp = static_cast<long long>(GET_EXP(ch)) + static_cast<long long>(gain);
							if(projected_exp < static_cast<long long>(titles[ i ][ class_level + 1].exp)) {
								continue;
							}
							sprintf(buf, "Sei abbastanza esperto per essere un %s\n\r",
									GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i) + 1));
							send_to_char(buf, ch);
							send_to_char("Devi passare dalla tua gilda per crescere di "
										 "livello.\n\r", ch);
							const int next_title_exp = titles[ i ][ class_level + 2 ].exp;
							if(projected_exp >= static_cast<long long>(next_title_exp)) {

								/*BUG BUG overflow*/
								if(next_title_exp > 1) {
									GET_EXP(ch) = next_title_exp - 1;
								}
								else {
									send_to_char("Non prendi punti esperienza perche' oltrepassi la soglia massima!!!",ch);
								}
								return;
							}
						}
					}
				}

				const long long next_exp = static_cast<long long>(GET_EXP(ch)) + gain;
				if(next_exp > 0) {
					GET_EXP(ch) = static_cast<int>(next_exp);
				} else {
					send_to_char("Non prendi punti esperienza perche' oltrepassi la soglia massima!!!",ch);
				}

				for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
					if(GET_LEVEL(ch, i) &&
							GET_LEVEL(ch, i) < RacialMax[ chrace ][ i ]) {
						const int class_level = GET_LEVEL(ch, i);
						const int next_title_exp = titles[ i ][ class_level + 2 ].exp;
						if(static_cast<long long>(GET_EXP(ch)) > static_cast<long long>(next_title_exp)) {
							GET_EXP(ch) = next_title_exp - 1;
						}
					}
				}
			}
			else { /*IS_NPC*/
				GET_EXP(ch) = static_cast<int>(static_cast<long long>(GET_EXP(ch)) + gain);
			}
		}
		else if(gain < 0) {
			/*	 gain/=MAX(1,nClassiNonMaxxate); */
			gain/=MAX(1,HowManyClasses(ch));
			if(IS_PC(ch)) {
				mudlog(LOG_PLAYERS, "%s ha perso %d punti di esperienza.",
					   GET_NAME(ch), -gain);
				mudlog(LOG_SYSERR, "PKill loss 1");
			}
			GET_EXP(ch) = static_cast<int>(static_cast<long long>(GET_EXP(ch)) + gain);
			if(GET_EXP(ch) < 0) {
				GET_EXP(ch) = 0;
			}
		}
	}
}


void gain_exp(struct char_data* ch, int gain) {
	/* gain completo se negativo, diviso per le classi se positivo
	 * Per le classi che hanno una razza maxxata, consente il gain completo
	 * */

	int i;
	char buf[256];
	short chrace;
	short nClassiNonMaxxate=0;
	if(IS_POLY(ch) && IS_IMMORTAL(xp_owner(ch))) {
		return; /* immortale in poly: il mob non accumula xp da riportare sul pg */
	}
	mark_body_save_pending(ch); /* salvato a breve da body_save_pulse */
	gain=gain_corretto(ch,gain);
	if(!IS_PC(ch) && ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
		if(ch->master->in_room == ch->in_room) {
			if(gain > 1) {
				gain /= 2;
				sprintf(buf, "Guadagni i %d punti di esperienza di $N", gain);
				act(buf, 0, ch->master, 0, ch, TO_CHAR);
				gain_exp(ch->master, gain);
			}
		}
		return;
	}

	if(!IS_IMMORTAL(ch)) {
		if(gain > 0) {

			if(GetMaxLevel(ch) == 1) {
				gain *= 2;
			}

			if(IS_PC(ch)) {
				if(ch->desc && ch->desc->original) {
					chrace = ch->desc->original->race;
				}
				else {
					chrace = GET_RACE(ch);
				}
				for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
					/*ALAR:check sui limiti razziali per i  livelli
					* Conta le classi attive
					*/
					if(GET_LEVEL(ch, i) && GET_LEVEL(ch, i) < RacialMax[ chrace ][ i ]) {
						nClassiNonMaxxate++;
					}
				}
				/*	    gain/=MAX(1,nClassiNonMaxxate); */

				gain/=MAX(1,HowManyClasses(ch));

				/* Qui sistemiamo l'overflow dei mono... Flyp*/
				const long long sum = static_cast<long long>(GET_EXP(ch)) + static_cast<long long>(gain);
				if(sum > static_cast<long long>(std::numeric_limits<int>::max())) {
					mudlog(LOG_PLAYERS, "WARNING: %s EXP OVERFLOW!", GET_NAME(ch));
					send_exp_limit_message(ch);
				}
				else {
					GET_EXP(ch) = static_cast<int>(sum);
				}
				/* Ma i multiclasse mi overflowano ancora... Flyp */

				/* Tetto di gilda (condiviso con add_exp_checked) */
				const long long guild_cap = exp_guild_cap(ch);
				if(static_cast<long long>(GET_EXP(ch)) > guild_cap) {
					send_to_char("Devi passare alla tua gilda prima di guadagnare ulteriore esperienza.\n\r", ch);
					GET_EXP(ch) = static_cast<int>(guild_cap);
					mudlog(LOG_SYSERR, "(LIMITS)Maxxa gli xp a %d", static_cast<int>(guild_cap));
				}

				/* Secondo loop, avvisa per il possibile passaggio */
				for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
					if((GET_LEVEL(ch,i) < BARONE && GET_LEVEL(ch,i) > 0 && (GET_EXP(ch) > titles[ i ][ GET_LEVEL(ch,i)+1].exp)) && GET_LEVEL(ch,i)<RacialMax[chrace][i]) {
						sprintf(buf, "Sei sufficientemente esperto per essere un %s\n\r",
								GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i) + 1));
						send_to_char(buf, ch);
						send_to_char("Devi passare dalla tua gilda per crescere di livello.\n\r", ch);
					}
				}
				check_prince_promotion(ch);
			}

			else { /*IS_NPC*/
				GET_EXP(ch) += gain;
			}
		}
		else if(gain < 0) {
			if(IS_PC(ch)) {
				mudlog(LOG_PLAYERS, "%s ha perso %d punti di esperienza.", GET_NAME(ch), -gain);
				mudlog(LOG_SYSERR, "PKill loss 2");
			}
			GET_EXP(ch) += gain;
			if(GET_EXP(ch) < 0) {
				GET_EXP(ch) = 0;
			}
		}
	}
}


void gain_exp_regardless(struct char_data* ch, int gain, int iClass,
						 int iMaxLevel) {
	int i,q;
	bool is_altered = FALSE;

	mark_body_save_pending(ch); /* salvato a breve da body_save_pulse */
	if(!IS_NPC(ch)) {
		if(gain >= 0) {
			/* Qui vediamo l'overflow per i multi... Flyp */
			q=GET_EXP(ch);
			q+=gain;
			if(q < 0) {
				mudlog(LOG_PLAYERS, "WARNING: %s EXP OVERFLOW!", GET_NAME(ch));
			}
			/* ...e se non parte il warning, possiamo assegnare gli xp.. Flyp */

			else {
				GET_EXP(ch) += gain;
				if(iClass < 0 || iClass >= MAX_CLASS) {
					mudlog(LOG_SYSERR,
						   "gain_exp_regardless: iClass=%d invalid for %s", iClass,
						   GET_NAME_DESC(ch));
				}
				else {
					const int start_level = GET_LEVEL(ch, iClass);
					for(i = start_level + 1;
						i < ABS_MAX_LVL && i <= iMaxLevel &&
						titles[iClass][i].exp <= GET_EXP(ch);
						++i) {
						send_to_char("Cresci di un livello!\n\r", ch);
						advance_level(ch, iClass);
						is_altered = TRUE;
						mudlog(LOG_SYSERR, "(LIMITS2)Maxxa la classe %d a %d", iClass,
							   titles[iClass][GET_LEVEL(ch, iClass)].exp);
						if(GET_LEVEL(ch, iClass) >= iMaxLevel) {
							break;
						}
					}
				}
			}
		}
		else if(gain < 0) {
			GET_EXP(ch) += gain;
		}
		if(GET_EXP(ch) < 0)	{
			GET_EXP(ch) = 0;
		}
	}
	if(is_altered) {
		set_title(ch);
	}
}

void gain_condition(struct char_data* ch,int condition,int value) {
	bool intoxicated=false;
	if(IS_IMMORTAL(ch)) {
		return;
	}


	if(GET_COND(ch, condition)==-1) { /* No change */
		return;
	}

	intoxicated=(GET_COND(ch, DRUNK) > 0);

	GET_COND(ch, condition)  += value;
	if(GetMaxLevel(ch) <=5 and (condition == FULL or condition==THIRST)) {
		GET_COND(ch,condition) = MAX(1,GET_COND(ch,condition));
	}
	else {
		GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
	}
	GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));
	if(GET_COND(ch,condition)) {
		return;
	}

	switch(condition) {
	case FULL : {
		if(!affected_by_spell(ch, SKILL_MIND_OVER_BODY)) {
			send_to_char("Hai Fame.\n\r",ch);
		}

		return;
	}
	case THIRST : {
		if(!affected_by_spell(ch, SKILL_MIND_OVER_BODY)) {
			send_to_char("Hai sete.\n\r",ch);
		}
		return;
	}
	case DRUNK : {
		if(intoxicated) {
			send_to_char("Sei sobrio.\n\r",ch);
		}
		return;
	}
	default :
		break;
	}

}


#define VOID_PULL_TIME   30
#define FORCE_RENT_TIME  40
void check_idling(struct char_data* ch) {
	if(IS_LINKDEAD(ch) && ch->specials.timer < 30) {
		ch->specials.timer=30;
	}
	if(++(ch->specials.timer) == 8) {
		do_save(ch, "", 0);

	}
	else if(ch->specials.timer == VOID_PULL_TIME) {
		if(IS_POLY(ch)) {
			ch = linkdead_unpoly(ch, true);
		}
		if(ch->in_room != NOWHERE && ch->in_room != 0) {
			ch->specials.was_in_room = ch->in_room;
			if(ch->specials.fighting) {
				stop_fighting(ch->specials.fighting);
				stop_fighting(ch);
			}
			act("$n sparisce nel nulla.", TRUE, ch, 0, 0, TO_ROOM);
			act("Sei ferm$b da troppo tempo e finisci nel nulla.", FALSE, ch, 0, 0,
				TO_CHAR);
			char_from_room(ch);
			char_to_room(ch, 1);  /* Into room number 1 */
			ch->specials.timer=0;
		}
	}
	else if(ch->specials.timer == FORCE_RENT_TIME) {
		forcerent_extract_player(ch);
	}
}

void ObjFromCorpse(struct obj_data* c) {
	struct obj_data* jj, *next_thing;

	for(jj = c->contains; jj; jj = next_thing) {
		next_thing = jj->next_content; /* Next in inventory */
		if(jj->in_obj) {
			obj_from_obj(jj);
			if(c->in_obj) {
				obj_to_obj(jj,c->in_obj);
			}
			else if(c->carried_by) {
				obj_to_room(jj,c->carried_by->in_room);
				check_falling_obj(jj, c->carried_by->in_room);
			}
			else if(c->in_room != NOWHERE) {
				obj_to_room(jj,c->in_room);
				check_falling_obj(jj, c->in_room);
			}
			else {
				assert(FALSE);
			}
		}
		else {
			/*
			 **  hmm..  it isn't in the object it says it is in.
			 **  don't extract it.
			 */
			c->contains = 0;
			mudlog(LOG_SYSERR, "Memory lost in ObjFromCorpse.");
			return;
		}
	}
	extract_obj(c);
}




void ClassSpecificStuff(struct char_data* ch) {
	/* Elimino slowness e haste*/
	if(affected_by_spell(ch,SPELL_SLOW) || affected_by_spell(ch,SPELL_HASTE)) {
		affect_from_char(ch,SPELL_SLOW);
		affect_from_char(ch,SPELL_HASTE);
		send_to_char("Xanathon ti rimette in equilibrio.",ch);
	}

	reset_original_numattacks(ch);

	/* other stuff.. immunities, etc, are set here */

	if(HasClass(ch, CLASS_MONK)) {
		/*
		 */
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 10) {
			SET_BIT(ch->M_immune, IMM_HOLD);
		}
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 18) {
			SET_BIT(ch->immune, IMM_CHARM);
		}
		if(GET_LEVEL(ch, MONK_LEVEL_IND) > 22) {
			SET_BIT(ch->M_immune, IMM_POISON);
		}
		if(OnlyClass(ch, CLASS_MONK) && GET_LEVEL(ch, MONK_LEVEL_IND) > 49) {
			SET_BIT(ch->M_immune, IMM_DRAIN);
		}


	}
	else {
		if(HasClass(ch, CLASS_DRUID)) {
			if(GET_LEVEL(ch, DRUID_LEVEL_IND) >= 14) {
				SET_BIT(ch->immune, IMM_CHARM);
			}
			if(GET_LEVEL(ch, DRUID_LEVEL_IND) >= 32) {
				SET_BIT(ch->M_immune, IMM_POISON);
			}
		}

		if(HasClass(ch, CLASS_THIEF)) {

			if(OnlyClass(ch, CLASS_THIEF)) {
				GET_CHR(ch)+=1;
			}

			GET_CHR(ch) += GET_LEVEL(ch, THIEF_LEVEL_IND)/10;
		}
	}

	SyncInnateAffects(ch);
}


} // namespace Alarmud
