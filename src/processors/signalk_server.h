/**
 * \file signalk_server.h
 * \brief Provide SignalK server interfacer
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards\n
 *            Copyright (C) 2020-2022 Orca Software
 *
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

#ifndef SIGNALK_SERVER_H_
#define SIGNALK_SERVER_H_

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <list>

using boost::asio::ip::tcp;
class tcp_server;
class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context,tcp_server* server){
    return pointer(new tcp_connection(io_context, server));
  }

  tcp::socket& socket()
  {
    return socket_;
  }
  void start();
  void send(std::string message);
  void close();

private:
  tcp_connection(boost::asio::io_context& io_context, tcp_server* server);

  void handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/);

  tcp::socket socket_;
  tcp_server* server_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_context& io_context);
  void add_connection(boost::shared_ptr<tcp_connection> connection);
  void send(std::string delta);
  void stop();
  std::string get_status() const;

private:
  void start_accept();

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error);

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  std::list<boost::shared_ptr<tcp_connection> > connections_;
  bool stopped_;
};


#endif // SIGNALK_SERVER_H_
