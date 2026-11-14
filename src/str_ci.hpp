/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef STR_CI_HPP
#define STR_CI_HPP
/***************************  System  include ************************************/
#include <algorithm>
#include <cctype>
#include <string_view>
/***************************  Local    include ************************************/
namespace Alarmud {

/** Confronti di stringhe senza distinguere maiuscole e minuscole (ASCII). */
inline bool str_ci_char_equal(char a, char b) {
	return std::tolower(static_cast<unsigned char>(a)) ==
		   std::tolower(static_cast<unsigned char>(b));
}

/** a e b uguali a meno di maiuscole/minuscole. */
inline bool str_iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), str_ci_char_equal);
}

/** s comincia con prefix (maiuscole e minuscole uguali). */
inline bool str_istarts_with(std::string_view s, std::string_view prefix) {
	return s.size() >= prefix.size() &&
		   std::equal(prefix.begin(), prefix.end(), s.begin(), str_ci_char_equal);
}

} // namespace Alarmud
#endif // STR_CI_HPP
