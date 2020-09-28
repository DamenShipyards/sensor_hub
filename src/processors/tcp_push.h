/**
 * \file tcp_push.h
 * \brief Provide tcp push service processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2020 Damen Shipyards
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


#ifndef TCP_H_
#define TCP_H_

#include <string>
#include <memory>
#include <boost/asio.hpp>

#include "../processor.h"

struct Tcp_pusher: public Processor {
  Tcp_pusher(): Processor(), _port(4001), _address("0.0.0.0") {}

  void insert_value(const Stamped_quantity&) override {
  }

  double operator[](size_t) override {
    return 0.0;
  }

  std::string get_json() const override;

  size_t size() override {
    return 0;
  }

  void set_param(const std::string& name, const std::string& value) override {
    if (name == "address") {
      _address = value;
    }
  }

  void set_param(const std::string& name, const double& value) override { 
    if (name == "port") {
      _port = round(value);
    }
  }

private:
  int _port;
  std::string _address;
  
};

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
