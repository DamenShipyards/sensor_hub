#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include "signalk_converter.h"
std::string signalk_converter::get_delta(const Stamped_quantity& q){
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

std::string signalk_converter::get_path(const Quantity& q){
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

void signalk_converter::get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer){
  Quantity quantity = q.quantity;
  if (quantity == Quantity::ut) {
    std::string time =  timestamp_to_string(q.value) + "Z";
    writer.String(time);
  }else {
    writer.Double(q.value);
  }
}