/**
 * This test requires a USB device that should be indicated with
 * $USB/%USB% environment variable like ``export USB=2639:0017`` or
 * have an Xsens mru connected with product id 0017
 * The adapter should have pins 2 and 3 shorted so it becomes a loopback
 * device.
 */
#define BOOST_TEST_MODULE usb_test
#include <boost/test/unit_test.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

#include "../src/usb.h" 

std::string usb_device;

tt::assertion_result usb_available(ut::test_unit_id test_id) {
  std::cout << "Checking USB available" << std::endl;
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

  FILE *lsusb = popen("/usr/bin/lsusb", "r");
  if (lsusb == nullptr) {
    std::cout << "Failed to popen lsusb" << std::endl;
    return false;
  }
  char buffer[1024];
  char *line = fgets(buffer, sizeof(buffer), lsusb);
  bool usb_present = false;
  while (line != nullptr) {
    usb_present = strstr(line, usb_device.c_str()) != nullptr;
    if (usb_present)
      break;
    char *line = fgets(buffer, sizeof(buffer), lsusb);
  }
  pclose(lsusb);

  return usb_present;
}

BOOST_AUTO_TEST_CASE(usb_connection_test, *ut::precondition(usb_available))
{
  Usb usb;
  BOOST_TEST(usb.open_device(usb_device) == true);
}
