/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* act.obj.cpp — merged from act.obj1.c + act.obj2.c */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <array>
#include <sstream>
#include <string>
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
#include "act.obj.hpp"
#include "act.obj_common.hpp"
#include "act.obj_get.hpp"
#include "act.obj_wear.hpp"
#include "act.info.hpp"
#include "act.other.hpp"
#include "cmdid.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "regen.hpp"
#include "signals.hpp"
#include "snew.hpp"
#include "spell_parser.hpp"
#include "trap.hpp"
#include "utility.hpp"

namespace Alarmud {



ACTION_FUNC(do_get) {
	obj_run_get(ch, arg, cmd);
}
void get_trophy(struct char_data* ch, struct obj_data* ob) {
	std::array<char, MAX_STRING_LENGTH> buffer{};
	struct obj_data* cp = nullptr;
	int r_num = 0;
	if(ch == nullptr || ob == nullptr) {
		return;
	}
	if(!ob->affected[0].modifier) {
		const char* const shortDescr = ob->short_description;
		const bool useDei = shortDescr != nullptr && shortDescr[1] == ' ';
		std::ostringstream os;
		os << "Non e' rimasto abbastanza " << (useDei ? "dei" : "de")
		   << (shortDescr != nullptr ? shortDescr + 1 : "") << " per farne un trofeo.";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	ob->affected[0].modifier = 0;
#define SEVERED_HEAD 60
#define CORPO        "corpo di " /* vedi in fight.c */
	if((r_num = real_object(SEVERED_HEAD)) >= 0) {
		cp = read_object(r_num, REAL);
		const char* const nameSource =
		    (ob->short_description != nullptr && strstr(ob->short_description, CORPO) != nullptr)
		        ? strstr(ob->short_description, CORPO) + strlen(CORPO)
		        : ob->name;
		obj_copy_cstr(buffer.data(), buffer.size(), nameSource != nullptr ? nameSource : "");
		const std::string victimName(buffer.data());
		obj_replace_dup_cstr(cp->name, "trofeo " + victimName);

		const std::string trophyShort = "un trofeo dell'uccisione di " + victimName;
		obj_replace_dup_cstr(cp->short_description, trophyShort);
		obj_replace_dup_cstr(cp->action_description, trophyShort);

		obj_replace_dup_cstr(cp->description, "Un trofeo dell'uccisione di " + victimName + " giace per terra.");
		cp->obj_flags.type_flag = ITEM_CONTAINER;
		cp->obj_flags.wear_flags = ITEM_TAKE;
		cp->obj_flags.value[0] = 0;
		cp->char_vnum = ob->char_vnum;
		cp->affected[0].modifier = ob->affected[0].modifier;
		cp->affected[1].modifier = ob->affected[1].modifier;
		cp->obj_flags.value[3] = 0;
		cp->obj_flags.timer = 0;
		cp->obj_flags.cost_per_day = 1000;
		cp->obj_flags.cost = 0;
		cp->obj_flags.weight = 0;
		obj_to_room(cp, ch->in_room);
		std::ostringstream osChar;
		osChar << "Ti avventi sul corpo di " << buffer.data() << " e ne trai un trofeo!";
		act(osChar.str().c_str(), true, ch, nullptr, nullptr, TO_CHAR);
		std::ostringstream osRoom;
		osRoom << "$n si avventa sul corpo di " << buffer.data() << " e ne trae un trofeo!";
		act(osRoom.str().c_str(), true, ch, nullptr, nullptr, TO_ROOM);
		do_get(ch, "trofeo\0", CMD_GET);
	}
}



ACTION_FUNC(do_drop) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_drop (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> tmp{};
	std::array<char, MAX_INPUT_LENGTH> tmp2{};
	std::array<char, 1000> newarg{};
	int amount = 0;
	struct obj_data* tmp_object = nullptr;
	struct obj_data* next_obj = nullptr;
	bool test = false;
	int num = 0;
	int p = 0;

	argument_interpreter(arg, tmp.data(), tmp2.data());

	if(tmp[0] != '\0' && tmp2[0] != '\0') {
		if(is_number(tmp.data()) && (obj_prefix_equal(tmp2.data(), "coin") || obj_prefix_equal(tmp2.data(), "monet"))) {
			if((amount = atoi(tmp.data())) > 0) {
				if(GET_GOLD(ch) < amount) {
					send_to_char("Non hai tutte quelle monete!\n\r", ch);
				}
				else {

					tmp_object = create_money(amount);
					obj_to_room(tmp_object, ch->in_room);
					GET_GOLD(ch) -= amount;
					if(amount == 1) {
						send_to_char("Posi una moneta.\n\r", ch);
						act("$n posa una moneta.", true, ch, nullptr, nullptr, TO_ROOM);
					}
					else {
						send_to_char("Posi alcune monete.\n\r", ch);
						act("$n posa alcune monete.", true, ch, nullptr, nullptr, TO_ROOM);
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
	else if(tmp[0] != '\0') {
		if(obj_arg_is_all(tmp.data())) {
			for(tmp_object = ch->carrying;
					tmp_object;
					tmp_object = next_obj) {
				next_obj = tmp_object->next_content;
				if(!IS_OBJ_STAT(tmp_object, ITEM_NODROP) || IS_IMMORTAL(ch)) {
					/* if it is a limited items check if the PC EGO is
					   strong enough to control all the limited items
					   Gaia 2001 */
					if(IS_RARE(tmp_object) && !EgoSave(ch)) {
						act("Non hai la forza di posare $p.",false, ch, tmp_object, nullptr, TO_CHAR);
						return ;
					}
					obj_from_char(tmp_object);
					obj_to_room(tmp_object,ch->in_room);
					check_falling_obj(tmp_object, ch->in_room);
					test = true;
				}
				else {
					if(CAN_SEE_OBJ(ch, tmp_object)) {
						act("Non puoi posare $p.", false, ch, tmp_object, nullptr,
							TO_CHAR);
						test = true;
					}
				}
			}
			if(!test) {
				send_to_char("Non hai nulla da posare.\n\r", ch);
			}
			else {
				act("Posi tutto quello che hai.", false, ch, nullptr, nullptr, TO_CHAR);
				act("$n posa tutto quello che ha.", true, ch, nullptr, nullptr, TO_ROOM);
			}
#if NODUPLICATES
			do_save(ch, "", 0);
#endif
		}
		else {
			struct obj_data* pObjList;
			int bFound = false;
			/* &&&&&& */
			if(getall(tmp.data(), newarg.data()) == true) {
				num = -1;
				obj_copy_cstr(tmp.data(), tmp.size(), newarg.data());
			}
			else if((p = getabunch(tmp.data(), newarg.data())) != 0) {
				num = p;
				obj_copy_cstr(tmp.data(), tmp.size(), newarg.data());
			}
			else {
				num = 1;
			}

			pObjList = ch->carrying;

			while(num != 0) {
				tmp_object = get_obj_in_list_vis(ch, tmp.data(), pObjList);

				if(tmp_object) {
					pObjList = tmp_object->next_content;
					if(!IS_OBJ_STAT(tmp_object, ITEM_NODROP) || IS_IMMORTAL(ch)) {
						/* if it is a limited items check if the PC EGO is strong enough
						   Gaia 2001 */
						if(IS_RARE(tmp_object) && !EgoSave(ch)) {
							act("Non hai la forza di posare $p.",false, ch, tmp_object, nullptr, TO_CHAR);
							return ;
						}
						act("Posi $p.", false, ch, tmp_object, nullptr, TO_CHAR);
						act("$n posa $p.", true, ch, tmp_object, nullptr, TO_ROOM);
						obj_from_char(tmp_object);
						obj_to_room(tmp_object,ch->in_room);

						check_falling_obj(tmp_object, ch->in_room);

					}
					else {
						act("Non puoi posare $p.", false, ch, tmp_object, nullptr, TO_CHAR);
					}
					if(num > 0) {
						num--;
					}
					bFound = true;
				}
				else {
					break;
				}
			}
			if(!bFound) {
				std::ostringstream os;
				os << "Non hai nessun " << tmp.data() << ".\n\r";
				send_to_char(os.str().c_str(), ch);
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
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_put (act.obj.cpp)");
		return;
	}

	std::array<char, 128> arg1{};
	std::array<char, 128> arg2{};
	std::array<char, 100> newarg{};
	struct obj_data* obj_object = nullptr;
	struct obj_data* sub_object = nullptr;
	struct char_data* tmp_char = nullptr;
	int bits = 0;
	int num = 0;
	int p = 0;

	argument_interpreter(arg, arg1.data(), arg2.data());

	if(arg1[0] != '\0') {
		if(arg2[0] != '\0') {
			if(getall(arg1.data(), newarg.data()) == true) {
				num = -1;
				obj_copy_cstr(arg1.data(), arg1.size(), newarg.data());
			}
			else if((p = getabunch(arg1.data(), newarg.data())) != 0) {
				num = p;
				obj_copy_cstr(arg1.data(), arg1.size(), newarg.data());
			}
			else {
				num = 1;
			}

			if(obj_arg_is_all(arg1.data())) {
				send_to_char("Non puoi farlo.\n\r", ch);
				return;
			}

			while(num != 0) {
				bits = generic_find(arg1.data(), FIND_OBJ_INV, ch, &tmp_char, &obj_object);

				if(obj_object != nullptr) {
						if(IS_OBJ_STAT(obj_object, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
							act("Non puoi mettere $p da nessuna parte.", false, ch,
								obj_object, nullptr, TO_CHAR);
							return;
						}
						/* if it is a limited items check if the PC EGO is strong enough
						   Gaia 2001 */
						if(IS_RARE(obj_object) && !EgoSave(ch)) {
							act("Preferisci tenere $p vicino a te.",false, ch, obj_object, sub_object, TO_CHAR);
							return ;
						}

						bits = generic_find(arg2.data(), FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char,
						                    &sub_object);
						if(sub_object != nullptr) {
							if(GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
								if(!IS_SET(sub_object->obj_flags.value[ 1 ], CONT_CLOSED)) {
									if(obj_object == sub_object) {
										act("Giocherelli un po' con $p ma senza risultato.",
											false, ch, obj_object, nullptr, TO_CHAR);

										return;
									}
									if(((sub_object->obj_flags.weight) +
											(obj_object->obj_flags.weight)) <
											(sub_object->obj_flags.value[0])) {
										act("Metti $p in $P.",true, ch, obj_object, sub_object,
											TO_CHAR);
										if(bits==FIND_OBJ_INV) {
											obj_from_char(obj_object);
											obj_to_obj(obj_object, sub_object);
										}
										else {
											obj_from_room(obj_object);
											obj_to_obj(obj_object, sub_object);
										}

										act("$n mette $p in $P.",true, ch, obj_object, sub_object,
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
								act("$p non e' un contenitore.", false, ch, sub_object, nullptr,
									TO_CHAR);
								num = 0;
							}
						}
						else {
							std::ostringstream os;
							os << "Non hai nessun " << arg2.data() << ".\n\r";
							send_to_char(os.str().c_str(), ch);
							num = 0;
						}
					}
					else {
						if(num > 0 || num == -1) {
							std::ostringstream os;
							os << "Non hai nessun " << arg1.data() << ".\n\r";
							send_to_char(os.str().c_str(), ch);
						}
						num = 0;
					}
				}
#if NODUPLICATES
			do_save(ch, "", 0);
#endif
		}
		else {
			std::ostringstream os;
			os << "Dove vorresti mettere un " << arg1.data() << "?\n\r";
			send_to_char(os.str().c_str(), ch);
		}
	}
	else {
		send_to_char("Cosa vuoi mettere e dove?\n\r", ch);
	}
}

int newstrlen(const char* p) {
	int i;

	for(i=0; i<10&&*p; i++,p++);
	return(i);
}

void truegivexp(struct char_data* ch, struct char_data* victim, int amount) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	gain_exp(ch, -(amount / HowManyClasses(ch)));
	{
		std::ostringstream os;
		os << "Dai " << amount << " xp a $N.";
		act(os.str().c_str(), false, ch, nullptr, victim, TO_CHAR);
	}
	amount /= 100;
	amount *= (100 - number(3, 10));
	{
		std::ostringstream os;
		os << "$n ti da' " << amount << " xp.";
		act(os.str().c_str(), false, ch, nullptr, victim, TO_VICT);
	}
	GET_EXP(victim) += amount / HowManyClasses(victim);
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
			send_to_char("Che esagerazione... cosi' e' troppo!",ch);
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
			send_to_char("Che esagerazione... cosi' e' troppo!",ch);
		}

	}
	return;
}



ACTION_FUNC(do_give) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_give (act.obj.cpp)");
		return;
	}

	std::array<char, 200> obj_name{};
	std::array<char, 80> vict_name{};
	std::array<char, 80> tmp{};
	std::array<char, 100> newarg{};
	int amount = 0;
	int num = 0;
	int p = 0;
	int count = 0;
	struct char_data* vict = nullptr;
	struct obj_data* obj = nullptr;

	arg = one_argument(arg, obj_name.data());
	if(is_number(obj_name.data())) {
		if(newstrlen(obj_name.data()) >= 10) {
			obj_name[10] = '\0';
		}
		amount = atoi(obj_name.data());
		arg = one_argument(arg, tmp.data());
		if(!obj_prefix_equal(tmp.data(), "coin") && !obj_prefix_equal(tmp.data(), "monet") &&
		   !obj_prefix_equal(tmp.data(), "xp")) {
			send_to_char("Eh?\n\r", ch);
			return;
		}
		if(amount <= 0) {
			send_to_char("Non hai ben chiaro il valore delle cose.\n\r", ch);
			return;
		}

		arg = one_argument(arg, vict_name.data());

		if(vict_name[0] == '\0') {
			send_to_char("A chi vuoi dare delle monete?\n\r", ch);
		}
		else if((vict = get_char_room_vis(ch, vict_name.data())) == nullptr) {
			send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
		}
		else {
			if(obj_prefix_equal(tmp.data(), "xp")) {
				givexp(ch, vict, amount);
				return;
			}

			if(GET_GOLD(ch) < amount && (IS_NPC(ch) || GetMaxLevel(ch) < DIO)) {
				send_to_char("Non hai tutti quei soldi.\n\r", ch);
				return;
			}
			if(amount == 1) {
				act("$n ti da' una moneta d'oro.", false, ch, nullptr, vict, TO_VICT);
				act("Dai una moneta $N.", false, ch, nullptr, vict, TO_CHAR);
			}
			else {
				std::ostringstream osVict;
				osVict << "$n ti da' " << amount << " monete d'oro.";
				act(osVict.str().c_str(), false, ch, nullptr, vict, TO_VICT);
				std::ostringstream osChar;
				osChar << "Dai " << amount << " monete d'oro a $N.";
				act(osChar.str().c_str(), false, ch, nullptr, vict, TO_CHAR);
			}

			act("$n da' alcune monete a $N.", true, ch, nullptr, vict, TO_NOTVICT);
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
		arg = one_argument(arg, vict_name.data());

		if(obj_name[0] == '\0' || vict_name[0] == '\0') {
			send_to_char("Vuoi dare cosa a chi?\n\r", ch);
			return;
		}
		if(getall(obj_name.data(), newarg.data()) == true) {
			num = -1;
			obj_copy_cstr(obj_name.data(), obj_name.size(), newarg.data());
		}
		else if((p = getabunch(obj_name.data(), newarg.data())) != 0) {
			num = p;
			obj_copy_cstr(obj_name.data(), obj_name.size(), newarg.data());
		}
		else {
			num = 1;
		}

		count = 0;
		if((vict = get_char_room_vis(ch, vict_name.data())) != nullptr) {
			while(num != 0) {
				if((obj = get_obj_in_list_vis(ch, obj_name.data(), ch->carrying)) == nullptr) {
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
						act("Non vuoi separarti da $p.", false, ch, obj, vict, TO_CHAR);
						return ;
					}
					if(IS_RARE(obj) && !EgoSave(vict)) {
						act("$n cerca insistentemente di darti $p.", false, ch, obj, vict, TO_VICT);
						act("Cerchi di dare $p a $N ma lui lo rifiuta.", false, ch, obj, vict, TO_CHAR);
						act("$n cerca invano di dare $p a $N.", true, ch, obj, vict, TO_NOTVICT);
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

					if(IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
						act("$N sembra avere le mani piene.", false, ch, nullptr, vict, TO_CHAR);
						return;
					}
					if(obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
						act("$N non riesce a tenere $p, pesa troppo.", false, ch, obj, vict, TO_CHAR);
						return;
					}
					if(!IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && IS_OBJ_STAT2(obj, ITEM2_EDIT) &&
					   IS_MAESTRO_DEGLI_DEI(ch)) {
						pers_obj(ch, vict, obj, CMD_GIVE);
					}
					act("$n da' $p a $N.", true, ch, obj, vict, TO_NOTVICT);
					act("$n ti da' $p.", false, ch, obj, vict, TO_VICT);
					act("Dai $p a $N.", false, ch, obj, vict, TO_CHAR);
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
			send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
		}
	}
}


ACTION_FUNC(do_pquest) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_pquest (act.obj.cpp)");
		return;
	}

	std::array<char, 200> obj_name{};
	std::array<char, 80> vict_name{};
	int punti_quest = 0;
	int old_punti = 0;
	struct char_data* vict = nullptr;

	arg = one_argument(arg, obj_name.data());
	if(is_number(obj_name.data())) {
		if(newstrlen(obj_name.data()) >= 10) {
			obj_name[10] = '\0';
		}
		punti_quest = atoi(obj_name.data());
		arg = one_argument(arg, vict_name.data());

		if(punti_quest == 0) {
			if((vict = get_char_room_vis(ch, vict_name.data())) == nullptr) {
				send_to_char("Di chi vuoi conoscere le rune degli Dei?\n\r", ch);
				return;
			}
			std::ostringstream os;
			os << GET_NAME(vict) << " ha " << GET_RUNEDEI(vict) << " rune degli Dei.\n\r";
			send_to_char(os.str().c_str(), ch);
			return;
		}

		if(punti_quest > 0) {
			if(vict_name[0] == '\0') {
				send_to_char("A chi vuoi assegnare rune degli Dei?\n\r", ch);
				return;
			}
			else if((vict = get_char_room_vis(ch, vict_name.data())) == nullptr) {
				send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
				return;
			}
			else {
				old_punti = GET_RUNEDEI(vict);
				if(GET_RUNEDEI(ch) < punti_quest && (IS_NPC(ch) || GetMaxLevel(ch) < IMMORTALE)) {
					send_to_char("Non puoi assegnare rune degli Dei!!\n\r", ch);
					return;
				}
				if(punti_quest == 1) {
					act("Fiamme divine ti avvolgono e piano piano si concentrano in una zona del tuo corpo!",
					    false, ch, nullptr, vict, TO_VICT);
					act("$n ti tatua una runa degli Dei sul corpo!", false, ch, nullptr, vict, TO_VICT);
					std::ostringstream os;
					os << "Assegni una runa dagli Dei a $N, ne aveva " << old_punti << '.';
					act(os.str().c_str(), false, ch, nullptr, vict, TO_CHAR);
				}
				else {
					act("Fiamme divine ti avvolgono e piano piano si concentrano in diverse zone del tuo corpo!",
					    false, ch, nullptr, vict, TO_VICT);
					std::ostringstream osVict;
					osVict << "$n ti tatua " << punti_quest << " rune degli Dei.";
					act(osVict.str().c_str(), false, ch, nullptr, vict, TO_VICT);
					std::ostringstream osChar;
					osChar << "Assegni " << punti_quest << " rune degli Dei a $N, ne aveva " << old_punti
					       << '.';
					act(osChar.str().c_str(), false, ch, nullptr, vict, TO_CHAR);
				}
				act("Ad un gesto della mano di $n, fiamme divine avvolgono il corpo di $N.", true, ch,
				    nullptr, vict, TO_NOTVICT);
				act("$n tatua delle rune degli Dei sul corpo di $N.", true, ch, nullptr, vict,
				    TO_NOTVICT);
				if(IS_NPC(ch) || GetMaxLevel(ch) <= QUESTMASTER) {
					GET_RUNEDEI(ch) -= punti_quest;
				}
				GET_RUNEDEI(vict) += punti_quest;
				save_char(ch, AUTO_RENT, 0);
				mudlog(LOG_PLAYERS, "%s assegna %d rune degli Dei a %s, ne aveva %d", GET_NAME(ch), punti_quest, GET_NAME(vict), old_punti);
				return;
			}
		}

		if(punti_quest < 0) {
			if(vict_name[0] == '\0') {
				send_to_char("A chi vuoi togliere rune degli Dei?\n\r", ch);
				return;
			}
			else if((vict = get_char_room_vis(ch, vict_name.data())) == nullptr) {
				send_to_char("Non vedi nessuno con quel nome.\n\r", ch);
				return;
			}
			else {
				old_punti = GET_RUNEDEI(vict);
				if(GET_RUNEDEI(ch) < -punti_quest && (IS_NPC(ch) || GetMaxLevel(ch) < IMMORTALE)) {
					send_to_char("Non puoi togliere rune degli Dei!!\n\r", ch);
					return;
				}
				if((old_punti + punti_quest) < 0) {
					send_to_char("Non puoi togliere quello che non si ha!! Hai esagerato!?!\n\r", ch);
					return;
				}
				if(punti_quest == -1) {
					act("$n fa un gesto con la mano, ed una runa tatuata sul tuo corpo sbiadice lentamente "
					    "fino a scomparire!",
					    false, ch, nullptr, vict, TO_VICT);
					std::ostringstream os;
					os << "Togli una runa degli Dei a $N, ne aveva " << old_punti << '.';
					act(os.str().c_str(), false, ch, nullptr, vict, TO_CHAR);
				}
				else {
					std::ostringstream osVict;
					osVict << "$n fa un gesto con la mano e " << -punti_quest
					       << " rune che avevi tatuate lentamente sbiadiscono fino a scomparire.";
					act(osVict.str().c_str(), false, ch, nullptr, vict, TO_VICT);
					std::ostringstream osChar;
					osChar << "Togli " << -punti_quest << " rune degli Dei a $N, ne aveva " << old_punti
					       << '.';
					act(osChar.str().c_str(), false, ch, nullptr, vict, TO_CHAR);
				}
				act("Ad un gesto della mano di $n, alcune rune tetuate sul corpo di $N lentamente sbiadisco.",
				    true, ch, nullptr, vict, TO_NOTVICT);
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





void weight_change_object(struct obj_data* obj, int weight) {
	struct obj_data* tmp_obj;
	struct char_data* tmp_ch;

	if(GET_OBJ_WEIGHT(obj) + weight < 1) {
		weight = 0 - (GET_OBJ_WEIGHT(obj) -1);
		mudlog(LOG_ERROR, "Bad weight change on %s, carried by %s.",
			   obj->name,
			   (obj->carried_by ? GET_NAME_DESC(obj->carried_by) : "none"));
	}

	if(obj->in_room != NOWHERE) {
		GET_OBJ_WEIGHT(obj) += weight;
	}
	else if((tmp_ch = obj->carried_by)) {
		obj_from_char(obj);
		GET_OBJ_WEIGHT(obj) += weight;
		obj_to_char(obj, tmp_ch);
	}
	else if((tmp_obj = obj->in_obj)) {
		obj_from_obj(obj);
		GET_OBJ_WEIGHT(obj) += weight;
		obj_to_obj(obj, tmp_obj);
	}
	else {
		mudlog(LOG_ERROR, "Unknown attempt to subtract weight from an object.");
	}
}



void name_from_drinkcon(struct obj_data* obj) {
	if(obj == nullptr || obj->name == nullptr) {
		return;
	}
	char* p = obj->name;
	for(; *p != '\0' && *p != ' '; ++p) {
	}
	if(*p == ' ') {
		obj_replace_dup_cstr(obj->name, p + 1);
	}
}

void name_to_drinkcon(struct obj_data* obj, int type) {
	if(obj == nullptr || obj->name == nullptr) {
		return;
	}
	std::ostringstream os;
	os << drinknames[type] << ' ' << obj->name;
	obj_replace_dup_cstr(obj->name, os.str());
}



ACTION_FUNC(do_drink) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_drink (act.obj.cpp)");
		return;
	}

	struct obj_data* temp;
	struct affected_type af;
	int amount;

	std::array<char, MAX_INPUT_LENGTH> argBuf{};
	only_argument(arg, argBuf.data());

	if(!(temp = get_obj_in_list_vis(ch, argBuf.data(), ch->carrying))) {
		act("Non hai nulla del genere con te!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag!=ITEM_DRINKCON) {
		act("Non riesci a bere da $p!", false, ch, temp, nullptr, TO_CHAR);
		return;
	}

	if((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0)) {
		/* The pig is drunk */
		act("Hai bevuto troppo! Ti senti schifat$b.", false, ch, nullptr, nullptr, TO_CHAR);
		act("$n e' MOLTO ubriac$b.", true, ch, nullptr, nullptr, TO_ROOM);
		return;
	}

	if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) { /* Stomach full */
		act("Sei pien$b, non riesci a bere ulteriormente!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag == ITEM_DRINKCON) {
		if(temp->obj_flags.value[1]>0) {  /* Not empty */
			const char* const articolo = obj_liquid_article_for(temp->obj_flags.value[2]);
			const char* const drinkName = drinks[temp->obj_flags.value[2]];
			{
				std::ostringstream os;
				os << "$n beve " << articolo << drinkName << " da $p.";
				act(os.str().c_str(), true, ch, temp, nullptr, TO_ROOM);
			}
			{
				std::ostringstream os;
				os << "Bevi " << articolo << drinkName << ".\n\r";
				send_to_char(os.str().c_str(), ch);
			}

			if(drink_aff[temp->obj_flags.value[2]][DRUNK] > 0)
				amount = (25-GET_COND(ch,THIRST))/
						 drink_aff[temp->obj_flags.value[2]][DRUNK];
			else {
				amount = number(3,10);
			}

			amount = MIN(amount,temp->obj_flags.value[1]);

			gain_condition(ch,DRUNK,(int)((int)drink_aff
										  [temp->obj_flags.value[2]][DRUNK]*amount)/4);

			gain_condition(ch,FULL,(int)((int)drink_aff
										 [temp->obj_flags.value[2]][FULL]*amount)/4);

			gain_condition(ch,THIRST,(int)((int)drink_aff
										   [temp->obj_flags.value[2]][THIRST]*amount)/4);

			if(GET_COND(ch,DRUNK)>10) {
				act("Ti senti ubriac$b.", false, ch, nullptr, nullptr,TO_CHAR);
			}

			if(GET_COND(ch,THIRST)>20) {
				act("Hai placato la tua sete.", false, ch, nullptr, nullptr,TO_CHAR);
			}

			if(GET_COND(ch,FULL)>20) {
				act("Sei pien$b.", false, ch, nullptr, nullptr,TO_CHAR);
			}

			/* The shit was poisoned ! */
			if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)) {
				act("Oops, ha un sapore piuttosto strano vero?!?", false, ch, nullptr, nullptr,TO_CHAR);
				act("$n soffoca ed emette strani suoni.", true, ch, nullptr, nullptr, TO_ROOM);
				af.type = SPELL_POISON;
				af.duration = amount*3;
				af.modifier = 0;
				af.location = APPLY_NONE;
				af.bitvector = AFF_POISON;
				affect_join(ch,&af, false, false);
				GET_HIT(ch) -= 1 ;   // Gaia 2001
				alter_hit(ch, 0);    // Gaia 2001
			}

			/* Subtract amount, if not a never-emptying container */
			if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM)) {
				weight_change_object(temp, - amount);
				temp->obj_flags.value[1] -= amount;
			}

			if(!temp->obj_flags.value[1]) {
				/* The last bit */
				/* empty the container, and no longer poison. */
				temp->obj_flags.value[2]=0;
				temp->obj_flags.value[3]=0;
				name_from_drinkcon(temp);
				/* Ho sempre odiato il fatto che i bicchieri vuoti sparissero.... */
#if 0
				if(temp->obj_flags.value[0] < 20) {
					extract_obj(temp);  /* get rid of it */
				}
#endif
			}
		}
		else {
			act("Non c'e' nulla in $p.", false, ch, temp, nullptr, TO_CHAR);
		}
	}
}



ACTION_FUNC(do_eat) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_eat (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	int j = 0;
	int num = 0;
	struct obj_data* temp;
	struct affected_type af;

	one_argument(arg, buf.data());

	if(!(temp = get_obj_in_list_vis(ch, buf.data(), ch->carrying)))  {
		act("Cosa vuoi mangiare?", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if((temp->obj_flags.type_flag != ITEM_FOOD) &&
			(GetMaxLevel(ch) < IMMORTALE)) {
		act("Il tuo stomaco si rifiuta di mangiare quella cosa li'!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(GET_COND(ch,FULL)>20) { /* Stomach full */
		act("Sei troppo pien$b per mangiare altro!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	act("$n mangia $p.", true, ch, temp, nullptr,TO_ROOM);
	act("Mangi $p.", false, ch, temp, nullptr,TO_CHAR);

	gain_condition(ch,FULL,temp->obj_flags.value[0]);

	if(GET_COND(ch,FULL)>20) {
		act("Sei pien$b.", false, ch, nullptr, nullptr,TO_CHAR);
	}

	for(j=0; j<MAX_OBJ_AFFECT; j++)
		if(temp->affected[j].location == APPLY_EAT_SPELL) {
			num = temp->affected[j].modifier;

			/* hit 'em with the spell */

			((*spell_info[num].spell_pointer)
			 (6, ch, "", SPELL_TYPE_POTION, ch, 0));
		}

	if(temp->obj_flags.value[3] && (GetMaxLevel(ch) < IMMORTALE)) {
		act("Che strano sapore!", false, ch, nullptr, nullptr,TO_CHAR);
		act("$n tossisce ed emette strani suoni.",
			false, ch, nullptr, nullptr, TO_ROOM);

		af.type = SPELL_POISON;
		af.duration = temp->obj_flags.value[0]*2;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = AFF_POISON;
		affect_join(ch,&af, false, false);
		GET_HIT(ch) -= 1 ;   // Gaia 2001
		alter_hit(ch, 0);    // Gaia 2001
	}

	extract_obj(temp);
}


ACTION_FUNC(do_pour) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_pour (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	struct obj_data* from_obj;
	struct obj_data* to_obj;
	int temp;

	argument_interpreter(arg, arg1.data(), arg2.data());

	if(arg1[0] == '\0') { /* No arguments */
		act("Cosa vuoi svuotare?", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(!(from_obj = get_obj_in_list_vis(ch, arg1.data(), ch->carrying))) {
		act("Non hai niente del genere!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON) {
		act("Non puoi svuotare $p!", false, ch, from_obj, nullptr, TO_CHAR);
		return;
	}

	if(from_obj->obj_flags.value[1]==0) {
		act("Non puoi svuotare $p ulteriormente!",false, ch, from_obj, nullptr,TO_CHAR);
		return;
	}

	if(arg2[0] == '\0') {
		act("Dove vuoi svuotare $p? Dentro cosa o fuori (out)?", false, ch, from_obj, nullptr,TO_CHAR);
		return;
	}

	if(obj_ci_equal(arg2.data(), "out")) {
		act("$n svuota $p", true, ch, from_obj, nullptr,TO_ROOM);
		act("Hai svuotato $p.", false, ch, from_obj, nullptr,TO_CHAR);

		weight_change_object(from_obj, -from_obj->obj_flags.value[1]);

		from_obj->obj_flags.value[1]=0;
		from_obj->obj_flags.value[2]=0;
		from_obj->obj_flags.value[3]=0;
		name_from_drinkcon(from_obj);

		return;

	}

	if(!(to_obj = get_obj_in_list_vis(ch, arg2.data(), ch->carrying))) {
		act("Non hai niente del genere!", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON) {
		act("Non puoi farlo, non e' un contenitore per bevande.", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if((to_obj->obj_flags.value[1]!=0)&&
			(to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2])) {
		act("$p contiene un liquido diverso!", false, ch, to_obj, nullptr, TO_CHAR);
		return;
	}

	if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0])) {
		act("Non c'e' abbastanza spazio.", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	{
		const char* const articolo = obj_liquid_article_for(from_obj->obj_flags.value[2]);
		std::ostringstream os;
		os << "Riempi $p con " << articolo << drinks[from_obj->obj_flags.value[2]] << '.';
		act(os.str().c_str(), false, ch, to_obj, nullptr, TO_CHAR);
	}
	act("$n riempie $p.", false, ch, to_obj, nullptr, TO_ROOM);

	/* New alias */
	if(to_obj->obj_flags.value[1]==0) {
		name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);
	}

	/* First same type liq. */
	to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

	/*
	 * Il nuovo, migliore modo di farlo... :)
	 */
	temp = MIN(from_obj->obj_flags.value[1], to_obj->obj_flags.value[0] -
			   to_obj->obj_flags.value[1]);

	/* Se l'oggetto from_obj e' di tipo DRINK_PERM, in questo modo puo' essere
	 * svuotato. Devo decidere se e' una feature od un bug :) Per il momento
	 * rimane cosi', perche' ha vantaggi e svantaggi per il giocatore.
	 */
	from_obj->obj_flags.value[1] -= temp;
	to_obj->obj_flags.value[1] += temp;
	weight_change_object(to_obj, temp);
	weight_change_object(from_obj, -temp);


	/* Se il from_obj era avvelenato, ovviemente anche il to_obj sara'
	 * avvelenato.
	 */
	if(IS_SET(from_obj->obj_flags.value[3], DRINK_POISON)) {
		SET_BIT(to_obj->obj_flags.value[3], DRINK_POISON);
	}

	if(!from_obj->obj_flags.value[1]) {
		from_obj->obj_flags.value[2]=0;
		from_obj->obj_flags.value[3]=0;
		name_from_drinkcon(from_obj);
	}

	return;
}

ACTION_FUNC(do_sip) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_sip (act.obj.cpp)");
		return;
	}

	struct affected_type af;
	std::array<char, MAX_STRING_LENGTH> tmp{};
	struct obj_data* temp;

	one_argument(arg, tmp.data());

	if(!(temp = get_obj_in_list_vis(ch, tmp.data(), ch->carrying)))    {
		act("Vuoi bere un sorso di cosa?", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag!=ITEM_DRINKCON)    {
		act("Non puoi bere niente da $p!", false, ch, temp, nullptr, TO_CHAR);
		return;
	}

	if(GET_COND(ch,DRUNK)>10) { /* The pig is drunk ! */
		act("Ma se non riesci neppure a raggiungere la tua bocca!", false, ch, nullptr, nullptr,TO_CHAR);
		act("$n non riesce a raggiungere la sua bocca con $p!", true, ch, temp, nullptr, TO_ROOM);
		return;
	}

	if(!temp->obj_flags.value[1]) { /* Empty */
		act("Ma se non c'e' niente...", false, ch, nullptr, nullptr,TO_CHAR);
		return;
	}

	act("$n beve un sorso da $p.", true, ch, temp, nullptr,TO_ROOM);
	{
		std::ostringstream os;
		os << "Sa di " << drinks[temp->obj_flags.value[2]] << ".\n\r";
		send_to_char(os.str().c_str(), ch);
	}

	gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]]
								  [DRUNK]/4));

	gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

	gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]]
								   [THIRST]/4));


	if(GET_COND(ch,DRUNK)>10) {
		act("Ti senti ubriac$b.", false, ch, nullptr, nullptr,TO_CHAR);
	}

	if(GET_COND(ch,THIRST)>20) {
		act("Hai placato la tua sete.", false, ch, nullptr, nullptr,TO_CHAR);
	}

	if(GET_COND(ch,FULL)>20) {
		act("Ti senti pien$b.", false, ch, nullptr, nullptr,TO_CHAR);
	}

	if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)
			&& !IS_AFFECTED(ch,AFF_POISON)) {
		/* The shit was poisoned ! */
		act("Ha anche uno strano sapore!", false, ch, nullptr, nullptr,TO_CHAR);

		af.type = SPELL_POISON;
		af.duration = 3;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = AFF_POISON;
		affect_to_char(ch,&af);
	}

	if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM)) {
		temp->obj_flags.value[1]--;
		weight_change_object(temp, -1); /* Subtract one unit, unless permanent */
	}

	if(!temp->obj_flags.value[1]) { /* The last bit */
		temp->obj_flags.value[2]=0;
		temp->obj_flags.value[3]=0;
		name_from_drinkcon(temp);
	}

	return;

}


ACTION_FUNC(do_taste) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_taste (act.obj.cpp)");
		return;
	}

	struct affected_type af;
	std::array<char, MAX_INPUT_LENGTH> tmp{};
	struct obj_data* temp;

	one_argument(arg, tmp.data());

	if(!(temp = get_obj_in_list_vis(ch, tmp.data(), ch->carrying)))    {
		act("Cosa vorresti assaggiare?", false, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag==ITEM_DRINKCON)    {
		do_sip(ch,arg,0);
		return;
	}

	if(!(temp->obj_flags.type_flag==ITEM_FOOD))    {
		act("Non puoi mangiare $p!", false, ch, temp, nullptr, TO_CHAR);
		return;
	}

	act("$n assaggia $p.", false, ch, temp, nullptr, TO_ROOM);
	act("Assaggi $p.", false, ch, temp, nullptr, TO_CHAR);

	gain_condition(ch,FULL,1);

	if(GET_COND(ch,FULL)>20) {
		act("Sei pien$b.", false, ch, nullptr, nullptr, TO_CHAR);
	}

	if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON))     {
		act("Ooups, $p non ha proprio un buon sapore!", false, ch, temp, nullptr, TO_CHAR);

		af.type = SPELL_POISON;
		af.duration = 2;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = AFF_POISON;
		affect_to_char(ch,&af);
	}

	temp->obj_flags.value[0]--;

	if(!temp->obj_flags.value[0]) {       /* Nothing left */
		act("Era l'ultimo pezzetto!", false, ch, nullptr, nullptr,TO_CHAR);
		extract_obj(temp);
	}

	return;

}



ACTION_FUNC(do_wield) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wield (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	struct obj_data* obj_object;
	int keyword = 12;

	argument_interpreter(arg, arg1.data(), arg2.data());
	if(arg1[0] != '\0') {
		obj_object = get_obj_in_list_vis(ch, arg1.data(), ch->carrying);
		if(obj_object) {
			wear(ch, obj_object, keyword);
		}
		else {
			std::string msg = "Non sembra che ti abbia un '";
			msg += arg1.data();
			msg += "'.\n\r";
			send_to_char(msg.c_str(), ch);
		}
	}
	else {
		send_to_char("Cosa vuoi impugnare?\n\r", ch);
	}
}


ACTION_FUNC(do_grab) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_grab (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	struct obj_data* obj_object;

	argument_interpreter(arg, arg1.data(), arg2.data());

	if(arg1[0] != '\0') {
		obj_object = get_obj_in_list(arg1.data(), ch->carrying);
		if(obj_object) {
			if(obj_object->obj_flags.type_flag == ITEM_LIGHT) {
				wear(ch, obj_object, WEAR_LIGHT);
			}
			else {
				wear(ch, obj_object, 13);
			}
		}
		else {
			std::ostringstream os;
			os << "Non sembra che tu abbia un '" << arg1.data() << "'.\n\r";
			send_to_char(os.str().c_str(), ch);
		}
	}
	else {
		send_to_char("Cosa vuoi usare?\n\r", ch);
	}
}


ACTION_FUNC(do_remove) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_remove (act.obj.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> arg1{};
	char* T = nullptr;
	char* P = nullptr;
	std::array<int, 20> Rem_List{};
	int Num_Equip = 0;
	struct obj_data* obj_object;
	struct obj_data* loaded_object = nullptr;    /* Gaia 2001 */
	int j = 0;

	one_argument(arg, arg1.data());

	if(arg1[0] != '\0') {
		if(obj_arg_is_all(arg1.data()))
			/* elimino il remove all in combattimento Gaia 2001 */
		{
			if(GET_POS(ch) == POSITION_FIGHTING) {
				send_to_char("Non riesci a spogliarti mentre combatti!\n\r",ch);
				return ;
			}
			for(j=0; j<MAX_WEAR; j++) {
				if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
					if(ch->equipment[j]) {
						if(IS_OBJ_STAT(ch->equipment[j],ITEM_NODROP) && !IS_IMMORTAL(ch)) {   // SALVO rem all non toglie obj cursato
							send_to_char("Non puoi lasciarlo andare, deve essere stregato!\n\r", ch);
						}
						else if((obj_object = unequip_char(ch,j))!=nullptr) {
							/* Introduced to remove loaded weapons Gaia 2001 */
							if(ch->equipment[LOADED_WEAPON]) {
								if((loaded_object = unequip_char(ch,LOADED_WEAPON))!=nullptr) {
									obj_to_char(loaded_object, ch);
									act("Smetti di usare $p.", false, ch, loaded_object, nullptr,TO_CHAR);
								}
							}
							obj_to_char(obj_object, ch);

							act("Smetti di usare $p.",false,ch, obj_object, nullptr,TO_CHAR);
							if(j == WEAR_LIGHT &&
									obj_object->obj_flags.type_flag == ITEM_LIGHT &&
									obj_object->obj_flags.value[ 2 ]) {
								real_roomp(ch->in_room)->light--;
							}

						}
					}
				}
				else {
					send_to_char("Non puoi trasportare altro.\n\r",ch);
					j = MAX_WEAR;
				}
			}
			act("$n smette di usare il suo equipaggiamento.", true, ch, nullptr,
				nullptr, TO_ROOM);
			return;
		}
		if(isdigit(arg1[ 0 ])) {
			/* PAT-PAT-PAT */

			/* Make a list of item numbers for stuff to remove */

			for(Num_Equip = j = 0; j< MAX_WEAR; j++) {
				if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
					if(ch->equipment[ j ]) {
						Rem_List[Num_Equip++] = j;
					}
				}
			}

			T = arg1.data();

			while(isdigit(*T) && (*T != '\0')) {
				P = T;
				if(strchr(T, ',')) {
					P = strchr(T, ',');
					*P = '\0';
				}
				if(atoi(T) > 0 && atoi(T) <= Num_Equip) {
					if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
						j = Rem_List[ atoi(T) - 1 ];
						if(ch->equipment[ j ]) {
							if((obj_object = unequip_char(ch, j)) != nullptr) {
								obj_to_char(obj_object, ch);
								act("Smetti di usare $p.",false,ch, obj_object, nullptr,TO_CHAR);
								act("$n smette di usare $p.",true,ch, obj_object, nullptr,TO_ROOM);

								if(j == WEAR_LIGHT &&
										obj_object->obj_flags.type_flag == ITEM_LIGHT &&
										obj_object->obj_flags.value[2]) {
									real_roomp(ch->in_room)->light--;
								}

							}
						}
					}
					else {
						send_to_char("Non riesci a trasportare altra roba.\r",ch);
						j = MAX_WEAR;
					}
				}
				else {
					std::ostringstream os;
					os << "Sembra che tu non abbia un " << T << ".\n\r";
					send_to_char(os.str().c_str(), ch);
				}
				if(T != P) {
					T = P + 1;
				}
				else {
					*T = '\0';
				}
			}
		}
		else {
			obj_object = get_object_in_equip_vis(ch, arg1.data(), ch->equipment, &j);
			if(obj_object) {
				if(IS_OBJ_STAT(obj_object,ITEM_NODROP) && !IS_IMMORTAL(ch)) {
					send_to_char("Non puoi lasciarlo andare, deve essere stregato!\n\r",
								 ch);
					return;
				}
#if EGO
				/* if it is a limited items check if the PC EGO is strong enough
				   Gaia 2001 */
				if(IS_RARE(obj_object) && !EgoSave(ch)) {
					act("$p ti sta troppo bene addosso!", false, ch, obj_object, nullptr, TO_CHAR);
					return ;
				}
#endif
				if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {

					obj_to_char(unequip_char(ch, j), ch);

					act("Smetti di usare $p.",false,ch, obj_object, nullptr,TO_CHAR);
					act("$n smette di usare $p.",true,ch, obj_object, nullptr,TO_ROOM);

					if(j == WEAR_LIGHT &&
							obj_object->obj_flags.type_flag == ITEM_LIGHT &&
							obj_object->obj_flags.value[2]) {
						real_roomp(ch->in_room)->light--;
					}

				}
				else {
					send_to_char("Non puoi trasportare altra roba.\n\r", ch);
				}
			}
			else {
				send_to_char("Non lo stai usando.\n\r", ch);
			}
		}
	}
	else {
		send_to_char("Cosa vuoi rimuovere?\n\r", ch);
	}

	check_falling(ch);
}

} // namespace Alarmud
