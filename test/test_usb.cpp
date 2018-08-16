#define BOOST_TEST_MODULE usb_test
#include <boost/test/unit_test.hpp>

#include "../src/usb.h" 


BOOST_AUTO_TEST_CASE(usb_connection_test)
{
  Usb usb;
  BOOST_TEST(usb.open_device(0x2639, 0x0017) == true);
}
