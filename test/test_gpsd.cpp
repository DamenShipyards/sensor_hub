#define BOOST_TEST_MODULE gpsd_test
#include "../src/devices/gpsd.h"

#include "test_common.h"

#include <memory>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

using namespace gpsd;

BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<Gpsd<Ctx> >();
}

