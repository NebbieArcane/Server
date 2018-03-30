/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __TRAP_HPP
#define __TRAP_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#define TRAP_EFFECT_TYPE   0
#define TRAP_DAM_TYPE      1
#define TRAP_LEVEL         2
#define TRAP_CHARGES       3

/*
   trap damage types...
*/

#define TRAP_DAM_POISON    -4
#define TRAP_DAM_SLEEP     -3
#define TRAP_DAM_TELEPORT  -2
#define TRAP_DAM_FIRE      SPELL_FIREBALL
#define TRAP_DAM_COLD      SPELL_FROST_BREATH
#define TRAP_DAM_ACID      SPELL_ACID_BLAST
#define TRAP_DAM_ENERGY    SPELL_COLOUR_SPRAY
#define TRAP_DAM_BLUNT     TYPE_BLUDGEON
#define TRAP_DAM_PIERCE    TYPE_PIERCE
#define TRAP_DAM_SLASH     TYPE_SLASH

#define TRAP_EFF_MOVE      1  /* trigger on movement    */
#define TRAP_EFF_OBJECT    2  /* trigger on get or put  */
#define TRAP_EFF_ROOM      4  /* affect all in froom    */
#define TRAP_EFF_NORTH     8  /*  movement in this dir  */
#define TRAP_EFF_EAST     16
#define TRAP_EFF_SOUTH    32
#define TRAP_EFF_WEST     64
#define TRAP_EFF_UP      128
#define TRAP_EFF_DOWN    256

#define GET_TRAP_LEV(obj) (obj)->obj_flags.value[TRAP_LEVEL]
#define GET_TRAP_EFF(obj) (obj)->obj_flags.value[TRAP_EFFECT_TYPE]
#define GET_TRAP_CHARGES(obj) (obj)->obj_flags.value[TRAP_CHARGES]
#define GET_TRAP_DAM_TYPE(obj) (obj)->obj_flags.value[TRAP_DAM_TYPE]

int CheckForAnyTrap(struct char_data* ch, struct obj_data* i) ;
int CheckForGetTrap(struct char_data* ch, struct obj_data* i) ;
int CheckForInsideTrap(struct char_data* ch, struct obj_data* i) ;
int CheckForMoveTrap(struct char_data* ch, int dir) ;
void FindTrapDamage( struct char_data* v, struct obj_data* i) ;
void InformMess( struct char_data* v) ;
void TrapDam(struct char_data* v, int damtype, int amnt, struct obj_data* t) ;
void TrapDamage(struct char_data* v, int damtype, int amnt, struct obj_data* t) ;
void TrapPoison(struct char_data* v, struct obj_data* t) ;
void TrapSleep(struct char_data* v) ;
void TrapTeleport(struct char_data* v) ;
int TriggerTrap( struct char_data* ch, struct obj_data* i) ;
ACTION_FUNC(do_settrap) ;
} // namespace Alarmud
#endif // __TRAP_HPP

