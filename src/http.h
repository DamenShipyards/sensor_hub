/**
 * \file http.h
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

#ifndef HTTP_H_
#define HTTP_H_

#include <string>
#include <memory>
#include <boost/asio.hpp>


struct Http_server {
  explicit Http_server(boost::asio::io_context& ctx, const std::string& address, const int port);
  ~Http_server();
  void stop();
private:
  struct Server;
  std::unique_ptr<Server> server_;
};

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
