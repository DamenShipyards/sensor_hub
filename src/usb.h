/**
 * \file usb.h
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

#ifndef USB_H_
#define USB_H_

#include <string>
#include <memory>
#include <libusb-1.0/libusb.h>

struct Usb {
  Usb();
  ~Usb();
  bool open_device(int vendor_id, int product_id, int seq=0);
  bool open_device(const std::string& device_str, int seq=0);
  void close_device();
  bool read();
  std::string data;
  bool called_back;
private:
  libusb_context* ctx_;
  libusb_device_handle* device_;
  struct Usb_descriptors;
  std::unique_ptr<Usb_descriptors> descriptors_;
};

#endif
