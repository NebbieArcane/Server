/* src/create.cpp */
#ifndef __CREATE_HPP
#define __CREATE_HPP
#include "config.hpp"
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
#endif
