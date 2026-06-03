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
#include "act.obj_get.hpp"
#include "act.obj.hpp"
#include "act.other.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "signals.hpp"
#include "spell_parser.hpp"
#include "trap.hpp"
#include "utility.hpp"

namespace Alarmud {

namespace {

enum class ObjGetMode {
	MissingTarget,
	AllInRoom,
	NamedInRoom,
	AllFromAllError,
	AllFromContainer,
	OneContainerOnlyError,
	NamedFromContainer,
};

enum class ObjPickupResult {
	Success,
	SkippedNotVisible,
	FailedNotake,
	FailedTooHeavy,
	FailedHandsFull,
	AbortedMounted,
	AbortedTrap,
};

struct ObjPickupPolicy {
	enum class TrapCheck { None, AnyTrap, GetTrap, InsideTrap };
	enum class MsgStyle { Room, Named, Container };

	TrapCheck trapCheck = TrapCheck::None;
	struct obj_data* insideTrapContainer = nullptr;
	bool requireVisible = false;
	bool carryingContainerInInv = false;
	int carryCountSlack = 0;
	bool weightStrictLess = false;
	MsgStyle msgStyle = MsgStyle::Room;
};

ObjGetMode obj_parse_get_mode(const char* arg1, const char* arg2) {
	if(arg1 == nullptr || arg1[0] == '\0') {
		return ObjGetMode::MissingTarget;
	}
	if(arg2 == nullptr || arg2[0] == '\0') {
		return obj_arg_is_all(arg1) ? ObjGetMode::AllInRoom : ObjGetMode::NamedInRoom;
	}
	if(obj_arg_is_all(arg1)) {
		return obj_arg_is_all(arg2) ? ObjGetMode::AllFromAllError : ObjGetMode::AllFromContainer;
	}
	return obj_arg_is_all(arg2) ? ObjGetMode::OneContainerOnlyError : ObjGetMode::NamedFromContainer;
}

bool obj_get_abort_if_mounted(struct char_data* ch, struct obj_data* obj, const char* statusTag) {
	if(ch == nullptr || obj == nullptr) {
		return false;
	}
	if((ch->player.oggetti > (MAX_OBJ_SAVE - 5)) && MOUNTED(ch)) {
		act("Non riesci a prendere $p mentre stai cavalcando.", false, ch, obj, nullptr, TO_CHAR);
		SetStatus(statusTag, nullptr);
#if NODUPLICATES
		if(IS_PC(ch)) {
			do_save(ch, "", 0);
		}
#endif
		return true;
	}
	return false;
}

bool obj_get_run_trap_check(struct char_data* ch, struct obj_data* obj, struct obj_data* container,
                            const ObjPickupPolicy& policy) {
	if(ch == nullptr || IS_NPC(ch)) {
		return false;
	}
	switch(policy.trapCheck) {
	case ObjPickupPolicy::TrapCheck::AnyTrap:
		return CheckForAnyTrap(ch, obj);
	case ObjPickupPolicy::TrapCheck::GetTrap:
		return CheckForGetTrap(ch, obj);
	case ObjPickupPolicy::TrapCheck::InsideTrap:
		return container != nullptr && CheckForInsideTrap(ch, container);
	default:
		return false;
	}
}

ObjPickupResult obj_try_pickup(struct char_data* ch, struct obj_data* obj, struct obj_data* container,
                               const ObjPickupPolicy& policy, bool& found, bool& fail) {
	if(ch == nullptr || obj == nullptr) {
		return ObjPickupResult::SkippedNotVisible;
	}
	if(policy.requireVisible && !CAN_SEE_OBJ(ch, obj)) {
		return ObjPickupResult::SkippedNotVisible;
	}
	if(obj_get_run_trap_check(ch, obj, container, policy)) {
		return ObjPickupResult::AbortedTrap;
	}

	const int carryLimit = CAN_CARRY_N(ch) - policy.carryCountSlack;
	if(IS_CARRYING_N(ch) >= carryLimit) {
		const bool hide = policy.msgStyle == ObjPickupPolicy::MsgStyle::Container;
		if(policy.msgStyle == ObjPickupPolicy::MsgStyle::Named) {
			act("Non riesci a prendere $p, hai troppa roba in mano.", hide, ch, obj, nullptr, TO_CHAR);
		}
		else if(policy.msgStyle == ObjPickupPolicy::MsgStyle::Container) {
			act("Non puoi prendere $p, hai gia' roba in mano.", hide, ch, obj, nullptr, TO_CHAR);
		}
		else {
			act("Non puoi prendere $p, hai gia' troppa roba in mano.", hide, ch, obj, nullptr, TO_CHAR);
		}
		fail = true;
		return ObjPickupResult::FailedHandsFull;
	}

	const long long totalWeight = static_cast<long long>(IS_CARRYING_W(ch)) + obj->obj_flags.weight;
	const bool weightOk =
	    policy.carryingContainerInInv ||
	    (policy.weightStrictLess ? totalWeight < CAN_CARRY_W(ch) : totalWeight <= CAN_CARRY_W(ch));
	if(!weightOk) {
		const bool hide = policy.msgStyle == ObjPickupPolicy::MsgStyle::Container;
		if(policy.msgStyle == ObjPickupPolicy::MsgStyle::Named) {
			act("Non riesci a prendere $p, pesa troppo.", hide, ch, obj, nullptr, TO_CHAR);
		}
		else {
			act("Non puoi prendere $p, pesa troppo.", hide, ch, obj, nullptr, TO_CHAR);
		}
		fail = true;
		return ObjPickupResult::FailedTooHeavy;
	}

	if(!CAN_WEAR(obj, ITEM_TAKE)) {
		const bool hide = policy.msgStyle == ObjPickupPolicy::MsgStyle::Container;
		act("Non puoi prendere $p.", hide, ch, obj, nullptr, TO_CHAR);
		fail = true;
		return ObjPickupResult::FailedNotake;
	}

	get(ch, obj, container);
	found = true;
	return ObjPickupResult::Success;
}

bool obj_get_handle_pickup_result(struct char_data* ch, ObjPickupResult result, const char* trapStatus,
                                  const char* mountedStatus) {
	if(result == ObjPickupResult::AbortedTrap) {
		SetStatus(trapStatus, nullptr);
		return true;
	}
	if(result == ObjPickupResult::AbortedMounted) {
		SetStatus(mountedStatus, nullptr);
		return true;
	}
	(void)ch;
	return false;
}

void obj_get_all_visible_in_room(struct char_data* ch, bool& found, bool& fail) {
	ObjPickupPolicy policy{};
	policy.trapCheck = ObjPickupPolicy::TrapCheck::AnyTrap;
	policy.requireVisible = true;
	policy.msgStyle = ObjPickupPolicy::MsgStyle::Room;

	struct obj_data* next = nullptr;
	for(struct obj_data* obj = real_roomp(ch->in_room)->contents; obj != nullptr; obj = next) {
		next = obj->next_content;
		if(obj_get_abort_if_mounted(ch, obj, "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get")) {
			return;
		}
		const ObjPickupResult result = obj_try_pickup(ch, obj, nullptr, policy, found, fail);
		if(obj_get_handle_pickup_result(ch, result, "Ending after CheckForAnyTrap in do_get",
		                               "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get")) {
			return;
		}
	}
}

void obj_get_named_in_room(struct char_data* ch, char* itemName, int num, bool& found, bool& fail) {
	ObjPickupPolicy policy{};
	policy.trapCheck = ObjPickupPolicy::TrapCheck::GetTrap;
	policy.carryCountSlack = 1;
	policy.weightStrictLess = true;
	policy.msgStyle = ObjPickupPolicy::MsgStyle::Named;

	struct obj_data* list = real_roomp(ch->in_room)->contents;
	while(num != 0) {
		struct obj_data* const obj = get_obj_in_list_vis(ch, itemName, list);
		if(obj_get_abort_if_mounted(ch, obj,
		                            "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 2")) {
			return;
		}
		if(obj == nullptr) {
			break;
		}
		list = obj->next_content;
		if(IS_CORPSE(obj) && num != 1) {
			send_to_char("Puoi prendere un solo corpo alla volta.\n\r", ch);
			SetStatus("Ending after IS_CORPSE in do_get", nullptr);
			return;
		}

		const ObjPickupResult result = obj_try_pickup(ch, obj, nullptr, policy, found, fail);
		if(obj_get_handle_pickup_result(ch, result, "Ending after CheckForGetTrap in do_get",
		                               "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 2")) {
			return;
		}
		if(result == ObjPickupResult::FailedHandsFull) {
			break;
		}
		if(num > 0) {
			--num;
		}
	}
}

bool obj_get_open_container(struct char_data* ch, const char* containerName, struct obj_data*& container,
                            bool& fail) {
	container = get_obj_vis_accessible(ch, containerName);
	if(container == nullptr) {
		std::ostringstream os;
		os << "Non vedi nessun " << containerName << '.';
		send_to_char(os.str().c_str(), ch);
		fail = true;
		return false;
	}
	if(GET_ITEM_TYPE(container) != ITEM_CONTAINER) {
		act("$p non e' un contenitore.", false, ch, container, nullptr, TO_CHAR);
		fail = true;
		return false;
	}
	return true;
}

void obj_get_all_from_container(struct char_data* ch, struct obj_data* container, bool carryingContainerInInv,
                                bool& found, bool& fail) {
	ObjPickupPolicy policy{};
	policy.trapCheck = ObjPickupPolicy::TrapCheck::GetTrap;
	policy.requireVisible = true;
	policy.carryingContainerInInv = carryingContainerInInv;
	policy.carryCountSlack = 1;
	policy.msgStyle = ObjPickupPolicy::MsgStyle::Room;

	struct obj_data* next = nullptr;
	for(struct obj_data* obj = container->contains; obj != nullptr; obj = next) {
		next = obj->next_content;
		if(obj_get_abort_if_mounted(
		        ch, obj, "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 4")) {
			return;
		}
		const ObjPickupResult result = obj_try_pickup(ch, obj, container, policy, found, fail);
		if(obj_get_handle_pickup_result(ch, result, "Ending after CheckForGetTrap 2 in do_get",
		                               "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 4")) {
			return;
		}
		if(result == ObjPickupResult::FailedHandsFull) {
			break;
		}
	}
	if(!found && !fail) {
		act("Non c'e niente in $p.", false, ch, container, nullptr, TO_CHAR);
		fail = true;
	}
}

void obj_get_named_from_container(struct char_data* ch, struct obj_data* container, char* itemName, int num,
                                  bool carryingContainerInInv, bool& found, bool& fail) {
	ObjPickupPolicy policy{};
	policy.trapCheck = ObjPickupPolicy::TrapCheck::InsideTrap;
	policy.insideTrapContainer = container;
	policy.carryingContainerInInv = carryingContainerInInv;
	policy.carryCountSlack = 1;
	policy.msgStyle = ObjPickupPolicy::MsgStyle::Container;

	struct obj_data* list = container->contains;
	while(num != 0) {
		struct obj_data* const obj = get_obj_in_list_vis(ch, itemName, list);
		if(obj_get_abort_if_mounted(
		        ch, obj, "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 6")) {
			return;
		}
		if(obj == nullptr) {
			break;
		}
		list = obj->next_content;

		const ObjPickupResult result = obj_try_pickup(ch, obj, container, policy, found, fail);
		if(obj_get_handle_pickup_result(ch, result, "Ending after CheckForInsideTrap in do_get",
		                               "Ending after MOUNTED and player.oggetti > MAX_OBJ_SAVE in do_get case 6")) {
			return;
		}
		if(result == ObjPickupResult::FailedHandsFull) {
			break;
		}
		if(num > 0) {
			--num;
		}
	}

	if(!fail && !found) {
		std::ostringstream os;
		os << "$p non contiene nessun " << itemName << '.';
		act(os.str().c_str(), true, ch, container, nullptr, TO_CHAR);
	}
}
} // namespace

void obj_run_get(struct char_data* ch, const char* arg, int cmd) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in obj_run_get (act.obj_get.cpp)");
		return;
	}

	std::array<char, MAX_STRING_LENGTH> arg1{};
	std::array<char, MAX_STRING_LENGTH> arg2{};
	std::array<char, 1000> newarg{};
	struct obj_data* sub_object = nullptr;
	bool found = false;
	bool fail = false;
	bool has = false;
	int num = 0;

	argument_interpreter(arg, arg1.data(), arg2.data());

	const ObjGetMode mode = obj_parse_get_mode(arg1.data(), arg2.data());

	switch(mode) {
	case ObjGetMode::MissingTarget:
		send_to_char("Cosa vuoi prendere?\n\r", ch);
		break;
	case ObjGetMode::AllInRoom:
		obj_get_all_visible_in_room(ch, found, fail);
		if(!found && !fail) {
			send_to_char("Non c'e' nulla da prendere.\n\r", ch);
		}
		break;
	case ObjGetMode::NamedInRoom:
		num = obj_parse_take_count(arg1.data(), arg1.size(), newarg.data());
		obj_get_named_in_room(ch, arg1.data(), num, found, fail);
		if(!found && !fail) {
			std::ostringstream os;
			os << "Non c'e' nessun " << arg1.data() << " qui.\n\r";
			send_to_char(os.str().c_str(), ch);
		}
		break;
	case ObjGetMode::AllFromAllError:
		send_to_char("Non puoi prendere tutto da tutto.\n\r", ch);
		break;
	case ObjGetMode::AllFromContainer:
		if(obj_get_open_container(ch, arg2.data(), sub_object, fail)) {
			has = get_obj_in_list_vis(ch, arg2.data(), ch->carrying) != nullptr;
			obj_get_all_from_container(ch, sub_object, has, found, fail);
		}
		break;
	case ObjGetMode::OneContainerOnlyError:
		act("Puoi prendere gli oggetti da un solo contenitore alla volta.", false, ch, nullptr, nullptr,
		    TO_CHAR);
		break;
	case ObjGetMode::NamedFromContainer:
		if(!obj_get_open_container(ch, arg2.data(), sub_object, fail)) {
			break;
		}
		if(obj_ci_equal(arg1.data(), "trofeo") && IS_CORPSE(sub_object)) {
			get_trophy(ch, sub_object);
			return;
		}
		has = get_obj_in_list_vis(ch, arg2.data(), ch->carrying) != nullptr;
		num = obj_parse_take_count(arg1.data(), arg1.size(), newarg.data());
		obj_get_named_from_container(ch, sub_object, arg1.data(), num, has, found, fail);
		break;
	}
	SetStatus("Before saving PC in do_get", nullptr);
#if NODUPLICATES
	if(found && IS_PC(ch)) {
		do_save(ch, "", 0);
	}
#endif
	SetStatus("Returning from do_get", nullptr);
}

void get(struct char_data* ch, struct obj_data* obj_object, struct obj_data* sub_object) {
	if(ch == nullptr || obj_object == nullptr) {
		return;
	}

	if(IS_RARE(obj_object) && !EgoSave(ch)) {
		act("Non hai voglia di prendere $p.", false, ch, obj_object, sub_object, TO_CHAR);
		return;
	}

	if(!CheckEgoGet(ch, obj_object) || !CheckGetBarbarianOK(ch, obj_object)) {
		return;
	}

	if(sub_object != nullptr) {
		if(!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
			act("Prendi $p da $P.", false, ch, obj_object, sub_object, TO_CHAR);
			act("$n prende $p da $P.", true, ch, obj_object, sub_object, TO_ROOM);
			obj_from_obj(obj_object);
			obj_to_char(obj_object, ch);
			ch->player.oggetti += 1;
		}
		else {
			act("Prima dovresti aprire $P.", true, ch, nullptr, sub_object, TO_CHAR);
			return;
		}
	}
	else {
		if(obj_object->in_room == NOWHERE) {
			obj_object->in_room = ch->in_room;
		}
		act("Prendi $p.", false, ch, obj_object, nullptr, TO_CHAR);
		act("$n prende $p.", true, ch, obj_object, nullptr, TO_ROOM);
		obj_from_room(obj_object);
		obj_to_char(obj_object, ch);
		ch->player.oggetti += 1;
	}

	if(obj_object->obj_flags.type_flag == ITEM_MONEY && obj_object->obj_flags.value[0] >= 1) {
		const int coins = obj_object->obj_flags.value[0];
		obj_from_char(obj_object);
		std::ostringstream os;
		if(coins > 1) {
			os << "C'erano " << coins << " monete.\n\r";
		}
		else {
			os << "C'era una miserabile moneta.\n\r";
		}
		send_to_char(os.str().c_str(), ch);
		GET_GOLD(ch) += coins;
		if(GET_GOLD(ch) > 500000 && coins > 100000) {
			mudlog(LOG_PLAYERS, "%s just got %d coins", GET_NAME(ch), coins);
		}
		extract_obj(obj_object);
	}
}

} // namespace Alarmud
