/**
 * \file device.h
 * \brief Provide interface to device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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


#ifndef DEVICE_H_
#define DEVICE_H_

#include <vector>
#include <memory>
#include <exception>
#include <iomanip>

#include "quantities.h"
#include "log.h"
#include "datetime.h"
#include "processor.h"
#include "types.h"

// asio uses "old" boost::coroutine instead of boost::coroutine2
#ifndef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#endif
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace asio = boost::asio;
namespace prtr = boost::property_tree;

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
      log(level::info, "Setting device id from \"%\" to \"%\"", id_, id);
      id_ = id;
    }
  }


  void set_name(const std::string& name) {
    if (name != name_) {
      log(level::info, "Setting name to \"%\"", name);
      name_ = name;
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
            connected_(false), data_(), 
            enable_logging_(false), max_log_files_(32), max_log_size_(64 * 1024 * 1024), 
            device_log_initialized_(false), processors_() {
    log(level::debug, "Constructing Device");
    ++seq_;
  }

  explicit Device(Device const&) = delete;

  Device& operator=(Device const&) = delete;


  virtual ~Device() {
    log(level::debug, "Destroying Device");
  }


  virtual bool get_value(const Quantity& quantity, Value_type& value) const {
    auto it = data_.find(quantity);
    if (it == data_.end() || it->second.empty()) {
      return false;
    }
    value = it->second.back().value;
    return true;
  }


  virtual bool get_sample(const Quantity& quantity, Stamped_value& sample) const {
    auto it = data_.find(quantity);
    if (it == data_.end() || it->second.empty()) {
      return false;
    }
    sample = it->second.back();
    return true;
  }


  Value_type get_value(const Quantity& quantity) const {
    Value_type value;
    if (!get_value(quantity, value))
      throw Quantity_not_available();
    return value;
  }


  virtual void connect(asio::yield_context yield) {
    bool initialized = initialize(yield);
    if (initialized)
      this->set_connected(initialized);
    else
      throw Device_exception("Failed to initialize device");
  }


  virtual bool initialize(asio::yield_context) {
    return true;
  }


  virtual bool reset(asio::yield_context) {
    return true;
  }


  virtual bool is_connected() const {
    return connected_;
  }


  virtual void disconnect() {
    this->set_connected(false);
  }


  virtual std::string get_auto_connection_string() const {
    return "unimplemented_auto_connection_string";
  }


  virtual void set_options(const prtr::ptree&) {}


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

  void set_max_log_files(const int value) {
    log(level::info, "Set max log files to % for  %", value, this->get_name());
    max_log_files_ = value;
  }

  void set_max_log_size(const size_t value) {
    log(level::info, "Set max log size to % for  %", value, this->get_name());
    max_log_size_ = static_cast<int>(value);
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


  void check_setup_device_log() {
    if (this->is_connected()) {
      setup_device_log();
    }
  }


protected:

  void setup_device_log() {
    if (!enable_logging_ || device_log_initialized_)
      return;
    device_log_initialized_ = init_device_log(this->get_id(), this->get_name(), 
        max_log_files_, max_log_size_);
    if (device_log_initialized_) {
      log(level::info, "Device log started: %", this->get_name());
    }
  }

  void insert_value(const Stamped_quantity& value) {
    // When registered as time source, adjust application clock when receiving time sample
    if (use_as_time_source_ && value.quantity == Quantity::ut) {
      adjust_clock_diff(value.value - value.stamp);
    }
    // Add value to sample cache
    auto item = data_.try_emplace(value.quantity);
    auto& queue = item.first->second;
    queue.push_back({value.value, value.stamp});

    // Pass value to processors
    for (auto&& processor: processors_) {
      processor->insert_value(value);
    }
    // Prune sample cache: maximum 256K or 1h of samples
    while (queue.size() > 0x0040000 || value.stamp - queue.front().stamp > 3600) {
      queue.pop_front();
    }
    // Write device log if enabled
    if (enable_logging_) {
      try {
        std::stringstream ss;
        ss << std::setprecision(15) << value.stamp << "," << value.quantity << "," << value.value;
        if (enable_logging_ && device_log_initialized_) {
          log(this->get_name(), ss.str());
        }
      }
      catch (...) {
        static int err_count = 0;
        if ((err_count % 10000) == 0) {
          log(level::error, "Failed to write device log");
        }
        ++err_count;
      }
    }
  }


  void set_connected(const bool connected) {
    if (connected == connected_) {
      log(level::warning, "Connected state of device was already: %", connected);
    }
    if (connected) {
      log(level::info, "Device \"%\" : % connected", this->get_name(), this->get_id());
    }
    else {
      log(level::info, "Device \"%\" : % disconnected", this->get_name(), this->get_id());
    }
    connected_ = connected;
  }



private:
  static int seq_;
  bool connected_;
  std::string connection_string_;
  Data_map data_;
  bool enable_logging_;
  int max_log_files_;
  int max_log_size_;
  bool device_log_initialized_;
  bool use_as_time_source_;
  Processors processors_;

};


template<class ContextProvider>
struct Context_device: public Device {

  void wait(int milli_seconds, asio::yield_context yield) {
    asio::deadline_timer waiter(ContextProvider::get_context(), pt::milliseconds(milli_seconds));
    waiter.async_wait(yield);
  }

  auto get_executor() {
    return ContextProvider::get_context().get_executor();
  }

};


/**
 * \brief Port timeout
 */
template <typename Port>
struct Port_timeout {
  Port_timeout(Port& port, int timeout): timeout_timer_(port.get_executor(), pt::milliseconds(timeout)) {
    timeout_timer_.async_wait(
        [&](const boost::system::error_code& error) {
          if (!error)
            port.cancel();
        });
  }
  ~Port_timeout() {
    timeout_timer_.cancel();
  }
private:
  asio::deadline_timer timeout_timer_;
};

extern int int_placeholder_;

/**
 * \brief Device that controls an IO port supporting asio's basic_io_object interface
 */
template <typename Port, class ContextProvider>
struct Port_device: public Context_device<ContextProvider> {

  Port_device()
      : Context_device<ContextProvider>(),
        port_(ContextProvider::get_context()){}

  ~Port_device() {
    disconnect();
  }

  typedef Port port_type;

  void connect(asio::yield_context yield) override {
    if (this->is_connected()) {
      log(level::warning, "Connecting device % that is already connected", this->get_name());
      return;
    }


    std::string connection_string = this->get_connection_string();
    try {
      port_.open(connection_string);
      log(level::info, "Connected device port: %", connection_string);
    }
    catch (std::exception& e) {
      log(level::error, "Failed to connect \"%\" using \"%\": \"%\"",
          this->get_name(), connection_string, e.what());
      return;	
    }


    try {
      // Write out the command
      Context_device<ContextProvider>::connect(yield);
    }
    catch (std::exception& e) {
      log(level::error, "Failed to connect \"%\": %", this->get_name(), e.what());
      port_.close();
    }
  }


  void disconnect() override {
    if (this->is_connected()) {
      this->set_connected(false);
      port_.close();
    }
  }


  Port& get_port() {
    return port_;
  }


  void write(cbytes_t& command, asio::yield_context yield) {
    Port& port = this->get_port();
    asio::async_write(port, asio::buffer(command), yield);

    std::stringstream ssc;
    ssc << command;
    log(level::debug, "Sent to %: %", this->get_name(), ssc.str());
  }


  void read(bytes_t& response, asio::yield_context yield, size_t len=0x1000) {
    Port& port = this->get_port();
    asio::streambuf read_buf;
    size_t bytes_read = port.async_read_some(read_buf.prepare(len), yield);
    read_buf.commit(bytes_read);
    auto buf_begin = asio::buffers_begin(read_buf.data());
    auto buf_end = buf_begin + bytes_read;

    cbytes_t received(buf_begin, buf_end);
    std::stringstream ssr;
    ssr << received;
    log(level::debug, "Received from %: %", this->get_name(), ssr.str());

    response.insert(response.end(), buf_begin, buf_end);

    read_buf.consume(bytes_read);
  }


  bool command(
      cbytes_t& command,
      cbytes_t& expected_response,
      cbytes_t& error_response,
      asio::yield_context yield,
      int timeout=1000) {
    Port& port = this->get_port();

    // Set a timeout for the command to complete
    Port_timeout(port, timeout);

    // Write out the command
    write(command, yield);

    // ... and look for the expected response
    try {
      bytes_t response;
      for(;;) {
        read(response, yield);
        if (find_error_(response, error_response)) {
          return false;
        }
        if (find_response_(response, expected_response)) {
          return true;
        }
      }
    }
    catch (std::exception& e) {
      // Probably a timeout e.g. port cancelled by timer
      log(level::error, "%: Error executing command: %", this->get_name(), e.what());
      return false;
    }
  }

  bool query(
      cbytes_t& command,
      cbytes_t& expected_response,
      cbytes_t& error_response,
      asio::yield_context yield,
      bytes_t* response,
      size_t expected_len=0,
      int len_offset_ls=-1,
      int len_offset_ms=-1,
      int timeout=1000) {
    if (expected_len == 0) {
      size_t next_offset = static_cast<size_t>(std::max(len_offset_ls, len_offset_ms) + 1);
      expected_len = expected_response.size() + next_offset;
    }

    Port& port = this->get_port();

    // Set a timeout for the command to complete
    Port_timeout(port, timeout);

    // Write out the command
    write(command, yield);

    // ... and look for the expected response
    try {
      bytes_t data;
      int response_offset=-1;
      for(;;) {
        read(data, yield);

        if (find_error_(data, error_response)) {
          return false;
        }

        if (find_response_(data, expected_response, response_offset)) {
          if (response_offset > 0) {
            data.erase(data.begin(), data.begin() + response_offset);
          }
          if (data.size() >= expected_len) {
            if (len_offset_ls >= 0) {
              expected_len += static_cast<size_t>(
                  data[expected_response.size() + len_offset_ls]);
              if (len_offset_ms >= 0) {
                expected_len += static_cast<size_t>(
                    data[expected_response.size() + len_offset_ms]) << 8;
              }
            }
          }
          log(level::debug, "Expecting % bytes", expected_len);
          while (data.size() < expected_len) {
            read(data, yield, expected_len - data.size());
          }
          response->insert(response->end(), data.begin(), data.begin() + expected_len);
          return true;
        }
      }
    }
    catch (std::exception& e) {
      // Probably a timeout e.g. port cancelled by timer
      log(level::error, "%: Error executing query: %", this->get_name(), e.what());
      return false;
    }
  }

  auto get_executor() {
    return this->get_port().get_executor();
  }

private:
  Port port_;

  inline bool find_error_(cbytes_t& response, cbytes_t& error_response) {
    int error_offset = contains_at(response, error_response);
    if (error_offset >= 0) {
      int error_code_offset = static_cast<int>(error_response.size());
      if ((error_offset + error_code_offset) < static_cast<int>(response.size())) {
        log(level::error, "Received % error: %", this->get_name(),
            static_cast<int>(response[error_offset + error_code_offset]));
      }
      else {
        log(level::error, "Received % error", this->get_name());
      }
    }
    return error_offset >= 0;
  }

  inline bool find_response_(cbytes_t& response, cbytes_t& expected_response, int& response_offset=int_placeholder_) {
    response_offset = contains_at(response, expected_response);
    return response_offset >= 0;
  }

};



template <class DeviceClass>
struct Polling_mixin {
  virtual void start_polling() {
    auto executor = this->get_device()->get_executor();
    asio::post(
        executor,
        [executor, this]() {
          asio::spawn(executor, boost::bind(&Polling_mixin::poll_data, this, _1));
        }
    );
  }

  virtual void poll_data(asio::yield_context yield) = 0;

  void set_poll_size(size_t poll_size) {
    poll_size_ = poll_size;
  }

protected:
  DeviceClass* get_device() {
    return dynamic_cast<DeviceClass*>(this);
  }

  size_t get_poll_size() {
    return poll_size_;
  }

private:
  size_t poll_size_ = 0x200;

};  // struct Polling_mixin



template <class DeviceClass>
struct Port_polling_mixin: public Polling_mixin<DeviceClass> {

  void poll_data(asio::yield_context yield) override {
    log(level::debug, "Start polling %", this->get_device()->get_name());
    asio::streambuf buf;
    while (this->get_device()->is_connected()) {
      try {
        auto bytes_read = this->get_device()->get_port().async_read_some(
            buf.prepare(this->get_poll_size()), yield);
        double stamp = get_time();
        log(level::debug, "% read % bytes", this->get_device()->get_name(), bytes_read);
        if (bytes_read > 0) {
          buf.commit(bytes_read);
          auto buf_begin = asio::buffers_begin(buf.data());
          auto buf_end = buf_begin + buf.size();
#ifdef DEBUG
          cbytes_t data(buf_begin, buf_end);
          std::stringstream ss;
          ss << data;
          log(level::debug, "% received: %", this->get_device()->get_name(), ss.str());
#endif
          this->get_device()->handle_data(stamp, buf_begin, buf_end);
          buf.consume(bytes_read);
        }
      }
      catch (std::exception& e) {
        log(level::error, "Error while polling %: %", this->get_device()->get_name(), e.what());
        this->get_device()->disconnect();
      }
    }
    log(level::debug, "Stopped polling %", this->get_device()->get_name());
  }
};  // struct Port_polling_mixin


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
extern Device_factory_ptr& add_device_factory(const std::string& name,
    Device_factory_ptr&& device_factory);

/**
 * Have a factory from the factory registry create a device instance
 */
extern Device_ptr create_device(const std::string& name);

/**
 * Get boost property tree from device
 */
extern prtr::ptree get_device_tree(const Device& device);

/**
 * Get json string from device
 */
extern std::string get_device_json(const Device& device);


#endif  // ifndef DEVICE_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
