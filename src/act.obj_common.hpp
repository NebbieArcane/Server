/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _ACT_OBJ_COMMON_HPP
#define _ACT_OBJ_COMMON_HPP
#include <cstddef>
#include <string>
namespace Alarmud {

bool obj_arg_is_all(const char* s);
bool obj_ci_equal(const char* a, const char* b);
bool obj_prefix_equal(const char* arg, const char* prefix);
void obj_copy_cstr(char* dest, std::size_t destSize, const char* src);
int obj_parse_take_count(char* nameBuf, std::size_t nameSize, char* newargBuf);
void obj_replace_dup_cstr(char*& field, const std::string& value);
const char* obj_liquid_article_for(int drinkType);

} // namespace Alarmud
#endif // _ACT_OBJ_COMMON_HPP
