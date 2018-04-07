/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#ifndef __OPINION_HPP
#define __OPINION_HPP
int AddFeared(struct char_data* ch, struct char_data* pud) ;
int AddFears(struct char_data* ch, int parm_type, int parm) ;
int AddHated(struct char_data* ch, struct char_data* pud) ;
int AddHatred(struct char_data* ch, int parm_type, int parm) ;
void DeleteFears(struct char_data* ch) ;
void DeleteHatreds(struct char_data* ch) ;
int Fears(struct char_data* ch, struct char_data* v) ;
struct char_data* FindAFearee(struct char_data* ch) ;
struct char_data* FindAHatee(struct char_data* ch) ;
void FreeFears(struct char_data* ch) ;
void FreeHates(struct char_data* ch) ;
int Hates(struct char_data* ch, struct char_data* v) ;
int RemFeared(struct char_data* ch, struct char_data* pud) ;
int RemHated(struct char_data* ch, struct char_data* pud) ;
void RemHatred(struct char_data* ch, unsigned short bitv) ;
void ZeroFeared(struct char_data* ch, struct char_data* v) ;
void ZeroHatred(struct char_data* ch, struct char_data* v) ;
#endif // __OPINION_HPP
} // namespace Alarmud

