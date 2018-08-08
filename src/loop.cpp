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

#include <boost/asio.hpp>

namespace asio = boost::asio;

int mainloop() {
  asio::io_service srvc;

  srvc.run();


  return 0;
}
