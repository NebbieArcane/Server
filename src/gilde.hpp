#ifndef __GILDE_HPP
#define __GILDE_HPP
void AppendToGuildList( int nIndex, char* szNomeMembro ) ;
void AssignMob( int vnum, int (*proc)( struct char_data*, int, char*,struct char_data*, int ) ) ;
void AssignObj( int vnum, int (*proc)( struct char_data*, int, char*,struct obj_data*, int ) ) ;
void AssignRoom( int vnum, int (*proc)( struct char_data*, int, char*,struct room_data*, int ) ) ;
void BootGuilds() ;
void GuildBalance( struct char_data* pCh, struct char_data* pMob,int nIndex ) ;
int GuildBalanceXP( struct char_data* pChar, struct char_data* pMob,int nIndex ) ;
int GuildBanker( struct char_data* pCh, int nCmd, char* pArg,struct char_data* pMob, int nType ) ;
void GuildDeposit( struct char_data* pCh, struct char_data* pMob,char* pArg, int nIndex ) ;
int GuildDepositXP( struct char_data* pChar, char* szArg,struct char_data* pMob, int nIndex ) ;
int GuildMemberBook( struct char_data* pChar, int nCmd, char* szArg,struct obj_data* pObj, int nType ) ;
void GuildWithdraw( struct char_data* pCh, struct char_data* pMob,char* pArg, int nIndex ) ;
int GuildWithdrawXP( struct char_data* pChar, char* szArg,struct char_data* pMob, int nIndex ) ;
int GuildXPBanker( struct char_data* pChar, int nCmd, char* szArg,struct char_data* pMob, int nType ) ;
void InitializeMemberList( int nIndex ) ;
int IsGuildGuardRoomFP( int room, void* tgt_room ) ;
int IsInGuildList( int nIndex, char* szNomeMembro ) ;
int PlayersGuildGuard( struct char_data* pCh, int nCmd, char* pArg,struct char_data* pMob, int nType ) ;
void RemoveFromGuildList( int nIndex, char* szNomeMembro ) ;
void UpdateGuildListFile( int nIndex ) ;
#endif // __GILDE_HPP
