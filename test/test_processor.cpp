#define BOOST_TEST_MODULE log_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/processor.h" 

#include <iostream>
#include <typeinfo>

#include <sys/stat.h>
#include <fcntl.h>

#include "test_common.h"

namespace posix_time = boost::posix_time;



BOOST_AUTO_TEST_CASE(statistics_test) {
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
