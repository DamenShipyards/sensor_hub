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
#include <map>

#include <fmt/core.h>

#include "quantities.h"
#include "loop.h"
#include "log.h"

/**
 * Base class for all sensor devices
 *
 * The application will maintain a list of instantiations of descendants of this class
 */
struct Device {
  Device(): id_(fmt::format("id_{:d}", seq_)), name_(fmt::format("device_{:d}", seq_)),
            connected_(false) {
    ++seq_;
  }
  explicit Device(Device const&) = delete;
  Device& operator=(Device const&) = delete;
  virtual ~Device() {
  }
  const std::string& get_id() const {
    return id_;
  }
  const std::string& get_name() const {
    return name_;
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
  virtual bool connect(const std::string& connection_string) {
    connected_ = true;
    return connected_;
  }
  virtual bool is_connected() {
    return connected_;
  }
  virtual void disconnect() {
    connected_ = false;
  }
protected:
  void set_id(const std::string& id) {
    id_ = id;
  }
  void set_name(const std::string& name) {
    name_ = name;
  }
  void set_connected(const bool connected) {
    connected_ = connected;
    if (connected) {
      log(level::info, "Device % : % connected", name_, id_);
    }
    else {
      log(level::info, "Device % : % disconnected", name_, id_);
    }
  }
private:
  static int seq_;
  std::string id_;
  std::string name_;
  bool connected_;
};


template <typename Port>
struct Port_device: public Device {
  Port_device(): Device(), strand_(get_context()), port_(std::make_unique<Port>(get_context())) {}
protected:
  asio::io_context::strand strand_;
  std::unique_ptr<Port> port_;
};


using Device_ptr = std::unique_ptr<Device>;
//! Container with pointers to devices
using Devices = std::vector<Device_ptr>;

struct Device_factory_base {
  virtual Device_ptr&& get_instance() {
    return std::move(Device_ptr(nullptr));
  }
};

template <class DeviceType>
struct Device_factory: public Device_factory_base {
  typedef DeviceType device_type;
  Device_ptr&& get_instance() override {
    Device_ptr result = std::make_unique<device_type>();
    return std::move(result);
  }
};

using Device_factory_ptr = std::unique_ptr<Device_factory_base>;
using Device_factory_map = std::map<std::string, Device_factory_ptr>;

extern Device_factory_ptr& add_device_factory(const std::string name, Device_factory_ptr&& device_factory);

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
