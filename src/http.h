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

#include "device.h"

struct Request;
struct Reply;

struct Request_handler {
  Request_handler(const Request_handler&) = delete;
  Request_handler& operator=(const Request_handler&) = delete;

  explicit Request_handler(Devices& devices): devices_(devices), css_() {};

  void handle_request(const Request& req, Reply& rep);
  std::string get_content(const std::string& path, std::string& content);

  void set_css(const std::string& css) {
    css_ = css;
  }
private:
  Devices& devices_;
  std::string css_;
  static bool url_decode(const std::string& in, std::string& out);
};


struct Http_server {
  explicit Http_server(
      boost::asio::io_context& ctx, 
      std::shared_ptr<Request_handler> handler,
      const std::string& address, 
      const int port);
  ~Http_server();
  void stop();
  void set_css(const std::string& css);
private:
  struct Server;
  std::unique_ptr<Server> server_;
};

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
