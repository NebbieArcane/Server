/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PARSER_HPP
#define __PARSER_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#include "typedefs.hpp"
namespace Alarmud {

#define MAGIC    96  /* Magic number for the hash table */

typedef struct command_node NODE;

struct command_node {
	char* name;
	command_func func;
	int number;
	byte min_pos;
	byte min_level;
	byte log;
	struct command_node* next;
	struct command_node* previous;
};

struct radix_list {
	struct command_node* next;
	unsigned short int number;
	byte max_len;
};
extern struct radix_list radix_head[27];
extern byte HashTable[256];

void AddCommand(char* name, command_func,int number, int min_pos, int min_lev);
void AddNodeTail(NODE* n, int length, int radix) ;
char* FindCommandName(int num) ;
int FindCommandNumber(char* cmd) ;
NODE* FindValidCommand(char* name) ;
void GenerateHash() ;
void InitRadix() ;
NODE* SearchForNodeByName(NODE* head, char* name, int len) ;
} // namespace Alarmud
#endif // __PARSER_HPP

