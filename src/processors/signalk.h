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
#include <rapidjson/writer.h>


struct SignalK: public Processor {
  SignalK(): Processor() {}

  void insert_value(const Stamped_quantity& q) override {
    log(level::debug, "SignalK processor received: %", q);
    log(level::debug, get_delta(q));
  }

  double operator[](size_t) override {
    return 0.0;
  }

  std::string get_json() const override;

  size_t size() override {
    return 0;
  }

  void set_param(const std::string&, const double&) override { 
  }

private:
  std::string context_;
  std::string get_path(const Quantity& q);
  void get_value(const Stamped_quantity& q,  rapidjson::Writer<rapidjson::StringBuffer>& writer);
  std::string get_delta(const Stamped_quantity& q);
};

#endif // SIGNALK_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
