/**
 * \file loop.cpp
 * \brief Provide application mainloop implementation
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "loop.h"
#include "log.h"
#include "http.h"
#include "config.h"
#include "modbus.h"
#include "device.h"
#include "processor.h"

#include "driver/install.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/make_shared.hpp>

#ifdef DEBUG
#include <thread>
#include <chrono>
#endif

#include <string>
#include <memory>


namespace pt = boost::posix_time;


struct Service {

  //! Disable copying for singleton service
  Service(Service const&) = delete;

  //! Disable assignment for singleton service
  void operator=(Service const&) = delete;


  /**
   * Return singleton instance
   */
  static Service& get_instance() {
    static Service instance;
    return instance;
  }


  /**
   * Get service IO context
   */
  asio::io_context& get_context() {
    return ctx_;
  }

  Http_server& get_http_server() {
    return *http_server_;
  }

  Modbus_server& get_modbus_server() {
    return *modbus_server_;
  }


  /**
   * Start built-in HTTP server
   */
  void start_http_server(const std::string& host, const int port) {
    log(level::info, "Starting HTTP server on %:%", host, port);
    auto handler = std::make_shared<Request_handler>(devices_, processors_);
    http_server_ = std::make_unique<Http_server>(ctx_, handler, host, port);
  }


  /**
   * Stop built-in HTTP server
   */
  void stop_http_server() {
    if (http_server_ != nullptr) {
      log(level::info, "Stopping HTTP server");
      http_server_->stop();
    }
  }


  /**
   * Start built-in modbus server
   */
  void start_modbus_server(const int port) {
    log(level::info, "Starting Modbus server on port %", port);
    auto handler = boost::make_shared<Modbus_handler>(devices_, processors_);
    modbus_server_ = std::make_unique<Modbus_server>(ctx_, handler, port);
  }


  /**
   * Stop built-in modbus server
   */
  void stop_modbus_server() {
    if (modbus_server_ != nullptr) {
      log(level::info, "Stopping Modbus server");
      modbus_server_->stop();
    }
  }


  /**
   * Returns device list associated with this service
   */
  Devices& get_devices() {
    return devices_;
  }


  /**
   * Returns processor list associated with this service
   */
  Processors& get_processors() {
    return processors_;
  }


  /**
   * Setup the service device list from provided configuration
   */
  void setup_devices(boost::property_tree::ptree& cfg) {
#ifdef DEBUG
    using namespace std::chrono_literals;
    // Allow the debugger some time to connect
    std::this_thread::sleep_for(10s);
#endif
    int device_count = cfg.get("devices.count", 0);
    for (int i = 0; i < device_count; ++i) {
      std::string device_section = fmt::format("device{:d}", i);
      std::string device_type = cfg.get(fmt::format("{:s}.type", device_section), "missing_device_type");
      Device_ptr device = create_device(device_type);
      device->set_name(cfg.get(fmt::format("{:s}.name", device_section), "missing_device_name"));
      device->set_enabled(cfg.get(fmt::format("{:s}.enabled", device_section), false));
      std::string connection_string = cfg.get(fmt::format("{:s}.connection_string", device_section), "missing_connection_string");
      auto usb_address = get_usb_address(connection_string);
      check_install_usb_driver(usb_address.first, usb_address.second);
      device->set_connection_string(connection_string);
      device->enable_logging(cfg.get(fmt::format("{:s}.enable_logging", device_section), false));
      device->use_as_time_source(cfg.get(fmt::format("{:s}.use_as_time_source", device_section), false));
      devices_.push_back(std::move(device));
    }
  }


  /**
   * Setup the service processor list from provided configuration
   */
  void setup_processors(boost::property_tree::ptree& cfg) {
    int processor_count = cfg.get("processors.count", 0);
    for (int i = 0; i < processor_count; ++i) {
      std::string processor_section = fmt::format("processor{:d}", i);
      std::string processor_type = cfg.get(fmt::format("{:s}.type", processor_section), "missing_processor_type");
      Processor_ptr processor = create_processor(processor_type);
      processor->set_name(cfg.get(fmt::format("{:s}.name", processor_section), "missing_processor_name"));
      processor->set_params(cfg.get(fmt::format("{:s}.parameters", processor_section), ""));
      processor->set_filter(cfg.get(fmt::format("{:s}.filter", processor_section), ""));
      std::string device_name = cfg.get(fmt::format("{:s}.device", processor_section), "missing_processor_device");
      for (auto&& device: devices_) {
        if (device->get_name() == device_name) {
          device->add_processor(processor);
        }
      }
      processors_.push_back(processor);
    }
  }


  /**
   * Close all devices
   */
  void close_devices() {
    devices_.clear();
  }


  /**
   * Attempt to connect all configured devices
   */
  void connect_devices(asio::yield_context yield) {
    for (auto&& device: devices_) {
      if (device->is_enabled() && !device->is_connected()) {
        device->connect(yield);
      }
    }
  }


  /**
   * Try to setup device logs that haven't been setup already
   */
  void check_setup_device_logs() {
    for (auto&& device: devices_) {
      device->check_setup_device_log();
    }
  }


  /**
   * Called every second
   */
  void one_second_service(asio::yield_context yield) {
    boost::system::error_code ec;
    tmr_.async_wait(yield[ec]);
    if (!ec) {
      // Setup next tick
      tmr_.expires_from_now(pt::seconds(1));
      asio::spawn(ctx_, boost::bind(&Service::one_second_service, this, _1));

      static int counter = 0;
      ++counter;
      if (counter % 10 == 0) {
        ten_seconds_service(yield);
      }
      if (counter % 60 == 0) {
        one_minute_service(yield);
      }
      if (counter % 300 == 0) {
        five_minutes_service(yield);
      }
      if (counter % 3600 == 0) {
        hourly_service(yield);
      }
    }
  }


  /**
   * Called every ten seconds
   */
  void ten_seconds_service(asio::yield_context yield) {
  }


  /**
   * Called every minute of uptime
   */
  void one_minute_service(asio::yield_context yield) {
    connect_devices(yield);
    check_setup_device_logs();
  }


  /**
   * Called every five minutes of uptime
   */
  void five_minutes_service(asio::yield_context yield) {
    static int counter = 0;
    counter += 5;
    log(level::debug, "Uptime: % minutes", counter);
  }


  /**
   * Called every hour of uptime
   */
  void hourly_service(asio::yield_context yield) {
    static int counter = 0;
    log(level::info, "Uptime: % hours", ++counter);
  }


  /**
   * Run service
   *
   * Sets up 1 and 10 second clocks for doing regular servicing and
   * calls run on boost::asio's io_context. Blocks until explicitly
   * stopped.
   */
  int run() {
    tmr_.expires_from_now(pt::seconds(1));
    asio::spawn(ctx_, boost::bind(&Service::one_second_service, this, _1));
    return static_cast<int>(ctx_.run());
  }

private:
  /**
   * Private default constructor for singleton
   */
  Service()
      : ctx_(), 
        tmr_(ctx_),
        signals_(ctx_, SIGINT, SIGTERM),
        http_server_(nullptr),
        modbus_server_(nullptr),
        devices_(),
        processors_() {
    log(level::info, "Constructing service instance");
		signals_.async_wait(
				[this](boost::system::error_code ec, int signo)
				{
          if (!ec) {
            log(level::info, "Received signal: %", signo);
            ctx_.stop();
          }
          else {
            log(level::error, "Failed to wait on signals: %", ec.message());
          }
				}
    );
	}
  asio::io_context ctx_;
  asio::deadline_timer tmr_;
  boost::asio::signal_set signals_;
  std::unique_ptr<Http_server> http_server_;
  std::unique_ptr<Modbus_server> modbus_server_;
  Devices devices_;
  Processors processors_;
};


int enter_loop() {
  int result = 0;

  boost::property_tree::ptree& cfg = get_config();
  set_log_level(cfg.get("logging.level", "info"));
  log(level::debug, "Debug logging enabled");
  set_device_log_dir(cfg.get("logging.device_log_dir", ""));

  Service& service = Service::get_instance();

  if (cfg.get("http.enabled", false)) {
    service.start_http_server(cfg.get("http.address", "localhost"), cfg.get("http.port", 12080));
    service.get_http_server().set_css(cfg.get("http.css", ""));
  }

  if (cfg.get("modbus.enabled", false)) {
    service.start_modbus_server(cfg.get("modbus.port", 502));
  }

  service.setup_devices(cfg);
  service.setup_processors(cfg);

  log(level::info, "Running IO service");
  try {
    result = service.run();
    log(level::info, "IO service exited with code: %", result);
  }
  catch (std::exception& e) {
    log(level::error, "Exception in IO service: %", e.what());
  }
  stop_loop();
  return result;
}


void stop_loop() {
  Service& service = Service::get_instance();
  service.stop_http_server();
  service.stop_modbus_server();
  service.close_devices();
  asio::io_context& ctx = service.get_context();
  if (!ctx.stopped()) {
    log(level::info, "Stopping IO service");
    ctx.stop();
  }
}


asio::io_context& Context_provider::get_context() {
  return Service::get_instance().get_context();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
