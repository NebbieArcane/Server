/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __STRUCTS
#define __STRUCTS
/***************************  System  include ************************************/
/***************************  Local    include ************************************/

#include <string>
#include "typedefs.hpp"
#include "odb/account.hpp"
#include "odb/odb.hpp"
#include "specialproc_other.hpp"
#include "specialproc_room.hpp"
namespace Alarmud {

/*
Flyp new define
*/

#define MAX_XP 2147483647

/*
 *  my new stuff
 */

#define ALL_DARK        1
#define FOREST_DARK     2
#define NO_DARK         3

typedef struct alias_type {
	char* com[10]; /* 10 aliases */
} Alias;

/*
 * Quest stuff
 */


struct QuestItem {
	int item;
	const char* where;
};

/*
 * Achievements
 */
struct ClassAchieTable
{
    int achie_number;
    const char* achie_string1;
    const char* achie_string2;
    const char* lvl1;
    int lvl1_val;
    const char* lvl2;
    int lvl2_val;
    const char* lvl3;
    int lvl3_val;
    const char* lvl4;
    int lvl4_val;
    const char* lvl5;
    int lvl5_val;
    const char* lvl6;
    int lvl6_val;
    const char* lvl7;
    int lvl7_val;
    const char* lvl8;
    int lvl8_val;
    const char* lvl9;
    int lvl9_val;
    const char* lvl10;
    int lvl10_val;
    int classe;
    int grado_diff;
    int n_livelli;
};

struct RandomEquipTable
{
    const char* name;
    int gender;
    const char* key;
};

struct RandomMaterialsTable
{
    const char* fem_gen;
    const char* neu_gen;
    const char* m_fem_gen;
    const char* m_neu_gen;
    const char* key;
};

struct ObjAchieTable
{
    int vnum_obj;
    int item_type;
    int bonus1;
    int bonus2;
    int bonus3;
    int bonus4;
    int bonus5;
    int perc1;
    int perc2;
    int perc3;
    int perc4;
    int perc5;
};

struct ObjBonusTable
{
    int lev1_10;
    int lev11_20;
    int lev21_30;
    int lev31_40;
    int lev41_50;
};

struct MobQuestAchie
{
    int numero_mob;
    int mob_0;
    int mob_1;
    int mob_2;
    int mob_3;
    int mob_4;
    int mob_5;
    int mob_6;
    int mob_7;
    int mob_8;
    int mob_9;
};

struct XpAchieTable
{
    int lev_1_xp;
    int lev_2_xp;
    int lev_3_xp;
    int lev_4_xp;
    int lev_5_xp;
    int lev_6_xp;
    int lev_7_xp;
    int lev_8_xp;
    int lev_9_xp;
    int lev_10_xp;
};
    
#define MIN_GLOB_TRACK_LEV 31   /* mininum level for global track */
/*
**  Site locking stuff.. written by Scot Gardner
*/
#define MAX_BAN_HOSTS 15

/*
**  Newbie authorization stuff
*/

#define NEWBIE_REQUEST 1
#define NEWBIE_START   100
#define NEWBIE_AXE     0
#define NEWBIE_CHANCES 3

/*
** Log messages stuff
*/



/*
**  Limited item Stuff
*/

#define LIM_ITEM_COST_MIN 20000     /* mininum cost of a lim. item
                                     *  makes it a limited item... */

#define MAX_LIM_ITEMS 18            /* max number of limited items you can
                                     * rent with */

#define MIN_COST_ITEM_EGO 10000     /* minimo costo sotto il quale non viene
                                     * effettuato il check dell'ego. */


/*
**  distributed monster stuff
*/



/*
**  multiclassing stuff
*/


#define TUTTI         0 /* per facilitare la lettura dell' interpreter.c */

/*Nuova Gerarchia dal 50 a Principe by Ryltar*/
#define CONTE	        200000000
#define MARCHESE	250000000
#define DUCA		300000000
#define GRANDUCA	350000000
/*Vecchie gerarchie per i player Nebbie Arcane*/
#define NEWBIE        1
#define CHUMP        11
#define MEDIUM       21
#define EXPERT       31
#define MASTER       41
#define ADEPT        46
#define LORD         50
/*Nuove gerarchie per i player Nebbie Arcane*/
/*Modificate da Ryltar*/
#define NOVIZIO       1
#define ALLIEVO  11
#define APPRENDISTA     21
#define INIZIATO       31
#define VASSALLO     31
#define ESPERTO      41
#define MAESTRO     46
#define BARONE      50
#define PRINCIPE     51
/*Gerarchie di Nebbie Arcane*/
#define IMMORTALE    			 52
#define DIO_MINORE     			 53
#define DIO         			 54
#define MAESTRO_DEGLI_DEI                55
#define CREATORE     			 56
#define QUESTMASTER  			 57
#define MAESTRO_DEL_CREATO               58
#define MAESTRO_DEI_CREATORI             59
#define IMMENSO      			 60
/*Vecchie gerarchie di SD, da rimpiazzare */
#define MAX_MORT     51
#define LOW_IMMORTAL 52
#define IMMORTAL     52
#define CREATOR      53
#define SAINT        53
#define DEMIGOD      54
#define LESSER_GOD   55
#define GOD          56
#define GREATER_GOD  57
#define SILLYLORD    58
#define IMPLEMENTOR  59
#define BIG_GUY      60
#define MAX_IMMORT   60

/* Special previlagies for Immos/Morts ch->permissions */

#define PREV_AREA_MAKER 1
#define PREV_NONE       2
#define PREV_NONE1      4



#define MAX_ROOMS   5000

struct nodes {
	int visited;
	int ancestor;
};

struct room_q {
	int room_nr;
	struct room_q* next_q;
};

struct string_block {
	int size;
	char* data;
};


/*
 *  memory stuff
 */

struct char_list {
	struct char_data* op_ch;
	char name[50];
	struct char_list* next;
};

typedef struct {
	struct char_list*  clist;
	int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
	int    race;  /*number */
	int    iClass; /* 1=m,2=c,4=f,8=t */
	int    vnum;  /* # */
	int    evil;  /* align < evil = attack */
	int    good;  /* align > good = attack */
} Opinion;




/*
 *  old stuff.
 */
#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */

#define PULSE_PER_SEC  4
#define PULSE_PER_MIN  (60 * PULSE_PER_SEC)
#define PULSE_MAXUSAGE		 10
#define PULSE_ZONE          (60 * PULSE_PER_SEC)
#define PULSE_MOBILE        ( 9 * PULSE_PER_SEC )
#define PULSE_VIOLENCE      ( 4 * PULSE_PER_SEC  )
#define PULSE_RIVER         ( 4 * PULSE_PER_SEC )
#define PULSE_TELEPORT      ( 3 * PULSE_PER_SEC )
#define PULSE_TICK          (75 * PULSE_PER_SEC)
#define PULSE_EQ            ( 5 * PULSE_PER_MIN)
#define TICK_WRAP_COUNT 3   /*  PULSE_MOBILE / PULSE_TELEPORT */
#define PLR_TICK_WRAP   24  /*  this should be a divisor of 24 (hours) */

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

/* Lowered to 65 from 75 as a test */
#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)
/*
 * Note:  This stuff is all code dependent,
 * Don't change it unless you know what you
 * are doing.  comm.c and mobact.c hold the
 * stuff that you will HAVE to rewrite if you
 * change either of those constants.
 */

#define WAIT_SEC       4
#define WAIT_ROUND     PULSE_VIOLENCE


/* We were getting purify hits on MAX_STRING_LENGTH */
#define MAX_STRING_LENGTH   20480
#define MAX_INPUT_LENGTH     160

#define SMALL_BUFSIZE        512
#define LARGE_BUFSIZE        MAX_STRING_LENGTH*2

#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3


/* The following defs are for obj_data  */

/* For 'type_flag' */









/* special addition for drinks */
#define DRINK_POISON  (1<<0)
#define DRINK_PERM    (1<<1)


/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

#define SLAYER_GOOD    1
#define SLAYER_NEUTRAL 2
#define SLAYER_EVIL    4


#define EXDESC_VALID_MAGIC 1235
#define EXDESC_FREED_MAGIC 1236
struct extra_descr_data {
	int nMagicNumber;
	char* keyword;                 /* Keyword in look/examine          */
	char* description;             /* What to see                      */
	struct extra_descr_data* next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 5         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data {
	int value[4];       /* Values of the item (see list)    */
	ubyte type_flag;     /* Type of item                     */
	unsigned int wear_flags;     /* Where you can wear it            */
	unsigned int extra_flags;    /* If it hums,glows etc             */
    unsigned int extra_flags2;  /* nuovi flags quest, edit, etc     */
	int weight;         /* Weigt what else                  */
	int cost;           /* Value when sold (gp.)            */
	int cost_per_day;   /* Cost to keep pr. real day        */
	int timer;          /* Timer for object                 */
	unsigned int bitvector;     /* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
	short location;      /* Which ability to change (APPLY_XXX) */
	int modifier;      /* How much it changes by      */
};

/* ======================== Structure for object ========================*/
struct obj_data {
	sh_int item_number;             /* Where in data-base             */
	int in_room;                    /* In what room -1 when conta/carr  */
	struct obj_flag_data obj_flags; /* Object information             */
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
	/* Which abilities in PC to change*/

	sh_int sector;                 /* for large rooms      */
	int char_vnum;                /* for resurrection     */
	char oldfilename[ 20 ];
	char* name;                    /* Title of object :get etc.      */
	char* description ;            /* When in room                   */
	char* short_description;       /* when worn/carry/in cont.       */
	char* action_description;      /* What to write when used        */
	struct extra_descr_data* ex_description; /* extra descriptions   */
	struct char_data* carried_by;  /* Carried by :NULL in room/conta */
	byte   eq_pos;                 /* what is the equip. pos?        */
	struct char_data* equipped_by; /* equipped by :NULL in room/conta*/
	struct obj_data* in_obj;       /* In what object NULL when none  */
	struct obj_data* contains;     /* Contains objects               */
	struct obj_data* next_content; /* For 'contains' lists           */
	struct obj_data* next;         /* For the object list            */
	char* szForbiddenWearToChar;   /* messaggi da visualizzare quando  */
	char* szForbiddenWearToRoom;   /* si tenta di indossare un oggetto */
	/* proibito.                        */
	int iGeneric;                  /* Valore generico a disposizione delle
                                  * procedure speciali */
	int iGeneric1;                  /* Valore generico a disposizione delle
                                  * procedure speciali */
	int iGeneric2;                  /* Valore generico a disposizione delle
                                  * procedure speciali */
	void* pGeneric;                /* Puntatore generico a disposizione delle
                                  * procedure speciali */
	void* pGeneric1;                /* Puntatore generico a disposizione delle
                                  * procedure speciali */
	void* pGeneric2;                /* Puntatore generico a disposizione delle
                                  * procedure speciali */
};
/* ======================================================================*/

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database      */
#define AUTO_RENT  -2    /* other special room, for auto-renting */

/* Bitvector For 'room_flags' */
struct large_room_data {
	long flags[9];
};

struct room_direction_data {
	char* general_description;    /* When look DIR.                  */
	char* keyword;                /* for open/close                  */

	long exit_info;             /* Exit info                       */
	long key;                      /* Key's number (-1 for no key)    */
	long to_room;                  /* Where direction leeds (NOWHERE) */
	long open_cmd;                      /* cmd needed to OPEN/CLOSE door   */

};

/* ========================= Structure for room ========================== */
struct room_data {
	/* sh_int */
	long number;               /* Rooms number                       */
	long zone;                 /* Room zone (for resetting)          */
	long continent;            /* Which continent/mega-zone          */
	long sector_type;             /* sector type (move/hide)            */

	byte blood;					/*Per il sangue nelle stanze		*/
	byte dig;					/*Per la skill miner		*/

	int river_dir;               /* dir of flow on river               */
	int river_speed;             /* speed of flow on river             */

	int  tele_time;              /* time to a teleport                 */
	int  tele_targ;              /* target room of a teleport          */
	char tele_mask;              /* flags for use with teleport        */
	int  tele_cnt;               /* countdown teleports                */

	unsigned char moblim;        /* # of mobs allowed in room.         */

	char* name;                  /* Rooms name 'You are ...'           */
	char* description;           /* Shown when entered                 */
	struct extra_descr_data* ex_description; /* for examine/look       */
	struct room_direction_data* dir_option[6]; /* Directions           */
	long room_flags;             /* DEATH,DARK ... etc                 */
	byte light;                  /* Number of lightsources in room     */
	ubyte dark;
	roomspecial_func funct;
	/* special procedure                  */
	const char* specname;
	char* specparms;
	struct obj_data* contents;   /* List of items in room              */
	struct char_data* people;    /* List of NPC / PC in room           */
	struct char_data* listeners;  /* per eavesdrop						 */

	struct large_room_data* large;  /* special for large rooms         */
	/* Queste due stringhe vengono visualizzate quando la locazione e` di tipo
	 * BRIGHT. La prima quando e` notte e il BRIGHT entra in funzione, la
	 * seconda quando e` giorno ed il BRIGHT e` disattivo.
	 * Sono utili per strade illuminate dai lampioni ad esempio.
	 * */
	char* szWhenBrightAtNight;
	char* szWhenBrightAtDay;

};
/* ======================================================================== */


/* For 'char_player_data' */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
	byte hours, day, month;
	sh_int year;
	sh_int ayear;
};

/* Data used in the display code checks -DM */
struct last_checked {
	int mana;
	int mmana;
	int hit;
	int mhit;
	int move;
	int mmove;
	int exp;
	int gold;
};

/* These data contain information about a players time data */
struct time_data {
	time_t birth;    /* This represents the characters age                */
	time_t logon;    /* Time of the last logon (used to calculate played) */
	int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data {

	ubyte sex;           /* PC / NPC s sex                       */
	short weight;       /* PC / NPC s weight                    */
	short height;       /* PC / NPC s height                    */
	bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC/not used for languagesn */
	long user_flags;        /* no delete, ansi etc... */
	int speaks;                /* current language speaking */

	char* name;                /* PC / NPC s name (kill ...  )         */
	char* vassallodi; /* questo pc e' vassallo di */
	char* short_descr;  /* for 'actions'                        */
	char* long_descr;   /* for 'look'.. Only here for testing   */
	char* description;  /* Extra descriptions                   */
	char* title;        /* PC / NPC s title                     */
	char* sounds;       /* Sound that the monster makes (in room) */
	char* distant_snds; /* Sound that the monster makes (other) */
    
	int iClass;         /* PC s class or NPC alignment          */
	int hometown;       /* PC s Hometown (zone)                 */

	long extra_flags;   /* for ressurection in the future , etc */

	struct time_data time; /* PC s AGE in days                  */

	ubyte level[ABS_MAX_CLASS];      /* NEEDS TO BE ABS_MAX_CLASS */
	/* PC / NPC s level         */

};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data {
	sbyte str;
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis;
	sbyte dex;
	sbyte con;
	sbyte chr;
	sbyte extra;
	sbyte extra2;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
	sh_int mana;
	sh_int max_mana;
	ubyte  mana_gain;

	sh_int hit;
	sh_int max_hit;      /* Max hit for NPC                         */
	ubyte  hit_gain;

	sh_int move;
	sh_int max_move;     /* Max move for NPC                        */
	ubyte  move_gain;
	ush_int pRuneDei;  /* SALVO extra1 lo uso per le rune da 0 a 65535*/

	sh_int extra1;  /* extra stuff */
	sh_int extra2;
	ubyte  extra3;

	sh_int armor;        /* Internal -100..100, external -10..10 AC */
	int gold;            /* Money carried                           */
	int bankgold;        /* gold in the bank.                       */
	int exp;             /* The experience of the player            */
	int true_exp;        /* gonna be used for dual class            */
	int extra_dual;      /* case I need an extra duall class bit    */


	sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
	sbyte damroll;       /* Any bonus or penalty to the damage roll */

	sbyte libero;       /* SALVO ex pQuest torna libero*/
};


struct char_special_data {
	short spellfail;        /* max # for spell failure (101) */
	ubyte tick;             /* the tick that the mob/player is on  */
	ubyte pmask;            /* poof mask                           */
	ubyte position;         /* Standing or ...                        */
	ubyte default_pos;      /* Default position for NPC              */
	byte spells_to_learn;  /* How many can you learn yet this level*/
	ubyte carry_items;      /* Number of items carried              */
	char last_direction;   /* The last direction the monster went */
	unsigned sev;          /* log severety level for gods */

	int start_room;  /* so people can be set to start certain places */
	int edit;                /* edit state */

	sbyte mobtype;                        /* mob type simple, A, L, B */
	unsigned int exp_flag;                 /* exp flag for this mob */
	sbyte hp_num_dice;                        /* number of HPS dice */
	unsigned int hp_size_dice;                /* size of HPS dice */
	unsigned int hp_bonus_hps;                /* bonus hps number */

	ubyte damnodice;           /* The number of damage dice's         */
	ubyte damsizedice;         /* The size of the damage dice's       */

	unsigned int dam_bonus;                        /* damage bonus */
	ubyte medit;                           /*  mob edit menu at */
	struct char_data* mobedit;            /*  mob editing */

	ubyte oedit;                            /*  obj editing menu at */
	struct obj_data* objedit;             /*  object editing */

	int tick_to_lag;

	sbyte conditions[MAX_CONDITIONS];      /* Drunk full etc.        */
	int permissions;
	int zone;   /* zone that an NPC lives in */
	int carry_weight;        /* Carried weight                       */
	int timer;               /* Timer for update                     */
	int was_in_room;         /* storage of location for linkdead people */
	int attack_type;         /* The Attack Type Bitvector for NPC's */
	int alignment;           /* +-1000 for alignments               */

    /* ACHIEVEMENTS */
    int achievements[MAX_ACHIE_CLASSES][MAX_ACHIE_TYPE];
    int quest_mob[MAX_QUEST_ACHIE][MAX_MOB_QUEST];

	char* poofin;
	char* poofout;
	char* prompt;
	char* lastversion;        /* Ultima versione conosciuta al player GGPATCH*/
	char* email;              /* email address in aux */
	char* realname;           /*real name, by GGPATCH*/
	char* authcode;           /*Codice di autorizazione GGPATCH*/
	char* AuthorizedBy;       /*Nome dell'immortale che ha autorizzato
			    * per l'identificazione dei players GGPATCH*/
	char* supporting;   /* guy we'd like to heal*/
	char* bodyguarding; /* guy we'd like to rescue*/
	char* bodyguard; /* our body guard */
	long TempoPassatoIn[MAX_POSITION];
	short PosPrev;
	short WimpyLevel;
	/* tiene traccia del tempo passato nelle singole posizioni */
	char* Sextra;
	char* group_name; /* current group name if any... */

	Alias*   A_list;
	struct char_data* misc;
	struct char_data* fighting; /* Opponent                          */
    struct char_data* quest_ref; /*  For NPCs it stores the quest owner, For PCs it can be used to store his questor/target */
    float eq_val_idx; /* Eq Value Index: can be used to store the eq value in a moment and compare it later */

	struct char_data* hunting;  /* Hunting person..                  */

	struct char_data* ridden_by;
	struct char_data* mounted_on;

	struct char_data* charging;        /* we are charging this person */
	int  charge_dir;                /* direction charging */

	unsigned long affected_by;  /* Bitvector for spells/skills affected by */
	unsigned long affected_by2; /* Other special things */

	unsigned long act; /* flags for NPC behavior */

	sh_int apply_saving_throw[MAX_SAVES]; /* Saving throw (Bonuses)  */

};

/* skill_data special */
#define SKILL_SPECIALIZED         1
#define SKILL_UNDEFINED           2
#define SKILL_UNDEFINED2          4

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data {
	ubyte learned;           /* % chance for success 0 = not learned   */

	/* change to int or long */
	byte flags;             /* SKILL KNOWN? bit settings               */
	byte special;           /* spell/skill specializations             */
	byte nummem;            /* number of times this spell is memorized */
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
	short type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	int modifier;       /* This is added to apropriate ability     */
	int location;        /* Tells which ability to change(APPLY_XXX)*/
	int bitvector;       /* Tells which bits to set (AFF_XXX)       */
	struct affected_type* next;
};
struct affected_type_u { // To be only used in file definition,
	short type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	int modifier;       /* This is added to apropriate ability     */
	int location;        /* Tells which ability to change(APPLY_XXX)*/
	int bitvector;       /* Tells which bits to set (AFF_XXX)       */
	int next;
};

struct follow_type {
	struct char_data* follower;
	struct follow_type* next;
};

/* ================== Structure for player/non-player ===================== */
#define CHAR_VALID_MAGIC 1234
#define CHAR_FREEDED_MAGIC 2345
struct char_data {
	int nMagicNumber;
	int nr;                         /* monster nr */
	int in_room;                    /* Location                    */

	int term;
	int size;
	struct last_checked last;        /* For displays                */
	unsigned immune;                 /* Immunities                  */
	unsigned M_immune;               /* Meta Immunities             */
	unsigned susc;                   /* susceptibilities            */
	float mult_att;                  /* the number of attacks      */
	byte attackers;
	byte sector;                     /* which part of a large room am i in?  */
	int generic;                     /* generic int */
	int commandp;                    /* command poitner for scripts */
	int waitp;                       /* waitp for scripts           */
	int commandp2;                   /* place-holder for gosubs, etc. */
	int script;

	sh_int race;
	sh_int hunt_dist;                /* max dist the player can hunt */

	unsigned short hatefield;
	unsigned short fearfield;

	Opinion hates;
	Opinion fears;

	sh_int  persist;
	int     old_room;

	void* act_ptr;    /* numeric argument for the mobile actions */

	struct char_player_data player;       /* Normal data            */
	struct char_ability_data abilities;   /* Abilities              */
	struct char_ability_data tmpabilities;/* The abilities we use  */
	struct affected_type* affected;       /* affected by what spells */
	struct char_point_data points;        /* Points                 */
	struct char_special_data specials;    /* Special plaing constant */
	struct char_data* next_listener;       /* Prossimo che fa eavesdrop */
	int listening_to;		    /* Stanza per eavesdrop, modificato da sh_int a int */

	/* monitor these three */
	struct char_skill_data* skills;       /* Skills                */
	struct obj_data* equipment[MAX_WEAR]; /* Equipment array         */
	struct obj_data* carrying;            /* Head of list            */
	/* end monitor */

	struct descriptor_data* desc;         /* NULL for mobiles        */
	struct char_data* orig;               /* Special for polymorph   */

	struct char_data* next_in_room;     /* For room->people - list   */
	struct char_data* next;             /* all in game list  */
	struct char_data* next_fighting;    /* For fighting list         */

	struct follow_type* followers;        /* List of chars followers */
	struct char_data* master;             /* Who is char following?  */
	int invis_level;                      /* visibility of gods */
	int nTeleCount;                     /* Contatore per i teleport di tipo
                                       * TELE_COUNT */
	long lStartRoom;
	long AgeModifier;
	struct event* points_event[3];   /* events for regening H/M/V */
	
    char* lastpkill; // last player killed, used also for destroy checks
    char* lastmkill; // last mob killed, used also for quest checks

};


/* ======================================================================== */

struct weather_data {
	int pressure;        /* How is the pressure ( Mb ) */
	int change;        /* How fast and what way does it change. */
	e_sky sky;        /* How is the sky. */
	e_sunlight sunlight;        /* And how much sun. */
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u {
	int iClass;
	ubyte sex;
	ubyte level[ABS_MAX_CLASS];
	unsigned int birth;  /* Time of birth of character     */
	int played;    /* Number of secs played in total */
	int   race;
	unsigned int weight;
	unsigned int height;
	char title[80];
	char extra_str[255];
	sh_int hometown;
	char description[240];
	bool talks[MAX_TOUNGE];
	int extra_flags;
	sh_int load_room;            /* Which room to place char in  */
	struct char_ability_data abilities; // No pointers inside, same size on 32 and 64 bit
	struct char_point_data points; // No pointers inside, same size on 32 and 64 bit
	struct char_skill_data skills[MAX_SKILLS]; // No pointers inside, same size on 32 and 64 bit
	struct affected_type_u affected[MAX_AFFECT];
	/* specials */
	byte spells_to_learn;
	int alignment;
	unsigned int affected_by;
	unsigned int affected_by2;
	unsigned int last_logon;  /* Time (in secs) of last logon */
	unsigned int    act;        /* ACT Flags                    */

	/* char data */
	char name[20];
	char authcode[7];  /* codice di autorizzazione */
	char WimpyLevel[4]; /* Wimpy level */
	char dummy[19];      /* per usi futuri */
	char pwd[11];
	sh_int apply_saving_throw[MAX_SAVES];
	int conditions[MAX_CONDITIONS];
	int startroom;  /* which room the player should start in */
	int user_flags;        /* no-delete,use ansi,etc... */
	int speaks;                /* language currently speakin in */
	int agemod;
};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct obj_cost {
	/* used in act.other.c:do_save as well as in reception2.c */
	unsigned int total_cost;
	unsigned int no_carried;
	bool ok;
};

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct obj_file_elem {
	/* Bug, rendeva impossibile rentare oggetti oltre il 32565 */
	/*sh_int item_number;*/
	ush_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	unsigned int bitvector;
	char name[128];  /* big, but not horrendously so */
	char sd[128];
	char desc[256];
	ubyte wearpos;
	ubyte depth;
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
    int extra_flags2;
};

struct old_obj_file_elem {
    /* Bug, rendeva impossibile rentare oggetti oltre il 32565 */
    /*sh_int item_number;*/
    ush_int item_number;

    int value[4];
    int extra_flags;
    int weight;
    int timer;
    unsigned int bitvector;
    char name[128];  /* big, but not horrendously so */
    char sd[128];
    char desc[256];
    ubyte wearpos;
    ubyte depth;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u {
	char owner[20];    /* Name of player                     */
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	int last_update;  /* Time in seconds, when last updated */
	int minimum_stay; /* For stasis */
	int  number;       /* number of objects */
	struct obj_file_elem objects[MAX_OBJ_SAVE];
};

struct old_obj_file_u {
    char owner[20];    /* Name of player                     */
    int gold_left;     /* Number of goldcoins left at owner  */
    int total_cost;    /* The cost for all items, per day    */
    int last_update;  /* Time in seconds, when last updated */
    int minimum_stay; /* For stasis */
    int  number;       /* number of objects */
    struct old_obj_file_elem objects[MAX_OBJ_SAVE];
};

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */

struct txt_block {
	char* text;
	struct txt_block* next;
};

struct txt_q {
	struct txt_block* head;
	struct txt_block* tail;
};




struct snoop_data {
	struct char_data* snooping; /* Who is this char snooping */
	struct char_data* snoop_by; /* And who is snooping on this char */
};

class descriptor_data {
public:
	bool justCreated=false;
	bool impersonating=false;
	bool AlreadyInGame = false;                  /* flag di presenza */
	user AccountData;
	std::vector<std::string> toons;
	e_connection_types last_state;
	string currentInput;
	int descriptor;                    /* file descriptor for socket */

	char* name;                /* ptr to name for mail system */

	char host[50];                /* hostname                   */
	char pwd[12];                 /* password                   */
	int pos;                      /* position in player-file    */
	e_connection_types connected;                /* mode of 'connectedness'    */
	int wait;                     /* wait for how many loops    */
	char* showstr_head;              /* for paging through texts   */
	const char* showstr_point;              /*       -                    */
	char** str;                   /* for the modify-str system  */
	unsigned int max_str;                  /* -                          */
	int prompt_mode;              /* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];/* the last input         */

	char stat[MAX_STAT];         /* stat priorities            */

	/* for the new write_to_out */
	char small_outbuf[SMALL_BUFSIZE];
	int bufptr;
	unsigned int bufspace;
	struct txt_block* large_outbuf;
#ifdef BLOCK_WRITE
	char* output;
#else
	struct txt_q output;          /* q of strings to send       */
#endif
	struct txt_q input;           /* q of unprocessed input     */
	struct char_data* character;  /* linked to char             */
	struct char_data* original;   /* original char              */
	struct snoop_data snoop;      /* to snoop people.           */
	struct descriptor_data* next; /* link to next descriptor    */
	/* Questi campi mi servono per tenere traccia della presenza del player
	 * in eventuale ld non riconosciuto, fra l'accettazione del nome e quella
	 * della password
	 */
	struct descriptor_data* ToBeKilled;  /* descrittore gia' in gioco,
					 * da killare */
	/*campo per decifrare il tipo di rollata
	 */
#if defined (NEW_ROLL)
	char TipoRoll; /* (V)ecchia, (S)emplice, (N)uova, (R)andomizzata */
#endif
};

struct msg_type {
	char* attacker_msg;  /* message to attacker */
	char* victim_msg;    /* message to victim   */
	char* room_msg;      /* message to room     */
};

struct message_type {
	struct msg_type die_msg;      /* messages when death            */
	struct msg_type miss_msg;     /* messages when miss             */
	struct msg_type hit_msg;      /* messages when hit              */
	struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
	struct msg_type god_msg;      /* messages when hit on god       */
	struct message_type* next;/* to next messages of this kind.*/
};

struct message_list {
	int a_type;               /* Attack type                             */
	int number_of_attacks;    /* How many attack messages to chose from. */
	struct message_type* msg; /* List of messages.                       */
};

struct dex_skill_type {
	sh_int p_pocket;
	sh_int p_locks;
	sh_int traps;
	sh_int sneak;
	sh_int hide;
};

struct dex_app_type {
	sh_int reaction;
	sh_int miss_att;
	sh_int defensive;
};

struct str_app_type {
	sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	sh_int todam;    /* Damage Bonus/Penalty                */
	sh_int carry_w;  /* Maximum weight that can be carrried */
	sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type {
	ubyte bonus;       /* how many bonus skills a player can */
	/* practice pr. level                 */
};

struct int_app_type {
	byte learn;       /* how many % a player learns a spell/skill */
	sh_int memorize;  /* quanti incantesimi puo` imparare un giocatore */
	/* contemporaneamente. */
};

struct con_app_type {
	sh_int hitp;
	sh_int shock;
};

struct chr_app_type {
	sh_int num_fol;
	sh_int reaction;
};
struct wiznest {
	char* name;
	char* title;
};

struct wiznode {
	struct wiznest stuff[150];
};

struct wizlistgen {
	int number[61];
	struct wiznode lookup[61];
};

} // namespace Alarmud
#endif

