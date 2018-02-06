/* Alarmud
 * $Id: signals.h,v 1.1.1.1 2002/02/13 11:14:55 root Exp $
 * */
#ifndef __SIGNALS
#define __SIGNALS 1
#define LPUSH SetLine(__FILE__,__LINE__)
float AverageEqIndex(float toadd);
void SetLine(char* srcfile,int srcline);
void SetStatus( char* szStatus);
void SetStatus( char* szStatus, char* szString);
void SetStatus( char* szStatus, char* szString, void* pGeneric );
void PushStatus(const char* szStatus);
void PopStatus();
void buscrash(int dummy);
#endif
