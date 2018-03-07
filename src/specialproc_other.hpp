/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SPECIALPROC_HPP
#define __SPECIALPROC_HPP
#define SPECIALPROC_OTHER(name) int (name)( struct char_data*, int, char*, void*, int )
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
typedef SPECIALPROC_OTHER(*special_proc);
struct special_proc_entry {
	char* nome;
	special_proc proc;
};
extern struct special_proc_entry otherproc[];
} // namespace Alarmud
#endif __SPECIALPROC_HPP

