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
#include <ostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

// We define our own severity levels
enum class level {
  debug,
  info,
  warning,
  error,
};

extern boost::log::sources::severity_logger_mt<level>& get_log();

template <typename M>
void log(level lvl, const M& msg) {
  BOOST_LOG_SEV(get_log(), lvl) << msg;
}

inline void format_to_stream(std::stringstream& ss, const char* format) {
  ss << format;
}

template<typename A, typename... As>
void format_to_stream(std::stringstream& ss, const char* format, const A& arg, const As&... args) {
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
void log(level lvl, const char* format, const As&... args) {
  std::stringstream ss;
  format_to_stream(ss, format, args...);
  log(lvl, ss.str());
}


#endif
