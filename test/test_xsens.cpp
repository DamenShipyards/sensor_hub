#define BOOST_TEST_MODULE xsens_test
#include <boost/test/unit_test.hpp>

#include "../src/xsens.h" 
#include "../src/usb.h" 

#include <memory>

BOOST_AUTO_TEST_CASE(construction_test)
{
  Device_ptr dev = std::make_unique<Xsens_MTi_G_710<Usb> >();
}
