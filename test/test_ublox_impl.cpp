#define BOOST_TEST_MODULE ublox_test
#include <boost/test/unit_test.hpp>

#include "../src/types.h"
#include "../src/devices/ublox.h"
#include "../src/devices/ublox_impl.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdint>
#include <algorithm>


using namespace ubx;


BOOST_AUTO_TEST_CASE(ublox_parse_acceleration_test) {
  std::string data = "\x40\x20\x0c\xbd\x77\x48\x07\xbc\x0e\xdc\x7b\x41\x1c\xd1\x56";
  /*
  auto cur = data.begin();
  ubx_parser::Acceleration acceleration;
  BOOST_TEST(std::is_floating_point<parser::Acceleration::bytes_type>::value);
  auto result = x3::parse(cur, data.end(), ubx_parser::acceleration, acceleration);
  BOOST_TEST(result);
  BOOST_TEST(acceleration.data.size() == 3);
  BOOST_TEST(acceleration.data[0] == to_float(data.data() + 3));
  BOOST_TEST(acceleration.data[1] == to_float(data.data() + 7));
  BOOST_TEST(acceleration.data[2] == to_float(data.data() + 11));
  */
}

cbytes_t empty_packet = { 0xB5, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
cbytes_t cfg_gnss_dummy = {0xb5, 0x62, 0x06, 0x3E, 0x00, 0x00, 0x00, 0x00};

BOOST_AUTO_TEST_CASE(ublox_data_packet_test) {
  auto packet = parser::Data_packet();
  BOOST_TEST(packet.get_data() == empty_packet);
  packet = parser::Data_packet(command::cls_cfg, command::cfg::gnss);
  BOOST_TEST(packet.get_data() == cfg_gnss_dummy);
}
