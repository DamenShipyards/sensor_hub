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

int enter_loop() {
  asio::io_context& ctx = Service::get_instance().get_context();
  Www_server www_server{ctx, "localhost", "11180"};
  return ctx.run();
}

void stop_loop() {
  Service::get_instance().get_context().stop();
}

asio::io_context& get_context() {
  return Service::get_instance().get_context();
}

