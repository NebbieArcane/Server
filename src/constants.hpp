/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD */
/* $Id: constants.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $ */
#ifndef __CONSTANTS_HPP
#define __CONSTANTS_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
/* modificato da Jethro, agosto 2011 */
#define EQINDEX 600.0
#define MAX_ALIAS 10
#define BEG_OF_TIME 827100000
#define PRINCEEXP 400000000
extern const char* ignoreKeywords[];
extern const char* eqKeywords[];
extern const char* exitKeywords[];
extern const char* lookKeywords[];
extern const char* stats[];
/*Messaggi per quando una spell scade*/
extern const char* spell_wear_off_msg[];
/*Messaggio per la stanza quando una spell/skill scade*/
extern const char* spell_wear_off_room_msg[];
/* Messaggio di opreavviso di fine spell*/
extern const char* spell_wear_off_soon_msg[];

extern const char* spell_wear_off_soon_room_msg[];

extern int rev_dir[];

extern int TrapDir[];

extern int movement_loss[];

extern const  char* exits[];
/* used in listing exits for this room */
extern const  char* listexits[];

extern const char* dirsTo[];

extern const char* dirsFrom[];

extern const char* dirs[];

extern const char* ItemDamType[];

extern const char* weekdays[7];

extern const char* month_name[17];

extern const int sharp[];

extern const char* eqWhere[];

extern const char* drinks[];

extern const char* drinknames[];

extern int RacialMax[MAX_RACE+1][MAX_CLASS];

extern int RacialHome[MAX_RACE+1][2];

/*  fire cold elec blow acid */

extern int ItemSaveThrows[22][5];

extern int drink_aff[][3];

extern const char* color_liquid[];

extern const char* fullness[];

extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];

extern const char* RaceName[];

extern const char* item_types[];

extern const char* wear_bits[];

extern const char* extra_bits[];

extern const char* extra_bits2[];

extern const char* room_bits[];

extern const char* exit_bits[];

extern const char* sector_types[];

extern const char* equipment_types[];

extern const char* affected_bits[];

extern const char* affected_bits2[];

extern const char* special_user_flags[];

extern const char* immunity_names[];

extern const char* apply_types[];

extern const char* pc_class_types[];

extern const char* npc_class_types[];

extern const char* system_flag_types [];

extern const char* aszLogMessagesType[];

extern const char* action_bits[];


extern const char* player_bits[];


extern const char* position_types[];

extern const char* connected_types[];

extern const char* aszWeaponType[];

extern const char* aszWeaponSpecialEffect[];

extern const char* gaszAlignSlayerBits[];


/* [class], [level] (all) */
extern int thaco[MAX_CLASS][ABS_MAX_LVL];

/* [ch] strength apply (all) */

// tohit, todam, carry_w, wield_w

extern struct str_app_type str_app[31];

/* [dex] skillapply (thieves only) */
extern struct dex_skill_type dex_app_skill[26];

/* [level] backstab multiplyer (thieves only) */
extern byte backstab_mult[ABS_MAX_LVL];

/* [dex] apply (all) */
extern struct dex_app_type dex_app[26];

/* [con] apply (all) */
extern struct con_app_type con_app[26];

/* [int] apply (all) */
extern struct int_app_type int_app[26];

/* [wis] apply (all) */
extern struct wis_app_type wis_app[26];

extern struct chr_app_type chr_apply[26];

extern const char* spell_desc[];

extern struct QuestItem QuestList[4][IMMORTAL];

            /* Achievements */
extern struct XpAchieTable RewardXp[ABS_MAX_LVL];
extern struct MobQuestAchie QuestMobAchie[MAX_QUEST_ACHIE];
extern struct ClassAchieTable AchievementsList[MAX_ACHIE_CLASSES][MAX_ACHIE_TYPE];

extern const char* QuestKind[];
    
extern const char* NameGenStart[];
    
extern const char* NameGenMid[];
    
extern const char* NameGenEnd[];
    
extern const char* att_kick_kill_ch[];

extern const char* att_kick_kill_victim[];

extern const char* att_kick_kill_room[];

extern const char* att_kick_miss_ch[];

extern const char* att_kick_miss_victim[];

extern const char* att_kick_miss_room[];

extern const char* att_kick_hit_ch[];

extern const char* att_kick_hit_victim[];

extern const char* att_kick_hit_room[];
} // namespace Alarmud
#endif

