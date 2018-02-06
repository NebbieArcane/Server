/*$Id: wizlist.h,v 1.2 2002/02/13 12:30:59 root Exp $
*/

struct wiznest {
	char* name;
	char* title;
};

struct wiznode {
	struct wiznest stuff[150];
};

struct wizlistgen {
	int number[61];
	struct wiznode lookup[61];
};

