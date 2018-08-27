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
#include <exception>

#include <fmt/core.h>

#include "quantities.h"


struct Device {
  Device(): id_(fmt::format("device_{:d}", seq_++)) {
  }
  virtual ~Device() {
  }
  const std::string& get_id() const {
    return id_;
  }
  virtual bool get_value(const Quantity& quantity, Value_type& value) {
    return false;
  }
  Value_type get_value(const Quantity& quantity) {
    Value_type value;
    if (!get_value(quantity, value))
      throw Quantity_not_available();
    return value;
  }
protected:
  void set_id(const std::string& id) {
    id_ = id;
  }
private:
  static int seq_;
  std::string id_;
};

using Devices = std::vector<std::unique_ptr<Device>>;

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
