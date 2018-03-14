
/*AlarMUD*/
/* $Id: mem_test.c,v 1.1.1.1 2002/02/13 11:14:54 root Exp $
 * */
/* definisco __SNEW per evitare eventuali inclusioni di snew.h da file
 * inclusi (tipo signals.h)
 * */
#define __SNEW 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include "signals.hpp"
#include "structs.hpp"
#include "utility.hpp"
void* ggcalloc (unsigned int nelem, unsigned int size, char* file_name, int line_number);
void* ggmalloc (unsigned int size, char* file_name, int line_number) {
	void* ptr=NULL;
	SetLine(file_name,line_number);
	ptr = ggcalloc (1,size,file_name,line_number);
	/*
	 *  SetLine(file_name,-line_number);
	 *  if (! ptr)
	 *    mudlog(LOG_ERROR,"Malloc failed in %s at %d",file_name,line_number);
	 *
	 * */
	return (ptr);

}

void* ggcalloc (unsigned int nelem, unsigned int size, char* file_name, int line_number) {
	void* ptr=NULL;
	SetLine(file_name,line_number);
	ptr = calloc (nelem, size);

	SetLine(file_name,-line_number);

	if (! ptr) {
		mudlog(LOG_ERROR,"Calloc failed in %s at %d",file_name,line_number);
	}
	fflush(NULL);
	return (ptr);

}

void* ggrealloc (void* in_ptr, unsigned int size, char* file_name, int line_number) {
	void* out_ptr=NULL;
	SetLine(file_name,line_number);
	out_ptr = realloc (in_ptr, size);
	SetLine(file_name,-line_number);

	if (! out_ptr) {
		mudlog(LOG_ERROR,"realloc failed in %s at %d",file_name,line_number);
	}
	return (out_ptr);

}

void ggfree (void* ptr, const char* file_name, int line_number) {
	SetLine(file_name,line_number);
	free (ptr);
	SetLine(file_name,-line_number);

}

void ggcfree (void* ptr, const char* file_name, int line_number) {
	SetLine(file_name,line_number);
#ifdef CYGWIN
	free (ptr);
#else
	cfree (ptr);
#endif
	SetLine(file_name,-line_number);

}

char* ggstrdup (char* s1, char* file_name, int line_number) {
	char* ptr=NULL;
	SetLine(file_name,line_number);

	ptr = (char*) strdup (s1);
	SetLine(file_name,-line_number);

	if (! ptr) {
		mudlog(LOG_ERROR,"strdup failed in %s at %d",file_name,line_number);
	}
	return (ptr);

}
char* ggstrtok (char* s,const char* delim, char* file_name, int line_number) {
	char* ptr=NULL;
	SetLine(file_name,line_number);
	ptr= (char*) strtok(s,delim);
	SetLine(file_name,-line_number);
	return (ptr);
}
int ggstrcmp (const char* s1,const char* s2,char* file_name, int line_number) {
	int res=0;
	SetLine(file_name,line_number);
	if (s1 && s2 )
	{ res=(strcmp(s1,s2)); }
	else if (s1)
	{ res=-1; }
	else
	{ res=1; }
	SetLine(file_name,-line_number);
	return(res);

}
int ggstrncmp (const char* s1,const char* s2,size_t n,char* file_name, int line_number) {
	int res=0;
	SetLine(file_name,line_number);
	if (s1 && s2 )
	{ res=(strncmp(s1,s2,n)); }
	else if (s1)
	{ res=-1; }
	else
	{ res=1; }
	SetLine(file_name,-line_number);
	return(res);
}
int ggstrcasecmp (const char* s1,const char* s2, char* file_name, int line_number) {
	int res=0;
	SetLine(file_name,line_number);
	if (s1 && s2 )
	{ res=(strcasecmp(s1,s2)); }
	else if (s1)
	{ res=-1; }
	else
	{ res=1; }
	SetLine(file_name,-line_number);
	return(res);

}
int ggstrncasecmp (const char* s1,const char* s2, size_t n,char* file_name, int line_number) {
	int res=0;
	SetLine(file_name,line_number);
	if (s1 && s2 )
	{ res=(strncasecmp(s1,s2,n)); }
	else if (s1)
	{ res=-1; }
	else
	{ res=1; }
	SetLine(file_name,-line_number);
	return(res);
}
char* ggstrcpy(char* dest,  const char* src,char* file_name, int line_number) {
	SetLine(file_name,line_number);
	char* ptr=NULL;
	if (dest) {

		if (src) {
			ptr=(char*)strcpy(dest,src);
		}
	}
	else
	{ mudlog(LOG_ERROR,"Dest NULLL in strcpy in %s at %d",file_name,line_number); }

	SetLine(file_name,-line_number);
	return (ptr);
}
char* ggstrncpy(char* dest,  const char* src, size_t n,char* file_name,int line_number) {
	SetLine(file_name,line_number);
	char* ptr=NULL;
	if (dest) {
		if (src)
		{ ptr=(char*) strncpy(dest,src,n); }
	}
	else {
		mudlog(LOG_ERROR,"Dest NULL in strncpy in %s at %d",file_name,line_number);
	}
	SetLine(file_name,-line_number);
	return (char*) ptr;
}
char* ggstrcat (char* dest, const char* src,char* file_name,int line_number) {
	SetLine(file_name,line_number);
	char* ptr=NULL;
	ptr=(char*)strcat(dest,src);
	return(ptr);
}
char* ggstrncat (char* dest, const char* src,size_t n,char* file_name,int line_number) {
	SetLine(file_name,line_number);
	char* ptr=NULL;
	ptr=(char*)strncat(dest,src,n);
	return(ptr);
}
