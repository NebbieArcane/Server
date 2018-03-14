#include "locks.hpp"

#include <sys/file.h>
#include <time.h>

int glock(int fd,int timeout) {

	int rval;
	int start;
	start=time((time_t)NULL);
	while ((rval=flock(fd,LOCK_EX + LOCK_NB))!=0) {

		if (timeout<(time((time_t)NULL)-start))
		{ break; }
	}

	return !rval;
}


int gunlock(int fd) {

	return   flock(fd,LOCK_UN + LOCK_NB);
}
