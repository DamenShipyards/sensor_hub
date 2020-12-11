/**
 * \file install.h
 * \brief
 * Installs WinUSB driver for libusb if it's not
 * already installed.
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../log.h"

#ifdef _WIN32
#include <libwdi.h>
#endif

#include <utility>
#include <string>

#include <boost/algorithm/string.hpp>

#define INF_NAME    "usb_device.inf"
#define DEFAULT_DIR "usb_driver"

inline std::pair<int, int> get_usb_address(const std::string& connection_string) {
  std::vector<std::string> fields;
  boost::split(fields, connection_string, [](char c) { return c == ','; });
  std::string device_str = fields[0];
  boost::split(fields, device_str, [](char c) { return c == ':'; });
  int vendor_id = -1;
  int product_id = -1;
  try {
    if (fields.size() == 2) {
      vendor_id = std::stol(fields[0], 0, 16);
      product_id = std::stol(fields[1], 0, 16);
    }
  } catch (std::exception& e) {
    log(level::debug, "Failed to get USB address from: %", connection_string);
  }
  return std::pair<int, int>(vendor_id, product_id);
}

#ifdef _WIN32
inline void check_install_usb_driver(int vid, int pid) {
  if (vid <= 0)
    return;
  struct wdi_device_info *device, *list;
  if (wdi_create_list(&list, NULL) == WDI_SUCCESS) {
    for (device = list; device != NULL; device = device->next) {
      if (device->vid == vid && device->pid == pid) {
        int result = WDI_SUCCESS;
        log(level::info, "Installing WinUSB driver for %", device->desc);

        wdi_options_prepare_driver prepare_options = { 0 };
        prepare_options.driver_type = WDI_WINUSB;
        prepare_options.vendor_name = "Damen Shipyards";
        prepare_options.cert_subject = "Damen Shipyards Signer";

        wdi_options_install_driver install_options = {0};
        install_options.pending_install_timeout = 10000;

        if ((result = wdi_prepare_driver(device, DEFAULT_DIR, INF_NAME, &prepare_options)) == WDI_SUCCESS) {
          if ((result = wdi_install_driver(device, DEFAULT_DIR, INF_NAME, NULL)) == WDI_SUCCESS) {
            log(level::info, "Successfully installed WinUSB driver");
          }
          else {
            log(level::error, "Failed to install USB driver: %, %", result, wdi_strerror(result));
          }
        }
        else {
          log(level::error, "Failed to prepare USB driver: %, %", result, wdi_strerror(result));
        }
      }
    }
    wdi_destroy_list(list);
  }
}
#else
inline void check_install_usb_driver(int, int) {}
#endif
