/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: parser.c,v 1.1.1.1 2002/02/13 11:14:54 root Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cstdlib>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "parser.hpp"
#include "snew.hpp"
namespace Alarmud {


/* Sorted selection of the commands for quick lookup */
struct radix_list radix_head[27];

/* Quick reference hash table */
byte HashTable[256];

/* Command list is allocated at run-time in the following order:
** name, command pointer, number, min_position, min_level, next, previous.
** the number can be anything, it's no longer really needed, although
** it is recommended that you keep them in numeric order to avoid confusion.
** NOTE: next and previous MUST be defined as NULL to avoid any possible
** problems.
*/


/* Adds a command to the Command List radix. */
void AddCommand(char* name, command_func func, int number, int min_pos, int min_lev) {
	NODE* n;
	int len, radix;

	n = (NODE*) malloc(sizeof(NODE));

	n->name = (char*)strdup(name);
	n->func = func;
	n->number = number;
	n->min_pos = min_pos;
	n->min_level = min_lev;
	n->next = NULL;
	n->previous = NULL;
	n->log = 0;

	radix = HashTable[ (int)(*name) ];
	len = strlen( name );

	AddNodeTail(n, len, radix);
}


/* Generates a hash table that assigns 1 - 26 to 'a' - 'z' and 'A' - 'Z'.
** All other results are 0
*/
void GenerateHash() {
	register int i;

	for(i = 0; i <= 255; i++)
		if((i >= 'a') && (i <= 'z'))
		{ HashTable[i] = i - MAGIC; }
		else if((i >= 'A') && (i <= 'Z'))
		{ HashTable[i] = i - (MAGIC - 32); }
		else
		{ HashTable[i] = 0; }
}



/* Adds a node to the end of a radix-sorted linked list.
*/
void AddNodeTail(NODE* n, int length, int radix) {
	/* Check to see if we're at the beginning, if so, start here. */
	if(radix_head[radix].next == NULL) {
		radix_head[radix].next = n;
		radix_head[radix].number = 1;
		radix_head[radix].max_len = length;
		n->previous = NULL;
		return;
	}

	/* Traverse the list until we find the end, when we find it, add to it */
	{
		register NODE* i;

		for(i = radix_head[radix].next; i->next; i = i->next);
		i->next = n;
		n->previous = i;
		radix_head[radix].number++;
		n->next = NULL;
		if(radix_head[radix].max_len < length)
		{ radix_head[radix].max_len = length; }
	}
}


/* This will search by name for a specific node and return a pointer to it.
** Passed is a pointer to the first node in the radix.  Any checking as to
** whether or not the node is valid should happen before entering here.
** NOTE: This uses partial matching, change strncmp to strcmp for full matching
** Return value is the node if it exists, or NULL if it does not.
*/
NODE* SearchForNodeByName(NODE* head, char* name, int len) {
	register NODE* i;

	i = head;
	while(i) {
		if(!(strncmp(i->name, name, len)))
		{ return(i); }
		i = i->next;
	}

	return(NULL);
}


/* Initialization for the radix sorting routines.  Call this to begin the sort.
** This will generate the hash table and sort everything via the hash-table.
*/
void InitRadix() {
	register int i;

	for(i = 0; i <= 26; i++) {
		radix_head[i].next = NULL;
		radix_head[i].number = 0;
		radix_head[i].max_len = 0;
	}

	GenerateHash();

}


/* This will do all of the validation and search for a NODE by name.
** Will return a pointer to the NODE if it exists, NULL if it doesn't.
*/
NODE* FindValidCommand(char* name) {
	register int len;
	register int radix;

	radix = HashTable[ (int)(*name) ];
	len = strlen(name);

	if(radix_head[radix].number && len <= radix_head[radix].max_len)
	{ return(SearchForNodeByName(radix_head[radix].next, name, len)); }

	return(NULL);
}

/* some useful&stupid functions */

int FindCommandNumber(char* cmd) {
	int     i;
	NODE*    n;
	for(i=0; i<27; i++)
		if(radix_head[i].number)
			for(n=radix_head[i].next; n; n=n->next)
				if(strcmp(cmd,n->name)==0) { return n->number; }
	return -1;
}

char* FindCommandName(int num) {
	int     i;
	NODE*    n;
	for(i=0; i<27; i++)
		if(radix_head[i].number)
			for(n=radix_head[i].next; n; n=n->next)
				if(n->number==num) { return n->name; }
	return NULL;
}

} // namespace Alarmud

