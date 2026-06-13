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
/** Tempio nella foresta: corpi da aree effimere e preghiera di soccorso. */
constexpr long PROCAREA_DARKSTAR_TEMPLE = 3014L;

constexpr int PROCAREA_MIN_LEVEL = 1;

/* Curva di difficolta' continua da GetCharBonusIndex() medio del gruppo in ingresso. */
constexpr float PROCAREA_EQ_SCALE_MIN = 30.0f;
constexpr float PROCAREA_EQ_SCALE_MAX = 12000.0f;

constexpr int PROCAREA_ROOMS_MIN = 12;
constexpr int PROCAREA_ROOMS_MAX = 90;

constexpr int PROCAREA_BOSS_VNUM_BASE = 65000;
constexpr int PROCAREA_BOSSES_PER_BAND = 10;
constexpr int PROCAREA_BOSS_COUNT = 10;

constexpr int PROCAREA_MOB_VNUM_BASE = 65100;
constexpr int PROCAREA_MOBS_PER_BAND = 9;
constexpr int PROCAREA_MOB_POOL_SIZE = 8;
constexpr int PROCAREA_ARCHETYPE_COUNT = 19;
constexpr int PROCAREA_TEMPLATE_BANDS = 6;

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

bool procarea_try_pull_fountain(struct char_data* ch, const char* arg);
bool procarea_try_push_fountain(struct char_data* ch, const char* arg);
bool procarea_try_enter_nebbia(struct char_data* ch, const char* arg);

/** Crea al boot il Tempio di DarkStar (3014) e l'uscita verso la piazza fontana. */
void procarea_boot_darkstar_temple();

/** Sposta il corpo PG dal dungeon effimero al tempio DarkStar (3014). */
void procarea_relocate_pc_corpse_to_temple(struct char_data* ch, struct obj_data* corpse);

/** pray DarkStar aiuto: da area effimera o piazza fontana → tempio (3014). */
bool procarea_try_darkstar_aid(struct char_data* ch, const char* prayer);

} // namespace Alarmud
#endif // __PROCAREA_HPP
