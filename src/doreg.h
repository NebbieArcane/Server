/*$Id: doreg.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/
#define __DOREG
#ifndef __DOREG
#define __DOREG 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REG_PC      0
#define REG_DIO     1
#define REG_CODICE  2
#define REG_NOME    3
#define REG_EMAIL   4
#define REG_ALTRO   5
#define REG_MAX     6
#define REG_LAST    5
#define REG_CODELEN 6
#define NEW 1
#define ADD 2
#define CHG 3
#define NOMEDB1 "regdb_d.db"
#define NOMEDB2 "regdb_i.db"
#define DBMASK 0666

#if ALAR
#include "snew.h"
#include "protos.h"

void doreg(struct char_data *ch,int argc,char *argv[10]);
char *Info2Char(int level, char *buf);
void Gdbm2Info(char *buf);
void RegInfoClean();
bool gdbPut(sh_int fl);
bool gdbGet();
bool gdbDel();
long gdbShow(struct char_data *ch);
long RegCount();
bool ck_immortal(char *nome);
void pwd2info();
bool GeneraMail(struct RegInfoData *RI);
bool chgpwd();
bool autorent(struct char_data *ch);
bool regdelete(char *nome);
#endif

struct RegInfoData
{
   char info[REG_MAX][81];
};
#endif
