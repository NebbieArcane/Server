/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include <cstdlib>
#include <cstring>
#include <string>
#include "utility.hpp"

namespace Alarmud {

bool obj_arg_is_all(const char* s) {
	return s != nullptr && *s != '\0' && str_cmp(s, "all") == 0;
}

bool obj_ci_equal(const char* a, const char* b) {
	if(a == nullptr || b == nullptr) {
		return a == b;
	}
	return str_cmp(a, b) == 0;
}

bool obj_prefix_equal(const char* arg, const char* prefix) {
	return arg != nullptr && prefix != nullptr && str_cmp2(arg, prefix) == 0;
}

void obj_copy_cstr(char* dest, std::size_t destSize, const char* src) {
	if(dest == nullptr || destSize == 0) {
		return;
	}
	if(src == nullptr) {
		dest[0] = '\0';
		return;
	}
	std::strncpy(dest, src, destSize - 1);
	dest[destSize - 1] = '\0';
}

int obj_parse_take_count(char* nameBuf, std::size_t nameSize, char* newargBuf) {
	if(getall(nameBuf, newargBuf) == true) {
		obj_copy_cstr(nameBuf, nameSize, newargBuf);
		return -1;
	}
	const int bunch = getabunch(nameBuf, newargBuf);
	if(bunch != 0) {
		obj_copy_cstr(nameBuf, nameSize, newargBuf);
		return bunch;
	}
	return 1;
}

void obj_replace_dup_cstr(char*& field, const std::string& value) {
	if(field != nullptr) {
		free(field);
		field = nullptr;
	}
	if(!value.empty()) {
		field = strdup(value.c_str());
	}
}

const char* obj_liquid_article_for(int drinkType) {
	switch(drinkType) {
	case 0:
	case 7:
	case 14:
		return "dell'";
	case 1:
	case 3:
	case 4:
	case 6:
	case 8:
	case 15:
		return "della ";
	default:
		return "del ";
	}
}

} // namespace Alarmud
