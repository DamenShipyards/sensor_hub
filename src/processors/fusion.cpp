/**
 * \file fusion.cpp
 * \brief Provide sensor fusion processor
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
#include "fusion.h"


std::string Fusion::get_json() const {
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

uint16_t Fusion::get_modbus_reg(size_t, const Base_scale&) const {
  return 0; 
}


using Fusion_factory = Processor_factory<Fusion>;
static auto& fusion_factory =
    add_processor_factory("fusion", std::move(std::make_unique<Fusion_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
