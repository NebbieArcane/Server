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
struct special_proc_func_entry {
	const char* nome;
	genericspecial_func proc;
};
extern struct special_proc_entry otherproc[];
} // namespace Alarmud
#endif /* __SPECIALPROC_HPP */

