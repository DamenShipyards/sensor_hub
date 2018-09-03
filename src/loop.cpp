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
#include "device.h"

#include <fmt/core.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/asio/spawn.hpp>

#include <string>

namespace posix_time = boost::posix_time;


struct Service {
  // Disable copying and assignment for singleton service
  Service(Service const&) = delete;
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

  /**
   * Start built-in HTTP server
   */
  void start_http_server(const std::string& host, const int port) {
    log(level::info, "Starting HTTP server");
    http_server_ = std::make_unique<Http_server>(ctx_, host, port);
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
   * Returns device list associated with this service
   */
  Devices& get_devices() {
    return devices_;
  }

  /**
   * Setup the service device list from provided configuration
   */
  void setup_devices(boost::property_tree::ptree& cfg) {
    int device_count = cfg.get("devices.count", 0);
    for (int i = 0; i < device_count; ++i) {
      std::string device_section = fmt::format("device{:d}", i);
      std::string device_type = cfg.get(fmt::format("{:s}.type", device_section), "missing_device_type");
      Device_ptr device = create_device(device_type);
      device->set_name(cfg.get(fmt::format("{:s}.name", device_section), "missing_device_name"));
      device->set_connection_string(cfg.get(fmt::format("{:s}.connection_string", device_section), "missing_connection_string"));
      devices_.push_back(std::move(device));
    }
  }

  /**
   * Called every second
   */
  void one_second_service(asio::yield_context yield) {
    asio::deadline_timer tmr(ctx_, posix_time::seconds(1));
    tmr.async_wait(yield);
    asio::spawn(ctx_, boost::bind(&Service::one_second_service, this, _1));
  }

  /**
   * Called every ten seconds
   *
   * Checks all devices for their connection status and tries to connect
   * them if they weren't already
   */
  void ten_seconds_service(asio::yield_context yield) {
    asio::deadline_timer tmr(ctx_, posix_time::seconds(10));
    tmr.async_wait(yield);
    asio::spawn(ctx_, boost::bind(&Service::ten_seconds_service, this, _1));
    for (auto&& device: devices_) {
      if (!device->is_connected()) {
        device->connect(yield);
      }
    }
  }

  /**
   * Run service
   *
   * Sets up 1 and 10 second clocks for doing regular servicing and
   * calls run on boost::asio's io_context. Blocks until explicitly 
   * stopped.
   */
  int run() {
    asio::spawn(ctx_, boost::bind(&Service::one_second_service, this, _1));
    asio::spawn(ctx_, boost::bind(&Service::ten_seconds_service, this, _1));
    return static_cast<int>(ctx_.run());
  }

private:
  /**
   * Private default constructor for singleton
   */
  Service(): ctx_(), http_server_(nullptr), devices_() {
  }
  asio::io_context ctx_;
  std::unique_ptr<Http_server> http_server_;
  Devices devices_;
};




int enter_loop() {
  int result = 0;
  Service& service = Service::get_instance();

  boost::property_tree::ptree& cfg = get_config();

  if (cfg.get("http.active", false)) {
    service.start_http_server(cfg.get("http.address", "localhost"), cfg.get("http.port", 12080));
  }

  service.setup_devices(cfg);

  log(level::info, "Running IO service");
  try {
    result = service.run();
  }
  catch (std::exception& e) {
    log(level::error, "Exception in IO service: %", e.what());
    throw;
  }
  log(level::info, "IO service exited with code: %", result);
  return result;
}


void stop_loop() {
  Service& service = Service::get_instance();
  service.stop_http_server();
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
