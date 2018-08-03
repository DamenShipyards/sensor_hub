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

static int ctx_refcnt_ = 0;
static libusb_context* single_ctx_ = nullptr;

libusb_context* get_ctx() {
  if (ctx_refcnt_ == 0) {
    int r = libusb_init(&single_ctx_);
    if (r < 0) {
      log(level::error, "Failed to acquire USB context. Error %.", r);
      return nullptr;
    }
    ++ctx_refcnt_;
    return single_ctx_;
  }
}

Usb::Usb(): ctx_(nullptr) {
  ctx_ = get_ctx();
}

bool Usb::open_device(int vendor_id, int device_id, int seq) {
  libusb_device **devs;
  
  

}

void Usb::close_device() {
}
