/*AlarMUD*/
/* $Id: mem_test.h,v 1.1.1.1 2002/02/13 11:14:55 root Exp $
 * */
/* definisco __SNEW per evitare eventuali inclusioni di snew.h da file
 * inclusi (tipo signals.h)
 * */
#define __SNEW 1
/* Definizione pseudofunzioni */
#ifndef __MEM_TEST
#define __MEM_TEST 1
#define malloc(size) ggmalloc(size,__FILE__,__LINE__)
#define realloc(ptr,size) ggrealloc(ptr,size,__FILE__,__LINE__)
#define free(ptr) ggfree(ptr,__FILE__,__LINE__)
#define cfree(ptr) ggcfree(ptr,__FILE__,__LINE__)
#define strdup(ptr) ggstrdup(ptr,__FILE__,__LINE__)
#define calloc(nelem,size) ggcalloc(nelem,size,__FILE__,__LINE__)
#define strtok(ptr,delim) ggstrtok(ptr,delim,__FILE__,__LINE__)
#define strcmp(ptr1,ptr2) ggstrcmp(ptr1,ptr2,__FILE__,__LINE__)
#define strncmp(ptr1,ptr2,n) ggstrncmp(ptr1,ptr2,n,__FILE__,__LINE__)
#define strcasecmp(ptr1,ptr2) ggstrcasecmp(ptr1,ptr2,__FILE__,__LINE__)
#define strncasecmp(ptr1,ptr2,n) ggstrncasecmp(ptr1,ptr2,n,__FILE__,__LINE__)
#define strcpy(s1,s2) ggstrcpy(s1,s2,__FILE__,__LINE__)
#define strncpy(s1,s2,n) ggstrncpy(s1,s2,n,__FILE__,__LINE__)
#define strcat(s1,s2) ggstrcat(s1,s2,__FILE__,__LINE__)
#define strncat(s1,s2,n) ggstrncat(s1,s2,n,__FILE__,__LINE__)

void* ggmalloc (unsigned int size, char* file_name, int line_number);
void* ggcalloc (unsigned int nelem, unsigned int size, char* file_name, int line_number);
void* ggrealloc (void* in_ptr, unsigned int size, char* file_name, int line_number);
void ggfree (void* ptr, const char* file_name, int line_number);
void ggcfree (void* ptr, const char* file_name, int line_number);
char* ggstrdup (char* s1, const char* file_name, int line_number);
char* ggstrtok (char* s,const char* delim, char* file_name, int line_number);
int ggstrcmp (const char* s1,const char* s2,char* file_name, int line_number) ;
int ggstrncmp (const char* s1,const char* s2,size_t n,char* file_name, int line_number) ;
int ggstrcasecmp (const char* s1,const char* s2,char* file_name, int line_number) ;
int ggstrncasecmp (const char* s1,const char* s2, size_t n,char* file_name, int line_number) ;
char* ggstrcpy(char* dest, const char* src,char* file_name, int line_number);
char* ggstrncpy(char* dest,  const char* src, size_t n,char* file_name,int line_number);
char* ggstrcat (char* dest, const char* src,char* file_name,int line_number);
char* ggstrncat (char* dest, const char* src,size_t n,char* file_name,int line_number);
#endif
