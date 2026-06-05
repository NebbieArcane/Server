/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include <array>
#include <sstream>
#include <string>
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "act.obj_common.hpp"
#include "act.obj_wear.hpp"
#include "act.obj_wear_infer.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "utility.hpp"

namespace Alarmud {

int obj_infer_wear_keyword(const struct obj_data* obj) {
	if(obj == nullptr) {
		return kObjWearKeywordNone;
	}
	return obj_infer_wear(obj->obj_flags.wear_flags, obj->obj_flags.type_flag).keyword;
}

int obj_infer_wear_equip_pos(const struct obj_data* obj) {
	if(obj == nullptr) {
		return kObjWearEquipPosNone;
	}
	return obj_infer_wear(obj->obj_flags.wear_flags, obj->obj_flags.type_flag).equipPos;
}

/* functions related to wear */

void perform_wear(struct char_data* ch, struct obj_data* obj_object,
				  long keyword) {
	switch(keyword) {
	case 0 :
		if(obj_object->obj_flags.value[2] != -1) {
			act("$n accende $p.", false, ch, obj_object, nullptr,TO_ROOM);
		}
		else {
			act("$n usa $p.", false, ch, obj_object, nullptr,TO_ROOM);
		}
		break;
	case 1 :
		act("$n indossa $p su un dito.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 2 :
		act("$n mette $p intorno al collo.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 3 :
		act("$n indossa $p sul corpo.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 4 :
		act("$n mette $p in testa.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 5 :
		act("$n indossa $p sulle gambe.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 6 :
		act("$n si mette $p ai piedi.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 7 :
		act("$n mette $p sulle mani.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 8 :
		act("$n indossa $p sulle braccia.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 9 :
		act("$n si avvolge con $p.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 10 :
		act("$n avvolge $p alla vita.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 11 :
		act("$n mette $p intorno ad un polso.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 12 :
		act("$n impugna $p.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 13 :
		act("$n prende $p in mano.",true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 14 :
		act("$n utilizza $p.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 15:
		act("$n si mette $p sulle spalle.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 16:
		act("$n appende $p ad un orecchio.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	case 17:
		act("$n si mette $p sugli occhi.", true, ch, obj_object, nullptr,TO_ROOM);
		break;
	}
}


int IsRestricted(struct obj_data* obj, int Class) {
	int Mask = GetItemClassRestrictions(obj);
	for(unsigned long i = CLASS_MAGIC_USER; i<= CLASS_PSI; i*=2) {
		if(IS_SET(i, Mask) && (!IS_SET(i, Class))) {
			Mask -= i;
		} /* if */
	} /* for */

	if(Mask == Class  && !IS_SET(obj->obj_flags.extra_flags, ITEM_ONLY_CLASS)) {
		return(true);
	}

	return(false);
}


void wear(struct char_data* ch, struct obj_data* obj_object, long keyword) {
	int BitMask;
	struct room_data* rp;
    struct char_data* tch;

    tch = ch;

    if(IS_POLY(ch))
    {
        tch = ch->desc->original;
    }

    if(IS_OBJ_STAT2(obj_object, ITEM2_PERSONAL) && !IS_IMMORTAL(ch) && !pers_on(ch,obj_object))
    {
		std::ostringstream os;
		os << "Non puoi usare " << obj_object->short_description << ", non ti appartiene!\n\r";
		send_to_char(os.str().c_str(), ch);
        return;
    }

    if(IS_OBJ_STAT2(obj_object, ITEM2_NO_PRINCE) && (IS_PRINCE(ch) || (IS_POLY(ch) && IS_PRINCE(ch->desc->original))) && !IS_IMMORTAL(ch))
    {
		std::ostringstream os;
		os << "Sei troppo potente per usare " << obj_object->short_description << "!\n\r";
		send_to_char(os.str().c_str(), ch);
        return;
    }

    if(IS_OBJ_STAT2(obj_object, ITEM2_ONLY_PRINCE) && (!IS_PRINCE(ch) && (IS_POLY(ch) && !IS_PRINCE(ch->desc->original))) && !IS_IMMORTAL(ch))
    {
		std::ostringstream os;
		os << obj_object->short_description << " e' troppo potente per te!\n\r";
		send_to_char(os.str().c_str(), ch);
        return;
    }

	if(!IS_IMMORTAL(ch) && IS_PC(ch)) {

		BitMask = GetItemClassRestrictions(obj_object);
		/* only class items */

		if(IS_SET(obj_object->obj_flags.extra_flags,ITEM_ONLY_CLASS)) {
			if(IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
				BitMask |= CLASS_SORCERER;
			}

			if(!OnlyClass(tch, BitMask)) {
				/* check here only for class restricts */
				if(obj_object->szForbiddenWearToChar &&
						*obj_object->szForbiddenWearToChar) {
					act(obj_object->szForbiddenWearToChar, false, ch, obj_object, nullptr,
						TO_CHAR);
				}
				else {
					std::ostringstream os;
					os << "Non sei la persona adatta ad usare " << obj_object->short_description
					   << ".\n\r";
					send_to_char(os.str().c_str(), ch);
				}
				if(obj_object->szForbiddenWearToRoom &&
						*obj_object->szForbiddenWearToRoom) {
					act(obj_object->szForbiddenWearToRoom, true, ch, obj_object, nullptr,
						TO_ROOM);
				}
				return;
			}
		}
		else if(IsRestricted(obj_object, tch->player.iClass))
			/* not only-class, okay to check normal anti-settings */
		{
			if(obj_object->szForbiddenWearToChar &&
					*obj_object->szForbiddenWearToChar) {
				act(obj_object->szForbiddenWearToChar, false, ch, obj_object, nullptr,
					TO_CHAR);
			}
			else {
				std::ostringstream os;
				os << "Non riesci ad usare " << obj_object->short_description << ".\n\r";
				send_to_char(os.str().c_str(), ch);
			}
			if(obj_object->szForbiddenWearToRoom &&
					*obj_object->szForbiddenWearToRoom) {
				act(obj_object->szForbiddenWearToRoom, true, ch, obj_object, nullptr,
					TO_ROOM);
			}
			return;
		}
	}


	if(anti_barbarian_stuff(obj_object) &&
			GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
			GetMaxLevel(ch) < IMMORTALE && IS_PC(ch)) {
		act("Eck! Non questo! Percepisci la magia su $p e lo "
			"getti via schifato!", false, ch, obj_object, nullptr, TO_CHAR);
		act("$n rabbrividisce e getta $p!", false, ch, obj_object, nullptr, TO_ROOM);
		obj_from_char(obj_object);
		obj_to_room(obj_object, ch->in_room);
		return;
	}

	if(IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_MEN)) {
		if(GET_SEX(ch) != SEX_FEMALE) {
			act("Solo le femmine possono utilizzare $p.", false, ch, obj_object,
				nullptr, TO_CHAR);
			return;
		}
	}

	if(IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_WOMEN)) {
		if(GET_SEX(ch) != SEX_MALE) {
			act("Solo i maschi possono utilizzare $p.", false, ch, obj_object,
				nullptr, TO_CHAR);
			return;
		}
	}


	rp = real_roomp(ch->in_room);

	if(!IsHumanoid(ch)) {
		if((keyword != 13) || (!HasHands(ch))) {
			act("Non puoi indossare $p! E' per umanoidi.", false, ch, obj_object,
				nullptr, TO_CHAR);
			return;
		}
	}

	switch(keyword) {
	case 0:  /* LIGHT SOURCE */
		if(ch->equipment[WEAR_LIGHT]) {
			send_to_char("Stai gia' usando una sorgente di luce.\n\r", ch);
		}
		else if(ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight >
				str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
			send_to_char("Non puoi impugnare un'arma a due mani ed usare una "
						 "sorgente di luce.\n\r", ch);
		}
		else if((ch->equipment[WIELD] && ch->equipment[HOLD])) {
			send_to_char("Hai solo due mani.\n\r", ch);
		}
		else if(rp->sector_type == SECT_UNDERWATER &&
				obj_object->obj_flags.value[2] != -1) {
			send_to_char("Non puoi accenderla sott'acqua!\n\r", ch);
		}
		else {
			if(obj_object->obj_flags.value[2]) {
				real_roomp(ch->in_room)->light++;
				if(real_roomp(ch->in_room)->light < 1) {
					real_roomp(ch->in_room)->light = 1;
				}

				if(obj_object->obj_flags.value[2] != -1) {
					act("Accendi $p.", false, ch, obj_object, nullptr, TO_CHAR);
				}
				else {
					act("Usi $p.", false, ch, obj_object, nullptr, TO_CHAR);
				}
				perform_wear(ch,obj_object,keyword);
			}
			else {
				act("Non ti sembra che $p illumini molto.", false, ch, obj_object,
					nullptr, TO_CHAR);
			}
			obj_from_char(obj_object);
			equip_char(ch,obj_object, WEAR_LIGHT);
		}
		break;

	case 1:
		if(CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) {
			if((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
				send_to_char("Sta gia' indossando qualcosa sulle tue dita.\n\r", ch);
			}
			else {
				perform_wear(ch,obj_object,keyword);
				if(ch->equipment[WEAR_FINGER_L]) {
					act("Indossi $p su un dito della mano destra.",
						false, ch, obj_object, nullptr, TO_CHAR);
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WEAR_FINGER_R);
				}
				else {
					act("Indossi $p su un dito della mano sinistra.",
						false, ch, obj_object, nullptr, TO_CHAR);
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WEAR_FINGER_L);
				}
			}
		}
		else {
			send_to_char("Non puoi indossarlo sulle tue dita.\n\r", ch);
		}
		break;

	case 2:
		if(CAN_WEAR(obj_object,ITEM_WEAR_NECK)) {
			if((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
				send_to_char("Non puoi mettere nient'altro intorno al collo.\n\r",
							 ch);
			}
			else {
				act("Ti metti $p intorno al collo.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				if(ch->equipment[WEAR_NECK_1]) {
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WEAR_NECK_2);
				}
				else {
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WEAR_NECK_1);
				}
			}
		}
		else {
			send_to_char("Non puoi metterlo intorno al collo.\n\r", ch);
		}
		break;

	case 3:
		if(CAN_WEAR(obj_object,ITEM_WEAR_BODY)) {
			if(ch->equipment[WEAR_BODY]) {
				send_to_char("Stai gia' indossando qualcosa sul corpo.\n\r", ch);
			}
			else {
				act("Indossi $p.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch,  obj_object, WEAR_BODY);
			}
		}
		else {
			send_to_char("Non puoi indossarlo sul corpo.\n\r", ch);
		}
		break;

	case 4:
		if(CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
			if(ch->equipment[WEAR_HEAD]) {
				send_to_char("Hai gia' qualcosa in testa.\n\r", ch);
			}
			else {
				act("Ti metti $p in testa.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_HEAD);
			}
		}
		else {
			send_to_char("Non puoi metterlo in testa.\n\r", ch);
		}
		break;

	case 5:
		if(CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) {
			if(ch->equipment[WEAR_LEGS]) {
				send_to_char("Hai gia' qualcosa sulle tue gambe.\n\r", ch);
			}
			else {
				act("Indossi $p sulle gambe.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_LEGS);
			}
		}
		else {
			send_to_char("Non puoi metterlo sulle gambe.\n\r", ch);
		}
		break;

	case 6:
		if(CAN_WEAR(obj_object,ITEM_WEAR_FEET)) {
			if(ch->equipment[WEAR_FEET]) {
				send_to_char("Hai gia' un qualche tipo di calzatura.\n\r", ch);
			}
			else {
				act("Ti metti $p ai piedi.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_FEET);
			}
		}
		else {
			send_to_char("Non mi sembra una calzatura.\n\r", ch);
		}
		break;

	case 7:
		if(CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
			if(ch->equipment[WEAR_HANDS]) {
				send_to_char("Indossi gia' qualcosa sulle tue mani.\n\r", ch);
			}
			else {
				act("Metti $p sulle mani.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_HANDS);
			}
		}
		else {
			send_to_char("Non vedo come possa indossarlo sulle mani.\n\r", ch);
		}
		break;

	case 8:
		if(CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
			if(ch->equipment[WEAR_ARMS]) {
				send_to_char("Hai gia' qualcosa sulle braccia.\n\r", ch);
			}
			else {
				act("Indossi $p sulle braccia.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_ARMS);
			}
		}
		else {
			send_to_char("Non puoi metterlo sulle braccia.\n\r", ch);
		}
		break;

	case 9:
		if(CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
			if(ch->equipment[WEAR_ABOUT]) {
				send_to_char("Indossi gia' qualcosa intorno al tuo corpo.\n\r", ch);
			}
			else {
				act("Ti avvolgi con $p.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_ABOUT);
			}
		}
		else {
			send_to_char("Non puo' essere indossato intorno al corpo.\n\r", ch);
		}
		break;

	case 10:
		if(CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
			if(ch->equipment[WEAR_WAISTE]) {
				send_to_char("Hai gia' qualcosa intorno alla vita.\n\r", ch);
			}
			else {
				act("Ti metti $p alla vita.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch,  obj_object, WEAR_WAISTE);
			}
		}
		else {
			send_to_char("Non vedo come possa metterlo intorno alla vita.\n\r", ch);
		}
		break;

	case 11:
		if(CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
			if((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
				send_to_char("Hai gia' oggetti intorno a tutti e due i polsi.\n\r",
							 ch);
			}
			else {
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				if(ch->equipment[WEAR_WRIST_L]) {
					act("Indossi $p intorno al polso destro.",
						false, ch, obj_object, nullptr, TO_CHAR);
					equip_char(ch,  obj_object, WEAR_WRIST_R);
				}
				else {
					act("Indossi $p intorno al polso sinistro.",
						false, ch, obj_object, nullptr, TO_CHAR);
					equip_char(ch, obj_object, WEAR_WRIST_L);
				}
			}
		}
		else {
			send_to_char("Non puoi metterlo intorno ai polsi.\n\r", ch);
		}
		break;

	case 12:
		if(CAN_WEAR(obj_object,ITEM_WIELD)) {
			if(ch->equipment[WIELD]) {
				send_to_char("Stai gia' impugnando qualcosa.\n\r", ch);
			}
			else if(ch->equipment[WEAR_LIGHT] && ch->equipment[HOLD]) {
				send_to_char("Devi prima togliere qualcosa dalle tue mani.\n\r", ch);
			}
			else {
				if(GET_OBJ_WEIGHT(obj_object) >
						str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
					send_to_char("E' troppo pesante per usarlo con una mano sola..\n\r",
								 ch);
					if(GET_OBJ_WEIGHT(obj_object) <
							(3*str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)/2) {
						send_to_char("Ma puoi usarlo con due mani.\n\r", ch);
						if(ch->equipment[WEAR_SHIELD]) {
							send_to_char("Se rimuovi il tuo scudo.\n\r", ch);
						} /* wearing shield */
						else if(ch->equipment[HOLD] || ch->equipment[WEAR_LIGHT]) {
							send_to_char("Se rimuovi quello che hai nell'altra mano.\n\r",
										 ch);
						} /* holding light type */
						else {
							perform_wear(ch,obj_object,keyword);
							obj_from_char(obj_object);
							equip_char(ch, obj_object, WIELD);
						} /*  fine, wear it */
					} /* strong enough */
					else {
						send_to_char("Sei troppo debole anche per usarlo a due mani.\n\r",
									 ch);
					} /* to weak to wield two handed */
				}
				else {
					act("Impugni $p.",
						false, ch, obj_object, nullptr, TO_CHAR);
					perform_wear(ch,obj_object,keyword);
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WIELD);
				}
			}
		}
		else {
			send_to_char("Non e' una cosa che puo' essere impugnata.\n\r", ch);
		}
		break;

	case 13:
		if(CAN_WEAR(obj_object,ITEM_HOLD)) {
			if(ch->equipment[HOLD]) {
				send_to_char("Stai gia' usando qualcosa.\n\r", ch);
			}
			else if(ch->equipment[WIELD] &&
					ch->equipment[WIELD]->obj_flags.weight >
					str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
				send_to_char("Non puoi impugnare un'arma a due mani e usare "
							 "qualcos'altro.\n\r", ch);
			}
			else if(ch->equipment[WEAR_LIGHT] && ch->equipment[WIELD]) {
				send_to_char("Hai tutte e due le mani occupate!\n\r", ch);
			}
			else {
				if(CAN_WEAR(obj_object,ITEM_WIELD)) {
					if(ch->equipment[WEAR_SHIELD]) {
						send_to_char("Non puoi usare uno scudo, un'arma e tenere "
									 "un'altra cosa contemporaneamente!\n\r", ch);
						return;
					}

					if(GET_OBJ_WEIGHT(obj_object) >
							str_app[STRENGTH_APPLY_INDEX(ch)].wield_w/2) {
						send_to_char("Quest'arma e' troppo pesante da tenere.\n\r",
									 ch);
						return;
					}
				}
				else if(ch->equipment[WIELD]) {
					if(GET_OBJ_WEIGHT(obj_object) >
							str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
						send_to_char("Questo oggetto e' troppo pesante da tenere.\n\r",
									 ch);
						return;
					}
				}

				act("Prendi $p in mano.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, HOLD);
			}
		}
		else {
			send_to_char("Non e' possibile tenerlo.\n\r", ch);
		}
		break;

	case 14:
		if(CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
			if((ch->equipment[WEAR_SHIELD])) {
				send_to_char("Stai gia' usando uno scudo.\n\r", ch);
			}
			else if(ch->equipment[WIELD] &&
					ch->equipment[WIELD]->obj_flags.weight >
					str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
				send_to_char("Non puoi impugnare un'arma a due mani ed usare uno "
							 "scudo.\n\r", ch);
			}
			else if(ch->equipment[HOLD] &&
					CAN_WEAR(ch->equipment[HOLD],ITEM_WIELD)) {
				send_to_char("Non puoi usare uno scudo e tenere un'arma!\n\r", ch);
				return;
			}
			else {
				perform_wear(ch,obj_object,keyword);
				act("Usi $p.", false, ch, obj_object, nullptr, TO_CHAR);
				obj_from_char(obj_object);
				equip_char(ch, obj_object, WEAR_SHIELD);
			}
		}
		else {
			send_to_char("Non sembra uno scudo.\n\r", ch);
		}
		break;

	case 15:
#if 0
		if(CAN_WEAR(obj_object, ITEM_WEAR_BACK) &&
				obj_object->obj_flags.type_flag == ITEM_CONTAINER)
#else
		if(CAN_WEAR(obj_object, ITEM_WEAR_BACK))
#endif
		{
			if(ch->equipment[WEAR_BACK]) {
				send_to_char("Hai gia' qualcosa sulle tue spalle.\n\r", ch);
			}
			else {
				act("Ti metti $p sulle spalle.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch,  obj_object, WEAR_BACK);
			}
		}
		else {
			send_to_char("Non vedo come puoi metterlo sulle spalle.\n\r", ch);
		}
		break;

	case 16:
		if(CAN_WEAR(obj_object,ITEM_WEAR_EAR)) {
			if((ch->equipment[WEAR_EAR_L]) && (ch->equipment[WEAR_EAR_R])) {
				send_to_char("Hai gia' qualcosa su entrambe le orecchie.\n\r", ch);
			}
			else {
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				if(ch->equipment[WEAR_EAR_L]) {
					std::ostringstream os;
					os << "Appendi " << obj_object->short_description << " al tuo orecchio destro.\n\r";
					send_to_char(os.str().c_str(), ch);
					equip_char(ch,  obj_object, WEAR_EAR_R);
				}
				else {
					std::ostringstream os;
					os << "Appendi " << obj_object->short_description << " al tuo orecchio sinistro.\n\r";
					send_to_char(os.str().c_str(), ch);
					equip_char(ch, obj_object, WEAR_EAR_L);
				}
			}
		}
		else {
			send_to_char("Non puoi appenderlo alle orecchie.\n\r", ch);
		}
		break;

	case 17:
		if(CAN_WEAR(obj_object,ITEM_WEAR_EYE)) {
			if(ch->equipment[WEAR_EYES]) {
				send_to_char("Hai gia' qualcosa sui tuoi occhi.\n\r", ch);
			}
			else {
				act("Ti metti $p davanti agli occhi.",
					false, ch, obj_object, nullptr, TO_CHAR);
				perform_wear(ch,obj_object,keyword);
				obj_from_char(obj_object);
				equip_char(ch,  obj_object, WEAR_EYES);
			}
		}
		else {
			send_to_char("No, non sugli occhi.\n\r", ch);
		}
		break;

	case -1: {
		std::ostringstream os;
		os << "Indossare " << obj_object->short_description << "? E dove?!?\n\r";
		send_to_char(os.str().c_str(), ch);
		break;
	}
	case -2: {
		std::ostringstream os;
		os << "Non puoi indossare " << obj_object->short_description << ".\n\r";
		send_to_char(os.str().c_str(), ch);
		break;
	}
	default:
		mudlog(LOG_ERROR, "Unknown type called in wear.");
		break;
	}
}


ACTION_FUNC(do_wear) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wear (act.obj_wear.cpp)");
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	struct obj_data* obj_object = nullptr;
	struct obj_data* next_obj = nullptr;

	argument_interpreter(arg, arg1.data(), arg2.data());
	if(arg1[0] == '\0') {
		send_to_char("Che cosa vuoi indossare?\n\r", ch);
		return;
	}

	if(obj_arg_is_all(arg1.data())) {
		if(GET_POS(ch) == POSITION_FIGHTING) {
			send_to_char("Mentre combatti? Meglio vestirsi in un momento piu' tranquillo!\n\r", ch);
			return;
		}
		for(obj_object = ch->carrying; obj_object != nullptr; obj_object = next_obj) {
			next_obj = obj_object->next_content;
			const int keyword = obj_infer_wear_keyword(obj_object);
			if(keyword != kObjWearKeywordNone) {
				wear(ch, obj_object, keyword);
			}
		}
		return;
	}

	obj_object = get_obj_in_list_vis(ch, arg1.data(), ch->carrying);
	if(obj_object == nullptr) {
		std::ostringstream os;
		os << "Non sembra che tu abbia un '" << arg1.data() << "'.\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}

	if(arg2[0] != '\0') {
		const int slot = search_block(arg2.data(), eqKeywords, false);
		if(slot == -1) {
			std::ostringstream os;
			os << "Non puoi indossare nulla su un " << arg2.data() << ".\n\r";
			send_to_char(os.str().c_str(), ch);
			return;
		}
		wear(ch, obj_object, slot + 1);
		return;
	}

	wear(ch, obj_object, obj_infer_wear_keyword(obj_object));
}

} // namespace Alarmud
