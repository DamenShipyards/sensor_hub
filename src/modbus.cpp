/**
 * \file modbus.cpp
 * \brief Provide implementation for modbus server
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


#include "modbus.h"
#include "quantities.h"
#include "version.h"
#include "tools.h"
#include "log.h"

#include <cmath>

using namespace modbus;

static Stamped_value const zero = {};
static constexpr uint16_t const plain_base_address = 10000;
static constexpr uint16_t const processor_base_address = 20000;


void Modbus_handler::plain_map(const Device& device, 
    int reg_index, const int count, response::read_input_registers& resp) {
  Quantity curq = Quantity::end;
  Stamped_value sample;
  for (int index = 0; index < count; ++index) {
    Quantity newq = static_cast<Quantity>(reg_index / 8);
    if (newq != curq) {
      if (!device.get_sample(newq, sample)) {
        sample = zero;
      }
      curq = newq;
    }
    int off = reg_index % 8;
    int shft = (3 - off % 4) * 16;
    int i = off / 4;
    double value = sample[i];
    // Cram the selected part of the 64 bit double value in a 16 bit register
    resp.values[index] = (*reinterpret_cast<const uint64_t*>(&value) >> shft) & 0xFFFF;
    ++reg_index;
  }
}


void Modbus_handler::base_map(const Device& device, 
    int reg_index, const int count, response::read_input_registers& resp) {

  for (int index = 0; index < count; ++index) {
    Quantity q;
    double value = 0;
    switch (reg_index) {
      case 0: {
          std::string version = STRINGIFY(VERSION);
          uint16_t ver = 0;
          size_t n;
          while ((n = version.find(".")) != version.npos) {
            ver *= 100;
            ver += std::stoi(version.substr(0, n));
            version.replace(0, n + 1, "");
          };
          resp.values[index] = ver;
        }
        break;
      case 1:
      case 2: 
        q = Quantity::ut;
        if (device.get_value(q, value)) {
          uint32_t t = scaler_.scale_to<uint32_t>(q, value);
          resp.values[index] = reg_index == 1 ? t >> 16 : t & 0xFFFF;
        }
        break;
      case 3:
      case 4: 
        q = Quantity::la;
        if (device.get_value(q, value)) {
          uint32_t v = scaler_.scale_to<uint32_t>(q, value);
          resp.values[index] = reg_index == 3 ? v >> 16 : v & 0xFFFF;
        }
        break;
      case 5:
      case 6:
        q = Quantity::lo;
        if (device.get_value(q, value)) {
          uint32_t v = scaler_.scale_to<uint32_t>(q, value);
          resp.values[index] = reg_index == 5 ? v >> 16 : v & 0xFFFF;
        }
        break;
      default: 
        q = static_cast<Quantity>(reg_index - 4);
        if (device.get_value(q, value)) {
          resp.values[index] = scaler_.scale_to<uint16_t>(q, value);
        }
    }
    ++reg_index;
  }
}

void Modbus_handler::processor_map(const Processor& processor,
    int reg_index, const int count, response::read_input_registers& resp) {
  for (int i = 0; i < count; ++i) {
    resp.values[i] = processor.get_modbus_reg(i + reg_index, scaler_);
  }
}

response::read_input_registers Modbus_handler::handle(uint8_t unit_id, const request::read_input_registers& req) {
  log(level::debug, "Received modbus read_input_registers for unit %, reg %, count %",
      static_cast<int>(unit_id), req.address, req.count);
  // Unit ID 255 means "not used" -> assume device 0
  if (unit_id == 0xFF)
    unit_id = 0;
  auto resp = Default_handler::handle(unit_id, req);
  if (req.address >= processor_base_address) {
    if (unit_id < processors_.size()) {
      const Processor& processor = *processors_[unit_id];
      log(level::debug, "Returning values for processor %", processor.get_name());
      processor_map(processor, req.address - processor_base_address, req.count, resp);
    }
  }
  else if (unit_id < devices_.size()) { 
    const Device& device = *devices_[unit_id];
    log(level::debug, "Returning values for device %", device.get_name());
    if (req.address >= plain_base_address) {
      plain_map(device, req.address - plain_base_address, req.count, resp);
    }
    // Cast to swallow compiler warning
    else {
      base_map(device, req.address, req.count, resp);
    }
  }
  return resp;
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
