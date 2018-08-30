#define BOOST_TEST_MODULE quantity_test
#include <boost/test/unit_test.hpp>

#include "../src/quantities.h" 

#include <iostream>
#include <utility>
#include <type_traits>
#include <string>

template <typename T>
void print_impl() {
}

template <typename T, size_t value, size_t... values>
void print_impl() {
  std::cout << value << std::endl;
  print_impl<T, values...>();
}

template <size_t... values>
void print() {
  print_impl<int, values...>();
}

template <size_t... values>
void print(const std::index_sequence<values...>) {
  (std::cout << (values == 0 ? "" : ",") << values, ...);
}


BOOST_AUTO_TEST_CASE(iteration_test)
{
  int count = 0;

  print<1,2,3,4,5>();
  print(std::make_index_sequence<10>());

  for (auto&& q: Quantity_iter()) {
    std::cout << get_quantity_name(q) << std::endl;
    ++count;
  }

  BOOST_TEST(count == 32);
  constexpr const char* const qn = Quantity_name<Quantity::yr>::value;
  std::string s(qn);
  BOOST_TEST(s == "yr");

  Quantity q = Quantity::la;
  auto s1 = get_quantity_name<static_cast<Quantity>(1)>();
  auto s2 = get_quantity_name(q);
  BOOST_TEST(s1 == "la");
  BOOST_TEST(s2 == "la");
}
