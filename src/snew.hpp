/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SNEW
#define __SNEW 1
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "specass2.hpp"
#include "version.hpp"
#include "signals.hpp"
#include "utility.hpp"
namespace Alarmud {

#define STR 0
#define INT 1
#define WIS 2
#define DEX 3
#define CON 4
#define CHR 5
#define MONK_MAX_RENT 20
/* Valori per GetTargetType */
#define gtt_FIRST 0
#define gtt_IS_NONE 0
#define gtt_IS_SELF 1
#define gtt_IS_ENEMY 2
#define gtt_IS_POLY 3
#define gtt_IS_PLAYER 4
#define gtt_IS_FRIEND 5
#define gtt_IS_SUPPORTED 6
#define gtt_IS_BODYGUARDED 7
#define gtt_IS_CLAN 8
#define gtt_LAST 8
#define MIN_EXP(ch) (titles[GetMaxClass(ch)][GET_AVE_LEVEL(ch)].exp)
#define Ansi_len(s) (strlen(ParseAnsiColors(FALSE,s)))
int GetTargetType(struct char_data* ch,struct char_data* target,int ostility);
void AlignMod(struct char_data* ch,struct char_data* victim,int cmd);
void do_setalign(struct char_data* ch,char* argument,int cmd);
void ActionAlignMod(struct char_data* ch,struct char_data* victimi,int ostility);
const char* GetTargetTypeString(int target);
int GetAverageLevel(struct char_data* ch);
char* HostName();
bool IsTest();
bool SetTest(bool test=true);
char* GetKey(char* db, char* chiave);
int PutKey(char* db,char* chiave, char* value);
void DelKey(char* db,char* chiave);
void do_associa(struct char_data* ch,const char* argument, int cmd);
void do_ripudia(struct char_data* ch,const char* argument, int cmd);
void clone_obj_to_obj(struct obj_data* obj, struct obj_data* osrc);
void clone_container_obj( struct obj_data* to, struct obj_data* obj );
struct obj_data* clone_obj(struct obj_data* obj);

void do_vomita(struct char_data* ch,const char* argument, int cmd);
int GetStat(struct char_data* ch, int stat);
long StatCost(struct char_data* ch, int stat); // SALVO meglio se long
int MaxStat(struct char_data* ch,int stat);
int SetStat(struct char_data* ch,int stat,int value);
int GetNum(struct char_data* pMob,int pos);
int GetNumR(struct room_data* pRoom,int pos);
struct char_data* CloneChar(struct char_data* ch,long nroom);
float GetCharBonusIndex(struct char_data* ch);
} // namespace Alarmud
#endif

