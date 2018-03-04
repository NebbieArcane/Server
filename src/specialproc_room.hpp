#ifndef __SPECIALPROC_ROOM_HPP
#define __SPECIALPROC_ROOM_HPP
typedef int (*room_proc)( struct char_data*, int, char*, struct room_data*, int );
struct RoomSpecialProcEntry {
	char* nome;
	room_proc proc;
};
extern struct RoomSpecialProcEntry roomproc[];
#endif // __SPECIALPROC_ROOM_HPP
