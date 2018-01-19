/* AlarMUD
 * Questo file serve per avere un punto unico in cui definire oggetti che 
 * voglio avere in TUTTI i file
 * A differenza di utility pero', NON e' definito nelle dependencies, 
 * quindi la sua modifica non causa automaticamente il rebuild del Mud
 * $Id: snew.h,v 1.2 2002/03/23 16:43:20 Thunder Exp $
 * * */
#ifndef __SNEW
#define __SNEW 1
#define CHECK_RENT_INACTIVE 1
#define NICE_LIMITED true
#define CLEAN_AT_BOOT false
#define NEW_ALIGN 1
#define MARKS(s) SetLine(s,__LINE__)
#define MARK SetLine(__FILE__,__LINE__)
#define GIOVANNI 1
#define MAX_ALIAS 10
#define BEG_OF_TIME 827100000
#define PRINCEEXP 400000000
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
#include "mem_test.h"
#include "structs.h"
#include "specass2.h"
#include "version.h"
#include "signals.h"
#define MIN_EXP(ch) (titles[GetMaxClass(ch)][GET_AVE_LEVEL(ch)].exp)
#define Ansi_len(s) (strlen(ParseAnsiColors(FALSE,s)))
int GetTargetType(struct char_data *ch,struct char_data *target,int ostility);
void AlignMod(struct char_data *ch,struct char_data *victim,int cmd);
void do_setalign(struct char_data *ch,char *argument,int cmd);
void ActionAlignMod(struct char_data *ch,struct char_data *victimi,int ostility);
char * GetTargetTypeString(int target);
int GetAverageLevel(struct char_data *ch);
char *HostName();
int IsTest();
char *GetKey(char *db, char *chiave);
int PutKey(char *db,char *chiave, char *value);
void DelKey(char *db,char *chiave);
void do_associa(struct char_data *ch, char *argument, int cmd);
void do_ripudia(struct char_data *ch, char *argument, int cmd);
void PushStatus(const char *szStatus);
void PopStatus();
void do_vomita(struct char_data *ch, char *argument, int cmd);
int GetStat(struct char_data *ch, int stat);
long StatCost(struct char_data *ch, int stat); // SALVO meglio se long
int MaxStat(struct char_data *ch,int stat);
int SetStat(struct char_data *ch,int stat,int value);
int MaxStrForRace(struct char_data *ch);
int MaxIntForRace(struct char_data *ch);
int MaxWisForRace(struct char_data *ch);
int MaxDexForRace(struct char_data *ch);
int MaxConForRace(struct char_data *ch);
int MaxChrForRace(struct char_data *ch);
int GetNum(struct char_data *pMob,int pos);
int GetNumR(struct room_data *pRoom,int pos);
struct char_data *CloneChar(struct char_data *ch,long nroom);
float GetCharBonusIndex(struct char_data *ch);
#endif
