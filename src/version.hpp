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
/** True when this binary was built as a real rX.Y[.Z] release (see genrelease.sh). */
bool is_release(void);
/** Short Server motd version (e.g. "3.6.5"); empty if not a release build. */
const char *motd_version(void);
/** Optional short motd motto from MOTD_HEADLINE; may be empty. */
const char *motd_headline(void);
const char *compilazione(void);
void defaults(void);
} // namespace Alarmud
#endif
