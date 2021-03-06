/**
 * \file log.h
 * \brief Provide interface to logging facility
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2019 Damen Shipyards
 * \license
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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
#include <boost/filesystem.hpp>

#include <fmt/format.h>

namespace fs = boost::filesystem;

// We define our own severity levels
enum class level {
  debug,
  info,
  warning,
  error,
  fatal,
};

extern boost::log::sources::severity_logger_mt<level>& get_log();
extern boost::log::sources::logger& get_device_log();
extern bool init_device_log(const std::string& device_id, const std::string& device_name, 
    const int max_files=32, const size_t file_size=64*1024*1024);
extern void set_log_level(level lvl);
extern void set_device_log_dir(const fs::path& dir);


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

inline void set_log_level(const std::string& slevel) {
  if (slevel == "debug") {
    set_log_level(level::debug);
  }
  else if (slevel == "info") {
    set_log_level(level::info);
  }
  else if (slevel == "warning") {
    set_log_level(level::warning);
  }
  else if (slevel == "error") {
    set_log_level(level::error);
  }
  else {
    log(level::error, "Unexpected log level: %", slevel);
  }
}

extern std::string get_current_log_file();

#ifdef DEBUG
#define DEBUGLOG(...) log(level::debug, __VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
