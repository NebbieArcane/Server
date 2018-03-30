/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PEDIT_HPP
#define __PEDIT_HPP
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
MOBSPECIAL_FUNC(EditMaster);
void SayMenu(struct char_data* pCh, const char* apchMenu[]) ;
int calc_costopq(int i, int p) ;
int calc_costoxp(int i, int p) ;
static struct char_data* find_editman(struct char_data* ch) ;
} // namespace Alarmud
#endif // __PEDIT_HPP

