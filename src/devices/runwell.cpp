/**
 * \file runwell.cpp
 * \brief Provide implementation for runwell device
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2020 Damen Shipyards\n
 *            Copyright (C) 2020-2022 Orca Software
 *
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


#include "runwell.h"

#include "../serial.h"
#include "../socket.h"
// For context provider
#include "../loop.h"

using Runwell_device_serial = runwell::Runwell_device<Serial, Context_provider>;
using Runwell_device_socket = runwell::Runwell_device<Socket, Context_provider>;

using Runwell_device_serial_factory = Device_factory<Runwell_device_serial>;
using Runwell_device_socket_factory = Device_factory<Runwell_device_socket>;

static auto& runwell_serial_device_factory =
    add_device_factory("runwell_driver_serial", std::move(std::make_unique<Runwell_device_serial_factory>()));
static auto& runwell_socket_device_factory =
    add_device_factory("runwell_driver_socket", std::move(std::make_unique<Runwell_device_socket_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
