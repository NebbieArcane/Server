/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _ACT_OBJ_WEAR_INFER_HPP
#define _ACT_OBJ_WEAR_INFER_HPP
namespace Alarmud {

constexpr int kObjWearKeywordNone = -2;
constexpr int kObjWearEquipPosNone = -1;

struct ObjWearInfer {
	int keyword;
	int equipPos;
};

ObjWearInfer obj_infer_wear(long wearFlags, int typeFlag);

} // namespace Alarmud
#endif // _ACT_OBJ_WEAR_INFER_HPP
