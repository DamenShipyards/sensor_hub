/**
 * \file log.cpp
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
#include <memory>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/sinks.hpp>
#include <boost/log/core/core.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/async_frontend.hpp>

#ifdef _WIN32
#include <shlobj.h>
#include <objbase.h>
#endif

using namespace boost::log;
namespace pt = boost::posix_time;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", level)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)

static const char* level_strings[] = {
  " - DEBUG   - ",
  " - INFO    - ",
  " - WARNING - ",
  " - ERROR   - ",
};


struct device_log_exception_handler {

  typedef void result_type;

  void handle(std::string const& msg) const {
    int cur = pt::second_clock::local_time().time_of_day().hours();
    static int last = cur - 1;

    if (cur != last) {
      log(level::info, msg);
      last = cur;
    }
  }

  void operator() (std::exception const& e) const {
    handle(fmt::format("Exception in device logger: {:s}", e.what()));
  }

  void operator() () const {
    handle("Unknown exception in device logger");
  }
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

  sources::logger& get_device_log() {
    return device_log_;
  }

  void flush() {
    file_sink_->flush();
  }

  fs::path get_log_dir() {
#   ifdef _WIN32
    PWSTR szPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath);
    fs::path p(szPath);
    CoTaskMemFree(szPath);
    p = p / "Damen" / "SensorHub" / "Log";
    fs::create_directories(p);
#   else
    fs::path p("/var/log/sensor_hub");
    try {
      fs::create_directories(p);
    }
    catch(fs::filesystem_error& fse) {
      p = getenv("HOME");
      p /= ".local/var/log/sensor_hub";
      fs::create_directories(p);
    }
#   endif
    return p;
  }


  void set_device_log_dir(const fs::path& dir) {
    device_log_dir_ = dir;
  }


  fs::path get_device_log_dir() {
    if (device_log_dir_.empty()) {
#   ifdef _WIN32
      PWSTR szPath = nullptr;
      SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath);
      fs::path p(szPath);
      CoTaskMemFree(szPath);
      p = p / "Damen" / "SensorHub" / "DeviceLogs";
#   else
      fs::path p = get_log_dir() / "device_logs";
#   endif
      fs::create_directories(p);
      return p;
    }
    else {
      return device_log_dir_;
    }
  }


  void set_log_level(level lvl) {
    file_sink_->set_filter(!expressions::has_attr(tag_attr) && severity >= lvl);
  }


  std::string get_current_log_file() {
    return file_sink_->locked_backend()->get_current_file_name().string();
  }

private:

  Logger(): device_log_dir_(), log_(), device_log_() {
    add_common_attributes();
    core::get()->add_global_attribute("UtcStamp", boost::log::attributes::utc_clock());
    core::get()->set_exception_handler(make_exception_suppressor());

    file_sink_ =
        add_file_log(
            keywords::file_name = get_log_filename(),
            keywords::open_mode = std::ios_base::app,
            keywords::auto_flush = true,
            keywords::rotation_size = 8 * 1024 * 1024,
            keywords::format =
              expressions::stream
                << expressions::format_date_time<pt::ptime>(
                  "UtcStamp", "%Y-%m-%dT%H:%M:%S.%fZ")
                << expressions::attr<level, level_tag>("Severity")
                << expressions::smessage
        );
    auto collector = sinks::file::make_collector(
        keywords::target = get_log_dir(),
        keywords::max_files = 10
    );
    file_sink_->locked_backend()->set_file_collector(collector);
    file_sink_->locked_backend()->scan_for_files();
    file_sink_->set_filter(!expressions::has_attr(tag_attr));
  }

  fs::path device_log_dir_;
  sources::severity_logger_mt<level> log_;
  sources::logger device_log_;
  boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend> > file_sink_;

  fs::path get_log_filename() {
    return  get_log_dir() / "sensor_hub.%8N.log";
  }
};


sources::severity_logger_mt<level>& get_log() {
  return Logger::get_instance().get_log();
}

void flush_log() {
  Logger::get_instance().flush();
}


sources::logger& get_device_log() {
  return Logger::get_instance().get_device_log();
}


bool init_device_log(const std::string& device_id, const std::string& device_name) {
  try {
    auto log_dir = Logger::get_instance().get_device_log_dir();
    typedef sinks::asynchronous_sink<sinks::text_file_backend> file_sink;
    auto filename = log_dir / (device_id + ".%Y%m%dT%H%M%S."  + device_name + ".%8N.log");
    auto collector = sinks::file::make_collector(
        keywords::target = log_dir,
        keywords::min_free_space = 256 * 1024 * 1024,
        keywords::max_files = 32
        );
    auto sink = boost::make_shared<file_sink>(
        keywords::file_name = filename,
        keywords::open_mode = std::ios_base::app,
        keywords::rotation_size = 32 * 1024 * 1024,
        keywords::auto_flush = false
        );
    sink->set_exception_handler(make_exception_handler<std::exception>(device_log_exception_handler()));
    sink->locked_backend()->set_file_collector(collector);
    sink->locked_backend()->scan_for_files();
    sink->set_filter(tag_attr == device_name);
    core::get()->add_sink(sink);
  }
  catch (std::exception const& e) {
    log(level::debug, "Can't initialize device log: %", e.what());
    return false;
  }
  return true;
}


void set_log_level(level lvl) {
  Logger::get_instance().set_log_level(lvl);
}

void set_device_log_dir(const fs::path& dir) {
  Logger::get_instance().set_device_log_dir(dir);
}

std::string get_current_log_file() {
  return Logger::get_instance().get_current_log_file();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
