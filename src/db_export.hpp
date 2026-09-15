/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* db_export.hpp - comando wiz dbexport: MySQL -> CSV (no password).
 * */
#ifndef SRC_DB_EXPORT_HPP_
#define SRC_DB_EXPORT_HPP_
/***************************  System  include ************************************/
/***************************  General include ************************************/
#include "typedefs.hpp"
/***************************  Local    include ************************************/
namespace Alarmud {

/**
 * Export MySQL -> CSV sotto exports/ (cwd tipicamente lib/).
 * Password e authcode mai esportati; email solo con flag pii.
 */
ACTION_FUNC(do_dbexport);

} // namespace Alarmud

#endif // SRC_DB_EXPORT_HPP_
