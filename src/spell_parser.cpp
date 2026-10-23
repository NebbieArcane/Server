/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD
* $Id: spell_parser.c,v 1.5 2002/03/11 00:15:36 Thunder Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "spell_parser.hpp"
#include "act.info.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "breath.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magicutils.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "utility.hpp"

namespace Alarmud {

#define MANA_MU 1
#define MANA_CL 1

/* Global data */




struct spell_info_type spell_info[MAX_SPL_LIST];
/* 100 is the MAX_MANA for a character */
#define USE_MANA( ch, sn ) MAX( (int)spell_info[ sn ].min_usesmana, 100 /     MAX( 2,( (GetAverageLevel(ch)==51?4:2) + GET_LEVEL( ch, BestMagicClass( ch ) ) - SPELL_LEVEL( ch, sn ))))
#define SPELLFAIL_MOD( cn, sn ) (int)((cn - sn) / 3 * 2)
#define ABSNEUTRAL 350

const char* spells[]= {
	"armor",               /* 1 */
	"teleport",
	"bless",
	"blindness",
	"burning hands",
	"call lightning",
	"charm person",
	"chill touch",
	"clone",
	"colour spray",
	"control weather",     /* 11 */
	"create food",
	"create water",
	"cure blind",
	"cure critic",
	"cure light",
	"curse",
	"detect evil",
	"detect invisibility",
	"detect magic",
	"detect poison",       /* 21 */
	"dispel evil",
	"earthquake",
	"enchant weapon",
	"energy drain",
	"fireball",
	"harm",
	"heal",
	"invisibility",
	"lightning bolt",
	"locate object",      /* 31 */
	"magic missile",
	"poison",
	"protection from evil",
	"remove curse",
	"sanctuary",
	"shocking grasp",
	"sleep",
	"strength",
	"summon",
	"ventriloquate",      /* 41 */
	"word of recall",
	"remove poison",
	"sense life",         /* 44 */

	/* RESERVED SKILLS */
	"sneak",        /* 45 */
	"hide",
	"steal",
	"backstab",
	"pick",
	"kick",         /* 50 */
	"bash",
	"rescue",
	/* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

	"identify",           /* 53 */
	"infravision",
	"cause light",
	"cause critical",
	"flamestrike",
	"dispel good",
	"weakness",
	"dispel magic",
	"knock",
	"know alignment",
	"animate dead",
	"paralyze",
	"remove paralysis",
	"fear",
	"acid blast",  /* 67 */
	"water breath",
	"fly",
	"cone of cold",   /* 70 */
	"meteor swarm",
	"ice storm",
	"shield",
	"monsum one",
	"monsum two",
	"monsum three",
	"monsum four",
	"monsum five",
	"monsum six",
	"monsum seven",  /* 80 */
	"fireshield",
	"charm monster",
	"cure serious",
	"cause serious",
	"refresh",
	"second wind",
	"turn",
	"succor",
	"create light",
	"continual light",        /* 90 */
	"calm",
	"stone skin",
	"conjure elemental",
	"true sight",
	"minor creation",
	"faerie fire",
	"faerie fog",
	"cacaodemon",
	"polymorph self",
	"mana",        /* 100 */
	"astral walk",
	"resurrection",
	"heroes feast",  /* 103 */
	"group fly",
	"breath",
	"web",
	"minor track",
	"major track",
	"golem",
	"find familiar",        /* 110 */
	"changestaff",
	"holy word",
	"unholy word",
	"power word kill",
	"power word blind",
	"chain lightning",
	"scare",
	"aid",
	"command",
	"change form",        /* 120 */
	"feeblemind",
	"shillelagh",
	"goodberry",
	"elemental blade",
	"animal growth",
	"insect growth",
	"creeping death",
	"commune",
	"animal summon one",
	"animal summon two",        /* 130 */
	"animal summon three",
	"fire servant",
	"earth servant",
	"water servant",
	"wind servant",
	"reincarnate",
	"charm vegetable",
	"vegetable growth",
	"tree",
	"animate rock",        /* 140 */
	"tree travel",
	"travelling",
	"animal friendship",
	"invis to animals",
	"slow poison",
	"entangle",
	"snare",
	"gust of wind",
	"barkskin",
	"sunray",        /* 150 */
	"warp weapon",
	"heat stuff",
	"find traps",
	"firestorm",
	"haste",
	"slowness",
	"dust devil",
	"know monster",
	"transport via plant",
	"speak with plants",        /* 160 */
	"silence",
	"sending",
	"teleport without error",
	"portal",
	"dragon ride",
	"mount",
	"" /* SPELL_NO_MESSAGE*/,
	"minor heal",
	"mantra", /*SKILL_MANTRA*/
	"first aid",        /* 170 */
	"sign language",
	"riding",
	"switch opponents",
	"dodge",
	"remove trap",
	"retreat",
	"quivering palm",
	"safe fall",
	"feign death",
	"hunt",        /* 180 */
	"find trap",
	"spring leap",
	"disarm",
	"read magic",
	"evaluate",
	"spy",
	"doorbash",
	"swim",
	"necromancy",
	"vegetable lore",        /* 190 */
	"demonology",
	"animal lore",
	"reptile lore",
	"people lore",
	"giant lore",
	"other lore",        /* 196 */
	"disguise",
	"climb",
	"finger",
	"geyser",        /* 200 SALVO descr */
	"mirror images",
	"tspy",
	"eavesdrop",
	"Parry",
	"miner", // ACIDUS 2003
	"green slime",	/* SALVO descr */
	"berserk",
	"tan",
	"avoid back attack",
	"find food",    /* 210 */
	"find water",   /* 211 */
	"pray",               /* spell Prayer, 212 */
	"memorizing",
	"bellow",
	"darkness",
	"minor invulnerability",
	"major invulnerability",
	"protection from drain",
	"protection from breath",
	"anti magic shell",         /* anti magic shell */
	"doorway",
	"psi portal",
	"psi summon",
	"psi invisibility",
	"canibalize",
	"flame shroud",
	"aura sight",
	"great sight",
	"psionic blast",
	"hypnosis",
	"meditate",
	"scry",
	"adrenalize",
	"brew",
	"ration",
	"warcry",
	"blessing",
	"lay on hands",
	"heroic rescue",
	"dual wield",
	"psi shield",
	"protection from evil group",
	"prismatic spray",
	"incendiary cloud",
	"disintegrate",
	"language common",
	"language elvish",
	"language halfling",
	"language dwarvish",
	"language orcish",
	"language giantish",
	"language ogre",
	"language gnomish",
	"esp",
	"comprehend languages",
	"protection from fire",
	"protection from cold",
	"protection from energy",
	"protection from electricity",
	"enchant armor",
	"messenger",
	"protection fire breath",
	"protection frost breath",
	"protection electric breath",
	"protection acid breath",
	"protection gas breath",
	"wizardeye",
	"mind burn",
	"clairvoyance",
	"psionic danger sense",
	"psionic disintegrate",
	"telekinesis",
	"levitation",
	"cell adjustment",
	"chameleon",
	"psionic strength",
	"mind over body",
	"probability travel",
	"psionic teleport",
	"domination",
	"mind wipe",
	"psychic crush",
	"tower of iron will",
	"mindblank",
	"psychic impersonation",
	"ultra blast",
	"intensify",
	"spot",
    "immolation",
    "quest",
    "daimoku",
    "forge",
    "determine",
    "equilibrium",
	"!innate fly!",             /* 295 INNATE_RACE_FLY */
	"!innate waterbreath!",     /* 296 INNATE_RACE_WATERBREATH */
	"!innate infravision!",     /* 297 INNATE_RACE_INFRAVISION */
	"!innate detect evil!",     /* 298 INNATE_CLASS_DETECT_EVIL */
	"!innate prot evil!",       /* 299 INNATE_CLASS_PROT_EVIL */
	"minor harm",               /* 300 SPELL_MINOR_HARM */
	"manashield",               /* 301 SPELL_MANASHIELD */
	"mind over matter",         /* 302 SPELL_MIND_OVER_MATTER */
	"!ricordo hp!",             /* 303 SPELL_RICORDO_HP (solo oggetti) */
	"!destino hp!",             /* 304 SPELL_DESTINO_HP (solo oggetti) */
	"\n"
};


const byte saving_throws[MAX_CLASS][5][ABS_MAX_LVL] = {
	{
		/* mage */
		{16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* cleric */
	{
		{11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* warrior */
	{
		{15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* thief */
	{
		{16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* druid */
	{
		{11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* monk */
	{
		{16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* barbarian */
	{
		{15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* sorcerer */
	{
		{16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* paladin (same as cleric) */
	{
		{11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* ranger (same as druid) */
	{
		{11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	/* psi */
	{
		{16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	}
};

void spellid(int nr,struct char_data* ch,int cl,int sl) {
	char buf[1000];
	char aligndesc[100];
	char ostilitydesc[100];
	int vmin,vmax;
	int ostilityvalue;
	int alignvalue;
	if(!IS_MAESTRO_DEL_CREATO(ch)) {
		vmin=MIN((int)((45+sl-cl)/10)*-1,0);
		vmax=MAX((int)((45+sl-cl)/10),0);
	}
	else {
		vmin=vmax=0;
	}
	alignvalue=spell_info[nr].align_modifier;
	ostilityvalue=spell_info[nr].ostility_level;
	if(alignvalue<10) 		{
		strcpy(aligndesc,"$c0009Demoniaco$c0007       ");
	}
	if(alignvalue==0) 		{
		strcpy(aligndesc,"$c0010Neutrale$c0007        ");
	}
	if(alignvalue>0) 		{
		strcpy(aligndesc,"$c0014Buono$c0007           ");
	}
	if(alignvalue>0) 		{
		strcpy(aligndesc,"$c0014Angelico$c0007        ");
	}
	if(ostilityvalue<0) 	{
		strcpy(ostilitydesc,"$c0015Non aggressiva$c0007  ");
	}
	if(ostilityvalue<10) 	{
		strcpy(ostilitydesc,"$c0015Carezzevole$c0007     ");
	}
	if(ostilityvalue==0) 	{
		strcpy(ostilitydesc,"$c0010Non applicabile$c0007 ");
	}
	if(ostilityvalue>0) 	{
		strcpy(ostilitydesc,"$c0001Aggressiva$c0007      ");
	}
	if(ostilityvalue>10) 	{
		strcpy(ostilitydesc,"$c0001Molto aggressiva$c0007");
	}
	snprintf(buf,sizeof(buf)-1,
			"\n\rYou cast as about %d level, with spell at about %d level\n\r"
			"Lag      : %d\n\r"
			"Fighting : %s\n\r"
			"Cleric   : %u\n\r"
			"Mage     : %u\n\r"
			"Druid    : %u\n\r"
			"Sorcer   : %u\n\r"
			"Palad    : %u\n\r"
			"Ranger   : %u\n\r"
			"Psi      : %u\n\r"
			"Mana     : %d\n\r"
			/*	   "Target   : %d\n\r" */
			"Spellfail: %d\n\r"
			"Alignment: %s\n\r"
			"Ostility : %s\n\r",
			cl,
			sl,
			spell_info[nr].beats+number(vmin,vmax),
			(spell_info[nr].minimum_position<POSITION_STANDING?"Yes":"No"),
			spell_info[nr].min_level_cleric,
			spell_info[nr].min_level_magic,
			spell_info[nr].min_level_druid,
			spell_info[nr].min_level_sorcerer,
			spell_info[nr].min_level_paladin,
			spell_info[nr].min_level_ranger,
			spell_info[nr].min_level_psi,
			spell_info[nr].min_usesmana+number(vmin,vmax)*10,
			/*	   spell_info[nr].targets          ,*/
			spell_info[nr].spellfail+number(vmin,vmax)*2,
			aligndesc,
			ostilitydesc);
	send_to_char(buf,ch);
	return;
}

void spello(int nr, ubyte beat, ubyte pos,
			ubyte mlev, ubyte clev, ubyte dlev,
			ubyte slev, ubyte plev, ubyte rlev, ubyte ilev,
			ubyte mana, sh_int tar, spell_func func,
			sh_int sf,
			sh_int align,
			sh_int ostile) {
	spell_info[nr].spell_pointer      = func;
	spell_info[nr].beats              = beat;
	spell_info[nr].minimum_position   = pos;
	spell_info[nr].min_level_cleric   = clev;
	spell_info[nr].min_level_magic    = mlev;
	spell_info[nr].min_level_druid    = dlev;
	spell_info[nr].min_level_sorcerer = slev;
	spell_info[nr].min_level_paladin  = plev;
	spell_info[nr].min_level_ranger   = rlev;
	spell_info[nr].min_level_psi      = ilev;
	spell_info[nr].min_usesmana       = mana;
	spell_info[nr].targets            = tar;
	spell_info[nr].spellfail          = sf;
	spell_info[nr].align_modifier     = align;
	spell_info[nr].ostility_level     = ostile;
}


int SPELL_LEVEL(struct char_data* ch, int sn) {
	int min;

	min = ABS_MAX_LVL;

	if(HasClass(ch, CLASS_MAGIC_USER)) {
		min = MIN(min, spell_info[sn].min_level_magic);
	}

	if(HasClass(ch, CLASS_SORCERER)) {
		min = MIN(min, spell_info[sn].min_level_sorcerer);
	}

	if(HasClass(ch, CLASS_CLERIC)) {
		min = MIN(min, spell_info[sn].min_level_cleric);
	}

	if(HasClass(ch, CLASS_PALADIN)) {
		min = MIN(min, spell_info[sn].min_level_paladin);
	}

	if(HasClass(ch, CLASS_RANGER)) {
		min = MIN(min, spell_info[sn].min_level_ranger);
	}

	if(HasClass(ch, CLASS_PSI)) {
		min = MIN(min, spell_info[sn].min_level_psi);
	}

	if(HasClass(ch, CLASS_DRUID)) {
		min = MIN(min, spell_info[sn].min_level_druid);
	}


	return(min);
}


void SpellWearOffSoon(int s, struct char_data* ch) {

	if(s > MAX_SKILLS+10) {
		return;
	}

    if(s == STATUS_QUEST && !IS_PC(ch)) {
        return;
    }

	if(spell_wear_off_soon_msg[s] && *spell_wear_off_soon_msg[s]) {
		act(spell_wear_off_soon_msg[s], FALSE, ch, NULL, NULL, TO_CHAR);
	}

	if(spell_wear_off_soon_room_msg[s] && *spell_wear_off_soon_room_msg[s] &&
			(IS_NPC(ch) || !IS_SET(ch->specials.act, PLR_STEALTH))) {
		act(spell_wear_off_soon_room_msg[s], TRUE, ch, 0, 0, TO_ROOM);
	}

}


int check_falling(struct char_data* ch) {
	struct room_data* rp, *targ;
	int done, count, saved;


	if(IS_IMMORTAL(ch)) {        /* so if this guy is using redit the mud does not */
		return(FALSE);    /* crash when he falls... */
	}

	if(GET_POS(ch) <= POSITION_DEAD) {
		return(FALSE);    /* ch will be purged, check corpse instead, bug fix msw */
	}

	if(IS_AFFECTED(ch, AFF_FLYING)) {
		return(FALSE);
	}

	rp = real_roomp(ch->in_room);
	if(!rp) {
		return(FALSE);
	}

	if(rp->sector_type != SECT_AIR) {
		return(FALSE);
	}


	if(ch->skills && number(1,101) < ch->skills[SKILL_SAFE_FALL].learned) {
		act("Riesci a contenere la caduta, abbastanza da rimanere in vita...",
			TRUE, ch, 0, 0, TO_CHAR);
		saved = TRUE;
	}
	else {
		act("Il mondo gira! Perdi completamente il controllo...",
			TRUE, ch, 0, 0, TO_CHAR);
		saved = FALSE;
	}


	done = FALSE;
	count = 0;

	while(!done && count < 100) {

		/* check for an exit down.  if there is one, go through it.  */
		if(rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
			targ = real_roomp(rp->dir_option[DOWN]->to_room);
		}
		else {
			/* pretend that this is the smash room.  */
			if(count > 1) {
				send_to_char("La tua caduta termina non appena ti frantumi al suolo in piccoli pezzi.\n\r", ch);
				act("$n si frantuma al suolo ad altissima velocita'!",
					FALSE, ch, 0, 0, TO_ROOM);
				act("Schizzi di $c0001sangue$c0007 e pezzi di $c0013carne$c0007 si spargono ovunque.",
					FALSE,ch, 0, 0, TO_ROOM);

				/* should damage all their stuff */
				DamageAllStuff(ch, BLOW_DAMAGE);

				if(!IS_IMMORTAL(ch)) {
					GET_HIT(ch) = 0;
					alter_hit(ch,0);
					mudlog(LOG_PLAYERS, "%s has fallen to death", GET_NAME(ch));
					if(!ch->desc) {
						GET_GOLD(ch) = 0;
					}
					die(ch, 0, NULL);   /* change to the smashed type */
				}
				return(TRUE);

			}
			else {
				send_to_char("Atterri con un risonante $c5009THUMP$c0007! Subito dopo svieni...\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n atterra con un risonante $c5009THUMP$c0007!", FALSE, ch, 0, 0, TO_ROOM);

				/* should damage all their stuff */
				DamageAllStuff(ch, BLOW_DAMAGE);

				return(TRUE);

			}
		}

		act("$n si tuffa verso l'oblio!", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char("Ti tuffi dal $c0014cielo$c0007!\n\r", ch);
		char_from_room(ch);
		char_to_room(ch, rp->dir_option[DOWN]->to_room);
		act("$n cade dal $c0014cielo$c0007!", FALSE, ch, 0, 0, TO_ROOM);
		count++;

		do_look(ch, "", 15);

		if(IS_SET(targ->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
			NailThisSucker(ch);
			return(TRUE);
		}

		if(targ->sector_type != SECT_AIR) {
			/* do damage, or kill */
			if(count == 1) {
				send_to_char("Atterri con un risonante $c5009THUMP$c0007! Subito dopo svieni...\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n atterra con un risonante $c5009THUMP$c0007!", FALSE, ch, 0, 0, TO_ROOM);
				/*
				 * should damage all their stuff
				 */
				DamageAllStuff(ch, BLOW_DAMAGE);

				return(TRUE);

			}
			else if(!saved) {
				send_to_char("La tua caduta termina non appena ti frantumi al suolo in piccoli pezzi.\n\r", ch);
				if(targ->sector_type >= SECT_WATER_SWIM)
					act("$n e' ridotto in $c0003poltiglia$c0007 dal suo impatto con l'$c0012acqua$c0007!",
						FALSE, ch, 0, 0, TO_ROOM);
				else
					act("$n si frantuma al suolo ad altissima velocita'!",
						FALSE, ch, 0, 0, TO_ROOM);
				act("Schizzi di $c0001sangue$c0007 e pezzi di $c0013carne$c0007 si spargono ovunque.", FALSE, ch, 0, 0, TO_ROOM);

				/* should damage all their stuff */
				DamageAllStuff(ch, BLOW_DAMAGE);

				if(!IS_IMMORTAL(ch)) {
					GET_HIT(ch) = 0;
					alter_hit(ch,0);
					mudlog(LOG_PLAYERS, "%s has fallen to death", GET_NAME(ch));
					if(!ch->desc) {
						GET_GOLD(ch) = 0;
					}
					die(ch, 0, NULL);
				}
				return(TRUE);

			}
			else {
				send_to_char("Atterri con un risonante $c5009THUMP$c0007! Subito dopo svieni...\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n atterra con un risonante $c5009THUMP$c0007!", FALSE, ch, 0, 0, TO_ROOM);

				/* should damage all their stuff */
				DamageAllStuff(ch, BLOW_DAMAGE);

				return(TRUE);
			}
		}
		else {
			/* time to try the next room */
			rp = targ;
			targ = 0;
		}
	}
	if(count >= 100) {
		mudlog(LOG_ERROR, "Someone fucked up an air room.");
		char_from_room(ch);
		char_to_room(ch, 2);
		do_look(ch, "", 15);
		return(FALSE);
	}
	return FALSE;
}

void check_drowning(struct char_data* ch) {
	struct room_data* rp;

	if(IS_AFFECTED(ch, AFF_WATERBREATH)) {
		return;
	}

	rp = real_roomp(ch->in_room);

	if(!rp) {
		return;
	}

	if(rp->sector_type == SECT_UNDERWATER) {
		send_to_char("SEI NEL PANICO! Stai $c0012annegando$c0007!!!!!!", ch);
		GET_HIT(ch)-=number(1,30);
		alter_hit(ch,0);
		GET_MOVE(ch) -= number(10,50);
		alter_move(ch,0);
		update_pos(ch);
		if(GET_HIT(ch) < -10) {
			mudlog(LOG_PLAYERS, "%s killed by drowning", GET_NAME(ch));
			if(!ch->desc) {
				GET_GOLD(ch) = 0;
			}
			die(ch, 0, NULL);
		}
	}
}

void check_decharm(struct char_data* ch) {
	struct char_data* m;

	if(!ch->master) {
		return;
	}

	m = ch->master;
	stop_follower(ch);     /* stop following the master */
	REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
	AddFeared(ch, m);
	do_flee(ch, "", 0);

}


void SpellWearOff(int s, struct char_data* ch) {
	if(s > MAX_SKILLS+10) {
		return;
	}

    /* se l'affect che sparisce è paralisi cambio momentaneamente la position */
    if(s == SPELL_PARALYSIS || s == SPELL_ENTANGLE || s == SKILL_DAIMOKU)
    {
        GET_POS(ch) = POSITION_RESTING;
    }

	if(spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
		if(!IS_SET(ch->specials.act, ACT_POLYSELF) && s == SPELL_CHANGE_FORM)
		{
			send_to_char("$c0010Puoi nuovamente mutare la tua forma.\n\r", ch);
		}
		else if(s == SPELL_CREEPING_DEATH) {
			/* Due affect dello stesso tipo: silence breve + cooldown lungo.
			 * Se ne resta un altro, non dire ancora "ti senti meglio". */
			int left = 0;
			for(struct affected_type* a = ch->affected; a; a = a->next) {
				if(a->type == SPELL_CREEPING_DEATH) {
					++left;
				}
			}
			if(left > 1) {
				send_to_char(
					"Il silenzio si allenta, ma senti ancora gli effetti del creeping death.\n\r",
					ch);
			}
			else {
				act(spell_wear_off_msg[s], FALSE, ch, NULL, NULL, TO_CHAR);
			}
		}
		else
		{
			act(spell_wear_off_msg[s], FALSE, ch, NULL, NULL, TO_CHAR);
		}
	}

    if(s == SPELL_PARALYSIS || s == SPELL_ENTANGLE || s == SKILL_DAIMOKU)
    {
        GET_POS(ch) = POSITION_STUNNED;
    }

	if(spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s] &&
			(IS_NPC(ch) || !IS_SET(ch->specials.act, PLR_STEALTH))) {
		act(spell_wear_off_room_msg[s], TRUE, ch, 0, 0, TO_ROOM);
	}

    if(s == STATUS_QUEST) {
        if(IS_PC(ch)) {
            affect_from_char(ch, STATUS_QUEST);
            CheckQuestFail(ch);
            /* Non estrarre il mob qui: affect_update scorre character_list.
             * Se il PG scade prima del mob, extract lascerebbe un char
             * freed nella lista e abortirebbe al magic-number check.
             * Il mob si toglie da solo al proprio wear-off o a EVENT_TICK. */
            unlink_quest_refs(ch);
        }
        else {
            /* fine dei giochi, si torna a casa */
            unlink_quest_refs(ch);
            switch(GET_POS(ch)) {

                case POSITION_FIGHTING  :
                    if(ch->specials.fighting) {
                        WAIT_STATE(ch->specials.fighting, PULSE_VIOLENCE*3);
                    }
                    act("\n\r$c0014$n coglie l'occasione buona e se la da' a gambe per sempre!\n\r", FALSE, ch, 0, ch, TO_ROOM);
                    stop_fighting(ch);
                    extract_char(ch);
                    break;

                case POSITION_DEAD  :
                    break;

                default:
                    act("\n\r$c0014$n si confonde tra la folla e scompare per sempre...\n\r", FALSE, ch, 0, ch, TO_ROOM);
                    extract_char(ch);
                    break;
            }
        }
    }

    if(s == SPELL_CREEPING_DEATH) {
        REMOVE_BIT(ch->specials.affected_by, AFF_SILENCE);
    }

	if(s == SPELL_CHARM_PERSON || s == SPELL_CHARM_MONSTER) {
		check_decharm(ch);
	}

	if(s == SPELL_FLY) {
		check_falling(ch);
	}

	if(s == SPELL_WATER_BREATH) {
		check_drowning(ch);
	}

	if(IsAbsorptionShieldSpell(s)) {
		RefundAbsorptionShield(ch, s);
		affect_from_char(ch, static_cast<short>(s));
	}
}

int check_nature(struct char_data* i) {
	if(check_falling(i)) {
		return TRUE;
	}
	check_drowning(i);
	return FALSE;
}
int CheckMulti(struct char_data* i) {
	char buf[256];
	struct char_data* test;
	for(test=character_list; test; test=test->next) {

		if(GET_AUTHCODE(test)
				&& GET_AUTHCODE(i)
				&& test!=i) {
			if(!strcmp(GET_AUTHCODE(test),GET_AUTHCODE(i)) && !IS_DIO_MINORE(test) && !IS_DIO_MINORE(i)) {
				sprintf(buf,"Che ci fate tu e %s qui contemporaneamente?\n\r",
						GET_NAME(i));
				send_to_char(buf,test);
				send_to_char("Lo spirito vitale ti abbandona.\n\r"
							 "Alar ti ha congelato",test);
				act("$N e' stat$B congelat$B da Alar",FALSE,test,0,
					test,TO_NOTVICT);
				SET_BIT(test->specials.act,PLR_FREEZE);
				sprintf(buf,"Che ci fate tu e %s qui contemporaneamente?\n\r",
						GET_NAME(test));
				send_to_char(buf,i);
				send_to_char("Lo spirito vitale ti abbandona."
							 "Alar ti ha congelato!\n\r",i);
				act("$N e' stat$B congelat$B da Alar",FALSE,i,0,
					i,TO_NOTVICT);
				SET_BIT(i->specials.act,PLR_FREEZE);
				mudlog(LOG_PLAYERS,"%s e %s congelati per MP",
					   GET_NAME(test),
					   GET_NAME(i));
				do_title(test,"has been frozen by Alar!",CMD_TITLE);
				do_title(i,"has been frozen by Alar!",CMD_TITLE);
				return 1;
			}
		}
	}
	return 0;
}
void CheckSpecialties(struct char_data* ch, struct affected_type* af)
/* gestioni speciali per classi/razze.
 * Viene chiamata per ogni spell, per ogni PC
 * Sara' bene tenerla "leggera" */
{
	int i=0,sulcorpo=0;
	struct obj_data* ob;
	if(GET_RACE(ch)==RACE_DARK_ELF && af->type==SPELL_GLOBE_DARKNESS
			&& GetMaxLevel(ch) >= EXPERT) {
		af->duration=10; /* tanto per renderla infinita */
	}
	if(af->type==SKILL_MANTRA) {
		/* Attenzione, questo test e ripetuto anche in skills.c */
		for(i=0; i<=MAX_WEAR_POS; i++) {
			ob=(struct obj_data*)WEARING(ch,i);
			if(ob && (isname("tunica",ob->name) || isname("robe",ob->name))) {
				sulcorpo=1;
				break;
			}
		}
		if(!sulcorpo) {
			send_to_char("Devi indossare vesti piu adatte alla meditazione.",ch);
			af->duration=0;
		}

	}
}

namespace {
/** Messaggio di dissoluzione per N corpi uguali nella stessa stanza. */
[[nodiscard]] std::string corpse_group_message(const std::string& short_descr, std::size_t count) {
	static const std::string prefix = "il corpo di ";
	const std::string n = std::to_string(count);
	if(short_descr.compare(0, prefix.size(), prefix) == 0) {
		return n + " corpi di " + short_descr.substr(prefix.size()) + " si dissolvono.";
	}
	return n + " resti si dissolvono (" + short_descr + ").";
}

/**
 * Annuncia la dissoluzione a ciascun presente, come TO_CHAR su se stesso:
 * non dipende piu' dal poter vedere il primo occupante della stanza
 * (prima act() usava room->people come attore con hide_invisible).
 * $p resta soggetto a CAN_SEE_OBJ del singolo spettatore.
 */
void announce_corpse_decay(room_data* room, const std::string& text, obj_data* corpse) {
	for(char_data* viewer = room->people; viewer != nullptr; viewer = viewer->next_in_room) {
		if(viewer->desc != nullptr) {
			act(text.c_str(), false, viewer, corpse, nullptr, TO_CHAR);
		}
	}
}

/**
 * Timer dei cadaveri: quelli in terra che scadono nello stesso passaggio
 * vengono annunciati con una riga sola per stanza e descrizione.
 */
void update_corpses() {
	std::vector<obj_data*> expired_in_room;

	for(obj_data* j = object_list; j != nullptr;) {
		obj_data* const next_thing = j->next;

		if(GET_ITEM_TYPE(j) == ITEM_CONTAINER && j->obj_flags.value[3]) {
			if(j->obj_flags.timer > 0) {
				j->obj_flags.timer--;
			}
			else if(!j->carried_by) {  /* E' in terra.. */
				j->obj_flags.timer++;
			}
			if(!j->obj_flags.timer) {
				if(j->carried_by) {
					act("$p si dissolve nelle tue mani e tutto quello che"
						" aveva addosso cade a terra.", false, j->carried_by, j, nullptr, TO_CHAR);
					act("$p si dissolve nelle mani di $n e tutto quello che"
						" aveva addosso cade a terra.", false, j->carried_by, j, nullptr, TO_ROOM);
					ObjFromCorpse(j);
				}
				else if(j->in_room != NOWHERE) {
					expired_in_room.push_back(j);
				}
				else {
					ObjFromCorpse(j);
				}
			}
		}
		j = next_thing;
	}

	/* raggruppa per stanza e descrizione, mantenendo l'ordine */
	std::map<std::pair<long, std::string>, std::vector<obj_data*>> groups;
	std::vector<std::pair<long, std::string>> order;
	for(obj_data* corpse : expired_in_room) {
		const std::string descr = corpse->short_description != nullptr ? corpse->short_description : "";
		std::pair<long, std::string> key(static_cast<long>(corpse->in_room), descr);
		std::vector<obj_data*>& bucket = groups[key];
		if(bucket.empty()) {
			order.push_back(key);
		}
		bucket.push_back(corpse);
	}

	for(const auto& key : order) {
		const std::vector<obj_data*>& bucket = groups[key];
		room_data* const room = real_roomp(bucket.front()->in_room);
		if(room != nullptr) {
			if(bucket.size() == 1) {
				announce_corpse_decay(room, "$p si dissolve.", bucket.front());
			}
			else {
				announce_corpse_decay(room, corpse_group_message(key.second, bucket.size()), nullptr);
			}
		}
		for(obj_data* corpse : bucket) {
			ObjFromCorpse(corpse);
		}
	}
}
} // namespace

void affect_update(unsigned long localPulse) {
	static struct affected_type* af, *next_af_dude;
	struct char_data* ch;
	struct char_data*  next_char;
	struct room_data* rp;
	int dead=FALSE;
	int full_gain=0;
	int thirst_gain=0;
	int drunk_gain=0;
	int k =0;
	int ggtmp=0;
	int posprev=0;
	int regainroom=0;

	/* Chiamata ogni ora mud, ovvero 4*75 pulse */
	for(ch = character_list; ch; ch = next_char) {
		next_char = ch->next;
		if(IS_SET(ch->specials.act,PLR_FREEZE)) {
			continue;
		}
		if(CheckMulti(ch)) {
			continue;
		}
		/* Imposta un po' di puntatori e flag */
		rp = real_roomp(ch->in_room);
        if(!IS_SET(rp->room_flags, NO_REGAIN))
        {
            regainroom = 1;
        }

		/* Calcola la posizione prevalente (indici 0..E_POSITIONS_MAX) */
		for(k = 0; k < E_POSITIONS_COUNT; k++) {
			ggtmp = GET_TEMPO_IN(ch, k);
			if(ggtmp>posprev) {
				GET_POS_PREV(ch)=k;
				posprev=ggtmp;
			}
		}
		GET_TEMPO_IN(ch,GET_POS_PREV(ch))=MAX(0,ggtmp-(PULSE_PER_SEC *
											SECS_PER_MUD_HOUR));
		if(ch->nMagicNumber != CHAR_VALID_MAGIC) {
			mudlog(LOG_SYSERR, "Invalid char magic number %d in affect_update",
				   ch->nMagicNumber);
			abort();
		}

		/*
		 *  check the effects on the char
		 */
		dead=FALSE;

		for(af = ch->affected; af && !dead; af = next_af_dude)
			/* Affect loop */
		{
			next_af_dude = af->next;
			if(IsInnateAffectType(af->type)) {
				continue;
			}
			CheckSpecialties(ch,af);

			if(af->duration >= 1) {
				af->duration--;

				if(af->duration == 1 &&
						(!af->next ||
						 af->next->type != af->type ||
						 af->next->duration != 2) &&
						!(af->next && af->next->type == af->type &&
						  af->next->duration == 1))
					/* && af->location != APPLY_INTRINSIC) */
				{
					SpellWearOffSoon(af->type, ch);
				}
			}
			else {
				/* It must be a spell */
				if(af->type > 0 && af->type < FIRST_BREATH_WEAPON) {
					int iType = af->type;

					/* Absorption shields: refund once and strip, advancing past
					 * any companion before free. */
					if(IsAbsorptionShieldSpell(iType)) {
						while(next_af_dude && IsAbsorptionShieldSpell(next_af_dude->type)) {
							next_af_dude = next_af_dude->next;
						}
						SpellWearOff(iType, ch);
					}
					else {
						if(!af->next || af->next->type != af->type ||
								af->next->duration > 0) {
							SpellWearOff(af->type, ch);
						}

						/* Se il tipo e' SPELL_CHARM_PERSON o STATUS_QUEST,
						 * l'affect e' gia' stato tolto da SpellWearOff */
						if(iType == STATUS_QUEST && !char_is_live(ch)) {
							dead = TRUE;
							break;
						}
						if(iType != SPELL_CHARM_PERSON && iType != STATUS_QUEST) {
							check_memorize(ch, af);
							affect_remove(ch, af);
							if(iType == SPELL_POLY_SELF || iType == SPELL_TREE || iType == SPELL_CHANGE_FORM)
							{
								if(IS_SET(ch->specials.act, ACT_POLYSELF))
								{
									do_return(ch, "", -1);
								}
							}
						}
					}
				}
				else if(af->type >= FIRST_BREATH_WEAPON &&
						af->type <= LAST_BREATH_WEAPON) {
					const int breath_damage = -(af->modifier / 2);
					bweapons[ af->type-FIRST_BREATH_WEAPON ](breath_damage, ch, "",
							SPELL_TYPE_SPELL, ch, 0);
					if(!ch->affected) {
						/* oops, you're dead :) */
						dead = TRUE;
						break;
					}
					affect_remove(ch, af);
				}
			}
		} /*Fine affect loop*/

		if(!dead)
			/* Regen check*/
		{
			/* Ho tolto && (i->desc || !IS_PC(i)) cosi' i PC riguadagnano mana
			 * anche se sono link dead, ma non se sono nelle "storage room", ossia
			 * nelle stanze dove finiscono i polimorfati. Inoltre esiste il rischio
			 * di morire per il veleno anche se si finisce LD.
			 */
			if(GET_POS(ch) >= POSITION_STUNNED && ch->in_room != 3 &&
					ch->in_room != 2) {
				/* note - because of poison, this one has to be in the
				 * opposite order of the others.  The logic:
				 *
				 * hit_gain() modifies the characters hps if they are poisoned.
				 * but if they were in the opposite order,
				 * the total would be: hps before poison + gain.  But of course,
				 * the hps after poison are lower, but No one cares!
				 * and that is why the gain is added to the hits, not vice versa
				 */
#if NOEVENTS
				int hgain = hit_gain(ch);
				if(ch==NULL || GET_NAME(ch)==NULL) { // SALVO controllo che dopo hit_gain sia ancora vivo
					return;
				}
				GET_HIT(ch)  = MIN((regainroom * hgain) + GET_HIT(ch),  hit_limit(ch));
				GET_MANA(ch) = MIN(GET_MANA(ch) + (regainroom * mana_gain(ch)), mana_limit(ch));
				GET_MOVE(ch) = MIN(GET_MOVE(ch) + (regainroom * move_gain(ch)), move_limit(ch));
#endif
				if(GET_POS(ch) == POSITION_STUNNED) {
					update_pos(ch);
				}
			}
			else if(GET_POS(ch) == POSITION_INCAP) {
				/* do nothing */
				damage(ch, ch, 0, TYPE_SUFFERING, 5);
			}
			else if(GET_POS(ch) == POSITION_MORTALLYW) {
				damage(ch, ch, 1, TYPE_SUFFERING, 5);
			}

			if(IS_PC(ch)) {
				update_char_objects(ch);
				if(GetMaxLevel(ch) < DIO && ch->in_room != 3 && ch->in_room != 2) {
					check_idling(ch);
				}
				/*
				 * Se e' linkdead
				 * non diventa assetato od affamato.
				 * */
				if(rp && ch->desc) {
					/*
					* Controllo sul tipo della stanza
					* */
					if(rp->sector_type == SECT_WATER_SWIM ||
							rp->sector_type == SECT_WATER_NOSWIM) {
						full_gain=1;
						drunk_gain=1;
						thirst_gain=-10;
					}
					else if(rp->sector_type == SECT_DESERT) {
						full_gain=1;
						drunk_gain=2;
						thirst_gain=2;
					}

					else if(rp->sector_type == SECT_MOUNTAIN ||
							rp->sector_type == SECT_HILLS) {
						full_gain=2;
						drunk_gain=3;
						thirst_gain=1;
					}
					else if(rp->sector_type == SECT_INSIDE ||
							rp->sector_type == SECT_CITY) {
						full_gain=1;
						drunk_gain=2;
						thirst_gain=1;
					}
					else {
						full_gain=1;
						drunk_gain=1;
						thirst_gain=1;
					} /* Fine Check per stanze */
					/*
					* Controllo per razza
					* */
					if(GET_RACE(ch) == RACE_HALFLING) {
						full_gain+=1;
						drunk_gain+=1;
						thirst_gain+=1;
					}
					else if((GET_RACE(ch) == RACE_HALF_OGRE) || (GET_RACE(ch) == RACE_HALF_GIANT)) {
						full_gain+=2;
						drunk_gain+=0;
						thirst_gain+=1;
					}
					/*
					* Controllo per classe
					* */
					if(HasClass(ch,CLASS_MONK)) {
						if(localPulse % (PULSE_TICK*
										 MAX((GET_LEVEL(ch,MONK_LEVEL_IND)-29),1)
										)
						  ) {
							full_gain=0;
						}
						thirst_gain=0;
					}
					gain_condition(ch,FULL,-MAX(0,full_gain) * regainroom);
					gain_condition(ch,DRUNK,-MAX(0,drunk_gain) * regainroom);
					gain_condition(ch,THIRST,-MAX(0,thirst_gain) * regainroom);

				}
				if(ch->specials.tick == time_info.hours) {
					/* works for 24, change for anything else */
					/* the special case for room 3 is a hack to keep link dead
					* people who have no stuff from being saved without stuff..  */
					if(!IS_IMMORTAL(ch)
							&& ch->in_room != 3
							&& ch->in_room != NOWHERE) {
						do_save(ch,"",0);
					}
				}
			}
			else if(IS_POLY(ch)) {
				if(GetMaxLevel(ch) < DIO && ch->in_room != 3 && ch->in_room != 2) {
					check_idling(ch);
				}
			}
			check_nature(ch);  /* check falling, check drowning, etc */
		} /*Regen check loop */

	}

	update_corpses();
}




/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data* ch, struct char_data* victim) {
	struct char_data* k;

	for(k=victim; k; k=k->master) {
		if(k == ch) {
			return(TRUE);
		}
	}

	return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data* ch) {
	struct follow_type* j, *k;

	if(!ch->master) {
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM)) {
		act("Realizzi che $N ti ha ingannato!", FALSE, ch, 0, ch->master, TO_CHAR);
		act("$n realizza che $N l'ha ingannat$b!", FALSE, ch, 0, ch->master,
			TO_NOTVICT);
		act("$n ti odia!", FALSE, ch, 0, ch->master, TO_VICT);
		if(affected_by_spell(ch, SPELL_CHARM_PERSON)) {
			affect_from_char(ch, SPELL_CHARM_PERSON);
		}
	}
	else {
		act("Smetti di seguire $N.", FALSE, ch, 0, ch->master, TO_CHAR);
		if(!IS_SET(ch->specials.act,PLR_STEALTH)) {
			act("$n smette di seguire $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
			act("$n smette di seguirti.", FALSE, ch, 0, ch->master, TO_VICT);
		}
	}

	if(ch->master->followers->follower == ch) {
		/* Head of follower-list? */
		k = ch->master->followers;
		ch->master->followers = k->next;
		free(k);
	}
	else {
		/* locate follower who is not head of list */

		for(k = ch->master->followers; k->next && k->next->follower!=ch;
				k=k->next)
			;

		if(k->next) {
			j = k->next;
			k->next = j->next;
			free(j);
		}
		else {
			assert(FALSE);
		}
	}

	ch->master = 0;
	REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data* ch) {
	struct follow_type* j, *k;

	if(ch->master) {
		stop_follower(ch);
	}

	for(k=ch->followers; k; k=j) {
		j = k->next;
		stop_follower(k->follower);
	}
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data* ch, struct char_data* leader) {
	struct follow_type* k;

	if(!ch || !leader || ch->nMagicNumber != CHAR_VALID_MAGIC ||
	   leader->nMagicNumber != CHAR_VALID_MAGIC) {
		mudlog(LOG_SYSERR, "add_follower: invalid ch=%p leader=%p", (void*)ch,
			   (void*)leader);
		return;
	}

#if 0
	assert(!ch->master);
#else
	/* instead of crashing the mud we try this */
	if(ch->master) {
		act("$n non puo' seguirti, ha le sue valide ragioni!",TRUE,ch,0,leader,TO_CHAR);
		act("Per delle valide ragioni non puoi seguire $N.",TRUE,ch,0,leader,TO_CHAR);
		mudlog(LOG_SYSERR, "%s cannot follow %s for some reason", GET_NAME(ch),
			   GET_NAME(leader));
		return;
	}
#endif

	ch->master = leader;

	CREATE(k, struct follow_type, 1);

	k->follower = ch;
	k->next = leader->followers;
	leader->followers = k;


	act("Inizi a seguire $N.", FALSE, ch, 0, leader, TO_CHAR);
	if(!IS_SET(ch->specials.act, PLR_STEALTH)) {
		act("$n inizia a seguirti.", TRUE, ch, 0, leader, TO_VICT);
		act("$n inizia a seguire $N.", TRUE, ch, 0, leader, TO_NOTVICT);
	}
}


struct syllable {
	char achOrg[10];
	char achNew[10];
};

struct syllable syls[] = {
	{ " ", " " },
	{ "ar", "abra"   },
	{ "au", "kada"    },
	{ "bless", "fido" },
	{ "blind", "nose" },
	{ "bur", "mosa" },
	{ "cu", "judi" },
	{ "ca", "jedi" },
	{ "de", "oculo"},
	{ "en", "unso" },
	{ "light", "dies" },
	{ "lo", "hi" },
	{ "mor", "zak" },
	{ "move", "sido" },
	{ "ness", "lacri" },
	{ "ning", "illa" },
	{ "per", "duda" },
	{ "ra", "gru"   },
	{ "re", "candus" },
	{ "son", "sabru" },
	{ "se",  "or"},
	{ "tect", "infra" },
	{ "tri", "cula" },
	{ "ven", "nofo" },
	{"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
	{"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
	{"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
	{"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
};

void say_spell(struct char_data* ch, int si) {
	char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	int j, offs;
	struct char_data* temp_char;

	strcpy(buf, "");
	strcpy(splwd, spells[si-1]);

	offs = 0;

	while(*(splwd+offs)) {
		for(j=0; *(syls[j].achOrg); j++) {
			if(strncmp(syls[j].achOrg, splwd+offs, strlen(syls[j].achOrg))==0) {
				strcat(buf, syls[j].achNew);
				if(strlen(syls[j].achOrg)) {
					offs+=strlen(syls[j].achOrg);
				}
				else {
					++offs;
				}
			}
		}
	}




	std::snprintf(buf2, sizeof(buf2), "$n pronuncia le parole, '$c0015%.20000s$c0007'.", buf);
	std::snprintf(buf, sizeof(buf), "$n pronuncia le parole, '$c0015%s$c0007'.", spells[si-1]);

	for(temp_char = real_roomp(ch->in_room)->people;
			temp_char;
			temp_char = temp_char->next_in_room) {
		if(temp_char != ch) {
			if(GET_RACE(ch) == GET_RACE(temp_char)) {
				act(buf, FALSE, ch, 0, temp_char, TO_VICT);
			}
			else {
				act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
			}

		}
	}
}



bool saves_spell(struct char_data* ch, sh_int save_type) {
	int save;

	/* Negative apply_saving_throw makes saving throw better! */
	save = ch->specials.apply_saving_throw[save_type];

	if(!IS_NPC(ch)) {
		save += saving_throws[BestMagicClass(ch)][save_type]
				[(int)GET_LEVEL(ch, BestMagicClass(ch)) ];
		if(GetMaxLevel(ch) > MAX_MORT) {
			return(TRUE);
		}
	}

	if(GET_RACE(ch) == RACE_GOD) {  /* gods always save */
		return(1);
	}

	return(MAX(1,save) < number(1,20));
}

bool ImpSaveSpell(struct char_data* ch, sh_int save_type, int mod) {
	int save;

	/* Positive mod is better for save */

	/* Negative apply_saving_throw makes saving throw better! */

	save = ch->specials.apply_saving_throw[save_type] - mod;

	if(!IS_NPC(ch)) {

		save += saving_throws[ BestMagicClass(ch) ][ save_type ]
				[(int)GET_LEVEL(ch, BestMagicClass(ch)) ];
		if(GetMaxLevel(ch) >= IMMORTALE) {
			return(TRUE);
		}
	}

	return(MAX(1,save) < number(1,20));
}



char* skip_spaces(const char* buffer) {
	for(; *buffer && (*buffer)==' '; buffer++);

	return const_cast<char*>(buffer);
}

namespace {
[[nodiscard]] bool EqNotForCaster(char_data* mind, char_data* body, int spl);

/**
 * Corpo che "pensa" il cast: l'originale se polimorfato (poly self),
 * altrimenti ch stesso. Livelli, classi, skill e mana vengono da qui;
 * stanza, posizione, equipaggiamento e bersagli dal corpo attivo.
 */
[[nodiscard]] char_data* cast_mind(char_data* ch) {
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}

/** Guerrieri, ladri e monaci non lanciano incantesimi. */
[[nodiscard]] bool caster_class_ok(char_data* mind, char_data* ch) {
	if(IS_IMMORTAL(mind) || IS_PRINCE(mind)) {
		return true;
	}
	const int best = BestMagicClass(mind);
	if(best == WARRIOR_LEVEL_IND) {
		send_to_char("Pensa a combattere, idiota!\n\r", ch);
		return false;
	}
	if(best == THIEF_LEVEL_IND) {
		send_to_char("Credi di poter rubare anche la magia?\n\r", ch);
		return false;
	}
	if(best == MONK_LEVEL_IND) {
		send_to_char("Ehi... ma questa e' stregoneria!\n\r", ch);
		return false;
	}
	return true;
}

/** cast / mind / recall devono corrispondere alla classe. */
[[nodiscard]] bool cast_command_ok(int cmd, char_data* mind, char_data* ch) {
	if(cmd == CMD_SPELLID || IS_IMMORTAL(mind)) {
		return true;
	}
	if(cmd != CMD_MIND && OnlyClass(mind, CLASS_PSI)) {
		send_to_char("Usa la mente! (mind)\n\r", ch);
		return false;
	}
	if(cmd != CMD_RECALL && OnlyClass(mind, CLASS_SORCERER)) {
		send_to_char("Usa la memoria! (recall).\n\r", ch);
		return false;
	}
	if(cmd == CMD_MIND && !HasClass(mind, CLASS_PSI)) {
		send_to_char("Non sei uno psi!\n\r", ch);
		return false;
	}
	if(cmd == CMD_RECALL && !HasClass(mind, CLASS_SORCERER)) {
		send_to_char("Non sei uno stregone!\n\r", ch);
		return false;
	}
	return true;
}

void send_position_refusal(char_data* ch) {
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING:
		send_to_char("Sogni di armi, amori e grandi poteri.\n\r", ch);
		break;
	case POSITION_RESTING:
		send_to_char("E alzati prima, no?.\n\r", ch);
		break;
	case POSITION_SITTING:
		send_to_char("Stai ancora annaspando per rialzarti!\n\r", ch);
		break;
	case POSITION_FIGHTING:
		send_to_char("Un pugno nei denti ti impedisce di formulare l'incantesimo.\n\r", ch);
		break;
	default:
		send_to_char("Non mi sembri in gran forma!\n\r", ch);
		break;
	}
}

/** Miglior livello di classe che conosce la spell, e livello della spell. */
void find_cast_levels(char_data* mind, int spl, int& caster_level, int& spell_level) {
	struct LevelPair {
		int min_level;
		int class_index;
	};
	const LevelPair pairs[] = {
		{ spell_info[spl].min_level_magic,    MAGE_LEVEL_IND },
		{ spell_info[spl].min_level_sorcerer, SORCERER_LEVEL_IND },
		{ spell_info[spl].min_level_cleric,   CLERIC_LEVEL_IND },
		{ spell_info[spl].min_level_paladin,  PALADIN_LEVEL_IND },
		{ spell_info[spl].min_level_ranger,   RANGER_LEVEL_IND },
		{ spell_info[spl].min_level_psi,      PSI_LEVEL_IND },
	};
	for(const LevelPair& pair : pairs) {
		const int tlev = GET_LEVEL(mind, pair.class_index);
		if(pair.min_level <= tlev && tlev > caster_level) {
			caster_level = tlev;
			spell_level = pair.min_level;
		}
	}
	/* il druido vince sempre se ha il livello (comportamento storico) */
	const int druid_level = GET_LEVEL(mind, DRUID_LEVEL_IND);
	if(spell_info[spl].min_level_druid <= druid_level) {
		caster_level = druid_level;
		spell_level = spell_info[spl].min_level_druid;
	}
	if(caster_level > 10) {
		caster_level -= (HowManyClasses(mind) - 1);
	}
}

struct CastTarget {
	char_data* tar_char = nullptr;
	obj_data* tar_obj = nullptr;
	bool ok = false;
};

/** Cerca il bersaglio indicato per nome. */
CastTarget find_named_target(char_data* ch, int spl, char* name) {
	CastTarget t;
	const auto targets = spell_info[spl].targets;

	if(IS_SET(targets, TAR_CHAR_ROOM)) {
		t.tar_char = get_char_room_vis(ch, name);
		if(t.tar_char != nullptr) {
			if(t.tar_char == ch || t.tar_char == ch->specials.fighting ||
			   t.tar_char->attackers < 6 || t.tar_char->specials.fighting == ch) {
				t.ok = true;
			}
			else {
				send_to_char("Volano troppi schiaffi qui intorno!\n\r", ch);
			}
		}
	}
	if(!t.ok && IS_SET(targets, TAR_CHAR_WORLD)) {
		t.tar_char = get_char_vis(ch, name);
		t.ok = t.tar_char != nullptr;
	}
	if(!t.ok && IS_SET(targets, TAR_OBJ_INV)) {
		t.tar_obj = get_obj_in_list_vis(ch, name, ch->carrying);
		t.ok = t.tar_obj != nullptr;
	}
	if(!t.ok && IS_SET(targets, TAR_OBJ_ROOM)) {
		t.tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents);
		t.ok = t.tar_obj != nullptr;
	}
	if(!t.ok && IS_SET(targets, TAR_OBJ_WORLD)) {
		t.tar_obj = get_obj_vis(ch, name);
		t.ok = t.tar_obj != nullptr;
	}
	if(!t.ok && IS_SET(targets, TAR_OBJ_EQUIP)) {
		for(int i = 0; i < MAX_WEAR && !t.ok; i++) {
			if(ch->equipment[i] != nullptr && str_cmp(name, ch->equipment[i]->name) == 0) {
				t.tar_obj = ch->equipment[i];
				t.ok = true;
			}
		}
	}
	if(!t.ok && IS_SET(targets, TAR_SELF_ONLY) && str_cmp(GET_NAME(ch), name) == 0) {
		t.tar_char = ch;
		t.ok = true;
	}
	if(!t.ok && IS_SET(targets, TAR_NAME)) {
		/* storico: le spell TAR_NAME ricevono il nome al posto dell'oggetto */
		t.tar_obj = reinterpret_cast<obj_data*>(name);
		t.ok = true;
	}
	return t;
}

/** Bersaglio implicito (nessun nome scritto). */
CastTarget find_default_target(char_data* ch, int spl) {
	CastTarget t;
	const auto targets = spell_info[spl].targets;

	if(IS_SET(targets, TAR_FIGHT_SELF)) {
		char_data* supported = nullptr;
		if(ch->specials.fighting && ch->specials.fighting->specials.fighting) {
			t.tar_char = ch->specials.fighting->specials.fighting;
			t.ok = true;
		}
		else if(ch->specials.supporting &&
				(supported = get_char_room_vis(ch, ch->specials.supporting)) != nullptr &&
				supported->specials.fighting &&
				supported->specials.fighting->specials.fighting) {
			t.tar_char = supported->specials.fighting->specials.fighting;
			t.ok = true;
		}
	}
	if(!t.ok && IS_SET(targets, TAR_FIGHT_VICT)) {
		char_data* supported = nullptr;
		if(ch->specials.fighting) {
			t.tar_char = ch->specials.fighting;
			t.ok = true;
		}
		else if(ch->specials.supporting &&
				(supported = get_char_room_vis(ch, ch->specials.supporting)) != nullptr &&
				supported->specials.fighting) {
			t.tar_char = supported->specials.fighting;
			t.ok = true;
		}
	}
	if(!t.ok && IS_SET(targets, TAR_SELF_ONLY)) {
		t.tar_char = ch;
		t.ok = true;
	}
	if(!t.ok && IS_SET(targets, TAR_ROOM)) {
		t.tar_char = nullptr;
		t.ok = true;
	}
	return t;
}

void send_no_target(char_data* ch, int spl, bool named) {
	const auto targets = spell_info[spl].targets;
	if(!named) {
		if(targets < TAR_OBJ_INV) {
			send_to_char("Su chi vorresti lanciare l'incantesimo?\n\r", ch);
		}
		else {
			send_to_char("Su cosa vorresti lanciare l'incantesimo?\n\r", ch);
		}
		return;
	}
	if(IS_SET(targets, TAR_CHAR_WORLD)) {
		send_to_char("Non c'e' nessuno con quel nome.\n\r", ch);
	}
	else if(IS_SET(targets, TAR_CHAR_ROOM)) {
		send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
	}
	else if(IS_SET(targets, TAR_OBJ_INV)) {
		send_to_char("Non hai con te niente del genere.\n\r", ch);
	}
	else if(IS_SET(targets, TAR_OBJ_ROOM)) {
		send_to_char("Non c'e' niente del genere qui.\n\r", ch);
	}
	else if(IS_SET(targets, TAR_OBJ_WORLD)) {
		send_to_char("Non c'e' niente del genere.\n\r", ch);
	}
	else if(IS_SET(targets, TAR_OBJ_EQUIP)) {
		send_to_char("Non stai indossando niente del genere.\n\r", ch);
	}
}

void debug_spellfail(char_data* mind, char_data* ch, const std::string& text) {
	if(IS_MAESTRO_DEGLI_DEI(mind)) {
		send_to_char((text + "\n\r").c_str(), ch);
	}
}

/** Tetto del dado di fallimento (piu' alto = piu' facile fallire). */
int spellfail_ceiling(char_data* mind, char_data* ch, int spl, int caster_level,
					  int spell_level, bool& eq_penalty) {
	int max = ch->specials.spellfail;
	debug_spellfail(mind, ch, "Spellfail PRIMA del check sull'eq: " + std::to_string(max));
	mudlog(LOG_CHECK, "Checking eq after spell");

	eq_penalty = false;
	if(!IS_IMMORTAL(mind) && EqNotForCaster(mind, ch, spl)) {
		eq_penalty = true;
		max += 20; /* 40% harder to cast spells */
		if(OnlyClass(mind, CLASS_SORCERER)) {
			max += 10;
		}
	}
	debug_spellfail(mind, ch, "Spellfail DOPO il check sull'eq: " + std::to_string(max));
	max -= SPELLFAIL_MOD(caster_level, spell_level);
	debug_spellfail(mind, ch, "Spellfail DOPO aggiustamento : " + std::to_string(max));
	max += GET_COND(ch, DRUNK) * 10; /* 0 - 240 */

	const int caster_align = GET_ALIGNMENT(ch) + 1000;
	int spell_align = 0;
	if(spell_info[spl].align_modifier < 0) {
		spell_align = -1;
	}
	else if(spell_info[spl].align_modifier > 0) {
		spell_align = 1;
	}
	spell_align = spell_align * 1000 + 1000;
	debug_spellfail(mind, ch, "Align: Spell " + std::to_string(spell_align) +
					" Caster " + std::to_string(caster_align));
	debug_spellfail(mind, ch, "Spellfail prima dell'align : " + std::to_string(max));
	max += (std::abs(spell_align - caster_align) - 1000) / 50;
	debug_spellfail(mind, ch, "Spellfail dopo l'align : " + std::to_string(max));

	if(ch->attackers > 0) {
		max += spell_info[spl].spellfail;
	}
	else if(ch->specials.fighting) {
		max += spell_info[spl].spellfail / 2;
	}
	if(IS_IMMORTAL(mind)) {
		max = std::max(1, static_cast<int>(mind->skills[spl].learned));
	}
	return max;
}

/** Scala il mana dal corpo originale e allinea quello del poly. */
void spend_cast_mana(char_data* mind, char_data* ch, int amount) {
	if(!IS_PC(mind)) {
		return;
	}
	GET_MANA(mind) -= amount;
	alter_mana(mind, 0);
	if(mind != ch) {
		GET_MANA(ch) = GET_MANA(mind);
	}
}

[[nodiscard]] bool anti_magic_blocks(char_data* mind, char_data* ch, char_data* tar_char, int cmd) {
	if(cmd == CMD_MIND || IS_IMMORTAL(mind)) {
		return false;
	}
	if(tar_char != nullptr && affected_by_spell(tar_char, SPELL_ANTI_MAGIC_SHELL)) {
		act("Il tuo incantesimo svanisce contro lo scudo $c0012anti-magia$c0007 di $N!", false, ch, nullptr, tar_char, TO_CHAR);
		act("$n prova inutilmente a lanciare un incantesimo contro lo scudo $c0012anti-magia$c0007 di $N", false, ch, nullptr, tar_char, TO_ROOM);
		if(ch != tar_char) {
			act("$n lancia un incantesimo ed impreca mentre svanisce contro lo scudo $c0012anti-magia$c0007!", false, ch, nullptr, tar_char, TO_VICT);
		}
		return true;
	}
	return false;
}

[[nodiscard]] bool zone_blocks_magic(char_data* mind, char_data* ch, int cmd) {
	if(IS_IMMORTAL(mind)) {
		return false;
	}
	if(cmd == CMD_CAST &&
	   check_nomagic(ch, "Il mana si rifuta di scorrere in questa zona.",
					 "La magia di $n muore sulle sue labbra.")) {
		return true;
	}
	if(cmd == CMD_RECALL &&
	   check_nomagic(ch, "Le parole ti si confondono davanti agli occhi in questa zona.",
					 "La magia di $n muore sulle sue labbra.")) {
		return true;
	}
	if(cmd == CMD_MIND &&
	   check_nomind(ch, "Non riesci a concentrarti abbastanza in questo posto.",
					"$n cerca invano di concentrarsi.")) {
		return true;
	}
	return false;
}

void send_gibberish(char_data* ch) {
	switch(number(1, 5)) {
	case 1:
		send_to_char("Bylle Grylle Grop Gryf???\n\r", ch);
		break;
	case 2:
		send_to_char("Olle Bolle Snop Snyf?\n\r", ch);
		break;
	case 3:
		send_to_char("Olle Grylle Bolle Bylle?!?\n\r", ch);
		break;
	case 4:
		send_to_char("Gryffe Olle Gnyffe Snop???\n\r", ch);
		break;
	default:
		send_to_char("Bolle Snylle Gryf Bylle?!?\n\r", ch);
		break;
	}
}

/*
 * Equipaggiamento che impedisce di castare: i livelli di classe vengono
 * da chi pensa il cast (mind), l'equipaggiamento dal corpo che lo indossa.
 */
[[nodiscard]] bool EqNotForCaster(char_data* mind, char_data* body, int spl) {
	const spell_info_type& info = spell_info[spl];
	return (info.min_level_magic > GET_LEVEL(mind, MAGE_LEVEL_IND) || EqWBits(body, ITEM_ANTI_MAGE)) &&
		   (info.min_level_sorcerer > GET_LEVEL(mind, SORCERER_LEVEL_IND) ||
			EqWBits(body, ITEM_ANTI_MAGE) || EqWBits(body, ITEM_ANTI_SORCERER)) &&
		   (info.min_level_cleric > GET_LEVEL(mind, CLERIC_LEVEL_IND) || EqWBits(body, ITEM_ANTI_CLERIC)) &&
		   (info.min_level_paladin > GET_LEVEL(mind, PALADIN_LEVEL_IND) || EqWBits(body, ITEM_ANTI_PALADIN)) &&
		   (info.min_level_ranger > GET_LEVEL(mind, RANGER_LEVEL_IND) || EqWBits(body, ITEM_ANTI_RANGER)) &&
		   (info.min_level_psi > GET_LEVEL(mind, PSI_LEVEL_IND) || EqWBits(body, ITEM_ANTI_PSI)) &&
		   (info.min_level_druid > GET_LEVEL(mind, DRUID_LEVEL_IND) || EqWBits(body, ITEM_ANTI_DRUID));
}
} // namespace

/* Assumes that *argument does start with first letter of chopped string */
ACTION_FUNC(do_cast) {
	/* corpo attivo (ch) e chi pensa il cast (mind: l'originale se in poly) */
	char_data* const mind = cast_mind(ch);

	if(!IsHumanoid(ch)) {
		send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
		return;
	}
	if(!caster_class_ok(mind, ch)) {
		return;
	}
	if(cmd != CMD_MIND && !IS_IMMORTAL(mind) && apply_soundproof(ch)) {
		return;
	}

	const std::string work = (arg != nullptr) ? std::string(arg).substr(0, 255) : std::string();
	const char* argument = skip_spaces(work.c_str());

	if(!*argument) {
		send_to_char("Cosa, chi, come?\n\r", ch);
		return;
	}
	if(*argument != '\'') {
		send_to_char("Usa sempre i simboli sacri della $c0012magia$c0007: $c0011'\n\r", ch);
		return;
	}
	int qend = 1;
	while(argument[qend] && argument[qend] != '\'') {
		qend++;
	}
	if(argument[qend] != '\'') {
		send_to_char("Usa sempre i simboli sacri della $c0012magia$c0007: $c0011'\n\r", ch);
		return;
	}

	const int spl = old_search_block(argument, 1, qend - 1, spells, 0);
	if(!spl) {
		send_to_char("Fantastico! Non e' successo nulla!\n\r", ch);
		return;
	}

	if(mind->skills == nullptr) {
		/* i mob senza skill non possono castare (prima andava in crash piu' avanti) */
		if(IS_PC(mind)) {
			send_to_char("Non hai alcuna skill!\n\r", ch);
		}
		return;
	}
	if(!cast_command_ok(cmd, mind, ch)) {
		return;
	}

	/* this should make sorcerer learned spells be forced to be recalled */
	if((cmd == CMD_CAST || cmd == CMD_MIND) && HasClass(mind, CLASS_SORCERER) &&
	   !IS_IMMORTAL(mind) && IS_SET(mind->skills[spl].flags, SKILL_KNOWN_SORCERER)) {
		send_to_char("Questo e' un incantesimo da stregone...\n\r", ch);
		return;
	}

	if(spl <= 0 || spl >= MAX_SKILLS || !spell_info[spl].spell_pointer) {
		if(spl > 0) {
			mudlog(LOG_SYSERR, "Spellid %d cmid %d command %s", spl, cmd, arg);
		}
		send_gibberish(ch);
		return;
	}

	if(!IS_IMMORTAL(mind) && GET_POS(ch) < spell_info[spl].minimum_position) {
		send_position_refusal(ch);
		return;
	}

	/* Controlla il numero di classi per le spell solo monoclasse */
	if(IS_SET(spell_info[spl].targets, TAR_MONOCLASSE) && GetMaxLevel(mind) < DIO &&
	   HowManyClasses(mind) > 1) {
		send_to_char("Questo incantesimo e' solo per i veri adepti!\n\r", ch);
		return;
	}

	int caster_level = -1;
	int spell_level = 100;
	find_cast_levels(mind, spl, caster_level, spell_level);

	if(!IS_IMMORTAL(mind)) {
		if(caster_level < spell_level) {
			/* skill prince */
			if(IS_PRINCE(mind) && (spl == SPELL_REINCARNATE || spl == SPELL_CURE_CRITIC ||
								   spl == SPELL_CHAIN_LIGHTNING)) {
				caster_level = PRINCIPE;
				spell_level = PRINCIPE;
			}
			else {
				send_to_char("Devi ancora crescere!\n\r", ch);
				return;
			}
		}
	}
	else {
		send_to_char(("spell: " + std::to_string(spell_level) + " caster: " +
					  std::to_string(caster_level) + "\n\r").c_str(), ch);
	}

	if(cmd == CMD_SPELLID) {
		if(!IS_CASTER(mind)) {
			send_to_char("Lascia perdere...\n\r", ch);
			return;
		}
		spellid(spl, ch, caster_level, spell_level);
		return;
	}

	argument += qend + 1;        /* Point to the last apice */
	while(*argument == ' ') {
		argument++;
	}

	if(!IS_IMMORTAL(mind) && IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
	   check_peaceful(ch, "Questa zona sembra refrattaria alla $c0012magia$c0007!\n\r")) {
		return;
	}

	if(IS_PC(mind) && GetMaxLevel(mind) < MAESTRO_DEI_CREATORI) {
		mudlog(LOG_CHECK, "%s cast %s", GET_NAME(mind), arg);
	}

	/* **************** Locate targets **************** */
	char name[MAX_INPUT_LENGTH] = "";
	CastTarget target;
	if(!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
		mudlog(LOG_CHECK, "NOT TAR_IGNORE");
		argument = one_argument(argument, name);
		target = *name ? find_named_target(ch, spl, name) : find_default_target(ch, spl);
		/* Mob ACT_IMMORTAL rifiutato anche come bersaglio di default. */
		if(target.tar_char != nullptr && target.tar_char != ch && is_untouchable_mob(target.tar_char)) {
			immortal_mob_refusal(ch, target.tar_char);
			return;
		}
	}
	else {
		target.ok = true; /* No target, is a good target */
	}

	if(!target.ok) {
		send_no_target(ch, spl, *name != '\0');
		return;
	}
	char_data* const tar_char = target.tar_char;
	obj_data* const tar_obj = target.tar_obj;

	/* Magie violente: pace, mob intoccabile o PK negato -> solo messaggio, niente mana. */
	if(tar_char != nullptr && tar_char != ch && IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
	   (combat_denied_notify(ch, tar_char, spl) || immortal_pc_denied_notify(ch, tar_char))) {
		return;
	}

	if(tar_char == ch && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
		act("Non puoi lanciare questo incantesimo su te stess$b!", false, ch, nullptr, nullptr, TO_CHAR);
		return;
	}
	if(tar_char != ch && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
		act("Puoi lanciare questo incantesimo solo su te stess$b!", false, ch, nullptr, nullptr, TO_CHAR);
		if(GetMaxLevel(mind) < DIO + 1) {
			return;
		}
	}
	else if(IS_AFFECTED(ch, AFF_CHARM) && ch->master == tar_char && tar_char != nullptr) {
		if(GET_SEX(ch->master) == SEX_FEMALE) {
			send_to_char("Hai paura che potrebbe danneggiare la tua padrona.\n\r", ch);
		}
		else {
			send_to_char("Hai paura che potrebbe danneggiare il tuo padrone.\n\r", ch);
		}
		return;
	}

	const int cost = static_cast<int>(USE_MANA(mind, spl));
	if(cmd == CMD_RECALL && !IS_IMMORTAL(mind)) {
		if(!MEMORIZED(mind, spl)) {
			send_to_char("Non hai questo incantesimo memorizzato!\n\r", ch);
			return;
		}
	}
	else if(cmd != CMD_RECALL && IS_PC(mind) && GetMaxLevel(mind) < IMMORTALE &&
			(GET_MANA(mind) < cost || GET_MANA(mind) <= 0)) {
		if(cmd != CMD_MIND) {
			send_to_char("Non hai abbastanza $c0011energia$c0007!\n\r", ch);
		}
		else {
			send_to_char("Non hai abbastanza $c0009energia mentale$c0007!\n\r", ch);
		}
		return;
	}

	if(spl != SPELL_VENTRILOQUATE && cmd != CMD_MIND) {  /* :-) */
		say_spell(ch, spl);    /* psi's do not utter! */
	}

	if(mind == ch || GetMaxLevel(mind) < DEMIGOD) {
		WAIT_STATE(ch, spell_info[spl].beats);
	}

	bool eq_penalty = false;
	const int max = spellfail_ceiling(mind, ch, spl, caster_level, spell_level, eq_penalty);
	int dice = number(1, max);
	if(cmd == CMD_RECALL) {
		dice -= dice / 10;
	}
	if(!IS_IMMORTAL(mind) && dice > mind->skills[spl].learned &&
	   !IsSpecialized(mind->skills[spl].special)) {
		send_to_char("Perdi la tua concentrazione!\n\r", ch);
		if(eq_penalty) {
			act("Certo.. con tutta quella robaccia addosso...", false, ch, nullptr, nullptr, TO_CHAR);
		}
		if(cmd == CMD_RECALL) {
			/* give chance to forget */
			if(number(1, 130) > mind->skills[spl].learned) {
				FORGET(mind, spl);
			}
		}
		else {
			spend_cast_mana(mind, ch, cost >> 1);
		}
		LearnFromMistake(mind, spl, 0, 95);
		return;
	}

	if(anti_magic_blocks(mind, ch, tar_char, cmd)) {
		return;
	}
	if(tar_char != nullptr && GET_POS(tar_char) == POSITION_DEAD) {
		send_to_char("Il tuo incantesimo svanisce sul cadavere...\n\r", ch);
		return;
	}
	/* psi shit ain't magic */
	if(cmd != CMD_MIND && !IS_IMMORTAL(mind) && affected_by_spell(ch, SPELL_ANTI_MAGIC_SHELL)) {
		act("Il tuo incantesimo svanisce contro il tuo scudo $c0012anti-magia$c0007!", false, ch, nullptr, nullptr, TO_CHAR);
		act("$n prova inutilmente a lanciare un incantesimo oltre il suo scudo $c0012anti-magia$c0007, muhahaha!", false, ch, nullptr, nullptr, TO_ROOM);
		return;
	}
	if(zone_blocks_magic(mind, ch, cmd)) {
		return;
	}

	act((std::string("Pronunci le parole, '$c0015") + spells[spl - 1] + "$c0007'.").c_str(),
		false, ch, nullptr, nullptr, TO_CHAR);

	/* costi PRIMA della spell (il forget dopo il cast non dimenticava il recall 'poly') */
	if(cmd == CMD_RECALL) {
		FORGET(mind, spl);
	}
	else {
		spend_cast_mana(mind, ch, cost);
	}

#ifdef NEW_ALIGN
	/* Intrinseca */
	if(tar_char && tar_char != ch) {
		GET_ALIGNMENT(ch) += spell_info[spl].align_modifier;
		mudlog(LOG_CHECK, "Char %s spell %s intrinseca: %5d", GET_NAME(ch), arg,
			   spell_info[spl].align_modifier);
	}
#endif
	PushStatus("AlignMod");
	AlignMod(ch, tar_char, spell_info[spl].ostility_level);
	PushStatus("About Casting spell");
	mudlog(LOG_CHECK, "Motore di cast terminato, going to real procedure");
	{
		/* righe uguali (es. spell ad area su mob uguali) raggruppate */
		const OutputBatch batch;
		(*spell_info[spl].spell_pointer)(static_cast<byte>(GET_LEVEL(mind, BestMagicClass(mind))),
										 ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj);
	}
	PopStatus();
	PopStatus();
}

void assign_spell_pointers() {
	int i;

	/* make sure defaults are 0! msw */
	for(i=0; i<MAX_SPL_LIST; i++) {
		spell_info[i].spell_pointer = 0;
		spell_info[i].min_level_cleric   = 0;
		spell_info[i].min_level_magic    = 0;
		spell_info[i].min_level_druid    = 0;
		spell_info[i].min_level_sorcerer = 0;
		spell_info[i].min_level_paladin  = 0;
		spell_info[i].min_level_ranger   = 0;
		spell_info[i].min_level_psi      = 0;
	}

}

void check_falling_obj(struct obj_data* obj, int room) {
	struct room_data* rp, *targ;
	int done, count;

	if(obj->in_room != room) {
		mudlog(LOG_SYSERR, "unusual object information in check_falling_obj");
		return;
	}

	rp = real_roomp(room);
	if(!rp) {
		return;
	}

	if(rp->sector_type != SECT_AIR) {
		return;
	}

	done = FALSE;
	count = 0;

	while(!done && count < 100) {

		if(rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
			targ = real_roomp(rp->dir_option[DOWN]->to_room);
		}
		else {
			/*
			 * pretend that this is the smash room.
			 */
			if(count > 1) {
				if(rp->people) {
					act("$p si schianta al suolo a gran velocita'.",
						FALSE, rp->people, obj, 0, TO_ROOM);
					act("$p si schianta al suolo a gran velocita'.",
						FALSE, rp->people, obj, 0, TO_CHAR);
				}
				return;

			}
			else {
				if(rp->people) {
					act("$p atterra con un rumoroso $c5009THUMP$c0007!",
						FALSE, rp->people, obj, 0, TO_ROOM);
					act("$p atterra con un rumoroso $c5009THUMP$c0007!",
						FALSE, rp->people, obj, 0, TO_CHAR);
				}
				return;

			}
		}

		if(rp->people) {
			/* have to reference a person */
			act("$p precipita fino a sparire.", FALSE, rp->people, obj, 0, TO_ROOM);
			act("$p precipita fino a sparire.", FALSE, rp->people, obj, 0, TO_CHAR);
		}
		obj_from_room(obj);
		obj_to_room(obj, rp->dir_option[DOWN]->to_room);
		if(targ->people) {
			act("$p cade dal $c0014cielo$c0007.", FALSE, targ->people, obj, 0, TO_ROOM);
			act("$p cade dal $c0014cielo$c0007.", FALSE, targ->people, obj, 0, TO_CHAR);
		}
		count++;

		if(targ->sector_type != SECT_AIR) {
			if(count == 1) {
				if(targ->people) {
					act("$p atterra con un rumoroso $c5009THUMP$c0007!", FALSE, targ->people, obj, 0, TO_ROOM);
					act("$p atterra con un rumoroso $c5009THUMP$c0007!", FALSE, targ->people, obj, 0, TO_CHAR);
				}
				return;
			}
			else {
				if(targ->people) {
					if(targ->sector_type >= SECT_WATER_SWIM) {
						act("$p si schianta sulla superfice dell'$c0012acqua$c0007 ad alta velocita'.",
							FALSE, targ->people, obj, 0, TO_ROOM);
						act("$p si schianta sulla superfice dell'$c0012acqua$c0007 ad alta velocita'.",
							FALSE, targ->people, obj, 0, TO_CHAR);
					}
					else {
						act("$p si schianta al suolo a gran velocita'.",
							FALSE, targ->people, obj, 0, TO_ROOM);
						act("$p si schianta al suolo a gran velocita'.",
							FALSE, targ->people, obj, 0, TO_CHAR);
					}
				}
				return;

			}
		}
		else {
			/*
			 * time to try the next room
			 */
			rp = targ;
			targ = 0;
		}
	}

    if(IS_SET(rp->room_flags, DEATH)) {
        obj_from_room(obj);
        obj_to_room(obj, 1);
        return;
    }

	if(count >= 100) {
		mudlog(LOG_ERROR, "Someone screwed up an air room.");
		obj_from_room(obj);
		obj_to_room(obj, 4);
		return;
	}
}

std::string spell_display_name(int spell_num) {
	if(spell_num == SPELL_RICORDO_HP) {
		return "echoes of the past";
	}
	if(spell_num == SPELL_DESTINO_HP) {
		return "threads of fate";
	}
	int count = 0;
	while(spells[count] != nullptr && *spells[count] != '\n') {
		++count;
	}
	if(spell_num < 1 || spell_num > count) {
		return "UNDEFINED(" + std::to_string(spell_num) + ")";
	}
	return spells[spell_num - 1];
}

} // namespace Alarmud
