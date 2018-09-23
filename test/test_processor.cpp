#define BOOST_TEST_MODULE log_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/processor.h" 

#include <iostream>
#include <typeinfo>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;



BOOST_AUTO_TEST_CASE(statistics_test) {
  Statistics stats;
  Stamped_quantity value;
  value.quantity = Quantity::ax;
  int o = static_cast<int>(Quantity::ax);
  value.stamp = 2.0;
  value.value = 1.0;
  stats.insert_value(value);
  double mean = stats[2 * o];
  double stdd = stats[2 * o + 1];
  BOOST_TEST(mean == 1.0);
  value.stamp = 2.9;
  value.value = 1.2;
  stats.insert_value(value);
  mean = stats[2 * o];
  stdd = stats[2 * o + 1];
  BOOST_TEST(mean == 1.1);
  //BOOST_TEST(stdd == 0.14142135623730939);
  value.stamp = 3.05;
  value.value = 1.4;
  stats.insert_value(value);
  mean = stats[2 * o];
  stdd = stats[2 * o + 1];
  BOOST_TEST(fabs(mean - 1.3) < 1E-8);
  //BOOST_TEST(stdd == 0.14142135623730939);
}



BOOST_AUTO_TEST_CASE(horizontal_acceleration_peak_test) {
}

BOOST_AUTO_TEST_CASE(factory_test) {
  add_processor_factory("my_processor", std::move(std::make_unique<Processor_factory<Statistics> >()));
  Processor_ptr processor = create_processor("my_processor");
  processor->set_name("My Processor");
  BOOST_TEST(typeid(*processor).hash_code() == typeid(Statistics).hash_code());
  processor = nullptr;
  BOOST_TEST(processor.get() == nullptr);
}



BOOST_AUTO_TEST_CASE(json_test) {
  Processor processor;
  std::string json = processor.get_json();
  BOOST_TEST(json == "{}");
}
