/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __BOARD_HPP
#define __BOARD_HPP
namespace Alarmud {
int board( struct char_data* ch, int cmd, char* arg, struct obj_data* obj,int type);
int board_check_locks (int bnum, struct char_data* ch) ;
int board_display_msg(struct char_data* ch, char* arg, int bnum) ;
void board_load_board() ;
int board_remove_msg(struct char_data* ch, char* arg, int bnum) ;
void board_save_board( int bnum ) ;
int board_show_board(struct char_data* ch, char* arg, int bnum) ;
void board_write_msg(struct char_data* ch, char* arg, int bnum) ;
char* fix_returns(char* text_string) ;
} // namespace Alarmud
#endif // __BOARD_HPP

