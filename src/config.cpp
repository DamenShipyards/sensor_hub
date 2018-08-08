/**
 * \file config.cpp
 * \brief Provide application configuration implementation
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "config.h"
#include "log.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>

#ifdef _WIN32
#include <shlobj.h>
#include <objbase.h>
#endif

namespace fs = boost::filesystem;
using pth = boost::filesystem::path;
namespace pt = boost::property_tree;

struct Config {
  Config(): config_() {
    set_default();
    pth config_file = get_config_file();
    log(level::info, "Using configuration file: %", config_file);
    if (fs::exists(config_file)) {
      load(config_file);
    }
    save(config_file);
  }
  Config(Config const&) = delete;
  void operator=(Config const&) = delete;

  static Config& get_instance() {
    static Config instance; 
    return instance;
  }
  pt::ptree& get_config() {
    return config_;
  }
private:
  pt::ptree config_;

  pth get_config_dir() {
#   ifdef _WIN32
    PWSTR szPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_AppData, NULL, 0, &szPath);
    pth p(szPath);
    CoTaskMemFree(szPath);
    p /= "Damen";
    p /= "SensorHub";
    fs::create_directories(p);
#   else
    pth p("/etc/sensor_hub");
    try {
      fs::create_directories(p);
    }
    catch(fs::filesystem_error) {
      p = getenv("HOME");
      p /= ".local/etc/sensor_hub";
      fs::create_directories(p);
    }
#   endif
    return p;
  }

  pth get_config_file() {
    return  get_config_dir() / "sensor_hub.conf";
  }

  void load(const pth& p) {
    pt::read_ini(p.string(), config_);
  }

  void save(const pth& p) {
    pt::write_ini(p.string(), config_);
  }

  void set_default() {
    config_.put("www.port", 10080);
  }
};

pt::ptree& get_config() {
  return Config::get_instance().get_config();
}
