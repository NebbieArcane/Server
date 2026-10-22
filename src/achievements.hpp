/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __ACHIEVEMENTS_HPP
#define __ACHIEVEMENTS_HPP
/***************************  System  include ************************************/
#include <string>
/***************************  Local    include ************************************/
#include "typedefs.hpp"
namespace Alarmud {
bool CheckMercyTable(struct char_data* ch, int quest, int amount);
int CheckMobQuest(int vnumber);
void CheckQuestFail(struct char_data* ch);
void AssignMobQuestToToon(struct char_data* ch, int quest, int vnumber);
bool CheckQuest(struct char_data* ch, int quest_number);
int n_bosskill(int vnumber, int achievement_class);
int race_achievement(int race);
void restringReward(struct obj_data* obj, int obj_slot_number, int max_name, int val_random);
/**
 * Remap legacy achievement potions (1370) / dust (1373) onto Staff protos
 * 37051-37073. Call after inventorio overlay at login. true se ha rimappato.
 */
[[nodiscard]] bool achie_reward_remap_obj(struct obj_data* obj);
/** true se vnum e' una polvere achievement (legacy 1373 o Staff 37069-37073). */
[[nodiscard]] bool is_achie_dust_vnum(int vnum) noexcept;
/** Vnum Staff pozione da signature spell (value1/2/3), 0 se sconosciuta. */
[[nodiscard]] int achie_potion_vnum_from_values(int v1, int v2, int v3) noexcept;
void CheckAchie(struct char_data* ch, int achievement_type, int achievement_class, int amount = 1,
				bool defer_save = false);
struct LivelloAchie StringaAchie(int valore, int achievement_type, int achievement_class);
ACTION_FUNC(do_achievements) ;
ACTION_FUNC(do_checkachielevel) ;
} // namespace Alarmud
#endif // __ACHIEVEMENTS_HPP
