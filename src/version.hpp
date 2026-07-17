/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef _VERSION_HPP
#define _VERSION_HPP
/***************************  System  include
 * ************************************/
/***************************  Local    include
 * ************************************/
namespace Alarmud {
const char *version(void);
const char *release(void);
/** Commit message body baked in at compile time (may be empty). */
const char *release_body(void);
/** Git commit author (%an) baked in at compile time (may be empty). */
const char *release_author(void);
const char *compilazione(void);
void defaults(void);
} // namespace Alarmud
#endif
