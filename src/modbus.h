/**
 * \file modbus.h
 * \brief Provide interface for modbus server
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


#ifndef MODBUS_H_
#define MODBUS_H_

#include "device.h"

#ifdef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#undef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#endif
#include <modbus/server.hpp>

struct Modbus_handler: public modbus::Default_handler {
  Modbus_handler(const Devices& devices, const Processors& processors, const prtr::ptree& config)
    : modbus::Default_handler(), devices_(devices), processors_(processors), scaler_(config) {}
  using modbus::Default_handler::handle;
  modbus::response::read_input_registers handle(
      uint8_t unit_id, const modbus::request::read_input_registers& req);
private:
  void plain_map(const Device& device, int reg_index, int count, modbus::response::read_input_registers& resp);
  void base_map(const Device& device, int reg_index, int count, modbus::response::read_input_registers& resp);
  void processor_map(const Processor& processor, 
      int reg_index, int count, modbus::response::read_input_registers& resp);
  const Devices& devices_;
  const Processors& processors_;
  Base_scale scaler_;
};

using Modbus_server = modbus::Server<Modbus_handler>;

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
