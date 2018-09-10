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
#include "spirit_x3.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>


#include <xsens/xsxbusmessageid.h>
#include <xsens/xsdataidentifier.h>
#include <xsens/xsdataidentifiervalue.h>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>

namespace posix_time = boost::posix_time;

// Data types for data communicated with the sensor
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;


namespace data {

extern cbyte_t packet_start;
extern cbyte_t sys_command;
extern cbyte_t conf_command;

extern cdata_t goto_config;
extern cdata_t config_ack;

extern cdata_t goto_measurement;
extern cdata_t measurement_ack;

extern cdata_t set_option_flags;
extern cdata_t option_flags_ack;

extern cdata_t req_reset;
extern cdata_t reset_ack;

extern cdata_t req_product_code;
extern cdata_t req_utc_time;
extern cdata_t req_firmware_rev;

extern cdata_t set_output_configuration;
extern cdata_t output_configuration_ack;

}


extern std::ostream& operator<<(std::ostream& os, cdata_t data);


namespace parser {

namespace x3 = boost::spirit::x3;

struct Packet_parser {
  Packet_parser();
  ~Packet_parser();
  struct Data_packets;
  struct Data_visitor;
  std::deque<uint8_t> queue;
  std::vector<uint8_t> data;
  std::unique_ptr<Data_packets> data_packets;
  std::unique_ptr<Data_visitor> visitor;
  typename std::deque<uint8_t>::iterator cur;

  template <typename Iterator>
  void parse(Iterator begin, Iterator end) {
    if (queue.size() > 0x1000)
      // Something is wrong. Hose the queue
      queue.clear();
    queue.insert(queue.end(), begin, end);
    parse();
  }
  void parse();
  const std::vector<double>& get_data() const;
};

} //parser



template <typename Port, typename ContextProvider>
struct Xsens: public Port_device<Port, ContextProvider> {

  bool exec_command(cdata_t& command, cdata_t& expected_response, asio::yield_context yield) {
    Port& port = this->get_port();

    // Set a timeout for the command to complete
    asio::deadline_timer timeout(ContextProvider::get_context(), posix_time::milliseconds(400));
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


  void poll_data(asio::yield_context yield) {
    log(level::debug, "Polling Xsens");
    asio::streambuf buf;
    while (this->is_connected()) {
      auto bytes_read = this->get_port().async_read_some(buf.prepare(512), yield);
      log(level::debug, "Read % bytes", bytes_read);
      if (bytes_read > 0) {
        buf.commit(bytes_read);
        auto buf_begin = asio::buffers_begin(buf.data());
        auto buf_end = buf_begin + buf.size();
#ifdef DEBUG
        cdata_t data(buf_begin, buf_end);
        std::stringstream ss;
        ss << data;
        log(level::debug, "XSens received: %", ss.str());
#endif
        parser_.parse(buf_begin, buf_end);
        buf.consume(bytes_read);
      }
    }
  }

  bool goto_config(asio::yield_context yield) {
    return exec_command(data::goto_config, data::config_ack, yield);
  }

  bool goto_measurement(asio::yield_context yield) {
    return exec_command(data::goto_measurement, data::measurement_ack, yield);
  }

  virtual bool set_output_configuration(asio::yield_context yield) {
    return true;
  }

  virtual bool set_option_flags(asio::yield_context yield) {
    return true;
  }

  void start_polling() {
    auto executor = this->get_port().get_executor();
    asio::post(
        executor,
        [executor, this]() {
          asio::spawn(executor, boost::bind(&Xsens::poll_data, this, _1));
        }
    );
  }

  bool initialize(asio::yield_context yield) override {
    bool result = goto_config(yield)
        && set_option_flags(yield)
        && set_output_configuration(yield)
        && goto_measurement(yield);


    if (result) {
      log(level::info, "Successfully initialized Xsens device");
    }
    else {
      log(level::error, "Failed to initialize Xsens device");
    }

    start_polling();

    return result;
  }

  const parser::Packet_parser& get_parser() const {
    return parser_;
  }
private:
  parser::Packet_parser parser_;
};


template <typename Port, typename ContextProvider>
struct Xsens_MTi_G_710: public Xsens<Port, ContextProvider> {

  Xsens_MTi_G_710(): Xsens<Port, ContextProvider>() {
    log(level::info, "Constructing Xsens_MTi_G_710");
  }

  ~Xsens_MTi_G_710() override {
    log(level::info, "Destroying Xsens_MTi_G_710");
  }

  bool set_output_configuration(asio::yield_context yield) override {
    return this->exec_command(data::set_output_configuration, data::output_configuration_ack, yield);
  }

  bool set_option_flags(asio::yield_context yield) override {
    return this->exec_command(data::set_option_flags, data::option_flags_ack, yield);
  }
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
