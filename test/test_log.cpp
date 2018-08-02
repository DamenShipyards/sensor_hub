#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>

#include "../src/log.h" 


BOOST_AUTO_TEST_CASE(log_test)
{
  log(level::debug, "Test Debug");
  log(level::info, "Test message");
  log(level::warning, "Test format % ?", 88);
  log(level::error, "Test % %", "ERROR", 88.88);
}
