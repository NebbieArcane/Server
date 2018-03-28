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
void do_addhost(struct char_data* ch,const char* argument, int cmd) ;
void do_advance(struct char_data* ch,const char* argument, int cmd) ;
void do_at(struct char_data* ch,const char* argument, int cmd) ;
void do_auth(struct char_data* ch,const char* argument, int cmd) ;
void do_bamfin(struct char_data* ch,const char* argument, int cmd) ;
void do_bamfout(struct char_data* ch,const char* argument, int cmd) ;
void do_beep(struct char_data* ch,const char* argument, int cmd) ;
void do_clone(struct char_data* ch,const char* argument, int cmd) ;
void do_create( struct char_data* ch, char* argument, int cmd) ;
void do_cset(struct char_data* ch,const char* argument, int cmd) ;
void do_debug(struct char_data* ch,const char* argument, int cmd) ;
void do_disconnect(struct char_data* ch,const char* argument, int cmd ) ;
void do_drainlevel(struct char_data* ch,const char* argument, int cmd) ;
void do_echo(struct char_data* ch,const char* argument, int cmd) ;
void do_emote( struct char_data* ch, const char* argument, int cmd ) ;
void do_event(struct char_data* ch,const char* argument, int cmd) ;
void do_force(struct char_data* ch,const char* argument, int cmd) ;
void do_force_rent( struct char_data* ch, char* argument, int cmd ) ;
void do_freeze(struct char_data* ch,const char* argument, int cmd) ;
void do_ghost(struct char_data* ch,const char* argument, int cmd) ;
void do_god_interven(struct char_data* ch,const char* argument, int cmd) ;
void do_goto(struct char_data* ch,const char* argument, int cmd) ;
void do_highfive(struct char_data* ch,const char* argument, int cmd) ;
void do_immort(struct char_data* ch,const char* argument, int cmd) ;
void do_imptest(struct char_data* ch,const char* argument, int cmd) ;
void do_invis(struct char_data* ch,const char* argument, int cmd) ;
void do_listhosts(struct char_data* ch,const char* argument, int cmd) ;
void do_mforce(struct char_data* ch,const char* argument, int cmd) ;
void do_mload(struct char_data* ch,const char* argument, int cmd) ;
void do_nohassle(struct char_data* ch,const char* argument, int cmd) ;
void do_noshout(struct char_data* ch,const char* argument, int cmd) ;
void do_nuke(struct char_data* ch,const char* argument, int cmd) ;
void do_oload(struct char_data* ch,const char* argument, int cmd) ;
void do_ooedit(struct char_data* ch,const char* argument, int cmd) ;
void do_osave(struct char_data* ch,const char* argument, int cmd) ;
void do_passwd(struct char_data* ch,const char* argument, int cmd) ;
void do_purge(struct char_data* ch,const char* argument, int cmd) ;
void do_register(struct char_data* ch,const char* argument, int cmd) ;
void do_removehost(struct char_data* ch,const char* argument, int cmd) ;
void do_reroll(struct char_data* ch,const char* argument, int cmd) ;
void do_resetskills( struct char_data* ch, char* argument, int cmd ) ;
void do_restore(struct char_data* ch,const char* argument, int cmd) ;
void do_return(struct char_data* ch,const char* argument, int cmd) ;
void do_rload(struct char_data* ch,const char* argument, int cmd) ;
void do_rsave(struct char_data* ch,const char* argument, int cmd) ;
void do_set(struct char_data* ch,const char* argument, int cmd) ;
void do_set_log(struct char_data* ch,const char* argument, int cmd) ;
void do_setsev(struct char_data* ch,const char* argument, int cmd) ;
void do_show(struct char_data* ch,const char* argument, int cmd) ;
void do_showskills( struct char_data* ch, char* argument, int cmd ) ;
void do_shutdow(struct char_data* ch,const char* argument, int cmd) ;
void do_shutdown(struct char_data* ch,const char* argument, int cmd) ;
void do_silence(struct char_data* ch,const char* argument, int cmd) ;
void do_snoop(struct char_data* ch,const char* argument, int cmd) ;
void do_start(struct char_data* ch) ;
void do_stat(struct char_data* ch,const char* argument, int cmd) ;
void do_stealth(struct char_data* ch,const char* argument, int cmd) ;
void do_switch(struct char_data* ch,const char* argument, int cmd) ;
void do_system(struct char_data* ch,const char* argument, int cmd) ;
void do_trans(struct char_data* ch,const char* argument, int cmd) ;
void do_viewfile(struct char_data* ch,const char* argument, int cmd) ;
void do_wizlock(struct char_data* ch,const char* argument, int cmd) ;
void do_wreset (struct char_data* ch, char* argument, int cmd) ;
void do_zclean(struct char_data* ch,const char* argument, int cmd) ;
void do_zload(struct char_data* ch,const char* argument, int cmd) ;
void do_zsave(struct char_data* ch,const char* argument, int cmd) ;
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

