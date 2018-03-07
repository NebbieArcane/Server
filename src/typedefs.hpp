/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_TYPEDEFS_HPP_
#define SRC_TYPEDEFS_HPP_
/***************************  System  include ************************************/
#include <sys/types.h>
/***************************  Local    include ************************************/
namespace Alarmud {


typedef char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
#if !defined( __cplusplus ) || defined( DEFINE_BOOL )
typedef char bool;
#endif
typedef char byte;
} // namespace Alarmud
#endif /* SRC_TYPEDEFS_HPP_ */

