/**
 * \file log.h
 * \brief Provide interface to logging facility
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

#include <boost/log/core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

// We define our own severity levels
enum class level {
  debug,
  info,
  warning,
  error,
};

extern boost::log::sources::severity_logger_mt<level>& get_log();
extern boost::log::sources::logger& get_device_log();
extern void init_device_log(const std::string& device_name);

template <typename M>
inline void log(level lvl, const M& msg) {
  BOOST_LOG_SEV(get_log(), lvl) << msg;
}

inline void format_to_stream(std::stringstream& ss, const char* format) {
  ss << format;
}

template<typename A, typename... As>
inline void format_to_stream(std::stringstream& ss, const char* format, const A& arg, const As&... args) {
  for ( ; *format != '\0'; format++) {
    if (*format == '%') {
      ss << arg;
      format_to_stream(ss, format + 1, args...);
      return;
    }
    ss << *format;
  }
}

template <typename... As>
inline void log(level lvl, const char* format, const As&... args) {
  std::stringstream ss;
  format_to_stream(ss, format, args...);
  log(lvl, ss.str());
}

template <typename M>
inline void log(const std::string& device_name, const M& msg) {
  BOOST_LOG_SCOPED_THREAD_TAG("Tag", device_name);
  BOOST_LOG(get_device_log()) << msg;
}

extern void flush_log();


#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
