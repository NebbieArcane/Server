/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_OTHER_HPP
#define _ACT_OTHER_HPP
namespace Alarmud {
int CanMemorize(struct char_data* ch) ;
int CheckContempMemorize( struct char_data* pChar ) ;
char* Condition(struct char_data* ch) ;
void Dismount(struct char_data* ch, struct char_data* h, int pos) ;
char* Tiredness(struct char_data* ch) ;
int TotalMaxCanMem( struct char_data* ch ) ;
int TotalMemorized( struct char_data* ch ) ;
void check_memorize(struct char_data* ch, struct affected_type* af) ;
void ck_eq_action( struct char_data* ch, struct obj_data* obj ) ;
ACTION_FUNC(do_alias) ;
ACTION_FUNC(do_brief) ;
ACTION_FUNC(do_bug) ;
ACTION_FUNC(do_compact) ;
void do_ego_eq_action( struct char_data* ch ) ;
ACTION_FUNC(do_gain) ;
ACTION_FUNC(do_group) ;
ACTION_FUNC(do_group_name) ;
ACTION_FUNC(do_guard) ;
ACTION_FUNC(do_hide) ;
ACTION_FUNC(do_idea) ;
ACTION_FUNC(do_junk) ;
ACTION_FUNC(do_memorize) ;
ACTION_FUNC(do_mount) ;
ACTION_FUNC(do_not_here) ;
ACTION_FUNC(do_plr_noauction) ;
ACTION_FUNC(do_plr_nogossip) ;
ACTION_FUNC(do_plr_noshout) ;
ACTION_FUNC(do_plr_notell) ;
ACTION_FUNC(do_practice) ;
ACTION_FUNC(do_quaff) ;
ACTION_FUNC(do_qui) ;
ACTION_FUNC(do_quit) ;
ACTION_FUNC(do_recite) ;
ACTION_FUNC(do_save) ;
ACTION_FUNC(do_set_afk) ;
ACTION_FUNC(do_set_flags) ;
ACTION_FUNC(do_set_prompt) ;
ACTION_FUNC(do_sneak) ;
ACTION_FUNC(do_steal) ;
ACTION_FUNC(do_title) ;
ACTION_FUNC(do_tspy) ;
ACTION_FUNC(do_typo) ;
ACTION_FUNC(do_use) ;
ACTION_FUNC(do_whois) ;
void parse_eq_action( struct char_data* ch, struct obj_data* obj ) ;
} // namespace Alarmud
#endif // _ACT_OTHER_HPP

