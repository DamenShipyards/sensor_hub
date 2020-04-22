#define BOOST_TEST_MODULE regex_test
#include "../src/types.h"
#include "../src/devices/regex.h"
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

using namespace regexp;
namespace fs = boost::filesystem;
namespace prtr = boost::property_tree;

static prtr::ptree opts;

using Regex = Regex_device<Socket, Ctx>;

decltype(auto) create() {
  auto device = std::make_shared<Regex>();
  device->set_connection_string("127.0.0.1:1999");
  return device;
}

template<typename DEV>
void set_options(DEV& dev) {
  prtr::ptree options;
  options.put("ax.filter", "^([0-9.]+)$");
  options.put("ay.filter", "^([0-9]+),([0-9]+)$");
  options.put("ay.multiplier1", 0.02);
  options.put("az.filter", "^([0-9,]+)$");
  options.put("ut.filter", "^([0-9\\-]+T[0-9:]+)\\.([0-9]+)Z$");
  options.put("ut.format0", "%Y-%m-%dT%H:%M:%S");
  options.put("ut.format1", "f");
  options.put("ut.multiplier1", "0.001");
  // Abuse of other quantity for time value
  options.put("vx.filter", "^([0-9\\-]+ [0-9:.]+)$");
  options.put("vx.format0", "dt");
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


// Create a tcp server that spits out pi for testing. 
struct tcp_connection {
  using pointer = tcp_connection*;
  static pointer create(boost::asio::io_context& io_context) {
    return new tcp_connection(io_context);
  }
  auto& socket() {
    return socket_;
  }
  void start() {
    message_ = "3.1415927\n2020-04-22T17:10:43.782Z\n2020-04-22 17:10:43.782\n1,22";
    boost::asio::async_write(socket_, boost::asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
private:
  tcp_connection(boost::asio::io_context& io_context): socket_(io_context) {}
  void handle_write(const boost::system::error_code&, size_t) {
    log(level::info, "Written the data to the socket");
  }
  tcp::socket socket_;
  std::string message_;
};


struct tcp_server {
  tcp_server(boost::asio::io_context& io_context)
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
        boost::asio::placeholders::error));
  }
  void handle_accept(tcp_connection::pointer connection,
      const boost::system::error_code& error) {
    if (!error) {
      connection->start();
    }
  }
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  // We'll only support a single connection
  tcp_connection* connection_;
};


tcp_server server(Ctx::get_context());


BOOST_FIXTURE_TEST_CASE(full_test, Log, * utf::tolerance(0.00001)) {
  log(level::info, "Starting regex full test");
  auto dev = create();
  set_options(dev);
  // Connect the device as soon as the context starts running
  asio::spawn(Ctx::get_context(), boost::bind(&Regex::connect, dev, _1));
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
