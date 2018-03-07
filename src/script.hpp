/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: script.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {

struct foo_data {
	char* line;
};

struct scripts {
	char* filename; /* Script name to execute */
	int iVNum; /* Virtual number of the mob associated with the script */
	struct foo_data* script; /* actual script */
};

extern struct scripts* gpScript_data;

struct script_com {
	void (*p) (char* arg, struct char_data* ch);
	char* arg;
};

extern struct script_com* gpComp;


} // namespace Alarmud

