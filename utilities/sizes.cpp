/*
 * sizes : compile it with or without -m32 to check if sizes differs between architecture
 */
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;
#include "../src/structs.h"
int main() {
	char c=0;
	short s=0;
	int i=0;
	long l=0;
	long long ll=0;
	time_t t=0;
	char_file_u u;
	struct affected_type *next;

	cout << "byte      :" << sizeof(c)  << " bytes" << endl;
	cout << "short     :" << sizeof(s)  << " bytes" << endl;
	cout << "int       :" << sizeof(i)  << " bytes" << endl;
	cout << "long      :" << sizeof(l) << " bytes" << endl;
	cout << "longlong  :" << sizeof(ll)<< " bytes"  << endl;
	cout << "time_t    :" << sizeof(t)<< " bytes"  << endl;
	cout << "user      :" << sizeof(u) << " bytes"  << endl;
	cout << "ptr       :" << sizeof(next) << " bytes"  << endl;


}
