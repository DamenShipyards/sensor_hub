#ifndef SIGNALK_CONVERTER_H_
#define SIGNALK_CONVERTER_H_

#include <rapidjson/writer.h>
#include "../quantities.h"
class SignalK_converter {
public:
  SignalK_converter():cache_(){}
  std::string get_delta(const Stamped_quantity& q);
  bool produces_delta(const Stamped_quantity& q);
private:
std::string get_path(const Quantity& q);
void get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer);
Stamped_quantity cache_ [static_cast<int>(Quantity::end)];
inline void add_to_cache(const Stamped_quantity& q){
  cache_[static_cast<int>(q.quantity)] = q;
}
inline Stamped_quantity& get_from_cache(Quantity q){
  return cache_[static_cast<int>(q)];
}

};
#endif // SIGNALK_CONVERTER_H_
