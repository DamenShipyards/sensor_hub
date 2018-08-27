#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>

#include "../src/quantities.h" 

#include <iostream>
#include <utility>
#include <type_traits>
#include <string>


BOOST_AUTO_TEST_CASE(iteration_test)
{
  int count = 0;
  int sum = 0;

  constexpr auto qend = static_cast<size_t>(Quantity::end);

  auto qs = std::make_index_sequence<qend>();

  for (auto&& q: Quantity_iter()) {
    ++count;
  }

  BOOST_TEST(count == 32);
  constexpr const char* const qn = Quantity_name<Quantity::yr>::value;
  std::string s(qn);
  BOOST_TEST(s == "yr");
}
