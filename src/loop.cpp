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

#include "log.h"
#include "http.h"
#include "loop.h"
#include "config.h"

#include <string>


struct Service {
  Service(): ctx_() {
  }
  Service(Service const&) = delete;
  void operator=(Service const&) = delete;

  static Service& get_instance() {
    static Service instance; 
    return instance;
  }
  asio::io_context& get_context() {
    return ctx_;
  }
  void start_http_server(const std::string& host, const int port) {
    log(level::info, "Starting HTTP server");
    http_server_ = std::make_unique<Http_server>(ctx_, host, port);
  }
  void stop_http_server() {
    if (http_server_ != nullptr) {
      log(level::info, "Stopping HTTP server");
      http_server_->stop();
    }
  }

private:
  asio::io_context ctx_;
  std::unique_ptr<Http_server> http_server_;
};


int enter_loop() {
  int result = 0;
  Service& service = Service::get_instance();
  asio::io_context& ctx = service.get_context();

  boost::property_tree::ptree& cfg = get_config();

  if (cfg.get("http.active", false)) {
    service.start_http_server(cfg.get("http.address", "localhost"), cfg.get("http.port", 12080));
  }
  log(level::info, "Running IO service");
  try {
    result = static_cast<int>(ctx.run());
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


asio::io_context& get_context() {
  return Service::get_instance().get_context();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
