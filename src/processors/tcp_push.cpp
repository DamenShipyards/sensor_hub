/**
 * \file tcp_push.cpp
 * \brief Provide tcp push service processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2020 Damen Shipyards\n
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


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include "tcp_push.h"

namespace tcp_push {

struct Request {};
struct Reply {
  Reply(): status(status_type::ok), content() {}
  Reply(const std::string& s): status(status_type::ok), content(s) {}

  /// The status of the reply.
  enum status_type {
    ok = 200,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
  } status;

  /// The content to be sent in the reply.
  std::string content;

  std::vector<boost::asio::const_buffer> to_buffers() {
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(content));
    return buffers;
  }
};

struct Request_handler {
  Request_handler(const Request_handler&) = delete;
  Request_handler& operator=(const Request_handler&) = delete;

  void handle_request(const Request& req, Reply& rep) {
    (void)req;
    (void)rep;
  }
  std::string get_content(const std::string& path, std::string& content);
private:

};

struct Request_parser {
  enum result_type { good, bad, indeterminate };

  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(Request& req,
      InputIterator begin, InputIterator end) {
    (void)req;
    (void)end;
    return std::make_tuple(indeterminate, begin);
  }
};


struct Connection_manager;


class Connection
  : public std::enable_shared_from_this<Connection> {
public:
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  explicit Connection(boost::asio::ip::tcp::socket socket,
                      Connection_manager& manager, Request_handler& handler)
	  : socket_(std::move(socket)), connection_manager_(manager), request_handler_(handler) {}

  void start() {
    do_read();
  }

  void stop() {
    socket_.close();
  }

private:
  void do_read();

  void do_write();

  boost::asio::ip::tcp::socket socket_;

  Connection_manager& connection_manager_;

  Request_handler& request_handler_;

  std::array<char, 8192> buffer_;

  Request request_;

  Request_parser request_parser_;

  Reply reply_;
};


typedef std::shared_ptr<Connection> Connection_ptr;


struct Connection_manager {
  Connection_manager(const Connection_manager&) = delete;
  Connection_manager& operator=(const Connection_manager&) = delete;

  Connection_manager(): connections_() {}

  void start(Connection_ptr c) {
	  connections_.insert(c);
	  c->start();
  }

  void stop(Connection_ptr c) {
	  connections_.erase(c);
	  c->stop();
  }

  void stop_all() {
	  for (auto c: connections_)
	    c->stop();
	  connections_.clear();
  }

private:
  std::set<Connection_ptr> connections_;
};


void Connection::do_read() {
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          Request_parser::result_type result;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.data(), buffer_.data() + bytes_transferred);

          if (result == Request_parser::good) {
            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else if (result == Request_parser::bad) {
            reply_ = Reply("bad_request");
            do_write();
          }
          else {
            do_read();
          }
        }
        else if (ec != boost::asio::error::operation_aborted) {
          connection_manager_.stop(shared_from_this());
        }
      }
  );
}


void Connection::do_write()
{
  auto self(shared_from_this());
  boost::asio::async_write(socket_, reply_.to_buffers(),
      [this, self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
          // Initiate graceful connection closure.
          boost::system::error_code ignored_ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }

        if (ec != boost::asio::error::operation_aborted) {
          connection_manager_.stop(shared_from_this());
        }
      }
  );
}

}  // end tcp_pusher

/*
struct Http_server::Server {
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  explicit Server(
      boost::asio::io_context& ctx,
      std::shared_ptr<Request_handler> handler,
      const std::string& address,
      const int port)
    : io_context_(ctx),
      acceptor_(io_context_),
      connection_manager_(),
      request_handler_(handler) {
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::asio::ip::tcp::endpoint endpoint =
      *resolver.resolve(address, fmt::format("{:d}", port)).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
  }

  void stop() {
	  acceptor_.close();
	  connection_manager_.stop_all();
  }

  void set_css(const std::string& css) {
    request_handler_->set_css(css);
  }

private:
  void do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
          // Check whether the server was stopped by a signal before this
          // completion handler had a chance to run.
          if (!acceptor_.is_open()) {
            return;
          }

          if (!ec) {
            connection_manager_.start(std::make_shared<Connection>(
                  std::move(socket), connection_manager_, *request_handler_));
          }

          do_accept();
        }
    );
  }


  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  Connection_manager connection_manager_;
  std::shared_ptr<Request_handler> request_handler_;
};


Http_server::Http_server(
    boost::asio::io_context& ctx,
    std::shared_ptr<Request_handler> handler,
    const std::string& address,
    const int port)
  : server_{new Server{ctx, handler, address, port}} {
}

Http_server::~Http_server() = default;

void Http_server::stop() {
  server_->stop();
}

void Http_server::set_css(const std::string& css) {
  server_->set_css(css);
}
*/

std::string Tcp_pusher::get_json() const {
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("name"); writer.String(get_name());
  writer.String("data"); writer.StartObject();
  writer.EndObject();
  writer.EndObject();
  return sb.GetString();
}

using Tcp_pusher_factory = Processor_factory<Tcp_pusher>;
static auto& tcp_pusher_factory =
    add_processor_factory("tcp_pusher", std::move(std::make_unique<Tcp_pusher_factory>()));

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
