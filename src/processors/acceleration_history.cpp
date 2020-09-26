/**
 * \file acceleration_history.cpp
 * \brief Provide implementation for acceleration history processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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

uint16_t Acceleration_history::get_modbus_reg(size_t index, const Base_scale& scaler) const {
  size_t i = index / (Acceleration_peak::size() + 1);
  size_t m = index % (Acceleration_peak::size() + 1);
  if (i >= peaks_.size())
    return 0;
  const Acceleration_peak& peak = peaks_[i];
  switch (m) {
    case Acceleration_peak::f_start:
      return static_cast<uint16_t>(scaler.scale_to<uint32_t>(Quantity::ut, peak.start) >> 16);
    case Acceleration_peak::f_start + 1:
      return static_cast<uint16_t>(scaler.scale_to<uint32_t>(Quantity::ut, peak.start) & 0xFFFF);
    case Acceleration_peak::f_duration + 1:
      return scaler.scale_to<uint16_t>(Quantity::du, peak.duration);
    default:
      return scaler.scale_to<uint16_t>(Quantity::ax, peak[m - 1]);
  }
}


using Acceleration_history_factory = Processor_factory<Acceleration_history>;
static auto& acceleration_history_factory =
    add_processor_factory("acceleration_history", std::move(std::make_unique<Acceleration_history_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
