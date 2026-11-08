/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
 * logging.hpp
 *
 *  Created on: 10 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */
 #ifndef LOGGING_HPP_
 #define LOGGING_HPP_
 /***************************  System  include ************************************/
 #include <string>
 #include <log4cxx/logger.h>
 #include <boost/format.hpp>
 /***************************  Local    include ************************************/
 namespace Alarmud {
 using std::string;
 extern boost::format  my_fmt(const std::string &f_string);

 /* Formatta UNA volta (printf-style via boost::format/my_fmt) e ritorna la
  * stringa pronta. Gli argomenti sono normali argomenti di funzione: vengono
  * valutati una sola volta, accettano qualsiasi tipo "streamable"
  * (std::string, string_view, ...), ?: senza parentesi e piu' di 10 argomenti.
  * Non solleva mai eccezioni: se il formato e' errato (es. "%l ", "%" finale)
  * ritorna il formato grezzo, cosi' un log sbagliato non crasha il MUD. */
 template <typename... Args>
 [[nodiscard]] std::string log_format(const std::string &f_string, const Args &... args) {
 	try {
 		boost::format bf = my_fmt(f_string);
 		(void)(bf % ... % args);
 		return bf.str();
 	} catch(...) {
 		return f_string;
 	}
 }

 /* Le macro mudlog_LOG_xxx ricevono la stringa GIA' formattata (std::string). */
 #define mudlog_LOG_ALWAYS(msg) LOG4CXX_FATAL(Alarmud::logger,"LSERVICE  " << (msg))
 #define mudlog_LOG_SYSERR(msg) {LOG4CXX_FATAL(Alarmud::errlogger,"LSYSERR  " << (msg)); LOG4CXX_FATAL(Alarmud::logger,"LSYSERR  " << (msg));}

 #define mudlog_LOG_ERROR(msg) {LOG4CXX_ERROR(Alarmud::errlogger,"LERROR " << (msg)); LOG4CXX_ERROR(Alarmud::logger,"LERROR  " << (msg));}
 #define mudlog_LOG_CONNECT(msg) LOG4CXX_ERROR(Alarmud::logger,"LCONNECT " << (msg))

 #define mudlog_LOG_CHECK(msg) LOG4CXX_WARN(Alarmud::logger,"LCHECK " << (msg))

 #define mudlog_LOG_PLAYERS(msg) LOG4CXX_INFO(Alarmud::logger,"LPLAYERS " << (msg))
 #define mudlog_LOG_MOBILES(msg) LOG4CXX_INFO(Alarmud::logger,"LMOBILES " << (msg))

 #define mudlog_LOG_SAVE(msg) LOG4CXX_DEBUG(Alarmud::logger,"LSAVE " << (msg))
 #define mudlog_LOG_MAIL(msg) LOG4CXX_DEBUG(Alarmud::logger,"LMAIL " << (msg))
 #define mudlog_LOG_RANK(msg) LOG4CXX_DEBUG(Alarmud::logger,"LRANK " << (msg))
 #define mudlog_LOG_QUERY(msg) LOG4CXX_DEBUG(Alarmud::querylogger,"LQUERY " << (msg))

 #define mudlog_LOG_WHO(msg) LOG4CXX_TRACE(Alarmud::logger,"LWHO " << (msg))
 #define mudlog_LOG_WORLD(msg) LOG4CXX_TRACE(Alarmud::logger,"LWORLD " << (msg))

 /* Formatta una sola volta, poi log4cxx e godTrace usano la stessa stringa. */
 #define mudlog(level,...) do { const std::string mudlog_msg_ = Alarmud::log_format(__VA_ARGS__); mudlog_##level(mudlog_msg_); Alarmud::godTrace(level, mudlog_msg_); } while(0)
 #define buglog(level,...) do { const std::string mudlog_msg_ = Alarmud::log_format(__VA_ARGS__); LOG4CXX_INFO(Alarmud::buglogger, #level << " " << mudlog_msg_); Alarmud::godTrace(level, mudlog_msg_); } while(0)

 // Note that TRACE level is compiled out in release (non-debug) mode.
 #define LOG_TRACE(x) LOG4CXX_TRACE(Alarmud::logger, x) //6
 #define LOG_DBG(x)   LOG4CXX_DEBUG(Alarmud::logger, x) //5
 #define LOG_INFO(x)  LOG4CXX_INFO(Alarmud::logger, x) //4
 #define LOG_WARN(x)  LOG4CXX_WARN(Alarmud::logger, x) //3
 #define LOG_ALERT(x) {LOG4CXX_ERROR(Alarmud::logger, x);LOG4CXX_ERROR(Alarmud::errlogger, x);} //2
 #define LOG_FATAL(x) {LOG4CXX_FATAL(Alarmud::logger, x);LOG4CXX_FATAL(Alarmud::errlogger, x);} //1
 
 extern log4cxx::LoggerPtr logger;
 extern log4cxx::LoggerPtr buglogger;
 extern log4cxx::LoggerPtr errlogger;
 extern log4cxx::LoggerPtr querylogger;
 void godTrace(unsigned uType, const std::string &message);
 log4cxx::LoggerPtr log_configure(log4cxx::LoggerPtr &logger,string logname,string suffix,log4cxx::LevelPtr debugLevel,bool inConsole);
 log4cxx::LevelPtr get_level(unsigned short debug_level);
 } // namespace Alarmud
 #endif /* LOGGING_HPP_ */
 
 