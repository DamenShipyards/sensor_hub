/**
 * \file statistics.cpp
 * \brief Provide implementation for statistics processor
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


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include "../modbus.h"

#include "statistics.h"


std::string Statistics::get_json() const {
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
      const Statistic& stat = sit->second;
      writer.String("time"); writer.Double(stat.time);
      writer.String("samples"); writer.Int(stat.n);
      writer.String("mean"); writer.Double(stat.mean);
      writer.String("stddev"); writer.Double(sqrt(stat.variance));
      writer.EndObject();
    };
  }
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

uint16_t Statistics::get_modbus_reg(size_t index, const Base_scale& scaler) const {
  Quantity q = static_cast<Quantity>(index / (Statistic::size() + 1));
  size_t m = index % (Statistic::size() + 1);
  auto qit = statistics_.find(q);
  if (qit == statistics_.end())
    return 0;
  const Statistic& stat = qit->second;
  switch (m) {
    case Statistic::f_time:
      return static_cast<uint16_t>(scaler.scale_to_u32(Quantity::ut, stat.time) >> 16);
    case Statistic::f_time + 1:
      return static_cast<uint16_t>(scaler.scale_to_u32(Quantity::ut, stat.time) && 0xFFFF);
    case Statistic::f_n + 1:
      return static_cast<uint16_t>(stat.n);
    case Statistic::f_mean + 1:
      return static_cast<uint16_t>(scaler.scale_to_u16(q, stat.mean));
    case Statistic::f_stddev + 1:
      return static_cast<uint16_t>(scaler.scale_to_u16(q, sqrt(stat.variance)));
    default:
      // Shouldn't happen
      return 0;
  }
}


using Statistics_factory = Processor_factory<Statistics>;
static auto& statistics_factory =
    add_processor_factory("statistics", std::move(std::make_unique<Statistics_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
