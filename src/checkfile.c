/*$Id: checkfile.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>

typedef char sbyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef unsigned short int ush_int;
typedef unsigned char ubyte;
typedef char byte;

struct char_list
{
  struct char_data *op_ch;
  char name[50];
  struct char_list *next;
};

typedef struct 
{
  struct char_list  *clist;
  int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
  int    race;  /*number */
  int    iClass; /* 1=m,2=c,4=f,8=t */
  int    vnum;  /* # */
  int    evil;  /* align < evil = attack */
  int    good;  /* align > good = attack */
} Opinion;

typedef struct alias_type 
{
  char *com[10]; /* 10 aliases */
} Alias;

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)
#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

#define MAX_POSITION        9
#define MAX_CONDITIONS 5  /* USER FILE, DO NOT CHANGE */
#define MAX_SAVES   8     /* number of saving throws types... DO NOT CHANGE*/
#define ABS_MAX_CLASS 20          /* USER FILE, DO NOT CHANGE! */
#define MAX_TOUNGE  3         /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR_POS   22
#define MAX_WEAR    (MAX_WEAR_POS+1)
#define MAX_CLASS 11

#define NOWHERE    -1    /* nil reference for room-database      */
#define APPLY_NONE              0
#define EXDESC_VALID_MAGIC 1235
#define MAX_STRING_LENGTH   20480
#define MAX_OBJ_AFFECT 5         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define REAL 0
#define VIRTUAL 1
#define CREATE(result, type, number)  do {\
        if (!((result) = (type *) calloc ((number), sizeof(type))))\
                { abort(); }\
                } while(0)
#define OBJ_DIR  	"lib/objects"
#define OBJ_FILE 	"lib/myst.obj"
#define MOB_DIR  	"lib/mobiles"
#define MOB_FILE 	"lib/myst.mob"
#define PLAYERS_DIR "lib/players"
#define WORLD_FILE  "lib/myst.wld"     /* room definitions           */
#define ZONE_FILE   "lib/myst.zon"     /* zone definitions           */
#define WORLD_DIR   "lib/world"     /* world room definitions           */
#define RENT_DIR 	"lib/rent"
#define WORLD_SIZE 50000
#define SAVE_ROOM     8192 /* room will save eq and load at reboot */
#define MAX_OBJ_SAVE 200
#define TICK_WRAP_COUNT 3   /*  PULSE_MOBILE / PULSE_TELEPORT */

#define SKILL_HUNT                   180

#define HATE_SEX    1
#define HATE_RACE   2
#define HATE_CHAR   4
#define HATE_CLASS  8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define IMM_FIRE        1
#define IMM_COLD        2
#define IMM_ELEC        4
#define IMM_ENERGY      8
#define IMM_BLUNT      16
#define IMM_PIERCE     32
#define IMM_SLASH      64
#define IMM_ACID      128
#define IMM_POISON    256
#define IMM_DRAIN     512
#define IMM_SLEEP    1024
#define IMM_CHARM    2048
#define IMM_HOLD     4096
#define IMM_NONMAG   8192
#define IMM_PLUS1   16384
#define IMM_PLUS2   32768
#define IMM_PLUS3   65536
#define IMM_PLUS4  131072

#define AFF_NONE              0x00000000
#define AFF_BLIND             0x00000001
#define AFF_INVISIBLE         0x00000002
#define AFF_DETECT_EVIL       0x00000004
#define AFF_DETECT_INVISIBLE  0x00000008
#define AFF_DETECT_MAGIC      0x00000010
#define AFF_SENSE_LIFE        0x00000020
#define AFF_LIFE_PROT         0x00000040
#define AFF_SANCTUARY         0x00000080
#define AFF_DRAGON_RIDE       0x00000100
#define AFF_GROWTH            0x00000200 /* this was the one that was missing*/
#define AFF_CURSE             0x00000400
#define AFF_FLYING            0x00000800
#define AFF_POISON            0x00001000
#define AFF_TREE_TRAVEL       0x00002000
#define AFF_PARALYSIS         0x00004000
#define AFF_INFRAVISION       0x00008000
#define AFF_WATERBREATH       0x00010000
#define AFF_SLEEP             0x00020000
#define AFF_TRAVELLING        0x00040000  /* i.e. can't be stoned */
#define AFF_SNEAK             0x00080000
#define AFF_HIDE              0x00100000
#define AFF_SILENCE           0x00200000
#define AFF_CHARM             0x00400000
#define AFF_FOLLOW            0x00800000
#define AFF_PROTECT_FROM_EVIL 0x01000000  /*  */
#define AFF_TRUE_SIGHT        0x02000000
#define AFF_SCRYING           0x04000000   /* seeing other rooms */
#define AFF_FIRESHIELD        0x08000000
#define AFF_GROUP             0x10000000
#define AFF_TELEPATHY         0x20000000
#define AFF_GLOBE_DARKNESS    0x40000000  /* Added by REQUIEM 2018 */
#define AFF_UNDEF_AFF_1       0x80000000

#define ACT_SPEC       (1<<0)  /* special routine to be called if exist   */
#define ACT_SENTINEL   (1<<1)  /* this mobile not to be moved             */
#define ACT_SCAVENGER  (1<<2)  /* pick up stuff lying around              */
#define ACT_ISNPC      (1<<3)  /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1<<4)  /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1<<5)  /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE  (1<<6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      (1<<7)  /* MOB Will flee when injured, and if      */
#define ACT_ANNOYING   (1<<8)  /* MOB is so utterly irritating that other */
#define ACT_HATEFUL    (1<<9)  /* MOB will attack a PC or NPC matching a  */
#define ACT_AFRAID    (1<<10)  /* MOB is afraid of a certain PC or NPC,   */
#define ACT_IMMORTAL  (1<<11)  /* MOB is a natural event, can't be kiled  */
#define ACT_HUNTING   (1<<12)  /* MOB is hunting someone                  */
#define ACT_DEADLY    (1<<13)  /* MOB has deadly poison                   */
#define ACT_POLYSELF  (1<<14)  /* MOB is a polymorphed person             */
#define ACT_META_AGG  (1<<15)  /* MOB is _very_ aggressive                */
#define ACT_GUARDIAN  (1<<16)  /* MOB will guard master                   */
#define ACT_ILLUSION  (1<<17)  /* MOB is illusionary                      */
#define ACT_HUGE      (1<<18)  /* MOB is too large to go indoors          */
#define ACT_SCRIPT    (1<<19)  /* MOB has a script assigned to it DO NOT SET */
#define ACT_GREET     (1<<20)  /* MOB greets people */
#define ACT_MAGIC_USER  (1<<21)
#define ACT_WARRIOR     (1<<22)
#define ACT_CLERIC      (1<<23)
#define ACT_THIEF       (1<<24)
#define ACT_DRUID       (1<<25)
#define ACT_MONK        (1<<26)
#define ACT_BARBARIAN   (1<<27)
#define ACT_PALADIN     (1<<28)
#define ACT_RANGER      (1<<29)
#define ACT_PSI         (1<<30)
#define ACT_ARCHER      (1<<31)
		
#define CLASS_MAGIC_USER   1
#define CLASS_CLERIC       2
#define CLASS_WARRIOR      4
#define CLASS_THIEF        8
#define CLASS_DRUID       16
#define CLASS_MONK        32
#define CLASS_BARBARIAN   64
#define CLASS_SORCERER   128
#define CLASS_PALADIN    256
#define CLASS_RANGER     512
#define CLASS_PSI       1024

#define MAGE_LEVEL_IND       0
#define CLERIC_LEVEL_IND     1
#define WARRIOR_LEVEL_IND    2
#define THIEF_LEVEL_IND      3
#define DRUID_LEVEL_IND      4
#define MONK_LEVEL_IND       5
#define BARBARIAN_LEVEL_IND  6
#define SORCERER_LEVEL_IND   7
#define PALADIN_LEVEL_IND    8
#define RANGER_LEVEL_IND     9
#define PSI_LEVEL_IND       10

#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_MOUNTED    9

/* Race -- Npc, otherwise */
#define RACE_HALFBREED 0
#define RACE_HUMAN     1
#define RACE_ELVEN     2
#define RACE_DWARF     3
#define RACE_HALFLING  4
#define RACE_GNOME     5
#define RACE_REPTILE  6
#define RACE_SPECIAL  7
#define RACE_LYCANTH  8
#define RACE_DRAGON   9
#define RACE_UNDEAD   10
#define RACE_ORC      11
#define RACE_INSECT   12
#define RACE_ARACHNID 13
#define RACE_DINOSAUR 14
#define RACE_FISH     15
#define RACE_BIRD     16
#define RACE_GIANT    17        /* generic giant more specials down ---V */
#define RACE_PREDATOR 18
#define RACE_PARASITE 19
#define RACE_SLIME    20
#define RACE_DEMON    21
#define RACE_SNAKE    22
#define RACE_HERBIV   23
#define RACE_TREE     24
#define RACE_VEGGIE   25
#define RACE_ELEMENT  26
#define RACE_PLANAR   27
#define RACE_DEVIL    28
#define RACE_GHOST    29
#define RACE_GOBLIN   30
#define RACE_TROLL    31
#define RACE_VEGMAN   32
#define RACE_MFLAYER  33
#define RACE_PRIMATE  34
#define RACE_ENFAN    35
#define RACE_DROW     36
#define RACE_DARK_ELF 36
#define RACE_GOLEM    37
#define RACE_SKEXIE   38
#define RACE_TROGMAN  39
#define RACE_PATRYN   40
#define RACE_LABRAT   41
#define RACE_SARTAN   42
#define RACE_TYTAN    43
#define RACE_SMURF    44
#define RACE_ROO      45
#define RACE_HORSE    46
#define RACE_DRAAGDIM 47
#define RACE_ASTRAL   48
#define RACE_GOD      49

#define RACE_GIANT_HILL   50
#define RACE_GIANT_FROST  51
#define RACE_GIANT_FIRE   52
#define RACE_GIANT_CLOUD  53
#define RACE_GIANT_STORM  54
#define RACE_GIANT_STONE  55

#define RACE_DRAGON_RED    56
#define RACE_DRAGON_BLACK  57
#define RACE_DRAGON_GREEN  58
#define RACE_DRAGON_WHITE  59
#define RACE_DRAGON_BLUE   60
#define RACE_DRAGON_SILVER 61
#define RACE_DRAGON_GOLD   62
#define RACE_DRAGON_BRONZE 63
#define RACE_DRAGON_COPPER 64
#define RACE_DRAGON_BRASS  65

#define RACE_UNDEAD_VAMPIRE 66
#define RACE_UNDEAD_LICH    67
#define RACE_UNDEAD_WIGHT   68
#define RACE_UNDEAD_GHAST   69
#define RACE_UNDEAD_SPECTRE 70
#define RACE_UNDEAD_ZOMBIE  71
#define RACE_UNDEAD_SKELETON 72
#define RACE_UNDEAD_GHOUL    73

/* a few pc races */
#define RACE_HALF_ELVEN   74
#define RACE_HALF_OGRE    75
#define RACE_HALF_ORC     76
#define RACE_HALF_GIANT   77
/* end pc */

#define RACE_LIZARDMAN 78

/* evil pc's */
#define RACE_DARK_DWARF 79
#define RACE_DEEP_GNOME 80
/* end evil */

#define RACE_GNOLL        81

#define RACE_GOLD_ELF        82
#define RACE_WILD_ELF        83
#define RACE_SEA_ELF        84

#define MAX_RACE      84
#define PRINCIPE     51
#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))
#define IS_PC(ch) (!IS_NPC((ch)) || IS_SET((ch)->specials.act, ACT_POLYSELF))
#define IS_PRINCE(ch) (!IS_NPC(ch)&&(GetMaxLevel(ch)==PRINCIPE))
#define GET_AC(ch)      ((ch)->points.armor)
#define SET_BIT(var,bit)  ((var) = (var) | (bit))
#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit))
#define GET_CLASS(ch)   ((ch)->player.iClass)
#define GET_LEVEL(ch, i)   ((ch)->player.level[(i)])
#define GET_RACE(ch)     ((ch)->race)
#define GET_NAME(ch)    ((ch)->player.name)
#define GET_HIT(ch)     ((ch)->points.hit)
#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)
#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)
#define GET_EXP(ch)     ((ch)->points.exp)
#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])
#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define log(msg) puts(msg)

int top_of_p_table = 0;               /* ref to top of table             */
int top_of_alloc_objt = 0;
int top_of_alloc_mobt = 0;
int top_of_sort_objt = 0;
int top_of_sort_mobt = 0;
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
FILE *obj_f;
FILE *mob_f;
struct index_data *obj_index;         /* index table for object file     */
struct index_data *mob_index;         /* index table for mobile file     */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_mobt = 0;                  /* top of mobile index table       */
long obj_count =0L;
long mob_count =0L;
int mob_tick_count=0;
   
/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
  byte hours, day, month;
  sh_int year;
   sh_int ayear;
};

/* Data used in the display code checks -DM */
struct last_checked 
{
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
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{

  ubyte sex;           /* PC / NPC s sex                       */
  short weight;       /* PC / NPC s weight                    */
  short height;       /* PC / NPC s height                    */
  bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC/not used for languagesn */
  long user_flags;        /* no delete, ansi etc... */
  int speaks;                /* current language speaking */

  char *name;                /* PC / NPC s name (kill ...  )         */
   char *vassallodi; /* questo pc e' vassallo di */
   char *short_descr;  /* for 'actions'                        */
  char *long_descr;   /* for 'look'.. Only here for testing   */
  char *description;  /* Extra descriptions                   */
  char *title;        /* PC / NPC s title                     */
  char *sounds;       /* Sound that the monster makes (in room) */
  char *distant_snds; /* Sound that the monster makes (other) */

  int iClass;         /* PC s class or NPC alignment          */
  int hometown;       /* PC s Hometown (zone)                 */
  
  long extra_flags;   /* for ressurection in the future , etc */
  
  struct time_data time; /* PC s AGE in days                  */
  
  ubyte level[ABS_MAX_CLASS];      /* NEEDS TO BE ABS_MAX_CLASS */ 
                        /* PC / NPC s level         */

};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
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
struct char_point_data
{
  sh_int mana;         
  sh_int max_mana;
  ubyte  mana_gain; 

  sh_int hit;   
  sh_int max_hit;      /* Max hit for NPC                         */
  ubyte  hit_gain; 

  sh_int move;  
  sh_int max_move;     /* Max move for NPC                        */
  ubyte  move_gain;

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
  
  sbyte pQuest;       /* extra utilizzato per i Punti Quest*/
};

struct char_special_data
{
  short spellfail;        /* max # for spell failure (101) */
  ubyte tick;             /* the tick that the mob/player is on  */
  ubyte pmask;            /* poof mask                           */
  ubyte position;         /* Standing or ...                        */
  ubyte default_pos;      /* Default position for NPC              */
  byte spells_to_learn;  /* How many can you learn yet this level*/
  ubyte carry_items;      /* Number of items carried              */
  char last_direction;   /* The last direction the monster went */
  unsigned sev;          /* logseverety level for gods */
 
  int start_room;  /* so people can be set to start certain places */
  int edit;                /* edit state */

  sbyte mobtype;                        /* mob type simple, A, L, B */ 
  unsigned long exp_flag;                 /* exp flag for this mob */
  sbyte hp_num_dice;                        /* number of HPS dice */
  unsigned int hp_size_dice;                /* size of HPS dice */
  unsigned int hp_bonus_hps;                /* bonus hps number */

  ubyte damnodice;           /* The number of damage dice's         */
  ubyte damsizedice;         /* The size of the damage dice's       */

  unsigned int dam_bonus;                        /* damage bonus */
  ubyte medit;                           /*  mob edit menu at */
  struct char_data *mobedit;            /*  mob editing */
        
  ubyte oedit;                            /*  obj editing menu at */
  struct obj_data *objedit;             /*  object editing */

  int tick_to_lag;

  sbyte conditions[MAX_CONDITIONS];      /* Drunk full etc.        */
  int permissions;
  int zone;   /* zone that an NPC lives in */
  int carry_weight;        /* Carried weight                       */
  int timer;               /* Timer for update                     */
  int was_in_room;         /* storage of location for linkdead people */
  int attack_type;         /* The Attack Type Bitvector for NPC's */
  int alignment;           /* +-1000 for alignments               */
  
  char *poofin;
  char *poofout;
  char *prompt;
  char *lastversion;        /* Ultima versione conosciuta al player GGPATCH*/
  char *email;              /* email address in aux */
  char *realname;           /*real name, by GGPATCH*/
  char *authcode;           /*Codice di autorizazione GGPATCH*/
  char *AuthorizedBy;       /*Nome dell'immortale che ha autorizzato
			    * per l'identificazione dei players GGPATCH*/
   char *supporting;   /* guy we'd like to heal*/
   char *bodyguarding; /* guy we'd like to rescue*/
   char *bodyguard; /* our body guard */
   long TempoPassatoIn[MAX_POSITION];
  short PosPrev;
  short WimpyLevel;
   /* tiene traccia del tempo passato nelle singole posizioni */
  char *Sextra;
  char *group_name; /* current group name if any... */  
  
  Alias   *A_list;
  struct char_data *misc;  
  struct char_data *fighting; /* Opponent                          */
   
  struct char_data *hunting;  /* Hunting person..                  */

  struct char_data *ridden_by;
  struct char_data *mounted_on;
  
  struct char_data *charging;        /* we are charging this person */
  int  charge_dir;                /* direction charging */
  
  unsigned long affected_by;  /* Bitvector for spells/skills affected by */
  unsigned long affected_by2; /* Other special things */

  unsigned long act; /* flags for NPC behavior */  

  sh_int apply_saving_throw[MAX_SAVES]; /* Saving throw (Bonuses)  */

};

/* ================== Structure for player/non-player ===================== */
#define CHAR_VALID_MAGIC 1234
#define CHAR_FREEDED_MAGIC 2345
struct char_data
{
  int nMagicNumber;
  long nr;                         /* monster nr */
  long in_room;                    /* Location                    */

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

  void *act_ptr;    /* numeric argument for the mobile actions */

  struct char_player_data player;       /* Normal data            */
  struct char_ability_data abilities;   /* Abilities              */
  struct char_ability_data tmpabilities;/* The abilities we use  */
  struct affected_type *affected;       /* affected by what spells */        
  struct char_point_data points;        /* Points                 */
  struct char_special_data specials;    /* Special plaing constant */
  struct char_data *next_listener;       /* Prossimo che fa eavesdrop */
  sh_int listening_to;		    /* Stanza per eavesdrop		*/
		  
  /* monitor these three */
  struct char_skill_data *skills;       /* Skills                */
  struct obj_data *equipment[MAX_WEAR]; /* Equipment array         */
  struct obj_data *carrying;            /* Head of list            */
  /* end monitor */
        
  struct descriptor_data *desc;         /* NULL for mobiles        */
  struct char_data *orig;               /* Special for polymorph   */

  struct char_data *next_in_room;     /* For room->people - list   */
  struct char_data *next;             /* all in game list  */
  struct char_data *next_fighting;    /* For fighting list         */

  struct follow_type *followers;        /* List of chars followers */
  struct char_data *master;             /* Who is char following?  */
  int invis_level;                      /* visibility of gods */
  int nTeleCount;                     /* Contatore per i teleport di tipo
                                       * TELE_COUNT */
  long lStartRoom;
  long AgeModifier;   
  struct event *points_event[3];   /* events for regening H/M/V */

};

struct extra_descr_data
{
  int nMagicNumber;
  char *keyword;                 /* Keyword in look/examine          */
  char *description;             /* What to see                      */
  struct extra_descr_data *next; /* Next in list                     */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type 
{
  short location;      /* Which ability to change (APPLY_XXX) */
  long modifier;      /* How much it changes by      */
};

struct obj_flag_data
{
  int value[4];       /* Values of the item (see list)    */
  ubyte type_flag;     /* Type of item                     */
  unsigned long wear_flags;     /* Where you can wear it            */
  unsigned long extra_flags;    /* If it hums,glows etc             */
  int weight;         /* Weigt what else                  */
  int cost;           /* Value when sold (gp.)            */
  int cost_per_day;   /* Cost to keep pr. real day        */
  int timer;          /* Timer for object                 */
  long bitvector;     /* To set chars bits                */
};

struct obj_data
{
  sh_int item_number;             /* Where in data-base             */
  int in_room;                    /* In what room -1 when conta/carr  */ 
  struct obj_flag_data obj_flags; /* Object information             */
  struct obj_affected_type affected[MAX_OBJ_AFFECT];  
                                  /* Which abilities in PC to change*/

  sh_int sector;                 /* for large rooms      */
  long char_vnum;                /* for resurrection     */
  char oldfilename[ 20 ];
  char *name;                    /* Title of object :get etc.      */
  char *description ;            /* When in room                   */
  char *short_description;       /* when worn/carry/in cont.       */
  char *action_description;      /* What to write when used        */
  struct extra_descr_data *ex_description; /* extra descriptions   */
  struct char_data *carried_by;  /* Carried by :NULL in room/conta */
  byte   eq_pos;                 /* what is the equip. pos?        */
  struct char_data *equipped_by; /* equipped by :NULL in room/conta*/
  struct obj_data *in_obj;       /* In what object NULL when none  */
  struct obj_data *contains;     /* Contains objects               */
  struct obj_data *next_content; /* For 'contains' lists           */
  struct obj_data *next;         /* For the object list            */
  char *szForbiddenWearToChar;   /* messaggi da visualizzare quando  */
  char *szForbiddenWearToRoom;   /* si tenta di indossare un oggetto */
                                 /* proibito.                        */
  int iGeneric;                  /* Valore generico a disposizione delle 
                                  * procedure speciali */
  int iGeneric1;                  /* Valore generico a disposizione delle
                                  * procedure speciali */
  int iGeneric2;                  /* Valore generico a disposizione delle 
                                  * procedure speciali */
  void *pGeneric;                /* Puntatore generico a disposizione delle 
                                  * procedure speciali */
  void *pGeneric1;                /* Puntatore generico a disposizione delle 
                                  * procedure speciali */
  void *pGeneric2;                /* Puntatore generico a disposizione delle 
                                  * procedure speciali */
};

/* element in monster and object index-tables   */
struct index_data
{
  int iVNum;      /* virtual number of this mob/obj           */
  long pos;       /* file position of this field              */
  int number;     /* number of existing units of this mob/obj        */
  int (*func)( struct char_data *, int, char *, void *, int );  
                  /* special procedure for this mob/obj       */
   char *specname;
   char *specparms;
  void *data;
  char *name;
  char *short_desc;
  char *long_desc;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
  ubyte learned;           /* % chance for success 0 = not learned   */

  /* change to int or long */
  byte flags;             /* SKILL KNOWN? bit settings               */
  byte special;           /* spell/skill specializations             */
  byte nummem;            /* number of times this spell is memorized */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
  short type;           /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  long modifier;       /* This is added to apropriate ability     */
  long location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  struct affected_type *next;
};

/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */
struct char_file_u
{
   int iClass;
   ubyte sex;
   ubyte level[ABS_MAX_CLASS];
   time_t birth;  /* Time of birth of character     */
   int played;    /* Number of secs played in total */
   int   race;
   unsigned int weight;
   unsigned int height;
   char title[80];
   char extra_str[255];
   sh_int hometown;
   char description[240];
   bool talks[MAX_TOUNGE];
   long extra_flags;
   sh_int load_room;            /* Which room to place char in  */
   struct char_ability_data abilities;
   struct char_point_data points;
   struct char_skill_data skills[350];
   struct affected_type affected[40];
   /* specials */
   byte spells_to_learn;
   int alignment;
   unsigned long affected_by;
   unsigned long affected_by2;
   time_t last_logon;  /* Time (in secs) of last logon */
   unsigned long    act;        /* ACT Flags                    */

   /* char data */
   char name[20];
   char authcode[7];  /* codice di autorizzazione */
   char WimpyLevel[4]; /* Wimpy level */
   char dummy[19];      /* per usi futuri */
   char pwd[11];
   sh_int apply_saving_throw[MAX_SAVES];
   int conditions[MAX_CONDITIONS];
   int startroom;  /* which room the player should start in */
   long user_flags;        /* no-delete,use ansi,etc... */
   int speaks;                /* language currently speakin in */
   long agemod;
};

struct obj_file_elem 
{
   /* Bug, rendeva impossibile rentare oggetti oltre il 32565 */
  /*sh_int item_number;*/
   ush_int item_number;

  int value[4];
  int extra_flags;
  int weight;
  int timer;
  unsigned long bitvector;
  char name[128];  /* big, but not horrendously so */
  char sd[128];
  char desc[256];
  ubyte wearpos;
  ubyte depth;
  struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

/* structure for the reset commands */
struct reset_com
{
  char command;   /* current command                      */ 
  bool if_flag;   /* if TRUE: exe only if preceding exe'd */
  int arg1;       /*                                      */
  int arg2;       /* Arguments to the command             */
  int arg3;       /*                                      */
  int arg4;

  /* 
   *  Commands:              *
   *  'M': Read a mobile     *
   *  'O': Read an object    *
   *  'G': Give obj to mob   *
   *  'P': Put obj in obj    *
   *  'G': Obj to char       *
   *  'E': Obj to char equip *
   *  'D': Set state of door *
   */
};

/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
  char *name;             /* name of this zone                  */
  int num;                /* number of this zone                 */
  int lifespan;           /* how long between resets (minutes)  */
  int age;                /* current age of this zone (minutes) */
  int top;                /* upper limit for rooms in this zone */
  int bottom;             /* bottom limit for rooms in this zone */
  short start;            /* has this zone been reset yet?      */
  
  int reset_mode;         /* conditions for reset (see below)   */
  struct reset_com *cmd;  /* command table for reset                   */
  char races[50];  /* races that are tolerated here */
  float CurrTotGold, LastTotGold;
  float CurrToPCGold, LastToPCGold;

  /*
   *  Reset mode:                              *
   *  0: Don't reset, and don't update age.    *
   *  1: Reset if no PC's are located in zone. *
   *  2: Just reset.                           *
   */
};

struct char_data *character_list = 0; /* global l-list of chars          */
long number_of_saved_rooms = 0;
long saved_rooms[WORLD_SIZE]; 
long room_count=0;
long total_bc = 0;
int top_of_world = 0;                 /* ref to the top element of world */
int top_of_zone_table = 0;
struct zone_data *zone_table = NULL;     /* table of reset data             */
char curfile[256]; /* Informazioni sul file in lettura */
struct room_data *room_db[WORLD_SIZE];
#define IS_SET(flag,bit)  ((flag) & (bit))
#define TELE_COUNT           2
#define SECT_WATER_NOSWIM    7
#define SECT_UNDERWATER      9
#define TUNNEL         256 /* Limited #s of people in room */

struct room_direction_data
{
  char *general_description;    /* When look DIR.                  */ 
  char *keyword;                /* for open/close                  */
  long exit_info;             /* Exit info                       */
  long key;                      /* Key's number (-1 for no key)    */
  long to_room;                  /* Where direction leeds (NOWHERE) */
  long open_cmd;                      /* cmd needed to OPEN/CLOSE door   */
};

/* ========================= Structure for room ========================== */
struct room_data
{
        /* sh_int */
  long number;               /* Rooms number                       */
  long zone;                 /* Room zone (for resetting)          */
  long continent;            /* Which continent/mega-zone          */
  long sector_type;             /* sector type (move/hide)            */
  byte blood;					/*Per il sangue nelle stanze		*/
  
  int river_dir;               /* dir of flow on river               */
  int river_speed;             /* speed of flow on river             */

  int  tele_time;              /* time to a teleport                 */
  int  tele_targ;              /* target room of a teleport          */
  char tele_mask;              /* flags for use with teleport        */
  int  tele_cnt;               /* countdown teleports                */

  unsigned char moblim;        /* # of mobs allowed in room.         */
  
  char *name;                  /* Rooms name 'You are ...'           */
  char *description;           /* Shown when entered                 */
  struct extra_descr_data *ex_description; /* for examine/look       */
  struct room_direction_data *dir_option[6]; /* Directions           */
  long room_flags;             /* DEATH,DARK ... etc                 */ 
  byte light;                  /* Number of lightsources in room     */
  ubyte dark;
  int (*funct)( struct char_data *, int, char *, struct room_data *, int);
                               /* special procedure                  */
   char *specname;
   char *specparms;
  struct obj_data *contents;   /* List of items in room              */
  struct char_data *people;    /* List of NPC / PC in room           */
  struct char_data *listeners;  /* per eavesdrop						 */ 

  struct large_room_data *large;  /* special for large rooms         */
  /* Queste due stringhe vengono visualizzate quando la locazione e` di tipo
   * BRIGHT. La prima quando e` notte e il BRIGHT entra in funzione, la
   * seconda quando e` giorno ed il BRIGHT e` disattivo.
   * Sono utili per strade illuminate dai lampioni ad esempio.
   * */
  char *szWhenBrightAtNight;      
  char *szWhenBrightAtDay;

};
int prterr(int err, int tipo, int num, char *nome, char *msg)
{
	char buf[160];

	if (!err)
   	{
   		log("{-------------------------------------------------------------------------------");
		if (tipo ==0)
      		sprintf(buf, "ERRORE in #%d, %s", num, nome);
		else if (tipo ==1)
      		sprintf(buf, "ERRORE su %s", nome);
		log(buf);
  	}
   	log(msg);
	err =1;
	return err;
}

int MIN(int a, int b)
{
        return a < b ? a:b;
}

int MAX(int a, int b)
{
        return a > b ? a:b;
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
   if (to - from + 1 )
        return((random() % (to - from + 1)) + from);
   else
       return(from);
}

/* simulates dice roll */
int dice(int number, int size) 
{
  int r;
  int sum = 0;

  if (size == 0) return(0);

  for (r = 1; r <= number; r++) sum += ((random() % size)+1);
  return(sum);
}

int IsSmall( struct char_data *ch)
{
  switch(GET_RACE(ch)) 
  {
   case RACE_SMURF:
   case RACE_GNOME:
   case RACE_HALFLING:
   case RACE_GOBLIN:
   case RACE_ENFAN:
   case RACE_DEEP_GNOME:
    return(1);
   default:
    return(0);
  }
}

int IsGiant ( struct char_data *ch)
{
  switch(GET_RACE(ch)) 
  {
   case RACE_GIANT:
   case RACE_GIANT_HILL   :
   case RACE_GIANT_FROST  :
   case RACE_GIANT_FIRE   :
   case RACE_GIANT_CLOUD  :
   case RACE_GIANT_STORM  :
   case RACE_GIANT_STONE  :
   case RACE_HALF_GIANT        :
   case RACE_TYTAN:
   case RACE_GOD:
    return(1);
   case RACE_TROLL:
     return((number(1,100)>20));
   default:
    return(0);
    }
  }
}

int GetMaxLevel(struct char_data *ch)
{
  register int max=0, i;

  for (i=0; i< MAX_CLASS; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch,i);
  }

  return(max);
}

int DetermineExp( struct char_data *mob, int exp_flags)
{
   /* Un intervento qui si riflette su TUTTI i mob */

  int base;
  int phit;
  int sab;
  
  if (exp_flags > 400) 
  { 
    mudlog( LOG_ERROR, "Exp flags on %s are > 400 (%d)", GET_NAME(mob), 
             exp_flags );
  }

  /* 
   * reads in the monster, and adds the flags together 
   * for simplicity, 1 exceptional ability is 2 special abilities 
   */

  if (GetMaxLevel(mob) < 0)
    return(1);

  switch(GetMaxLevel(mob)) 
  {

   case 0:   
    base = 1;
    phit = 1;
    sab =  1;
    break;

   case 1:   
    base = 8;
    phit = 2;
    sab =  2;
    break;

   case 2: 
    base = 12;
    phit = 2;
    sab =  3;
    break;

   case 3:
    base = 17;
    phit = 3;
    sab =  4;
    break;

   case 4:
    base = 23;
    phit = 3;
    sab =  6;
    break;

   case 5:
    base = 30;
    phit = 3;
    sab =  7;
    break;

   case 6: 
    base = 37;
    phit = 3;
    sab =  9;
    break;

   case 7:  
    base = 47;
    phit = 4;
    sab =  12;
    break;

   case 8:  
    base = 57;
    phit = 4;
    sab  = 14;
    break;

   case 9: 
    base = 70;
    phit = 4;
    sab  = 17;
    break;

   case 10:
    base = 84;
    phit  = 4;
    sab   = 21;
    break;

   case 11:  
    base = 101;
    phit  = 5;
    sab   = 25;
    break;

   case 12:
    base = 122;
    phit  = 5;
    sab   = 30;
    break;

   case 13: 
    base = 146;
    phit  = 5;
    sab   = 36;
    break;

   case 14:
    base = 175;
    phit  = 6;
    sab   = 44;
    break;

   case 15:
    base = 209;
    phit  = 6;
    sab   = 52;
    break;

   case 16:
    base = 251;
    phit  = 6;
    sab   = 63;
    break;

   case 17:
    base = 301;
    phit  = 7;
    sab   = 75;
    break;

   case 18:
    base = 362;
    phit  = 7;
    sab   = 90;
    break;

   case 19:
    base = 435;
    phit  = 8;
    sab   = 109;
    break;

   case 20: 
    base = 524;
    phit  = 8;
    sab   = 131;
    break;

   case 21: 
    base = 632;
    phit  = 9;
    sab   =  158;
    break;

   case 22:
    base = 762;
    phit  = 9;
    sab   = 190;
    break;

   case 23:  
    base = 919;
    phit  = 10;
    sab   = 230;
    break;

   case 24:
    base = 1110;
    phit  = 10;
    sab   = 277;
    break;

   case 25: 
    base = 1339;
    phit  = 11;
    sab   = 335;
    break;

   case 26: 
    base = 1617;
    phit  = 12;
    sab   = 404;
    break;

   case 27: 
    base = 1951;
    phit  = 12;
    sab   = 488;
    break;

   case 28: 
    base = 2354;
    phit  = 13;
    sab   = 588;
    break;

   case 29:
    base = 2838;
    phit  = 14;
    sab   = 710;
    break;

   case 30: 
    base = 3421;
    phit  = 15;
    sab   = 855;
    break;

   case 31: 
    base = 4120;
    phit  = 16;
    sab   = 1030;
    break;

   case 32:
    base = 4960;
    phit  = 17;
    sab   = 1240;
    break;

   case 33:
    base = 5968;
    phit  = 18;
    sab   = 1492;
    break;

   case 34:
    base = 7175;
    phit = 19;
    sab  = 1794;
    break;
      
   case 35:
    base = 8621;
    phit  = 21;
    sab   = 2155;
    break;

   case 36:
    base = 10351;
    phit  = 22;
    sab   = 2588;
    break;

   case 37:
    base = 12422;
    phit  = 23;
    sab   = 3105;
    break;

   case 38:
    base = 14897;
    phit  = 25;
    sab   = 3724;
    break;

   case 39:
    base = 17857;
    phit = 26;
    sab  = 4464;
    break;
    
   case 40:
    base = 21394;
    phit  = 28;
    sab   = 5348;
    break;

   case 41:
    base = 25619;
    phit = 29;
    sab  = 6405;
    break;

   case 42:
    base = 30666;
    phit  = 31;
    sab   = 7666;
    break;

   case 43:
    base = 36692;
    phit = 33;
    sab  = 9173;
    break;

   case 44:
    base = 43889;
    phit  = 35;
    sab   = 10975;
    break;

   case 45:
    base = 52482;
    phit = 37;
    sab  = 13121;
    break;

   case 46:
    base = 62742;
    phit = 40;
    sab  = 15685;
    break;

   case 47:
    base = 74992;
    phit = 42;
    sab  = 18748;
    break;
    
   case 48:
    base = 89618;
    phit = 45;
    sab  = 22405;
    break;

   case 49:
    base = 107084;
    phit = 47;
    sab  = 26771;
    break;

   case 50:
    base = 127942;
    phit = 50;
    sab  = 31985;
    break;

   case 51:
    base = 150000;
    phit = 53;
    sab  = 37500;
    break;

   case 52:
    base = 170000;
    phit = 55;
    sab  = 42500;
    break;

   case 53:
    base = 190000;
    phit = 57;
    sab  = 47500;
    break;

   case 54:
    base = 210000;
    phit = 59;
    sab  = 52500;
    break;

   case 55:
    base = 230000;
    phit = 61;
    sab  = 57500;
    break;

   case 56:
    base = 250000;
    phit = 63;
    sab  = 62500;
    break;

   case 57:
    base = 275000;
    phit = 65;
    sab  = 68750;
    break;

   case 58:
    base = 300000;
    phit = 67;
    sab  = 75000;
    break;

   case 59:
    base = 325000;
    phit = 69;
    sab  = 71250;
    break;

   default:
    base = 350000;
    phit = 70;
    sab  = 87500;
    break;
  }

  return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));
}

int AddHatred( struct char_data *ch, int parm_type, int parm)
{
  if( IS_PC( ch ) )
  { 
    return 0;
  }

  switch(parm_type) 
  {
  case OP_SEX :
    SET_BIT(ch->hatefield, HATE_SEX);
    ch->hates.sex = parm;
    break;
  case OP_RACE:
    SET_BIT(ch->hatefield, HATE_RACE);
    ch->hates.race = parm;
    break;
  case OP_GOOD:
    SET_BIT(ch->hatefield, HATE_GOOD);
    ch->hates.good = parm;
    break;
  case OP_EVIL:
    SET_BIT(ch->hatefield, HATE_EVIL);
    ch->hates.evil = parm;
    break;
  case OP_CLASS:
    SET_BIT(ch->hatefield, HATE_CLASS);
    ch->hates.iClass = parm;
    break;
  case OP_VNUM:
    SET_BIT(ch->hatefield, HATE_VNUM);
    ch->hates.vnum = parm;
    break;
  default:
    fprintf(stderr,"Invaild parm type in AddHatred for %s\n", GET_NAME(ch));
    return 0;
    break;
  }
  SET_BIT(ch->specials.act, ACT_HATEFUL);
  return 1;
}

void SetRacialStuff( struct char_data *mob)
{

  switch(GET_RACE(mob)) 
  {
   case RACE_BIRD:
    SET_BIT(mob->specials.affected_by, AFF_FLYING);    
    break;
   case RACE_FISH:
    SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
    break;
   case RACE_SEA_ELF:
    SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
    /* e poi prosegue per le altre caratteristiche degli elfi */
   case RACE_ELVEN:
   case RACE_DROW:
   case RACE_GOLD_ELF:
   case RACE_WILD_ELF:
    SET_BIT(mob->specials.affected_by,AFF_INFRAVISION);
    SET_BIT(mob->immune, IMM_CHARM);
    break; 
   case RACE_DWARF:
   case RACE_DARK_DWARF:
   case RACE_DEEP_GNOME:
   case RACE_GNOME:
   case RACE_MFLAYER:
   case RACE_TROLL:
   case RACE_ORC:
   case RACE_GOBLIN:
   case RACE_HALFLING:
   case RACE_GNOLL:
    SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
    break;
   case RACE_INSECT:
   case RACE_ARACHNID:
    if (IS_PC(mob)) 
    {
      GET_STR(mob) = 18;
      GET_ADD(mob) = 100;
    }
    break;
   case RACE_LYCANTH:
    SET_BIT(mob->M_immune, IMM_NONMAG);
    break;
   case RACE_PREDATOR:
    if (mob->skills)
      mob->skills[SKILL_HUNT].learned = 100;
    break;

   case RACE_GIANT_FROST  :
    SET_BIT(mob->M_immune, IMM_COLD);
    SET_BIT(mob->susc,IMM_FIRE);
    break;
   case RACE_GIANT_FIRE   :
    SET_BIT(mob->M_immune, IMM_FIRE);
    SET_BIT(mob->susc,IMM_COLD);    
    break;
   case RACE_GIANT_CLOUD  :
    SET_BIT(mob->M_immune, IMM_SLEEP);  /* should be gas... but no IMM_GAS */
    SET_BIT(mob->susc,IMM_ACID);    
    break;
   case RACE_GIANT_STORM  :
    SET_BIT(mob->M_immune, IMM_ELEC);
    break;

   case RACE_GIANT_STONE  :
    SET_BIT(mob->M_immune, IMM_PIERCE);
    break;
   case RACE_UNDEAD:
   case RACE_UNDEAD_VAMPIRE : 
   case RACE_UNDEAD_LICH    : 
   case RACE_UNDEAD_WIGHT   :
   case RACE_UNDEAD_GHAST   :
   case RACE_UNDEAD_GHOUL   :
   case RACE_UNDEAD_SPECTRE :   
   case RACE_UNDEAD_ZOMBIE  :
   case RACE_UNDEAD_SKELETON : 
    SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
    SET_BIT(mob->M_immune,IMM_POISON+IMM_DRAIN+IMM_SLEEP+IMM_HOLD+IMM_CHARM);
    break;
    
   case RACE_DRAGON_RED    : 
    SET_BIT(mob->M_immune,IMM_FIRE);
    SET_BIT(mob->susc,IMM_COLD);
    break;
   case RACE_DRAGON_BLACK  : 
    SET_BIT(mob->M_immune,IMM_ACID);
    break;

   case RACE_DRAGON_GREEN  : 
    SET_BIT(mob->M_immune,IMM_SLEEP);
    break;

   case RACE_DRAGON_WHITE  : 
    SET_BIT(mob->M_immune,IMM_COLD);
    SET_BIT(mob->susc,IMM_FIRE);
    break;
   case RACE_DRAGON_BLUE   : 
    SET_BIT(mob->M_immune,IMM_ELEC);
    break;
   case RACE_DRAGON_SILVER : 
    SET_BIT(mob->M_immune,IMM_ENERGY);
    break;
   case RACE_DRAGON_GOLD   : 
    SET_BIT(mob->M_immune,IMM_SLEEP+IMM_ENERGY);
    break;
   case RACE_DRAGON_BRONZE :
    SET_BIT(mob->M_immune,IMM_COLD+IMM_ACID);
    break;
   case RACE_DRAGON_COPPER :
    SET_BIT(mob->M_immune,IMM_FIRE);
    break;
   case RACE_DRAGON_BRASS  : 
    SET_BIT(mob->M_immune,IMM_ELEC);
    break;
 
   case RACE_HALF_ELVEN:
   case RACE_HALF_OGRE:
   case RACE_HALF_ORC:
   case RACE_HALF_GIANT:
    SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
    break;

   default:
    break;
  }

  /* height and weight      / Hatred Foes! / */
  if (IS_NPC(mob))
  { 
    switch(GET_RACE(mob)) 
    {
     case RACE_HUMAN:
      break;

     case RACE_ELVEN:
     case RACE_GOLD_ELF:
     case RACE_WILD_ELF:
     case RACE_SEA_ELF:
      AddHatred(mob,OP_RACE,RACE_ORC);
      break;
     case RACE_GNOME:
      break;
      
     case RACE_DWARF:
      AddHatred(mob,OP_RACE,RACE_GOBLIN);
      AddHatred(mob,OP_RACE,RACE_ORC);
      break;
     case RACE_HALFLING:
      break;
      
     case RACE_LYCANTH:
      AddHatred(mob,OP_RACE,RACE_HUMAN);
      break;
      
     case RACE_UNDEAD:
      break;
     case RACE_DARK_DWARF:                  /* these guys hate good people */
     case RACE_UNDEAD_VAMPIRE :
     case RACE_UNDEAD_LICH    : 
     case RACE_UNDEAD_WIGHT   :
     case RACE_UNDEAD_GHAST   :
     case RACE_UNDEAD_GHOUL   :
     case RACE_UNDEAD_SPECTRE :   
      AddHatred(mob,OP_GOOD,1000);
      break;
     case RACE_UNDEAD_ZOMBIE  :
      break;
     case RACE_UNDEAD_SKELETON :
      break;
     case RACE_VEGMAN:
      break;
     case RACE_MFLAYER:
      break;      
     case RACE_DROW:
      AddHatred(mob,OP_RACE,RACE_ELVEN);
      break;
     case RACE_SKEXIE:
      break;
     case RACE_TROGMAN:
      break;
     case RACE_LIZARDMAN:
      break;
     case RACE_SARTAN:
      break;
     case RACE_PATRYN:
      break;
     case RACE_DRAAGDIM:
      break;
     case RACE_ASTRAL: 
      break;

     case RACE_HORSE:
      mob->player.weight = 400;
      mob->player.height = 175;
      break;

     case RACE_ORC:
      AddHatred(mob,OP_GOOD,1000);
      AddHatred(mob,OP_RACE,RACE_ELVEN);
      mob->player.weight = 150;
      mob->player.height = 140;
      break;
      
     case RACE_SMURF:
      mob->player.weight = 5;
      mob->player.height = 10;
      break;

     case RACE_GOBLIN:
     case RACE_GNOLL:
      AddHatred(mob,OP_GOOD,1000);
      AddHatred(mob,OP_RACE,RACE_DWARF);
      break;
      
     case RACE_ENFAN:
      mob->player.weight = 120;
      mob->player.height = 100;
      break;

     case RACE_LABRAT:
      break;
     case RACE_INSECT:
      break;
     case RACE_ARACHNID:
      break;
     case RACE_REPTILE:
      break;
     case RACE_DINOSAUR:
      break;
     case RACE_FISH:
      break;
     case RACE_PREDATOR:
      break;
     case RACE_SNAKE:
      break;
     case RACE_HERBIV:
      break;
     case RACE_VEGGIE:
      break;
     case RACE_ELEMENT:
      break;
     case RACE_PRIMATE:
      break;
      
     case RACE_GOLEM:
      mob->player.weight = 10+GetMaxLevel(mob)*GetMaxLevel(mob)*2;
      mob->player.height = 20+MIN(mob->player.weight,600);
      break;
      
     case RACE_DRAGON:
     case RACE_DRAGON_RED    :
     case RACE_DRAGON_BLACK  :
     case RACE_DRAGON_GREEN  :
     case RACE_DRAGON_WHITE  :
     case RACE_DRAGON_BLUE   :
     case RACE_DRAGON_SILVER :
     case RACE_DRAGON_GOLD   :
     case RACE_DRAGON_BRONZE :
     case RACE_DRAGON_COPPER :
     case RACE_DRAGON_BRASS  : 
      mob->player.weight = MAX(60, GetMaxLevel(mob)*GetMaxLevel(mob)*2);
      mob->player.height = 100+ MIN(mob->player.weight, 500);
      break;
      
     case RACE_BIRD: 
     case RACE_PARASITE: 
     case RACE_SLIME: 
      mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
      mob->player.height = 10*GetMaxLevel(mob);
      break;
      
      
     case RACE_GHOST:
      mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
      mob->player.height = 10*GetMaxLevel(mob);
      break;
     case RACE_TROLL: 
      AddHatred(mob,OP_GOOD,1000);
      mob->player.height = 200+GetMaxLevel(mob)*15;
      mob->player.weight = (int)( mob->player.height * 1.5 );
      break;
      
     case RACE_GIANT:
     case RACE_GIANT_HILL   :
     case RACE_GIANT_FROST  :
     case RACE_GIANT_FIRE   :
     case RACE_GIANT_CLOUD  :
     case RACE_GIANT_STORM  :
      mob->player.height = 200+GetMaxLevel(mob)*15;
      mob->player.weight = (int)( mob->player.height * 1.5 );
      AddHatred(mob,OP_RACE,RACE_DWARF);
      break;
      
     case RACE_DEVIL: 
     case RACE_DEMON: 
      AddHatred(mob,OP_GOOD,1000);
      mob->player.height = 200+GetMaxLevel(mob)*15;
      mob->player.weight = (int)( mob->player.height * 1.5 );
      break;
      
      
     case RACE_PLANAR:    
      mob->player.height = 200+GetMaxLevel(mob)*15;
      mob->player.weight = (int)( mob->player.height * 1.5 );
      break;
      
     case RACE_GOD: break;
     case RACE_TREE: break;
     case RACE_TYTAN: 
      mob->player.weight = MAX(500, GetMaxLevel(mob)*GetMaxLevel(mob)*10);
      mob->player.height = GetMaxLevel(mob)/2*100;
      break;
      
     case RACE_HALF_ELVEN:
     case RACE_HALF_OGRE:
     case RACE_HALF_ORC:
     case RACE_HALF_GIANT:    
      break;
    } /* end switch */
  } 
  else /* !IS_NPC( mob ) */
  {
    switch( GET_RACE( mob ) )
    {
     case RACE_ORC:
     case RACE_GOBLIN:
     case RACE_TROLL:
     case RACE_DARK_DWARF:
     case RACE_DEEP_GNOME:
      GET_ALIGNMENT( mob ) = -1000;
      break;
     default:
      break;      
    }
  }                   
}

/****************************************************************************
 * Legge un numero dal file puntato da pFIle. Se il numero contiene il 
 * carattere | le due porzioni di numero vengono addizionate. Ad esempio
 * 4|128 diventa 132. Molto utile per i flags.
 ****************************************************************************/
long fread_number( FILE *pFile )
{
   long number;
   bool sign;
   char c;
   char memo[1024];
   long l;
   l=0;
   do
   {
      c = getc( pFile );
      if (l<1023) memo[l++]=c;
   } while( isspace(c) );
   
   number = 0;
   
   sign = 0;
   if( c == '+' )
   {
      c = getc( pFile );
   }
   else if ( c == '-' )
   {
      sign = 1;
      c = getc( pFile );
      if (l<1023) memo[l++]=c;
      
   }
   
   if( !isdigit(c) )
   {
      memo[l]=0;
      ungetc( c, pFile );
      return 0;
   }
   
   while( isdigit(c) )
   {
      number = number * 10 + c - '0';
      c = getc( pFile );
   }
   
   if( sign )
     number = 0 - number;
   
   if( c == '|' )
     number += fread_number( pFile );
   else if ( c != ' ' )
     ungetc( c, pFile );
   
   return number;
}

long fread_if_number( FILE *pFile )
{
   long number;
   bool sign;
   char c;
   
   do
   {
      c = getc( pFile );
   } while( isspace(c) );
   
   number = 0;
   
   sign = 0;
   if( c == '+' )
   {
      c = getc( pFile );
   }
   else if ( c == '-' )
   {
      sign = 1;
      c = getc( pFile );
   }
   
   if( !isdigit(c) )
   {
      ungetc( c, pFile );
      return 0;
   }
   
   while( isdigit(c) )
   {
      number = number * 10 + c - '0';
      c = getc( pFile );
   }
   
   if( sign )
     number = 0 - number;
   
   if( c == '|' )
     number += fread_number( pFile );
   else if ( c != ' ' )
     ungetc( c, pFile );
   
   return number;
}

char *fread_string(FILE *f1)
{
  char buf[ MAX_STRING_LENGTH ];
  int i = 0, tmp;
  char *pReturnString = NULL;
  
  buf[ 0 ] = '\0';

  while( i < MAX_STRING_LENGTH - 3 )
  {
    if( ( tmp = fgetc(f1) ) == EOF )
      break;

    if(tmp == '~') 
    {
      break;
    }

    buf[i++] = (char)tmp;
    if (buf[i-1] == '\n')
      buf[i++] = '\r';
  }

  if( i >= MAX_STRING_LENGTH - 3 )
  { 
    while( ( tmp = fgetc( f1 ) ) != EOF )
      if( tmp == '~' )
        break;
  }
  
  buf[ i ] = '\0';

  fgetc( f1 );
  
  
  if( strlen( buf ) )
  {
     pReturnString = (char *) malloc(strlen(buf)+1); 
     if (pReturnString) 
     {
	strcpy(pReturnString,buf);
     }
     
     fflush(NULL);
  }
  
  return pReturnString;
}

int read_obj_from_file(struct obj_data *obj, FILE *f)
{
  int i,tmp;
  long bc = 0L;
  char chk[ 161 ];
  struct extra_descr_data *new_descr;
  
  obj->name = fread_string(f);

   if (obj->name )
  {
    bc += strlen(obj->name);
  }
  obj->short_description = fread_string(f);
  if( obj->short_description )
  {
    bc += strlen(obj->short_description);
  }
  obj->description = fread_string(f);
  if( obj->description )
  {
    bc += strlen(obj->description);
  }
  obj->action_description = fread_string(f);
  if( obj->action_description )
  {
    bc += strlen(obj->action_description);
  }

  /* *** numeric data *** */
  obj->obj_flags.type_flag = fread_number( f );
  obj->obj_flags.extra_flags = fread_number( f );
  obj->obj_flags.wear_flags = fread_number( f );
  obj->obj_flags.value[0] = fread_number( f );
  obj->obj_flags.value[1] = fread_number( f );
  obj->obj_flags.value[2] = fread_number( f );
  obj->obj_flags.value[3] = fread_number( f );
  obj->obj_flags.weight = fread_number( f );
  obj->obj_flags.cost = fread_number( f );
  obj->obj_flags.cost_per_day = fread_number( f );

  /* *** extra descriptions *** */
  obj->ex_description = 0;

  while( fscanf( f, " %160s \n", chk ) == 1 && *chk == 'E' )
  {
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
    bc += sizeof(struct extra_descr_data);
    new_descr->keyword = fread_string(f);
    if( new_descr->keyword )
      bc += strlen(new_descr->keyword);
    new_descr->description = fread_string(f);
    if( new_descr->description )
      bc += strlen(new_descr->description);

    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }

  for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)     
  {
    fscanf(f, " %d ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(f, " %d \n", &tmp);
    obj->affected[i].modifier = tmp;
    if( fscanf(f, " %160s \n", chk) != 1 )
    {
      i++;
      break;
    }
  }

  for (;(i < MAX_OBJ_AFFECT);i++)       
  {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }
  
  if( *chk == 'P' )
  {
    obj->szForbiddenWearToChar = fread_string( f );
    obj->szForbiddenWearToRoom = fread_string( f );
    fscanf( f, " %160s \n", chk );
  }
  else
  {
    obj->szForbiddenWearToChar = NULL;
    obj->szForbiddenWearToRoom = NULL;
  }
  return bc;
}

void clear_object(struct obj_data *obj)
{
  memset(obj, '\0', sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room      = NOWHERE;
  obj->eq_pos       = -1;
}

/* returns the real number of the object with given virtual number */
int real_object( int nVNum )
{
  long bot, top, mid;

  bot = 0;
  top = top_of_sort_objt - 1;

  /* perform binary search on obj-table */
  for(;;)
  {
    mid = (bot + top) / 2;

    if( (obj_index + mid)->iVNum == nVNum )
      return(mid);
    if( bot >= top ) 
    {
        /* start unsorted search now */
      for( mid = top_of_sort_objt; mid < top_of_objt; mid++ )
        if( (obj_index + mid )->iVNum == nVNum )
          return( mid );
      return( -1 );
    }
    if( (obj_index + mid)->iVNum > nVNum )
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int iVNum)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_sort_mobt - 1;

  /* perform binary search on mob-table */
  for (;;)
  {
    mid = (bot + top) / 2;

    if( (mob_index + mid)->iVNum == iVNum )
      return(mid);
    if( bot >= top )
    {
      /* start unsorted search now */
      for( mid = top_of_sort_mobt; mid < top_of_mobt; mid++ )
        if( (mob_index + mid)->iVNum == iVNum )
          return( mid );
      return( -1 );
    }
    if( (mob_index + mid)->iVNum > iVNum )
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
  memset(ch, '\0', sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100; /* Basic Armor */
  ch->size = 25;
  ch->nMagicNumber = CHAR_VALID_MAGIC;
}

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
   FILE *f;
   struct obj_data *obj;
   int i;
   long bc;
   char buf[100];
   
   i = nr;
   if( type == VIRTUAL )
      nr = real_object( nr );
   if( nr < 0 || nr >= top_of_objt )
      return NULL;
   
   CREATE(obj,struct obj_data,1);
   bc = sizeof(struct obj_data);
   
   clear_object(obj);
   
   if(obj_index[nr].data == NULL)
   {
      /* object haven't data structure */
      if(obj_index[nr].pos == -1)
      {
	 /* object in external file */
	 sprintf(buf,"%s/%d",OBJ_DIR,obj_index[nr].iVNum);
	 if((f = fopen(buf,"rt"))==NULL)
	 {
	    free(obj);
	    return(0);
	 }
	 fscanf( f, "#%*d \n" );
	 read_obj_from_file(obj, f);
	 fclose(f);
      }
      else
      {
	 if( fseek(obj_f, obj_index[nr].pos, 0) == 0 )
	 {
	    read_obj_from_file(obj, obj_f);
	 }
	 else
	 {
	    free( obj );
	    return NULL;
	 }
      }
   }
   else
   {
      /* data for object present */
	  log("clone_obj_to_obj(obj, (struct obj_data *)obj_index[nr].data)");
   }
   
   obj->in_room = NOWHERE;
   obj->next_content = 0;
   obj->carried_by = 0;
   obj->equipped_by = 0;
   obj->eq_pos = -1;
   obj->in_obj = 0;
   obj->contains = 0;
   obj->item_number = nr;
   obj->in_obj = 0;
   
   obj->next = object_list;
   object_list = obj;
   
   obj_index[nr].number++;
   
   obj_count++;
//   fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].iVNum, bc);
   return (obj);
}

int CountBits(struct char_data *ch, int iClass)
{
  if (iClass == 1) return(1);
  if (iClass == 2) return(2);
  if (iClass == 4) return(3);
  if (iClass == 8) return(4);
  if (iClass == 16) return(5);
  if (iClass == 32) return(6);
  if (iClass == 64) return(7);
  if (iClass == 128) return(8);
  if (iClass == 256) return(9);
  if (iClass == 512) return(10);
  if (iClass == 1024) return(11);
  if (iClass == 2048) return(12);

  fprintf(stderr,"Unrecognized class in CountBits for %s\n",GET_NAME(ch));
  return 0;
}

int GetClassLevel(struct char_data *ch, int iClass)
{

  if (IS_SET(ch->player.iClass, iClass)) {
    return(GET_LEVEL(ch, CountBits(ch, iClass)-1));
  }
  return(0);
}

int OnlyClass( struct char_data *pChar, int nClass )
{
  int i;

  for( i = 1; i <= CLASS_PSI; i *= 2 )
  {
    if( GetClassLevel( pChar, i ) != 0 )
      if( !( i & nClass ) )
        return 0;
  }
  return 1;

}

int HasClass(struct char_data *ch, int iClass)
{

  if (IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF))
  {
    /* Se non e` specificata una classe particolare, restituisce TRUE, visto
     * che gli NPC sono di tutte e nessuna classe. */
    if( !IS_SET( iClass, CLASS_MONK) || !IS_SET( iClass, CLASS_DRUID) ||
        !IS_SET( iClass, CLASS_BARBARIAN) || !IS_SET( iClass, CLASS_SORCERER) ||
        !IS_SET( iClass, CLASS_PALADIN) || !IS_SET( iClass, CLASS_RANGER) ||
        !IS_SET( iClass, CLASS_PSI ) )
      return(1);
    /* Se invece la creatura ha una classe particolare (diversa dal WARRIOR)
     * fa lo stesso controllo dei giocatori. */
  } /* was NPC */

  if (IS_SET(ch->player.iClass, iClass))
    return(1);

  return 0;
}

int HowManyClasses(struct char_data *ch)
{
  short i, tot=0;

  for (i=0;i<MAX_CLASS;i++) 
  {
    if (GET_LEVEL(ch, i)) 
    {
      tot++;
    }
  } /* end for */
  
  if (tot) 
    return(tot);
    
  else 
  {

  
    if (IS_SET(ch->player.iClass, CLASS_MAGIC_USER)) 
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_WARRIOR)) 
      tot++;
      
    if (IS_SET(ch->player.iClass, CLASS_THIEF))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_CLERIC))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_DRUID))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_MONK))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_BARBARIAN))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_SORCERER))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_PALADIN))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_RANGER))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_PSI))
      tot++;

   }
  
  return(tot);
}

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return(p0);                               /* < 15   */
  else if (age <= 29)
    return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
  else if (age <= 44)
    return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
  else if (age <= 59)
    return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
  else if (age <= 79)
    return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
  else
    return(p6);                               /* >= 80 */
}

void mud_time_passed2(time_t t2, time_t t1, struct time_info_data *t)
{
   long secs, monthsecs, daysecs, hoursecs;
   
   /* eld (6-9-93) -- Hopefully, this will fix the negative month, day, etc.  */
   /*                 problems...                                             */
   
   
   if(t2 >= t1)
   {
      secs = (long) (t2 - t1);
      
      t->year = secs/SECS_PER_MUD_YEAR;
      
      monthsecs = secs % SECS_PER_MUD_YEAR;
      t->month = monthsecs/SECS_PER_MUD_MONTH;
      
      daysecs = monthsecs % SECS_PER_MUD_MONTH;
      t->day = daysecs/SECS_PER_MUD_DAY;
      
      hoursecs = daysecs % SECS_PER_MUD_DAY;
      t->hours = hoursecs/SECS_PER_MUD_HOUR;
   }
   else
   {
      
      secs = (long) (t1 - t2);
      
      t->year = secs/SECS_PER_MUD_YEAR;
      
      monthsecs = secs % SECS_PER_MUD_YEAR;
      t->month = monthsecs/SECS_PER_MUD_MONTH;
      
      daysecs = monthsecs % SECS_PER_MUD_MONTH;
      t->day = daysecs/SECS_PER_MUD_DAY;
      
      hoursecs = daysecs % SECS_PER_MUD_DAY;
      t->hours = hoursecs/SECS_PER_MUD_HOUR;
      
      if(t->hours) {
	 t->hours = 24 - t->hours;
	 t->day = t->day + 1;
      }
      if(t->day) {
	 t->day = 35 - t->day;
	 t->month = t->month + 1;
      }
      if(t->month) {
	 t->month = 17 - t->month;
	 t->year = t->year + 1;
      }
      if(t->year)
	t->year = -t->year;                
   }
}

void age2(struct char_data *ch, struct time_info_data *g)
{

  mud_time_passed2(time(0),ch->player.time.birth, g);
  g->ayear=g->year;
  g->year += 17;   /* All players start at 17 */
  g->ayear=g->year; /* QUi resta l'eta' anagrafica */
  g->year = MAX(17,g->ayear+ch->AgeModifier); /* Qui l'eta' effettiva */
   
   /* I principi non invecchiano oltre un certo limite*/
   if (IS_PRINCE(ch)) 
   {
      g->year=(MIN(g->year,50));
   }
   
  
}

/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
   int max, tmp;
   
   max = 0;
   
   if (!IS_PC(ch)) return(100);
   if (HasClass(ch, CLASS_MAGIC_USER)) {
      max += 100;
      max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
   }
   
   /* actually this is worthless as Sorcerer's do not */
   /* use mana at all.... */
   if (HasClass(ch, CLASS_SORCERER)) {
      max += 100;
      max += GET_LEVEL(ch, SORCERER_LEVEL_IND) * 5;
   }
   
   if (HasClass(ch, CLASS_PSI)) {
      max += 100;
      max += GET_LEVEL(ch, PSI_LEVEL_IND) * 5;
   }
   
   
   if (HasClass(ch, CLASS_PALADIN)) {
      max += 100;
      max += (GET_LEVEL(ch, PALADIN_LEVEL_IND)/4) * 5;
   }
   
   if (HasClass(ch, CLASS_RANGER)) {
      max += 100;
      max += (GET_LEVEL(ch, RANGER_LEVEL_IND)/4) * 5;
   }
   
   if (HasClass(ch, CLASS_CLERIC)) {
      max += 100;
      max += (GET_LEVEL(ch, CLERIC_LEVEL_IND)/3) * 5;
   }
   
   if (HasClass(ch, CLASS_DRUID)) {
      max += 100;
      max += (GET_LEVEL(ch, DRUID_LEVEL_IND)/3) * 5;
   }
   
   if (HasClass(ch, CLASS_THIEF)) {
      max += 100;
   }
   
   if (HasClass(ch, CLASS_WARRIOR)) {
      max += 100;
   }
   
   if (HasClass(ch, CLASS_MONK)) {
      max += 100;
   }
   
   max /= HowManyClasses(ch);
   
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

   struct time_info_data ma;
   age2(ch, &ma);
   max += (graf(ma.year, 6,8,12,16,20,24,28))-10;
   
   /*
    * Add class mana maximums here...
    */
   
   if (OnlyClass(ch,CLASS_BARBARIAN)) /* 100 mana max for barbs */
   {
      max=100;  /* barbarians only get 100 mana... */
   }
   
   return(max);
}

int IsRideable( struct char_data *ch)
{
  if (IS_NPC(ch) && !IS_PC(ch)) 
  {
    switch(GET_RACE(ch)) 
    {
     case RACE_HORSE:
     case RACE_DRAGON:
     case RACE_DRAGON_RED    :
     case RACE_DRAGON_BLACK  :
     case RACE_DRAGON_GREEN  :
     case RACE_DRAGON_WHITE  :
     case RACE_DRAGON_BLUE   :
     case RACE_DRAGON_SILVER :
     case RACE_DRAGON_GOLD   :
     case RACE_DRAGON_BRONZE :
     case RACE_DRAGON_COPPER :
     case RACE_DRAGON_BRASS  :
      return(1);
      break;
     default:
      return(0);
      break;
    }
  } 
  else 
    return(0);
}

int move_limit(struct char_data *ch)
{
  int max;

  if (IS_PC(ch))
  {
    max = 100 ;
  }
  else
  {
     max = ch->points.max_move;
     if (IsRideable(ch))
     max *= 2;
     return(max);
  }
   struct time_info_data ma;
   age2(ch, &ma);
   max -= (graf(ma.year, -4,0,2,6,10,14,18));
   
   
   
   if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_GNOME)
   max -= 35;
   else if (GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_DROW  ||
	    GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
	    GET_RACE(ch) == RACE_SEA_ELF ||
	    GET_RACE(ch)== RACE_HALF_ELVEN)
   max += 20;
   else if (GET_CLASS(ch) == CLASS_BARBARIAN)
   max +=45;  /* barbs get more move ... */
   else if (GET_RACE(ch) == RACE_HALFLING)
   max -= 45 ;
   else if (GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL)
   max +=60;
   else if (GET_RACE(ch) == RACE_HALF_OGRE)
   max +=50        ;
   
   max += ch->points.max_move;  /* move bonus */
   
   return (max);
}

/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
  int i;
  long tmp, tmp2, tmp3, bc=0;
  struct char_data *mob;
  char letter;

  extern int mob_tick_count;
  extern long mob_count;

  i = nr;
  if( type == VIRTUAL )
  {
    if( ( nr = real_mobile( nr ) ) < 0 )     
      return NULL;
  }

  fseek(mob_f, mob_index[nr].pos, 0);

  CREATE(mob, struct char_data, 1);
  if (!mob)
    return(0);

  bc = sizeof(struct char_data);
  clear_char(mob);

  mob->specials.last_direction = -1;  /* this is a fix for wander */

  /***** String data *** */
  mob->player.name = fread_string(mob_f);
  if( mob->player.name )
    bc += strlen( mob->player.name );
  mob->player.short_descr = fread_string(mob_f);
  if( mob->player.short_descr )
    bc += strlen(mob->player.short_descr);
  mob->player.long_descr = fread_string(mob_f);
  if( mob->player.long_descr )
    bc += strlen(mob->player.long_descr);
  mob->player.description = fread_string(mob_f);
  if( mob->player.description )
    bc += strlen( mob->player.description );
  mob->player.title = 0;

  /* *** Numeric data *** */

  mob->mult_att = 1.0;
  mob->specials.spellfail = 101;

  mob->specials.act = fread_number( mob_f );
  SET_BIT(mob->specials.act, ACT_ISNPC);
  if( IS_SET( mob->specials.act, ACT_POLYSELF ) )
    REMOVE_BIT( mob->specials.act, ACT_POLYSELF );

  mob->specials.affected_by = fread_number( mob_f );

  mob->specials.alignment = fread_number( mob_f );

  mob->player.iClass = CLASS_WARRIOR;

  fscanf(mob_f, " %c ", &letter);
  if (letter == 'S') 
  {
     /* SIMPLE MOB */
    fscanf(mob_f, "\n");

    tmp = fread_number( mob_f );
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.str   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.intel =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.wis   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.dex   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.con   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.chr   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );


    mob->points.hitroll = 20 - fread_number( mob_f );

    tmp = fread_number( mob_f );
    
    if (tmp > 10 || tmp < -10)
      tmp /= 10;

    mob->points.armor = 10 * tmp;

    fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
    mob->points.max_hit = dice(tmp, tmp2)+tmp3;
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;

    mob->points.max_mana = 10;
    mob->points.max_move = 50;

    tmp = fread_number( mob_f );
    if (tmp == -1) 
    {
      mob->points.gold = fread_number( mob_f );
      GET_EXP(mob) = fread_number( mob_f );
      GET_RACE(mob) = fread_number( mob_f );
      if(IsGiant(mob))
        mob->abilities.str += number(1,4);
      if(IsSmall(mob))
        mob->abilities.str -= 1;
    }
    else 
    {
      mob->points.gold = tmp;
      GET_EXP(mob) = fread_number( mob_f );
    }
    mob->specials.position = fread_number( mob_f );

    mob->specials.default_pos = fread_number( mob_f );

    tmp = fread_number( mob_f );
    if (tmp < 3) 
    {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }
    else if (tmp < 6)
    {
      mob->player.sex = tmp - 3;
      mob->immune = fread_number( mob_f );
      mob->M_immune = fread_number( mob_f );
      mob->susc = fread_number( mob_f );
    }
    else
    {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    fscanf(mob_f,"\n");

    mob->player.iClass = 0;

    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] =
                                MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
  } /* FINE SIMPLE */
  else if( letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L' )
  {  
    if( letter == 'A' || letter == 'B' || letter == 'L' )
    {
      mob->mult_att = (float)fread_number( mob_f );
    }

    fscanf(mob_f, "\n");
    tmp = fread_number( mob_f );
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.str   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.intel =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.wis   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.dex   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.con   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.chr   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );

    mob->points.hitroll = 20 - fread_number( mob_f );

    mob->points.armor = 10 * fread_number( mob_f );
    
    tmp = fread_number( mob_f );
    mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;
 
    mob->points.max_mana = 100;
    mob->points.max_move = 50;

    tmp = fread_number( mob_f );
    if (tmp == -1)
    {
      mob->points.gold = fread_number( mob_f );
      tmp = fread_number( mob_f );
      if (tmp >= 0)
        GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
      else
        GET_EXP(mob) = -tmp;
      GET_RACE(mob) = fread_number( mob_f );
      if(IsGiant(mob))
        mob->abilities.str += number(1,4);
      if(IsSmall(mob))
        mob->abilities.str -= 1;
    }
    else
    {
      mob->points.gold = tmp;

      /* this is where the new exp will come into play */
      tmp = fread_number( mob_f );
      if (tmp >= 0)
        GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
      else
        GET_EXP(mob) = -tmp;
    }

    mob->specials.position = fread_number( mob_f );

    mob->specials.default_pos = fread_number( mob_f );

    tmp = fread_number( mob_f );
    if( tmp < 3 )
    {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }
    else if( tmp < 6 )
    {
      mob->player.sex = tmp - 3;
      mob->immune = fread_number( mob_f );
      mob->M_immune = fread_number( mob_f );
      mob->susc = fread_number( mob_f );
    }
    else
    {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    /* read in the sound string for a mobile */
    if (letter == 'L')
    {
      mob->player.sounds = fread_string(mob_f);
      if (mob->player.sounds && *mob->player.sounds)
        bc += strlen(mob->player.sounds);

      mob->player.distant_snds = fread_string(mob_f);
      if (mob->player.distant_snds && *mob->player.distant_snds)
        bc += strlen(mob->player.distant_snds);
    }
    else
    {
      mob->player.sounds = 0;
      mob->player.distant_snds = 0;
    }

    if( letter == 'B' )
    {
      SET_BIT(mob->specials.act, ACT_HUGE);
    }

    mob->player.iClass = 0;

    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[ i ] = 
        MAX( 20 - GET_LEVEL( mob, WARRIOR_LEVEL_IND ), 2 );
  }
  else
  {  /* The old monsters are down below here */
    fscanf(mob_f, "\n");

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.str = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.intel = tmp;
 
    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.wis = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.dex = tmp;
 
    fscanf(mob_f, " %ld \n", &tmp);
    mob->abilities.con = tmp;


    fscanf(mob_f, " %ld ", &tmp);
    fscanf(mob_f, " %ld ", &tmp2);

    mob->points.max_hit = number(tmp, tmp2);
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.armor = 10*tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.mana = tmp;
    mob->points.max_mana = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.move = tmp;
    mob->points.max_move = tmp;
 
    fscanf(mob_f, " %ld ", &tmp);
    mob->points.gold = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    GET_EXP(mob) = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.position = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.default_pos = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->player.sex = tmp;

    mob->player.iClass = fread_number( mob_f );

    fscanf(mob_f, " %ld ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.chr = MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    
    fscanf(mob_f, " %ld ", &tmp);
    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);

    fscanf(mob_f, " %ld ", &tmp);
    mob->player.weight = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    mob->player.height = tmp;

    for (i = 0; i < 3; i++)
    {
      fscanf(mob_f, " %ld ", &tmp);
      GET_COND(mob, i) = tmp;
    }
    fscanf(mob_f, " \n ");

    for (i = 0; i < 5; i++)
    {
      fscanf(mob_f, " %ld ", &tmp);
      mob->specials.apply_saving_throw[i] = tmp;
    }

    fscanf(mob_f, " \n ");

    /* Set the damage as some standard 1d4 */
    mob->points.damroll = 0;
    mob->specials.damnodice = 1;
    mob->specials.damsizedice = 6;

    /* Calculate THAC0 as a formular of Level */
    mob->points.hitroll = MAX(1, GET_LEVEL(mob,WARRIOR_LEVEL_IND)-3);
  }

  mob->tmpabilities = mob->abilities;

  for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
    mob->equipment[i] = 0;

  mob->nr = nr;

  mob->desc = 0;

  if (!IS_SET(mob->specials.act, ACT_ISNPC))
    SET_BIT(mob->specials.act, ACT_ISNPC);

  mob->generic = 0;
  mob->commandp = 0;
  mob->commandp2 = 0;
  mob->waitp = 0;

  /* Check to see if associated with a script, if so, set it up */
  if(IS_SET(mob->specials.act, ACT_SCRIPT))
    REMOVE_BIT(mob->specials.act, ACT_SCRIPT);

/**** SALVO da decidere se controllare
  for(i = 0; i < top_of_scripts; i++)
  {
    if(gpScript_data[i].iVNum == mob_index[nr].iVNum)
    {
      SET_BIT(mob->specials.act, ACT_SCRIPT);
      mob->script = i;
      break;
    }
  }
****/

  /* insert in list */

  mob->next = character_list;
  character_list = mob;

#if LOW_GOLD
  if (mob->points.gold >= 10)
    mob->points.gold /= 5;
  else if (mob->points.gold > 0)
    mob->points.gold = 1;
#endif

  /* set up things that all members of the race have */
  SetRacialStuff(mob);

  /* change exp for wimpy mobs (lower) */
  if (IS_SET(mob->specials.act, ACT_WIMPY))
    GET_EXP(mob) -= GET_EXP(mob)/10;

  /* change exp for agressive mobs (higher) */
  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE))
  {
    GET_EXP(mob) += GET_EXP(mob)/10;
    /* big bonus for fully aggressive mobs for now */
    if (!IS_SET(mob->specials.act, ACT_WIMPY)||
        IS_SET(mob->specials.act, ACT_META_AGG))
      GET_EXP(mob) += (GET_EXP(mob)/2);
  }

  mob->points.mana = mana_limit( mob );
  mob->points.move = move_limit( mob );
  
  /* set up distributed movement system */

  mob->specials.tick = mob_tick_count++;

  if( mob_tick_count == TICK_WRAP_COUNT )
    mob_tick_count=0;

  mob_index[ nr ].number++;

  mob_count++;

  return(mob);
}

int compare_index(const void *p1, const void *p2)
{
   struct index_data *s1,*s2;
   s1=(struct index_data *) p1;
   s2=(struct index_data *) p2;
   return(s1->iVNum-s2->iVNum);
}

/* generate index table for object or monster file */
struct index_data *generate_indices( FILE *fl, int *top, int *sort_top, int *alloc_top, char *dirname )
{
   FILE *f;
   DIR *dir;
   struct index_data *index;
   struct dirent *ent;
   long i = 0, di = 0, vnum, j;
   long bc=2000;
   long dvnums[2000];               /* guess 2000 stored objects is enuff */
   int numvnum=0;
   char buf[82], tbuf[128];
   char loaded[100000];
   for (i=0;i<100000;i++) loaded[i]=0;
   i=0;

   rewind(fl);
   for (;;) 
   {
      if (fgets(buf, sizeof(buf), fl)) 
      {
	 if (*buf == '#') 
	 {
	    if (!i)                                          /* first cell */
	    CREATE(index, struct index_data, bc);
	    else if (i >= bc) 
	    {
	       if (!(index = (struct index_data*)
		     realloc(index, (i + 50) * sizeof(struct index_data)))) 
	       {
		  perror("load indices");
		  assert(0);
	       }
	       bc += 50;
	    }
	    sscanf(buf, "#%d", &index[i].iVNum );
	    numvnum=index[i].iVNum;
	    if (!loaded[numvnum]) 
	    {
	       loaded[numvnum]=1;
	       sprintf(tbuf,"%s/%d",dirname,index[i].iVNum);
	       /* Se non esiste nella dir obj/mob salvati */
	       if((f=fopen(tbuf,"rt"))==NULL) 
	       {
		  index[i].pos = ftell(fl);
		  index[i].name = (index[i].iVNum < 99999) ? 
		    fread_string(fl) : 
		  strdup("omega");
	       } 
	       else 
	       {
		  index[i].pos = -1;
		  fscanf(f, "#%*d\n");
		  index[i].name = (index[i].iVNum < 99999) ? fread_string(f) :
		  strdup("omega");
		  dvnums[di++] = index[i].iVNum;
		  fclose(f);
	       }
	       index[i].number = 0;
	       index[i].func = 0;
	       index[i].data = NULL;
	       index[i].specname = NULL;
	       index[i].specparms = NULL;
	       i++;
	    } 
		else
	    {
		   fprintf(stderr, "Numero #%d in uso\n", numvnum);
	    } 
	 }
	 else 
	 {
	    if (*buf == '%' && buf[ 1 ] == '%' )        /* EOF */
	    break;
	 }
      } 
      else 
      {
	 fprintf(stderr,"generate indices\n");
	 assert(0);
      }
   }
   *sort_top = i;
   *alloc_top = bc;
   *top = *sort_top;
   qsort(index,i,sizeof(index_data),compare_index);
   if((dir=opendir(dirname))==NULL) 
      return(index);
   while((ent=readdir(dir)) != NULL) 
   {
      if(*ent->d_name=='.') 
      continue;
      vnum=atoi(ent->d_name);
      if(vnum == 0) 
      continue;
      /* search if vnum was already sorted in main database */
      for(j=0;j<di;j++)
      if(dvnums[j] == vnum)
      break;
      if(dvnums[j] == vnum)
      continue;
      sprintf(buf,"%s/%s",dirname, ent->d_name);
      if((f=fopen(buf,"rt")) == NULL) 
			 continue;
      if (!i)
      CREATE(index, struct index_data, bc);
      else if (i >= bc) 
      {
	 if (!(index = (struct index_data*)
	       realloc(index, (i + 50) * sizeof(struct index_data)))) 
	 {
	    perror("load indices");
	    assert(0);
	 }
	 bc += 50;
      }
      fscanf(f, "#%*d\n");
      index[i].iVNum = vnum; 
      index[i].pos = -1; 
      index[i].name = (index[i].iVNum<99999)?fread_string(f):strdup("omega");
      index[i].number = 0;
      index[i].func = 0;
      index[i].data = NULL;
      fclose(f);
      i++;
   }
   *alloc_top = bc;
   *top = i;
   *sort_top = *top;
   qsort(index,i,sizeof(index_data),compare_index);
   return(index);
}

struct obj_file_u
{
  char owner[20];    /* Name of player                     */
  int gold_left;     /* Number of goldcoins left at owner  */
  int total_cost;    /* The cost for all items, per day    */
  long last_update;  /* Time in seconds, when last updated */
  long minimum_stay; /* For stasis */
  int  number;       /* number of objects */
  struct obj_file_elem objects[MAX_OBJ_SAVE];
};

char *fname(char *namelist)
{
#define ML 30
   static char holder[ML];
   int i=ML;
   register char *point;
   
   if( namelist )
   {
      for (point = holder; isalpha(*namelist) && i ; i--,namelist++, point++)
      {
	 *point = *namelist;
      }
      
      *point = '\0';
   }
   else
   holder[ 0 ] = 0;
   
   return(holder);
}

int ReadObjs( FILE *fl, struct obj_file_u *st)
{
  int i;

  if (feof(fl)) 
  {
    fclose(fl);
    return(0);
  }

  fread(st->owner, sizeof(st->owner), 1, fl);
  if (feof(fl)) {
    fclose(fl);
    return(0);
  }
  fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
  if (feof(fl)) {
    fclose(fl);
    return(0);
  }
  fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
  if (feof(fl)) {
    fclose(fl);
    return(0);
  }
  fread(&st->last_update, sizeof(st->last_update), 1, fl);
  if (feof(fl)) {
    fclose(fl);
    return(0);
  }
  fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
  if (feof(fl)) {
    fclose(fl);
    return(0);
  }
  fread(&st->number, sizeof(st->number), 1, fl);
  if (feof(fl) || st->number >256) {
    fclose(fl);
    return(0);
  }

  for (i=0;i<st->number;i++) {
     fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
  }
  return 1;
}

void checkobj(struct obj_data *j, int errlev)
{
  #undef INT_ERR
  #define INT_ERR errore, 0, obj_index[j->item_number].iVNum, j->name
  char buf[MAX_STRING_LENGTH];
  int  ciclo, errore =0;

  if (j && j->name && strcmp(j->name,"%%"))
  {
	  if (j->in_room < -1 || j->in_room >WORLD_SIZE-1)
  	  {
		sprintf(buf, "in_room ______________________: %d (< -1 o >%d)", j->in_room, WORLD_SIZE-1);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (0)
  	  {
      	sprintf(buf, "obj_flags.value[0] ___________: %d", j->obj_flags.value[0]); log(buf);
	  }
	  if (0)
  	  {
      	sprintf(buf, "obj_flags.value[1] ___________: %d", j->obj_flags.value[1]); log(buf);
	  }
	  if (0)
  	  {
      	sprintf(buf, "obj_flags.value[2] ___________: %d", j->obj_flags.value[2]); log(buf);
	  }
	  if (0)
  	  {
      	sprintf(buf, "obj_flags.value[3] ___________: %d", j->obj_flags.value[3]); log(buf);
	  }
	  if (j->obj_flags.type_flag <1 || j->obj_flags.type_flag >26)
  	  {
      	sprintf(buf, "obj_flags.type_flag __________: %d (<1 o >26)", j->obj_flags.type_flag);
      	errore =prterr(INT_ERR, buf);
	  }

	  if (j->obj_flags.wear_flags >524288 || j->obj_flags.wear_flags ==65536)
  	  {
      	sprintf(buf, "obj_flags.wear_flags _________: %ld (>524288 o =65536)", j->obj_flags.wear_flags);
      	errore =prterr(INT_ERR, buf);
	  }

	  if (j->obj_flags.extra_flags >(long unsigned)0x80000000)
  	  {
      	sprintf(buf, "obj_flags.extra_flags ________: %ld (>2147483648)", j->obj_flags.extra_flags);
      	errore =prterr(INT_ERR, buf);
	  }
	  if ((unsigned short int)j->obj_flags.weight >65534)
  	  {
      	sprintf(buf, "obj_flags.weight _____________: %d (>65534)", j->obj_flags.weight);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (0)
  	  {
      	sprintf(buf, "obj_flags.cost _______________: %d", j->obj_flags.cost); log(buf);
	  }
	  if (j->obj_flags.cost_per_day < -1)
  	  {
      	sprintf(buf, "obj_flags.cost_per_day _______: %d (<0)", j->obj_flags.cost_per_day);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (j->obj_flags.timer <0)
  	  {
      	sprintf(buf, "obj_flags.timer ______________: %d (<0)", j->obj_flags.timer);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (j->obj_flags.bitvector <0x00000000 || (unsigned)j->obj_flags.bitvector >0x80000000)
	  {
      	sprintf(buf, "obj_flags.bitvector __________: %ld", j->obj_flags.bitvector);
      	errore =prterr(INT_ERR, buf);
	  }
	  for (ciclo =0; ciclo <=MAX_OBJ_AFFECT; ciclo++)
	  {
		if (j->affected[ciclo].location <0 || j->affected[ciclo].location >65)
	  	{
      		sprintf(buf, "affected[%d].location _________: %d (<0 o >65)", ciclo, j->affected[ciclo].location);
      		errore =prterr(INT_ERR, buf);
	  	}
		if (0)
	  	{
      		sprintf(buf, "affected[%d].modifier _________: %ld", ciclo, j->affected[ciclo].modifier); log(buf);
	  	}
	  }
	  if (0)
	  {
      	sprintf(buf, "sector _______________________: %d", j->sector); log(buf);
	  }
	  if (j->char_vnum <0L)
	  {
      	sprintf(buf, "char_vnum ____________________: %ld (<0)", j->char_vnum);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (strlen(j->oldfilename) >20)
	  {
      	sprintf(buf, "oldfilename __________________: %s (%d >20 car.)", j->oldfilename, strlen(j->oldfilename));
      	errore =prterr(INT_ERR, buf);
	  }
	  if (j->action_description)
	  {
	  	if (strlen(j->action_description) >80)
	  	{
      		sprintf(buf, "action_description ___________: %s (%d >80 car.)", j->action_description, strlen(j->action_description));
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
      if (j->ex_description)
	  {
		if (j->ex_description->nMagicNumber <1234 || (j->ex_description->nMagicNumber >1236 && j->ex_description->nMagicNumber !=2345))
	  	{
      		sprintf(buf, "ex_description->nMagicNumber _: %d (<1234 o >1236 e !=2345)", j->ex_description->nMagicNumber);
      		errore =prterr(INT_ERR, buf);
	  	}
		if (j->ex_description->keyword)
	  	{
			if (strlen(j->ex_description->keyword) >80)
	  		{
      			sprintf(buf, "ex_description->keyword ______: %s (%d >80 car.)", j->ex_description->keyword, strlen(j->ex_description->keyword));
      			errore =prterr(INT_ERR, buf);
	  		}
	  	}
		if (j->ex_description->description)
	  	{
			if (strlen(j->ex_description->description) >MAX_STRING_LENGTH)
	  		{
      			sprintf(buf, "ex_description->description __: %s (%d >%d car.)", j->ex_description->description, strlen(j->ex_description->description),MAX_STRING_LENGTH);
      			errore =prterr(INT_ERR, buf);
	  		}
	  	}
		if (j->ex_description->next)
	  	{
  			struct extra_descr_data *desc;

        	for (desc = j->ex_description->next; desc; desc = desc->next) 
        	{
				if (desc->description)
	  			{
					if (strlen(desc->description) >MAX_STRING_LENGTH)
	  				{
      					sprintf(buf, "ex_description->next->description __: %s (%d >%d car.)", desc->description, strlen(desc->description),MAX_STRING_LENGTH);
      					errore =prterr(INT_ERR, buf);
        			}
        		}
	  		}
	  	}
	  }
	  if (0)
	  {
      	sprintf(buf, "carried_by ___________________: %s", (j->carried_by) ? j->carried_by->player.short_descr : "(null)"); log(buf);
	  }
	  if (j->eq_pos < -1 || j->eq_pos >23)
	  {
      	sprintf(buf, "eq_pos _______________________: %d (< -1 o >23)", j->eq_pos);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "equipped_by __________________: %s", (j->equipped_by) ? j->equipped_by->player.short_descr : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "in_obj _______________________: %s", (j->in_obj) ? j->in_obj->name : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "contains _____________________: %s", (j->contains) ? j->contains->name : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "next_content _________________: %s", (j->next_content) ? j->next_content->name : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "next->name ___________________: %s", (j->next) ? j->next->name : "(null)"); log(buf);
	  }
	  if (j->szForbiddenWearToChar)
	  {
	  	if (strlen(j->szForbiddenWearToChar) >80)
	  	{
      		sprintf(buf, "szForbiddenWearToChar ________: %s (%d >80 car.)", j->szForbiddenWearToChar, strlen(j->szForbiddenWearToChar));
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
	  if (j->szForbiddenWearToRoom)
	  {
	  	if (strlen(j->szForbiddenWearToRoom) >80)
	  	{
      		sprintf(buf, "szForbiddenWearToRoom ________: %s (%d >80 car.)", j->szForbiddenWearToRoom, strlen(j->szForbiddenWearToRoom));
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
	  if (0)
	  {
      	sprintf(buf, "iGeneric _____________________: %d", j->iGeneric); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "iGeneric1 ____________________: %d", j->iGeneric1); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "iGeneric2 ____________________: %d", j->iGeneric2); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "pGeneric _____________________: %s", (j->pGeneric) ? "ATTENZIONE c'e' assegnata una procedura speciale" : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "pGeneric1 ____________________: %s", (j->pGeneric1) ? "ATTENZIONE c'e' assegnata una procedura speciale" : "(null)"); log(buf);
	  }
	  if (0)
	  {
      	sprintf(buf, "pGeneric2 ____________________: %s", (j->pGeneric2) ? "ATTENZIONE c'e' assegnata una procedura speciale" : "(null)"); log(buf);
	  }
	  if ((unsigned short int)j->item_number >65535)
  	  {
      	sprintf(buf, "item_number __________________: %d (>65535)", obj_index[j->item_number].iVNum);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (j->description)
	  {
	  	if (strlen(j->description) >MAX_STRING_LENGTH)
	  	{
      		sprintf(buf, "description __________________: %s (%d >%d car.)", j->description, strlen(j->description),MAX_STRING_LENGTH);
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
	  if(j->short_description)
	  {
	  	if (strlen(j->short_description) >80)
	  	{
      		sprintf(buf, "short_description ____________: %s (%d >80 car.)", j->short_description, strlen(j->short_description));
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
	  if (j->name)
	  {
		if (strlen(j->name) >80)
	  	{
      		sprintf(buf, "name _________________________: %s (%d >80 car.)", j->name, strlen(j->name));
      		errore =prterr(INT_ERR, buf);
	  	}
	  }
	  else
	  {
      	sprintf(buf, "name _________________________: %s", j->name);
      	errore =prterr(INT_ERR, buf);
	  }
	  if (errore)
      	log("-------------------------------------------------------------------------------}");
	}
}

/**** SALVO attenzione non e' implementato il controllo sugli script ****/
void checkmob(struct char_data *ch, int errlev)
{
  #undef INT_ERR
  #define INT_ERR errore, 0, mob_index[ch->nr].iVNum, ch->player.name
  char buf[MAX_STRING_LENGTH];
  int  ciclo, errore =0;

  if (ch && ch->player.name && strcmp(ch->player.name,"%%"))
  {
	if ((ch->nMagicNumber <1234 || (ch->nMagicNumber >1236 && ch->nMagicNumber !=2345))&& ch->nMagicNumber !=0)
	{
    	sprintf(buf, "nMagicNumber _______________________: %d (<1234 o >1236 e !=2345)", ch->nMagicNumber);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->nr <0 || ch->nr >99998)
	{
    	sprintf(buf, "nr _________________________________: %ld (<0 o >99998)", ch->nr);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->in_room < -1 || ch->in_room >WORLD_SIZE-1)
  	{
		sprintf(buf, "in_room ____________________________: %ld (< -1 o >%d)", ch->in_room, WORLD_SIZE-1);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->term <0)
  	{
		sprintf(buf, "term _______________________________: %d (<0)", ch->term);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->size <0)
  	{
		sprintf(buf, "size _______________________________: %d (<0)", ch->size);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.mana <0 || ch->last.mana >1000)
  	{
		sprintf(buf, "last.mana __________________________: %d (<0 o >1000)", ch->last.mana);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.mmana <0 || ch->last.mmana >1000)
  	{
		sprintf(buf, "last.mmana _________________________: %d (<0 o >1000)", ch->last.mmana);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.hit <0 || ch->last.hit >1000)
  	{
		sprintf(buf, "last.hit ___________________________: %d (<0 o >1000)", ch->last.hit);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.mhit <0 || ch->last.mhit >1000)
  	{
		sprintf(buf, "last.mhit __________________________: %d (<0 o >1000)", ch->last.mhit);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.move <0 || ch->last.move >1000)
  	{
		sprintf(buf, "last.move __________________________: %d (<0 o >1000)", ch->last.move);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.mmove <0 || ch->last.mmove >1000)
  	{
		sprintf(buf, "last.mmove _________________________: %d (<0 o >1000)", ch->last.mmove);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.exp <0 || ch->last.exp >1000000)
  	{
		sprintf(buf, "last.exp ___________________________: %d (<0 o >1M)", ch->last.exp);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->last.gold <0 || ch->last.gold >1000000)
  	{
		sprintf(buf, "last.gold __________________________: %d (<0 o >1M)", ch->last.gold);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->immune >131072)
  	{
		sprintf(buf, "immune _____________________________: %u (>131072)", ch->immune);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->M_immune >131072)
  	{
		sprintf(buf, "M_immune ___________________________: %u (>131072)", ch->M_immune);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->susc >131072)
  	{
		sprintf(buf, "susc _______________________________: %u (>131072)", ch->susc);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->mult_att <0.0 || ch->mult_att >10.0)
  	{
		sprintf(buf, "mult_att ___________________________: %f (<0 o >10)", ch->mult_att);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->attackers <0)
  	{
		sprintf(buf, "attackers __________________________: %d (<0)", ch->attackers);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->sector <0 || ch->sector >12)
  	{
		sprintf(buf, "sector _____________________________: %d (<0 o >12)", ch->sector);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "generic ____________________________: %d ()", ch->generic);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->commandp <0)
  	{
		sprintf(buf, "commandp ___________________________: %d (<0)", ch->commandp);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "waitp ______________________________: %d ()", ch->waitp);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->commandp2 <0)
  	{
		sprintf(buf, "commandp2 __________________________: %d (<0)", ch->commandp2);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->script <0)
  	{
		sprintf(buf, "script _____________________________: %d (<0)", ch->script);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->race <0 || ch->sector >MAX_RACE)
  	{
		sprintf(buf, "race _______________________________: %d (<0 o >%d)", ch->race, MAX_RACE);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->hunt_dist <0 || ch->hunt_dist >100)
  	{
		sprintf(buf, "hunt_dist __________________________: %d (<0 o >100)", ch->hunt_dist);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->hatefield >64)
  	{
		sprintf(buf, "hatefield __________________________: %u (>64)", ch->hatefield);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->fearfield >64)
  	{
		sprintf(buf, "fearfield __________________________: %u (>64)", ch->fearfield);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->hates.clist)
 	{
		sprintf(buf, "hates.clist ________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
		if (ch->hates.clist->op_ch)
		{
			sprintf(buf, "hates.clist->op_ch _________________: %s (puntatore inizializzato)", "ATTENZIONE");
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.clist->name)
		{
			if (strlen(ch->hates.clist->name) >50)
			{
				sprintf(buf, "hates-clist->name ____________: %s (%d >50 car.)", ch->hates.clist->name, strlen(ch->hates.clist->name));
				errore =prterr(INT_ERR, buf);
			}
		}
		else
		{
			sprintf(buf, "hates-clist->name __________________: %s", ch->hates.clist->name);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.clist->next)
		{
			sprintf(buf, "hates.clist->next __________________: %s (puntatore inizializzato)", "ATTENZIONE");
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.sex <1 || ch->hates.sex >7)
		{
			sprintf(buf, "hates.sex __________________________: %d (<1 o >7)", ch->hates.sex);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.race <0 || ch->hates.race >MAX_RACE)
		{
			sprintf(buf, "hates.race _________________________: %d (<0 o >%d)", ch->hates.race, MAX_RACE);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.iClass <0 || ch->hates.iClass >1024)
		{
			sprintf(buf, "hates.iClass _______________________: %d (<0 o >1024)", ch->hates.iClass);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.vnum <0 || ch->hates.vnum >WORLD_SIZE-1)
		{
			sprintf(buf, "hates.vnum _________________________: %d (<0 o >%d)", ch->hates.vnum, WORLD_SIZE-1);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.evil < -1000 || ch->hates.evil >1000)
		{
			sprintf(buf, "hates.evil _________________________: %d (< -1000 o >1000)", ch->hates.evil);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.good < -1000 || ch->hates.good >1000)
		{
			sprintf(buf, "hates.good _________________________: %d (< -1000 o >1000)", ch->hates.good);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->fears.clist)
 	{
		sprintf(buf, "fears.clist ________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
		if (ch->fears.clist->op_ch)
		{
			sprintf(buf, "fears.clist->op_ch _________________: %s (puntatore inizializzato)", "ATTENZIONE");
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.clist->name)
		{
			if (strlen(ch->fears.clist->name) >50)
			{
				sprintf(buf, "fears-clist->name ____________: %s (%d >50 car.)", ch->fears.clist->name, strlen(ch->fears.clist->name));
				errore =prterr(INT_ERR, buf);
			}
		}
		else
		{
			sprintf(buf, "fears-clist->name __________________: %s", ch->fears.clist->name);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->hates.clist->next)
		{
			sprintf(buf, "fears.clist->next __________________: %s (puntatore inizializzato)", "ATTENZIONE");
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.sex <1 || ch->fears.sex >7)
		{
			sprintf(buf, "fears.sex __________________________: %d (<1 o >7)", ch->fears.sex);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.race <0 || ch->fears.race >MAX_RACE)
		{
			sprintf(buf, "fears.race _________________________: %d (<0 o >%d)", ch->fears.race, MAX_RACE);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.iClass <0 || ch->fears.iClass >1024)
		{
			sprintf(buf, "fears.iClass _______________________: %d (<0 o >1024)", ch->fears.iClass);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.vnum <0 || ch->fears.vnum >WORLD_SIZE-1)
		{
			sprintf(buf, "fears.vnum _________________________: %d (<0 o >%d)", ch->fears.vnum, WORLD_SIZE-1);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.evil < -1000 || ch->fears.evil >1000)
		{
			sprintf(buf, "fears.evil _________________________: %d (< -1000 o >1000)", ch->fears.evil);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->fears.good < -1000 || ch->fears.good >1000)
		{
			sprintf(buf, "fears.good _________________________: %d (< -1000 o >1000)", ch->fears.good);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (0)
	{
		sprintf(buf, "persist ____________________________: %d ()", ch->persist);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->old_room <0 || ch->old_room >WORLD_SIZE-1)
	{
		sprintf(buf, "old_room ___________________________: %d (<0 o >%d)", ch->old_room, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->act_ptr)
 	{
		sprintf(buf, "act_ptr ____________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->player.sex >7)
	{
		sprintf (buf, "player.sex _________________________: %u (>7)", ch->player.sex);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.weight <0 || ch->player.weight >900)
	{
		sprintf (buf, "player.weight ______________________: %d (<0 o >900)", ch->player.weight);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.height <0 || ch->player.height >620)
	{
		sprintf (buf, "player.height ______________________: %d (<0 o >620)", ch->player.height);
		errore =prterr(INT_ERR, buf);
	}
	for (ciclo =0; ciclo <MAX_TOUNGE; ciclo++)
	{
		if (ch->player.talks[ciclo] !=0 && ch->player.talks[ciclo] !=1)
		{
			sprintf (buf, "player.talks[%1d] ____________________: %d (<0 o >1)", ciclo, ch->player.talks[ciclo]);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.user_flags <0 || ch->player.user_flags >8192)
	{
		sprintf (buf, "player.user_flags __________________: %ld (<0 o >8192)", ch->player.user_flags);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.speaks <0 || ch->player.speaks >8)
	{
		sprintf (buf, "player.speaks ______________________: %d (<0 o >8)", ch->player.speaks);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.name)
	{
		if (strlen(ch->player.name) >20)
		{
			sprintf (buf, "player.name ________________________: %s (%d >20 car.)", ch->player.name,strlen(ch->player.name));
			errore =prterr(INT_ERR, buf);
		}
	}
	else
	{
		sprintf (buf, "player.name ________________________: (null)");
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.vassallodi)
	{
		if (strlen(ch->player.vassallodi) >20)
		{
			sprintf (buf, "player.vassallodi __________________: %s (%d >20 car.)", ch->player.vassallodi,strlen(ch->player.vassallodi));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.short_descr)
	{
		if (strlen(ch->player.short_descr) >80)
		{
			sprintf (buf, "player.short_descr _________________: %s (%d >80 car.)", ch->player.short_descr,strlen(ch->player.short_descr));
			errore =prterr(INT_ERR, buf);
		}
	}
	else
	{
		sprintf (buf, "player.short_descr _________________: (null)");
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.long_descr)
	{
		if (strlen(ch->player.long_descr) >80)
		{
			sprintf (buf, "player.long_descr __________________: %s (%d >80 car.)", ch->player.long_descr,strlen(ch->player.long_descr));
			errore =prterr(INT_ERR, buf);
		}
	}
	else
	{
		sprintf (buf, "player.long_descr __________________: (null)");
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.description)
	{
		if (strlen(ch->player.description) >240)
		{
			sprintf (buf, "player.description _________________: %s (%d >240 car.)", ch->player.description,strlen(ch->player.description));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.title)
	{
		if (strlen(ch->player.title) >80)
		{
			sprintf (buf, "player.title _______________________: %s (%d >80 car.)", ch->player.title,strlen(ch->player.title));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.sounds)
	{
		if (strlen(ch->player.sounds) >80)
		{
			sprintf (buf, "player.sounds ______________________: %s (%d >80 car.)", ch->player.sounds,strlen(ch->player.sounds));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.distant_snds)
	{
		if (strlen(ch->player.distant_snds) >80)
		{
			sprintf (buf, "player.distant_snds ________________: %s (%d >80 car.)", ch->player.distant_snds,strlen(ch->player.distant_snds));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->player.iClass <0 || ch->player.iClass >2048)
	{
		sprintf (buf, "player.iClass ______________________: %d (<0 o >2048)", ch->player.iClass);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.hometown <0 || ch->player.hometown >WORLD_SIZE-1)
	{
		sprintf (buf, "player.hometown ____________________: %d (<0 o >%d)", ch->player.hometown, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "player.extra_flags _________________: %ld ()", ch->player.extra_flags); log(buf);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.time.birth ==(time_t)-1)
	{
		sprintf (buf, "player.time.birth __________________: %ld ()", ch->player.time.birth); log(buf);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.time.birth ==(time_t)-1)
	{
		sprintf (buf, "player.time.logon __________________: %ld ()", ch->player.time.logon); log(buf);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->player.time.played <0)
	{
		sprintf (buf, "player.time.played _________________: %d (<0)", ch->player.time.played); log(buf);
		errore =prterr(INT_ERR, buf);
	}
	for (ciclo =0; ciclo <ABS_MAX_CLASS; ciclo++)
	{
		if (ch->player.level[ciclo] >60)
		{
			sprintf (buf, "player.level[%2d] ___________________: %u (>60)", ciclo, ch->player.level[ciclo]);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->abilities.str <0 || ch->abilities.str >25)
	{
		sprintf(buf, "abilities.str ______________________: %d (<0 o >25)", ch->abilities.str);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.str_add <0 || ch->abilities.str_add >100)
	{
		sprintf(buf, "abilities.str_add __________________: %d (<0 o >100)", ch->abilities.str_add);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.intel <0 || ch->abilities.intel >25)
	{
		sprintf(buf, "abilities.intel ____________________: %d (<0 o >25)", ch->abilities.intel);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.wis <0 || ch->abilities.wis >25)
	{
		sprintf(buf, "abilities.wis ______________________: %d (<0 o >25)", ch->abilities.wis);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.dex <0 || ch->abilities.dex >25)
	{
		sprintf(buf, "abilities.dex ______________________: %d (<0 o >25)", ch->abilities.dex);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.con <0 || ch->abilities.con >25)
	{
		sprintf(buf, "abilities.con ______________________: %d (<0 o >25)", ch->abilities.con);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->abilities.chr <0 || ch->abilities.chr >25)
	{
		sprintf(buf, "abilities.chr ______________________: %d (<0 o >25)", ch->abilities.chr);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf(buf, "abilities.extra ____________________: %d", ch->abilities.extra); log(buf);
	}
	if (0)
	{
		sprintf(buf, "abilities.extra2 ___________________: %d", ch->abilities.extra2); log(buf);
	}
	if (ch->tmpabilities.str <0 || ch->tmpabilities.str >25)
	{
		sprintf(buf, "tmpabilities.str ___________________: %d (<0 o >25)", ch->tmpabilities.str);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.str_add <0 || ch->tmpabilities.str_add >100)
	{
		sprintf(buf, "tmpabilities.str_add _______________: %d (<0 o >100)", ch->tmpabilities.str_add);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.intel <0 || ch->tmpabilities.intel >25)
	{
		sprintf(buf, "tmpabilities.intel _________________: %d (<0 o >25)", ch->tmpabilities.intel);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.wis <0 || ch->tmpabilities.wis >25)
	{
		sprintf(buf, "tmpabilities.wis ___________________: %d (<0 o >25)", ch->tmpabilities.wis);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.dex <0 || ch->tmpabilities.dex >25)
	{
		sprintf(buf, "tmpabilities.dex ___________________: %d (<0 o >25)", ch->tmpabilities.dex);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.con <0 || ch->tmpabilities.con >25)
	{
		sprintf(buf, "tmpabilities.con ___________________: %d (<0 o >25)", ch->tmpabilities.con);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->tmpabilities.chr <0 || ch->tmpabilities.chr >25)
	{
		sprintf(buf, "tmpabilities.chr ___________________: %d (<0 o >25)", ch->tmpabilities.chr);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf(buf, "tmpabilities.extra _________________: %d", ch->tmpabilities.extra); log(buf);
	}
	if (0)
	{
		sprintf(buf, "tmpabilities.extra2 ________________: %d", ch->tmpabilities.extra2); log(buf);
	}
	if (ch->affected)
 	{
		if (ch->affected->type < -1 || ch->affected->type >288)
		{
			sprintf (buf, "affected->type _____________________: %d (< -1 o >288)", ch->affected->type);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->affected->duration <0 || ch->affected->duration >300)
		{
			sprintf (buf, "affected->duration _________________: %d (<0 o >300)", ch->affected->duration);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->affected->modifier < -100 || ch->affected->modifier >10500)
		{
			sprintf (buf, "affected->modifier _________________: %ld (< -100 o >10500)", ch->affected->modifier);
			errore =prterr(INT_ERR, buf);
		}
		if ((long unsigned)ch->affected->bitvector >(long unsigned)0x80000000)
		{
			sprintf (buf, "affected->bitvector ________________: %lu (>%lu)", (long unsigned)ch->affected->bitvector,(long unsigned)0x80000000);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->affected->next)
		{
			sprintf (buf, "affected->next _____________________: ATTENZIONE puntatore inizializzato"); log(buf);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->points.mana <0 || ch->points.mana >1000)
	{
		sprintf (buf, "points.mana ________________________: %d (<0 o >1000)", ch->points.mana);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.max_mana <0 || ch->points.max_mana >1000)
	{
		sprintf (buf, "points.max_mana ____________________: %d (<0 o >1000)", ch->points.max_mana);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.mana_gain >1000)
	{
		sprintf (buf, "points.mana_gain ___________________: %u (>1000)", ch->points.mana_gain);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.hit <0 || ch->points.hit >1000)
	{
		sprintf (buf, "points.hit _________________________: %d (<0 o >1000)", ch->points.hit);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.max_hit <0 || ch->points.max_hit >1000)
	{
		sprintf (buf, "points.max_hit _____________________: %d (<0 o >1000)", ch->points.max_hit);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.hit_gain >1000)
	{
		sprintf (buf, "points.hit_gain ____________________: %u (>1000)", ch->points.hit_gain);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.move <0 || ch->points.move >1000)
	{
		sprintf (buf, "points.move ________________________: %d (<0 o >1000)", ch->points.move);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.max_move <0 || ch->points.max_move >1000)
	{
		sprintf (buf, "points.max_move ____________________: %d (<0 o >1000)", ch->points.max_move);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.move_gain >1000)
	{
		sprintf (buf, "points.move_gain ___________________: %u (>1000)", ch->points.move_gain);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "points.extra1 ______________________: %d", ch->points.extra1); log(buf);
	}
	if (0)
	{
		sprintf (buf, "points.extra2 ______________________: %d", ch->points.extra2); log(buf);
	}
	if (0)
	{
		sprintf (buf, "points.extra3 ______________________: %u", ch->points.extra3); log(buf);
	}
	if (ch->points.armor < -100 || ch->points.armor >100)
	{
		sprintf (buf, "points.armor _______________________: %d (< -100 o >100)", ch->points.armor);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.gold <0 || ch->points.gold >2000000)
	{
		sprintf (buf, "points.gold ________________________: %d (<0 0 >2M)", ch->points.gold);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.bankgold <0 || ch->points.bankgold >4000000)
	{
		sprintf (buf, "points.bankgold ____________________: %d (<0 0 >4M)", ch->points.bankgold);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "points.true_exp ____________________: %d", ch->points.true_exp); log(buf);
	}
	if (0)
	{
		sprintf (buf, "points.extra_dual __________________: %d", ch->points.extra_dual); log(buf);
	}
	if (ch->points.hitroll < -50 || ch->points.hitroll >50)
	{
		sprintf (buf, "points.hitroll _____________________: %d (< -50 o >50)", ch->points.hitroll);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.damroll < -50 || ch->points.damroll >50)
	{
		sprintf (buf, "points.damroll _____________________: %d (< -50 o >50)", ch->points.damroll);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->points.pQuest <0 || ch->points.pQuest >100)
	{
		sprintf (buf, "points.pQuest ______________________: %d (<0 o >100)", ch->points.pQuest);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
 	{
		sprintf(buf, "specials ___________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.spellfail !=101)
	{
		sprintf (buf, "specials.spellfail _________________: %d (!=101)", ch->specials.spellfail);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.tick ______________________: %u ()", ch->specials.tick);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.pmask >2)
	{
		sprintf (buf, "specials.pmask _____________________: %u (>2)", ch->specials.pmask);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.position >MAX_POSITION)
	{
		sprintf (buf, "specials.position __________________: %u (>%d)", ch->specials.position, MAX_POSITION);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.position <1)
	{
		sprintf (buf, "specials.position __________________: morto (%s)", "?");
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.default_pos >MAX_POSITION)
	{
		sprintf (buf, "specials.default_pos _______________: %u (>%d)", ch->specials.default_pos, MAX_POSITION);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.default_pos <1)
	{
		sprintf (buf, "specials.default_pos _______________: morto (%s)", "?");
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.spells_to_learn <0 || ch->specials.spells_to_learn >99)
	{
		sprintf (buf, "specials.spells_to_learn ___________: %d (<0 o >99)", ch->specials.spells_to_learn);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.carry_items !=0)
	{
		sprintf (buf, "specials.carry_items _______________: %u (!=0)", ch->specials.carry_items);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.last_direction < -1 || ch->specials.last_direction >6)
	{
		sprintf (buf, "specials.last_direction ____________: %d (< -1 o >6)", ch->specials.last_direction);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.sev >0x8000)
	{
		sprintf (buf, "specials.sev _______________________: %u (>%d)", ch->specials.sev, 0x8000);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.start_room <0 || ch->specials.start_room >WORLD_SIZE-1)
	{
		sprintf(buf, "specials.start_room ________________: %d (<0 o >%d)", ch->specials.start_room, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.edit <0 || ch->specials.edit >33)
	{
		sprintf (buf, "specials.edit ______________________: %d (<0 o >33)", ch->specials.edit);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.mobtype ___________________: %u ()", ch->specials.mobtype);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.exp_flag >100L)
	{
		sprintf (buf, "specials.exp_flag __________________: %lu (>100)", ch->specials.exp_flag);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.hp_num_dice _______________: %d ()", ch->specials.hp_num_dice);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.hp_size_dice ______________: %u ()", ch->specials.hp_size_dice);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.hp_bonus_hps ______________: %u ()", ch->specials.hp_bonus_hps);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.damnodice >90)
	{
		sprintf (buf, "specials.damnodice _________________: %u (>90)", ch->specials.damnodice);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.damsizedice >90)
	{
		sprintf (buf, "specials.damsizedice _______________: %u (>90)", ch->specials.damsizedice);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.dam_bonus _________________: %u ()", ch->specials.dam_bonus);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.medit >33)
	{
		sprintf (buf, "specials.medit _____________________: %d (>33)", ch->specials.medit);
		errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *mobedit;            /*  mob editing */
	if (ch->specials.mobedit)
 	{
		sprintf(buf, "specials.mobedit ___________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.oedit >33)
	{
		sprintf (buf, "specials.oedit _____________________: %d (>33)", ch->specials.oedit);
		errore =prterr(INT_ERR, buf);
	}
//	  struct obj_data *objedit;             /*  object editing */
	if (ch->specials.objedit)
 	{
		sprintf(buf, "specials.objedit ___________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.tick_to_lag _______________: %d ()", ch->specials.tick_to_lag);
		errore =prterr(INT_ERR, buf);
	}
	for (ciclo =0; ciclo <MAX_CONDITIONS; ciclo++)
	{
		if (ch->specials.conditions[ciclo] < -1 || ch->specials.conditions[ciclo] >24)
		{
			sprintf (buf, "specials.conditions[%1d] _____________: %d (< -1 o >24)", ciclo, ch->specials.conditions[ciclo]);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (0)
	{
		sprintf (buf, "specials.permissions _______________: %d ()", ch->specials.permissions);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.zone <0 || ch->specials.zone >999998)
	{
		sprintf (buf, "specials.zone ______________________: %d (<0 o >999998)", ch->specials.zone);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.carry_weight !=0)
	{
		sprintf (buf, "specials.carry_weight ______________: %d (!=0)", ch->specials.carry_weight);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.timer <0)
	{
		sprintf (buf, "specials.timer _____________________: %d (<0)", ch->specials.timer);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.was_in_room < -1 || ch->specials.was_in_room >WORLD_SIZE-1)
	{
		sprintf(buf, "specials.was_in_room _______________: %d (< -1 o >%d)", ch->specials.was_in_room, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if ((ch->specials.attack_type <311 && ch->specials.attack_type != -1 && ch->specials.attack_type !=0) || ch->specials.attack_type >331)
	{
		sprintf(buf, "specials.attack_type _______________: %d (!=0 o != -1 e <311 o >331)", ch->specials.attack_type);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.alignment < -1000 || ch->specials.alignment >1000)
	{
		sprintf (buf, "specials.alignment _________________: %d (< -1000 o >1000)", ch->specials.alignment);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.poofin)
	{
		if (strlen(ch->specials.poofin) >255)
		{
			sprintf (buf, "specials.poofin ____________________: %s (%d >255 car.)", ch->specials.poofin,strlen(ch->specials.poofin));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.poofout)
	{
		if (strlen(ch->specials.poofout) >255)
		{
			sprintf (buf, "specials.poofout ___________________: %s (%d >255 car.)", ch->specials.poofout,strlen(ch->specials.poofout));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.prompt)
	{
		if (strlen(ch->specials.prompt) >512)
		{
			sprintf (buf, "specials.prompt ____________________: %s (%d >512 car.)", ch->specials.prompt,strlen(ch->specials.prompt));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.lastversion)
	{
		if (strlen(ch->specials.lastversion) >254)
		{
			sprintf (buf, "specials.lastversion _______________: %s (%d >254 car.)", ch->specials.lastversion,strlen(ch->specials.lastversion));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.email)
	{
		if (strlen(ch->specials.email) >254)
		{
			sprintf (buf, "specials.email _____________________: %s (%d >254 car.)", ch->specials.email,strlen(ch->specials.email));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.realname)
	{
		if (strlen(ch->specials.realname) >254)
		{
			sprintf (buf, "specials.realname __________________: %s (%d >254 car.)", ch->specials.realname,strlen(ch->specials.realname));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.authcode)
	{
		if (strlen(ch->specials.authcode) >7)
		{
			sprintf (buf, "specials.authcode __________________: %s (%d >7 car.)", ch->specials.authcode,strlen(ch->specials.authcode));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.AuthorizedBy)
	{
		if (strlen(ch->specials.AuthorizedBy) >20)
		{
			sprintf (buf, "specials.AuthorizedBy ______________: %s (%d >20 car.)", ch->specials.AuthorizedBy,strlen(ch->specials.AuthorizedBy));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.supporting)
	{
		if (strlen(ch->specials.supporting) >20)
		{
			sprintf (buf, "specials.supporting ________________: %s (%d >20 car.)", ch->specials.supporting,strlen(ch->specials.supporting));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.bodyguarding)
	{
		if (strlen(ch->specials.bodyguarding) >20)
		{
			sprintf (buf, "specials.bodyguarding ______________: %s (%d >20 car.)", ch->specials.bodyguarding,strlen(ch->specials.bodyguarding));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.bodyguard)
	{
		if (strlen(ch->specials.bodyguard) >20)
		{
			sprintf (buf, "specials.bodyguard _________________: %s (%d >20 car.)", ch->specials.bodyguard,strlen(ch->specials.bodyguard));
			errore =prterr(INT_ERR, buf);
		}
	}
	for (ciclo =0; ciclo <MAX_POSITION; ciclo++)
	{
		if (ch->specials.TempoPassatoIn[ciclo] !=0)
		{
			sprintf (buf, "specials.TempoPassatoIn[%1d] _________: %ld (!=0)", ciclo, ch->specials.TempoPassatoIn[ciclo]);
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.PosPrev <0 || ch->specials.PosPrev >MAX_POSITION)
	{
		sprintf (buf, "specials.PosPrev ___________________: %d (<0 o >%d)", ch->specials.PosPrev, MAX_POSITION);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.WimpyLevel <0)
	{
		sprintf (buf, "specials.WimpyLevel ________________: %d (<0)", ch->specials.WimpyLevel);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.Sextra)
	{
		if (strlen(ch->specials.Sextra) >254)
		{
			sprintf (buf, "specials.Sextra ____________________: %s (%d >254 car.)", ch->specials.Sextra,strlen(ch->specials.Sextra));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.group_name)
	{
		if (strlen(ch->specials.group_name) >256)
		{
			sprintf (buf, "specials.group_name ________________: %s (%d >256 car.)", ch->specials.group_name,strlen(ch->specials.group_name));
			errore =prterr(INT_ERR, buf);
		}
	}
	if (ch->specials.A_list)
 	{
		for (ciclo =0; ciclo <10; ciclo++) /* Initialisering Ok */
 		{
			if (ch->specials.A_list->com[ciclo])
			{
				if (strlen(ch->specials.A_list->com[ciclo]) >80)
				{
					sprintf (buf, "specials.A_list->com[%d] ________: %s (%d >80 car.)", ciclo, ch->specials.A_list->com[ciclo],strlen(ch->specials.A_list->com[ciclo]));
					errore =prterr(INT_ERR, buf);
				}
			}
		}
	}
//	  struct char_data *misc;  
	if (ch->specials.misc)
 	{
		sprintf(buf, "specials.misc ______________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *fighting; /* Opponent                          */
	if (ch->specials.fighting)
 	{
		sprintf(buf, "specials.fighting __________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *hunting;  /* Hunting person..                  */
	if (ch->specials.hunting)
 	{
		sprintf(buf, "specials.hunting ___________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *ridden_by;
	if (ch->specials.ridden_by)
 	{
		sprintf(buf, "specials.ridden_by _________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *mounted_on;
	if (ch->specials.mounted_on)
 	{
		sprintf(buf, "specials.mounted_on ________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//	  struct char_data *charging;        /* we are charging this person */
	if (ch->specials.charging)
 	{
		sprintf(buf, "specials.charging __________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.charge_dir < -1 || ch->specials.charge_dir >6)
	{
		sprintf (buf, "specials.charge_dir ________________: %d (< -1 o >6)", ch->specials.charge_dir);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.affected_by >0x80000000)
	{
		sprintf (buf, "specials.affected_by _______________: %lu (>%lu)", ch->specials.affected_by,(long unsigned)0x80000000);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->specials.affected_by2 >(long unsigned)0x00000040)
	{
		sprintf (buf, "specials.affected_by2 ______________: %lu (<0 o >%lu)", ch->specials.affected_by2,(long unsigned)0x00000040);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "specials.act _______________________: %lu ()", ch->specials.act);
		errore =prterr(INT_ERR, buf);
	}
	for (ciclo =0; ciclo <MAX_SAVES; ciclo++)
	{
		if (0)
		{
			sprintf (buf, "apply_saving_throw[%1d] : %d ()", ciclo, ch->specials.apply_saving_throw[ciclo]);
			errore =prterr(INT_ERR, buf);
		}
	}
//  struct char_data *next_listener;       /* Prossimo che fa eavesdrop */
	if (ch->next_listener)
 	{
		sprintf(buf, "listener ___________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->listening_to <0 || ch->listening_to >WORLD_SIZE-1)
	{
		sprintf(buf, "listening_to _______________________: %d (<0 o >%d)", ch->listening_to, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
//  struct char_skill_data *skills;       /* Skills                */
	if (ch->skills)
	{
		if (ch->skills->learned >100)
		{
			sprintf (buf, "skills->learned ____________________: %u (>100)", ch->skills->learned);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->skills->flags <0)
		{
			sprintf (buf, "skills->flags ______________________: %d (<0)", ch->skills->flags);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->skills->special <0 || ch->skills->special >1)
		{
			sprintf (buf, "skills->special ____________________: %d (<0 o >1)", ch->skills->special);
			errore =prterr(INT_ERR, buf);
		}
		if (ch->skills->nummem <0 || ch->skills->nummem >99)
		{
			sprintf (buf, "skills->nummem _____________________: %d (<0 o >99)", ch->skills->nummem);
			errore =prterr(INT_ERR, buf);
		}
	}
//  struct obj_data *equipment[MAX_WEAR]; /* Equipment array         */
	for (ciclo =0; ciclo <MAX_WEAR; ciclo++) /* Initialisering Ok */
 	{
		if (ch->equipment[ciclo])
 		{
			sprintf(buf, "equipment[%d] _______________________: %s (puntatore inizializzato)", ciclo, "ATTENZIONE");
      		errore =prterr(INT_ERR, buf);
		}
	}
//  struct obj_data *carrying;            /* Head of list            */
	if (ch->carrying)
 	{
		sprintf(buf, "carrying ___________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct descriptor_data *desc;         /* NULL for mobiles        */
	if (ch->desc)
 	{
		sprintf(buf, "desc _______________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct char_data *orig;               /* Special for polymorph   */
	if (ch->orig)
 	{
		sprintf(buf, "orig _______________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct char_data *next_in_room;     /* For room->people - list   */
	if (ch->next_in_room)
 	{
		sprintf(buf, "next_in_room _______________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct char_data *next;             /* all in game list  */
	if (ch->next)
 	{
		if (ch->nr <0 || ch->nr >99998)
		{
			sprintf(buf, "next _______________________________: %s (puntatore inizializzato fuori limite)", ch->next->player.name);
      		errore =prterr(INT_ERR, buf);
		}
	}
//  struct char_data *next_fighting;    /* For fighting list         */
	if (ch->next_fighting)
 	{
		sprintf(buf, "next_fighting ______________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct follow_type *followers;        /* List of chars followers */
	if (ch->followers)
 	{
		sprintf(buf, "followers __________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
//  struct char_data *master;             /* Who is char following?  */
	if (ch->master)
 	{
		sprintf(buf, "master _____________________________: %s (puntatore inizializzato)", "ATTENZIONE");
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->invis_level >60)
	{
		sprintf(buf, "invis_level ________________________: %u (>60)", ch->invis_level);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf(buf, "nTeleCount _________________________: %d ()", ch->nTeleCount);
      	errore =prterr(INT_ERR, buf);
	}
	if (ch->lStartRoom <0 || ch->lStartRoom >WORLD_SIZE-1)
	{
		sprintf(buf, "lStartRoom _________________________: %ld (<0 o >%d)", ch->lStartRoom, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (ch->AgeModifier <0 || ch->AgeModifier >1000)
	{
		sprintf(buf, "AgeModifier ________________________: %ld (<0 o >%d)", ch->AgeModifier, WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
//  struct event *points_event[3];   /* events for regening H/M/V */
	for (ciclo =0; ciclo <3; ciclo++) /* Initialisering Ok */
 	{
		if (ch->points_event[ciclo])
 		{
			sprintf(buf, "points_event[%d] ____________________: %s (puntatore inizializzato)", ciclo, "ATTENZIONE");
      		errore =prterr(INT_ERR, buf);
		}
	}
	  if (errore)
      	log("-------------------------------------------------------------------------------}");
  }
}

void checkplayer()
{
   #undef INT_ERR
   #define INT_ERR errore, 1, 0, szFileName
   DIR *dir;
   int j, i, ciclo;
   char buf[MAX_STRING_LENGTH];
   
   top_of_p_table = 0;
   
   if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
   {
      struct dirent *ent;
      while( ( ent = readdir( dir ) ) != NULL )
      {
	 FILE *pFile;
	 char szFileName[ 60 ];
	 int errore =0;
	 
	 if( *ent->d_name == '.' )
	 	continue;
	 if (!strstr(ent->d_name,".dat")) 
	    continue;
	 sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );
	 {
	    struct char_file_u Player;
	    if (!(pFile=fopen(szFileName,"r")))
	    	continue;
	    if( fread( &Player, 1, sizeof( Player ), pFile ) == sizeof( Player ) )
	    {
	       int max;
	       
	       top_of_p_table++;
	       
	       for( j = 0, max = 0; j < 11; j++ )
	       {
		  if (Player.level [j]> 60) 
		  {
		     Player.level[j]=0;
		     max=70+1;
		  }
		  
	       if( Player.level[ j ] > max )
	       max = Player.level[ j ];
	       }
	       if (max >=(70+1))
	       {
			 sprintf (buf, "ERR: Levels [%2d][%2d][%2d][%2d][%2d][%2d][%2d][%2d] %s",
			 Player.level[0], Player.level[1], 
			 Player.level[2], Player.level[3], Player.level[4], 
			 Player.level[5], Player.level[6], Player.level[7], Player.name);
      		 errore =prterr(INT_ERR, buf);
	       }
		   else
	       {
			if (Player.iClass <1 || Player.iClass >2048)
	       	{
				sprintf (buf, "iClass                : %d (<1 o >2048)", Player.iClass);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.sex <1 || Player.sex >7)
	       	{
				sprintf (buf, "sex                   : %d (<1 o >7)", Player.sex);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			for (ciclo =0; ciclo <ABS_MAX_CLASS-9; ciclo++)
	       	{
				if (Player.level[ciclo] >60)
	       		{
					sprintf (buf, "level[%2d]             : %u (>60)", ciclo, Player.level[ciclo]);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (0)
  	  		{
				sprintf (buf, "birth                 : %ld", Player.birth); log(buf);
	       	}
			if (0)
  	  		{
				sprintf (buf, "played                : %d", Player.played); log(buf);
	       	}
			if (Player.race <1 || Player.race >MAX_RACE)
  	  		{
				sprintf (buf, "race                  : %d (<1 o >%d)", Player.race, MAX_RACE);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.weight >900)
  	  		{
				sprintf (buf, "weight                : %u (>900)", Player.weight);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.height >620)
  	  		{
				sprintf (buf, "height                : %u (>620)", Player.height);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.title)
  	  		{
				if (strlen(Player.title) >80)
  	  			{
					sprintf (buf, "title                 : %s (%d >80 car.)", Player.title,strlen(Player.title));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.extra_str)
  	  		{
				if (strlen(Player.extra_str) >255)
  	  			{
					sprintf (buf, "extra_str             : %s (%d >255 car.)", Player.extra_str,strlen(Player.extra_str));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.hometown <0 || Player.hometown >WORLD_SIZE-1)
  	  		{
				sprintf (buf, "hometown              : %d (<0 o >%d)", Player.hometown, WORLD_SIZE-1);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.description)
  	  		{
				if (strlen(Player.description) >240)
  	  			{
					sprintf (buf, "description           : %s (%d >240 car.)", Player.description,strlen(Player.description));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			for (ciclo =0; ciclo <MAX_TOUNGE; ciclo++)
	       	{
				if (Player.talks[ciclo] !=0 && Player.talks[ciclo] !=1)
	       		{
					sprintf (buf, "talks[%1d]              : %d (<0 o >1)", ciclo, Player.talks[ciclo]);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (0)
	       	{
				sprintf (buf, "extra_flags           : %ld", Player.extra_flags); log(buf);
	       	}
			if (Player.load_room < -2 || Player.load_room > -1)
	       	{
				sprintf (buf, "load_room             : %d (< -2 o > -1)", Player.load_room);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.str <0 || Player.abilities.str >25)
	       	{
				sprintf (buf, "abilities.str         : %d (<0 o >25)", Player.abilities.str);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.str_add <0 || Player.abilities.str_add >100)
	       	{
				sprintf (buf, "abilities.str_add     : %d (<0 o >100)", Player.abilities.str_add);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.intel <0 || Player.abilities.intel >25)
	       	{
				sprintf (buf, "abilities.intel       : %d (<0 o >25)", Player.abilities.intel);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.wis <0 || Player.abilities.wis >25)
	       	{
				sprintf (buf, "abilities.wis         : %d (<0 o >25)", Player.abilities.wis);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.dex <0 || Player.abilities.dex >25)
	       	{
				sprintf (buf, "abilities.dex         : %d (<0 o >25)", Player.abilities.dex);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.con <0 || Player.abilities.con >25)
	       	{
				sprintf (buf, "abilities.con         : %d (<0 o >25)", Player.abilities.con);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.abilities.chr <0 || Player.abilities.chr >25)
	       	{
				sprintf (buf, "abilities.chr         : %d (<0 o >25)", Player.abilities.chr);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "abilities.extra       : %d", Player.abilities.extra); log(buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "abilities.extra2      : %d", Player.abilities.extra2); log(buf);
	       	}
			if (Player.points.mana <0 || Player.points.mana >1000)
	       	{
				sprintf (buf, "points.mana           : %d (<0 o >1000)", Player.points.mana);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.max_mana <0 || Player.points.max_mana >1000)
	       	{
				sprintf (buf, "points.max_mana       : %d (<0 o >1000)", Player.points.max_mana);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.mana_gain >1000)
	       	{
				sprintf (buf, "points.mana_gain      : %u (>1000)", Player.points.mana_gain);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.hit <0 || Player.points.hit >1000)
	       	{
				sprintf (buf, "points.hit            : %d (<0 o >1000)", Player.points.hit);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.max_hit <0 || Player.points.max_hit >1000)
	       	{
				sprintf (buf, "points.max_hit        : %d (<0 o >1000)", Player.points.max_hit);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.hit_gain >1000)
	       	{
				sprintf (buf, "points.hit_gain       : %u (>1000)", Player.points.hit_gain);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.move <0 || Player.points.move >1000)
	       	{
				sprintf (buf, "points.move           : %d (<0 o >1000)", Player.points.move);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.max_move <0 || Player.points.max_move >1000)
	       	{
				sprintf (buf, "points.max_move       : %d (<0 o >1000)", Player.points.max_move);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.move_gain >1000)
	       	{
				sprintf (buf, "points.move_gain      : %u (>1000)", Player.points.move_gain);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "points.extra1         : %d", Player.points.extra1); log(buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "points.extra2         : %d", Player.points.extra2); log(buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "points.extra3         : %u", Player.points.extra3); log(buf);
	       	}
			if (Player.points.armor < -100 || Player.points.armor >100)
	       	{
				sprintf (buf, "points.armor          : %d (< -100 o >100)", Player.points.armor);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.gold <0 || Player.points.gold >2000000)
	       	{
				sprintf (buf, "points.gold           : %d (<0 0 >2M)", Player.points.gold);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.bankgold <0 || Player.points.bankgold >4000000)
	       	{
				sprintf (buf, "points.bankgold       : %d (<0 0 >4M)", Player.points.bankgold);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "points.true_exp       : %d", Player.points.true_exp); log(buf);
	       	}
			if (0)
	       	{
				sprintf (buf, "points.extra_dual     : %d", Player.points.extra_dual); log(buf);
	       	}
			if (Player.points.hitroll < -50 || Player.points.hitroll >50)
	       	{
				sprintf (buf, "points.hitroll        : %d (< -50 o >50)", Player.points.hitroll);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.damroll < -50 || Player.points.damroll >50)
	       	{
				sprintf (buf, "points.damroll        : %d (< -50 o >50)", Player.points.damroll);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.points.pQuest <0 || Player.points.pQuest >100)
	       	{
				sprintf (buf, "points.pQuest         : %d (<0 o >100)", Player.points.pQuest);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			for (ciclo =0; ciclo <350; ciclo++)
	       	{
				if (Player.skills[ciclo].learned >100)
	       		{
					sprintf (buf, "skills[%3d].learned   : %u (>100)", ciclo, Player.skills[ciclo].learned);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.skills[ciclo].flags <0)
	       		{
					sprintf (buf, "skills[%3d].flags     : %d (<0)", ciclo, Player.skills[ciclo].flags);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.skills[ciclo].special <0 || Player.skills[ciclo].special >1)
	       		{
					sprintf (buf, "skills[%3d].special   : %d (<0 o >1)", ciclo, Player.skills[ciclo].special);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.skills[ciclo].nummem <0 || Player.skills[ciclo].nummem >99)
	       		{
					sprintf (buf, "skills[%3d].nummem    : %d (<0 o >99)", ciclo, Player.skills[ciclo].nummem);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			for (ciclo =0; ciclo <40; ciclo++)
	       	{
				if (Player.affected[ciclo].type < -1 || Player.affected[ciclo].type >288)
	       		{
					sprintf (buf, "affected[%2d].type     : %d (< -1 o >288)", ciclo, Player.affected[ciclo].type);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.affected[ciclo].duration <0 || Player.affected[ciclo].duration >300)
	       		{
					sprintf (buf, "affected[%2d].duration : %d (<0 o >300)", ciclo, Player.affected[ciclo].duration);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.affected[ciclo].modifier < -100 || Player.affected[ciclo].modifier >10500)
	       		{
					sprintf (buf, "affected[%2d].modifier : %ld (< -100 o >10500)", ciclo, Player.affected[ciclo].modifier);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if ((long unsigned)Player.affected[ciclo].bitvector >(long unsigned)0x80000000)
	       		{
					sprintf (buf, "affected[%2d].bitvector: %lu (>%lu)", ciclo, (long unsigned)Player.affected[ciclo].bitvector,(long unsigned)0x80000000);
      		 		errore =prterr(INT_ERR, buf);
	       		}
				if (Player.affected[ciclo].next)
	       		{
					sprintf (buf, "affected[%2d].next     : ATTENZIONE puntatore inizializzato", ciclo); log(buf);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.spells_to_learn >99)
	       	{
				sprintf (buf, "spells_to_learn       : %d (>99)", Player.spells_to_learn);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.alignment < -1000 || Player.alignment >1000)
	       	{
				sprintf (buf, "alignment             : %d (< -1000 o >1000)", Player.alignment);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.affected_by >0x80000000)
	       	{
				sprintf (buf, "affected_by           : %lu (>%lu)", Player.affected_by,(long unsigned)0x80000000);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.affected_by2 >(long unsigned)0x00000040)
	       	{
				sprintf (buf, "affected_by2          : %lu (<0 o >%lu)", Player.affected_by2,(long unsigned)0x00000040);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.last_logon <0)
	       	{
				sprintf (buf, "last_logon            : %ld (<0)", Player.last_logon);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.act <0)
	       	{
				sprintf (buf, "act                   : %ld (<0)", Player.act);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.name)
	       	{
				if (strlen(Player.name) >20)
	       		{
					sprintf (buf, "name                  : %s (%d >20 car.)", Player.name,strlen(Player.name));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			else
	       	{
				sprintf (buf, "name                  : (null)");
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.authcode)
	       	{
				if (strlen(Player.authcode) >7)
	       		{
					sprintf (buf, "authcode              : %s (%d >7 car.)", Player.authcode,strlen(Player.authcode));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.WimpyLevel)
	       	{
				if (strlen(Player.WimpyLevel) >4)
	       		{
					sprintf (buf, "WimpyLevel            : %s (%d >4 car.)", Player.WimpyLevel,strlen(Player.WimpyLevel));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (0 && Player.dummy)
	       	{
				if (strlen(Player.dummy) >19)
	       		{
					sprintf (buf, "dummy                 : %s (%d >19 car.)", Player.dummy,strlen(Player.dummy));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.pwd)
	       	{
				if (strlen(Player.pwd) >11)
	       		{
					sprintf (buf, "pwd                   : %s (%d >11 car.)", Player.pwd,strlen(Player.pwd));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			for (ciclo =0; ciclo <MAX_SAVES; ciclo++)
	       	{
				if (0)
	       		{
					sprintf (buf, "apply_saving_throw[%1d] : %d ()", ciclo, Player.apply_saving_throw[ciclo]);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			for (ciclo =0; ciclo <MAX_CONDITIONS-2; ciclo++)
	       	{
				if (Player.conditions[ciclo] < -1 || Player.conditions[ciclo] >24)
	       		{
					sprintf (buf, "conditions        [%1d] : %d (< -1 o >24)", ciclo, Player.conditions[ciclo]);
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.startroom <0 || Player.startroom >WORLD_SIZE-1)
	       	{
				sprintf (buf, "startroom             : %d (<0 o >%d)", Player.startroom, WORLD_SIZE-1);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.user_flags <1 || Player.user_flags >8192)
	       	{
				sprintf (buf, "user_flags            : %ld (<1 o >8192)", Player.user_flags);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.speaks <1 || Player.speaks >8)
	       	{
				sprintf (buf, "speaks                : %d (<1 o >8)", Player.speaks);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.agemod < -30)
	       	{
				sprintf (buf, "agemod                : %ld (< -30)", Player.agemod);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (errore)
      			log("-------------------------------------------------------------------------------}");
	       }
	    }
	    fclose( pFile );
	 }
      }
   }
   else
   {
      sprintf(buf, "Cannot open dir %s", PLAYERS_DIR );
	  log(buf);
   }
   return;
}

void checkrent()
{
   #undef INT_ERR
   #define INT_ERR errore, 1, 0, ent->d_name
   struct obj_file_u st;
   char buf[MAX_STRING_LENGTH];
   
   
   DIR *dir;
   
   if( ( dir = opendir( RENT_DIR ) ) != NULL )
   {
      struct dirent *ent;
      while( ( ent = readdir( dir ) ) != NULL )
      {
   	 FILE *pObjFile;
	 char szFileName[ 40 ];
   	 int ciclo, ciclo2, errore =0;
	 
	 if( *ent->d_name == '.' )
	 continue;
	 
	 if(strstr(ent->d_name, "."))
	 continue;

	 sprintf( szFileName, "%s/%s", RENT_DIR, ent->d_name );
	 
	   if( ( pObjFile = fopen( szFileName, "r+b") ) != NULL )
	   {
//	 	  sprintf( buf, "Apro il file %s\n", szFileName);
// 		  fprintf(stderr, buf);
		  if( ReadObjs( pObjFile, &st ) )
		  {
		     if( strcasecmp( st.owner, ent->d_name ) == 0 )
		     {
				if (st.owner)
		     	{
					if (strlen(st.owner) >20)
		     		{
						sprintf(buf,"owner                    :%s (%d >20 car.)", st.owner, strlen(st.owner));
      					errore =prterr(INT_ERR, buf);
		     		}
		     	}
				else
		     	{
					sprintf(buf,"owner                    :(null)");
      				errore =prterr(INT_ERR, buf);
		     	}
				if (st.gold_left <0 || st.gold_left >2000000)
		     	{
					sprintf(buf,"gold_left                :%d (<0 o >2M)", st.gold_left);
      				errore =prterr(INT_ERR, buf);
		     	}
				if (st.total_cost <0 || st.total_cost >2000000)
		     	{
					sprintf(buf,"total_cost               :%d (<0 o >2M)", st.total_cost);
      				errore =prterr(INT_ERR, buf);
		     	}
				if (st.last_update <0)
		     	{
					sprintf(buf,"last_update              :%ld (<0)", st.last_update);
      				errore =prterr(INT_ERR, buf);
		     	}
				if (st.minimum_stay !=0)
		     	{
					sprintf(buf,"minimum_stay             :%ld (!=0)", st.minimum_stay);
      				errore =prterr(INT_ERR, buf);
		     	}
				if (st.number <0 || st.number >MAX_OBJ_SAVE)
		     	{
					sprintf(buf,"number                   :%d (<0 o >%d)", MAX_OBJ_SAVE, st.number);
      				errore =prterr(INT_ERR, buf);
		     	}
				for (ciclo =0; ciclo <st.number; ciclo++)
		     	{
					if (st.objects[ciclo].item_number >65535)
		     		{
						sprintf(buf,"objects[%3d].item_number :%u (>65535)", ciclo, st.objects[ciclo].item_number);
      					errore =prterr(INT_ERR, buf);
		     		}
					for (ciclo2 =0; ciclo2 <4; ciclo2++)
		     		{
						if (st.objects[ciclo].value[ciclo2] < -32767 || st.objects[ciclo].value[ciclo2] >32768)
		     			{
							sprintf(buf,"objects[%3d].value[%1d]    :%d (< -32767 o >32768)", ciclo, ciclo2, st.objects[ciclo].value[ciclo2]);
      						errore =prterr(INT_ERR, buf);
		     			}
		     		}
					if ((unsigned)st.objects[ciclo].extra_flags >(long unsigned)0x80000000)
		     		{
						sprintf(buf,"objects[%3d].extra_flags :%u (>2147483648)", ciclo, st.objects[ciclo].extra_flags);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].weight < -32767 || st.objects[ciclo].weight >32768)
		     		{
						sprintf(buf,"objects[%3d].weight      :%d (< -32767 o >32768)", ciclo, st.objects[ciclo].weight);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].timer <0)
		     		{
						sprintf(buf,"objects[%3d].timer       :%d (<0)", ciclo, st.objects[ciclo].timer);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].bitvector >(long unsigned)0x80000000)
		     		{
						sprintf(buf,"objects[%3d].bitvector   :%lu (<0 o >%lu)", ciclo, st.objects[ciclo].bitvector, (long unsigned)0x80000000);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].name)
		     		{
						if (strlen(st.objects[ciclo].name) >128)
		     			{
							sprintf(buf,"objects[%3d].name        :%s (%d >128 car.)", ciclo, st.objects[ciclo].name, strlen(st.objects[ciclo].name));
      						errore =prterr(INT_ERR, buf);
		     			}
		     		}
					else
		     		{
						sprintf(buf,"objects[%3d].name        :(null)", ciclo);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].sd)
		     		{
						if (strlen(st.objects[ciclo].sd) >128)
		     			{
							sprintf(buf,"objects[%3d].sd          :%s (%d >128 car.)", ciclo, st.objects[ciclo].sd, strlen(st.objects[ciclo].sd));
      						errore =prterr(INT_ERR, buf);
		     			}
		     		}
					else
		     		{
						sprintf(buf,"objects[%3d].sd          :(null)", ciclo);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (st.objects[ciclo].desc)
		     		{
						if (strlen(st.objects[ciclo].desc) >256)
		     			{
							sprintf(buf,"objects[%3d].desc        :%s (%d >256 car.)", ciclo, st.objects[ciclo].desc, strlen(st.objects[ciclo].desc));
      						errore =prterr(INT_ERR, buf);
		     			}
		     		}
					else
		     		{
						sprintf(buf,"objects[%3d].desc        :(null)", ciclo);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (0)
		     		{
						sprintf(buf,"objects[%3d].wearpos     :%u ()", ciclo, st.objects[ciclo].wearpos);
      					errore =prterr(INT_ERR, buf);
		     		}
					if (0)
		     		{
						sprintf(buf,"objects[%3d].depth       :%u ()", ciclo, st.objects[ciclo].depth);
      					errore =prterr(INT_ERR, buf);
		     		}
					for (ciclo2 =0; ciclo2 <MAX_OBJ_AFFECT; ciclo2++)
		     		{
						if (st.objects[ciclo].affected[ciclo2].location <0 || st.objects[ciclo].affected[ciclo2].location >65)
			     		{
							sprintf(buf,"objects[%3d].affected[%1d].location :%d (<0 o >65)", ciclo, ciclo2, st.objects[ciclo].affected[ciclo2].location);
      						errore =prterr(INT_ERR, buf);
		     			}
						if (0)
							sprintf(buf,"objects[%3d].affected[%1d].modifier :%ld", ciclo, ciclo2, st.objects[ciclo].affected[ciclo2].modifier);
		     		}
		     	}
				if (errore)
      				log("-------------------------------------------------------------------------------}");
		     }
		  }
	   	  else
	   	  {
	        sprintf(buf, "File degli oggetti vuoto o con errori: %s", szFileName );
		    log(buf);
	   	  }
	    }
	   	else
	   	{
	      sprintf(buf, "Error reading file %s", szFileName );
		  log(buf);
	   	}
	   }
   }
   else
   {
      sprintf(buf, "Cannot open dir %s", RENT_DIR );
	  log(buf);
   }
}

void caricaobj ()
{
  int numogg;
  char buf[81];

  sprintf(buf,"%s", OBJ_FILE);
  if (!(obj_f = fopen(buf, "r")))    
  {
    sprintf(buf, "Cannot open dir %s", OBJ_FILE);
	log(buf);
	exit(0);
  }
  obj_index = generate_indices( obj_f, &top_of_objt, &top_of_sort_objt, 
                                &top_of_alloc_objt, OBJ_DIR );
  for (numogg =0; numogg <99999 && !feof(obj_f); numogg++)
		checkobj(read_object( numogg, REAL ), 0);
  fclose(obj_f);
  if (obj_count)
   fprintf(stderr, "\nTotale oggetti duplicati %ld\n", obj_count);
}

void caricamob ()
{
  int nummob;
  char buf[81];

  sprintf(buf,"%s", MOB_FILE);
  if (!(mob_f = fopen(buf, "r")))    
  {
    sprintf(buf, "Cannot open dir %s", OBJ_FILE);
	log(buf);
	exit(0);
  }
  mob_index = generate_indices( mob_f, &top_of_mobt, &top_of_sort_mobt, 
                                &top_of_alloc_mobt, MOB_DIR );
  for (nummob =0; nummob <99999 && !feof(mob_f); nummob++)
		checkmob(read_mobile( nummob, REAL ), 0);
  fclose(mob_f);
  if (mob_count)
   fprintf(stderr, "\nTotale mob duplicati %ld\n", mob_count);
}

void aiuto_sintassi()
{
	puts("");
	puts("checkfile -{o|p|r|s}");
	puts("	-o	controlla file oggetti		myst.obj	in ./lib");
	puts("	-g	controlla file giocatori	nome.dat	in ./lib/players");
	puts("	-r	controlla file dei rent 	nome    	in ./lib/rent");
	puts("	-s	controlla file stanze		myst.wld  	in ./lib");
	puts("	-m	controlla file mob   		myst.mob  	in ./lib");
	puts("	-z	controlla file zone 		myst.zon  	in ./lib");
	puts("");
	puts("Versione 1.1 aggiunto controllo sulla posizione dead dei mob.");
	puts("Versione 1.0");
	puts("    senza controllo sugli scripts dei mob");
}

void setup_dir(FILE *fl, struct room_data *rp, int dir)
{
  long tmp;
   
   CREATE(rp->dir_option[dir], struct room_direction_data, 1);
   bzero(rp->dir_option[dir], sizeof(struct room_direction_data));
   
   rp->dir_option[dir]->general_description = fread_string(fl);
   rp->dir_option[dir]->keyword = fread_string(fl);
   
   rp->dir_option[dir]->exit_info = fread_number( fl );
   
   rp->dir_option[dir]->key = fread_number( fl );
   
   rp->dir_option[dir]->to_room = fread_number( fl );
   
   tmp = -1;
   fscanf(fl, " %ld ", &tmp);
   rp->dir_option[dir]->open_cmd = tmp;
}

void load_one_room( FILE *fl, struct room_data *rp )
{
  char chk[ 161 ];
  int   bc=0;
  long int tmp;

  struct extra_descr_data *new_descr;

  bc = sizeof(struct room_data);

  rp->name = fread_string(fl);
  sprintf(chk, "LR: vnum %ld name %s",rp->number,rp->name);
   
  if (rp->name && *rp->name)
    bc += strlen(rp->name);
  rp->description = fread_string(fl);
  if (rp->description && *rp->description)
    bc += strlen(rp->description);

  sprintf(curfile,"\nCaricando stanza %ld nome %s",rp->number, rp->name);
  tmp = fread_number( fl );
  rp->room_flags = tmp;
  tmp = fread_number( fl );
  rp->sector_type = tmp;
  
  sprintf(curfile,"Letto flags= %ld , sector= %ld",rp->room_flags,rp->sector_type); 
  if (tmp == -1)
  {
  sprintf(curfile,"Stanza con settore -1");
    tmp = fread_number( fl );
    rp->tele_time = tmp;
    tmp = fread_number( fl );
    rp->tele_targ = tmp;
    tmp = fread_number( fl );
    rp->tele_mask = tmp;
    if( IS_SET( TELE_COUNT, rp->tele_mask ) )
    {
      tmp = fread_number( fl );
      rp->tele_cnt = tmp;
    }
    else
    {
      rp->tele_cnt = 0;
    }
    tmp = fread_number( fl );
    rp->sector_type = tmp;
  }
  else
  {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_mask = 0;
    rp->tele_cnt  = 0;
  }

   if( tmp == SECT_WATER_NOSWIM || tmp == SECT_UNDERWATER) 
   { 
      /* river 
       * read direction and rate of flow 
       * su myst non tutte le stanze subaquee hanno la corrente, per cui
       * uso fread_if_number che non da` errore se non trova un numero
       * */
       tmp = fread_if_number( fl );
       rp->river_speed = tmp;
       tmp = fread_if_number( fl );
       rp->river_dir = tmp;
    }

  if( rp->room_flags & TUNNEL )
  {
    /* read in mobile limit on tunnel */
    tmp = fread_number( fl );
    rp->moblim = tmp;
  }

  rp->funct = 0;
  rp->light = 0; /* Zero light sources */

  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;

  rp->ex_description = 0;

  while( fscanf( fl, " %160s \n", chk ) == 1 )
  { 
    switch( *chk )
    {
     case 'D':
      setup_dir(fl, rp, atoi(chk + 1));
      bc += sizeof(struct room_direction_data);
      break;
     case 'E': /* extra description field */

      CREATE(new_descr,struct extra_descr_data,1);
      bc += sizeof(struct extra_descr_data);

      new_descr->keyword = fread_string(fl);
      if (new_descr->keyword && *new_descr->keyword)
        bc += strlen(new_descr->keyword);
      else
  	  {
        sprintf(curfile, "No keyword in room %ld", rp->number);
      }

      new_descr->description = fread_string(fl);
      if( new_descr->description && *new_descr->description)
        bc += strlen(new_descr->description);
      else
  	  {
        sprintf(curfile, "No desc in room %ld", rp->number);
      }

      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      break;
     case 'L':
      rp->szWhenBrightAtNight = fread_string( fl );
      rp->szWhenBrightAtDay = fread_string( fl );
      break;
     case 'S':   /* end of current room */

      total_bc += bc;
      room_count++;

      if( IS_SET( rp->room_flags, SAVE_ROOM ) )
      {
        saved_rooms[ number_of_saved_rooms ] = rp->number;
        number_of_saved_rooms++;
      }
      else
      {
        FILE *fp;
        char buf[255];

        sprintf( buf, "world/%ld", rp->number );
        fp = fopen( buf, "r" );
        if( fp ) 
        {
          saved_rooms[ number_of_saved_rooms ] = rp->number;
          number_of_saved_rooms++;
          fclose( fp );
        }
      }
      return;
     case 'C':
      /* Commento, non deve fare nulla. Il tutto deve stare su una sola 
       * linea. */
      break;
     default:
      sprintf(curfile, "unknown auxiliary code `%s' in room load of #%ld",
              chk, rp->number);
      break;
    }
  }
}

void checkroom(struct room_data *rp)
{
  #undef INT_ERR
  #define INT_ERR errore, 0, rp->number, rp->name
  char buf[MAX_STRING_LENGTH];
  int  ciclo, errore =0;

  if (rp && rp->name && *rp->name)
  {
	if (rp->number <0 || rp->number >WORLD_SIZE-1)
  	{
		sprintf(buf, "number _____________________________: %ld (<0 o >%ld)", rp->number, (long)WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (rp->zone <0 || rp->zone >999998)
  	{
		sprintf(buf, "zone _______________________________: %ld (<0 o >999998)", rp->zone);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "continent __________________________: %ld ()", rp->continent);
		errore =prterr(INT_ERR, buf);
	}
	if (0 && (rp->sector_type <0 || rp->sector_type >12))
  	{
		sprintf(buf, "sector_type ________________________: %ld (<0 o >12)", rp->sector_type);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "blood ______________________________: %d ()", rp->blood);
		errore =prterr(INT_ERR, buf);
	}
	if (0 && (rp->river_dir <0 || rp->river_dir >5))
  	{
		sprintf(buf, "river_dir __________________________: %d (<0 o >5)", rp->river_dir);
		errore =prterr(INT_ERR, buf);
	}
	if (0 && (rp->river_speed !=0 && rp->river_speed !=1))
  	{
		sprintf(buf, "river_speed ________________________: %d (<0 o >1)", rp->river_speed);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "tele_time __________________________: %d ()", rp->tele_time);
		errore =prterr(INT_ERR, buf);
	}
	if (rp->tele_targ <0 || rp->tele_targ >WORLD_SIZE-1)
  	{
		sprintf(buf, "tele_targ __________________________: %d (<0 o >%ld)", rp->tele_targ, (long)WORLD_SIZE-1);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "tele_mask __________________________: %d ()", rp->tele_mask);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "tele_cnt ___________________________: %d ()", rp->tele_cnt);
		errore =prterr(INT_ERR, buf);
	}
	if (0)
  	{
		sprintf(buf, "moblim _____________________________: %u ()", rp->moblim);
		errore =prterr(INT_ERR, buf);
	}
	if (rp->name)
	{
		if (strlen(rp->name) >80)
	  	{
      		sprintf(buf, "name _______________________________: %s (%d >80 car.)", rp->name, strlen(rp->name));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	else
	{
      	sprintf(buf, "name _______________________________: %s", rp->name);
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->description)
	{
		if (strlen(rp->description) >MAX_STRING_LENGTH)
	  	{
      		sprintf(buf, "description ________________________: %s (%d >%d car.)", rp->description, strlen(rp->description),MAX_STRING_LENGTH);
      		errore =prterr(INT_ERR, buf);
	  	}
	}
    if (rp->ex_description)
	{
		if ((rp->ex_description->nMagicNumber <1234 || (rp->ex_description->nMagicNumber >1236 && rp->ex_description->nMagicNumber !=2345))&& rp->ex_description->nMagicNumber !=0)
	  	{
      		sprintf(buf, "ex_description->nMagicNumber _______: %d (<1234 o >1236 e !=2345)", rp->ex_description->nMagicNumber);
      		errore =prterr(INT_ERR, buf);
	  	}
		if (rp->ex_description->keyword)
	  	{
			if (strlen(rp->ex_description->keyword) >80)
	  		{
      			sprintf(buf, "ex_description->keyword ____________: %s (%d >80 car.)", rp->ex_description->keyword, strlen(rp->ex_description->keyword));
      			errore =prterr(INT_ERR, buf);
	  		}
	  	}
		if (rp->ex_description->description)
	  	{
			if (strlen(rp->ex_description->description) >MAX_STRING_LENGTH)
	  		{
      			sprintf(buf, "ex_description->description ________: %s (%d >%d car.)", rp->ex_description->description, strlen(rp->ex_description->description),MAX_STRING_LENGTH);
      			errore =prterr(INT_ERR, buf);
	  		}
	  	}
		if (rp->ex_description->next)
	  	{
  			struct extra_descr_data *desc;

        	for (desc = rp->ex_description->next; desc; desc = desc->next) 
        	{
				if (desc->description)
	  			{
					if (strlen(desc->description) >MAX_STRING_LENGTH)
	  				{
    					sprintf(buf, "ex_description->next->description __: %s (%d >%d car.)", desc->description, strlen(desc->description),MAX_STRING_LENGTH);
      					errore =prterr(INT_ERR, buf);
        			}
        		}
	  		}
	  	}
	}
    if (rp->dir_option)
	{
		for (ciclo =0; ciclo <6; ciclo++)
	   	{
			if (rp->dir_option[ciclo])
	   		{
				if (rp->dir_option[ciclo]->general_description)
	   			{
					if (strlen(rp->dir_option[ciclo]->general_description) >MAX_STRING_LENGTH)
	   				{
						sprintf (buf, "dir_option[%1d]->general_description _: %s (>%d car.)", ciclo, rp->dir_option[ciclo]->general_description, MAX_STRING_LENGTH);
      	 				errore =prterr(INT_ERR, buf);
	   				}
	   			}
				if (rp->dir_option[ciclo]->keyword)
	   			{
					if (strlen(rp->dir_option[ciclo]->keyword) >MAX_STRING_LENGTH)
	   				{
						sprintf (buf, "dir_option[%1d]->keyword _____________: %s (>%d car.)", ciclo, rp->dir_option[ciclo]->keyword,MAX_STRING_LENGTH);
      	 				errore =prterr(INT_ERR, buf);
	   				}
	   			}
				if (rp->dir_option[ciclo]->exit_info <0 || rp->dir_option[ciclo]->exit_info >256)
	   			{
					sprintf (buf, "dir_option[%1d]->exit_info ___________: %ld (<0 o >256)", ciclo, rp->dir_option[ciclo]->exit_info);
      	 			errore =prterr(INT_ERR, buf);
	   			}
				if (rp->dir_option[ciclo]->key < -1 || rp->dir_option[ciclo]->key >WORLD_SIZE-1)
	   			{
					sprintf (buf, "dir_option[%1d]->key _________________: %ld (< -1 o >%d)", ciclo, rp->dir_option[ciclo]->key, WORLD_SIZE-1);
      	 			errore =prterr(INT_ERR, buf);
	   			}
				if (rp->dir_option[ciclo]->to_room < -1 || rp->dir_option[ciclo]->to_room >WORLD_SIZE-1)
	   			{
					sprintf (buf, "dir_option[%1d]->to_room _____________: %ld (< -1 o >%d)", ciclo, rp->dir_option[ciclo]->to_room, WORLD_SIZE-1);
      	 			errore =prterr(INT_ERR, buf);
	   			}
				if (rp->dir_option[ciclo]->open_cmd < -1 || rp->dir_option[ciclo]->open_cmd >509)
	   			{
					sprintf (buf, "dir_option[%1d]->open_cmd ____________: %ld (< -1 o >509)", ciclo, rp->dir_option[ciclo]->open_cmd);
      	 			errore =prterr(INT_ERR, buf);
	   			}
	   		}
	   	}
	}
	if (rp->room_flags < -1 || rp->room_flags >1073741824)
	{
		sprintf (buf, "room_flags _________________________: %ld (< -1 o >1073741824)", rp->room_flags);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "light ______________________________: %d ()", rp->light);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
		sprintf (buf, "dark _______________________________: %u ()", rp->dark);
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->funct)
	{
		sprintf (buf, "funct ______________________________: special procedure (attenzione abilitata)");
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->specname)
	{
		if (strlen(rp->specname) >80)
	  	{
      		sprintf(buf, "specname ___________________________: %s (%d >80 car.)", rp->specname, strlen(rp->specname));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	if (rp->specparms)
	{
		if (strlen(rp->specparms) >80)
	  	{
      		sprintf(buf, "specparms __________________________: %s (%d >80 car.)", rp->specparms, strlen(rp->specparms));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	if (rp->contents)
	{
    	sprintf(buf, "contents ___________________________: oggetti");
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->people)
	{
    	sprintf(buf, "people _____________________________: PC o NPC");
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->listeners)
	{
    	sprintf(buf, "listeners __________________________: PC o NPC");
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
    	sprintf(buf, "large ______________________________: non usata");
      	errore =prterr(INT_ERR, buf);
	}
	if (rp->szWhenBrightAtNight)
	{
		if (strlen(rp->szWhenBrightAtNight) >80)
	  	{
      		sprintf(buf, "szWhenBrightAtNight ________________: %s (%d >80 car.)", rp->szWhenBrightAtNight, strlen(rp->szWhenBrightAtNight));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	if (rp->szWhenBrightAtDay)
	{
		if (strlen(rp->szWhenBrightAtDay) >80)
	  	{
      		sprintf(buf, "szWhenBrightAtDay __________________: %s (%d >80 car.)", rp->szWhenBrightAtDay, strlen(rp->szWhenBrightAtDay));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	if (errore)
    	log("-------------------------------------------------------------------------------}");

  }
}

void caricaroom ()
{
   FILE *fl;
   long lVNum, last, totdup=0L, totroom=0L;
   struct room_data rp;
   int numroom[WORLD_SIZE], i;
   
   if (!(fl = fopen(WORLD_FILE, "r")))
   {
      perror("fopen");
      log("Could not open world file.");
      assert(0);
   }
   
   for (i=0; i<WORLD_SIZE; i++)
		   numroom[i] =0;
   last = 0;
   while (1==fscanf(fl, " #%ld\n", &lVNum))
   {
	  numroom[lVNum]++;
	  bzero(&rp, sizeof(struct room_data));
      rp.number = lVNum;
      load_one_room(fl, &rp);
	  if (numroom[lVNum] >1)
   	  {
		fprintf (stderr, "Stanza #%ld duplicata.\n", lVNum);
		totdup++;
   	  }
	  else
   	  {
	    checkroom(&rp);
		totroom++;
   	  }
      last = lVNum;
   }
   printf("\nTotale stanze:%ld, duplicate:%ld.\n",totroom, totdup);
   fclose(fl);
}

void checkzona(struct zone_data *zone_table, int zon)
{
  #undef INT_ERR
  #define INT_ERR errore, 0, zone_table[zon].num, zone_table[zon].name
  char buf[MAX_STRING_LENGTH];
  int  ciclo, errore =0;

  if (zone_table && zone_table[zon].name && zone_table[zon].name)
  {
	if (zone_table[zon].name)
	{
		if (strlen(zone_table[zon].name) >80)
	  	{
      		sprintf(buf, "name _______________________________: %s (%d >80 car.)", zone_table[zon].name, strlen(zone_table[zon].name));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	else
	{
      	sprintf(buf, "name _______________________________: %s", zone_table[zon].name);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].num <0 || zone_table[zon].num >999998)
	{
      	sprintf(buf, "num ________________________________: %d (<0 o >999998)", zone_table[zon].num);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].lifespan < -1)
	{
      	sprintf(buf, "lifespan ___________________________: %d (< -1)", zone_table[zon].lifespan);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].age < -1)
	{
      	sprintf(buf, "age ________________________________: %d (< -1)", zone_table[zon].age);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].top <0 || zone_table[zon].top >WORLD_SIZE-1)
	{
      	sprintf(buf, "top ________________________________: %d (<0 o >%d)", zone_table[zon].top, WORLD_SIZE-1);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].bottom <0 || zone_table[zon].bottom >WORLD_SIZE-1)
	{
      	sprintf(buf, "bottom _____________________________: %d (<0 o >%d)", zone_table[zon].bottom,WORLD_SIZE-1);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].bottom >=zone_table[zon].top)
	{
		if (zone_table[zon].bottom)
		{
      	sprintf(buf, "bottom _____________________________: %d (>= a top %d)", zone_table[zon].bottom,zone_table[zon].top);
      	errore =prterr(INT_ERR, buf);
		}
	}
	if (zone_table[zon].start !=0 && zone_table[zon].start !=1)
	{
      	sprintf(buf, "start ______________________________: %d (<0 o >1)", zone_table[zon].start);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].reset_mode <0 || zone_table[zon].reset_mode >32)
	{
      	sprintf(buf, "reset_mode _________________________: %d (<0 o >32)", zone_table[zon].reset_mode);
      	errore =prterr(INT_ERR, buf);
	}
	if (zone_table[zon].cmd)
   	{
   		for(ciclo =0; ;ciclo++ ) 
   		{
      		if (zone_table[zon].cmd[ciclo].command =='S')
      			break;
			if (!strchr("MOGPCEDH", zone_table[zon].cmd[ciclo].command))
   			{
      			sprintf(buf, "cmd[%3d].command ____________________: %c (non implementato)", ciclo, zone_table[zon].cmd[ciclo].command);
      			errore =prterr(INT_ERR, buf);
      			sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
				log(buf);
   			}
			else
   			{
				int max_arg1=999998, max_arg2=999998, max_arg3=999998, max_arg4=998;
				int min_arg1=0, min_arg2=0, min_arg3=-1, min_arg4=-1;
				if (strchr("MC", zone_table[zon].cmd[ciclo].command))
   				{
					max_arg1 =99999;
					max_arg3 =WORLD_SIZE-1;
   				}
				if (strchr("OGEP", zone_table[zon].cmd[ciclo].command))
   				{
					max_arg1 =99999;
					if (strchr("O", zone_table[zon].cmd[ciclo].command))
						max_arg3 =WORLD_SIZE-1;
					if (strchr("P", zone_table[zon].cmd[ciclo].command))
						max_arg3 =99999;
					if (strchr("E", zone_table[zon].cmd[ciclo].command))
						min_arg3 =0;
   				}
				if (strchr("D", zone_table[zon].cmd[ciclo].command))
   				{
					max_arg1 =WORLD_SIZE-1;
					max_arg2 =5;
   				}
				if (strchr("H", zone_table[zon].cmd[ciclo].command))
					min_arg2 =-32767;
				if (zone_table[zon].cmd[ciclo].if_flag !=0 && zone_table[zon].cmd[ciclo].if_flag !=1)
				{
      				sprintf(buf, "cmd[%3d].if_flag ____________________: %d (<0 o >1)", ciclo, zone_table[zon].cmd[ciclo].if_flag);
     		 		errore =prterr(INT_ERR, buf);
      				sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
					log(buf);
				}
				if (zone_table[zon].cmd[ciclo].arg1 <min_arg1 || zone_table[zon].cmd[ciclo].arg1 >max_arg1)
				{
					sprintf(buf, "cmd[%3d].arg1 _______________________: %d (<%d o >%d)", ciclo, zone_table[zon].cmd[ciclo].arg1,min_arg1,max_arg1);
					errore =prterr(INT_ERR, buf);
      				sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
					log(buf);
				}
				if (zone_table[zon].cmd[ciclo].arg2 <min_arg2 || zone_table[zon].cmd[ciclo].arg2 >max_arg2)
				{
      				sprintf(buf, "cmd[%3d].arg2 _______________________: %d (<%d o >%d)", ciclo, zone_table[zon].cmd[ciclo].arg2,min_arg2,max_arg2);
      				errore =prterr(INT_ERR, buf);
      				sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
					log(buf);
				}
				if (zone_table[zon].cmd[ciclo].arg3 <min_arg3 || zone_table[zon].cmd[ciclo].arg3 >max_arg3)
				{
					sprintf(buf, "cmd[%3d].arg3 _______________________: %d (<%d o >%d)", ciclo, zone_table[zon].cmd[ciclo].arg3,min_arg3,max_arg3);
					errore =prterr(INT_ERR, buf);
      				sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
					log(buf);
				}
				if (zone_table[zon].cmd[ciclo].arg4 <min_arg4 || zone_table[zon].cmd[ciclo].arg4 >max_arg4)
				{
      				sprintf(buf, "cmd[%3d].arg4 _______________________: %d (<%d o >%d)", ciclo, zone_table[zon].cmd[ciclo].arg4,min_arg4,max_arg4);
      				errore =prterr(INT_ERR, buf);
      				sprintf(buf, "[%c %d %d %d %d %d]", zone_table[zon].cmd[ciclo].command, zone_table[zon].cmd[ciclo].if_flag, zone_table[zon].cmd[ciclo].arg1, zone_table[zon].cmd[ciclo].arg2, zone_table[zon].cmd[ciclo].arg3, zone_table[zon].cmd[ciclo].arg4);
					log(buf);
				}
   			}
   		}
	}
	if (zone_table[zon].races)
	{
		if (strlen(zone_table[zon].races) >50)
	  	{
      		sprintf(buf, "races ______________________________: %s (%d >50 car.)", zone_table[zon].races, strlen(zone_table[zon].races));
      		errore =prterr(INT_ERR, buf);
	  	}
	}
	if (0)
	{
      	sprintf(buf, "CurrTotGold ________________________: %.2f ()", zone_table[zon].CurrTotGold);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
      	sprintf(buf, "LastTotGold ________________________: %.2f ()", zone_table[zon].LastTotGold);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
      	sprintf(buf, "CurrToPCGold _______________________: %.2f ()", zone_table[zon].CurrToPCGold);
      	errore =prterr(INT_ERR, buf);
	}
	if (0)
	{
      	sprintf(buf, "LastToPCGold _______________________: %.2f ()", zone_table[zon].LastToPCGold);
      	errore =prterr(INT_ERR, buf);
	}
	if (errore)
    	log("-------------------------------------------------------------------------------}");
  }
}

void caricazona()
{
   
   FILE *fl;
   int zon = 0, cmd_no = 0, expand, tmp, bc=100, cc = 22, znumber, totdup =0;
   char *check, buf[256];
   int numzon[1000000];
   
   for (znumber=0; znumber<1000000; znumber++)
		   numzon[znumber] =0;
   if (!(fl = fopen(ZONE_FILE, "r")))   
   {
      perror("Errore apertura file zone");
      assert(0);
   }
   
   for (;;)     
   {
	  znumber =-1;
      fscanf(fl, " #%d\n",&znumber);
      check = fread_string(fl);
      if (*check == '$')
      break;            /* end of file */

      if (!zon)
      CREATE(zone_table, struct zone_data, bc);
      else if (zon >= bc)
      {
	 if (!(zone_table = (struct zone_data *) realloc(zone_table,
							 (zon + 10) * sizeof(struct zone_data))))
	 {
	    perror("boot_zones realloc");
	    assert(0);
	 }
	 bc += 10;
      }
      zone_table[zon].num = znumber;
	  numzon[znumber]++;
      zone_table[zon].name = check;
      zone_table[zon].bottom = -1;
      zone_table[zon].top = -1;
      zone_table[zon].lifespan = -1;
      zone_table[zon].reset_mode =-1;
      fscanf(fl, "%d", &zone_table[zon].top);
      fscanf(fl, "%d", &zone_table[zon].lifespan);
      fscanf(fl, "%d", &zone_table[zon].reset_mode);
      zone_table[zon].bottom=zon ? zone_table[zon-1].top+1 : 0;
/*      printf("#%-3d(%6d): %-20.20s %5d-%5d %2dm %d\n",
	     zon,
	     zone_table[zon].num,
	     zone_table[zon].name,
	     zone_table[zon].bottom,
	     zone_table[zon].top,
	     zone_table[zon].lifespan,
	     zone_table[zon].reset_mode);
*/
      cmd_no = 0;
      
	  if (numzon[znumber] >1)
   	  {
      	fprintf(stderr, "Zona #%-3d(%6d) duplicata\n", zon, znumber);
		totdup++;
   	  }
      if (zon == 0)
      cc = 20;
      
      for (expand = 1;;)
      {
	 if (expand)
	 {
	    if (!cmd_no)
	    CREATE(zone_table[zon].cmd, struct reset_com, cc);
	    else if (cmd_no >= cc)
	    {
	       cc += 5;
	       if (!(zone_table[zon].cmd =
		     (struct reset_com *) realloc(zone_table[zon].cmd,
						  (cc * sizeof(struct reset_com)))))
	       {
		  perror("reset command load");
		  assert(0);
	       }
	    }
	 }
	 
	 expand = 1;
	 fscanf(fl, " "); /* skip blanks */
	 fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
	 if (!strchr(" HFMCOGEPD*;SR",zone_table[zon].cmd[cmd_no].command))
	 {
	    fgets(buf, 250, fl); /* skip command */
      	fprintf(stderr, "Zona #%-3d(%6d): '%c' codice strano in zona %s: %s\n", zon, znumber,zone_table[zon].cmd[cmd_no].command,check,buf);
	    continue;
	 }
	 
	 if (zone_table[zon].cmd[cmd_no].command == 'S')
	 break;

	 if (zone_table[zon].cmd[cmd_no].command == '#')
	 {
	    fgets(buf, 80, fl); /* skip command */
      	fprintf(stderr, "Zona #%-3d(%6d): non terminata con S in zona %s riga saltata: %s\n", zon, znumber,check, buf);
	 }
	 if (zone_table[zon].cmd[cmd_no].command == 'R')
	 {
	    fgets(buf, 80, fl); /* skip command */
      	fprintf(stderr, "Zona #%-3d(%6d): non implementato R in zona %s riga saltata: %s\n", zon, znumber,check, buf);
	 }
	 
	 if (zone_table[zon].cmd[cmd_no].command == '*')
	 {
	    expand = 0;
	    fgets(buf, 250, fl); /* skip command */
	    continue;
	 }

	 if (zone_table[zon].cmd[cmd_no].command == ';')
	 {
	    expand = 0;
	    fgets(buf, 250, fl); /* skip command */
	    continue;
	 }
	 fgets(buf,255,fl);
	 zone_table[zon].cmd[cmd_no].if_flag=0;
	 zone_table[zon].cmd[cmd_no].arg1=-1;
	 zone_table[zon].cmd[cmd_no].arg2=0;
	 zone_table[zon].cmd[cmd_no].arg3=-1;
	 zone_table[zon].cmd[cmd_no].arg4=0;
	 sscanf(buf," %d %d %d %d %d",
	 &tmp,
	 &zone_table[zon].cmd[cmd_no].arg1,
	 &zone_table[zon].cmd[cmd_no].arg2,
	 &zone_table[zon].cmd[cmd_no].arg3,
	 &zone_table[zon].cmd[cmd_no].arg4);
	 zone_table[zon].cmd[cmd_no].if_flag=tmp;
	 cmd_no++;
      }
	  checkzona(zone_table,zon);
      zon++;
   }
   free(check);
   top_of_zone_table = --zon;
   fclose(fl);
   printf("\nTotale zone:%d, duplicate:%d.\n",top_of_zone_table, totdup);
}

void lastlogon()
{
DIR *dir;
   int j, i, ciclo;
   char buf[MAX_STRING_LENGTH];
   
   top_of_p_table = 0;
   
   if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
   {
      struct dirent *ent;
      while( ( ent = readdir( dir ) ) != NULL )
      {
	 FILE *pFile;
	 char szFileName[ 60 ];
	 int errore =0;
	 
	 if( *ent->d_name == '.' )
	 	continue;
	 if (!strstr(ent->d_name,".dat")) 
	    continue;
	 sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );
	 {
	    struct char_file_u Player;
	    if (!(pFile=fopen(szFileName,"r")))
	    	continue;
	    if( fread( &Player, 1, sizeof( Player ), pFile ) == sizeof( Player ) )
	    int max;
	       
	       top_of_p_table++;
	       
	       for( j = 0, max = 0; j < 11; j++ )
	       {
		  if (Player.level [j]> 60) 
		  {
		     Player.level[j]=0;
		     max=70+1;
		  }
		  
	       if( Player.level[ j ] > max )
	       max = Player.level[ j ];
	       }
	       if (max >=(70+1))
	       {
			 sprintf (buf, "ERR: Levels [%2d][%2d][%2d][%2d][%2d][%2d][%2d][%2d] %s",
			 Player.level[0], Player.level[1], 
			 Player.level[2], Player.level[3], Player.level[4], 
			 Player.level[5], Player.level[6], Player.level[7], Player.name);
      		 errore =prterr(INT_ERR, buf);
	       }
		   else
	       {
			if (0)
  	  		{
				sprintf (buf, "birth                 : %ld", Player.birth); log(buf);
	       	}
			if (0)
  	  		{
				sprintf (buf, "played                : %d", Player.played); log(buf);
	       	}
			
	   	if (Player.title)
  	  		{
				if (strlen(Player.title) >80)
  	  			{
					sprintf (buf, "title                 : %s (%d >80 car.)", Player.title,strlen(Player.title));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.last_logon <0)
	       	{
				sprintf (buf, "last_logon            : %ld (<0)", Player.last_logon);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.act <0)
	       	{
				sprintf (buf, "act                   : %ld (<0)", Player.act);
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.name)
	       	{
				if (strlen(Player.name) >20)
	       		{
					sprintf (buf, "name                  : %s (%d >20 car.)", Player.name,strlen(Player.name));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			else
	       	{
				sprintf (buf, "name                  : (null)");
      		 	errore =prterr(INT_ERR, buf);
	       	}
			if (Player.authcode)
	       	{
				if (strlen(Player.authcode) >7)
	       		{
					sprintf (buf, "authcode              : %s (%d >7 car.)", Player.authcode,strlen(Player.authcode));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.WimpyLevel)
	       	{
				if (strlen(Player.WimpyLevel) >4)
	       		{
					sprintf (buf, "WimpyLevel            : %s (%d >4 car.)", Player.WimpyLevel,strlen(Player.WimpyLevel));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (0 && Player.dummy)
	       	{
				if (strlen(Player.dummy) >19)
	       		{
					sprintf (buf, "dummy                 : %s (%d >19 car.)", Player.dummy,strlen(Player.dummy));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (Player.pwd)
	       	{
				if (strlen(Player.pwd) >11)
	       		{
					sprintf (buf, "pwd                   : %s (%d >11 car.)", Player.pwd,strlen(Player.pwd));
      		 		errore =prterr(INT_ERR, buf);
	       		}
	       	}
			if (errore)
      			log("-------------------------------------------------------------------------------}");
	       }
	    }
	    fclose( pFile );
	 }
      }
   }
   else
   {
      sprintf(buf, "Cannot open dir %s", PLAYERS_DIR );
	  log(buf);
   }
   return;
}

}

int main (int argc, char *argv[])
{
	int opz =getopt(argc, argv, "ogrsmzl");
	switch (opz)
	{
		case 'r':
  			checkrent();
			break;
		case 'g':
  			checkplayer();
			break;
		case 'o':
  			caricaobj();
			break;
		case 's':
  			caricaroom();
			break;
		case 'm':
  			caricamob();
			break;
		case 'z':
  			caricazona();
    case 'l':
        lastlogon();
			break;
   		default:
			aiuto_sintassi();
			break;
	}
	return 0;
}
