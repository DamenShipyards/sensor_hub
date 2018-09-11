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
#include <deque>
#include <iomanip>

#include <fmt/core.h>

#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/asio/spawn.hpp>
namespace asio = boost::asio;

#include "quantities.h"
#include "log.h"
#include "datetime.h"

using Data_queue = std::deque<Stamped_value>;
using Data_map = std::map<Quantity, Data_queue>;

/**
 * Base class for all sensor devices
 *
 * The application will maintain a list of instantiations of descendants of this class
 */
struct Device {
  Device(): id_(fmt::format("id_{:d}", seq_)), name_(fmt::format("device_{:d}", seq_)),
            connected_(false), data_(), enable_logging_(false) {
    log(level::debug, "Constructing Device");
    ++seq_;
  }

  explicit Device(Device const&) = delete;

  Device& operator=(Device const&) = delete;

  virtual ~Device() {
    log(level::debug, "Destroying Device");
  }

  const std::string& get_id() const {
    return id_;
  }

  const std::string& get_name() const {
    return name_;
  }

  void set_name(const std::string& name) {
    log(level::debug, "Setting device name to \"%\"", name);
    init_device_log(name);
    name_ = name;
  }

  virtual const bool get_value(const Quantity& quantity, Value_type& value) const {
    auto it = data_.find(quantity);
    if (it == data_.end() || it->second.empty()) {
      return false;
    }
    value = it->second.back().value;
    return true;
  }

  const Value_type get_value(const Quantity& quantity) const {
    Value_type value;
    if (!get_value(quantity, value))
      throw Quantity_not_available();
    return value;
  }

  virtual void connect(asio::yield_context yield) {
    set_connected(true);
  }

  virtual bool initialize(asio::yield_context yield) {
    return true;
  }

  virtual const bool is_connected() const {
    return connected_;
  }

  virtual void disconnect() {
    set_connected(false);
  }

  const std::string& get_connection_string() const {
    return connection_string_;
  }

  void set_connection_string(const std::string& connection_string) {
    connection_string_ = connection_string;
  }

  void enable_logging(const bool value) {
    enable_logging_ = value;
    if (value) {
      log(level::info, "Logging enabled for %", name_);
    }
    else {
      log(level::info, "Logging disabled for %", name_);
    }
  }

  void use_as_time_source(const bool value) {
    use_as_time_source_ = value;
    if (value) {
      log(level::info, "Using % as time source", name_);
    }
  }
protected:
  void set_id(const std::string& id) {
    id_ = id;
  }

  void insert_value(Stamped_quantity&& value) {
    auto item = data_.try_emplace(value.quantity);
    item.first->second.push_back(value);
    if (enable_logging_) {
      std::stringstream ss;
      ss << std::setprecision(15) << value.stamp << "," << value.quantity << "," << value.value;
      log(name_, ss.str());
    }
    if (value.quantity == Quantity::ut && use_as_time_source_) {
      adjust_clock(value.value);
    }
  }

  void set_connected(const bool connected) {
    if (connected == connected_) {
      log(level::warning, "Connected state of device was already: %", connected);
    }
    connected_ = connected;
    if (connected) {
      log(level::info, "Device \"%\" : % connected", name_, id_);
    }
    else {
      log(level::info, "Device \"%\" : % disconnected", name_, id_);
    }
  }

private:
  static int seq_;
  std::string id_;
  std::string name_;
  bool connected_;
  std::string connection_string_;
  Data_map data_;
  bool enable_logging_;
  bool use_as_time_source_;
};


/**
 * Device that controls an IO port supporting asio's basic_io_object interface
 */
template <typename Port, class ContextProvider>
struct Port_device: public Device {
  Port_device()
      : Device(),
        port_(ContextProvider::get_context()){}
  ~Port_device() {
    disconnect();
  }
  typedef Port port_type;
  void connect(asio::yield_context yield) override {
    std::string connection_string = get_connection_string();
    try {
      port_.open(connection_string);
      log(level::info, "Connected device port: %", connection_string);
      bool initialized = initialize(yield);
      set_connected(initialized);
    }
    catch (std::exception& e) {
      log(level::error, "Failed to connect using \"%\" error \"%\"", connection_string, e.what());
    }
  }
  void disconnect() override {
    if (is_connected())
      set_connected(false);
    port_.close();
  }
  Port& get_port() {
    return port_;
  }
protected:
  Port port_;
};

//! Unique pointer to a device
using Device_ptr = std::unique_ptr<Device>;

//! Container with pointers to devices
using Devices = std::vector<Device_ptr>;

/**
 * Base class for a device factory
 */
struct Device_factory_base {
  virtual Device_ptr get_instance() {
    return Device_ptr(nullptr);
  }
};

/**
 * Device factory for "DeviceType"
 */
template <class DeviceType>
struct Device_factory: public Device_factory_base {
  typedef DeviceType device_type;
  Device_ptr get_instance() override {
    return std::make_unique<device_type>();
  }
};

//! Unique pointer to a device factory
using Device_factory_ptr = std::unique_ptr<Device_factory_base>;

/**
 * Add a device factory to the global device factory registry
 */
extern Device_factory_ptr& add_device_factory(const std::string& name, Device_factory_ptr&& device_factory);

/**
 * Have a factory from the factory registry create a device instance
 */
extern Device_ptr create_device(const std::string& name);

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
