/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPECIALPROC_ROOM_HPP
#define __SPECIALPROC_ROOM_HPP
#define SPECIALPROC_ROOM(name) int (name)( struct char_data*, int, char*, struct room_data*, int )
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {

typedef SPECIALPROC_ROOM(*room_proc);
struct RoomSpecialProcEntry {
	char* nome;
	room_proc proc;
};
extern struct RoomSpecialProcEntry roomproc[];
} // namespace Alarmud
#endif // __SPECIALPROC_ROOM_HPP

