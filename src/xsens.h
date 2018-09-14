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
#include "datetime.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/endian/conversion.hpp>

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


namespace command {

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
extern cdata_t product_code_resp;

extern cdata_t req_device_id;
extern cdata_t device_id_resp;

extern cdata_t req_firmware_rev;
extern cdata_t firmware_rev_resp;

extern cdata_t req_utc_time;

extern cdata_t set_output_configuration;
extern cdata_t output_configuration_ack;

extern cdata_t error_resp;
}


extern std::ostream& operator<<(std::ostream& os, cdata_t data);


namespace parser {

namespace x3 = boost::spirit::x3;

using Values_type = std::vector<Quantity_value>;
using Values_queue = std::deque<Quantity_value>;

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
  Values_queue& get_values();
};

} //parser



template <typename Port, typename ContextProvider>
struct Xsens: public Port_device<Port, ContextProvider> {

  bool exec_command(cdata_t& command, cdata_t& expected_response, asio::yield_context yield, std::string* data=nullptr) {
    Port& port = this->get_port();

    // Set a timeout for the command to complete
    asio::deadline_timer timeout(ContextProvider::get_context(), posix_time::milliseconds(1000));
    timeout.async_wait(
        [&](const boost::system::error_code& error) {
          if (!error)
            port.cancel();
        });

    // Write out the command string...
    asio::async_write(port, asio::buffer(command), yield);

    std::stringstream ssc;
    ssc << command;
    log(level::debug, "Sent to XSens: %", ssc.str());

    // ... and look for the expected response
    try {
      int repeats = 4;
      data_t response{};
      int response_found = -1;
      do {
        asio::streambuf read_buf;
        size_t bytes_read = port.async_read_some(read_buf.prepare(0x1000), yield);
        read_buf.commit(bytes_read);
        auto buf_begin = asio::buffers_begin(read_buf.data());
        auto buf_end = buf_begin + bytes_read;

        cdata_t data(buf_begin, buf_end);
        std::stringstream ssr;
        ssr << data;
        log(level::debug, "Received from XSens: %", ssr.str());

        response.insert(response.end(), buf_begin, buf_end);

        read_buf.consume(bytes_read);
        response_found = contains_at(response, expected_response);
        int error_found = contains_at(response, command::error_resp);
        if (error_found >= 0) {
          if (error_found + 4 < static_cast<int>(response.size())) {
            log(level::error, "Received Xsens error: %", static_cast<int>(response[error_found + 4]));
            timeout.cancel();
            return false;
          }
        }
      } while (--repeats > 0 && response_found < 0);

      timeout.cancel();

      if (repeats > 0) {
        if (data != nullptr) {
          if (response_found + 3 < static_cast<int>(response.size())) {
            uint8_t size = response[response_found + 3];
            if (response_found + 4 + size < static_cast<int>(response.size())) {
              auto data_start = response.begin() + response_found + 4;
              data->insert(data->end(), data_start, data_start + size);
            }
          }
        }
      }
      else
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
      try {
        auto bytes_read = this->get_port().async_read_some(buf.prepare(512), yield);
        double stamp = get_time();
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
          auto& values = parser_.get_values();
          while (!values.empty()) {
            this->insert_value(stamped_quantity(stamp, values.front()));
            values.pop_front();
          }
        }
      }
      catch (std::exception& e) {
        log(level::error, "Error while polling Xsens: %", e.what());
        this->disconnect();
      }
    }
  }

  bool goto_config(asio::yield_context yield) {
    return exec_command(command::goto_config, command::config_ack, yield);
  }

  bool goto_measurement(asio::yield_context yield) {
    return exec_command(command::goto_measurement, command::measurement_ack, yield);
  }

  virtual bool set_output_configuration(asio::yield_context yield) {
    return true;
  }

  virtual bool set_option_flags(asio::yield_context yield) {
    return true;
  }

  virtual bool reset(asio::yield_context yield) {
    return this->exec_command(command::req_reset, command::reset_ack, yield);
  }


  virtual bool request_product_code(asio::yield_context yield) {
    std::string data;
    bool result = this->exec_command(command::req_product_code, command::product_code_resp, yield, &data);
    if (result) {
      log(level::info, "Product code: %", data);
    }
    return result;
  }

  virtual bool request_identifier(asio::yield_context yield) {
    std::string data;
    bool result = this->exec_command(command::req_device_id, command::device_id_resp, yield, &data);
    if (result && data.size() == 4) {
      std::string serial_no = fmt::format("{:02X}{:02X}{:02X}{:02X}", 
          static_cast<uint8_t>(data[0]), 
          static_cast<uint8_t>(data[1]), 
          static_cast<uint8_t>(data[2]),
          static_cast<uint8_t>(data[3])
      );
      log(level::info, "Device serial#: %", serial_no);
      this->set_id("xsens_" + serial_no);
    }
    return result;
  }

  virtual bool request_firmware(asio::yield_context yield) {
    std::string data;
    bool result = this->exec_command(command::req_firmware_rev, command::firmware_rev_resp, yield, &data);
    if (result && data.size() == 11) {
      uint8_t maj = data[0];
      uint8_t min = data[1]; 
      uint8_t rev = data[2];
      uint32_t build = *reinterpret_cast<uint32_t*>(data.data() + 3);
      uint32_t svnrev = *reinterpret_cast<uint32_t*>(data.data() + 7);
      boost::endian::big_to_native_inplace(build);
      boost::endian::big_to_native_inplace(svnrev);
      log(level::info, fmt::format("Device firmware: {}.{}.{}.{} svn {}", maj, min, rev, build, svnrev));
    }
    return result;
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
        && request_product_code(yield)
        && request_identifier(yield)
        && request_firmware(yield)
        && set_option_flags(yield)
        && set_output_configuration(yield)
        && goto_measurement(yield);


    if (result) {
      log(level::info, "Successfully initialized Xsens device");
      start_polling();
    }
    else {
      log(level::error, "Failed to initialize Xsens device");
      if (reset(yield)) {
        log(level::info, "Successfully reset device");
      }
    }

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
    return this->exec_command(command::set_output_configuration, command::output_configuration_ack, yield);
  }

  bool set_option_flags(asio::yield_context yield) override {
    return this->exec_command(command::set_option_flags, command::option_flags_ack, yield);
  }

};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
