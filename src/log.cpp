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

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>
#include <boost/log/support/date_time.hpp>

#ifdef _WIN32
#include <shlobj.h>
#include <objbase.h>
#endif

using namespace boost::log;
namespace fs = boost::filesystem;
using pth = boost::filesystem::path;

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
  Logger(): log_() {
    add_common_attributes();
    add_file_log(
      keywords::file_name = get_log_filename(),
      keywords::open_mode = std::ios_base::app,
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::format =  
          expressions::stream 
            << expressions::format_date_time<boost::posix_time::ptime>(
              "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << expressions::attr<level, level_tag>("Severity")
            << expressions::smessage
    );
  }

  sources::severity_logger_mt<level> log_;
  
  pth get_log_dir() {
#   ifdef _WIN32
    PWSTR szPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, NULL, 0, &szPath);
    pth p(szPath);
    CoTaskMemFree(szPath);
    p /= "Damen";
    p /= "SensorHub";
    fs::create_directories(p);
#   else
    pth p("/var/log/sensor_hub");
    try {
      fs::create_directories(p);
    }
    catch(fs::filesystem_error) {
      p = getenv("HOME");
      p /= ".local/var/log/sensor_hub";
      fs::create_directories(p);
    }
#   endif
    return p;
  }
  pth get_log_filename() {
    return  get_log_dir() / "sensor_hub_%N.log";
  }
};


sources::severity_logger_mt<level>& get_log() {
  return Logger::get_instance().get_log();
}
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
