/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPECIALPROC_HPP
#define __SPECIALPROC_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "typedefs.hpp"
namespace Alarmud {
struct OtherSpecialProcEntry {
	const char* nome;
	genericspecial_func proc;
};
extern struct OtherSpecialProcEntry otherproc[];
} // namespace Alarmud
#endif /* __SPECIALPROC_HPP */

