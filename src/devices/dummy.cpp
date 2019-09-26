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
using Dummy_factory = Device_factory<Dummy_device>;

static auto& dummy_factory =
    add_device_factory("dummy_device", std::move(std::make_unique<Dummy_factory>()));


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
