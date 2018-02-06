/* db_add.c: storing
   written by rock <shirock@residence.educities.edu.tw>, 1999.11.29
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdbm.h>

#define datum_set(um, buf) { um.dptr = buf; um.dsize = strlen(buf); }

int db_ls(char *dbname) {
	GDBM_FILE dbf;
	datum key, next, data;
	int n = 1;

	dbf = gdbm_open(dbname, 4096, GDBM_READER, 0666, NULL);
	if(!dbf) {
		printf("%s\n", gdbm_strerror(gdbm_errno));
		return 1;
	}
	key = gdbm_firstkey(dbf);
	while( key.dptr ) {
		data = gdbm_fetch(dbf, key);
		printf("%s :@: %s\n", key.dptr, data.dptr);
		next = gdbm_nextkey(dbf, key);
		free(key.dptr);
		key = next;
	}
	gdbm_close(dbf);
	return 0;
}
int main(int argc, char *argv[]) {
	char keyb[256], datab[256], *dbname;
	GDBM_FILE dbf;
	datum key, data;

	if( argc < 2) {
		printf("Usage: %s <dbname>\n", argv[0]);
		return 1;
	}

  /*
	if (!strcmp(argv[1], "add")){
		db_add(argv[2]);
	} else if (!strcmp(argv[1], "ls")){
		db_ls(argv[2]);
	} else if (!strcmp(argv[1], "rm")){
		db_rm(argv[2]);
	}
  */

  db_ls(argv[1]);

	return 0;
}

