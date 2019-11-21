#define BOOST_TEST_MODULE xsens_test
#include "../src/devices/xsens.h" 
#include "../src/devices/xsens_impl.h" 
#include "../src/usb.h" 
#include "../src/log.h" 
#include "../src/serial.h" 

#include "test_common.h"

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>



using namespace xsens;
namespace fs = boost::filesystem;
namespace prtr = boost::property_tree;

static prtr::ptree opts;

BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<MTi_G_710<Usb, Ctx> >();
}

BOOST_AUTO_TEST_CASE(connection_test_g_710, *ut::precondition(xsens_g_710_available)) {
  asio::io_context& ctx = Ctx::get_context();
  MTi_G_710<Usb, Ctx> mti_g_710;
  asio::deadline_timer tmr(ctx, pt::milliseconds(3000));
  tmr.async_wait(
      [&](boost::system::error_code) {
        BOOST_TEST(mti_g_710.is_connected());
        log(level::info, "Disconnecting MTi G 710");
        mti_g_710.disconnect();
        log(level::info, "Stopping IO context");
        ctx.stop();
      }
  );

  mti_g_710.set_name("xsens-g_710-test");
  mti_g_710.set_connection_string("2639:0017");
  opts.put("filter_profile", 1);
  mti_g_710.set_options(opts);
  BOOST_TEST(!mti_g_710.is_connected());
  asio::spawn(ctx, 
      [&](asio::yield_context yield) {
        mti_g_710.connect(yield);
      }
  );
  ctx.run();
  ctx.restart();
}

BOOST_AUTO_TEST_CASE(connection_test_670, *ut::precondition(xsens_670_available)) {
  asio::io_context& ctx = Ctx::get_context();
  MTi_670<Serial, Ctx> mti_670;
  //MTi_670<Usb, Ctx> mti_670;

  asio::deadline_timer tmr(ctx, pt::milliseconds(3000));
  tmr.async_wait(
      [&](boost::system::error_code) {
        BOOST_TEST(mti_670.is_connected());
        log(level::info, "Disconnecting MTi 670");
        mti_670.disconnect();
        log(level::info, "Stopping IO context");
        ctx.stop();
      }
  );

  mti_670.set_name("xsens-670-test");
  fs::path p = "/dev/sensor_hub/xsens_mti_usb_serial-ttyUSB0";
  if (!fs::exists(p)) {
    p = "/dev/sensor_hub/xsens_mti_usb_serial-ttyUSB1";
  }
  BOOST_TEST(fs::exists(p));
  //mti_670.set_connection_string(p.string() + ":921600");
  mti_670.set_connection_string(p.string() + ":115200");
  //mti_670.set_connection_string("2639:0300");
  BOOST_TEST(!mti_670.is_connected());
  asio::spawn(ctx, 
      [&](asio::yield_context yield) {
        mti_670.set_poll_size(0x80);
        mti_670.connect(yield);
      }
  );
  ctx.run();
  ctx.restart();
}
