/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_WIZARD_HPP
#define _ACT_WIZARD_HPP
namespace Alarmud {
extern char EasySummon;
void CreateOneRoom( int loc_nr) ;
void clone_container_obj( struct obj_data* to, struct obj_data* obj ) ;
struct obj_data* clone_obj(struct obj_data* obj) ;
ACTION_FUNC(do_addhost) ;
ACTION_FUNC(do_advance) ;
ACTION_FUNC(do_at) ;
ACTION_FUNC(do_auth) ;
ACTION_FUNC(do_bamfin) ;
ACTION_FUNC(do_bamfout) ;
ACTION_FUNC(do_beep) ;
ACTION_FUNC(do_clone) ;
ACTION_FUNC(do_create) ;
ACTION_FUNC(do_cset) ;
ACTION_FUNC(do_debug) ;
ACTION_FUNC(do_disconnect) ;
ACTION_FUNC(do_drainlevel) ;
ACTION_FUNC(do_echo) ;
ACTION_FUNC(do_emote) ;
ACTION_FUNC(do_event) ;
ACTION_FUNC(do_force) ;
ACTION_FUNC(do_force_rent) ;
ACTION_FUNC(do_freeze) ;
ACTION_FUNC(do_ghost) ;
ACTION_FUNC(do_god_interven) ;
ACTION_FUNC(do_goto) ;
ACTION_FUNC(do_highfive) ;
ACTION_FUNC(do_immort) ;
ACTION_FUNC(do_imptest) ;
ACTION_FUNC(do_invis) ;
ACTION_FUNC(do_listhosts) ;
ACTION_FUNC(do_mforce) ;
ACTION_FUNC(do_mload) ;
ACTION_FUNC(do_nohassle) ;
ACTION_FUNC(do_noshout) ;
ACTION_FUNC(do_nuke) ;
ACTION_FUNC(do_oload) ;
ACTION_FUNC(do_ooedit) ;
ACTION_FUNC(do_osave) ;
ACTION_FUNC(do_passwd) ;
ACTION_FUNC(do_purge) ;
ACTION_FUNC(do_register) ;
ACTION_FUNC(do_removehost) ;
ACTION_FUNC(do_reroll) ;
ACTION_FUNC(do_resetskills) ;
ACTION_FUNC(do_restore) ;
ACTION_FUNC(do_return) ;
ACTION_FUNC(do_rload) ;
ACTION_FUNC(do_rsave) ;
ACTION_FUNC(do_set) ;
ACTION_FUNC(do_set_log) ;
ACTION_FUNC(do_setsev) ;
ACTION_FUNC(do_show) ;
ACTION_FUNC(do_showskills) ;
ACTION_FUNC(do_shutdow) ;
ACTION_FUNC(do_shutdown) ;
ACTION_FUNC(do_silence) ;
ACTION_FUNC(do_snoop) ;
void do_start(struct char_data* ch) ;
ACTION_FUNC(do_stat) ;
ACTION_FUNC(do_stealth) ;
ACTION_FUNC(do_switch) ;
ACTION_FUNC(do_system) ;
ACTION_FUNC(do_trans) ;
ACTION_FUNC(do_viewfile) ;
ACTION_FUNC(do_wizlock) ;
ACTION_FUNC(do_wreset) ;
ACTION_FUNC(do_zclean) ;
ACTION_FUNC(do_zload) ;
ACTION_FUNC(do_zsave) ;
void dsearch(char* string, char* tmp) ;
void force_return(struct char_data* ch, const char* argument, int cmd) ;
void print_death_room(int rnum, struct room_data* rp, struct string_block* sb) ;
void print_private_room(int rnum, struct room_data* rp, struct string_block* sb) ;
void print_room(int rnum, struct room_data* rp, struct string_block* sb) ;
void purge_one_room(int rnum, struct room_data* rp, int* range) ;
void roll_abilities(struct char_data* ch) ;
void show_room_zone(int rnum, struct room_data* rp,struct show_room_zone_struct* srzs);
} // namespace Alarmud
#endif // _ACT_WIZARD_HPP

