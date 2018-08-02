/**
 * \file log.ccp
 * \brief Provide implementation for logging facility
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */


#include "log.h"

#include <sstream>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>
#include <boost/log/support/date_time.hpp>


using namespace boost::log;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", level);

static const char* level_strings[] = {
  " - DEBUG   - ",
  " - INFO    - ",
  " - WARNING - ",
  " - ERROR   - ",
};

std::ostream& operator<< (std::ostream& strm, level lvl)
{
  if (static_cast< std::size_t >(lvl) < sizeof(level_strings) / sizeof(*level_strings))
    strm << level_strings[static_cast< std::size_t >(lvl)];
  else
    strm << static_cast< int >(lvl);

  return strm;
}

formatting_ostream& operator<< (formatting_ostream& strm, to_log_manip<level, tag::severity> const& manip)
{
  level lvl = manip.get();
  if (static_cast< std::size_t >(lvl) < sizeof(level_strings) / sizeof(*level_strings))
    strm << level_strings[static_cast< std::size_t >(lvl)];
  else
    strm << static_cast< int >(lvl);

  return strm;
}

struct level_tag;

struct Logger {
  Logger(): log_() {
    add_common_attributes();
    add_file_log(
        keywords::file_name = "sensor_hub_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::format =  
        expressions::stream 
          << expressions::format_date_time<boost::posix_time::ptime>(
              "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
          << expressions::attr<level, level_tag>("Severity")
          << expressions::smessage
        );
  }
  Logger(Logger const&) = delete;
  void operator=(Logger const&) = delete;

  static Logger& get_instance() {
    static Logger instance; 
    return instance;
  }
  sources::severity_logger_mt<level>& get_log() {
    return log_;
  }
  private:
  sources::severity_logger_mt<level> log_;
};


sources::severity_logger_mt<level>& get_log() {
  return Logger::get_instance().get_log();
}
