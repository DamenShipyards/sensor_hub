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

#define _USE_MATH_DEFINES

#include "config.h"
#include "log.h"
#include "loop.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <shlobj.h>
#include <objbase.h>
#endif

namespace prtr = boost::property_tree;
namespace fs = boost::filesystem;

struct Config {
  Config(Config const&) = delete;
  void operator=(Config const&) = delete;

  static Config& get_instance() {
    static Config instance; 
    return instance;
  }

  prtr::ptree& get_config() {
    return config_;
  }

  void save_config() {
    save(get_config_file());
  }

private:
  Config(): config_() {
    fs::path config_file = get_config_file();
    log(level::info, "Using configuration file: %", config_file);
    if (fs::exists(config_file)) {
      load(config_file);
    }
    set_defaults();      
  }

  prtr::ptree config_;

  fs::path get_config_dir() {
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

  fs::path get_config_file() {
    return  get_config_dir() / "sensor_hub.conf";
  }

  void load(const fs::path& p) {
    prtr::read_ini(p.string(), config_);
  }

  void save(const fs::path& p) {
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

    set_default("modbus.ut_min", 0.0);
    set_default("modbus.ut_max", 1.0 * 0x100000000);
    set_default("modbus.du_min", 0.0);
    set_default("modbus.du_max", 0.001 * 0x100000000);

    set_default("modbus.la_min", -M_PI);
    set_default("modbus.la_max",  M_PI);
    set_default("modbus.lo_min", -M_PI);
    set_default("modbus.lo_max",  M_PI);

    set_default("modbus.h1_min", -327.68);
    set_default("modbus.h1_max",  327.68);
    set_default("modbus.h2_min", -327.68);
    set_default("modbus.h2_max",  327.68);

    set_default("modbus.hdg_min", 0);
    set_default("modbus.hdg_max", 2 * M_PI);
    set_default("modbus.crs_min", 0);
    set_default("modbus.crs_max", 2 * M_PI);

    set_default("modbus.mx_min", -0.00032768);
    set_default("modbus.mx_max",  0.00032768);
    set_default("modbus.my_min", -0.00032768);
    set_default("modbus.my_max",  0.00032768);
    set_default("modbus.mz_min", -0.00032768);
    set_default("modbus.mz_max",  0.00032768);

    set_default("modbus.ax_min", -32.768);
    set_default("modbus.ax_max",  32.768);
    set_default("modbus.ay_min", -32.768);
    set_default("modbus.ay_max",  32.768);
    set_default("modbus.az_min", -32.768);
    set_default("modbus.az_max",  32.768);
    set_default("modbus.vx_min", -32.768);
    set_default("modbus.vx_max",  32.768);
    set_default("modbus.vy_min", -32.768);
    set_default("modbus.vy_max",  32.768);
    set_default("modbus.vz_min", -32.768);
    set_default("modbus.vz_max",  32.768);

    set_default("modbus.ro_min", -M_PI);
    set_default("modbus.ro_max",  M_PI);
    set_default("modbus.pi_min", -M_PI);
    set_default("modbus.pi_max",  M_PI);
    set_default("modbus.ya_min", -M_PI);
    set_default("modbus.ya_max",  M_PI);

    set_default("modbus.q1.min", -1.0);
    set_default("modbus.q1.max",  1.0);
    set_default("modbus.q2.min", -1.0);
    set_default("modbus.q2.max",  1.0);
    set_default("modbus.q3.min", -1.0);
    set_default("modbus.q3.max",  1.0);
    set_default("modbus.q4.min", -1.0);
    set_default("modbus.q4.max",  1.0);

    set_default("modbus.rr_min", -M_PI);
    set_default("modbus.rr_max",  M_PI);
    set_default("modbus.pr_min", -M_PI);
    set_default("modbus.pr_max",  M_PI);
    set_default("modbus.yr_min", -M_PI);
    set_default("modbus.yr_max",  M_PI);

    set_default("modbus.fax_min", -32.768);
    set_default("modbus.fax_max",  32.768);
    set_default("modbus.fay_min", -32.768);
    set_default("modbus.fay_max",  32.768);
    set_default("modbus.faz_min", -32.768);
    set_default("modbus.faz_max",  32.768);

    set_default("modbus.du_min", 0.0);
    set_default("modbus.du_max", 6553.6);

    set_default("modbus.hacc_min", 0.0);
    set_default("modbus.hacc_max", 655.36);
    set_default("modbus.vacc_min", 0.0);
    set_default("modbus.vacc_max", 655.36);
    set_default("modbus.sacc_min", 0.0);
    set_default("modbus.sacc_max", 655.36);
    set_default("modbus.cacc_min", 0.0);
    set_default("modbus.cacc_max", 655.36);
    set_default("modbus.racc_min", 0.0);
    set_default("modbus.racc_max", 655.36);
    set_default("modbus.pacc_min", 0.0);
    set_default("modbus.pacc_max", 655.36);
    set_default("modbus.yacc_min", 0.0);
    set_default("modbus.yacc_max", 655.36);
    set_default("modbus.hdac_min", 0.0);
    set_default("modbus.hdac_max", 655.36);

    set_default("modbus.rax_min", -32.768);
    set_default("modbus.rax_max",  32.768);
    set_default("modbus.ray_min", -32.768);
    set_default("modbus.ray_max",  32.768);
    set_default("modbus.raz_min", -32.768);
    set_default("modbus.raz_max",  32.768);

    set_default("modbus.rrr_min", -M_PI);
    set_default("modbus.rrr_max",  M_PI);
    set_default("modbus.rpr_min", -M_PI);
    set_default("modbus.rpr_max",  M_PI);
    set_default("modbus.ryr_min", -M_PI);
    set_default("modbus.ryr_max",  M_PI);

    set_default("modbus.rmx_min", -0.00032768);
    set_default("modbus.rmx_max",  0.00032768);
    set_default("modbus.rmy_min", -0.00032768);
    set_default("modbus.rmy_max",  0.00032768);
    set_default("modbus.rmz_min", -0.00032768);
    set_default("modbus.rmz_max",  0.00032768);

    set_default("modbus.gtmp_min", 0.0);
    set_default("modbus.gtmp_max", 655.36);
    set_default("modbus.stmp_min", 0.0);
    set_default("modbus.stmp_max", 655.36);
    set_default("modbus.wtmp_min", 0.0);
    set_default("modbus.wtmp_max", 655.36);
    set_default("modbus.atmp_min", 0.0);
    set_default("modbus.atmp_max", 655.36);
    set_default("modbus.etmp_min", 0.0);
    set_default("modbus.etmp_max", 6553.6);
    set_default("modbus.otmp_min", 0.0);
    set_default("modbus.otmp_max", 655.36);


    set_default("devices.count", 2);

    set_default("device0.type", "xsens_mti_g_710_usb");
    set_default("device0.name", "Xsens-MTi-G-710");
    set_default("device0.enabled", false);
    set_default("device0.connection_string", "auto");
    set_default("device0.enable_logging", false);
    set_default("device0.use_as_time_source", false);
    set_default("device0.options", "{}");

    set_default("device1.type", "ublox_neo_m8u_serial");
    set_default("device1.name", "Ublox-NEO-M8U");
    set_default("device1.enabled", false);
    set_default("device1.connection_string", "auto");
    set_default("device1.enable_logging", false);
    set_default("device1.use_as_time_source", false);
    set_default("device1.options", "{ \"dyn_model\": \"sea\", \"gnss_type\": \"glonass\" }");

    set_default("processors.count", 5);

    set_default("processor0.type", "acceleration_history");
    set_default("processor0.name", "Xsens-Acceleration-Peaks");
    set_default("processor0.parameters", "value_threshold=0.4,duration_threshold=0.5,item_count=5,direction=3");
    set_default("processor0.device", "Xsens-MTi-G-710");

    set_default("processor1.type", "statistics");
    set_default("processor1.name", "Xsens-1-Sec-Statistics");
    set_default("processor1.parameters", "period=1.0");
    set_default("processor1.device", "Xsens-MTi-G-710");

    set_default("processor2.type", "statistics");
    set_default("processor2.name", "Xsens-10-Sec-Statistics");
    set_default("processor2.parameters", "period=10");
    set_default("processor2.device", "Xsens-MTi-G-710");
    set_default("processor2.filter", "h2,ro,pi,ya,vy,vz");

    set_default("processor3.type", "statistics");
    set_default("processor3.name", "Xsens-1-Min-Statistics");
    set_default("processor3.parameters", "period=60");
    set_default("processor3.device", "Xsens-MTi-G-710");
    set_default("processor3.filter", "h2,ro,pi,ya,vy,vz");

    set_default("processor4.type", "statistics");
    set_default("processor4.name", "Xsens-10-Min-Statistics");
    set_default("processor4.parameters", "period=600");
    set_default("processor4.device", "Xsens-MTi-G-710");
    set_default("processor4.filter", "h2,ro,pi,ya,vy,vz");
  }
};


prtr::ptree& get_config() {
  return Config::get_instance().get_config();
}

void update_config() {
  return Config::get_instance().save_config();
}



// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
