/**
 * \file xsens.h
 * \brief Provide interface to xsens device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2019 Damen Shipyards
 * \license
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

#ifndef XSENS_H_
#define XSENS_H_

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

namespace command {

constexpr byte_t packet_start = 0xFA;
constexpr byte_t sys_command = 0xFF;
constexpr byte_t conf_command = 0x01;


extern cbytes_t option_flags;
extern cbytes_t string_output_type;
extern cbytes_t string_output_type_6;
extern cbytes_t output_configuration;
extern cbytes_t output_configuration_630;
extern cbytes_t error_resp;

constexpr unsigned size_offset = 3;
constexpr unsigned data_offset = 4;

byte_t checksum(cbytes_t data) {
  byte_t result = 0;
  auto i = data.cbegin();
  ++i;
  while (i != data.cend()) {
    result -= *i++;
  }
  return result;
}

Bytes packet_head(cbyte_t mid) {
  Bytes result = { packet_start, sys_command };
  return result << mid;
}

Bytes packet(cbyte_t mid, cbytes_t command=cbytes_t()) {
  Bytes result = packet_head(mid);
  result << static_cast<byte_t>(command.size()) << command;
  result << checksum(result);
  return result;
}

}  // command


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
  void set_flip_axes(const bool value) {
    flip_axes_ = value;
  }
private:
  bool flip_axes_;
  bool parse_single(const double& stamp);
};

} //parser


template <typename Port, typename ContextProvider>
struct Xsens: public Port_device<Port, ContextProvider>,
    public Port_polling_mixin<Xsens<Port, ContextProvider> > {

  template <typename Iterator>
  void handle_data(double stamp, Iterator buf_begin, Iterator buf_end) {
    parser_.add_and_parse(stamp, buf_begin, buf_end);
    auto& values = parser_.get_values();
    while (!values.empty()) {
      this->insert_value(values.front());
      values.pop_front();
    }
  }

  virtual bool look_for_wakeup(asio::yield_context) {
    // Some devices when just powered on, send wakeup messages. Override
    // this method to handle them.
    return true;
  }

  bool do_command(cbyte_t mid, cbyte_t ack, asio::yield_context yield,
        const std::string& message) {
    this->wait(50, yield);
    log(level::info, message);
    return this->exec_command(
        command::packet(mid), 
        command::packet(ack), 
        command::error_resp, yield);
  }

  bool do_set(cbyte_t mid, cbyte_t ack, asio::yield_context yield,
        cbytes_t setting, const std::string& message) {
    this->wait(50, yield);
    log(level::info, message);
    return this->exec_command(
        command::packet(mid, setting),
        command::packet_head(ack), 
        command::error_resp, yield);
  }

  bool do_check(cbyte_t mid, cbyte_t ack, asio::yield_context yield,
        cbytes_t setting, const std::string& message) {
    this->wait(50, yield);
    log(level::info, message);
    return this->exec_command(
        command::packet(mid),
        command::packet(ack, setting), 
        command::error_resp, yield);
  }

  bool do_request(cbyte_t mid, cbyte_t ack, asio::yield_context yield, 
      bytes_t* response, const std::string& message) {
    this->wait(50, yield);
    log(level::info, message);
    return this->exec_command(
        command::packet(mid), 
        command::packet_head(ack), 
        command::error_resp, yield, 
        response);
  }

  bool goto_config(asio::yield_context yield) {
    return do_command(XMID_GotoConfig, XMID_GotoConfigAck, 
        yield, "Xsens GotoConfig");
  }

  bool goto_measurement(asio::yield_context yield) {
    return do_command(XMID_GotoMeasurement, XMID_GotoMeasurementAck,
        yield, "Xsens GotoMeasurement");
  }

  virtual bool check_output_configuration(asio::yield_context) {
    return true;
  }

  virtual bool set_output_configuration(asio::yield_context) {
    return true;
  }

  virtual bool set_option_flags(asio::yield_context) {
    return true;
  }

  virtual bool set_string_output_type(asio::yield_context) {
    return true;
  }

  virtual bool set_filter_profile(asio::yield_context yield) {
    if (filter_profile_ != 0) {
      cbytes_t payload = { 0x0, filter_profile_ };
      return do_set(XMID_SetFilterProfile, XMID_SetFilterProfileAck, 
          yield, payload, fmt::format("Xsens SetFilterProfile: {}", filter_profile_));
    }
    else {
      log(level::info, "Filter profile not configured");
      return true;
    }
  }

  bool reset(asio::yield_context yield) override {
    return do_command(XMID_Reset, XMID_ResetAck, yield, "Xsens Reset");
  }

  bool init_mt(asio::yield_context yield) {
    return do_request(XMID_Initbus, XMID_InitBusResults, yield, 
        nullptr, "Xsens InitMT");
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
    bytes_t response = { command::size_offset };
    bool result = do_request(XMID_ReqProductCode, XMID_ProductCode, yield,
        &response, "Xsens GetProductCode");
    if (result) {
      std::string data = get_string_from_response(response);
      log(level::info, "Product code: %", data);
    }
    return result;
  }

  virtual bool request_identifier(asio::yield_context yield) {
    bytes_t response = { command::size_offset };
    std::string serial_no = "";
    bool result = do_request(XMID_ReqDid, XMID_DeviceId, yield, &response, "Xsens GetIdentifier");
    result &= response.size() > command::size_offset;
    if (result) {
      size_t size = response[command::size_offset];
      if (response.size() > command::size_offset + size) {
        for (size_t i = 0; i < size; ++i) {
          serial_no += fmt::format("{:02X}", static_cast<uint8_t>(response[command::data_offset + i]));
        }
        log(level::info, "Xsens device serial#: %", serial_no);
        this->set_id("xsens_" + serial_no);
      }
      else {
        log(level::warning, "Failed to get Xsens serial#");
        this->set_id("xsens_unknown_serial");
      }
    }
    return result;
  }

  virtual bool request_firmware(asio::yield_context yield) {
    bytes_t response = { command::size_offset };
    bool result = do_request(XMID_ReqFirmwareRevision, XMID_FirmwareRevision, yield,
        &response, "Xsens GetFirmwareVersion");
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
        && set_filter_profile(yield)
        && (check_output_configuration(yield)
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

  void set_options(const prtr::ptree& options) override {
    filter_profile_ = static_cast<byte_t>(options.get("filter_profile", 0));

    bool flip_axes = options.get("flip_axes", get_default_flip_axes());
    parser_.set_flip_axes(flip_axes);
    log(level::info, "Set flip axes: %", flip_axes);
  }

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
    // Decrease clock adjust rate because of high sample frequency of xsens
    set_clock_adjust_rate(0.0001);
  }

  const parser::Xsens_parser& get_parser() const {
    return parser_;
  }

protected:
  virtual bool get_default_flip_axes() {
    return true;
  }

private:
  parser::Xsens_parser parser_;
  byte_t filter_profile_;
};


template <typename Port, class ContextProvider>
struct MTi_GPS_based: public Xsens<Port, ContextProvider> {

  bool check_output_configuration(asio::yield_context yield) override {
    return this->do_check(
        XMID_ReqOutputConfiguration, XMID_ReqOutputConfigurationAck, 
        yield, command::output_configuration, "Xsens ReqOutputConfiguration");
  }


  bool set_output_configuration(asio::yield_context yield) override {
    return this->do_set(
        XMID_SetOutputConfiguration, XMID_SetOutputConfigurationAck,
        yield, command::output_configuration, "Xsens SetOutputConfiguration");
  }


  bool set_option_flags(asio::yield_context yield) override {
    return this->do_set(XMID_SetOptionFlags, XMID_SetOptionFlagsAck,
        yield, command::option_flags, "Xsens SetOptionFlags");
  }


  bool set_string_output_type(asio::yield_context yield) override {
    return this->do_set(XMID_SetStringOutputType, XMID_SetStringOutputTypeAck, 
        yield, command::string_output_type, "Xsens SetStringOutputType");
  }

};


template <typename Port, class ContextProvider>
struct MTi_G_710: public MTi_GPS_based<Port, ContextProvider> {

  MTi_G_710(): MTi_GPS_based<Port, ContextProvider>() {
    log(level::info, "Constructing Xsens_MTi_G_710");
  }


  ~MTi_G_710() override {
    log(level::info, "Destroying Xsens_MTi_G_710");
  }

  bool look_for_wakeup(asio::yield_context yield) override {
    // When the device just powered on, it sends wakeup messages.
    Port& port = this->get_port();
    asio::streambuf read_buf;
    boost::system::error_code ec;
    bytes_t response{};
    log(level::info, "Xsens LookForWakeup");

    // Set a timeout for the command to complete
    asio::deadline_timer timeout_timer(ContextProvider::get_context(), pt::milliseconds(2000));
    timeout_timer.async_wait(
        [&](const boost::system::error_code& error) {
          if (!error)
            port.cancel();
        });

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
      if (contains(response, command::packet(XMID_Wakeup))) {
        log(level::info, "Received WakeUp from XSens: Acknowledging");
        asio::async_write(port, asio::buffer(command::packet(XMID_WakeupAck)), yield);
        // This device will spit out its configuration, which we will just swallow
        port.async_read_some(read_buf.prepare(0x1000), yield[ec]);
        this->wait(500, yield);
      } else {
        this->wait(50, yield);
      }
    }
    // Always return true as we don't really care about how this was handled
    return true;
  }

};


template <typename Port, class ContextProvider>
struct MTi_670: public MTi_GPS_based<Port, ContextProvider> {

  MTi_670(): MTi_GPS_based<Port, ContextProvider>() {
    log(level::info, "Constructing Xsens_MTi_670");
  }


  ~MTi_670() override {
    log(level::info, "Destroying Xsens_MTi_670");
  }

  bool set_string_output_type(asio::yield_context yield) override {
    return this->do_set(XMID_SetStringOutputType, XMID_SetStringOutputTypeAck, 
        yield, command::string_output_type_6, "Xsens SetStringOutputType 670");
  }

protected:
  bool get_default_flip_axes() override {
    // Assume 6XX sensors are mounted up side down, so don't flip
    return false;
  }
};

template <typename Port, class ContextProvider>
struct MTi_630: public Xsens<Port, ContextProvider> {

  MTi_630(): Xsens<Port, ContextProvider>() {
    log(level::info, "Constructing Xsens_MTi_630");
  }


  ~MTi_630() override {
    log(level::info, "Destroying Xsens_MTi_630");
  }

  bool check_output_configuration(asio::yield_context yield) override {
    return this->do_check(
        XMID_ReqOutputConfiguration, XMID_ReqOutputConfigurationAck, 
        yield, command::output_configuration_630, "Xsens ReqOutputConfiguration");
  }

  bool set_option_flags(asio::yield_context yield) override {
    return this->do_set(XMID_SetOptionFlags, XMID_SetOptionFlagsAck,
        yield, command::option_flags, "Xsens SetOptionFlags");
  }


  bool set_output_configuration(asio::yield_context yield) override {
    return this->do_set(
        XMID_SetOutputConfiguration, XMID_SetOutputConfigurationAck,
        yield, command::output_configuration_630, "Xsens SetOutputConfiguration");
  }

  bool set_string_output_type(asio::yield_context yield) override {
    return this->do_set(XMID_SetStringOutputType, XMID_SetStringOutputTypeAck, 
        yield, command::string_output_type_6, "Xsens SetStringOutputType 630");
  }

protected:
  bool get_default_flip_axes() override {
    // Assume 6XX sensors are mounted up side down, so don't flip
    return false;
  }

};

}  // namespace xsens

#endif  // XSENS_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
