#define BOOST_TEST_MODULE log_test
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "../src/device.h" 
#include "../src/usb.h"
#include "../src/xsens.h"

#include <iostream>
#include <typeinfo>

namespace posix_time = boost::posix_time;


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


BOOST_AUTO_TEST_CASE(container_test) {
  Devices devices;
  devices.emplace_back(std::make_unique<MyDevice>());
  for (auto& device: devices) {
    BOOST_TEST(device->get_id() == "id_0");
  }

  devices.pop_back();
  BOOST_TEST(destructor_called);
}

struct IdNameDevice: public MyDevice {
  IdNameDevice(): MyDevice() {
    set_id("test_id");
    set_name("test_name");
  }
};


BOOST_AUTO_TEST_CASE(id_name_test) {
  MyDevice dev1;
  MyDevice dev2;

  BOOST_TEST(dev1.get_id() == "id_1");
  BOOST_TEST(dev1.get_name() == "device_1");
  BOOST_TEST(dev2.get_id() == "id_2");
  BOOST_TEST(dev2.get_name() == "device_2");

  IdNameDevice dev3;
  BOOST_TEST(dev3.get_id() == "test_id");
  BOOST_TEST(dev3.get_name() == "test_name");
}

BOOST_AUTO_TEST_CASE(factory_test) {
  Device_ptr dev = create_device("xsens_mti_g_710_usb");
  dev->set_name("My XSens Sensor");
  BOOST_TEST(typeid(*dev).hash_code() == typeid(Xsens_MTi_G_710<Usb>).hash_code());
  dev = nullptr;
  BOOST_TEST(dev.get() == nullptr);
}


struct Ctx {
  static asio::io_context& get_context() {
    static asio::io_context instance;
    return instance;
  }
};

struct Conn_device: public Port_device<asio::serial_port, Ctx> {
  bool connected = false;
  void initialize(asio::yield_context yield) override {
    set_id("test_connection_device_id");
    asio::deadline_timer tmr(Ctx::get_context(), posix_time::seconds(1));
    tmr.async_wait(yield);
    connected = true;
  }
};

BOOST_AUTO_TEST_CASE(connection_test) {
  asio::io_context& ctx = Ctx::get_context();
  Conn_device dev;
  dev.set_name("Test Connection Device");
  dev.set_connection_string("/dev/ttyS0");
  asio::spawn(ctx, boost::bind(&Conn_device::connect, &dev, _1));
  ctx.run();
  BOOST_TEST(dev.connected);
}
