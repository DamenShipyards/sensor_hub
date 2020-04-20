#include <stdio.h>
#include <string.h>
#include <string>

#include <boost/test/unit_test.hpp>
#ifndef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#endif
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace tt = boost::test_tools;
namespace ut = boost::unit_test;
namespace asio = boost::asio;
namespace posix_time = boost::posix_time;

std::string usb_device;


// tt::assertion takes no parameter, so usb_device is passed as global
tt::assertion_result usb_dev_available()  {
  FILE *lsusb = popen("/usr/bin/lsusb", "r");
  if (lsusb == nullptr) {
    return false;
  }
  char buffer[1024];
  char* line = fgets(buffer, sizeof(buffer), lsusb);
  bool usb_present = false;
  while (line != nullptr) {
    usb_present = strstr(line, usb_device.c_str()) != nullptr;
    if (usb_present)
      break;
    line = fgets(buffer, sizeof(buffer), lsusb);
  }
  pclose(lsusb);

  return usb_present;
}



tt::assertion_result usb_available(ut::test_unit_id) {
  const char* device_str = getenv("USB");
  if (device_str != nullptr) {
    usb_device = device_str;
  }
  else {
    usb_device = "";
  }
  if (usb_device == "") {
    usb_device = "2639:0017";
  }

  return usb_dev_available();
}


tt::assertion_result xsens_g_710_available(ut::test_unit_id) {
  usb_device = "2639:0017";

  return usb_dev_available();
}

tt::assertion_result xsens_670_available(ut::test_unit_id) {
  usb_device = "2639:0300";

  return usb_dev_available();
}

tt::assertion_result ublox_available(ut::test_unit_id) {
  usb_device = "1546:01a8";

  return usb_dev_available();
}


struct Ctx {
  static asio::io_context& get_context() {
    static asio::io_context instance;
    return instance;
  }
  static void run(int seconds) {
    boost::asio::deadline_timer timer(get_context());
    timer.expires_from_now(boost::posix_time::seconds(seconds));
    timer.async_wait(boost::bind(&asio::io_context::stop, &get_context()));
    get_context().run();
  }
};
