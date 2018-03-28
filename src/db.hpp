/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: db.h,v 1.2 2002/02/13 12:30:57 root Exp $
 * */
#ifndef __DB_HPP
#define __DB_HPP
/***************************  System  include ************************************/
#include <cstdio>
/***************************  Local    include ************************************/
#include "hash.hpp"
#include "specialproc_other.hpp"
#include "specialproc_room.hpp"
namespace Alarmud {
/* data files used by the game system */

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
#define RENT_INACTIVE 2    /* delete the users rent files after 2 month */
#define NEW_ZONE_SYSTEM
#define killfile "killfile"
#define fread_number(FF)  fread_number_int(FF,__FILE__,__LINE__,curfile)
#define OBJ_DIR "objects"
#define MOB_DIR "mobiles"
// Refactored stuff
extern const long beginning_of_time;
extern int no_mail;
extern int top_of_scripts;
extern int top_of_world; /* ref to the top element of world */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data* room_db[WORLD_SIZE];
#endif
extern struct weather_data weather_info;
extern struct obj_data* object_list; /* the global linked list of obj's */
extern struct char_data* character_list; /* global l-list of chars          */

extern struct zone_data* zone_tabl;
extern int top_of_zone_table;
extern struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
extern struct player_index_element* player_table; /* index to player file   */
extern int top_of_p_table; /* ref to top of table             */
extern int top_of_p_file;
extern long total_bc;
extern long room_count;
extern long mob_count;
extern long obj_count;
extern long total_mbc;
extern long total_obc;
extern int top_of_objt;
extern int top_of_mobt;
extern struct index_data* mob_index; /* index table for mobile file     */
extern struct index_data* obj_index; /* index table for object file     */
extern struct help_index_element* help_index;
extern struct help_index_element* wizhelp_index;
extern int top_of_wizhelpt;
extern int top_of_helpt;

extern FILE* mob_f; /* file containing mob prototypes  */
extern FILE* obj_f; /* obj prototypes                  */
extern FILE* help_fl; /* file for help texts (HELP <kwd>)*/
extern FILE* wizhelp_fl; /* file for wizhelp */



extern int mob_tick_count;
extern char wmotd[MAX_STRING_LENGTH];
extern char credits[MAX_STRING_LENGTH]; /* the Credits List                */
extern char news[MAX_STRING_LENGTH]; /* the news                        */
extern char wiznews[MAX_STRING_LENGTH]; /* wiz news                        */
extern char motd[MAX_STRING_LENGTH]; /* the messages of today           */
extern char help[MAX_STRING_LENGTH]; /* the main help page              */
extern char info[MAX_STRING_LENGTH]; /* the info text                   */
extern char wizlist[MAX_STRING_LENGTH * 2]; /* the wizlist                     */
extern char princelist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
extern char immlist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
extern char rarelist[MAX_STRING_LENGTH * 2]; /*Acidus 2004-show rare*/
extern char login[MAX_STRING_LENGTH];
// Old stuff
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
extern struct zone_data* zone_table; /* table of reset data */




/* element in monster and object index-tables   */
struct index_data {
	int iVNum;      /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj        */
	genericspecial_func func;
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
extern struct time_info_data time_info;

#define ZONE_NEVER           0
#define ZONE_EMPTY           1
#define ZONE_ALWAYS          2
#define ZONE_ASTRAL          4
#define ZONE_DESERT          8
#define ZONE_ARCTIC         16
#define ZONE_UNDER_GROUND   32
int fwrite_string (FILE* fl, char* buf);
void fwrite_flag( FILE* pFile, unsigned long ulFlags );
void SaveTheWorld();
void boot_db();
void reset_time();
void update_time();
void build_player_index();
struct index_data* generate_indices(FILE* fl, int* top, int* sort_top, int* alloc_top, const char* dirname) ;
void cleanout_room(struct room_data* rp);
void completely_cleanout_room(struct room_data* rp);
void load_one_room(FILE* fl, struct room_data* rp);
void boot_world();
void boot_saved_zones();
void boot_saved_rooms();
void allocate_room(long room_number);
void setup_dir(FILE* fl, long room, int dir);
void renum_zone_table(int spec_zone);
void boot_zones();
struct char_data* read_mobile(int nr, int type);
struct obj_data* read_object(int nr, int type);
void zone_update();
void reset_zone(int zone);
int is_empty(int zone_nr);
int load_char(char* name, struct char_file_u* char_element);
void store_to_char(struct char_file_u* st, struct char_data* ch);
void char_to_store(struct char_data* ch, struct char_file_u* st);
int create_entry(char* name);
void save_char(struct char_data* ch, sh_int load_room, int bonus);
/* void save_char(struct char_data *ch, sh_int load_room); */
int compare(struct player_index_element* arg1, struct player_index_element
			*arg2);
long fread_number_int( FILE* pFile,const char* cmdfile,int cmdline,const char* infofile);
long fread_if_number( FILE* pFile );
char* fread_string(FILE* fl);
void free_char(struct char_data* ch);
void free_obj(struct obj_data* obj);
int file_to_string(const char* name, char* buf);
void ClearDeadBit(struct char_data* ch);
void reset_char(struct char_data* ch);
void clear_char(struct char_data* ch);
void clear_object(struct obj_data* obj);
void init_char(struct char_data* ch);
int real_mobile(int iVNum);
int real_object(int iVNum);
int ObjRoomCount(int nr, struct room_data* rp);
int MobRoomCount(int nr, struct room_data* rp);
void ReadTextZone(FILE* fl) ;
int CheckKillFile(int iVNum);
int str_len(char* buf);
void reboot_text(struct char_data* ch, char* arg, int cmd);
void InitScripts();
void ReloadRooms();
void FreeZone(int zone_nr);
void write_obj_to_file( struct obj_data* obj, FILE* f );
void InsertObject( struct obj_data* pObj, int nVNum );
void InsertMobile( struct char_data* pMob, int nVNum );
void Start_Auction();
} // namespace Alarmud
#endif


