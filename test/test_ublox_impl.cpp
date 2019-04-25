#define BOOST_TEST_MODULE ublox_parse_test
#include <boost/test/unit_test.hpp>

#include "../src/devices/ublox.h"
#include "../src/devices/ublox_impl.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <cstdint>
#include <algorithm>


namespace x3 = boost::spirit::x3;
using namespace std::string_literals;


auto to_float = [](char *c){
  char data[4];
  for (int i=3; i>=0; --i) {
    data[i] = *c++;
  }
  return *reinterpret_cast<float*>(&data);
};

BOOST_AUTO_TEST_CASE(ublox_parse_acceleration_test) {
  std::string data = "\x40\x20\x0c\xbd\x77\x48\x07\xbc\x0e\xdc\x7b\x41\x1c\xd1\x56"s;
  /*
  auto cur = data.begin();
  ubx_parser::Acceleration acceleration;
  BOOST_TEST(std::is_floating_point<parser::Acceleration::data_type>::value);
  auto result = x3::parse(cur, data.end(), ubx_parser::acceleration, acceleration);
  BOOST_TEST(result);
  BOOST_TEST(acceleration.data.size() == 3);
  BOOST_TEST(acceleration.data[0] == to_float(data.data() + 3));
  BOOST_TEST(acceleration.data[1] == to_float(data.data() + 7));
  BOOST_TEST(acceleration.data[2] == to_float(data.data() + 11));
  */
}

