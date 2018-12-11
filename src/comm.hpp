/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: comm.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
#ifndef __COMM_HPP
#define __COMM_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3
#if not BLOCK_WRITE
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)
#else
#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
#endif
#define FLUSH_TO_Q(messg,desc) write_to_descriptor(desc->descriptor,messg)


#define PLAYER_AUTH 0
#define DFLT_DIR		"lib"				/* default data directory     */

extern struct descriptor_data* descriptor_list;
extern struct descriptor_data* next_to_process;
extern uint16_t NumTimeCheck ; /* dovrebbe essere il piu' grande dei PULSE */
extern uint64_t aTimeCheck[ PULSE_MOBILE ];
extern unsigned long pulse;
extern bool no_specials;
extern int slow_death;     /* Shut her down, Martha, she's sucking mud */
extern int mudshutdown;       /* clean shutdown */
extern int rebootgame;         /* reboot the game after a shutdown */
extern bool no_specials;    /* Suppress ass. of special routines */
extern long Uptime;            /* time that the game has been up */
extern int tics;
extern int PORT;

void InitScreen(struct char_data* ch) ;
void ParseAct(const char* str, struct char_data* ch, struct char_data* to, void* vict_obj, struct obj_data* obj, char* buf);
char* ParseAnsiColors(int UsingAnsi, const char* txt) ;
void UpdateScreen(struct char_data* ch, int update) ;
int _affected_by_s(struct char_data* ch, int skill) ;
void act(const char* str, int hide_invisible, struct char_data* ch,struct obj_data* obj, void* vict_obj, int type);
void actall(const char* s1, const char* s2, const char* s3,	struct char_data* ch, struct char_data* vc);
void close_socket(struct descriptor_data* d) ;
void close_socket_fd(int desc) ;
void close_sockets(int s) ;
void coma(int s) ;
void construct_prompt(char* outbuf, struct char_data* ch) ;
void flush_queues(struct descriptor_data* d) ;
void game_loop(int s) ;
int get_from_q(struct txt_q* queue, char* dest) ;
int init_socket(int port) ;
int run(int argc, const char* argv) ;
int new_connection(int s) ;
int new_descriptor(int s) ;
void nonblock(int s) ;
int process_input(struct descriptor_data* t) ;
int process_output(struct descriptor_data* t) ;
void raw_force_all(const char* to_force);
void run_the_game(int port) ;
void save_all() ;
void send_to_all(const char* messg) ;
void send_to_arctic(const char* messg) ;
void send_to_char(const char* messg, struct char_data* ch) ;
void send_to_desert(const char* messg) ;
void send_to_except(const char* messg, struct char_data* ch) ;
void send_to_out_other(const char* messg) ;
void send_to_outdoor(const char* messg) ;
void send_to_room(const char* messg, int room) ;
void send_to_room_except(const char* messg, int room, struct char_data* ch) ;
void send_to_room_except_two(const char* messg, int room, struct char_data* ch1, struct char_data* ch2);
void send_to_zone(const char* messg, struct char_data* ch) ;
void str2ansi(char* p2, const char* p1, int start, int stop) ;
struct timeval timediff(struct timeval* a, struct timeval* b) ;
int update_max_usage(void) ;
int write_to_descriptor(int desc, const char* txt) ;
void write_to_output(const char* txt, struct descriptor_data* t) ;
void write_to_q(char* txt, struct txt_q* queue) ;
} // namespace Alarmud
#endif

