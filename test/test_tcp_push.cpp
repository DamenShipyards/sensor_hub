#define BOOST_TEST_MODULE fusion_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind/bind.hpp>

#include "../src/processor.h"
#include "../src/processors/tcp_push.h"

#include <iostream>
#include <typeinfo>
#include <random>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;

using namespace boost::placeholders;


BOOST_AUTO_TEST_CASE(construction_test, * ut::tolerance(0.00000001)) {
  auto tcp_pusher = Tcp_pusher();
}

