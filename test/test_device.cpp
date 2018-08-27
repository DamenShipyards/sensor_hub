#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>

#include "../src/device.h" 


struct Port {
};

static bool destructor_called = false;

template<typename P>
struct SomeDevice: public Device {
  ~SomeDevice() override {
    destructor_called = true;
  }

  P port;
};

using MyDevice = SomeDevice<Port>;

BOOST_AUTO_TEST_CASE(container_test)
{
  Devices devices;
  devices.push_back(std::make_unique<MyDevice>());
  for (auto& device: devices) {
  }

  devices.pop_back();
  BOOST_TEST(destructor_called);
}
