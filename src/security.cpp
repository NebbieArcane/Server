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

int SecCheck(char* arg, char* site) {
	char buf[ 255 ];
	int  result=0;
	int doloop=0;
	FILE* f1;
	char* hostname;
	snprintf(buf,250,"security/%s",arg);
	buf[250]=0;
	if(!(f1 = fopen(buf, "rt"))) {
		mudlog( LOG_CHECK, "Unable to open security file for %s.", arg);
		return(-1);
	}
	result=0;
	while (1) {
		doloop=fscanf(f1,"%as\n",&hostname);
		mudlog( LOG_CHECK, "Checking %s against %s.",site,hostname);
		if( hostname && *hostname && !strncmp( site, hostname, strlen( hostname ) ) ) {
			result=1;
			free(hostname);
			break;
		}
		if (doloop < 1)
		{ break; }
		else
		{ free(hostname); }
	}
	fclose(f1);
	if (result <1)
		mudlog( LOG_CHECK, "Site %s not allowed for %s. Security Violation.",
				site,
				arg);

	return(result);
}

} // namespace Alarmud

