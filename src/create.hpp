/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* src/create.cpp */
#ifndef __CREATE_HPP
#define __CREATE_HPP
namespace Alarmud {
void ChangeRoomFlags(struct room_data* rp, struct char_data* ch, char* arg, int type);
void do_redit(struct char_data* ch, char* arg, int cmd);
void UpdateRoomMenu(struct char_data* ch);
void RoomEdit(struct char_data* ch, char* arg);
void ChangeRoomName(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeRoomDesc(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeRoomType(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeExitDir(struct room_data* rp, struct char_data* ch, char* arg, int type);
void AddExitToRoom(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeExitNumber(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeKeyNumber(struct room_data* rp, struct char_data* ch, char* arg, int type);
void ChangeExitKeyword(struct room_data* rp, struct char_data* ch, char* arg, int type);
void DeleteExit(struct room_data* rp, struct char_data* ch, char* arg, int type);
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
} // namespace Alarmud
#endif

