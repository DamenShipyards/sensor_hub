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

  /* Muck around with index sequences ...
  print<1,2,3,4,5>();
  print(std::make_index_sequence<10>());
  */

  std::vector<std::string> qs;
  for (auto&& q: Quantity_iter()) {
    qs.push_back(get_quantity_name(q));
    ++count;
  }

  BOOST_TEST(count == 35);
  BOOST_TEST(qs[34] == "faz");
  constexpr const char* const qn = Quantity_name<Quantity::yr>::value;
  std::string s(qn);
  BOOST_TEST(s == "yr");

  Quantity q = Quantity::la;
  auto s1 = get_quantity_name<static_cast<Quantity>(1)>();
  auto s2 = get_quantity_name(q);
  BOOST_TEST(s1 == "la");
  BOOST_TEST(s2 == "la");
}

BOOST_AUTO_TEST_CASE(data_assignment_test) {
  Quantity_value qv = {Quantity::ut, 88.0};
  Data_value dv;
  Stamped_value sv = {100.0, 99.0};
  Stamped_quantity sq = {Quantity::lo, 50.0, 1.0};
  sv = sq;
  dv = sv;
  BOOST_TEST(sv.stamp == sq.stamp);
  BOOST_TEST(sv.value == sq.value);
  BOOST_TEST(dv.value == sv.value);
  dv = qv;
  BOOST_TEST(dv.value == qv.value);
}
