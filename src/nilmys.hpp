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
#define TAMARANG_PRINCE     9001
#define UMAG_ULBAR          9002
#define DAGGAR_IVRAM        9003
#define IREIIN_DRUID        9004
#define CORMAC_RUNAR        9005
#define ISKRA_STANISLAV     9006
#define ARKHAT_GOD          9007
#define GAREBETH            9008
#define GONHAG              9009
#define UGUIK_AURUM         9029
#define BORIS_IVANHOE_CLONE 9048
#define ATROPAL             9049
    
    //  rooms
#define STANISLAV_ROOM      8957
#define SHADOWS_PORTAL      8987
#define BORIS_HOME          9013
#define UMAG_ROOM           9015
#define ARKHAT_GOD_ROOM     9116
#define ATROPAL_ROOM        9149
#define MASS_GRAVE          9199
    
    //  objects
#define VLAD_ARMOR          9008
#define VLAD_SWORD          9027
#define SHADOW_WALLS_KEY    9032
#define NILMYS_SHADOW_KEY   9045
#define STANISLAV_BONES     9047
#define NILMYS_BLEEDER      9050
#define NILMYS_FLASK        9051
#define GONHAG_KEY          9052
#define GONHAG_TOOL         9054
#define VICTORY_PORTAL      9066
#define NILNYS_VICTORY_KEY  9067
#define FAREWELL_PORTAL     9071
#define LOST_SOUL_EQ        9080
#define NILMYS_DIAMOND      9096
#define NILMYS_CORPSE       9100
#define NILMYS_COIN         9101

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
MOBSPECIAL_FUNC(Atropal);
MOBSPECIAL_FUNC(Boris_Ivanhoe);
MOBSPECIAL_FUNC(Garebeth);
MOBSPECIAL_FUNC(Uguik_Aurum);
MOBSPECIAL_FUNC(Umag_Ulbar);
MOBSPECIAL_FUNC(stanislav_spirit);
    //  rooms
ROOMSPECIAL_FUNC(gonhag_block);
ROOMSPECIAL_FUNC(gonhag_chain);
ROOMSPECIAL_FUNC(portale_ombra);
ROOMSPECIAL_FUNC(reward_giver);

    
                    //  other
void BorisDeath(struct char_data* umag);
void BorisInFight(struct char_data* boris);
void CheckBorisRoom(struct char_data* boris);
void CheckReward(struct char_data* boris);
bool CheckUguikRoom(struct char_data* uguik, struct char_data* boris);
bool FindKeyByNumber(struct char_data* ch, int number);
void GarebethDeath(struct char_data* boris);
void GiveRewardNilmys(struct char_data* boris, struct char_data* ch);
void MoveToonInRangeToRoom(int low_number, int high_number, int room);
} // namespace Alarmud
#endif // __SPEC_PROCS3_HPP

