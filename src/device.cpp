/**
 * \file device.cpp
 * \brief Provide implementation for device base class
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


#include "device.h"


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>

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
    log(level::info, "Created device with type \"%\"", name);
    return factory->get_instance();
  }
  catch (std::out_of_range&) {
    log(level::error, "Device with type \"%\" does not appear to be registered", name);
    throw;
  }
}



prtr::ptree get_device_tree(const Device& device) {
  prtr::ptree tree;
  tree.put("name", device.get_name());
  tree.put("id", device.get_id());
  tree.put("connected", device.is_connected());
  tree.put("time", get_time());
  for (auto it = Quantity_iter::begin(); it != Quantity_iter::end(); ++it) {
    Stamped_value sample;
    if (device.get_sample(*it, sample)) {
      tree.put(fmt::format("data.{}.time", get_quantity_name(*it)),sample.stamp);
      tree.put(fmt::format("data.{}.value", get_quantity_name(*it)),sample.value);
    };
  }
  return tree;
}


std::string get_device_json(const Device& device) {
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(device.get_name());
  writer.String("id"); writer.String(device.get_id());
  writer.String("connected"); writer.Bool(device.is_connected());
  writer.String("time"); writer.Double(get_time());
  writer.String("data"); writer.StartObject();
  for (auto it = Quantity_iter::begin(); it != Quantity_iter::end(); ++it) {
    Stamped_value sample;
    if (device.get_sample(*it, sample)) {
      writer.String(get_quantity_name(*it)); writer.StartObject();
      writer.String("time"); writer.Double(sample.stamp);
      writer.String("value"); writer.Double(sample.value);
      writer.EndObject();
    };
  }
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
