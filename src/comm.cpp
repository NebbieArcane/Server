/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* $Id: comm.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
*** AlarMUD        comm.c main communication routines. Based on DIKU and
***                       SillyMUD.
*/
/***************************  System  include ************************************/
#include <cerrno>
#include <cstdio>
#include <cctype>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <sys/time.h>
#include <fcntl.h>
#include <csignal>
#include <sys/resource.h>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <chrono>
#include <thread>
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
#include "auction.hpp"
#include "act.info.hpp"
#include "act.other.hpp"
#include "act.wizard.hpp"
#include "ansi_parser.hpp"
#include "create.hpp"
#include "create.mob.hpp"
#include "create.obj.hpp"
#include "db.hpp"
#include "events.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "mobact.hpp"
#include "modify.hpp"
#include "signals.hpp"
#include "skills.hpp"
#include "snew.hpp"
#include "spell_parser.hpp"
#include "vt100c.hpp"
#include "weather.hpp"
#include "comm.hpp"

namespace Alarmud {
using std::chrono::steady_clock;
using std::chrono::microseconds;
using std::chrono::time_point;
using std::chrono::duration_cast;

#define PIDFILE "myst.pid"
#define MAXIDLESTARTTIME 1000
#define MAX_CONNECTS 1024  /* max number of descriptors (connections) */
/* THIS IS SYSTEM DEPENDANT, use 64 is not sure! */


#define MUDUSER "root"
#define DFLT_PORT 4000        /* default port */
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256

#define STATE(d) ((d)->connected)

unsigned long pulse;
struct descriptor_data* descriptor_list, *next_to_process;
struct txt_block* bufpool = 0;  /* pool of large output buffers */
int     buf_largecount;         /* # of large buffers which exist */
int     buf_overflows;          /* # of overflows of output */
int     buf_switches;           /* # of switches from small to large buf */

/* int slow_nameserver = FALSE; */

int slow_death = 0;     /* Shut her down, Martha, she's sucking mud */
int mudshutdown = 0;       /* clean shutdown */
int rebootgame = 0;         /* reboot the game after a shutdown */
bool no_specials = false;    /* Suppress ass. of special routines */
long Uptime;            /* time that the game has been up */

int maxdesc, avail_descs;
int tics = 0;        /* for extern checkpointing */
int PORT=0;

uint64_t aTimeCheck[ PULSE_MOBILE ];
uint16_t NumTimeCheck = sizeof(aTimeCheck); /* dovrebbe essere il piu` grande dei PULSE */


struct affected_type*  Check_hjp, *Check_old_af;
struct char_data* Check_c;
char* Check_p = NULL;


void str2ansi(char* p2, const char* p1, int start, int stop) {
	int i,j;

	if((start > stop) || (start < 0)) {
		p2[0] = '\0';    /* null terminate string */
	}
	else {
		if(start == stop) {        /* will copy only 1 char at pos=start */
			p2[ 0 ] = p1[ start ];
			p2[ 1 ] = '\0';
		}
		else {
			j = 0;

			/* start or (start-1) depends on start index */
			/* if starting index for arrays is 0 then use start */
			/* if starting index for arrays is 1 then use start-1 */

			for(i = start; i <= stop; i++) {
				p2[ j++ ] = p1[ i ];
			}
			p2[j] = '\0';    /* null terminate the string */
		}
	}
}

char* ParseAnsiColors(int UsingAnsi, const char* txt) {
	static char buf [MAX_STRING_LENGTH ] = "";
	char tmp[20];

	int f = 0;

	buf[ 0 ] = 0;
	for(int l=0; *txt;) {
		if(*txt == '\\' && *(txt + 1) == '$') {
			txt += 2;
			buf[ l++ ] = '$';
		}
		else if(*txt == '$' && (toupper(*(txt + 1)) == 'C' ||
								(*(txt + 1) == '$' &&
								 toupper(*(txt + 2)) == 'C'))) {
			if(*(txt + 1) == '$') {
				txt += 3;
			}
			else {
				txt += 2;
			}
			str2ansi(tmp, txt, 0, 3);

			/* if using ANSI */
			if(UsingAnsi) {
				strcat(buf, ansi_parse(tmp));
			}
			else
				/* if not using ANSI   */
			{
				strcat(buf, "");
			}

			txt += 4;
			l = strlen(buf);
			f++;
		}
		else {
			buf[ l++ ] = *txt++;
		}
		buf[ l ] = 0;
	}
	if(f && UsingAnsi) {
		strcat(buf, ansi_parse("0007"));
	}

	return buf;
}


/* *********************************************************************
*  main game loop and related stuff                                       *
********************************************************************* */

//int __main ()
//{
//  return(1);
//}

/* jdb code - added to try to handle all the different ways the connections
   can die, and try to keep these 'invalid' sockets from getting to select
*/

void close_socket_fd(int desc) {
	struct descriptor_data* d;

#if defined( LOG_DEBUG )
	mudlog(LOG_CHECK, "begin close_socket_fd");
#endif

	for(d = descriptor_list; d; d=d->next) {
		if(d->descriptor == desc) {
			close_socket(d);
		}
	}

#if defined( LOG_DEBUG )
	mudlog(LOG_CHECK, "end close_socket_fd");
#endif
}

int run(int port, const char* dir) {
#ifdef env_release
	boost::format fmt("Release: Starting %s rel %s ");
#endif
#ifdef env_master
	boost::format fmt("Master: Starting %s rel %s ");
#endif
#ifdef env_devel
	boost::format fmt("Devel: Starting %s rel %s ");
#endif
#ifdef env_vagrant
	boost::format fmt("Vagrant: Starting %s rel %s ");
#endif
	fmt % version() % release() ;
	mudlog(LOG_ALWAYS, fmt.str().c_str());
#ifndef env_vagrant
	slackNotify(fmt.str().c_str()  ,":european_castle:");
#endif

	mudlog(LOG_ALWAYS, "Compiled on %s",compilazione());

	mudlog(LOG_ALWAYS,"Pulse:%-10s%-10s%-10s%-10s%-10s%-10s","zone","river","teleport","violence","mobile","tick");
	mudlog(LOG_ALWAYS,"Value:%10d%10d%10d%10d%10d%10d",
		   PULSE_ZONE, PULSE_RIVER,PULSE_TELEPORT,
		   PULSE_VIOLENCE,PULSE_MOBILE,(PULSE_PER_SEC * SECS_PER_MUD_HOUR));
	mudlog(LOG_ALWAYS,"Reading data from ./%s",dir);
	mudlog(LOG_ALWAYS,"Test mode: %s",(IsTest()?"ON":"off"));
	printFlags();
	Uptime = time(0);

	mudlog(LOG_ALWAYS, "Running game on port %d.", port);
	mudlog(LOG_ALWAYS, "Using %s as data directory.", dir);
	mudlog(LOG_ALWAYS, "Pid: %d",getpid());
	mudlog(LOG_ALWAYS,"Host: %s",HostName());
	/* scrive il proprio pid in 'dir' */
	FILE* fd;
	fd=fopen(PIDFILE,"w");
	if(fd) {
		fprintf(fd,"%d",getpid());
		fclose(fd);
	}

	srandom(0);
	WizLock = FALSE;

	run_the_game(port);
	return(0);
}


/* Init sockets, run game, and cleanup sockets */
void run_the_game(int port) {
	int s;
	descriptor_list = NULL;
	mudlog(LOG_CHECK, "Opening mother connection.");
	s = init_socket(port);
	mudlog(LOG_CHECK, "Signal trapping.");
	signal_setup();
	event_init();
	boot_db();
	LOG_FATAL("Verbosity 1: LSYSERR LSERVICE error level enabled");
	LOG_ALERT("Verbosity 2: LERROR LCONNECT error level also enabled");
	LOG_WARN("Verbosity 3: LCHECK error level also enabled");
	LOG_INFO("Verbosity 4: LPLAYERS LMOBILES error level also enabled");
	LOG_TRACE("Verbosity 5: LSAVE,LMAIL,LRANK error level also enabled");
	LOG_DBG("Verbosity 6: LWHO error level also enabled");
	PORT=port;
	mudlog(LOG_ALWAYS, "Entering game loop on port %d",PORT);
	game_loop(s);
	close_sockets(s);
	if(rebootgame) {
		mudlog(LOG_ALWAYS, "Rebooting.");
	}
	mudlog(LOG_ALWAYS, "Normal termination of game.");
}

/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void game_loop(int s) {
	fd_set input_set, output_set, exc_set;

	char comm[MAX_INPUT_LENGTH];
	char promptbuf[255];
	struct descriptor_data* point, *next_point;
	for(uint i = 0; i < sizeof(aTimeCheck)/sizeof(aTimeCheck[0]); i++) {
		aTimeCheck[ i ] = OPT_USEC;
	}

	maxdesc = s;


	/* !! Change if more needed !! */
	avail_descs = getdtablesize() - 2; /* never used, pointless? */
	std::chrono::time_point<steady_clock,steady_clock::duration> next_tick=steady_clock::now();
	/* Main loop */
	while(!mudshutdown) {
		next_tick+= microseconds(OPT_USEC); // In caso di lag, il tick successivo avviene prima
		FD_ZERO(&input_set);
		FD_ZERO(&output_set);
		FD_ZERO(&exc_set);

		FD_SET(s, &input_set);
		/* Attivo il selettore della mother connection */
		for(point = descriptor_list; point; point = point->next)
			/* Attivo i descrittori per tutti i player connessi */
		{
			FD_SET(point->descriptor, &input_set);
			FD_SET(point->descriptor, &exc_set);
			FD_SET(point->descriptor, &output_set);

			if(maxdesc < point->descriptor) {
				maxdesc = point->descriptor;
			}
			/* Mi porto in maxdesc il numero piu' alto di descrittore */
		}


		/**
		 * Select called with null time do not block
		 */
		static timeval null_time({0,0});
		if(select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time)
				< 0) {
			mudlog(LOG_ERROR,"Select poll: %s",strerror(errno));
			/* one of the descriptors is broken... */
			for(point = descriptor_list; point; point = next_point) {
				next_point = point->next;
				write_to_descriptor(point->descriptor, "\n\r");
			}
		}
		/**
		 * Changed timing out selct with thread_sleep as a first step to a multithuread version
		 * We could at least move the non playing sockets to their own thread
		 */
		do {
			std::this_thread::sleep_until(next_tick);
		}
		while(next_tick > steady_clock::now());


		/* Respond to whatever might be happening */
		/* New connection? */
		if(FD_ISSET(s, &input_set)) {
			mudlog(LOG_CONNECT, "New connection started");
			if(new_descriptor(s) < 0) {
				mudlog(LOG_CONNECT,"Error on select:%s",strerror(errno));
			}
			mudlog(LOG_CONNECT, "Connection stabilited");
		}

		/* kick out the freaky folks */
		/* Se ci sono eccezioni sul descrittore lo chiude */
		for(point = descriptor_list; point; point = next_point) {
			next_point = point->next;
			if(FD_ISSET(point->descriptor, &exc_set)) {
				FD_CLR(point->descriptor, &input_set);
				FD_CLR(point->descriptor, &output_set);
				close_socket(point);
			}
		}
		// Reading from ready descriptors
		for(point = descriptor_list; point; point = next_point) {
			next_point = point->next;
			if(FD_ISSET(point->descriptor, &input_set))
				if(process_input(point) < 0) {
					close_socket(point);
				}

		}

		/* process_commands; */
		for(point = descriptor_list; point; point = next_to_process) {
			next_to_process = point->next;
			if(point->connected == CON_PLYNG) {
				/* aggiorno il contatore delle posizioni
				 * */
				GET_TEMPO_IN(point->character,GET_POS(point->character))++;
			}
			else {
				if((point->wait<-1) && (abs(point->wait) > abs(MAXIDLESTARTTIME)) && (point->connected !=CON_PLYNG)) {
					// Was not doing anything useful, probably waiting at initial prompt
					mudlog(LOG_CHECK,"Fried dummy connection from [HOST:%s]",point->host);
					write_to_descriptor(point->descriptor,"Timeout!\n\r");
					close_socket(point);
				}
			}
			if((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
				if(point->character && point->connected == CON_PLYNG && point->character->specials.was_in_room != NOWHERE) {
					point->character->specials.was_in_room = NOWHERE;
					act("$n e` rientrat$b.", TRUE, point->character, 0, 0, TO_ROOM);
				}
				point->wait = 1;
				if(point->character) {
					point->character->specials.timer = 0;
				}
				point->prompt_mode = 1;
				if(point->str) {
					string_add(point, comm);
				}
				else if(point->connected == CON_PLYNG && !point->showstr_point) {
					command_interpreter(point->character, comm);
				}
				else if(point->showstr_point) {
					show_string(point, comm);
				}
				else if(point->connected == CON_EDITING) {
					RoomEdit(point->character,comm);
				}
				else if(point->connected == CON_OBJ_EDITING) {
					ObjEdit(point->character,comm);
				}
				else if(point->connected == CON_MOB_EDITING) {
					MobEdit(point->character,comm);
				}
				else if(point->connected == CON_OBJ_FORGING) {
					ForgeString(point->character,comm,0);
				}
				else {
					nanny(point, comm);
				}
			}
		}

		/* either they are out of the game */
		/* or they want a prompt.          */
		for(point = descriptor_list; point; point = next_point) {
			next_point = point->next;

#if not BLOCK_WRITE
			if(FD_ISSET(point->descriptor, &output_set) && point->output.head)
#else
			if(FD_ISSET(point->descriptor, &output_set) && *(point->output)) {
#endif
				if(process_output(point) < 0) {
					close_socket(point);
				}

				else {
					point->prompt_mode = 1;
				}
		}
	}

	/* give the people some prompts  */
	for(point = descriptor_list; point; point = point->next) {
		if(point->prompt_mode) {
			if(point->str) {
				write_to_descriptor(point->descriptor, "> ");
			}
			else if(point->showstr_point)
				write_to_descriptor(point->descriptor,
									point->connected == CON_PLYNG ?
									"[Batti INVIO per continuare/Q per uscire] " :
									"[Batti INVIO] ");
			else if(point->connected == CON_PLYNG) {
				if(point->character->term == VT100) {
					struct char_data* ch;
					int update = 0;

					ch = point->character;

					if(GET_MOVE(ch) != ch->last.move) {
						SET_BIT(update, INFO_MOVE);
						ch->last.move = GET_MOVE(ch);
					}
					if(GET_MAX_MOVE(ch) != ch->last.mmove) {
						SET_BIT(update, INFO_MOVE);
						ch->last.mmove = GET_MAX_MOVE(ch);
					}
					if(GET_HIT(ch) != ch->last.hit) {
						SET_BIT(update, INFO_HP);
						ch->last.hit = GET_HIT(ch);
					}
					if(GET_MAX_HIT(ch) != ch->last.mhit) {
						SET_BIT(update, INFO_HP);
						ch->last.mhit = GET_MAX_HIT(ch);
					}
					if(GET_MANA(ch) != ch->last.mana) {
						SET_BIT(update, INFO_MANA);
						ch->last.mana = GET_MANA(ch);
					}
					if(GET_MAX_MANA(ch) != ch->last.mmana) {
						SET_BIT(update, INFO_MANA);
						ch->last.mmana = GET_MAX_MANA(ch);
					}
					if(GET_GOLD(ch) != ch->last.gold) {
						SET_BIT(update, INFO_GOLD);
						ch->last.gold = GET_GOLD(ch);
					}
					if(GET_EXP(ch) != ch->last.exp) {
						SET_BIT(update, INFO_EXP);
						ch->last.exp = GET_EXP(ch);
					}
					if(update) {
						UpdateScreen(ch, update);
					}
					sprintf(promptbuf,"> ");
				}
				else {
					construct_prompt(promptbuf,point->character);
				}
				write_to_descriptor(point->descriptor,
									ParseAnsiColors(
										IS_SET(point->character->player.user_flags,USE_ANSI),
										promptbuf));
			}
			point->prompt_mode = 0;
		}
	}

	/* handle heartbeat stuff */
	/* Note: pulse now changes every 1/4 sec  */

	pulse++;
	event_process();

	if(!(pulse % PULSE_ZONE)) {
		zone_update();
		check_reboot();
	}

	if(!(pulse % PULSE_MAXUSAGE)) {
		update_max_usage();
	}

	if(!(pulse % PULSE_RIVER)) {
		RiverPulseStuff(pulse);
	}

	if(!(pulse % PULSE_TELEPORT)) {
		TeleportPulseStuff(pulse);
	}

	if(!(pulse % PULSE_VIOLENCE)) {
		check_mobile_activity(pulse);
		perform_violence(pulse);
	}
#if ENABLE_AUCTION
	if(!(pulse % (PULSE_AUCTION))) {
		auction_update();
	}
#endif
	if(!(pulse % (SECS_PER_MUD_HOUR*PULSE_PER_SEC))) {
		weather_and_time(1);
		affect_update(pulse);  /* things have been sped up by combining */
		if(time_info.hours == 1) {
			update_time();
		}
	}

	if(!(pulse % PULSE_EQ)) {  /* ogni 5 minuti registra il valore dell'eq */
		buglog(LOG_CHECK,"Valore medio eq in gioco: %f",AverageEqIndex(-1));
	}

	tics++;        /* tics since last checkpoint signal */
	/**
	 * Changed timing out select with thread_sleep as a first step to a multithuread version
	 * We could at least move the non playing sockets to their own thread
	 * Also moved to the end of the loop
	 */
	microseconds lag(1);
	do {
		std::this_thread::sleep_until(next_tick);
		lag=duration_cast<microseconds>(steady_clock::now()-next_tick);
	}
	while(lag.count()<0);
	/* check out the time */
	aTimeCheck[ NumTimeCheck % sizeof(aTimeCheck)] = lag.count()+OPT_USEC;
	NumTimeCheck++;


} /* main loop ebd */
}

/* ******************************************************************
 * general utility stuff for local use                              *
 ****************************************************************** */


int get_from_q(struct txt_q* queue, char* dest) {
	struct txt_block* tmp;

	/* Q empty? */
	if(!queue) {
		return(0);
	}
	if(!queue->head) {
		return(0);
	}

	if(!dest) {
		mudlog(LOG_SYSERR, "Sending message to null destination.");
		return(0);
	}

	tmp = queue->head;

	if(dest && queue->head->text) {
		strcpy(dest, queue->head->text);
	}

	queue->head = queue->head->next;

	free(tmp->text);

	free(tmp);

	return(1);
}



void write_to_q(char* txt, struct txt_q* queue) {
	struct txt_block* pNew;
	char tbuf[256];
	int strl;

	if(!queue) {
		mudlog(LOG_ERROR, "Output message to non-existant queue");
		return;
	}

	CREATE(pNew, struct txt_block, 1);
	strl = strlen(txt);
	if(strl < 0 || strl > 45000) {
		mudlog(LOG_ERROR,
			   "strlen returned bogus length in write_to_q, string was:");
		for(strl = 0; strl < 120; strl++) {
			tbuf[ strl ] = txt[ strl ];
		}
		tbuf[ strl ] = 0;

		mudlog(LOG_CHECK, tbuf);

		free(pNew);
		return;
	}
	pNew->text = (char*)strdup(txt);
	pNew->next = NULL;

	/* Q empty? */
	if(!queue->head) {
		queue->head = queue->tail = pNew;
	}
	else {
		queue->tail->next = pNew;
		queue->tail = pNew;
	}
}


#if BLOCK_WRITE
void write_to_output(const char* txt, struct descriptor_data* t) {
	unsigned int size;
	static char tmpoutbuf[128];
	size = strlen(txt);

	/* if we're in the overflow state already, ignore this */
	if(t->bufptr < 0 || !txt) {
		return;
	}
	/* if we have enough space, just write to buffer and that's it! */
	if(t->bufspace >= size) {
		snprintf(tmpoutbuf,127,"wto: output=%ld bufptr=%ld size=%d/%d txt=%s",
				 (long)t->output,(long)t->bufptr,size,SMALL_BUFSIZE,txt);
		tmpoutbuf[127]=0;
		strcat(t->output, txt);

		t->bufspace -= size;
		t->bufptr = strlen(t->output);

	}
	else {
		/* otherwise, try to switch to a large buffer */
		if(t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
			/* we`re already using large buffer, or even the large buffer
			 * in`t big enough -- switch to overflow state */
			t->bufptr = -1;
			buf_overflows++;
			mudlog(LOG_ERROR, "over flow stat in write_to_output, comm.c");
			return;
		}

		buf_switches++;
		/* if the pool has a buffer in it, grab it */
		if(bufpool) {
			t->large_outbuf = bufpool;
			bufpool = bufpool->next;
		}
		else {
			/* else create one */
			CREATE(t->large_outbuf, struct txt_block, 1);
			CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
			buf_largecount++;
		}

		strcpy(t->large_outbuf->text, t->output);
		t->output = t->large_outbuf->text;
		strcat(t->output, txt);
		t->bufspace = LARGE_BUFSIZE-1 - strlen(t->output);
		t->bufptr = strlen(t->output);
	}
}
#endif

/**
 * Returns a - b as timeval
 */
struct timeval timediff(struct timeval* a, struct timeval* b) {
	struct timeval rslt;

	rslt.tv_usec=a->tv_usec;
	rslt.tv_sec=a->tv_sec;
	if(rslt.tv_usec > b->tv_usec) {
		rslt.tv_usec -= b->tv_usec;
	}
	else {
		rslt.tv_usec = rslt.tv_usec + 1000000 - b->tv_usec;
		if(rslt.tv_sec>0) {
			--rslt.tv_sec;
		}
		else {
			rslt.tv_usec = 0;
			rslt.tv_sec =0;
		}
	}
	if(rslt.tv_sec >=b->tv_sec) {
		rslt.tv_sec -=b->tv_sec;
	}
	else {
		rslt.tv_usec = 0;
		rslt.tv_sec =0;
	}
	return rslt;
}





#if not BLOCK_WRITE
/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data* d) {
	char dummy[MAX_STRING_LENGTH];

	while(get_from_q(&d->output, dummy));

	while(get_from_q(&d->input, dummy));

}
#else
void flush_queues(struct descriptor_data* d) {
	char buf2[MAX_STRING_LENGTH];

	if(d->large_outbuf) {

		d->large_outbuf->next = bufpool;

		bufpool = d->large_outbuf;

	}

	while(get_from_q(&d->input, buf2))
		;

}
#endif






/* ******************************************************************
 *  socket handling                                                 *
 ****************************************************************** */



int init_socket(int port) {
	int s;
	int bReuse = 1;
	char hostname[MAX_HOSTNAME+1];
	struct sockaddr_in sa;
	struct hostent* hp;

#if !defined(CYGWIN)
	bzero(&sa, sizeof(struct sockaddr_in));
	gethostname(hostname, MAX_HOSTNAME);

	hp = gethostbyname(hostname);
	if(hp == NULL) {
		mudlog(LOG_ERROR,"%s:%s","gethostbyname",strerror(errno));
		exit(1);
	}
#endif

	sa.sin_family      = AF_INET; /* hp->h_addrtype; */
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port        = htons(port);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0) {
		mudlog(LOG_ERROR,"%s:%s","Init-socket",strerror(errno));
		exit(1);
	}
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &bReuse, sizeof(bReuse)) < 0) {
		mudlog(LOG_ERROR,"%s:%s","setsockopt REUSEADDR",strerror(errno));
		exit(1);
	}


#if NETBSD
	if(bind(s, (struct sockaddr*) &sa, sizeof(sa)) < 0)
#else
	if(bind(s, &sa, sizeof(sa), 0) < 0)
#endif
	{
		mudlog(LOG_ERROR,"%s:%s","bind",strerror(errno));
		close(s);
		exit(1);
	}

	listen(s, 5);
	return(s);
}



int new_connection(int s) {
	struct sockaddr_in isa;
#ifdef sun
	struct sockaddr peer;
	char buf[ 100 ];
#endif
#ifdef CYGWIN
	int i;
#else
	unsigned  int i;
#endif
	int t;

	i = sizeof(isa);
#if 0
	getsockname(s, &isa, &i);
#endif

	nonblock(s);

	if((t = accept(s, (struct sockaddr*)&isa, &i)) < 0) {
		mudlog(LOG_ERROR,"%s:%s","Accept",strerror(errno));
		return(-1);
	}
	nonblock(t);

#ifdef sun

	i = sizeof(peer);
	if(!getpeername(t, &peer, &i)) {
		*(peer.sa_data + 49) = '\0';
		mudlog(LOG_CONNECT, "New connection from addr [HOSTREV:%s].", peer.sa_data);
	}

#endif

	return(t);
}

int new_descriptor(int s) {

	int desc;
#ifdef CYGWIN
	unsigned int i;
	int size;
#else
	unsigned int size,i;
#endif
	struct hostent* from;
	struct sockaddr_in sock;
	char buf[200];

	if((desc = new_connection(s)) < 0) {
		return (-1);
	}


	if((desc + 1) >= MAX_CONNECTS) {
		sprintf(buf,"Mi dispiace... Il gioco e` pieno (# giocatori %d). "
				"Riprova piu` tardi.\n\r", desc);
		write_to_descriptor(desc,buf);
		close(desc);

		for(struct descriptor_data* d = descriptor_list; d; d = d->next) {
			if(!d->character) {
				close_socket(d);
			}

		}
		return(0);
	}
	else {
		if(desc > maxdesc) {
			maxdesc = desc;
		}
	}

	struct descriptor_data* newd = new descriptor_data();
	//CREATE(newd, struct descriptor_data, 1);

	if(!newd) {
		sprintf(buf,"Mi dispiace... Non posso lasciarti entrare adesso. "
				"Riprova piu` tardi.\n\r");
		write_to_descriptor(desc,buf);
		close(desc);
		for(struct descriptor_data* d = descriptor_list; d; d = d->next) {
			if(!d->character) {
				close_socket(d);
			}

		}
		return(0);
	}


	/* find info */
	size = sizeof(sock);

	if(getpeername(desc, (struct sockaddr*) & sock, &size) < 0) {
		mudlog(LOG_ERROR,"%s:%s","getpeername",strerror(errno));
		*newd->host = '\0';
	}
	else if(IS_SET(SystemFlags,SYS_SKIPDNS) ||
			!(from = gethostbyaddr((char*)&sock.sin_addr,
								   sizeof(sock.sin_addr), AF_INET))) {
		i = sock.sin_addr.s_addr;
		sprintf(newd->host, "%d.%d.%d.%d", (i & 0xFF000000) >>24,
				(i & 0x00FF0000) >> 16, (i & 0x0000FF00) >> 8,
				(i & 0x000000FF));
	}
	else {
		strncpy(newd->host, from->h_name, 49);
		*(newd->host + 49) = '\0';
	}

#if 0
	if(isbanned(newd->host) == BAN_ALL) {
		close(desc);
		sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
		log(buf2);
		free(newd);
		return(0);
	}
#endif


	/* end newer code */

	/* init desc data */
	newd->descriptor = desc;
	newd->connected  = CON_NME;
	newd->wait = -1;
	newd->prompt_mode = 0;
	*newd->buf = '\0';
	newd->str =  nullptr;
	newd->showstr_head =  nullptr;
	newd->showstr_point = nullptr;
	*newd->last_input= '\0';
	mudlog(LOG_CONNECT, "New connection from addr [HOST:%s]: %d: %d (wait: %d)", newd->host,desc, maxdesc,newd->wait);
#if not BLOCK_WRITE
	newd->output.head = NULL;
#else
	newd->output=newd->small_outbuf;
	*(newd->output)='\0';
	newd->bufspace=SMALL_BUFSIZE-1;
	newd->large_outbuf= NULL;
#endif
	newd->input.head = NULL;
	newd->next = descriptor_list;
	newd->character =  NULL;
	newd->original =  NULL;
	newd->snoop.snooping =  NULL;
	newd->snoop.snoop_by =  NULL;

	/* prepend to list */

	descriptor_list = newd;

	SEND_TO_Q(login, newd);
	SEND_TO_Q(
		ParseAnsiColors(TRUE,"$c0007"
						"$c0011Inserisci l'$c0012email del tuo account su http://www.nebbiearcane.it$c0011 o il $c0004nome$c0011 di un personaggio.\r\n"
						"Se non hai o non ricordi il codice di accesso (diverso dalla password del sito) vai su \r\nhttp://www.nebbiearcane.it/mudcode\r\n e crealo.\r\n"
						"$c0007Email o nome del personaggio su Nebbie Arcane? "),
		newd);

	return(0);
}



#if not BLOCK_WRITE
int process_output(struct descriptor_data* t) {
	char i[MAX_STRING_LENGTH + MAX_STRING_LENGTH];

	if(!t->prompt_mode && !t->connected)
		if(write_to_descriptor(t->descriptor, "\n\r") < 0) {
			return(-1);
		}


	/* Cycle thru output queue */
	while(get_from_q(&t->output, i))        {
		if((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
			SEND_TO_Q("% ",t->snoop.snoop_by->desc);
			SEND_TO_Q(i,t->snoop.snoop_by->desc);
		}
		if(write_to_descriptor(t->descriptor, i)) {
			return(-1);
		}
	}

	if(!t->connected && !(t->character && !IS_NPC(t->character) &&
						  IS_SET(t->character->specials.act, PLR_COMPACT)))
		if(write_to_descriptor(t->descriptor, "\n\r") < 0) {
			return(-1);
		}

	return(1);
}
#else

/* SEARCH HERE */
int process_output(struct descriptor_data* t) {
	static char i[LARGE_BUFSIZE + 20];

	/* start writing at the 2nd space so we can prepend "% " for snoop */
	if(!t->prompt_mode && !t->connected) {
		strcpy(i+2, "\n\r");
		strcat(i+2, t->output);
	}
	else {
		strcpy(i+2, t->output);
	}

	if(t->bufptr < 0) {
		mudlog(LOG_ERROR, "***** OVER FLOW **** in process_output, comm.c");
		strcat(i+2, "**OVERFLOW**");
	}

	if(!t->connected && !(t->character && !IS_NPC(t->character) &&
						  IS_SET(t->character->specials.act, PLR_COMPACT))) {
		strcat(i+2, "\n\r");
	}

	if(write_to_descriptor(t->descriptor, i+2) < 0) {
		return -1;
	}

	if(t->snoop.snoop_by) {
		i[0] = '%';
		i[1] = ' ';
		SEND_TO_Q(i, t->snoop.snoop_by->desc);
	}

	/* if we were using a large buffer, put the large buffer on the buffer
	   pool and switch back to the small one */
	if(t->large_outbuf) {
		t->large_outbuf->next = bufpool;
		bufpool = t->large_outbuf;
		t->large_outbuf = NULL;
		t->output = t->small_outbuf;
	}

	/* reset total bufspace back to that of a small buffer */
	t->bufspace = SMALL_BUFSIZE-1;
	t->bufptr = 0;
	*(t->output) = '\0';

	return(1);
}

#endif

#if 1         /* reset to use this code, lets see if it helps */
int write_to_descriptor(int desc, const char* txt) {
	int sofar, thisround, total;

	total = strlen(txt);
	sofar = 0;

	do {
		thisround = write(desc, txt + sofar, total - sofar);
		if(thisround < 0) {
			if(errno == EWOULDBLOCK) {
				break;
			}
			mudlog(LOG_ERROR,"%s:%s","Write to socket",strerror(errno));
			/*
			 * lets see if this stops it from crashing close_socket_fd(desc);
			 * arioch
			 */
			return(-1);
		}
		sofar += thisround;
	}
	while(sofar < total);


	return(0);
}
#else
/* merc code */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
int write_to_descriptor(int desc, char* txt) {
	int iStart;
	int nWrite;
	int nBlock, length;
	char buf[256];

	length = strlen(txt);

	for(iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
		if((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
			if(errno == EWOULDBLOCK) {
				break;
			}
			mudlog(LOG_ERROR, "<#=%d> had a error (%d) in write to descriptor "
				   "(Broken Pipe?)", desc, errno);
			mudlog(LOG_ERROR,"%s:%s","Write_to_descriptor",strerror(errno));
			/* close_socket_fd(desc); */
			return(-1);
		}
	}

	return(0);
}

#endif




/* SEARCH HERE */
int process_input(struct descriptor_data* t) {
	int sofar, thisround, begin, squelch, i, k, flag;
	char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];

	sofar = 0;
	flag = 0;
	begin = strlen(t->buf);

	/* Read in some stuff */
	do {
		if((thisround = read(t->descriptor, t->buf + begin + sofar,
							 MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
			sofar += thisround;
		}
		else {
			if(thisround < 0) {
				if(errno != EWOULDBLOCK) {
					mudlog(LOG_ERROR,"%s:%s","Read1 - ERROR",strerror(errno));
					return(-1);
				}
				else {
					break;
				}
			}
			else {
				mudlog(LOG_CONNECT, "EOF encountered on socket read.");
				return(-1);
			}
		}
	}
	while(!ISNEWL(*(t->buf + begin + sofar - 1)));

	*(t->buf + begin + sofar) = 0;

	/* if no newline is contained in input, return without proc'ing */
	for(i = begin; !ISNEWL(*(t->buf + i)); i++)
		if(!*(t->buf + i)) {
			return(0);
		}

	/* input contains 1 or more newlines; process the stuff */
	for(i = 0, k = 0; *(t->buf + i);) {
		if(!ISNEWL(*(t->buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2)))) {
			if(*(t->buf + i) == '\b') {        /* backspace */
				if(k) {  /* more than one char ? */
					if(*(tmp + --k) == '$') {
						k--;
					}
					i++;
				}
				else {
					i++;  /* no or just one char.. Skip backsp */
				}
			}
			else if(isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
				/*
				 * trans char, double for '$' (printf)
				 */
				if((*(tmp + k) = *(t->buf + i)) == '$') {
					*(tmp + ++k) = '$';
				}
				k++;
				i++;
			}
			else {
				i++;
			}
		}
		else {
			*(tmp + k) = 0;
			if(*tmp == '!') {
				strcpy(tmp,t->last_input);
			}
			else {
				strcpy(t->last_input,tmp);
			}

			write_to_q(tmp, &t->input);

			if((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
				SEND_TO_Q("% ",t->snoop.snoop_by->desc);
				SEND_TO_Q(tmp,t->snoop.snoop_by->desc);
				SEND_TO_Q("\n\r",t->snoop.snoop_by->desc);
			}

			if(flag) {
				sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
				if(write_to_descriptor(t->descriptor, buffer) < 0) {
					return(-1);
				}

				/* skip the rest of the line */
				for(; !ISNEWL(*(t->buf + i)); i++);
			}

			/* find end of entry */
			for(; ISNEWL(*(t->buf + i)); i++);

			/* squelch the entry from the buffer */
			for(squelch = 0;; squelch++)
				if((*(t->buf + squelch) =
							*(t->buf + i + squelch)) == '\0') {
					break;
				}
			k = 0;
			i = 0;
		}
	}
	return(1);
}

void close_sockets(int s) {
	mudlog(LOG_CHECK, "Closing all sockets.");

	while(descriptor_list) {
		close_socket(descriptor_list);
	}
	close(s);
}

void close_socket(struct descriptor_data* d) {
	if(!d) {
		return;
	}


	close(d->descriptor);

	flush_queues(d);

	if(d->descriptor == maxdesc) {
		--maxdesc;
	}

	/* Forget snooping */
	if(d->snoop.snooping) {
		d->snoop.snooping->desc->snoop.snoop_by = 0;
	}

	if(d->snoop.snoop_by) {
		send_to_char("La tua vittima non e` piu` fra noi.\n\r",
					 d->snoop.snoop_by);
		d->snoop.snoop_by->desc->snoop.snooping = 0;
	}

	if(d->character) {
		if(d->connected == CON_PLYNG) {
			do_save(d->character, "", 0);
			act("$n ha perso il senso della realta`.", TRUE, d->character, 0, 0,
				TO_ROOM);
			mudlog(LOG_CONNECT, "Closing link to: %s.", GET_NAME(d->character));
			if(IS_NPC(d->character)) {
				/* poly, or switched god */
				if(d->character->desc) {
					/* d->character->orig = d->character->desc->original;

					    Provo a forzare un return al momento in cui il mud
					    si accorge del link dead. Gaia 2001 */

					do_return(d->character, "", 1);
					mudlog(LOG_CONNECT, "%s e' linkdead", GET_NAME(d->character));
				}
			}
			d->character->desc = 0;
			mudlog(LOG_CONNECT, "%s descriptor set to 0", GET_NAME(d->character));

			if(!IS_AFFECTED(d->character, AFF_CHARM)) {
				if(d->character->master) {
					stop_follower(d->character);
				}
			}
			d->character->invis_level = IMMORTALE;

		}
		else {
			free_char(d->character);
		}
	}
	else {
		mudlog(LOG_CONNECT, "Losing descriptor without char.");
	}


	if(next_to_process == d) {           /* to avoid crashing the process loop */
		next_to_process = next_to_process->next;
	}


	if(d == descriptor_list) { /* this is the head of the list */
		descriptor_list = descriptor_list->next;
	}
	else { /* This is somewhere inside the list */
		struct descriptor_data* tmp;
		/* Locate the previous element */
		for(tmp = descriptor_list; (tmp) && (tmp->next) != d; tmp = tmp->next)
			;

		if(tmp) {
			tmp->next = d->next;
		}
		else {
			/* not sure where this gets fried, but it keeps poping up now and
			 * then, let me know if you figure it out. msw */
			mudlog(LOG_SYSERR, "Descriptor out of list for:");
			/* Work around: esco senza ripulire d
			 * non e' bello... ma tanto ci crasha sopra
			 * */
			return;
			/* I know that when a new user cuts link in the login process the
			 * first time it will get here, how to stop it I do not know
			 * end bug */
		}

	}/* end inside the list */

	if(d) {
		if(d->connected == CON_PLYNG) {

			if(d->showstr_head) {
				free(d->showstr_head);
				d->showstr_head=0;
			}
		}
		delete(d);
	}
	d=nullptr;

}




#if defined( LINUX )

void nonblock(int s) {
	int nFlags;

	nFlags = fcntl(s, F_GETFL);
	nFlags |= O_NONBLOCK;
	if(fcntl(s, F_SETFL, nFlags) < 0) {
		mudlog(LOG_ERROR,"%s:%s","Fatal error executing nonblock (comm.c,strerror(errno))");
	}
	return;
}

#else

void nonblock(int s) {
	if(fcntl(s, F_SETFL, FNDELAY) == -1) {
		mudlog(LOG_ERROR,"%s:%s","Noblock",strerror(errno));
		assert(0);
	}
}

#endif





/******************************************************************
*        Public routines for system-to-player-communication          *
**************************************************************** */


void send_to_char(const char* messg, struct char_data* ch) {
	if(ch)
		if(ch->desc && messg)
			SEND_TO_Q(
				ParseAnsiColors(IS_SET(ch->player.user_flags, USE_ANSI),messg),
				ch->desc);
}


void save_all() {
	struct descriptor_data* i;

	for(i = descriptor_list; i; i = i->next)
		if(i->character) {
			save_char(i->character, AUTO_RENT, 0);
		}
}

void send_to_all(const char* messg) {
	struct descriptor_data* i;

	if(messg)
		for(i = descriptor_list; i; i = i->next)
			if(!i->connected)
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),
									messg), i);

}


void send_to_outdoor(const char* messg) {
	struct descriptor_data* i;

	if(messg)
		for(i = descriptor_list; i; i = i->next)
			if(!i->connected)
				if(OUTSIDE(i->character))
					SEND_TO_Q(
						ParseAnsiColors(IS_SET(i->character->player.user_flags,
											   USE_ANSI), messg), i);
}

void send_to_desert(const char* messg) {
	struct descriptor_data* i;
	struct room_data* rp;

	if(messg) {
		for(i = descriptor_list; i; i = i->next) {
			if(!i->connected) {
				if(OUTSIDE(i->character)) {
#if 1
					if((rp = real_roomp(i->character->in_room)) != NULL) {
						if(IS_SET(zone_table[ rp->zone ].reset_mode, ZONE_DESERT) ||
								rp->sector_type == SECT_DESERT) {
							SEND_TO_Q(ParseAnsiColors(IS_SET(i->character->player.user_flags,
															 USE_ANSI), messg), i);
						}
					}
#else
					if(IS_SET(RM_FLAGS(i->character->in_room), DESERTIC))
						SEND_TO_Q(ParseAnsiColors(IS_SET(i->character->player.user_flags,
														 USE_ANSI), messg), i);
#endif
				}
			}
		}
	}
}

void send_to_out_other(const char* messg) {
	struct descriptor_data* i;
	struct room_data* rp;

	if(messg) {
		for(i = descriptor_list; i; i = i->next) {
			if(!i->connected) {
				if(OUTSIDE(i->character)) {
					if((rp = real_roomp(i->character->in_room)) != NULL) {
						if(!IS_SET(zone_table[ rp->zone ].reset_mode, ZONE_DESERT) &&
								!IS_SET(zone_table[ rp->zone ].reset_mode, ZONE_ARCTIC) &&
								rp->sector_type != SECT_DESERT) {
							SEND_TO_Q(ParseAnsiColors(
										  IS_SET(i->character->player.user_flags, USE_ANSI),
										  messg), i);
						}
					}
				}
			}
		}
	}
}


void send_to_arctic(const char* messg) {
	struct descriptor_data* i;
	struct room_data* rp;

	if(messg) {
		for(i = descriptor_list; i; i = i->next) {
			if(!i->connected) {
				if(OUTSIDE(i->character)) {
					if((rp = real_roomp(i->character->in_room))!=NULL) {
						if(IS_SET(zone_table[rp->zone].reset_mode, ZONE_ARCTIC)) {
							SEND_TO_Q(ParseAnsiColors(
										  IS_SET(i->character->player.user_flags, USE_ANSI),messg), i);

						}
					}
				}
			}
		}
	}
}


void send_to_except(const char* messg, struct char_data* ch) {
	struct descriptor_data* i;

	if(messg)
		for(i = descriptor_list; i; i = i->next)
			if(ch->desc != i && !i->connected)
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
					i);

}


void send_to_zone(const char* messg, struct char_data* ch) {
	struct descriptor_data* i;

	if(messg)
		for(i = descriptor_list; i; i = i->next)
			if(ch->desc != i && !i->connected)
				if(real_roomp(i->character->in_room)->zone ==
						real_roomp(ch->in_room)->zone)
					SEND_TO_Q(
						ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
						i);

}




void send_to_room(const char* messg, int room) {
	struct char_data* i;
	char buf[MAX_STRING_LENGTH];

	if(messg)
		for(i = real_roomp(room)->people; i; i = i->next_in_room)
			if(i->desc)
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
					i->desc);

	if(!IS_SET(real_roomp(room)->room_flags, SILENCE)) {
		for(i = real_roomp(room)->listeners; i ; i= i->next_listener) {
			if(i->desc) {
				sprintf(buf, "Origli: %s", messg);
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),buf),
					i->desc);
			}
		}
	}
}




void send_to_room_except(const char* messg, int room, struct char_data* ch) {
	struct char_data* i;

	if(messg)
		for(i = real_roomp(room)->people; i; i = i->next_in_room)
			if(i != ch && i->desc)
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
					i->desc);

}

void send_to_room_except_two
(const char* messg, int room, struct char_data* ch1, struct char_data* ch2) {
	struct char_data* i;

	if(messg)
		for(i = real_roomp(room)->people; i; i = i->next_in_room)
			if(i != ch1 && i != ch2 && i->desc)
				SEND_TO_Q(
					ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
					i->desc);

}


/* higher-level communication */

/* ACT */
void actall(const char* s1, const char* s2, const char* s3,
			struct char_data* ch, struct char_data* vc) {
	char buf[1024];
	sprintf(buf,"%s $N",s1);
	act(buf,FALSE,ch,0,vc,TO_CHAR);
	sprintf(buf,"$n %s",s2);
	act(buf,FALSE,ch,0,vc,TO_VICT);
	sprintf(buf,"$n %s $N",s3);
	act(buf,FALSE,ch,0,vc,TO_NOTVICT);
	return;
}

void ParseAct(const char* str, struct char_data* ch, struct char_data* to, void* vict_obj, struct obj_data* obj, char* buf)

{
	char* strp;
	char* point;
	const char* i;
	char tmp[5];

	for(strp = const_cast<char*>(str), point = buf;;) {
		if(*strp == '$') {
			switch(*(++strp)) {
			case 'n':
				i = PERS(ch, to);
				break;
			case 'N':
				if(vict_obj != NULL) {
					i =PERS((struct char_data*) vict_obj, to);
				}
				else {
					mudlog(LOG_SYSERR, "$N e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'b':
				i = SSLF(ch);
				break;
			case 'B':
				if(vict_obj != NULL) {
					i = SSLF((struct char_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$B e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'd':
				i = LEGLI(ch);
				break;
			case 'D':
				if(vict_obj != NULL) {
					i = LEGLI((struct char_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$D e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'm':
				i = HMHR(ch);
				break;
			case 'M':
				if(vict_obj != NULL) {
					i = HMHR((struct char_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$M e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 's':
				i = HSHR(ch);
				break;
			case 'S':
				if(vict_obj != NULL) {
					i = HSHR((struct char_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$S e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'e':
				i = HSSH(ch);
				break;
			case 'E':
				if(vict_obj != NULL) {
					i = HSSH((struct char_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$E e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'o':
				if(obj != NULL) {
					i = OBJN(obj, to);
				}
				else {
					mudlog(LOG_SYSERR, "$o e obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'O':
				if(vict_obj != NULL) {
					i = OBJN((struct obj_data*) vict_obj, to);
				}
				else {
					mudlog(LOG_SYSERR, "$O e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'p':
				if(obj != NULL) {
					i = OBJS(obj, to);
				}
				else {
					mudlog(LOG_SYSERR, "$p e obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'P':
				if(vict_obj != NULL) {
					i = OBJS((struct obj_data*) vict_obj, to);
				}
				else {
					mudlog(LOG_SYSERR, "$P e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'a':
				if(obj != NULL) {
					i = SANA(obj);
				}
				else {
					mudlog(LOG_SYSERR, "$a e obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'A':
				if(vict_obj != NULL) {
					i = SANA((struct obj_data*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$A e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'T':
				if(vict_obj != NULL) {
					i = (char*) vict_obj;
				}
				else {
					mudlog(LOG_SYSERR, "$T e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case 'F':
				if(vict_obj != NULL) {
					i = fname((char*) vict_obj);
				}
				else {
					mudlog(LOG_SYSERR, "$F e vict_obj == NULL in act(comm.c)");
					i = "";
				}
				break;
			case '$':
				i = "$";
				break;
			default:
				tmp[ 0 ] = '$';
				tmp[ 1 ] = *strp;
				tmp[ 2 ] = '\0';
				i = tmp;
				break;
			}

			if(point && i) {
				while((*point = *(i++)) != 0) {
					++point;
				}

				++strp;
			}
		}
		else if(!(*(point++) = *(strp++))) {
			break;
		}
	}
	*(--point) = '\n';
	*(++point) = '\r';
	*(++point) = '\0';


}
void act(const char* str, int hide_invisible, struct char_data* ch,
		 struct obj_data* obj, void* vict_obj, int type) {
	struct char_data* to, *z;
	char buf[MAX_STRING_LENGTH];

	if(ch == NULL) {
		mudlog(LOG_SYSERR, "ch == NULL in act (comm.c). str == %s", str);
		return;
	}

	if(!str) {
		return;
	}
	if(!*str) {
		return;
	}

	if(ch->in_room <= -1) {
		return;    /* can't do it. in room -1 */
	}

	if(type == TO_VICT) {
		to = static_cast<struct char_data*>(vict_obj);
	}
	else if(type == TO_CHAR) {
		to = ch;
	}
	else {
		to = real_roomp(ch->in_room)->people;
	}

	for(; to; to = to->next_in_room) {
		if(to->desc &&
				(!hide_invisible || CAN_SEE(to, ch)) &&
				GET_POS(to) > POSITION_SLEEPING &&
				((type == TO_CHAR && to == ch) ||
				 (type == TO_VICT && to == (struct char_data*) vict_obj) ||
				 (type == TO_ROOM && to != ch) ||
				 (type == TO_NOTVICT && to != (struct char_data*) vict_obj &&
				  to != ch))) {

			ParseAct(str,ch,to,vict_obj,obj,buf);
			CAP(buf);
			send_to_char(buf, to);
			send_to_char("$c0007", to);
		}

		if((type == TO_VICT) || (type == TO_CHAR)) {
			return;
		}
	}

	if(type == TO_ROOM) {
		if(!IS_SET(real_roomp(ch->in_room)->room_flags, SILENCE)) {
			for(z = real_roomp(ch->in_room)->listeners; z ; z = z ->next_listener) {
				if(z->desc) {
					ParseAct(str,ch,z,vict_obj,obj,buf);
					send_to_char("Origli: ",z);
					send_to_char(buf, z);
				}
			}
		}
	}


}


void raw_force_all(const char* to_force)

{
	struct descriptor_data* i;
	char buf[400];

	for(i = descriptor_list; i; i = i->next) {
		if(i->connected == CON_PLYNG) {
			sprintf(buf, "Qualcuno ha eseguito per te il comando '%s'.\n\r",
					to_force);
			send_to_char(buf, i->character);
			command_interpreter(i->character, to_force);
		}
	}
}

int _affected_by_s(struct char_data* ch, int skill) {
	struct affected_type* hjp;
	int  fs=0,fa=0;

	switch(skill) {
	case SPELL_FIRESHIELD:
		if(IS_AFFECTED(ch, AFF_FIRESHIELD)) {
			fa=1;
		}
		break;
	case SPELL_SANCTUARY:
		if(IS_AFFECTED(ch, AFF_SANCTUARY)) {
			fa=1;
		}
		break;
	case SPELL_INVISIBLE:
		if(IS_AFFECTED(ch, AFF_INVISIBLE)) {
			fa=1;
		}
		break;
	case SPELL_TRUE_SIGHT:
		if(IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
			fa=1;
		}
		break;
	case SPELL_PROT_ENERGY_DRAIN:
		if(IS_SET(ch->immune,IMM_DRAIN) || IS_SET(ch->M_immune,IMM_DRAIN)) {
			fa=1;
		}
		break;
	case SPELL_GLOBE_DARKNESS:
		if(IS_AFFECTED(ch, AFF_GLOBE_DARKNESS)) {
			fa=1;
		}
		break;
	}
	if(ch->affected)
		for(hjp = ch->affected; hjp; hjp = hjp->next)
			if(hjp->type == skill) {
				fs = (hjp->duration + 1);    /* in case it's 0 */
			}

	if(!fa && !fs) {
		return -1;
	}
	else if(fa && !fs) {
		return 999;
	}
	else {
		return fs-1;
	}
}


void construct_prompt(char* outbuf, struct char_data* ch) {
	struct room_data* rm;
	struct char_data* tmp=0;
	char tbuf[255];
	const char* pr_scan;
	const char* mask;
	long l,exp,texp;
	int i,s_flag=0;
	int IsSupporting=0;
	struct char_data* victim=NULL;

	if(ch == NULL) {
		mudlog(LOG_SYSERR, "ch == NULL in construct_prompt");
		return;
	}
	if(outbuf == NULL) {
		mudlog(LOG_SYSERR, "output == NULL in construct_prompt");
		return;
	}

	*outbuf=0;

	if(ch->desc && ch->desc->original) {
		mask = ch->desc->original->specials.prompt;
	}
	else {
		mask = ch->specials.prompt;
	}

	if(mask == NULL) {
		/* use default prompts */
		if(IS_DIO(ch)) {
			mask="%N R%R [%iI/%iN/%iS]>> ";
		}
		else {
			mask="%N H%h M%m V%v X%x [%S] %c/%C>> ";
		}
	}
	if(!ch->specials.fighting
			&& ch->specials.supporting
			&& (victim=get_char_room_vis(ch,ch->specials.supporting))
			&& victim->specials.fighting) {
		/* Per il comando support. */
		IsSupporting=1;
		ch->specials.fighting=victim->specials.fighting;
	}

	for(pr_scan = mask; *pr_scan; pr_scan++) {
		if(*pr_scan == '%') {
			if(*(++pr_scan) == '%') {
				tbuf[ 0 ]='%';
				tbuf[ 1 ]=0;
			}
			else {
				switch(*pr_scan) {
				/* stats for character */
				case 'N':
				case 'n':
					sprintf(tbuf, "%s", ch->player.name);
					break;
				case 'H':
					sprintf(tbuf, "%d", GET_MAX_HIT(ch));
					break;
				case 'h':
					sprintf(tbuf, "%d", GET_HIT(ch));
					break;
				case 'M':
					sprintf(tbuf, "%d", GET_MAX_MANA(ch));
					break;
				case 'm':
					sprintf(tbuf, "%d", GET_MANA(ch));
					break;
				case 'V':
					sprintf(tbuf, "%d", GET_MAX_MOVE(ch));
					break;
				case 'v':
					sprintf(tbuf, "%d", GET_MOVE(ch));
					break;
				case 'G':
					sprintf(tbuf, "%d", GET_BANK(ch));
					break;
				case 'g':
					sprintf(tbuf, "%d", GET_GOLD(ch));
					break;
				case 'X': /* xp stuff */
					sprintf(tbuf, "%d", GET_EXP(ch));
					break;
				case 'x': /* xp left to level (any level, btw..) */
					if IS_SET(ch->specials.act,ACT_POLYSELF)
						/* per i polymorfati devo usare i massimi razziali del
						* personaggio originale
						* */
					{
						tmp=ch->desc->original;
					}
					if(!tmp) {
						tmp=ch;
					}
#define DUMMYVALUE 999999999
					for(l=1, i=0, texp=exp=DUMMYVALUE;
							i <= PSI_LEVEL_IND;
							i++, l <<= 1) {
						if(HasClass(ch, l)) {
							if(GET_LEVEL(ch,i)<RacialMax[GET_RACE(tmp)][i])
								texp = (titles[ i ][ GET_LEVEL(ch, i) + 1 ].exp) -
									   GET_EXP(ch);
							if(texp < exp) {
								exp=texp;
							}
						}
					}
					if(exp==DUMMYVALUE) {
						exp=(PRINCEEXP-GET_EXP(ch));
					}
					sprintf(tbuf, "%ld", exp);
					break;
				case 'C':   /* mob condition */
					if(ch->specials.fighting) {
						i = (100 * GET_HIT(ch->specials.fighting)) /
							GET_MAX_HIT(ch->specials.fighting);
						if(i >= 100) {
							strcpy(tbuf, "eccellente");
						}
						else if(i>=80) {
							strcpy(tbuf, "graffiato");
						}
						else if(i>=60) {
							strcpy(tbuf, "tagliato");
						}
						else if(i >= 40) {
							strcpy(tbuf, "ferito");
						}
						else if(i >= 20) {
							strcpy(tbuf, "sanguinante");
						}
						else if(i >= 0) {
							strcpy(tbuf, "squarciato");
						}
						else {
							strcpy(tbuf, "morente");
						}
					}
					else {
						strcpy(tbuf, "*");
					}
					break;
				case 'c':   /* tank condition */
					if(ch->specials.fighting &&
							ch->specials.fighting->specials.fighting) {
						i = (100 *
							 GET_HIT(ch->specials.fighting->specials.fighting)) /
							GET_MAX_HIT(ch->specials.fighting->specials.fighting);
						if(i >= 100) {
							strcpy(tbuf, "eccellente");
						}
						else if(i>=80) {
							strcpy(tbuf, "graffiato");
						}
						else if(i>=60) {
							strcpy(tbuf, "tagliato");
						}
						else if(i >= 40) {
							strcpy(tbuf, "ferito");
						}
						else if(i >= 20) {
							strcpy(tbuf, "sanguinante");
						}
						else if(i >= 0) {
							strcpy(tbuf, "squarciato");
						}
						else {
							strcpy(tbuf, "morente");
						}
					}
					else {
						strcpy(tbuf, "*");
					}
					break;
					break;
				case 'T':   /* mob name */
					if(ch->specials.fighting) {
						strcpy(tbuf, GET_NAME(ch->specials.fighting));
					}
					else {
						strcpy(tbuf, "*");
					}
					break;
				case 't':   /* tank name */
					if(ch->specials.fighting &&
							ch->specials.fighting->specials.fighting) {
						strcpy(tbuf,GET_NAME(ch->specials.fighting->specials.fighting));
					}
					else {
						strcpy(tbuf, "*");
					}
					break;
				case 's':
					s_flag = 1;
				/* no break */
				case 'S':   /* affected spells */
					*tbuf=0;
					if((i = _affected_by_s(ch, SPELL_FIRESHIELD)) != -1) {
						strcat(tbuf, (i > 1) ? "F" : "f");
					}
					else if(s_flag) {
						strcat(tbuf,"-");
					}
					if((i = _affected_by_s(ch, SPELL_SANCTUARY)) != -1) {
						strcat(tbuf, (i > 1) ? "S" : "s");
					}
					else if(s_flag) {
						strcat(tbuf, "-");
					}
					if((i = _affected_by_s(ch, SPELL_INVISIBLE)) != -1) {
						strcat(tbuf, (i > 1) ? "I" : "i");
					}
					else if(s_flag) {
						strcat(tbuf,"-");
					}
					if((i = _affected_by_s(ch, SPELL_TRUE_SIGHT)) != -1) {
						strcat(tbuf, (i > 1) ? "T" : "t");
					}
					else if(s_flag) {
						strcat(tbuf, "-");
					}
					if((i = _affected_by_s(ch, SPELL_PROT_ENERGY_DRAIN)) != -1) {
						strcat(tbuf,(i>1)?"D":"d");
					}
					else if(s_flag) {
						strcat(tbuf, "-");
					}
					if((i = _affected_by_s(ch, SPELL_ANTI_MAGIC_SHELL)) != -1) {
						strcat(tbuf, (i > 1) ? "A" : "a");
					}
					else if(s_flag) {
						strcat(tbuf, "-");
					}
					break;
				case 'R': /* room number for immortals */
					if(IS_DIO(ch)) {
						rm = real_roomp(ch->in_room);
						if(!rm) {
							char_to_room(ch, 0);
							rm = real_roomp(ch->in_room);
						}
						sprintf(tbuf, "%ld", rm->number);
					}
					else {
						*tbuf = 0;
					}
					break;
				case 'i':   /* immortal stuff going */
					pr_scan++;
					if(!IS_DIO(ch)) {
						*tbuf = 0;
						break;
					}
					switch(*pr_scan) {
					case 'I':   /* invisible status */
						sprintf(tbuf, "%d", ch->invis_level);
						break;
					case 'S':   /* stealth mode */
						strcpy(tbuf,IS_SET(ch->specials.act, PLR_STEALTH)?"On":"Off");
						break;
					case 'N':   /* snoop name */
						if(ch->desc->snoop.snooping) {
							strcpy(tbuf, ch->desc->snoop.snooping->player.name);
						}
						else {
							*tbuf=0;
						}
						break;
					default:
						*tbuf=0;
						break;
					}
					break;
				default:
					*tbuf=0;
					break;
				}
			}
		}
		else { /* end of if ch=='%'  */
			tbuf[ 0 ] = *pr_scan;
			tbuf[ 1 ] = 0;
		}
		strcat(outbuf,tbuf);
	} /* fine del for */
	if(IsSupporting) {
		/* se questo flag e` attivo ch->specials.fighting e` stato impostato
		 * in questa routine. Lo riannullo */
		ch->specials.fighting=NULL;
	}
}
void UpdateScreen(struct char_data* ch, int update) {
	char buf[255];
	int size;

	size = ch->size;

	if(size <= 0) {
		return;
	}

	if(IS_SET(update, INFO_MANA)) {
		sprintf(buf, VT_CURSAVE);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 2, 7);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "          ");
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 2, 7);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURREST);
		write_to_descriptor(ch->desc->descriptor, buf);
	}

	if(IS_SET(update, INFO_MOVE)) {
		sprintf(buf, VT_CURSAVE);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 3, 58);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "          ");
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 3, 58);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURREST);
		write_to_descriptor(ch->desc->descriptor, buf);
	}

	if(IS_SET(update, INFO_HP)) {
		sprintf(buf, VT_CURSAVE);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 3, 13);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "          ");
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 3, 13);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURREST);
		write_to_descriptor(ch->desc->descriptor, buf);
	}

	if(IS_SET(update, INFO_GOLD)) {
		sprintf(buf, VT_CURSAVE);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 2, 47);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "                ");
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 2, 47);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "%d", GET_GOLD(ch));
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURREST);
		write_to_descriptor(ch->desc->descriptor, buf);
	}

	if(IS_SET(update, INFO_EXP)) {
		sprintf(buf, VT_CURSAVE);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 1, 20);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "                ");
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURSPOS, size - 1, 20);
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, "%d", GET_EXP(ch));
		write_to_descriptor(ch->desc->descriptor, buf);
		sprintf(buf, VT_CURREST);
		write_to_descriptor(ch->desc->descriptor, buf);
	}
}


void InitScreen(struct char_data* ch) {
	char buf[255];
	int size;

	size = ch->size;
	sprintf(buf, VT_HOMECLR);
	send_to_char(buf, ch);
	sprintf(buf, VT_MARGSET, 0, size - 5);
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 4, 1);
	send_to_char(buf, ch);
	sprintf(buf, "-===========================================================================-");
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 3, 1);
	send_to_char(buf, ch);
	sprintf(buf, "Hit Points: ");
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 3, 40);
	send_to_char(buf, ch);
	sprintf(buf, "Movement Points: ");
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 2, 1);
	send_to_char(buf, ch);
	sprintf(buf, "Mana: ");
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 2, 40);
	send_to_char(buf, ch);
	sprintf(buf, "Gold: ");
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 1, 1);
	send_to_char(buf, ch);
	sprintf(buf, "Experience Points: ");
	send_to_char(buf, ch);

	ch->last.mana = GET_MANA(ch);
	ch->last.mmana = GET_MAX_MANA(ch);
	ch->last.hit = GET_HIT(ch);
	ch->last.mhit = GET_MAX_HIT(ch);
	ch->last.move = GET_MOVE(ch);
	ch->last.mmove = GET_MAX_MOVE(ch);
	ch->last.exp = GET_EXP(ch);
	ch->last.gold = GET_GOLD(ch);

	/* Update all of the info parts */
	sprintf(buf, VT_CURSPOS, size - 3, 13);
	send_to_char(buf, ch);
	sprintf(buf, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 3, 58);
	send_to_char(buf, ch);
	sprintf(buf, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 2, 7);
	send_to_char(buf, ch);
	sprintf(buf, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 2, 47);
	send_to_char(buf, ch);
	sprintf(buf, "%d", GET_GOLD(ch));
	send_to_char(buf, ch);
	sprintf(buf, VT_CURSPOS, size - 1, 20);
	send_to_char(buf, ch);
	sprintf(buf, "%d", GET_EXP(ch));
	send_to_char(buf, ch);

	sprintf(buf, VT_CURSPOS, 0, 0);
	send_to_char(buf, ch);

}

int update_max_usage(void) {
	static int max_usage = 0;
	int sockets_playing = 0;
	struct descriptor_data* d;
	char buf[256];

	for(d = descriptor_list; d; d = d->next)
		if(!d->connected) {
			sockets_playing++;
		}

	if(sockets_playing > max_usage) {
		max_usage = sockets_playing;
		sprintf(buf, "Nuovo massimo di giocatori dal reboot: %3d", max_usage);
		slackNotify(buf,":dance:");
		mudlog(LOG_PLAYERS, buf);
	}

	return max_usage;
}
} // namespace Alarmud

