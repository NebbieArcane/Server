/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: signals.c,v 1.2 2002/03/23 16:43:20 Thunder Exp $ */
/***************************  System  include ************************************/
#include <cerrno>
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
	mudlog(LOG_SYSERR, "Connections from start: %llu",
		   static_cast<unsigned long long>(HowManyConnection(0)));
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
	const long long next_ptr = static_cast<long long>(gnPtr) + 1LL;
	if(next_ptr < 0LL || next_ptr >= static_cast<long long>(STACK_SIZE)) {
		return;
	}
	gnPtr = static_cast<int>(next_ptr);
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

namespace {
bool install_handler(int signum, void (*handler)(int)) {
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; /* resta installato, niente signal() nel gestore */
	return sigaction(signum, &sa, nullptr) == 0;
}

/* Solo write(): sicura dentro un gestore di segnale. */
void signal_safe_write(const char* text) {
	if(text == nullptr) {
		return;
	}
	std::size_t len = 0;
	while(text[len] != '\0') {
		++len;
	}
	while(len > 0) {
		const ssize_t written = write(STDERR_FILENO, text, len);
		if(written <= 0) {
			return;
		}
		text += written;
		len -= static_cast<std::size_t>(written);
	}
}

void signal_safe_write_int(long value) {
	char digits[24];
	int pos = static_cast<int>(sizeof(digits)) - 1;
	digits[pos] = '\0';
	const bool negative = value < 0;
	unsigned long magnitude = negative ? 0UL - static_cast<unsigned long>(value)
								: static_cast<unsigned long>(value);
	do {
		digits[--pos] = static_cast<char>('0' + (magnitude % 10UL));
		magnitude /= 10UL;
	}
	while(magnitude > 0UL && pos > 1);
	if(negative) {
		digits[--pos] = '-';
	}
	signal_safe_write(&digits[pos]);
}
} // namespace

void signal_setup() {
	struct itimerval itime;
	struct timeval interval;

	install_handler(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	install_handler(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	install_handler(SIGINT, diesig);
	install_handler(SIGALRM, logsig);
	install_handler(SIGTERM, diesig);
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
	else if(!install_handler(SIGVTALRM, checkpointing)) {
		mudlog(LOG_ERROR,"%s:%s","Calling 'sigaction' in signal_setup",strerror(errno));
	}
}

/* I gestori girano in modo asincrono, anche nel mezzo di una malloc: dentro
 * si possono usare solo funzioni async-signal-safe (write, abort) e variabili
 * volatile sig_atomic_t. Il lavoro vero (log, save, reload, shutdown) lo fa il
 * game loop in process_pending_signals(). I gestori sono installati con
 * sigaction e restano installati: non serve reinstallarli dentro il gestore. */
static volatile std::sig_atomic_t g_checkpoint_pending = 0;
static volatile std::sig_atomic_t g_hup_pending = 0;
static volatile std::sig_atomic_t g_usr2_pending = 0;
static volatile std::sig_atomic_t g_die_pending = 0;
static volatile std::sig_atomic_t g_alarm_pending = 0;

void checkpointing(int dummy) {
	(void)dummy;
	const int saved_errno = errno;
	if(tics == 0) {
		/* Game loop bloccato: niente mudlog/PrintStatus, potrebbero bloccarsi
		 * sullo stesso lock. Stessi dati di PrintStatus, ma solo con write(). */
		signal_safe_write("CHECKPOINT shutdown: tics not updated\n");
		signal_safe_write("LastTrack ");
		signal_safe_write(currentfile);
		signal_safe_write(" at ");
		signal_safe_write_int(currentline);
		signal_safe_write("\nMud status: ");
		signal_safe_write(gszMudStatus);
		signal_safe_write("\nLast Name: ");
		signal_safe_write(gszName);
		signal_safe_write("\n");
		abort();
	}
	tics = 0;
	g_checkpoint_pending = 1;
	errno = saved_errno;
}

void hupsig(int dummy) {
	(void)dummy;
	g_hup_pending = 1;
}

void shutdown_request(int dummy) {
	(void)dummy;
	g_usr2_pending = 1;
}

/* kick out players etc */
void diesig(int dummy) {
	(void)dummy;
	g_die_pending = 1;
}

void logsig(int dummy) {
	(void)dummy;
	g_alarm_pending = 1;
}

void process_pending_signals() {
	if(g_checkpoint_pending != 0) {
		g_checkpoint_pending = 0;
		mudlog(LOG_SAVE, "CHECKPOINT: tics updated");
	}
	if(g_alarm_pending != 0) {
		g_alarm_pending = 0;
		mudlog(LOG_ALWAYS, "Signal SIGALARM received. Ignoring.");
	}
	if(g_hup_pending != 0) {
		g_hup_pending = 0;
		mudlog(LOG_ALWAYS, "Received SIGHUP");
		reload_files_and_scripts();
	}
	if(g_usr2_pending != 0) {
		g_usr2_pending = 0;
		mudlog(LOG_ALWAYS, "Received USR2 - shutdown request");
		mudshutdown = 1;
	}
	if(g_die_pending != 0) {
		g_die_pending = 0;
		mudlog(LOG_ALWAYS, "Received SIGINT, or SIGTERM. Shutting down");
		raw_force_all("return");
		raw_force_all("save");
		for(int i = 0; i < 30; i++) {
			SaveTheWorld();
		}
		mudshutdown = rebootgame = 1;
	}
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

