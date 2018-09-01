/**
 * \file xsens.h
 * \brief Provide interface to xsens device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "device.h"
#include "port.h"
#include "log.h"

template <typename Port>
struct Xsens: public Port_device<Port> {
};

template <typename Port>
struct Xsens_MTi_G_710: public Xsens<Port> {
  Xsens_MTi_G_710(): Xsens<Port>() {
    log(level::info, "Constructing Xsens_MTi_G_710");
  }
  ~Xsens_MTi_G_710() override {
    log(level::info, "Destroying Xsens_MTi_G_710");
  }
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
