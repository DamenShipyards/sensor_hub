/**
 * \file processor.cpp
 * \brief Provide implementation for processor data processors
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */


#include "processor.h"
#include "modbus.h"


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>


using Processor_factory_map = std::map<std::string, Processor_factory_ptr>;

/**
 * Return singleton processor factory registry
 */
Processor_factory_map& get_processor_factory_map() {
  static Processor_factory_map instance;
  return instance;
}

Processor_factory_ptr& add_processor_factory(const std::string& name, Processor_factory_ptr&& factory) {
  Processor_factory_map& factories = get_processor_factory_map();
  factories.emplace(Processor_factory_map::value_type(name, std::move(factory)));
  return factories[name];
}

Processor_ptr create_processor(const std::string& name) {
  try {
    decltype(auto) factory = get_processor_factory_map().at(name);
    return factory->get_instance();
  }
  catch (std::out_of_range&) {
    log(level::error, "Processor with name % does not appear to be registered", name);
    throw;
  }
}

std::string Statistics::get_json() {
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(get_name());
  writer.String("data"); writer.StartObject();
  for (auto it = Quantity_iter::begin(); it != Quantity_iter::end(); ++it) {
    auto sit = statistics_.begin();
    if ((sit = statistics_.find(*it)) != statistics_.end()) {
      writer.String(get_quantity_name(*it)); writer.StartObject();
      Statistic& stat = sit->second;
      writer.String("time"); writer.Double(stat.time);
      writer.String("samples"); writer.Double(stat.n);
      writer.String("mean"); writer.Double(stat.mean);
      writer.String("variance"); writer.Double(stat.variance);
      writer.EndObject();
    };
  }
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

uint16_t Statistics::get_modbus_reg(int index) {
  static Base_scale base_scale;

  Quantity q = static_cast<Quantity>(index / (Statistic::size() + 1));
  int m = index % (Statistic::size() + 1);
  auto qit = statistics_.find(q);
  if (qit == statistics_.end())
    return 0;
  Statistic& stat = qit->second;
  switch (m) {
    case Statistic::f_time:
      return static_cast<uint16_t>(static_cast<uint32_t>(stat.time) >> 16);
    case Statistic::f_time + 1:
      return static_cast<uint16_t>(static_cast<uint32_t>(stat.time) && 0xFFFF);
    case Statistic::f_n + 1:
      return static_cast<uint16_t>(stat.n);
    case Statistic::f_mean + 1:
      base_scale.scale_to_u16(q, stat.mean);
    case Statistic::f_variance + 1:
      // Return RMS/stddev instead of variance
      base_scale.scale_to_u16(q, sqrt(stat.variance));
    default:
      // Shouldn't happen
      return 0;
  }
}

std::string Acceleration_history::get_json() {
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(get_name());
  writer.String("data"); writer.StartObject();
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

uint16_t Acceleration_history::get_modbus_reg(int index) {
  return 0;
}
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
