/**
 * Test of generic tools
 */
#define BOOST_TEST_MODULE tools_test
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <istream>
#include <exception>
#include <vector>

#include <stdio.h>
#include <string.h>

#include "../src/tools.h"

// Data types for data communicated with the sensor
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;

BOOST_AUTO_TEST_CASE(test_contains) {
  cdata_t c1{0x01, 0x02, 0x03, 0x04, 0x04};
  cdata_t c2{0x01, 0x02, 0x03};
  cdata_t c3{0x02, 0x03, 0x04};
  cdata_t c4{0x03, 0x04, 0x04};
  cdata_t c5{0x04};
  cdata_t c6{0x04, 0x04};
  cdata_t c7{0x04, 0x04, 0x04};
  cdata_t c8{0x04, 0x04, 0x01};
  cdata_t c9{};
  cdata_t c10{0x01};
  cdata_t c11{0x01, 0x03};
  BOOST_TEST(contains(c1, c1));
  BOOST_TEST(contains(c1, c2));
  BOOST_TEST(contains(c1, c3));
  BOOST_TEST(contains(c1, c4));
  BOOST_TEST(contains(c1, c5));
  BOOST_TEST(contains(c1, c6));
  BOOST_TEST(!contains(c1, c7));
  BOOST_TEST(!contains(c1, c8));
  BOOST_TEST(!contains(c2, c1));
  BOOST_TEST(!contains(c9, c10));
  BOOST_TEST(contains(c10, c9));
  BOOST_TEST(!contains(c1, c11));
  BOOST_TEST(contains_at(c1, c2) == 0);
  BOOST_TEST(contains_at(c1, c3) == 1);
  BOOST_TEST(contains_at(c1, c4) == 2);
  BOOST_TEST(contains_at(c1, c5) == 3);
  BOOST_TEST(contains_at(c1, c6) == 3);
  BOOST_TEST(contains_at(c1, c7) == -1);
}

