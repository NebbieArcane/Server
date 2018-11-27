/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __SIGNALS_HPP
#define __SIGNALS_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#define LPUSH SetLine(__FILE__,__LINE__)
float AverageEqIndex(float toadd);
void SetLine(const char* srcfile,int srcline);
void SetStatus(const char* szStatus);
void SetStatus(const char* szStatus, const char* szString);
void SetStatus(const char* szStatus, const char* szString, void* pGeneric);
void signal_setup();
void PushStatus(const char* szStatus);
void PopStatus();
void buscrash(int dummy);
void checkpointing(int dummy);
void shutdown_request(int dummy);
void logsig(int dummy);
void hupsig(int dummy);
void diesig(int dummy);
void badcrash(int dummy);
void PrintStatus();
void PrintStatus(int level);
} // namespace Alarmud
#endif

