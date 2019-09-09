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

#include "../spirit_x3.h"
#include "../types.h"
#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../datetime.h"
#include "../parser.h"

#include <boost/bind.hpp>

#include <xsens/xsxbusmessageid.h>
#include <xsens/xsdataidentifier.h>
#include <xsens/xsdataidentifiervalue.h>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>

namespace xsens {

namespace posix_time = boost::posix_time;

namespace command {

constexpr byte_t packet_start = 0xFA;
constexpr byte_t sys_command = 0xFF;
constexpr byte_t conf_command = 0x01;

extern cbytes_t goto_config;
extern cbytes_t config_ack;

extern cbytes_t goto_measurement;
extern cbytes_t measurement_ack;

extern cbytes_t init_mt;
extern cbytes_t mt_ack;

extern cbytes_t set_option_flags;
extern cbytes_t option_flags_ack;

extern cbytes_t req_reset;
extern cbytes_t reset_ack;

extern cbytes_t wakeup;
extern cbytes_t wakeup_ack;

extern cbytes_t req_product_code;
extern cbytes_t product_code_resp;

extern cbytes_t req_device_id;
extern cbytes_t device_id_resp;

extern cbytes_t req_firmware_rev;
extern cbytes_t firmware_rev_resp;

extern cbytes_t req_utc_time;

extern cbytes_t get_output_configuration;
extern cbytes_t get_output_configuration_ack;
extern cbytes_t set_output_configuration;
extern cbytes_t output_configuration_ack;

extern cbytes_t set_string_output_type;
extern cbytes_t string_output_type_ack;

extern cbytes_t error_resp;

constexpr unsigned size_offset = 3;
constexpr unsigned data_offset = 4;

}


namespace parser {


struct Xsens_parser: public Packet_parser {
  Xsens_parser();
  ~Xsens_parser();
  struct Data_packets;
  struct Data_visitor;
  std::unique_ptr<Data_packets> data_packets;
  std::unique_ptr<Data_visitor> visitor;

  void parse(const double& stamp) override;
  Stamped_queue& get_values() override;
};

} //parser



template <typename Port, typename ContextProvider>
struct Xsens: public Port_device<Port, ContextProvider>, public Polling_mixin<Xsens<Port, ContextProvider> > {

  template <typename Iterator>
  void handle_data(double stamp, Iterator buf_begin, Iterator buf_end) {
    parser_.add_and_parse(stamp, buf_begin, buf_end);
    auto& values = parser_.get_values();
    while (!values.empty()) {
      this->insert_value(values.front());
      values.pop_front();
    }
  }

  bool look_for_wakeup(asio::yield_context yield) {
    // When the device just powered on, it sends wakeup messages.
    Port& port = this->get_port();
    asio::streambuf read_buf;
    boost::system::error_code ec;
    bytes_t response{};
    log(level::info, "Xsens LookForWakeup");
    size_t bytes_read = port.async_read_some(read_buf.prepare(0x100), yield[ec]);
    if (!ec) {
      read_buf.commit(bytes_read);
      auto buf_begin = asio::buffers_begin(read_buf.data());
      auto buf_end = buf_begin + bytes_read;

      cbytes_t data(buf_begin, buf_end);
      std::stringstream ssr;
      ssr << data;
      log(level::debug, "Received from XSens while looking for wakeup: %", ssr.str());

      response.insert(response.end(), buf_begin, buf_end);

      read_buf.consume(bytes_read);
      if (contains(response, command::wakeup)) {
        log(level::info, "Received WakeUp from XSens: Acknowledging");
        asio::async_write(port, asio::buffer(command::wakeup_ack), yield);
        // This device will spit out its configuration, which we will just swallow
        port.async_read_some(read_buf.prepare(0x1000), yield[ec]);
        this->wait(500, yield);
      }
    }
    // Always return true as we don't really care about how this was handled
    return true;
  }

  bool goto_config(asio::yield_context yield) {
    log(level::info, "Xsens GotoConfig");
    return this->exec_command(command::goto_config, command::config_ack, command::error_resp, yield);
  }

  bool goto_measurement(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GotoMeasurement");
    return this->exec_command(command::goto_measurement, command::measurement_ack, command::error_resp, yield);
  }

  virtual bool get_output_configuration(asio::yield_context yield) {
    return true;
  }

  virtual bool set_output_configuration(asio::yield_context yield) {
    return true;
  }

  virtual bool set_option_flags(asio::yield_context yield) {
    return true;
  }

  virtual bool set_string_output_type(asio::yield_context yield) {
    return true;
  }

  bool reset(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens Reset");
    return this->exec_command(command::req_reset, command::reset_ack, command::error_resp, yield);
  }

  bool init_mt(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens InitMT");
    return this->exec_command(command::init_mt, command::mt_ack, command::error_resp, yield);
  }

  std::string get_string_from_response(cbytes_t response) {
    std::string result;
    if (response.size() > command::size_offset) {
      int size = response[command::size_offset];
      if (response.size() > (size + command::data_offset)) {
        auto data_start = response.begin() + command::data_offset;
        result.insert(result.end(), data_start, data_start + size);
      }
    }
    return result;
  }

  virtual bool request_product_code(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GetProductCode");
    bytes_t response;
    bool result = this->exec_command(command::req_product_code, command::product_code_resp, command::error_resp, yield, &response);
    if (result) {
      std::string data = get_string_from_response(response);
      log(level::info, "Product code: %", data);
    }
    return result;
  }

  virtual bool request_identifier(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GetIdentifier");
    bytes_t response;
    bool result = this->exec_command(command::req_device_id, command::device_id_resp, command::error_resp, yield, &response);
    if (result && response.size() >= (command::data_offset + 4)) {
      std::string serial_no = fmt::format("{:02X}{:02X}{:02X}{:02X}",
          static_cast<uint8_t>(response[command::data_offset + 0]),
          static_cast<uint8_t>(response[command::data_offset + 1]),
          static_cast<uint8_t>(response[command::data_offset + 2]),
          static_cast<uint8_t>(response[command::data_offset + 3])
      );
      log(level::info, "Xsens device serial#: %", serial_no);
      this->set_id("xsens_" + serial_no);
    }
    return result;
  }

  virtual bool request_firmware(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GetFirmwareVersion");

    bytes_t response;
    bool result = this->exec_command(command::req_firmware_rev, command::firmware_rev_resp, command::error_resp, yield, &response);
    if (result && response.size() >= (command::data_offset + 11)) {
      uint8_t maj = response[command::data_offset + 0];
      uint8_t min = response[command::data_offset + 1];
      uint8_t rev = response[command::data_offset + 2];
      uint32_t build = *reinterpret_cast<uint32_t*>(response.data() + command::data_offset + 3);
      uint32_t svnrev = *reinterpret_cast<uint32_t*>(response.data() + command::data_offset + 7);
      boost::endian::big_to_native_inplace(build);
      boost::endian::big_to_native_inplace(svnrev);
      log(level::info, fmt::format("Device firmware: {}.{}.{}.{} svn {}", maj, min, rev, build, svnrev));
    }
    return result;
  }

  bool initialize(asio::yield_context yield) override {
    bool result =
        look_for_wakeup(yield)
        && goto_config(yield)
        && request_identifier(yield)
        && request_product_code(yield)
        && request_firmware(yield)
        && set_option_flags(yield)
        && set_string_output_type(yield)
        && (get_output_configuration(yield)
            || (set_output_configuration(yield) && init_mt(yield)))
        && goto_measurement(yield);


    if (result) {
      log(level::info, "Successfully initialized Xsens device");
      this->start_polling();
    }
    else {
      log(level::error, "Failed to initialize Xsens device");
      if (reset(yield)) {
        log(level::info, "Successfully reset Xsens device");
      }
    }

    return result;
  }

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
    // Decrease clock adjust rate because of high sample frequency of xsens
    set_clock_adjust_rate(0.0001);
  }

  const parser::Xsens_parser& get_parser() const {
    return parser_;
  }
private:
  parser::Xsens_parser parser_;
};


template <typename Port, class ContextProvider>
struct MTi_G_710: public Xsens<Port, ContextProvider> {

  MTi_G_710(): Xsens<Port, ContextProvider>() {
    log(level::info, "Constructing Xsens_MTi_G_710");
  }


  ~MTi_G_710() override {
    log(level::info, "Destroying Xsens_MTi_G_710");
  }


  bool get_output_configuration(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens GetOutputConfiguration");
    return this->exec_command(command::get_output_configuration, 
        command::get_output_configuration_ack, command::error_resp, yield);
  }


  bool set_output_configuration(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetOutputConfiguration");
    return this->exec_command(command::set_output_configuration, 
        command::output_configuration_ack, command::error_resp, yield);
  }


  bool set_option_flags(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetOptionFlags");
    return this->exec_command(command::set_option_flags, 
        command::option_flags_ack, command::error_resp, yield);
  }


  bool set_string_output_type(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetStringOutputType");
    return this->exec_command(command::set_string_output_type, 
        command::string_output_type_ack, command::error_resp, yield);
  }

};

}  // namespace xsens

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
