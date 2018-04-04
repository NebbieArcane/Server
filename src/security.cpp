/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: security.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
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
#include "security.hpp"

namespace Alarmud {

int SecCheck(const char* arg, const char* site) {
	int  result=0;
	string fname("security/");
	string compare(site);
	fname.append(arg);
	std::ifstream securefile(arg);
	if (securefile.fail()) {
		mudlog( LOG_CHECK, "Unable to open security file for %s.", arg);
		return(-1);
	}
	fname.erase();
	while (securefile >> fname ) {
		if (fname==compare) {
			result=1;
			break;
		}
		else {
			mudlog( LOG_CHECK, "Checking site %s n for %s.",fname.c_str(),arg);

		}
	}
	securefile.close();
	if (result <1) {
		mudlog( LOG_SYSERR, "Site %s not allowed for %s. Security Violation.",site,arg);
	}
	return result;

}

} // namespace Alarmud

