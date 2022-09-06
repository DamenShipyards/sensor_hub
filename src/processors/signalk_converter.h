/**
 * \file signalk_converter.h
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
  Stamped_quantity cache_ [static_cast<int>(Quantity::end)];

  std::string get_path(const Quantity& q);
  void get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer);


  inline void add_to_cache(const Stamped_quantity& q){
    cache_[static_cast<int>(q.quantity)] = q;
  }
  inline Stamped_quantity& get_from_cache(Quantity q){
    return cache_[static_cast<int>(q)];
  }

};
#endif // SIGNALK_CONVERTER_H_
