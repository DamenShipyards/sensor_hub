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

#include <fmt/core.h>

#include <exception>
#include <cstdlib>

using std::runtime_error;
namespace asio = boost::asio;


class Usb_exception: public runtime_error {
  using runtime_error::runtime_error;
};



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
      : handle(nullptr), active_config(nullptr), descriptor() {
    handle = device_handle;
    libusb_device* device = libusb_get_device(device_handle);
    int r = libusb_get_device_descriptor(device, &descriptor);
    if (r != LIBUSB_SUCCESS) {
      throw Usb_exception("Failed to get device descriptor");
    }
    r = libusb_get_active_config_descriptor(device, &active_config);
    if (r != LIBUSB_SUCCESS) {
      throw Usb_exception("Failed to get active config descriptor");
    }
  }
  ~Usb_descriptors() {
    if (active_config != nullptr) {
      libusb_free_config_descriptor(active_config);
    }
  }
  std::string get_string_descriptor(uint8_t index) {
    unsigned char str[256];
    int len = libusb_get_string_descriptor_ascii(handle, index, str, sizeof(str));
    if (len < 0) {
      return fmt::format("Failed to get string at index {}", index);
    }
    return std::string{str, str + len};
  }
  void log_device_info() {
    std::string manufacturer = get_string_descriptor(descriptor.iManufacturer);
    std::string product = get_string_descriptor(descriptor.iProduct);
    std::string serial = get_string_descriptor(descriptor.iSerialNumber);
    log(level::info, "USB device: Manufacturer: %, Product: %, Serial: %, Configs: %, Class: %, SubClass: %, Protocol: %",
        manufacturer, product, serial, static_cast<int>(descriptor.bNumConfigurations),
        get_usb_class_string(descriptor.bDeviceClass),static_cast<int>(descriptor.bDeviceSubClass),
        static_cast<int>(descriptor.bDeviceProtocol));
    std::string config_name = get_string_descriptor(active_config->iConfiguration);
    log(level::info, "  Device configuration: %, Attributes %, Interfaces: %", 
        config_name, static_cast<int>(active_config->bmAttributes), static_cast<int>(active_config->bNumInterfaces));
    for (int i = 0; i < active_config->bNumInterfaces; ++i) {
      libusb_interface iface = active_config->interface[i];
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
    if (active_config != nullptr) {
      return static_cast<int>(active_config->bNumInterfaces);
    }
    else {
      return 0;
    }
  }
  libusb_device_handle* handle;
  libusb_device_descriptor descriptor;
  libusb_config_descriptor* active_config;
};


Usb::Usb(asio::io_context& io_context)
    : io_ctx_(io_context), ctx_(nullptr), device_(nullptr), descriptors_(nullptr) {
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

        // Get set of descriptors for this device
        try {
          descriptors_ = std::make_unique<Usb_descriptors>(device_);
          descriptors_->log_device_info();
        }
        catch (Usb_exception& e) {
          log(level::error, "Failed to get device descriptors, error %", e.what());
          continue;
        }

        // On Linux, we want to detach any kernel driver. Shouldn't do any harm on Windows
        libusb_set_auto_detach_kernel_driver(device_, 1);

        // Claim interfaces
        for (int i = 0; i < descriptors_->get_interface_count(); ++i) {
          r = libusb_claim_interface(device_, i); 
          if (r != LIBUSB_SUCCESS) {
            log(level::error, "Failed to claim USB interface %, error %", i, r);
            close();
            goto interface_failure;
          }
        }

        log(level::info, "Successfully opened USB device");
        break;
interface_failure:
        continue;
      }
    }
  }
  libusb_free_device_list(devs, 1);
  return device_ != nullptr;
}

bool Usb::open(const std::string& device_str, int seq) {
  if (device_str.size() != 9) 
    return false;
  const char* device_cstr = device_str.c_str();
  char* endp;
  int vendor_id = std::strtol(device_cstr, &endp, 16);
  // Skip colon
  ++endp;
  int product_id = std::strtol(endp, &endp, 16);
  return open(vendor_id, product_id, seq);
}


void Usb::close() {
  if (device_ != nullptr) {
    for (int i = 0; i < descriptors_->get_interface_count(); ++i) {
      int r = libusb_release_interface(device_, i); 
      if (r != LIBUSB_SUCCESS) {
        log(level::error, "Failed to release USB interface %, error %", i, r);
      }
    }
    descriptors_ = nullptr;
    libusb_close(device_);
    log(level::info, "Closed USB device");
    device_ = nullptr;
  }
}

void handle_transfer(libusb_transfer* trnsfr) {
  Usb* usb = (Usb*)(trnsfr->user_data);
  switch(trnsfr->status) {
	case LIBUSB_TRANSFER_COMPLETED:
      std::cout << "Called back with " << trnsfr->actual_length << " bytes of data!" << std::endl;
	  break;
	case LIBUSB_TRANSFER_CANCELLED:
      std::cout << "Called back with " << trnsfr->actual_length << " bytes of data!" << std::endl;
      break;
	case LIBUSB_TRANSFER_NO_DEVICE:
      std::cout << "Called back with no device error" << std::endl;
      break;
	case LIBUSB_TRANSFER_TIMED_OUT:
      std::cout << "Called back with transfer timeout" << std::endl;
      break;
	case LIBUSB_TRANSFER_ERROR:
      std::cout << "Called back with transfer error" << std::endl;
      break;
	case LIBUSB_TRANSFER_STALL:
      std::cout << "Called back with transfer stall" << std::endl;
      break;
	case LIBUSB_TRANSFER_OVERFLOW:
      std::cout << "Called back with transfer overflow" << std::endl;
	  break;
    default:
      std::cout << "Called back with unexpected status" << std::endl;
  }
}

bool Usb::read() {
  libusb_transfer* trnsfr = libusb_alloc_transfer(1);
  if (trnsfr == nullptr) {
    log(level::error, "Failed to allocate USB transfer buffer");
    return false;
  }

  unsigned char* buf = (unsigned char*)malloc(0x100);
  if (buf == nullptr) {
    log(level::error, "Failed to allocate buffer for transfer data");
    return false;
  }
  libusb_fill_bulk_transfer(
      trnsfr, 
      device_,
      131,
      buf,
      0x80,
      &handle_transfer,
      this,
      5000);


  int r = libusb_submit_transfer(trnsfr);
  if (r != 0) {
    log(level::error, "Failed to submit USB transfer, error %", r);
    return false;
  }

  r = libusb_handle_events_completed(ctx_, nullptr);
  if (r != 0) {
    log(level::error, "Failed to handle USB events, error %", r);
    return false;
  }

  return true;
}

