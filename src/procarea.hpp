/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_HPP
#define __PROCAREA_HPP
#include "typedefs.hpp"
namespace Alarmud {

constexpr long PROCAREA_VNUM_BASE = 60000L;
/** Zona runtime (non in myst.zon): #372 Dimensione Effimera, astral per IsOnPmp. */
constexpr int PROCAREA_ZONE_NUM = 372;
constexpr int PROCAREA_ZONE_BUFFER_TOP = 59999;
constexpr int PROCAREA_ZONE_TOP = 99998;
constexpr int PROCAREA_ZONE_PIN = 99999;
constexpr int PROCAREA_LAST_AREA_ZONE_NUM = 374;
constexpr int PROCAREA_SLOTS_PER_INSTANCE = 96;
constexpr int PROCAREA_MAX_ACTIVE = 200;

constexpr long PROCAREA_FOUNTAIN_ROOM = 3001L;
/** Tempio nella foresta: corpi dalle Dimensioni Effimere e preghiera di soccorso. */
constexpr long PROCAREA_DARKSTAR_TEMPLE = 3014L;

constexpr int PROCAREA_MIN_LEVEL = 1;
/** Livello massimo PG (mortali) usato per il calcolo livello mob in dimensione. */
constexpr int PROCAREA_PC_MAX_LEVEL = 51;
/** Cap livello mostrato per mob/trappole/boss generati in dimensione. */
constexpr int PROCAREA_MOB_LEVEL_CAP = 60;

/* Curva di difficolta' continua da GetCharBonusIndex() medio del gruppo in ingresso. */
constexpr float PROCAREA_EQ_SCALE_MIN = 30.0f;
constexpr float PROCAREA_EQ_SCALE_MAX = 12000.0f;

constexpr int PROCAREA_ROOMS_MIN = 12;
constexpr int PROCAREA_ROOMS_MAX = 90;

constexpr int PROCAREA_BOSS_VNUM_BASE = 65000;
constexpr int PROCAREA_BOSSES_PER_BAND = 55;
constexpr int PROCAREA_BOSS_COUNT = 55;

/** Legacy vnum range (non usato dallo spawn runtime della dimensione). */
constexpr int PROCAREA_MOB_VNUM_BASE = 65000;
constexpr int PROCAREA_MOBS_PER_BAND = 170;
constexpr int PROCAREA_MOB_POOL_SIZE = 150;
constexpr int PROCAREA_TRAP_POOL_SIZE = 20;
constexpr int PROCAREA_ARCHETYPE_COUNT = 225;
constexpr int PROCAREA_THEME_COUNT = 50;
constexpr int PROCAREA_TEMPLATE_BANDS = 6;

/** Vnum logico runtime oggetto procarea (65000+; puo' coincidere con mob/stanze). */
constexpr int PROCAREA_TREASURE_HOARD_OBJ = PROCAREA_MOB_VNUM_BASE + 1;

/** Premi istanza: scudi indossabili su shield e back (65100+). */
constexpr int PROCAREA_REWARD_SHIELD_VNUM_BASE = 65100;
constexpr int PROCAREA_REWARD_SHIELD_COUNT = 6;
/** Bonus scudo premio per livello max gruppo/solitario: 1-10→1 … 51→5 slot. */
constexpr int PROCAREA_REWARD_BONUS_MAX = 5;

/** Cooldown pray DarkStar aiuto per uscire dalla dimensione → tempio (secondi). */
constexpr int PROCAREA_DARKSTAR_EXIT_COOLDOWN_SEC = 30 * 60;
/** Decadimento % drop oggetto tesoro per ogni cumulo oltre il primo. */
constexpr int PROCAREA_TREASURE_GEAR_DROP_DECAY_PCT = 25;

/** Premi equip istanza (65106+): un prototipo per slot/banda (12 se slot doppio). */
constexpr int PROCAREA_REWARD_GEAR_VNUM_BASE = 65106;
constexpr int PROCAREA_REWARD_GEAR_COUNT = 132;

enum class ProcRewardWeaponDamage : int {
	Slash = 0,
	Pierce = 1,
	Blunt = 2,
};

enum class ProcRewardGearSlot : int {
	Light = 0,
	Finger,
	Neck,
	Body,
	Head,
	Legs,
	Feet,
	Hands,
	Arms,
	About,
	Waist,
	Wrist,
	Wield,
	Hold,
	Ear,
	Eyes,
	Count = 16,
};

/** Vnum prototipo premio gear per slot, banda (0-5) e sotto-variante (lato doppio o tipo danno). */
long procarea_reward_gear_vnum(ProcRewardGearSlot slot, int band, int sub_variant = 0);

/** Roll bonus/proc su copia runtime di un'arma premio (band >= 4: slay e cause). */
void procarea_roll_reward_weapon(struct obj_data* obj, int template_band,
								 bool instance_has_ranger = false);

ACTION_FUNC(do_antro);
ROOMSPECIAL_FUNC(procarea_portal);
ROOMSPECIAL_FUNC(procarea_boss_exit);
ROOMSPECIAL_FUNC(procarea_treasure);
ROOMSPECIAL_FUNC(procarea_t1_portal);
ROOMSPECIAL_FUNC(procarea_t1_exit);

long procarea_vnum_to_instance(long vnum);
bool procarea_is_generated_room(long vnum);
void procarea_on_mob_death(struct char_data* victim);
/** Vnum logico 65000+; mob runtime con nr==-1 lo conserva in generic. */
int procarea_mob_iVNum(const char_data* mob);
void procarea_maybe_destroy(long instance_id);
void procarea_tick_cleanup();

bool procarea_try_pull_fountain(struct char_data* ch, const char* arg);
bool procarea_try_push_fountain(struct char_data* ch, const char* arg);
bool procarea_try_enter_nebbia(struct char_data* ch, const char* arg);
/** Sentiero solitario: touch fontana → entra nel vortice (vortice effimero). */
bool procarea_try_touch_solo_fountain(struct char_data* ch, const char* arg);
bool procarea_try_enter_vortice(struct char_data* ch, const char* arg);

/** Spezza last_area e aggiunge zona #372 Dimensione Effimera (solo codice, senza myst.zon). */
void procarea_boot_zone();

/** Crea al boot il Tempio di DarkStar (3014) e l'uscita verso la piazza fontana. */
void procarea_boot_darkstar_temple();

/** Scrive in objects/ i prototipi premio (65100+) prima dell'indicizzazione oggetti. */
void procarea_boot_reward_shields();

/** Scrive in objects/ i prototipi gear premio (65106+) prima dell'indicizzazione oggetti. */
void procarea_boot_reward_gear();

/** Sposta il corpo PG dal dungeon effimero al tempio DarkStar (3014). */
void procarea_relocate_pc_corpse_to_temple(struct char_data* ch, struct obj_data* corpse);

/** pray DarkStar aiuto: in dimensione → tempio; fuori con istanza attiva → ingresso. */
bool procarea_try_darkstar_aid(struct char_data* ch, const char* prayer);

} // namespace Alarmud
#endif // __PROCAREA_HPP
