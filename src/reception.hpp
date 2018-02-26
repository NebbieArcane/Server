#ifndef __RECEPTION_HPP
#define __RECEPTION_HPP
#include "config.hpp"
#include <stdio.h>
#include "structs.hpp"
void CountLimitedItems( struct obj_file_u* st ) ;
void PrintLimitedItems() ;
int ReadObjs( FILE* fl, struct obj_file_u* st) ;
void WriteObjs( FILE* fl, struct obj_file_u* st) ;
void ZeroRent( char* n) ;
void add_obj_cost(struct char_data* ch, struct char_data* re,struct obj_data* obj, struct obj_cost* cost );
int contained_weight(struct obj_data* container) ;
int creceptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
void load_char_extra(struct char_data* ch) ;
void load_char_objs(struct char_data* ch) ;
void load_room_objs( int room ) ;
void obj_store_to_char(struct char_data* ch, struct obj_file_u* st) ;
void obj_store_to_room( int room, struct obj_file_u* st ) ;
void obj_to_store(struct obj_data* obj, struct obj_file_u* st, struct char_data* ch, int bDelete);
void put_obj_in_store( struct obj_data* obj, struct obj_file_u* st ) ;
bool recep_offer(struct char_data* ch, struct char_data* receptionist, struct obj_cost* cost, int forcerent);
int receptionist(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) ;
void save_obj(struct char_data* ch, struct obj_cost* cost, int bDelete) ;
void save_room( int room ) ;
void update_file(struct char_data* ch, struct obj_file_u* st) ;
void update_obj_file() ;
void write_char_extra( struct char_data* ch) ;
void zero_rent( struct char_data* ch) ;
#endif // __RECEPTION_HPP
