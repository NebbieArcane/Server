/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _ACT_COMM_HPP
#define _ACT_COMM_HPP
namespace Alarmud {
ACTION_FUNC(do_ask) ;
ACTION_FUNC(do_auction) ;
ACTION_FUNC(do_commune) ;
ACTION_FUNC(do_eavesdrop) ;
ACTION_FUNC(do_gossip) ;
ACTION_FUNC(do_gtell) ;
ACTION_FUNC(do_new_say) ;
ACTION_FUNC(do_pray) ;
ACTION_FUNC(do_report) ;
ACTION_FUNC(do_say) ;
ACTION_FUNC(do_shout) ;
ACTION_FUNC(do_sign) ;
ACTION_FUNC(do_speak) ;
ACTION_FUNC(do_split) ;
ACTION_FUNC(do_telepathy) ;
ACTION_FUNC(do_tell) ;
ACTION_FUNC(do_whisper) ;
ACTION_FUNC(do_write) ;
bool is_same_group( struct char_data* ach, struct char_data* bch ) ;
char* scrambler(struct char_data* ch,const char* message) ;
void talk_auction(const char* argument) ;
void thief_listen(struct char_data* ch,struct char_data* victim, char* frase,int cmd) ;
} // namespace Alarmud
#endif // _ACT_COMM_HPP

