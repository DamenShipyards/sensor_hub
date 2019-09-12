#define BOOST_TEST_MODULE modbus_test
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../src/modbus.h" 
#include "../src/device.h" 
#include "../src/processor.h"
#include "../3rdparty/modbus/include/modbus/client.hpp"

namespace asio = boost::asio;


static int result1 = 0;
static int result2 = 0;

struct Client: public modbus::Client {
  using modbus::Client::Client;

  void on_read_reply(
      modbus::tcp_mbap const & header, 
      modbus::response::read_holding_registers const & response, 
      boost::system::error_code const & error ) {
    if (!error) {
      if (response.values.size() == 2) {
        result1 = response.values[0];
        result2 = response.values[1];
      }
    }
  }

  void on_write_reply(
      modbus::tcp_mbap const & header, 
      modbus::response::write_multiple_registers const & response, 
      boost::system::error_code const & error) {
    if (!error)
      read_holding_registers(0, 128, 2, 
          boost::bind(&Client::on_read_reply, this, _1, _2, _3));
  }

  void on_connect(boost::system::error_code const & error) {
    if (!error)
      write_multiple_registers(0, 128, {1234, 4321}, 
          boost::bind(&Client::on_write_reply, this, _1, _2, _3));
  }
};


BOOST_AUTO_TEST_CASE(talk_test) {
  asio::io_context ctx;

  Client client{ctx};
  Devices devices;
  Processors processors;
  auto handler = boost::make_shared<Modbus_handler>(devices, processors);
  Modbus_server server{ctx, handler, 1502};

  client.connect("localhost", 1502, boost::bind(&Client::on_connect, &client, _1));

  asio::deadline_timer stopper(ctx, boost::posix_time::seconds(2));
  stopper.async_wait(
      [&server](const boost::system::error_code&) {
        server.stop();
      }
  );

  ctx.run();

  BOOST_TEST(result1 == 1234);
  BOOST_TEST(result2 == 4321);
}
