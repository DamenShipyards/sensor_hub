#define BOOST_TEST_MODULE datetime_test
#include <boost/test/unit_test.hpp>

#include "../src/datetime.h" 

#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

BOOST_AUTO_TEST_CASE(datetime_test)
{
  std::cout << "BOOST SYSTEM: " << chrono::system_clock::now().time_since_epoch().count() << '\n';
#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
  std::cout << "BOOST STEADY: " << chrono::steady_clock::now().time_since_epoch().count() << '\n';
#endif
  std::cout << "BOOST HIGHRE: " << chrono::high_resolution_clock::now().time_since_epoch().count() << '\n';

  std::cout << "  STD SYSTEM: " << std::chrono::system_clock::now().time_since_epoch().count() << '\n';
  std::cout << "  STD STEADY: " << std::chrono::steady_clock::now().time_since_epoch().count() << '\n';
  std::cout << "  STD HIGHRE: " << std::chrono::high_resolution_clock::now().time_since_epoch().count() << '\n';

  boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
  std::cout << "BOOST  PTIME: " << (boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time() - epoch).total_nanoseconds() << '\n';
  std::cout << "BOOST  PTIME: " << (boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time() - epoch).ticks() << '\n';
  std::cout << std::endl;

  auto bs1 = chrono::system_clock::now().time_since_epoch().count();
  auto bs2 = chrono::system_clock::now().time_since_epoch().count();
  auto bs3 = chrono::system_clock::now().time_since_epoch().count();
  auto ss1 = std::chrono::system_clock::now().time_since_epoch().count();
  auto ss2 = std::chrono::system_clock::now().time_since_epoch().count();
  auto ss3 = std::chrono::system_clock::now().time_since_epoch().count();
  auto ps1 = boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time();
  auto ps2 = boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time();
  auto ps3 = boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time();
  std::cout << "BOOST SYSTEM: " << bs1 << '\n';
  std::cout << "BOOST SYSTEM: " << bs2 << '\n';
  std::cout << "BOOST SYSTEM: " << bs3 << '\n';
  std::cout << "  STD SYSTEM: " << ss1 << '\n';
  std::cout << "  STD SYSTEM: " << ss2 << '\n';
  std::cout << "  STD SYSTEM: " << ss3 << '\n';
  std::cout << "BOOST  PTIME: " << (ps1 - epoch).total_nanoseconds() << '\n';
  std::cout << "BOOST  PTIME: " << (ps2 - epoch).total_nanoseconds() << '\n';
  std::cout << "BOOST  PTIME: " << (ps3 - epoch).total_nanoseconds() << '\n';
}

// Custom clock that runs 10 seconds ahead
double get_test_time() {
  auto result = std::chrono::system_clock::now();
  result += std::chrono::seconds(10);
  auto musecs = std::chrono::duration_cast<std::chrono::microseconds>(result.time_since_epoch());
  return static_cast<double>(musecs.count()) * 1E-6;
}

BOOST_AUTO_TEST_CASE(datetime_adjust)
{
  std::cout << std::setprecision(15);
  // Starting out with 10 second difference
  std::cout << get_time() << std::endl;
  std::cout << get_test_time() << std::endl;
  // Adjust once
  adjust_clock(get_test_time());
  double diff = get_test_time() - get_time();
  BOOST_TEST(diff > 9.0);
  BOOST_TEST(diff < 10.0);
  // ... adjust a 100 times ...
  for (int i = 0; i < 100; ++i) {
    adjust_clock(get_test_time());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  diff = get_test_time() - get_time();
  // ... and clock diff should be less than one second
  BOOST_TEST(diff > 0.0); 
  BOOST_TEST(diff < 1.0);
  std::cout << std::setprecision(15);
  std::cout << get_time() << std::endl;
  std::cout << get_test_time() << std::endl;
}
