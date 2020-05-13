#define BOOST_TEST_MODULE log_test
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/processor.h" 
#include "../src/processors/statistics.h" 
#include "../src/processors/acceleration_history.h" 
#include "../src/socket.h"

#include <iostream>
#include <typeinfo>
#include <random>

#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "test_common.h"

namespace posix_time = boost::posix_time;


BOOST_AUTO_TEST_CASE(statistics_test, * ut::tolerance(0.00000001)) {
  Statistics stats;
  Stamped_quantity value;
  value.quantity = Quantity::ax;
  int o = static_cast<int>(Quantity::ax);
  //double t = stats[Statistic::size() * o + Statistic::f_time];
  double n = stats[Statistic::size() * o  + Statistic::f_n];
  double mean = stats[Statistic::size() * o + Statistic::f_mean];
  double stdd = stats[Statistic::size() * o + Statistic::f_stddev];
  BOOST_TEST(n == 0);
  BOOST_TEST(mean == 0);
  BOOST_TEST(stdd == 0);
  for (int i = 0; i < 500000; ++i) {
    value.stamp = i + 0.0;
    value.value = 0.9;
    stats.insert_value(value);
    value.stamp = i + 0.25;
    value.value = 1.1;
    stats.insert_value(value);
    value.stamp = i + 0.5;
    value.value = 1.3;
    stats.insert_value(value);
    value.stamp = i + 0.75;
    value.value = 1.1;
    stats.insert_value(value);
  }
  n = stats[Statistic::size() * o  + Statistic::f_n];
  mean = stats[Statistic::size() * o + Statistic::f_mean];
  stdd = stats[Statistic::size() * o + Statistic::f_stddev];
  BOOST_TEST(n == 5);
  BOOST_TEST(mean == 1.1);
  BOOST_TEST(stdd == 0.1);

  value.quantity = Quantity::hdg;
  o = static_cast<int>(Quantity::hdg);
  for (int i = 0; i < 100000; ++i) {
    value.stamp = i + 0.0;
    value.value = 2 * M_PI - 0.25;
    stats.insert_value(value);
    value.stamp = i + 0.25;
    value.value = 2 * M_PI - 0.05;
    stats.insert_value(value);
    value.stamp = i + 0.5;
    value.value = 0.15;
    stats.insert_value(value);
    value.stamp = i + 0.75;
    value.value = 2 * M_PI - 0.05;
    stats.insert_value(value);
  }
  n = stats[Statistic::size() * o  + Statistic::f_n];
  mean = stats[Statistic::size() * o + Statistic::f_mean];
  stdd = stats[Statistic::size() * o + Statistic::f_stddev];
  BOOST_TEST(n == 5);
  BOOST_TEST(mean == (2 * M_PI - 0.05));
  BOOST_TEST(stdd ==  0.1);
}

BOOST_AUTO_TEST_CASE(port_processor_test, * ut::tolerance(0.00000001)) {
  auto processor = Port_processor<Socket, Ctx>();
  // TODO: do something interesting with the socket here maybe
}

BOOST_AUTO_TEST_CASE(random_statistics_test, * ut::tolerance(0.00000001)) {
  // Setup mersenne twister pseudo random number generator
  std::uniform_real_distribution<double> distribution(-0.3, 0.5);
  std::mt19937 engine;
  auto generator = std::bind(distribution, engine);

  Stamped_quantity value;
  Statistics stats;
  value.quantity = Quantity::ax;

  for (int k = 0; k < 10; ++k) {
    std::vector<double> nums;
    for (int i = 0; i < 11; ++i) {
      value.stamp += 0.099;
      value.value = generator();
      nums.push_back(value.value);
      stats.insert_value(value);
    }

    int o = static_cast<int>(Quantity::ax);
    double n = stats[Statistic::size() * o  + Statistic::f_n];
    double mean = stats[Statistic::size() * o + Statistic::f_mean];
    double stdd = stats[Statistic::size() * o + Statistic::f_stddev];

    // Compute expected values
    double sum = 0;
    for (int i = 1; i < 11; ++i) {
      sum += 0.5 * (nums[i] + nums[i - 1]);
    }
    double expected_mean = sum / 10;
    sum = 0;
    for (int i = 1; i < 11; ++i) {
      sum += sqr(0.5 * (nums[i] + nums[i - 1]) - expected_mean);
    }
    double expected_stdd = sqrt(sum / 10);

    BOOST_TEST(n == 11);
    BOOST_TEST(mean == expected_mean);
    BOOST_TEST(stdd == expected_stdd);
  }
}

BOOST_AUTO_TEST_CASE(horizontal_acceleration_peak_test, * ut::tolerance(0.00000001)) {
  Acceleration_history history;

  history.set_name("Test-Acceleration-History");
  Stamped_quantity value;
  std::cout << value << std::endl;
  for (int i = 0; i < 400; ++i) {
    value.quantity = Quantity::fax;
    value.stamp += 0.01 * M_PI;
    value.value = sqrt(2) * sin(value.stamp);
    history.insert_value(value);
    value.quantity = Quantity::fay;
    history.insert_value(value);
  }
  BOOST_TEST(history.size() == 20);
  BOOST_TEST(history[0] == 9.958848711879645);
  BOOST_TEST(history[1] == 2.0734511513692639);
  BOOST_TEST(history[2] == 2.0);
  BOOST_TEST(history[3] == 1.6603646188180439);
  BOOST_TEST(history[6] == 2.0734511513692639);
  BOOST_TEST(history[7] == 2.0);
  BOOST_TEST(history[8] == 1.6603646188180439);


  history.set_params("direction=1");

  for (int i = 0; i < 400; ++i) {
    value.quantity = Quantity::fax;
    value.stamp += 0.01 * M_PI;
    value.value = 2 * sin(value.stamp);
    history.insert_value(value);
    value.quantity = Quantity::fay;
    history.insert_value(value);
  }
  BOOST_TEST(history.size() == 40);
  BOOST_TEST(history[0] == (4 * M_PI + 9.958848711879645));
  BOOST_TEST(history[1] == 2.0734511513692639);
  BOOST_TEST(history[2] == -2.0);
  BOOST_TEST(history[3] == -1.6603646188180439);
  BOOST_TEST(history[4] == 1.6867085636136444);
  BOOST_TEST(history[6] == 2.0734511513692639);
  BOOST_TEST(history[7] == 2.0);
  BOOST_TEST(history[8] == 1.6603646188180439);
  BOOST_TEST(history[9] == 1.6867085636136444);
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
