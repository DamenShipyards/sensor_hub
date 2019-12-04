/**
 * \file regex.cpp
 * \brief Provide implementation for regex parser device class
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


#include "regex.h"

#include "../usb.h"
#include "../serial.h"
// For context provider
#include "../loop.h"

using Regex_device_usb = regex::Regex_device<Usb, Context_provider>;
using Regex_device_serial = regex::Regex_device<Serial, Context_provider>;

using Regex_device_usb_factory = Device_factory<Regex_device_usb>;
using Regex_device_serial_factory = Device_factory<Regex_device_serial>;

static auto& regex_usb_devicefactory =
    add_device_factory("regex_device_usb", std::move(std::make_unique<Regex_device_usb_factory>()));
static auto& regex_serial_device_factory =
    add_device_factory("regex_device_serial", std::move(std::make_unique<Regex_device_serial_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2