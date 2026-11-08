/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_OFF_HPP
#define _ACT_OFF_HPP
namespace Alarmud {

/** Motivo di blocco di clearpath (linea retta stanza→stanza). */
enum class ClearpathBlock {
	Ok = 0,
	Invalid,  /* no exit / room invalida / ch invalido */
	Closed,
	Nolookt,  /* EX_NOLOOKT (solo percorso LOS) */
	Special,  /* passaggio non-porta con altri flag */
	Oneway,   /* uscita monodirezionale */
};

/**
 * Cammino lineare stanza→stanza.
 * @param respect_nolookt true = LOS/tiro (blocca NOLOOKT); false = movimento (run/charge).
 * @param reason se non null, motivo di blocco (o Ok).
 * @return vnum stanza di destinazione, o 0 se bloccato.
 */
int clearpath_ex(struct char_data* ch, long room, int direc, bool respect_nolookt,
				 ClearpathBlock* reason);
/** Linea di vista / tiro: rispetta NOLOOKT. */
int clearpath(struct char_data* ch, long room, int direc);
/** Movimento in linea (run/charge): ignora NOLOOKT. */
int clearpath_move(struct char_data* ch, long room, int direc);
ACTION_FUNC(do_assist) ;
ACTION_FUNC(do_backstab) ;
ACTION_FUNC(do_bash) ;
ACTION_FUNC(do_berserk) ;
ACTION_FUNC(do_bodyguard) ;
ACTION_FUNC(do_fire) ;
ACTION_FUNC(do_flee) ;
ACTION_FUNC(do_hit) ;
ACTION_FUNC(do_kick) ;
ACTION_FUNC(do_kill) ;
ACTION_FUNC(do_order) ;
ACTION_FUNC(do_order_old) ;
ACTION_FUNC(do_parry) ;
ACTION_FUNC(do_quivering_palm) ;
ACTION_FUNC(do_rescue) ;
ACTION_FUNC(do_shoot) ;
ACTION_FUNC(do_slay) ;
ACTION_FUNC(do_springleap) ;
ACTION_FUNC(do_stopfight) ;
ACTION_FUNC(do_support) ;
ACTION_FUNC(do_throw) ;
ACTION_FUNC(do_vomita) ;
ACTION_FUNC(do_weapon_load) ;
ACTION_FUNC(do_wimp) ;
/** damage < 0 = miss; 0 = hit assorbito; > 0 = hit/kill col danno finale. */
void kick_messages(struct char_data* ch, struct char_data* victim, int damage) ;
void throw_object(struct obj_data* o, int dir, int from) ;
void throw_weapon(struct obj_data* o, int dir, struct char_data* targ,struct char_data* ch);
} // namespace Alarmud
#endif // _ACT_OFF_HPP

