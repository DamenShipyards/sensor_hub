// Avoid warnings from libusb.h which we won't fix
#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif

#include <libusb-1.0/libusb.h>

#include <iostream>

int main() {
  std::cout << "USB device info" << std::endl;

  libusb_context* ctx_;
  int r = libusb_init(&ctx_);
  if (r < 0) {
    std::cerr << "Failed to acquire USB context: " <<  r << std::endl;
  }


  libusb_device** devs;

  auto cnt = libusb_get_device_list(ctx_, &devs);
  if (cnt < 0) {
    std::cerr << "Get device list error: " << cnt << std::endl;
    return 1;
  } 

  for (int i = 0; i < cnt; i++) {
    libusb_device* dev = devs[i];
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      std::cerr <<  "Failed to get device descriptor: " << r << std::endl;
      continue;
    }
    libusb_device_handle* device;
    r = libusb_open(dev, &device);
    if (r < 0) {
      std::cout << "Failed to open device: " << r << std::endl;
      continue;
    }
    unsigned char s[256];
    r = libusb_get_string_descriptor_ascii(device, 1, s, 255);
    if (r < 0) {
      std::cout << "Failed to  get string descriptor: " << r << std::endl;
      continue;
    }
    std::cout << s << std::endl;

    libusb_close(device);
  }

}
