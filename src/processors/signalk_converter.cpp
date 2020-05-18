#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
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
  } else {
    return true;
  }
}

std::string SignalK_converter::get_delta(const Stamped_quantity& q){
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
    writer.String("latitude"); writer.Double(latitude);
    writer.String("longitude"); writer.Double(longitude);
    writer.EndObject();

  }else {
    writer.Double(q.value);
  }
}
