#define BOOST_TEST_MODULE parse_test
#include <boost/test/unit_test.hpp>

#include "../src/devices/xsens.h"
#include "../src/devices/xsens_impl.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <cstdint>
#include <algorithm>


namespace x3 = boost::spirit::x3;
using namespace std::string_literals;

using namespace xsens;

float to_float(const char* c) {
  char data[4];
  for (int i=3; i>=0; --i) {
    data[i] = *c++;
  }
  return *reinterpret_cast<float*>(&data);
};

BOOST_AUTO_TEST_CASE(xsens_parse_acceleration_test) {
  std::string data = "\x40\x20\x0c\xbd\x77\x48\x07\xbc\x0e\xdc\x7b\x41\x1c\xd1\x56"s;
  auto cur = data.begin();
  parser::Acceleration acceleration;
  BOOST_TEST(std::is_floating_point<parser::Acceleration::bytes_type>::value);
  auto result = x3::parse(cur, data.end(), parser::acceleration, acceleration);
  BOOST_TEST(result);
  BOOST_TEST(acceleration.data.size() == 3);
  BOOST_TEST(acceleration.data[0] == to_float(data.data() + 3));
  BOOST_TEST(acceleration.data[1] == to_float(data.data() + 7));
  BOOST_TEST(acceleration.data[2] == to_float(data.data() + 11));
}

BOOST_AUTO_TEST_CASE(xsens_parse_date_time_test) {
  std::string data = "\x10\x10\x0c\x14\x70\x3d\x20\x07\xe2\x09\x0a\x08\x39\x38\x37"s;
  auto cur = data.begin();
  parser::Date_time date_time;
  auto result = x3::parse(cur, data.end(), parser::date_time, date_time);
  BOOST_TEST(result);
  BOOST_TEST(date_time.hour == 8);
  BOOST_TEST(date_time.minute == 57);
  BOOST_TEST(date_time.get_values()[0].quantity == Quantity::ut);
  BOOST_TEST(date_time.get_values()[0].value == 1536569876.3429);
}

BOOST_AUTO_TEST_CASE(data_converter_test) {
  BOOST_TEST(parser::IdentityConverter<3>::factor(0) == 1.0);
  BOOST_TEST(parser::IdentityConverter<3>::factor(1) == -1.0);
  BOOST_TEST(parser::RadConverter<2>::factor(0) == M_PI / 180.0);
}

BOOST_AUTO_TEST_CASE(xsens_parse_bytes_test) {
  std::string data = "\x40\x20\x0c\xbd\x77\x48\x07\xbc\x0e\xdc\x7b\x41\x1c\xd1\x56\x01\x00\x02\x00\x00\x10\x10\x0c\x14\x70\x3d\x20\x07\xe2\x09\x0a\x08\x39\x38\x37"s;
  parser::Xsens_parser::Data_packets packets;
  auto cur = data.begin();
  bool result = x3::parse(cur, data.end(), parser::data_parser, packets);
  BOOST_TEST(result);
  BOOST_TEST(packets.size() == 3);
  parser::Xsens_parser::Data_visitor visitor;
  for (auto& packet: packets) {
    boost::apply_visitor(visitor, packet);
  }
  auto values = visitor.values;
  BOOST_TEST(values.size() == 4);
  BOOST_TEST(values[0].value == to_float(data.data() + 3));
  BOOST_TEST(values[0].quantity == Quantity::ax);
  BOOST_TEST(values[1].value == -to_float(data.data() + 7));
  BOOST_TEST(values[1].quantity == Quantity::ay);
  BOOST_TEST(values[2].value == -to_float(data.data() + 11));
  BOOST_TEST(values[3].value == 1536569876.3429);
}
