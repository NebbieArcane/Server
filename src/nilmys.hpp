/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __NILMYS_HPP
#define __NILMYS_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
                    //  defines
    // mobiles
#define STANISLAV_SPECTRE   8916
#define BORIS_IVANHOE       9000
#define ARKHAT_GOD          9007
#define UGUIK_AURUM         9029
    
    //  rooms
#define STANISLAV_ROOM      8957
#define SHADOWS_PORTAL      8987
#define BORIS_HOME          9013
#define ARKHAT_GOD_ROOM     9119
#define MASS_GRAVE          9132
    
    //  objects
#define SHADOW_WALLS_KEY    9032
#define NILMYS_FLASK        9051
#define NILNYS_VICTORY_KEY  9067
#define LOST_SOUL_EQ        9080
#define NILMYS_DIAMOND      9096
#define NILMYS_CORPSE       9100

    //  other
#define NILMYS_PORTAL_ONE   1
#define NILMYS_PORTAL_TWO   2
#define NILMYS_PORTAL_THREE 4
#define NILMYS_PORTAL_FOUR  8
#define NILMYS_PORTAL_FIVE  16

                    //  special function
    //  objects
OBJSPECIAL_FUNC(urna_nilmys);
    //  mobiles
MOBSPECIAL_FUNC(Arkhat);
MOBSPECIAL_FUNC(Boris_Ivanhoe);
MOBSPECIAL_FUNC(stanislav_spirit);
    //  rooms
ROOMSPECIAL_FUNC(portale_ombra);

    
                    //  other
void CheckBorisRoom(struct char_data* boris);
bool FindKeyByNumber(struct char_data* ch, int number);
} // namespace Alarmud
#endif // __SPEC_PROCS3_HPP

