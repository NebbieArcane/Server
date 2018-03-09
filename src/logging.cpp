/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __LOGGING_HPP
#define __LOGGING_HPP
/***************************  System  include ************************************/
#include <stdarg.h>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/consoleappender.h>
#include <boost/filesystem.hpp>
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
#include "general.hpp"
#if HAS_LOG
namespace Alarmud {


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
constexpr auto LAYOUT_1="%d{yy-MM-dd HH:mm:ss.SSS} %5p [...%.20F at %5L] - %m%n";
constexpr auto LAYOUT_2="%d{yy-MM-dd HH:mm:ss.SSS} [...%.16F at %5L] - %m%n";
log4cxx::LoggerPtr log_configure(log4cxx::LoggerPtr &logger,string logname,string suffix,log4cxx::LevelPtr debugLevel,bool inConsole) {
	bool append = (logname=="bugs");
	int numLogs= (logname=="bugs")?5:20ls;
	string logfile(boost::filesystem::current_path().string());
	logfile.append("/").append(logname).append(suffix);
	log4cxx::helpers::Pool p;
	log4cxx::LayoutPtr l(new log4cxx::PatternLayout(LAYOUT_2));
	log4cxx::RollingFileAppenderPtr r(new log4cxx::RollingFileAppender(l, logfile,append));
	r->setMaxBackupIndex(numLogs);
	r->setMaximumFileSize("200M");
	r->setBufferedIO(true);
	r->setBufferSize(1024);
	r->activateOptions(p);
	logger->addAppender(r);
	if (inConsole) {
		cout << "Logger " << logname << "." << suffix << " on console " << std::endl;
		log4cxx::ConsoleAppenderPtr r2(new log4cxx::ConsoleAppender(l));
		logger->addAppender(r2);
	}
	logger->setLevel(debugLevel);
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
} // namespace Alarmud
#endif
#endif // __LOGGING_HPP

