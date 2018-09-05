/**
 * This test requires a USB device that should be indicated with
 * $USB/%USB% environment variable like ``export USB=2639:0017`` or
 * have an Xsens mru connected with product id 0017
 * The adapter should have pins 2 and 3 shorted so it becomes a loopback
 * device.
 */
#define BOOST_TEST_MODULE usb_test
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <exception>
#include <vector>


#include "../src/usb.h"
#include "../src/tools.h"

#include "test_common.h"

// Data types for data communicated with the sensor
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;


namespace asio = boost::asio;


static bool read_returned = false;
static size_t bytes_handled = 0;
static size_t bytes_read = 0;
static boost::asio::io_context io_context;

void handle_read(boost::system::error_code ec, size_t bytes_read) {
  bytes_handled = bytes_read;
}

void read(Usb& usb, boost::asio::yield_context yield) {
  asio::streambuf b;
  bytes_read = asio::async_read(usb, b.prepare(128), yield);
  read_returned = true;
}

BOOST_AUTO_TEST_CASE(usb_read_test, *ut::precondition(usb_available)) {
  asio::streambuf b;
  Usb usb(io_context);
  read_returned = false;
  bytes_read = 0;
  if (usb.open(usb_device)) {
    try {
      // One read with callback
      asio::async_read(usb, b.prepare(128), handle_read);
      // ... and one read with coroutine
      asio::spawn(io_context, boost::bind(read, boost::ref(usb), _1));
      io_context.run();
    }
    catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << std::endl;
      std::cout.flush();
    }
    BOOST_TEST(bytes_handled == 128U);
    BOOST_TEST(read_returned == true);
    BOOST_TEST(bytes_read == 128U);
    usb.close();
  }
  else {
    BOOST_TEST(false, "Failed to open device");
  }
}

static data_t response1;
static data_t response2;
static int i = 20;
// Command bytes
cbyte_t packet_start = 0xFA;
cbyte_t sys_command = 0xFF;
cbyte_t conf_command = 0x01;

// Command strings
cdata_t goto_config = { packet_start, sys_command, 0x30, 0x00, 0xD1 }; // switch to config mode
cdata_t goto_measurement = { packet_start, sys_command, 0x10, 0x00, 0xF1 }; // switch to measurement mode

// Response header strings
cdata_t config_ok = { packet_start, sys_command, 0x31, 0x00, 0xD0};
cdata_t measurement_ok = { packet_start, sys_command, 0x11, 0x00, 0xF0};

void xsens(Usb& usb, boost::asio::yield_context yield) {
  asio::streambuf read_buf;

  boost::asio::async_write(usb, asio::buffer(goto_config), yield);
  i = 20;
  int j;
  do {
    size_t bytes_read1 = usb.async_read_some(read_buf.prepare(256), yield);
    read_buf.commit(bytes_read1);
    response1 = data_t(asio::buffers_begin(read_buf.data()), asio::buffers_begin(read_buf.data()) + bytes_read1);
    read_buf.consume(bytes_read1);
    j = contains_at(response1, config_ok);
  } while (--i > 0 && j < 0);

  boost::asio::async_write(usb, asio::buffer(goto_measurement), yield);
  size_t bytes_read2 = boost::asio::async_read(usb, read_buf.prepare(config_ok.size()), yield);
  read_buf.commit(bytes_read2);
  response2 = data_t(asio::buffers_begin(read_buf.data()), asio::buffers_begin(read_buf.data()) + bytes_read2);
  read_buf.consume(bytes_read2);

  read_returned = true;
}

BOOST_AUTO_TEST_CASE(usb_xsens_test, *ut::precondition(usb_available)) {
  Usb usb(io_context);
  read_returned = false;
  if (usb.open(usb_device)) {
    try {
      io_context.restart();
      boost::asio::spawn(io_context, boost::bind(xsens, boost::ref(usb), _1));
      io_context.run();
    }
    catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << std::endl;
      std::cout.flush();
    }
    BOOST_TEST(read_returned == true);
    BOOST_TEST(response2 == measurement_ok);
    BOOST_TEST(i > 0);
    usb.close();
  }
  else {
    BOOST_TEST(false, "Failed to open device");
  }
}
