/**
 * \file signalk_converter.cpp
 * \brief Provide implementation for acceleration history processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards\n
 *            Copyright (C) 2020-2022 Orca Software
 *
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
#include <rapidjson/writer.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "signalk_converter.h"

bool SignalK_converter::produces_delta(const Stamped_quantity& q){
    add_to_cache(q);
    Quantity quantity = q.quantity;
    if (quantity == Quantity::la) {
        return (q.stamp == get_from_cache(Quantity::lo).stamp);
    } else if (quantity == Quantity::lo) {
        return (q.stamp == get_from_cache(Quantity::la).stamp);
    } else if (quantity == Quantity::ro) {
        return (q.stamp == get_from_cache(Quantity::pi).stamp && q.stamp == get_from_cache(Quantity::ya).stamp);
    } else if (quantity == Quantity::pi) {
        return (q.stamp == get_from_cache(Quantity::ro).stamp && q.stamp == get_from_cache(Quantity::ya).stamp);
    } else if (quantity == Quantity::ya) {
        return (q.stamp == get_from_cache(Quantity::pi).stamp && q.stamp == get_from_cache(Quantity::ro).stamp);
    } else {
        return get_path(quantity) != "";
    }
}

std::string SignalK_converter::get_delta(const Stamped_quantity& q){
    using namespace rapidjson;
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("updates"); writer.StartArray();
    writer.StartObject();
    writer.Key("$source"); writer.String("sensor_hub");
    writer.Key("timestamp"); writer.String(timestamp_to_string(q.stamp) + "Z");
    writer.Key("values");  writer.StartArray();
    writer.StartObject();
    writer.Key("path"); writer.String(get_path(q.quantity));
    writer.Key("value"); get_value(q,writer);
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}

std::string SignalK_converter::get_path(const Quantity& q){
    if (q == Quantity::ut) {
        return "navigation.datetime";
    }
    else if (q == Quantity::la) {
        return "navigation.position";
    }
    else if (q == Quantity::lo) {
        return "navigation.position";
    }
    else if (q == Quantity::vog) {
        return "navigation.speedOverGround";
    }
    else if (q == Quantity::vtw) {
        return "navigation.speedThroughWater";
    }
    else if (q == Quantity::hdg) {
        return "navigation.headingTrue";
    }
    else if (q == Quantity::crs) {
        return "navigation.courseOverGroundTrue";
    }
    else if (q == Quantity::mn) {
        return "navigation.headingMagnetic";
    }
    else if (q == Quantity::ro) {
        return "navigation.attitude";
    }
    else if (q == Quantity::pi) {
        return "navigation.attitude";
    }
    else if (q == Quantity::ya) {
        return "navigation.attitude";
    }
    else if (q == Quantity::wtmp) {
        return "environment.water.temperature";
    }
    else {
        return "";
    }

}

void SignalK_converter::get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer){
    Quantity quantity = q.quantity;
    if (quantity == Quantity::ut) {
        std::string time =  timestamp_to_string(q.value) + "Z";
        writer.String(time);
    }else if (quantity == Quantity::lo || quantity == Quantity::la){
        double latitude = get_from_cache(Quantity::la).value *180 / M_PI;
        double longitude = get_from_cache(Quantity::lo).value * 180 / M_PI;
        writer.StartObject();
        writer.Key("latitude"); writer.Double(latitude);
        writer.Key("longitude"); writer.Double(longitude);
        writer.EndObject();
    } else if (quantity == Quantity::ro || quantity == Quantity::pi || quantity == Quantity::ya){
        double roll = get_from_cache(Quantity::ro).value;
        double pitch = get_from_cache(Quantity::pi).value;
        double yaw = get_from_cache(Quantity::ya).value;
        writer.StartObject();
        writer.Key("roll"); writer.Double(roll);
        writer.Key("pitch"); writer.Double(pitch);
        writer.Key("yaw"); writer.Double(yaw);
        writer.EndObject();
    }else {
        writer.Double(q.value);
    }
}
