/**
 * \file device.h
 * \brief Provide interface to device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <vector>
#include <memory>

#include "quantities.h"

struct Device {
};

using Devices = std::vector<std::unique_ptr<Device>>;

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
