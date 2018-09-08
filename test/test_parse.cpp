#define BOOST_TEST_MODULE parse_test
#include <boost/test/unit_test.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary/binary.hpp>
#include <boost/spirit/home/x3/auxiliary/eps.hpp>

#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <cstdint>
#include <algorithm>

namespace x3 = boost::spirit::x3;
using namespace std::string_literals;

BOOST_AUTO_TEST_CASE(spirit_basic_test) {

  std::string data = "255";
  int value = 0;
  auto cur = data.begin();
  auto result1 = x3::parse(cur, data.end(), x3::int_, value);
  BOOST_TEST(result1);
  BOOST_TEST(value == 255);


  data = "\x00\x00\x01\x00"s;
  BOOST_TEST(data.size() == 4);
  std::vector<uint8_t> vdata = {0, 0, 1, 0};

  value = 0;
  auto vcur = vdata.begin();
  auto result2 = x3::parse(vcur, vdata.end(), x3::big_dword, value);
  BOOST_TEST((vcur == vdata.end()));
  BOOST_TEST(result2);
  BOOST_TEST(value == 256);


  value = 0;
  cur = data.begin();
  auto result3 = x3::parse(cur, data.end(), x3::big_dword, value);
  BOOST_TEST((cur == data.end()));
  BOOST_TEST(result3);
  BOOST_TEST(value == 256);

  BOOST_TEST((cur == data.end()));
}

BOOST_AUTO_TEST_CASE(spirit_compound_test) {
  // Match a preamble
  std::string data = "\x08\x00\xff\xfa\x36\x10"s;
  uint8_t value;
  auto preamble = x3::lit("\xff\xfa");
  std::vector<uint8_t> rubbish;
  std::vector<uint8_t> expected_rubbish = {0x08, 0x00};
  auto getr = [&](auto& ctx) { rubbish = x3::_attr(ctx); };
  auto getv = [&](auto& ctx) { value = x3::_attr(ctx); };
  auto junk = *(x3::byte_ - preamble);
  auto p = junk[getr] >> preamble >> x3::byte_[getv];
  auto cur = data.begin();
  auto result = x3::parse(cur, data.end(), p);
  BOOST_TEST(result);
  BOOST_TEST(rubbish.size() == 2);
  BOOST_TEST(rubbish == expected_rubbish);
  BOOST_TEST(value == 0x36);

  data = "\x08\x00\xff\xfa\x36\x03\x01\x02\x03\x04\x05"s;
  cur = data.begin();
  uint8_t mid;
  int len;
  uint8_t checksum;
  std::vector<uint8_t> content;
  std::vector<uint8_t> expected_content = {0x01, 0x02, 0x03};
  auto getmid =  [&](auto& ctx) { mid = x3::_attr(ctx); };
  auto getlen =  [&](auto& ctx) { len = x3::_attr(ctx); };
  auto getcont =  [&](auto& ctx) { content = x3::_attr(ctx); };
  auto getc = [&](auto& ctx) { checksum = x3::_attr(ctx); };
  auto have_data = [&len](auto& ctx) { _pass(ctx) = len-- > 0; };
  auto have_all = [&len](auto& ctx) { _pass(ctx) = len < 0; };
  auto cont_parser = *(x3::eps[have_data] >> x3::byte_) >> x3::eps[have_all];
  auto p2 = junk >> preamble >> x3::byte_[getmid] >> x3::byte_[getlen] >> cont_parser[getcont] >> x3::byte_[getc];
  auto result2 = x3::parse(cur, data.end(), p2);
  BOOST_TEST(result2);
  BOOST_TEST(content == expected_content);
  BOOST_TEST(checksum == 4);
}
