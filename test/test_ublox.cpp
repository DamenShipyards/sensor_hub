#define BOOST_TEST_MODULE ublox_test
#include "../src/devices/ublox.h" 

#include "test_common.h"

#include <memory>

using namespace ubx;


BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<Ublox_NEO_M8U<asio::serial_port, Ctx> >();
}

BOOST_AUTO_TEST_CASE(connection_test, *ut::precondition(xsens_available)) {
  asio::io_context& ctx = Ctx::get_context();
  Ublox_NEO_M8U<asio::serial_port, Ctx> ublox;
  asio::deadline_timer tmr(ctx, posix_time::milliseconds(5000));
  tmr.async_wait(
      [&ctx](boost::system::error_code ec) {
        ctx.stop();
      }
  );

  ublox.set_name("ublox-test");
  ublox.set_connection_string("/dev/ttyACM0");
  BOOST_TEST(!ublox.is_connected());
  asio::spawn(ctx, 
      [&](asio::yield_context yield) {
        ublox.connect(yield);
      }
  );
  ctx.run();
  BOOST_TEST(ublox.is_connected());
}
