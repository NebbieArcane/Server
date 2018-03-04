#ifndef __SPECIALPROC_HPP
#define __SPECIALPROC_HPP
typedef int (*special_proc)( struct char_data*, int, char*, void*, int );
struct special_proc_entry {
	char* nome;
	//int (*proc)( struct char_data *, int, char *, void *, int );
	special_proc proc;
};
struct special_proc_entry otherproc[];
#endif __SPECIALPROC_HPP
