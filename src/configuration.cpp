/**
 * \file configuration.cpp
 * \brief Provide application configuration implementation
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


#define _USE_MATH_DEFINES

#include "configuration.h"
#include "log.h"
#include "loop.h"

#include <boost/property_tree/ini_parser.hpp>

#ifdef _WIN32
#include <shlobj.h>
#include <objbase.h>
#endif


namespace prtr = boost::property_tree;
namespace fs = boost::filesystem;


struct Config_file {

  static Config_file& get_instance() {
    static Config_file instance{}; 
    return instance;
  }

  fs::path get_dir() {
#   ifdef _WIN32
    PWSTR szPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath);
    fs::path p(szPath);
    CoTaskMemFree(szPath);
    p = p / "Damen" / "SensorHub" / "Config";
    fs::create_directories(p);
#   else
    fs::path p("/etc/sensor_hub");
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


  fs::path get() {
    if (file_ != "") {
      return file_;
    }
    else {
      return  get_dir() / "sensor_hub.conf";
    }
  }

  void set(const fs::path& config_file) {
    file_ = config_file;
  }


private:
  fs::path file_;

};


struct Config {

  Config(Config const&) = delete;
  void operator=(Config const&) = delete;


  static Config& get_instance() {
    static Config instance{}; 
    return instance;
  }


  prtr::ptree& get() {
    return config_;
  }


  void load() {
    load(Config_file::get_instance().get());
  }


  void save() {
    save(Config_file::get_instance().get());
  }


private:
  prtr::ptree config_;
  fs::path loaded_;

  Config(): config_(), loaded_() {
    load();
  }


  void load(const fs::path& p) {
    if (p != loaded_) {
      log(level::info, "Using configuration file: %", p);

      if (fs::exists(p)) {
        prtr::read_ini(p.string(), config_);
      }
      else {
        log(level::warning, "Configuration file % doesn't exist", p);
      }
      set_defaults();      
      loaded_ = p;
    }
  }


  void save(const fs::path& p) {
    log(level::info, "Writing configuration to: %", p);
    prtr::write_ini(p.string(), config_);
  }


  template <typename Value>
  void set_default(const std::string& key, Value def) {
    config_.put(key, config_.get(key, def));
  }


  void set_defaults() {
    set_default("logging.level", "info");
    set_default("logging.device_log_dir", "");

    set_default("http.enabled", true);
    set_default("http.address", "localhost");
    set_default("http.port", 16080);
    set_default("http.css", "html { font-family: sans-serif; background-color: #85b0d0; }");

    set_default("modbus.enabled", true);
    set_default("modbus.port", 16502);


    set_default("devices.count", 2);

    set_default("device0.type", "xsens_mti_g_710_usb");
    set_default("device0.name", "MTi-G-710");
    set_default("device0.enabled", false);
    set_default("device0.connection_string", "auto");
    set_default("device0.enable_logging", false);
    set_default("device0.use_as_time_source", false);
    set_default("device0.options", "{}");

    set_default("device1.type", "ublox_neo_m8u_serial");
    set_default("device1.name", "NEO-M8U");
    set_default("device1.enabled", false);
    set_default("device1.connection_string", "auto");
    set_default("device1.enable_logging", false);
    set_default("device1.use_as_time_source", false);
    set_default("device1.options", "{ \"dyn_model\": \"sea\", \"gnss_type\": \"glonass\" }");

    set_default("processors.count", 6);

    set_default("processor0.type", "acceleration_history");
    set_default("processor0.name", "Xsens-Acceleration-Peaks");
    set_default("processor0.parameters", "value_threshold=0.4,duration_threshold=0.5,item_count=5,direction=3");
    set_default("processor0.device", "MTi-G-710");

    set_default("processor1.type", "statistics");
    set_default("processor1.name", "Xsens-1-Sec-Statistics");
    set_default("processor1.parameters", "period=1.0");
    set_default("processor1.device", "MTi-G-710");

    set_default("processor2.type", "statistics");
    set_default("processor2.name", "Xsens-10-Sec-Statistics");
    set_default("processor2.parameters", "period=10");
    set_default("processor2.device", "MTi-G-710");
    set_default("processor2.filter", "hmsl,ro,pi,ya,vy,vz");

    set_default("processor3.type", "statistics");
    set_default("processor3.name", "Xsens-1-Min-Statistics");
    set_default("processor3.parameters", "period=60");
    set_default("processor3.device", "MTi-G-710");
    set_default("processor3.filter", "hmsl,ro,pi,ya,vy,vz");

    set_default("processor4.type", "statistics");
    set_default("processor4.name", "Xsens-10-Min-Statistics");
    set_default("processor4.parameters", "period=600");
    set_default("processor4.device", "MTi-G-710");
    set_default("processor4.filter", "hmsl,ro,pi,ya,vy,vz");

    set_default("processor5.type", "fusion");
    set_default("processor5.name", "Ublox-Fusion");
    set_default("processor5.parameters", "period=0.2");
    set_default("processor5.device", "NEO-M8U");
  }
};


prtr::ptree& get_config() {
  return Config::get_instance().get();
}


void update_config() {
  return Config::get_instance().save();
}


fs::path get_config_file() {
  return Config_file::get_instance().get();
}


void set_config_file(const fs::path config_file, bool reload) {
  Config_file::get_instance().set(config_file);
  if (reload) {
    Config::get_instance().load();
  }
}



// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
