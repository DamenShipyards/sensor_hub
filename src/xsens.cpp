/**
 * \file xsens.cpp
 * \brief Provide implementation for Xsens device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "xsens.h"

#include <memory>

using Xsens_MTi_G_710_usb = Xsens_MTi_G_710<Usb>;
using Xsens_MTi_G_710_serial = Xsens_MTi_G_710<asio::serial_port>;
using Xsens_MTi_G_710_usb_factory = Device_factory<Xsens_MTi_G_710_usb>;
using Xsens_MTi_G_710_serial_factory = Device_factory<Xsens_MTi_G_710_serial>;

static auto& mti_g_710_usb_factory =
    add_device_factory("xsens_mti_g_710_usb", std::move(std::make_unique<Xsens_MTi_G_710_usb_factory>()));
static auto& mti_g_710_serial_factory =
    add_device_factory("xsens_mti_g_710_serial", std::move(std::make_unique<Xsens_MTi_G_710_serial_factory>()));


std::ostream& operator<<(std::ostream& os, cdata_t data) {
  os << std::hex;
  bool tween = false;
  for (auto&& b: data) {
    if (tween)
      os << " ";
    else 
      tween = true;
    os << std::setfill('0') << std::setw(2) << int(b);
  }
  os << std::dec;
  return os;
}


namespace data {

cbyte_t packet_start = 0xFA;
cbyte_t sys_command = 0xFF;
cbyte_t conf_command = 0x01;

cdata_t goto_config = {packet_start, sys_command, 0x30, 0x00, 0xD1};
cdata_t config_ack = { packet_start, sys_command, 0x31, 0x00, 0xD0};

cdata_t goto_measurement = {packet_start, sys_command, 0x10, 0x00, 0xF1};
cdata_t measurement_ack = { packet_start, sys_command, 0x11, 0x00, 0xF0};

cdata_t set_option_flags = {packet_start, sys_command, 0x48, 0x08,
    0x00, 0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x00, 0x1E};
cdata_t option_flags_ack = {packet_start, sys_command, 0x49, 0x00, 0xB8};
cdata_t req_reset = {0xFA, 0xFF, 0x40, 0x00, 0xC1};
cdata_t reset_ack = {0xFA, 0xFF, 0x41, 0x00, 0xC0};

cdata_t req_product_code = {0xFA, 0xFF, 0x1C, 0x00, 0xE5};
cdata_t req_utc_time = {0xFA, 0xFF, 0x60, 0x00, 0xA1};
cdata_t req_firmware_rev = {0xFA, 0xFF, 0x12, 0x00, 0xEF};

cdata_t set_output_configuration = {
  packet_start, sys_command,
  0xC0, 0x24,             // Command + size
  0x10, 0x10, 0x00, 0x00, // Utc time
  0x40, 0x20, 0x00, 0x64, // Acceleration, 100Hz
  0x40, 0x30, 0x00, 0x64, // Free Acceleration, 100Hz
  0x80, 0x20, 0x00, 0x64, // Rate of turn, 100Hz
  0x50, 0x40, 0x00, 0x0A, // LatLon, 10Hz
  0xC0, 0x20, 0x00, 0x0A, // Magnetic flux, 10Hz
  0xD0, 0x10, 0x00, 0x0A, // Velocity, 10Hz
  0x50, 0x20, 0x00, 0x0A, // Altitude above ellipsoid, 10Hz
  0x50, 0x10, 0x00, 0x0A, // Altitude above MSL, 10Hz
  0x0F
};

cdata_t output_configuration_ack = {
  packet_start, sys_command,
  0xC1, 0x24,
  0x10, 0x10, 0xFF, 0xFF,
  0x40, 0x20, 0x00, 0x64,
  0x40, 0x30, 0x00, 0x64,
  0x80, 0x20, 0x00, 0x64,
  0x50, 0x40, 0x00, 0x0A,
  0xC0, 0x20, 0x00, 0x0A,
  0xD0, 0x10, 0x00, 0x0A,
  0x50, 0x20, 0x00, 0x0A,
  0x50, 0x10, 0x00, 0x0A,
  0x10
};

} // namespace data


// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
