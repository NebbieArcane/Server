/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPECIALI_HPP
#define __SPECIALI_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
int BlockAlign( struct char_data* ch, int cmd, char* arg, struct room_data* pRoom, int type ) ;
int ChangeDam( struct char_data* pChar, int nCmd, char* szArg, struct char_data* pMob, int nType ) ;
int LadroOfferte( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) ;
int LibroEroi(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
int MobBlockAlign( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) ;
int Nightmare( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) ;
int Vampire_Summoner( struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type ) ;
int sBlockWay( struct char_data* pChar, int nCmd, char* szArg, struct room_data* pRoom, int nType ) ;
int sEgoWeapon( struct char_data* pChar, int nCmd, char* szArg,struct char_data* pMob, int nType ) ;
int sMobBlockWay( struct char_data* pChar, int nCmd, char* szArg, struct char_data* pMob, int nType );
} // namespace Alarmud
#endif // __SPECIALI_HPP

