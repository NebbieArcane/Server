/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#ifndef __MULTICLASS_HPP
#define __MULTICLASS_HPP
int BestClassBIT(struct char_data* ch) ;
int BestClassIND(struct char_data* ch) ;
int BestFightingClass(struct char_data* ch) ;
int BestMagicClass(struct char_data* ch) ;
int BestThiefClass(struct char_data* ch) ;
int CountBits(int iClass) ;
int GetALevel(struct char_data* ch, int which) ;
int GetClassLevel(struct char_data* ch, int iClass) ;
int GetHighClass(struct char_data* ch);
int GetMaxClass(struct char_data* ch);
int GetMaxLevel(struct char_data* ch) ;
int GetSecMaxLev(struct char_data* ch) ;
int GetThirdMaxLev(struct char_data* ch) ;
int GetTotLevel(struct char_data* ch) ;
int HasClass(struct char_data* ch, int iClass) ;
int HowManyClasses(struct char_data* ch) ;
int OnlyClass( struct char_data* pChar, int nClass ) ;
void StartLevels(struct char_data* ch) ;
#endif // __MULTICLASS_HPP
} // namespace Alarmud

