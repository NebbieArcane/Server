/*
 * modify.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_MODIFY_HPP_
#define SRC_MODIFY_HPP_

void bisect_arg(char* arg, int* field, char* string);
struct help_index_element* build_help_index(FILE* fl, int* num);
void check_reboot();
void do_edit(struct char_data* ch, const char* arg, int cmd);
void do_setskill(struct char_data* ch, const char* arg, int cmd);
void do_string(struct char_data* ch, const char* arg, int cmd);
char* one_word(const char* argument,  char* first_arg );
void page_string(struct descriptor_data* d, const char* str, int keep_internal);
void quad_arg(char* arg, int* type, const char* name, int* field, char* string);
void show_string(struct descriptor_data* d, const char* input );
void string_add(struct descriptor_data* d,  char* str);



#endif /* SRC_MODIFY_HPP_ */
