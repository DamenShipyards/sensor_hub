#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>

#include "../src/quantities.h" 

#include <iostream>


BOOST_AUTO_TEST_CASE(iteration_test)
{
  int count = 0;

  for (const auto& q: Quantity_iter()) {
    ++count;
  }

  BOOST_TEST(count == 31);
}
