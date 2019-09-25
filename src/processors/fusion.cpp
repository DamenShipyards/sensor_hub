/**
 * \file fusion.cpp
 * \brief Provide sensor fusion processor
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
