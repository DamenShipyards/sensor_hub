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

struct Socket: public tcp_socket {
  using tcp_socket::tcp_socket;

  void open(const std::string& device_str) {
    (void)device_str;
  }
};



#endif  // SOCKET_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2 filetype=cpp
