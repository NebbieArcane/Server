/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * logging.cpp
 *
 *  Created on: 10 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */
/***************************  System  include ************************************/
#include <stdarg.h>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/consoleappender.h>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "logging.hpp"
#include "comm.hpp"
#include "multiclass.hpp"
namespace Alarmud {


#if HAS_LOG
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("MainLogger"));
log4cxx::LoggerPtr buglogger(log4cxx::Logger::getLogger("BugLogger"));
log4cxx::LoggerPtr errlogger(log4cxx::Logger::getLogger("ErrLogger"));
boost::format  my_fmt(const std::string &f_string) {
	using namespace boost::io;
	boost::format fmter(f_string);
	fmter.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit )  );
	return fmter;
}

void godTrace(unsigned uType, const char* const szString, ... ) {

	va_list argptr;
	char szBuffer[ LARGE_BUFSIZE ];
	char* pchTimeStr;
	struct descriptor_data* pDesc;

	va_start( argptr, szString );
	vsprintf( szBuffer, szString, argptr );
	va_end( argptr );
	for ( pDesc = descriptor_list; pDesc; pDesc = pDesc->next) {
		if( pDesc->connected == CON_PLYNG && pDesc->str == NULL &&
				GetMaxLevel( pDesc->character ) >= MAESTRO_DEGLI_DEI &&
				( pDesc->character->specials.sev & uType or uType & LOG_CONNECT) ) {
			send_to_char( "$c0014Sysmess: $c0007", pDesc->character );
			send_to_char( szBuffer, pDesc->character );
			send_to_char( "\n\r", pDesc->character );
		}
	}
}
namespace Alarmud {
void log_init(string log_filename,unsigned short debug_level) {
	log_configure(logger,log_filename,".log")->setLevel(get_level(debug_level));
	log_configure(errlogger,"errors",".log")->setLevel(log4cxx::Level::getError());
	log_configure(buglogger,"bugs","")->setLevel(log4cxx::Level::getAll());
}
constexpr auto LAYOUT_1="%d{yy-MM-dd HH:mm:ss.SSS} %5p [...%.20F at %5L] - %m%n";
constexpr auto LAYOUT_2="%d{yy-MM-dd HH:mm:ss.SSS} [...%.16F at %5L] - %m%n";
log4cxx::LoggerPtr log_configure(log4cxx::LoggerPtr &logger,string logname,string suffix) {
	logname.append(suffix);
	log4cxx::helpers::Pool p;
	if (logname!="bugs") {
		log4cxx::LayoutPtr l(new log4cxx::PatternLayout(LAYOUT_2));
		log4cxx::RollingFileAppenderPtr r(new log4cxx::RollingFileAppender(l, logname,false));
		r->setMaxBackupIndex(5);
		r->setMaximumFileSize(10240);
		r->setBufferedIO(false);
		r->setBufferSize(1024);
		r->activateOptions(p);
		logger->addAppender(r);
		if (logname!="errors.log") {
			log4cxx::ConsoleAppenderPtr r2(new log4cxx::ConsoleAppender(l));
			logger->addAppender(r2);
		}
	}
	else {
		log4cxx::LayoutPtr l(new log4cxx::PatternLayout(LAYOUT_2));
		log4cxx::RollingFileAppenderPtr r(new log4cxx::RollingFileAppender(l, logname));
		r->setMaxBackupIndex(15);
		r->setMaximumFileSize(10240);
		r->setBufferedIO(false);
		r->setBufferSize(1024);
		r->activateOptions(p);
		logger->addAppender(r);
	}
	return logger;
}

log4cxx::LevelPtr get_level(unsigned short debug_level) {
	log4cxx::LevelPtr level;
	switch(debug_level) {
	case 0:
		level=log4cxx::Level::getOff();
		break;
	case 1:
		level=log4cxx::Level::getFatal();
		break;
	case 2:
		level=log4cxx::Level::getError();
		break;
	case 3:
		level=log4cxx::Level::getWarn();
		break;
	case 4:
		level=log4cxx::Level::getInfo();
		break;
	case 5:
		level=log4cxx::Level::getDebug();
		break;
	case 6:
		level=log4cxx::Level::getTrace();
		break;
	default:
		level=log4cxx::Level::getInfo();
		break;
	}
	return level;
}
}

#endif
} // namespace Alarmud

