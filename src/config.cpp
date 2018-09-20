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
#include "xsens.h"
#include "loop.h"

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
  Config(): config_() {
    pth config_file = get_config_file();
    log(level::info, "Using configuration file: %", config_file);
    if (fs::exists(config_file)) {
      load(config_file);
    }
    set_defaults();
    save(config_file);
  }

  pt::ptree config_;

  pth get_config_dir() {
#   ifdef _WIN32
    PWSTR szPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath);
    pth p(szPath);
    CoTaskMemFree(szPath);
    p = p / "Damen" / "SensorHub" / "Config";
    fs::create_directories(p);
#   else
    pth p("/etc/sensor_hub");
    try {
      fs::create_directories(p);
    }
    catch(fs::filesystem_error& fse) {
      p = getenv("HOME");
      p /= ".config/sensor_hub";
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

  template <typename Value>
  void set_default(const std::string& key, Value def) {
    config_.put(key, config_.get(key, def));
  }

  void set_defaults() {
    set_default("logging.level", "info");
    set_default("http.enabled", true);
    set_default("http.address", "localhost");
    set_default("http.port", 10080);
    set_default("http.css", "html { font-family: sans-serif; background-color: #85b0d0; }");
    set_default("modbus.enabled", true);
    set_default("modbus.port", 502);
    set_default("devices.count", 1);
    set_default("device0.type", "xsens_mti_g_710_usb");
    set_default("device0.name", "Xsens-MTi-G-710");
    set_default("device0.connection_string", "2639:0017,0");
    set_default("device0.enable_logging", true);
    set_default("device0.use_as_time_source", false);
  }
};


pt::ptree& get_config() {
  return Config::get_instance().get_config();
}


using Xsens_MTi_G_710_usb = Xsens_MTi_G_710<Usb, Context_provider>;
using Xsens_MTi_G_710_serial = Xsens_MTi_G_710<asio::serial_port, Context_provider>;

using Xsens_MTi_G_710_usb_factory = Device_factory<Xsens_MTi_G_710_usb>;
using Xsens_MTi_G_710_serial_factory = Device_factory<Xsens_MTi_G_710_serial>;

static auto& mti_g_710_usb_factory =
    add_device_factory("xsens_mti_g_710_usb", std::move(std::make_unique<Xsens_MTi_G_710_usb_factory>()));
static auto& mti_g_710_serial_factory =
    add_device_factory("xsens_mti_g_710_serial", std::move(std::make_unique<Xsens_MTi_G_710_serial_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
