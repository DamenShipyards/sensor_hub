#define BOOST_TEST_MODULE ublox_test
#include "../src/devices/ublox.h"
#include "../src/devices/ublox_impl.h"
#include "../src/serial.h"

#include "test_common.h"

#include <memory>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

using namespace ubx;
namespace fs = boost::filesystem;

const std::string path_0 = "/dev/sensor_hub/ublox_neo_m8u-ttyACM0";
const std::string path_1 = "/dev/sensor_hub/ublox_neo_m8u-ttyACM1";
const std::string baud_string = ":921600";

BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<NEO_M8U<Serial, Ctx> >();
}

BOOST_AUTO_TEST_CASE(connection_test, *ut::precondition(ublox_available)) {
  auto argc = ut::framework::master_test_suite().argc;
  auto** argv = ut::framework::master_test_suite().argv;
  int running_time = 2000;
  if (argc > 1) {
    running_time = atoi(argv[1]);
  }
  std::cout << "Arguments: " << argc << ", Running time: " << running_time << std::endl;
  asio::io_context& ctx = Ctx::get_context();
  NEO_M8U<Serial, Ctx> ublox;
  asio::deadline_timer tmr(ctx, posix_time::milliseconds(running_time));
  tmr.async_wait(
      [&ctx](boost::system::error_code ec) {
        ctx.stop();
      }
  );

  ublox.set_name("ublox-test");
  if (fs::exists(path_0)) {
    ublox.set_connection_string(path_0 + baud_string);
  }
  else if (fs::exists(path_1)) {
    ublox.set_connection_string(path_1 + baud_string);
  }
  else {
    BOOST_TEST(false, "No path to serial device could be found");
  }

  ublox.enable_logging(true);
  BOOST_TEST(!ublox.is_connected());
  asio::spawn(ctx,
      [&](asio::yield_context yield) {
        ublox.connect(yield);
      }
  );
  ctx.run();
  BOOST_TEST(ublox.is_connected());
  ublox.disconnect();
  BOOST_TEST(!ublox.is_connected());
}
