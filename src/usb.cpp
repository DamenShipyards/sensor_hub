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

// for timeval
#include <time.h>

#include <cstdlib>
#include <functional>

#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace asio = boost::asio;
namespace posix_time = boost::posix_time;


std::string get_usb_class_string(uint8_t class_enum) {
  switch(class_enum) {
    case LIBUSB_CLASS_PER_INTERFACE: return "From device/per interface";
    case LIBUSB_CLASS_AUDIO: return "Audio";
    case LIBUSB_CLASS_COMM: return "Communications";
    case LIBUSB_CLASS_HID: return "Human interface device";
    case LIBUSB_CLASS_PHYSICAL: return "Physical";
    case LIBUSB_CLASS_PRINTER: return "Printer";
    case LIBUSB_CLASS_IMAGE: return "Image";
    case LIBUSB_CLASS_MASS_STORAGE: return "Mass storage";
    case LIBUSB_CLASS_HUB: return "Hub";
    case LIBUSB_CLASS_DATA: return "Data";
    case LIBUSB_CLASS_SMART_CARD: return "Smart card";
    case LIBUSB_CLASS_CONTENT_SECURITY: return "Content security";
    case LIBUSB_CLASS_VIDEO: return "Video";
    case LIBUSB_CLASS_PERSONAL_HEALTHCARE: return "Personal healthcare";
    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE: return "Diagnostic device";
    case LIBUSB_CLASS_WIRELESS: return "Wireless";
    case LIBUSB_CLASS_APPLICATION: return "Application";
    case LIBUSB_CLASS_VENDOR_SPEC: return "Vendor-specific";
	default: return "Unknown";
  }
}



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



struct Usb::Usb_descriptors {
  Usb_descriptors() = delete;
  Usb_descriptors(libusb_device_handle* device_handle)
      : handle_(device_handle), descriptor_(), active_config_(nullptr) {
    libusb_device* device = libusb_get_device(handle_);
    int r = libusb_get_device_descriptor(device, &descriptor_);
    if (r != LIBUSB_SUCCESS) {
      throw Usb_exception("Failed to get device descriptor");
    }
    r = libusb_get_active_config_descriptor(device, &active_config_);
    if (r != LIBUSB_SUCCESS) {
      throw Usb_exception("Failed to get active config descriptor");
    }
  }

  ~Usb_descriptors() {
    close();
  }

  void close() {
    if (active_config_ != nullptr) {
      libusb_free_config_descriptor(active_config_);
    }
    active_config_ = nullptr;
  }

  std::string get_string_descriptor(uint8_t index) {
    unsigned char str[256];
    int len = libusb_get_string_descriptor_ascii(handle_, index, str, sizeof(str));
    if (len < 0) {
      return fmt::format("Failed to get string at index {}", index);
    }
    return std::string{str, str + len};
  }

  void foreach_endpoint(const std::function<void(libusb_endpoint_descriptor&)>& f) {
    if (active_config_ == nullptr)
      return;
    for (int i = 0; i < active_config_->bNumInterfaces; ++i) {
      libusb_interface iface = active_config_->interface[i];
      for (int j = 0; j < iface.num_altsetting; ++j) {
        libusb_interface_descriptor iface_desc = iface.altsetting[j];
        for (int k = 0; k < iface_desc.bNumEndpoints; ++k) {
          libusb_endpoint_descriptor endpoint = iface_desc.endpoint[k];
          f(endpoint);
        }
      }
    }
  }

  int get_read_endpoint() {
    int result = LIBUSB_ENDPOINT_IN;
    foreach_endpoint(
        [&](libusb_endpoint_descriptor& endpoint) {
          if (endpoint.bmAttributes == LIBUSB_TRANSFER_TYPE_BULK) {
            if ((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) != 0)
              result = static_cast<int>(endpoint.bEndpointAddress);
          }
        }
    );
    return result;
  }

  size_t get_read_packet_size(int endpoint_address) {
    int result = 512;
    foreach_endpoint(
        [&](libusb_endpoint_descriptor& endpoint) {
          if (endpoint.bEndpointAddress == endpoint_address) {
            result = static_cast<size_t>(endpoint.wMaxPacketSize);
          }
        }
    );
    return result;
  }

  int get_write_endpoint() {
    int result = LIBUSB_ENDPOINT_OUT;
    foreach_endpoint(
        [&](libusb_endpoint_descriptor& endpoint) {
          if (endpoint.bmAttributes == LIBUSB_TRANSFER_TYPE_BULK) {
            if ((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == 0)
              result = static_cast<int>(endpoint.bEndpointAddress);
          }
        }
    );
    return result;
  }

  void log_device_info() {
    std::string manufacturer = get_string_descriptor(descriptor_.iManufacturer);
    std::string product = get_string_descriptor(descriptor_.iProduct);
    std::string serial = get_string_descriptor(descriptor_.iSerialNumber);
    log(level::info, "USB device: Manufacturer: %, Product: %, Serial: %, Configs: %, Class: %, SubClass: %, Protocol: %",
        manufacturer, product, serial, static_cast<int>(descriptor_.bNumConfigurations),
        get_usb_class_string(descriptor_.bDeviceClass),static_cast<int>(descriptor_.bDeviceSubClass),
        static_cast<int>(descriptor_.bDeviceProtocol));
    std::string config_name = get_string_descriptor(active_config_->iConfiguration);
    log(level::info, "  Device configuration: %, Attributes %, Interfaces: %",
        config_name, static_cast<int>(active_config_->bmAttributes), static_cast<int>(active_config_->bNumInterfaces));
    for (int i = 0; i < active_config_->bNumInterfaces; ++i) {
      libusb_interface iface = active_config_->interface[i];
      for (int j = 0; j < iface.num_altsetting; ++j) {
        libusb_interface_descriptor iface_desc = iface.altsetting[j];
        std::string iface_name = get_string_descriptor(iface_desc.iInterface);
        log(level::info, "    Interface: %, %, Endpoints: %, Class: %, SubClass: %, Protocol: %",
            static_cast<int>(iface_desc.bInterfaceNumber), iface_name, static_cast<int>(iface_desc.bNumEndpoints),
            get_usb_class_string(iface_desc.bInterfaceClass),static_cast<int>(iface_desc.bInterfaceSubClass),
            static_cast<int>(iface_desc.bInterfaceProtocol));
        for (int k = 0; k < iface_desc.bNumEndpoints; ++k) {
          libusb_endpoint_descriptor endpoint = iface_desc.endpoint[k];
          log(level::info, "      Endpoint: %, Addresss: %, Attributes: %, Max packet size: %, Poll interval: %",
              k, static_cast<int>(endpoint.bEndpointAddress), static_cast<int>(endpoint.bmAttributes),
              static_cast<int>(endpoint.wMaxPacketSize), static_cast<int>(endpoint.bInterval));
        }
      }
    }
  }
  int get_interface_count() {
    if (active_config_ != nullptr) {
      return static_cast<int>(active_config_->bNumInterfaces);
    }
    else {
      return 0;
    }
  }
private:
  libusb_device_handle* handle_;
  libusb_device_descriptor descriptor_;
  libusb_config_descriptor* active_config_;
};



struct Usb::Usb_event_handler {
  Usb_event_handler() = delete;
  Usb_event_handler(libusb_context* usb_ctx)
      : usb_ctx_(usb_ctx),
        handler_ctx_(), work_guard_(make_work_guard(handler_ctx_)),
        worker_(boost::bind(&asio::io_context::run, &handler_ctx_)) {
  }
  ~Usb_event_handler() {
    close();
  }
  void close() {
    usb_ctx_ = nullptr;
    if (!handler_ctx_.stopped()) {
      handler_ctx_.stop();
      worker_.join();
    }
  }
  void handle_events() {
    if (usb_ctx_ != nullptr && !handler_ctx_.stopped())
      handler_ctx_.post(boost::bind(&Usb_event_handler::handle_events_, this));
  }
private:
  libusb_context* usb_ctx_;
  asio::io_context handler_ctx_;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  boost::thread worker_;

  void handle_events_() {
    timeval tv = timeval{0, 100000};
    while (usb_ctx_ != nullptr) {
      int r = libusb_handle_events_timeout_completed(usb_ctx_, &tv, nullptr);
      if (r != 0) {
        log(level::error, "Failed to handle USB events, error %", r);
      }
    }
  }

};


Usb::Usb(boost::asio::io_context& io_context)
    : io_ctx_(io_context), strand_(io_context),
      ctx_(nullptr), device_(nullptr), descriptors_(nullptr),
      read_endpoint_(0), write_endpoint_(0), read_packet_size_(0),
      transfers_() {
  ctx_ = Usb_context::get_instance().get_context();
}


Usb::~Usb() {
  close();
  // Context will be freed by Usb_context singleton
}


bool Usb::open(int vendor_id, int product_id, int seq) {
  libusb_device** devs;

  // Start by closing any previously opened device
  close();

  auto cnt = libusb_get_device_list(ctx_, &devs);
  if (cnt < 0) {
    log(level::error, "Get device list error %", cnt);
    return false;
  }

  for (int i = 0; i < cnt; i++) {
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

        // Get set of descriptors for this device
        try {
          descriptors_ = std::make_unique<Usb_descriptors>(device_);
          descriptors_->log_device_info();
        }
        catch (Usb_exception& e) {
          log(level::error, "Failed to get device descriptors, error %", e.what());
          goto descriptor_failure;
        }

        // On Linux, we want to detach any kernel driver. Shouldn't do any harm on Windows
        libusb_set_auto_detach_kernel_driver(device_, 1);

        // Claim interfaces
        for (int i = 0; i < descriptors_->get_interface_count(); ++i) {
          r = libusb_claim_interface(device_, i);
          if (r != LIBUSB_SUCCESS) {
            log(level::error, "Failed to claim USB interface %, error %", i, r);
            goto interface_failure;
          }
        }

        read_endpoint_ = descriptors_->get_read_endpoint();
        write_endpoint_ = descriptors_->get_write_endpoint();
        read_packet_size_ = descriptors_->get_read_packet_size(read_endpoint_);

        event_handler_ = std::make_unique<Usb_event_handler>(ctx_);
        event_handler_->handle_events();

        log(level::info, "Successfully opened USB device with endpoints: %, %: %",
            write_endpoint_, read_endpoint_, read_packet_size_);
        break;
      interface_failure:
        descriptors_ = nullptr;
      descriptor_failure:
        libusb_close(device_);
        device_ = nullptr;
        continue;
      }
    }
  }
  libusb_free_device_list(devs, 1);
  return device_ != nullptr;
}

bool Usb::open(const std::string& device_str, int seq) {
  std::vector<std::string> fields;
  boost::split(fields, device_str, [](char c) { return c == ':'; });
  if (fields.size() != 2) {
    log(level::error, "Invalid USB connection string: %", device_str);
    return false;
  }
  int vendor_id = std::stol(fields[0], 0, 16);
  int product_id = std::stol(fields[1], 0, 16);
  return open(vendor_id, product_id, seq);
}

void Usb::open(const std::string& device_str) {
  std::vector<std::string> fields;
  boost::split(fields, device_str, [](char c){ return c == ','; });
  bool result = false;
  switch (fields.size()) {
    case 1:
      result = open(fields[0], 0);
      break;
    case 2: {
      int seq = std::stoi(fields[1]);
      result = open(fields[0], seq);
      break;
    }
    default:
      log(level::error, "Invalid USB connection string: %", device_str);
  }
  if (!result) {
    throw Usb_exception("USB open failure");
  }
}

void Usb::close() {
  cancel();
  event_handler_ = nullptr;
  if (device_ != nullptr) {
    if (descriptors_ != nullptr) {
      for (int i = 0; i < descriptors_->get_interface_count(); ++i) {
        int r = libusb_release_interface(device_, i);
        if (r != LIBUSB_SUCCESS) {
          log(level::error, "Failed to release USB interface %, error %", i, r);
        }
      }
      descriptors_ = nullptr;
    }
    libusb_close(device_);
    log(level::info, "Closed USB device");
    device_ = nullptr;
  }
}

void Usb::cancel() {
  transfers_.cancel();
  asio::deadline_timer tmr(io_ctx_, posix_time::milliseconds(50));
  tmr.wait();
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
