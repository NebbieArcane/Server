/*$Id: security.c,v 1.2 2002/02/13 12:30:59 root Exp $
*/

#include <stdio.h>
#include <string.h>

#include "snew.hpp"
#include "utility.hpp"
/* ATTENZIONE Controllare che corrispondano con i valori definiti in structs.h
   ed in utility.c */
//void mudlog( unsigned uType, char *szString, ... );
#define LOG_CHECK    0x0002

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
#ifdef CYGWIN
	hostname = strdup("localhost");
#endif
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

