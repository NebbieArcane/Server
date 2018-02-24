/**
*/

#ifndef __PROTOS_HPP
#define __PROTOS_HPP
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "hash.hpp"
#include "mail.hpp"
#include "maximums.hpp"
#include "parser.hpp"
#include "race.hpp"
#include "script.hpp"
#include "spells.hpp"
#include "structs.hpp"
#include "trap.hpp"
#include "utils.hpp"
#include "vt100c.hpp"
#include "wizlist.hpp"
#ifdef KLUDGE_STRING
char* strdup(char*);
#else
#endif
#include "fight.hpp"
#include "regen.hpp"
#include "opinion.hpp"
#include "sound.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.obj2.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "board.hpp"
#include "comm.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "hash.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magic3.hpp"
#include "mobact.hpp"
#include "modify.hpp"
#include "multiclass.hpp"
#include "reception.hpp"	
#include "security.hpp"
#include "shop.hpp"
#include "signals.hpp"
#include "skills.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "spec_procs3.hpp"
#include "spells.hpp"
#include "spells2.hpp"
#include "mind_use1.hpp"
#include "utility.hpp"
#include "create.hpp"
#include "ansi_parser.hpp"
#include "mail.hpp"
#include "create.hpp"
#include "status.hpp"
#include "trap.hpp"
#include "magicutils.hpp"
#include "weather.hpp"


/*
int is_murdervict(struct char_data* ch);
void assign_mobiles();
void clone_char(struct char_data *ch);
void clone_obj(struct obj_data *obj);

bool circle_follow(struct char_data* ch, struct char_data* victim);
bool saves_spell(struct char_data* ch, sh_int save_type);
bool ImpSaveSpell(struct char_data* ch, sh_int save_type, int mod);
void AddCommand( char* name, void (*func)( struct char_data*, char*, int ),
				 int number, int min_pos, int min_lev );




bool isNullChar (struct char_data* ch); // SALVO nuova funzione

void Zwrite (FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
			 int arg4, char* desc);
void RecZwriteObj(FILE* fp, struct obj_data* o);
int SaveZoneFile(FILE* fl, int start_room, int end_room);
int LoadZoneFile(FILE* fl, int zon);
FILE* MakeZoneFile( struct char_data* c, int zone);
FILE* OpenZoneFile(struct char_data* c, int zone);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
void mud_time_passed2(time_t t2, time_t t1, struct time_info_data* t);
bool CheckPrac (int classe, int id, int liv); // SALVO nuova funzione
*/

#endif
