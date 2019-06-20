/**
 * \file acceleration_history.cpp
 * \brief Provide implementation for acceleration history processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include "../modbus.h"

#include "acceleration_history.h"


std::string Acceleration_history::get_json() const {
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(get_name());
  writer.String("data"); writer.StartArray();
  for (auto&& peak: peaks_) {
    writer.StartObject();
    writer.String("time"); writer.Double(peak.start);
    writer.String("duration"); writer.Double(peak.duration);
    writer.String("peak"); writer.Double(peak.peak);
    writer.String("mean"); writer.Double(peak.mean);
    writer.String("rms"); writer.Double(peak.rms);
    writer.EndObject();
  }
  writer.EndArray();
  writer.EndObject();
  return sb.GetString();
}

uint16_t Acceleration_history::get_modbus_reg(size_t index) const {
  static Base_scale base_scale;

  size_t i = index / (Acceleration_peak::size() + 1);
  size_t m = index % (Acceleration_peak::size() + 1);
  if (i >= peaks_.size())
    return 0;
  const Acceleration_peak& peak = peaks_[i];
  switch (m) {
    case Acceleration_peak::f_start:
      return static_cast<uint16_t>(base_scale.scale_to_u32(Quantity::ut, peak.start) >> 16);
    case Acceleration_peak::f_start + 1:
      return static_cast<uint16_t>(base_scale.scale_to_u32(Quantity::ut, peak.start) && 0xFFFF);
    case Acceleration_peak::f_duration + 1:
      return base_scale.scale_to_u16(Quantity::du, peak.duration);
    default:
      return base_scale.scale_to_u16(Quantity::ax, peak[m - 1]);
  }
}


using Acceleration_history_factory = Processor_factory<Acceleration_history>;
static auto& acceleration_history_factory =
    add_processor_factory("acceleration_history", std::move(std::make_unique<Acceleration_history_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
