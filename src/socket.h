/**
 * \file socket.h
 * \brief Provide convenience header for asio::ip::tcp::socket
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2020 Damen Shipyards
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


#ifndef SOCKET_H_
#define SOCKET_H_

#include <vector>
#include <string>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>

using tcp_socket = boost::asio::ip::tcp::socket;
namespace ip = boost::asio::ip;

struct Socket: public tcp_socket {
  using tcp_socket::tcp_socket;

  void open(const std::string& device_str) {
    std::vector<std::string> fields;
    boost::split(fields, device_str, [](char c) { return c == ':'; });
    std::string host_s = "127.0.0.1";
    std::string port_s = "2947";
    switch (fields.size()) {
      case 2:
        port_s = fields[1];
        /* fall through */
      case 1:
        host_s = fields[0];
        break;
      default:
        log(level::info, "Using default local gpsd");
    }

    size_t len = 0;
    auto host = ip::address::from_string(host_s);
    auto port = int(std::stol(port_s, &len, 10));
    auto endpoint = ip::tcp::endpoint(host, port);

    tcp_socket::connect(endpoint);
    log(level::info, "Succesfully TCP socket %, %", host_s, port_s);
  }
};



#endif  // SOCKET_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2 filetype=cpp
