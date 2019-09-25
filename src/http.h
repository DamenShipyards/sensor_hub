/**
 * \file http.h
 * \brief Provide webserver interface
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2019 Damen Shipyards
 * \license
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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

  explicit Request_handler(const Devices& devices, const Processors& processors)
    : devices_(devices), processors_(processors), css_() {};

  void handle_request(const Request& req, Reply& rep);
  std::string get_content(const std::string& path, std::string& content);

  void set_css(const std::string& css) {
    css_ = css;
  }
private:
  const Devices& devices_;
  const Processors& processors_;
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
