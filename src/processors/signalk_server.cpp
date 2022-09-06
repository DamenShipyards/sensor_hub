/**
 * \file signalk_server.cpp
 * \brief Provide SignalK server implementation
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

#include "signalk_server.h"
#include "../log.h"
using boost::asio::ip::tcp;
  void tcp_connection::send(std::string message){
     boost::asio::async_write(socket_, boost::asio::buffer(message),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  void tcp_connection::start()  {
    log(level::debug, "start received");
    server_->add_connection(shared_from_this());
  }

  tcp_connection::tcp_connection(boost::asio::io_context& io_context, tcp_server* server)
    : socket_(io_context), server_(server)
  {
  }

  void tcp_connection::handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/)
  {
  }






  tcp_server::tcp_server(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 4123))
  {
    start_accept();

  }
  void tcp_server::add_connection(boost::shared_ptr<tcp_connection> connection){
    connections_.push_back(connection);
  }
  void tcp_server::send(std::string delta){
    for (boost::shared_ptr<tcp_connection> connection : connections_)
    {
      connection->send(delta);
    }
  }

  void tcp_server::start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_,this);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void tcp_server::handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
        log(level::debug, "connection received");
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }
  std::string tcp_server::get_status() const {
    return std::to_string(connections_.size());
  }
