/**
 * \file nmea.cpp
 * \brief Provide implementation for generic nmea device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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


#include "nmea.h"

#include "../usb.h"
#include "../serial.h"
// For context provider
#include "../loop.h"

using Generic_Nmea_usb = nmea::Generic_Nmea<Usb, Context_provider>;
using Generic_Nmea_serial = nmea::Generic_Nmea<Serial, Context_provider>;

using Generic_Nmea_usb_factory = Device_factory<Generic_Nmea_usb>;
using Generic_Nmea_serial_factory = Device_factory<Generic_Nmea_serial>;

static auto& nmea_usb_devicefactory =
    add_device_factory("generic_nmea_usb", std::move(std::make_unique<Generic_Nmea_usb_factory>()));
static auto& nmea_serial_device_factory =
    add_device_factory("generic_nmea_serial", std::move(std::make_unique<Generic_Nmea_serial_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
