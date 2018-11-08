/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "typedefs.hpp"
namespace Alarmud {
#ifndef __GILDE_HPP
#define __GILDE_HPP
void AppendToGuildList(int nIndex, const char* szNomeMembro) ;
void AssignMob(int vnum, mobspecial_func, const char* specname, const char* specparams) ;
void AssignObj(int vnum, objspecial_func);
void AssignRoom(int vnum, roomspecial_func);
void BootGuilds() ;
void GuildBalance(struct char_data* pCh, struct char_data* pMob,int nIndex) ;
int GuildBalanceXP(struct char_data* pChar, struct char_data* pMob,int nIndex) ;
int GuildBanker(struct char_data* pCh, int nCmd, char* pArg,struct char_data* pMob, int nType) ;
void GuildDeposit(struct char_data* pCh, struct char_data* pMob,const char* pArg, int nIndex) ;
int GuildDepositXP(struct char_data* pChar, const char* szArg,struct char_data* pMob, int nIndex) ;
int GuildMemberBook(struct char_data* pChar, int nCmd, char* szArg,struct obj_data* pObj, int nType) ;
void GuildWithdraw(struct char_data* pCh, struct char_data* pMob,const char* pArg, int nIndex) ;
int GuildWithdrawXP(struct char_data* pChar, const char* szArg,struct char_data* pMob, int nIndex) ;
int GuildXPBanker(struct char_data* pChar, int nCmd, char* szArg,struct char_data* pMob, int nType) ;
void InitializeMemberList(int nIndex) ;
int IsGuildGuardRoomFP(int room, void* tgt_room) ;
int IsInGuildList(int nIndex, const char* szNomeMembro) ;
MOBSPECIAL_FUNC(PlayersGuildGuard);
void RemoveFromGuildList(int nIndex,const char* szNomeMembro) ;
void UpdateGuildListFile(int nIndex) ;
#endif // __GILDE_HPP
} // namespace Alarmud

