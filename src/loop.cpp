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
  asio::io_service& get_service() {
    return srvc_;
  }
private:
  asio::io_service srvc_;
};

int enter_loop() {
  return Service::get_instance().get_service().run();
}

void stop_loop() {
  Service::get_instance().get_service().stop();
}

asio::io_service& get_service() {
  return Service::get_instance().get_service();
}
