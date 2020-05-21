#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>

#include "../src/log.h" 


BOOST_AUTO_TEST_CASE(log_test) {
  log(level::debug, "Test Debug");
  log(level::info, "Test message");
  log(level::warning, "Test format % ?", 88);
  log(level::error, "Test % %", "ERROR", 88.88);
  log(level::fatal, "Test % %", "FATAL", 88.88);
  set_log_level("info");
  log(level::debug, "SHOULDN'T BE IN THERE");
  set_log_level(level::debug);
  log(level::debug, "Should be in there");
}

BOOST_AUTO_TEST_CASE(device_log_test) {
  init_device_log("test_id", "test_device");
  log("test_device", "Test device message");
  log(level::info, "Logged test device message");
}

