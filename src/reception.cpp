/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
//#include <sys/ctime>
#include <ctime>
#include <dirent.h>
#include <cstring>
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
#include "reception.hpp"
#include "act.other.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "spec_procs.hpp"

namespace Alarmud {


#define OBJ_SAVE_FILE "pcobjs.obj"
#define OBJ_FILE_FREE "\0\0\0"
#define IS_RENT 127

int cur_depth = 0;
int DontShow = 0;
static int RentItem=0;
/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

void add_obj_cost(struct char_data* ch, struct char_data* re,
				  struct obj_data* obj, struct obj_cost* cost) {
	char buf[MAX_INPUT_LENGTH*2]; // SALVO aumento la lunghezza della stringa per il bug su offer

	int ItemType=0;
	/* Add cost for an item and it's contents, and next->contents */

	if(obj) {
		if(obj->item_number > -1 &&
				cost->ok && ItemEgoClash(ch, obj, 0) >= -10 &&
				obj->obj_flags.cost_per_day >= 0) {

			cost->total_cost += obj->obj_flags.cost_per_day;

			if(re && DontShow==0) {
				sprintf(buf, "%-40s : %s%7d%s monete %s\n\r",
						obj->short_description,
						obj->obj_flags.cost_per_day ? "$c0015" : "",
						obj->obj_flags.cost_per_day,
						obj->obj_flags.cost_per_day ? "$c0007" : "",
						obj->obj_flags.cost>LIM_ITEM_COST_MIN?"$c0001[RARO]$c0007":"");
				send_to_char(buf, ch);
			}
			ItemType=(GET_ITEM_TYPE(obj));
			switch(ItemType) {
			case ITEM_FOOD:
			case ITEM_DRINKCON:
				RentItem--;
			default:
				cost->no_carried++;
				RentItem++;
				break;
			}

			add_obj_cost(ch, re, obj->contains, cost);
			add_obj_cost(ch, re, obj->next_content, cost);
		}
		else if(cost->ok) {
			if(re) {
				act("$n ti dice 'Non posso tenere $p'",FALSE,re,obj,ch,TO_VICT);
				cost->ok = FALSE;
			}
			else {
#if NODUPLICATES
#else
				act("$p non puo` essere salvato.",FALSE,ch,obj,0,TO_CHAR);
#endif
				cost->ok = FALSE;

			}
		}
	}
}


/* forcerent can be:
 * 0 = normal rent
 * 1 = autorent
 * n = store
 * */
bool recep_offer(struct char_data* ch,  struct char_data* receptionist,
				 struct obj_cost* cost, int forcerent) {
	int i,ii,limited_items=0,limited_items_carrying=0;
	char buf[MAX_INPUT_LENGTH];
	struct obj_data* tmp, *tmp_next_obj;
	cost->total_cost = 100; /* Minimum cost */
	cost->no_carried = 0;
	cost->ok = TRUE; /* Use if any "-1" objects */

#if EGO
	/* Chiamata per il controllo dell'EGO dell'eq raro
	   per evitare che uno possa entrare e rentare
	   completamente indenne!
	   Gaia 2001 */

	if(IS_SET(ch->specials.act, ACT_POLYSELF) || (IS_PC(ch) && !EgoSave(ch))) {
		do_ego_eq_action(ch);
	}

#endif

	if(IS_AFFECTED2((IS_POLY(ch)) ? ch->desc->original : ch, AFF2_PKILLER)) {  // SALVO controllo pkiller
		if(receptionist) {
			sprintf(buf, "$n tells you 'Sorry, but someone accused you of murder"
					"... so you better stay a little more!!");
			act(buf,FALSE,receptionist,0,ch,TO_VICT);
		}
		return(FALSE);
	}
	if(forcerent==1) {
		mudlog(LOG_PLAYERS, "%s is being force rented!", GET_NAME(ch));
	}
	RentItem=0;
	add_obj_cost(ch, receptionist, ch->carrying, cost);
	limited_items +=CountLims(ch->carrying);
	for(i = 0; i<MAX_WEAR; i++) {
		add_obj_cost(ch, receptionist, ch->equipment[i], cost);
		limited_items +=CountLims(ch->equipment[i]);
	}

	if(!cost->ok) {
		return(FALSE);
	}


	if(cost->no_carried == 0) {
		if(receptionist)
			act("$n ti dice 'Ma se non hai niente con te!!!'", FALSE,
				receptionist, 0, ch, TO_VICT);
		return(FALSE);
	}


#if LIMITED_ITEMS
	if(limited_items > MaxLimited(GetMaxLevel(ch))|| limited_items_carrying) {
		if(receptionist) {
			sprintf(buf, "$n tells you 'Sorry, but I can't store more than %d "
					"limited items.",
					MaxLimited(GetMaxLevel(ch)));
			act(buf,FALSE,receptionist,0,ch,TO_VICT);
			return(FALSE);
		}

		/* auto renting a idle person, lets wack items they should not
		 * carry here! (limited items) */

		if(!receptionist && (forcerent==1)) {
			i = (limited_items-MaxLimited(GetMaxLevel(ch)));
			/* check carrying items first, least important */
			if(CountLims(ch->carrying)) {
				mudlog(LOG_PLAYERS,"Removing carryed items from %s in force rent.",
					   GET_NAME(ch));
				for(tmp = ch->carrying; tmp; tmp = tmp_next_obj) {
					tmp_next_obj = tmp->next_content;
					if(CountLims(tmp) && (i>0)) {
						obj_from_char(tmp);
						extract_obj(tmp); /* could just drop it in that room... */
						i--;
					}
				} /* end for */
			}

			/* check equiped items next if still over max limited */

			if(i >MaxLimited(GetMaxLevel(ch))) {
				mudlog(LOG_PLAYERS,"Removing equiped items from %s in force rent.",
					   GET_NAME(ch));
				for(ii = 0; ii < MAX_WEAR && i > 0; ii++) {
					if(ch->equipment[ii] && CountLims(ch->equipment[ii])) {
						extract_obj(unequip_char(ch, ii));
						i--;
					}
				} /* end EQ for */
			}

			if(i >MaxLimited(GetMaxLevel(ch))) {
				mudlog(LOG_PLAYERS, "%s force rented and still had more limited "
					   "items than suppose to.", GET_NAME(ch));
			}
		} /* end remove limited on force rent */
	}
#endif


	if(cost->no_carried > MAX_OBJ_SAVE) {
		if(receptionist) {
			sprintf(buf,"$n tells you 'Sorry, but I can't store more than %d items.",
					MAX_OBJ_SAVE);
			act(buf,FALSE,receptionist,0,ch,TO_VICT);
		}
		return(FALSE);
	}

	if(!IS_IMMORTAL(ch) && HasClass(ch, CLASS_MONK)) {
		if(cost->no_carried > MONK_MAX_RENT) {
			send_to_char("Your vows forbid you to carry more than 20 items\n\r", ch);
			return(FALSE);
		}
	}

#if NEW_RENT
	/* RENTAL COST ADJUSTMENT modificare qui e tutte le occorrenze RENTAL COST ADJ  per forzare rent a zero*/
	cost->total_cost = 0;
#else
#endif
#if ALAR_RENT
	int sconto=0;
	sconto=(RentItem/2)-15;
	if(RentItem>30) {
		sconto+=(RentItem/5);
	}
	if(RentItem>60) {
		sconto+=(RentItem/3);
	}
	if(RentItem>90) {
		sconto+=(RentItem/2);
	}
	if(RentItem>120) {
		sconto+=(RentItem/2);
	}
	if(RentItem>150) {
		sconto+=(RentItem/2);
	}
	if(RentItem>180) {
		sconto+=(RentItem/2);
	}
	sconto+=limited_items;
	cost->total_cost=(cost->total_cost)*(100+sconto)/100;
#endif

	if(receptionist) {
#if ALAR_RENT
		sprintf(buf, "$n ti dice 'Hai con te %d oggett%c escluso le vettovaglie.'",
				RentItem,RentItem>1?'i':'o');
		act(buf,FALSE,receptionist,0,ch,TO_VICT);
		sprintf(buf, "$n ti dice 'Il che significa %s$c0007 del %d%%!'",
				sconto>0?"$c0001una maggiorazione":"$c0010uno sconto",
				abs(sconto));
#endif
		act(buf,FALSE,receptionist,0,ch,TO_VICT);
		sprintf(buf,
				"$n ti dice 'Ti viene a costare $c0015%d$c0007 monete al giorno.'",
				cost->total_cost);
		act(buf,FALSE,receptionist,0,ch,TO_VICT);

		/* Controllo se sono in una rece di un castello e aggiusto il conto ***Flyp 20020214 *** */
		if(FindMobInRoomWithFunction(receptionist->in_room, reinterpret_cast<genericspecial_func>(creceptionist))) {
			cost->total_cost=(int)(cost->total_cost*0.8);
			sprintf(buf, "$n ti dice 'Ma sei nel castello, quindi ti costa $c0015%d$c0007 monete al giorno.'", cost->total_cost);
			act(buf,FALSE,receptionist,0,ch,TO_VICT);
		}

		if(forcerent>1) {
			forcerent=MAX(forcerent,20);
			cost->total_cost*=(forcerent/2);
			if(forcerent > 1000000) {
				GET_GOLD(ch)=-5000000;
				cost->total_cost=1000000;
				sprintf(buf,"$n ti dice 'Poiche` hai barato.....  prenotando  %d giorni, "
						"ci rimetti anche la camicia.",
						forcerent);
				mudlog(LOG_CHECK,"%s ci ha provato e ora ha %d coins.",GET_NAME(ch),GET_GOLD(ch));
			}
			else {
				sprintf(buf,"$n ti dice 'Poiche` hai prenotato per %d giorni, "
						"$c0001 ti costa SUBITO$c0007 $c0015%d$c0007 monete.",
						forcerent,cost->total_cost);
			}
			act(buf,FALSE,receptionist,0,ch,TO_VICT);
		}

		/* just a bit on informative coding, wasted space... msw */

		if(limited_items <= 5)
			sprintf(buf, "$n ti dice 'Hai %d oggetti rari.'",
					limited_items);
		else if(limited_items <= 8)
			sprintf(buf, "$n ti dice 'Hum, ha i %d oggetti rari, mica male.'",
					limited_items);
		else if(limited_items < 10)
			sprintf(buf, "$n ti dice 'Hai %d oggetti rari. Cosa vuoi fare? Aprire un supermarket?'",
					limited_items);
		else if(limited_items >= 10)
			sprintf(buf, "$n ti dice 'WOW! Hai %d oggetti  rari. Pensi di essere sol$b a giocare?!?'",
					limited_items);

		act(buf,FALSE,receptionist,0,ch,TO_VICT);

		if(cost->total_cost > (unsigned int)GET_GOLD(ch)) {  // SALVO meglio unsigned
			act("$n ti dice 'Che oltretutto nemmeno ti puoi permettere'",
				FALSE,receptionist,0,ch,TO_VICT);

			if(GetMaxLevel(ch) >=IMMORTALE) {
				act("$n ti dice 'Va beh... visto che sei Immortale... suppongo vada bene cosi`",
					FALSE,receptionist,0,ch,TO_VICT);
				cost->total_cost = 0;
			}
		}

	} /* If receptionist */



	if(cost->total_cost > (unsigned int)GET_GOLD(ch)) {    // SALVO meglio unsigned
		if(forcerent==1) {
			mudlog(LOG_PLAYERS, "%s is being force rented and does not have gold!",
				   GET_NAME(ch));
		}
		return(FALSE);
	}
	else {
		if(forcerent>1) {
			mudlog(LOG_CHECK,"%s: autostoring %d days (%d coins)",GET_NAME(ch),
				   (forcerent*2),cost->total_cost);
			GET_GOLD(ch)-=cost->total_cost;
			cost->total_cost = 0;

		}
		return(TRUE);
	}
}



/* ************************************************************************
* General save/load routines                                              *
************************************************************************* */

void update_file(struct char_data* ch, struct obj_file_u* st) {
	FILE* fl;
	char buf[200];
	struct char_data*  k;
	/*
	 * write the aliases and bamfs:
	 */
#if 0
	if(IS_LINKDEAD(ch)) {
		mudlog(LOG_CHECK,"Can't save %s (linkdead)",GET_NAME(ch));
		return;
	}
#endif
	if(IS_POLY(ch)) {
		k=ch->desc->original;
	}
	else {
		k=ch;
	}
	PushStatus("update_file");
	sprintf(buf, "%s/%s", RENT_DIR, lower(GET_NAME(k)));
	PushStatus("update_file1");

	write_char_extra(k);
	PushStatus("update_file2");

	if(!(fl = fopen(buf, "w"))) {
		mudlog(LOG_ERROR,"%s:%s","saving PC's objects",strerror(errno));
		assert(0);
	}

	rewind(fl);
	strcpy(st->owner, GET_NAME(k));

	WriteObjs(fl, st);

	fclose(fl);
	PopStatus();
	PopStatus();
	PopStatus();
}


/**************************************************************************
 * Routines used to load a characters equipment from disk
 **************************************************************************/
#define LOW_EDITED_ITEMS    34030
#define HIGH_EDITED_ITEMS   34999
    
void obj_store_to_char(struct char_data* ch, struct obj_file_u* st) {
	struct obj_data* obj;
	struct obj_data* in_obj[64],*last_obj = NULL;
	int tmp_cur_depth=0;
	int i, j, iRealObjNumber;

	void obj_to_char(struct obj_data *object, struct char_data *ch);

	for(i = 0; i < 64; i++) {
		in_obj[ i ] = NULL;
	}

	SetStatus(STATUS_OTCBEFORELOOP, NULL);

	for(i=0; i<st->number; i++) {
		SetStatus(STATUS_OTCREALOBJECT, NULL);

		if(st->objects[i].item_number > 0 &&
				(iRealObjNumber = real_object(st->objects[i].item_number)) > -1) {
			SetStatus(STATUS_OTCREADOBJECT, NULL);
			if((obj = read_object(st->objects[i].item_number, VIRTUAL)) !=
					NULL) {
#if LIMITED_ITEMS
				/* Se l` oggetto costa al rent, e` considerato raro, e percio` viene
				 * gia` contato nella procedura CountLimitedItems. Questo dovrebbe
				 * risolvere il problema degli oggetti rari che non ripoppano come
				 * dovrebbero.
				 */
				if(obj->item_number >= 0 &&
						obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
					obj_index[ obj->item_number ].number--;
				}
#endif
				SetStatus(STATUS_OTCCOPYVALUE, NULL);

				obj->obj_flags.value[0] = st->objects[i].value[0];
				obj->obj_flags.value[1] = st->objects[i].value[1];
				obj->obj_flags.value[2] = st->objects[i].value[2];
				obj->obj_flags.value[3] = st->objects[i].value[3];
				obj->obj_flags.extra_flags  = st->objects[i].extra_flags;
                if(IS_SET(ch->specials.act,PLR_NEW_EQ))
                {
                    obj->obj_flags.extra_flags2 = st->objects[i].extra_flags2;
                }
				obj->obj_flags.weight       = st->objects[i].weight;
				obj->obj_flags.timer        = st->objects[i].timer;
				obj->obj_flags.bitvector    = st->objects[i].bitvector;
                
				SetStatus(STATUS_OTCFREESTRING, NULL);

				if(obj->name) {
					free(obj->name);
				}
				if(obj->short_description) {
					free(obj->short_description);
				}
				if(obj->description) {
					free(obj->description);
				}

				SetStatus(STATUS_OTCALLOCSTRING, NULL);

				obj->name = (char*)malloc(strlen(st->objects[i].name)+1);
				obj->short_description = (char*)malloc(strlen(st->objects[i].sd)+1);
				obj->description = (char*)malloc(strlen(st->objects[i].desc)+1);

				SetStatus(STATUS_OTCCOPYSTRING, NULL);

				strcpy(obj->name, st->objects[i].name);
				strcpy(obj->short_description, st->objects[i].sd);
				strcpy(obj->description, st->objects[i].desc);

                if(st->objects[i].item_number >= LOW_EDITED_ITEMS && st->objects[i].item_number <= HIGH_EDITED_ITEMS && !pers_on(ch,obj) && !IS_OBJ_STAT2(obj, ITEM2_PERSONAL))
                {
                    pers_obj(ch, ch, obj, 1000);
                    if(!IS_OBJ_STAT2(obj, ITEM2_EDIT))
                    {
                        SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
                    }
                    if(!IS_OBJ_STAT2(obj, ITEM2_PERSONAL))
                    {
                        SET_BIT(obj->obj_flags.extra_flags2, ITEM2_PERSONAL);
                    }
                }

				SetStatus(STATUS_OTCCOPYAFFECT, NULL);

				for(j=0; j<MAX_OBJ_AFFECT; j++) {
					obj->affected[j] = st->objects[i].affected[j];
				}

				SetStatus(STATUS_OTCBAGTREE, NULL);

				/* item restoring */
				if(st->objects[i].depth > 60) {
					mudlog(LOG_SYSERR, "weird! object have depth >60.");
					st->objects[i].depth = 0;
				}
				if(st->objects[i].depth && st->objects[i].wearpos) {
					mudlog(LOG_SYSERR,
						   "weird! object (%s) weared and in cointainer.",obj->name);
					st->objects[i].depth = st->objects[i].wearpos=0;
				}
				if(st->objects[i].depth > tmp_cur_depth) {
					if(st->objects[i].depth != tmp_cur_depth + 1) {
						mudlog(LOG_SYSERR, "weird! object depth changed from %d to %d",
							   tmp_cur_depth, st->objects[i].depth);
					}
					if(last_obj == NULL) {
						mudlog(LOG_SYSERR,
							   "weird! object depth > current depth but not last_obj in "
							   "obj_store_to_char (reception.c).");
					}

					in_obj[ tmp_cur_depth++ ] = last_obj;
				}
				else if(st->objects[i].depth<tmp_cur_depth) {
					tmp_cur_depth--;
				}
				if(st->objects[ i ].wearpos) {
					equip_char(ch, obj, st->objects[ i ].wearpos - 1);
				}
				else if(tmp_cur_depth) {
					if(in_obj[ tmp_cur_depth - 1 ]) {
						obj_to_obj(obj, in_obj[ tmp_cur_depth - 1 ]);
					}
					else
						mudlog(LOG_SYSERR,
							   "weird! in_obj[ tmp_cur_depth - 1 ] == NULL in "
							   "obj_store_to_char (reception.c).");
				}
				else {
					obj_to_char(obj, ch);
				}
				last_obj = obj;
			}
		}
		SetStatus(STATUS_OTCENDLOOP, NULL);
	}
	SetStatus(STATUS_OTCAFTERLOOP, NULL);
}

void old_obj_store_to_char(struct char_data* ch, struct old_obj_file_u* st)
{
    struct obj_data* obj;
    struct obj_data* in_obj[64],*last_obj = NULL;
    int tmp_cur_depth=0;
    int i, j, iRealObjNumber;

    void obj_to_char(struct obj_data *object, struct char_data *ch);

    for(i = 0; i < 64; i++) {
        in_obj[ i ] = NULL;
    }

    SetStatus(STATUS_OTCBEFORELOOP, NULL);

    for(i=0; i<st->number; i++) {
        SetStatus(STATUS_OTCREALOBJECT, NULL);

        if(st->objects[i].item_number > 0 &&
            (iRealObjNumber = real_object(st->objects[i].item_number)) > -1) {
            SetStatus(STATUS_OTCREADOBJECT, NULL);
            if((obj = read_object(st->objects[i].item_number, VIRTUAL)) !=
                NULL) {
#if LIMITED_ITEMS
                /* Se l` oggetto costa al rent, e` considerato raro, e percio` viene
                 * gia` contato nella procedura CountLimitedItems. Questo dovrebbe
                 * risolvere il problema degli oggetti rari che non ripoppano come
                 * dovrebbero.
                 */
                if(obj->item_number >= 0 &&
                    obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
                    obj_index[ obj->item_number ].number--;
                }
#endif
                SetStatus(STATUS_OTCCOPYVALUE, NULL);

                obj->obj_flags.value[0] = st->objects[i].value[0];
                obj->obj_flags.value[1] = st->objects[i].value[1];
                obj->obj_flags.value[2] = st->objects[i].value[2];
                obj->obj_flags.value[3] = st->objects[i].value[3];
                obj->obj_flags.extra_flags  = st->objects[i].extra_flags;
                obj->obj_flags.extra_flags2 = 0;
                obj->obj_flags.weight       = st->objects[i].weight;
                obj->obj_flags.timer        = st->objects[i].timer;
                obj->obj_flags.bitvector    = st->objects[i].bitvector;

                SetStatus(STATUS_OTCFREESTRING, NULL);

                if(obj->name) {
                    free(obj->name);
                }
                if(obj->short_description) {
                    free(obj->short_description);
                }
                if(obj->description) {
                    free(obj->description);
                }

                SetStatus(STATUS_OTCALLOCSTRING, NULL);

                obj->name = (char*)malloc(strlen(st->objects[i].name)+1);
                obj->short_description = (char*)malloc(strlen(st->objects[i].sd)+1);
                obj->description = (char*)malloc(strlen(st->objects[i].desc)+1);

                SetStatus(STATUS_OTCCOPYSTRING, NULL);

                strcpy(obj->name, st->objects[i].name);
                strcpy(obj->short_description, st->objects[i].sd);
                strcpy(obj->description, st->objects[i].desc);

                if(st->objects[i].item_number >= LOW_EDITED_ITEMS && st->objects[i].item_number <= HIGH_EDITED_ITEMS && !pers_on(ch,obj) && !IS_OBJ_STAT2(obj, ITEM2_PERSONAL))
                {
                    pers_obj(ch, ch, obj, 1000);
                    if(!IS_OBJ_STAT2(obj, ITEM2_EDIT))
                    {
                        SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
                    }
                    if(!IS_OBJ_STAT2(obj, ITEM2_PERSONAL))
                    {
                        SET_BIT(obj->obj_flags.extra_flags2, ITEM2_PERSONAL);
                    }
                }

                SetStatus(STATUS_OTCCOPYAFFECT, NULL);

                for(j=0; j<MAX_OBJ_AFFECT; j++) {
                    obj->affected[j] = st->objects[i].affected[j];
                }

                SetStatus(STATUS_OTCBAGTREE, NULL);

                /* item restoring */
                if(st->objects[i].depth > 60) {
                    mudlog(LOG_SYSERR, "weird! object have depth >60.");
                    st->objects[i].depth = 0;
                }
                if(st->objects[i].depth && st->objects[i].wearpos) {
                    mudlog(LOG_SYSERR,
                            "weird! object (%s) weared and in cointainer.",obj->name);
                    st->objects[i].depth = st->objects[i].wearpos=0;
                }
                if(st->objects[i].depth > tmp_cur_depth) {
                    if(st->objects[i].depth != tmp_cur_depth + 1) {
                        mudlog(LOG_SYSERR, "weird! object depth changed from %d to %d",
                                tmp_cur_depth, st->objects[i].depth);
                    }
                    if(last_obj == NULL) {
                        mudlog(LOG_SYSERR,
                                "weird! object depth > current depth but not last_obj in "
                                "old_obj_store_to_char (reception.c).");
                    }

                    in_obj[ tmp_cur_depth++ ] = last_obj;
                }
                else if(st->objects[i].depth<tmp_cur_depth) {
                    tmp_cur_depth--;
                }
                if(st->objects[ i ].wearpos) {
                    equip_char(ch, obj, st->objects[ i ].wearpos - 1);
                }
                else if(tmp_cur_depth) {
                    if(in_obj[ tmp_cur_depth - 1 ]) {
                        obj_to_obj(obj, in_obj[ tmp_cur_depth - 1 ]);
                    }
                    else
                        mudlog(LOG_SYSERR,
                                "weird! in_obj[ tmp_cur_depth - 1 ] == NULL in "
                                "old_obj_store_to_char (reception.c).");
                }
                else {
                    obj_to_char(obj, ch);
                }
                last_obj = obj;
            }
        }
        SetStatus(STATUS_OTCENDLOOP, NULL);
    }
    SetStatus(STATUS_OTCAFTERLOOP, NULL);
}

// date  +%s --date='TZ="Europe/Rome" 00:00 today'
#define MUD_REBOOT_TIME 1516316400
void load_char_objs(struct char_data* ch) {
	FILE* fl;
	bool found = FALSE;
	long timegold;
	struct obj_file_u st;
    struct old_obj_file_u old_st;
	char tbuf[200];
	/*
	 * load in aliases and poofs first
	 */

	snprintf(tbuf,199,"Reading %s.aux",GET_NAME(ch));
	SetStatus(tbuf);
	load_char_extra(ch);

	snprintf(tbuf,199,"Reading %s.aux done",GET_NAME(ch));
	SetStatus(tbuf);


	sprintf(tbuf, "%s/%s", RENT_DIR, lower(ch->player.name));


	/* r+b is for Binary Reading/Writing */
	if(!(fl = fopen(tbuf, "r+b"))) {
		mudlog(LOG_PLAYERS, "Char has no equipment");
		return;
	}

	rewind(fl);

    if(IS_SET(ch->specials.act,PLR_NEW_EQ))
    {
        if(!ReadObjs(fl, &st))
        {
            mudlog(LOG_PLAYERS, "No objects found");
            snprintf(tbuf,199,"File %s.aux chiuso",GET_NAME(ch));
            SetStatus(tbuf);
            return;
        }
    }
    else
    {
        if(!ReadObjsOld(fl, &old_st))
        {
            mudlog(LOG_PLAYERS, "No objects found");
            snprintf(tbuf,199,"File %s.aux chiuso",GET_NAME(ch));
            SetStatus(tbuf);
            return;
        }
        
        old_st_to_st(&old_st, &st);
    }

	if(str_cmp(st.owner, GET_NAME(ch)) != 0) {
		mudlog(LOG_ERROR,
			   "Bad item-file write. %s is losing his/her objects",GET_NAME(ch));
		fclose(fl);
		return;
	}
	/* Nebbie was stopped for 2 years, here we make sure people are not naked.
	 * We fixed a mud epoch :)
	 */
	st.last_update=st.last_update > MUD_REBOOT_TIME?st.last_update:MUD_REBOOT_TIME;

	/*
	 * if the character has been out for 12 real hours, they are fully healed
	 * upon re-entry.  if they stay out for 24 full hours, all affects are
	 * removed, including bad ones.
	 */
	mudlog(LOG_CHECK, "  Last update %ld, time %ld.", st.last_update,
		   time(0));

	if(st.last_update + 12 * SECS_PER_REAL_HOUR < time(0)) {
		RestoreChar(ch);
	}

	if(st.last_update + 24 * SECS_PER_REAL_HOUR < time(0))
		//mudlog( LOG_CHECK, "aa");
	{
		RemAllAffects(ch);
	}

	if(ch->in_room == NOWHERE &&
			st.last_update + 1*SECS_PER_REAL_HOUR > time(0)) {
		/* you made it back from the crash in time, 1 hour grace period. */
		mudlog(LOG_PLAYERS, "%s reconnecting.", GET_NAME(ch));
		found = TRUE;
	}
	else {
		char buf[ 81 ];
		if(ch->in_room == NOWHERE) {
			mudlog(LOG_PLAYERS, "%s reconnecting after autorent", GET_NAME(ch));
		}

#if NEW_RENT
		timegold = long(0 * (double(time(0) - st.last_update) /
							 SECS_PER_REAL_DAY));
#else
		timegold = long(st.total_cost * (double(time(0) - st.last_update) /
										 SECS_PER_REAL_DAY));
#endif

		mudlog(LOG_PLAYERS, "Char ran up charges of %ld gold in rent", timegold);
		sprintf(buf, "Il conto della pensione e` di %ld monete.\n\r", timegold);
		send_to_char(buf, ch);
		sprintf(buf, "%d monete al giorno.\n\r", st.total_cost);   // Gaia 2001
		send_to_char(buf, ch); // Gaia 2001
		GET_GOLD(ch) -= timegold;
		found = TRUE;
		/* inizia modifica Robin hood Gaia 2001*/
		constexpr int mega=1000000;
		bool robin=false;
		if(GET_GOLD(ch) <= 3*mega)  {
			robin=(number(1,100)<2);
		}
		else  if(GET_GOLD(ch) <=  4 * mega) {
			robin=(number(1,100)<2);
		}
		else if(GET_GOLD(ch) <= 8 * mega) {
			robin=(number(1,100)<5);
		}
		else if(GET_GOLD(ch) <= 10 * mega) {
			robin=(number(1,100)<31);
		}
		else if(GET_GOLD(ch) <= 50 * mega) {
			robin=(number(1,100)<41);
		}
		else if(GET_GOLD(ch) <=  100 * mega) {
			robin=(number(1,100)<51);
		}
		else if(GET_GOLD(ch) <=  100 * mega) {
			robin=(number(1,100)<51);
		}
		else if(GET_GOLD(ch) <=  110 * mega) {
			robin=(number(1,100)<61);
		}
		else if(GET_GOLD(ch) <=  120 * mega) {
			robin=(number(1,100)<71);
		}
		else if(GET_GOLD(ch) <=  150 * mega) {
			robin=(number(1,100)<81);
		}
		else if(GET_GOLD(ch) <=  999 * mega) {
			robin=(number(1,100)<91);
		}
		else  {
			robin=(number(1,100)<100);
		}
		if(robin) {
			mudlog(LOG_PLAYERS, "Robin ruba a %s %d ", GET_NAME(ch),GET_GOLD(ch));
			GET_GOLD(ch)/=2;
			send_to_char("Mentre stavi riposando qualcuno si e' introdotto nella tua stanza  e ha rubato meta` dei tuoi soldi!!!\n\rForse farti vedere con tutto quel denaro addosso non e' stata una buona idea...\n\r",
						 ch);
		}
		/* termine modifica Robin hood */
		if(GET_GOLD(ch) < -5000) {
			mudlog(LOG_PLAYERS, "** %s badly ran out of money in rent **",
				   GET_NAME(ch));
			send_to_char("$c0011Hai finito i soldi. "
						 "La tua roba e` stata venduta\n\r", ch);
			GET_GOLD(ch) = 0;
			found = FALSE;
		}

		if(GET_GOLD(ch) < 0) {
			mudlog(LOG_PLAYERS, "** %s ran out of money in rent **",
				   GET_NAME(ch));
			send_to_char("$c0011Occhio, amico... hai finito i soldi. "
						 "O ti dai da fare... o resti nudo\n\r", ch);
			found = TRUE;
		}
	}

	fclose(fl);

	if(found) {
		mudlog(LOG_CHECK, "Reading objects...");
        if(IS_SET(ch->specials.act,PLR_NEW_EQ))
        {
            mudlog(LOG_CHECK,"New Obj on %s",GET_NAME(ch));
            obj_store_to_char(ch, &st);
        }
        else
        {
            mudlog(LOG_CHECK,"Old Obj on %s",GET_NAME(ch));
            old_obj_store_to_char(ch, &old_st);
        }
#if LIMITEEQALRIENTRO
		limited_items_carrying =CountLims(ch->carrying);
		if(limited_items_carrying && !IS_IMMORTALE(ch)) {
			send_to_char("$c0011Ci hai provato, ma ti avevo detto che non potevi portare con te oggetti\n\r rari oltre a quelli indossati... ora li hai persi!!!",ch);
			mudlog(LOG_PLAYERS,"Rimozione %d oggetti limited in inventario al rientro di %s.",
				   limited_items_carrying,GET_NAME(ch));
			for(tmp = ch->carrying; tmp; tmp = tmp_next_obj) {
				tmp_next_obj = tmp->next_content;
				if(tmp->obj_flags.cost >= LIM_ITEM_COST_MIN && limited_items_carrying) {
					obj_from_char(tmp);
					extract_obj(tmp); /* could just drop it in that room... */
					limited_items_carrying--;
				}
			}
		}
#endif
	}
	else {
		mudlog(LOG_CHECK, "Zeroing objects...");
		ZeroRent(GET_NAME(ch));
	}

	/* Save char, to avoid strange data if crashing */
	mudlog(LOG_CHECK, "Saving character...");
	save_char(ch, AUTO_RENT, 0);
	AverageEqIndex(GetCharBonusIndex(ch));
}


/* ************************************************************************
 * Routines used to save a characters equipment from disk                  *
 ************************************************************************* */

/* Puts object in store, at first item which has no -1 */
void put_obj_in_store(struct obj_data* obj, struct obj_file_u* st) {
	int j;
	struct obj_file_elem* oe;

	if(st->number >= MAX_OBJ_SAVE) {
		mudlog(LOG_CHECK, "Trying to rent more than %d items. Ignored.",
			   st->number);
		return;
	}

	oe = st->objects + st->number;

	oe->item_number = obj->item_number >= 0 ?
					  obj_index[obj->item_number].iVNum : 0;
	oe->value[0] = obj->obj_flags.value[0];
	oe->value[1] = obj->obj_flags.value[1];
	oe->value[2] = obj->obj_flags.value[2];
	oe->value[3] = obj->obj_flags.value[3];

	oe->extra_flags = obj->obj_flags.extra_flags;
    oe->extra_flags2 = obj->obj_flags.extra_flags2;
	oe->weight  = obj->obj_flags.weight;
	oe->timer  = obj->obj_flags.timer;
	oe->bitvector  = obj->obj_flags.bitvector;

	if(obj->name) {
		strcpy(oe->name, obj->name);
	}
	else {
		mudlog(LOG_SYSERR, "object %d has no name!",
			   (obj->item_number >= 0 ? obj_index[obj->item_number].iVNum : 0));
		*oe->name = '\0';
	}

	if(obj->short_description) {
		strcpy(oe->sd, obj->short_description);
	}
	else {
		mudlog(LOG_SYSERR, "object %d has no short description!",
			   (obj->item_number >= 0 ? obj_index[obj->item_number].iVNum : 0));
		*oe->sd = '\0';
	}
	if(obj->description) {
		strcpy(oe->desc, obj->description);
	}
	else {
		*oe->desc = '\0';
	}

	for(j = 0; j < MAX_OBJ_AFFECT; j++) {
		oe->affected[j] = obj->affected[j];
	}

	oe->depth = cur_depth;
	st->number++;
}

int contained_weight(struct obj_data* container) {
	struct obj_data* tmp;
	int   rval = 0;

	for(tmp = container->contains; tmp; tmp = tmp->next_content) {
		rval += GET_OBJ_WEIGHT(tmp);
	}
	return rval;
}

/* Destroy inventory after transferring it to "store inventory" */
#if 1
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
				  struct char_data* ch, int bDelete) {
	if(!obj) {
		return;
	}

	if(obj->obj_flags.timer < 0 && obj->obj_flags.timer != OBJ_NOTIMER) {
#if NODUPLICATES
#else
		if(ch != NULL) {
			sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for "
					"you.'\n\r", obj->short_description);
			send_to_char(buf, ch);
		}
#endif
	}
	else if(obj->obj_flags.cost_per_day < 0) {

#if NODUPLICATES
#else
		if(ch != NULL) {
			sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for "
					"you.'\n\r", obj->short_description);
			send_to_char(buf, ch);
		}
#endif
	}
	else if(obj->item_number != -1) {
		int weight = contained_weight(obj);

		GET_OBJ_WEIGHT(obj) -= weight;
		put_obj_in_store(obj, st);
		GET_OBJ_WEIGHT(obj) += weight;
	}

	if(obj->contains) {
		cur_depth++;
		obj_to_store(obj->contains, st, ch, bDelete);
		cur_depth--;
	}
	obj_to_store(obj->next_content, st, ch, bDelete);

	/* and now we can destroy object */
	if(bDelete) {
		if(obj->in_obj) {
			obj_from_obj(obj);
		}
#if LIMITED_ITEMS
		/* Se lo oggetto e` raro, non ne deve essere decrementato il numero
		 * presente nel mondo. Questo dovrebbe risolvere il problema di alcuni
		 * oggetti rari che non ripoppano.
		 */
		if(obj->item_number >= 0 &&
				obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
			obj_index[ obj->item_number ].number++;
		}
#endif
		extract_obj(obj);
	}
}
#else
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
				  struct char_data* ch, int bDelete) {
	static char buf[240];

	if(!obj) {
		return;
	}

	obj_to_store(obj->contains, st, ch, bDelete);
	obj_to_store(obj->next_content, st, ch, bDelete);

	if((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) {
#if NODUPLICATES
#else
		sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for you.'\n\r", obj->short_description);
		send_to_char(buf, ch);
#endif
	}
	else if(obj->obj_flags.cost_per_day < 0) {

#if NODUPLICATES
#else
		if(ch != NULL) {
			sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for you.'\n\r", obj->short_description);
			send_to_char(buf, ch);
		}
#endif

		if(bDelete) {
			if(obj->in_obj) {
				obj_from_obj(obj);
			}
			extract_obj(obj);
		}
	}
	else if(obj->item_number == -1) {
		if(bDelete) {
			if(obj->in_obj) {
				obj_from_obj(obj);
			}
			extract_obj(obj);
		}
	}
	else {
		int weight = contained_weight(obj);
		GET_OBJ_WEIGHT(obj) -= weight;
		put_obj_in_store(obj, st);
		GET_OBJ_WEIGHT(obj) += weight;
		if(bDelete) {
			if(obj->in_obj) {
				obj_from_obj(obj);
			}
			extract_obj(obj);
		}
	}
}


#endif



/* write the vital data of a player to the player file */
void save_obj(struct char_data* ch, struct obj_cost* cost, int bDelete) {
	static struct obj_file_u st;
	int i;

	st.number = 0;
	st.gold_left = GET_GOLD(ch);

	st.total_cost = cost->total_cost;
	st.last_update = time(0);
	st.minimum_stay = 0; /* XXX where does this belong? */
	mudlog(LOG_PLAYERS, "save_obj: %s",GET_NAME(ch));

	cur_depth=0;

	for(i=0; i<MAX_OBJ_SAVE; i++) {
		st.objects[i].wearpos=0;
		st.objects[i].depth=0;
	}

	for(i = 0; i < MAX_WEAR; i++) {
		if(ch->equipment[ i ]) {
			st.objects[ st.number ].wearpos = i + 1;
			if(bDelete) {
				obj_to_store(unequip_char(ch, i), &st, ch, bDelete);
			}
			else {
				obj_to_store(ch->equipment[i], &st, ch, bDelete);
			}
		}
	}

	obj_to_store(ch->carrying, &st, ch, bDelete);
	if(bDelete) {
		ch->carrying = 0;
	}

	mudlog(LOG_PLAYERS, "Saving %d objects of %s:%d",
		   st.number, GET_NAME(ch), GET_GOLD(ch));

	update_file(ch, &st);
	mudlog(LOG_PLAYERS, "Saved  %d objects of %s:%d",
		   st.number, GET_NAME(ch), GET_GOLD(ch));
}



/* ************************************************************************
* Routines used to update object file, upon boot time                     *
************************************************************************* */

void update_obj_file() {
    struct obj_file_u st;
    struct old_obj_file_u old_st;
    struct char_file_u ch_st;
    long days_passed;
    bool ok = FALSE;

    FILE* pObjFile;
    DIR* dir;

    if((dir = opendir(PLAYERS_DIR)) != NULL)
    {
        struct dirent* ent;
        while((ent = readdir(dir)) != NULL)
        {
            FILE* pCharFile;
            char szFileName[ 40 ];

            if(*ent->d_name == '.')
            {
                continue;
            }
            if(!strstr(ent->d_name,".dat"))
            {
                continue;
            }

            sprintf(szFileName, "%s/%s", PLAYERS_DIR, ent->d_name);

            if((pCharFile = fopen(szFileName, "r+")) != NULL)
            {
                if(fread(&ch_st, 1, sizeof(ch_st), pCharFile) == sizeof(ch_st))
                {
                    sprintf(szFileName, "%s/%s", RENT_DIR, lower(ch_st.name));
                    /* r+b is for Binary Reading/Writing */
                    if((pObjFile = fopen(szFileName, "r+b")) != NULL)
                    {
                        if(!IS_SET(ch_st.act,PLR_NEW_EQ))
                        {
                            mudlog(LOG_CHECK,"Provo a leggere il vecchio EQ per %s",ch_st.name);
                            if(ReadObjsOld(pObjFile, &old_st))
                            {
                                ok = TRUE;
                                old_st_to_st(&old_st, &st);
                                SET_BIT(ch_st.act,PLR_NEW_EQ);
                            }
                        }
                        else
                        {
                            mudlog(LOG_CHECK,"Provo a leggere il nuovo EQ per %s",ch_st.name);
                            if(ReadObjs(pObjFile, &st))
                            {
                                ok = TRUE;
                            }
                        }
                        
                        if(ok)
                        {
                            if(str_cmp(st.owner, ch_st.name) == 0)
                            {
                                mudlog(LOG_SAVE, " Processing %s.", st.owner);
                                days_passed = ((time(0) - st.last_update) / SECS_PER_REAL_DAY);

                                if(ch_st.load_room == AUTO_RENT)
                                {
                                    /* this person was autorented */
                                    ch_st.load_room = NOWHERE;
                                    st.last_update = time(0)+3600;  /* one hour grace period */

                                    mudlog(LOG_SAVE, "   Deautorenting %s", st.owner);

#if LIMITED_ITEMS
                                    CountLimitedItems(&st);
#endif
                                    rewind(pCharFile);
                                    fwrite(&ch_st, sizeof(ch_st), 1, pCharFile);

                                    rewind(pObjFile);
                                    WriteObjs(pObjFile, &st);

                                    fclose(pObjFile);
                                }
                                else
                                { /* if( ch_st.load_room == AUTO_RENT ) */
                                    if(days_passed > 0)
                                    {
#if NEW_RENT
                                        /* RENTAL COST ADJUSTMENT */
                                        st.total_cost = 0;
#endif
                                        if((st.total_cost * days_passed) > st.gold_left)
                                        {
                                            mudlog(LOG_PLAYERS, "Dumping %s from object file.", ch_st.name);
                                            mudlog(LOG_PLAYERS,"  Total cost: %d Days %ld Gold left %d", st.total_cost, days_passed, st.gold_left);
                                            ch_st.points.gold = 0;
                                            ch_st.load_room = NOWHERE;

                                            rewind(pCharFile);
                                            fwrite(&ch_st, sizeof(ch_st), 1, pCharFile);

                                            fclose(pObjFile);
                                            ZeroRent(ch_st.name);

                                        }
                                        else
                                        {
                                            mudlog(LOG_SAVE, "   Updating %s", st.owner);
#if 0
                                            st.gold_left -= st.total_cost * days_passed;
                                            st.last_update = time(0) - secs_lost;
#endif
                                            rewind(pObjFile);
                                            WriteObjs(pObjFile, &st);
                                            fclose(pObjFile);
#if LIMITED_ITEMS
                                            CountLimitedItems(&st);
#endif
                                        }
                                    }
                                    else
                                    {

#if LIMITED_ITEMS
                                        CountLimitedItems(&st);
#endif
                                        mudlog(LOG_CHECK, "   same day update on %s", st.owner);

                                        rewind(pObjFile);
                                        WriteObjs(pObjFile, &st);
                                        fclose(pObjFile);
                                    }
                                } /* if( ch_st.load_room == AUTO_RENT ) else */
                            }
                            else
                            {
                                mudlog(LOG_SYSERR, "Wrong person written into object file! (%s/%s)", st.owner, ch_st.name);
                                assert(0);
                            }
                        } /* Il personaggio non ha oggetti. */
                    } /* Non esiste il file degli oggetti. */
                }
                else
                {
                    mudlog(LOG_ERROR, "Error reading file %s. %d", szFileName, sizeof(ch_st));
                }
                fclose(pCharFile);
            }
            else
            {
                mudlog(LOG_ERROR, "Errore opening file %s.", szFileName);
            }
        } /* Fine dei giocatori */
    }
    else
    {
        mudlog(LOG_ERROR, "Cannot open dir %s.", PLAYERS_DIR);
    }
}

void old_st_to_st(struct old_obj_file_u* old_st, struct obj_file_u* st)
{
    int i, j, iRealObjNumber;

    strcpy(st->owner, old_st->owner);
    st->gold_left       = old_st->gold_left;
    st->total_cost      = old_st->total_cost;
    st->last_update     = old_st->last_update;
    st->minimum_stay    = old_st->minimum_stay;
    st->number          = old_st->number;

    for(i = 0; i < old_st->number; i++)
    {
        if(old_st->objects[i].item_number > 0 &&
           (iRealObjNumber = real_object(old_st->objects[i].item_number)) > -1)
        {
            st->objects[i].item_number   = old_st->objects[i].item_number;
            st->objects[i].value[0]      = old_st->objects[i].value[0];
            st->objects[i].value[1]      = old_st->objects[i].value[1];
            st->objects[i].value[2]      = old_st->objects[i].value[2];
            st->objects[i].value[3]      = old_st->objects[i].value[3];
            st->objects[i].value[4]      = old_st->objects[i].value[4];
            st->objects[i].extra_flags   = old_st->objects[i].extra_flags;
            st->objects[i].extra_flags2  = 0;
            st->objects[i].weight        = old_st->objects[i].weight;
            st->objects[i].timer         = old_st->objects[i].timer;
            st->objects[i].bitvector     = old_st->objects[i].bitvector;
            
            strcpy(st->objects[i].name, old_st->objects[ i ].name);
            strcpy(st->objects[i].sd, old_st->objects[ i ].sd);
            strcpy(st->objects[i].desc, old_st->objects[ i ].desc);

            st->objects[i].wearpos       = old_st->objects[i].wearpos;
            st->objects[i].depth         = old_st->objects[i].depth;

            for(j=0; j<MAX_OBJ_AFFECT; j++)
            {
                st->objects[i].affected[j] = old_st->objects[i].affected[j];
            }
        }
    }
}

void CountLimitedItems(struct obj_file_u* st) {
	int i;
	struct obj_data* obj;
	char buf[MAX_STRING_LENGTH];

	if(!st->owner[0]) {
		return;    /* don't count empty rent units */
	}

	for(i = 0; i < st->number; i++) {
		if(st->objects[ i ].item_number > 0 &&
				real_object(st->objects[ i ].item_number) > -1) {
			/* eek.. read in the object, and then extract it.
			 (all this just to find rent cost.)  *sigh* */
			if((obj = read_object(st->objects[ i ].item_number, VIRTUAL))) {
				/* if the cost is >= LIM_ITEM_COST_MIN, then mark before extractin */
				if(obj->item_number >= 0 &&
						obj->obj_flags.cost >= LIM_ITEM_COST_MIN) {
					obj_index[ obj->item_number ].number++;

					/*Acidus 2004-show rare*/
					sprintf(buf, "  %5d %s %s\n\r", obj->item_number,obj->name,st->owner);
					strncat(rarelist, " ",MAX_STRING_LENGTH);
					strncat(rarelist, buf,MAX_STRING_LENGTH);

				}
				extract_obj(obj);
			}
		}
	}
}


void PrintLimitedItems() {
	int i;
	mudlog(LOG_CHECK, "Rare item rented:");
	mudlog(LOG_CHECK, "  VNUM  Amount  Name");
	for(i = 0; i < top_of_objt; i++) {
		if(obj_index[ i ].number > 0) {
#if NICE_LIMITED
			obj_index[i].number/=2;
#endif

			mudlog(LOG_SAVE, "  %5d [%5d] %s", obj_index[ i ].iVNum,
				   obj_index[i].number,
				   obj_index[i].name);
		}
	}
}


/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */



int receptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) {
	struct obj_cost cost;
	struct char_data* recep = 0;
	struct char_data* temp_char;
	int save_room;
	sh_int action_tabel[9];
	unsigned long giorni = 0;

	if(!ch->desc) {
		return(FALSE);    /* You've forgot FALSE - NPC couldn't leave */
	}

	action_tabel[0] = 23;
	action_tabel[1] = 24;
	action_tabel[2] = 36;
	action_tabel[3] = 105;
	action_tabel[4] = 106;
	action_tabel[5] = 109;
	action_tabel[6] = 111;
	action_tabel[7] = 142;
	action_tabel[8] = 147;


	for(temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep);
			temp_char = temp_char->next_in_room)
		if(IS_MOB(temp_char))
			if(mob_index[temp_char->nr].func == reinterpret_cast<genericspecial_func>(receptionist)) {
				recep = temp_char;
			}

	if(!recep) {
		mudlog(LOG_SYSERR, "No_receptionist.\n\r");
		assert(0);
	}

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	if((cmd != CMD_RENT) && (cmd != CMD_OFFER) && (cmd != CMD_STORE)) {
		if(!cmd) {
			if(recep->specials.fighting) {
				return(citizen(recep,0,"",mob,type));
			}
		}
		if(!number(0, 30)) {
			do_action(recep, "", action_tabel[number(0,8)]);
		}
		return(FALSE);
	}

	if(!AWAKE(recep)) {
		act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
		return(TRUE);
	}

	if(!CAN_SEE(recep, ch)) {
		act("$n says, 'I just can't deal with people I can't see!'", FALSE,
			recep, 0, 0, TO_ROOM);
		act("$n bursts into tears", FALSE, recep, 0, 0, TO_ROOM);
		return(TRUE);
	}

	if(cmd == CMD_RENT) {
		/* Rent  */
		PushStatus("Command rent");
		DontShow=1;
		if(recep_offer(ch, recep, &cost,0)) {

			act("$n mette le tue cose nella cassaforte e ti accompagna in stanza.",
				FALSE, recep, 0, ch, TO_VICT);
			act("$n accompagna $N nella sua stanza.",FALSE, recep,0,ch,TO_NOTVICT);

			save_obj(ch, &cost,1);
			save_room = ch->in_room;

			if(ch->specials.start_room != 2) { /* hell */
				ch->specials.start_room = save_room;
			}

			extract_char(ch);
			/* you don`t delete CHARACTERS when you extract them */
			save_char(ch, save_room, 0);
			ch->in_room = save_room;

		}

	}
	else if(cmd == CMD_OFFER) {
		/* Offer */
		PushStatus("Command offer");
		DontShow=0;
		recep_offer(ch, recep, &cost,1);
		act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
	}
	else if(cmd==CMD_STORE) {
		PushStatus("Command store");
		if(!arg || !*arg) {
			giorni=0;
		}
		else {
			giorni=atoi(arg);
		}
		if(giorni<2) {
			act("$N ti dice 'Per quanti giorni?!?'",FALSE,ch,0,recep,TO_CHAR);
			PopStatus();
			return(TRUE);
		}
		DontShow=1;
		if(recep_offer(ch, recep, &cost,giorni)) {
			act("$n mette le tue cose nella cassaforte e ti accompagna in stanza.",
				FALSE, recep, 0, ch, TO_VICT);
			act("$n accompagna $N nella sua stanza.",FALSE, recep,0,ch,TO_NOTVICT);

			save_obj(ch, &cost,1);
			save_room = ch->in_room;

			if(ch->specials.start_room != 2) { /* hell */
				ch->specials.start_room = save_room;
			}

			extract_char(ch);  /* you don't delete CHARACTERS when you extract
them */
			save_char(ch, save_room,giorni);
			ch->in_room = save_room;

		}


	}
	PopStatus();
	return(TRUE);
}

/* Procedura per le receptionist dei castelli *** Flyp 20020214 *** */
int creceptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) {
	struct obj_cost cost;
	struct char_data* recep = 0;
	struct char_data* temp_char;
	int save_room;
	sh_int action_tabel[9];
	unsigned long giorni = 0;

	if(!ch->desc) {
		return(FALSE);    /* You've forgot FALSE - NPC couldn't leave */
	}

	action_tabel[0] = 23;
	action_tabel[1] = 24;
	action_tabel[2] = 36;
	action_tabel[3] = 105;
	action_tabel[4] = 106;
	action_tabel[5] = 109;
	action_tabel[6] = 111;
	action_tabel[7] = 142;
	action_tabel[8] = 147;


	for(temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep);
			temp_char = temp_char->next_in_room)
		if(IS_MOB(temp_char))
			if(mob_index[temp_char->nr].func == reinterpret_cast<genericspecial_func>(creceptionist)) {
				recep = temp_char;
			}

	if(!recep) {
		mudlog(LOG_SYSERR, "No_receptionist.\n\r");
		assert(0);
	}

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	if((cmd != CMD_RENT) && (cmd != CMD_OFFER) && (cmd != CMD_STORE)) {
		if(!cmd) {
			if(recep->specials.fighting) {
				return(citizen(recep,0,"",mob,type));
			}
		}
		if(!number(0, 30)) {
			do_action(recep, "", action_tabel[number(0,8)]);
		}
		return(FALSE);
	}

	if(!AWAKE(recep)) {
		act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
		return(TRUE);
	}

	if(!CAN_SEE(recep, ch)) {
		act("$n says, 'I just can't deal with people I can't see!'", FALSE,
			recep, 0, 0, TO_ROOM);
		act("$n bursts into tears", FALSE, recep, 0, 0, TO_ROOM);
		return(TRUE);
	}

	if(cmd == CMD_RENT) {
		/* Rent  */
		PushStatus("Command rent");
		DontShow=1;

		if(recep_offer(ch, recep, &cost,0)) {

			if(GetMaxLevel(ch)==PRINCIPE) {
				act("$n ti dice 'Buon riposo mio Principe'", FALSE, recep, 0, ch,TO_VICT);
			}
			else {
				act("$n ti dice 'Buon riposo!'", FALSE, recep, 0, ch, TO_VICT);
			}

			act("$n mette le tue cose nella cassaforte e ti accompagna in stanza.",
				FALSE, recep, 0, ch, TO_VICT);
			act("$n accompagna $N nella sua stanza.",FALSE, recep,0,ch,TO_NOTVICT);


			save_obj(ch, &cost,1);
			save_room = ch->in_room;

			if(ch->specials.start_room != 2) { /* hell */
				ch->specials.start_room = save_room;
			}

			extract_char(ch);
			/* you don`t delete CHARACTERS when you extract them */
			save_char(ch, save_room, 0);
			ch->in_room = save_room;

		}

	}
	else if(cmd == CMD_OFFER) {
		/* Offer */
		PushStatus("Command offer");
		DontShow=0;
		recep_offer(ch, recep, &cost,1);
		act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
	}
	else if(cmd==CMD_STORE) {
		PushStatus("Command store");
		if(!arg || !*arg) {
			giorni=0;
		}
		else {
			giorni=atoi(arg);
		}
		if(giorni<2) {
			act("$N ti dice 'Per quanti giorni?!?'",FALSE,ch,0,recep,TO_CHAR);
			PopStatus();
			return(TRUE);
		}
		DontShow=1;
		if(recep_offer(ch, recep, &cost,giorni)) {
			act("$n mette le tue cose nella cassaforte e ti accompagna in stanza.",
				FALSE, recep, 0, ch, TO_VICT);
			act("$n accompagna $N nella sua stanza.",FALSE, recep,0,ch,TO_NOTVICT);

			save_obj(ch, &cost,1);
			save_room = ch->in_room;

			if(ch->specials.start_room != 2) { /* hell */
				ch->specials.start_room = save_room;
			}

			extract_char(ch);  /* you don't delete CHARACTERS when you extract
them */
			save_char(ch, save_room,giorni);
			ch->in_room = save_room;

		}


	}
	PopStatus();
	return(TRUE);
}

/*
    removes a player from the list of renters
*/

void zero_rent(struct char_data* ch) {

	if(IS_NPC(ch)) {
		return;
	}

	ZeroRent(GET_NAME(ch));

}

void ZeroRent(char* n) {
	FILE* fl;
	char buf[200];

	sprintf(buf, "%s/%s", RENT_DIR, lower(n));

	if(!(fl = fopen(buf, "w"))) {
		mudlog(LOG_ERROR,"%s:%s","saving PC's objects",strerror(errno));
		assert(0);
	}

	fclose(fl);
	return;

}

int ReadObjs(FILE* fl, struct obj_file_u* st) {
	int i;

	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}

	fread(st->owner, sizeof(st->owner), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	fread(&st->last_update, sizeof(st->last_update), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	fread(&st->number, sizeof(st->number), 1, fl);
	if(feof(fl)) {
		fclose(fl);
		return(FALSE);
	}
	mudlog(LOG_SAVE,"Letto %s %d %d %d %d %d",st->owner,
		   st->gold_left,st->total_cost,st->last_update,st->minimum_stay,st->number);
	for(i=0; i<st->number; i++) {
        mudlog(LOG_SAVE,"Leggo_new %s: %d, %s",st->owner, i, st->objects[i].name);
		fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
	}
	return TRUE;
}

int ReadObjsOld(FILE* fl, struct old_obj_file_u* st) {
    int i;

    if(feof(fl))
    {
        fclose(fl);
        return(FALSE);
    }

    fread(st->owner, sizeof(st->owner), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    fread(&st->last_update, sizeof(st->last_update), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    fread(&st->number, sizeof(st->number), 1, fl);
    if(feof(fl)) {
        fclose(fl);
        return(FALSE);
    }
    mudlog(LOG_SAVE,"Letto %s %d %d %d %d %d",st->owner, st->gold_left,st->total_cost,st->last_update,st->minimum_stay,st->number);
    for(i=0; i<st->number; i++) {
            fread(&st->objects[i], sizeof(struct old_obj_file_elem), 1, fl);
            mudlog(LOG_SAVE,"Leggo_old %s: %d, %s",st->owner, i, st->objects[i].name);
    }
    return TRUE;
}

void WriteObjs(FILE* fl, struct obj_file_u* st) {
	int i;
	PushStatus("WriteObjs");
	fwrite(st->owner, sizeof(st->owner), 1, fl);
	fwrite(&st->gold_left, sizeof(st->gold_left), 1, fl);
#if NEW_RENT
	/* RENTAL COST ADJUSTMENT */
	st->total_cost = 0;
#endif
	fwrite(&st->total_cost, sizeof(st->total_cost), 1, fl);
	fwrite(&st->last_update, sizeof(st->last_update), 1, fl);
	fwrite(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
	fwrite(&st->number, sizeof(st->number), 1, fl);
	for(i=0; i<st->number; i++) {
        fwrite(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
        mudlog(LOG_SAVE,"Scrivo %s: %d, %s",st->owner, i, st->objects[i].name);
	}
	PopStatus();
}


void load_char_extra(struct char_data* ch) {
	FILE* fp;
	char buf[80];
	char line[260];
	char  tmp[260];
	char* p, *s, *chk;
	int n;
	/* Cerca prima il file col nome in lower case */
	sprintf(buf, "%s/%s.aux", RENT_DIR, lower(GET_NAME(ch)));
	if((fp = fopen(buf, "r")) == NULL) {
		/* Ok, proviamo col nome non trasformato */
		sprintf(buf, "%s/%s.aux", RENT_DIR, GET_NAME(ch));
		if((fp = fopen(buf, "r")) == NULL) {
			return;    /* nothing to look at */
		}
	}
	/*
	* open the file.. read in the lines, use them as the aliases and
	* poofin and outs, depending on tags:
	*
	* format:
	*
	* <id>:string
	*
	*/

	PushStatus("load char extra");
	while(!feof(fp)) {
		chk = fgets(line, 260, fp);
		if(chk) {
			p = (char*)strtok(line, ":");
			s = (char*)strtok(0, "\0");
			if(p) {
				if(!strcmp(p,"out")) {
					/*setup bamfout */
					do_bamfout(ch, s, 0);
				}
				else if(!strcmp(p, "in")) {
					/* setup bamfin */
					do_bamfin(ch, s, 0);
				}
				else if(!strcmp(p, "email")) {
					/* setup email */
					RECREATE(GET_EMAIL(ch),char,strlen(s));
					strcpy(GET_EMAIL(ch),replace(s,'\n','\0'));
				}
				else if(!strcmp(p, "realname")) {
					/* setup realname */
					RECREATE(GET_REALNAME(ch),char,strlen(s));
					strcpy(GET_REALNAME(ch),replace(s,'\n','\0'));
				}
				else if(!strcmp(p, "principe")) {
					/* setup realname */
					RECREATE(GET_PRINCE(ch),char,strlen(s));
					strcpy(GET_PRINCE(ch),replace(s,'\n','\0'));
				}
				else if(!strcmp(p, "version")) {
					/* setup realname */
					RECREATE(ch->specials.lastversion,char,strlen(s));
					strcpy(ch->specials.lastversion,replace(s,'\n','\0'));
				}
				else if(!strcmp(p, "zone")) {
					/* set zone permisions */
					GET_ZONE(ch) = atoi(s);
				}
				else if(!strcmp(p, "prompt")) {
					/* setup prompt */
					if(strchr(s, '\n') != 0) {
						*((char*)strchr(s,'\n')) = 0;
					}
					if(strchr(s, '\r') != 0) {
						*((char*)strchr(s, '\r')) = 0;
					}
					do_set_prompt(ch, s, 0);
				}
				else {
					if(s) {
						s[strlen(s)]= '\0';
						n = atoi(p);
						if(n >=0 && n <= 9) {
							/* set up alias */
							sprintf(tmp, "%d %s", n, s+1);
							do_alias(ch, replace(tmp,'\n','\0'), CMD_ALIAS);
						}
					}
				}
			}
		}
		else {
			break;
		}
	}
	fclose(fp);
	PopStatus();
}

void write_char_extra(struct char_data* ch) {
	FILE* fp;
	char buf[80];
	int i;
	PushStatus("write char extra");
	sprintf(buf, "%s/%s.aux", RENT_DIR, lower(GET_NAME(ch)));
	/*
	 * open the file.. read in the lines, use them as the aliases and
	 * poofin and outs, depending on tags:
	 *
	 * format:
	 *
	 * <id>:string
	 *
	 */

	if((fp = fopen(buf, "w")) == NULL) {
		return;  /* nothing to write */
	}

	if(IS_IMMORTAL(ch)) {
		if(ch->specials.poofin) {
			fprintf(fp, "in:%-s\n", ch->specials.poofin);
		}
		if(ch->specials.poofout) {
			fprintf(fp, "out:%-s\n", ch->specials.poofout);
		}
		fprintf(fp, "zone:%d\n", GET_ZONE(ch));
	}
	if(ch->specials.prompt) {
		fprintf(fp, "prompt:%s\n", ch->specials.prompt);
	}

	if(ch->specials.email) {
		fprintf(fp, "email:%s\n", ch->specials.email);
	}
	if(GET_PRINCE(ch)) {
		fprintf(fp, "principe:%s\n", GET_PRINCE(ch));
	}

	if(ch->specials.realname) {
		fprintf(fp, "realname:%s\n", ch->specials.realname);
	}

	fprintf(fp,"version:%s\n",version());

	if(ch->specials.A_list) {
		for(i=0; i<10; i++) {
			if(GET_ALIAS(ch, i)) {
				fprintf(fp, "%d: %s\n", i, replace(GET_ALIAS(ch, i),'\n','\0'));
			}
		}
	}
	fclose(fp);
	PopStatus();
}


void obj_store_to_room(int room, struct obj_file_u* st) {
	struct obj_data* obj = NULL;
	struct obj_data* in_obj[ 64 ];
	struct obj_data* last_obj = NULL;
	int tmp_cur_depth = 0;
	int i, j;


	for(i = 0; i < st->number; i++) {
		if(st->objects[ i ].item_number > 0 &&
				real_object(st->objects[i].item_number) > -1) {

			obj = read_object(st->objects[ i ].item_number, VIRTUAL);
			obj->obj_flags.value[ 0 ] = st->objects[ i ].value[ 0 ];
			obj->obj_flags.value[ 1 ] = st->objects[ i ].value[ 1 ];
			obj->obj_flags.value[ 2 ] = st->objects[ i ].value[ 2 ];
			obj->obj_flags.value[ 3 ] = st->objects[ i ].value[ 3 ];
			obj->obj_flags.extra_flags = st->objects[ i ].extra_flags;
            obj->obj_flags.extra_flags2 = st->objects[ i ].extra_flags2;
			obj->obj_flags.weight       = st->objects[ i ].weight;
			obj->obj_flags.timer        = st->objects[ i ].timer;
			obj->obj_flags.bitvector    = st->objects[ i ].bitvector;

			if(obj->name) {
				free(obj->name);
			}
			if(obj->short_description) {
				free(obj->short_description);
			}
			if(obj->description) {
				free(obj->description);
			}

			obj->name = (char*)malloc(strlen(st->objects[ i ].name) + 1);
			obj->short_description = (char*)malloc(strlen(st->objects[ i ].sd) +
												   1);
			obj->description = (char*)malloc(strlen(st->objects[ i ].desc) + 1);

			strcpy(obj->name, st->objects[ i ].name);
			strcpy(obj->short_description, st->objects[ i ].sd);
			strcpy(obj->description, st->objects[ i ].desc);


			for(j = 0; j < MAX_OBJ_AFFECT; j++) {
				obj->affected[ j ] = st->objects[ i ].affected[ j ];
			}

			/* item restoring */
			if(st->objects[i].depth > 60) {
				mudlog(LOG_SYSERR, "Object have depth >60 in obj_store_to_room.\r\n");
				st->objects[i].depth = 0;
			}
			if(st->objects[i].depth > tmp_cur_depth) {
				if(st->objects[i].depth != tmp_cur_depth + 1) {
					mudlog(LOG_SYSERR, "weird! object depth changed from %d to %d",
						   tmp_cur_depth, st->objects[i].depth);
				}
				in_obj[ tmp_cur_depth ] = last_obj;
				tmp_cur_depth++;
			}
			else if(st->objects[i].depth < tmp_cur_depth) {
				tmp_cur_depth--;
			}
			if(tmp_cur_depth) {
				obj_to_obj(obj, in_obj[ tmp_cur_depth - 1 ]);
			}
			else {
				obj_to_room2(obj, room);
			}
			last_obj = obj;
		}
	}
}

void load_room_objs(int room) {
	FILE* fl;
	struct obj_file_u st;
	char buf[200];

	sprintf(buf, "world/%d", room);
	mudlog(LOG_CHECK,"loading saved room: %s",buf);

	if((fl = fopen(buf, "r+b")) != NULL) {
		rewind(fl);

		if(ReadObjs(fl, &st)) {
			obj_store_to_room(room, &st);
		}

		fclose(fl);
		/*save_room( room );*/
	}
}

void save_room(int room) {
	struct obj_file_u st;
	struct obj_data* obj;
	struct room_data* rm = 0;
	char buf[255];
	FILE* f1 = 0;

	rm = real_roomp(room);

	obj = rm->contents;
	sprintf(buf, "world/%d", room);
	st.number = 0;

	if(obj) {
		f1 = fopen(buf, "w");

		if(f1) {
			rewind(f1);
			obj_to_store(obj, &st, NULL, 0);

			if(st.number > 50) {
				st.number = 50;
			}

			sprintf(buf, "Room %d", room);
			strcpy(st.owner, buf);
			st.gold_left = 0;
			st.total_cost = 0;
			st.last_update = 0;
			st.minimum_stay = 0;
			WriteObjs(f1, &st);
			fclose(f1);
		}
	}
}

} // namespace Alarmud

