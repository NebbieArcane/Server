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
//#define mudlog(level,...) { LOG_INFO(#level << " " << my_fmt FORMAT(__VA_ARGS__)); }
#define mudlog_LOG_ALWAYS(...) LOG4CXX_FATAL(logger,"LSERVICE  " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_SYSERR(...) {LOG4CXX_FATAL(errlogger,"LSYSERR  " << my_fmt FORMAT(__VA_ARGS__)); LOG4CXX_FATAL(logger,"LSYSERR  " << my_fmt FORMAT(__VA_ARGS__))}

#define mudlog_LOG_ERROR(...) {LOG4CXX_ERROR(errlogger,"LERROR " << my_fmt FORMAT(__VA_ARGS__)); LOG4CXX_ERROR(logger,"LERROR  " << my_fmt FORMAT(__VA_ARGS__))}
#define mudlog_LOG_CONNECT(...) LOG4CXX_ERROR(logger,"LCONNECT " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog_LOG_CHECK(...) LOG4CXX_WARN(logger,"LCHECK " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog_LOG_PLAYERS(...) LOG4CXX_INFO(logger,"LPLAYERS " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_MOBILES(...) LOG4CXX_INFO(logger,"LMOBILES " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog_LOG_SAVE(...) LOG4CXX_DEBUG(logger,"LSAVE " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_MAIL(...) LOG4CXX_DEBUG(logger,"LMAIL " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_RANK(...) LOG4CXX_DEBUG(logger,"LRANK " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_QUERY(...) LOG4CXX_DEBUG(querylogger,"LQUERY " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog_LOG_WHO(...) LOG4CXX_TRACE(logger,"LWHO " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_WORLD(...) LOG4CXX_TRACE(logger,"LWORLD " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog(level,...) {mudlog_##level(__VA_ARGS__);godTrace(level,__VA_ARGS__);}
#define buglog(level,...) {LOG4CXX_INFO(buglogger, #level << " " << my_fmt FORMAT(__VA_ARGS__));godTrace(level,__VA_ARGS__);}

// Note that TRACE level is compiled out in release (non-debug) mode.
#define LOG_TRACE(x) LOG4CXX_TRACE(logger, x) //6
#define LOG_DBG(x)   LOG4CXX_DEBUG(logger, x) //5
#define LOG_INFO(x)  LOG4CXX_INFO(logger, x) //4
#define LOG_WARN(x)  LOG4CXX_WARN(logger, x) //3
#define LOG_ALERT(x) {LOG4CXX_ERROR(logger, x);LOG4CXX_ERROR(errlogger, x)} //2
#define LOG_FATAL(x) {LOG4CXX_FATAL(logger, x);LOG4CXX_FATAL(errlogger, x)} //1

extern log4cxx::LoggerPtr logger;
extern log4cxx::LoggerPtr buglogger;
extern log4cxx::LoggerPtr errlogger;
extern log4cxx::LoggerPtr querylogger;
void godTrace(unsigned uType, const char* const szString, ...);
log4cxx::LoggerPtr log_configure(log4cxx::LoggerPtr &logger,string logname,string suffix,log4cxx::LevelPtr debugLevel,bool inConsole);
log4cxx::LevelPtr get_level(unsigned short debug_level);
} // namespace Alarmud
#endif /* LOGGING_HPP_ */

