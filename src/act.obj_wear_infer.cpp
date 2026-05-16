/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "act.obj_wear_infer.hpp"
#include "autoenums.hpp"

namespace Alarmud {

namespace {

bool obj_has_wear_flag(long wearFlags, unsigned long part) {
	return (wearFlags & static_cast<long>(part)) != 0;
}

} // namespace

ObjWearInfer obj_infer_wear(long wearFlags, int typeFlag) {
	ObjWearInfer result{kObjWearKeywordNone, kObjWearEquipPosNone};

	if(obj_has_wear_flag(wearFlags, ITEM_HOLD)) {
		result = {13, WEAR_LIGHT};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_SHIELD)) {
		result = {14, WEAR_SHIELD};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_FINGER)) {
		result = {1, WEAR_FINGER_R};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_NECK)) {
		result = {2, WEAR_NECK_1};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_WRIST)) {
		result = {11, WEAR_WRIST_R};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_WAISTE)) {
		result = {10, WEAR_WAISTE};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_ARMS)) {
		result = {8, WEAR_ARMS};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_HANDS)) {
		result = {7, WEAR_HANDS};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_FEET)) {
		result = {6, WEAR_FEET};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_LEGS)) {
		result = {5, WEAR_LEGS};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_ABOUT)) {
		result = {9, WEAR_ABOUT};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_HEAD)) {
		result = {4, WEAR_HEAD};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_BODY)) {
		result = {3, WEAR_BODY};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WIELD)) {
		result = {12, WIELD};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_BACK) && typeFlag == ITEM_CONTAINER) {
		result = {15, WEAR_BACK};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_EYE)) {
		result = {17, WEAR_EYES};
	}
	if(obj_has_wear_flag(wearFlags, ITEM_WEAR_EAR)) {
		result = {16, WEAR_EAR_R};
	}

	return result;
}

} // namespace Alarmud
