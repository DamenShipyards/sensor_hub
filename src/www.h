/**
 * \file www.h
 * \brief Provide webserver interface
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef WWW_H_
#define WWW_H_

#include <string>
#include <memory>
#include <boost/asio.hpp>


struct Www_server {
  explicit Www_server(boost::asio::io_context& ctx, const std::string& address, const std::string& port);
  ~Www_server();
private:
  struct Server;
  std::unique_ptr<Server> server_;
};

#endif
