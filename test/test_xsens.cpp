#define BOOST_TEST_MODULE xsens_test
#include "../src/devices/xsens.h" 
#include "../src/devices/xsens_impl.h" 
#include "../src/usb.h" 

#include "test_common.h"

#include <memory>

using namespace xsens;


BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<MTi_G_710<Usb, Ctx> >();
}

BOOST_AUTO_TEST_CASE(connection_test, *ut::precondition(xsens_available)) {
  asio::io_context& ctx = Ctx::get_context();
  MTi_G_710<Usb, Ctx> xsens;
  asio::deadline_timer tmr(ctx, pt::milliseconds(3000));
  tmr.async_wait(
      [&ctx](boost::system::error_code ec) {
        ctx.stop();
      }
  );

  xsens.set_name("xsens-test");
  xsens.set_connection_string("2639:0017");
  BOOST_TEST(!xsens.is_connected());
  asio::spawn(ctx, 
      [&](asio::yield_context yield) {
        xsens.connect(yield);
      }
  );
  ctx.run();
  BOOST_TEST(xsens.is_connected());
}
