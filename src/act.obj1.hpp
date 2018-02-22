#ifndef _ACT_OBJ1_HPP
#define _ACT_OBJ1_HPP
void do_drop(struct char_data* ch,const char* argument, int cmd) ;
void do_get(struct char_data* ch,const char* argument, int cmd) ;
void do_give(struct char_data* ch,const char* argument, int cmd) ;
void do_pquest(struct char_data* ch,const char* argument, int cmd) ;
void do_put(struct char_data* ch,const char* argument, int cmd) ;
void get( struct char_data* ch, struct obj_data* obj_object,struct obj_data* sub_object );
void get_trophy(struct char_data* ch, struct obj_data* ob) ;
void givexp(struct char_data* ch, struct char_data* victim, int amount);
int newstrlen(const char* p) ;
void truegivexp(struct char_data* ch, struct char_data* victim, int amount) ;
#endif // _ACT_OBJ1_HPP
