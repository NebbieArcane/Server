/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __MAGICUTILS_HPP
#define __MAGICUTILS_HPP
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
void FailCalm(struct char_data* victim, struct char_data* ch) ;
void FailCharm(struct char_data* victim, struct char_data* ch) ;
void FailPara(struct char_data* victim, struct char_data* ch) ;
void FailPoison(struct char_data* victim, struct char_data* ch) ;
void FailSleep(struct char_data* victim, struct char_data* ch) ;
void FailSnare(struct char_data* victim, struct char_data* ch) ;
void SwitchStuff( struct char_data* giver, struct char_data* taker) ;
} // namespace Alarmud
#endif // __MAGICUTILS_HPP

