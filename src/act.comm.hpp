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
char RandomChar() ;
const char* RandomWord() ;
void do_ask(struct char_data* ch, const char* argument, int cmd) ;
void do_auction(struct char_data* ch, const char* argument, int cmd) ;
void do_commune(struct char_data* ch, const char* argument, int cmd) ;
void do_eavesdrop(struct char_data* ch, const char* argument, int cmd) ;
void do_gossip(struct char_data* ch, const char* argument, int cmd) ;
void do_gtell(struct char_data* ch, const char* argument, int cmd) ;
void do_new_say(struct char_data* ch, const char* argument, int cmd) ;
void do_pray( struct char_data* ch, const char* argument, int cmd ) ;
void do_report(struct char_data* ch, const char* argument, int cmd) ;
void do_say( struct char_data* ch, const char* argument, int cmd ) ;
void do_shout(struct char_data* ch, const char* argument, int cmd) ;
void do_sign(struct char_data* ch, const char* argument, int cmd) ;
void do_speak(struct char_data* ch, const char* argument, int cmd) ;
void do_split(struct char_data* ch, const char* argument, int cmd) ;
void do_telepathy( struct char_data* ch, const char* argument, int cmd ) ;
void do_tell(struct char_data* ch, const char* argument, int cmd) ;
void do_whisper(struct char_data* ch, const char* argument, int cmd) ;
void do_write(struct char_data* ch, const char* argument, int cmd) ;
bool is_same_group( struct char_data* ach, struct char_data* bch ) ;
char* scrambler(struct char_data* ch,const char* message) ;
void talk_auction(const char* argument) ;
void thief_listen(struct char_data* ch,struct char_data* victim, char* frase,int cmd) ;
} // namespace Alarmud
#endif // _ACT_COMM_HPP

