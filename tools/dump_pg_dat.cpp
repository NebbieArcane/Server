#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"

using namespace Alarmud;

static void clean(char* s, size_t max) {
	if(max == 0) {
		return;
	}
	s[max - 1] = 0;
	for(char* p = s; *p; ++p) {
		if(*p == '\t' || *p == '\n' || *p == '\r' || *p == '\'') {
			*p = ' ';
		}
	}
}

int main(int argc, char** argv) {
	if(argc < 2) {
		return 1;
	}
	FILE* f = fopen(argv[1], "rb");
	if(!f) {
		return 2;
	}
	struct stat st {};
	fstat(fileno(f), &st);
	char_file_u u {};
	const size_t n =
		(size_t)st.st_size < sizeof(u) ? (size_t)st.st_size : sizeof(u);
	if(n == 0 || fread(&u, n, 1, f) != 1) {
		fclose(f);
		return 3;
	}
	fclose(f);
	clean(u.name, sizeof(u.name));
	clean(u.pwd, sizeof(u.pwd));
	clean(u.title, sizeof(u.title));
	if(!u.name[0]) {
		return 4;
	}
	printf("%s\t%s\t%s\t%u\n", u.name, u.pwd, u.title, u.last_logon);
	return 0;
}
