/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: signals.c,v 1.2 2002/03/23 16:43:20 Thunder Exp $ */
/***************************  System  include ************************************/
#include <csignal>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>
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
#include "signals.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "db.hpp"
namespace Alarmud {

/* La ridefinizione di funzioni di memoria qui causerebbe ricorsione
 * */
#define LOG_CRASH 0 // Alar, abbiamo gdb, meglio non modificare i crash
#define MAX_FNAME_LEN 32
#define STACK_SIZE 15
int gnPtr =-1;

char currentfile[MAX_FNAME_LEN+1]="";
int  currentline=0;
char gszMudStatus[ 150 ] = "";
char gszName[ 150 ] = "";
char gszStack[STACK_SIZE][150]= {
	"","","","","","","","","","","","","","",""
};


void* gpGeneric = NULL;




void PrintStatus() {
	PrintStatus(0);
}
void PrintStatus(int level) {
	int i=0;
	mudlog(LOG_SYSERR, "Connections from start: %d",
		   HowManyConnection(0));
	if(level==1) {
		mudlog(LOG_SYSERR,"CurrentTrack %s at %d",currentfile,currentline);
		mudlog(LOG_SYSERR, "Mud status: '%s'",gszMudStatus);
	}
	else {

		mudlog(LOG_SYSERR,"LastTrack %s at %d",currentfile,currentline);
		mudlog(LOG_SYSERR, "Mud status when crashed: '%s'",gszMudStatus);
	}
	mudlog(LOG_SYSERR, "  Last Name '%s'", gszName);
	if(gnPtr>=0) {
		mudlog(LOG_SYSERR,    " Calling Stack");
		for(i=0; i<=gnPtr; i++) {
			mudlog(LOG_SYSERR, "       %2d.%s",i,gszStack[i]);
		}
	}
}
void SetLine(const char* srcfile,int srcline) {
	int i;
	i=MIN(strlen(srcfile)+1,MAX_FNAME_LEN);
	memcpy(currentfile,srcfile,i);
	currentfile[i-1]=0;
	currentline=srcline;
}

void SetStatus(const char* szStatus, const char* szString, void* pGeneric) {
	int i;
	if(szStatus) {
		i=MIN(strlen(szStatus)+1,sizeof(gszMudStatus));
		memcpy(gszMudStatus, szStatus, i);
		gszMudStatus[ i - 1 ] = 0;
	}

	if(szString) {
		i=MIN(strlen(szString)+1,sizeof(gszName));
		memcpy(gszName, szString, i);
		gszName[ i - 1 ] = 0;
	}

	if(pGeneric) {
		gpGeneric = pGeneric;
	}
}
void SetStatus(const char* szStatus) {
	SetStatus(szStatus,NULL,NULL);
}
void SetStatus(const char* szStatus, const char* szString) {
	SetStatus(szStatus,szString,NULL);
}

void PushStatus(const char* szStatus, const char* szNome) {
	gnPtr++;
	if(gnPtr>=STACK_SIZE) {
		return;
	}
	if(gnPtr<0) {
		return;
	}
	snprintf(gszStack[gnPtr],149,"%s %s",
			 szStatus?szStatus:"",
			 szNome?szNome:"");
	gszStack[gnPtr][ 149 ] = 0;

	return;
}

void PushStatus(const char* szStatus) {
	PushStatus(szStatus, (char*) NULL);
	return;
}

void PopStatus() {
	if(gnPtr>=0) {
		gnPtr--;
	}

	return;
}

void signal_setup() {
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, diesig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, diesig);
#if LOG_CRASH
	signal(SIGSEGV, badcrash);
	signal(SIGBUS, buscrash);

#endif

	/* set up the deadlock-protection */

	interval.tv_sec = 300;    /* 5 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	if(setitimer(ITIMER_VIRTUAL, &itime, 0) < 0) {
		mudlog(LOG_ERROR,"%s:%s","Setting Virtual timer in signal_setup",strerror(errno));
	}
	else if(signal(SIGVTALRM, checkpointing) == SIG_ERR) {
		mudlog(LOG_ERROR,"%s:%s","Calling 'signal' in signal_setup",strerror(errno));
	}
}

void checkpointing(int dummy) {

	if(!tics) {
		mudlog(LOG_SYSERR, "CHECKPOINT shutdown: tics not updated");
		PrintStatus();

		abort();
	}
	else {
		mudlog(LOG_SAVE, "CHECKPOINT: tics updated");
		tics = 0;
	}
	if(signal(SIGVTALRM, checkpointing) == SIG_ERR) {
		mudlog(LOG_ERROR,"%s:%s","Calling 'signal' in checkpointing",strerror(errno));
		abort();
	}

}
void hupsig(int dummy) {
	mudlog(LOG_ALWAYS, "Received SIGHUP %d",dummy);
	reload_files_and_scripts();
}
void shutdown_request(int dummy) {

	mudlog(LOG_ALWAYS, "Received USR2 - shutdown request %d",dummy);
	mudshutdown = 1;
}


/* kick out players etc */
void diesig(int dummy) {

	mudlog(LOG_ALWAYS, "Received SIGINT, or SIGTERM. Shutting down %d",dummy);

	raw_force_all("return");
	raw_force_all("save");
	for(int i=0; i<30; i++) {
		SaveTheWorld();
	}
	mudshutdown = rebootgame = 1;
}

void logsig(int dummy) {
	mudlog(LOG_ALWAYS, "Signal SIGALARM received. Ignoring. %d",dummy);
	signal(SIGALRM, logsig);
}

#if LOG_CRASH
void badcrash(int dummy) {
	static int graceful_tried = 0;
	struct descriptor_data* desc;

	mudlog(LOG_CHECK,
		   "SIGSEGV received. Trying to shut down gracefully.");

	PrintStatus();

	if(!graceful_tried) {
#if 0
		close(mother_desc);
#endif
		mudlog(LOG_CHECK, "Trying to close all sockets.");
		graceful_tried = 1;
		for(desc = descriptor_list; desc; desc = desc->next) {
			close(desc->descriptor);
		}
	}
	abort();
}
void buscrash(int dummy) {
	static int graceful_tried = 0;
	struct descriptor_data* desc;

	mudlog(LOG_CHECK,
		   "SIGBUS received. Trying to shut down gracefully.");

	PrintStatus();

	if(!graceful_tried) {
#if 0
		close(mother_desc);
#endif
		mudlog(LOG_CHECK, "Trying to close all sockets.");
		graceful_tried = 1;
		for(desc = descriptor_list; desc; desc = desc->next) {
			close(desc->descriptor);
		}
	}
	abort();
}
#endif
float AverageEqIndex(float toadd) {
	static float curmedia=0.0;
	static float numerocasi=0.0;
	float modifier;
	if(toadd<100) {
		return(curmedia);
	}
	modifier=numerocasi;
	numerocasi++;
	modifier=modifier /numerocasi;
	curmedia*=modifier;
	curmedia+=toadd/numerocasi;
	return(curmedia);
}
} // namespace Alarmud

