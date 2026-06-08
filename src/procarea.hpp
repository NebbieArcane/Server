/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_HPP
#define __PROCAREA_HPP
#include "typedefs.hpp"
namespace Alarmud {

constexpr long PROCAREA_VNUM_BASE = 60000L;
constexpr int PROCAREA_SLOTS_PER_INSTANCE = 96;
constexpr int PROCAREA_MAX_ACTIVE = 200;

constexpr long PROCAREA_FOUNTAIN_ROOM = 3001L;

constexpr int PROCAREA_MIN_LEVEL = 1;

/* Curva di difficolta' continua da GetCharBonusIndex() medio del gruppo in ingresso. */
constexpr float PROCAREA_EQ_SCALE_MIN = 30.0f;
constexpr float PROCAREA_EQ_SCALE_MAX = 1200.0f;

constexpr int PROCAREA_ROOMS_MIN = 12;
constexpr int PROCAREA_ROOMS_MAX = 90;

constexpr int PROCAREA_BOSS_VNUM_BASE = 65000;
constexpr int PROCAREA_BOSSES_PER_BAND = 10;
constexpr int PROCAREA_BOSS_COUNT = 50;

constexpr int PROCAREA_MOB_VNUM_BASE = 65100;
constexpr int PROCAREA_MOBS_PER_BAND = 9;
constexpr int PROCAREA_MOB_POOL_SIZE = 8;
constexpr int PROCAREA_TEMPLATE_BANDS = 5;

ACTION_FUNC(do_antro);
ROOMSPECIAL_FUNC(procarea_portal);
ROOMSPECIAL_FUNC(procarea_boss_exit);
ROOMSPECIAL_FUNC(procarea_t1_portal);
ROOMSPECIAL_FUNC(procarea_t1_exit);

long procarea_vnum_to_instance(long vnum);
bool procarea_is_generated_room(long vnum);
void procarea_on_mob_death(struct char_data* victim);
void procarea_maybe_destroy(long instance_id);
void procarea_tick_cleanup();

bool procarea_try_push_fountain(struct char_data* ch, const char* arg);
bool procarea_try_enter_nebbia(struct char_data* ch, const char* arg);

} // namespace Alarmud
#endif // __PROCAREA_HPP
