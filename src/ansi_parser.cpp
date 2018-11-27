/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: ansi_parser.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
* *** DaleMUD        ANSI_PARSER.C
***                Parser ansi colors for act();
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <ctime>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "ansi_parser.hpp"
#include "interpreter.hpp"
namespace Alarmud {




/*
 *   $CMBFG, where M is modier, B is back group color and FG is fore
 *   $C0001 would be normal, black back, red fore.
 *   $C1411 would be bold, blue back, light yellow fore
 */

char* ansi_parse(const char* code) {
	char m[ 10 ], b[ 10 ],f[ 10 ];
	static char szResult[ 256 ];

	if(!code) {
		return(NULL);
	}

	if(IS_SET(SystemFlags,SYS_NOANSI)) {
		return(NULL);
	}

	/* do modifier */
	switch(code[0]) {
	case '0':
		sprintf(m,"%s",ANSI_MOD_NORMAL);
		break;
	case '1':
		sprintf(m,"%s",ANSI_MOD_BOLD);
		break;
	case '2':
		sprintf(m,"%s",ANSI_MOD_FAINT);
		break;
	/* not used in ansi that I know of */
	case '3':
		sprintf(m,"%s",ANSI_MOD_NORMAL);
		break;
	case '4':
		sprintf(m,"%s",ANSI_MOD_UNDERLINE);
		break;
	case '5':
		sprintf(m,"%s",ANSI_MOD_BLINK);
		break;
	case '6':
		sprintf(m,"%s",ANSI_MOD_REVERSE);
		break;

	default:
		sprintf(m,"%s",ANSI_MOD_NORMAL);
		break;
	}

	/* do back ground color */
	switch(code[1]) {
	case '0':
		sprintf(b,"%s",ANSI_BK_BLACK);
		break;
	case '1':
		sprintf(b,"%s",ANSI_BK_RED);
		break;
	case '2':
		sprintf(b,"%s",ANSI_BK_GREEN);
		break;
	case '3':
		sprintf(b,"%s",ANSI_BK_BROWN);
		break;
	case '4':
		sprintf(b,"%s",ANSI_BK_BLUE);
		break;
	case '5':
		sprintf(b,"%s",ANSI_BK_MAGENTA);
		break;
	case '6':
		sprintf(b,"%s",ANSI_BK_CYAN);
		break;
	case '7':
		sprintf(b,"%s",ANSI_BK_LT_GRAY);
		break;
	default:
		sprintf(b,"%s",ANSI_BK_BLACK);
		break;
	}

	/* do foreground color */
	switch(code[2]) {
	case '0':
		switch(code[3]) {
		/* 00-09 */
		case '0':
			sprintf(f,"%s",ANSI_FG_BLACK);
			break;
		case '1':
			sprintf(f,"%s",ANSI_FG_RED);
			break;
		case '2':
			sprintf(f,"%s",ANSI_FG_GREEN);
			break;
		case '3':
			sprintf(f,"%s",ANSI_FG_BROWN);
			break;
		case '4':
			sprintf(f,"%s",ANSI_FG_BLUE);
			break;
		case '5':
			sprintf(f,"%s",ANSI_FG_MAGENTA);
			break;
		case '6':
			sprintf(f,"%s",ANSI_FG_CYAN);
			break;
		case '7':
			sprintf(f,"%s",ANSI_FG_LT_GRAY);
			break;
		case '8':
			sprintf(f,"%s",ANSI_FG_DK_GRAY);
			break;
		case '9':
			sprintf(f,"%s",ANSI_FG_LT_RED);
			break;
		default:
			sprintf(f,"%s",ANSI_FG_DK_GRAY);
			break;
		}
		break;

	case '1':
		switch(code[3]) {
		/* 10-15 */
		case '0':
			sprintf(f,"%s",ANSI_FG_LT_GREEN);
			break;
		case '1':
			sprintf(f,"%s",ANSI_FG_YELLOW);
			break;
		case '2':
			sprintf(f,"%s",ANSI_FG_LT_BLUE);
			break;
		case '3':
			sprintf(f,"%s",ANSI_FG_LT_MAGENTA);
			break;
		case '4':
			sprintf(f,"%s",ANSI_FG_LT_CYAN);
			break;
		case '5':
			sprintf(f,"%s",ANSI_FG_WHITE);
			break;
		default:
			sprintf(f,"%s",ANSI_FG_LT_GREEN);
			break;
		}
		break;

	default :
		sprintf(f,"%s",ANSI_FG_LT_RED);
		break;
	}


	sprintf(szResult, "\033[%s;%s;%sm", m, b, f);
	return szResult;
}
} // namespace Alarmud

