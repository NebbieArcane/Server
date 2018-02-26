/*$Id: parser.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/

#ifndef __PARSER_HPP
#define __PARSER_HPP
#define MAGIC    96  /* Magic number for the hash table */
#include "config.hpp"
#include "structs.hpp"

typedef struct command_node NODE;

struct command_node {
	char* name;
	void (*func)(struct char_data* ch, char* arg, int cmd);
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
typedef void (*pCommandFunc) ( struct char_data*, char*, int );
void AddCommand(char* name, pCommandFunc,int number, int min_pos, int min_lev);
void AddNodeTail(NODE* n, int length, int radix) ;
char* FindCommandName(int num) ;
int FindCommandNumber(char* cmd) ;
NODE* FindValidCommand(char* name) ;
void GenerateHash() ;
void InitRadix() ;
NODE* SearchForNodeByName(NODE* head, char* name, int len) ;
#endif // __PARSER_HPP
