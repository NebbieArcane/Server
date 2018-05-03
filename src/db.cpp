/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
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
#include "db.hpp"
#include "auction.hpp"
#include "comm.hpp"
#include "act.other.hpp"
#include "mobact.hpp"
#include "modify.hpp"
#include "mail.hpp"
#include "fight.hpp"
#include "act.social.hpp"
#include "gilde.hpp"
#include "interpreter.hpp"
#include "spell_parser.hpp"
#include "reception.hpp"
#include "weather.hpp"
#include "handler.hpp"
#include "opinion.hpp"
#include "spell_list.hpp"

#include "Registered.hpp"
#include "regen.hpp"
#include "script.hpp"
#include "Sql.hpp"

namespace Alarmud {

/**************************************************************************
 *  declarations of most of the 'global' variables                         *
 ************************************************************************ */
const long beginning_of_time = BEG_OF_TIME;
int no_mail = 0;
int top_of_scripts = 0;
int top_of_world = 0; /* ref to the top element of world */
#if HASH
struct hash_header room_db;
#else
struct room_data* room_db[WORLD_SIZE];
#endif

struct obj_data* object_list = 0; /* the global linked list of obj's */
struct char_data* character_list = 0; /* global l-list of chars          */

struct zone_data* zone_table = NULL; /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element* player_table = 0; /* index to player file   */
int top_of_p_table = 0; /* ref to top of table             */
int top_of_p_file = 0;
long total_bc = 0;
long room_count = 0;
long mob_count = 0;
long obj_count = 0;
long total_mbc = 0;
long total_obc = 0;


/*
 **  distributed monster stuff
 */
int mob_tick_count = 0;
char wmotd[MAX_STRING_LENGTH];
char credits[MAX_STRING_LENGTH]; /* the Credits List                */
char news[MAX_STRING_LENGTH]; /* the news                        */
char wiznews[MAX_STRING_LENGTH]; /* wiz news                        */
char motd[MAX_STRING_LENGTH]; /* the messages of today           */
char help[MAX_STRING_LENGTH]; /* the main help page              */
char info[MAX_STRING_LENGTH]; /* the info text                   */
char wizlist[MAX_STRING_LENGTH * 2]; /* the wizlist                     */
char princelist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
char immlist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
char rarelist[MAX_STRING_LENGTH * 2]; /*Acidus 2004-show rare*/
char login[MAX_STRING_LENGTH];

FILE* mob_f; /* file containing mob prototypes  */
FILE* obj_f; /* obj prototypes                  */
FILE* help_fl; /* file for help texts (HELP <kwd>)*/
FILE* wizhelp_fl; /* file for wizhelp */

struct index_data* mob_index; /* index table for mobile file     */
struct index_data* obj_index; /* index table for object file     */
struct help_index_element* help_index = 0;
struct help_index_element* wizhelp_index = 0;
int top_of_mobt = 0; /* top of mobile index table       */
int top_of_objt = 0; /* top of object index table       */
int top_of_sort_mobt = 0;
int top_of_sort_objt = 0;
int top_of_alloc_mobt = 0;
int top_of_alloc_objt = 0;
int top_of_helpt; /* top of help index table         */
int top_of_wizhelpt; /* top of wiz help index table         */

struct time_info_data time_info; /* the infomation about the time   */
struct weather_data weather_info; /* the infomation about the weather */
long saved_rooms[WORLD_SIZE];
long number_of_saved_rooms = 0;
struct script_com* gpComp = NULL;
struct scripts* gpScript_data = NULL;
struct reset_q_type gReset_q = { NULL, NULL };
char curfile[256]; /* Informazioni sul file in lettura */

/*************************************************************************
 *  routines for booting the system                                       *
 *********************************************************************** */
void FrozeHim_old(struct char_data* ch, int hp) {
	char buf[500];
	snprintf(buf, 499,
			 "$c0115Non ti sembra che %d hp siano davvero un po' troppi?$c0007\n\r",
			 hp);
	send_to_char(buf, ch);
	SET_BIT(ch->specials.act, PLR_FREEZE);
	buglog(LOG_PLAYERS, "%s congelato per HP", GET_NAME(ch));
	do_title(ch, "has been frozen by Alar! [CDB]", CMD_TITLE);
}

void FrozeHim(struct char_data* ch, int hp) {
	char buf[500];

	if(GetMaxLevel(ch) < IMMORTAL) {
		snprintf(buf, 499,
				 "$c0115Non ti sembra che %d hp siano davvero un po' troppi?$c0007\n\r",
				 hp);
		send_to_char(buf, ch);
		SET_BIT(ch->specials.act, PLR_FREEZE);
		buglog(LOG_PLAYERS, "%s congelato per HP", GET_NAME(ch));
		do_title(ch, "has been frozen by Alar! [CDB]", CMD_TITLE);
	}
	else {
		snprintf(buf, 499,
				 "$c0115Non ti sembra che %d hp siano troppi? Segnala la cosa ai capoccia$c0007\n\r",
				 hp);
		send_to_char(buf, ch);
		buglog(LOG_PLAYERS, "%s e' immortale ma gli HP sono strani...",
			   GET_NAME(ch));
	}
}

/* body of the booting system */
void boot_db() {
	int i;

	mudlog(LOG_CHECK, "Boot db -- BEGIN.");

	mudlog(LOG_CHECK, "Resetting the game time:");
	reset_time();

	mudlog(LOG_CHECK, "Reading newsfile, credits, help-page, info and motd.");
	file_to_string(NEWS_FILE, news);
	file_to_string(WIZNEWS_FILE, wiznews);
	file_to_string(CREDITS_FILE, credits);
	file_to_string(MOTD_FILE, motd);
	file_to_string(WIZ_MOTD_FILE, wmotd);
	file_to_string(HELP_PAGE_FILE, help);
	file_to_string(INFO_FILE, info);
	file_to_string(WIZLIST_FILE, wizlist);
	file_to_string(LOGIN_FILE, login);

	mudlog(LOG_CHECK, "Initializing Script Files.");

	/* some machines are pre-allocation specific when dealing with realloc */
	gpScript_data = (struct scripts*) malloc(sizeof(struct scripts));
	if(gpScript_data == NULL) {
		mudlog(LOG_SYSERR, "Cannot allocate memory for gpScript_data");
		abort();
	}
	CommandSetup();
	InitScripts();
	mudlog(LOG_CHECK, "Opening mobile, object and help files.");
	if(!(mob_f = fopen(MOB_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","Opening mob file",strerror(errno));
		abort();
	}

	if(!(obj_f = fopen(OBJ_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","Opening obj file",strerror(errno));
		abort();
	}
	if(!(help_fl = fopen(HELP_KWRD_FILE, "r"))) {
		mudlog(LOG_ERROR, "   Could not open help file.");
	}
	else {
		help_index = build_help_index(help_fl, &top_of_helpt);
	}
	if(!(wizhelp_fl = fopen(WIZ_HELP_FILE, "r"))) {
		mudlog(LOG_ERROR, "   Could not open wizhelp file.");
	}
	else {
		wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);
	}
#if CLEAN_AT_BOOT
	mudlog(LOG_CHECK, "Clearing inactive players");
	clean_playerfile();
#else
	mudlog(LOG_CHECK, "Skipping inactive players check");
#endif

	mudlog(LOG_CHECK, "Booting mail system.");
	if(!scan_mail_file()) {
		mudlog(LOG_ERROR, "   Mail system error -- mail system disabled!");
		no_mail = 1;
	}
#if ENABLE_AUCTION
	Start_Auction();
#endif
	mudlog(LOG_CHECK, "Loading zone table.");
	boot_zones();

	mudlog(LOG_CHECK, "Loading saved zone table.");
	boot_saved_zones();

	mudlog(LOG_CHECK, "Loading rooms.");
	boot_world();

	mudlog(LOG_CHECK, "Loading saved rooms.");
	boot_saved_rooms();

	mudlog(LOG_CHECK, "Generating index tables for mobile and object files.");
	mob_index = generate_indices(mob_f, &top_of_mobt, &top_of_sort_mobt,
								 &top_of_alloc_mobt, MOB_DIR);
	obj_index = generate_indices(obj_f, &top_of_objt, &top_of_sort_objt,
								 &top_of_alloc_objt, OBJ_DIR);

	mudlog(LOG_CHECK, "Renumbering zone table.");
	renum_zone_table(0);

	mudlog(LOG_CHECK, "Generating player index.");
	build_player_index();

	mudlog(LOG_CHECK, "Loading fight messages.");
	load_messages();

	mudlog(LOG_CHECK, "Loading social messages.");
	boot_social_messages();

	mudlog(LOG_CHECK, "Loading pose messages.");
	boot_pose_messages();

	mudlog(LOG_CHECK, "Assigning function pointers:");
	if(!no_specials) {
		mudlog(LOG_CHECK, "   Mobiles, Objects, Rooms.");
		assign_speciales();
		mudlog(LOG_CHECK, "   Guilds.");
		BootGuilds();
	}

	mudlog(LOG_CHECK, "   Commands.");
	assign_command_pointers();

	mudlog(LOG_CHECK, "   Nannies.");
	assign_nannies_pointers();

	mudlog(LOG_CHECK, "   Spells.");
	assign_spell_pointers();
	boot_spells();

	mudlog(LOG_CHECK, "Updating characters with saved items:");
	update_obj_file();

#if LIMITED_ITEMS
	PrintLimitedItems();
#endif

	mudlog(LOG_CHECK, "Loading objects for saved rooms.");
	ReloadRooms();

	for(i = 0; i <= top_of_zone_table; i++) {
		char* s;
		int d, e;
		s = zone_table[i].name;
		d = (i ? (zone_table[i - 1].top + 1) : 0);
		zone_table[i].bottom = d;
		e = zone_table[i].top;
		mudlog(LOG_WORLD, "Performing boot-time init of %d:%s (rooms %d-%d).",
			   zone_table[i].num, s, d, e);
		zone_table[i].start = 0;

		if(i == 0) {
			mudlog(LOG_WORLD, "Performing boot-time reload of static mobs.");
			reset_zone(0);
		}

		if(i == 1) {
			mudlog(LOG_WORLD, "Reset of %s", s);
			reset_zone(1);
		}
	}

	gReset_q.head = gReset_q.tail = 0;

	mudlog(LOG_CHECK, "Boot db -- DONE.");
}

/* reset the time in the game from file */
void reset_time() {

	//struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

	moontype = time_info.day;

	switch(time_info.hours) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4: {
		weather_info.sunlight = SUN_DARK;
		switch_light(MOON_SET);
		break;
	}
	case 5:
	case 6: {
		weather_info.sunlight = SUN_RISE;
		switch_light(SUN_RISE);
		break;
	}
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18: {
		weather_info.sunlight = SUN_LIGHT;
		break;
	}
	case 19:
	case 20: {
		weather_info.sunlight = SUN_SET;
		break;
	}
	case 21:
	case 22:
	case 23:
	default: {
		switch_light(SUN_DARK);
		weather_info.sunlight = SUN_DARK;
		break;
	}
	}

	mudlog(LOG_CHECK, "   Current Gametime: %dH %dD %dM %dY.",
		   static_cast<int>(time_info.hours), static_cast<int>(time_info.day),
		   static_cast<int>(time_info.month), static_cast<int>(time_info.year));

	weather_info.pressure = 960;
	if((time_info.month >= 7) && (time_info.month <= 12)) {
		weather_info.pressure += dice(1, 50);
	}
	else {
		weather_info.pressure += dice(1, 80);
	}

	weather_info.change = 0;

	if(weather_info.pressure <= 980) {
		if((time_info.month >= 3) && (time_info.month <= 14)) {
			weather_info.sky = SKY_LIGHTNING;
		}
		else {
			weather_info.sky = SKY_LIGHTNING;
		}
	}
	else if(weather_info.pressure <= 1000) {
		if((time_info.month >= 3) && (time_info.month <= 14)) {
			weather_info.sky = SKY_RAINING;
		}
		else {
			weather_info.sky = SKY_RAINING;
		}
	}
	else if(weather_info.pressure <= 1020) {
		weather_info.sky = SKY_CLOUDY;
	}
	else {
		weather_info.sky = SKY_CLOUDLESS;
	}
}

/* update the time file */
void update_time() {
	return;
}

struct wizs {
	char name[20];
	int level;
};

int intcomp(struct wizs* j, struct wizs* k) {
	return (k->level - j->level);
}

char* GeneraSezione(int livello, struct wizlistgen* list_wiz) {
#define SBB 10240
	char buf[512];
	static char bigbuf[SBB];
	int center, i, j, ciclo;
	bigbuf[0] = '\0';
	switch(livello) {
	case IMMENSO:
		sprintf(buf, "$c0011-* Immenso *-$c0007\n\r");
		if(list_wiz->number[livello] > 1) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEI_CREATORI:
		sprintf(buf, "$c0011-* Maestro dei Creatori *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEL_CREATO:
		sprintf(buf, "$c0011-* Maestro del Creato *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case QUESTMASTER:
		sprintf(buf, "$c0011-* Maestro del Fato *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case CREATORE:
		sprintf(buf, "$c0011-* Creatore *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEGLI_DEI:
		sprintf(buf, "$c0011-* Maestro degli Dei *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case DIO:
		sprintf(buf, "$c0011-* Dio *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case DIO_MINORE:
		sprintf(buf, "$c0011 -* Dio Minore *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case IMMORTALE:
		sprintf(buf, "$c0011-* Immortale *-$c0007\n\r");
		break;
	case PRINCIPE:
		sprintf(buf, "$c0011-* Principi *-$c0007\n\r");
		break;
	}
	/*   if (list_wiz->number[livello]==0)
	 return("\0"); */
	center = 38 - (int)(Ansi_len(buf) / 2);
	for(i = 0; i <= center; i++) {
		strcat(bigbuf, " ");
	}
	strcat(bigbuf, buf);

	for(i = 0; i < list_wiz->number[livello]; i++) {
		sprintf(buf, "%s %s\n\r", list_wiz->lookup[livello].stuff[i].name,
				list_wiz->lookup[livello].stuff[i].title);

		center = 38 - (int)(Ansi_len(buf) / 2);
		for(j = 0; j <= center; j++) {
			strncat(bigbuf, " ", SBB);
		}
		strncat(bigbuf, buf, SBB);
	}
	for(; livello > DIO_MINORE && i < ciclo; i++) {
		sprintf(buf, "%s %s\n\r", " ", " ");

		center = 38 - (int)(Ansi_len(buf) / 2);
		for(j = 0; j <= center; j++) {
			strncat(bigbuf, " ", SBB);
		}
		strncat(bigbuf, buf, SBB);
	}
	return (bigbuf);
}
/**
 * Search a toon name in the
 */
bool getFromDb(const char* cname,const char* pwd, const char* title) {
	string name(cname);
	DB* db = Sql::getMysql();
	//odb::session s;
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	toon pg("","");
	if(!db->query_one<toon>(toonQuery::name==name,pg)) {
		pg.name=name;
		pg.password.assign(pwd);
		pg.title.assign(title);
		mudlog(LOG_CONNECT, "Creating record for %s", pg.name.c_str());
		try {
			db->persist<toon>(pg);
			t.commit();
			return true;
		}
		catch(odb::exception &e) {
			mudlog(LOG_ERROR, "Error creating record for %s %s", pg.name.c_str(),e.what());
		}
	}
	t.commit();
	return false;
}

/* generate index table for the player file */
void build_player_index() {
	using namespace boost::filesystem;
	struct wizlistgen list_wiz;
	int j, i;
	char buf[512];

	/* might use ABS_MAX_CLASS here some time */
	for(j = 0; j < MAX_CLASS; j++) {
		list_wiz.number[j] = 0;
	}

	top_of_p_table = 0;
	path p(current_path());
	p/=PLAYERS_DIR; // Overloaded operator: concats adding path separator
	vector<path> todelete;
	if(exists(p) and is_directory(p)) {
		for(auto &entry : boost::make_iterator_range(directory_iterator(p), directory_iterator())) {
			const path &file=entry.path();
			if(is_regular_file(file) and file.extension()==".dat") {
				FILE* pFile;
				struct char_file_u Player;
				if(!(pFile = fopen(file.c_str(), "r"))) {
					continue;
				}
				if(fread(&Player, 1, sizeof(Player), pFile)
						== sizeof(Player)) {
					int max;
					if(strcasecmp(file.stem().c_str(),Player.name)) {
						mudlog(LOG_SYSERR,"Strangeness: %s contains wrong name %s",file.filename().c_str(),Player.name);
						todelete.push_back(file);
						fclose(pFile);
						continue;
					}
					if(forceDbInit and not getFromDb(Player.name,Player.pwd,Player.title)) {
						mudlog(LOG_ERROR,"Updated: %s for %s",file.filename().c_str(),Player.name);

					}
					top_of_p_table++;

					for(j = 0, max = 0; j < MAX_CLASS; j++) {
						if(Player.level[j] > MAX_IMMORT) {
							Player.level[j] = 0;
							max = ABS_MAX_LVL + 1;
						}

						if(Player.level[j] > max) {
							max = Player.level[j];
						}
					}
					if(max >= (ABS_MAX_LVL + 1)) {
						mudlog(LOG_ERROR,
							   "ERR: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",
							   Player.name,
							   static_cast<unsigned int>(Player.level[0]),
							   static_cast<unsigned int>(Player.level[1]),
							   static_cast<unsigned int>(Player.level[2]),
							   static_cast<unsigned int>(Player.level[3]),
							   static_cast<unsigned int>(Player.level[4]),
							   static_cast<unsigned int>(Player.level[5]),
							   static_cast<unsigned int>(Player.level[6]),
							   static_cast<unsigned int>(Player.level[7]));
						mudlog(LOG_CHECK, "ERR: %s", file.c_str());
					}
					else if(max >= MAESTRO_DEL_CREATO) {
						/*		       (max==PRINCIPE && Player.points.exp>=PRINCEEXP) */
						/**Modifica Urhar sull' esperienza dei principi: con il nuovo livello
						 il check sui px non e' piu' necessario */
						mudlog(LOG_CHECK,
							   "GOD: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",
							   Player.name,
							   static_cast<unsigned int>(Player.level[0]),
							   static_cast<unsigned int>(Player.level[1]),
							   static_cast<unsigned int>(Player.level[2]),
							   static_cast<unsigned int>(Player.level[3]),
							   static_cast<unsigned int>(Player.level[4]),
							   static_cast<unsigned int>(Player.level[5]),
							   static_cast<unsigned int>(Player.level[6]),
							   static_cast<unsigned int>(Player.level[7]));

						list_wiz.lookup[max].stuff[list_wiz.number[max]].name =
							(char*) strdup(Player.name);
						list_wiz.lookup[max].stuff[list_wiz.number[max]].title =
							(char*) strdup(Player.title);
						list_wiz.number[max]++;
					}
				}
				fclose(pFile);
			}
		}
	}
	for(auto &file : todelete) {
		mudlog(LOG_SYSERR,"Removed invalid file %s",file.c_str());
		remove(file.string());
	}

	mudlog(LOG_CHECK, "Began Wizlist Generation.");

	sprintf(wizlist, "\033[2J\033[0;0H\n\r\n\r");
	for(i = IMMENSO; i > IMMORTALE; i--) {
		strncat(wizlist, GeneraSezione(i, &list_wiz), MAX_STRING_LENGTH * 2);
	}
	strncat(wizlist, "\n\r", MAX_STRING_LENGTH * 2);
	j = 0;
	for(i = DIO_MINORE; i <= IMMENSO; i++) {
		j += list_wiz.number[i];
	}
	sprintf(buf, "$c0007Totale Dei: %d\n\r", j);
	strncat(wizlist, buf, MAX_STRING_LENGTH);

	/* Immortali */
	sprintf(immlist, "\033[2J\033[0;0H\n\r\n\r");
	strncat(immlist, GeneraSezione(IMMORTALE, &list_wiz), MAX_STRING_LENGTH);
	/* Principi */
	sprintf(princelist, "\033[2J\033[0;0H\n\r\n\r");
	strncat(princelist, GeneraSezione(PRINCIPE, &list_wiz), MAX_STRING_LENGTH);

	return;
}

void ReplaceInIndex(struct index_data* pIndex, char* szName, int nRNum,
					int nVNum, int nTop) {

	if(nRNum < 0 || nRNum >= nTop) {
		mudlog(LOG_SYSERR, "Invalid RNum in ReplaceInIndex (db.c).");
		return;
	}

	pIndex[nRNum].iVNum = nVNum;
	pIndex[nRNum].pos = -1;
	pIndex[nRNum].name = strdup(szName);
	pIndex[nRNum].data = NULL;
}

struct index_data* InsertInIndex(struct index_data* pIndex, char* szName,
								 int nVNum, int* alloc_top, int* top) {
	if(*top >= *alloc_top) {
		if(!(pIndex = (struct index_data*) realloc(pIndex,
					  (*top + 50) * sizeof(struct index_data)))) {
			mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
			assert(0);
		}
		*alloc_top += 50;
	}
	pIndex[*top].iVNum = nVNum;
	pIndex[*top].pos = -1;
	pIndex[*top].name = strdup(szName);
	pIndex[*top].number = 0;
	pIndex[*top].func = 0;
	pIndex[*top].data = NULL;
	(*top)++;
	return pIndex;
}

void InsertObject(struct obj_data* pObj, int nVNum) {
	int nRNum = real_object(nVNum);
	if(nRNum < 0) {
		obj_index = InsertInIndex(obj_index, pObj->name, nVNum,
								  &top_of_alloc_objt, &top_of_objt);
	}
	else {
		ReplaceInIndex(obj_index, pObj->name, nRNum, nVNum, top_of_objt);
	}
}

void InsertMobile(struct char_data* pMob, int nVNum) {
	int nRNum = real_mobile(nVNum);
	if(nRNum < 0) {
		obj_index = InsertInIndex(mob_index, GET_NAME(pMob), nVNum,
								  &top_of_alloc_mobt, &top_of_mobt);
	}
	else {
		ReplaceInIndex(mob_index, GET_NAME(pMob), nRNum, nVNum, top_of_mobt);
	}
}

void read_object_to_memory(int nVNum) {
	int i = real_object(nVNum);
	if(i >= 0) {
		obj_index[i].data = (void*) read_object(i, REAL);
	}
}

int compare_index(const void* p1, const void* p2) {
	const struct index_data* s1, *s2;
	s1 = static_cast<const struct index_data*>(p1);
	s2 = static_cast<const struct index_data*>(p2);
	return (s1->iVNum - s2->iVNum);
}

/* generate index table for object or monster file */
struct index_data* generate_indices(FILE* fl, int* top, int* sort_top,
									int* alloc_top, const char* dirname) {
	FILE* f;
	DIR* dir;
	struct index_data* index;
	struct dirent* ent;
	long i = 0, di = 0, vnum, j;
	long bc = 2000;
	long dvnums[2000]; /* guess 2000 stored objects is enuff */
	int mobvnum = 0;
	char buf[82], tbuf[128];
	char loaded[100000];
	for(i = 0; i < 100000; i++) {
		loaded[i] = 0;
	}
	i = 0;
	/* scan main obj file */
	rewind(fl);
	for(;;) {
		if(fgets(buf, sizeof(buf), fl)) {
			if(*buf == '#') {
				if(!i) {  /* first cell */
					CREATE(index, struct index_data, bc);
				}
				else if(i >= bc) {
					if(!(index = (struct index_data*) realloc(index,
								 (i + 50) * sizeof(struct index_data)))) {
						mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
						assert(0);
					}
					bc += 50;
				}
				sscanf(buf, "#%d", &index[i].iVNum);
				mobvnum = index[i].iVNum;
				if(!loaded[mobvnum]) {
					loaded[mobvnum] = 1;
					sprintf(tbuf, "%s/%d", dirname, index[i].iVNum);
					/* Se non esiste nella dir obj/mob salvati */
					if((f = fopen(tbuf, "rt")) == NULL) {
						index[i].pos = ftell(fl);
						index[i].name =
							(index[i].iVNum < 99999) ?
							fread_string(fl) : strdup("omega");
					}
					else {
						index[i].pos = -1;
						fscanf(f, "#%*d\n");
						index[i].name =
							(index[i].iVNum < 99999) ?
							fread_string(f) : strdup("omega");
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
			}
			else {
				if(*buf == '%' && buf[1] == '%') {  /* EOF */
					break;
				}
			}
		}
		else {
			fprintf(stderr, "generate indices");
			assert(0);
		}
	}
	*sort_top = i;
	*alloc_top = bc;
	*top = *sort_top;
	mudlog(LOG_CHECK, "Sorting %s....", dirname);
	qsort(index, i, sizeof(index_data), compare_index);
	mudlog(LOG_CHECK, "Done!");
	/* scan for directory entrys */
	if((dir = opendir(dirname)) == NULL) {
		mudlog(LOG_CHECK, "Ignored missing directory %s", dirname);
		return (index);
	}
	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		vnum = atoi(ent->d_name);
		if(vnum == 0) {
			continue;
		}
		/* search if vnum was already sorted in main database */
		for(j = 0; j < di; j++)
			if(dvnums[j] == vnum) {
				break;
			}
		if(dvnums[j] == vnum) {
			continue;
		}
		sprintf(buf, "%s/%s", dirname, ent->d_name);
		if((f = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		if(!i) {
			CREATE(index, struct index_data, bc);
		}
		else if(i >= bc) {
			if(!(index = (struct index_data*) realloc(index,
						 (i + 50) * sizeof(struct index_data)))) {
				mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
				assert(0);
			}
			bc += 50;
		}
		fscanf(f, "#%*d\n");
		index[i].iVNum = vnum;
		index[i].pos = -1;
		index[i].name =
			(index[i].iVNum < 99999) ? fread_string(f) : strdup("omega");
		index[i].number = 0;
		index[i].func = 0;
		index[i].data = NULL;
		fclose(f);
		i++;
	}
	*alloc_top = bc;
	*top = i;
	*sort_top = *top;
	mudlog(LOG_CHECK, "Extra Sorting %s ....", dirname);
	qsort(index, i, sizeof(index_data), compare_index);
	mudlog(LOG_CHECK, "Done!");
	return (index);
}

void cleanout_room(struct room_data* rp) {
	int i;
	struct extra_descr_data* exptr, *nptr;

	free(rp->name);
	rp->name = NULL;
	free(rp->description);
	rp->description = NULL;
	for(i = 0; i < 6; i++) {
		if(rp->dir_option[i]) {
			free(rp->dir_option[i]->general_description);
			free(rp->dir_option[i]->keyword);
			free(rp->dir_option[i]);
			rp->dir_option[i] = NULL;
		}
	}

	for(exptr = rp->ex_description; exptr; exptr = nptr) {
		nptr = exptr->next;
		free(exptr->keyword);
		free(exptr->description);
		free(exptr);
	}
	rp->ex_description = NULL;
}

void completely_cleanout_room(struct room_data* rp) {
	struct char_data* ch;
	struct obj_data* obj;

	while(rp->people) {
		ch = rp->people;
		act(
			"The hand of god sweeps across the land and you are swept into the Void.",
			FALSE, NULL, NULL, NULL, TO_VICT);
		char_from_room(ch);
		char_to_room(ch, 0); /* send character to the void */
	}

	while(rp->contents) {
		obj = rp->contents;
		obj_from_room(obj);
		obj_to_room(obj, 0); /* send item to the void */
	}

	cleanout_room(rp);
}

void load_one_room(FILE* fl, struct room_data* rp) {
	char chk[161];
	int bc = 0;
	long int tmp;

	struct extra_descr_data* new_descr;

	bc = sizeof(struct room_data);

	rp->name = fread_string(fl);
	if(IsTest()) {
		mudlog(LOG_WORLD, "LR: vnum %d name %s", rp->number, rp->name);
	}

	if(rp->name && *rp->name) {
		bc += strlen(rp->name);
	}
	rp->description = fread_string(fl);
	if(rp->description && *rp->description) {
		bc += strlen(rp->description);
	}

	if(top_of_zone_table >= 0) {
		int zone;
		fscanf(fl, " %*d ");

		/* OBS: Assumes ordering of input rooms */

		for(zone = 0;
				rp->number > zone_table[zone].top && zone <= top_of_zone_table;
				zone++)
			;
		if(zone > top_of_zone_table) {
			mudlog(LOG_SYSERR, "Room %ld (%s) is outside of any zone (%d/%d).\n",
				   rp->number, rp->name,zone,top_of_zone_table);
			exit(1);
		}
		rp->zone = zone;
	}
	sprintf(curfile, "Caricando stanza %ld .\n", rp->number);
	tmp = fread_number(fl);
	rp->room_flags = tmp;
	tmp = fread_number(fl);
	rp->sector_type = tmp;

	sprintf(curfile, "Letto flags= %ld , sector= %ld \n", rp->room_flags,
			rp->sector_type);
	if(tmp == -1) {
		sprintf(curfile, "Stanza con settore -1 (teleport) room: %ld %s\n",
				rp->number, rp->name);
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s teletime %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_time = tmp;
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s teletarget %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_targ = tmp;
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s telemask %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_mask = tmp;
		if(IS_SET(TELE_COUNT, rp->tele_mask)) {
			tmp = fread_number(fl);
			sprintf(curfile,
					"Stanza con settore -1 (teleport) room: %ld %s telecount %ld\n",
					rp->number, rp->name, tmp);
			rp->tele_cnt = tmp;
		}
		else {
			rp->tele_cnt = 0;
		}
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s true sector %ld\n",
				rp->number, rp->name, tmp);
		rp->sector_type = tmp;
	}
	else {
		rp->tele_time = 0;
		rp->tele_targ = 0;
		rp->tele_mask = 0;
		rp->tele_cnt = 0;
	}

	if(tmp == SECT_WATER_NOSWIM || tmp == SECT_UNDERWATER) {
		/* river
		 * read direction and rate of flow
		 * su myst non tutte le stanze subaquee hanno la corrente, per cui
		 * uso fread_if_number che non da` errore se non trova un numero
		 * */
		tmp = fread_if_number(fl);
		rp->river_speed = tmp;
		tmp = fread_if_number(fl);
		rp->river_dir = tmp;
	}

	if(rp->room_flags & TUNNEL) {
		/* read in mobile limit on tunnel */
		tmp = fread_number(fl);
		/* corretto per evitare stanze in cui non si puo' entrare Gaia 2001 */
		rp->moblim = MAX(tmp, 1);
	}

	rp->funct = 0;
	rp->light = 0; /* Zero light sources */

	for(tmp = 0; tmp <= 5; tmp++) {
		rp->dir_option[tmp] = 0;
	}

	rp->ex_description = 0;

	while(fscanf(fl, " %160s \n", chk) == 1) {
		switch(*chk) {
		case 'D':
			setup_dir(fl, rp->number, atoi(chk + 1));
			bc += sizeof(struct room_direction_data);
			break;
		case 'E': /* extra description field */

			CREATE(new_descr, struct extra_descr_data, 1);
			bc += sizeof(struct extra_descr_data);

			new_descr->keyword = fread_string(fl);
			if(new_descr->keyword && *new_descr->keyword) {
				bc += strlen(new_descr->keyword);
			}
			else {
				sprintf(curfile, "No keyword in room %ld\n", rp->number);
			}

			new_descr->description = fread_string(fl);
			if(new_descr->description && *new_descr->description) {
				bc += strlen(new_descr->description);
			}
			else {
				sprintf(curfile, "No desc in room %ld\n", rp->number);
			}

			new_descr->next = rp->ex_description;
			rp->ex_description = new_descr;
			break;
		case 'L':
			rp->szWhenBrightAtNight = fread_string(fl);
			rp->szWhenBrightAtDay = fread_string(fl);
			break;
		case 'S': /* end of current room */

			total_bc += bc;
			room_count++;

			if(IS_SET(rp->room_flags, SAVE_ROOM)) {
				saved_rooms[number_of_saved_rooms] = rp->number;
				number_of_saved_rooms++;
			}
			else {
				FILE* fp;
				char buf[255];

				sprintf(buf, "world/%ld", rp->number);
				fp = fopen(buf, "r");
				if(fp) {
					saved_rooms[number_of_saved_rooms] = rp->number;
					number_of_saved_rooms++;
					fclose(fp);
				}
			}
			return;
		case 'C':
			/* Commento, non deve fare nulla. Il tutto deve stare su una sola
			 * linea. */
			break;
		default:
			mudlog(LOG_ERROR,
				   "unknown auxiliary code `%s' in room load of #%ld", chk,
				   rp->number)
			;
			break;
		}
	}
}

/* load the rooms */
void boot_world() {
	FILE* fl;
	long lVNum, last;
	struct room_data* rp;

#if HASH
	init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
	init_world(room_db);
#endif
	character_list = 0;
	object_list = 0;

	if(!(fl = fopen(WORLD_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","fopen",strerror(errno));
		mudlog(LOG_ERROR, "boot_world: could not open world file.");
		assert(0);
	}
	/* GG Sto meditando di dividere il file delle stanze in un file per ogni zona
	 * Questo e' un pezzo di codice che scorre una directory leggendo tutti i
	 * files presenti
	 *   DIR *dir;
	 *  struct wizlistgen list_wiz;
	 *  int j, i, center;
	 *  char buf[ 256 ];
	 *
	 *   might use ABS_MAX_CLASS here some time
	 *  for(j = 0; j < MAX_CLASS; j++)
	 *    list_wiz.number[j] = 0;
	 *
	 * top_of_p_table = 0;
	 *
	 * if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
	 * {
	 *   struct dirent *ent;
	 *   while( ( ent = readdir( dir ) ) != NULL )
	 *   {
	 *     FILE *pFile;
	 *     char szFileName[ 40 ];
	 *
	 *     if( *ent->d_name == '.' )
	 *       continue;
	 *
	 *     sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );
	 *
	 *     if( ( pFile = fopen( szFileName, "r+" ) ) != NULL )
	 */

	last = 0;
	while(1 == fscanf(fl, " #%ld\n", &lVNum)) {
		allocate_room(lVNum);
		rp = real_roomp(lVNum);
		if(rp)
#ifdef CYGWIN
			bzero((char*) rp, sizeof(*rp));
#else
			bzero(rp, sizeof(*rp));
#endif
		else {
			fprintf(stderr, "Error, room %ld not in database!(%ld)\n", lVNum,
					last);
			assert(0);
		}

		rp->number = lVNum;
		load_one_room(fl, rp);
		last = lVNum;
	}

	fclose(fl);
}

void allocate_room(long room_number) {
	if(room_number > top_of_world) {
		top_of_world = room_number;
	}
#if HASH
	hash_find_or_create(&room_db, room_number);
#else
	room_find_or_create(room_db, room_number);
#endif
}

/* read direction data */
void setup_dir(FILE* fl, long room, int dir) {
	long tmp;
	int trashroom = 0;
	struct room_data* rp, dummy;

	rp = real_roomp(room);

	if(!rp) {
		mudlog(LOG_CHECK, "Trashing room: %d", room);
		trashroom = 1;
		rp = &dummy; /* this is a quick fix to make the game */
		dummy.number = room; /* stop crashing   */
	}

	CREATE(rp->dir_option[dir], struct room_direction_data, 1);

	rp->dir_option[dir]->general_description = fread_string(fl);
	rp->dir_option[dir]->keyword = fread_string(fl);

	rp->dir_option[dir]->exit_info = fread_number(fl);

	rp->dir_option[dir]->key = fread_number(fl);

	rp->dir_option[dir]->to_room = fread_number(fl);

	tmp = -1;
	fscanf(fl, " %ld ", &tmp);
	rp->dir_option[dir]->open_cmd = tmp;
	if(trashroom) {

		/* Ho fatto lo stesso tutta la routine per posizionare correttamente
		 * il file, pero adesso rimuovo e libero la memoria
		 * */
		free(rp->dir_option[dir]->general_description);
		free(rp->dir_option[dir]->keyword);
		free(rp->dir_option[dir]);
	}

}

void boot_saved_zones() {
	DIR* dir;
	FILE* fp;
	struct dirent* ent;
	char buf[80];
	long zone;

	if((dir = opendir("zones")) == NULL) {
		mudlog(LOG_ERROR, "Unable to open zones directory.\n");
		return;
	}

	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		zone = atoi(ent->d_name);
		if(!zone || zone > top_of_zone_table) {
			continue;
		}
		sprintf(buf, "zones/%s", ent->d_name);
		if((fp = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		mudlog(LOG_CHECK, "Loading saved zone %ld:%s", zone,
			   zone_table[zone].name);
		LoadZoneFile(fp, zone);
		fclose(fp);
	}
}

void boot_saved_rooms() {
	DIR* dir;
	FILE* fp;
	long oldnum = 0;
	struct dirent* ent;
	char buf[80];
	struct room_data* rp;
	long rooms = 0, vnum;

	if((dir = opendir("rooms")) == NULL) {
		mudlog(LOG_ERROR, "Unable to open rooms directory.\n");
		return;
	}

	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		vnum = atoi(ent->d_name);
		if(!vnum || vnum > top_of_world) {
			continue;
		}
		sprintf(buf, "rooms/%s", ent->d_name);
		if((fp = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		while(!feof(fp)) {
			fscanf(fp, "#%ld\n", &oldnum);
			mudlog(LOG_CHECK, "Extra room %s (old number %d)", buf, oldnum);
			if((rp = real_roomp(vnum)) == NULL) {  /* empty room */
				rp = (struct room_data*) malloc(sizeof(struct room_data));
				if(rp)
#ifdef CYGWIN
					bzero((char*) rp, sizeof(struct room_data));
#else
					bzero(rp, sizeof(struct room_data));
#endif
#if HASH
				room_enter(&room_db, vnum, rp);
#else
				room_enter(room_db, vnum, rp);
#endif
			}
			else {
				cleanout_room(rp);
			}
			rp->number = vnum;
			load_one_room(fp, rp);
		}
		fclose(fp);
		rooms++;
	}
	if(rooms) {
		mudlog(LOG_CHECK, "Loaded %ld rooms", rooms);
	}
}

#define LOG_ZONE_ERROR(ch, type, zone, cmd) { mudlog( LOG_ERROR,"error in zone %s cmd %ld (%c) resolving %s number",zone_table[zone].name, cmd, ch, type);}
#define CKNUM(num,s) /*if (num<zone_table[zone].bottom || num>zone_table[zone].top) mudlog(LOG_ERROR,"NUMERR in %d %s\n%s %d",zone_table[zone].num,zone_table[zone].name,s,num);*/

void renum_zone_table(int spec_zone) {
	long zone, comm, start, end;
	struct reset_com* cmd;

	if(spec_zone) {
		start = end = spec_zone;
	}
	else {
		start = 0;
		end = top_of_zone_table;
	}

	for(zone = start; zone <= end; zone++) {
		for(comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
			switch((cmd = zone_table[zone].cmd + comm)->command) {
			case 'M':
				CKNUM(cmd->arg1, "mob");
				cmd->arg1 = real_mobile(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('M', "mobile", zone, comm);
				}
				if(cmd->arg3 < 0 || real_roomp(cmd->arg3) == NULL) {
					LOG_ZONE_ERROR('M', "room", zone, comm);
				}
				break;
			case 'C':
				CKNUM(cmd->arg1, "mob");
				cmd->arg1 = real_mobile(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('C', "mobile", zone, comm);
				}
				break;
			case 'O':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('O', "object", zone, comm);
				}
				if(cmd->arg3 != NOWHERE) {
					/*cmd->arg3 = real_room(cmd->arg3);*/
					if(cmd->arg3 < 0 || real_roomp(cmd->arg3) == NULL) {
						LOG_ZONE_ERROR('O', "room", zone, comm);
					}
				}
				break;
			case 'G':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('G', "object", zone, comm);
				}
				break;
			case 'E':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('E', "object", zone, comm);
				}
				break;
			case 'P':
				CKNUM(cmd->arg1, "obj");
				CKNUM(cmd->arg3, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('P', "object", zone, comm);
				}
				cmd->arg3 = real_object(cmd->arg3);
				if(cmd->arg3 < 0) {
					LOG_ZONE_ERROR('P', "object", zone, comm);
				}
				break;
			case 'D':
				/*cmd->arg1 = real_room(cmd->arg1);*/
				if(cmd->arg1 < 0 || real_roomp(cmd->arg1) == NULL) {
					LOG_ZONE_ERROR('D', "room", zone, comm);
				}
				break;
			}
		}
	}
}

/* load the zone table and command tables */
void boot_zones() {

	FILE* fl;
	int zon = 0, cmd_no = 0, expand, tmp, bc = 100, cc = 22, znumber;
	char* check, buf[256];

	if(!(fl = fopen(ZONE_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","boot_zones",strerror(errno));
		assert(0);
	}

	for(;;) {
		/*read Riga1 = #nnn */
		fscanf(fl, " #%d\n", &znumber);
		/*read riga2 Nome della zona~ */
		check = fread_string(fl);
		if(*check == '$') {
			mudlog(LOG_WORLD,"Letto $");
			break;
		} /* end of file */
		SetStatus(check, NULL);
		/* alloc a new zone */

		if(!zon) {
			CREATE(zone_table, struct zone_data, bc);
		}
		else if(zon >= bc) {
			if(!(zone_table = (struct zone_data*) realloc(zone_table,
							  (zon + 10) * sizeof(struct zone_data)))) {
				mudlog(LOG_ERROR,"%s:%s","boot_zones realloc",strerror(errno));
				assert(0);
			}
			bc += 10;
		}
		zone_table[zon].num = znumber;
		zone_table[zon].name = check;
		zone_table[zon].bottom = -1;
		zone_table[zon].top = -1;
		zone_table[zon].lifespan = -1;
		zone_table[zon].reset_mode = -1;
		fscanf(fl, "%d", &zone_table[zon].top);
		fscanf(fl, "%d", &zone_table[zon].lifespan);
		fscanf(fl, "%d", &zone_table[zon].reset_mode);
		zone_table[zon].bottom = zon ? zone_table[zon - 1].top + 1 : 0;
		strncpy(buf, zone_table[zon].name, 20);
		buf[20] = '\0';
		mudlog(LOG_WORLD, "#%d(%d): %s %d-%d %dm %d", zon, zone_table[zon].num,
			   buf, zone_table[zon].bottom, zone_table[zon].top,
			   zone_table[zon].lifespan, zone_table[zon].reset_mode);

		/* read the command table */

		/*
		 * new code to allow the game to be 'static' i.e. all the mobs are saved
		 * in one big zone file, and restored later.
		 */

		cmd_no = 0;

		if(zon == 0) {
			cc = 20;
		}

		for(expand = 1;;) {
			if(expand) {
				if(!cmd_no) {
					CREATE(zone_table[zon].cmd, struct reset_com, cc);
				}
				else if(cmd_no >= cc) {
					cc += 5;
					if(!(zone_table[zon].cmd = (struct reset_com*) realloc(
												   zone_table[zon].cmd,
												   (cc * sizeof(struct reset_com))))) {
						mudlog(LOG_ERROR,"%s:%s","reset command load",strerror(errno));
						assert(0);
					}
				}
			}

			expand = 1;
			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
			if(!strchr(" HFMCOGEPD*;SR",
					   zone_table[zon].cmd[cmd_no].command)) {
				fgets(buf, 250, fl); /* skip command */
				mudlog(LOG_CHECK, "'%c' strangeness in zon %s: %s",
					   zone_table[zon].cmd[cmd_no].command, check, buf);
				continue;
				/* Codice strano nel file zon */
			}

			if(zone_table[zon].cmd[cmd_no].command == 'S') {
				break;
			}

			if(zone_table[zon].cmd[cmd_no].command == '#') {
				mudlog(LOG_ERROR, "!!!!!!!!!!!!!!!!!!!!!!");
				fgets(buf, 80, fl); /* skip command */
				mudlog(LOG_ERROR, "-->%s", buf); /* Sta saltando una zona...*/
			}
			if(zone_table[zon].cmd[cmd_no].command == 'R') {
				fgets(buf, 80, fl); /* skip command */
				mudlog(LOG_ERROR, "R not implemented in %s", buf); /* Sta saltando una zona...*/
			}

			if(zone_table[zon].cmd[cmd_no].command == '*') {
				expand = 0;
				fgets(buf, 250, fl); /* skip command */
				continue;
			}

			if(zone_table[zon].cmd[cmd_no].command == ';') {
				expand = 0;
				fgets(buf, 250, fl); /* skip command */
				continue;
			}
			fgets(buf, 255, fl);
			zone_table[zon].cmd[cmd_no].if_flag = 0;
			zone_table[zon].cmd[cmd_no].arg1 = -1;
			zone_table[zon].cmd[cmd_no].arg2 = 0;
			zone_table[zon].cmd[cmd_no].arg3 = -1;
			zone_table[zon].cmd[cmd_no].arg4 = 0;
			sscanf(buf, " %d %d %d %d %d", &tmp,
				   &zone_table[zon].cmd[cmd_no].arg1,
				   &zone_table[zon].cmd[cmd_no].arg2,
				   &zone_table[zon].cmd[cmd_no].arg3,
				   &zone_table[zon].cmd[cmd_no].arg4);
			zone_table[zon].cmd[cmd_no].if_flag = tmp;
			/*         mudlog(LOG_CHECK,"Skipping comment: %s",buf);*/
			cmd_no++;
		}
		zon++;
		if(zon == 1) {
			/* fix the cheat */
			/*      if (fl != tmp_fl && fl != 0)
			 fclose(fl);
			 mudlog(LOG_CHECK,"File chiuso???");
			 fl = tmp_fl;*/
		}

	}
	top_of_zone_table = --zon;
	free(check);
	fclose(fl);
}

/*************************************************************************
 *  procedures for resetting, both play-time and boot-time                *
 *********************************************************************** */

/* read a mobile from MOB_FILE */
struct char_data* read_mobile(int nr, int type) {
	int i;
	long tmp, tmp2, tmp3, bc = 0;
	struct char_data* mob;
	char letter;

	i = nr;
	if(type == VIRTUAL) {
		if((nr = real_mobile(nr)) < 0) {
			mudlog(LOG_ERROR, "Mobile (V) %d does not exist in database.", i);
			return NULL;
		}
	}

	fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(mob, struct char_data, 1);

	if(!mob) {
		mudlog(LOG_SYSERR, "Cannot create mob?! db.c read_mobile");
		return nullptr;
	}

	bc = sizeof(struct char_data);
	clear_char(mob);

	mob->specials.last_direction = -1; /* this is a fix for wander */

	/***** String data *** */
	mob->player.name = fread_string(mob_f);
	if(mob->player.name) {
		bc += strlen(mob->player.name);
	}
	mob->player.short_descr = fread_string(mob_f);
	if(mob->player.short_descr) {
		bc += strlen(mob->player.short_descr);
	}
	mob->player.long_descr = fread_string(mob_f);
	if(mob->player.long_descr) {
		bc += strlen(mob->player.long_descr);
	}
	mob->player.description = fread_string(mob_f);
	if(mob->player.description) {
		bc += strlen(mob->player.description);
	}
	mob->player.title = 0;

	/* *** Numeric data *** */

	mob->mult_att = 1.0;
	mob->specials.spellfail = 101;

	mob->specials.act = fread_number(mob_f);
	SET_BIT(mob->specials.act, ACT_ISNPC);
	if(IS_SET(mob->specials.act, ACT_POLYSELF)) {
		mudlog(LOG_ERROR, "ACT_POLYSELF bit set in mob #%d.",
			   mob_index[nr].iVNum);
		REMOVE_BIT(mob->specials.act, ACT_POLYSELF);
	}

	mob->specials.affected_by = fread_number(mob_f);

	mob->specials.alignment = fread_number(mob_f);

	mob->player.iClass = CLASS_WARRIOR;

	fscanf(mob_f, " %c ", &letter);
	if(letter == 'S') {
		/* SIMPLE MOB */
		fscanf(mob_f, "\n");

		tmp = fread_number(mob_f);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.intel = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.wis = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.dex = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.con = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		mob->points.hitroll = 20 - fread_number(mob_f);

		tmp = fread_number(mob_f);

		if(tmp > 10 || tmp < -10) {
			tmp /= 10;
		}

		mob->points.armor = 10 * tmp;

		fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
		mob->points.max_hit = dice(tmp, tmp2) + tmp3;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.max_mana = 10;
		mob->points.max_move = 50;

		tmp = fread_number(mob_f);
		if(tmp == -1) {
			mob->points.gold = fread_number(mob_f);
			GET_EXP(mob) = fread_number(mob_f);
			GET_RACE(mob) = fread_number(mob_f);
			if(IsGiant(mob)) {
				mob->abilities.str += number(1, 4);
			}
			if(IsSmall(mob)) {
				mob->abilities.str -= 1;
			}
		}
		else {
			mob->points.gold = tmp;
			GET_EXP(mob) = fread_number(mob_f);
		}
		mob->specials.position = fread_number(mob_f);

		mob->specials.default_pos = fread_number(mob_f);

		tmp = fread_number(mob_f);
		if(tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}
		else if(tmp < 6) {
			mob->player.sex = tmp - 3;
			mob->immune = fread_number(mob_f);
			mob->M_immune = fread_number(mob_f);
			mob->susc = fread_number(mob_f);
		}
		else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

		fscanf(mob_f, "\n");

		mob->player.iClass = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for(i = 0; i < 3; i++) {
			GET_COND(mob, i) = -1;
		}

		for(i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] = MAX(
					20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
	}/* FINE SIMPLE */
	else if(letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L') {
		if(letter == 'A' || letter == 'B' || letter == 'L') {
			mob->mult_att = (float) fread_number(mob_f);
			/*Fine prima riga con dati numerici */
#if 0
			/*  read in types: */
			for(i = 0; i < mob->mult_att && i < 10; i++) {
				mob->att_type[i] = fread_number(mob_f);
			}
#endif
		}

		fscanf(mob_f, "\n");
		/* Righe numeriche */
		/* LIVELLO THAC0 AC HITPBONUS DAMROLL
		 * -1 GOLD XPBONUS RACE or GOLD XPBONUS
		 * POSITON DEFAULTPOS SEX (se sex >3 allora IMMUNE METAIMM SUSC
		 */
		tmp = fread_number(mob_f);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.intel = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.wis = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.dex = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.con = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		mob->points.hitroll = 20 - fread_number(mob_f);

		mob->points.armor = 10 * fread_number(mob_f);

		tmp = fread_number(mob_f);
		mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8) + tmp;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.max_mana = 100;
		mob->points.max_move = 50;

		tmp = fread_number(mob_f);
		if(tmp == -1) {
			mob->points.gold = fread_number(mob_f);
			tmp = fread_number(mob_f);
			if(tmp >= 0) {
				GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
			}
			else {
				GET_EXP(mob) = -tmp;
			}
			GET_RACE(mob) = fread_number(mob_f);
			if(IsGiant(mob)) {
				mob->abilities.str += number(1, 4);
			}
			if(IsSmall(mob)) {
				mob->abilities.str -= 1;
			}
		}
		else {
			mob->points.gold = tmp;

			/* this is where the new exp will come into play */
			tmp = fread_number(mob_f);
			if(tmp >= 0) {
				GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
			}
			else {
				GET_EXP(mob) = -tmp;
			}
		}

		mob->specials.position = fread_number(mob_f);

		mob->specials.default_pos = fread_number(mob_f);

		tmp = fread_number(mob_f);
		if(tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}
		else if(tmp < 6) {
			mob->player.sex = tmp - 3;
			mob->immune = fread_number(mob_f);
			mob->M_immune = fread_number(mob_f);
			mob->susc = fread_number(mob_f);
		}
		else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

		/* read in the sound string for a mobile */
		if(letter == 'L') {
			mob->player.sounds = fread_string(mob_f);
			if(mob->player.sounds && *mob->player.sounds) {
				bc += strlen(mob->player.sounds);
			}

			mob->player.distant_snds = fread_string(mob_f);
			if(mob->player.distant_snds && *mob->player.distant_snds) {
				bc += strlen(mob->player.distant_snds);
			}
		}
		else {
			mob->player.sounds = 0;
			mob->player.distant_snds = 0;
		}

		if(letter == 'B') {
			SET_BIT(mob->specials.act, ACT_HUGE);
		}

		mob->player.iClass = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for(i = 0; i < 3; i++) {
			GET_COND(mob, i) = -1;
		}

		for(i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] = MAX(
					20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
	}
	else {   /* The old monsters are down below here */
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
		mob->points.armor = 10 * tmp;

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

		mob->player.iClass = fread_number(mob_f);

		fscanf(mob_f, " %ld ", &tmp);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.weight = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->player.height = tmp;

		for(i = 0; i < 3; i++) {
			fscanf(mob_f, " %ld ", &tmp);
			GET_COND(mob, i) = tmp;
		}
		fscanf(mob_f, " \n ");

		for(i = 0; i < 5; i++) {
			fscanf(mob_f, " %ld ", &tmp);
			mob->specials.apply_saving_throw[i] = tmp;
		}

		fscanf(mob_f, " \n ");

		/* Set the damage as some standard 1d4 */
		mob->points.damroll = 0;
		mob->specials.damnodice = 1;
		mob->specials.damsizedice = 6;

		/* Calculate THAC0 as a formular of Level */
		mob->points.hitroll = MAX(1, GET_LEVEL(mob, WARRIOR_LEVEL_IND) - 3);
	}

	mob->tmpabilities = mob->abilities;

	for(i = 0; i < MAX_WEAR; i++) {  /* Initialisering Ok */
		mob->equipment[i] = 0;
	}

	mob->nr = nr;

	mob->desc = 0;

	if(!IS_SET(mob->specials.act, ACT_ISNPC)) {
		SET_BIT(mob->specials.act, ACT_ISNPC);
	}

	mob->generic = 0;
	mob->commandp = 0;
	mob->commandp2 = 0;
	mob->waitp = 0;

	/* Check to see if associated with a script, if so, set it up */
	if(IS_SET(mob->specials.act, ACT_SCRIPT)) {
		REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
	}

	for(i = 0; i < top_of_scripts; i++) {
		if(gpScript_data[i].iVNum == mob_index[nr].iVNum) {
			SET_BIT(mob->specials.act, ACT_SCRIPT);
			mob->script = i;
			break;
		}
	}

	/* insert in list */

	mob->next = character_list;
	character_list = mob;

#if LOW_GOLD
	if(mob->points.gold >= 10) {
		mob->points.gold /= 5;
	}
	else if(mob->points.gold > 0) {
		mob->points.gold = 1;
	}
#endif

	/* set up things that all members of the race have */
	SetRacialStuff(mob);

	/* change exp for wimpy mobs (lower) */
	if(IS_SET(mob->specials.act, ACT_WIMPY)) {
		GET_EXP(mob) -= GET_EXP(mob) / 10;
	}

	/* change exp for agressive mobs (higher) */
	if(IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		GET_EXP(mob) += GET_EXP(mob) / 10;
		/* big bonus for fully aggressive mobs for now */
		if(!IS_SET(mob->specials.act, ACT_WIMPY) ||
				IS_SET(mob->specials.act, ACT_META_AGG)) {
			GET_EXP(mob) += (GET_EXP(mob) / 2);
		}
	}

	mob->points.mana = mana_limit(mob);
	mob->points.move = move_limit(mob);

	/* set up distributed movement system */

	mob->specials.tick = mob_tick_count++;

	if(mob_tick_count == TICK_WRAP_COUNT) {
		mob_tick_count = 0;
	}

	mob_index[nr].number++;

	total_mbc += bc;
	mob_count++;

	return (mob);
}

void clone_obj_to_obj(struct obj_data* obj, struct obj_data* osrc) {
	struct extra_descr_data* new_descr, *tmp_descr;
	int i;

	if(osrc->name) {
		obj->name = strdup(osrc->name);
	}
	if(osrc->short_description) {
		obj->short_description = strdup(osrc->short_description);
	}
	if(osrc->description) {
		obj->description = strdup(osrc->description);
	}
	if(osrc->action_description) {
		obj->action_description = strdup(osrc->action_description);
	}

	/* *** numeric data *** */

	obj->obj_flags.type_flag = osrc->obj_flags.type_flag;
	obj->obj_flags.extra_flags = osrc->obj_flags.extra_flags;
	obj->obj_flags.wear_flags = osrc->obj_flags.wear_flags;
	obj->obj_flags.value[0] = osrc->obj_flags.value[0];
	obj->obj_flags.value[1] = osrc->obj_flags.value[1];
	obj->obj_flags.value[2] = osrc->obj_flags.value[2];
	obj->obj_flags.value[3] = osrc->obj_flags.value[3];
	obj->obj_flags.weight = osrc->obj_flags.weight;
	obj->obj_flags.cost = osrc->obj_flags.cost;
	obj->obj_flags.cost_per_day = osrc->obj_flags.cost_per_day;

	/* *** extra descriptions *** */

	obj->ex_description = 0;

	if(osrc->ex_description) {
		for(tmp_descr = osrc->ex_description; tmp_descr;
				tmp_descr = tmp_descr->next) {
			CREATE(new_descr, struct extra_descr_data, 1);
			new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
			if(tmp_descr->keyword) {
				new_descr->keyword = strdup(tmp_descr->keyword);
			}
			if(tmp_descr->description) {
				new_descr->description = strdup(tmp_descr->description);
			}
			new_descr->next = obj->ex_description;
			obj->ex_description = new_descr;
		}
	}

	for(i = 0; i < MAX_OBJ_AFFECT; i++) {
		obj->affected[i].location = osrc->affected[i].location;
		obj->affected[i].modifier = osrc->affected[i].modifier;
	}

	if(osrc->szForbiddenWearToChar) {
		obj->szForbiddenWearToChar = strdup(osrc->szForbiddenWearToChar);
	}
	if(osrc->szForbiddenWearToRoom) {
		obj->szForbiddenWearToRoom = strdup(osrc->szForbiddenWearToRoom);
	}
}

int read_obj_from_file(struct obj_data* obj, FILE* f) {
	int i, tmp;
	long bc = 0L;
	char chk[161];
	struct extra_descr_data* new_descr;

	obj->name = fread_string(f);

	if(obj->name) {
		bc += strlen(obj->name);
	}
	obj->short_description = fread_string(f);
	if(obj->short_description) {
		bc += strlen(obj->short_description);
	}
	obj->description = fread_string(f);
	if(obj->description) {
		bc += strlen(obj->description);
	}
	obj->action_description = fread_string(f);
	if(obj->action_description) {
		bc += strlen(obj->action_description);
	}

	/* *** numeric data *** */

	obj->obj_flags.type_flag = fread_number(f);
	obj->obj_flags.extra_flags = fread_number(f);
	obj->obj_flags.wear_flags = fread_number(f);
	obj->obj_flags.value[0] = fread_number(f);
	obj->obj_flags.value[1] = fread_number(f);
	obj->obj_flags.value[2] = fread_number(f);
	obj->obj_flags.value[3] = fread_number(f);
	obj->obj_flags.weight = fread_number(f);
	obj->obj_flags.cost = fread_number(f);
	obj->obj_flags.cost_per_day = fread_number(f);

	/* *** extra descriptions *** */

	obj->ex_description = 0;

	while(fscanf(f, " %160s \n", chk) == 1 && *chk == 'E') {
		CREATE(new_descr, struct extra_descr_data, 1);
		new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
		bc += sizeof(struct extra_descr_data);
		new_descr->keyword = fread_string(f);
		if(new_descr->keyword) {
			bc += strlen(new_descr->keyword);
		}
		new_descr->description = fread_string(f);
		if(new_descr->description) {
			bc += strlen(new_descr->description);
		}

		new_descr->next = obj->ex_description;
		obj->ex_description = new_descr;
	}

	for(i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
		fscanf(f, " %d ", &tmp);
		obj->affected[i].location = tmp;
		fscanf(f, " %d \n", &tmp);
		obj->affected[i].modifier = tmp;
		if(fscanf(f, " %160s \n", chk) != 1) {
			i++;
			break;
		}
	}

	for(; (i < MAX_OBJ_AFFECT); i++) {
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;
	}

	SetStatus("Reading forbidden string in read_obj_from_file", NULL);

	if(*chk == 'P') {
		obj->szForbiddenWearToChar = fread_string(f);
		obj->szForbiddenWearToRoom = fread_string(f);
		fscanf(f, " %160s \n", chk);
	}
	else {
		obj->szForbiddenWearToChar = NULL;
		obj->szForbiddenWearToRoom = NULL;
	}

	SetStatus("Returning from read_obj_from_file", "None");

	return bc;
}

void write_obj_to_file(struct obj_data* obj, FILE* f) {
	int i;
	struct extra_descr_data* descr;

	fprintf(f, "#%d\n",
			obj->item_number >= 0 ? obj_index[obj->item_number].iVNum : 0);
	fwrite_string(f, obj->name);
	fwrite_string(f, obj->short_description);
	fwrite_string(f, obj->description);
	fwrite_string(f, obj->action_description);

	fprintf(f, "%d %d %d\n", obj->obj_flags.type_flag,
			obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);
	fprintf(f, "%d %d %d %d\n", obj->obj_flags.value[0],
			obj->obj_flags.value[1], obj->obj_flags.value[2],
			obj->obj_flags.value[3]);
	fprintf(f, "%d %d %d\n", obj->obj_flags.weight, obj->obj_flags.cost,
			obj->obj_flags.cost_per_day);

	/* *** extra descriptions *** */
	if(obj->ex_description)
		for(descr = obj->ex_description; descr; descr = descr->next) {
			fprintf(f, "E\n");
			fwrite_string(f, descr->keyword);
			fwrite_string(f, descr->description);
		}

	for(i = 0; i < MAX_OBJ_AFFECT; i++) {
		if(obj->affected[i].location != APPLY_NONE)
			fprintf(f, "A\n%d %d\n", obj->affected[i].location,
					obj->affected[i].modifier);
	}

	if(obj->szForbiddenWearToChar) {
		fprintf(f, "P\n");
		fwrite_string(f, obj->szForbiddenWearToChar);
		fwrite_string(f, obj->szForbiddenWearToRoom);
	}

}

/* read an object from OBJ_FILE */
struct obj_data* read_object(int nr, int type) {
	FILE* f;
	struct obj_data* obj;
	int i;
	long bc;
	char buf[100];

	SetStatus("read_object start", NULL);
	i = nr;
	if(type == VIRTUAL) {
		SetStatus("before real_object", NULL);
		nr = real_object(nr);
	}
	if(nr < 0 || nr >= top_of_objt) {
		mudlog(LOG_ERROR, "Object (V) %d does not exist in database.", i);
		return NULL;
	}

	SetStatus("before CREATE object", NULL);

	CREATE(obj, struct obj_data, 1);

	bc = sizeof(struct obj_data);

	SetStatus("before clear_object", NULL);
	clear_object(obj);

	if(obj_index[nr].data == NULL) {
		/* object haven't data structure */
		if(obj_index[nr].pos == -1) {
			/* object in external file */
			sprintf(buf, "%s/%d", OBJ_DIR, obj_index[nr].iVNum);
			if((f = fopen(buf, "rt")) == NULL) {
				mudlog(LOG_ERROR, "can't open object file for object %d",
					   obj_index[nr].iVNum);
				free(obj);
				return (0);
			}
			fscanf(f, "#%*d \n");
			SetStatus("before read_obj_from_file 1", NULL);
			read_obj_from_file(obj, f);
			fclose(f);
		}
		else {
			if(fseek(obj_f, obj_index[nr].pos, 0) == 0) {
				SetStatus("before read_obj_from_file 2", NULL);
				read_obj_from_file(obj, obj_f);
			}
			else {
				mudlog(LOG_ERROR,
					   "Cannot seek obj file at %l for obj n. %d(%d) in "
					   "read_object (%s).", obj_index[nr].pos, nr,
					   obj_index[nr].iVNum, __FILE__);
				free(obj);
				return NULL;
			}
		}
	}
	else {
		SetStatus("before clone_obj_to_obj", NULL);
		/* data for object present */
		clone_obj_to_obj(obj, (struct obj_data*) obj_index[nr].data);
	}

	SetStatus("before inzializing object", NULL);

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

	total_obc += bc;

	SetStatus("ending read_object", NULL);

	return (obj);
}

#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update() {
	int i;
	struct reset_q_element* update_u, *temp, *tmp2;

	/* enqueue zones */

	for(i = 0; i <= top_of_zone_table; i++) {
		if(zone_table[i].start) {
			if(zone_table[i].age < zone_table[i].lifespan
					&& zone_table[i].reset_mode) {

				(zone_table[i].age)++;
			}
			else if(zone_table[i].age < ZO_DEAD
					&& zone_table[i].reset_mode) {

				/* enqueue zone */

				CREATE(update_u, struct reset_q_element, 1);

				update_u->zone_to_reset = i;
				update_u->next = 0;

				if(!gReset_q.head) {
					gReset_q.head = gReset_q.tail = update_u;
				}
				else {
					gReset_q.tail->next = update_u;
					gReset_q.tail = update_u;
				}

				zone_table[i].age = ZO_DEAD;
			}
		}
	}

	/* dequeue zones (if possible) and reset */

	for(update_u = gReset_q.head; update_u; update_u = tmp2) {
		if(update_u->zone_to_reset > top_of_zone_table) {

			/*  this may or may not work */
			/*  may result in some lost memory, but the loss is not signifigant
			 *   over the short run
			 */
			update_u->zone_to_reset = 0;
			update_u->next = 0;
		}
		tmp2 = update_u->next;

		if(IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_ALWAYS)
				|| (IS_SET(zone_table[update_u->zone_to_reset].reset_mode,
						   ZONE_EMPTY)
					&& is_empty(update_u->zone_to_reset))) {
			SetStatus("Before reset_zone", NULL);
			reset_zone(update_u->zone_to_reset);
			SetStatus("After reset_zone", NULL);
			/* dequeue */

			if(update_u == gReset_q.head) {
				gReset_q.head = gReset_q.head->next;
			}
			else {
				for(temp = gReset_q.head; temp->next != update_u;
						temp = temp->next)
					;

				if(!update_u->next) {
					gReset_q.tail = temp;
				}

				temp->next = update_u->next;
			}
			free(update_u);
		}
	}
}

#if 0

typedef struct tagZoneCommand {
	int nCmdNo;

} ZoneCommand;

void ExecuteZoneCommand(ZoneCommand* pZC, NumberType NT) {

}
#endif

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */

/* I have gotten a memory out of bounds on this function, not sure where the */
/* problem came from... need to look for it, could possibly be a pointer */

/* going out of range or a variable not getting assigned. msw */

void reset_zone(int zone) {
	int cmd_no, nLastCmd = TRUE;
	char buf[256];
	char rbuf[256];
	struct char_data* pMob = NULL;
	struct char_data* pMaster = NULL;
	struct obj_data* pObj, *pCont;
	struct room_data* rp;
	//static int done = FALSE;
	struct char_data* pLastMob = 0;
	// Qui veniva messo il puntatore all'ultimo container utilizzato, dato che poi non veniva mai utilizzato
	// Lascio commentato nel caso scopra invece che mi era sfuggito l'utilizzo
	//struct obj_data* pLastCont = 0;
	char* s;
	int d, e;

	s = zone_table[zone].name;
	d = (zone ? (zone_table[zone - 1].top + 1) : 0);
	zone_table[zone].bottom = d;
	e = zone_table[zone].top;
	if(zone_table[zone].start == 0)
		sprintf(buf, "Run time initialization of zone %s (%d), rooms (%d-%d)",
				s, zone, d, e);
	else
		sprintf(buf, "Run time reset of zone %s (%d), rooms (%d-%d)", s, zone,
				d, e);

	mudlog(LOG_CHECK, buf);

	if(!zone_table[zone].cmd) {
		return;
	}

	for(cmd_no = 0;; cmd_no++) {
		if(ZCMD.command == 'S') {
			break;
		}

		if(nLastCmd || ZCMD.if_flag <= 0) {
			sprintf(rbuf, "<%d %d %d %d %d>",
					ZCMD.if_flag, ZCMD.arg1, ZCMD.arg2, ZCMD.arg3, ZCMD.arg4);
			switch(ZCMD.command) {
			case 'M': /* read a mobile */
				SetStatus("Command M", rbuf);
				rp = real_roomp(ZCMD.arg3);
				if((ZCMD.arg2 == 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2)  // World cap
						&& (ZCMD.arg4 == 0 || MobRoomCount(real_mobile(ZCMD.arg1), rp) < ZCMD.arg4)// Room cap
						&& !fighting_in_room(ZCMD.arg3)//Combattimento in corso
						&& !CheckKillFile(mob_index[ZCMD.arg1].iVNum)//Disabilitato dal kill file
						&& (pMob = read_mobile(ZCMD.arg1, REAL)) != NULL// Mob esiste
						&& rp != NULL) { // stanza esiste
					pLastMob = pMaster = pMob;
					pMob->specials.zone = zone;
					char_to_room(pMob, ZCMD.arg3);
					if(IS_SET(pMob->specials.act, ACT_SENTINEL)) {
						pMob->lStartRoom = ZCMD.arg3;
					}

					if(GET_RACE(pMob) > RACE_GNOME &&
							!strchr(zone_table[ zone ].races, GET_RACE(pMob))) {
						zone_table[ zone ].races[ strlen(zone_table[ zone ].races) ] =
							GET_RACE(pMob);
					}

					nLastCmd = TRUE;
				}
				else {
					pLastMob = pMaster = pMob = NULL;
					nLastCmd = FALSE;
				}
				if(rp == NULL)
					mudlog(LOG_ERROR, "M Cannot find room #%d in zone %s",
						   ZCMD.arg3, s);
				break;

			case 'C': /* read a mobile.  Charm them to follow prev. */
				SetStatus("Command C", rbuf);
				if((ZCMD.arg2 == 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2)
						&& (ZCMD.arg4 == 0 || MobRoomCount(real_mobile(ZCMD.arg1), rp) < ZCMD.arg4)
						&& !CheckKillFile(mob_index[ ZCMD.arg1 ].iVNum)
						&& pMaster
						&& (pMob = read_mobile(ZCMD.arg1, REAL)) != NULL) {
					pLastMob = pMob;
					pMob->specials.zone = zone;
					if(GET_RACE(pMob) > RACE_GNOME &&
							!strchr(zone_table[ zone ].races, GET_RACE(pMob)))
						zone_table[ zone ].races[ strlen(zone_table[ zone ].races) ] =
							GET_RACE(pMob);

					char_to_room(pMob, pMaster->in_room);
					/* add the charm bit to the dude.  */
					add_follower(pMob, pMaster);
					SET_BIT(pMob->specials.affected_by, AFF_CHARM);
					SET_BIT(pMob->specials.act, ZCMD.arg3);
					nLastCmd = TRUE;
				}
				else {
					pLastMob = pMob = NULL;
					nLastCmd = FALSE;
				}
				break;

			case 'Z': /* set the last mobile to this zone */
				SetStatus("Command Z", rbuf);
				if(pLastMob) {
					pLastMob->specials.zone = zone;

					if(GET_RACE(pLastMob) > RACE_GNOME &&
							!strchr(zone_table[ zone].races,
									GET_RACE(pLastMob)))
						zone_table[zone].races[strlen(zone_table[zone].races)] =
							GET_RACE(pLastMob);
				}
				break;

			case 'O': /* read an object */
				SetStatus("Command O", rbuf);
				pObj = NULL;
				nLastCmd = FALSE;
				if(ZCMD.arg1 >= 0 && (ZCMD.arg2 == 0 || obj_index[ ZCMD.arg1 ].number < ZCMD.arg2)
				  ) {
					if((ZCMD.arg3 >= 0 && (rp = real_roomp(ZCMD.arg3)) != NULL)) {
						if(ZCMD.arg4 == 0 || ObjRoomCount(ZCMD.arg1, rp) < ZCMD.arg4) {
							if((pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
								obj_to_room(pObj, ZCMD.arg3);
								nLastCmd = TRUE;
								//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
								//{ pLastCont = pObj; }
							}
						}
					}
					else {
						mudlog(LOG_ERROR, "O Cannot find room #%d in zone %s",
							   ZCMD.arg3, s);
					}
				}
				break;

			case 'P': /* object to object */
				SetStatus("Command P", rbuf);
				if(ZCMD.arg1 >= 0 &&
						(ZCMD.arg2 == 0 ||
						 obj_index[ ZCMD.arg1 ].number < ZCMD.arg2) &&
						(pCont = get_obj_num(ZCMD.arg3)) != NULL &&
						(pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
					obj_to_obj(pObj, pCont);
					nLastCmd = TRUE;
				}
				else {
					pObj = pCont = NULL;
					nLastCmd = FALSE;
				}
				break;

			case 'G': /* obj_to_char */
				SetStatus("Command G", rbuf);
				if(ZCMD.arg1 >= 0 &&
						(ZCMD.arg2 == 0 ||
						 obj_index[ ZCMD.arg1 ].number < ZCMD.arg2) &&
						pLastMob && (pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
					obj_to_char(pObj, pLastMob);
					//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
					//{ pLastCont = pObj; }
				}
				break;

			case 'H': /* hatred to char */
				SetStatus("Command H", rbuf);
				if(pLastMob) {
					AddHatred(pLastMob, ZCMD.arg1, ZCMD.arg2);
				}
				break;

			case 'F': /* fear to char */
				SetStatus("Command F", rbuf);
				if(pLastMob) {
					AddFears(pLastMob, ZCMD.arg1, ZCMD.arg2);
				}
				break;

			case 'E': /* object to equipment list */
				SetStatus("Command E", rbuf);
				if(ZCMD.arg1 >= 0 && (ZCMD.arg2 == 0 ||
									  obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
						pLastMob && (pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
					if(!pLastMob->equipment[ ZCMD.arg3 ]) {
						equip_char(pLastMob, pObj, ZCMD.arg3);
						//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
						//{ pLastCont = pObj; }
					}
					else {
						mudlog(LOG_ERROR, "eq error - zone %d, cmd %d, "
							   "item %d, mob %d, "
							   "loc %d in zone %s\n", zone, cmd_no,
							   obj_index[ ZCMD.arg1 ].iVNum,
							   mob_index[ pLastMob->nr ].iVNum,
							   ZCMD.arg3, s);
					}
				}
				break;

			case 'D': /* set state of door */
				SetStatus("Command D", rbuf);
				rp = real_roomp(ZCMD.arg1);
				if(rp && rp->dir_option[ZCMD.arg2]) {
					if(!IS_SET(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR)) {
						mudlog(LOG_ERROR,
							   "Door error - zone %d, cmd %d, loc %d (fixed) in %s",
							   zone, cmd_no, ZCMD.arg1, s);
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR);
					}
					switch(ZCMD.arg3) {
					case 0:
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						break;
					case 1:
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						break;
					case 2:
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						break;
					}
				}
				else {
					/* that exit doesn't exist anymore */
					mudlog(LOG_ERROR, "Exit error - zone %d, cmd %d, loc %d",
						   zone, cmd_no, ZCMD.arg1);
				}
				break;

			default:
				mudlog(LOG_ERROR, "Undef cmd '%c' in reset: zone %s cmd# %d\n\r",
					   ZCMD.command, s, cmd_no);
				break;
			}
		}
		else {
			nLastCmd = FALSE;
		}
	}

	zone_table[zone].age = 0;
	zone_table[zone].start = 1;
	SetStatus("Out of loop", rbuf);
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr) {
	struct descriptor_data* i;

	for(i = descriptor_list; i; i = i->next)
		if(!i->connected)
			if(real_roomp(i->character->in_room)->zone == zone_nr) {
				return (0);
			}

	return (1);
}

/*************************************************************************
 *  stuff related to the save/load player system                                                           *
 *********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(const char* name, struct char_file_u* char_element) {
	FILE* fl;
	struct stat fileinfo;
	char szFileName[41];
	long filesize = 0;

	sprintf(szFileName, "%s/%s.dat", PLAYERS_DIR, lower(name));
	if((fl = fopen(szFileName, "r")) != NULL) {
		if(stat(szFileName, &fileinfo)) {
			filesize = fileinfo.st_size;
		}
		else {
			filesize = 999999;
		}
		char_element->agemod = 0;
		fread(char_element, MIN(filesize, sizeof(struct char_file_u)), 1, fl);
		fclose(fl);
		/*
		 **  Kludge for ressurection
		 */
		char_element->talks[2] = FALSE; /* they are not dead */
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u* st, struct char_data* ch) {
	int i;
	int max;

	GET_SEX(ch) = st->sex;
	ch->player.iClass = st->iClass;

	for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
		ch->player.level[i] = st->level[i];
	}

	/* to make sure all levels above the normal are 0 */
	for(i = MAX_CLASS; i < ABS_MAX_CLASS; i++) {
		ch->player.level[i] = 0;
	}
	ch->points.exp = st->points.exp;

	/* azzero i contatori delle posizioni */
	for(i = 0; i < MAX_POSITION; i++) {
		GET_TEMPO_IN(ch, i) = 0;
	}

	GET_POS_PREV(ch) = POSITION_STANDING;

	GET_RACE(ch) = st->race;

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;

	if(*st->title) {
		CREATE(ch->player.title, char, strlen(st->title) + 1);
		strcpy(ch->player.title, st->title);
	}
	else {
		GET_TITLE(ch) = 0;
	}

	if(*st->description) {
		CREATE(ch->player.description, char, strlen(st->description) + 1);
		strcpy(ch->player.description, st->description);
	}
	else {
		ch->player.description = 0;
	}

	ch->player.hometown = st->hometown;

	ch->player.time.birth = st->birth;

	ch->player.time.played = st->played;

	ch->player.time.logon = time(0);

	for(i = 0; i <= MAX_TOUNGE - 1; i++) {
		ch->player.talks[i] = st->talks[i];
	}

	ch->player.weight = st->weight;
	ch->player.height = st->height;

	ch->abilities = st->abilities;
	ch->tmpabilities = st->abilities;
	mudlog(LOG_SAVE, "<-Mana/Hits prima di reload: %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	ch->points = st->points;
	mudlog(LOG_SAVE, "<-Mana/Hits dopo reload    : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	mudlog(LOG_SAVE, "<-MMana/MHits from points: %d/%d", ch->points.max_mana,
		   ch->points.max_hit);
	mudlog(LOG_SAVE, "<- Mana/ Hits from points: %d/%d", ch->points.mana,
		   ch->points.hit);

	SpaceForSkills(ch);

	if(IS_IMMORTAL(ch)) {
		max = 100;
	}
	else if(HowManyClasses(ch) >= 3) {
		max = 81;
	}
	else if(HowManyClasses(ch) == 2) {
		max = 86;
	}
	else {
		max = 95;
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		ch->skills[i].flags = st->skills[i].flags;
		ch->skills[i].special = st->skills[i].special;
		ch->skills[i].nummem = st->skills[i].nummem;
		ch->skills[i].learned = MIN(st->skills[i].learned, max);
	}

	/* Specials */
	ch->specials.spells_to_learn = st->spells_to_learn;
	ch->specials.alignment = st->alignment;

	ch->specials.act = st->act;
	ch->specials.WimpyLevel = atoi(st->WimpyLevel);
	if(IS_SET(ch->specials.act, PLR_WIMPY)
			&& (ch->specials.WimpyLevel < 1
				|| ch->specials.WimpyLevel > GET_MAX_HIT(ch) / 3 * 2)) {
		ch->specials.WimpyLevel = GET_MAX_HIT(ch) / 5;
	}
	if(!IS_SET(ch->specials.act, PLR_WIMPY)) {
		ch->specials.WimpyLevel = 0;
	}

	ch->specials.carry_weight = 0;
	ch->specials.carry_items = 0;
	ch->specials.pmask = 0;
	ch->specials.poofin = 0;
	ch->specials.poofout = 0;
	ch->specials.group_name = 0;
	ch->points.armor = 100;
	ch->points.hitroll = 0;
	ch->points.damroll = 0;
	ch->specials.affected_by = st->affected_by;
	ch->specials.affected_by2 = st->affected_by2;
	ch->specials.start_room = st->startroom;
	ch->player.vassallodi = 0;
	ch->specials.email = 0;
	ch->specials.realname = 0;
	ch->specials.authcode = 0;
	ch->specials.AuthorizedBy = 0;
	ch->specials.supporting = 0;
	ch->specials.bodyguarding = 0;
	ch->specials.bodyguard = 0;
	ch->specials.lastversion = 0;
	/* Reset posizioni */
	for(i = 0; i < MAX_POSITION; i++) {
		GET_TEMPO_IN(ch, i) = 0;
	}

	GET_POS_PREV(ch) = POSITION_STANDING;

	ch->player.speaks = st->speaks;
	ch->player.user_flags = st->user_flags;
	ch->player.extra_flags = st->extra_flags;
	ch->AgeModifier = st->agemod;
	CREATE(GET_NAME(ch), char, strlen(st->name) + 1);

	strcpy(GET_NAME(ch), st->name);

	/*GGPATCH*/
	if(*st->authcode) {

		CREATE(GET_AUTHCODE(ch), char, Registered::REG_CODELEN + 1);
		strncpy(GET_AUTHCODE(ch), st->authcode, Registered::REG_CODELEN);
	}
	else {
		GET_AUTHCODE(ch) = 0;
	}
	mudlog(LOG_PLAYERS, "Loading %s registrato come %s", GET_NAME(ch),
		   GET_AUTHCODE(ch));

	for(i = 0; i <= 4; i++) {
		ch->specials.apply_saving_throw[i] = 0;
	}

	for(i = 0; i <= 2; i++) {
		GET_COND(ch, i) = st->conditions[i];
	}

	/* Add all spell effects */
	for(i = 0; i < MAX_AFFECT; i++) {
		if(st->affected[i].type) {
			/* Inside file, we had to save a fake structure because reserving space for the pointer was architecture dependend
			 * Now, we copy the data in a temporary structure.
			 * Fortunately, the passed value will be copied so we dont need to allocate memory
			 */

			struct affected_type temp_affect;
			temp_affect.bitvector = st->affected[i].bitvector;
			temp_affect.duration = st->affected[i].duration;
			temp_affect.location = st->affected[i].location;
			temp_affect.modifier = st->affected[i].modifier;
			temp_affect.type = st->affected[i].type;
			temp_affect.next = (struct affected_type*) NULL;
			affect_to_char(ch, &temp_affect);
		}
	}
	mudlog(LOG_SAVE, "<-Mana/Hits dopo affect   : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));

	//Acidus 2003 - poiche la load_room e definita sul file come sh_int, uso i valori
	//negativi per le reception con vnum compreso tra 32768 e 65535 (oltre non si puo)

	if(st->load_room < -2) {
		ch->in_room = st->load_room + 65536;
	}
	else {
		ch->in_room = st->load_room;
	}

	ch->term = 0;

	/* set default screen size */
	ch->size = 25;

	affect_total(ch);

	mudlog(LOG_SAVE, "<-Mana/Hits dopo affecttot : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	ch->nMagicNumber = CHAR_VALID_MAGIC;
	/* Rimuove il FREEZE */
	if(IS_SET(ch->specials.act, PLR_FREEZE)) {
		REMOVE_BIT(ch->specials.act, PLR_FREEZE);
		mudlog(LOG_PLAYERS, "FREEZE removed from %s", GET_NAME(ch));
	}

} /* store_to_char */

/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data* ch, struct char_file_u* st) {
	int i;
	struct affected_type* af;
	struct obj_data* char_eq[MAX_WEAR];
	char buf[300];
	sh_int hit, mana, move;

	hit = GET_HIT(ch);
	mana = GET_MANA(ch);
	move = GET_MOVE(ch);

	/* Unaffect everything a character can be affected by */

	mudlog(LOG_SAVE, "Saving %s.dat", GET_NAME(ch));
	/* inizializzo area dummy */
	strcpy(st->dummy, "123456789012345678"); // SALVO la dummy e un array di 19
	for(i = 0; i < MAX_WEAR; i++) {
		if(ch->equipment[i]) {
			char_eq[i] = unequip_char(ch, i);
		}
		else {
			char_eq[i] = 0;
		}
	}
	mudlog(LOG_CHECK, "Removing all affects from %s", GET_NAME(ch));
	for(af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
		if(af) {
			/* Inside file, we had to save a fake structure because reserving space for the pointer was architecture dependend
			 * Now, we need to assign item per item
			 */
			st->affected[i].bitvector = af->bitvector;
			st->affected[i].duration = af->duration;
			st->affected[i].location = af->location;
			st->affected[i].modifier = af->modifier;
			st->affected[i].type = af->type;
			st->affected[i].next = 0;
			/* subtract effect of the spell or the effect will be doubled */
			affect_modify(ch, st->affected[i].location,
						  st->affected[i].modifier, st->affected[i].bitvector, FALSE);
			sprintf(buf, "Saving %s modifies %s by %d points", GET_NAME(ch),
					apply_types[st->affected[i].location],
					st->affected[i].modifier);

			af = af->next;
		}
		else {
			st->affected[i].type = 0; /* Zero signifies not used */
			st->affected[i].duration = 0;
			st->affected[i].modifier = 0;
			st->affected[i].location = 0;
			st->affected[i].bitvector = 0;
			st->affected[i].next = 0;
		}
	}

	if((i >= MAX_AFFECT) && af && af->next) {
		mudlog(LOG_CHECK, "WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
	}

	ch->tmpabilities = ch->abilities;

	st->birth = ch->player.time.birth;
	st->played = ch->player.time.played;
	st->played += (long)(time(0) - ch->player.time.logon);
	st->last_logon = time(0);

	ch->player.time.played = st->played;
	ch->player.time.logon = time(0);

	st->hometown = ch->player.hometown;

	st->weight = GET_WEIGHT(ch);
	st->height = GET_HEIGHT(ch);
	st->sex = GET_SEX(ch);
	st->iClass = ch->player.iClass;

	for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
		st->level[i] = ch->player.level[i];
	}

	st->race = GET_RACE(ch);

	ch->specials.charging = 0; /* null it out to be sure. */
	ch->specials.charge_dir = -1; /* null it out */

	st->abilities = ch->abilities;

	st->points = ch->points;

	st->alignment = ch->specials.alignment;
	st->spells_to_learn = ch->specials.spells_to_learn;
	st->act = ch->specials.act;
	st->affected_by = ch->specials.affected_by;
	st->affected_by2 = ch->specials.affected_by2;
	sprintf(st->WimpyLevel, "%03d", ch->specials.WimpyLevel);
	/* do not store group_name */
	st->startroom = ch->specials.start_room;
	st->extra_flags = ch->player.extra_flags;
	st->agemod = ch->AgeModifier;

	st->speaks = ch->player.speaks;
	st->user_flags = ch->player.user_flags;

	st->points.armor = 100;
	st->points.hitroll = 0;
	st->points.damroll = 0;

	if(GET_TITLE(ch)) {
		strcpy(st->title, GET_TITLE(ch));
	}
	else {
		*st->title = '\0';
	}

	/*GGPATCH*/

	if(GET_AUTHBY(ch) && GET_AUTHCODE(ch)) {
		strncpy(st->authcode, GET_AUTHCODE(ch), Registered::REG_CODELEN);
	}
	else {
		*st->authcode = '\0';
	}

	if(ch->player.description) {
		strcpy(st->description, ch->player.description);
	}
	else {
		*st->description = '\0';
	}

	for(i = 0; i <= MAX_TOUNGE - 1; i++) {
		st->talks[i] = ch->player.talks[i];
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		st->skills[i] = ch->skills[i];
		st->skills[i].flags = ch->skills[i].flags;
		st->skills[i].special = ch->skills[i].special;
		st->skills[i].nummem = ch->skills[i].nummem;
	}

	strcpy(st->name, GET_NAME(ch));

	for(i = 0; i <= 4; i++) {
		st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];
	}

	for(i = 0; i <= 2; i++) {
		st->conditions[i] = GET_COND(ch, i);
	}

	for(af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
		if(af) {
			/* Add effect of the spell or it will be lost */
			/* When saving without quitting               */
			affect_modify(ch, st->affected[i].location,
						  st->affected[i].modifier, st->affected[i].bitvector, TRUE);
			af = af->next;
		}
	}

	for(i = 0; i < MAX_WEAR; i++) {
		if(char_eq[i]) {
			equip_char(ch, char_eq[i], i);
		}
	}

	affect_total(ch);

	//FLYP 2003 Perdono
	/*if (strcmp(ch->has_killed,"perdonato")==0)
	 {
	 for(af = st->affected, i = 0; i<MAX_AFFECT; i++)
	 {
	 if (af && st->affected[i].bitvector==AFF2_PKILLER)
	 {
	 affect_remove (ch, af);
	 af = af->next;
	 }
	 }
	 }*/

	GET_HIT(ch) = hit;
	GET_MANA(ch) = mana;
	GET_MOVE(ch) = move;
} /* Char to store */

/* write the vital data of a player to the player file */

void save_char(struct char_data* ch, sh_int load_room, int bonus) {
	struct char_file_u st;
	FILE* fl;
	char szFileName[200];
	struct char_data* tmp = NULL;

	if(!IS_PC(ch)) {
		return;
	}

	if(IS_POLY(ch)) {
		if(!ch->desc) {
			return;
		}
		tmp = ch->desc->original;
		if(!tmp) {
			return;
		}
	}
	else {
		if(!ch->desc) {
			return;
		}
	}
	if(!tmp) {
		tmp = ch;
	}

	char_to_store(tmp, &st);

	st.load_room = load_room;
	st.last_logon += bonus * 60 * 60 * 24;
	strcpy(st.pwd, ch->desc->pwd);

	sprintf(szFileName, "%s/%s.dat", PLAYERS_DIR, lower(tmp->player.name));
	if((fl = fopen(szFileName, "r+b")) == NULL) {
		if((fl = fopen(szFileName, "wb")) == NULL) {
			mudlog(LOG_ERROR, "Cannot create file %s for saving player.",
				   szFileName);
			return;
		}
	}

	rewind(fl);
	fwrite(&st, sizeof(struct char_file_u), 1, fl);
	fclose(fl);

}
/* void save_char(struct char_data *ch, sh_int load_room)
 {
 save_char(ch,load_room,0);
 } */

/* for possible later use with qsort */
int compare(struct player_index_element* arg1,
			struct player_index_element* arg2) {
	return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
 *  procs of a (more or less) general utility nature
 *
 ********************************************************************** */

int fwrite_string(FILE* fl, char* buf) {
	if(buf) {
		return (fprintf(fl, "%s~\n", buf));
	}
	else {
		return (fprintf(fl, "~\n"));
	}
}

char* fread_string(FILE* f1) {
	char buf[ MAX_STRING_LENGTH];
	int i = 0, tmp;
	char* pReturnString = NULL;

	buf[0] = '\0';

	while(i < MAX_STRING_LENGTH - 3) {
		if((tmp = fgetc(f1)) == EOF) {
			mudlog(LOG_ERROR, "Error '%s' reading file in fread_string",
				   strerror(errno));
			break;
		}

		if(tmp == '~') {
			break;
		}

		buf[i++] = (char) tmp;
		if(buf[i - 1] == '\n') {
			buf[i++] = '\r';
		}
	}

	if(i >= MAX_STRING_LENGTH - 3) {
		/* We filled the buffer */
		mudlog(LOG_ERROR, "Line too long (fread_string). Flushing");
		while((tmp = fgetc(f1)) != EOF)
			if(tmp == '~') {
				break;
			}
	}

	buf[i] = '\0';

	fgetc(f1);

	if(strlen(buf)) {

		/*     if (!malloc(strlen(buf)+1))
		 mudlog(LOG_ERROR,"Malloc ha ritornato un puntatore nullo"); */

		PushStatus("fread_string malloc");
		pReturnString = (char*) malloc(strlen(buf) + 1);
		if(pReturnString) {
			PushStatus("fread_string strcpy");
			strcpy(pReturnString, buf);
			PopStatus();
		}
		PopStatus();

		if(pReturnString == NULL) {
			mudlog(LOG_ERROR, "Fread_string:Errore nel ritornare la stringa %s",
				   buf);
		}
		fflush(NULL);

	}

	return pReturnString;
}

/****************************************************************************
 * Legge un numero dal file puntato da pFIle. Se il numero contiene il
 * carattere | le due porzioni di numero vengono addizionate. Ad esempio
 * 4|128 diventa 132. Molto utile per i flags.
 ****************************************************************************/
long fread_number_int(FILE* pFile, const char* cmdfile, int cmdline,
					  const char* infofile) {
	long number;
	bool sign;
	char c;
	char memo[1024];
	long l;
	l = 0;
	SetStatus(infofile);
	SetLine(cmdfile, cmdline);
	do {
		c = getc(pFile);
		if(l < 1023) {
			memo[l++] = c;
		}
	}
	while(isspace(c));

	number = 0;

	sign = FALSE;
	if(c == '+') {
		c = getc(pFile);
	}
	else if(c == '-') {
		sign = TRUE;
		c = getc(pFile);
		if(l < 1023) {
			memo[l++] = c;
		}

	}

	if(!isdigit(c)) {
		memo[l] = 0;
		mudlog(LOG_ERROR, "Fread_number: bad char %c line %s Info: ", c, memo,
			   infofile);
		PrintStatus(1);
		ungetc(c, pFile);
		return 0;
	}

	while(isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(pFile);
	}

	if(sign) {
		number = 0 - number;
	}

	if(c == '|') {
		number += fread_number(pFile);
	}
	else if(c != ' ') {
		ungetc(c, pFile);
	}

	return number;
}

long fread_if_number(FILE* pFile) {
	long number;
	bool sign;
	char c;

	do {
		c = getc(pFile);
	}
	while(isspace(c));

	number = 0;

	sign = FALSE;
	if(c == '+') {
		c = getc(pFile);
	}
	else if(c == '-') {
		sign = TRUE;
		c = getc(pFile);
	}

	if(!isdigit(c)) {
		ungetc(c, pFile);
		return 0;
	}

	while(isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(pFile);
	}

	if(sign) {
		number = 0 - number;
	}

	if(c == '|') {
		number += fread_number(pFile);
	}
	else if(c != ' ') {
		ungetc(c, pFile);
	}

	return number;
}

void fwrite_flag(FILE* pFile, unsigned long ulFlags) {
	unsigned long ulBit = 1;
	short bPrimaVolta = TRUE;

	while(ulFlags) {
		if(ulFlags & 1) {
			if(!bPrimaVolta) {
				fprintf(pFile, "|");
			}
			else {
				bPrimaVolta = FALSE;
			}

			fprintf(pFile, "%lu", ulBit);
		}
		ulBit *= 2;
		ulFlags >>= 1;
	}
}

/* release memory allocated for a char struct */
void free_char(struct char_data* ch) {
	struct affected_type* af, *pNext = NULL;
	int i;

	if(auction->item) {  // SALVO pulisco in auction
		if(auction->seller == ch) {
			auction->seller = NULL;
		}
		if(auction->buyer == ch) {
			auction->buyer = NULL;
		}
	}
#ifndef NOEVENTS
	/* cancel point updates */
	for(i = 0; i < 3; i++)
		if(GET_POINTS_EVENT(ch, i)) {
			GET_POINTS_EVENT(ch, i) = NULL;
		}
#endif

	if(ch->nMagicNumber != CHAR_VALID_MAGIC) {
		mudlog(LOG_SYSERR,
			   "Characters char %s with uncorrect magic number in free_char!",
			   GET_NAME_DESC(ch));
		return;
	}

	mudlog(LOG_CHECK, "Freeing char %s (ADDR: %p, magic %d)", GET_NAME_DESC(ch),
		   ch, ch->nMagicNumber);

	if(GET_NAME(ch)) {
		free(GET_NAME(ch));
		GET_NAME(ch) = NULL;
	}
	if(GET_PRINCE(ch)) {
		free(GET_PRINCE(ch));
		GET_PRINCE(ch) = NULL;
	}

	if(ch->specials.poofin) {
		free(ch->specials.poofin);
		ch->specials.poofin = NULL;
	}
	if(ch->specials.poofout) {
		free(ch->specials.poofout);
		ch->specials.poofout = NULL;
	}
	if(ch->specials.prompt) {
		free(ch->specials.prompt);
		ch->specials.prompt = NULL;
	}
	if(ch->specials.lastversion) {
		free(ch->specials.lastversion);
		ch->specials.lastversion = NULL;
	}
	if(ch->specials.email) {
		free(ch->specials.email);
		ch->specials.email = NULL;
	}
	if(ch->specials.realname) {
		free(ch->specials.realname);
		ch->specials.realname = NULL;
	}
	if(ch->specials.authcode) {
		free(ch->specials.authcode);
		ch->specials.authcode = NULL;
	}
	if(ch->specials.AuthorizedBy) {
		free(ch->specials.AuthorizedBy);
		ch->specials.AuthorizedBy = NULL;
	}
	if(ch->specials.supporting) {
		free(ch->specials.supporting);
		ch->specials.supporting = NULL;
	}
	if(ch->specials.bodyguarding) {
		free(ch->specials.bodyguarding);
		ch->specials.bodyguarding = NULL;
	}
	if(ch->specials.bodyguard) {
		free(ch->specials.bodyguard);
		ch->specials.bodyguard = NULL;
	}

	if(ch->player.title) {
		free(ch->player.title);
		ch->player.title = NULL;
	}
	if(ch->player.short_descr) {
		free(ch->player.short_descr);
		ch->player.short_descr = NULL;
	}
	if(ch->player.long_descr) {
		free(ch->player.long_descr);
		ch->player.long_descr = NULL;
	}
	if(ch->player.description) {
		free(ch->player.description);
		ch->player.description = NULL;
	}
	if(ch->player.sounds) {
		free(ch->player.sounds);
		ch->player.sounds = NULL;
	}
	if(ch->player.distant_snds) {
		free(ch->player.distant_snds);
		ch->player.distant_snds = NULL;
	}
	if(ch->specials.A_list) {
		for(i = 0; i < MAX_ALIAS; i++) {
			if(GET_ALIAS(ch, i)) {
				free(GET_ALIAS(ch, i));
				GET_ALIAS(ch, i) = NULL;
			}
		}
		free(ch->specials.A_list);
		ch->specials.A_list = NULL;
	}

	for(af = ch->affected; af; af = pNext) {
		pNext = af->next;
		affect_remove(ch, af);
	}

	if(ch->skills) {
		free(ch->skills);
		ch->skills = NULL;
	}
	if(ch->nMagicNumber != CHAR_FREEDED_MAGIC) {
		ch->nMagicNumber = CHAR_FREEDED_MAGIC;
		free(ch);
	}

}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data* obj) {
	struct extra_descr_data* pExDescr, *next_one;

	if(!obj) {
		/* bug fix, msw */
		mudlog(LOG_SYSERR, "!obj in free_obj, db.c");
		return;
	}
	free(obj->name);
	obj->name = NULL;
	free(obj->description);
	obj->description = NULL;
	free(obj->short_description);
	obj->short_description = NULL;
	free(obj->action_description);
	obj->action_description = NULL;

	for(pExDescr = obj->ex_description; pExDescr; pExDescr = next_one) {
		if(pExDescr->nMagicNumber == EXDESC_VALID_MAGIC) {
			next_one = pExDescr->next;
			pExDescr->nMagicNumber = EXDESC_FREED_MAGIC;
			free(pExDescr->keyword);
			pExDescr->keyword = NULL;
			free(pExDescr->description);
			pExDescr->description = NULL;
			free(pExDescr);
		}
		else {
			next_one = NULL;
			mudlog(LOG_SYSERR,
				   "Invalid extra description freeing object in free_obj (db.c)");
		}
	}
	obj->ex_description = NULL;

	free(obj->szForbiddenWearToChar);
	obj->szForbiddenWearToChar = NULL;
	free(obj->szForbiddenWearToRoom);
	obj->szForbiddenWearToRoom = NULL;

	free(obj);
}

/* read contents of a text file, and place in buf */
int file_to_string(const char* name, char* buf) {
	FILE* fl;
	char tmp[100];
	struct stat fileinfo;
	*buf = '\0';
	stat(name, &fileinfo);
	mudlog(LOG_CHECK, "File %s lunghezza %d", name, fileinfo.st_size);
	if(!(fl = fopen(name, "r"))) {
		mudlog(LOG_ERROR, "Unable to open %s, continuing", name);
		*buf = '\0';
		return (-1);
	}

	do {
		fgets(tmp, 99, fl);

		if(!feof(fl)) {
			if(strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
				mudlog(LOG_ERROR, "fl->strng: string too big");
				*buf = '\0';
				fclose(fl);
				return (-1);
			}

			strcat(buf, tmp);
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while(!feof(fl));

	fclose(fl);

	return (0);
}

void ClearDeadBit(struct char_data* ch) {
}

/* clear some of the the working variables of a char */
void reset_char(struct char_data* ch) {
	double ratio = 0.0;
	int i;
	double absmaxhp;
	mudlog(LOG_SAVE, "Resetting char %s", GET_NAME(ch));
	for(i = 0; i < MAX_WEAR; i++) {  /* Initializing */
		ch->equipment[i] = 0;
	}

	ch->followers = 0;
	ch->master = 0;
	ch->carrying = 0;
	ch->next = 0;

	ch->immune = 0;
	ch->M_immune = 0;
	ch->susc = 0;
	ch->mult_att = 1.0;

	if(!GET_RACE(ch)) {
		GET_RACE(ch) = RACE_HUMAN;
	}

	for(i = 0; i < MAX_CLASS; i++) {
		if(GET_LEVEL(ch, i) > IMMENSO) {
			GET_LEVEL(ch, i) = 51;
		}
	}

	SET_BIT(ch->specials.act, PLR_ECHO);

	ch->hunt_dist = 0;
	ch->hatefield = 0;
	ch->fearfield = 0;
	ch->hates.clist = 0;
	ch->fears.clist = 0;

	/* AC adjustment */
	GET_AC(ch) = 100;

	GET_HITROLL(ch) = 0;
	GET_DAMROLL(ch) = 0;

	ch->next_fighting = 0;
	ch->next_in_room = 0;
	ch->specials.fighting = 0;
	ch->specials.PosPrev = POSITION_STANDING;
	for(i = 0; i < MAX_POSITION; i++) {
		ch->specials.TempoPassatoIn[i] = 0;
	}
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	ch->specials.carry_weight = 0;
	ch->specials.carry_items = 0;
	ch->specials.spellfail = 101;

	if(GET_HIT(ch) <= 0) {
		GET_HIT(ch) = 1;
	} /*** SALVO il regen parte prima di uscire ***/
	if(GET_MOVE(ch) <= 0) {
		GET_MOVE(ch) = 1;
	}
	if(GET_MANA(ch) <= 0) {
		GET_MANA(ch) = 1;
	}
	ch->points.max_mana = 0;
	ch->points.max_move = GET_CON(ch) + number(0, 6) - 3;
	ch->points.mana_gain = 0;
	ch->points.move_gain = 0;
	ch->points.hit_gain = 0;

	if(IS_IMMORTAL(ch)) {
		GET_BANK(ch) = 0;
		GET_GOLD(ch) = 1000000;
	}

	if(GET_BANK(ch) > GetMaxLevel(ch) * 10000) {
		mudlog(LOG_PLAYERS, "%s has %d coins in bank.", GET_NAME(ch),
			   GET_BANK(ch));
	}
	if(GET_GOLD(ch) > GetMaxLevel(ch) * 10000) {
		mudlog(LOG_PLAYERS, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
	}

	/* rimettiamo a posto le condizioni di affamato od assetato in modo che
	 * qualche bug non tolga la necessita di bere o di mangiare al PC >:) */
	if(!IS_IMMORTALE(ch)) {
		if(GET_COND(ch, DRUNK) < 0) {
			GET_COND(ch, DRUNK) = 0;
		}
		if(GET_COND(ch, FULL) < 0) {
			GET_COND(ch, FULL) = 0;
		}
		if(GET_COND(ch, THIRST) < 0) {
			GET_COND(ch, THIRST) = 0;
		}
	}

	/*
	 * Class specific Stuff
	 */

	ClassSpecificStuff(ch);

	if(HasClass(ch, CLASS_MONK)) {
		GET_AC(ch) -= MIN(150, (GET_LEVEL(ch, MONK_LEVEL_IND) * 5));
		ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
	}

	/*
	 * racial stuff
	 */
	SetRacialStuff(ch);

	/*
	 * update the affects on the character.
	 */

	ch->specials.sev = LOG_SYSERR | LOG_ERROR | LOG_CONNECT;
#if 0
	for(af = ch->affected; af; af = af->next) {
		affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
	}
#endif
	if(!HasClass(ch, CLASS_MONK)) {
		GET_AC(ch) += dex_app[(int) GET_DEX(ch)].defensive;
	}

	/* could add barbarian double dex bonus here.... ... Nah! */

	if(GET_AC(ch) > 100) {
		GET_AC(ch) = 100;
	}

	/*
	 * clear out the 'dead' bit on characters
	 */
	if(ch->desc) {
		ClearDeadBit(ch);
	}
	/*
	 * Clear out berserk flags case there was a crash in a fight
	 */
	if(IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_BERSERK);
	}
	/*
	 * Clear out Parry flags case there was a crash in a fight
	 */
	if(IS_SET(ch->specials.affected_by2, AFF2_PARRY)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
	}
	/*
	 * Clear out MAILING flags case there was a crash
	 */
	if(IS_SET(ch->specials.act, PLR_MAILING)) {
		REMOVE_BIT(ch->specials.act, PLR_MAILING);
	}

	/*
	 * Clear out objedit flags
	 */
	if(IS_SET(ch->player.user_flags, CAN_OBJ_EDIT)) {
		REMOVE_BIT(ch->player.user_flags, CAN_OBJ_EDIT);
	}
	/*
	 * Clear out group/order/AFK flags
	 */

	REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
	if(IS_SET(ch->specials.affected_by2, AFF2_CON_ORDER)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_CON_ORDER);
	}
	if(IS_AFFECTED2(ch, AFF2_AFK)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
	}

	/*
	 * Remove bogus flags on mortals
	 */

	if(IS_SET(ch->specials.act, PLR_NOHASSLE) && GetMaxLevel(ch) < IMMORTALE) {
		REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
	}

	/* check spells and if lower than 95 remove special flag */
	if(!IS_IMMORTALE(ch)) {
		for(i = 0; i < MAX_SKILLS - 1; i++) {
			if(ch->skills[i].learned < 95||
					!IS_SET(ch->skills[i].flags, SKILL_KNOWN)) {
				ch->skills[i].special = 0;
			}
		}
	}

	SetDefaultLang(ch);

	/* Imposta i livelli di default obbligatori per gli dei */
	//if( !strcmp(GET_NAME(ch),"Isildur"))
	//{
	//GET_LEVEL(ch,0) = 60;
	//}
	//if( !strcmp(GET_NAME(ch),"Flyp"))
	//{
	//GET_LEVEL(ch,0) = 60;
	//}
	if(!strcmp(GET_NAME(ch), "Alar")) {  //Giovanni
		GET_LEVEL(ch, 0) = 60;
	}
	if(!strcmp(GET_NAME(ch), "Isildur")) {  //Nicola
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Requiem")) {  //Francesco
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Flyp")) {  //Enrico
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Nihil")) {  //Marco
		GET_LEVEL(ch, 0) = 58;
	}
	if(!strcmp(GET_NAME(ch), "Ladyofpain")) {  //Giuseppe
		GET_LEVEL(ch, 0) = 58;
	}

	/* this is to clear up bogus levels on people that where here before */
	/* these classes where made... */

	if(!HasClass(ch, CLASS_MAGIC_USER)) {
		ch->player.level[0] = 0;
	}
	if(!HasClass(ch, CLASS_CLERIC)) {
		ch->player.level[1] = 0;
	}
	if(!HasClass(ch, CLASS_WARRIOR)) {
		ch->player.level[2] = 0;
	}
	if(!HasClass(ch, CLASS_THIEF)) {
		ch->player.level[3] = 0;
	}
	if(!HasClass(ch, CLASS_WARRIOR)) {
		ch->player.level[2] = 0;
	}
	if(!HasClass(ch, CLASS_DRUID)) {
		ch->player.level[4] = 0;
	}
	if(!HasClass(ch, CLASS_MONK)) {
		ch->player.level[5] = 0;
	}
	if(!HasClass(ch, CLASS_BARBARIAN)) {
		ch->player.level[6] = 0;
	}
	if(!HasClass(ch, CLASS_SORCERER)) {
		ch->player.level[7] = 0;
	}
	if(!HasClass(ch, CLASS_PALADIN)) {
		ch->player.level[8] = 0;
	}
	if(!HasClass(ch, CLASS_RANGER)) {
		ch->player.level[9] = 0;
	}
	if(!HasClass(ch, CLASS_PSI)) {
		ch->player.level[10] = 0;
	}
	for(i = MAX_CLASS; i < ABS_MAX_CLASS; i++) {
		ch->player.level[i] = 0;
	}
	/* Qui viene impostato il flag PKILL
	 * per i personaggi dal 31esimo livello in su e rimosso per gli immortali
	 * */
	if(GetMaxLevel(ch) >= INIZIATO) {
		if(IS_PC(
					ch) && !IS_IMMORTALE(ch) && !IS_SET(ch->player.user_flags, RACE_WAR)) {
			send_to_char(
				"$c0115      ATTENTO! ADESSO SEI PKILL                $c0007\n\r",
				ch);
			SET_BIT(ch->player.user_flags, RACE_WAR);
		}
		if(IS_PC(ch) && IS_IMMORTAL(ch)) {
			REMOVE_BIT(ch->player.user_flags, RACE_WAR);
		}

	}
	/* Controlli per giocatori scorretti */
	if(GetMaxLevel(ch) >= DIO_MINORE) {
		ratio = 0.0;
	}
	else if(GetMaxLevel(ch) >= BARONE) {
		ratio = 0.8;
	}
	else if(GetMaxLevel(ch) >= MAESTRO) {
		ratio = 0.7;
	}
	else if(GetMaxLevel(ch) >= ESPERTO) {
		ratio = 0.6;
	}
	else if(GetMaxLevel(ch) >= INIZIATO) {
		ratio = 0.4;
	}
	else if(GetMaxLevel(ch) >= ALLIEVO) {
		ratio = 0.2;
	}
	else if(GetMaxLevel(ch) >= NOVIZIO) {
		ratio = 0.1;
	}

	{
		if((ch->player.time.played / SECS_PER_REAL_HOUR)
				< (ratio * GetTotLevel(ch))) {
			int minplayed = (ch->player.time.played % SECS_PER_REAL_HOUR) / 60;
			buglog(LOG_PLAYERS,
				   "%s ha fatto %d livelli in %5d ore e %2d minuti",
				   GET_NAME(ch), GetTotLevel(ch),
				   (ch->player.time.played / SECS_PER_REAL_HOUR), minplayed);
		}
	}

	/* Controllo sugli hp, per bug o trucchi*/
	absmaxhp = (float) GetExtimatedHp(ch);
	{
		// SALVO controllo se ha spell_mana castato
		struct affected_type* hjp;

		for(hjp = ch->affected; hjp; hjp = hjp->next) {
			if(hjp->type == SPELL_MANA) {
				GET_MANA(ch) = GET_MHIT(ch) - GET_HIT(ch);
				if(GET_MANA(ch) < 0) {
					GET_MANA(ch) = 0;
				}
				absmaxhp = absmaxhp + (float) hjp->modifier;
			}
		}
	}
	ratio = (float) GET_MHIT(ch) / absmaxhp;
	if(ratio > 1.0) {
		buglog(LOG_SYSERR, "%s ha %d hp (stimati: %f) ratio %f", GET_NAME(ch),
			   hit_limit(ch), absmaxhp, ratio);
	}

	/* start regening new points */
	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);

	if(ratio > 1.19) {
		if(GetMaxLevel(ch) > CHUMP) {
			FrozeHim(ch, GET_MHIT(ch));
		}
	}
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data* ch) {
	memset(ch, '\0', sizeof(struct char_data));

	ch->in_room = NOWHERE;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	GET_AC(ch) = 100; /* Basic Armor */
	ch->size = 25;
	ch->nMagicNumber = CHAR_VALID_MAGIC;
}

void clear_object(struct obj_data* obj) {
	memset(obj, '\0', sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room = NOWHERE;
	obj->eq_pos = -1;
}

/* initialize a new character only if class is set */
void init_char(struct char_data* ch) {
	int i;

	/* *** if this is our first player --- he be God *** */
	mudlog(LOG_CHECK, "Creazione di %s in corso", GET_NAME(ch));

	if((top_of_p_table < 0) || !strcmp(GET_NAME(ch), "Alar")) {

		mudlog(LOG_CHECK, "Building FIRST CHAR, setting up IMPLEMENTOR STATUS!");

		GET_EXP(ch) = 500000000;
		GET_LEVEL(ch, 0) = IMMENSO;
		ch->points.max_hit = 1000;

		/* set all levels */

		for(i = 0; i < MAX_CLASS; i++) {
			if(GET_LEVEL(ch, i) < GetMaxLevel(ch)) {
				GET_LEVEL(ch, i) = GetMaxLevel(ch);
			}
		}/* for */

		/* set all classes */
		for(i = 1; i <= CLASS_PSI; i *= 2) {
			if(!HasClass(ch, i)) {
				ch->player.iClass += i;
			}
		} /* for */

	} /* end implmentor setup */

	set_title(ch);

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.hometown = number(1, 4);

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	SET_BIT(ch->player.user_flags, USE_PAGING);

	for(i = 0; i < MAX_TOUNGE; i++) {
		ch->player.talks[i] = 0;
	}

	GET_STR(ch) = 9;
	GET_INT(ch) = 9;
	GET_WIS(ch) = 9;
	GET_DEX(ch) = 9;
	GET_CON(ch) = 9;
	GET_CHR(ch) = 9;

	/* make favors for sex */
	switch(GET_RACE(ch)) {
	case RACE_HUMAN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_DWARF:
	case RACE_GNOME:
	case RACE_DARK_DWARF:
	case RACE_DEEP_GNOME:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(100, 150);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(100, 150);
		}
		break;

	case RACE_HALFLING:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(70, 120);
			ch->player.height = number(80, 120);
		}
		else {
			ch->player.weight = number(60, 110);
			ch->player.height = number(70, 115);
		}
		break;

	case RACE_ELVEN:
	case RACE_DARK_ELF:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(100, 150);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(80, 230);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_HALF_ELVEN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(110, 160);
			ch->player.height = number(140, 180);
		}
		else {
			ch->player.weight = number(90, 150);
			ch->player.height = number(140, 170);
		}
		break;

	case RACE_HALF_OGRE:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(200, 400);
			ch->player.height = number(200, 230);
		}
		else {
			ch->player.weight = number(180, 350);
			ch->player.height = number(190, 220);
		}
		break;

	case RACE_HALF_ORC:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_HALF_GIANT:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(300, 900);
			ch->player.height = number(300, 400);
		}
		else {
			ch->player.weight = number(250, 800);
			ch->player.height = number(290, 350);
		}
		break;

	case RACE_ORC:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(140, 200);
			ch->player.height = number(150, 190);
		}
		else {
			ch->player.weight = number(120, 180);
			ch->player.height = number(140, 170);
		}
		break;

	case RACE_GOBLIN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(60, 90);
			ch->player.height = number(130, 160);
		}
		else {
			ch->player.weight = number(60, 90);
			ch->player.height = number(120, 150);
		}
		break;

	case RACE_DEMON:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 150);
			ch->player.height = number(150, 190);
		}
		{
			ch->player.weight = number(135, 170);
			ch->player.height = number(160, 200);
		}
		break;

	case RACE_TROLL:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(220, 430);
			ch->player.height = number(210, 250);
		}
		else {
			ch->player.weight = number(200, 400);
			ch->player.height = number(200, 240);
		}
		break;

	default:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
	}

	ch->points.mana = GET_MAX_MANA(ch);
	ch->points.hit = GET_MAX_HIT(ch);
	ch->points.move = GET_MAX_MOVE(ch);

	ch->points.armor = 100;

	if(!ch->skills) {
		SpaceForSkills(ch);
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		if(GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
			ch->skills[i].learned = 0;
			ch->skills[i].flags = 0;
			ch->skills[i].special = 0;
			ch->skills[i].nummem = 0;
		}
		else {
			ch->skills[i].learned = 100;
			ch->skills[i].flags = 0;
			ch->skills[i].special = 1;
			ch->skills[i].nummem = 0;
		}
	}

	ch->specials.affected_by = 0;
	ch->specials.spells_to_learn = 0;

	for(i = 0; i < 5; i++) {
		ch->specials.apply_saving_throw[i] = 0;
	}

	for(i = 0; i < 3; i++) {
		GET_COND(ch, i) = (GetMaxLevel(ch) > CREATORE ? -1 : 24);
	}
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int iVNum) {
	int bot, top, mid;

	bot = 0;
	top = top_of_sort_mobt - 1;

	/* perform binary search on mob-table */
	for(;;) {
		mid = (bot + top) / 2;

		if((mob_index + mid)->iVNum == iVNum) {
			return (mid);
		}
		if(bot >= top) {
			/* start unsorted search now */
			for(mid = top_of_sort_mobt; mid < top_of_mobt; mid++)
				if((mob_index + mid)->iVNum == iVNum) {
					return (mid);
				}
			return (-1);
		}
		if((mob_index + mid)->iVNum > iVNum) {
			top = mid - 1;
		}
		else {
			bot = mid + 1;
		}
	}
	return -1;
}

/* returns the real number of the object with given virtual number */
int real_object(int nVNum) {
	long bot, top, mid;

	bot = 0;
	top = top_of_sort_objt - 1;

	/* perform binary search on obj-table */
	for(;;) {
		mid = (bot + top) / 2;

		if((obj_index + mid)->iVNum == nVNum) {
			return (mid);
		}
		if(bot >= top) {
			/* start unsorted search now */
			for(mid = top_of_sort_objt; mid < top_of_objt; mid++)
				if((obj_index + mid)->iVNum == nVNum) {
					return (mid);
				}
			return (-1);
		}
		if((obj_index + mid)->iVNum > nVNum) {
			top = mid - 1;
		}
		else {
			bot = mid + 1;
		}
	}
	return -1;
}

int ObjRoomCount(int nr, struct room_data* rp) {
	struct obj_data* o;
	int count = 0;

	for(o = rp->contents; o; o = o->next_content) {
		if(o->item_number == nr) {
			count++;
		}
	}
	return (count);
}

int MobRoomCount(int nr, struct room_data* rp) {
	struct char_data* o;
	int count = 0;

	for(o = rp->people; o; o = o->next_in_room) {
		if(o->nr == nr) {
			count++;
		}
	}
	return (count);
}

int str_len(char* buf) {
	int i = 0;
	while(buf[i] != '\0') {
		i++;
	}
	return (i);
}
void reload_files_and_scripts() {

	mudlog(LOG_CHECK, "Rebooting Essential Text Files.");

	file_to_string(NEWS_FILE, news);
	file_to_string(WIZNEWS_FILE, wiznews);
	file_to_string(CREDITS_FILE, credits);
	file_to_string(MOTD_FILE, motd);
	file_to_string(WIZ_MOTD_FILE, wmotd);
	file_to_string(HELP_PAGE_FILE, help);
	mudlog(LOG_CHECK, "Initializing Scripts.");
	InitScripts();

	/* jdb -- you don't appear to re-install the scripts after you
	 * reset the script db
	 */

	for(struct char_data* p = character_list; p; p = p->next) {
		for(int i = 0; i < top_of_scripts; i++) {
			if(gpScript_data[i].iVNum == mob_index[p->nr].iVNum) {
				SET_BIT(p->specials.act, ACT_SCRIPT);
				mudlog(LOG_CHECK, "Setting SCRIPT bit for mobile %s, file %s.",
					   GET_NAME(p), gpScript_data[i].filename);
				p->script = i;
				break;
			}
		}
	}
	return;

}
ACTION_FUNC(reboot_text) {
	reload_files_and_scripts();
}
void InitScripts() {
	char buf[255], buf2[255];
	FILE* f1, *f2;
	int count;
	struct char_data* mob;

	if(!gpScript_data) {
		top_of_scripts = 0;
	}

	/* what is ths for?  turn off all the scripts ??? */
	/* -yes, just in case the script file was removed, saves pointer probs */

	for(mob = character_list; mob; mob = mob->next) {
		if(IS_MOB(mob) && IS_SET(mob->specials.act, ACT_SCRIPT)) {
			mob->commandp = 0;
			REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
		}
	}

	if(!(f1 = fopen("scripts.dat", "r"))) {
		mudlog(LOG_ERROR, "Unable to open file \"scripts.dat\".");
		return;
	}

	if(gpScript_data) {
		for(int i = 0; i < top_of_scripts; i++) {
			free(gpScript_data[i].script);
			free(gpScript_data[i].filename);
		}
		free(gpScript_data);
		top_of_scripts = 0;
	}

	gpScript_data = NULL;
	gpScript_data = (struct scripts*) malloc(sizeof(struct scripts));

	while(1) {
		int i;
		if(fgets(buf, 254, f1) == NULL) {
			break;
		}

		if(buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = '\0';
		}

		sscanf(buf, "%s %d", buf2, &i);

		sprintf(buf, "scripts/%s", buf2);
		if(!(f2 = fopen(buf, "r"))) {
			mudlog(LOG_ERROR, "Unable to open script \"%s\" for reading.", buf2);
		}
		else {

			gpScript_data = (struct scripts*) realloc(gpScript_data,
							(top_of_scripts + 1) * sizeof(struct scripts));

			count = 0;
			while(!feof(f2)) {
				fgets(buf, 254, f2);
				if(buf[strlen(buf) - 1] == '\n') {
					buf[strlen(buf) - 1] = '\0';
				}
				/* you really don't want to do a lot of reallocs all at once */
				if(count == 0) {
					gpScript_data[top_of_scripts].script =
						(struct foo_data*) malloc(sizeof(struct foo_data));
				}
				else {
					gpScript_data[top_of_scripts].script =
						(struct foo_data*) realloc(
							gpScript_data[top_of_scripts].script,
							sizeof(struct foo_data) * (count + 1));
				}
				gpScript_data[top_of_scripts].script[count].line =
					(char*) malloc(sizeof(char) * (strlen(buf) + 1));

				strcpy(gpScript_data[top_of_scripts].script[count].line, buf);

				count++;
			}

			gpScript_data[top_of_scripts].iVNum = i;
			gpScript_data[top_of_scripts].filename = (char*) malloc(
						(strlen(buf2) + 1) * sizeof(char));
			strcpy(gpScript_data[top_of_scripts].filename, buf2);
			mudlog(LOG_CHECK, "Script %s assigned to mobile %d.", buf2, i);
			top_of_scripts++;
			fclose(f2);
		}
	}

	if(top_of_scripts) {
		mudlog(LOG_CHECK, "%d scripts assigned.", top_of_scripts);
	}
	else {
		mudlog(LOG_CHECK, "No scripts found to assign.");
	}
	fclose(f1);
}

int CheckKillFile(int iVNum) {
	FILE* f1;
	char buf[255];
	int i;

	if(!(f1 = fopen(killfile, "r"))) {
		mudlog(LOG_ERROR, "Unable to find killfile.");
		exit(0);
	}

	while(fgets(buf, 254, f1) != NULL) {
		sscanf(buf, "%d", &i);
		if(i == iVNum) {
			fclose(f1);
			return (1);
		}
	}

	fclose(f1);
	return (0);
}

void ReloadRooms() {
	int i;

	for(i = 0; i < number_of_saved_rooms; i++) {
		load_room_objs(saved_rooms[i]);
	}
}

void SaveTheWorld() {
#if SAVEWORLD

	static int ctl = 0;
	char cmd, buf[80];
	int i, j, arg1, arg2, arg3;
	struct char_data* p;
	struct obj_data* o;
	struct room_data* room;
	FILE* fp;

	if(ctl == WORLD_SIZE) {
		ctl = 0;
	}

	sprintf(buf, "world/mobs.%d", ctl);
	fp = (FILE*) fopen(buf, "w"); /* append */

	if(!fp) {
		mudlog(LOG_ERROR, "Unable to open zone writing file.");
		return;
	}

	i = ctl;
	ctl += 1000;

	for(; i < ctl; i++) {
		room = real_roomp(i);
		if(room && !IS_SET(room->room_flags, DEATH)) {
			/*
			 *  first write out monsters
			 */
			for(p = room->people; p; p = p->next_in_room) {
				if(!IS_PC(p)) {
					cmd = 'M';
					arg1 = MobVnum(p);
					arg2 = mob_index[p->nr].number;
					arg3 = i;
					Zwrite(fp, cmd, 0, arg1, arg2, arg3, 0, p->player.short_descr);
					fprintf(fp, "Z 1 %d 1\n", p->specials.zone);

					/* save hatreds && fears */
					if(IS_SET(p->hatefield, HATE_SEX)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->hates.sex);
					}
					if(IS_SET(p->hatefield, HATE_RACE)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->hates.race);
					}
					if(IS_SET(p->hatefield, HATE_GOOD)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->hates.good);
					}
					if(IS_SET(p->hatefield, HATE_EVIL)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->hates.evil);
					}
					if(IS_SET(p->hatefield, HATE_CLASS)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->hates.iClass);
					}
					if(IS_SET(p->hatefield, HATE_VNUM)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->hates.vnum);
					}

					if(IS_SET(p->fearfield, FEAR_SEX)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->fears.sex);
					}
					if(IS_SET(p->fearfield, FEAR_RACE)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->fears.race);
					}
					if(IS_SET(p->fearfield, FEAR_GOOD)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->fears.good);
					}
					if(IS_SET(p->fearfield, FEAR_EVIL)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->fears.evil);
					}
					if(IS_SET(p->fearfield, FEAR_CLASS)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->fears.iClass);
					}
					if(IS_SET(p->fearfield, FEAR_VNUM)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->fears.vnum);
					}

					for(j = 0; j < MAX_WEAR; j++) {
						if(p->equipment[j]) {
							if(p->equipment[j]->item_number >= 0) {
								cmd = 'E';
								arg1 = ObjVnum(p->equipment[j]);
								arg2 = obj_index[p->equipment[j]->item_number].number;
								arg3 = j;
								strcpy(buf, p->equipment[j]->short_description);
								Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
								RecZwriteObj(fp, p->equipment[j]);
							}
						}
					}
					for(o = p->carrying; o; o = o->next_content) {
						if(o->item_number >= 0) {
							cmd = 'G';
							arg1 = ObjVnum(o);
							arg2 = obj_index[o->item_number].number;
							arg3 = 0;
							strcpy(buf, o->short_description);
							Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
							RecZwriteObj(fp, o);
						}
					}
				}
			}
		}
	}
	fprintf(fp, "S\n");
	fclose(fp);

#endif
}

int ENomeValido(char* pchNome) {
	if(pchNome) {
		while(strlen(pchNome) && !isalpha(pchNome[strlen(pchNome) - 1])) {
			pchNome[strlen(pchNome) - 1] = 0;
		}

		if(strlen(pchNome)) {
			while(pchNome) {
				if(!isalpha(*pchNome)) {
					return FALSE;
				}
				pchNome++;
			}
			return TRUE;
		}
	}
	return FALSE;
}
void clean_playerfile() {

	struct junk {
		struct char_file_u dummy;
		bool AXE;
	};
	struct junk grunt;
	struct dirent* ent;

	time_t timeH;
	int j, max, num_warned, num_processed, num_deleted, num_demoted, ones;
	long age;
	long life;
	DIR* dir;
	long lSize;

	num_warned = num_processed = num_deleted = num_demoted = ones = 0;
	timeH = time(0);

	mudlog(LOG_SYSERR, "time now %i", timeH);

	if((dir = opendir(PLAYERS_DIR)) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			FILE* pFile;
			char szFileName[256];

			if(*ent->d_name == '.') {
				continue;
			}

			sprintf(szFileName, "%s/%s", PLAYERS_DIR, ent->d_name);

			if(strstr(ent->d_name, ".dat")) {
				if((pFile = fopen(szFileName, "r+")) != NULL) {
					grunt.AXE = FALSE;

					fseek(pFile, 0, SEEK_END);
					lSize = ftell(pFile);
					rewind(pFile);

					mudlog(LOG_SYSERR, "%i -- %i", sizeof(grunt.dummy), lSize);
					if(fread(&grunt.dummy, 1, sizeof(grunt.dummy), pFile) != sizeof(grunt.dummy)) {
						mudlog(LOG_SYSERR, "Nome: %s", grunt.dummy.name);
						num_processed++;

						/* Fa la lista dei personaggi attivi.. a bit tedious */
						const char* classname[] = { "Mu", "Cl", "Wa", "Th",
													"Dr", "Mo", "Ba", "So", "Pa", "Ra", "Ps", "?",
													"??"
												  };
						char classes[100];
						classes[0] = '\0';
						int i;

						for(i = max = 0; i < MAX_CLASS; i++) {
							/*calcola il livello piu` alto in max*/
							if(grunt.dummy.level[i] > max) {
								max = grunt.dummy.level[i];
							}

							if(grunt.dummy.level[i] > 0) {
								if(strlen(classes) != 0) {
									strcat(classes, "/");
								}

								sprintf(classes + strlen(classes), "%s",
										classname[i]);
							}
						}

						if(grunt.dummy.points.max_hit > 500) {
							buglog(LOG_PLAYERS,
								   "%s (%s) ha piu' di 500 hp (%d)",
								   grunt.dummy.name, classes,
								   grunt.dummy.points.max_hit);
						}

						if(max < IMMORTALE) {
							j = (int) max;
							if(j < 5) {
								j = 5;
							}

							age = timeH - grunt.dummy.last_logon;

							mudlog(LOG_SYSERR,
								   "*****%s****Last logon: %i***age-->%i",
								   grunt.dummy.name, grunt.dummy.last_logon,
								   age);
							mudlog(LOG_SYSERR, "*****%s",
								   grunt.dummy.description);

							/* BUG BUG BUG */
							/* Cancella il player dopo j settimane
							 mudlog(LOG_PLAYERS, "%---Trascorso da Last Log %i---Time to delete %i", age, (long) j * (SECS_PER_REAL_DAY * 7));
							 if (!grunt.AXE && age > (long) j * (SECS_PER_REAL_DAY * 7) && !IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
							 num_deleted++;
							 regdelete(grunt.dummy.name);
							 grunt.AXE = TRUE;
							 mudlog(LOG_PLAYERS, "%s marked for deletion after more than %d weeks of inactivity.", grunt.dummy.name, j);
							 } */

							/* even the no_deletes get deleted after a time
							 if (IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
							 j *= 3;
							 if (!grunt.AXE && age > (long) (j) * (SECS_PER_REAL_DAY * 7)) {
							 num_deleted++;
							 // regdelete(lower(grunt.dummy.name));
							 grunt.AXE = TRUE;
							 mudlog(LOG_PLAYERS, "%s marked for deletion after %d weeks of inactivity (NO_DELETE).", grunt.dummy.name, j);
							 }
							 } */

							/* Avviso di cancellazione imminente*/

							if(!grunt.AXE
									&& age
									> (long)(j - 1)
									* (SECS_PER_REAL_DAY * 7)&& !IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
								num_warned++;
								life = (long)(j * 7)
									   - (age / SECS_PER_REAL_DAY);
								if(life < 2) {
									mudlog(LOG_PLAYERS,
										   "XXX %s to be deleted in %d day",
										   grunt.dummy.name, (int) life);
								}
							}
						}
						else if(max >= IMMORTALE) {
							if(max > IMMENSO) {
								num_deleted++;
								grunt.AXE = TRUE;
								mudlog(LOG_PLAYERS,
									   "%s marked for deletion (TOOHIGHLEVEL).",
									   grunt.dummy.name);
							}
						}
						else {
							mudlog(LOG_ERROR, "Error %s reading file %s.",
								   strerror(errno), szFileName);
						}

						fclose(pFile);

						if(grunt.AXE) {
							mudlog(LOG_CHECK, "%s doveva essere cancellato",
								   grunt.dummy.name);
						}
					}
					else {
						mudlog(LOG_ERROR, "Error opening file %s.", szFileName);
					}
				}
			}
		} /* while */
	}
	else {
		mudlog(LOG_ERROR, "Error opening dir %s.", PLAYERS_DIR);
	}
	mudlog(LOG_CHECK, "-- %d characters processed.", num_processed);
	mudlog(LOG_CHECK, "-- %d characters warned.", num_warned);
	mudlog(LOG_CHECK, "-- %d characters deleted.  ", num_deleted);
	mudlog(LOG_CHECK, "-- %d gods demoted due to inactivity.", num_demoted);
	mudlog(LOG_CHECK, "Cleaning done.");
}

#if ENABLE_AUCTION

void Start_Auction() {
	auction = (AUCTION_DATA*) calloc(1, sizeof(AUCTION_DATA)); /* DOH!!! */
	if(auction == NULL) {
		mudlog(LOG_ERROR, "malloc'ing AUCTION_DATA didn't give %d bytes",
			   sizeof(AUCTION_DATA));
		exit(1);
	}

	auction->item = NULL; /* nothing is being sold */

}
#endif

ACTION_FUNC(do_WorldSave) {
	if(!ch->desc) {
		return;
	}

	send_to_char("Comando disabilitato\r\n", ch);
	return;
}

}


