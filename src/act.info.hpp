/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
#ifndef _ACT_INFO_HPP
#define _ACT_INFO_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
const char* AlignDesc(int a) ;
const char* ArmorDesc(int a) ;
const char* DamRollDesc(int a) ;
const char* DescAttacks(float a) ;
const char* DescArmorf(float f) ; /* theirs / yours */
const char* DescDamage(float dam) ;
const char* DescRatio(float f) ; /* theirs / yours */
const char* DescRatioF(float f) ; /* theirs / yours */
const char* EqDesc(float a) ;
double GetLagIndex() ;
const char* HitRollDesc(int a) ;
int HowManyConnection(int ToAdd) ;
int MobLevBonus(struct char_data* ch) ;
void ScreenOff(struct char_data* ch) ;
void ShowAltezzaCostituzione(struct char_data* pChar, struct char_data* pTo) ;
const char* SpellfailDesc(int a) ;
void argument_split_2(const char* argument,char* first_arg,char* second_arg) ;
int can_see_linear(struct char_data* ch, struct char_data* targ, int* rng,int* dr);
ACTION_FUNC(do_achievements) ;
ACTION_FUNC(do_checkachielevel) ;
ACTION_FUNC(do_actual_wiz_help) ;
ACTION_FUNC(do_attribute) ;
ACTION_FUNC(do_command_list) ;
ACTION_FUNC(do_consider) ;
ACTION_FUNC(do_credits) ;
ACTION_FUNC(do_display) ;
ACTION_FUNC(do_equipment) ;
ACTION_FUNC(do_examine) ;
ACTION_FUNC(do_exits) ;
ACTION_FUNC(do_help) ;
ACTION_FUNC(do_immortal) ;
ACTION_FUNC(do_info) ;
ACTION_FUNC(do_inventory) ;
ACTION_FUNC(do_levels) ;
ACTION_FUNC(do_look) ;
ACTION_FUNC(do_news) ;
ACTION_FUNC(do_prince) ;
ACTION_FUNC(do_read) ;
ACTION_FUNC(do_resize) ;
ACTION_FUNC(do_scan) ;
ACTION_FUNC(do_score) ;
ACTION_FUNC(do_show_skill) ;
ACTION_FUNC(do_spells) ;
ACTION_FUNC(do_status) ;
ACTION_FUNC(do_time) ;
ACTION_FUNC(do_users) ;
ACTION_FUNC(do_value) ;
ACTION_FUNC(do_weather) ;
ACTION_FUNC(do_where) ;
void do_where_object(struct char_data* ch, struct obj_data* obj,int recurse, struct string_block* sb);
void do_where_person(struct char_data* ch, struct char_data* person,struct string_block* sb);
void owhere(struct char_data* ch, char* nome);
ACTION_FUNC(do_who) ;
ACTION_FUNC(do_wizhelp) ;
ACTION_FUNC(do_wizlist) ;
ACTION_FUNC(do_wiznews) ;
ACTION_FUNC(do_world) ;
char* find_ex_description(char* word, struct extra_descr_data* list) ;
struct char_data* get_char_linear(struct char_data* ch,const char* arg, int* rf, int* df);
struct obj_data* get_object_in_equip_vis(struct char_data* ch,const char* arg, struct obj_data* equipment[], int* j) ;
void list_char_in_room(struct char_data* list, struct char_data* ch) ;
void list_char_to_char(struct char_data* list, struct char_data* ch,  int mode);
void list_exits_in_room(struct char_data* ch) ;
void list_groups(struct char_data* ch,const char* szArg, int iCmd) ;
void list_obj_in_heap(struct obj_data* list, struct char_data* ch) ;
void list_obj_in_room(struct obj_data* list, struct char_data* ch) ;
void list_obj_to_char(struct obj_data* list, struct char_data* ch, int mode, bool show);
char* numbered_object(struct char_data* ch, struct obj_data* obj) ;
char* numbered_person(struct char_data* ch, struct char_data* person) ;
void show_char_to_char(struct char_data* i, struct char_data* ch, int mode) ;
void show_mult_char_to_char(struct char_data* i, struct char_data* ch, int mode, int num);
void show_mult_obj_to_char(struct obj_data* object, struct char_data* ch,int mode, int num);
void show_obj_to_char(struct obj_data* object, struct char_data* ch, int mode) ;
int singular(struct obj_data* o) ;
int which_number_mobile(struct char_data* ch, struct char_data* mob) ;
int which_number_object(struct obj_data* obj) ;
} // namespace Alarmud
#endif // _ACT_INFO_HPP
