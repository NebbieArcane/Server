/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_TYPEDEFS_HPP_
#define SRC_TYPEDEFS_HPP_
/***************************  System  include ************************************/
//#include <sys/types.h>
/***************************  Local    include ************************************/
namespace Alarmud {


typedef char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef char byte;
//Callbacks
enum DamageResult {
	AllLiving, SubjectDead, VictimDead
};
struct char_data;
struct room_data;
struct obj_data;
#define DAMAGE_FUNC(name) DamageResult damage_func( struct char_data*,struct char_data*,int, int, int )
typedef DAMAGE_FUNC((*damage_func));
#define ITERATE_FUNC(name) void name ( int, struct room_data*, void* )
typedef ITERATE_FUNC((*iterate_func));
//do_<something>
#define COMMAND_FUNC(name) void name(struct char_data* ch, const char* arg, int cmd)
typedef COMMAND_FUNC((*command_func));
// Called by findpath
#define FIND_FUNC(name) int name( int, void*)
typedef FIND_FUNC((*find_func));
// Mob/object special procedure
#define GENERICSPECIAL_FUNC(name) int name( struct char_data* ch, int cmd, const char* arg, void* generic, int type)
typedef GENERICSPECIAL_FUNC((*genericspecial_func));
// Room speciam procedureMob/object special procedure
#define ROOMSPECIAL_FUNC(name) int name( struct char_data* ch, int cmd, const char* arg, struct room_data* room, int type )
typedef ROOMSPECIAL_FUNC((*roomspecial_func));

#define OBJSPECIAL_FUNC(name) int name( struct char_data* ch, int cmd, const char* arg, struct obj_data* obj, int type )
typedef OBJSPECIAL_FUNC((*objspecial_func));

#define MOBSPECIAL_FUNC(name) int name( struct char_data* ch, int cmd, const char* arg, struct char_data* mob, int type )
typedef MOBSPECIAL_FUNC((*mobspecial_func));
// Breath function
#define BREATH_FUNC(name) void name( char, struct char_data*, const char*, int, struct char_data*, struct obj_data* )
typedef BREATH_FUNC((*breath_func));
//cast_<something>
#define SPELL_FUNC(name) void name (byte, struct char_data*, const char*, int,struct char_data*, struct obj_data*)
typedef SPELL_FUNC((*spell_func)) ;

#define SCRIPT_FUNC(name) void  name (const char* arg, struct char_data* ch)
typedef SCRIPT_FUNC((*script_func)) ;

#define ACTION_FUNC(name) void name( struct char_data* ch, const char* arg, int cmd)
typedef ACTION_FUNC((*action_func)) ;


} // namespace Alarmud
#endif /* SRC_TYPEDEFS_HPP_ */

