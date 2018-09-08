#define BOOST_TEST_MODULE xsens_test
#include "../src/xsens.h" 
#include "../src/usb.h" 

#include "test_common.h"

#include <memory>


BOOST_AUTO_TEST_CASE(construction_test)
{
  asio::io_context& ctx = Ctx::get_context();
  Device_ptr dev = std::make_unique<Xsens_MTi_G_710<Usb, Ctx> >();

  asio::deadline_timer tmr(ctx, posix_time::milliseconds(2500));
  tmr.async_wait(
      [&ctx](boost::system::error_code ec) {
        ctx.stop();
      }
  );

  dev->set_connection_string("2639:0017");
  BOOST_TEST(!dev->is_connected());
  asio::spawn(ctx, 
      [&](asio::yield_context yield) {
        dev->connect(yield);
      }
  );
  ctx.run();
  BOOST_TEST(dev->is_connected());
}
