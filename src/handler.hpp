/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef __HANDLER_HPP
#define __HANDLER_HPP
namespace Alarmud {

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

void AddAffects( struct char_data* ch, struct obj_data* o) ;
void CheckCharList() ;
int GiveMinStrToWield(struct obj_data* obj, struct char_data* ch) ;
void affect_from_char( struct char_data* ch, short skill) ;
void affect_join( struct char_data* ch, struct affected_type* af, bool avg_dur, bool avg_mod );
void affect_modify(struct char_data* ch,byte loc, long mod, long bitv,bool add) ;
void affect_remove( struct char_data* ch, struct affected_type* af ) ;
void affect_to_char( struct char_data* ch, struct affected_type* af ) ;
void affect_total(struct char_data* ch) ;
bool affected_by_spell( struct char_data* ch, short skill ) ;
void append_to_string_block(struct string_block* sb, const char* str) ;
int apply_ac(struct char_data* ch, int eq_pos) ;
void char_from_room(struct char_data* ch) ;
void char_to_room(struct char_data* ch, long room) ;
struct obj_data* create_money( int amount ) ;
void destroy_string_block(struct string_block* sb) ;
void equip_char(struct char_data* ch, struct obj_data* obj, int pos) ;
void extract_char(struct char_data* ch) ;
void extract_char_smarter(struct char_data* ch, long save_room) ;
void extract_obj(struct obj_data* obj) ;
char* fname(char* namelist) ;
int generic_find(const char* arg, int bitvector, struct char_data* ch,struct char_data** tar_ch, struct obj_data** tar_obj) ;
struct char_data* get_char(char* name) ;
struct char_data* get_char_near_room_vis(struct char_data* ch, char* name, long next_room) ;
struct char_data* get_char_num(int nr) ;
struct char_data* get_char_room(char* name, int room) ;
struct char_data* get_char_room_vis(struct char_data* ch, const char* name) ;
struct char_data* get_char_vis( struct char_data* ch, const char* name ) ;
struct char_data* get_char_vis_world(struct char_data* ch, const char* name,int* count) ;
int get_number(char** name) ;
struct obj_data* get_obj(char* name) ;
struct obj_data* get_obj_in_list(char* name, struct obj_data* list) ;
struct obj_data* get_obj_in_list_num(int num, struct obj_data* list) ;
struct obj_data* get_obj_in_list_vis(struct char_data* ch, const char* name,struct obj_data* list) ;
struct obj_data* get_obj_num(int nr) ;
struct obj_data* get_obj_vis( struct char_data* ch, char* name ) ;
struct obj_data* get_obj_vis_accessible(struct char_data* ch, char* name) ;
struct obj_data* get_obj_vis_world(struct char_data* ch, char* name,int* count);
int how_many_spell(struct char_data* ch, short skill) ;
void init_string_block(struct string_block* sb) ;
void reset_original_numattacks(struct char_data* ch) ;
int isname(const char* str, const char* namelist) ;
int isname2(const char* str, const char* namelist) ;
void obj_from_char(struct obj_data* object) ;
void obj_from_obj(struct obj_data* obj) ;
void obj_from_room(struct obj_data* object) ;
void obj_to_char( struct obj_data* object, struct char_data* ch ) ;
void obj_to_obj(struct obj_data* obj, struct obj_data* obj_to) ;
void obj_to_room(struct obj_data* object, long room) ;
void obj_to_room2(struct obj_data* object, long room) ;
void object_list_new_owner(struct obj_data* list, struct char_data* ch) ;
void one_affect_from_char( struct char_data* ch, short skill) ;
void page_string_block(struct string_block* sb, struct char_data* ch) ;
int split_string(char* str, const char* sep, char** argv);
struct obj_data* unequip_char(struct char_data* ch, int pos) ;
void update_char_objects( struct char_data* ch ) ;
void update_object( struct obj_data* obj, int use) ;
} // namespace Alarmud
#endif // _HANDLER_HPP

