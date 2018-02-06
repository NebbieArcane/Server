/*$Id: heap.h,v 1.2 2002/02/13 12:30:58 root Exp $
*/

struct StrHeapList {
	char* string;   /* the matching string */
	int  total;     /* total # of occurences */
};

struct StrHeap {
	int uniq;   /* number of uniq items in list */
	struct StrHeapList* str;   /* the list of strings and totals */
};
