/*$Id: version.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/
#include "version.hpp"

#include <stdio.h>
#include <stdlib.h>
#include "release.hpp"
#include <string.h>
char* version() {
	return VERSION;
}

char* release() {
	return BUILD;
}

char* compilazione() {
	static char COMPILAZIONE[50];
	snprintf(COMPILAZIONE,49,"%s %s",__DATE__,__TIME__);
	return(COMPILAZIONE);
}

#ifdef STANDALONE
int main() {
	printf("%s %s %s \n\r",version(),release(),compilazione());
	return(0);
}
#endif


