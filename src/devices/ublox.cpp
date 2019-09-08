/**
 * \file ublox.cpp
 * \brief Provide implementation for u-blox device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */


// Implementation is put in a separate header for inclusion in tests
#include "ublox_impl.h"

// For context provider
#include "../loop.h"

// Ports:
#include "../serial.h"

struct Ublox_NEO_M8U_serial: public ubx::NEO_M8U<Serial, Context_provider> {

  std::string get_auto_connection_string() const override {
#ifdef _WIN32
    std::string result = get_serial_connection_string(Context_provider::get_context(), "UBLOXUSBPort", "ubloxusb", "USB\\VID_1546&PID_01A8");
#else
    std::string result = get_serial_connection_string(Context_provider::get_context(), "ublox_neo_m8u-ttyACM");
#endif
    result += ":921600";
    return result;
  }

};

using Ublox_NEO_M8U_serial_factory = Device_factory<Ublox_NEO_M8U_serial>;

static auto& neo_m8u_serial_factory = 
    add_device_factory("ublox_neo_m8u_serial", std::move(std::make_unique<Ublox_NEO_M8U_serial_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
