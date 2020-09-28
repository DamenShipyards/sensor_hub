#define BOOST_TEST_MODULE signalk_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind/bind.hpp>

#include "../src/processor.h" 
#include "../src/devices/dummy.h"
#include "../src/processors/signalk.h" 
#include "../src/processors/signalk_converter.h" 

#include <iostream>
#include <typeinfo>
#include <random>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;
using namespace boost::placeholders;


BOOST_AUTO_TEST_CASE(provider_test, * ut::tolerance(0.00000001)) {
  auto device = dummy::Dummy_gps<Ctx>();
  auto signalk = std::make_shared<SignalK_pusher<Ctx>>();
  device.add_processor(signalk);
  asio::spawn(Ctx::get_context(), boost::bind(&dummy::Dummy_gps<Ctx>::connect, &device, _1));
  Ctx::run(5);
}

BOOST_AUTO_TEST_CASE(json_test_time, * ut::tolerance(0.00000001)) {
    auto converter = new SignalK_converter();
    Stamped_quantity sq = {1.0, 1.0, Quantity::ut};
    BOOST_TEST(converter->get_delta(sq) == "{\"updates\":[{\"$source\":\"sensor_hub\",\"timestamp\":\"1970-01-01T00:00:01Z\",\"values\":[{\"path\":\"navigation.datetime\",\"value\":\"1970-01-01T00:00:01Z\"}]}]}");
}
BOOST_AUTO_TEST_CASE(json_test_double, * ut::tolerance(0.00000001)) {
    auto converter = new SignalK_converter();
    Stamped_quantity sq = {1.0, 1.0, Quantity::vog};
    BOOST_TEST(converter->get_delta(sq) == "{\"updates\":[{\"$source\":\"sensor_hub\",\"timestamp\":\"1970-01-01T00:00:01Z\",\"values\":[{\"path\":\"navigation.speedOverGround\",\"value\":1.0}]}]}");
}
BOOST_AUTO_TEST_CASE(json_test_pos, * ut::tolerance(0.00000001)) {
    auto converter = new SignalK_converter();
    Stamped_quantity sq_lo = {0.0, 1.0, Quantity::lo};
    Stamped_quantity sq_la = {0.0, 1.0, Quantity::la};
    converter->produces_delta(sq_lo);
    converter->produces_delta(sq_la);
    BOOST_TEST(converter->get_delta(sq_la) == "{\"updates\":[{\"$source\":\"sensor_hub\",\"timestamp\":\"1970-01-01T00:00:01Z\",\"values\":[{\"path\":\"navigation.position\",\"value\":{\"latitude\":0.0,\"longitude\":0.0}}]}]}");
}
BOOST_AUTO_TEST_CASE(json_test_pos_2, * ut::tolerance(0.00000001)) {
    auto converter = new SignalK_converter();
    Stamped_quantity sq_lo = {1.0, 1.0, Quantity::lo};
    Stamped_quantity sq_la = {-1.0, 1.0, Quantity::la};
    converter->produces_delta(sq_lo);
    converter->produces_delta(sq_la);
    BOOST_TEST(converter->get_delta(sq_la) == "{\"updates\":[{\"$source\":\"sensor_hub\",\"timestamp\":\"1970-01-01T00:00:01Z\",\"values\":[{\"path\":\"navigation.position\",\"value\":{\"latitude\":-57.29577951308232,\"longitude\":57.29577951308232}}]}]}");

BOOST_AUTO_TEST_CASE(construction_test, * ut::tolerance(0.00000001)) {
  auto signalk = SignalK_pusher();
}

BOOST_AUTO_TEST_CASE(produces_delta_test, * ut::tolerance(0.00000001)) {
    auto converter = new SignalK_converter();
    Stamped_quantity sq1 = {1.0, 1.0, Quantity::ut};
    Stamped_quantity sq2 = {1.0, 1.0, Quantity::lo};
    Stamped_quantity sq3 = {1.0, 1.0, Quantity::la};
    Stamped_quantity sq4 = {1.0, 2.0, Quantity::la};
    BOOST_TEST(converter->produces_delta(sq1) == true);
    BOOST_TEST(converter->produces_delta(sq2) == false);
    BOOST_TEST(converter->produces_delta(sq3) == true);
    BOOST_TEST(converter->produces_delta(sq4) == false);
}
