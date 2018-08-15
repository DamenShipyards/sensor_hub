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
#include "www.h"
#include "loop.h"
#include "config.h"


namespace asio = boost::asio;

struct Service {
  Service(): srvc_() {
  }
  Service(Service const&) = delete;
  void operator=(Service const&) = delete;

  static Service& get_instance() {
    static Service instance; 
    return instance;
  }
  asio::io_context& get_context() {
    return srvc_;
  }
private:
  asio::io_context srvc_;
};

static std::unique_ptr<Www_server> www_server;

int enter_loop() {
  int result = 0;
  asio::io_context& ctx = Service::get_instance().get_context();

  boost::property_tree::ptree& cfg = get_config();

  if (cfg.get("www.active", false)) {
    log(level::info, "Starting WWW server");
    www_server = std::make_unique<Www_server>(ctx, cfg.get("www.address", "localhost"), cfg.get("www.port", 12080));
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
  if (www_server != nullptr) {
    log(level::info, "Stopping WWW server");
    www_server->stop();
  }
  asio::io_context& ctx = Service::get_instance().get_context();
  if (!ctx.stopped()) {
    log(level::info, "Stopping IO service");
    Service::get_instance().get_context().stop();
  }
}

asio::io_context& get_context() {
  return Service::get_instance().get_context();
}

