/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: speciali.c,v 1.1.1.1 2002/02/13 11:14:54 root Exp $*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
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
#include "speciali.hpp"
#include "act.comm.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "fight.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magic3.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "utility.hpp"

namespace Alarmud {

/****************************************************************************
*  Blocca il passaggio in una certa direzione. Room Procedure
****************************************************************************/
ROOMSPECIAL_FUNC(sBlockWay) {
	const char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=room->specparms;
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if(type == EVENT_COMMAND) {
		if((cmd != ndir) ||
				((GetMaxLevel(ch)>=nlev1) && (GetMaxLevel(ch)<=nlev2) && !IS_PRINCE(ch))) {    // Gaia 2001
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}

			std::string out_msg = msg;
			out_msg += "\r\n";
			send_to_char(out_msg.c_str(), ch);
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************
*  Blocca il passaggio in una certa direzione. Mob/Obj Procedure
****************************************************************************/
MOBSPECIAL_FUNC(sMobBlockWay) {
	const char* p;
	char dir[256];
	char lev1[256];
	char lev2[256];
	char msg[256];
	int ndir,nlev1,nlev2;
	p=GET_SPEC_PARM(mob);
	p=one_argument(p,dir);
	p=one_argument(p,lev1);
	p=one_argument(p,lev2);
	only_argument(p,msg);
	ndir=atoi(dir);
	nlev1=atoi(lev1);
	nlev2=atoi(lev2);
	if(type == EVENT_COMMAND) {
		if((cmd != ndir) ||
				((GetMaxLevel(ch)>=nlev1) && (GetMaxLevel(ch)<=nlev2))) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			std::string out_msg = msg;
			out_msg += "\r\n";
			act(out_msg.c_str(), FALSE, mob, 0, ch, TO_VICT);
			act("$n dice qualcosa a $N.", FALSE, mob, 0, ch, TO_NOTVICT);
			return TRUE;
		}
	}
	return FALSE;
}
MOBSPECIAL_FUNC(sEgoWeapon) {
	const char* p;
	char pcname[256];
	p=GET_SPEC_PARM(mob);
	if(strlen(p)>255) {
		return FALSE;
	}
	p=one_argument(p,pcname);
	if(type == EVENT_COMMAND) {
		return TRUE;
	}
	return FALSE;
}

/***** FLYP WORK *****/

/***** FENICE START *****/

/***********************************************************************************
*  Il Mobbo cambia il tipo di danno - va passato alla speciale il codice del danno
*  *Flyp*
***********************************************************************************/
MOBSPECIAL_FUNC(ChangeDam) {
	const char* p;
	char dam[256];
	int damType;

	p=GET_SPEC_PARM(mob);
	p=one_argument(p,dam);
	damType=atoi(dam);

	mob->specials.attack_type=damType;

	return FALSE;

}

/***** FENICE END *****/

/***** TEMPLI EROI START *****/

/****************************************************************************
*  Libro degli eroi - Casta lo spell e scala le rune
*  *Flyp*
****************************************************************************/
/****************************************************************************
*  Guardiano - Blocca gli align diversi
*	 Alla speciale si passa -1000 per far passare evil, 0 neutral, 1000 good
*  *Flyp*
****************************************************************************/
MOBSPECIAL_FUNC(MobBlockAlign) {
	const char* p;
	char dir[256];
	char align[256];
	char msg[256];

	int ndir, nalign, tmpalign;


	if(type == EVENT_COMMAND) {
		p=GET_SPEC_PARM(mob);

		p=one_argument(p,dir);
		p=one_argument(p,align);
		only_argument(p,msg);

		ndir=atoi(dir);
		nalign=atoi(align);


		tmpalign=GET_ALIGNMENT(ch);

		//definiamo gli allineamenti
		if(tmpalign<=-350) {
			tmpalign=-1000;
		}
		else if(tmpalign>=350) {
			tmpalign=1000;
		}
		else {
			tmpalign=0;
		}
		if((cmd != ndir) || (tmpalign==nalign)) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			//sprintf(lev2,"%s\r\n",msg);
			act(msg, FALSE, mob, 0, ch, TO_VICT);
			act("$n dice qualcosa a $N.", FALSE, mob, 0, ch, TO_NOTVICT);
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************************
*  Ingresso - Blocca gli align diversi
*	 Alla speciale si passa -1000 per far passare evil, 0 neutral, 1000 good
*  *Flyp*
****************************************************************************/
ROOMSPECIAL_FUNC(BlockAlign) {
	const char* p;
	char dir[256];
	char align[256];
	char msg[256];

	int ndir, nalign, tmpalign;

	if(type == EVENT_COMMAND) {
		p=room->specparms;
		p=one_argument(p,dir);

		p=one_argument(p,align);
		only_argument(p,msg);

		ndir=atoi(dir);
		nalign=atoi(align);


		tmpalign=GET_ALIGNMENT(ch);

		//definiamo gli allineamenti
		if(tmpalign<=-350) {
			tmpalign=-1000;
		}
		else if(tmpalign>=350) {
			tmpalign=1000;
		}
		else {
			tmpalign=0;
		}

		if((cmd != ndir) || (tmpalign==nalign)) {
			return(FALSE);
		}
		else {
			if(!msg[0]) {
				sprintf(msg,"Una forza oscura ti impedisce di passare");
			}
			std::string out_msg = msg;
			out_msg += "\r\n";
			send_to_char(out_msg.c_str(), ch);
			return TRUE;
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(LadroOfferte) {
	char buf[256], buf2[256];

	one_argument(arg,buf);
	only_argument(arg,buf2);

	if(type == EVENT_COMMAND) {
		if(cmd == CMD_GET) {
			if((strstr(buf,"monete"))||(strstr(buf2,"monete"))) {
				do_kill(mob,GET_NAME(ch),0);
				return FALSE;
			}
		}
	}
	return FALSE;
}

/***** TEMPLI EROI END *****/

/***** NEO ORSHINGAL START *****/


/***** NEO ORSHINGAL END *****/
} // namespace Alarmud

