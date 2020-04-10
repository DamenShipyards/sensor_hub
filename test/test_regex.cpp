#define BOOST_TEST_MODULE regex_test
#include "../src/types.h" 
#include "../src/devices/regex.h" 
#include "../src/log.h" 
#include "../src/socket.h" 

#include "test_common.h"

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>


using namespace regex;
namespace fs = boost::filesystem;
namespace prtr = boost::property_tree;

static prtr::ptree opts;

BOOST_AUTO_TEST_CASE(construction_test) {
  Device_ptr dev = std::make_unique<Regex_device<Socket, Ctx> >();
}

