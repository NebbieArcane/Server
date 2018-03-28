/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPECIALPROC_ROOM_HPP
#define __SPECIALPROC_ROOM_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "typedefs.hpp"
namespace Alarmud {

struct RoomSpecialProcEntry {
	const char* nome;
	roomspecial_func proc;
};
extern struct RoomSpecialProcEntry roomproc[];
} // namespace Alarmud
#endif // __SPECIALPROC_ROOM_HPP

