#define BOOST_TEST_MODULE spirit_test
#include <boost/test/unit_test.hpp>

#include "../src/types.h" 
#include "../src/spirit_x3.h" 

#include <vector>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <cstdint>
#include <algorithm>

namespace x3 = boost::spirit::x3;
using namespace std::string_literals;
using x3::byte_;
using x3::little_word;

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

  data = "\x08\x00\xff\xfa"s;
  cur = data.begin();
  auto result3 = x3::parse(cur, data.end(), p2);
  BOOST_TEST(!result3);
}

struct Unknown {
  Unknown(): _id(0), len(0) {}
  Unknown(uint8_t id): _id(id), len(0) {}
  uint8_t _id;
  int len;
  virtual const int get_val() const {
    return 0;
  }
};

template <uint8_t DataId>
struct Id_data_packet: public Unknown {
  Id_data_packet(): Unknown(DataId) {}
  uint8_t len;
  uint8_t val1;
  const int get_val() const override {
    return static_cast<int>(val1);
  }

};

template <uint8_t DataId>
struct Id_data_packet2: public Id_data_packet<DataId> {
  uint16_t val2;
  const int get_val() const override {
    return static_cast<int>(val2);
  }
};

using id_pack_1 = Id_data_packet2<1>;
using id_pack_2 = Id_data_packet<2>;
using id_pack_3 = Id_data_packet<4>;


x3::rule<class uknown, Unknown> const unknown = "unknown";
x3::rule<class id_data_packet1, id_pack_1> const id_data_packet1 = "id_data_packet1";
x3::rule<class id_data_packet2, id_pack_2> const id_data_packet2 = "id_data_packet2";
x3::rule<class id_data_packet3, id_pack_3> const id_data_packet3 = "id_data_packet3";

auto set_id = [](auto& ctx) { x3::_val(ctx)._id = x3::_attr(ctx); };
auto set_len = [](auto& ctx) { x3::_val(ctx).len = x3::_attr(ctx); };
auto more = [](auto& ctx) { x3::_pass(ctx) = x3::_val(ctx).len-- > 0; };
auto done = [](auto& ctx) { x3::_pass(ctx) = x3::_val(ctx).len < 0; };
auto unknown_def = x3::byte_[set_id] >> x3::byte_[set_len] >> *(x3::eps[more] >> x3::byte_) >> x3::eps[done];
//auto data_packet_def = x3::byte_ >> x3::byte_ >> x3::byte_ >> x3::byte_;
auto id_data_packet1_def = x3::byte_(1) >> x3::byte_ >> x3::byte_ >> x3::big_word;
auto id_data_packet2_def = x3::byte_(2) >> x3::byte_ >> x3::byte_;
auto id_data_packet3_def = x3::byte_(4) >> x3::byte_ >> x3::byte_;


BOOST_SPIRIT_DEFINE(
    unknown,
    id_data_packet1,
    id_data_packet2,
    id_data_packet3
)


BOOST_FUSION_ADAPT_STRUCT(id_pack_1, len, val1, val2)
BOOST_FUSION_ADAPT_STRUCT(id_pack_2, len, val1)
BOOST_FUSION_ADAPT_STRUCT(id_pack_3, len, val1)

struct Visitor {
  std::vector<int> values;
  void operator() (const Unknown& u) {
    values.push_back(u.get_val());
  }
};

BOOST_AUTO_TEST_CASE(spirit_def_test) {
  std::string data = "\x01\x03\x08\x00\x01\x03\x02\x05\x06\x02\x01\x03\x04\x01\x04"s;
  std::vector<boost::variant<id_pack_1, id_pack_2, id_pack_3, Unknown> > packets;
  auto cur = data.begin();
  auto data_parser = *(id_data_packet1 | id_data_packet2 | id_data_packet3 | unknown);
  x3::parse(cur, data.end(), data_parser, packets);
  BOOST_TEST(packets.size() == 4);
  Visitor v;
  for (auto const&p: packets) {
    boost::apply_visitor(v, p);
  }
  BOOST_TEST(v.values.size() == 4);
  BOOST_TEST(v.values[0] == 1);
  BOOST_TEST(v.values[1] == 0);
  BOOST_TEST(v.values[2] == 3);
  BOOST_TEST(v.values[3] == 4);
}


struct Parent {
  uint16_t m;
  struct Child {
    uint8_t m1;
    uint8_t m2;
  };
  std::vector<Child> children;
};

x3::rule<struct chld, Parent::Child> const chld = "child";
auto chld_def = byte_ >> byte_;
BOOST_SPIRIT_DEFINE(chld)

x3::rule<struct prnt, Parent> const prnt = "parent";
auto prnt_def = byte_(0xFF) >> little_word >> *chld_def;
BOOST_SPIRIT_DEFINE(prnt)

BOOST_FUSION_ADAPT_STRUCT(Parent::Child, m1, m2)
BOOST_FUSION_ADAPT_STRUCT(Parent, m, children)


BOOST_AUTO_TEST_CASE(parse_nested_test) {
  Parent p;
  cbytes_t data = { 0x01, 0xFF, 0x88, 0x08, 0x01, 0x03, 0x02, 0x04 };
  auto cur = data.begin();
  bool skip_is_successful = x3::parse(cur, data.end(), byte_ - byte_(0xFF));
  BOOST_TEST(skip_is_successful);
  if (!skip_is_successful)
    return;
  bool parse_is_successful = x3::parse(cur, data.end(), prnt_def, p);
  BOOST_TEST(parse_is_successful);
  if (!parse_is_successful)
    return;
  BOOST_TEST(p.m == 0x888);
  BOOST_TEST(p.children.size() == 2);
  BOOST_TEST(p.children[0].m1 == 1);
  BOOST_TEST(p.children[0].m2 == 3);
  BOOST_TEST(p.children[1].m1 == 2);
  BOOST_TEST(p.children[1].m2 == 4);
}

