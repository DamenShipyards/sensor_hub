/**
 * \file modbus.h
 * \brief Provide interface for modbus server
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#ifdef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#undef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#endif
#include <modbus/server.hpp>

#include "device.h"

struct Modbus_handler: public modbus::Default_handler {
  Modbus_handler(const Devices& devices): modbus::Default_handler(), devices_(devices) {}
  using modbus::Default_handler::handle;
  modbus::response::read_input_registers handle(uint8_t unit_id, const modbus::request::read_input_registers& req);
private:
  void plain_map(const Device& device, int reg_index, int count, modbus::response::read_input_registers& resp);
  void base_map(const Device& device, int reg_index, int count, modbus::response::read_input_registers& resp);
  const Devices& devices_;
};

using Modbus_server = modbus::Server<Modbus_handler>;

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
