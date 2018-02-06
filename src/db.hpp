/* AlarMUD
* $Id: db.h,v 1.2 2002/02/13 12:30:57 root Exp $
 * */
/* data files used by the game system */
#ifndef __DB
#define __DB 1
#define WORLD_FILE        "myst.wld"     /* room definitions           */
#define MOB_FILE          "myst.mob"     /* monster prototypes         */
#define OBJ_FILE          "myst.obj"     /* object prototypes          */
#define ZONE_FILE         "myst.zon"   /* zone defs & command tables */
#define POSEMESS_FILE     "myst.pos"   /* for 'pose'-command         */
#define MESS_FILE         "myst.dam"   /* damage message             */
#define SOCMESS_FILE      "myst.act"   /* messgs for social acts     */
#define LOGIN_FILE        "login"
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define WIZNEWS_FILE      "wiznews"       /* for the 'wiznews' command  */
#define MOTD_FILE         "motd"          /* messages of today          */
#define WIZ_MOTD_FILE     "wizmotd"
#define PLAYER_FILE       "players.dat"   /* the player database        */
#define PLAYERS_DIR       "players"
#define RENT_DIR          "rent"
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define HELP_KWRD_FILE    "helptbl"       /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>              */
#define WIZ_HELP_FILE     "wizhelptbl"      /* For wizhelp <keyword>      */
#define INFO_FILE         "info"          /* for INFO                   */
#define WIZLIST_FILE      "wizlist"       /* for WIZLIST                */
#define MAIL_FILE          "mud_mail"          /* */
#define DELETED_DIR       "deleted"
#define DELETED_RENT_DIR  "deletedRent"

#define REAL 0
#define VIRTUAL 1


/* structure for the reset commands */
struct reset_com {
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
struct zone_data {
	char* name;             /* name of this zone                  */
	int num;                /* number of this zone                 */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	int top;                /* upper limit for rooms in this zone */
	int bottom;             /* bottom limit for rooms in this zone */
	short start;            /* has this zone been reset yet?      */

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com* cmd;  /* command table for reset                   */
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




/* element in monster and object index-tables   */
struct index_data {
	int iVNum;      /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj        */
	int (*func)( struct char_data*, int, char*, void*, int );
	/* special procedure for this mob/obj       */
	char* specname;
	char* specparms;
	void* data;
	char* name;
	char* short_desc;
	char* long_desc;
};




/* for queueing zones for update   */
struct reset_q_element {
	int zone_to_reset;            /* ref to zone_data */
	struct reset_q_element* next;
};



/* structure for the update queue     */
struct reset_q_type {
	struct reset_q_element* head;
	struct reset_q_element* tail;
};

extern struct reset_q_type gReset_q;



struct player_index_element {
	char* name;
	int nr;
};


struct help_index_element {
	char* keyword;
	long pos;
};


#define ZONE_NEVER           0
#define ZONE_EMPTY           1
#define ZONE_ALWAYS          2
#define ZONE_ASTRAL          4
#define ZONE_DESERT          8
#define ZONE_ARCTIC         16
#define ZONE_UNDER_GROUND   32
#endif
