/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMud
* $Id: act.info.c,v 1.6 2002/03/11 21:15:20 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <array>
#include <cerrno>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unistd.h>
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
#include "act.info.hpp"
#include "act.comm.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "breath.hpp"
#include "cmdid.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "modify.hpp"
#include "multiclass.hpp"
#include "parser.hpp"
#include "signals.hpp"
#include "skills.hpp"
#include "snew.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "specialproc_other.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"        // for spell_info_type, SKILL_EVALUATE, SPELL_G...
#include "Sql.hpp"
#include "trap.hpp"
#include "utility.hpp"
#include "version.hpp"
#include "vt100c.hpp"

namespace Alarmud {

int attrefzone=0;


std::uint64_t HowManyConnection(int ToAdd) {
	static std::uint64_t NumberOfConnection = 0;
	if(ToAdd > 0) {
		NumberOfConnection += static_cast<std::uint64_t>(ToAdd);
	}
	else if(ToAdd < 0) {
		const std::uint64_t dec = static_cast<std::uint64_t>(-ToAdd);
		NumberOfConnection = (dec > NumberOfConnection) ? 0 : NumberOfConnection - dec;
	}
	return NumberOfConnection;
}

bool singular(const struct obj_data* o) {
	if(o == nullptr) {
		return true;
	}

	return !(IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) ||
	         IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) ||
	         IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) ||
	         IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS));
}

/* Procedures related to 'look' */

void argument_split_2(const char* argument,char* first_arg,char* second_arg) {
	if(first_arg == nullptr || second_arg == nullptr) {
		return;
	}

	first_arg[0] = '\0';
	second_arg[0] = '\0';
	if(argument == nullptr) {
		return;
	}

	int begin = 0;
	int look_at = 0;

	/* Find first non blank */
	for(; argument[begin] == ' '; begin++) {
	}

	/* Copy first word lowercased */
	for(look_at = 0; argument[begin + look_at] > ' '; look_at++) {
		first_arg[look_at] = LOWER(argument[begin + look_at]);
	}
	first_arg[look_at] = '\0';
	begin += look_at;

	/* Find first non blank */
	for(; argument[begin] == ' '; begin++) {
	}

	/* Copy second word lowercased */
	for(look_at = 0; argument[begin + look_at] > ' '; look_at++) {
		second_arg[look_at] = LOWER(argument[begin + look_at]);
	}
	second_arg[look_at] = '\0';
}

struct obj_data* get_object_in_equip_vis(struct char_data* ch,const char* arg, struct obj_data* equipment[], int* j) {
	if(ch == nullptr || arg == nullptr || equipment == nullptr || j == nullptr) {
		return nullptr;
	}

	for(*j = 0; *j < MAX_WEAR; (*j)++) {
		struct obj_data* equipped = equipment[*j];
		if(equipped == nullptr) {
			continue;
		}
		if(!CAN_SEE_OBJ(ch, equipped)) {
			continue;
		}
		if(isname(arg, equipped->name) || isname2(arg, equipped->name)) {
			return equipped;
		}
	}

	return nullptr;
}

char* find_ex_description(const char* word, const struct extra_descr_data* list) {
	if(word != nullptr && *word != '\0') {
		for(const struct extra_descr_data* i = list; i != nullptr; i = i->next) {
			if(i->keyword != nullptr && isname(word, i->keyword)) {
				return i->description;
			}
		}

#if 0
		for(const struct extra_descr_data* i = list; i != nullptr; i = i->next) {
			if(i->keyword != nullptr && isname2(word, i->keyword)) {
				return i->description;
			}
		}
#endif
	}

	return nullptr;
}

namespace {

bool info_cstr_nonempty_equal(const char* a, const char* b) {
	return a != nullptr && b != nullptr && *a != '\0' && std::string_view(a) == std::string_view(b);
}

void CapitalizeGameLabel(std::string& text) {
	if(text.size() > 7 && text[1] == '$') {
		text[7] = UPPER(text[7]);
	}
	else if(!text.empty()) {
		text[0] = UPPER(text[0]);
	}
}

std::string BuildAuraDisplayName(struct char_data* ch) {
	if(IS_PC(ch)) {
		return ch->player.name ? ch->player.name : "";
	}

	std::array<char, MAX_STRING_LENGTH> tempName{};
	const std::string shortDescr = ch->player.short_descr ? ch->player.short_descr : "";
	const std::size_t copyLen = std::min(shortDescr.size(), tempName.size() - 1);
	std::copy_n(shortDescr.data(), copyLen, tempName.data());
	tempName[copyLen] = '\0';
	RemColorString(tempName.data());
	if(tempName[0] != '\0') {
		CAP(tempName.data());
	}
	return tempName.data();
}

void EmitAuraEffects(struct char_data* target, struct char_data* viewer,
                     const std::string& displayName) {
	if(IS_AFFECTED(target, AFF_SANCTUARY) &&
	   !IS_AFFECTED(target, AFF_GLOBE_DARKNESS)) {
		std::string auraMsg = "$c0015";
		auraMsg += displayName;
		auraMsg += " brilla di luce propria!";
		act(auraMsg.c_str(), FALSE, target, nullptr, viewer, TO_VICT);
	}

	if(IS_AFFECTED(target, AFF_GROWTH)) {
		std::string growthMsg = "$c0003";
		growthMsg += displayName;
		growthMsg += " e' enorme!";
		act(growthMsg.c_str(), FALSE, target, nullptr, viewer, TO_VICT);
	}

	if(IS_AFFECTED(target, AFF_FIRESHIELD) &&
	   !IS_AFFECTED(target, AFF_GLOBE_DARKNESS)) {
		std::string fireMsg = "$c0001";
		fireMsg += displayName;
		fireMsg += " e' avvolt$b in una luce fiammeggiante!";
		act(fireMsg.c_str(), FALSE, target, nullptr, viewer, TO_VICT);
	}

	if(IS_AFFECTED(target, AFF_GLOBE_DARKNESS)) {
		std::string darkMsg = "$c0008";
		darkMsg += displayName;
		darkMsg += " e' avvolt$b nell'oscurita'!";
		act(darkMsg.c_str(), FALSE, target, nullptr, viewer, TO_VICT);
	}
}

bool ShowVisibleEquipment(struct char_data* target, struct char_data* viewer) {
	bool found = false;
	for(int j = 0; j < MAX_WEAR; j++) {
		if(target->equipment[j] && CAN_SEE_OBJ(viewer, target->equipment[j])) {
			found = true;
			break;
		}
	}
	if(!found) {
		return false;
	}

	act("\n\r$n sta usando:", FALSE, target, nullptr, viewer, TO_VICT);
	for(int j = 0; j < MAX_WEAR; j++) {
		if(target->equipment[j] && CAN_SEE_OBJ(viewer, target->equipment[j])) {
			send_to_char(eqWhere[j], viewer);
			show_obj_to_char(target->equipment[j], viewer, 1);
		}
	}
	return true;
}

std::string BuildConditionSubject(struct char_data* target, bool capitalizeNpc) {
	if(IS_NPC(target)) {
		std::string npcShort = target->player.short_descr ? target->player.short_descr : "";
		if(capitalizeNpc) {
			CapitalizeGameLabel(npcShort);
		}
		return npcShort;
	}
	return GET_NAME(target);
}

void AppendConditionSuffix(std::string& text, int percent, bool useColorFormatting) {
	if(useColorFormatting) {
		if(percent >= 100) {
			text += " e' in condizioni $c0010eccellenti$c0007.";
		}
		else if(percent >= 80) {
			text += " ha pochi graffi.";
		}
		else if(percent >= 60) {
			text += " ha alcuni tagli ed abrasioni.";
		}
		else if(percent >= 40) {
			text += " e' ferit$B.";
		}
		else if(percent >= 20) {
			text += " $c0001sanguina$c0007 abbondantemente.";
		}
		else if(percent >= 0) {
			text += " $c0001ha grossi squarci aperti$c0007.";
		}
		else {
			text += " $c0009sta morendo per le ferite ed i colpi ricevuti$c0007.";
		}
		return;
	}

	if(percent >= 100) {
		text += " e' in condizioni eccellenti.";
	}
	else if(percent >= 80) {
		text += " ha pochi graffi.";
	}
	else if(percent >= 60) {
		text += " ha alcuni tagli ed abrasioni.";
	}
	else if(percent >= 40) {
		text += " e' ferit$B.";
	}
	else if(percent >= 20) {
		text += " sanguina abbondatemente.";
	}
	else if(percent >= 0) {
		text += " $c0001ha grossi squarci aperti.";
	}
	else {
		text += " $c0009sta morendo per le ferite ed i colpi ricevuti.";
	}
}

bool FillObjectBaseText(struct obj_data* object, struct char_data* ch, int mode,
                        std::string& buffer) {
	if((mode == 0) && object->description && *object->description) {
		buffer = object->description;
		CapitalizeGameLabel(buffer);
		return true;
	}

	if(object->short_description &&
	   (mode == 1 || mode == 2 || mode == 3 || mode == 4)) {
		buffer = object->short_description;
		CapitalizeGameLabel(buffer);
		return true;
	}

	if(mode != 5) {
		return true;
	}

	if(object->obj_flags.type_flag == ITEM_NOTE) {
		if(object->action_description && *object->action_description) {
			std::string noteText = "C'e' scritto sopra qualcosa:\n\r\n\r";
			noteText += object->action_description;
			page_string(ch->desc, noteText.c_str(), true);
		}
		else {
			act("Non c'e' scritto nulla.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		return false;
	}

	if(object->obj_flags.type_flag != ITEM_DRINKCON) {
		buffer = "Non vedi nulla di speciale...";
	}
	else {
		buffer = "Sembra un contenitore per bevande.";
	}
	return true;
}

void UpdateHuntingStateForLook(struct char_data* ch, bool polyAwareForPc) {
	int huntingBit = ACT_HUNTING;
	if(IS_PC(ch)) {
		huntingBit = polyAwareForPc
		             ? (IS_POLY(ch) ? ACT_HUNTING : PLR_HUNTING)
		             : PLR_HUNTING;
	}

	if(!IS_SET(ch->specials.act, huntingBit)) {
		return;
	}

	if(ch->specials.hunting) {
		if(!track(ch, ch->specials.hunting)) {
			ch->specials.hunting = 0;
			ch->hunt_dist = 0;
			REMOVE_BIT(ch->specials.act, huntingBit);
		}
		return;
	}

	ch->hunt_dist = 0;
	REMOVE_BIT(ch->specials.act, huntingBit);
}

void EmitDangerSenseAdjacentDeath(struct char_data* ch) {
	if(!affected_by_spell(ch, SKILL_DANGER_SENSE) &&
	   !IS_AFFECTED2(ch, AFF2_DANGER_SENSE)) {
		return;
	}
	for(int i = 0; i < 6; i++) {
		struct room_direction_data* const pExit = EXIT(ch, i);
		if(pExit == nullptr) {
			continue;
		}
		struct room_data* const pRoom = real_roomp(pExit->to_room);
		if(pRoom != nullptr && IS_SET(pRoom->room_flags, DEATH)) {
			act("$c0009Percepisci un grande pericolo qui intorno.",
			    FALSE, ch, nullptr, nullptr, TO_CHAR);
			break;
		}
	}
}
} // namespace


void show_obj_to_char(struct obj_data* object, struct char_data* ch, int mode) {
	std::string buffer;
	if(!FillObjectBaseText(object, ch, mode, buffer)) {
		return;  /* note handled */
	}

	std::string output = buffer;
	if(mode != 3) {
		const bool isSingular = singular(object);
		const auto appendSingularPlural = [&output, isSingular](
		                                     const char* singularText,
		                                     const char* pluralText) {
			output += isSingular ? singularText : pluralText;
		};

		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			output += "$c0011 (invisibile)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			appendSingularPlural("$c0009 (ha un alone di luce rossa)$c0007",
			                     "$c0009 (hanno un alone di luce rossa)$c0007");
		}
		if(IS_OBJ_STAT(object, ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			appendSingularPlural("$c0012 (ha un alone di luce blu)$c0007",
			                     "$c0012 (hanno un alone di luce blu)$c0007");
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			appendSingularPlural("$c0015 (ha un alone luminoso)$c0007",
			                     "$c0015 (hanno un alone luminoso)$c0007");
		}
		if(IS_OBJ_STAT(object, ITEM_HUM)) {
			appendSingularPlural("$c0008 (emette un forte ronzio)$c0007",
			                     "$c0008 (emettono un forte ronzio)$c0007");
		}
		if(object->obj_flags.type_flag == ITEM_ARMOR) {
			if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 4)) {
				appendSingularPlural("$c0009 (distrutto)$c0007",
				                     "$c0009 (sono distrutti)$c0007");
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 3)) {
				appendSingularPlural("$c0009 (ha bisogno di essere riparato)$c0007",
				                     "$c0009 (hanno bisogno di essere riparati)$c0007");
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 2)) {
				output += "$c0011 (in buone condizioni)$c0007";
			}
			else if(object->obj_flags.value[ 0 ] <
					object->obj_flags.value[1]) {
				output += "$c0010 (in ottime condizioni)$c0007";
			}
			else {
				output += "$c0010 (in condizioni eccellenti)$c0007";
			}
		}
	}
	if(!output.empty()) {
		output += "\n\r";
		page_string(ch->desc, output.c_str(), true);
	}
}

void show_mult_obj_to_char(struct obj_data* object, struct char_data* ch,
						   int mode, int num) {
	std::string buffer;
	if(!FillObjectBaseText(object, ch, mode, buffer)) {
		return;
	}

	std::string output = buffer;
	if(mode != 3) {
		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			output += " $c0011(invisibile)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			output += " $c0009(ha un alone di luce rossa)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			output += " $c0012(ha un alone di luce blu)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			output += " $c0015(ha un alone luminoso)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_HUM)) {
			output += " $c0008(emette un forte ronzio)$c0007";
		}
	}

	if(num > 1) {
		output += "[";
		output += std::to_string(num);
		output += "]";
	}
	if(!output.empty()) {
		output += "\n\r";
		page_string(ch->desc, output.c_str(), true);
	}
}

void list_obj_in_room(struct obj_data* list, struct char_data* ch) {
	constexpr int kMaxGroupedObjects = 50;
	struct obj_data* cond_ptr[kMaxGroupedObjects];
	int cond_tot[kMaxGroupedObjects];
	int inventoryNum = 1;
	int cond_top = 0;

	for(struct obj_data* i = list; i != nullptr; i = i->next_content) {
		if(!CAN_SEE_OBJ(ch, i)) {
			continue;
		}

		if(cond_top < kMaxGroupedObjects) {
			bool found = false;
			for(int k = 0; k < cond_top && !found; k++) {
				if((i->item_number == cond_ptr[k]->item_number) &&
				   info_cstr_nonempty_equal(i->description, cond_ptr[k]->description)) {
					cond_tot[k] += 1;
					found = true;
				}
			}
			if(!found) {
				cond_ptr[cond_top] = i;
				cond_tot[cond_top] = 1;
				cond_top += 1;
			}
			continue;
		}

		show_obj_to_char(i, ch, 0);
	}

	for(int k = 0; k < cond_top; k++) {
		if(cond_tot[k] > 1) {
			std::ostringstream prefix;
			prefix << "[" << std::setw(2) << inventoryNum++ << "] ";
			send_to_char(prefix.str().c_str(), ch);
			show_mult_obj_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
		}
		else {
			show_obj_to_char(cond_ptr[k], ch, 0);
		}
	}
}


void list_obj_in_heap(struct obj_data* list, struct char_data* ch) {
	constexpr int kMaxGroupedObjects = 50;
	struct obj_data* cond_ptr[kMaxGroupedObjects];
	int cond_tot[kMaxGroupedObjects];
	int numInventory = 1;
	int cond_top = 0;

	for(struct obj_data* i = list; i != nullptr; i = i->next_content) {
		if(!CAN_SEE_OBJ(ch, i)) {
			continue;
		}

		if(cond_top < kMaxGroupedObjects) {
			bool found = false;
			for(int k = 0; k < cond_top && !found; k++) {
				if((i->item_number == cond_ptr[k]->item_number) &&
				   info_cstr_nonempty_equal(i->short_description, cond_ptr[k]->short_description)) {
					cond_tot[k] += 1;
					found = true;
				}
			}
			if(!found) {
				cond_ptr[cond_top] = i;
				cond_tot[cond_top] = 1;
				cond_top += 1;
			}
			continue;
		}

		show_obj_to_char(i, ch, 2);
	}

	for(int k = 0; k < cond_top; k++) {
		std::ostringstream prefix;
		prefix << "[" << std::setw(2) << numInventory++ << "] ";
		send_to_char(prefix.str().c_str(), ch);
		if(cond_tot[k] > 1) {
			numInventory += cond_tot[k] - 1;
			show_mult_obj_to_char(cond_ptr[k], ch, 2, cond_tot[k]);
		}
		else {
			show_obj_to_char(cond_ptr[k], ch, 2);
		}
	}
}

void list_obj_to_char(struct obj_data* list, struct char_data* ch, int mode,
					  bool show) {
	int numInBag = 1;
	bool found = false;

	for(struct obj_data* i = list; i; i = i->next_content) {
		if(CAN_SEE_OBJ(ch, i)) {
			std::ostringstream prefix;
			prefix << "[" << std::setw(2) << numInBag++ << "] ";
			send_to_char(prefix.str().c_str(), ch);
			show_obj_to_char(i, ch, mode);
			found = true;
		}
	}
	if(!found && show) {
		send_to_char("Nulla.\n\r", ch);
	}
}


void ShowAltezzaCostituzione(struct char_data* pChar, struct char_data* pTo) {
	std::string szBuf = "$n e' ";

	const std::array<const char*, 6> descAltezze = {
		"altissim$b",
		"molto alt$b",
		"alt$b",
		"di altezza media",
		"bass$b",
		"molto bass$b"
	};

	const std::array<const char*, 6> descCostituzione = {
		"$c0010VERAMENTE gross$b$c0007",
		"gross$b",
		"robust$b",
		"di costituzione media",
		"magr$b",
		"gracile"
	};
	if(!pChar || !pTo) {
		mudlog(LOG_SYSERR,
			   "!pChar || !pTo in ShowAltezzaCostituzione (act.info.c).");
		return;
	}

	if(!GET_WEIGHT(pChar) || ! GET_HEIGHT(pChar)) {
		mudlog(LOG_ERROR,
			   "L'altezza od il peso di %s e' a zero.", GET_NAME_DESC(pChar));
		return;
	}

	if(!IsHumanoid(pChar)) {
		return;
	}

	if(pChar->player.height > 250) {
		szBuf += descAltezze[0];
	}
	else if(pChar->player.height > 190) {
		szBuf += descAltezze[1];
	}
	else if(pChar->player.height > 170) {
		szBuf += descAltezze[2];
	}
	else if(pChar->player.height > 155) {
		szBuf += descAltezze[3];
	}
	else if(pChar->player.height > 140) {
		szBuf += descAltezze[4];
	}
	else {
		szBuf += descAltezze[5];
	}

	szBuf += " e ";

	const float fRapp = static_cast<float>(GET_HEIGHT(pChar)) /
	                    (static_cast<float>(pChar->player.weight) * 0.4536f);


	if(fRapp > 3.27) {
		szBuf += descCostituzione[5];
	}
	else if(fRapp > 3) {
		szBuf += descCostituzione[4];
	}
	else if(fRapp > 2.25) {
		szBuf += descCostituzione[3];
	}
	else if(fRapp > 2) {
		szBuf += descCostituzione[2];
	}
	else if(fRapp > 1.6) {
		szBuf += descCostituzione[1];
	}
	else {
		szBuf += descCostituzione[0];
	}

	szBuf += ".";
	act(szBuf.c_str(), FALSE, pChar, nullptr, pTo, TO_VICT);
}

void show_char_to_char(struct char_data* i, struct char_data* ch, int mode) {
	int j, found, percent, otype;
	struct obj_data* tmp_obj;
	struct affected_type* aff;



	if(!ch || !i) {
		mudlog(LOG_SYSERR, "!ch || !i in act.info.c show_char_to_char");
		return;
	}

	if(mode == 0) {

		if(IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
			if(IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i)) {
				send_to_char("Percepisci una forma di vita invisibile nella "
							 "stanza.\n\r", ch);
				return;
			}
			else {
				/* no see nothing */
				return;
			}
		}

		if(!(i->player.long_descr) ||
				(GET_POS(i) != i->specials.default_pos)) {
			/* A player char or a mobile without long descr, or not in default pos.*/
			std::string line;
			if(!IS_NPC(i)) {
				std::string playerLine = GET_NAME(i);
				playerLine += " ";
				if(GET_TITLE(i)) {
					playerLine += GET_TITLE(i);
				}
				line = playerLine;
			}
			else {
				std::string npcShort = i->player.short_descr ? i->player.short_descr : "";
				CapitalizeGameLabel(npcShort);
				line = npcShort;
			}

			if(IS_AFFECTED(i, AFF_INVISIBLE) || i->invis_level >= IMMORTALE) {
				line += " $c0011(invisibile)$c0007";
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				line += " $c0015(schiavo)$c0007";
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				line += " giace qui, svenut$b.";
				break;
			case POSITION_INCAP:
				line += " giace qui, incapacitat$b.";
				break;
			case POSITION_MORTALLYW:
				line += " giace qui, ferit$b a morte.";
				break;
			case POSITION_DEAD:
				line += " giace qui, mort$b.";
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					line += " e' qui, cavalcando ";
					line += MOUNTED(i)->player.short_descr;
				}
				else {
					line += " e' qui, in piedi.";
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						line += " sta galleggiando qui.";
					}
					else {
						line += " e' qui, in piedi.";
					}
				}
				else {
					line += " vola qui intorno.";
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta galleggiando qui.";
				}
				else {
					line += " e' sedut$b qui.";
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta facendo il morto nell'$c0012acqua$c0007.";
				}
				else {
					line += " sta riposando qui.";
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta dormendo qui nell'$c0012acqua$c0007.";
				}
				else {
					line += " sta dormendo qui.";
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					line += " e' qui, combattendo contro ";
					if(i->specials.fighting == ch) {
						line += " DI TE!";
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								line += i->specials.fighting->player.short_descr;
							}
							else {
								line += GET_NAME(i->specials.fighting);
							}
						}
						else {
							line += "qualcuno che se ne e' appena andato.";
						}
					}
				}
				else { /* NIL fighting pointer */
					line += " e' qui dimenandosi contro l'aria.";
				}
				break;
			default:
				line += " fluttua qui intorno.";
				break;
			} /*switch */

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				line += "$c0006 (AFK)$c0007";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					line += "$c0009 (alone rosso)$c0007";
				}
			}

			if(IS_LINKDEAD(i)) {
				line += "$c0015 (link dead)$c0007";
			}

			act(line.c_str(), FALSE, i, nullptr, ch, TO_VICT);
			if(!IS_AFFECTED(ch,AFF_TRUE_SIGHT)) {
				for(j=how_many_spell(i, SPELL_MIRROR_IMAGES); j>0; j--) {
					act(line.c_str(), FALSE, i, nullptr, ch, TO_VICT);
				}
			}

		}
		else {
			/* npc with long */

			std::string longLine;
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				longLine = "*";
			}

			if(IS_LINKDEAD(i)) {
				longLine += "$c0015(link dead)$c0007 ";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				longLine += "$c0009(alone rosso)$c0007 ";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				longLine += "$c0006(AFK)$c0007 ";
			}

			longLine += i->player.long_descr;

			while(!longLine.empty() &&
					(longLine.back() == '\r' ||
					 longLine.back() == '\n' ||
					 longLine.back() == ' ')) {
				longLine.pop_back();
			}
			act(longLine.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}

		EmitAuraEffects(i, ch, BuildAuraDisplayName(i));

	}
	else if(mode == 1) {
		if(i->player.description) {
			std::string description = i->player.description;
			if(description.size() > 1 && description[1] == '$') {
				if(description.size() > 7) {
					description[7] = UPPER(description[7]);
				}
			}
			else if(!description.empty()) {
				description[0] = UPPER(description[0]);
			}
			send_to_char(description.c_str(), ch);
		}
		else {
			if(IS_MAESTRO_DEL_CREATO(i)) {
				act("Una pulsante aura di potere avvolge $n.", FALSE, i, nullptr, ch, TO_VICT);
			}
			else if(IS_MAESTRO_DEGLI_DEI(i))
				act("$n appartiene alla cerchia dei Demiurghi.",
					FALSE, i, nullptr, ch, TO_VICT);
			else if(IS_DIO(i))
				act("$n appartiene alla cerchia degli Dei.",
					FALSE, i, nullptr, ch, TO_VICT);
			else if(IS_IMMORTALE(i))
				act("$n appartiene ai ranghi degli Immortali.",
					FALSE, i, nullptr, ch, TO_VICT);
			else if(IS_PRINCE(i))
				act("$n appartiene ai ranghi dei Principi.",
					FALSE, i, nullptr, ch, TO_VICT);
			else {
				act("$n e' mortale.", FALSE, i, nullptr, ch, TO_VICT);
			}

		}
		if(HAS_PRINCE(i)) {
			std::string clanMsg = "$n appartiene al clan di ";
			clanMsg += (GET_PRINCE(i) ? GET_PRINCE(i) : "");
			clanMsg += ".";
			act(clanMsg.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}

		/*
		 * personal descriptions.
		 */

		ShowAltezzaCostituzione(i, ch);

		if(IS_PC(i)) {
			std::string raceMsg = "$n e' un$b ";
			raceMsg += RaceName[ GET_RACE(i) ];
			raceMsg += ".";
			act(raceMsg.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}


		if(MOUNTED(i)) {
			std::string mountMsg = "$n sta cavalcando ";
			mountMsg += MOUNTED(i)->player.short_descr;
			mountMsg += ".";
			act(mountMsg.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}

		if(RIDDEN(i)) {
			std::string riddenMsg = "$n e' cavalcat$b da ";
			riddenMsg += IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.short_descr :
						 GET_NAME(RIDDEN(i));
			riddenMsg += ".";
			act(riddenMsg.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}

		/* Show a character to another */

		if(GET_MAX_HIT(i) > 0) {
			percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
		}
		else {
			percent = -1;    /* How could MAX_HIT be < 1?? */
		}

		std::string conditionMsg = BuildConditionSubject(i, true);
		AppendConditionSuffix(conditionMsg, percent, true);

		act(conditionMsg.c_str(), FALSE, ch, nullptr, i, TO_CHAR);


		/*
		 * spell_descriptions, etc.
		 */
		otype = -1;
        if((IS_AFFECTED(i, AFF_GLOBE_DARKNESS) || affected_by_spell(i, SPELL_GLOBE_DARKNESS)) && (!is_same_group(i, ch) && !IS_IMMORTALE(ch) && ch != i && saves_spell(i, SAVING_SPELL)))
        {
            act(spell_desc[ SPELL_GLOBE_DARKNESS ], FALSE, i, nullptr, ch, TO_VICT);
        }
        else
        {
            for(aff = i->affected; aff; aff = aff->next)
            {
                if(aff->type < MAX_EXIST_SPELL)
                {
                    if(spell_desc[ aff->type ] && *spell_desc[ aff->type ])
                    {
                        if(aff->type != otype)
                        {
                            act(spell_desc[ aff->type ], FALSE, i, nullptr, ch, TO_VICT);
                            otype = aff->type;
                        }
                    }
                }
            }
            if(IS_AFFECTED(i, AFF_SANCTUARY) && !affected_by_spell(i, SPELL_SANCTUARY))
            {
                act(spell_desc[ SPELL_SANCTUARY ], FALSE, i, nullptr, ch, TO_VICT);
            }
            if(IS_AFFECTED(i, AFF_FIRESHIELD) && !affected_by_spell(i, SPELL_FIRESHIELD))
            {
                act(spell_desc[ SPELL_FIRESHIELD ], FALSE, i, nullptr, ch, TO_VICT);
            }
        }

		ShowVisibleEquipment(i, ch);
		if(HasClass(ch, CLASS_THIEF) && (ch != i) && (!IS_IMMORTAL(ch))) {
			found = FALSE;
			send_to_char("\n\rGuardandogli nelle tasche, vedi:\n\r", ch);
			for(tmp_obj = i->carrying; tmp_obj;
					tmp_obj = tmp_obj->next_content) {
				if(CAN_SEE_OBJ(ch, tmp_obj) &&
						(number(0, MAX_MORT) < GetMaxLevel(ch))) {
					show_obj_to_char(tmp_obj, ch, 1);
					found = TRUE;
				}
			}
			if(!found) {
				send_to_char("Nulla.\n\r", ch);
			}
		}
		else if(IS_IMMORTAL(ch)) {
			send_to_char("Inventario:\n\r",ch);
			for(tmp_obj = i->carrying; tmp_obj;
					tmp_obj = tmp_obj->next_content) {
				show_obj_to_char(tmp_obj, ch, 1);
				found = TRUE;
			}
			if(!found) {
				send_to_char("Nulla\n\r",ch);
			}
		}
	}
	else if(mode == 2) {
		/* Lists inventory */
		act("$n sta trasportando:", FALSE, i, nullptr, ch, TO_VICT);
		list_obj_in_heap(i->carrying, ch);
	}
}


void show_mult_char_to_char(struct char_data* i, struct char_data* ch,
							int mode, int num) {
	int found, percent;
	struct obj_data* tmp_obj;

	if(mode == 0) {
		if(IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
			if(IS_AFFECTED(ch, AFF_SENSE_LIFE) || IS_IMMORTAL(i)) {
				if(num == 1)
					act("$c0002Percepisci una forma di vita invisibile qui intorno.",
						FALSE, ch, nullptr, nullptr, TO_CHAR);
				else
					act("$c0002Percepisci alcune forme di vita invisibile qui intorno.",
						FALSE, ch, nullptr, nullptr, TO_CHAR);
				return;
			}
			else {
				/* no see nothing */
				return;
			}
		}

		if(!(i->player.long_descr) ||
				(GET_POS(i) != i->specials.default_pos)) {
			/* A player char or a mobile without long descr, or not in default pos. */
			std::string line;
			if(!IS_NPC(i)) {
				line = GET_NAME(i);
				line += " ";
				if(GET_TITLE(i)) {
					line += GET_TITLE(i);
				}
			}
			else {
				line = i->player.short_descr ? i->player.short_descr : "";
				if(line.size() > 7 && line[1] == '$') {
					line[7] = UPPER(line[7]);
				}
				else if(!line.empty()) {
					line[0] = UPPER(line[0]);
				}
			}
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				line += "$c0011 (invisibile)";
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				line += "$c0010 (schiavo)";
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				line += " giace qui, svenut$b.";
				break;
			case POSITION_INCAP:
				line += " giace qui, incapacitat$b.";
				break;
			case POSITION_MORTALLYW:
				line += " giace qui, ferit$b a morte.";
				break;
			case POSITION_DEAD:
				line += " giace qui, mort$b.";
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					line += " e' qui, montando ";
					line += MOUNTED(i)->player.short_descr;
				}
				else {
					line += " e' qui, in piedi.";
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						line += " sta galleggiano qui.";
					}
					else {
						line += " e' qui, in piedi.";
					}
				}
				else {
					line += " vola qui intorno.";
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta galleggiando qui.";
				}
				else {
					line += " e' sedut$b qui.";
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta riposando qui nell'acqua.";
				}
				else {
					line += " sta riposando qui.";
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta dormendo qui nell'acqua.";
				}
				else {
					line += " sta dormendo qui.";
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					line += " e' qui, combattendo contro ";
					if(i->specials.fighting == ch) {
						line += " DI TE!";
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								line += i->specials.fighting->player.short_descr;
							}
							else {
								line += GET_NAME(i->specials.fighting);
							}
						}
						else {
							line += "qualcuno che se ne e' appena andato.";
						}
					}
				}
				else { /* NIL fighting pointer */
					line += " e' qui dimenandosi contro l'aria.";
				}
				break;
			default:
				line += " fluttua qui intorno.";
				break;
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				line += "$c0009 (alone rosso)$c0007";
			}

			if(IS_LINKDEAD(i)) {
				line += "$c0015 (link dead)$c0007";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				line += "$c0006 (AFK)$c0007";
			}

			if(num > 1) {
				line += " [";
				line += std::to_string(num);
				line += "]";
			}
			act(line.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}
		else {
			/* npc with long */

			std::string longLine;
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				longLine = "*";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				longLine += "$c0009(alone rosso)$c0007 ";
			}

			if(IS_LINKDEAD(i)) {
				longLine += "$c0015(link dead)$c0007 ";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				longLine += "$c0006(AFK)$c0007 ";
			}

			longLine += i->player.long_descr;

			/* this gets a little annoying */

			if(num > 1) {
				while(!longLine.empty() &&
						(longLine.back() == '\r' ||
						 longLine.back() == '\n' ||
						 longLine.back() == ' ')) {
					longLine.pop_back();
				}
				longLine += " [";
				longLine += std::to_string(num);
				longLine += "]";
			}

			act(longLine.c_str(), FALSE, i, nullptr, ch, TO_VICT);
		}

		EmitAuraEffects(i, ch, BuildAuraDisplayName(i));

	}
	else if(mode == 1) {
		if(i->player.description) {
			send_to_char(i->player.description, ch);
		}
		else {
			act("Non vedi nulla di speciale in $n.", FALSE, i, nullptr, ch, TO_VICT);
		}

		ShowAltezzaCostituzione(i, ch);

		/* Show a character to another */

		if(GET_MAX_HIT(i) > 0) {
			percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
		}
		else {
			percent = -1;    /* How could MAX_HIT be < 1?? */
		}

		std::string conditionMsg = BuildConditionSubject(i, false);
		AppendConditionSuffix(conditionMsg, percent, false);

		act(conditionMsg.c_str(), FALSE, ch, nullptr, i, TO_CHAR);

		ShowVisibleEquipment(i, ch);
		if((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
			found = FALSE;
			send_to_char("\n\rGuardandogli nelle tasche, vedi:\n\r", ch);
			for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
				if(CAN_SEE_OBJ(ch, tmp_obj) &&
						(number(0, MAX_MORT) < GetMaxLevel(ch))) {
					show_obj_to_char(tmp_obj, ch, 1);
					found = TRUE;
				}
			}
			if(!found) {
				send_to_char("Nulla.\n\r", ch);
			}
		}
	}
	else if(mode == 2) {

		/* Lists inventory */
		act("$n sta trasportando:", FALSE, i, nullptr, ch, TO_VICT);
		list_obj_in_heap(i->carrying, ch);
	}
}


void list_char_in_room(struct char_data* list, struct char_data* ch) {
	constexpr int kMaxGroupedChars = 50;
	struct char_data* cond_ptr[kMaxGroupedChars];
	int cond_tot[kMaxGroupedChars];
	int cond_top = 0;

	for(struct char_data* i = list; i != nullptr; i = i->next_in_room) {
		const bool canObserve = (ch != i) && (!RIDDEN(i)) &&
		                        (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		                         (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)));
		if(!canObserve) {
			continue;
		}

		if((cond_top < kMaxGroupedChars) && !MOUNTED(i)) {
			bool found = false;
			if(IS_NPC(i)) {
				for(int k = 0; (k < cond_top && !found); k++) {
					if(i->nr == cond_ptr[k]->nr &&
					   (GET_POS(i) == GET_POS(cond_ptr[k])) &&
					   (i->specials.affected_by == cond_ptr[k]->specials.affected_by) &&
					   (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
					   info_cstr_nonempty_equal(i->player.short_descr, cond_ptr[k]->player.short_descr)) {
						cond_tot[k] += 1;
						found = true;
					}
				}
			}
			if(!found) {
				cond_ptr[cond_top] = i;
				cond_tot[cond_top] = 1;
				cond_top += 1;
			}
		}
		else {
			show_char_to_char(i, ch, 0);
		}
	}

	for(int k = 0; k < cond_top; k++) {
		if(cond_tot[k] > 1) {
			show_mult_char_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
		}
		else {
			show_char_to_char(cond_ptr[k], ch, 0);
		}
	}
}


void list_char_to_char(struct char_data* list, struct char_data* ch,
					   int mode) {
	for(struct char_data* i = list; i; i = i->next_in_room) {
		if((ch != i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		                 (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
			show_char_to_char(i, ch, mode);
		}
	}
}

/* Added by Mike Wilson 9/23/93 */

void list_exits_in_room(struct char_data* ch) {
	std::string buf;
	for(int door = 0; door <= 5; door++) {
		struct room_direction_data* exitdata = EXIT(ch, door);
		if(exitdata == nullptr) {
			continue;
		}
		if(real_roomp(exitdata->to_room) == nullptr) {
			continue;
		}

		bool seeit = false;
		if(GET_RACE(ch) == RACE_ELVEN ||
		   GET_RACE(ch) == RACE_GOLD_ELF ||
		   GET_RACE(ch) == RACE_WILD_ELF ||
		   GET_RACE(ch) == RACE_SEA_ELF) {
			/* elves can see secret doors 1-3 on d6 */
			seeit = (number(1, 6) <= 3);
		}
		else if(GET_RACE(ch) == RACE_HALF_ELVEN) {
			/* half-elves can see exits, not as good as full */
			seeit = (number(1, 6) <= 2);
		}
		else if(GET_RACE(ch) == RACE_DWARF ||
		        GET_RACE(ch) == RACE_DARK_DWARF) {
			/* I nani le vedono meglio di tutti!!!! */
			seeit = (number(1, 12) <= 7);
		}

		if(exitdata->to_room == NOWHERE && !IS_IMMORTAL(ch)) {
			continue;
		}

		const bool isClosed = IS_SET(exitdata->exit_info, EX_CLOSED);
		const bool isSecret = IS_SET(exitdata->exit_info, EX_SECRET);
		if((!isClosed || IS_IMMORTAL(ch)) || (isSecret && seeit)) {
			buf += ' ';
			buf += listexits[door];
			if(isClosed && IS_IMMORTAL(ch)) {
				buf += " (chiuso)";
			}
			if(isSecret && (seeit || IS_IMMORTAL(ch))) {
				buf += " $c5009(segreto)$c0007";    /* blink red */
			}
		}
	}

	if(!buf.empty()) {
		send_to_char("Uscite:", ch);
		act(buf.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}
}


ACTION_FUNC(do_look) {
	std::array<char, MAX_INPUT_LENGTH> arg1{};
	std::array<char, MAX_INPUT_LENGTH> arg2{};
	int keyword_no;
	int j, bits, temp;
	struct room_data* roomp;
	bool found;
	struct obj_data* tmp_object, *found_object;
	struct char_data* tmp_char;
	char* tmp_desc;

	static int bNotShowTitle = 0;
	struct room_data* pRoomWithChar;

	const char* blood_messages[] = {
		"$c0009Non si deve vedere questo.$c0007",
		"$c0009C'e' un po di sangue qui a terra.$c0007",
		"$c0009C'e' del sangue ai tuoi piedi.$c0007",
		"$c0009Del sangue scorre in terra.$c0007",
		"$c0009C'e' tanto sangue da dare la nausea!$c0007",
		"$c0009C'e' sangue ovunque guardi.$c0007",
		"$c0009Che carneficina! Il Dio della Morte fara' festa questa notte!$c0007",
		"$c0009Non riesci a non sporcarti con il sangue che vedi ovunque!$c0007",
		"$c0009Tutto pare rivoltarsi per la morte e distruzione che aleggia qui!$c0007",
		"$c0009Gli Dei dovrebbero avere pieta' e ripulire questo orrido posto dal sangue!$c0007",
		"$c0009C'e' cosi' tanto sangue qui che rischi di affogarci dentro!$c0007",
		"\n"
	};



	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_look (act.info.c)");
		return;
	}

	if(ch->desc == nullptr) {
		return;
	}

	pRoomWithChar =  real_roomp(ch->in_room);

	if(GET_POS(ch) < POSITION_SLEEPING) {
		send_to_char("Non vedi nient'altro che stelle!\n\r", ch);
	}
	else if(GET_POS(ch) == POSITION_SLEEPING) {
		send_to_char("Non puoi vedere nulla, stai dormendo!\n\r", ch);
	}
	else if(IS_AFFECTED(ch, AFF_BLIND)) {
		send_to_char("Non riesci a vedere un tubo, sei cieco!\n\r", ch);
	}
	else if((IS_DARK_P(pRoomWithChar)) && (!IS_IMMORTAL(ch)) &&
			(!IS_AFFECTED(ch, AFF_TRUE_SIGHT)) &&
			(!IS_AFFECTED(ch, AFF_SCRYING)) &&
			GET_RACE(ch)!=RACE_DARK_ELF && GET_RACE(ch)!=RACE_DARK_DWARF && // Gaia 2001
			GET_RACE(ch)!=RACE_DEEP_GNOME) {
		send_to_char("E' molto buio qui...\n\r", ch);
		if(IS_AFFECTED(ch, AFF_INFRAVISION)) {
			list_char_in_room(pRoomWithChar->people, ch);
		}
	}
	else {
		only_argument(arg, arg1.data());

		if(0==strn_cmp(arg1.data(),"at",2) && isspace(arg1[2])) {
			only_argument(arg+3, arg2.data());
			keyword_no = 7;
		}
		else if(0==strn_cmp(arg1.data(),"in",2) && isspace(arg1[2])) {
			only_argument(arg+3, arg2.data());
			keyword_no = 6;
		}
		else {
			keyword_no = search_block(arg1.data(), lookKeywords, FALSE);
		}

		if((keyword_no == -1) && arg1[0] != '\0') {
			keyword_no = 7;
			only_argument(arg, arg2.data());
		}


		found = false;
		tmp_object = nullptr;
		tmp_char = nullptr;
		tmp_desc = nullptr;

		switch(keyword_no) {
		/* look <dir> */
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 :
		case 5 : {
			struct room_direction_data* const exitp = EXIT(ch, keyword_no);
			if(exitp == nullptr) {
				send_to_char("Non vedi nulla di speciale.\n\r", ch);
			}
			else {
				if(exitp->general_description && *(exitp->general_description)) {
					char chLast = exitp->general_description
								  [ strlen(exitp->general_description) - 1 ];
					send_to_char(exitp->general_description, ch);

					if(chLast != '\n' && chLast != '\r') {
						send_to_char("\n\r", ch);
					}
				}
				else {
					struct room_data* pRoom = real_roomp(exitp->to_room);

					if(pRoom != nullptr && pRoom->name && *(pRoom->name) &&
							((IS_SET(exitp->exit_info, EX_ISDOOR) &&
							  !IS_SET(exitp->exit_info, EX_CLOSED)) ||
							 !IS_SET(exitp->exit_info, EX_ISDOOR))) {
						send_to_char(pRoom->name, ch);
						if(pRoom->name[ strlen(pRoom->name) - 1 ] != '.') {
							send_to_char(".", ch);
						}
						send_to_char("\n\r", ch);
					}
					else if(exitp->keyword && *(exitp->keyword) &&
							!IS_SET(exitp->exit_info, EX_SECRET)) {
						std::string line = IS_SET(exitp->exit_info, EX_MALE) ? "Un " : "Una ";
						line += fname(exitp->keyword);
						line += ".\n\r";
						send_to_char(line.c_str(), ch);
					}
					else {
						send_to_char("Non vedi nulla di speciale.\n\r", ch);
					}
				}

				if(IS_SET(exitp->exit_info, EX_ISDOOR)) {
					if(IS_SET(exitp->exit_info, EX_CLOSED)) {
						if(!IS_SET(exitp->exit_info, EX_SECRET)) {
							if(exitp->keyword && *(exitp->keyword)) {
								std::string line = IS_SET(exitp->exit_info, EX_MALE) ? "Il " : "La ";
								line += fname(exitp->keyword);
								line += IS_SET(exitp->exit_info, EX_MALE) ? " e' chiuso.\n\r" : " e' chiusa.\n\r";
								send_to_char(line.c_str(), ch);
							}
							else {
								std::string line = "La porta ";
								line += dirsTo[keyword_no];
								line += " e' chiusa.\n\r";
								send_to_char(line.c_str(), ch);
							}
						}
					}
					else {
						if(!IS_SET(exitp->exit_info, EX_SECRET)) {
							if(exitp->keyword && *(exitp->keyword)) {
								std::string line = IS_SET(exitp->exit_info, EX_MALE) ? "Il " : "La ";
								line += fname(exitp->keyword);
								line += IS_SET(exitp->exit_info, EX_MALE) ? " e' aperto.\n\r" : " e' aperta.\n\r";
								send_to_char(line.c_str(), ch);
							}
							else {
								std::string line = "La porta ";
								line += dirsTo[keyword_no];
								line += " e' aperta.\n\r";
								send_to_char(line.c_str(), ch);
							}
						}
						else {
							std::string line = "Il passaggio ";
							line += dirsTo[keyword_no];
							line += " e' aperto.\n\r";
							send_to_char(line.c_str(), ch);
						}
					}
				}
			}

			if(exitp != nullptr && exitp->to_room > 0 &&
					(!IS_SET(exitp->exit_info, EX_ISDOOR) ||
					 !IS_SET(exitp->exit_info, EX_CLOSED))) {

				if((IS_AFFECTED(ch, AFF_SCRYING) &&
						!IS_SET(exitp->exit_info, EX_NOLOOKT)) || IS_IMMORTAL(ch)) {
					struct room_data* rp;
					std::string lookLine = "Guardando ";
					lookLine += dirsTo[keyword_no];
					lookLine += " vedi...\n\r";
					send_to_char(lookLine.c_str(), ch);

					if(!IS_IMMORTAL(ch) || !IS_SET(ch->specials.act, PLR_STEALTH)) {
						std::string roomAct = "$n guarda ";
						roomAct += dirsTo[keyword_no];
						roomAct += ".";
						act(roomAct.c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
					}

					rp = real_roomp(exitp->to_room);
					if(rp == nullptr) {
						send_to_char("Un turbinante chaos.\n\r", ch);
					}
					else {
						bNotShowTitle = TRUE;
						const std::string lookCommand = std::to_string(exitp->to_room) + " look";
						do_at(ch, lookCommand.c_str(), 0);
						bNotShowTitle = FALSE;
					}
				}
			}
			break;
		}

		/* look 'in'        */
		case 6: {
			if(arg2[0] != '\0') {
				/* Item carried */
				bits = generic_find(arg2.data(), FIND_OBJ_INV | FIND_OBJ_ROOM |
									FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

				if(bits) {
					/* Found something */
					if(GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) {
						if(tmp_object->obj_flags.value[1] <= 0) {
							act("E' vuoto.", FALSE, ch, nullptr, nullptr, TO_CHAR);
						}
						else {
							temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
							std::string liquidLine = "E' ";
							liquidLine += fullness[temp];
							liquidLine += " di un liquido ";
							liquidLine += color_liquid[tmp_object->obj_flags.value[2]];
							liquidLine += ".\n\r";
							send_to_char(liquidLine.c_str(), ch);
						}
					}
					else if(GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
						if(!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
							if(tmp_object->short_description &&
							   *tmp_object->short_description) {
								send_to_char(tmp_object->short_description, ch);
							}
							else {
								send_to_char(fname(tmp_object->name), ch);
							}
							switch(bits) {
							case FIND_OBJ_INV :
								send_to_char(" (trasporta) : \n\r", ch);
								break;
							case FIND_OBJ_ROOM :
								send_to_char(" (qui) : \n\r", ch);
								break;
							case FIND_OBJ_EQUIP :
								send_to_char(" (usa) : \n\r", ch);
								break;
							}
							list_obj_in_heap(tmp_object->contains, ch);
						}
						else {
							send_to_char("E' chiuso.\n\r", ch);
						}
					}
					else {
						send_to_char("Non e' un contenitore.\n\r", ch);
					}
				}
				else {
					/* wrong argument */
					send_to_char("Non lo vedi qui intorno.\n\r", ch);
				}
			}
			else {
				/* no argument */
				send_to_char("Guardare in che cosa?\n\r", ch);
			}
			break;
		}
		/* look 'at'        */
		case 7 : {
			if(arg2[0] != '\0') {
				bits = generic_find(arg2.data(), FIND_OBJ_INV | FIND_OBJ_ROOM |
									FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
				if(tmp_char != nullptr) {
					show_char_to_char(tmp_char, ch, 1);
					if(ch != tmp_char && !IS_AFFECTED(ch,AFF_SNEAK)) {
						act("$n ti guarda.", TRUE, ch, nullptr, tmp_char, TO_VICT);
						act("$n guarda a $N.", TRUE, ch, nullptr, tmp_char, TO_NOTVICT);
					}
					return;
				}

				/*
				 * Search for Extra Descriptions in room and items
				 */

				/* Extra description in room?? */
				if(!found) {
					tmp_desc = find_ex_description(arg2.data(),
												   pRoomWithChar->ex_description);
					if(tmp_desc != nullptr) {
						page_string(ch->desc, tmp_desc, false);
						return;
					}
				}

				/* extra descriptions in items */

				/* Equipment Used */
				if(!found) {
					for(j = 0; j< MAX_WEAR && !found; j++) {
						if(ch->equipment[j] != nullptr) {
							if(CAN_SEE_OBJ(ch,ch->equipment[j])) {
								tmp_desc = find_ex_description(arg2.data(),
															   ch->equipment[j]->ex_description);
								if(tmp_desc != nullptr) {
									page_string(ch->desc, tmp_desc, true);
									found = true;
								}
							}
						}
					}
				}
				/* In inventory */
				if(!found) {
					for(tmp_object = ch->carrying;
							tmp_object != nullptr && !found;
							tmp_object = tmp_object->next_content) {
						if(CAN_SEE_OBJ(ch, tmp_object)) {
							tmp_desc = find_ex_description(arg2.data(),
														   tmp_object->ex_description);
							if(tmp_desc != nullptr) {
								page_string(ch->desc, tmp_desc, true);
								found = true;
							}
						}
					}
				}
				/* Object In room */

				if(!found) {
					for(tmp_object = pRoomWithChar->contents;
							tmp_object != nullptr && !found;
							tmp_object = tmp_object->next_content) {
						if(CAN_SEE_OBJ(ch, tmp_object)) {
							tmp_desc = find_ex_description(arg2.data(),
														   tmp_object->ex_description);
							if(tmp_desc != nullptr) {
								page_string(ch->desc, tmp_desc, true);
								found = true;
							}
						}
					}
				}
				/* wrong argument */
				if(bits) {
					/* If an object was found */
					if(!found) {
						show_obj_to_char(found_object, ch, 5);
					}
					/* Show no-description */
					else {
						show_obj_to_char(found_object, ch, 6);
					}
					/* Find hum, glow etc */
				}
				else if(!found) {
					send_to_char("Non lo vedi qui intorno.\n\r", ch);
				}
			}
			else {
				/* no argument */
				send_to_char("Guardare a cosa?\n\r", ch);
			}

			break;
		}

		/* look ''                */
		case 8 : {
			if(!bNotShowTitle && cmd == CMD_LOOK) {
				send_to_char(pRoomWithChar->name, ch);
				if(IS_DARK_P(pRoomWithChar) && IS_IMMORTAL(ch)) {
					send_to_char(" (buio)", ch);
				}
				send_to_char("\n\r", ch);
			}

			if(ch->desc != nullptr &&
					((ch->desc->original != nullptr &&
					  !IS_SET(ch->desc->original->specials.act, PLR_BRIEF)) ||
					 (ch->desc->original == nullptr &&
					  !IS_SET(ch->specials.act, PLR_BRIEF)))) {
				send_to_char(pRoomWithChar->description, ch);
			}

			if(IS_SET(pRoomWithChar->room_flags, BRIGHT)) {
				if(IsDarkOutside(pRoomWithChar) &&
						pRoomWithChar->szWhenBrightAtNight &&
						*pRoomWithChar->szWhenBrightAtNight) {

					send_to_char(pRoomWithChar->szWhenBrightAtNight, ch);
				}
				else if(!IsDarkOutside(pRoomWithChar) &&
						pRoomWithChar->szWhenBrightAtDay &&
						*pRoomWithChar->szWhenBrightAtDay) {

					send_to_char(pRoomWithChar->szWhenBrightAtDay, ch);
				}
			}
			if(RM_BLOOD(ch->in_room) > 0) {
				/* mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
				 * */
				act(blood_messages[static_cast<int8_t>(RM_BLOOD(ch->in_room))], FALSE, ch, nullptr, nullptr, TO_CHAR);
			}
			else {
				/*   mudlog(LOG_ERROR,"Blood trovato:
				 *   %d",RM_BLOOD(ch->in_room));*/
			}

			//ACIDUS 2003 skill know structure
			roomp = real_roomp(ch->in_room);
			if(ch->skills != nullptr) {
				if(
					roomp != nullptr && (roomp->tele_targ > 0) && (GET_RACE(ch)==RACE_DWARF)
					&& (number(1,100) < ch->skills[SKILL_DETERMINE].learned)
				) {
					act("$c0006Noti qualcosa di instabile nella struttura di questo luogo.", FALSE, ch, nullptr, nullptr, TO_CHAR);
				}
			}


			list_exits_in_room(ch);
			list_obj_in_room(pRoomWithChar->contents, ch);
			list_char_in_room(pRoomWithChar->people, ch);

			EmitDangerSenseAdjacentDeath(ch);

			UpdateHuntingStateForLook(ch, true);

			break;
		}

		/* wrong arg        */
		case -1 :
			send_to_char("Mi spiace, ma non capisco!\n\r", ch);
			break;

		/* look 'room' */
		case 9 : {

			send_to_char(pRoomWithChar->name, ch);
			send_to_char("\n\r", ch);
			send_to_char(pRoomWithChar->description, ch);


			UpdateHuntingStateForLook(ch, false);
			if(RM_BLOOD(ch->in_room) > 0) {
			//	mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
				act(blood_messages[static_cast<int8_t>(RM_BLOOD(ch->in_room))], FALSE, ch, nullptr, nullptr, TO_CHAR);
			}
			else {
			//	mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
			}
			list_exits_in_room(ch);
			list_obj_in_room(pRoomWithChar->contents, ch);
			list_char_in_room(pRoomWithChar->people, ch);

			break;
		}
		}
	}
}

/* end of look */




ACTION_FUNC(do_read) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_read (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	/* Same as "look at <arg>" — routed through do_look (case look at). */
	std::string lookArg = "at ";
	if(arg != nullptr) {
		lookArg += arg;
	}
	do_look(ch, lookArg.c_str(), CMD_LOOK);
}



ACTION_FUNC(do_examine) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_examine (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::array<char, 1000> name{};
	struct char_data* tmp_char = nullptr;
	struct obj_data* tmp_object = nullptr;

	std::string lookAtArg = "at ";
	if(arg != nullptr) {
		lookAtArg += arg;
	}
	do_look(ch, lookAtArg.c_str(), CMD_LOOK);

	one_argument(arg, name.data());

	if(name[0] == '\0') {
		send_to_char("Esaminare che cosa?\n\r", ch);
		return;
	}

	generic_find(name.data(), FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if(tmp_object != nullptr) {
		if((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
				(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
			send_to_char("Quando ci guardi dentro, vedi:\n\r", ch);
			std::string lookInArg = "in ";
			if(arg != nullptr) {
				lookInArg += arg;
			}
			do_look(ch, lookInArg.c_str(), CMD_LOOK);
		}
	}
}

/**************************************************************************
 * do_exits visualizza le uscite della locazione.
 **************************************************************************/
ACTION_FUNC(do_exits) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_exits (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::string buf;
	for(int door = 0; door <= 5; ++door) {
		struct room_direction_data* const exitdata = EXIT(ch, door);
		if(exitdata == nullptr || exitdata->to_room <= 0) {
			continue;
		}

		struct room_data* const dest = real_roomp(exitdata->to_room);
		if(dest == nullptr) {
			if(IS_IMMORTAL(ch)) {
				/* Stesso schema delle altre righe: dir - "nome" (tag) #vnum */
				buf += "$c0007";
				buf += exits[door];
				buf += " $c0015-$c0007 ";
				buf += "Caos vorticoso $c0009(stanza non caricata)$c0007";
				buf += " $c0015#";
				buf += std::to_string(exitdata->to_room);
				buf += "\n\r";
			}
			continue;
		}

		if(IS_DIO(ch)) {
			buf += "$c0007";
			buf += exits[door];
			buf += " $c0015-$c0007 ";
			buf += dest->name;
			if(IS_SET(exitdata->exit_info, EX_SECRET)) {
				buf += " $c0009(segreta)";
			}
			if(IS_SET(exitdata->exit_info, EX_CLOSED)) {
				buf += " $c0015(chiusa)";
			}
			if(IS_DARK(exitdata->to_room)) {
				buf += " $c0008(buia)";
			}
			buf += " $c0015#";
			buf += std::to_string(exitdata->to_room);
			buf += "\n\r";
		}
		else if(!IS_SET(exitdata->exit_info, EX_CLOSED) ||
				!IS_SET(exitdata->exit_info, EX_SECRET)) {
			buf += "$c0007";
			buf += exits[door];
			const bool dest_too_dark =
				IS_DARK(exitdata->to_room) && !IS_IMMORTAL(ch) &&
				!IS_AFFECTED(ch, AFF_TRUE_SIGHT) &&
				!IS_AFFECTED(ch, AFF_SCRYING) && GET_RACE(ch) != RACE_DARK_ELF &&
				GET_RACE(ch) != RACE_DARK_DWARF && GET_RACE(ch) != RACE_DEEP_GNOME;
			if(dest_too_dark) {
				buf += " $c0015-$c0008 Troppo buio per dirlo\n\r";
			}
			else {
				buf += " $c0015-$c0007 ";
				buf += dest->name;
				buf += "\n\r";
			}
		}
	}

	send_to_char("Uscite visibili:\n\r", ch);

	if(!buf.empty()) {
		send_to_char(buf.c_str(), ch);
	}
	else {
		send_to_char("$c0009Nessuna!\n\r", ch);
	}
}

ACTION_FUNC(do_status) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_status (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::string statusMsg = "$c0005Tu hai $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_HIT(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_HIT(ch));
	statusMsg += "$c0005) hit, $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_MANA(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_MANA(ch));
	statusMsg += "$c0005) mana e $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_MOVE(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_MOVE(ch));
	statusMsg += "$c0005) punti di movimento.";
	act(statusMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string expMsg = "$c0005Hai effettuato $c0015";
	expMsg += std::to_string(GET_EXP(ch));
	expMsg += "$c0005 exp, ed hai $c0015";
	expMsg += std::to_string(GET_GOLD(ch));
	expMsg += "$c0005 monete d'oro.";
	act(expMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
}

namespace {

const char* AchievementClasseLabelForDifficulty(int classe) {
	if(classe == 0) {
		return "tutte le classi";
	}
	if(classe == -1) {
		return "non abilitato";
	}
	return pc_class_types[classe];
}

struct AchievementMissingDiffScan {
	int category;
	int beginIndex;
	int endExclusive;
	const char* rowFormat;
	const char* typeLabel;
};

static constexpr std::array<AchievementMissingDiffScan, 5> kAchievementMissingDiffScans = {{
    {RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE,
     "$c0009%6d $c0010%-12s %6d %-34s %s\n\r", "Raceslayer:"},
    {BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE,
     "$c0009%6d $c0011%-12s %6d %-34s %s\n\r", "Bosskill:"},
    {CLASS_ACHIE, 1, MAX_CLASS_ACHIE,
     "$c0009%6d $c0012%%-12s %6d %-34s %s\n\r", "Class:"},
    {QUEST_ACHIE, 0, MAX_QUEST_ACHIE,
     "$c0009%6d $c0013%-12s %6d %-34s %s\n\r", "Quest:"},
    {OTHER_ACHIE, 0, MAX_OTHER_ACHIE,
     "$c0009%6d $c0014%-12s %6d %-34s %s\n\r", "Other:"},
}};

void AppendAchievementRowsWithUnsetDifficulty(std::string& sb, int& rowNumber) {
	for(const AchievementMissingDiffScan& scan : kAchievementMissingDiffScans) {
		for(int i = scan.beginIndex; i < scan.endExclusive; ++i) {
			const ClassAchieTable& entry = AchievementsList[scan.category][i];
			if(entry.grado_diff != 0) {
				continue;
			}
			++rowNumber;
			boost::format fmt(scan.rowFormat);
			fmt % rowNumber % scan.typeLabel % entry.achie_number % entry.achie_string2
			    % AchievementClasseLabelForDifficulty(entry.classe);
			sb.append(fmt.str().c_str());
			fmt.clear();
		}
	}
}

void AppendAchievementSectionBanner(std::string& sb, const unsigned dashCount, const char* title) {
	sb.append("$c0009");
	sb.append(dashCount, '-').append(" $c0015");
	sb.append(title);
	sb.append("$c0009 ");
	sb.append(dashCount, '-').append("\n\r");
}

int AchievementThresholdForTier(const ClassAchieTable& entry, const int tier) {
	switch(tier) {
	case 10:
		return entry.lvl10_val;
	case 9:
		return entry.lvl9_val;
	case 8:
		return entry.lvl8_val;
	case 7:
		return entry.lvl7_val;
	case 6:
		return entry.lvl6_val;
	case 5:
		return entry.lvl5_val;
	case 4:
		return entry.lvl4_val;
	case 3:
		return entry.lvl3_val;
	case 2:
		return entry.lvl2_val;
	case 1:
		return entry.lvl1_val;
	default:
		return entry.lvl1_val;
	}
}

void AppendCompactAchievementForIndex(std::string& sb, int& num, char_data* tch, const int category,
                                    const int index) {
	const int progress = tch->specials.achievements[category][index];
	if(progress <= 0) {
		return;
	}
	const ClassAchieTable& entry = AchievementsList[category][index];
	for(int tier = 10; tier >= 1; --tier) {
		if(progress >= AchievementThresholdForTier(entry, tier) && entry.n_livelli >= tier) {
			num += 1;
			sb.append(bufferAchie(tch, index, category, tier, num, false, 0));
			num += tier - 1;
			return;
		}
	}
}

void AppendCompactAchievementCategory(std::string& sb, int& num, char_data* tch, const int category,
                                     const int indexFrom, const int indexToExclusive,
                                     const int hasAchievementBannerFlag, const unsigned dashWidth,
                                     const char* sectionTitle) {
	if(hasAchievement(tch, category, hasAchievementBannerFlag)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendCompactAchievementForIndex(sb, num, tch, category, i);
	}
}

void AppendFullAchievementRowRaceBossQuestOther(std::string& sb, int& num, char_data* tch,
                                                char_data* ch, const int category, const int index) {
	if(AchievementsList[category][index].classe == -1) {
		return;
	}
	if(HasClass(tch, AchievementsList[category][index].classe) || IS_QUESTMASTER(ch)) {
		num += 1;
		sb.append(bufferAchie(tch, index, category, AchievementsList[category][index].n_livelli, num, true,
		                      0));
		num += AchievementsList[category][index].n_livelli - 1;
	}
	else if(tch->specials.achievements[category][index] > 0) {
		num += 1;
		sb.append(bufferAchie(tch, index, category, AchievementsList[category][index].n_livelli, num, true,
		                      0));
		num += AchievementsList[category][index].n_livelli - 1;
	}
}

void AppendFullAchievementRowClass(std::string& sb, int& num, char_data* tch, const int index) {
	if(AchievementsList[CLASS_ACHIE][index].classe == -1) {
		return;
	}
	if(HasClass(tch, AchievementsList[CLASS_ACHIE][index].classe) ||
	   AchievementsList[CLASS_ACHIE][index].classe == 0) {
		num += 1;
		sb.append(bufferAchie(tch, index, CLASS_ACHIE, AchievementsList[CLASS_ACHIE][index].n_livelli, num,
		                      true, 0));
		num += AchievementsList[CLASS_ACHIE][index].n_livelli - 1;
	}
}

void AppendFullListCategoryRaceBossQuestOther(std::string& sb, int& num, char_data* tch, char_data* ch,
                                              const int category, const int indexFrom,
                                              const int indexToExclusive, const unsigned dashWidth,
                                              const char* sectionTitle) {
	if(hasAchievement(tch, category, 1) || IS_QUESTMASTER(ch)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendFullAchievementRowRaceBossQuestOther(sb, num, tch, ch, category, i);
	}
}

void AppendFullListCategoryClass(std::string& sb, int& num, char_data* tch, char_data* ch,
                                 const int indexFrom, const int indexToExclusive,
                                 const unsigned dashWidth, const char* sectionTitle) {
	if(hasAchievement(tch, CLASS_ACHIE, 1) || IS_QUESTMASTER(ch)) {
		AppendAchievementSectionBanner(sb, dashWidth, sectionTitle);
	}
	for(int i = indexFrom; i < indexToExclusive; ++i) {
		AppendFullAchievementRowClass(sb, num, tch, i);
	}
}

} // namespace

ACTION_FUNC(do_checkachielevel)
{
    if(ch == nullptr) {
        mudlog(LOG_SYSERR, "ch==nullptr in do_checkachielevel (act.info.cpp)");
        return;
    }
    if(ch->desc == nullptr) {
        return;
    }

    static constexpr std::array<const char*, 3> kAchievementLevelReviewers = {{
        "Croneh", "Alar", "Requiem"}};
    const char* const playerName = GET_NAME(ch);
    const bool isReviewer = std::any_of(
        kAchievementLevelReviewers.begin(),
        kAchievementLevelReviewers.end(),
        [playerName](const char* allowedName) {
            return playerName != nullptr && allowedName != nullptr &&
                   std::string_view(playerName) == std::string_view(allowedName);
        });
    if(!isReviewer) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    int num = 0;
    std::string sb;

    sb.append("$c0011I seguenti achievement non hanno impostato il grado di difficolta':\n\r");
    AppendAchievementRowsWithUnsetDifficulty(sb, num);
    page_string(ch->desc, sb.c_str(), true);
}

ACTION_FUNC(do_achievements)
{
    if(ch == nullptr) {
        mudlog(LOG_SYSERR, "ch==nullptr in do_achievements (act.info.cpp)");
        return;
    }
    if(ch->desc == nullptr) {
        return;
    }
    if(!IS_PC(ch)) {
        return;
    }

    std::array<char, 128> arg1{};
    std::array<char, 128> arg2{};
    std::string sb;

    arg = one_argument(arg, arg1.data());
    const auto isCommand = [&](std::string_view cmd) {
        return std::string_view(arg1.data()) == cmd;
    };
    const auto applyPolyOriginalOrAbort = [&](char_data*& who) -> bool {
        if(!IS_POLY(who)) {
            return true;
        }
        if(who->desc == nullptr || who->desc->original == nullptr) {
            mudlog(LOG_SYSERR,
                   "IS_POLY without desc/original in do_achievements (applyPolyOriginalOrAbort)");
            send_to_char("In questo momento non riesci a identificarlo.\n\r", ch);
            return false;
        }
        who = who->desc->original;
        return true;
    };
    const auto resolveVisiblePlayerTarget = [&](char_data*& target, const char* name, const char* notFoundMsg, const char* notPcMsg) {
        target = get_char_vis_world(ch, name, nullptr);
        if(target == nullptr)
        {
            send_to_char(notFoundMsg, ch);
            return false;
        }

        if(!IS_PC(target))
        {
            send_to_char(notPcMsg, ch);
            return false;
        }

        return applyPolyOriginalOrAbort(target);
    };
    const auto parsePositiveIntArg = [&](const char* rawValue,
                                         const char* invalidMsg,
                                         const char* tooHighMsg,
                                         const char* nonPositiveMsg,
                                         int& outValue) {
        char* parseEnd = nullptr;
        errno = 0;
        const long long parsedValue = std::strtoll(rawValue, &parseEnd, 10);
        if(parseEnd == rawValue || *parseEnd != '\0')
        {
            send_to_char(invalidMsg, ch);
            return false;
        }
        if(errno == ERANGE || parsedValue > static_cast<long long>(std::numeric_limits<int>::max()))
        {
            send_to_char(tooHighMsg, ch);
            return false;
        }
        if(parsedValue <= 0)
        {
            send_to_char(nonPositiveMsg, ch);
            return false;
        }
        outValue = static_cast<int>(parsedValue);
        return true;
    };
    struct AchievementSlot
    {
        int category;
        int index;
        int baseNum;
        int levels;
    };
    constexpr std::array<int, 5> achievementCategoryOrder = {{
        RACESLAYER_ACHIE, BOSSKILL_ACHIE, CLASS_ACHIE, QUEST_ACHIE, OTHER_ACHIE
    }};
    const auto categoryStartIndex = [](int category) {
        return (category == CLASS_ACHIE) ? 1 : 0;
    };
    const auto categoryMaxCount = [](int category) {
        switch(category)
        {
            case RACESLAYER_ACHIE: return MAX_RACE_ACHIE;
            case BOSSKILL_ACHIE:   return MAX_BOSS_ACHIE;
            case CLASS_ACHIE:      return MAX_CLASS_ACHIE;
            case QUEST_ACHIE:      return MAX_QUEST_ACHIE;
            case OTHER_ACHIE:      return MAX_OTHER_ACHIE;
            default:               return 0;
        }
    };
    const auto isCategorySlotVisible = [&](int category, int index, char_data* target, char_data* actor) {
        if(AchievementsList[category][index].classe == -1)
        {
            return false;
        }
        if(category == CLASS_ACHIE)
        {
            return HasClass(target, AchievementsList[category][index].classe) ||
                   AchievementsList[category][index].classe == 0;
        }
        return HasClass(target, AchievementsList[category][index].classe) || IS_QUESTMASTER(actor);
    };
    const auto hasCategoryProgress = [&](int category, int index, char_data* target) {
        if(category == CLASS_ACHIE)
        {
            return false;
        }
        return target->specials.achievements[category][index] > 0;
    };
    const auto findAchievementSlotByNumber = [&](char_data* target,
                                                 char_data* actor,
                                                 const int requestedNumber,
                                                 const bool includeProgressFallback)
                                                 -> std::optional<AchievementSlot> {
        int num = 0;
        for(const int category : achievementCategoryOrder)
        {
            const int start = categoryStartIndex(category);
            const int maxCount = categoryMaxCount(category);
            for(int index = start; index < maxCount; ++index)
            {
                const bool visible = isCategorySlotVisible(category, index, target, actor);
                const bool includeByProgress = includeProgressFallback &&
                                               hasCategoryProgress(category, index, target);
                if(!visible && !includeByProgress)
                {
                    continue;
                }

                const int levels = AchievementsList[category][index].n_livelli;
                if(requestedNumber > num && requestedNumber <= (num + levels))
                {
                    return AchievementSlot{category, index, num, levels};
                }
                num += levels;
            }
        }
        return std::nullopt;
    };

    if(arg1[0] != '\0')
    {
        if(isCommand("all"))
        {
            char_data* tch = ch;

            if(IS_QUESTMASTER(ch))
            {
                arg = one_argument(arg, arg2.data());

                if(arg2[0] != '\0')
                {
                    tch = get_char_vis_world(ch, arg2.data(), nullptr);
                    if(tch == nullptr)
                    {
                        send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
                        return;
                    }
                }
            }

            if(!IS_PC(tch))
            {
                send_to_char("Vuoi veramente vedere gli achievements di uno stupido mob?\n\r", ch);
                return;
            }

            if(!applyPolyOriginalOrAbort(tch)) {
                return;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
            {
                int num = 0;

                if(ch == tch)
                {
                    send_to_char("\n\r$c0011Ecco tutti i tuoi achievements:\n\r", ch);
                }
                else
                {
                    act("\n\r$c0011Lista completa degli achievements di $c0015$N$c0011:", false, ch, nullptr, tch, TO_CHAR);
                }

                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE,
                                                         45u, "Race  Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE,
                                                         45u, "Boss  Achievements");
                AppendFullListCategoryClass(sb, num, tch, ch, 1, MAX_CLASS_ACHIE, 45u, "Skill Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, QUEST_ACHIE, 0, MAX_QUEST_ACHIE,
                                                         45u, "Quest Achievements");
                AppendFullListCategoryRaceBossQuestOther(sb, num, tch, ch, OTHER_ACHIE, 0, MAX_OTHER_ACHIE,
                                                         45u, "Other Achievements");

                page_string(ch->desc, sb.c_str(), true);
            }
            else if(ch == tch)
            {
                send_to_char("Non hai completato nessun achievement.\n\r", ch);
            }
            else
            {
                act("$N non ha completato nessun achievement.", false, ch, nullptr, tch, TO_CHAR);
            }
        }
        else if(isCommand("spam"))
        {
            char_data* tch = ch;

            static const std::array<const char*, 20> rand_spamAchie = {{
                "$c0011$n$c0007 e' proprio $u sborone:",
                "$c0011$n$c0007 si gigioneggia mostrandoti il suo achievement:",
                "$c0011$n$c0007 ti dice: 'Si ho fatto anche questo!',",
                "$c0011$n$c0007 mostra a tutti il cartello 'W i criceti' e poi dice:",
                "$c0011$n$c0007 canticchia allegr$b 'In the name of my new Achievemeeent!':",
                "$c0011$n$c0007 sogghigna mentre ti mostra che",
                "Inizi a schiumare di rabbia appena $c0011$n$c0007 ti dice che",
                "Ma davero davero? Nuovo achievement di $c0011$n$c0007? WTF,",
                "$c0011$n$c0007 si muove come $u supercafone mentre esclama che",
                "Mentre tu stai a rosica' $c0011$n$c0007 urla 'Daje!',",
                "Fai lo sborone e mostri a tutti che",
                "Ti gigioneggi mentre fai vedere il tuo achievement:",
                "Dici: 'E anche questo l'ho fatto,",
                "Mostri il cartello 'W i criceti', poi dici che",
                "Canti allegramente 'In the name of my new Achievemeeeent!':",
                "Sogghigni mentre mostri a tutti che",
                "Fai schiumare i presenti dalla rabbia appena esclami che",
                "Fai rimanere tutti a bocca aperta:",
                "Con il passo da $c5011K$c0007oatto esclami che",
                "Esclami 'Daje!' e fai rosicare tutti urlando che"
            }};

            constexpr std::size_t spamVariantsPerTarget = rand_spamAchie.size() / 2;
            constexpr int nMaxSpamAchie = static_cast<int>(spamVariantsPerTarget - 1);

            arg = one_argument(arg, arg2.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements spam <numero>\n\r", ch);
                return;
            }

            if(arg2[0] == '\0')
            {
                send_to_char("Quale achievement vuoi mostrare? Digita 'achievements all' per vedere il numero.\n\r", ch);
                return;
            }

            int n_spam = 0;
            if(!parsePositiveIntArg(arg2.data(),
                "Il numero achievement deve essere un intero valido.\n\r",
                "Il numero achievement inserito e' troppo alto.\n\r",
                "Quale achievement vuoi mostrare? Digita 'achievements all' per vedere il numero.\n\r",
                n_spam))
            {
                return;
            }

            if(!applyPolyOriginalOrAbort(tch)) {
                return;
            }

            if(n_spam > maxAchievements(tch))
            {
                std::string msgToChar = "Il numero massimo di achievements per te e' ";
                msgToChar += std::to_string(maxAchievements(tch));
                msgToChar += ".\n\r";
                send_to_char(msgToChar.c_str(), ch);
                return;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE))
            {
                const auto slot = findAchievementSlotByNumber(tch, tch, n_spam, true);
                if(!slot.has_value())
                {
                    send_to_char("Quell'achievement esiste solo nella tua fantasia...\n\r", ch);
                    return;
                }
                const int displayNum = slot->baseNum + 1;
                sb = bufferAchie(tch,
                                 slot->index,
                                 slot->category,
                                 slot->levels,
                                 displayNum,
                                 true,
                                 n_spam);

                const int spamIndex = number(0, nMaxSpamAchie);
                const std::size_t spamIndexSz = static_cast<std::size_t>(spamIndex);
                constexpr std::size_t selfSpamOffset = spamVariantsPerTarget;
                constexpr char completedPrefix[] = "com";
                constexpr std::size_t completedPrefixLen = sizeof(completedPrefix) - 1;
                const bool completedVerb = (sb.compare(0, completedPrefixLen, completedPrefix) == 0);
                const char* const toCharAux = completedVerb ? "hai" : "ti";
                const char* const toRoomAux = completedVerb ? "ha" : "$d";
                const auto emitSpamLine = [&](const char* prefix, const char* aux, int target) {
                    std::string spamLine = prefix;
                    spamLine += ' ';
                    spamLine += aux;
                    spamLine += ' ';
                    spamLine += sb;
                    spamLine += '.';
                    act(spamLine.c_str(), false, ch, nullptr, nullptr, target);
                };

                emitSpamLine(rand_spamAchie[spamIndexSz + selfSpamOffset], toCharAux, TO_CHAR);
                emitSpamLine(rand_spamAchie[spamIndexSz], toRoomAux, TO_ROOM);
            }
            else
            {
                send_to_char("Ma se non hai completato nessun achievement...\n\r", ch);
            }
        }
        else if(isCommand("delete") && IS_QUESTMASTER(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::string msgToChar;
            std::string msgToVict;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements delete <nome_pg> <numero>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti cancellare gli achievements?\n\r",
                    "Vuoi veramente cancellare gli achivements ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    int achi_d = 0;
                    if(!parsePositiveIntArg(arg3.data(),
                        "Il numero achievement deve essere un intero valido.\n\r",
                        "Il numero achievement inserito e' troppo alto.\n\r",
                        "Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.\n\r",
                        achi_d))
                    {
                        return;
                    }

                    if(achi_d > maxAchievements(tch))
                    {
                        std::string msgToChar = "Il numero massimo di achievements per $N e' ";
                        msgToChar += std::to_string(maxAchievements(tch));
                        msgToChar += ".";
                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(IS_SET(tch->specials.act, PLR_ACHIE))
                    {
                        const auto setAlreadyZeroMsg = [&](const char* achievementName) {
                            msgToChar = "L'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N e' gia' 0.";
                        };
                        const auto setDeletedMsgs = [&](const char* achievementName) {
                            msgToChar = "Hai cancellato l'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N.";
                            msgToVict = "$n ti ha cancellato l'achievement del numero di ";
                            msgToVict += achievementName;
                            msgToVict += ".";
                        };
                        const auto slot = findAchievementSlotByNumber(tch, ch, achi_d, false);
                        if(!slot.has_value())
                        {
                            act("$N non ha completato nessun achievement!", false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        const int category = slot->category;
                        const int index = slot->index;
                        if(tch->specials.achievements[category][index] == 0)
                        {
                            setAlreadyZeroMsg(AchievementsList[category][index].achie_string2);
                            act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        tch->specials.achievements[category][index] = 0;
                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s",
                               GET_NAME(ch), AchievementsList[category][index].achie_string2, GET_NAME(tch));
                        setDeletedMsgs(AchievementsList[category][index].achie_string2);
                        do_save(tch, "", 0);

                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                            act(msgToVict.c_str(), false, ch, nullptr, vict, TO_VICT);
                        }
                    }
                    else
                    {
                        act("$N non ha completato nessun achievement!", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti cancellare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("set") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::array<char, 128> arg4{};
            std::string msgToChar;
            std::string msgToVict;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());
            arg = one_argument(arg, arg4.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements set <nome_pg> <numero> <valore>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui!\n\r",
                    "Vuoi veramente assegnare un achivement ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    int achi_n = 0;
                    if(!parsePositiveIntArg(arg3.data(),
                        "Il numero achievement deve essere un intero valido.\n\r",
                        "Il numero achievement inserito e' troppo alto.\n\r",
                        "A quale achievement di $N vuoi assegnare un valore? Il numero lo vedi con $c0015achievement all $N$c0007.\n\r",
                        achi_n))
                    {
                        return;
                    }

                    if(achi_n > maxAchievements(tch))
                    {
                        msgToChar = "Il numero massimo di achievements per $N e' ";
                        msgToChar += std::to_string(maxAchievements(tch));
                        msgToChar += ".";
                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(arg4[0] != '\0')
                    {
                        int achi_v = 0;
                        if(!parsePositiveIntArg(arg4.data(),
                            "Il valore deve essere un numero intero valido.\n\r",
                            "Il valore inserito e' troppo alto.\n\r",
                            "Puoi assegnare solo un valore maggiore di zero.\n\r",
                            achi_v))
                        {
                            return;
                        }

                        if(!IS_SET(tch->specials.act, PLR_ACHIE))
                        {
                            SET_BIT(tch->specials.act, PLR_ACHIE);
                        }
                        const auto setMaxAssignableMsg = [&](const char* achievementName, const int maxValue) {
                            msgToChar = "Il numero massimo di ";
                            msgToChar += achievementName;
                            msgToChar += " assegnabili e' ";
                            msgToChar += std::to_string(maxValue);
                            msgToChar += ".";
                        };
                        const auto setAssignedMsgs = [&](const char* achievementName, const int assignedValue) {
                            msgToChar = "Hai assegnato all'achievement del numero di ";
                            msgToChar += achievementName;
                            msgToChar += " di $N un valore pari a ";
                            msgToChar += std::to_string(assignedValue);
                            msgToChar += ".";
                            msgToVict = "$n ti ha assegnato all'achievement del numero di ";
                            msgToVict += achievementName;
                            msgToVict += " un valore pari a ";
                            msgToVict += std::to_string(assignedValue);
                            msgToVict += ".";
                        };
                        const auto slot = findAchievementSlotByNumber(tch, ch, achi_n, false);
                        if(!slot.has_value())
                        {
                            act("Quale achievement di $N vuoi assegnare? Il numero lo vedi con $c0015achievement all $N$c0007.",
                                false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }

                        const int category = slot->category;
                        const int index = slot->index;
                        const int maxAssignableValue = MaxValueAchievement(category, index, slot->levels);
                        if(achi_v > maxAssignableValue)
                        {
                            setMaxAssignableMsg(AchievementsList[category][index].achie_string2, maxAssignableValue);
                            act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }
                        tch->specials.achievements[category][index] = achi_v;
                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d",
                               GET_NAME(ch), AchievementsList[category][index].achie_string2, GET_NAME(tch), achi_v);
                        setAssignedMsgs(AchievementsList[category][index].achie_string2, achi_v);
                        do_save(tch, "", 0);

                        act(msgToChar.c_str(), false, ch, nullptr, tch, TO_CHAR);
                        if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                            act(msgToVict.c_str(), false, ch, nullptr, vict, TO_VICT);
                        }
                    }
                    else
                    {
                        act("Quale valore vuoi assegnare all'achievement di $N?", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi assegnare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti assegnare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("reset") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;

            arg = one_argument(arg, arg2.data());
            while(*arg == ' ')
            {
                ++arg;
            }

            if(*arg != '\0')
            {
                send_to_char("Sintassi: achievements reset <nome_pg>\n\r", ch);
                return;
            }

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti resettare gli achievements?\n\r",
                    "Vuoi veramente resettare gli achievements di uno stupido mob?\n\r"))
                {
                    return;
                }

                mudlog(LOG_PLAYERS, "%s starts to delete all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                const auto resetAchievementCategory = [&](const int category, const int maxCount) {
                    if(hasAchievement(tch, category, 1))
                    {
                        for(int idx = 0; idx < maxCount; ++idx)
                        {
                            tch->specials.achievements[category][idx] = 0;
                        }
                    }
                };
                struct AchievementCategorySpec
                {
                    int category;
                    int maxCount;
                };
                constexpr std::array<AchievementCategorySpec, 5> achievementCategories = {{
                    {RACESLAYER_ACHIE, MAX_RACE_ACHIE},
                    {BOSSKILL_ACHIE, MAX_BOSS_ACHIE},
                    {CLASS_ACHIE, MAX_CLASS_ACHIE},
                    {QUEST_ACHIE, MAX_QUEST_ACHIE},
                    {OTHER_ACHIE, MAX_OTHER_ACHIE}
                }};
                for(const auto& categorySpec : achievementCategories)
                {
                    resetAchievementCategory(categorySpec.category, categorySpec.maxCount);
                }
                if(IS_SET(tch->specials.act, PLR_ACHIE))
                {
                    REMOVE_BIT(tch->specials.act, PLR_ACHIE);
                }
                mudlog(LOG_PLAYERS, "Done. %s has deleted all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                do_save(tch, "", 0);

                act("Tutti gli achievements di $N sono stati resettati.", false, ch, nullptr, tch, TO_CHAR);
                if(char_data* const vict = get_char_vis_world(ch, arg2.data(), nullptr)) {
                    act("$n ti ha resettato tutti gli achievements.", false, ch, nullptr, vict, TO_VICT);
                }
            }
            else
            {
                send_to_char("A chi vorresti resettare gli achievements?\n\r", ch);
                return;
            }
        }
        else
        {
            send_to_char("$c0009Sintassi:\n\r", ch);
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements$c0007                                 - mostra gli achievements che hai completato\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements$c0007               - mostra gli achievements che hai completato\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all$c0007                             - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all $c0007<$c0015nome_pg$c0007>                   - mostra tutti gli achievements del personaggio scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements all$c0007           - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007>                   - mostra in stanza l'achievement scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007> - mostra in stanza l'achievement scelto\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements delete $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007>       - cancella un determinato achievement dal personaggio scelto\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements set $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007> <$c0015valore$c0007> - assegna al personaggio scelto un valore ad un determinato achievement\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements reset $c0007<$c0015nome_pg$c0007>                 - resetta tutti gli achievements del personaggio scelto\n\r", ch);
            }

        }
    }
    else
    {
        char_data* tch = ch;

        if(!applyPolyOriginalOrAbort(tch)) {
            return;
        }

        if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
        {
            int num = 0;
            send_to_char("\n\r$c0011Ecco i tuoi achievements:\n\r", ch);

            AppendCompactAchievementCategory(sb, num, tch, RACESLAYER_ACHIE, 0, MAX_RACE_ACHIE, 0, 40u,
                                             "Race  Achievements");
            AppendCompactAchievementCategory(sb, num, tch, BOSSKILL_ACHIE, 0, MAX_BOSS_ACHIE, 0, 40u,
                                             "Boss  Achievements");
            AppendCompactAchievementCategory(sb, num, tch, CLASS_ACHIE, 1, MAX_CLASS_ACHIE, 0, 40u,
                                             "Skill Achievements");
            AppendCompactAchievementCategory(sb, num, tch, QUEST_ACHIE, 0, MAX_QUEST_ACHIE, 0, 40u,
                                             "Quest Achievements");
            AppendCompactAchievementCategory(sb, num, tch, OTHER_ACHIE, 0, MAX_OTHER_ACHIE, 0, 40u,
                                             "Other Achievements");

            page_string(ch->desc, sb.c_str(), true);
        }
        else {
            send_to_char("Non hai completato nessun achievement.\n\r", ch);
        }
    }
}

ACTION_FUNC(do_score) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_score (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	struct time_info_data playing_time;
	struct time_info_data my_age;
	struct time_info_data my_birth;
	struct ScoreClassSpec {
		int classId;
		int levelIndex;
		char shortLabel;
	};
	const auto actToChar = [ch](const char* msg) {
		act(msg, FALSE, ch, nullptr, nullptr, TO_CHAR);
	};

	int weekday, day;
	my_birth = mud_time_passed(beginning_of_time, ch->player.time.birth);
	age3(ch, &my_age);
	weekday = ((35 * my_birth.month) + my_birth.day + 1) % 7;/* 35 days in a month */
	day = my_birth.day + 1;   /* day in [1..35] */
	if(IS_IMMENSO(ch)) {
		my_birth.year = -1;
		my_age.year = time_info.year + 1;
		my_age.ayear = my_age.year;
	}
	else {
		my_birth.year = (time_info.year - my_age.ayear);
	}
	std::string ageMsg = "$c0005Sei nat$b nel ";
	ageMsg += weekdays[weekday] + 3;
	ageMsg += "$c0005, ";
	ageMsg += std::to_string(day);
	ageMsg += "^ del ";
	ageMsg += month_name[static_cast<int>(my_birth.month)];
	ageMsg += "$c0005, nell'anno ";
	ageMsg += std::to_string(static_cast<int>(my_birth.year));
	ageMsg += " ";
	ageMsg += (my_birth.year > 0) ? "dopo Nebbie" : "avanti Nebbie";
	ageMsg += ".";
	ageMsg += "\n\rHai $c0015";
	ageMsg += std::to_string(static_cast<int>(my_age.ayear));
	ageMsg += "$c0005 anni.";
	if(my_age.year != my_age.ayear) {
		ageMsg += "\nMa ne dimostri $c0015";
		ageMsg += std::to_string(static_cast<int>(my_age.year));
		ageMsg += "$c0005.";
	}

	if((my_age.month == my_birth.month) && (my_age.day == my_birth.day)) {
		ageMsg += "\n$c0015 Oggi e' il tuo compleanno!!";
	}

	act(ageMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string raceMsg = "$c0005Appartieni alla razza $c0015";
	raceMsg += RaceName[GET_RACE(ch)];
	raceMsg += "$c0005";
	act(raceMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	if(HAS_PRINCE(ch)) {
		std::string princeMsg = "$c0005Appartieni al clan di $c0015";
		princeMsg += GET_PRINCE(ch);
		princeMsg += "$c0005";
		act(princeMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}

	if(!IS_IMMORTAL(ch) && !IS_NPC(ch)) {
		if(GET_COND(ch, DRUNK) > 10) {
			actToChar("$c0011Sei sbronz$b.");
		}
		if(GET_COND(ch, FULL) < 2 && GET_COND(ch, FULL) != -1) {
			actToChar("$c0005Hai $c0015fame$c0005...");
		}
		if(GET_COND(ch, THIRST) < 2 && GET_COND(ch, THIRST) != -1) {
			actToChar("$c0005Hai $c0015sete$c0005...");
		}
	}

	std::string statsMsg = "$c0005Hai $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_HIT(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_HIT(ch));
	statsMsg += "$c0005) hit, $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_MANA(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_MANA(ch));
	statsMsg += "$c0005) mana e $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_MOVE(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_MOVE(ch));
	statsMsg += "$c0005) movimento.";
	act(statsMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string alignMsg = "$c0005Il tuo allineamento e': $c0015";
	alignMsg += AlignDesc(GET_ALIGNMENT(ch));
	act(alignMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string expMsg = "$c0005Hai effettuato $c0015";
	expMsg += std::to_string(GET_EXP(ch));
	expMsg += "$c0005 exp, ed hai $c0015";
	expMsg += std::to_string(GET_GOLD(ch));
	expMsg += "$c0005 monete d'oro.";
	act(expMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string donateMsg = "$c0005Non puoi donare xp se hai meno di: $c0015";
	donateMsg += std::to_string(IS_PRINCE(ch) ? PRINCEEXP : MIN_EXP(ch));
	donateMsg += "$c0005 exp";
	act(donateMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	/* the mud will crash without this check! */
	if(GetMaxLevel(ch)>MAX_MORT ||
			(IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF))) {
		/* do nothing! */
	}
	else {
		static constexpr std::array<ScoreClassSpec, 11> nextLevelClassOrder = {{
			{CLASS_MAGIC_USER, MAGE_LEVEL_IND, 'M'},
			{CLASS_CLERIC, CLERIC_LEVEL_IND, 'C'},
			{CLASS_THIEF, THIEF_LEVEL_IND, 'T'},
			{CLASS_WARRIOR, WARRIOR_LEVEL_IND, 'W'},
			{CLASS_DRUID, DRUID_LEVEL_IND, 'D'},
			{CLASS_MONK, MONK_LEVEL_IND, 'K'},
			{CLASS_BARBARIAN, BARBARIAN_LEVEL_IND, 'B'},
			{CLASS_SORCERER, SORCERER_LEVEL_IND, 'S'},
			{CLASS_PALADIN, PALADIN_LEVEL_IND, 'P'},
			{CLASS_RANGER, RANGER_LEVEL_IND, 'R'},
			{CLASS_PSI, PSI_LEVEL_IND, 'I'}
		}};

		std::string nextLevelMsg = "$c0005Exp al prossimo livello: ";
		const bool canGainLevels = (GetMaxLevel(ch) < MAX_IMMORT);
		for(const ScoreClassSpec& classSpec : nextLevelClassOrder) {
			if(!HasClass(ch, classSpec.classId)) {
				continue;
			}
			nextLevelMsg += classSpec.shortLabel;
			nextLevelMsg += ':';
			if(canGainLevels) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string(
					(titles[classSpec.levelIndex][GET_LEVEL(ch, classSpec.levelIndex) + 1].exp) -
					GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		act(nextLevelMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}

	static constexpr std::array<ScoreClassSpec, 11> levelsClassOrder = {{
		{CLASS_MAGIC_USER, MAGE_LEVEL_IND, 'M'},
		{CLASS_CLERIC, CLERIC_LEVEL_IND, 'C'},
		{CLASS_WARRIOR, WARRIOR_LEVEL_IND, 'W'},
		{CLASS_THIEF, THIEF_LEVEL_IND, 'T'},
		{CLASS_DRUID, DRUID_LEVEL_IND, 'D'},
		{CLASS_MONK, MONK_LEVEL_IND, 'K'},
		{CLASS_BARBARIAN, BARBARIAN_LEVEL_IND, 'B'},
		{CLASS_SORCERER, SORCERER_LEVEL_IND, 'S'},
		{CLASS_PALADIN, PALADIN_LEVEL_IND, 'P'},
		{CLASS_RANGER, RANGER_LEVEL_IND, 'R'},
		{CLASS_PSI, PSI_LEVEL_IND, 'I'}
	}};

	std::string levelsMsg = "$c0005I tuoi livelli:";
	for(const ScoreClassSpec& classSpec : levelsClassOrder) {
		if(!HasClass(ch, classSpec.classId)) {
			continue;
		}
		levelsMsg += ' ';
		levelsMsg += classSpec.shortLabel;
		levelsMsg += ":$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, classSpec.levelIndex)));
		levelsMsg += "$c0005";
	}

	act(levelsMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	if(GET_TITLE(ch)) {
		std::string titleMsg = "$c0005Questo ti qualifica come $c0015";
		titleMsg += GET_NAME(ch);
		titleMsg += " $c0011";
		titleMsg += GET_TITLE(ch);
		act(titleMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
	}

	playing_time = real_time_passed((time(0)-ch->player.time.logon) + ch->player.time.played, 0);
	std::string playingMsg = "$c0005Hai giocato per $c0015";
	playingMsg += std::to_string(static_cast<int>(playing_time.day));
	playingMsg += "$c0005 giorni e $c0015";
	playingMsg += std::to_string(static_cast<int>(playing_time.hours));
	playingMsg += "$c0005 ore.";
	act(playingMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	/* Drow fight -4 in lighted rooms! */
	if(!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DARK_ELF &&
			!affected_by_spell(ch, SPELL_GLOBE_DARKNESS) && !IS_UNDERGROUND(ch)) {
		actToChar("$c0011La luce nell'area ti provoca molto dolore$c0009!");
	}

	std::string settingsMsg = "$c0005I tuoi set sono: "
		"$c0015Achie$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, ACHIE_MODE) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Ansi$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, USE_ANSI) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Autoexits$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, SHOW_EXITS) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Pwp$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, PWP_MODE) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Warnings$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, WARNINGS_MODE_OFF) ? "off" : "on";
	settingsMsg += "$c0005) $c0015Who$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, SHOW_CLASSES) ? "on" : "off";
	settingsMsg += "$c0005)";
	act(settingsMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	std::string runesMsg = "$c0005Le rune degli Dei tatuate sul tuo corpo sono: $c0015";
	runesMsg += std::to_string(static_cast<int>(GET_RUNEDEI(ch)));
	runesMsg += " ";
	act(runesMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);

	switch(GET_POS(ch)) {
	case POSITION_DEAD :
		actToChar("$c0009Sei mort$b!");
		break;
	case POSITION_MORTALLYW :
		actToChar("$c0009Sei ferit$b a morte e dovresti cercare aiuto!");
		break;
	case POSITION_INCAP :
		actToChar("$c0009Sei incapacitat$b, e stai morendo lentamente");
		break;
	case POSITION_STUNNED :
		actToChar("$c0011Sei svenut$b! Non ti puoi muovere.");
		break;
	case POSITION_SLEEPING :
		actToChar("$c0010Stai dormendo.");
		break;
	case POSITION_RESTING  :
		actToChar("$c0012Stai riposando.");
		break;
	case POSITION_SITTING  :
		actToChar("$c0013Sei sedut$b.");
		break;
	case POSITION_FIGHTING :
		if(ch->specials.fighting) {
			act("$c1009Stai combattendo contro $N.", FALSE, ch, nullptr,
				ch->specials.fighting, TO_CHAR);
		}
		else {
			actToChar("$c1009Stai combattendo contro l'aria.");
		}
		break;
	case POSITION_STANDING :
		actToChar("$c0005Sei in piedi.");
		break;
	case POSITION_MOUNTED:
		if(MOUNTED(ch)) {
			std::string mountedMsg = "$c0005Stai montando su $c0015";
			mountedMsg += MOUNTED(ch)->player.short_descr;
			act(mountedMsg.c_str(), FALSE, ch, nullptr, nullptr, TO_CHAR);
		}
		else {
			actToChar("$c0005Sei in piedi.");
		}
		break;
	default :
		actToChar("$c0005Sta fluttuando.");
		break;
	}
}


ACTION_FUNC(do_time) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_time (act.info.cpp)");
		return;
	}

	std::string buf = "Sono le ";
	const int hour12 = static_cast<int>(time_info.hours) % 12;
	buf += std::to_string(hour12 == 0 ? 12 : hour12);
	buf += " del";
	buf += (time_info.hours >= 12) ? " pomeriggio" : "la mattina";
	buf += ", de ";

	const int weekday = ((35 * time_info.month) + time_info.day + 1) % 7; /* 35 days in a month */
	buf += weekdays[weekday];
	buf += ".\n\r";
	send_to_char(buf.c_str(), ch);

	const int day = time_info.day + 1;   /* day in [1..35] */
	buf = std::to_string(day);
	buf += "^ giorno del ";
	buf += month_name[static_cast<int>(time_info.month)];
	buf += ", nell'anno ";
	buf += std::to_string(static_cast<int>(time_info.year));
	buf += ".\n\r";
	send_to_char(buf.c_str(), ch);
}


ACTION_FUNC(do_weather) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_weather (act.info.cpp)");
		return;
	}

	std::string weatherMsg;
	static constexpr std::array<const char*, 4> skyLook = {{
		"$c0014sereno$c0007",
		"$c0015nuvoloso$c0007",
		"$c0012piovoso$c0007",
		"$c0011illuminato dai lampi$c0007"
	}};

	if(OUTSIDE(ch)) {
		weatherMsg = "Il cielo e' ";
		weatherMsg += skyLook[weather_info.sky];
		weatherMsg += " e";
		weatherMsg += (weather_info.change >= 0 ?
			" senti un vento $c0009caldo$c0007 da sud" :
			"d i tuoi reumatismi ti dicono che il tempo cambiera'");
		weatherMsg += ".\n\r";
		send_to_char(weatherMsg.c_str(), ch);
	}
	else {
		send_to_char("Non riesci a vedere il tempo che fa fuori.\n\r", ch);
	}
}

namespace {
bool ShowIndexedHelpEntry(struct char_data* ch,
                          const char* arg,
                          struct help_index_element* index,
                          int top,
                          FILE* fl,
                          const char* unavailableMsg,
                          const char* notFoundMsg) {
	if(arg == nullptr || *arg == '\0') {
		return false;
	}
	if(index == nullptr) {
		send_to_char(unavailableMsg, ch);
		return true;
	}

	int bot = 0;
	while(true) {
		const int mid = (bot + top) / 2;
		const int minlen = static_cast<int>(std::strlen(arg));
		const int chk = strn_cmp(arg, index[mid].keyword, minlen);
		if(chk == 0) {
			fseek(fl, index[mid].pos, 0);
			std::array<char, 80> lineBuf{};
			std::string buffer;
			while(true) {
				fgets(lineBuf.data(), static_cast<int>(lineBuf.size()), fl);
				if(lineBuf[0] == '#') {
					break;
				}
				if(buffer.size() + std::strlen(lineBuf.data()) + 1 > MAX_STRING_LENGTH - 2) {
					break;
				}
				buffer += lineBuf.data();
				buffer += "\r";
			}
			page_string(ch->desc, buffer.c_str(), true);
			return true;
		}
		if(bot >= top) {
			send_to_char(notFoundMsg, ch);
			return true;
		}
		if(chk > 0) {
			bot = mid + 1;
		}
		else {
			top = mid - 1;
		}
	}
}
} // namespace


ACTION_FUNC(do_help) {

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_help (act.info.cpp)");
		return;
	}

	if(ch->desc == nullptr) {
		return;
	}

	if(arg == nullptr) {
		arg = "";
	}
	for(; std::isspace(static_cast<unsigned char>(*arg)); ++arg) {
	}

	if(ShowIndexedHelpEntry(ch, arg, help_index, top_of_helpt, help_fl,
	                        "L'help non e' disponibile.\n\r",
	                        "Non c'e' aiuto per quella parola.\n\r")) {
		return;
	}
	if(help[0] == '\0') {
		send_to_char("L'help non e' disponibile.\n\r", ch);
		return;
	}

	page_string(ch->desc, help, true);
	/*send_to_char(help, ch);*/

}

namespace {
void AppendCommandColumns(std::string& out,
                          struct char_data* ch,
                          int minCommandLevel,
                          bool enforceMaxStringLength) {
	/* Longest registered name is 20 chars ("psionic invisibility"); 4 cols × 20 ≈ 80. */
	constexpr int kCommandColumnWidth = 20;
	constexpr int kCommandsPerRow = 4;

	int columnIndex = 1;
	for(int i = 0; i < 27; ++i) {
		for(NODE* n = radix_head[i].next; n != nullptr; n = n->next) {
			if(n->min_level > GetMaxLevel(ch) || n->min_level < minCommandLevel) {
				continue;
			}

			std::ostringstream formattedName;
			formattedName << std::left << std::setw(kCommandColumnWidth) << n->name;
			const std::string commandColumn = formattedName.str();
			if(!enforceMaxStringLength ||
			   (out.size() + commandColumn.size() <= MAX_STRING_LENGTH)) {
				out += commandColumn;
			}
			if(!(columnIndex % kCommandsPerRow)) {
				if(!enforceMaxStringLength || (out.size() + 2 <= MAX_STRING_LENGTH)) {
					out += "\n\r";
				}
			}
			++columnIndex;
		}
	}

	if(!enforceMaxStringLength || (out.size() + 2 <= MAX_STRING_LENGTH)) {
		out += "\n\r";
	}
}
} // namespace


ACTION_FUNC(do_wizhelp) {
	std::array<char, 1000> queryBuf{};

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wizhelp (act.info.cpp)");
		return;
	}

	if(IS_NPC(ch)) {
		return;
	}

	one_argument(arg, queryBuf.data());                /* new msw */
	if(queryBuf[0] != '\0') {
		/* asking for help on keyword, try looking in file */
		do_actual_wiz_help(ch, arg, cmd);
		return;
	}

	std::string buf = "Wizhelp <keyword>\n\rWizard Commands disponibili per te:\n\r\n\r";
	AppendCommandColumns(buf, ch, IMMORTALE, false);

	page_string(ch->desc, buf.c_str(), true);
}

ACTION_FUNC(do_actual_wiz_help) {

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_actual_wiz_help (act.info.cpp)");
		return;
	}

	if(ch->desc == nullptr) {
		return;
	}

	if(arg == nullptr) {
		arg = "";
	}
	for(; std::isspace(static_cast<unsigned char>(*arg)); ++arg) {
	}

	if(ShowIndexedHelpEntry(ch, arg, wizhelp_index, top_of_wizhelpt, wizhelp_fl,
	                        "Il wizhelp non e' disponibile.\n\r",
	                        "Non esiste aiuto per questa parola.\n\r")) {
		return;
	}
	/* send a generic wizhelp menu like help I guess send_to_char(help, ch); */
}


ACTION_FUNC(do_command_list) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_command_list (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	if(IS_NPC(ch)) {
		return;
	}

	std::string buf = "Comandi disponibili per te:\n\r\n\r";
	AppendCommandColumns(buf, ch, 0, true);

	page_string(ch->desc, buf.c_str(), true);
}

namespace {
/* Centra la stringa colorata livello+classi nel campo a 34 caratteri (logica who originale). */
std::string who_center_label34(const std::string& inner) {
	std::string pad(34, ' ');
	const int len = static_cast<int>(inner.size());
	int start = 17 - len / 2;
	if(start < 0) {
		start = 0;
	}
	const int n = std::min(len, 34 - start);
	for(int i = 0; i < n; ++i) {
		pad[static_cast<size_t>(start + i)] = inner[static_cast<size_t>(i)];
	}
	return pad;
}

constexpr std::size_t kWhoBufSoftLimit = (MAX_STRING_LENGTH * 2) - 512;

bool append_who_line_if_fits(std::string& whoBuf, const std::string& line) {
	if(whoBuf.size() + line.size() >= kWhoBufSoftLimit) {
		return false;
	}
	whoBuf += line;
	return true;
}

void append_who_visible_totals_if_fits(std::string& whoBuf, int count, bool godsOnly) {
	std::string totalsLine;
	if(godsOnly) {
		totalsLine = "\n\r$c0005Totale Divinita' visibili: $c0015" + std::to_string(count) + "\n\r";
	}
	else {
		totalsLine = "\n\r$c0005Totale giocatori visibili: $c0015" + std::to_string(count) + "\n\r";
	}
	append_who_line_if_fits(whoBuf, totalsLine);
}

void append_who_maxusage_if_fits(std::string& whoBuf) {
	const std::string maxLine =
		"\n\r$c0005Max giocatori connessi dall'ultimo reboot: $c0015"
		+ std::to_string(update_max_usage()) + "\r\n";
	append_who_line_if_fits(whoBuf, maxLine);
}

struct GodWhoFilters {
	bool gods = false;
	bool mortals = false;
	bool mage = false;
	bool cleric = false;
	bool warrior = false;
	bool thief = false;
	bool druid = false;
	bool monk = false;
	bool barbarian = false;
	bool sorcerer = false;
	bool paladin = false;
	bool ranger = false;
	bool psi = false;
	bool showLinkdead = false;
};

GodWhoFilters build_god_who_filters(const std::string& flags) {
	const auto hasFlag = [&flags](char f) {
		return flags.find(f) != std::string::npos;
	};
	GodWhoFilters filters;
	filters.gods = hasFlag('g');
	filters.mortals = hasFlag('o');
	filters.mage = hasFlag('1');
	filters.cleric = hasFlag('2');
	filters.warrior = hasFlag('3');
	filters.thief = hasFlag('4');
	filters.druid = hasFlag('5');
	filters.monk = hasFlag('6');
	filters.barbarian = hasFlag('7');
	filters.sorcerer = hasFlag('8');
	filters.paladin = hasFlag('9');
	filters.ranger = hasFlag('!');
	filters.psi = hasFlag('@');
	filters.showLinkdead = hasFlag('d');
	return filters;
}

bool should_skip_god_who_target(struct char_data* person, const GodWhoFilters& filters) {
	return (filters.gods && !IS_IMMORTAL(person))
	       || (filters.mortals && IS_IMMORTAL(person))
	       || (filters.mage && !HasClass(person, CLASS_MAGIC_USER))
	       || (filters.cleric && !HasClass(person, CLASS_CLERIC))
	       || (filters.warrior && !HasClass(person, CLASS_WARRIOR))
	       || (filters.thief && !HasClass(person, CLASS_THIEF))
	       || (filters.druid && !HasClass(person, CLASS_DRUID))
	       || (filters.monk && !HasClass(person, CLASS_MONK))
	       || (filters.barbarian && !HasClass(person, CLASS_BARBARIAN))
	       || (filters.sorcerer && !HasClass(person, CLASS_SORCERER))
	       || (filters.paladin && !HasClass(person, CLASS_PALADIN))
	       || (filters.ranger && !HasClass(person, CLASS_RANGER))
	       || (filters.psi && !HasClass(person, CLASS_PSI));
}

void append_god_who_detail_flag(std::string& godLine, char fc, struct char_data* person) {
	switch(fc) {
	case 'r': {
		std::array<char, 256> raceBuf{};
		sprinttype((person->race), RaceName, raceBuf.data());
		godLine += " [";
		godLine += raceBuf.data();
		godLine += "] ";
		break;
	}
	case 'e': {
		std::ostringstream out;
		out << "Eq:[" << GetCharBonusIndex(person) << "] ";
		godLine += out.str();
		break;
	}
	case 'a': {
		std::ostringstream out;
		out << "Al:[" << GET_ALIGNMENT(person) << "] ";
		godLine += out.str();
		break;
	}
	case 'i': {
		std::ostringstream out;
		out << "Idle:[" << std::left << std::setw(3) << person->specials.timer << "] ";
		godLine += out.str();
		break;
	}
	case 'l': {
		const auto lvl = [&](int idx) {
			return static_cast<int>(person->player.level[idx]);
		};
		std::ostringstream out;
		out << "Level:["
		    << std::left << std::setw(2) << lvl(0) << "/"
		    << std::left << std::setw(2) << lvl(1) << "/"
		    << std::left << std::setw(2) << lvl(2) << "/"
		    << std::left << std::setw(2) << lvl(3) << "/"
		    << std::left << std::setw(2) << lvl(4) << "/"
		    << std::left << std::setw(2) << lvl(5) << "/"
		    << std::left << std::setw(2) << lvl(6) << "/"
		    << std::left << std::setw(2) << lvl(7) << "/"
		    << std::left << std::setw(2) << lvl(8) << "/"
		    << std::left << std::setw(2) << lvl(9) << "/"
		    << std::left << std::setw(2) << lvl(10) << "] ";
		godLine += out.str();
		break;
	}
	case 'h': {
		std::ostringstream out;
		out << "Hit:[" << std::left << std::setw(3) << static_cast<int>(GET_HIT(person))
		    << "] Mana:[" << std::left << std::setw(3) << static_cast<int>(GET_MANA(person))
		    << "] Move:[" << std::left << std::setw(3) << static_cast<int>(GET_MOVE(person)) << "] ";
		godLine += out.str();
		break;
	}
	case 'c':
		if(HAS_PRINCE(person)) {
			godLine += "del clan di ";
			godLine += GET_PRINCE(person);
		}
		break;
	case 's':
		if(GET_STR(person) != 18) {
			std::ostringstream out;
			out << "[S:" << std::left << std::setw(2) << static_cast<int>(GET_STR(person))
			    << " I:" << std::left << std::setw(2) << static_cast<int>(GET_INT(person))
			    << " W:" << std::left << std::setw(2) << static_cast<int>(GET_WIS(person))
			    << " C:" << std::left << std::setw(2) << static_cast<int>(GET_CON(person))
			    << " D:" << std::left << std::setw(2) << static_cast<int>(GET_DEX(person))
			    << " CH:" << std::left << std::setw(2) << static_cast<int>(GET_CHR(person))
			    << "] ";
			godLine += out.str();
		}
		else {
			std::ostringstream out;
			out << "[S:" << std::left << std::setw(2) << static_cast<int>(GET_STR(person))
			    << "(" << std::setw(1) << static_cast<int>(GET_ADD(person)) << ")"
			    << " I:" << std::left << std::setw(2) << static_cast<int>(GET_INT(person))
			    << " W:" << std::left << std::setw(2) << static_cast<int>(GET_WIS(person))
			    << " C:" << std::left << std::setw(2) << static_cast<int>(GET_CON(person))
			    << " D:" << std::left << std::setw(2) << static_cast<int>(GET_DEX(person))
			    << " CH:" << std::left << std::setw(2) << static_cast<int>(GET_CHR(person))
			    << "] ";
			godLine += out.str();
		}
		break;
	case 't': {
		std::ostringstream out;
		out << " " << std::left << std::setw(16)
		    << (person->player.title ? person->player.title : "(null)") << " ";
		godLine += out.str();
		break;
	}
	default:
		break;
	}
}

void append_god_who_details(std::string& godLine, const std::string& flags, struct char_data* person) {
	for(const char fc : flags) {
		append_god_who_detail_flag(godLine, fc, person);
	}
}

void ShowStaticPagedText(struct char_data* ch, const char* text, const char* commandName) {
	if(ch == nullptr) {
		std::string logMsg = "ch==nullptr in ";
		logMsg += commandName;
		logMsg += " (act.info.cpp)";
		mudlog(LOG_SYSERR, logMsg.c_str());
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	if(text == nullptr) {
		send_to_char("Nessun contenuto disponibile.\n\r", ch);
		return;
	}
	SET_BIT(ch->player.user_flags, USE_PAGING);
	page_string(ch->desc, text, false);
}
} // namespace

ACTION_FUNC(do_who) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_who (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	if(arg == nullptr) {
		arg = "";
	}

	struct char_data* person;
	std::string whoBuf;
	std::array<char, 512> argBuf{};
	int count = 0;
	std::string flags;
	std::string nameMask;
	int listed = 0;
	int lcount = 0;
	int skip = FALSE;

	/*  check for an arg */
	auto storeWhoToken = [&](const char* token) {
		if(token[0] == '-' && token[1] != '\0') {
			flags = token + 1;
			if(flags.size() > 19) {
				flags.resize(19);
			}
		}
		else {
			nameMask = token;
			if(nameMask.size() > 39) {
				nameMask.resize(39);
			}
		}
	};

	arg = one_argument(arg, argBuf.data());
	storeWhoToken(argBuf.data());
	if(*arg) {
		arg = one_argument(arg, argBuf.data());
		storeWhoToken(argBuf.data());
	}

	const auto hasFlag = [&](char f) {
		return flags.find(f) != std::string::npos;
	};
	const auto lowerCopy = [](const std::string& value) {
		std::string out;
		out.reserve(value.size());
		for(const unsigned char ch : value) {
			out.push_back(static_cast<char>(std::tolower(ch)));
		}
		return out;
	};
	const std::string loweredNameMask = lowerCopy(nameMask);
	const auto matchesNameMask = [&](const struct char_data* target) {
		if(loweredNameMask.empty()) {
			return true;
		}
		const char* baseName = GET_NAME(target);
		if(baseName == nullptr) {
			return false;
		}
		const std::string loweredName = lowerCopy(baseName);
		return loweredName.compare(0, loweredNameMask.size(), loweredNameMask) == 0;
	};

	if((IS_DIO_MINORE(ch) && flags.empty()) || !IS_DIO_MINORE(ch) ||
			cmd == CMD_WHOZONE) {
		if(IS_DIO_MINORE(ch)) {
			whoBuf =
				"$c0005                         Giocatori [God Version -? for Help]\n\r"
				"                       -------------------------------------\n\r";
		}
		else if(cmd == CMD_WHOZONE) {
			whoBuf = "$c0005Giocatori\n\r"
					 "---------\n\r";
		}
		else {
			whoBuf =
				"$c0005                         Nelle nebbie........\n\r"
				"                       ---------------------------\n\r";
		}
		count=0;
		for(person = character_list; person; person = person->next) {
			if(!IS_LINKDEAD(person) and IS_PC(person) and
					matchesNameMask(person) and
					person->invis_level <= GetMaxLevel(ch) and
					(cmd != CMD_WHOZONE or
					 (real_roomp(person->in_room) and
					  real_roomp(person->in_room)->zone ==
					  real_roomp(ch->in_room)->zone))and
					(!hasFlag('g') or IS_IMMORTAL(person))) {
				count++;

				std::string row;
				if(cmd == CMD_WHOZONE) {
					/* it's a whozone command */
					if((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch))) {
						std::ostringstream wz;
						wz << "$c0012" << std::left << std::setw(25)
						   << (GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "")
						   << " - "
						   << (IS_AFFECTED(person, AFF_SNEAK) ?
							   "sneaking around" :
							   real_roomp(person->in_room)->name);
						row = wz.str();
						if(GetMaxLevel(ch) >= IMMORTALE) {
							row += " [";
							row += std::to_string(person->in_room);
							row += "]";
						}
					}
				}
				else {
					std::string levelsStr;
					std::string classesStr;
					const char* classname[]= { "Mu","Cl","Wa","Th","Dr","Mo","Ba","So","Pa",
											   "Ra","Ps"
											 };
					int i,total,classn;
					long bit;
					if(GetMaxLevel(person) < IMMORTALE) {
						for(bit=1,i=total=classn=0; i<PSI_LEVEL_IND+1; i++, bit<<=1) {
							if(HasClass(person,bit)) {
								classn++;
								total+=person->player.level[i];
								if(!classesStr.empty()) {
									classesStr += "/";
								}
								if(IS_SET(person->player.user_flags,SHOW_CLASSES)) {
									classesStr += classname[i];
								}
							}
						}
						/*		     total/=classn; */
						/* Modifica Urhar dello who in base alla classe piu' alta e non alla media */
						total= GetMaxLevel(person);
						/* fine modifica */
						if(IS_PRINCE(person) || total == PRINCIPE) {  // SALVO per riconoscere anche il poly
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0008Principe":
								   "$c0008Principessa";
						}
						else if(total<ALLIEVO)  /* 1-10 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0002Novizio":
								   "$c0002Novizia";
						else if(total<APPRENDISTA) {   /* 11-20 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0004Allievo":"$c0004Allieva";
						}
						else if(total<INIZIATO)       /* 21-30 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0006Apprendista":
								   "$c0006Apprendista";
						else if(total<ESPERTO)      /* 31-40 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0014Iniziato":
								   "$c0014Iniziata";
						else if(total<MAESTRO)      /* 41-45 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0009Esperto":
								   "$c0009Esperta";
						else if(total<BARONE)       /* 46-49 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Maestro":
								   "$c0015Maestra";
						else if(GET_EXP(person)<CONTE) /* 50 */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Barone":
								   "$c0015Baronessa";
						else if(GET_EXP(person)<MARCHESE)   /* fino ai 200M */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Conte":
								   "$c0015Contessa";
						else if(GET_EXP(person)<DUCA)       /* 250M */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Marchese":
								   "$c0015Marchesa";
						else if(GET_EXP(person)<GRANDUCA)   /* 300M */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Duca":
								   "$c0015Duchessa";
						else if(GET_EXP(person)<PRINCEEXP)   /* 350M */
							levelsStr = GET_SEX(person)==SEX_MALE?"$c0001Granduca":
								   "$c0001Granduchessa";
						const std::string mid = levelsStr + " $c0012" + classesStr;
						const std::string centered = who_center_label34(mid);
						const char* nm = GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "nemo";
						row = centered + "$c0005: $c0007" + std::string(nm) + " ";
						row += person->player.title ? person->player.title : "";
						row += " $c0007";
						row += HAS_PRINCE(person) ? "del clan di " : "";
						row += HAS_PRINCE(person) && GET_PRINCE(person) ? GET_PRINCE(person) : "";

					}
					else {
						if(GET_SEX(person) == SEX_FEMALE) {
							switch(GetMaxLevel(person)) {
							case 52:
								levelsStr = "Immortale";
								break;
							case 53:
								levelsStr = "Dea Minore";
								break;
							case 54:
								levelsStr = "Dea";
								break;
							case 55:
								levelsStr = "Maestra degli Dei";
								break;
							case 56:
								levelsStr = "Creatrice";
								break;
							case 57:
								levelsStr = "Maestra del Fato";
								break;
							case 58:
								levelsStr = "Maestra del Creato";
								break;
							case 59:
								levelsStr = "Maestra dei Creatori";
								break;
							case 60:
								levelsStr = "Immensa";
								break;
							}
						}
						else {
							switch(GetMaxLevel(person)) {
							case 52:
								levelsStr = "Immortale";
								break;
							case 53:
								levelsStr = "Dio Minore";
								break;
							case 54:
								levelsStr = "Dio";
								break;
							case 55:
								levelsStr = "Maestro degli Dei";
								break;
							case 56:
								levelsStr = "Creatore";
								break;
							case 57:
								levelsStr = "Maestro del Fato";
								break;
							case 58:
								levelsStr = "Maestro del Creato";
								break;
							case 59:
								levelsStr = "Maestro dei Creatori";
								break;
							case 60:
								levelsStr = "Immenso";
								break;
							};
						}
						const std::string immMid = std::string("$c0008") + levelsStr + " $c0005";
						const std::string centered = who_center_label34(immMid);
						std::ostringstream nm10;
						nm10 << std::left << std::setw(10)
							 << (GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "");
						row = centered + "$c0005: $c0011" + nm10.str() + " ";
						row += person->player.title ? person->player.title : "";
						row += "$c0007";
					}
				}
				if(!row.empty()) {
					if(IS_AFFECTED2(person,AFF2_AFK)) {
						row += "$c0008 AF";
					}
					if(IS_SET(person->player.user_flags,RACE_WAR)) {
						row += "$c0001 PK";
					}
					row += " $c0007\n\r";
					append_who_line_if_fits(whoBuf, row);
				}
			}
		}
		append_who_visible_totals_if_fits(whoBuf, count, hasFlag('g'));
		append_who_maxusage_if_fits(whoBuf);
	}
	else {
		/* GOD WHO */


		whoBuf =
			"$c0005Giocatori [God Version -? for Help]\n\r"
			"---------\n\r";

		count=0;
		lcount=0;
		if(hasFlag('?')) {
			send_to_char(whoBuf.c_str(), ch);
			send_to_char("$c0007[-]i=idle l=levels t=title h=hit/mana/move "
						 "s=stats r=race \n\r",ch);
			send_to_char("[-]d=linkdead e=eq g=God o=Mort a=align "
						 "[1]Mage[2]Cleric[3]War[4]Thief[5]Druid\n\r", ch);
			send_to_char("[-][6]Monk[7]Barb[8]Sorc[9]Paladin[!]Ranger[@]Psi\n\r",
						 ch);
			return;
		}
		const GodWhoFilters godFilters = build_god_who_filters(flags);

		for(person = character_list; person; person = person->next) {
			if(!IS_NPC(person) and CAN_SEE(ch, person) and matchesNameMask(person)) {
				count++;
				if(person->desc == nullptr) {
					lcount ++;
				}
				skip = should_skip_god_who_target(person, godFilters);

				if(!skip) {
					std::string godLine;
					if(person->desc == nullptr) {
						if(godFilters.showLinkdead) {
							std::ostringstream line;
							line << "$c0003[" << std::left << std::setw(12) << GET_NAME(person) << "] ";
							godLine = line.str();
							listed++;
						}
					}
					else if(IS_NPC(person) && IS_SET(person->specials.act, ACT_POLYSELF)) {
						std::ostringstream line;
						line << "(" << std::left << std::setw(12) << GET_NAME(person) << ") ";
						godLine = line.str();
						listed++;
					}
					else {
						std::ostringstream line;
						line << "$c0012" << std::left << std::setw(14) << GET_NAME(person) << " ";
						godLine = line.str();
						listed++;
					}
					const bool appendDetail =
						(person->desc != nullptr) || godFilters.showLinkdead;
					if(appendDetail) {
						append_god_who_details(godLine, flags, person);
					}
					if(appendDetail) {
						if(matchesNameMask(person)) {
							append_who_line_if_fits(whoBuf, godLine + "\n\r");
						}
					}
				}
			}
		}

		std::string totalsLine;
		if(listed<=0) {
			totalsLine = "\n\r$c0005Nessun personaggio con le caratteristiche richieste.\n\r";
		}
		else {
			const float linkdeadPct = (count > 0) ? (static_cast<float>(lcount) / static_cast<float>(count)) * 100.0f : 0.0f;
			std::ostringstream totals;
			totals << "\n\r$c0005Totale giocatori / Link dead ["
				   << count << "/" << lcount << "] ("
				   << std::fixed << std::setprecision(0) << linkdeadPct
				   << "%)\n\r";
			totalsLine = totals.str();
		}
		append_who_line_if_fits(whoBuf, totalsLine);
		append_who_maxusage_if_fits(whoBuf);
	}
	page_string(ch->desc, whoBuf.c_str(), true);
}

ACTION_FUNC(do_users) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_users (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::string buf = "Connessioni:\n\r------------\n\r";

	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		const bool canInspectByImmense = (GetMaxLevel(ch) == IMMENSO);
		const bool canInspectVisibleCharacter =
			(d->character != nullptr) && CAN_SEE(ch, d->character);
		if(canInspectVisibleCharacter || canInspectByImmense) {
			std::ostringstream row;
			if(d->character != nullptr && d->character->player.name != nullptr) {
				row << std::left << std::setw(16)
					<< (d->original ? d->original->player.name : d->character->player.name)
					<< ": ";
			}
			else {
				row << "NON DEFINITO  : ";
			}

			row << std::left << std::setw(22) << connected_types[d->connected]
				<< " [" << ((*d->host) ? d->host : "????") << "] "
				<< std::right << std::setw(5) << d->wait << "\n\r";
			buf += row.str();
		}
	}

	page_string(ch->desc, buf.c_str(), false);
}



ACTION_FUNC(do_inventory) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_inventory (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	send_to_char("Stai trasportando:\n\r", ch);
	list_obj_in_heap(ch->carrying, ch);
	bool hasVisibleItems = false;
	for(struct obj_data* item = ch->carrying; item != nullptr; item = item->next_content) {
		if(CAN_SEE_OBJ(ch, item)) {
			hasVisibleItems = true;
			break;
		}
	}
	if(!hasVisibleItems) {
		send_to_char(" Nulla.\n\r", ch);
	}
}


ACTION_FUNC(do_equipment) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_equipment (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	int wornIndex = 0;
	bool found = false;

	send_to_char("Stai usando:\n\r", ch);
	for(int j = 0; j < MAX_WEAR; j++) {
		if(ch->equipment[j] != nullptr) {
			wornIndex++;
			std::ostringstream itemLine;
			itemLine << "[" << std::setw(2) << wornIndex << "] " << eqWhere[j];
			send_to_char(itemLine.str().c_str(), ch);
			if(CAN_SEE_OBJ(ch, ch->equipment[j])) {
				show_obj_to_char(ch->equipment[j], ch, 1);
				found = true;
			}
			else {
				send_to_char("Qualcosa.\n\r", ch);
				found = true;
			}
		}
	}
	if(!found) {
		send_to_char(" Nulla.\n\r", ch);
	}
}


ACTION_FUNC(do_credits) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_credits (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, credits, "do_credits");
}
ACTION_FUNC(do_news) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_news (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, news, "do_news");
}
ACTION_FUNC(do_wiznews) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wiznews (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, wiznews, "do_wiznews");
}
ACTION_FUNC(do_info) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_info (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, info, "do_info");
}
ACTION_FUNC(do_wizlist) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wizlist (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, wizlist, "do_wizlist");
}
ACTION_FUNC(do_prince) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_prince (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, princelist, "do_prince");
}
ACTION_FUNC(do_immortal) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_immortal (act.info.cpp)");
		return;
	}
	ShowStaticPagedText(ch, immlist, "do_immortal");
}

int which_number_object(struct obj_data* obj) {
	if(obj == nullptr || obj->name == nullptr) {
		return 0;
	}

	const char* const name = fname(obj->name);
	int number = 0;
	for(struct obj_data* i = object_list; i != nullptr; i = i->next) {
		if(isname(name, i->name)) {
			number++;
			if(i == obj) {
				return number;
			}
		}
	}
	return 0;
}

char* numbered_object(struct char_data* ch, struct obj_data* obj) {
	static std::array<char, MAX_STRING_LENGTH> buf{};
	std::string value;

	if(obj == nullptr) {
		value = "(null)";
	}
	else if(ch != nullptr && IS_IMMORTAL(ch)) {
		value = std::to_string(which_number_object(obj));
		value += ".";
		value += (obj->name != nullptr) ? fname(obj->name) : "(null)";
	}
	else {
		value = obj->short_description ? obj->short_description : "";
	}
	const std::size_t copyLen = std::min(value.size(), buf.size() - 1);
	std::copy_n(value.data(), copyLen, buf.data());
	buf[copyLen] = '\0';
	return buf.data();
}

int which_number_mobile(struct char_data* ch, struct char_data* mob) {
	(void)ch;
	if(mob == nullptr || mob->player.name == nullptr) {
		return 0;
	}

	const char* const name = fname(mob->player.name);
	int number = 0;
	for(struct char_data* i = character_list; i != nullptr; i = i->next) {
		if(isname(name, i->player.name) && i->in_room != NOWHERE) {
			number++;
			if(i == mob) {
				return number;
			}
		}
	}
	return 0;
}

char* numbered_person(struct char_data* ch, struct char_data* person) {
	static std::array<char, MAX_STRING_LENGTH> buf{};
	std::string value;
	if(person == nullptr) {
		value = "(null)";
	}
	else if(IS_NPC(person) && ch != nullptr && IS_IMMORTAL(ch)) {
		value = std::to_string(which_number_mobile(ch, person));
		value += ".";
		value += person->player.name ? fname(person->player.name) : "(null)";
	}
	else {
		value = PERS(person, ch);
	}
	const std::size_t copyLen = std::min(value.size(), buf.size() - 1);
	std::copy_n(value.data(), copyLen, buf.data());
	buf[copyLen] = '\0';
	return buf.data();
}

namespace {

/* Shared by do_where (chars + objs) and owhere: Nth match vs list-all with [nnn] prefixes. */
bool where_iteration_matches(int number, int& count) {
	return number == 0 || (--count) == 0;
}

void where_append_multiline_prefix(struct string_block* sb, int number, int& count) {
	if(sb == nullptr || number != 0) {
		return;
	}
	std::ostringstream countPrefix;
	countPrefix << "[" << std::setw(3) << ++count << "] ";
	append_to_string_block(sb, countPrefix.str().c_str());
}

} // namespace

void do_where_person(struct char_data* ch, struct char_data* person,
					 struct string_block* sb) {
	if(ch == nullptr || person == nullptr || sb == nullptr) {
		return;
	}
	if(!CAN_SEE(ch, person)) {
		return;
	}
	const char* roomName = "Nowhere";
	if(person->in_room > -1) {
		if(struct room_data* const rp = real_roomp(person->in_room)) {
			roomName = rp->name;
		}
	}
	std::ostringstream row;
	row << std::left << std::setw(40) << PERS(person, ch) << "- "
		<< roomName << " ";
	if(GetMaxLevel(ch) >= DIO) {
		row << "[" << person->in_room << "]";
	}
	row << "\n\r";
	append_to_string_block(sb, row.str().c_str());
}

void do_where_object(struct char_data* ch, struct obj_data* obj, bool recurse,
					 struct string_block* sb) {
	if(ch == nullptr || obj == nullptr || sb == nullptr) {
		return;
	}
	const char* rawShortDesc = obj->short_description ? obj->short_description : "(null)";
	const int diff = static_cast<int>(std::strlen(rawShortDesc))
		- static_cast<int>(std::strlen(ParseAnsiColors(0, rawShortDesc)));
	std::ostringstream descBuilder;
	descBuilder << std::left << std::setw(diff + 55) << rawShortDesc;
	const std::string shortDesc = descBuilder.str();

	std::string tail;
	if(obj->in_room != NOWHERE) {
		tail = "- ";
		if(struct room_data* const rp = real_roomp(obj->in_room)) {
			tail += rp->name;
		}
		else {
			tail += "???";
		}
		tail += " [";
		tail += std::to_string(obj->in_room);
		tail += "]\n\r";
	}
	else if(obj->carried_by != nullptr) {
		tail = "- trasportato da ";
		tail += numbered_person(ch, obj->carried_by);
		tail += "\n\r";
	}
	else if(obj->equipped_by != nullptr) {
		tail = "- usato da ";
		tail += numbered_person(ch, obj->equipped_by);
		tail += "\n\r";
	}
	else if(obj->in_obj != nullptr) {
		tail = "- in ";
		tail += numbered_object(ch, obj->in_obj);
		tail += "\n\r";
	}
	else {
		tail = "- Nemmeno Dio sa dove...\n\r";
	}

	append_to_string_block(sb, (shortDesc + tail).c_str());

	if(!recurse) {
		return;
	}
	if(obj->in_room != NOWHERE) {
		return;
	}
	if(obj->carried_by != nullptr) {
		do_where_person(ch, obj->carried_by, sb);
	}
	else if(obj->equipped_by != nullptr) {
		do_where_person(ch, obj->equipped_by, sb);
	}
	else if(obj->in_obj != nullptr) {
		do_where_object(ch, obj->in_obj, true, sb);
	}
}

void owhere(struct char_data* ch, char* nome)
{
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in owhere (act.info.cpp)");
		return;
	}

	std::array<char, MAX_STRING_LENGTH> name{};
	struct obj_data* k;
	int number = 0;
	int count = 0;
	struct string_block sb;

	only_argument((nome != nullptr) ? nome : "", name.data());
	int N_oggetto = -1;
	if(is_number(name.data()))
	{
		char* parseEnd = nullptr;
		errno = 0;
		const long long parsedObjectNum = std::strtoll(name.data(), &parseEnd, 10);
		if(parseEnd != name.data() && *parseEnd == '\0')
		{
			if(errno == ERANGE || parsedObjectNum > static_cast<long long>(std::numeric_limits<int>::max()))
			{
				N_oggetto = std::numeric_limits<int>::max();
			}
			else if(parsedObjectNum < static_cast<long long>(std::numeric_limits<int>::min()))
			{
				N_oggetto = std::numeric_limits<int>::min();
			}
			else
			{
				N_oggetto = static_cast<int>(parsedObjectNum);
			}
		}
	}

	init_string_block(&sb);

	if(!is_number(name.data())) {
		for(k = object_list; k != nullptr; k = k->next) {
			if(isname(name.data(), k->name) && CAN_SEE_OBJ(ch, k)) {
				if(!where_iteration_matches(number, count)) {
					continue;
				}
				where_append_multiline_prefix(&sb, number, count);
				do_where_object(ch, k, number != 0, &sb);
				if(number != 0) {
					break;
				}
			}
		}
	}

	count++;
	bool found = false;
	struct stringa_valore sb_count;
	if(N_oggetto < 1) {
		for(number = 0; number < top_of_objt; number++) {
			if(isname(name.data(), obj_index[number].name)) {
				sb_count = find_obj(ch, obj_index[number].iVNum, count++);
				found = true;
				append_to_string_block(&sb, sb_count.sb.c_str());
				count = sb_count.conteggio;
			}
		}
		if(number >= top_of_objt) {
			number = -1;
		}
	}

	if((number < 0 || number >= top_of_objt) && !*sb.data) {
		send_to_char("Non trovo niente del genere da nessuna parte.\n\r", ch);
	}
	else {
		if(N_oggetto > 0 && N_oggetto < 99999) {
			sb_count = find_obj(ch, N_oggetto, count++);
			found = true;
			append_to_string_block(&sb, sb_count.sb.c_str());
			count = sb_count.conteggio;
		}

		if(!*sb.data) {
			send_to_char("Non trovo niente del genere da nessuna parte.\n\r", ch);
		}
		else if(!found) {
			append_to_string_block(&sb,
									"Non trovo niente del genere nei personaggi rentati.\n\r");
		}
		else {
			page_string_block(&sb, ch);
		}
	}
	destroy_string_block(&sb);
}

ACTION_FUNC(do_where) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_where (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> name{};
	char*        nameonly;
	struct char_data* i;
	struct obj_data* k;
	struct descriptor_data* d;
	int        number, count;
	struct string_block        sb;
	std::array<char, 10> tipo{};
	const char* copia = (arg != nullptr) ? arg : "";

	only_argument(copia, name.data());

	copia = one_argument(copia, tipo.data());
	if(std::string_view(tipo.data()) == "obj" && IS_DIO(ch)) {
		only_argument(copia, name.data());
		if(is_number(name.data())) {
			mudlog(LOG_PLAYERS, "Looking for object #%s on rented toon", name.data());
		}
		else {
			mudlog(LOG_PLAYERS, "Looking for '%s' in game and on rented toon", name.data());
		}
		owhere(ch, name.data());
		return;
	}

	if(name[0] == '\0') {
		if(GetMaxLevel(ch) < DIO) {
			send_to_char("Cosa stai cercando?\n\r", ch);
		}
		else {
			init_string_block(&sb);
			append_to_string_block(&sb, "Giocatori:\n\r"
								   "----------\n\r");

			for(d = descriptor_list; d != nullptr; d = d->next) {
				if(d->character && (d->connected == CON_PLYNG) &&
						(d->character->in_room != NOWHERE) &&
						CAN_SEE(ch, d->character)) {
					struct room_data* const drp = real_roomp(d->character->in_room);
					if(drp == nullptr) {
						continue;
					}
					std::ostringstream row;
					row << std::left << std::setw(20)
						<< (d->original ? d->original->player.name : d->character->player.name)
						<< " - " << drp->name
						<< " [" << std::setw(3) << d->character->in_room << "]";
					if(d->original) {   /* If switched */
						row << " Nel corpo di " << fname(d->character->player.name);
					}
					row << "\n\r";
					append_to_string_block(&sb, row.str().c_str());
				}
			}
			page_string_block(&sb, ch);
			destroy_string_block(&sb);
		}
		return;
	}

	if(std::isdigit(static_cast<unsigned char>(name[0]))) {
		nameonly = name.data();
		count = number = get_number(&nameonly);
	}
	else {
		count = number = 0;
	}

	init_string_block(&sb);

	for(i = character_list; i != nullptr; i = i->next) {
		if(!isname(name.data(), i->player.name) || !CAN_SEE(ch, i)) {
			continue;
		}

		if(!IS_PC(i) && affected_by_spell(i, STATUS_QUEST) && GetMaxLevel(ch) < IMMORTALE) {
			act("Non si bara! ;)\n\r", FALSE, ch, nullptr, ch, TO_CHAR);
			break;
		}

		struct room_data* const irp = real_roomp(i->in_room);
		struct room_data* const chrp = real_roomp(ch->in_room);
		const bool sameZone = (irp != nullptr && chrp != nullptr && irp->zone == chrp->zone);
		if((i->in_room == NOWHERE) || irp == nullptr
				|| !((GetMaxLevel(ch) >= IMMORTALE) || sameZone)) {
			continue;
		}

		if(!where_iteration_matches(number, count)) {
			/* Mortals stop at the first same-zone name match even when Nth-index has not arrived yet. */
			if(GetMaxLevel(ch) < IMMORTALE) {
				break;
			}
			continue;
		}
		where_append_multiline_prefix(&sb, number, count);
		do_where_person(ch, i, &sb);
		if(number != 0) {
			break;
		}
		if(GetMaxLevel(ch) < IMMORTALE) {
			break;
		}
	}
	/*  count = number;*/

	if(GetMaxLevel(ch) >= DIO) {
		for(k = object_list; k != nullptr; k = k->next) {
			if(!isname(name.data(), k->name) || !CAN_SEE_OBJ(ch, k)) {
				continue;
			}
			if(!where_iteration_matches(number, count)) {
				continue;
			}
			where_append_multiline_prefix(&sb, number, count);
			do_where_object(ch, k, number != 0, &sb);
			if(number != 0) {
				break;
			}
		}
	}

	if(!*sb.data) {
		send_to_char("Non trovo niente del genere.\n\r", ch);
	}
	else {
		page_string_block(&sb, ch);
	}
	destroy_string_block(&sb);
}




ACTION_FUNC(do_levels) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_levels (act.info.cpp)");
		return;
	}
	if(IS_NPC(ch)) {
		send_to_char("Non sei che uno stupido mob.\n\r", ch);
		return;
	}
	if(arg == nullptr) {
		send_to_char("Devi fornire una classe!\n\r", ch);
		return;
	}

	// Skip initial blanks safely (ctype macros expect unsigned char).
	while(*arg != '\0' && isspace(static_cast<unsigned char>(*arg))) {
		++arg;
	}
	if(*arg == '\0') {
		send_to_char("Devi fornire una classe!\n\r", ch);
		return;
	}

	int iClass = -1;
	int RaceMax = 0;
	std::string buf;
	const bool isFemale = (GET_SEX(ch) == SEX_FEMALE);

	switch(*arg) {
	case 'C':
	case 'c':
		iClass = CLERIC_LEVEL_IND;
		break;
	case 'F':
	case 'f':
	case 'W':
	case 'w':
		iClass = WARRIOR_LEVEL_IND;
		break;
	case 'M':
	case 'm':
		iClass = MAGE_LEVEL_IND;
		break;
	case 'T':
	case 't':
		iClass = THIEF_LEVEL_IND;
		break;
	case 'D':
	case 'd':
		iClass = DRUID_LEVEL_IND;
		break;
	case 'K':
	case 'k':
		iClass = MONK_LEVEL_IND;
		break;
	case 'B':
	case 'b':
		iClass = BARBARIAN_LEVEL_IND;
		break;
	case 'S':
	case 's':
		iClass = SORCERER_LEVEL_IND;
		break;

	case 'P':
	case 'p':
		iClass = PALADIN_LEVEL_IND;
		break;

	case 'R':
	case 'r':
		iClass = RANGER_LEVEL_IND;
		break;

	case 'I':
	case 'i':
		iClass = PSI_LEVEL_IND;
		break;

	default:
		{
			std::string unknownClassMsg = "Non riconosco ";
			unknownClassMsg += arg;
			unknownClassMsg += ".\n\r";
			send_to_char(unknownClassMsg.c_str(), ch);
			return;
		}
	}

	RaceMax = RacialMax[GET_RACE(ch)][iClass];
	buf.reserve(static_cast<std::size_t>(std::max(0, RaceMax) * 64 + 16));
	for(int i = 1; i <= RaceMax; ++i) {
		std::ostringstream levelLine;
		levelLine << "[" << std::setw(2) << i << "] "
				  << std::setw(9) << titles[iClass][i].exp
				  << "-" << std::left << std::setw(9) << titles[iClass][i + 1].exp
				  << " : "
				  << (isFemale ? titles[iClass][i].title_f : titles[iClass][i].title_m)
				  << "\n\r";
		buf += levelLine.str();
	}
	buf += "\n\r";

	if(ch->desc == nullptr) {
		return;
	}
	page_string(ch->desc, buf.c_str(), true);

}

namespace {

void send_consider_diff_message(struct char_data* ch, int diff) {
	if(diff <= -10) {
		send_to_char("Troppo facile per crederci.\n\r", ch);
	}
	else if(diff <= -5) {
		send_to_char("Non sara' un problema.\n\r", ch);
	}
	else if(diff <= -3) {
		send_to_char("Piu' che facile.\n\r", ch);
	}
	else if(diff <= -2) {
		send_to_char("Facile.\n\r", ch);
	}
	else if(diff <= -1) {
		send_to_char("Piuttosto facile.\n\r", ch);
	}
	else if(diff == 0) {
		send_to_char("L'incontro perfetto!\n\r", ch);
	}
	else if(diff <= 1) {
		send_to_char("Avrai bisogno di un po' di fortuna!\n\r", ch);
	}
	else if(diff <= 2) {
		send_to_char("Avrai bisogno di molta fortuna!\n\r", ch);
	}
	else if(diff <= 3) {
		send_to_char("Avrai bisogno di molta fortuna ed un buon "
					 "equipaggiamento!\n\r", ch);
	}
	else if(diff <= 5) {
		send_to_char("Non ti sarai un po' montato la testa?\n\r", ch);
	}
	else if(diff <= 10) {
		send_to_char("Sei scemo o cosa?\n\r", ch);
	}
	else if(diff <= 30) {
		send_to_char("Tu SEI matto!\n\r", ch);
	}
	else {
		send_to_char("Perche' non ti butti semplicemente a mare e ci risparmi la "
					 "fatica?\n\r", ch);
	}
}

/* Consider creature-type lines: pick primary skill, accumulate best learn, act to char. */
void consider_apply_cons_skill(struct char_data* ch, struct char_data* victim, bool predicate,
							   int skillId, int learnDivisor, int& primarySkill, int& learnOut,
							   const char* actStr) {
	if(!predicate || ch->skills == nullptr || ch->skills[skillId].learned == 0) {
		return;
	}
	const int div = std::max(1, learnDivisor);
	if(primarySkill == 0) {
		primarySkill = skillId;
	}
	const int rawLearn = ch->skills[skillId].learned / div;
	learnOut = std::max(learnOut, rawLearn);
	act(actStr, FALSE, ch, nullptr, victim, TO_CHAR);
}

} // namespace

ACTION_FUNC(do_consider) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_consider (act.info.cpp)");
		return;
	}
	struct char_data* victim;
	std::array<char, 256> name{};
	int diff;

	if(arg != nullptr) {
		only_argument(arg, name.data());
	}

	if(!(victim = get_char_room_vis(ch, name.data()))) {
		send_to_char("Chi stai considerando di uccidere?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Non dovrebbe essere difficile.\n\r", ch);
		return;
	}

	if(!IS_NPC(victim)) {
		send_to_char("Non ci pensare nemmeno!!\n\r", ch);
		act("$n sta considerando di ucciderti", FALSE, ch, nullptr, victim, TO_VICT);
		return;
	}

	act("$n guarda a $N", FALSE, ch, nullptr, victim, TO_NOTVICT);
	act("$n ti guarda", FALSE, ch, nullptr, victim, TO_VICT);


	const int chMaxLevel = GetMaxLevel(ch);
	const int victimMaxLevel = GetMaxLevel(victim);
	if(chMaxLevel >= IMMORTALE && chMaxLevel < IMMENSO) {
		act("Che bisogno hai di considerare $N?", FALSE, ch, nullptr, victim, TO_CHAR);
		return;
	}

	diff =  victimMaxLevel + HowManyClasses(victim) -
			chMaxLevel - HowManyClasses(ch);

	diff += MobLevBonus(victim);

	send_consider_diff_message(ch, diff);

	if(ch->skills != nullptr) {
		int skill = 0;
		int learn = 0;
		int num, num2;
		float fnum;

		consider_apply_cons_skill(ch, victim, IsAnimal(victim), SKILL_CONS_ANIMAL, 1, skill, learn,
								  "$N sembra un animale.");
		consider_apply_cons_skill(ch, victim, IsVeggie(victim), SKILL_CONS_VEGGIE, 1, skill, learn,
								  "$N sembra un vegetale ambulante.");
		consider_apply_cons_skill(ch, victim, IsDiabolic(victim), SKILL_CONS_DEMON, 1, skill, learn,
								  "$N sembra un demone!");
		consider_apply_cons_skill(ch, victim, IsReptile(victim), SKILL_CONS_REPTILE, 1, skill, learn,
								  "$N sembra un rettile.");
		consider_apply_cons_skill(ch, victim, IsUndead(victim), SKILL_CONS_UNDEAD, 1, skill, learn,
								  "$N sembra un non morto.");
		consider_apply_cons_skill(ch, victim, IsGiantish(victim), SKILL_CONS_GIANT, 1, skill, learn,
								  "$N sembra una creatura gigantesca.");
		consider_apply_cons_skill(ch, victim, IsPerson(victim), SKILL_CONS_PEOPLE, 1, skill, learn,
								  "$N sembra umano o mezzo umano.");
		consider_apply_cons_skill(ch, victim, IsOther(victim), SKILL_CONS_OTHER, 2, skill, learn,
								  "$N sembra essere un mostro che conosci.");

		if(learn > 95) {
			learn = 95;
		}

		if(learn == 0) {
			return;
		}

		WAIT_STATE(ch, PULSE_VIOLENCE*2);

#if 1

		num = static_cast<int>(GetApprox(GET_MAX_HIT(victim), learn));
		num2 = static_cast<int>(GET_MAX_HIT(ch));
		if(!num2) {
			num2=1;
		}
		fnum = static_cast<int>(num) / static_cast<int>(num2);
		{
			std::string msg = "Il numero massimo di punti ferita e' ";
			msg += DescRatio(fnum);
			msg += ".\n\r";
			send_to_char(msg.c_str(), ch);
		}

		num = 105 + static_cast<int>(GetApprox(GET_AC(victim), learn));
		num2 = 105 + static_cast<int>(GET_AC(ch));
//    if (!num2) num2=1;
		fnum = static_cast<int>(num) - static_cast<int>(num2);

		{
			std::string msg = "La classe di armatura e' ";
			msg += DescArmorf(fnum);
			msg += ".\n\r";
			send_to_char(msg.c_str(), ch);
		}



		if(learn > 60) {
			std::string msg = "Ha ";
			msg += DescAttacks(static_cast<int>(GetApprox(static_cast<int>(victim->mult_att), learn)));
			msg += " attacchi per turno.\n\r";
			send_to_char(msg.c_str(), ch);
		}

		if(learn > 70) {

			num = static_cast<int>(GetApprox(static_cast<int>(victim->specials.damnodice), learn));
			num2 = static_cast<int>(GetApprox(static_cast<int>(victim->specials.damsizedice), learn));
			if(!num2) {
				num2=1;
			}
			fnum = static_cast<int>(num) * (num2 / 2.0);
			{
				std::string msg = "Il danno per attacco e' ";
				msg += DescDamage(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}
		}

		if(learn > 80) {

			num = static_cast<int>(GetApprox(GET_HITROLL(victim), learn));
			num2 = static_cast<int>(21) - CalcThaco(ch, nullptr);
			if(!num2) {
				num2=1;
			}
			if(num2 > 0) {
				fnum = static_cast<int>(num) / static_cast<int>(num2);
			}
			else {
				fnum = 2.0;
			}

			{
				std::string msg = "La sua probabilita' di colpirti e' ";
				msg += DescRatioF(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}


			num =   GetApprox(GET_DAMROLL(victim), learn);
			num2 =  GET_DAMROLL(ch);
			if(!num2) {
				num2=1;
			}
			fnum = static_cast<int>(num) / static_cast<int>(num2);

			{
				std::string msg = "Il bonus al danno e' ";
				msg += DescRatio(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}

		}
#endif


	}

}

ACTION_FUNC(do_spells) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_spells (act.info.cpp)");
		return;
	}
	if(IS_NPC(ch)) {
		send_to_char("Non sei che uno stupido mob.\n\r", ch);
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	struct string_block sb;
	const int chMaxLevel = GetMaxLevel(ch);

	init_string_block(&sb);
	{
		std::ostringstream header;
		header << "[#  ] " << std::left << std::setw(30) << "SPELL/SKILL"
			   << "  MANA, Cl, Mu, Dr, Sc, Pa, Ra, Ps\n\r";
		append_to_string_block(&sb, header.str().c_str());
	}

	for(int i = 1, spl = 0; i <= MAX_EXIST_SPELL; ++i, ++spl) {
		if(chMaxLevel > IMMORTALE
				|| (spell_info[i].min_level_cleric < ABS_MAX_LVL
					&& spell_info[i].min_level_cleric > 0)) {  // SALVO controllo spell_info
			if(spells[spl] == nullptr) {
				mudlog(LOG_ERROR, "!spells[spl] on %d, do_spells in act.info.cpp", i);
			}
			else {
				std::ostringstream line;
				/* byte/ubyte are char types: stream as int or ostream prints ASCII glyphs. */
				line << "[" << std::setw(3) << i << "] " << std::left << std::setw(30) << spells[spl]
					 << std::right << "  <" << std::setw(3)
					 << static_cast<int>(spell_info[i].min_usesmana) << "> "
					 << std::setw(2) << static_cast<int>(spell_info[i].min_level_cleric) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_magic) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_druid) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_sorcerer) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_paladin) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_ranger) << " "
					 << std::setw(3) << static_cast<int>(spell_info[i].min_level_psi) << "\n\r";
				append_to_string_block(&sb, line.str().c_str());
			}
		}
	}
	append_to_string_block(&sb, "\n\r");
	page_string(ch->desc, sb.data, true);
	destroy_string_block(&sb);
}

double GetLagIndex() {
	const std::size_t n = sizeof(aTimeCheck) / sizeof(aTimeCheck[0]);
	if(n == 0) {
		return 0.0;
	}
	std::uint64_t sum = 0;
	for(std::size_t i = 0; i < n; ++i) {
		sum += aTimeCheck[i];
	}
	return static_cast<double>(sum) / static_cast<double>(n) / 1000000.0;
}

ACTION_FUNC(do_world) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_world (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	const auto worldCharLine = [ch](const std::string& msg) {
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	};

	const std::time_t ot_tt = static_cast<std::time_t>(Uptime);
	std::tm* const otm = std::localtime(&ot_tt);
	if(otm == nullptr) {
		mudlog(LOG_SYSERR, "localtime(Uptime) failed in do_world (act.info.cpp)");
		return;
	}
	char* const otmstr = std::asctime(otm);
	if(otmstr != nullptr) {
		*(otmstr + std::strlen(otmstr) - 1) = '\0';
	}

	{
		std::ostringstream o;
		o << "$c0005Base Source: $c0014AlarMUD\n$c0005"
			 "Versione $c0015" << version() << "\n$c0005Commit: $c0015" << release() << "$c0005.";
		worldCharLine(o.str());
	}
	{
		std::ostringstream o;
		o << "$c0005Compilazione del : $c0014" << compilazione() << ".";
		worldCharLine(o.str());
	}
	{
		std::ostringstream o;
		o << "$c0005Orario di partenza: $c0015" << (otmstr != nullptr ? otmstr : "?") << " $c0005";
		worldCharLine(o.str());
	}

	const std::time_t ct_tt = std::time(nullptr);
	std::tm* const ctm = std::localtime(&ct_tt);
	if(ctm == nullptr) {
		mudlog(LOG_SYSERR, "localtime(time(nullptr)) failed in do_world (act.info.cpp)");
		return;
	}
	char* const tmstr = std::asctime(ctm);
	if(tmstr != nullptr) {
		*(tmstr + std::strlen(tmstr) - 1) = '\0';
	}
	{
		std::ostringstream o;
		o << "$c0005Orario attuale    : $c0015" << (tmstr != nullptr ? tmstr : "?") << " $c0005";
		worldCharLine(o.str());
	}

	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Indice di attesa desiderato: $c0015" << (static_cast<double>(OPT_USEC) / 1000000.0)
		  << " $c0005secs";
		worldCharLine(o.str());
	}
	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Indice di attesa attuale   : $c0015" << GetLagIndex() << " $c0005sec";
		worldCharLine(o.str());
	}

	const int chMaxLevel = GetMaxLevel(ch);
	if(chMaxLevel >= IMMORTALE) {
		std::array<char, 256> tbuf{};
		sprintbit((unsigned long)SystemFlags, system_flag_types, tbuf.data());
		{
			std::ostringstream o;
			o << "$c0005Flags di sistema:[$c0015" << tbuf.data() << "$c0005]\n\r";
			worldCharLine(o.str());
		}
		{
			std::ostringstream o;
			o << "$c0005Connessioni dalla partenza:$c0015" << HowManyConnection(0) << "\n\r";
			worldCharLine(o.str());
		}
	}

#if HASH
	{
		std::ostringstream o;
		o << "$c0005Numero di stanze nel mondo          : $c0015" << room_db.klistlen;
		worldCharLine(o.str());
	}
#else
	{
		std::ostringstream o;
		o << "$c0005Numero di stanze nel mondo          : $c0015" << room_count;
		worldCharLine(o.str());
	}
#endif
	{
		std::ostringstream o;
		o << "$c0005Numero di zone nel mondo            : $c0015" << (top_of_zone_table + 1);
		worldCharLine(o.str());
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di personaggi attivi         : $c0015" << top_of_p_table;
		worldCharLine(o.str());
	}

	// Result of an aggregate query contains only one element so let's
	// use the query_value() shortcut.
	//
	auto c = Sql::getOne<userCount>();
	{
		std::ostringstream o;
		o << "$c0005Numero di giocatori registrati      : $c0015" << (c ? c->count : static_cast<std::size_t>(0));
		worldCharLine(o.str());
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di tipi di creature nel mondo: $c0015" << top_of_mobt;
		worldCharLine(o.str());
	}
	{
		std::ostringstream o;
		o << "$c0005Numero di tipi di oggetti nel mondo  : $c0015" << top_of_objt;
		worldCharLine(o.str());
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di creature nel gioco         : $c0015" << mob_count;
		worldCharLine(o.str());
	}
	{
		std::ostringstream o;
		o << "$c0005Numero di oggetti nel gioco          : $c0015" << obj_count;
		worldCharLine(o.str());
	}
	/**** SALVO controllo lag refresh zone init */
	if(IS_IMMORTAL(ch)) {
		int zonesInit = 0;
		for(int i = 0; i <= top_of_zone_table; ++i) {
			if(zone_table[i].start != 0) {
				++zonesInit;
			}
		}
		{
			std::ostringstream o;
			o << "$c0005Totale zone init nel mondo           : $c0015" << zonesInit << " su $c0015" << (top_of_zone_table + 1);
			worldCharLine(o.str());
		}
		{
			std::ostringstream o;
			o << "$c0005Indice attesa refresh zone init      : $c0015" << attrefzone << " $c0005sec";
			worldCharLine(o.str());
		}
	}
	/****/
	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Valore medio dell'eq in gioco        : $c0015" << AverageEqIndex(-1);
		worldCharLine(o.str());
	}

}

ACTION_FUNC(do_attribute) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_attribute (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	struct affected_type* aff;
	struct time_info_data my_age{};
	const int chMaxLevel = GetMaxLevel(ch);
	const bool isFemale = (GET_SEX(ch) == SEX_FEMALE);

	age3(ch, &my_age);

	{
		std::string msg = "$c0005Tu hai $c0014";
		msg += std::to_string(static_cast<int>(my_age.ayear));
		msg += "$c0005 (ma ne dimostri $c0014";
		msg += std::to_string(static_cast<int>(my_age.year));
		msg += "$c0005) anni e $c0014";
		msg += std::to_string(static_cast<int>(my_age.month));
		msg += "$c0005 mesi, sei ";
		msg += isFemale ? "alta " : "alto ";
		msg += "$c0014";
		msg += std::to_string(static_cast<int>(ch->player.height));
		msg += "$c0005 cm, e pesi $c0014";
		msg += std::to_string(static_cast<int>((ch->player.weight * 4536) / 10000));
		msg += "$c0005 chili.";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}

	{
		std::string msg = "$c0005Stai trasportando $c0014";
		msg += std::to_string(static_cast<int>((IS_CARRYING_W(ch) * 4536) / 1000));
		msg += "$c0005 etti di equipaggiamento su $c0014";
		msg += std::to_string(static_cast<int>((CAN_CARRY_W(ch) * 4536) / 1000));
		msg += ".";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}
	{
		std::string msg = "$c0005Stai trasportando $c0014";
		msg += std::to_string(static_cast<int>(IS_CARRYING_N(ch)));
		msg += "$c0005 oggetti su $c0014";
		msg += std::to_string(static_cast<int>(CAN_CARRY_N(ch)));
		msg += ".";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}

	{
		std::string msg = "$c0005Tu sei$c0014 ";
		msg += ArmorDesc(ch->points.armor);
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}
	if(chMaxLevel >= MAESTRO_DEL_CREATO) {
		std::string msg = "$c0005Armor class: $c0014";
		msg += std::to_string(static_cast<int>(ch->points.armor));
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}
	//GGPATCH, inserita valutazione spellfail
	int iSpellfail = 0;
	if(IS_CASTER(ch)) {
		iSpellfail=ch->specials.spellfail;
		if(chMaxLevel >= MAESTRO_DEL_CREATO) {
			std::string msg = "$c0005Spellfail : $c0014 ";
			msg += std::to_string(iSpellfail);
			act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
		}
		iSpellfail = ch->specials.spellfail-
					 GET_LEVEL(ch, BestMagicClass(ch))+
					 25+
					 (HowManyClasses(ch)-1)*5;
		iSpellfail+=((
						 (EqWBits(ch,ITEM_ANTI_MAGE) && HasClass(ch,CLASS_MAGIC_USER)) ||
						 (EqWBits(ch,ITEM_ANTI_CLERIC) && HasClass(ch,CLASS_CLERIC)) ||
						 (EqWBits(ch,ITEM_ANTI_DRUID) && HasClass(ch,CLASS_DRUID)) ||
						 (EqWBits(ch,ITEM_ANTI_PALADIN) && HasClass(ch,CLASS_PALADIN)) ||
						 (EqWBits(ch,ITEM_ANTI_RANGER) && HasClass(ch,CLASS_RANGER)) ||
						 (EqWBits(ch,ITEM_ANTI_PSI) && HasClass(ch,CLASS_PSI)))?20:0);
		{
			std::string msg = "$c0005La tua capacita' di lanciare incantesimi e'$c0014 ";
			msg += SpellfailDesc(IS_IMMORTAL(ch)?0:iSpellfail);
			act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
		}
	}
	if(chMaxLevel >= CHUMP) {
		std::string msg = "$c0005Tu hai $c0014";
		msg += std::to_string(static_cast<int>(GET_STR(ch)));
		msg += "$c0005/$c0015";
		msg += std::to_string(static_cast<int>(GET_ADD(ch)));
		msg += " $c0005STR, $c0014";
		msg += std::to_string(static_cast<int>(GET_INT(ch)));
		msg += " $c0005INT, $c0014";
		msg += std::to_string(static_cast<int>(GET_WIS(ch)));
		msg += " $c0005WIS, $c0014";
		msg += std::to_string(static_cast<int>(GET_DEX(ch)));
		msg += " $c0005DEX, $c0014";
		msg += std::to_string(static_cast<int>(GET_CON(ch)));
		msg += " $c0005CON, $c0014";
		msg += std::to_string(static_cast<int>(GET_CHR(ch)));
		msg += " $c0005CHR";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}

	{
		std::string msg = "$c0005I tuoi hit e damage bonus sono $c0014";
		msg += HitRollDesc(GET_HITROLL(ch));
		msg += "$c0005 e $c0014";
		msg += DamRollDesc(GET_DAMROLL(ch));
		msg += "$c0005 rispettivamente.";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}
	{
		std::string msg = "$c0005Il tuo equipaggiamento e' $c0014";
		msg += EqDesc(GetCharBonusIndex(ch));
		msg += "$c0005";
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}
	if(chMaxLevel >= MAESTRO_DEL_CREATO) {
		std::string msg = "$c0005Hit:$c0014+";
		msg += std::to_string(static_cast<int>(GET_HITROLL(ch)));
		msg += "$c0005 Dam:$c0014+";
		msg += std::to_string(static_cast<int>(GET_DAMROLL(ch)));
		act(msg.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
	}

	/*
	 **   by popular demand -- affected stuff
	 */
	if(ch->affected != nullptr) {
		bool bFirstTime = true;
		bool shown[MAX_EXIST_SPELL + 1] {};
		for(aff = ch->affected; aff != nullptr; aff = aff->next) {
			if(aff->type <= MAX_EXIST_SPELL && aff->type > 0 && !shown[aff->type]) {
				shown[aff->type] = true;

				switch(aff->type) {
				case SKILL_SNEAK:
				case SPELL_POISON:
				case SPELL_CURSE:
				case SPELL_PRAYER:
				case SPELL_FEEBLEMIND:
				case SKILL_SWIM:
				case SKILL_SPY:
				case SKILL_FIRST_AID:
				case SKILL_LAY_ON_HANDS:
				case SKILL_MEDITATE:
					break;

				default:
					if(bFirstTime) {
						act("\n\r$c0005Spells attivi:\n\r"
							"--------------", false, ch, nullptr, nullptr, TO_CHAR);
						bFirstTime = false;
					}
					if(*spells[aff->type-1] || *spells[aff->type-1]=='!') {

						std::string spellLine = "$c0005Spell : '$c0014";
						spellLine += spells[aff->type - 1];
						spellLine += "$c0005' - $c0014";
						spellLine += std::to_string(static_cast<int>(aff->duration));
						spellLine += "$c0005";
						act(spellLine.c_str(), false, ch, nullptr, nullptr, TO_CHAR);
					}

					break;
				}
			}
		}
	}
}

ACTION_FUNC(do_value) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_value (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	if(arg == nullptr) {
		send_to_char("Di chi o di cosa stai parlando?\n\r", ch);
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> name{};
	struct obj_data* obj = nullptr;
	struct char_data* vict = nullptr;

	/* Spell Names */


	/* For Objects */


	if(!HasClass(ch, CLASS_THIEF | CLASS_RANGER)) {
		send_to_char("Scordatelo!!", ch);
		return;
	}

	arg = one_argument(arg, name.data());

	if((obj = get_obj_in_list_vis(ch, name.data(), ch->carrying)) == nullptr) {
		if((vict = get_char_room_vis(ch, name.data())) == nullptr) {
			send_to_char("Di chi o di cosa stai parlando?\n\r", ch);
			return;
		}
		else {
			only_argument(arg, name.data());
			if((obj = get_obj_in_list_vis(ch, name.data(), vict->carrying)) == nullptr) {
				act("Non vedi nulla del genere addosso a $N.", false, ch, nullptr, vict, TO_CHAR);
				act("$n ti sta esaminando.", false, ch, nullptr, vict, TO_VICT);
				act("$n esamina $N.", false, ch, nullptr, vict, TO_NOTVICT);
				return;
			}
		}
	}

	WAIT_STATE(ch, PULSE_VIOLENCE * 2);

	if(!SpyCheck(ch)) {
		/* failed spying check */
		if(obj != nullptr && vict != nullptr) {
			act("$n ti guarda ed i suoi occhi indugiano su $p",
				false, ch, obj, vict, TO_VICT);
			act("$n studia $N.", false, ch, nullptr, vict, TO_ROOM);
		}
		else if(obj != nullptr) {
			act("$n studia $p intensamente.", false, ch, obj, nullptr, TO_ROOM);
		}
		else {
			return;
		}
	}


	{
		std::string header = "Oggetto: ";
		header += (obj->short_description ? obj->short_description : "(null)");
		header += ".  Tipo: ";
		std::array<char, 512> typeBuf{};
		sprinttype(GET_ITEM_TYPE(obj), item_types, typeBuf.data());
		header += typeBuf.data();
		header += "\n\r";
		send_to_char(header.c_str(), ch);
	}

	if(ch->skills == nullptr) {
		return;
	}


	if(number(1, 101) < ch->skills[SKILL_EVALUATE].learned / 3) {
		if(obj->obj_flags.bitvector) {
			std::string affLine = fname(obj->name);
			affLine += " ti da' le seguenti abilita': ";
			std::array<char, MAX_STRING_LENGTH> affBuf{};
			sprintbit((unsigned long)obj->obj_flags.bitvector, affected_bits, affBuf.data());
			affLine += affBuf.data();
			affLine += "\n\r";
			send_to_char(affLine.c_str(), ch);
		}
	}

	if(number(1, 101) < ch->skills[SKILL_EVALUATE].learned / 2) {
		send_to_char("L'oggetto e': ", ch);
		std::array<char, MAX_STRING_LENGTH> extraBuf{};
		sprintbit2((unsigned long)obj->obj_flags.extra_flags, extra_bits,
				   (unsigned long)obj->obj_flags.extra_flags2, extra_bits2, extraBuf.data());
		std::string extraLine(extraBuf.data());
		extraLine += "\n\r";
		send_to_char(extraLine.c_str(), ch);
	}

	{
		std::ostringstream statsLine;
		statsLine << "Peso: " << obj->obj_flags.weight
				  << ", Valore: " << GetApprox(obj->obj_flags.cost, ch->skills[SKILL_EVALUATE].learned - 10)
				  << ", Rent cost: " << GetApprox(obj->obj_flags.cost_per_day,
					  ch->skills[SKILL_EVALUATE].learned - 10)
				  << "  " << (obj->obj_flags.cost >= LIM_ITEM_COST_MIN ? "[RARO]" : " ") << "\n\r";
		send_to_char(statsLine.str().c_str(), ch);
	}

	if(ITEM_TYPE(obj) == ITEM_WEAPON) {
		std::ostringstream weaponLine;
		weaponLine << "Valore di danno: '"
				   << GetApprox(obj->obj_flags.value[1], ch->skills[SKILL_EVALUATE].learned - 10)
				   << "D"
				   << GetApprox(obj->obj_flags.value[2], ch->skills[SKILL_EVALUATE].learned - 10)
				   << "'\n\r";
		send_to_char(weaponLine.str().c_str(), ch);
	}
	else if(ITEM_TYPE(obj) == ITEM_ARMOR) {

		std::ostringstream armorLine;
		armorLine << "L'AC e' "
				  << GetApprox(obj->obj_flags.value[0], ch->skills[SKILL_EVALUATE].learned - 10)
				  << "\n\r";
		send_to_char(armorLine.str().c_str(), ch);
	}
}

const char* AlignDesc(int a) {
	if(a <= -990) {
		return "Perfid$b come un Demone!";
	}
	if(a <= -900) {
		return "Veramente malvagi$b";
	}
	if(a <= -500) {
		return "Malvagi$b";
	}
	if(a <= -351) {
		return "Cattiv$b";
	}
	if(a <= -100) {
		return "Antipatic$b";
	}
	if(a <= 100) {
		return "Bilanciat$b";
	}
	if(a <= 350) {
		return "Simpatic$b";
	}
	if(a <= 500) {
		return "Brav$b ragazz$b";
	}
	if(a <= 900) {
		return "Buon$b";
	}
	if(a <= 990) {
		return "Troppo buon$b";
	}
	return "Angelic$b";
}

const char* EqDesc(float a) {
	if(a >= 1400) {
		return "meglio di quanto credevo possibile!!";
	}
	if(a >= 1200) {
		return "qualcosa di mai visto!";
	}
	if(a >= 900) {
		return "il massimo!";
	}
	if(a >= 600) {
		return "davvero notevole";
	}
	if(a >= 400) {
		return "piuttosto buono";
	}
	if(a >= 200) {
		return "del tutto ordinario";
	}
	if(a >= 100) {
		return "scarso";
	}
	if(a >= 10) {
		return "il minimo per non essere nudo";
	}
	return "ai limiti dell'ascesi!";
}

const char* SpellfailDesc(int a) {
	if(a >= 100) {
		return "terribile";
	}
	if(a >= 90) {
		return "scarsa";
	}
	if(a >= 75) {
		return "nella norma";
	}
	if(a >= 55) {
		return "buona";
	}
	if(a >= 40) {
		return "eccellente";
	}
	return "veramente eccezionale!";
}

const char* ArmorDesc(int a) {
	if(a >= 90) {
		return "come se fossi nud$b";
	}
	if(a >= 50) {
		return "difes$b";
	}
	if(a >= 30) {
		return "ben difes$b";
	}
	if(a >= 10) {
		return "protett$b";
	}
	if(a >= -10) {
		return "ben protett$b";
	}
	if(a >= -30) {
		return "corazzat$b";
	}
	if(a >= -50) {
		return "ben corazzat$b";
	}
	if(a >= -90) {
		return "molto ben corazzat$b";
	}
	return "corazzat$b come un Demone";
}

const char* HitRollDesc(int a) {
	if(a < -5) {
		return "pessimo";
	}
	if(a < -1) {
		return "basso";
	}
	if(a <= 1) {
		return "nella media";
	}
	if(a < 3) {
		return "non male";
	}
	if(a < 8) {
		return "buono";
	}
	if(a < 12) {
		return "molto buono";
	}
	return "eccellente";
}

const char* DamRollDesc(int a) {
	return HitRollDesc(a);
}

const char* DescRatio(float f) { /* theirs / yours */
	if(f > 1.0f) {
		return "maggiore del tuo";
	}
	if(f > 0.75f) {
		return "piu' di una volta e mezzo il tuo";
	}
	if(f > 0.6f) {
		return "almeno un terzo piu' grande del tuo";
	}
	if(f > 0.4f) {
		return "piu' o meno come il tuo";
	}
	if(f > 0.3f) {
		return "un po' piu' basso del tuo";
	}
	if(f > 0.1f) {
		return "molto piu' basso del tuo";
	}
	return "estremamente piu' basso del tuo";
}

const char* DescArmorf(float f) { /* theirs / yours */
	if(f > 110.0f) {
		return "estremamente peggiore della tua";
	}
	if(f > 70.0f) {
		return "molto peggiore della tua";
	}
	if(f > 20.0f) {
		return "peggiore della tua";
	}
	if(f > -20.0f) {
		return "piu' o meno come la tua";
	}
	if(f > -70.0f) {
		return "migliore della tua";
	}
	if(f > -110.0f) {
		return "molto migliore della tua";
	}
	return "estremamente migliore della tua";
}

const char* DescRatioF(float f) { /* theirs / yours */
	if(f > 1.0f) {
		return "maggiore della tua";
	}
	if(f > 0.75f) {
		return "piu' di una volta e mezzo la tua";
	}
	if(f > 0.6f) {
		return "almeno un terzo piu' grande della tua";
	}
	if(f > 0.4f) {
		return "piu' o meno come la tua";
	}
	if(f > 0.3f) {
		return "un po' piu' bassa della tua";
	}
	if(f > 0.1f) {
		return "molto piu' bassa della tua";
	}
	return "estremamente piu' bassa della tua";
}

const char* DescDamage(float dam) {
	if(dam < 1.0f) {
		return "minimo";
	}
	if(dam <= 2.0f) {
		return "basso";
	}
	if(dam <= 4.0f) {
		return "avvertibile";
	}
	if(dam <= 10.0f) {
		return "alto";
	}
	if(dam <= 15.0f) {
		return "molto alto";
	}
	if(dam <= 25.0f) {
		return "doloroso";
	}
	if(dam <= 35.0f) {
		return "estremamente doloroso";
	}
	return "mortale";
}

const char* DescAttacks(float a) {
	if(a < 1.0f) {
		return "pochi";
	}
	if(a < 2.0f) {
		return "non piu' di due";
	}
	if(a < 3.0f) {
		return "un po' di";
	}
	if(a < 5.0f) {
		return "un bel po' di";
	}
	if(a < 9.0f) {
		return "molti";
	}
	return "un'enormita' di";
}


ACTION_FUNC(do_display) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_display (act.info.cpp)");
		return;
	}
	if(IS_NPC(ch)) {
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	int i = -1;
	if(arg != nullptr && *arg != '\0') {
		char* parseEnd = nullptr;
		errno = 0;
		const long parsedMode = std::strtol(arg, &parseEnd, 10);
		if(parseEnd != arg && *parseEnd == '\0' && errno != ERANGE &&
				parsedMode >= std::numeric_limits<int>::min() &&
				parsedMode <= std::numeric_limits<int>::max()) {
			i = static_cast<int>(parsedMode);
		}
	}

	switch(i) {
	case 0:
		if(ch->term == 0) {
			send_to_char("Modo di visualizzazione invariato.\n\r", ch);
			return;
		}
		ch->term = 0;
		ScreenOff(ch);
		send_to_char("Modo VT100 disattivato.\n\r", ch);
		return;

	case 1:
		if(ch->term == VT100) {
			send_to_char("Modo di visualizzazione invariato.\n\r", ch);
			return;
		}
		ch->term = VT100;
		InitScreen(ch);
		send_to_char("Modo VT100 attivo.\n\r", ch);
		return;

	default:
		if(ch->term == VT100) {
			send_to_char("Modo VT100 attivo.\n\r", ch);
			return;
		}
		send_to_char("Modo VT100 disattivo.\n\r", ch);
		return;
	}
}

void ScreenOff(struct char_data* ch) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in ScreenOff (act.info.cpp)");
		return;
	}
	/* Ripristina regione di scroll a tutto schermo (InitScreen usa size-5 per la barra). */
	std::array<char, 255> buf{};
	const int lastLine = std::max(0, ch->size - 1);
	std::snprintf(buf.data(), buf.size(), VT_MARGSET, 0, lastLine);
	send_to_char(buf.data(), ch);
	send_to_char(VT_HOMECLR, ch);
}

ACTION_FUNC(do_resize) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_resize (act.info.cpp)");
		return;
	}
	if(IS_NPC(ch)) {
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	if(arg == nullptr || *arg == '\0') {
		send_to_char("Specifica l'altezza in righe (es. resize 25).\n\r", ch);
		return;
	}

	char* parseEnd = nullptr;
	errno = 0;
	const long parsedHeight = std::strtol(arg, &parseEnd, 10);
	if(parseEnd == arg || *parseEnd != '\0' || errno == ERANGE ||
			parsedHeight < std::numeric_limits<int>::min() ||
			parsedHeight > std::numeric_limits<int>::max()) {
		send_to_char("Inserisci un numero valido.\n\r", ch);
		return;
	}
	const int i = static_cast<int>(parsedHeight);

	if(i < 7) {
		send_to_char("L'altezza dello schermo deve essere maggiore di 7.\n\r", ch);
		return;
	}

	if(i > 50) {
		send_to_char("L'altezza dello schermo deve essere minore di 50.\n\r", ch);
		return;
	}

	ch->size = i;

	if(ch->term == VT100) {
		ScreenOff(ch);
		InitScreen(ch);
	}

	send_to_char("Ok.\n\r", ch);
}

int MobLevBonus(struct char_data* ch) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in MobLevBonus (act.info.cpp)");
		return 0;
	}
	if(!IS_MOB(ch)) {
		return 0;
	}

	const uintptr_t mobFunc = reinterpret_cast<uintptr_t>(mob_index[ch->nr].func);
	int t = 0;

	if(mobFunc == reinterpret_cast<uintptr_t>(magic_user) ||
			IS_SET(ch->specials.act, ACT_MAGIC_USER)) {
		t += 5;
	}

	if(mobFunc == reinterpret_cast<uintptr_t>(BreathWeapon)) {
		t += 7;
	}
	if(mobFunc == reinterpret_cast<uintptr_t>(fighter) ||
			IS_SET(ch->specials.act, ACT_WARRIOR)) {
		t += 3;
	}

	if(mobFunc == reinterpret_cast<uintptr_t>(snake)) {
		t += 3;
	}

	t += static_cast<int>((ch->mult_att - 1) * 3);

	if(GET_HIT(ch) > GetMaxLevel(ch) * 8) {
		t += 1;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch) * 12) {
		t += 2;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch) * 16) {
		t += 3;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch) * 20) {
		t += 4;
	}

	return t;
}

namespace {

enum class ShowSkillSpellColumn {
	Magic,
	Cleric,
	Druid,
	Sorcerer,
	Paladin,
	Ranger,
	Psi
};

ubyte column_level(const spell_info_type& si, ShowSkillSpellColumn col) {
	switch(col) {
	case ShowSkillSpellColumn::Magic:
		return si.min_level_magic;
	case ShowSkillSpellColumn::Cleric:
		return si.min_level_cleric;
	case ShowSkillSpellColumn::Druid:
		return si.min_level_druid;
	case ShowSkillSpellColumn::Sorcerer:
		return si.min_level_sorcerer;
	case ShowSkillSpellColumn::Paladin:
		return si.min_level_paladin;
	case ShowSkillSpellColumn::Ranger:
		return si.min_level_ranger;
	case ShowSkillSpellColumn::Psi:
		return si.min_level_psi;
	}
	return 0;
}

bool append_show_skill_line(std::string& buffer, const std::string& line) {
	if(buffer.size() + line.size() + 1 > static_cast<std::size_t>((MAX_STRING_LENGTH * 2) - 2)) {
		return false;
	}
	buffer += line;
	buffer += "\r";
	return true;
}

void show_skill_append_cylinder_sheet(struct char_data* ch, ShowSkillSpellColumn col, std::string& buffer) {
	for(int max = 0; max < IMMORTALE; ++max) {
		for(int i = 0; i < MAX_SPL_LIST && spells[i] != nullptr && *spells[i] != '\n'; ++i) {
			if(spell_info[i + 1].spell_pointer == nullptr) {
				const int colLvl = static_cast<int>(column_level(spell_info[i + 1], col));
				if(colLvl <= 0 || colLvl >= IMMORTALE) {
					continue;
				}
			}
			if(static_cast<int>(column_level(spell_info[i + 1], col)) != max) {
				continue;
			}
			std::string line = "[";
			line += std::to_string(max);
			line += "] ";
			line += spells[i];
			line += " ";
			line += how_good(ch->skills[i + 1].learned);
			line += " \n\r";
			if(!append_show_skill_line(buffer, line)) {
				return;
			}
		}
	}
}

void show_skill_append_martial_sheet(struct char_data* ch, int classe, std::string& buffer) {
	const int liv = GetMaxLevel(ch);
	for(int i = 0; i < MAX_SPL_LIST && spells[i] != nullptr && *spells[i] != '\n'; ++i) {
		if(!CheckPrac(classe, i + 1, liv)) {
			continue;
		}
		/* Livello mostrato: placeholder 1 finche' spell_info non avra' colonne per guerriero/ladro/monaco/barbaro
		 * (vedi todo: estendere spell_list / spell_info_type per tutte le classi). */
		std::string line = "[1] ";
		line += spells[i];
		line += " ";
		line += how_good(ch->skills[i + 1].learned);
		line += " \n\r";
		if(!append_show_skill_line(buffer, line)) {
			return;
		}
	}
}

void show_skill_send_spell_sheet(struct char_data* ch, ShowSkillSpellColumn col, const char* introAct) {
	act(introAct, false, ch, nullptr, nullptr, TO_CHAR);
	SET_BIT(ch->player.user_flags, USE_PAGING);
	std::string buffer;
	show_skill_append_cylinder_sheet(ch, col, buffer);
	page_string(ch->desc, buffer.c_str(), true);
}

void show_skill_send_martial_sheet(struct char_data* ch, int classe, const char* introAct) {
	act(introAct, false, ch, nullptr, nullptr, TO_CHAR);
	SET_BIT(ch->player.user_flags, USE_PAGING);
	std::string buffer;
	show_skill_append_martial_sheet(ch, classe, buffer);
	page_string(ch->desc, buffer.c_str(), true);
}

} // namespace

ACTION_FUNC(do_show_skill) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_show_skill (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}
	if(ch->skills == nullptr) {
		return;
	}
	if(arg == nullptr) {
		act("$c0005Dovresti dirmi di quale classe vuoi la lista.$c0007\n\r", false, ch, nullptr, nullptr,
			TO_CHAR);
		return;
	}
	while(*arg != '\0' && std::isspace(static_cast<unsigned char>(*arg))) {
		++arg;
	}
	if(*arg == '\0') {
		act("$c0005Dovresti dirmi di quale classe vuoi la lista.$c0007\n\r", false, ch, nullptr, nullptr,
			TO_CHAR);
		return;
	}

	switch(*arg) {
	case 'w':
	case 'W':
	case 'f':
	case 'F':
		if(!HasClass(ch, CLASS_WARRIOR)) {
			act("$c0005Scommetto che pensi di essere $u guerrier$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_martial_sheet(
			ch, CLASS_WARRIOR,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 't':
	case 'T':
		if(!HasClass(ch, CLASS_THIEF)) {
			act("$c0005Scommetto che pensi di essere $u ladr$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_martial_sheet(
			ch, CLASS_THIEF,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 'M':
	case 'm':
		if(!HasClass(ch, CLASS_MAGIC_USER)) {
			act("$c0005Scommetto che pensi di essere $u mag$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Magic,
			"$c0005La tua classe puo' imparare i seguenti incantesimi:$c0007\n\r");
		return;

	case 'C':
	case 'c':
		if(!HasClass(ch, CLASS_CLERIC)) {
			act("$c0005Scommetto che pensi di essere $u chieric$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Cleric,
			"$c0005La tua classe puo' imparare i seguenti incantesimi:$c0007\n\r");
		return;

	case 'D':
	case 'd':
		if(!HasClass(ch, CLASS_DRUID)) {
			act("$c0005Scommetto che pensi di essere $u druid$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Druid,
			"$c0005La tua classe puo' imparare i seguenti incantesimi:$c0007\n\r");
		return;

	case 'K':
	case 'k':
		if(!HasClass(ch, CLASS_MONK)) {
			act("$c0005Scommetto che pensi di essere $u maestr$b d'arti marziali.$c0007\n\r", false, ch,
				nullptr, nullptr, TO_CHAR);
			return;
		}
		show_skill_send_martial_sheet(
			ch, CLASS_MONK,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 'b':
	case 'B':
		if(!HasClass(ch, CLASS_BARBARIAN)) {
			act("$c0005Scommetto che pensi di essere $u berserker.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_martial_sheet(
			ch, CLASS_BARBARIAN,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 'S':
	case 's':
		if(!HasClass(ch, CLASS_SORCERER)) {
			act("$c0005Scommetto che pensi di essere $u maestr$b delle arti oscure.$c0007\n\r", false, ch,
				nullptr, nullptr, TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Sorcerer,
			"$c0005La tua classe puo' imparare i seguenti incantesimi:$c0007\n\r");
		return;

	case 'p':
	case 'P':
		if(!HasClass(ch, CLASS_PALADIN)) {
			act("$c0005Scommetto che pensi di essere $u paladin$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Paladin,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 'R':
	case 'r':
		if(!HasClass(ch, CLASS_RANGER)) {
			act("$c0005Scommetto che pensi di essere $u ranger.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Ranger,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	case 'i':
	case 'I':
		if(!HasClass(ch, CLASS_PSI)) {
			act("$c0005Scommetto che pensi di essere $u psionic$b.$c0007\n\r", false, ch, nullptr, nullptr,
				TO_CHAR);
			return;
		}
		show_skill_send_spell_sheet(
			ch, ShowSkillSpellColumn::Psi,
			"$c0005La tua classe puo' imparare le seguenti abilita':$c0007\n\r");
		return;

	default:
		act("$c0005Quale classe?$c0007\n\r", false, ch, nullptr, nullptr, TO_CHAR);
		return;
	}
}

namespace {

constexpr int kScanDirLast = 5;

const char* scan_visible_char_label(struct char_data* spud) {
	const char* const label = IS_NPC(spud) ? spud->player.short_descr : GET_NAME(spud);
	return (label != nullptr) ? label : "";
}

/** Visita ogni stanza raggiungibile lungo dir. maxSteps < 0 = fino a muro.
 *  visit ritorna false per fermare la camminata (trovato / fine).
 *  onBlocked chiamato con range (1-based come il codice storico) se clearpath fallisce.
 *  skipLoopbackToStart: se true, non visitare quando la stanza successiva coincide con ch->in_room (do_scan).
 *  Ritorna true se visit ha ritornato false (fermato dal visitatore). */
using LinearVisitFn = bool (*)(struct char_data* ch, struct room_data* rp, int range, int dir, long roomVnum,
							   void* user);
using LinearBlockedFn = void (*)(int range, void* user);

static bool linear_walk(struct char_data* ch, int dir, int maxSteps, bool skipLoopbackToStart,
						LinearVisitFn visit, LinearBlockedFn onBlocked, void* user) {
	long rm = ch->in_room;
	int range = 0;
	while(maxSteps < 0 || range < maxSteps) {
		++range;
		const int to_room = clearpath(ch, rm, dir);
		if(to_room <= 0) {
			if(onBlocked != nullptr) {
				onBlocked(range, user);
			}
			return false;
		}
		rm = to_room;
		if(skipLoopbackToStart && rm == ch->in_room) {
			continue;
		}
		struct room_data* const rp = real_roomp(rm);
		if(rp == nullptr) {
			mudlog(LOG_SYSERR, "linear_walk: real_roomp(%ld) nullo (dir %d)", rm, dir);
			return false;
		}
		if(!visit(ch, rp, range, dir, rm, user)) {
			return true;
		}
	}
	return false;
}

void scan_emit_others_in_current_room(struct char_data* ch, int* nfnd) {
	struct room_data* const rp = real_roomp(ch->in_room);
	if(rp == nullptr) {
		return;
	}
	for(struct char_data* spud = rp->people; spud != nullptr; spud = spud->next_in_room) {
		if(spud == ch) {
			continue;
		}
		if(!CAN_SEE(ch, spud) || IS_SET(spud->specials.affected_by, AFF_HIDE)) {
			continue;
		}
		std::ostringstream hereLine;
		hereLine << std::setw(30) << std::right << scan_visible_char_label(spud) << " : qui\n\r";
		send_to_char(hereLine.str().c_str(), ch);
		++*nfnd;
	}
}

struct ScanAlongCtx {
	const char* const* rng_desc;
	std::size_t rng_desc_len;
	const char* dir_phrase;
	int* nfnd;
};

static bool scan_along_visit(struct char_data* ch, struct room_data* rp, int range, int dir, long roomVnum,
							  void* user) {
	(void)dir;
	(void)roomVnum;
	auto* const ctx = static_cast<ScanAlongCtx*>(user);
	const int rngIdx = std::min(range, static_cast<int>(ctx->rng_desc_len) - 1);
	for(struct char_data* spud = rp->people; spud != nullptr; spud = spud->next_in_room) {
		if(!CAN_SEE(ch, spud) || IS_SET(spud->specials.affected_by, AFF_HIDE)) {
			continue;
		}
		std::ostringstream scanLine;
		scanLine << std::setw(30) << std::right << scan_visible_char_label(spud) << " : "
				 << ctx->rng_desc[static_cast<std::size_t>(rngIdx)] << " " << ctx->dir_phrase << "\n\r";
		send_to_char(scanLine.str().c_str(), ch);
		++*ctx->nfnd;
	}
	return true;
}

void scan_emit_along_direction(struct char_data* ch, int dir, int max_range,
							   const char* const* rng_desc, std::size_t rng_desc_len, const char* dir_phrase,
							   int* nfnd) {
	ScanAlongCtx ctx{rng_desc, rng_desc_len, dir_phrase, nfnd};
	(void)linear_walk(ch, dir, max_range, true, scan_along_visit, nullptr, &ctx);
}

} // namespace

struct FindLinearTargCtx {
	struct char_data* targ;
	int* rng;
	int* dr;
};

static bool can_see_linear_visit(struct char_data* ch, struct room_data* rp, int range, int dir, long roomVnum,
								 void* user) {
	(void)roomVnum;
	auto* const ctx = static_cast<FindLinearTargCtx*>(user);
	for(struct char_data* spud = rp->people; spud != nullptr; spud = spud->next_in_room) {
		if(spud == ctx->targ && CAN_SEE(ch, spud)) {
			*ctx->rng = range;
			*ctx->dr = dir;
			return false;
		}
	}
	return true;
}

struct GetCharLinearWalkCtx {
	const char* name;
	int* rf;
	struct char_data* found;
};

static void get_char_linear_blocked(int range, void* user) {
	auto* const ctx = static_cast<GetCharLinearWalkCtx*>(user);
	*ctx->rf = range;
}

static bool get_char_linear_visit(struct char_data* ch, struct room_data* rp, int range, int dir, long roomVnum,
								   void* user) {
	(void)rp;
	(void)dir;
	auto* const ctx = static_cast<GetCharLinearWalkCtx*>(user);
	ctx->found = get_char_near_room_vis(ch, ctx->name, roomVnum);
	if(ctx->found != nullptr) {
		*ctx->rf = range;
		return false;
	}
	return true;
}

/* this command will only be used for immorts as I am using it as a way */
/* to figure out how to look into rooms next to this room. Will be using*/
/* the code for throwing items. I figure there is no IC reason for a PC */
/* to have a command like this. Do what ya want on your on MUD                 */
ACTION_FUNC(do_scan) {
	static const char* dir_desc[] = {
		"a nord",
		"ad est",
		"a sud",
		"ad ovest",
		"in alto",
		"in basso"
	};
	static const char* rng_desc[] = {
		"qui",
		"qui accanto",
		"nelle vicinanze",
		"andando",
		"andando",
		"laggiu'",
		"laggiu'",
		"in lontananza",
		"in lontananza"
	};
	std::array<char, MAX_STRING_LENGTH> arg1{};
	std::array<char, MAX_STRING_LENGTH> arg2{};

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_scan (act.info.cpp)");
		return;
	}
	if(ch->in_room == NOWHERE) {
		send_to_char("Non sei in una stanza valida.\n\r", ch);
		return;
	}
	if(real_roomp(ch->in_room) == nullptr) {
		mudlog(LOG_SYSERR, "do_scan: real_roomp(in_room) nullo per %s", GET_NAME(ch));
		send_to_char("Qualcosa non va in questa stanza.\n\r", ch);
		return;
	}

	/*
	 * Check mortals spot skill, and give THEM a max scan of
	 * 2 rooms.
	 */

	if(!ch->skills) {
		send_to_char("Non hai nessuno skill!\n\r",ch);
		return;
	}

	int max_range = 6;
	if(GetMaxLevel(ch)<IMMORTALE) {
		if(!ch->skills[SKILL_SPOT].learned) {
			send_to_char("Non sei stato allenato per localizzare (spot).\n\r",ch);
			return;
		}

		if(dice(1,101) > ch->skills[SKILL_SPOT].learned) {
			send_to_char("Assolutamente nessuno da nessuna parte.\n\r",ch);
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
			return;
		}  /* failed */

		max_range=2;       /* morts can only spot two rooms away */

	}  /* was mortal */

	argument_split_2(arg, arg1.data(), arg2.data());
	(void)arg2;
	const int rawSd = search_block(arg1.data(), dirs, FALSE);
	const bool fullScan = (rawSd < 0 || rawSd > kScanDirLast);
	std::string roomAct;
	std::string viewerMsg;
	int smin;
	int smax;
	int swt;
	if(fullScan) {
		smin = 0;
		smax = kScanDirLast;
		swt = 3;
		roomAct = "$n scruta intensamente i dintorni.";
		viewerMsg = "Guardandoti intorno con attenzione vedi...\n\r";
	}
	else {
		smin = rawSd;
		smax = rawSd;
		swt = 1;
		roomAct = "$n scruta intensamente ";
		roomAct += dirsTo[rawSd];
		roomAct += ".";
		viewerMsg = "Guardando attentamente ";
		viewerMsg += dirsTo[rawSd];
		viewerMsg += ", vedi...\n\r";
	}

	act(roomAct.c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	send_to_char(viewerMsg.c_str(), ch);
	int nfnd = 0;
	scan_emit_others_in_current_room(ch, &nfnd);
	for(int i = smin; i <= smax; i++) {
		scan_emit_along_direction(ch, i, max_range, rng_desc, std::size(rng_desc), dir_desc[i], &nfnd);
	}
	if(nfnd==0) {
		send_to_char("Assolutamente nessuno da nessuna parte.\n\r",ch);
	}
	WAIT_STATE(ch, swt * PULSE_VIOLENCE);
}

/* CheckCharAffected: in origine routine di debug che scorreva character_list e le affect,
 * verificando nMagicNumber e type degli affect, con abort() in caso di anomalie.
 * In Nebbie mancavano definizione e chiamate: solo un avanzamento orfano,
 * rimosso per evitare confusione. Se serve di nuovo il sanity-check, va reintrodotta
 * l'implementazione completa e i punti di invocazione.
 * void CheckCharAffected(char* msg); */

void list_groups(struct char_data* ch,const char* szArg, int iCmd) {
	(void)szArg;
	(void)iCmd;

	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in list_groups (act.info.cpp)");
		return;
	}
	if(ch->desc == nullptr) {
		return;
	}

	std::string buf;
	buf += "$c0015[------- Gruppi di avventurieri -------]\n\r";

	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		if(d->connected != 0) {
			continue;
		}
		struct char_data* const person = d->character;
		if(person == nullptr || person->master != nullptr || !IS_AFFECTED(person, AFF_GROUP)) {
			continue;
		}
		if(person->specials.group_name == nullptr || !CAN_SEE(ch, person)) {
			continue;
		}
		buf += "          $c0015";
		buf += person->specials.group_name;
		buf += "\n\r$c0014";
		buf += fname(GET_NAME(person));
		buf += "\n\r";

		int count = 0;
		for(struct follow_type* f = person->followers; f != nullptr; f = f->next) {
			if(f->follower == nullptr) {
				mudlog(LOG_ERROR,
					   "PG con AFF_GROUP ma follower nullo in list_groups (act.info.cpp).");
				continue;
			}
			if(!IS_AFFECTED(f->follower, AFF_GROUP) || !IS_PC(f->follower)) {
				continue;
			}
			++count;
			const char* const mname = GET_NAME(f->follower);
			if(CAN_SEE(ch, f->follower) && mname != nullptr && std::strlen(mname) > 1) {
				buf += "$c0013";
				buf += fname(mname);
				buf += "\n\r";
			}
			else {
				buf += "$c0013Qualcuno\n\r";
			}
		}
		if(count < 1) {
			send_to_char("Il nome del tuo gruppo e' stato rimosso "
						 "poiche' il gruppo e' troppo piccolo.\n\r", person);
			free(person->specials.group_name);
			person->specials.group_name = nullptr;
		}
	}
	buf += "\n\r$c0015[---------- Fine lista --------------]\n\r";
	page_string(ch->desc, buf.c_str(), true);
}

int can_see_linear(struct char_data* ch, struct char_data* targ, int* rng,
				   int* dr) {
	if(ch == nullptr || targ == nullptr || rng == nullptr || dr == nullptr) {
		return -1;
	}
	FindLinearTargCtx ctx{targ, rng, dr};
	for(int i = 0; i < 6; i++) {
		if(linear_walk(ch, i, 6, false, can_see_linear_visit, nullptr, &ctx)) {
			return i;
		}
	}
	return -1;
}

/**************************************************************************
 * Questa funzione restituisce il puntatore ad un personaggio se questo
 * esiste e se e' nella direzione indicata. La stringa arg deve essere nel
 * seguente formato: <dir> at <target> o semplicemente <target>.
 * Nel primo caso, cerca il personaggio con il nome <target> nella direzione
 * <dir>, nel secondo caso cerca il personaggio <target> nella locazione
 * di ch.
 * ***********************************************************************/

struct char_data* get_char_linear(struct char_data* ch,const char* arg, int* rf,
								  int* df) {
	std::array<char, MAX_STRING_LENGTH> tmpname{};

	if(ch == nullptr || arg == nullptr || rf == nullptr || df == nullptr) {
		return nullptr;
	}

	arg = one_argument(arg, tmpname.data());

	if(struct char_data* const inRoom = get_char_room_vis(ch, tmpname.data()); inRoom != nullptr) {
		*rf = 0;
		*df = -1;
		return inRoom;
	}

	*df = search_block(tmpname.data(), exitKeywords, FALSE);
	if(*df < 0) {
		return nullptr;
	}

	arg = one_argument(arg, tmpname.data());
	if(strn_cmp(tmpname.data(), "at", 2) && isspace(tmpname[ 2 ])) {
		arg = one_argument(arg, tmpname.data());
	}

	GetCharLinearWalkCtx ctx{tmpname.data(), rf, nullptr};
	(void)linear_walk(ch, *df, -1, false, get_char_linear_visit, get_char_linear_blocked, &ctx);
	return ctx.found;
}
} // namespace Alarmud
