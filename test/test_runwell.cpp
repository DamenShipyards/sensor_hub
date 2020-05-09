#define BOOST_TEST_MODULE runwell_test
#include "../src/types.h"
#include "../src/devices/runwell.h"
#include "../src/log.h"
#include "../src/socket.h"
#include "../src/quantities.h"

#include "test_common.h"

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

using namespace runwell;
namespace fs = boost::filesystem;
namespace prtr = boost::property_tree;

static prtr::ptree opts;

using Runwell = Runwell_device<Socket, Ctx>;

decltype(auto) create() {
  auto device = std::make_shared<Runwell>();
  device->set_connection_string("127.0.0.1:1999");
  return device;
}

template<typename DEV>
void set_options(DEV& dev) {
  prtr::ptree options;
  options.put("interval", "1");
  dev->set_options(options);
}


BOOST_AUTO_TEST_CASE(construction_test) {
  create();
}


BOOST_AUTO_TEST_CASE(options_test) {
  auto dev =  create();
  set_options(dev);
}


using boost::asio::ip::tcp;
namespace asio = boost::asio;


// Create a tcp server that serves as a mock for a serial runwell device. 
struct tcp_connection {
  using pointer = tcp_connection*;
  static pointer create(boost::asio::io_context& io_context) {
    return new tcp_connection(io_context);
  }
  auto& socket() {
    return socket_;
  }
  void start() {
      socket_.async_read_some(asio::buffer(read_buf_, 16),
          boost::bind(&tcp_connection::handle_read, this, 
            asio::placeholders::error, 
            asio::placeholders::bytes_transferred));
  }

  void handle_read( const boost::system::error_code& ec, std::size_t bytes_read) {
    if (!ec && bytes_read > 0) {
      switch (read_buf_[0]) {
        case '?':
          message_ = "Freq = 69767\nVolt = 18.927\nVSig = 18.984\nVBridge = 27.366\nIBridge = 0.630\nPOTs: 2253 2187 1\n";
          break;
        case 'a':
          message_ = "12:34:56:78:90:12\n";
          break;
        case 'l':
          message_ = "1,0,224,69767,18.927,18.984,27.366,0.630\n";
          break;
        case 'h':
          message_ = "Runwell mock driver\nVersion 0.1, May 9th, 2020\n";
          break;
        default:
          message_ = "Invalid request\n";
      }
    }
    asio::async_write(socket_, asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred));
    start();
  }
private:
  tcp_connection(asio::io_context& io_context): socket_(io_context) {}
  void handle_write(const boost::system::error_code&, size_t) {
    log(level::info, "Written the data to the socket");
  }
  tcp::socket socket_;
  std::string message_;
  char read_buf_[16];
};


struct tcp_server {
  tcp_server(asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 1999)),
      connection_(nullptr) {
    start_accept();
  }
  ~tcp_server() {
    if (connection_ != nullptr) {
      delete connection_;
    }
  }
private:
  void start_accept() {
    connection_ = tcp_connection::create(io_context_);
    acceptor_.async_accept(connection_->socket(),
        boost::bind(&tcp_server::handle_accept, this, connection_,
        asio::placeholders::error));
  }
  void handle_accept(tcp_connection::pointer connection,
      const boost::system::error_code& error) {
    if (!error) {
      connection->start();
    }
  }
  asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  // We'll only support a single connection
  tcp_connection* connection_;
};


tcp_server server(Ctx::get_context());


BOOST_FIXTURE_TEST_CASE(full_test, Log, * utf::tolerance(0.00001)) {
  log(level::info, "Starting runwell full test");
  auto dev = create();
  set_options(dev);
  // Connect the device as soon as the context starts running
  asio::spawn(Ctx::get_context(), boost::bind(&Runwell::connect, dev, _1));
  Ctx::run(1);
  auto value = dev->get_value(Quantity::ax);
  BOOST_TEST(value == 3.1415927);
  value = dev->get_value(Quantity::ay);
  BOOST_TEST(value == 1.44);
  value = dev->get_value(Quantity::az);
  BOOST_TEST(value == 1.22);
  value = dev->get_value(Quantity::ut);
  BOOST_TEST(value == 1587575443.782);
  value = dev->get_value(Quantity::vx);
  BOOST_TEST(value == 1587575443.782);
}
