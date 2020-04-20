#define BOOST_TEST_MODULE regex_test
#include "../src/types.h"
#include "../src/devices/regex.h"
#include "../src/log.h"
#include "../src/socket.h"
#include "../src/quantities.h"

#include "test_common.h"

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


using namespace regex;
namespace fs = boost::filesystem;
namespace prtr = boost::property_tree;

static prtr::ptree opts;

decltype(auto) create() {
  return std::make_unique<Regex_device<Socket, Ctx> >();
}


template<typename DEV>
void set_options(DEV& dev) {
  prtr::ptree options;
  options.put("ax", "^(.*)$\" }");
  dev->set_options(options);
}


BOOST_AUTO_TEST_CASE(construction_test) {
  create();
}


BOOST_AUTO_TEST_CASE(options_test) {
  auto dev =  create();
  set_options(dev);
}


BOOST_AUTO_TEST_CASE(parse_test) {
  auto dev =create();
  set_options(dev);
  dev->get_value(Quantity::ax);
}
