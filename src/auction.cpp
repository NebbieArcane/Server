
/*$Id: auction.c,v 1.3 2002/03/17 16:48:47 Thunder Exp $
*/
#if ENABLE_AUCTION
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "protos.hpp"
#include "utils.hpp"
#include "mem_test.hpp"
#include "auction.hpp" // SALVO aggiunta, modificate righe sotto
#include "utility.hpp"

#define SELLER 0 // SALVO per la nuova is_present
#define BUYER 1 // SALVO per la nuova is_present
#define PULSE_INTERNAL             (2) /* si moltiplica per pulse_auction */
AUCTION_DATA*  auction;
extern void talk_auction (char* argument);
void do_auction_int (struct char_data* ch, char* argument, int cmd);
void auction_update();
int advatoi (const char* s);
int parsebet (const int currentbet, const char* argument);

static bool is_present (int chi, const char* nome) { // SALVO riscritta
	struct descriptor_data* d;
	extern struct descriptor_data* descriptor_list;

	if (chi == SELLER) {
		if (auction->seller
				&& !(IS_LINKDEAD(auction->seller) && auction->seller->in_room==3))
			if (!strcmp(GET_NAME(auction->seller), nome))
			{ return TRUE; }
	}
	else {
		if (auction->buyer
				&& !(IS_LINKDEAD(auction->buyer) && auction->buyer->in_room==3))
			if (!strcmp(GET_NAME(auction->buyer), nome))
			{ return TRUE; }
	}
	for (d = descriptor_list; d; d = d->next) {
		if( d->character && d->character->player.name) {
			if (!strcmp(GET_NAME((d->original) ? d->original : d->character), nome)) {
				if (chi == SELLER)
				{ auction->seller = d->character; }
				else
				{ auction->buyer = d->character; }
				return TRUE;
			}
		}
	}
	if (chi == SELLER)
	{ auction->seller = NULL; }
	else
	{ auction->buyer = NULL; }
	return FALSE;
}

void do_auction_int (struct char_data* ch, char* argument, int cmd) {
	OBJ_DATA* obj;
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, arg1);

	if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
	{ return; }

	if (arg1[0] == '\0')
		if (auction->item != NULL) {
			if (auction->bet > 0) {
				sprintf (buf, "Offerta corrente, fatta da %s, per %s e' %d monete.\n\rOggetto messo all' asta da %s.\n\r",
						 auction->real_buyer // SALVO il vero comratore
						 , auction->item->short_description
						 , auction->bet
						 , auction->real_seller); // SALVO il vero venditore
			}
			else {
				sprintf (buf, "%s non ha' ricevuto offerte.\n\rOggetto messo all' asta da %s.\n\r",auction->item->short_description,auction->real_seller); // SALVO il vero venditore
			}
			send_to_char (buf,ch);

			if (IS_IMMORTAL(ch))
			{ spell_identify(10, ch,NULL, auction->item); } /* uuuh! */

			return;
		}
		else {
			send_to_char ("Mettere all'asta COSA?\n\r",ch);
			return;
		}

	if (IS_IMMORTAL(ch) && (!str_cmp(arg1,"purge")
							|| (!str_cmp(arg1,"stop"))
							|| (!str_cmp(arg1,"halt"))))
		if (auction->item == NULL) {
			send_to_char ("Non c'e' nessuna asta in corso da fermare.\n\r",ch);
			return;
		}
		else { /* stop the auction */
			if (!str_cmp(arg1,"stop") || !str_cmp(arg1,"halt")) {
				sprintf (buf,"La vendita di %s e' stata annullata da un Dio. Oggetto confiscato.", auction->item->short_description);
				mudlog(LOG_CHECK, "ASTA: vendita di (%s) annullata dal Dio %s", auction->item->name, GET_NAME(ch));
				obj_to_char (auction->item, ch);
			}
			else {
				sprintf (buf,"Spiacente, la vendita di %s e' stata revocata. Oggetto distrutto.", auction->item->short_description);
				mudlog(LOG_CHECK, "ASTA: vendita di (%s) revocata dal Dio %s", auction->item->name, GET_NAME(ch));
				extract_obj(auction->item);
			}

			is_present(SELLER, auction->real_seller); // SALVO allineo il venditore
			talk_auction (buf);
			auction->item = NULL;
			auction->bet = 0;
			auction->buyer = NULL;
			strcpy(auction->real_buyer, "(nessuno)"); // SALVO il vero compratore
			auction->seller = ch;
			strcpy(auction->real_seller, GET_NAME(ch)); // SALVO il vero venditore
			if (is_present(BUYER, auction->real_buyer)) { /* SALVO il vero compratore */
				GET_GOLD(auction->buyer) += auction->bet;
				send_to_char ("Ti sono stati restituiti i soldi.\n\r",auction->buyer);
			}
			/**** SALVO resetto auction */
			auction->buyer = NULL;
			strcpy(auction->real_buyer, "(nessuno)");
			auction->seller = NULL;
			strcpy(auction->real_seller, "(nessuno");
			/****/
			return;
		}

	if (cmd==CMD_BID)
		if (auction->item != NULL) {
			int newbet;

			/* make - perhaps - a bet now */
			if (arg1[0] == '\0') {
				send_to_char ("Quanto vuoi offrire?\n\r",ch);
				return;
			}

			newbet = parsebet (auction->bet, arg1);
			mudlog (LOG_PLAYERS,"ASTA: %s offre %d monete per (%s)", GET_NAME(ch), newbet, auction->item->name);

			if (newbet < (auction->bet + 100)) {
				send_to_char ("Devi impegnare almeno 100 monete oltre l'offerta attuale.\n\r",ch);
				return;
			}

			if (newbet > GET_GOLD(ch)) {
				send_to_char ("Non hai abbastanza soldi!\n\r",ch);
				return;
			}

			/* the actual bet is OK! */

			/* return the gold to the last buyer, if one exists */
			if (is_present(BUYER, auction->real_buyer))
			{ GET_GOLD(auction->buyer) += auction->bet; }

			GET_GOLD(ch) -= newbet; /* substract the gold - important :) */
			auction->buyer = ch;
			strcpy(auction->real_buyer, GET_NAME(ch)); // SALVO il vero compratore
			auction->bet   = newbet;
			auction->going = 0;
			auction->pulse = PULSE_AUCTION; /* start the auction over again */

			sprintf (buf,"Un offerta di %d monete e' stata ricevuta per %s.\n\r",
					 newbet,auction->item->short_description);
			is_present(SELLER, auction->real_seller); // SALVO allineo il venditore
			talk_auction (buf);
			return;


		}
		else {
			send_to_char ("Al momento non c'e' niente in vendita.\n\r",ch);
			return;
		}

	/* finally... */

	obj = get_obj_in_list (arg1, ch->carrying); /* does char have the item ? */

	if (obj == NULL) {
		send_to_char ("Non lo porti.\n\r",ch);
		return;
	}

	if (auction->item == NULL)
		switch (GET_ITEM_TYPE(obj)) {

		default:
			act ("Non puoi vendere all'asta $T.",0,ch, NULL, obj->short_description, TO_CHAR);
			return;

		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_STAFF:
		case ITEM_WAND:
		case ITEM_SCROLL:
			obj_from_char (obj);
			/* SALVO eventualmente inserirei un controllo del tipo se l'oggetto non
			         e' rentabile non si puo' vendere, oppure e' una delle cose che
			         puo' essere controllata dall'immortale che se vede le caratteristiche
			         dell'oggetto puo' intervenire in merito */
			auction->item = obj;
			auction->bet = 0;
			auction->buyer = NULL;
			strcpy(auction->real_buyer, "(nessuno)"); // SALVO il vero compratore
			auction->seller = ch;
			strcpy(auction->real_seller, GET_NAME(ch)); // SALVO il vero venditore
			auction->pulse = PULSE_AUCTION;
			auction->going = 0;

			sprintf (buf, "Un nuovo oggetto e' stato messo all'asta: %s.",
					 obj->short_description);
			talk_auction (buf);
			mudlog(LOG_PLAYERS, "ASTA: %s vende (%s)", auction->real_seller,auction->item->name); // SALVO il vero venditore

			return;

		} /* switch */
	else {
		act ("Riprova piu' tardi - attualmente e' in corso l'asta per $p!",
			 0,ch,auction->item,NULL,TO_CHAR);
		return;
	}
}

void auction_update (void) {
	char buf[MAX_STRING_LENGTH];

	if (auction->item != NULL)
		if (--auction->pulse <= 0) { /* decrease pulse */
			auction->pulse = PULSE_INTERNAL;
			switch (++auction->going) { /* increase the going state */
			case 1 : /* going once */
			case 2 : /* going twice */
				if (auction->bet > 0)
					sprintf (buf, "%s: going %s per %d monete.",
							 auction->item->short_description,
							 ((auction->going == 1) ? "uno" : "due"), auction->bet);
				else
					sprintf (buf, "%s: going %s (nessuna offerta).",
							 auction->item->short_description,
							 ((auction->going == 1) ? "uno" : "due"));

				is_present(SELLER, auction->real_seller); // SALVO allineo il venditore
				talk_auction (buf);
				break;

			case 3 : /* SOLD! */

				if (auction->bet >0) {
					if(is_present(BUYER, auction->real_buyer)) { // SALVO il vero compratore
						sprintf (buf, "%s aggiudicata a %s per %d monete.",
								 auction->item->short_description,
								 GET_NAME(auction->buyer),
								 auction->bet);
						is_present(SELLER, auction->real_seller); // SALVO allineo il venditore
						talk_auction(buf);
						mudlog(LOG_PLAYERS, "ASTA: %s compra (%s) per %d monete", GET_NAME(auction->buyer),auction->item->name, auction->bet);
						obj_to_char (auction->item,auction->buyer);
						act ("Il banditore d'asta appare in un soffio di fumo e ti mette nelle mani $p.", 0, auction->buyer,auction->item,NULL,TO_CHAR);
						act ("Il banditore d'asta appare a $n, da' a $m $p", 0, auction->buyer,auction->item,NULL,TO_ROOM);
						if (is_present(SELLER, auction->real_seller)) { // SALVO il vero venditore
							mudlog(LOG_PLAYERS, "ASTA: %s riceve %d per la vendita di (%s)",
								   GET_NAME(auction->seller),
								   auction->bet,
								   auction->item->name);
							GET_GOLD(auction->seller) += auction->bet; /* give him the money */
						}
						else {
							mudlog(LOG_CHECK, "ASTA: venditore assente, non prende %d monete dalla vendita", auction->bet);
						}
						auction->item = NULL; /* reset item */
					}
					else {
						sprintf (buf, "Asta non valida.");
						if (is_present(SELLER, auction->real_seller)) { // SALVO il vero venditore
							mudlog(LOG_CHECK, "ASTA: compratore assente, (%s) restituito a %s", auction->item->name, GET_NAME(auction->seller));
							obj_to_char (auction->item,auction->seller);
						}
						else {
							mudlog(LOG_CHECK, "ASTA: compratore e venditore assenti, (%s) rimosso", auction->item->name);
							extract_obj(auction->item);
						}
						auction->item = NULL;
						talk_auction(buf);
					}
				}
				else { /* not sold */
					sprintf (buf, "Non ci sono state offerte per %s - asta terminata.", auction->item->short_description);
					if (is_present(SELLER, auction->real_seller)) { // SALVO il vero venditore
						mudlog(LOG_PLAYERS, "ASTA: nessuna offerta, (%s) restituito a %s", auction->item->name,GET_NAME(auction->seller));
						obj_to_char (auction->item,auction->seller);
						talk_auction(buf);
						act ("Il banditore d'asta appare e ti restituisce $p.", 0,auction->seller,auction->item,NULL,TO_CHAR);
						act ("Il banditore d'asta appare a $n e restituisce $p a $m.", 0,auction->seller,auction->item,NULL,TO_ROOM);
					}
					else {
						mudlog(LOG_CHECK, "ASTA: venditore assente e nessuna offerta, (%s) rimosso", auction->item->name);
						extract_obj(auction->item);
						talk_auction(buf);
					}
					auction->item = NULL; /* clear auction */
					/**** SALVO resetto auction */
					auction->buyer = NULL;
					strcpy(auction->real_buyer, "(nessuno)");
					auction->seller = NULL;
					strcpy(auction->real_seller, "(nessuno");
					/****/
				} /* else */
			} /* switch */
		} /* if */
} /* func */
#endif
/***************************************************************/


int advatoi (const char* s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

	/* the pointer to buffer stuff is not really necessary, but originally I
	   modified the buffer, so I had to make a copy of it. What the hell, it
	   works:) (read: it seems to work:)
	*/

	char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
	char* stringptr = string; /* a pointer to the buffer so we can move around */
	char tempstring[2];       /* a small temp buffer to pass to atoi*/
	int number = 0;           /* number to be returned */
	int multiplier = 0;       /* multiplier used to get the extra digits right */


	strcpy (string,s);        /* working copy */

	while ( isdigit (*stringptr)) { /* as long as the current character is a digit */
		strncpy (tempstring,stringptr,1);           /* copy first digit */
		number = (number * 10) + atoi (tempstring); /* add to current number */
		stringptr++;                                /* advance */
	}

	switch (UPPER(*stringptr)) {
	case 'K'  :
		multiplier = 1000;
		number *= multiplier;
		stringptr++;
		break;
	case 'M'  :
		multiplier = 1000000;
		number *= multiplier;
		stringptr++;
		break;
	case '\0' :
		break;
	default   :
		return 0; /* not k nor m nor NUL - return 0! */
	}

	while ( isdigit (*stringptr) && (multiplier > 1)) { /* if any digits follow k/m, add those too */
		strncpy (tempstring,stringptr,1);           /* copy first digit */
		multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
		number = number + (atoi (tempstring) * multiplier);
		stringptr++;
	}

	if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
	{ return 0; } /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


	return (number);
}


int parsebet (const int currentbet, const char* argument) {

	int newbet = 0;                /* a variable to temporarily hold the new bet */
	char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
	char* stringptr = string;      /* a pointer we can move around */

	strcpy (string,argument);      /* make a work copy of argument */


	if (*stringptr) {             /* check for an empty string */
		if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
		{ newbet = advatoi (stringptr); } /* parse and set newbet to that value */

		else if (*stringptr == '+') { /* add ?? percent */
			if (strlen (stringptr) == 1) /* only + specified, assume default */
			{ newbet = (currentbet * 125) / 100; } /* default: add 25% */
			else
			{ newbet = (currentbet * (100 + atoi (++stringptr))) / 100; } /* cut off the first char */
		}
		else {
			printf ("considering: * x \n\r"); /* SALVO anche questa da me finisce nel log */
			if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
				if (strlen (stringptr) == 1) /* only x specified, assume default */
				{ newbet = currentbet * 2 ; } /* default: twice */
				else /* user specified a number */
				{ newbet = currentbet * atoi (++stringptr); } /* cut off the first char */
		}
	}

	return newbet;        /* return the calculated bet */
}

