/**
 * \file signalk.h
 * \brief Provide signalk data provider
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


#ifndef SIGNALK_H_
#define SIGNALK_H_


#include "../processor.h"


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>

#include "signalk_server.h"
#include "signalk_converter.h"
template <class ContextProvider>
struct SignalK: public Port_processor<tcp_server,ContextProvider> {
  SignalK(): Port_processor<tcp_server,ContextProvider>(), signalk_converter_(){
  }

  void insert_value(const Stamped_quantity& q) override {
    log(level::debug, "SignalK processor received: %", q);
    // log(level::debug, signalk_converter::get_delta(q));
    if (signalk_converter_.produces_delta(q)) {
      this->get_port().send(signalk_converter_.get_delta(q) + "\n");
    }
  }

  double operator[](size_t) override {
    return 0.0;
  }

  std::string get_json() const override{
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(this->get_name());
  writer.String("data"); writer.StartObject();
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

  size_t size() override {
    return 0;
  }

  void set_param(const std::string&, const double&) override { 
  }

private:
  std::string context_;
  SignalK_converter signalk_converter_;
};  

#endif // SIGNALK_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
