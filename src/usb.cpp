/**
 * \file usb.cpp
 * \brief Provide interface to libusb-1.0
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "usb.h"
#include "log.h"

#include <exception>

using std::runtime_error;

class Usb_exception: public runtime_error {
  using runtime_error::runtime_error;
};

struct Usb_context {
  Usb_context(Usb_context const&) = delete;
  void operator=(Usb_context const&) = delete;
  static Usb_context& get_instance() {
    static Usb_context instance;
    return instance;
  }
  libusb_context* get_context() {
    return ctx_;
  }
private:
  Usb_context(): ctx_(nullptr) {
    int r = libusb_init(&ctx_);
    if (r < 0) {
      log(level::error, "Failed to acquire USB context. Error %.", r);
      throw Usb_exception("Failed to acquire USB context");
    }
    log(level::info, "Acquired USB context");
  }
  ~Usb_context() {
    libusb_exit(ctx_);
    log(level::info, "Released USB context");
  }
  libusb_context* ctx_;
};


Usb::Usb(): ctx_(nullptr), device_(nullptr) {
  ctx_ = Usb_context::get_instance().get_context();
}

bool Usb::open_device(int vendor_id, int product_id, int seq) {
  libusb_device** devs;

  if (device_ != nullptr) {
    close_device();
  }

  auto cnt = libusb_get_device_list(ctx_, &devs); 
  if (cnt < 0) {
    log(level::error, "Get device list error %", cnt);
    return false;
  }

  for(int i = 0; i < cnt; i++) {
    libusb_device* dev = devs[i];
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      log(level::error, "Failed to get device descriptor, error %", r);
      continue;
    }
    if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
      --seq;
      if (seq < 0) {
        int bus = static_cast<int>(libusb_get_bus_number(dev));
        int port = static_cast<int>(libusb_get_port_number(dev));
        log(level::info, "Found usb device at position %, bus %, port %", i, bus, port);
        r = libusb_open(dev, &device_);
        if (r < 0) {
          log(level::error, "Failed to open device, error %", r);
          continue;
        }
        // On Linux, we want to detach any kernel driver. Shouldn't do any harm on Windows
        libusb_set_auto_detach_kernel_driver(device_, 1);
        r = libusb_claim_interface(device_, 0); 
        if (r != LIBUSB_SUCCESS) {
          log(level::error, "Failed to claim USB interface, error %", r);
          close_device();
          break;
        }
        log(level::info, "Successfully opened USB device");
        break;
      }
    }
  }
  libusb_free_device_list(devs, 1);
  return device_ != nullptr;
}

Usb::~Usb() {
  close_device();
  // Context will be freed by Usb_context singleton
}

void Usb::close_device() {
  if (device_ != nullptr) {
    int r = libusb_release_interface(device_, 0);
    if(r != 0) {
      log(level::error, "Cannot release USB interface, error %", r);
    }
    libusb_close(device_);
    log(level::info, "Closed USB device");
    device_ = nullptr;
  }
}
