#define BOOST_TEST_MODULE signalk_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/processor.h" 
#include "../src/devices/dummy.h"
#include "../src/processors/signalk.h" 

#include <iostream>
#include <typeinfo>
#include <random>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;


BOOST_AUTO_TEST_CASE(provider_test, * ut::tolerance(0.00000001)) {
  auto device = dummy::Dummy_gps<Ctx>();
  auto signalk = std::make_shared<SignalK>();
  device.add_processor(signalk);
  asio::spawn(Ctx::get_context(), boost::bind(&dummy::Dummy_gps<Ctx>::connect, &device, _1));
  Ctx::run(5);
}

