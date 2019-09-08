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
#include <iomanip>

// asio uses "old" boost::coroutine instead of boost::coroutine2
#ifndef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#endif

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/property_tree/ptree.hpp>

#include "quantities.h"
#include "log.h"
#include "datetime.h"
#include "processor.h"
#include "types.h"

namespace asio = boost::asio;
namespace pt = boost::property_tree;

using std::runtime_error;


class Device_exception: public runtime_error {
  using runtime_error::runtime_error;
};


struct Named_object {
  Named_object(): id_(), name_() {}
  Named_object(const std::string id, const std::string name):
    id_(id), name_(name), enabled_(false) {}

  const std::string& get_id() const {
    return id_;
  }


  const std::string& get_name() const {
    return name_;
  }


  void set_id(const std::string& id) {
    if (id != id_) {
      log(level::info, "Setting device id to \"%\"", this->get_id());
      id_ = id;
    }
  }


  void set_name(const std::string& name) {
    if (name != name_) {
      log(level::info, "Setting name to \"%\"", name);
      name_ = name;
      init_device_log(name);
    }
  }

  void set_enabled(const bool value) {
    enabled_ = value;
  }

  bool is_enabled() const {
    return enabled_;
  }

private:
  std::string id_;
  std::string name_;
  bool enabled_;
};

/**
 * Base class for all sensor devices
 *
 * The application will maintain a list of instantiations of descendants of this class
 */
struct Device: public Named_object {

  Device(): Named_object(fmt::format("id_{:d}", seq_), fmt::format("device_{:d}", seq_)),
            connected_(false), data_(), enable_logging_(false), processors_() {
    log(level::debug, "Constructing Device");
    ++seq_;
  }

  explicit Device(Device const&) = delete;

  Device& operator=(Device const&) = delete;


  virtual ~Device() {
    log(level::debug, "Destroying Device");
  }


  virtual const bool get_value(const Quantity& quantity, Value_type& value) const {
    auto it = data_.find(quantity);
    if (it == data_.end() || it->second.empty()) {
      return false;
    }
    value = it->second.back().value;
    return true;
  }


  virtual const bool get_sample(const Quantity& quantity, Stamped_value& sample) const {
    auto it = data_.find(quantity);
    if (it == data_.end() || it->second.empty()) {
      return false;
    }
    sample = it->second.back();
    return true;
  }


  const Value_type get_value(const Quantity& quantity) const {
    Value_type value;
    if (!get_value(quantity, value))
      throw Quantity_not_available();
    return value;
  }


  virtual void connect(asio::yield_context yield) {
    bool initialized = initialize(yield);
    if (initialized)
      set_connected(initialized);
    else
      throw Device_exception("Failed to initialize device");
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


  virtual std::string get_auto_connection_string() const {
    return "unimplemented_auto_connection_string";
  }


  const std::string get_connection_string() const {
    if (connection_string_ == "auto") {
      return this->get_auto_connection_string();
    }
    else {
      return connection_string_;
    }
  }


  void set_connection_string(const std::string& connection_string) {
    connection_string_ = connection_string;
  }


  void enable_logging(const bool value) {
    enable_logging_ = value;
    if (value) {
      log(level::info, "Logging enabled for %", this->get_name());
    }
    else {
      log(level::info, "Logging disabled for %", this->get_name());
    }
  }


  virtual void use_as_time_source(const bool value) {
    use_as_time_source_ = value;
    if (value) {
      log(level::info, "Using % as time source", this->get_name());
    }
  }


  void add_processor(Processor_ptr processor) {
    processors_.push_back(processor);
  }

protected:

  void insert_value(const Stamped_quantity& value) {
    if (use_as_time_source_ && value.quantity == Quantity::ut) {
      adjust_clock_diff(value.value - value.stamp);
    }
    auto item = data_.try_emplace(value.quantity);
    auto& queue = item.first->second;
    queue.push_back({value.stamp, value.value});
    for (auto&& processor: processors_) {
      processor->insert_value(value);
    }
    if (queue.size() > 0x0040000) {
      queue.pop_front();
    }
    if (enable_logging_) {
      std::stringstream ss;
      ss << std::setprecision(15) << value.stamp << "," << value.quantity << "," << value.value;
      log(this->get_name(), ss.str());
    }
  }


  void set_connected(const bool connected) {
    if (connected == connected_) {
      log(level::warning, "Connected state of device was already: %", connected);
    }
    connected_ = connected;
    if (connected) {
      log(level::info, "Device \"%\" : % connected", this->get_name(), this->get_id());
    }
    else {
      log(level::info, "Device \"%\" : % disconnected", this->get_name(), this->get_id());
    }
  }

private:
  static int seq_;
  bool connected_;
  std::string connection_string_;
  Data_map data_;
  bool enable_logging_;
  bool use_as_time_source_;
  Processors processors_;
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
    if (is_connected()) {
      log(level::warning, "Connecting device % that is already connected", this->get_name());
      return;
    }


    std::string connection_string = get_connection_string();
    try {
      port_.open(connection_string);
      log(level::info, "Connected device port: %", connection_string);
    }
    catch (std::exception& e) {
      log(level::error, "Failed to connect using \"%\" error \"%\"", connection_string, e.what());
      return;	
    }


    try {
      Device::connect(yield);
    }
    catch (std::exception& e) {
      log(level::error, "Failed to connect \"%\"", this->get_name(), e.what());
      port_.close();
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


  bool exec_command(
      cbytes_t& command,
      cbytes_t& expected_response,
      cbytes_t& error_response,
      asio::yield_context yield,
      bytes_t* data=nullptr,
      int timeout=1000) {
    Port& port = this->get_port();

    // Set a timeout for the command to complete
    asio::deadline_timer timeout_timer(ContextProvider::get_context(), posix_time::milliseconds(timeout));
    timeout_timer.async_wait(
        [&](const boost::system::error_code& error) {
          if (!error)
            port.cancel();
        });

    // Write out the command string...
    asio::async_write(port, asio::buffer(command), yield);

    std::stringstream ssc;
    ssc << command;
    log(level::debug, "Sent to %: %", this->get_name(), ssc.str());

    // ... and look for the expected response
    try {
      // Repeat response reading several times as the response might be cluttered with other data coming in
      int repeats = 4;
      bytes_t response;
      int response_found = -1;
      uint16_t len = 0;
      bool read_all = false;
      do {
        asio::streambuf read_buf;
        size_t bytes_read = port.async_read_some(read_buf.prepare(0x1000), yield);
        read_buf.commit(bytes_read);
        auto buf_begin = asio::buffers_begin(read_buf.data());
        auto buf_end = buf_begin + bytes_read;

        cbytes_t received(buf_begin, buf_end);
        std::stringstream ssr;
        ssr << received;
        log(level::debug, "Received from %: %", this->get_name(), ssr.str());

        response.insert(response.end(), buf_begin, buf_end);

        read_buf.consume(bytes_read);
        response_found = contains_at(response, expected_response);
        int error_found = contains_at(response, error_response);
        if (error_found >= 0) {
          int error_code_offset = static_cast<int>(error_response.size());
          if ((error_found + error_code_offset) < static_cast<int>(response.size())) {
            log(level::error, "Received % error: %", this->get_name(),
                static_cast<int>(response[error_found + error_code_offset]));
          }
          else {
            log(level::error, "Received % error", this->get_name());
          }
          timeout_timer.cancel();
          return false;
        }
        if (response_found >= 0) {
          if (data != nullptr && data->size() > 0) {
            // Get a length indicator for data in the response
            size_t len_offset_1 = (*data)[0] + response_found;
            size_t len_offset_2  = (data->size() > 1) ? (*data)[1] + response_found: -1;
            len = response.size() > len_offset_1 ? response[len_offset_1] : 0;
            len += len_offset_2 >= 0 && response.size() > len_offset_2 ? response[len_offset_2] << 8: 0;
            len += static_cast<uint16_t>(std::max(len_offset_1, len_offset_2));
            // ... and indicate whether we have read enough data (i.e. equal to or more than len)
            // starting from the first byte after the offset of the length indicator
            data->clear();
          }
          read_all = response.size() > len;
        }
      } while ((--repeats > 0 && response_found < 0) || !read_all);

      timeout_timer.cancel();

      if (response_found >= 0) {
        if (data != nullptr) {
          data->insert(data->end(), response.begin() + response_found, response.end());
        }
        return true;
      }
      else {
        log(level::error, "% didn't receive expected command response", this->get_name());
        return false;
      }
    }
    catch (std::exception& e) {
      // Probably a timeout i.e. USB cancelled by timer
      log(level::error, "%: Error executing command: %", this->get_name(), e.what());
      timeout_timer.cancel();
      port.cancel();
      return false;
    }
  }


  void wait(int milli_seconds, asio::yield_context yield) {
    asio::deadline_timer waiter(ContextProvider::get_context(), posix_time::milliseconds(milli_seconds));
    waiter.async_wait(yield);
  }


  virtual bool reset(asio::yield_context yield) {
    return true;
  }

protected:
  Port port_;

};

template <class DeviceClass>
struct Polling_mixin {
  virtual void start_polling() {
    auto executor = get_device()->get_port().get_executor();
    asio::post(
        executor,
        [executor, this]() {
          asio::spawn(executor, boost::bind(&Polling_mixin::poll_data, this, _1));
        }
    );
  }

  void poll_data(asio::yield_context yield) {
    log(level::debug, "Polling %", get_device()->get_name());
    asio::streambuf buf;
    while (get_device()->is_connected()) {
      try {
        auto bytes_read = get_device()->get_port().async_read_some(buf.prepare(512), yield);
        double stamp = get_time();
        log(level::debug, "% read % bytes", get_device()->get_name(), bytes_read);
        if (bytes_read > 0) {
          buf.commit(bytes_read);
          auto buf_begin = asio::buffers_begin(buf.data());
          auto buf_end = buf_begin + buf.size();
#ifdef DEBUG
          cbytes_t data(buf_begin, buf_end);
          std::stringstream ss;
          ss << data;
          log(level::debug, "% received: %", get_device()->get_name(), ss.str());
#endif
          get_device()->handle_data(stamp, buf_begin, buf_end);
          buf.consume(bytes_read);
        }
      }
      catch (std::exception& e) {
        log(level::error, "Error while polling %: %", get_device()->get_name(), e.what());
        get_device()->disconnect();
      }
    }
  }

private:
  DeviceClass* get_device() {
    return dynamic_cast<DeviceClass*>(this);
  }
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

/**
 * Get boost property tree from device
 */
extern pt::ptree get_device_tree(const Device& device);

/**
 * Get json string from device
 */
extern std::string get_device_json(const Device& device);


#endif  // ifndef DEVICE_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
