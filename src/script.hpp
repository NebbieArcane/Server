/*$Id: script.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/

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


