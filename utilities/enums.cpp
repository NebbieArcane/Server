/*
 * sizes : compile it with or without -m32 to check if sizes differs between architecture
 */
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;
#include "../src/autoenums.hpp"
namespace Alarmud {
void esegui(e_connection_types c) {
	cout << "Switch on enum:" << endl;
	switch(c) {
	case CON_ACCOUNT_NAME:
		cout << G::translate(c) << endl;
		break;
	case CON_ACCOUNT_PWD:
		cout << G::translate(c) << endl;
		break;
	case CON_ACCOUNT_TOON:
		cout << G::translate(c) << endl;
		break;
	default:
		cout << "NOT RECOGNIZED" <<endl;
	}
	cout << "Switch on int:" << endl;
	switch(static_cast<int>(c)) {
	case CON_ACCOUNT_NAME:
		cout << G::translate(c) << endl;
		break;
	case CON_ACCOUNT_PWD:
		cout << G::translate(c) << endl;
		break;
	case CON_ACCOUNT_TOON:
		cout << G::translate(c) << endl;
		break;
	default:
		cout << "NOT RECOGNIZED" <<endl;
	}
}
}
int main() {
	Alarmud::e_connection_types cn=Alarmud::CON_ACCOUNT_PWD;
	esegui(cn);
	return 0;
}
