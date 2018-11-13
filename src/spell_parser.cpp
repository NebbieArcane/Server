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
#include "breath.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "opinion.hpp"
#include "regen.hpp"

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
	"168 non implementata", // SALVO descr
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
    "",
    "quest",
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
	char aligndesc[17];
	char ostilitydesc[17];
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
		strcpy(aligndesc,"Demoniaco       ");
	}
	if(alignvalue==0) 		{
		strcpy(aligndesc,"Neutrale        ");
	}
	if(alignvalue>0) 		{
		strcpy(aligndesc,"Buono           ");
	}
	if(alignvalue>0) 		{
		strcpy(aligndesc,"Angelico        ");
	}
	if(ostilityvalue<0) 	{
		strcpy(ostilitydesc,"Non aggressiva  ");
	}
	if(ostilityvalue<10) 	{
		strcpy(ostilitydesc,"Carezzevole     ");
	}
	if(ostilityvalue==0) 	{
		strcpy(ostilitydesc,"Non applicabile ");
	}
	if(ostilityvalue>0) 	{
		strcpy(ostilitydesc,"Aggressiva      ");
	}
	if(ostilityvalue>10) 	{
		strcpy(ostilitydesc,"Molto aggressiva");
	}
	sprintf(buf,
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
		act("You manage to slow your fall, enough to stay alive..",
			TRUE, ch, 0, 0, TO_CHAR);
		saved = TRUE;
	}
	else {
		act("The world spins, and you sky-dive out of control",
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
				send_to_char("You are smashed into tiny pieces.\n\r", ch);
				act("$n smashes against the ground at high speed",
					FALSE, ch, 0, 0, TO_ROOM);
				act("You are drenched with blood and gore",
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
				send_to_char("You land with a resounding THUMP!\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

				/* should damage all their stuff */
				DamageAllStuff(ch, BLOW_DAMAGE);

				return(TRUE);

			}
		}

		act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char("You plunge from the sky\n\r", ch);
		char_from_room(ch);
		char_to_room(ch, rp->dir_option[DOWN]->to_room);
		act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
		count++;

		do_look(ch, "", 15);

		if(IS_SET(targ->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
			NailThisSucker(ch);
			return(TRUE);
		}

		if(targ->sector_type != SECT_AIR) {
			/* do damage, or kill */
			if(count == 1) {
				send_to_char("You land with a resounding THUMP!\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
				/*
				 * should damage all their stuff
				 */
				DamageAllStuff(ch, BLOW_DAMAGE);

				return(TRUE);

			}
			else if(!saved) {
				send_to_char("You are smashed into tiny pieces.\n\r", ch);
				if(targ->sector_type >= SECT_WATER_SWIM)
					act("$n is smashed to a pulp by $s impact with the water",
						FALSE, ch, 0, 0, TO_ROOM);
				else
					act("$n is smashed to a bloody pulp by $s impact with the ground",
						FALSE, ch, 0, 0, TO_ROOM);
				act("You are drenched with blood and gore", FALSE,ch, 0, 0, TO_ROOM);

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
				send_to_char("You land with a resounding THUMP!\n\r", ch);
				GET_HIT(ch) = 0;
				alter_hit(ch,0);
				GET_POS(ch) = POSITION_STUNNED;
				act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

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
		send_to_char("PANIC!  You're drowning!!!!!!", ch);
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
    char buf[128];
    
	if(s > MAX_SKILLS+10) {
		return;
	}
    
    if(s == STATUS_QUEST && !IS_PC(ch)) {
        
        /* fine dei giochi, si torna a casa */
        switch(GET_POS(ch)) {
                
            case POSITION_FIGHTING  :
            WAIT_STATE(ch->specials.fighting, PULSE_VIOLENCE*3);
            sprintf(buf,"\n\r$c0014%s coglie l'occasione buona e se la da' a gambe per sempre!$c0007\n\r",ch->player.name);
            act(buf, FALSE, ch, 0, ch, TO_ROOM);
            stop_fighting(ch);
            extract_char(ch);
                break;
            
            case POSITION_DEAD  :
                break;
                
            default:
                sprintf(buf,"\n\r$c0014%s si confonde tra la folla e scompare per sempre...$c0007\n\r",ch->player.name);
                act(buf, FALSE, ch, 0, ch, TO_ROOM);
                extract_char(ch);
                break;
        }
        
        return;
    }

	if(spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
		act(spell_wear_off_msg[s], FALSE, ch, NULL, NULL, TO_CHAR);
	}

	if(spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s] &&
			(IS_NPC(ch) || !IS_SET(ch->specials.act, PLR_STEALTH))) {
		act(spell_wear_off_room_msg[s], TRUE, ch, 0, 0, TO_ROOM);
	}
    
    if(s == STATUS_QUEST) {
        ch->specials.quest_ref = NULL;
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
	register struct char_data* test;
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
				act("$N e` stat$B congelat$B da Alar",FALSE,test,0,
					test,TO_NOTVICT);
				SET_BIT(test->specials.act,PLR_FREEZE);
				sprintf(buf,"Che ci fate tu e %s qui contemporaneamente?\n\r",
						GET_NAME(test));
				send_to_char(buf,i);
				send_to_char("Lo spirito vitale ti abbandona."
							 "Alar ti ha congelato!\n\r",i);
				act("$N e` stat$B congelat$B da Alar",FALSE,i,0,
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
			send_to_char("Devi indossare vesti piu adatte alla meditazione",ch);
			af->duration=0;
		}

	}
}

void affect_update(unsigned long localPulse) {
	static struct affected_type* af, *next_af_dude;
	register struct char_data* i;
	register struct obj_data* j;
	struct obj_data* next_thing;
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
	for(i = character_list; i; i = next_char) {
		next_char = i->next;
		if(IS_SET(i->specials.act,PLR_FREEZE)) {
			continue;
		}
		if(CheckMulti(i)) {
			continue;
		}
		/* Imposta un po` di puntatori e flag */
		rp = real_roomp(i->in_room);
		regainroom=(IS_SET(rp->room_flags,NO_REGAIN))?0:1;

		/* Calcola la posizione prevalente */
		for(k=0; k<=MAX_POSITION; k++) {
			ggtmp=GET_TEMPO_IN(i,k);
			if(ggtmp>posprev) {
				GET_POS_PREV(i)=k;
				posprev=ggtmp;
			}
		}
		GET_TEMPO_IN(i,GET_POS_PREV(i))=MAX(0,ggtmp-(PULSE_PER_SEC *
											SECS_PER_MUD_HOUR));
		if(i->nMagicNumber != CHAR_VALID_MAGIC) {
			mudlog(LOG_SYSERR, "Invalid char magic number %d in affect_update",
				   i->nMagicNumber);
			abort();
		}

		/*
		 *  check the effects on the char
		 */
		dead=FALSE;

		for(af = i->affected; af && !dead; af = next_af_dude)
			/* Affect loop */
		{
			next_af_dude = af->next;
			CheckSpecialties(i,af);

			if(af->duration >= 1) {
				af->duration--;

				if(af->duration == 1 &&
						(!af->next ||
						 af->next->type != af->type ||
						 af->next->duration != 2))
					/* && af->location != APPLY_INTRINSIC) */
				{
					SpellWearOffSoon(af->type, i);
				}
			}
			else {
				/* It must be a spell */
				if(af->type > 0 && af->type < FIRST_BREATH_WEAPON) {
					int iType = af->type;
					if(!af->next || af->next->type != af->type ||
							af->next->duration > 0) {
						/* if(af->location != APPLY_INTRINSIC) */
						SpellWearOff(af->type, i);
					}

					/* Se il tipo di affect e' SPELL_CHARM_PERSON, l'affect e` gia` stato
					* tolto da SpellWearOff */
					if(iType != SPELL_CHARM_PERSON) {
						check_memorize(i, af);
						affect_remove(i, af);
					}
				}
				else if(af->type >= FIRST_BREATH_WEAPON &&
						af->type <= LAST_BREATH_WEAPON) {
					bweapons[ af->type-FIRST_BREATH_WEAPON ](-af->modifier/2, i, "",
							SPELL_TYPE_SPELL, i, 0);
					if(!i->affected) {
						/* oops, you're dead :) */
						dead = TRUE;
						break;
					}
					affect_remove(i, af);
				}
			}
		} /*Fine affect loop*/

		if(!dead)
			/* Regen check*/
		{
			/* Ho tolto && (i->desc || !IS_PC(i)) cosi` i PC riguadagnano mana
			 * anche se sono link dead, ma non se sono nelle "storage room", ossia
			 * nelle stanze dove finiscono i polimorfati. Inoltre esiste il rischio
			 * di morire per il veleno anche se si finisce LD.
			 */
			if(GET_POS(i) >= POSITION_STUNNED && i->in_room != 3 &&
					i->in_room != 2) {
				/* note - because of poison, this one has to be in the
				 * opposite order of the others.  The logic:
				 *
				 * hit_gain() modifies the characters hps if they are poisoned.
				 * but if they were in the opposite order,
				 * the total would be: hps before poison + gain.  But of course,
				 * the hps after poison are lower, but No one cares!
				 * and that is why the gain is added to the hits, not vice versa
				 */
#ifdef NOEVENTS
				int hgain = hit_gain(i);
				if(ch==NULL || GET_NAME(ch)==NULL) { // SALVO controllo che dopo hit_gain sia ancora vivo
					return;
				}
				GET_HIT(i)  = MIN((regainroom * hgain) + GET_HIT(i),  hit_limit(i));
				GET_MANA(i) = MIN(GET_MANA(i) + (regainroom * mana_gain(i)), mana_limit(i));
				GET_MOVE(i) = MIN(GET_MOVE(i) + (regainroom * move_gain(i)), move_limit(i));
#endif
				if(GET_POS(i) == POSITION_STUNNED) {
					update_pos(i);
				}
			}
			else if(GET_POS(i) == POSITION_INCAP) {
				/* do nothing */
				damage(i, i, 0, TYPE_SUFFERING, 5);
			}
			else if(GET_POS(i) == POSITION_MORTALLYW) {
				damage(i, i, 1, TYPE_SUFFERING, 5);
			}

			if(IS_PC(i)) {
				update_char_objects(i);
				if(GetMaxLevel(i) < DIO && i->in_room != 3 && i->in_room != 2) {
					check_idling(i);
				}
				/*
				 * Se e` linkdead
				 * non diventa assetato od affamato.
				 * */
				if(rp && i->desc) {
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
					if(GET_RACE(i) == RACE_HALFLING) {
						full_gain+=1;
						drunk_gain+=1;
						thirst_gain+=1;
					}
					else if(GET_RACE(i) == RACE_HALF_OGRE || RACE_HALF_GIANT) {
						full_gain+=2;
						drunk_gain+=0;
						thirst_gain+=1;
					}
					/*
					* Controllo per classe
					* */
					if(HasClass(i,CLASS_MONK)) {
						if(localPulse % (PULSE_TICK*
										 MAX((GET_LEVEL(i,MONK_LEVEL_IND)-29),1)
										)
						  ) {
							full_gain=0;
						}
						thirst_gain=0;
					}
					gain_condition(i,FULL,-MAX(0,full_gain) * regainroom);
					gain_condition(i,DRUNK,-MAX(0,drunk_gain) * regainroom);
					gain_condition(i,THIRST,-MAX(0,thirst_gain) * regainroom);

				}
				if(i->specials.tick == time_info.hours) {
					/* works for 24, change for anything else */
					/* the special case for room 3 is a hack to keep link dead
					* people who have no stuff from being saved without stuff..  */
					if(!IS_IMMORTAL(i)
							&& i->in_room != 3
							&& i->in_room != NOWHERE) {
						do_save(i,"",0);
					}
				}
			}
			check_nature(i);  /* check falling, check drowning, etc */
		} /*Regen check loop */

	}

	/* update the objects */

	for(j = object_list; j ; j = next_thing) {
		next_thing = j->next; /* Next in object list */

		/* If this is a corpse */
		if(GET_ITEM_TYPE(j) == ITEM_CONTAINER && j->obj_flags.value[3]) {
			/* timer count down */
			if(j->obj_flags.timer > 0) {
				j->obj_flags.timer--;
			}
			else if(!j->carried_by) {  /* E` in terra.. */
				j->obj_flags.timer++;
			}
			if(!j->obj_flags.timer) {
				if(j->carried_by) {
					act("$p si dissolve nelle tue mani e tutto quello che"
						" aveva addosso cade a terra.", FALSE, j->carried_by, j, 0, TO_CHAR);
					act("$p si dissolve nelle mani di $n e tutto quello che"
						" aveva addosso cade a terra.", FALSE, j->carried_by, j, 0, TO_ROOM);
				}
				else if(j->in_room != NOWHERE && real_roomp(j->in_room)->people) {
					act("$p si dissolve.",
						TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
					act("$p si dissolve.",
						TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
				}
				ObjFromCorpse(j);
			}
		}
	}
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
		act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
		act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,
			TO_NOTVICT);
		act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
		if(affected_by_spell(ch, SPELL_CHARM_PERSON)) {
			affect_from_char(ch, SPELL_CHARM_PERSON);
		}
	}
	else {
		act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
		if(!IS_SET(ch->specials.act,PLR_STEALTH)) {
			act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
			act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
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

#if 0
	assert(!ch->master);
#else
	/* instead of crashing the mud we try this */
	if(ch->master) {
		act("$n cannot follow you for some reason.",TRUE,ch,0,leader,TO_CHAR);
		act("You cannot follow $N for some reason.",TRUE,ch,0,leader,TO_CHAR);
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


	act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
	if(!IS_SET(ch->specials.act, PLR_STEALTH)) {
		act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
		act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
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




	sprintf(buf2,"$n utters the words, '%s'", buf);
	sprintf(buf, "$n utters the words, '%s'", spells[si-1]);

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

bool EqNotForCaster(struct char_data* ch, int spl) {
	if((spell_info[ spl ].min_level_magic >
			GET_LEVEL(ch, MAGE_LEVEL_IND) ||
			EqWBits(ch, ITEM_ANTI_MAGE)) &&

			(spell_info[ spl ].min_level_sorcerer >
			 GET_LEVEL(ch, SORCERER_LEVEL_IND) ||
			 (EqWBits(ch, ITEM_ANTI_MAGE)||EqWBits(ch, ITEM_ANTI_SORCERER))) &&

			(spell_info[ spl ].min_level_cleric >
			 GET_LEVEL(ch, CLERIC_LEVEL_IND) ||
			 EqWBits(ch, ITEM_ANTI_CLERIC)) &&

			(spell_info[ spl ].min_level_paladin >
			 GET_LEVEL(ch, PALADIN_LEVEL_IND) ||
			 EqWBits(ch, ITEM_ANTI_PALADIN)) &&

			(spell_info[ spl ].min_level_ranger >
			 GET_LEVEL(ch, RANGER_LEVEL_IND) ||
			 EqWBits(ch, ITEM_ANTI_RANGER)) &&

			(spell_info[ spl ].min_level_psi >
			 GET_LEVEL(ch, PSI_LEVEL_IND) ||
			 EqWBits(ch, ITEM_ANTI_PSI)) &&

			(spell_info[ spl ].min_level_druid >
			 GET_LEVEL(ch, DRUID_LEVEL_IND) ||
			 EqWBits(ch, ITEM_ANTI_DRUID)))

	{
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}
/* Assumes that *argument does start with first letter of chopped string */
ACTION_FUNC(do_cast) {

	struct obj_data* tar_obj;
	struct char_data* tar_char;
	struct char_data* tmp_char;
	char name[MAX_INPUT_LENGTH];
	int qend=0;
	int spl=0;
	int i=0;
	int align_cost=0;
	int caster_level=-1;
	int spell_level=100;
	bool target_ok;
	int tlev=0;
	int slev=0;
	int max, cost;
	int nDado;
	int sf_pejus = 0;
	int spell_align=0;
	int caster_align=0;
	char szbuf[100];
	if(!IsHumanoid(ch)) {
		send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
		return;
	}

	if(!IS_IMMORTAL(ch)) {
		/**** SALVO skills prince VVVVVVVVVVVVVVV ****/
		if(!IS_PRINCE(ch)) {
			/**** fine skills prince ****/
			if(BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
				send_to_char("Pensa a combattere, idiota!\n\r", ch);
				return;
			}
			else if(BestMagicClass(ch) == THIEF_LEVEL_IND) {
				send_to_char("Credi di poter rubare anche la magia?\n\r", ch);
				return;
			}
			else if(BestMagicClass(ch) == MONK_LEVEL_IND) {
				send_to_char("Ehi.... ma questa e` stregoneria!\n\r", ch);
				return;
			}
		}
	}


	if(cmd != CMD_MIND && apply_soundproof(ch)) {
		return;
	}

	char work[256];     /* works on a copy of arg */
	strncpy(work,arg,255);
	work[255]='\0';
	// Argument is just a constant pointer to work
	const char* argument=work;
	argument = skip_spaces(argument);

	/* If there is no chars in argument */
	if(!(*argument)) {
		send_to_char("Cosa, chi, come?\n\r", ch);
		return;
	}

	if(*argument != '\'') {
		send_to_char("Usa sempre i simboli sacri della magia: '\n\r",ch);
		return;
	}

	/* Locate the last quote  */

	for(qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++);

	if(*(argument + qend) != '\'') {
		send_to_char("Usa sempre i simboli sacri della magia: '\n\r",ch);
		return;
	}

	spl = old_search_block(argument, 1, qend - 1, spells, 0);

	if(!spl) {
		send_to_char("Fantastico! Non e` successo nulla!\n\r",ch);
		return;
	}

	/* mobs do not get  skills so we just check it for PC`s */

	if(!ch->skills)
		if(IS_PC(ch)) {
			send_to_char("Non hai alcuna skill!\n\r",ch);
			return;
		}
	if(cmd != CMD_SPELLID) {


		if(cmd != CMD_MIND && OnlyClass(ch, CLASS_PSI)) {
			send_to_char("Usa la mente! (mind)\n\r",ch);
			return;
		}

		if((cmd != CMD_RECALL) && OnlyClass(ch,CLASS_SORCERER)) {
			send_to_char("Usa la memoria! (recall).\n\r",ch);
			return;
		}
		if(cmd == CMD_MIND && !HasClass(ch,CLASS_PSI)) {
			send_to_char("Non sei uno psi!\n\r",ch);
			return;
		}
		if(cmd == CMD_RECALL && !HasClass(ch,CLASS_SORCERER)) {
			send_to_char("Non sei uno stregone!\n\r",ch);
			return;
		}
	}


	/* this should make sorcerer learned spells be forced to be recalled */
	if((cmd == CMD_CAST || cmd == CMD_MIND) && HasClass(ch,CLASS_SORCERER) &&
			!IS_IMMORTAL(ch) && IS_SET(ch->skills[spl].flags,SKILL_KNOWN_SORCERER)) {
		send_to_char("Questo e` un incantesimo da stregone...\n\r",ch);
		return;
	}

	if(spl > 0 && spl < MAX_SKILLS && spell_info[spl].spell_pointer) {
		if(GET_POS(ch) < spell_info[spl].minimum_position) {
			switch(GET_POS(ch)) {
			case POSITION_SLEEPING :
				send_to_char("Sogni di armi, amori e grandi poteri.\n\r", ch);
				break;
			case POSITION_RESTING :
				send_to_char("E alzati prima, no?.\n\r",ch);
				break;
			case POSITION_SITTING :
				send_to_char("Stai ancora annaspando per rialzarti!\n\r", ch);
				break;
			case POSITION_FIGHTING :
				send_to_char("Un pugno nei denti ti impedisce di formulare l'incatesimo.\n\r", ch);
				break;
			default:
				send_to_char("Non mi sembri in gran forma!\n\r",ch);
				break;
			} /* Switch */
		}
		else {
			/* Controlla il numero di classi per le spell solo monoclasse */
			if(IS_SET(spell_info[spl].targets, TAR_MONOCLASSE)   && (GetMaxLevel(ch)<DIO)) {
				/**** modifica SALVO if (HowManyClasses(ch)>1) { */
				if(HowManyClasses(ch)>1 && (GetMaxLevel(ch)<DIO)) {
					/* fine modifica ****/
					send_to_char("Questo incantesimo e' solo per i veri adepti!\n\r",ch);

					return;
				}
			}
			/* Controllo sui livelli */
			/* Trova il livello spell corrispondente */
			if((slev=spell_info[spl].min_level_magic) <=
					(tlev=GET_LEVEL(ch,MAGE_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_sorcerer) <=
					(tlev=GET_LEVEL(ch,SORCERER_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_cleric) <=
					(tlev=GET_LEVEL(ch,CLERIC_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_paladin) <=
					(tlev=GET_LEVEL(ch,PALADIN_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_ranger) <=
					(tlev=GET_LEVEL(ch,RANGER_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_psi) <=
					(tlev=GET_LEVEL(ch,PSI_LEVEL_IND)) &&
					(tlev>caster_level)) {
				caster_level=tlev;
				spell_level=slev;
			}
			if((slev=spell_info[spl].min_level_druid) <=
					(tlev=GET_LEVEL(ch, DRUID_LEVEL_IND))) {
				caster_level=tlev;
				spell_level=slev;
			}
			if(caster_level>10) {
				caster_level-=(HowManyClasses(ch)-1);
			}
			if(!IS_IMMORTAL(ch)) {
				if(caster_level <spell_level) {
					/**** SALVO skills prince ****/
					if(IS_PRINCE(ch) && (spl ==SPELL_REINCARNATE || spl ==SPELL_CURE_CRITIC || spl ==SPELL_CHAIN_LIGHTNING)) {
						caster_level=PRINCIPE;
						spell_level=PRINCIPE;
					}
					else {
						/**** fine skills prince ****/
						send_to_char("Devi ancora crescere!\n\r", ch);
						return;
					}
				}
			}
			else {
				sprintf(szbuf,"spell: %d caster: %d",spell_level,caster_level);
				send_to_char(szbuf,ch);
			}
			if(cmd==CMD_SPELLID) {
				if(!IS_CASTER(ch)) {
					send_to_char("Lascia perdere...\n\r", ch);
					return;
				}
				spellid(spl
						,ch,
						caster_level,
						spell_level);
				return;
			}

			argument+=qend+1;        /* Point to the last apice */
			for(; *argument == ' '; argument++);

			/* **************** Locate targets **************** */

			target_ok = FALSE;
			tar_char = (struct char_data*)0;
			tar_obj = (struct obj_data*)0;
			tmp_char = (struct char_data*)0;

			if(!IS_IMMORTAL(ch)) {
				if(IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
						check_peaceful(ch, "Questa zona sembra refrattaria alla magia!\n\r")) {
					return;
				}
			}

			/* for seeing what the other guys are doing test */

			if(IS_PC(ch) && GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
				mudlog(LOG_CHECK, "%s cast %s", GET_NAME(ch), arg);
			}

			if(!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
				mudlog(LOG_CHECK,"NOT TAR_IGNORE");
				argument = one_argument(argument, name);

				if(*name) {
					if(IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
						if((tar_char = get_char_room_vis(ch, name))) {
							if(tar_char == ch ||
									tar_char == ch->specials.fighting ||
									tar_char->attackers < 6 ||
									tar_char->specials.fighting == ch) {
								target_ok = TRUE;

							}

							else {
								send_to_char("Volano troppi schiaffi qui intorno!\n\r", ch);

								target_ok = FALSE;
							}
						}
						else {

							target_ok = FALSE;
						}
					}
					/* TAR_CHAR_WORLD FINE*/
					if(!target_ok &&
							IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD)) {

						if((tar_char = get_char_vis(ch, name))) {
							target_ok = TRUE;
						}
					}
					/* TAR_CHAR_OBJ_INV*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
						if((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
							target_ok = TRUE;
						}

					/* TAR_CHAR_OBJ_ROOM*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
						if((tar_obj = get_obj_in_list_vis(ch, name,
														  real_roomp(ch->in_room)->contents))) {
							target_ok = TRUE;
						}

					/* TAR_CHAR_OBJ_WORLD*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
						if((tar_obj = get_obj_vis(ch, name))) {
							target_ok = TRUE;
						}

					/* TAR_CHAR_OBJ_EQUIP*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
						for(i=0; i<MAX_WEAR && !target_ok; i++)
							if(ch->equipment[i] &&
									str_cmp(name, ch->equipment[i]->name) == 0) {
								tar_obj = ch->equipment[i];
								target_ok = TRUE;
							}
					}
					/* TAR_CHAR_SELF_ONLY*/

					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
						if(str_cmp(GET_NAME(ch), name) == 0) {
							tar_char = ch;
							target_ok = TRUE;
						}

					/* TAR_NAME*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME)) {
						tar_obj = (struct obj_data*)name;
						target_ok = TRUE;
					}
					if(tar_char) {
						if(IS_NPC(tar_char)) {
							if(IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
								send_to_char("OH NO! NON PUOI!!!!",ch);
								return;
							}
						}
					}
				} /* Fine test su *name */
				else {
					/* No argument was typed */

					/* TAR_FIGHT_SELF*/
					if(IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF)) {
						if(ch->specials.fighting &&
								ch->specials.fighting->specials.fighting) {
							tar_char = ch->specials.fighting->specials.fighting;
							target_ok = TRUE;
						}
						else if(ch->specials.supporting &&
								(tmp_char=get_char_room_vis(ch,ch->specials.supporting))
								&& tmp_char->specials.fighting
								&& tmp_char->specials.fighting->specials.fighting) {
							tar_char=tmp_char->specials.fighting->specials.fighting;
							target_ok = TRUE;
						}
					}
					/*TAR_FIGHT_VICT*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT)) {

						if(ch->specials.fighting) {
							/* WARNING, MAKE INTO POINTER */
							tar_char = ch->specials.fighting;
							target_ok = TRUE;
						}
						else if(ch->specials.supporting &&
								(tmp_char=get_char_room_vis(ch,ch->specials.supporting))
								&& tmp_char->specials.fighting) {
							tar_char=tmp_char->specials.fighting;
							target_ok = TRUE;
						}
					}
					/*TAR_SELF_ONLY*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
						tar_char = ch;
						target_ok = TRUE;
					}
					/*TAR_ROOM*/
					if(!target_ok && IS_SET(spell_info[spl].targets, TAR_ROOM)) {
						tar_char = NULL;
						target_ok = TRUE;
					}
				} /* Fine else not name */

			} /* Fine NOT TAR_IGNORE */
			else {
				target_ok = TRUE; /* No target, is a good target */
			}

			if(!target_ok) {
				if(*name) {
					if(IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD)) {
						send_to_char("Nobody playing by that name.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
						send_to_char("Nobody here by that name.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_OBJ_INV)) {
						send_to_char("You are not carrying anything like that.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM)) {
						send_to_char("Nothing here by that name.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD)) {
						send_to_char("Nothing at all by that name.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
						send_to_char("You are not wearing anything like that.\n\r", ch);
					}
					else if(IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD)) {
						send_to_char("Nothing at all by that name.\n\r", ch);
					}

				}
				else {
					/* Nothing was given as argument */
					if(spell_info[spl].targets < TAR_OBJ_INV) {
						send_to_char("Who should the spell be cast upon?\n\r", ch);
					}
					else {
						send_to_char("What should the spell be cast upon?\n\r", ch);
					}
				}
				return;
			}
			else {
				/* TARGET IS OK */
				if((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
					send_to_char("You can not cast this spell upon yourself.\n\r", ch);
					return;
				}
				else if(tar_char != ch &&
						IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
					send_to_char("You can only cast this spell upon yourself.\n\r", ch);
					if((GetMaxLevel(ch)<DIO+1)) { // SALVO chi e' > di DIO puo' castare a tutti
						return;
					}
				}
				else if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
					send_to_char("You are afraid that it could harm your master.\n\r", ch);
					return;
				}
			}

			if(cmd == CMD_RECALL) {
				/* recall */
				if(!MEMORIZED(ch, spl)) {
					send_to_char("You don't have that spell memorized!\n\r", ch);
					return;
				}
			}
			else {
				if(GetMaxLevel(ch) < IMMORTALE) {
					if(GET_MANA(ch) < (int)USE_MANA(ch, (int)spl) ||
							GET_MANA(ch) <=0) {
						send_to_char("You can't summon enough energy!\n\r", ch);
						return;
					}
				}
			}

			if(spl != SPELL_VENTRILOQUATE && cmd != CMD_MIND) {  /* :-) */
				say_spell(ch, spl);    /* psi`s do not utter! */
			}

			WAIT_STATE(ch, spell_info[spl].beats);

			if((spell_info[spl].spell_pointer == 0) && spl>0)
				send_to_char("Sorry, this magic has not yet been implemented :(\n\r",
							 ch);
			else {
				max = ch->specials.spellfail;
				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Spellfail PRIMA del check sull'eq: %d\n\r",max);
					send_to_char(szbuf,ch);
				}
				mudlog(LOG_CHECK,"Checking eq after spell");


				/* check EQ and alter spellfail accordingly */
				if(EqNotForCaster(ch,spl)) {
					/* GGPATCH */
					if(IS_MAESTRO_DEGLI_DEI(ch)) {
						sprintf(szbuf,"Spellfail SE check sull'eq: %d\n\r",max);
						send_to_char(szbuf,ch);
					}
					sf_pejus =1;
					max += 20; /* 40% harder to cast spells */
					if(OnlyClass(ch, CLASS_SORCERER))  {
						max += 10;
					}
				}
				/* GGBEG */
				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Spellfail DOPO il check sull'eq: %d\n\r",max);
					send_to_char(szbuf,ch);
				}
				max-=SPELLFAIL_MOD(caster_level,spell_level);
				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Spellfail DOPO aggiustamento : %d\n\r",max);
					send_to_char(szbuf,ch);
				}
				/* GGEND */
				max += GET_COND(ch, DRUNK) * 10; /* 0 - 240 */
				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Spellfail prima dell'align : %d\n\r",max);
					send_to_char(szbuf,ch);
				}
				caster_align=GET_ALIGNMENT(ch)+1000;
				spell_align=spell_info[spl].align_modifier;
				if(spell_align <0) {
					spell_align=-1;
				}
				if(spell_align >0) {
					spell_align=1;
				}
				spell_align*=1000;
				spell_align+=1000;
				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Align: Spell %d Caster %d\n\r",spell_align,caster_align);
					send_to_char(szbuf,ch);
					sprintf(szbuf,"Spellfail prima dell'align : %d\n\r",max);
					send_to_char(szbuf,ch);
				}

				max+=((abs(spell_align-caster_align)-1000)/50);

				if(IS_MAESTRO_DEGLI_DEI(ch)) {
					sprintf(szbuf,"Spellfail dopo l'align : %d\n\r",max);
					send_to_char(szbuf,ch);
				}

				if(ch->attackers > 0) {
					max += spell_info[spl].spellfail;
				}
				else if(ch->specials.fighting) {
					max += spell_info[spl].spellfail/2;
				}

				if(IS_IMMORTAL(ch)) {
					max = MAX(1, ch->skills[ spl ].learned);
				}

				nDado = number(1, max);
				if(cmd==CMD_RECALL) {
					nDado-=(nDado/10);
				}
				if(nDado > ch->skills[ spl ].learned &&
						!IsSpecialized(ch->skills[ spl ].special)) {
					send_to_char("Perdi la tua concentrazione!\n\r", ch);
					if(sf_pejus) {
						act("Certo.. con tutta quella robaccia addosso.....",
							FALSE, ch, NULL, NULL, TO_CHAR);
					}
					cost = (int)USE_MANA(ch, (int)spl);
					if(cmd == CMD_RECALL) {
						/* give chance to forget */
						if(number(1,130) > ch->skills[spl].learned) {
							FORGET(ch, spl);
						}
					}
					else {
						GET_MANA(ch) -= (cost >> 1);
						alter_mana(ch,0);
					}

					LearnFromMistake(ch, spl, 0, 95);
					return;
				}

				if(tar_char) {
					/* psi shit ain`t magic */
					if(affected_by_spell(tar_char,SPELL_ANTI_MAGIC_SHELL) &&
							cmd != CMD_MIND && !IS_IMMORTAL(ch)) {
						act("Your magic fizzles against $N's anti-magic shell!",
							FALSE,ch,0,tar_char,TO_CHAR);
						act("$n wastes a spell on $N's anti-magic shell!",
							FALSE,ch,0,tar_char,TO_ROOM);
						act("$n casts a spell and growls as it fizzles against "
							"anti-magic shell!",FALSE,ch,0,tar_char,TO_VICT);
						return;
					}

					if(GET_POS(tar_char) == POSITION_DEAD) {
						send_to_char("The magic fizzles against the dead body.\n", ch);
						return;
					}
				}

				/* psi shit ain`t magic */
				if(affected_by_spell(ch,SPELL_ANTI_MAGIC_SHELL)
						&& cmd != CMD_MIND
						&& !IS_IMMORTAL(ch)) {
					act("Your magic fizzles against your anti-magic shell!",
						FALSE,ch,0,0,TO_CHAR);
					act("$n tries to cast a spell within a anti-magic shell, muhahaha!",
						FALSE,ch,0,0,TO_ROOM);
					return;
				}
				if(!IS_IMMORTAL(ch)) {
					if(cmd == CMD_CAST &&
							check_nomagic(ch, "Il mana si rifuta di scorrere in questa zona.",
										  "La magia di $n muore sulle sue labbra")) {
						return;
					}
					if(cmd == CMD_RECALL &&
							check_nomagic(ch, "Le parole ti si confondono davanti agli occhi in questa zona.",
										  "La magia di $n muore sulle sue labbra")) {
						return;
					}

					if(cmd == CMD_MIND &&
							check_nomind(ch, "Non riesci a concentrarti abbastanza in questo "
										 "posto",
										 "$n cerca invano di concentrarsi")) {
						return;
					}
				}
				send_to_char("Ok.\n\r",ch);
				/* Calcolo tutti i costi PRIMA della spell
				 * Fra l'altro mi sa che aver messo il forget DOPO il
				 * cast sia il motivo per cui il recall 'poly' non
				 * dimenticava
				 * */
				cost = (int)USE_MANA(ch, (int)spl);
				if(cmd == CMD_RECALL) {  /* recall */
					FORGET(ch, spl);
				}
				else {
					GET_MANA(ch) -= cost;
					alter_mana(ch,0);
				}
				/* Ogni spell ha una costante INTRINSECA di malvagita
				 * che non dipende dal bersaglio
				 * (Drenare est da cattivi, sempre).
				 * Inoltre, in base al target viene conteggiato un coefficiente
				 * di ostilita' e quindi di variazione align
				 * */
				/* Intrinseca */
				align_cost=spell_info[spl].align_modifier;
#ifdef NEW_ALIGN
				if(tar_char && (tar_char != ch)) {
					GET_ALIGNMENT(ch)+=align_cost;
					mudlog(LOG_CHECK,"Char %s spell %s intrinseca: %5d",
						   GET_NAME(ch),
						   arg,
						   align_cost);
					/* Relativa */
				}
#endif
				PushStatus("AlignMod");
				AlignMod(ch,tar_char,spell_info[spl].ostility_level);
				/* FINALMENTE, qui casta davvero!!!! :-) */
				PushStatus("About Casting spell");
				mudlog(LOG_CHECK,"Motore di cast terminato, going to real procedure");
				((*spell_info[spl].spell_pointer)(GET_LEVEL(ch, BestMagicClass(ch)),
												  ch, argument, SPELL_TYPE_SPELL,
												  tar_char, tar_obj));
				PopStatus();
				PopStatus();
				/* Effetto sullo stato, incluso alignment */
			}
		}        /* if GET_POS < min_pos */
		return;
	}
	if(spl > 0) {
		mudlog(LOG_SYSERR,"Spellid %d cmid %d command %s",spl,cmd,arg);
	}

	switch(number(1,5)) {
	case 1:
		send_to_char("Bylle Grylle Grop Gryf???\n\r", ch);
		break;
	case 2:
		send_to_char("Olle Bolle Snop Snyf?\n\r",ch);
		break;
	case 3:
		send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch);
		break;
	case 4:
		send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch);
		break;
	default:
		send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch);
		break;
	}
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
					act("$p smashes against the ground at high speed",
						FALSE, rp->people, obj, 0, TO_ROOM);
					act("$p smashes against the ground at high speed",
						FALSE, rp->people, obj, 0, TO_CHAR);
				}
				return;

			}
			else {
				if(rp->people) {
					act("$p lands with a loud THUMP!",
						FALSE, rp->people, obj, 0, TO_ROOM);
					act("$p lands with a loud THUMP!",
						FALSE, rp->people, obj, 0, TO_CHAR);
				}
				return;

			}
		}

		if(rp->people) {
			/* have to reference a person */
			act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
			act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
		}
		obj_from_room(obj);
		obj_to_room(obj, rp->dir_option[DOWN]->to_room);
		if(targ->people) {
			act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
			act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
		}
		count++;

		if(targ->sector_type != SECT_AIR) {
			if(count == 1) {
				if(targ->people) {
					act("$p lands with a loud THUMP!", FALSE, targ->people, obj, 0, TO_ROOM);
					act("$p lands with a loud THUMP!", FALSE, targ->people, obj, 0, TO_CHAR);
				}
				return;
			}
			else {
				if(targ->people) {
					if(targ->sector_type >= SECT_WATER_SWIM) {
						act("$p smashes against the water at high speed",
							FALSE, targ->people, obj, 0, TO_ROOM);
						act("$p smashes against the water at high speed",
							FALSE, targ->people, obj, 0, TO_CHAR);
					}
					else {
						act("$p smashes against the ground at high speed",
							FALSE, targ->people, obj, 0, TO_ROOM);
						act("$p smashes against the ground at high speed",
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

	if(count >= 100) {
		mudlog(LOG_ERROR, "Someone screwed up an air room.");
		obj_from_room(obj);
		obj_to_room(obj, 4);
		return;
	}
}

} // namespace Alarmud

