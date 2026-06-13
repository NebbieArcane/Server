/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include
 * ************************************/
#include <cstdio>
#include <iostream>
/***************************  General include
 * ************************************/
/***************************  Local    include
 * ************************************/
#include "config.hpp"
#include "release.hpp"
#include "version.hpp"
namespace Alarmud {
using std::cout;
using std::endl;
using std::string;
const char *version() { return VERSION; }

const char *release() { return BUILD; }

const char *compilazione() {
  static char COMPILAZIONE[50];
  snprintf(COMPILAZIONE, 49, "%s %s", __DATE__, __TIME__);
  return (COMPILAZIONE);
}
void defaults() {
  cout << "Compiled in defaults" << endl;
  cout << "MYSQL_USER: " << MYSQL_USER << endl;
  cout << "MYSQL_PASSWORD: " << MYSQL_PASSWORD << endl;
  cout << "MYSQL_HOST: " << MYSQL_HOST << endl;
  cout << "MYSQL_PORT: " << MYSQL_PORT << endl;
  cout << "MYSQL_DB: " << MYSQL_DB << endl;
#ifdef BOOST_VERSION
  cout << "BOOST_VERSION: " << BOOST_VERSION << endl;
#endif
#ifdef CURL_VERSION
  cout << "CURL_VERSION: " << CURL_VERSION << endl;
#endif
#ifdef MYSQL_VERSION
  cout << "MYSQL_VERSION: " << MYSQL_VERSION << endl;
#endif
#ifdef SQLITE_VERSION
  cout << "SQLITE_VERSION: " << SQLITE_VERSION << endl;
#endif
} // namespace Alarmud
#ifdef STANDALONE
int main() {
  printf("%s %s %s \n\r", Alarmud::version(), Alarmud::release(),
         Alarmud::compilazione());
  return (0);
}
#endif
} // namespace Alarmud
