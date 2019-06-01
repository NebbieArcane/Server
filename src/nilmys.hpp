/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __NILMYS_HPP
#define __NILMYS_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
    //  oggetti
OBJSPECIAL_FUNC(urna_nilmys);
    //  mob
MOBSPECIAL_FUNC(stanislav_spirit);
MOBSPECIAL_FUNC(Boris_Ivanhoe);
    //  stanze
ROOMSPECIAL_FUNC(portale_ombra);
    //  altro
void CheckBorisRoom(struct char_data* boris);
} // namespace Alarmud
#endif // __SPEC_PROCS3_HPP

