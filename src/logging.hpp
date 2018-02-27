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
#include "config.hpp"
#include "boost/format.hpp"
#include <string>
using std::string;
#include <log4cxx/logger.h>
extern boost::format  my_fmt(const std::string &f_string);
//#define mudlog(level,...) { LOG_INFO(#level << " " << my_fmt FORMAT(__VA_ARGS__)); }
#define mudlog_LOG_SYSERR(level,...) LOG4CXX_FATAL(::logger,"LSYSERR  " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_ERROR(level,...) LOG4CXX_ERROR(::logger,"LERROR " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_CHECK(level,...) LOG4CXX_WARN(::logger,"LCHECK " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_PLAYERS(level,...) LOG4CXX_DEBUG(::logger,"LPLAYERS " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_MOBILES(level,...) LOG4CXX_DEBUG(::logger,"LMOBILES " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_CONNECT(level,...) LOG4CXX_INFO(::logger,"LCONNECT " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_WHO(level,...) LOG4CXX_INFO(::logger,"LWHO " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_SAVE(level,...) LOG4CXX_TRACE(::logger,"LSAVE " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_MAIL(level,...) LOG4CXX_TRACE(::logger,"LMAIL " << my_fmt FORMAT(__VA_ARGS__))
#define mudlog_LOG_RANK(level,...) LOG4CXX_TRACE(::logger,"LRANK " << my_fmt FORMAT(__VA_ARGS__))

#define mudlog(level,...) {mudlog_##level(level,__VA_ARGS__);godTrace(level,__VA_ARGS__);}
#define buglog(level,...) {LOG4CXX_INFO(::buglogger, #level << my_fmt FORMAT(__VA_ARGS__));godTrace(level,__VA_ARGS__);}



// Note that TRACE level is compiled out in release (non-debug) mode.
#define LOG_TRACE(x) LOG4CXX_TRACE(::logger, x)
#define LOG_DBG(x) LOG4CXX_DEBUG(::logger, x)
#define LOG_INFO(x) LOG4CXX_INFO(::logger, x)
#define LOG_WARN(x) LOG4CXX_WARN(::logger, x)
#define LOG_ALERT(x) LOG4CXX_ERROR(::logger, x)
#define LOG_FATAL(x) LOG4CXX_FATAL(::logger, x)
extern log4cxx::LoggerPtr logger;
extern log4cxx::LoggerPtr buglogger;
void godTrace(unsigned uType, const char* const szString, ...);
namespace Alarmud {
void log_init(string log_filename,unsigned short debug_level);
log4cxx::LoggerPtr log_configure(log4cxx::LoggerPtr &logger,string logname,string suffix);
log4cxx::LevelPtr get_level(unsigned short debug_level);
}

#endif /* LOGGING_HPP_ */
