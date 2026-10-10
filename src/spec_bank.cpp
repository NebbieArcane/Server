/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
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
#include "spec_bank.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "utility.hpp"
namespace Alarmud {

/* Banca monete (deposit/withdraw/balance). */

ROOMSPECIAL_FUNC(bank) {

	static char buf[256];
	int money,tassa;
	float tasso_bancario=0.05;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	money = atoi(arg);

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	save_char(ch, ch->in_room, 0);

	if(GET_BANK(ch) > GetMaxLevel(ch)*40000 && GetMaxLevel(ch)<40) {
		send_to_char("I'm sorry, but we can no longer hold more than 40000 coins per level.\n\r", ch);
		GET_GOLD(ch) += GET_BANK(ch)-GetMaxLevel(ch)*40000;
		GET_BANK(ch) = GetMaxLevel(ch)*40000;
	}


	/*deposit*/
	if(cmd==CMD_DEPOSIT) {
		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
			send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
			return(TRUE);
		}


		if(money > GET_GOLD(ch)) {
			send_to_char("You don't have enough for that!\n\r", ch);
			return(TRUE);
		}
		else if(money <= 0) {
			send_to_char("Go away, you bother me.\n\r", ch);
			return(TRUE);
		}
		else if((money + GET_BANK(ch) > GetMaxLevel(ch)*40000) &&
				(GetMaxLevel(ch)<40)) {
			send_to_char("I'm sorry, Regulations only allow us to ensure 40000 coins per level.\n\r",ch);
			return(TRUE);
		}
		else {
			send_to_char("La ringraziamo.\n\r",ch);
			GET_GOLD(ch) = GET_GOLD(ch) - money;
			/* inizio procedura calcolo interessi operazione bancaria */
			tassa =(int)(money*tasso_bancario);
			money -= tassa;
			/* termina procedura calcolo interessi operazione bancaria */
			sprintf(buf,"Tassa applicata alla sua operazione: %d monete d'oro.\n\r",tassa);
			send_to_char(buf, ch);
			GET_BANK(ch) = GET_BANK(ch) + money;
			sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
			send_to_char(buf, ch);
			return(TRUE);
		}
		/*withdraw*/
	}
	else if(cmd==CMD_WITHDRAW) {

		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
			send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
			return(TRUE);
		}


		if(money > GET_BANK(ch)) {
			send_to_char("You don't have enough in the bank for that!\n\r", ch);
			return(TRUE);
		}
		else if(money <= 0) {
			send_to_char("Go away, you bother me.\n\r", ch);
			return(TRUE);
		}
		else {
			send_to_char("La ringraziamo.\n\r",ch);
			GET_BANK(ch) = GET_BANK(ch) - money;
			/* inizio procedura calcolo interessi operazione bancaria */
			/*tassa = (int)(money*tasso_bancario);*/
			/*money -= tassa;*/
			/*sprintf(buf,"Tassa applicata alla sua operazione: %d monete d'oro.\n\r",tassa);*/
			/*send_to_char(buf, ch);*/
			/* termina procedura calcolo interessi operazione bancaria */
			GET_GOLD(ch) = GET_GOLD(ch) + money;
			sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
			send_to_char(buf, ch);
			return(TRUE);
		}
		/* Balance */
	}
	else if(cmd == CMD_BALANCE) {
		sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
		send_to_char(buf, ch);
		return(TRUE);
	}
	return(FALSE);
}

} // namespace Alarmud
