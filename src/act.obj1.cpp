/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD
 *
 * $Id: act.obj1.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
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
#include "act.obj1.hpp"
#include "act.info.hpp"
#include "act.other.hpp"
#include "cmdid.hpp"      // for CMD_GET
#include "comm.hpp"
#include "db.hpp"         // for read_object, real_object, REAL
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "signals.hpp"    // for SetStatus
#include "snew.hpp"
#include "spell_parser.hpp"
#include "trap.hpp"
#include "utility.hpp"    // for str_cmp, getabunch, getall, CAN_SEE_OBJ
namespace Alarmud {



/* procedures related to get */
void get(struct char_data* ch, struct obj_data* obj_object,
		 struct obj_data* sub_object) {
	char buffer[ 256 ];

	/* if it is a limited items check if the PC EGO is
	   strong enough to control all the limited items
	   Gaia 2001 */
	if(IS_RARE(obj_object) && !EgoSave(ch)) {
		act("Non hai voglia di prendere $p.",FALSE, ch, obj_object, sub_object, TO_CHAR);
		return ;
	}


	/* check person to item ego */
	if(CheckEgoGet(ch, obj_object) && CheckGetBarbarianOK(ch, obj_object)) {
		if(sub_object) {
			if(!IS_SET(sub_object->obj_flags.value[ 1 ], CONT_CLOSED)) {
				act("Prendi $p da $P.", FALSE, ch, obj_object, sub_object, TO_CHAR);
				act("$n prende $p da $P.", TRUE, ch, obj_object, sub_object, TO_ROOM);
				obj_from_obj(obj_object);
				obj_to_char(obj_object, ch);
			}
			else {
				act("Prima dovresti aprire $P.", TRUE, ch, 0, sub_object, TO_CHAR);
				return;
			}
		}
		else {
			if(obj_object->in_room == NOWHERE) {
				obj_object->in_room = ch->in_room;
			}
			act("Prendi $p.", FALSE, ch, obj_object, 0, TO_CHAR);
			act("$n prende $p.", TRUE, ch, obj_object, 0, TO_ROOM);
			obj_from_room(obj_object);
			obj_to_char(obj_object, ch);
		}
		if(obj_object->obj_flags.type_flag == ITEM_MONEY &&
				obj_object->obj_flags.value[0] >= 1) {
			obj_from_char(obj_object);
			if(obj_object->obj_flags.value[0] > 1)
				sprintf(buffer, "C'erano %d monete.\n\r",
						obj_object->obj_flags.value[ 0 ]);
			else {
				sprintf(buffer, "C'era una miserabile moneta.\n\r");
			}
			send_to_char(buffer, ch);
			GET_GOLD(ch) += obj_object->obj_flags.value[ 0 ];
			if(GET_GOLD(ch) > 500000 && obj_object->obj_flags.value[0] > 100000) {
				mudlog(LOG_PLAYERS, "%s just got %d coins",
					   GET_NAME(ch),obj_object->obj_flags.value[0]);
			}
			extract_obj(obj_object);
		}
	}
	else {
		/* failed barb or ego item check */
	}
}
ACTION_FUNC(do_get) {
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct obj_data* sub_object, *obj_object,*next_obj;
	bool found = FALSE;
	bool fail  = FALSE;
	bool has   = FALSE;                /* :P */
	int type   = 3;
	char newarg[1000];
	int num, p;


	argument_interpreter(arg, arg1, arg2);

	/* get type */
	if(!*arg1) {
		type = 0;
	}
	if(*arg1 && !*arg2) {
		/* plain "get all" */
		if(!str_cmp(arg1,"all")) {
			type = 1;
		}
		else {
			type = 2; /* "get all.item" */
		}
	}
	if(*arg1 && *arg2) {
		if(!str_cmp(arg1,"all")) {
			/* "get all all" */
			if(!str_cmp(arg2,"all")) {
				type = 3;
			}
			else {
				type = 4; /* get all object */
			}
		}
		else {
			if(!str_cmp(arg2,"all")) {
				/* "get object all" */
				type = 5;
			}
			else {
				type = 6; /*get object object*/
			}
		}
	}

	switch(type) {
	/* get */
	case 0:
		send_to_char("Cosa vuoi prendere?\n\r", ch);
		break;
	/* get all */
	case 1:
		sub_object = 0;
		found = FALSE;
		fail = FALSE;
		for(obj_object = real_roomp(ch->in_room)->contents; obj_object;
				obj_object = next_obj) {
			next_obj = obj_object->next_content;
			/* check for a trap (traps fire often)
			       only PCS are affected by traps Gaia 2001
			if (CheckForAnyTrap(ch, obj_object)) */

			if(!IS_NPC(ch) && CheckForAnyTrap(ch, obj_object)) {
				SetStatus("Ending after CheckForAnyTrap in do_get", NULL);
				return;
			}


			if(CAN_SEE_OBJ(ch,obj_object)) {
				if((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) {
					if((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <=
							CAN_CARRY_W(ch)) {
						if(CAN_WEAR(obj_object,ITEM_TAKE)) {
							get(ch,obj_object,sub_object);
							found = TRUE;
						}
						else {
							act("Non puoi prendere $p.", FALSE, ch, obj_object, NULL,
								TO_CHAR);
							fail = TRUE;
						}
					}
					else {
						act("Non puoi prendere $p, pesa troppo.", FALSE, ch, obj_object,
							NULL, TO_CHAR);
						fail = TRUE;
					}
				}
				else {
					act("Non puoi prendere $p, hai gia` troppa roba in mano.", FALSE,
						ch, obj_object, NULL, TO_CHAR);
					fail = TRUE;
				}
			}
		}
		if(!found && !fail) {
			send_to_char("Non c'e nulla da prendere.\n\r", ch);
		}
		break;
	/* get ??? (something) */
	case 2: {
		struct obj_data* pObjList;

		sub_object = NULL;
		found = FALSE;
		fail = FALSE;
		if(getall(arg1, newarg) == TRUE) {
			strcpy(arg1, newarg);
			num = -1;
		}
		else if((p = getabunch(arg1, newarg)) != 0) {
			strcpy(arg1, newarg);
			num = p;
		}
		else {
			num = 1;
		}

		pObjList = real_roomp(ch->in_room)->contents;

		while(num != 0) {
			obj_object = get_obj_in_list_vis(ch, arg1, pObjList);

			if(obj_object) {
				pObjList = obj_object->next_content;
				if(IS_CORPSE(obj_object) && num != 1) {
					send_to_char("Puoi prendere un solo corpo alla volta.\n\r", ch);
					SetStatus("Ending after IS_CORPSE in do_get", NULL);
					return; /* no need for num and fail above I guess */
				}

				/* check for trap */

				/*  Only PCS are affected by traps  Gaia 2001
				        if( CheckForGetTrap( ch, obj_object ) ) */

				if(!IS_NPC(ch) && CheckForGetTrap(ch, obj_object)) {
					SetStatus("Ending after CheckForGetTrap in do_get", NULL);
					return;
				}

				if((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
							CAN_CARRY_W(ch)) {
						if(CAN_WEAR(obj_object, ITEM_TAKE)) {
							get(ch, obj_object, sub_object);
							found = TRUE;
						}
						else {
							act("Non puoi prendere $p.", FALSE, ch, obj_object, NULL,
								TO_CHAR);
							fail = TRUE;
						}
					}
					else {
						act("Non riesci a prendere $p, pesa troppo.", FALSE, ch,
							obj_object, NULL, TO_CHAR);
						fail = TRUE;
					}
				}
				else {
					act("Non riesci a prendere $p, hai troppa roba in mano", FALSE,
						ch, obj_object, NULL, TO_CHAR);
					fail = TRUE;
					break;
				}
				if(num > 0) {
					num--;
				}
			}
			else {
				break;
			}
		}

		if(!found && !fail) {
			sprintf(buffer, "Non c'e` nessun %s qui.\n\r", arg1);
			send_to_char(buffer, ch);
		}
	}

	break;
	/* get all all */

	case  3:
		send_to_char("Non puoi prendere tutto da tutto.\n\r", ch);
		break;
	/* get all ??? */
	case 4:
		found = FALSE;
		fail  = FALSE;
		has   = FALSE;
		sub_object = get_obj_vis_accessible(ch, arg2);
		if(sub_object) {
			if(GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
				if(get_obj_in_list_vis(ch, arg2, ch->carrying)) {
					has = TRUE;
				}
				for(obj_object = sub_object->contains;
						obj_object;
						obj_object = next_obj) {
					next_obj = obj_object->next_content;

					/* check for trap */
					/*  Only PCS are affected by traps  Gaia 2001
					    if( CheckForGetTrap( ch, obj_object ) ) */

					if(!IS_NPC(ch) && CheckForGetTrap(ch, obj_object)) {
						SetStatus("Ending after CheckForGetTrap 2 in do_get", NULL);
						return;
					}

					if(CAN_SEE_OBJ(ch, obj_object)) {
						if((IS_CARRYING_N(ch) + 1) < CAN_CARRY_N(ch)) {
							if(has ||
									(IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
									CAN_CARRY_W(ch)) {
								if(CAN_WEAR(obj_object,ITEM_TAKE)) {
									get(ch, obj_object, sub_object);
									found = TRUE;
								}
								else {
									act("Non puoi prendere $p.", FALSE, ch, obj_object, NULL,
										TO_CHAR);
									fail = TRUE;
								}
							}
							else {
								act("Non puoi prendere $p, pesa troppo.", FALSE, ch,
									obj_object, NULL, TO_CHAR);
								fail = TRUE;
							}
						}
						else {
							act("Non puoi prendere $p, hai troppa roba in mano", FALSE,
								ch, obj_object, NULL, TO_CHAR);
							fail = TRUE;
							break;
						}
					}
				}
				if(!found && !fail) {
					act("Non c'e niente in $p.", FALSE, ch, sub_object, NULL,
						TO_CHAR);
					fail = TRUE;
				}
			}
			else {
				act("$p non e` un contenitore.", FALSE, ch, sub_object, NULL,
					TO_CHAR);
				fail = TRUE;
			}
		}
		else {
			sprintf(buffer,"Non vedi nessun %s.\n\r", arg2);
			send_to_char(buffer, ch);
			fail = TRUE;
		}
		break;
	case 5:
		act("Puoi prendere gli oggetti da un solo contenitore alla volta.", FALSE,
			ch, 0, 0, TO_CHAR);
		break;

	/*  take ??? from ???   (is it??) */
	case 6:
		found = FALSE;
		fail  = FALSE;
		has   = FALSE;
		sub_object = (struct obj_data*) get_obj_vis_accessible(ch, arg2);
		if(sub_object) {
			if(!strcasecmp(arg1,"trofeo") && IS_CORPSE(sub_object)) {
				get_trophy(ch,sub_object);
				return;
			}

			if(GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
				struct obj_data* pObjList;

				if(get_obj_in_list_vis(ch, arg2, ch->carrying)) {
					has=TRUE;
				}
				if(getall(arg1, newarg) == TRUE) {
					num = -1;
					strcpy(arg1, newarg);
				}
				else if((p = getabunch(arg1, newarg)) != 0) {
					num = p;
					strcpy(arg1, newarg);
				}
				else {
					num = 1;
				}

				pObjList = sub_object->contains;

				while(num != 0) {
					obj_object = get_obj_in_list_vis(ch, arg1, pObjList);

					if(obj_object) {
						pObjList = obj_object->next_content;
						/* Only PCs are affected by Traps - Gaia 2001
						if( CheckForInsideTrap( ch, sub_object ) ) */

						if(!IS_NPC(ch) && CheckForInsideTrap(ch, sub_object)) {
							SetStatus("Ending after CheckForInsideTrap in do_get", NULL);
							return;
						}

						if((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
							if(has ||
									(IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
									CAN_CARRY_W(ch)) {
								if(CAN_WEAR(obj_object, ITEM_TAKE)) {
									get(ch, obj_object, sub_object);
									found = TRUE;
								}
								else {
									act("Non puoi prendere $p.", TRUE, ch, obj_object, 0,
										TO_CHAR);
									fail = TRUE;
								}
							}
							else {
								act("Non puoi prendere $p, pesa troppo.", TRUE,
									ch, obj_object, 0, TO_CHAR);
								fail = TRUE;
							}
						}
						else {
							act("Non puoi prendere $p, hai gia` roba in mano.", TRUE, ch,
								obj_object, NULL, TO_CHAR);
							fail = TRUE;
							break;
						}

						if(num > 0) {
							num--;
						}
					}
					else {
						break;
					}
				}

				if(!fail && !found) {
					sprintf(buffer, "$p non contiene nessun %s.", arg1);
					act(buffer, TRUE, ch, sub_object, 0, TO_CHAR);
				}
			}
			else {
				act("$p non e` un contenitore.", TRUE, ch, sub_object, 0, TO_CHAR);
				fail = TRUE;
			}
		}
		else {
			sprintf(buffer, "Non vedi nessun %s.", arg2);
			act(buffer, TRUE, ch, 0, 0, TO_CHAR);
			fail = TRUE;
		}
		break;
	}
	SetStatus("Before saving PC in do_get", NULL);
#if NODUPLICATES
	if(found && IS_PC(ch)) {
		do_save(ch, "", 0);
	}
#endif
	SetStatus("Returning from do_get", NULL);
}
void get_trophy(struct char_data* ch, struct obj_data* ob) {
	char buffer[MAX_STRING_LENGTH];
	char buf[256];
	struct obj_data* cp;
	int r_num;
	if(ob) {
		if(!ob->affected[0].modifier) {
			sprintf(buf,"Non e` rimasto abbastanza %s%s per farne un trofeo",
					*(ob->short_description+1)==' '?"dei":"de",
					ob->short_description+1);
			send_to_char(buf,ch);
			return;
		}
		ob->affected[0].modifier=0;
		/* Creazione testa */
#define SEVERED_HEAD       60
#define CORPO              "corpo di "  /* vedi in fight.c */
		/* Parto da un oggetto senza specials */
		if((r_num = real_object(SEVERED_HEAD)) >= 0) {
			cp = read_object(r_num, REAL);

			strncpy(buffer,strstr(ob->short_description,CORPO)?
					strstr(ob->short_description,CORPO)+strlen(CORPO):ob->name,
					160);
			sprintf(buf,"trofeo %s",buffer);
			free(cp->name);
			cp->name=strdup(buf);

			sprintf(buf,"un trofeo dell'uccisione di %s",buffer);
			free(cp->short_description);
			cp->short_description=strdup(buf);
			free(cp->action_description);
			cp->action_description=strdup(buf);

			sprintf(buf,"Un trofeo dell'uccisione di %s giace per terra.",buffer);
			free(cp->description);
			cp->description=strdup(buf);
			cp->obj_flags.type_flag = ITEM_CONTAINER;
			cp->obj_flags.wear_flags = ITEM_TAKE;
			cp->obj_flags.value[0] = 0; /* You can't store stuff in a trophy */
			cp->char_vnum=ob->char_vnum;
			cp->affected[0].modifier=ob->affected[0].modifier;
			cp->affected[1].modifier=ob->affected[1].modifier;
			cp->obj_flags.value[3] = 0; /* corpse identifyer */
			cp->obj_flags.timer=0;        /*I trofei sono oggetti permanenti*/
			cp->obj_flags.cost_per_day=1000;
			cp->obj_flags.cost=0;
			cp->obj_flags.weight=0;
			obj_to_room(cp,ch->in_room);
			sprintf(buf,"Ti avventi sul corpo di %s e ne trai un trofeo!",buffer);
			act(buf,TRUE,ch,0,0,TO_CHAR);
			sprintf(buf,"$n si avventa sul corpo di %s e ne trae un trofeo!",buffer);
			act(buf,TRUE,ch,0,0,TO_ROOM);
			do_get(ch,"trofeo\0",CMD_GET);
		}
	}
	return;
}



ACTION_FUNC(do_drop) {
	char tmp[MAX_INPUT_LENGTH];
	char tmp2[ MAX_INPUT_LENGTH ];
	int amount;
	char buffer[MAX_STRING_LENGTH];
	struct obj_data* tmp_object;
	struct obj_data* next_obj;
	bool test = FALSE;
	char newarg[1000];
	int num, p;

	argument_interpreter(arg, tmp, tmp2);

	if(*tmp && *tmp2) {
		if(is_number(tmp) && (str_cmp2("coin", tmp2) == 0 ||
							  str_cmp2("monet", tmp2) == 0)) {
			if((amount = atoi(tmp)) > 0) {
				if(GET_GOLD(ch) < amount) {
					send_to_char("Non hai tutte quelle monete!\n\r", ch);
				}
				else {

					tmp_object = create_money(amount);
					obj_to_room(tmp_object, ch->in_room);
					GET_GOLD(ch) -= amount;
					if(amount == 1) {
						send_to_char("Posi una moneta.\n\r", ch);
						act("$n posa una moneta.", TRUE, ch, 0, 0, TO_ROOM);
					}
					else {
						send_to_char("Posi alcune monete.\n\r", ch);
						act("$n posa alcune monete.", TRUE, ch, 0, 0, TO_ROOM);
					}
				}
			}
			else {
				send_to_char("Non hai ben chiaro il valore dell'oro.\n\r", ch);
			}
		}
		else {
			send_to_char("Non puoi farlo.\n\r", ch);
		}

		return;
	}
	else if(*tmp) {
		if(!str_cmp(tmp, "all")) {
			for(tmp_object = ch->carrying;
					tmp_object;
					tmp_object = next_obj) {
				next_obj = tmp_object->next_content;
				if(!IS_OBJ_STAT(tmp_object, ITEM_NODROP) || IS_IMMORTAL(ch)) {
					/* if it is a limited items check if the PC EGO is
					   strong enough to control all the limited items
					   Gaia 2001 */
					if(IS_RARE(tmp_object) && !EgoSave(ch)) {
						act("Non hai la forza di posare $p.",FALSE, ch, tmp_object, 0, TO_CHAR);
						return ;
					}
					obj_from_char(tmp_object);
					obj_to_room(tmp_object,ch->in_room);
					check_falling_obj(tmp_object, ch->in_room);
					test = TRUE;
				}
				else {
					if(CAN_SEE_OBJ(ch, tmp_object)) {
						act("Non puoi posare $p", FALSE, ch, tmp_object, NULL,
							TO_CHAR);
						test = TRUE;
					}
				}
			}
			if(!test) {
				send_to_char("Non hai nulla da posare.\n\r", ch);
			}
			else {
				act("Posi tutto quello che hai.", FALSE, ch, 0, 0, TO_CHAR);
				act("$n posa tutto quello che ha.", TRUE, ch, 0, 0, TO_ROOM);
			}
#if NODUPLICATES
			do_save(ch, "", 0);
#endif
		}
		else {
			struct obj_data* pObjList;
			int bFound = FALSE;
			/* &&&&&& */
			if(getall(tmp, newarg) == TRUE) {
				num = -1;
				strcpy(tmp,newarg);
			}
			else if((p = getabunch(tmp,newarg)) != 0) {
				num = p;
				strcpy(tmp,newarg);
			}
			else {
				num = 1;
			}

			pObjList = ch->carrying;

			while(num != 0) {
				tmp_object = get_obj_in_list_vis(ch, tmp, pObjList);

				if(tmp_object) {
					pObjList = tmp_object->next_content;
					if(!IS_OBJ_STAT(tmp_object, ITEM_NODROP) || IS_IMMORTAL(ch)) {
						/* if it is a limited items check if the PC EGO is strong enough
						   Gaia 2001 */
						if(IS_RARE(tmp_object) && !EgoSave(ch)) {
							act("Non hai la forza di posare $p.",FALSE, ch, tmp_object, 0, TO_CHAR);
							return ;
						}
						act("Posi $p.", FALSE, ch, tmp_object, NULL, TO_CHAR);
						act("$n posa $p.", TRUE, ch, tmp_object, 0, TO_ROOM);
						obj_from_char(tmp_object);
						obj_to_room(tmp_object,ch->in_room);

						check_falling_obj(tmp_object, ch->in_room);

					}
					else {
						act("Non puoi posare $p.", FALSE, ch, tmp_object, NULL, TO_CHAR);
					}
					if(num > 0) {
						num--;
					}
					bFound = TRUE;
				}
				else {
					break;
				}
			}
			if(!bFound) {
				sprintf(buffer,"Non hai nessun %s.\n\r", tmp);
				send_to_char(buffer, ch);
			}
#if NODUPLICATES
			else {
				do_save(ch, "", 0);
			}
#endif
		}
	}
	else {
		send_to_char("Cosa vuoi posare?\n\r", ch);
	}
}



ACTION_FUNC(do_put) {
	char buffer[256];
	char arg1[128];
	char arg2[128];
	struct obj_data* obj_object;
	struct obj_data* sub_object;
	struct char_data* tmp_char;
	int bits;
	char newarg[100];
	int num, p;

	argument_interpreter(arg, arg1, arg2);

	if(*arg1) {
		if(*arg2) {
			if(getall(arg1,newarg)==TRUE) {
				num = -1;
				strcpy(arg1,newarg);
			}
			else if((p = getabunch(arg1,newarg))!=0) {
				num = p;
				strcpy(arg1,newarg);
			}
			else {
				num = 1;
			}

			if(!strcmp(arg1,"all")) {

				send_to_char("Non puoi farlo.\n\r",ch);
				return;

			}
			else {
				while(num != 0) {
#if 1
					bits = generic_find(arg1, FIND_OBJ_INV,
										ch, &tmp_char, &obj_object);
#else
					obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif

					if(obj_object) {
						if(IS_OBJ_STAT(obj_object, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
							act("Non puoi mettere $p da nessuna parte.", FALSE, ch,
								obj_object, NULL, TO_CHAR);
							return;
						}
						/* if it is a limited items check if the PC EGO is strong enough
						   Gaia 2001 */
						if(IS_RARE(obj_object) && !EgoSave(ch)) {
							act("Preferisci tenere $p vicino a te.",FALSE, ch, obj_object, sub_object, TO_CHAR);
							return ;
						}

						bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
											ch, &tmp_char, &sub_object);
						if(sub_object) {
							if(GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
								if(!IS_SET(sub_object->obj_flags.value[ 1 ], CONT_CLOSED)) {
									if(obj_object == sub_object) {
										act("Giocherelli un po` con $p ma senza risultato.",
											FALSE, ch, obj_object, NULL, TO_CHAR);

										return;
									}
									if(((sub_object->obj_flags.weight) +
											(obj_object->obj_flags.weight)) <
											(sub_object->obj_flags.value[0])) {
										act("Metti $p in $P",TRUE, ch, obj_object, sub_object,
											TO_CHAR);
										if(bits==FIND_OBJ_INV) {
											obj_from_char(obj_object);
											obj_to_obj(obj_object, sub_object);
										}
										else {
											obj_from_room(obj_object);
											obj_to_obj(obj_object, sub_object);
										}

										act("$n mette $p in $P",TRUE, ch, obj_object, sub_object,
											TO_ROOM);
										num--;
									}
									else {
										if(singular(sub_object)) {
											send_to_char("Non c'entra.\n\r", ch);
										}
										else {
											send_to_char("Non c'entrano.\n\r", ch);
										}
										num = 0;
									}
								}
								else {
									if(singular(obj_object)) {
										send_to_char("Sembra chiuso.\n\r", ch);
									}
									else {
										send_to_char("Sembrano chiusi.\n\r", ch);
									}
									num = 0;
								}
							}
							else {
								act("$p non e` un contenitore.", FALSE, ch, sub_object, NULL,
									TO_CHAR);
								num = 0;
							}
						}
						else {
							sprintf(buffer, "Non hai nessun %s.\n\r", arg2);
							send_to_char(buffer, ch);
							num = 0;
						}
					}
					else {
						if(num > 0 || num == -1) {
							sprintf(buffer, "Non hai nessun %s.\n\r", arg1);
							send_to_char(buffer, ch);
						}
						num = 0;
					}
				}
#if   NODUPLICATES
				do_save(ch, "", 0);
#endif
			}
		}
		else {
			sprintf(buffer, "Dove vorresti mettere un %s?\n\r", arg1);
			send_to_char(buffer, ch);
		}
	}
	else {
		send_to_char("Cosa vuoi mettere e dove?\n\r",ch);
	}
}

int newstrlen(const char* p) {
	int i;

	for(i=0; i<10&&*p; i++,p++);
	return(i);
}

void truegivexp(struct char_data* ch, struct char_data* victim, int amount) {
	char buf[300];
	/* Workaround, se si aggiusta gain_exp DEVE sparire il diviso howmanyclas*/
	gain_exp(ch,-(amount/HowManyClasses(ch)));
	sprintf(buf, "Dai %d xp a $N.", amount);
	act(buf, FALSE, ch, NULL, victim, TO_CHAR);
	amount/=100;
	amount*=(100-number(3,10));
	sprintf(buf, "$n ti da` %d xp.", amount);
	act(buf, FALSE, ch, NULL, victim, TO_VICT);
	/* gain_exp(victim,amount); ERRORE GRAVE, corretto! */
	GET_EXP(victim) += amount/HowManyClasses(victim) ;
	return;
}

void givexp(struct char_data* ch, struct char_data* victim, int amount)
#define MAX_FOR_VASSALLI 180000001
#define MAX_FOR_PRINCE 430000001
#define MAX_AMOUNT 1000000
{
	int nc=1;
	int nv=1;
	if(!in_clan(ch,victim) ||
			(!IS_VASSALLOOF(ch,GET_NAME(victim))&&
			 !IS_VASSALLOOF(victim,GET_NAME(ch)))) {
		send_to_char("Scordatelo!\n",ch);
		return;
	}
	if(amount<=1) {
		send_to_char("E bravo furbone.... vuoi donare xp negativi?!\n",ch);
		return;
	}
	nc=HowManyClasses(ch);
	nv=HowManyClasses(victim);
	if(IS_PRINCE(ch)) {
		/* Il principe da al vassallo*/
		if((GET_EXP(ch)-(amount/nc))>=PRINCEEXP &&
				(GET_EXP(victim)+(amount/nv))<MAX_FOR_VASSALLI &&
				amount<=MAX_AMOUNT &&
				(GET_EXP(victim)*0.2)>=(amount/nv)) {
			truegivexp(ch,victim,amount);
		}
		else {
			send_to_char("Esagerato....... cosi` e` troppo!",ch);
		}
	}
	else {
		/* Il vassallo da al principe*/
		if((GET_EXP(ch)-(amount/nc))>=MIN_EXP(ch) &&
				amount<=MAX_AMOUNT &&
				(GET_EXP(victim)+(amount/nv))<MAX_XP) { //MAX_FOR_PRINCE )
			truegivexp(ch,victim,amount);
		}
		else {
			send_to_char("Esagerato....... cosi` e` troppo!",ch);
		}

	}
	return;
}



ACTION_FUNC(do_give) {
	char obj_name[200], vict_name[80], buf[132];
	char tmp[80], newarg[100];
	int amount, num, p, count;
	struct char_data* vict;
	struct obj_data* obj;

	arg = one_argument(arg, obj_name);
	if(is_number(obj_name)) {
		if(newstrlen(obj_name) >= 10) {
			obj_name[ 10 ] = '\0';
		}
		amount = atoi(obj_name);
		arg = one_argument(arg, tmp);
		if(str_cmp2("coin", tmp) &&
				str_cmp2("monet", tmp) &&
				str_cmp2("xp",tmp)) {
			send_to_char("Eh?\n\r",ch);
			return;
		}
		if(amount <= 0) {
			send_to_char("Non hai ben chiaro il valore delle cose.\n\r", ch);
			return;
		}

		arg = one_argument(arg, vict_name);

		if(!*vict_name) {
			send_to_char("A chi vuoi dare delle monete?\n\r", ch);
		}
		else if(!(vict = get_char_room_vis(ch, vict_name))) {
			send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
		}
		else {
			if(!str_cmp2("xp",tmp)) {
				givexp(ch,vict,amount);
				return;
			}

			if(GET_GOLD(ch) < amount &&
					(IS_NPC(ch) || GetMaxLevel(ch) < DIO)) {
				send_to_char("Non hai tutti quei soldi.\n\r", ch);
				return;
			}
			if(amount == 1) {
				act("$n ti da` una moneta d'oro.", FALSE, ch, NULL, vict, TO_VICT);
				act("Dai una moneta $N.", FALSE, ch, NULL, vict, TO_CHAR);
			}
			else {
				sprintf(buf, "$n ti da` %d monete d'oro.", amount);
				act(buf, FALSE, ch, NULL, vict, TO_VICT);
				sprintf(buf, "Dai %d monete d'oro a $N.", amount);
				act(buf, FALSE, ch, NULL, vict, TO_CHAR);
			}

			act("$n da` alcune monete a $N.", TRUE, ch, 0, vict, TO_NOTVICT);
			if(IS_NPC(ch) || GetMaxLevel(ch) < DIO) {
				GET_GOLD(ch) -= amount;
			}
			GET_GOLD(vict) += amount;
			save_char(ch, AUTO_RENT, 0);
			if(GET_GOLD(vict) > 500000 && amount > 100000) {
				mudlog(LOG_PLAYERS, "%s gave %d coins to %s", GET_NAME(ch),amount, GET_NAME(vict));
			}
		}

		return;
	}
	else {
		arg=one_argument(arg, vict_name);


		if(!*obj_name || !*vict_name) {
			send_to_char("Vuoi dare cosa a chi?\n\r", ch);
			return;
		}
		/* &&&& */
		if(getall(obj_name, newarg) == TRUE) {
			num = -1;
			strcpy(obj_name, newarg);
		}
		else if((p = getabunch(obj_name, newarg)) != 0) {
			num = p;
			strcpy(obj_name, newarg);
		}
		else {
			num = 1;
		}

		count = 0;
		if((vict = get_char_room_vis(ch, vict_name))) {
			while(num != 0) {
				if(!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
					if(count == 0)
						send_to_char("Non sembra che tu abbia nulla del genere.\n\r",
									 ch);
					return;
				}
				if(IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
					if(singular(obj)) {
						send_to_char("Non puoi lasciarlo andare.\n\r", ch);
					}
					else {
						send_to_char("Non puoi lasciarli andare.\n\r", ch);
					}

					return;
				}
				else {
					/* if it is a limited items check if the PC EGO is
					   strong enough to control all the limited items
					   Gaia 2001 */
					if(IS_RARE(obj) && !EgoSave(ch)) {
						act("Non vuoi separarti da $p .", FALSE, ch, obj, vict, TO_CHAR);
						return ;
					}
					if(IS_RARE(obj) && !EgoSave(vict)) {
						act("$n cerca insistentemente di darti $p.", FALSE, ch, obj, vict, TO_VICT);
						act("Cerchi di dare $p a $N ma lui lo rifiuta.", FALSE, ch, obj, vict, TO_CHAR);
						act("$n cerca invano di dare $p a $N.", TRUE, ch, obj, vict, TO_NOTVICT);
						return ;
					}

					if(!CheckGiveBarbarianOK(ch, vict, obj) ||
							!CheckEgoGive(ch, vict, obj)) {
						return;
					}
					if(vict == ch) {
						send_to_char("Ok.\n\r", ch);
						return;
					}

					if((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict)) {
						act("$N sembra avere le mani piene.", 0, ch, 0, vict, TO_CHAR);
						return;
					}
					if(obj->obj_flags.weight + IS_CARRYING_W(vict) >
							CAN_CARRY_W(vict)) {
						act("$N non riesce a tenere $p, pesa troppo.", 0, ch, obj, vict,
							TO_CHAR);
						return;
					}
                    if(!IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && IS_OBJ_STAT2(obj, ITEM2_EDIT) && IS_MAESTRO_DEGLI_DEI(ch))
                    {
                        pers_obj(ch, vict, obj, CMD_GIVE);
                    }
					act("$n da` $p a $N.", 1, ch, obj, vict, TO_NOTVICT);
					act("$n ti da` $p.", 0, ch, obj, vict, TO_VICT);
					act("Dai $p a $N", 0, ch, obj, vict, TO_CHAR);
					obj_from_char(obj);
					obj_to_char(obj, vict);

					if(num > 0) {
						num--;
					}
					count++;
				}
			}
#if   NODUPLICATES
			do_save(ch, "", 0);
			do_save(vict, "", 0);
#endif
		}
		else {
			send_to_char("Non c'e` nessuno con quel nome qui.\n\r", ch);
		}
	}
}


ACTION_FUNC(do_pquest) {

	char obj_name[200], vict_name[80], buf[132];
	int punti_quest,  old_punti;
	struct char_data* vict;

	arg = one_argument(arg, obj_name);
	if(is_number(obj_name)) {
		if(newstrlen(obj_name) >= 10) {
			obj_name[ 10 ] = '\0';
		}
		punti_quest = atoi(obj_name);
		arg = one_argument(arg, vict_name);

		if(punti_quest ==0) {
			if(!(vict = get_char_room_vis(ch, vict_name))) {
				send_to_char("Di chi vuoi conoscere le rune degli Dei? \n\r", ch);
				return;
			}
			sprintf(buf,"%s ha %d rune degli Dei.\n\r",GET_NAME(vict),GET_RUNEDEI(vict));
			send_to_char(buf, ch);
			return;
		}

		if(punti_quest >0) {
			if(!*vict_name) {
				send_to_char("A chi vuoi assegnare rune degli Dei? \n\r", ch);
				return;
			}
			else if(!(vict = get_char_room_vis(ch, vict_name))) {
				send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
				return;
			}
			else {
				old_punti =GET_RUNEDEI(vict);
				if(GET_RUNEDEI(ch) < punti_quest &&
						(IS_NPC(ch) || GetMaxLevel(ch) < IMMORTALE)) {
					send_to_char("Non puoi assegnare rune degli Dei!!\n\r", ch);
					return;
				}
				if(punti_quest == 1) {
					act("Fiamme divine ti avvolgono e piano piano si concentrano in una zona del tuo corpo!", FALSE, ch, NULL, vict, TO_VICT);
					act("$n ti tatua una runa degli Dei sul corpo!", FALSE, ch, NULL, vict, TO_VICT);
					sprintf(buf, "Assegni una rune dagli Dei a $N che ne aveva %d.", old_punti);
					act(buf, FALSE, ch, NULL, vict, TO_CHAR);
				}
				else {
					act("Fiamme divine ti avvolgono e piano piano si concentrano in diverse zone del tuo corpo!", FALSE, ch, NULL, vict, TO_VICT);
					sprintf(buf, "$n ti tatua %d rune degli Dei.", punti_quest);
					act(buf, FALSE, ch, NULL, vict, TO_VICT);
					sprintf(buf, "Assegni %d rune degli Dei a $N, ne aveva %d.", punti_quest, old_punti);
					act(buf, FALSE, ch, NULL, vict, TO_CHAR);
				}
				act("Ad un gesto della mano di $n, fiamme divine avvolgono il corpo di $N.", TRUE, ch, 0, vict, TO_NOTVICT);
				act("$n tatua delle rune degli Dei sul corpo di $N.", TRUE, ch, 0, vict, TO_NOTVICT);
				if(IS_NPC(ch) || GetMaxLevel(ch) <= QUESTMASTER) {
					GET_RUNEDEI(ch) -= punti_quest;
				}
				GET_RUNEDEI(vict) += punti_quest;
				save_char(ch, AUTO_RENT, 0);
				mudlog(LOG_PLAYERS, "%s assegna %d rune degli Dei a %s, ne aveva %d", GET_NAME(ch), punti_quest, GET_NAME(vict), old_punti);
				return;
			}
		}

		if(punti_quest <0) {
			if(!*vict_name) {
				send_to_char("A chi vuoi togliere rune degli Dei? \n\r", ch);
				return;
			}
			else if(!(vict = get_char_room_vis(ch, vict_name))) {
				send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
				return;
			}
			else {
				old_punti =GET_RUNEDEI(vict);
				if(GET_RUNEDEI(ch) < -punti_quest &&
						(IS_NPC(ch) || GetMaxLevel(ch) < IMMORTALE)) {
					send_to_char("Non puoi togliere rune degli Dei!!\n\r", ch);
					return;
				}
				if((old_punti+punti_quest) <0) {
					send_to_char("Non puoi togliere quello che non si ha!! Hai esagerato!?!\n\r", ch);
					return;
				}
				if(punti_quest == -1) {
					act("$n fa un gesto con la mano, ed una runa tatuata sul tuo corpo sbiadice lentamente fino a scomparire!", FALSE, ch, NULL, vict, TO_VICT);
					sprintf(buf, "Togli una runa degli Dei a $N, ne aveva %d.", old_punti);
					act(buf, FALSE, ch, NULL, vict, TO_CHAR);
				}
				else {
					sprintf(buf, "$n fa un gesto con la mano e %d rune che avevi tatuate lentamente sbiadiscono fino a scomparire.", -punti_quest);
					act(buf, FALSE, ch, NULL, vict, TO_VICT);
					sprintf(buf, "Togli %d rune degli Dei a $N, ne aveva %d.", -punti_quest, old_punti);
					act(buf, FALSE, ch, NULL, vict, TO_CHAR);
				}
				act("Ad un gesto della mano di $n, alcune rune tetuate sul corpo di $N lentamente sbiadisco.", TRUE, ch, 0, vict, TO_NOTVICT);
				GET_RUNEDEI(vict) += punti_quest;
				save_char(ch, AUTO_RENT, 0);
				mudlog(LOG_PLAYERS, "%s toglie %d rune degli Dei a %s, ne aveva %d", GET_NAME(ch), -punti_quest, GET_NAME(vict), old_punti);
				return;
			}
		}
	}
	else {
		send_to_char("Non hai ben chiaro il valore delle cose... lascia che ti rinfreschi la memoria:\n\n\r", ch);
	}
	send_to_char("pquest 0 target        per conoscere i pq\n\r", ch);
	send_to_char("pq valore target      per assegnare/rimuovere pquest\n\n\r", ch);
	return;
}


} // namespace Alarmud

