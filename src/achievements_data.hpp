/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#pragma once

/* Included only from constants.hpp (inside namespace Alarmud). Types/sizes come
 * from structs.hpp and autoenums.hpp already included there — no extra headers. */

extern struct XpAchieTable RewardXp[ABS_MAX_LVL];
extern struct RandomEquipTable EquipName[22][20];
extern struct RandomMaterialsTable MaterialName[5][100];
extern struct ObjAchieTable RewardObj[3][20];
extern struct ObjBonusTable AchieBonus[MAX_CLASS][12];
extern struct MobQuestAchie QuestMobAchie[MAX_QUEST_ACHIE];
extern struct MercySystem QuestNumber[MAX_QUEST_ACHIE];
extern struct ClassAchieTable AchievementsList[MAX_ACHIE_CLASSES]
                                              [MAX_ACHIE_TYPE];
extern struct QuestRewardsTable QuestNebbie[MAX_QUEST_ACHIE][100];
