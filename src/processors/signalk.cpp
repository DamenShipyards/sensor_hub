/**
 * \file signalk.cpp
 * \brief Provide sensor signalk processor
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
#include "signalk.h"


std::string SignalK::get_json() const {
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

std::string SignalK::get_delta(const Stamped_quantity& q){
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("context"); writer.String("vessels.urn:mrn:imo:mmsi:234567890");
  writer.String("updates"); writer.StartArray();
  writer.StartObject();
  writer.String("$source"); writer.String("sensor_hub");
  writer.String("timestamp"); writer.String(timestamp_to_string(q.stamp) + "Z");
  writer.String("values");  writer.StartArray();
  writer.StartObject();
  writer.String("path"); writer.String(get_path(q.quantity));
  writer.String("value"); get_value(q,writer);
  writer.EndObject();
  writer.EndArray();
  writer.EndObject();
  writer.EndArray();
  writer.EndObject();
  return sb.GetString();
}

std::string SignalK::get_path(const Quantity& q){
  if (q== Quantity::ut) {
    return "navigation.datetime";
  }
  else if (q == Quantity::la) {
    return "navigation.position";
  }
  else if (q == Quantity::lo) {
    return "navigation.position";
  }
  else {
    return "";
  }

}

void SignalK::get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer){
  Quantity quantity = q.quantity;
  if (quantity == Quantity::ut) {
    std::string time =  timestamp_to_string(q.value) + "Z";
    writer.String(time);
  }else {
    writer.Double(q.value);
  }
}

using SignalK_factory = Processor_factory<SignalK>;
static auto& signalk_factory =
    add_processor_factory("signalk", std::move(std::make_unique<SignalK_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
