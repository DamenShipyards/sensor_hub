/**
 * \file http.cpp
 * \brief Provide webserver implementation
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "http.h"
#include "log.h"
#include "loop.h"

#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

#include <fmt/core.h>

struct Header {
  std::string name;
  std::string value;
};

struct Request {
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<Header> headers;
};

struct Reply {
  /// The status of the reply.
  enum status_type {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  /// The headers to be included in the reply.
  std::vector<Header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> to_buffers();

  /// Get a stock reply.
  static Reply stock_reply(status_type status);
};


static struct Mapping {
  const char* extension;
  const char* mime_type;
} Mappings[] = {
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" }
};


std::string extension_to_type(const std::string& extension) {
  for (Mapping m: Mappings) {
    if (m.extension == extension)
    {
      return m.mime_type;
    }
  }

  return "text/plain";
}


namespace status_strings {

const std::string ok = "HTTP/1.0 200 OK\r\n";
const std::string created = "HTTP/1.0 201 Created\r\n";
const std::string accepted = "HTTP/1.0 202 Accepted\r\n";
const std::string no_content = "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices = "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently = "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified = "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request = "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden = "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found = "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented = "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway = "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable = "HTTP/1.0 503 Service Unavailable\r\n";


boost::asio::const_buffer to_buffer(Reply::status_type status) {
  switch (status)
  {
    case Reply::ok: return boost::asio::buffer(ok);
    case Reply::created: return boost::asio::buffer(created);
    case Reply::accepted: return boost::asio::buffer(accepted);
    case Reply::no_content: return boost::asio::buffer(no_content);
    case Reply::multiple_choices: return boost::asio::buffer(multiple_choices);
    case Reply::moved_permanently: return boost::asio::buffer(moved_permanently);
    case Reply::moved_temporarily: return boost::asio::buffer(moved_temporarily);
    case Reply::not_modified: return boost::asio::buffer(not_modified);
    case Reply::bad_request: return boost::asio::buffer(bad_request);
    case Reply::unauthorized: return boost::asio::buffer(unauthorized);
    case Reply::forbidden: return boost::asio::buffer(forbidden);
    case Reply::not_found: return boost::asio::buffer(not_found);
    case Reply::internal_server_error: return boost::asio::buffer(internal_server_error);
    case Reply::not_implemented: return boost::asio::buffer(not_implemented);
    case Reply::bad_gateway: return boost::asio::buffer(bad_gateway);
    case Reply::service_unavailable: return boost::asio::buffer(service_unavailable);
    default:
      return boost::asio::buffer(internal_server_error);
  }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings


std::vector<boost::asio::const_buffer> Reply::to_buffers() {
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(status_strings::to_buffer(status));
  for (std::size_t i = 0; i < headers.size(); ++i) {
    Header& h = headers[i];
    buffers.push_back(boost::asio::buffer(h.name));
    buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
    buffers.push_back(boost::asio::buffer(h.value));
    buffers.push_back(boost::asio::buffer(misc_strings::crlf));
  }
  buffers.push_back(boost::asio::buffer(misc_strings::crlf));
  buffers.push_back(boost::asio::buffer(content));
  return buffers;
}


namespace stock_replies {

struct Status_reply {
  int code;
  const char* msg;
};

const char ok[] = "";
const char status[] =
  "<html>"
  "<head><title>{1}</title></head>"
  "<body><h1>{0} {1}</h1></body>"
  "</html>";

const Status_reply created = {201, "Created"};
const Status_reply accepted = {202, "Accepted"};
const Status_reply no_content = {204, "No Content"};
const Status_reply multiple_choices = {300, "Multiple Choices"};
const Status_reply moved_permanently = {301, "Moved Permanently"};
const Status_reply moved_temporarily = {302, "Moved Temporarily"};
const Status_reply not_modified = {304, "Not Modified"};
const Status_reply bad_request = {400, "Bad Request"};
const Status_reply unauthorized= {401, "Unauthorized"};
const Status_reply forbidden = {403, "Forbidden"};
const Status_reply not_found = {404, "Not Found"};
const Status_reply internal_server_error = {500, "Internal Server Error"};
const Status_reply not_implemented = {501, "Not Implemented"};
const Status_reply bad_gateway = {502, "Bad Gateway"};
const Status_reply service_unavailable = {503, "Service Unavailable"};

std::string to_html(const Status_reply& status_reply) {
  return fmt::format(status, status_reply.code, status_reply.msg);
}

std::string to_string(Reply::status_type status) {
  switch (status)
  {
    case Reply::ok: return ok;
    case Reply::created: return to_html(created);
    case Reply::accepted: return to_html(accepted);
    case Reply::no_content: return to_html(no_content);
    case Reply::multiple_choices: return to_html(multiple_choices);
    case Reply::moved_permanently: return to_html(moved_permanently);
    case Reply::moved_temporarily: return to_html(moved_temporarily);
    case Reply::not_modified: return to_html(not_modified);
    case Reply::bad_request: return to_html(bad_request);
    case Reply::unauthorized: return to_html(unauthorized);
    case Reply::forbidden: return to_html(forbidden);
    case Reply::not_found: return to_html(not_found);
    case Reply::internal_server_error: return to_html(internal_server_error);
    case Reply::not_implemented: return to_html(not_implemented);
    case Reply::bad_gateway: return to_html(bad_gateway);
    case Reply::service_unavailable: return to_html(service_unavailable);
    default:
      return to_html(internal_server_error);
  }
}

} // namespace stock_replies

Reply Reply::stock_reply(Reply::status_type status) {
  Reply rep;
  rep.status = status;
  rep.content = stock_replies::to_string(status);
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/html";
  return rep;
}


class Request_handler {
public:
  Request_handler(const Request_handler&) = delete;
  Request_handler& operator=(const Request_handler&) = delete;

  explicit Request_handler() {};

  void handle_request(const Request& req, Reply& rep);

private:
  static bool url_decode(const std::string& in, std::string& out);
};



void Request_handler::handle_request(const Request& req, Reply& rep) {
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.uri, request_path)) {
    rep = Reply::stock_reply(Reply::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos) {
    rep = Reply::stock_reply(Reply::bad_request);
    return;
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/') {
    request_path += "index.html";
  }

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of("/");
  std::size_t last_dot_pos = request_path.find_last_of(".");
  std::string extension;
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
    extension = request_path.substr(last_dot_pos + 1);
  }

  // Open the file to send back.
  //std::string full_path = doc_root_ + request_path;
  //std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  //if (!is) {
  //  rep = Reply::stock_reply(Reply::not_found);
  //  return;
  //}

  // Fill out the reply to be sent to the client.
  rep.status = Reply::ok;
  static const char buf[] = "Welcome to Damen Sensor Hub";
  rep.content.append(buf, sizeof(buf));
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = extension_to_type(extension);
}


bool Request_handler::url_decode(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }
    else if (in[i] == '+') {
      out += ' ';
    }
    else {
      out += in[i];
    }
  }
  return true;
}

struct Request_parser {
  Request_parser(): state_(method_start) {}

  void reset()  {
	state_ = method_start;
  }

  enum result_type { good, bad, indeterminate };

  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(Request& req,
      InputIterator begin, InputIterator end) {
    while (begin != end) {
      result_type result = consume(req, *begin++);
      if (result == good || result == bad)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  }

private:
  result_type consume(Request& req, char input);

  static bool is_char(int c);

  static bool is_ctl(int c);

  static bool is_tspecial(int c);

  static bool is_digit(int c);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};



Request_parser::result_type Request_parser::consume(Request& req, char input) {
  switch (state_) {
    case method_start:
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      else {
        state_ = method;
        req.method.push_back(input);
        return indeterminate;
      }
    case method:
      if (input == ' ') {
        state_ = uri;
        return indeterminate;
      }
      else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      else {
        req.method.push_back(input);
        return indeterminate;
      }
    case uri:
      if (input == ' ') {
        state_ = http_version_h;
        return indeterminate;
      }
      else if (is_ctl(input)) {
        return bad;
      }
      else {
        req.uri.push_back(input);
        return indeterminate;
      }
    case http_version_h:
      if (input == 'H') {
        state_ = http_version_t_1;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_t_1:
      if (input == 'T') {
        state_ = http_version_t_2;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_t_2:
      if (input == 'T') {
        state_ = http_version_p;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_p:
      if (input == 'P') {
        state_ = http_version_slash;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_slash:
      if (input == '/') {
        req.http_version_major = 0;
        req.http_version_minor = 0;
        state_ = http_version_major_start;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_major_start:
      if (is_digit(input)) {
        req.http_version_major = req.http_version_major * 10 + input - '0';
        state_ = http_version_major;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_major:
      if (input == '.') {
        state_ = http_version_minor_start;
        return indeterminate;
      }
      else if (is_digit(input)) {
        req.http_version_major = req.http_version_major * 10 + input - '0';
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_minor_start:
      if (is_digit(input)) {
        req.http_version_minor = req.http_version_minor * 10 + input - '0';
        state_ = http_version_minor;
        return indeterminate;
      }
      else {
        return bad;
      }
    case http_version_minor:
      if (input == '\r') {
        state_ = expecting_newline_1;
        return indeterminate;
      }
      else if (is_digit(input)) {
        req.http_version_minor = req.http_version_minor * 10 + input - '0';
        return indeterminate;
      }
      else {
        return bad;
      }
    case expecting_newline_1:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      }
      else {
        return bad;
      }
    case header_line_start:
      if (input == '\r') {
        state_ = expecting_newline_3;
        return indeterminate;
      }
      else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
        state_ = header_lws;
        return indeterminate;
      }
      else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      else {
        req.headers.push_back(Header());
        req.headers.back().name.push_back(input);
        state_ = header_name;
        return indeterminate;
      }
    case header_lws:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      }
      else if (input == ' ' || input == '\t') {
        return indeterminate;
      }
      else if (is_ctl(input)) {
        return bad;
      }
      else {
        state_ = header_value;
        req.headers.back().value.push_back(input);
        return indeterminate;
      }
    case header_name:
      if (input == ':') {
        state_ = space_before_header_value;
        return indeterminate;
      }
      else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      else {
        req.headers.back().name.push_back(input);
        return indeterminate;
      }
    case space_before_header_value:
      if (input == ' ') {
        state_ = header_value;
        return indeterminate;
      }
      else {
        return bad;
      }
    case header_value:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      }
      else if (is_ctl(input)) {
        return bad;
      }
      else {
        req.headers.back().value.push_back(input);
        return indeterminate;
      }
    case expecting_newline_2:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      }
      else {
        return bad;
      }
    case expecting_newline_3:
      return (input == '\n') ? good : bad;
    default:
      return bad;
  }
}


bool Request_parser::is_char(int c) {
  return c >= 0 && c <= 127;
}


bool Request_parser::is_ctl(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}


bool Request_parser::is_tspecial(int c) {
  switch (c) {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
      return true;
    default:
      return false;
  }
}


bool Request_parser::is_digit(int c) {
  return c >= '0' && c <= '9';
}


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
            reply_ = Reply::stock_reply(Reply::bad_request);
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


struct Http_server::Server {
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  explicit Server(boost::asio::io_context& ctx, const std::string& address, const int port);

  void stop() {
	  acceptor_.close();
	  connection_manager_.stop_all();
  }

private:
  void do_accept();
  void do_await_stop();

  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  Connection_manager connection_manager_;
  Request_handler request_handler_;
};


Http_server::Server::Server(boost::asio::io_context& ctx, const std::string& address, const int port)
  : io_context_(ctx), acceptor_(io_context_),
    connection_manager_(), request_handler_() {
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


void Http_server::Server::do_accept() {
  acceptor_.async_accept(
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
		// Check whether the server was stopped by a signal before this
		// completion handler had a chance to run.
		if (!acceptor_.is_open()) {
		  return;
	    }

		if (!ec) {
		  connection_manager_.start(std::make_shared<Connection>(
				std::move(socket), connection_manager_, request_handler_));
		}

	    do_accept(); 
      }
  );
}

Http_server::Http_server(boost::asio::io_context& ctx, const std::string& address, const int port)
  : server_{new Server{ctx, address, port}} {
}

Http_server::~Http_server() = default;

void Http_server::stop() {
  server_->stop();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
