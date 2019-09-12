/**
 * This test requires a serial USB adapter connected at /dev/ttyUSB0
 * or other port indicated by $SERIAL/%SERIAL% environment variable
 * The adapter should have pins 2 and 3 shorted so it becomes a loopback
 * device.
 */
#define BOOST_TEST_MODULE serial_test
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <iostream>
#include <exception>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;
namespace asio = boost::asio;
namespace fs = boost::filesystem;
using pth = boost::filesystem::path;

tt::assertion_result serial_available(ut::test_unit_id) {
  const char* port_env = getenv("SERIAL");
  std::string port;
  if (port_env != nullptr) {
    port = port_env;
  }
  else {
    port = "/dev/ttyUSB0";
  }
  if (port == "") {
    port = "/dev/ttyUSB0";
  }
  bool serial_present = fs::exists(pth(port));
  return serial_present;
}

asio::io_context ctx;
asio::io_context::strand strnd(ctx);
asio::serial_port serial(ctx);
asio::streambuf response_buf;
std::string error_message;
std::string message = "Hello World!";
bool cancelled = false;
size_t bytes_written = 0;

void loopback(asio::yield_context yield) {
  asio::mutable_buffer rbuf = response_buf.prepare(message.size());
  asio::deadline_timer timeout(ctx);
  timeout.expires_from_now(boost::posix_time::seconds(2));
  timeout.async_wait([](const boost::system::error_code& error) {
        if (!error)
          serial.cancel();
      });
  asio::const_buffer buf = asio::buffer(message);
  asio::async_write(serial, buf, [] (
          const boost::system::error_code&,
          std::size_t bytes_transferred) {
        bytes_written = bytes_transferred;
      });
  try {
    size_t bytes_read = asio::async_read(serial, rbuf, yield);
    timeout.cancel();
    response_buf.commit(bytes_read);
  }
  catch(std::exception& e) {
    error_message = e.what(); 
    error_message +=  ". Timeout on port? Is it looped back? i.e. Are pins 2 and 3 shorted?";
    cancelled = true;
  }
  serial.close();
}

BOOST_AUTO_TEST_CASE(serial_loopback_test, 
    *ut::precondition(serial_available))
{
  serial.open("/dev/ttyUSB0");
  asio::spawn(strnd, loopback);
  ctx.run();
  BOOST_TEST(bytes_written == message.size());
  BOOST_TEST(!cancelled, error_message);
  BOOST_TEST(response_buf.size() == message.size());
  std::string response(asio::buffer_cast<const char*>(response_buf.data()), response_buf.size());
  BOOST_TEST(response == message);
}



