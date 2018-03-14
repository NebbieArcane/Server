/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <stdio.h>
/***************************  General include ************************************/
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

