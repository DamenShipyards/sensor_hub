/**
 * \file serial.h
 * \brief Provide convenience header for asio::serial_port
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <vector>
#include <string>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/filesystem.hpp>

using serial_port = boost::asio::serial_port;
using asio_serial = boost::asio::serial_port_base;
namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

using std::runtime_error;

class Serial_exception: public runtime_error {
  using runtime_error::runtime_error;
};

struct Serial: public serial_port {
  using serial_port::serial_port;

  void open(const std::string& device_str) {
    std::vector<std::string> fields;
    boost::split(fields, device_str, [](char c) { return c == ':'; });
    std::string device_s;
    std::string baud_rate_s = "9600";
    std::string data_bits_s = "8";
    std::string parity_s = "N";
    std::string stop_bits_s = "1";
    switch (fields.size()) {
      case 5: 
        stop_bits_s = fields[4];
        /* fall through */
      case 4:
        parity_s = fields[3];
        /* fall through */
      case 3:
        data_bits_s = fields[2];
        /* fall through */
      case 2:
        baud_rate_s = fields[1];
        /* fall through */
      case 1:
        device_s = fields[0];
        break;
      default:
        log(level::error, "Empty Serial connection string");
        throw Serial_exception("Empty Serial connection string");
    }

    size_t len = 0;
    auto baud_rate = serial_port_base::baud_rate(std::stol(baud_rate_s, &len, 10));
    auto data_bits = serial_port_base::character_size(std::stol(data_bits_s, &len, 10));
    auto parity = serial_port::parity(serial_port::parity::none);
    if (parity_s == "E") {
      parity = serial_port::parity(asio_serial::parity::even);
    }
    else if (parity_s == "O") {
      parity = serial_port::parity(asio_serial::parity::odd);
    }
    else if (parity_s != "N") {
      log(level::error, "Invalid value for Serial parity: %", parity_s);
      throw Serial_exception("Invalid value for Serial parity");
    }
    auto stop_bits = asio_serial::stop_bits(asio_serial::stop_bits::one);
    if (stop_bits_s == "1.5") {
      stop_bits = asio_serial::stop_bits(asio_serial::stop_bits::onepointfive);
    }
    else if (stop_bits_s == "2") {
      stop_bits = asio_serial::stop_bits(asio_serial::stop_bits::two);
    }
    else if (stop_bits_s != "1") {
      log(level::error, "Invalid value for Serial stop bits: %", stop_bits_s);
      throw Serial_exception("Invalid value for Serial stop_bits");
    }

    serial_port::open(device_s);
    serial_port::set_option(baud_rate);
    serial_port::set_option(data_bits);
    serial_port::set_option(parity);
    serial_port::set_option(stop_bits);
    log(level::info, "Succesfully opened Serial port %, %, %, %, %", 
        device_s, baud_rate_s, data_bits_s, parity_s, stop_bits_s);
  }
};


inline bool can_open_serial(boost::asio::io_context& ctx, const std::string& device_str) {
  boost::system::error_code ec;
  serial_port s{ctx};
  // not (!) because "open" returns an error code, so success means *not* an error
  if (!s.open(device_str, ec)) {
    s.close();
    return true;
  }
  else {
    return false;
  }
}

#ifdef _WIN32

inline std::string get_serial_connection_string(boost::asio::io_context& ctx, 
    const std::string& prefix,
    const std::string& service,
    const std::string& device) {
  int ret;
  HKEY key = NULL;
  int count = 0;
  DWORD size = MAX_PATH;
  char dev_str[MAX_PATH];
  char com_str[MAX_PATH];
  std::string service_key = "SYSTEM\\CurrentControlSet\\Services\\" + service + "\\Enum";
  std::string serial_key = "HARDWARE\\DEVICEMAP\\SERIALCOMM";

  // Get the ublox driver service instance count
  ret = RegGetValueA(HKEY_LOCAL_MACHINE, service_key.c_str(), "Count", RRF_RT_DWORD, NULL, &count, &size);
  if (ret != ERROR_SUCCESS) {
    log(level::info, "Couldn't read registry value: %\\Count, error %", service_key, ret);
    return "serial_connection_string_not_found";
  };
  // Then loop over the entries...
  for (int i = 0; i < count; ++i) {
    size = MAX_PATH;
    ret = RegGetValueA(HKEY_LOCAL_MACHINE, service_key.c_str(), std::to_string(i).c_str(), RRF_RT_REG_SZ, NULL, &dev_str, &size);
    if (ret != ERROR_SUCCESS) {
      log(level::error, "Couldn't read registry value: %\\%, error %", service_key, i, ret);
      return "serial_connection_string_not_found";
    };
    // .. to see whether we found a matching device
    std::string full_device = std::string(dev_str);
    if (algo::starts_with(full_device, device)) {
      size = MAX_PATH;
      // .., if it matches, get it's com port from the registry
      std::string value_name = fmt::format("{:s}{:d}", prefix, i);
      ret = RegGetValueA(HKEY_LOCAL_MACHINE, serial_key.c_str(), value_name.c_str(), RRF_RT_REG_SZ, NULL, &com_str, &size);
      if (ret != ERROR_SUCCESS) {
        log(level::error, "Couldn't read registry value: %\\%, error %", serial_key, value_name, ret);
        return "serial_connection_string_not_found";
      };
      std::string result = fmt::format("\\\\.\\{:s}", com_str);
      if (can_open_serial(ctx, result)) {
        return result;
      }
    }
  }
  log(level::info, "Serial device %* not found or already connected", device);
  return "serial_connection_string_not_found";
}

#else

inline std::string get_serial_connection_string(boost::asio::io_context& ctx, const std::string& prefix) {
  static const auto dev_dir = fs::path("/dev/sensor_hub");
  std::string match = (dev_dir / prefix).string();
  fs::directory_iterator it{dev_dir};
  while (it != fs::directory_iterator{}) {
    std::string item = it->path().string();
    if (algo::starts_with(item, match)) {
      if (can_open_serial(ctx, item)) {
        return item;
      }
    } 
    ++it;
  }
  log(level::info, "Serial device %* not found or already connected", match);
  return "dev_connection_string_not_found";
}

#endif  // ifdef _WIN32

#endif  // SERIAL_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
