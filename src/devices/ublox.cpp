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

// asio serial port wrapper
#include "../serial.h"

using Ublox_NEO_M8U_serial = ubx::NEO_M8U<Serial, Context_provider>;

using Ublox_NEO_M8U_serial_factory = Device_factory<Ublox_NEO_M8U_serial>;

static auto& neo_m8u_serial_factory = 
    add_device_factory("ublox_neo_m8u_serial", std::move(std::make_unique<Ublox_NEO_M8U_serial_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
