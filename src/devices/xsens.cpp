/**
 * \file xsens.cpp
 * \brief Provide implementation for Xsens device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "xsens.h"

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

using Xsens_MTi_G_710_serial = xsens::MTi_G_710<asio::serial_port, Context_provider>;

using Xsens_MTi_G_710_usb_factory = Device_factory<Xsens_MTi_G_710_usb>;
using Xsens_MTi_G_710_serial_factory = Device_factory<Xsens_MTi_G_710_serial>;

static auto& mti_g_710_usb_factory =
    add_device_factory("xsens_mti_g_710_usb", std::move(std::make_unique<Xsens_MTi_G_710_usb_factory>()));
static auto& mti_g_710_serial_factory =
    add_device_factory("xsens_mti_g_710_serial", std::move(std::make_unique<Xsens_MTi_G_710_serial_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
