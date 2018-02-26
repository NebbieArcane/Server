/* Alarmud
 * $Id: signals.h,v 1.1.1.1 2002/02/13 11:14:55 root Exp $
 * */
#ifndef __SIGNALS
#define __SIGNALS 1
#include "config.hpp"
#define LPUSH SetLine(__FILE__,__LINE__)
float AverageEqIndex(float toadd);
void SetLine(const char* srcfile,int srcline);
void SetStatus( const char* szStatus);
void SetStatus( const char* szStatus, const char* szString);
void SetStatus( const char* szStatus, const char* szString, void* pGeneric );
void PushStatus(const char* szStatus);
void PopStatus();
void buscrash(int dummy);
void checkpointing( int dummy );
void shutdown_request( int dummy );
void logsig( int dummy );
void hupsig( int dummy );
void badcrash( int dummy );
void PrintStatus();
void PrintStatus(int level);
#endif
