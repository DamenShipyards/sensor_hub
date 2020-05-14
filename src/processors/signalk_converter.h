#ifndef SIGNALK_CONVERTER_H_
#define SIGNALK_CONVERTER_H_

#include <rapidjson/writer.h>
#include "../quantities.h"
namespace signalk_converter {
std::string get_path(const Quantity& q);
void get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer);
std::string get_delta(const Stamped_quantity& q);
}
#endif // SIGNALK_CONVERTER_H_