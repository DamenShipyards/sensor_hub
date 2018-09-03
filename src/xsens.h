/**
 * \file xsens.h
 * \brief Provide interface to xsens device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "device.h"
#include "log.h"
#include "usb.h"
#include "tools.h"
#include "loop.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace posix_time = boost::posix_time;

// Data types for data communicated with the sensor
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;

cbyte_t packet_start = 0xFA;
cbyte_t sys_command = 0xFF;
cbyte_t conf_command = 0x01;

extern cdata_t goto_config_command;
extern cdata_t goto_measurement_command;

extern cdata_t config_ok;
extern cdata_t measurement_ok;


template <typename Port, typename ContextProvider=Context_provider>
struct Xsens: public Port_device<Port, ContextProvider> {
  bool exec_command(cdata_t& command, cdata_t& expected_response, asio::yield_context yield) {
    Port& port = this->get_port();

    // Set a timeout for the command to complete
    asio::deadline_timer timeout(ContextProvider::get_context(), posix_time::milliseconds(200));
    timeout.async_wait(
        [&](const boost::system::error_code& error) {
          if (!error)
            port.cancel();
        });

    // Write out the command string...
    asio::async_write(port, asio::buffer(command), yield);
    // ... and look for the expected response
    try {
      int repeats = 4;
      data_t response{};
      bool response_found = false;
      do {
        asio::streambuf read_buf;
        size_t bytes_read = port.async_read_some(read_buf.prepare(0x1000), yield);
        read_buf.commit(bytes_read);
        response.insert(
            response.end(), 
            asio::buffers_begin(read_buf.data()), 
            asio::buffers_begin(read_buf.data()) + bytes_read);
        read_buf.consume(bytes_read);
        response_found = contains(response, expected_response);
      } while (--repeats > 0 && !response_found);
      timeout.cancel();
      if (repeats <= 0)
        return false;
    }
    catch (std::exception& e) {
      // Probably a timeout
      log(level::error, "%: Error executing command: %", this->get_name(), e.what());
      timeout.cancel();
      port.cancel();
      return false;
    }
    return true;
  }

  bool goto_config(asio::yield_context yield) {
    return exec_command(goto_config_command, config_ok, yield);
  }

  bool goto_measurement(asio::yield_context yield) {
    return exec_command(goto_measurement_command, measurement_ok, yield);
  }

  bool initialize(asio::yield_context yield) override {
    return goto_config(yield)
      && goto_measurement(yield);
  }
};


template <typename Port, typename ContextProvider=Context_provider>
struct Xsens_MTi_G_710: public Xsens<Port, ContextProvider> {
  Xsens_MTi_G_710(): Xsens<Port>() {
    log(level::info, "Constructing Xsens_MTi_G_710");
  }
  ~Xsens_MTi_G_710() override {
    log(level::info, "Destroying Xsens_MTi_G_710");
  }
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
