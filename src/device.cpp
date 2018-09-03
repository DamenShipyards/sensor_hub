/**
 * \file device.cpp
 * \brief Provide implementation for device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "device.h"

int Device::seq_ = 0;

using Device_factory_map = std::map<std::string, Device_factory_ptr>;

/**
 * Return singleton device factory registry
 */
Device_factory_map& get_device_factory_map() {
  static Device_factory_map instance;
  return instance;
}

Device_factory_ptr& add_device_factory(const std::string& name, Device_factory_ptr&& factory) {
  Device_factory_map& factories = get_device_factory_map();
  factories.emplace(Device_factory_map::value_type(name, std::move(factory)));
  return factories[name];
}

Device_ptr create_device(const std::string& name) {
  try {
    decltype(auto) factory = get_device_factory_map().at(name);
    return factory->get_instance();
  }
  catch (std::out_of_range& e) {
    log(level::error, "Device with name % does not appear to be registered", name);
    throw;
  }
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
