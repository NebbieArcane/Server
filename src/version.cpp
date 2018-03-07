/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: version.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "version.hpp"
#include "release.hpp"
namespace Alarmud {

const char* version() {
	return VERSION;
}

const char* release() {
	return BUILD;
}

const char* compilazione() {
	static char COMPILAZIONE[50];
	snprintf(COMPILAZIONE,49,"%s %s",__DATE__,__TIME__);
	return(COMPILAZIONE);
}



} // namespace Alarmud
#ifdef STANDALONE
int main() {
	printf("%s %s %s \n\r",Alarmud::version(),Alarmud::release(),Alarmud::compilazione());
	return(0);
}
#endif

