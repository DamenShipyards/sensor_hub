#define BOOST_TEST_MODULE fusion_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/processor.h" 
#include "../src/processors/fusion.h" 

#include <iostream>
#include <typeinfo>
#include <random>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;



BOOST_AUTO_TEST_CASE(six_deg_test, * ut::tolerance(0.00000001)) {
}

