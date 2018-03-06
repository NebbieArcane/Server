/* GENERATED FILE DO NOT TOUCH
 * Generated from  by code_generator.php */
/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include <string>
#include "config.hpp"
#ifndef __AUTOENUMS_HPP_
#define __AUTOENUMS_HPP_
//#include "globals.hpp"
//#include "LuaBridge.hpp"
namespace Alarmud {

//Template for enum to unsigned byte


template <typename T>
inline unsigned char enum_to_ubyte(T value) {
    return value>255?0:static_cast<unsigned char>(value);
}

//Template for enum to int


template <typename T>
inline int enum_to_int(T value) {
    return static_cast<int>(value);
}
#define	ALL_DARK                                     1 
#define	FOREST_DARK                                  2 
#define	NO_DARK                                      3 
#define	ABS_MAX_EXP                                  2000000000 
#define	MAX_XP                                       2147483647 
#define	MAX_STAT                                     6 
#define	BIT_POOF_IN                                  1 
#define	BIT_POOF_OUT                                 2 
#define	MAX_WEAR_POS                                 22 
#define	MAX_TOUNGE                                   3 
#define	MAX_NEW_LANGUAGES                            10 
#define	MAX_SKILLS                                   350 
#define	MAX_WEAR                                     23 
#define	MAX_AFFECT                                   40 
#define	MAX_SAVES                                    8 
#define	MAX_CONDITIONS                               5 
#define	MAX_CLASS                                    11 
#define	OLD_MAX_CLASS                                4 
#define	ABS_MAX_CLASS                                20 
#define	MAX_POSITION                                 9 
#define	HIT_INCAP                                    -3 
#define	HIT_MORTALLYW                                -6 
#define	HIT_DEAD                                     -11 
#define	SKILL_SPECIALIZED                            1 
#define	SKILL_UNDEFINED                              2 
#define	SKILL_UNDEFINED2                             4 
#define	ABS_MAX_LVL                                  70 

// Enum type: E_AFFECTED_BY -- start
enum e_affected_by {
/*
Bitvector for 'affected_by'*/
	AFF_NONE                                     =0,
	AFF_BLIND                                    =1,
	AFF_INVISIBLE                                =2,
	AFF_DETECT_EVIL                              =4,
	AFF_DETECT_INVISIBLE                         =8,
	AFF_DETECT_MAGIC                             =16,
	AFF_SENSE_LIFE                               =32,
	AFF_LIFE_PROT                                =64,
	AFF_SANCTUARY                                =128,
	AFF_DRAGON_RIDE                              =256,
	AFF_GROWTH                                   =512, /*this was the one that was missing*/
	AFF_CURSE                                    =1024,
	AFF_FLYING                                   =2048,
	AFF_POISON                                   =4096,
	AFF_TREE_TRAVEL                              =8192,
	AFF_PARALYSIS                                =16384,
	AFF_INFRAVISION                              =32768,
	AFF_WATERBREATH                              =65536,
	AFF_SLEEP                                    =131072,
	AFF_TRAVELLING                               =262144, /*i.e. can't be stoned*/
	AFF_SNEAK                                    =524288,
	AFF_HIDE                                     =1048576,
	AFF_SILENCE                                  =2097152,
	AFF_CHARM                                    =4194304,
	AFF_FOLLOW                                   =8388608,
	AFF_PROTECT_FROM_EVIL                        =16777216,
	AFF_TRUE_SIGHT                               =33554432,
	AFF_SCRYING                                  =67108864, /*seeing other rooms*/
	AFF_FIRESHIELD                               =134217728,
	AFF_GROUP                                    =268435456,
	AFF_TELEPATHY                                =536870912,
	AFF_GLOBE_DARKNESS                           =1073741824, /*Added by REQUIEM 2018*/
	AFF_UNDEF_AFF_1                              =2147483648
};
#define E_AFFECTED_BY_COUNT 34
#define E_AFFECTED_BY_MIN 0
#define E_AFFECTED_BY_MAX 2147483648
#define E_AFFECTED_BY_KEY "e_affected_by"

#define E_AFFECTED_BY_ACCEPT_ZERO true
// Enum type: E_AFFECTED_BY -- end

// Enum type: E_AFFECTED_BY2 -- start
enum e_affected_by2 {
	AFF2_ANIMAL_INVIS                            =1,
	AFF2_HEAT_STUFF                              =2,
	AFF2_LOG_ME                                  =4,
	AFF2_BERSERK                                 =8,
	AFF2_PARRY                                   =16, /*Added by GAIA 2001*/
	AFF2_CON_ORDER                               =32,
	AFF2_AFK                                     =64,
	AFF2_PKILLER                                 =128
};
#define E_AFFECTED_BY2_COUNT 9
#define E_AFFECTED_BY2_MIN 1
#define E_AFFECTED_BY2_MAX 128
#define E_AFFECTED_BY2_KEY "e_affected_by2"

#define E_AFFECTED_BY2_ACCEPT_ZERO true
// Enum type: E_AFFECTED_BY2 -- end

// Enum type: E_APPLY -- start
enum e_apply {
/*
modifiers to char's abilities*/
	APPLY_NONE                                   =0,
	APPLY_STR                                    =1,
	APPLY_DEX                                    =2,
	APPLY_INT                                    =3,
	APPLY_WIS                                    =4,
	APPLY_CON                                    =5,
	APPLY_CHR                                    =6,
	APPLY_SEX                                    =7,
	APPLY_LEVEL                                  =8,
	APPLY_AGE                                    =9,
	APPLY_CHAR_WEIGHT                            =10,
	APPLY_CHAR_HEIGHT                            =11,
	APPLY_MANA                                   =12,
	APPLY_HIT                                    =13,
	APPLY_MOVE                                   =14,
	APPLY_GOLD                                   =15,
	APPLY_EXP                                    =16,
	APPLY_AC                                     =17,
	APPLY_HITROLL                                =18,
	APPLY_DAMROLL                                =19,
	APPLY_SAVING_PARA                            =20,
	APPLY_SAVING_ROD                             =21,
	APPLY_SAVING_PETRI                           =22,
	APPLY_SAVING_BREATH                          =23,
	APPLY_SAVING_SPELL                           =24,
	APPLY_SAVE_ALL                               =25,
	APPLY_IMMUNE                                 =26,
	APPLY_SUSC                                   =27,
	APPLY_M_IMMUNE                               =28,
	APPLY_SPELL                                  =29,
	APPLY_WEAPON_SPELL                           =30,
	APPLY_EAT_SPELL                              =31,
	APPLY_BACKSTAB                               =32,
	APPLY_KICK                                   =33,
	APPLY_SNEAK                                  =34,
	APPLY_HIDE                                   =35,
	APPLY_BASH                                   =36,
	APPLY_PICK                                   =37,
	APPLY_STEAL                                  =38,
	APPLY_TRACK                                  =39,
	APPLY_HITNDAM                                =40,
	APPLY_SPELLFAIL                              =41,
	APPLY_ATTACKS                                =42,
	APPLY_HASTE                                  =43,
	APPLY_SLOW                                   =44,
	APPLY_AFF2                                   =45,
	APPLY_FIND_TRAPS                             =46,
	APPLY_RIDE                                   =47,
	APPLY_RACE_SLAYER                            =48,
	APPLY_ALIGN_SLAYER                           =49,
	APPLY_MANA_REGEN                             =50,
	APPLY_HIT_REGEN                              =51,
	APPLY_MOVE_REGEN                             =52, /*Set thirst/hunger/drunk to MOD*/
	APPLY_MOD_THIRST                             =53,
	APPLY_MOD_HUNGER                             =54,
	APPLY_MOD_DRUNK                              =55,
	APPLY_T_STR                                  =56,
	APPLY_T_INT                                  =57,
	APPLY_T_DEX                                  =58,
	APPLY_T_WIS                                  =59,
	APPLY_T_CON                                  =60,
	APPLY_T_CHR                                  =61,
	APPLY_T_HPS                                  =62,
	APPLY_T_MOVE                                 =63,
	APPLY_T_MANA                                 =64,
	APPLY_SKIP                                   =65 /*Non viene toccato il bitvector*/
};
#define E_APPLY_COUNT 67
#define E_APPLY_MIN 0
#define E_APPLY_MAX 65
#define E_APPLY_KEY "e_apply"

#define E_APPLY_ACCEPT_ZERO true
// Enum type: E_APPLY -- end

// Enum type: E_CLASS_INDEX -- start
enum e_class_index {
/*
multiclassing stuff*/
	MAGE_LEVEL_IND                               =0,
	CLERIC_LEVEL_IND                             =1,
	WARRIOR_LEVEL_IND                            =2,
	THIEF_LEVEL_IND                              =3,
	DRUID_LEVEL_IND                              =4,
	MONK_LEVEL_IND                               =5,
	BARBARIAN_LEVEL_IND                          =6,
	SORCERER_LEVEL_IND                           =7,
	PALADIN_LEVEL_IND                            =8,
	RANGER_LEVEL_IND                             =9,
	PSI_LEVEL_IND                                =10
};
#define E_CLASS_INDEX_COUNT 12
#define E_CLASS_INDEX_MIN 0
#define E_CLASS_INDEX_MAX 10
#define E_CLASS_INDEX_KEY "e_class_index"

#define E_CLASS_INDEX_ACCEPT_ZERO true
// Enum type: E_CLASS_INDEX -- end

// Enum type: E_CLASSES -- start
enum e_classes {
	CLASS_MAGIC_USER                             =1,
	CLASS_CLERIC                                 =2,
	CLASS_WARRIOR                                =4,
	CLASS_THIEF                                  =8,
	CLASS_DRUID                                  =16,
	CLASS_MONK                                   =32,
	CLASS_BARBARIAN                              =64,
	CLASS_SORCERER                               =128,
	CLASS_PALADIN                                =256,
	CLASS_RANGER                                 =512,
	CLASS_PSI                                    =1024
};
#define E_CLASSES_COUNT 12
#define E_CLASSES_MIN 1
#define E_CLASSES_MAX 1024
#define E_CLASSES_KEY "e_classes"

#define E_CLASSES_ACCEPT_ZERO true
// Enum type: E_CLASSES -- end

// Enum type: E_CONDITIONS -- start
enum e_conditions {
	DRUNK                                        =0,
	FULL                                         =1,
	THIRST                                       =2
};
#define E_CONDITIONS_COUNT 4
#define E_CONDITIONS_MIN 0
#define E_CONDITIONS_MAX 2
#define E_CONDITIONS_KEY "e_conditions"

#define E_CONDITIONS_ACCEPT_ZERO true
// Enum type: E_CONDITIONS -- end

// Enum type: E_CONNECTION_TYPES -- start
enum e_connection_types {
/*
modes of connectedness
ATTENZIONE se si aggiungono altri stati di connessione, modificare anche
l'array connected_types in constants.c*/
	CON_PLYNG                                    =0,
	CON_NME                                      =1,
	CON_NMECNF                                   =2,
	CON_PWDNRM                                   =3,
	CON_PWDGET                                   =4,
	CON_PWDCNF                                   =5,
	CON_QSEX                                     =6,
	CON_RMOTD                                    =7,
	CON_SLCT                                     =8,
	CON_EXDSCR                                   =9,
	CON_QCLASS                                   =10,
	CON_LDEAD                                    =11,
	CON_PWDNEW                                   =12,
	CON_PWDNCNF                                  =13,
	CON_WIZLOCK                                  =14,
	CON_QRACE                                    =15,
	CON_RACPAR                                   =16,
	CON_AUTH                                     =17,
	CON_CITY_CHOICE                              =18,
	CON_STAT_LIST                                =19,
	CON_QDELETE                                  =20,
	CON_QDELETE2                                 =21,
	CON_STAT_LISTV                               =22,
	CON_WMOTD                                    =23,
	CON_EDITING                                  =24,
	CON_DELETE_ME                                =25,
	CON_CHECK_MAGE_TYPE                          =26,
	CON_OBJ_EDITING                              =27,
	CON_MOB_EDITING                              =28,
	CON_RNEWD                                    =29,
	CON_HELPRACE                                 =30,
	CON_ENDHELPRACE                              =31,
	CON_HELPCLASS                                =32,
	CON_ENDHELPCLASS                             =33,
	CON_HELPROLL                                 =34,
	CON_QROLL                                    =35,
	CON_CONF_ROLL                                =36,
	CON_EXTRA2                                   =37,
	CON_OBJ_FORGING                              =38
};
#define E_CONNECTION_TYPES_COUNT 40
#define E_CONNECTION_TYPES_MIN 0
#define E_CONNECTION_TYPES_MAX 38
#define E_CONNECTION_TYPES_KEY "e_connection_types"

#define E_CONNECTION_TYPES_ACCEPT_ZERO true
// Enum type: E_CONNECTION_TYPES -- end

// Enum type: E_DAMAGE_TYPE -- start
enum e_damage_type {
	FIRE_DAMAGE                                  =1,
	COLD_DAMAGE                                  =2,
	ELEC_DAMAGE                                  =3,
	BLOW_DAMAGE                                  =4,
	ACID_DAMAGE                                  =5
};
#define E_DAMAGE_TYPE_COUNT 6
#define E_DAMAGE_TYPE_MIN 1
#define E_DAMAGE_TYPE_MAX 5
#define E_DAMAGE_TYPE_KEY "e_damage_type"

#define E_DAMAGE_TYPE_ACCEPT_ZERO true
// Enum type: E_DAMAGE_TYPE -- end

// Enum type: E_EVENTS -- start
enum e_events {
	EVENT_COMMAND                                =0,
	EVENT_TICK                                   =1,
	EVENT_DEATH                                  =2,
	EVENT_SUMMER                                 =3,
	EVENT_SPRING                                 =4,
	EVENT_FALL                                   =5,
	EVENT_WINTER                                 =6,
	EVENT_GATHER                                 =7,
	EVENT_ATTACK                                 =8,
	EVENT_FOLLOW                                 =9,
	EVENT_MONTH                                  =10,
	EVENT_BIRTH                                  =11, /*birth event for the mob.*/
	EVENT_FAMINE                                 =12,
	EVENT_DWARVES_STRIKE                         =13, /*fitting number  eh? -DM*/
	EVENT_END_STRIKE                             =14,
	EVENT_END_FAMINE                             =15,
	EVENT_WEEK                                   =16,
	EVENT_GOBLIN_RAID                            =17,
	EVENT_END_GOB_RAID                           =18,
	EVENT_FIGHTING                               =19
};
#define E_EVENTS_COUNT 21
#define E_EVENTS_MIN 0
#define E_EVENTS_MAX 19
#define E_EVENTS_KEY "e_events"

#define E_EVENTS_ACCEPT_ZERO true
// Enum type: E_EVENTS -- end

// Enum type: E_EXIT_DIR -- start
enum e_exit_dir {
/*
For 'dir_option'*/
	NORTH                                        =0,
	EAST                                         =1,
	SOUTH                                        =2,
	WEST                                         =3,
	UP                                           =4,
	DOWN                                         =5
};
#define E_EXIT_DIR_COUNT 7
#define E_EXIT_DIR_MIN 0
#define E_EXIT_DIR_MAX 5
#define E_EXIT_DIR_KEY "e_exit_dir"

#define E_EXIT_DIR_ACCEPT_ZERO true
// Enum type: E_EXIT_DIR -- end

// Enum type: E_EXIT_TYPES -- start
enum e_exit_types {
	EX_ISDOOR                                    =1,
	EX_CLOSED                                    =2,
	EX_LOCKED                                    =4,
	EX_SECRET                                    =8,
	EX_NOTBASH                                   =16,
	EX_PICKPROOF                                 =32,
	EX_CLIMB                                     =64,
	EX_MALE                                      =128,
	EX_NOLOOKT                                   =256
};
#define E_EXIT_TYPES_COUNT 10
#define E_EXIT_TYPES_MIN 1
#define E_EXIT_TYPES_MAX 256
#define E_EXIT_TYPES_KEY "e_exit_types"

#define E_EXIT_TYPES_ACCEPT_ZERO true
// Enum type: E_EXIT_TYPES -- end

// Enum type: E_FEAR_TYPE -- start
enum e_fear_type {
	FEAR_SEX                                     =1,
	FEAR_RACE                                    =2,
	FEAR_CHAR                                    =4,
	FEAR_CLASS                                   =8,
	FEAR_EVIL                                    =16,
	FEAR_GOOD                                    =32,
	FEAR_VNUM                                    =64
};
#define E_FEAR_TYPE_COUNT 8
#define E_FEAR_TYPE_MIN 1
#define E_FEAR_TYPE_MAX 64
#define E_FEAR_TYPE_KEY "e_fear_type"

#define E_FEAR_TYPE_ACCEPT_ZERO true
// Enum type: E_FEAR_TYPE -- end

// Enum type: E_HATE_TYPE -- start
enum e_hate_type {
	HATE_SEX                                     =1,
	HATE_RACE                                    =2,
	HATE_CHAR                                    =4,
	HATE_CLASS                                   =8,
	HATE_EVIL                                    =16,
	HATE_GOOD                                    =32,
	HATE_VNUM                                    =64
};
#define E_HATE_TYPE_COUNT 8
#define E_HATE_TYPE_MIN 1
#define E_HATE_TYPE_MAX 64
#define E_HATE_TYPE_KEY "e_hate_type"

#define E_HATE_TYPE_ACCEPT_ZERO true
// Enum type: E_HATE_TYPE -- end

// Enum type: E_IMMUNITY_TYPE -- start
enum e_immunity_type {
	IMM_FIRE                                     =1,
	IMM_COLD                                     =2,
	IMM_ELEC                                     =4,
	IMM_ENERGY                                   =8,
	IMM_BLUNT                                    =16,
	IMM_PIERCE                                   =32,
	IMM_SLASH                                    =64,
	IMM_ACID                                     =128,
	IMM_POISON                                   =256,
	IMM_DRAIN                                    =512,
	IMM_SLEEP                                    =1024,
	IMM_CHARM                                    =2048,
	IMM_HOLD                                     =4096,
	IMM_NONMAG                                   =8192,
	IMM_PLUS1                                    =16384,
	IMM_PLUS2                                    =32768,
	IMM_PLUS3                                    =65536,
	IMM_PLUS4                                    =131072
};
#define E_IMMUNITY_TYPE_COUNT 19
#define E_IMMUNITY_TYPE_MIN 1
#define E_IMMUNITY_TYPE_MAX 131072
#define E_IMMUNITY_TYPE_KEY "e_immunity_type"

#define E_IMMUNITY_TYPE_ACCEPT_ZERO true
// Enum type: E_IMMUNITY_TYPE -- end

// Enum type: E_LARGE_FLAGS -- start
enum e_large_flags {
	LARGE_NONE                                   =0,
	LARGE_WATER                                  =1,
	LARGE_AIR                                    =2,
	LARGE_IMPASS                                 =4
};
#define E_LARGE_FLAGS_COUNT 5
#define E_LARGE_FLAGS_MIN 0
#define E_LARGE_FLAGS_MAX 4
#define E_LARGE_FLAGS_KEY "e_large_flags"

#define E_LARGE_FLAGS_ACCEPT_ZERO true
// Enum type: E_LARGE_FLAGS -- end

// Enum type: E_MOB_FLAGS -- start
enum e_mob_flags {
/*
for mobile actions: specials.act*/
	ACT_SPEC                                     =1, /*special routine to be called if exist*/
	ACT_SENTINEL                                 =2, /*this mobile not to be moved*/
	ACT_SCAVENGER                                =4, /*pick up stuff lying around*/
	ACT_ISNPC                                    =8, /*This bit is set for use with IS_NPC()*/
	ACT_NICE_THIEF                               =16, /*Set if a thief should NOT be killed*/
	ACT_AGGRESSIVE                               =32, /*Set if automatic attack on NPC's*/
	ACT_STAY_ZONE                                =64, /*MOB Must stay inside its own zone*/
	ACT_WIMPY                                    =128, /*MOB Will flee when injured  and if aggressive only attack sleeping players*/
	ACT_ANNOYING                                 =256, /*MOB is so utterly irritating that other monsters will attack it...*/
	ACT_HATEFUL                                  =512, /*MOB will attack a PC or NPC matching a  specified name*/
	ACT_AFRAID                                   =1024, /*MOB is afraid of a certain PC or NPC   and will always run away ....*/
	ACT_IMMORTAL                                 =2048, /*MOB is a natural event  can't be kiled*/
	ACT_HUNTING                                  =4096, /*MOB is hunting someone*/
	ACT_DEADLY                                   =8192, /*MOB has deadly poison*/
	ACT_POLYSELF                                 =16384, /*MOB is a polymorphed person*/
	ACT_META_AGG                                 =32768, /*MOB is _very_ aggressive*/
	ACT_GUARDIAN                                 =65536, /*MOB will guard master*/
	ACT_ILLUSION                                 =131072, /*MOB is illusionary*/
	ACT_HUGE                                     =262144, /*MOB is too large to go indoors*/
	ACT_SCRIPT                                   =524288, /*MOB has a script assigned to it DO NOT SET*/
	ACT_GREET                                    =1048576, /*MOB greets people*/
	ACT_MAGIC_USER                               =2097152,
	ACT_WARRIOR                                  =4194304,
	ACT_CLERIC                                   =8388608,
	ACT_THIEF                                    =16777216,
	ACT_DRUID                                    =33554432,
	ACT_MONK                                     =67108864,
	ACT_BARBARIAN                                =134217728,
	ACT_PALADIN                                  =268435456,
	ACT_RANGER                                   =536870912,
	ACT_PSI                                      =1073741824,
	ACT_ARCHER                                   =2147483648
};
#define E_MOB_FLAGS_COUNT 33
#define E_MOB_FLAGS_MIN 1
#define E_MOB_FLAGS_MAX 2147483648
#define E_MOB_FLAGS_KEY "e_mob_flags"

#define E_MOB_FLAGS_ACCEPT_ZERO true
// Enum type: E_MOB_FLAGS -- end

// Enum type: E_OPINION_OP -- start
enum e_opinion_op {
	OP_SEX                                       =1,
	OP_RACE                                      =2,
	OP_CHAR                                      =3,
	OP_CLASS                                     =4,
	OP_EVIL                                      =5,
	OP_GOOD                                      =6,
	OP_VNUM                                      =7
};
#define E_OPINION_OP_COUNT 8
#define E_OPINION_OP_MIN 1
#define E_OPINION_OP_MAX 7
#define E_OPINION_OP_KEY "e_opinion_op"

#define E_OPINION_OP_ACCEPT_ZERO true
// Enum type: E_OPINION_OP -- end

// Enum type: E_PLAYER_FLAGS -- start
enum e_player_flags {
/*
For players : specials.act*/
	PLR_BRIEF                                    =1,
	PLR_UNUSED                                   =2,
	PLR_COMPACT                                  =4,
	PLR_DONTSET                                  =8, /*Dont EVER set*/
	PLR_WIMPY                                    =16, /*character will flee when seriously injured*/
	PLR_NOHASSLE                                 =32, /*char won't be attacked by aggressives.*/
	PLR_STEALTH                                  =64, /*char won't be announced in a variety of situations*/
	PLR_HUNTING                                  =128, /*the player is hunting someone  do a track each look*/
	PLR_DEAF                                     =256, /*The player does not hear shouts*/
	PLR_ECHO                                     =512, /*Messages (tells  shout etc) echo back*/
	PLR_NOGOSSIP                                 =1024, /*New  gossip channel*/
	PLR_NOAUCTION                                =2048, /*New AUTCION channel*/
	PLR_NOTHING                                  =4096, /*empty*/
	PLR_NOTHING2                                 =8192, /*empty*/
	PLR_NOSHOUT                                  =16384, /*the player is not allowed to shout*/
	PLR_FREEZE                                   =32768, /*The player is frozen  must have pissed an immo off*/
	PLR_NOTELL                                   =65536, /*The player does not hear tells*/
	PLR_MAILING                                  =131072,
	PLR_EMPTY4                                   =262144,
	PLR_EMPTY5                                   =524288,
	PLR_NOBEEP                                   =1048576 /*ignore all beeps*/
};
#define E_PLAYER_FLAGS_COUNT 22
#define E_PLAYER_FLAGS_MIN 1
#define E_PLAYER_FLAGS_MAX 1048576
#define E_PLAYER_FLAGS_KEY "e_player_flags"

#define E_PLAYER_FLAGS_ACCEPT_ZERO true
// Enum type: E_PLAYER_FLAGS -- end

// Enum type: E_POSITIONS -- start
enum e_positions {
/*
positions*/
	POSITION_DEAD                                =0,
	POSITION_MORTALLYW                           =1,
	POSITION_INCAP                               =2,
	POSITION_STUNNED                             =3,
	POSITION_SLEEPING                            =4,
	POSITION_RESTING                             =5,
	POSITION_SITTING                             =6,
	POSITION_FIGHTING                            =7,
	POSITION_STANDING                            =8,
	POSITION_MOUNTED                             =9
};
#define E_POSITIONS_COUNT 11
#define E_POSITIONS_MIN 0
#define E_POSITIONS_MAX 9
#define E_POSITIONS_KEY "e_positions"

#define E_POSITIONS_ACCEPT_ZERO true
// Enum type: E_POSITIONS -- end

// Enum type: E_ROOM_FLAGS -- start
enum e_room_flags {
/*
Bitvector For 'room_flags'*/
	DARK                                         =1,
	DEATH                                        =2,
	NO_MOB                                       =4,
	INDOORS                                      =8,
	PEACEFUL                                     =16, /*No fighting*/
	NOSTEAL                                      =32, /*No Thieving*/
	NO_SUM                                       =64, /*no summoning*/
	NO_MAGIC                                     =128,
	TUNNEL                                       =256, /*Limited #s of people in room*/
	PRIVATE                                      =512,
	SILENCE                                      =1024,
	LARGE                                        =2048,
	NO_DEATH                                     =4096,
	SAVE_ROOM                                    =8192, /*room will save eq and load at reboot*/
	NO_TRACK                                     =16384,
	NO_MIND                                      =32768,
	DESERTIC                                     =65536,
	ARTIC                                        =131072,
	UNDERGROUND                                  =262144,
	HOT                                          =524288,
	WET                                          =1048576,
	COLD                                         =2097152,
	DRY                                          =4194304,
	BRIGHT                                       =8388608,
	NO_ASTRAL                                    =16777216,
	NO_REGAIN                                    =33554432,
	RM_1                                         =67108864,
	RM_2                                         =134217728,
	RM_3                                         =268435456,
	RM_4                                         =536870912,
	RM_5                                         =1073741824
};
#define E_ROOM_FLAGS_COUNT 32
#define E_ROOM_FLAGS_MIN 1
#define E_ROOM_FLAGS_MAX 1073741824
#define E_ROOM_FLAGS_KEY "e_room_flags"

#define E_ROOM_FLAGS_ACCEPT_ZERO true
// Enum type: E_ROOM_FLAGS -- end

// Enum type: E_SEASONS -- start
enum e_seasons {
	SEASON_WINTER                                =1,
	SEASON_SPRING                                =2,
	SEASON_SUMMER                                =4,
	SEASON_FALL                                  =8
};
#define E_SEASONS_COUNT 5
#define E_SEASONS_MIN 1
#define E_SEASONS_MAX 8
#define E_SEASONS_KEY "e_seasons"

#define E_SEASONS_ACCEPT_ZERO true
// Enum type: E_SEASONS -- end

// Enum type: E_SECTOR_TYPES -- start
enum e_sector_types {
/*
For 'Sector types'*/
	SECT_INSIDE                                  =0,
	SECT_CITY                                    =1,
	SECT_FIELD                                   =2,
	SECT_FOREST                                  =3,
	SECT_HILLS                                   =4,
	SECT_MOUNTAIN                                =5,
	SECT_WATER_SWIM                              =6,
	SECT_WATER_NOSWIM                            =7,
	SECT_AIR                                     =8,
	SECT_UNDERWATER                              =9,
	SECT_DESERT                                  =10,
	SECT_TREE                                    =11,
	SECT_DARKCITY                                =12
};
#define E_SECTOR_TYPES_COUNT 14
#define E_SECTOR_TYPES_MIN 0
#define E_SECTOR_TYPES_MAX 12
#define E_SECTOR_TYPES_KEY "e_sector_types"

#define E_SECTOR_TYPES_ACCEPT_ZERO true
// Enum type: E_SECTOR_TYPES -- end

// Enum type: E_SEX -- start
enum e_sex {
/*
sex*/
	SEX_NEUTRAL                                  =0,
	SEX_MALE                                     =1,
	SEX_FEMALE                                   =2
};
#define E_SEX_COUNT 4
#define E_SEX_MIN 0
#define E_SEX_MAX 2
#define E_SEX_KEY "e_sex"

#define E_SEX_ACCEPT_ZERO true
// Enum type: E_SEX -- end

// Enum type: E_SKILL_KNOWN -- start
enum e_skill_known {
/*
skill_data flags*/
	SKILL_KNOWN                                  =1,
	SKILL_KNOWN_CLERIC                           =2,
	SKILL_KNOWN_MAGE                             =4,
	SKILL_KNOWN_SORCERER                         =8,
	SKILL_KNOWN_THIEF                            =16,
	SKILL_KNOWN_MONK                             =32,
	SKILL_KNOWN_DRUID                            =64,
	SKILL_KNOWN_WARRIOR                          =128,
	SKILL_KNOWN_BARBARIAN                        =256, /*need to make byte flags, to long or int flags in char_skill_data*/
	SKILL_KNOWN_PALADIN                          =512, /*need to make byte flags, to long or int flags in char_skill_data*/
	SKILL_KNOWN_RANGER                           =1024, /*need to make byte flags, to long or int flags in char_skill_data*/
	SKILL_KNOWN_PSI                              =2048 /*need to make byte flags, to long or int flags in char_skill_data*/
};
#define E_SKILL_KNOWN_COUNT 13
#define E_SKILL_KNOWN_MIN 1
#define E_SKILL_KNOWN_MAX 2048
#define E_SKILL_KNOWN_KEY "e_skill_known"

#define E_SKILL_KNOWN_ACCEPT_ZERO true
// Enum type: E_SKILL_KNOWN -- end

// Enum type: E_SKY -- start
enum e_sky {
/*
And how is the sky ?*/
	SKY_CLOUDLESS                                =0,
	SKY_CLOUDY                                   =1,
	SKY_RAINING                                  =2,
	SKY_LIGHTNING                                =3
};
#define E_SKY_COUNT 5
#define E_SKY_MIN 0
#define E_SKY_MAX 3
#define E_SKY_KEY "e_sky"

#define E_SKY_ACCEPT_ZERO true
// Enum type: E_SKY -- end

// Enum type: E_SPEAKS -- start
enum e_speaks {
	SPEAK_COMMON                                 =1,
	SPEAK_ELVISH                                 =2,
	SPEAK_HALFLING                               =3,
	SPEAK_DWARVISH                               =4,
	SPEAK_ORCISH                                 =5,
	SPEAK_GIANTISH                               =6,
	SPEAK_OGRE                                   =7,
	SPEAK_GNOMISH                                =8
};
#define E_SPEAKS_COUNT 9
#define E_SPEAKS_MIN 1
#define E_SPEAKS_MAX 8
#define E_SPEAKS_KEY "e_speaks"

#define E_SPEAKS_ACCEPT_ZERO true
// Enum type: E_SPEAKS -- end

// Enum type: E_SPECIAL_EVENTS -- start
enum e_special_events {
	DWARVES_STRIKE                               =1,
	FAMINE                                       =2
};
#define E_SPECIAL_EVENTS_COUNT 3
#define E_SPECIAL_EVENTS_MIN 1
#define E_SPECIAL_EVENTS_MAX 2
#define E_SPECIAL_EVENTS_KEY "e_special_events"

#define E_SPECIAL_EVENTS_ACCEPT_ZERO true
// Enum type: E_SPECIAL_EVENTS -- end

// Enum type: E_SUNLIGHT -- start
enum e_sunlight {
/*
How much light is in the land ?*/
	SUN_DARK                                     =0,
	SUN_RISE                                     =1,
	SUN_LIGHT                                    =2,
	SUN_SET                                      =3,
	MOON_SET                                     =4,
	MOON_RISE                                    =5 /*moon changes -DM 7/16/92*/
};
#define E_SUNLIGHT_COUNT 7
#define E_SUNLIGHT_MIN 0
#define E_SUNLIGHT_MAX 5
#define E_SUNLIGHT_KEY "e_sunlight"

#define E_SUNLIGHT_ACCEPT_ZERO true
// Enum type: E_SUNLIGHT -- end

// Enum type: E_SYS_FLAGS -- start
enum e_sys_flags {
/*
system flags defined on the fly and by wizards for this boot*/
	SYS_NOPORTAL                                 =1, /*no one can portal*/
	SYS_NOASTRAL                                 =2, /*no one can astral*/
	SYS_NOSUMMON                                 =4, /*no one can summon*/
	SYS_NOKILL                                   =8, /*NO PC (good side or bad side) can fight*/
	SYS_LOGALL                                   =16, /*log ALL users to the system log*/
	SYS_ECLIPS                                   =32, /*the world is in constant darkness!*/
	SYS_SKIPDNS                                  =64, /*skips DNS name searches on connects*/
	SYS_REQAPPROVE                               =128, /*force god approval for new char*/
	SYS_NOANSI                                   =256, /*disable ansi colors world wide*/
	SYS_LOGMOB                                   =512 /*log ALL mob ti system log*/
};
#define E_SYS_FLAGS_COUNT 11
#define E_SYS_FLAGS_MIN 1
#define E_SYS_FLAGS_MAX 512
#define E_SYS_FLAGS_KEY "e_sys_flags"

#define E_SYS_FLAGS_ACCEPT_ZERO true
// Enum type: E_SYS_FLAGS -- end

// Enum type: E_TELEPORTS -- start
enum e_teleports {
	TELE_LOOK                                    =1,
	TELE_COUNT                                   =2,
	TELE_RANDOM                                  =4,
	TELE_SPIN                                    =8
};
#define E_TELEPORTS_COUNT 5
#define E_TELEPORTS_MIN 1
#define E_TELEPORTS_MAX 8
#define E_TELEPORTS_KEY "e_teleports"

#define E_TELEPORTS_ACCEPT_ZERO true
// Enum type: E_TELEPORTS -- end

// Enum type: E_USER_FLAGS -- start
enum e_user_flags {
/*
user flags*/
	NO_DELETE                                    =1, /*do not delete me  well until 6 months pass!*/
	USE_ANSI                                     =2, /*we use ANSI color  yeah!*/
	RACE_WAR                                     =4, /*if enabled they can be hit by opposing race*/
	SHOW_CLASSES                                 =8, /*If enabled the PG classes are shown in the WHO*/
	SHOW_EXITS                                   =16, /*for auto display exits*/
	MURDER_1                                     =32, /*actually killed someone!*/
	STOLE_1                                      =64, /*caught stealing form someone!*/
	MURDER_2                                     =128, /*rumored killed someone*/
	STOLE_2                                      =256, /*rumored stole from someone*/
	STOLE_X                                      =512, /*saved for later*/
	MURDER_X                                     =1024, /*saved for later*/
	USE_PAGING                                   =2048, /*pause screens?*/
	CAN_OBJ_EDIT                                 =4096, /*this user can use oedit to edit objects*/
	CAN_MOB_EDIT                                 =8192 /*this user can use medit to edit mobs*/
};
#define E_USER_FLAGS_COUNT 15
#define E_USER_FLAGS_MIN 1
#define E_USER_FLAGS_MAX 8192
#define E_USER_FLAGS_KEY "e_user_flags"

#define E_USER_FLAGS_ACCEPT_ZERO true
// Enum type: E_USER_FLAGS -- end

// Enum type: E_WEAR_AS -- start
enum e_wear_as {
	WEAR_LIGHT                                   =0,
	WEAR_FINGER_R                                =1,
	WEAR_FINGER_L                                =2,
	WEAR_NECK_1                                  =3,
	WEAR_NECK_2                                  =4,
	WEAR_BODY                                    =5,
	WEAR_HEAD                                    =6,
	WEAR_LEGS                                    =7,
	WEAR_FEET                                    =8,
	WEAR_HANDS                                   =9,
	WEAR_ARMS                                    =10,
	WEAR_SHIELD                                  =11,
	WEAR_ABOUT                                   =12,
	WEAR_WAISTE                                  =13,
	WEAR_WRIST_R                                 =14,
	WEAR_WRIST_L                                 =15,
	WIELD                                        =16,
	HOLD                                         =17,
	WEAR_BACK                                    =18,
	WEAR_EAR_R                                   =19,
	WEAR_EAR_L                                   =20,
	WEAR_EYES                                    =21,
	LOADED_WEAPON                                =22
};
#define E_WEAR_AS_COUNT 24
#define E_WEAR_AS_MIN 0
#define E_WEAR_AS_MAX 22
#define E_WEAR_AS_KEY "e_wear_as"

#define E_WEAR_AS_ACCEPT_ZERO true
// Enum type: E_WEAR_AS -- end

namespace G {
extern e_affected_by encode_e_affected_by(std::string s,e_affected_by fallback = AFF_NONE);
extern std::string translate(const e_affected_by e);
extern bool enum_validate(const e_affected_by value);
extern std::ostream & operator<<(std::ostream &out,const e_affected_by value);
extern e_affected_by2 encode_e_affected_by2(std::string s,e_affected_by2 fallback = AFF2_ANIMAL_INVIS);
extern std::string translate(const e_affected_by2 e);
extern bool enum_validate(const e_affected_by2 value);
extern std::ostream & operator<<(std::ostream &out,const e_affected_by2 value);
extern e_apply encode_e_apply(std::string s,e_apply fallback = APPLY_NONE);
extern std::string translate(const e_apply e);
extern bool enum_validate(const e_apply value);
extern std::ostream & operator<<(std::ostream &out,const e_apply value);
extern e_class_index encode_e_class_index(std::string s,e_class_index fallback = MAGE_LEVEL_IND);
extern std::string translate(const e_class_index e);
extern bool enum_validate(const e_class_index value);
extern std::ostream & operator<<(std::ostream &out,const e_class_index value);
extern e_classes encode_e_classes(std::string s,e_classes fallback = CLASS_MAGIC_USER);
extern std::string translate(const e_classes e);
extern bool enum_validate(const e_classes value);
extern std::ostream & operator<<(std::ostream &out,const e_classes value);
extern e_conditions encode_e_conditions(std::string s,e_conditions fallback = DRUNK);
extern std::string translate(const e_conditions e);
extern bool enum_validate(const e_conditions value);
extern std::ostream & operator<<(std::ostream &out,const e_conditions value);
extern e_connection_types encode_e_connection_types(std::string s,e_connection_types fallback = CON_PLYNG);
extern std::string translate(const e_connection_types e);
extern bool enum_validate(const e_connection_types value);
extern std::ostream & operator<<(std::ostream &out,const e_connection_types value);
extern e_damage_type encode_e_damage_type(std::string s,e_damage_type fallback = FIRE_DAMAGE);
extern std::string translate(const e_damage_type e);
extern bool enum_validate(const e_damage_type value);
extern std::ostream & operator<<(std::ostream &out,const e_damage_type value);
extern e_events encode_e_events(std::string s,e_events fallback = EVENT_COMMAND);
extern std::string translate(const e_events e);
extern bool enum_validate(const e_events value);
extern std::ostream & operator<<(std::ostream &out,const e_events value);
extern e_exit_dir encode_e_exit_dir(std::string s,e_exit_dir fallback = NORTH);
extern std::string translate(const e_exit_dir e);
extern bool enum_validate(const e_exit_dir value);
extern std::ostream & operator<<(std::ostream &out,const e_exit_dir value);
extern e_exit_types encode_e_exit_types(std::string s,e_exit_types fallback = EX_ISDOOR);
extern std::string translate(const e_exit_types e);
extern bool enum_validate(const e_exit_types value);
extern std::ostream & operator<<(std::ostream &out,const e_exit_types value);
extern e_fear_type encode_e_fear_type(std::string s,e_fear_type fallback = FEAR_SEX);
extern std::string translate(const e_fear_type e);
extern bool enum_validate(const e_fear_type value);
extern std::ostream & operator<<(std::ostream &out,const e_fear_type value);
extern e_hate_type encode_e_hate_type(std::string s,e_hate_type fallback = HATE_SEX);
extern std::string translate(const e_hate_type e);
extern bool enum_validate(const e_hate_type value);
extern std::ostream & operator<<(std::ostream &out,const e_hate_type value);
extern e_immunity_type encode_e_immunity_type(std::string s,e_immunity_type fallback = IMM_FIRE);
extern std::string translate(const e_immunity_type e);
extern bool enum_validate(const e_immunity_type value);
extern std::ostream & operator<<(std::ostream &out,const e_immunity_type value);
extern e_large_flags encode_e_large_flags(std::string s,e_large_flags fallback = LARGE_NONE);
extern std::string translate(const e_large_flags e);
extern bool enum_validate(const e_large_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_large_flags value);
extern e_mob_flags encode_e_mob_flags(std::string s,e_mob_flags fallback = ACT_SPEC);
extern std::string translate(const e_mob_flags e);
extern bool enum_validate(const e_mob_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_mob_flags value);
extern e_opinion_op encode_e_opinion_op(std::string s,e_opinion_op fallback = OP_SEX);
extern std::string translate(const e_opinion_op e);
extern bool enum_validate(const e_opinion_op value);
extern std::ostream & operator<<(std::ostream &out,const e_opinion_op value);
extern e_player_flags encode_e_player_flags(std::string s,e_player_flags fallback = PLR_BRIEF);
extern std::string translate(const e_player_flags e);
extern bool enum_validate(const e_player_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_player_flags value);
extern e_positions encode_e_positions(std::string s,e_positions fallback = POSITION_DEAD);
extern std::string translate(const e_positions e);
extern bool enum_validate(const e_positions value);
extern std::ostream & operator<<(std::ostream &out,const e_positions value);
extern e_room_flags encode_e_room_flags(std::string s,e_room_flags fallback = DARK);
extern std::string translate(const e_room_flags e);
extern bool enum_validate(const e_room_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_room_flags value);
extern e_seasons encode_e_seasons(std::string s,e_seasons fallback = SEASON_WINTER);
extern std::string translate(const e_seasons e);
extern bool enum_validate(const e_seasons value);
extern std::ostream & operator<<(std::ostream &out,const e_seasons value);
extern e_sector_types encode_e_sector_types(std::string s,e_sector_types fallback = SECT_INSIDE);
extern std::string translate(const e_sector_types e);
extern bool enum_validate(const e_sector_types value);
extern std::ostream & operator<<(std::ostream &out,const e_sector_types value);
extern e_sex encode_e_sex(std::string s,e_sex fallback = SEX_NEUTRAL);
extern std::string translate(const e_sex e);
extern bool enum_validate(const e_sex value);
extern std::ostream & operator<<(std::ostream &out,const e_sex value);
extern e_skill_known encode_e_skill_known(std::string s,e_skill_known fallback = SKILL_KNOWN);
extern std::string translate(const e_skill_known e);
extern bool enum_validate(const e_skill_known value);
extern std::ostream & operator<<(std::ostream &out,const e_skill_known value);
extern e_sky encode_e_sky(std::string s,e_sky fallback = SKY_CLOUDLESS);
extern std::string translate(const e_sky e);
extern bool enum_validate(const e_sky value);
extern std::ostream & operator<<(std::ostream &out,const e_sky value);
extern e_speaks encode_e_speaks(std::string s,e_speaks fallback = SPEAK_COMMON);
extern std::string translate(const e_speaks e);
extern bool enum_validate(const e_speaks value);
extern std::ostream & operator<<(std::ostream &out,const e_speaks value);
extern e_special_events encode_e_special_events(std::string s,e_special_events fallback = DWARVES_STRIKE);
extern std::string translate(const e_special_events e);
extern bool enum_validate(const e_special_events value);
extern std::ostream & operator<<(std::ostream &out,const e_special_events value);
extern e_sunlight encode_e_sunlight(std::string s,e_sunlight fallback = SUN_DARK);
extern std::string translate(const e_sunlight e);
extern bool enum_validate(const e_sunlight value);
extern std::ostream & operator<<(std::ostream &out,const e_sunlight value);
extern e_sys_flags encode_e_sys_flags(std::string s,e_sys_flags fallback = SYS_NOPORTAL);
extern std::string translate(const e_sys_flags e);
extern bool enum_validate(const e_sys_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_sys_flags value);
extern e_teleports encode_e_teleports(std::string s,e_teleports fallback = TELE_LOOK);
extern std::string translate(const e_teleports e);
extern bool enum_validate(const e_teleports value);
extern std::ostream & operator<<(std::ostream &out,const e_teleports value);
extern e_user_flags encode_e_user_flags(std::string s,e_user_flags fallback = NO_DELETE);
extern std::string translate(const e_user_flags e);
extern bool enum_validate(const e_user_flags value);
extern std::ostream & operator<<(std::ostream &out,const e_user_flags value);
extern e_wear_as encode_e_wear_as(std::string s,e_wear_as fallback = WEAR_LIGHT);
extern std::string translate(const e_wear_as e);
extern bool enum_validate(const e_wear_as value);
extern std::ostream & operator<<(std::ostream &out,const e_wear_as value);
} // namespace G
template<typename T>
std::string t(T e) {
    return G::translate(e);
}

}//namespace Alarmud
/* Ready for Lua
namespace luabridge {
template <>
struct Stack <gates::e_affected_by> {
    static void push (lua_State* L, gates::e_affected_by s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_affected_by get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_affected_by>(E_AFFECTED_BY_KEY,stringa,static_cast<gates::e_affected_by>(E_AFFECTED_BY_MIN));
    }
};

template <>
struct Stack <gates::e_affected_by2> {
    static void push (lua_State* L, gates::e_affected_by2 s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_affected_by2 get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_affected_by2>(E_AFFECTED_BY2_KEY,stringa,static_cast<gates::e_affected_by2>(E_AFFECTED_BY2_MIN));
    }
};

template <>
struct Stack <gates::e_apply> {
    static void push (lua_State* L, gates::e_apply s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_apply get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_apply>(E_APPLY_KEY,stringa,static_cast<gates::e_apply>(E_APPLY_MIN));
    }
};

template <>
struct Stack <gates::e_class_index> {
    static void push (lua_State* L, gates::e_class_index s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_class_index get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_class_index>(E_CLASS_INDEX_KEY,stringa,static_cast<gates::e_class_index>(E_CLASS_INDEX_MIN));
    }
};

template <>
struct Stack <gates::e_classes> {
    static void push (lua_State* L, gates::e_classes s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_classes get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_classes>(E_CLASSES_KEY,stringa,static_cast<gates::e_classes>(E_CLASSES_MIN));
    }
};

template <>
struct Stack <gates::e_conditions> {
    static void push (lua_State* L, gates::e_conditions s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_conditions get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_conditions>(E_CONDITIONS_KEY,stringa,static_cast<gates::e_conditions>(E_CONDITIONS_MIN));
    }
};

template <>
struct Stack <gates::e_connection_types> {
    static void push (lua_State* L, gates::e_connection_types s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_connection_types get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_connection_types>(E_CONNECTION_TYPES_KEY,stringa,static_cast<gates::e_connection_types>(E_CONNECTION_TYPES_MIN));
    }
};

template <>
struct Stack <gates::e_damage_type> {
    static void push (lua_State* L, gates::e_damage_type s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_damage_type get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_damage_type>(E_DAMAGE_TYPE_KEY,stringa,static_cast<gates::e_damage_type>(E_DAMAGE_TYPE_MIN));
    }
};

template <>
struct Stack <gates::e_events> {
    static void push (lua_State* L, gates::e_events s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_events get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_events>(E_EVENTS_KEY,stringa,static_cast<gates::e_events>(E_EVENTS_MIN));
    }
};

template <>
struct Stack <gates::e_exit_dir> {
    static void push (lua_State* L, gates::e_exit_dir s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_exit_dir get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_exit_dir>(E_EXIT_DIR_KEY,stringa,static_cast<gates::e_exit_dir>(E_EXIT_DIR_MIN));
    }
};

template <>
struct Stack <gates::e_exit_types> {
    static void push (lua_State* L, gates::e_exit_types s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_exit_types get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_exit_types>(E_EXIT_TYPES_KEY,stringa,static_cast<gates::e_exit_types>(E_EXIT_TYPES_MIN));
    }
};

template <>
struct Stack <gates::e_fear_type> {
    static void push (lua_State* L, gates::e_fear_type s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_fear_type get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_fear_type>(E_FEAR_TYPE_KEY,stringa,static_cast<gates::e_fear_type>(E_FEAR_TYPE_MIN));
    }
};

template <>
struct Stack <gates::e_hate_type> {
    static void push (lua_State* L, gates::e_hate_type s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_hate_type get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_hate_type>(E_HATE_TYPE_KEY,stringa,static_cast<gates::e_hate_type>(E_HATE_TYPE_MIN));
    }
};

template <>
struct Stack <gates::e_immunity_type> {
    static void push (lua_State* L, gates::e_immunity_type s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_immunity_type get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_immunity_type>(E_IMMUNITY_TYPE_KEY,stringa,static_cast<gates::e_immunity_type>(E_IMMUNITY_TYPE_MIN));
    }
};

template <>
struct Stack <gates::e_large_flags> {
    static void push (lua_State* L, gates::e_large_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_large_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_large_flags>(E_LARGE_FLAGS_KEY,stringa,static_cast<gates::e_large_flags>(E_LARGE_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_mob_flags> {
    static void push (lua_State* L, gates::e_mob_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_mob_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_mob_flags>(E_MOB_FLAGS_KEY,stringa,static_cast<gates::e_mob_flags>(E_MOB_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_opinion_op> {
    static void push (lua_State* L, gates::e_opinion_op s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_opinion_op get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_opinion_op>(E_OPINION_OP_KEY,stringa,static_cast<gates::e_opinion_op>(E_OPINION_OP_MIN));
    }
};

template <>
struct Stack <gates::e_player_flags> {
    static void push (lua_State* L, gates::e_player_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_player_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_player_flags>(E_PLAYER_FLAGS_KEY,stringa,static_cast<gates::e_player_flags>(E_PLAYER_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_positions> {
    static void push (lua_State* L, gates::e_positions s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_positions get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_positions>(E_POSITIONS_KEY,stringa,static_cast<gates::e_positions>(E_POSITIONS_MIN));
    }
};

template <>
struct Stack <gates::e_room_flags> {
    static void push (lua_State* L, gates::e_room_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_room_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_room_flags>(E_ROOM_FLAGS_KEY,stringa,static_cast<gates::e_room_flags>(E_ROOM_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_seasons> {
    static void push (lua_State* L, gates::e_seasons s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_seasons get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_seasons>(E_SEASONS_KEY,stringa,static_cast<gates::e_seasons>(E_SEASONS_MIN));
    }
};

template <>
struct Stack <gates::e_sector_types> {
    static void push (lua_State* L, gates::e_sector_types s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_sector_types get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_sector_types>(E_SECTOR_TYPES_KEY,stringa,static_cast<gates::e_sector_types>(E_SECTOR_TYPES_MIN));
    }
};

template <>
struct Stack <gates::e_sex> {
    static void push (lua_State* L, gates::e_sex s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_sex get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_sex>(E_SEX_KEY,stringa,static_cast<gates::e_sex>(E_SEX_MIN));
    }
};

template <>
struct Stack <gates::e_skill_known> {
    static void push (lua_State* L, gates::e_skill_known s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_skill_known get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_skill_known>(E_SKILL_KNOWN_KEY,stringa,static_cast<gates::e_skill_known>(E_SKILL_KNOWN_MIN));
    }
};

template <>
struct Stack <gates::e_sky> {
    static void push (lua_State* L, gates::e_sky s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_sky get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_sky>(E_SKY_KEY,stringa,static_cast<gates::e_sky>(E_SKY_MIN));
    }
};

template <>
struct Stack <gates::e_speaks> {
    static void push (lua_State* L, gates::e_speaks s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_speaks get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_speaks>(E_SPEAKS_KEY,stringa,static_cast<gates::e_speaks>(E_SPEAKS_MIN));
    }
};

template <>
struct Stack <gates::e_special_events> {
    static void push (lua_State* L, gates::e_special_events s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_special_events get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_special_events>(E_SPECIAL_EVENTS_KEY,stringa,static_cast<gates::e_special_events>(E_SPECIAL_EVENTS_MIN));
    }
};

template <>
struct Stack <gates::e_sunlight> {
    static void push (lua_State* L, gates::e_sunlight s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_sunlight get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_sunlight>(E_SUNLIGHT_KEY,stringa,static_cast<gates::e_sunlight>(E_SUNLIGHT_MIN));
    }
};

template <>
struct Stack <gates::e_sys_flags> {
    static void push (lua_State* L, gates::e_sys_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_sys_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_sys_flags>(E_SYS_FLAGS_KEY,stringa,static_cast<gates::e_sys_flags>(E_SYS_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_teleports> {
    static void push (lua_State* L, gates::e_teleports s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_teleports get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_teleports>(E_TELEPORTS_KEY,stringa,static_cast<gates::e_teleports>(E_TELEPORTS_MIN));
    }
};

template <>
struct Stack <gates::e_user_flags> {
    static void push (lua_State* L, gates::e_user_flags s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_user_flags get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_user_flags>(E_USER_FLAGS_KEY,stringa,static_cast<gates::e_user_flags>(E_USER_FLAGS_MIN));
    }
};

template <>
struct Stack <gates::e_wear_as> {
    static void push (lua_State* L, gates::e_wear_as s) {
        lua_pushstring (L, t(s).c_str());
    }
    static gates::e_wear_as get (lua_State* L, int index) {
        string stringa(luaL_checkstring(L,index));
        return gates::globals()->get<gates::e_wear_as>(E_WEAR_AS_KEY,stringa,static_cast<gates::e_wear_as>(E_WEAR_AS_MIN));
    }
};

}
*/
#endif /* __AUTOENUMS_HPP_ */

