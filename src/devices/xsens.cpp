/**
 * \file xsens.cpp
 * \brief Provide implementation for Xsens device base class
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

// Implementation is put in a separate header for inclusion in tests
#include "xsens_impl.h"

// For context provider
#include "../loop.h"

// Ports:
#include "../serial.h"
#include "../usb.h"

// Add factories to registry
struct Xsens_MTi_G_710_usb: public xsens::MTi_G_710<Usb, Context_provider> {

  std::string get_auto_connection_string() const override {
    return get_usb_connection_string("2639:0017");
  }

};

using Xsens_MTi_G_710_serial = xsens::MTi_G_710<Serial, Context_provider>;

using Xsens_MTi_G_710_usb_factory = Device_factory<Xsens_MTi_G_710_usb>;
using Xsens_MTi_G_710_serial_factory = Device_factory<Xsens_MTi_G_710_serial>;

struct Xsens_MTi_670: public xsens::MTi_670<Serial, Context_provider> {
  Xsens_MTi_670(): xsens::MTi_670<Serial, Context_provider>() {
    this->set_poll_size(0x41);
  };

  std::string get_auto_connection_string() const override {
#ifdef _WIN32
    std::string result = get_serial_connection_string(Context_provider::get_context(),
      "\\Device\\VCP",
      "FTDIBUS",
      "USB\\VID_2639&PID_0300");
#else
    std::string result = get_serial_connection_string(Context_provider::get_context(), "xsens_mti_usb_serial-ttyUSB");
#endif
    result += ":115200";
    return result;
  }
};

using Xsens_MTi_670_factory = Device_factory<Xsens_MTi_670>;

struct Xsens_MTi_630: public xsens::MTi_630<Serial, Context_provider> {
  Xsens_MTi_630(): xsens::MTi_630<Serial, Context_provider>() {
    this->set_poll_size(0x41);
  };

  std::string get_auto_connection_string() const override {
#ifdef _WIN32
    std::string result = get_serial_connection_string(Context_provider::get_context(),
      "\\Device\\VCP",
      "FTDIBUS",
      "USB\\VID_0403&PID_6015");
#else
    std::string result = get_serial_connection_string(Context_provider::get_context(), "ftdi_mti_usb_serial-ttyUSB");
#endif
    result += ":115200";
    return result;
  }
};

using Xsens_MTi_630_factory = Device_factory<Xsens_MTi_630>;


static auto& mti_g_710_usb_factory =
    add_device_factory("xsens_mti_g_710_usb", std::move(std::make_unique<Xsens_MTi_G_710_usb_factory>()));
static auto& mti_g_710_serial_factory =
    add_device_factory("xsens_mti_g_710_serial", std::move(std::make_unique<Xsens_MTi_G_710_serial_factory>()));

static auto& mti_670_factory =
    add_device_factory("xsens_mti_670", std::move(std::make_unique<Xsens_MTi_670_factory>()));
static auto& mti_630_factory =
    add_device_factory("xsens_mti_630", std::move(std::make_unique<Xsens_MTi_630_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
