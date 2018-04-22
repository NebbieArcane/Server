/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
**        Ansi color Codes
**        DaleMUD 2.0, msw addtions
**        03/07/94
** $Id: ansi.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __ANSI_PARSER_HPP
#define __ANSI_PARSER_HPP
namespace Alarmud {
#define CLEAR_CURSOR_TO_END    "\033[0K" /* Clear from cursor to end of line */
#define CLEAR_BEGIN_TO_CURSOR  "\033[1K" /* Clear from begin of line to cursor */
#define CLEAR_LINE             "\033[2K" /* Clear line containing cursor */
#define CLEAR_FROM_CURSOR      "\033[0J" /* Clear screen from cursor */
#define CLEAR_TO_CURSOR        "\033[1J" /* Clear screen to cursor */
#define CLEAR_SCREEN           "\033[2J" /* Clear entire screen */

/*
 #define CURSOR_UP(number)      printf("\033[%dA", number)
 #define CURSOR_DOWN(number)    printf("\033[%dB", number)
 #define CURSOR_FORWARD(number) printf("\033[%dC", number)
 #define CURSOR_BACKWARD(number) printf("\033[%dD", number)
 #define CURSOR_POSITION(row, column) printf("\033[%d;%dH", row, column)
*/

#define MOD_NORMAL    "\033[0m"
#define MOD_BOLD      "\033[1m"
#define MOD_FAINT     "\033[2m"
#define MOD_UNDERLINE "\033[4m"
#define MOD_BLINK     "\033[5m"
#define MOD_REVERSE   "\033[7m"

/* 00-09 */
#define FG_BLACK      "\033[0;30m"
#define FG_RED        "\033[0;31m"
#define FG_GREEN      "\033[0;32m"
#define FG_BROWN      "\033[0;33m"
#define FG_BLUE       "\033[0;34m"
#define FG_MAGENTA    "\033[0;35m"
#define FG_CYAN       "\033[0;36m"
#define FG_LT_GRAY    "\033[0;37m"
#define FG_DK_GRAY    "\033[1;30m"
#define FG_LT_RED     "\033[1;31m"
/* 10-15 */
#define FG_LT_GREEN   "\033[1;32m"
#define FG_YELLOW     "\033[1;33m"
#define FG_LT_BLUE    "\033[1;34m"
#define FG_LT_MAGENTA "\033[1;35m"
#define FG_LT_CYAN    "\033[1;36m"
#define FG_WHITE      "\033[1;37m"

#define BK_BLACK      "\033[0;40m"
#define BK_RED        "\033[0;41m"
#define BK_GREEN      "\033[0;42m"
#define BK_BROWN      "\033[0;43m"
#define BK_BLUE       "\033[0;44m"
#define BK_MAGENTA    "\033[0;45m"
#define BK_CYAN       "\033[0;46m"
#define BK_LT_GRAY    "\033[0;47m"

#define ANSI_MOD_NORMAL    "0"
#define ANSI_MOD_BOLD      "1"
#define ANSI_MOD_FAINT     "2"
#define ANSI_MOD_UNDERLINE "4"
#define ANSI_MOD_BLINK     "5"
#define ANSI_MOD_REVERSE   "7"

/* 00-09 */
#define ANSI_FG_BLACK      "30"
#define ANSI_FG_RED        "31"
#define ANSI_FG_GREEN      "32"
#define ANSI_FG_BROWN      "33"
#define ANSI_FG_BLUE       "34"
#define ANSI_FG_MAGENTA    "35"
#define ANSI_FG_CYAN       "36"
#define ANSI_FG_LT_GRAY    "37"
#define ANSI_FG_DK_GRAY    "1;30"
#define ANSI_FG_LT_RED     "1;31"
/* 10-15 */
#define ANSI_FG_LT_GREEN   "1;32"
#define ANSI_FG_YELLOW     "1;33"
#define ANSI_FG_LT_BLUE    "1;34"
#define ANSI_FG_LT_MAGENTA "1;35"
#define ANSI_FG_LT_CYAN    "1;36"
#define ANSI_FG_WHITE      "1;37"

#define ANSI_BK_BLACK      "40"
#define ANSI_BK_RED        "41"
#define ANSI_BK_GREEN      "42"
#define ANSI_BK_BROWN      "43"
#define ANSI_BK_BLUE       "44"
#define ANSI_BK_MAGENTA    "45"
#define ANSI_BK_CYAN       "46"
#define ANSI_BK_LT_GRAY    "47"
char* ansi_parse(const char* code);
/*
 #define WINDOW(top, bottom) printf("\033[%d;%dr%s", top, bottom, CLEAR_SCREEN)
*/
} // namespace Alarmud
#endif //__ANSI_PARSER_HPP

