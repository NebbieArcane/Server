/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
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
#include "spec_procs4.hpp"
#include "act.comm.hpp"
#include "act.obj1.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "modify.hpp"
#include "reception.hpp"
#include "spec_procs.hpp"
#include "utility.hpp"
namespace Alarmud {

/***************************  Procedure ***************************/

//	Mob negoziante generico dei premi delle quest
MOBSPECIAL_FUNC(quest_item_shop)
{
	struct char_data* keeper;
	struct obj_data* buono;
	char buf[MAX_STRING_LENGTH], numero[MAX_INPUT_LENGTH];
	int quest = -1, num, i;

	if(type != EVENT_COMMAND || (cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_VALUE))
	{
		return (FALSE);
	}

	keeper = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(quest_item_shop));

	if(!keeper)
	{
		return (FALSE);
	}

	if(!AWAKE(keeper))
	{
		return(FALSE);
	}

	if(!cmd)
	{
		if(keeper->specials.fighting)
		{
			return(fighter(ch, cmd, arg, keeper, type));
		}
		return(FALSE);
	}

	if(check_soundproof(ch))
	{
		return(FALSE);
	}

	if(!(CAN_SEE(keeper, ch)))
	{
		do_say(keeper, "Non faccio affari con chi non vedo!", CMD_SAY);
		return(TRUE);
	}

	if(IS_NPC(keeper) && (cmd == CMD_LIST || cmd == CMD_VALUE || cmd == CMD_BUY))
	{
		// controllo se il toon ha il buono in inventario
		if(!(buono = get_obj_in_list_vis(ch, obj_index[real_object(QUEST_REWARD)].name, ch->carrying)))
		{
			sprintf(buf, "%s Mi dispiace, non posso mostrarti la mia merce... Non hai quello che voglio!", GET_NAME(ch));
			do_tell(keeper, buf, CMD_TELL);
			return(TRUE);
		}

		// controllo se il buono e' personal sul toon
		if(!pers_on(ch, buono))
		{
			sprintf(buf, "%s Il buono non e' tuo! Non provare a fregarmi!", GET_NAME(ch));
			do_tell(keeper, buf, CMD_TELL);
			return(TRUE);
		}

		// il toon ha il buono ed e' il suo, cerco la quest
		for(i = 0; i < MAX_QUEST_ACHIE; i++)
		{
			if(isname(QuestNumber[i].mercy_name, buono->name))
			{
				quest = i;
				break;
			}
		}

		// manca il nome della quest sul buono
		if(quest == -1)
		{
			send_to_char("Cerca al piu' presto un Dio e consegnagli il buono...\n\r", ch);
			mudlog(LOG_ERROR,"Quest coupon (%s) of %s is missing quest's name", buono->short_description, GET_NAME(ch));
			return(TRUE);
		}

		// mostro la lista degli oggetti scambiabili con il buono
		if(cmd == CMD_LIST)
		{
			ListaOggetti(ch, quest);
			return(TRUE);
		}

		// mostra le caratteristiche dell'oggetto
		if(cmd == CMD_VALUE)
		{
			only_argument(arg, numero);
			num = atoi(numero);

			IdentifyObj(ch, keeper, quest, num);
			return(TRUE);
		}

		if(cmd == CMD_BUY)
		{
			only_argument(arg, numero);
			num = atoi(numero);

			SellObj(ch, keeper, quest, num);
			return(TRUE);
		}
	}
	return(FALSE);
}

/***************************  Funzioni ***************************/

void IdentifyObj(struct char_data* ch, struct char_data* keeper, int quest, int numero)
{
	char buf[MAX_STRING_LENGTH];
	int i = 0;
	struct obj_data* obj;

	while(QuestNebbie[quest][i].quest_item != -1)
	{
		i++;
	}

	if(numero <= 0 || numero > i + 1)
	{
		sprintf(buf, "%s Non ho quell'oggetto in lista, controlla meglio!", GET_NAME(ch));
		do_tell(keeper, buf, CMD_TELL);
		return;
	}

	obj = read_object(QuestNebbie[quest][numero - 1].quest_item, VIRTUAL);

	if(KnownObjQuest[quest].known[numero - 1])
	{
		act("\n\r$N inizia a studiare $p.\n\rDopo qualche attimo sorride ed esclama: 'Eureka!'.\n\r", FALSE, ch, obj, keeper, TO_NOTVICT);
		spell_identify(GET_LEVEL(keeper, WARRIOR_LEVEL_IND), ch, keeper, obj);
	}
	else
	{
		act("\n\r$N inizia a studiare $p.\n\rDopo qualche attimo scuote la testa e mette via $p.\n\r", FALSE, ch, obj, keeper, TO_NOTVICT);
		sprintf(buf, "%s Mi dispiace ma nessuno ha ancora scoperto le proprieta' di questo oggetto...", GET_NAME(ch));
		do_tell(keeper, buf, CMD_TELL);
	}

	extract_obj(obj);

	return;
}

void ListaOggetti(struct char_data* ch, int quest)
{
	int i, len, maxlen = 0, maxlentype = 0;
	std::string sb, desc, descBW;
	struct obj_data* obj;
	string buf;
	char buf2[256];

	for(i = 0; QuestNebbie[quest][i].quest_item != -1; i++)
	{
		obj = read_object(QuestNebbie[quest][i].quest_item, VIRTUAL);

		descBW.append(ParseAnsiColors(0, obj->short_description));
		len = descBW.length();

		sprinttype(GET_ITEM_TYPE(obj), item_types_ita, buf2);
		len += strlen(buf2);

		if(len > maxlen)
		{
			maxlen = len;
		}
		if(strlen(buf2) > maxlentype)
		{
			maxlentype = strlen(buf2);
		}

		descBW.clear();
		extract_obj(obj);
	}

	sb.append("\n\r$c0011Puoi scegliere tra i seguenti oggetti:\n\r$c0013");
	for(i = 0; i < maxlen + maxlentype + 12; i++)
	{
		sb.append("-");
	}
	sb.append("\n\r");

	for(i = 0; QuestNebbie[quest][i].quest_item != -1; i++)
	{
		obj = read_object(QuestNebbie[quest][i].quest_item, VIRTUAL);

		desc.append(obj->short_description).append("$c0011 ");
		desc.append(IS_RARE(obj) ? "$c0009[raro]$c0007" : quest == ACHIE_QUEST_XARAH && !IS_RARE(obj) ? get_last_word(obj->name) : "").append("$c0007");
		descBW.append(ParseAnsiColors(0, obj->short_description)).append(" ").append(IS_RARE(obj) ? "[raro]" : quest == ACHIE_QUEST_XARAH && !IS_RARE(obj) ? get_last_word(obj->name) : "").append("");
		len = desc.length() - descBW.length();

		sprinttype(GET_ITEM_TYPE(obj), item_types_ita, buf2);
		buf = "$c0013[$c0011%4d$c0013]$c0007 %-" + std::to_string(maxlen + 3 + len) + "s %s%-" + std::to_string(strlen(buf2)) + "s\n\r";

		boost::format fmt(buf);
		fmt % (i + 1) % desc % ((i + 1)%2 == 0 ? "$c0015" : "$c0007") % buf2;
		sb.append(fmt.str().c_str());

		desc.clear();
		descBW.clear();
		fmt.clear();

		extract_obj(obj);
	}

	sb.append("$c0013");
	for(i = 0; i < maxlen + maxlentype + 12; i++)
	{
		sb.append("-");
	}
	sb.append("\n\r\n\r$c0011Digita '$c0015buy numero$c0011' per comprare oppure '$c0015value numero$c0011' per conoscerne le proprieta'.\n\r$c0013");

	page_string(ch->desc, sb.c_str(), true);
	sb.clear();
}

void SellObj(struct char_data* ch, struct char_data* keeper, int quest, int numero)
{
	char buf[MAX_STRING_LENGTH];
	int i = 0, num;
	struct obj_data* obj;

	while(QuestNebbie[quest][i].quest_item != -1)
	{
		i++;
	}

	if(numero <= 0 || numero > i + 1)
	{
		sprintf(buf, "%s Non ho quell'oggetto in lista, controlla meglio!", GET_NAME(ch));
		do_tell(keeper, buf, CMD_TELL);
		return;
	}

	num = real_object(QuestNebbie[quest][numero - 1].quest_item);
	obj = read_object(num, REAL);

	SetPersonOnSave(ch, obj);

	obj_to_char(obj, keeper);

	act("\n\r$N apre un'anta dello scaffale dietro di $L e prende $p.\n\r", FALSE, ch, obj, keeper, TO_NOTVICT);
	sprintf(buf, "%s Ecco quello che mi hai chiesto.", GET_NAME(ch));
	do_tell(keeper, buf, CMD_TELL);

	sprintf(buf, "ed%s %s", GET_NAME(ch), GET_NAME(ch));
	do_give(keeper, buf, 0);

	return;
}

std::string get_last_word(const std::string& s)
{
	auto index = s.find_last_of(' ');
 	return s.substr(++index);
}

} // namespace Alarmud
