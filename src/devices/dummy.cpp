/**
 * \file dummy.cpp
 * \brief Provide implementation for dummy device class
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


#include "dummy.h"

// For context provider
#include "../loop.h"

using Dummy_device = dummy::Dummy_device<Context_provider>;
using Dummy_gps = dummy::Dummy_gps<Context_provider>;
using Dummy_imu = dummy::Dummy_imu<Context_provider>;
using Dummy_mru = dummy::Dummy_mru<Context_provider>;
using Dummy_device_factory = Device_factory<Dummy_device>;
using Dummy_gps_factory = Device_factory<Dummy_gps>;
using Dummy_imu_factory = Device_factory<Dummy_imu>;
using Dummy_mru_factory = Device_factory<Dummy_mru>;

static auto& dummy_devicefactory =
    add_device_factory("dummy_device", std::move(std::make_unique<Dummy_device_factory>()));
static auto& dummy_gps_factory =
    add_device_factory("dummy_gps", std::move(std::make_unique<Dummy_gps_factory>()));
static auto& dummy_imu_factory =
    add_device_factory("dummy_imu", std::move(std::make_unique<Dummy_imu_factory>()));
static auto& dummy_mru_factory =
    add_device_factory("dummy_mru", std::move(std::make_unique<Dummy_mru_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
