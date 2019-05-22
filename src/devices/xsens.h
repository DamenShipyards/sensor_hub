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
#include "../usb.h"
#include "../tools.h"
#include "../datetime.h"

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


namespace command {

extern cbyte_t packet_start;
extern cbyte_t sys_command;
extern cbyte_t conf_command;

extern cdata_t goto_config;
extern cdata_t config_ack;

extern cdata_t goto_measurement;
extern cdata_t measurement_ack;

extern cdata_t init_mt;
extern cdata_t mt_ack;

extern cdata_t set_option_flags;
extern cdata_t option_flags_ack;

extern cdata_t req_reset;
extern cdata_t reset_ack;

extern cdata_t wakeup;
extern cdata_t wakeup_ack;

extern cdata_t req_product_code;
extern cdata_t product_code_resp;

extern cdata_t req_device_id;
extern cdata_t device_id_resp;

extern cdata_t req_firmware_rev;
extern cdata_t firmware_rev_resp;

extern cdata_t req_utc_time;

extern cdata_t get_output_configuration;
extern cdata_t get_output_configuration_ack;
extern cdata_t set_output_configuration;
extern cdata_t output_configuration_ack;

extern cdata_t set_string_output_type;
extern cdata_t string_output_type_ack;

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

  void wait(int milli_seconds, asio::yield_context yield) {
    asio::deadline_timer waiter(ContextProvider::get_context(), posix_time::milliseconds(milli_seconds));
    waiter.async_wait(yield);
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

  bool look_for_wakeup(asio::yield_context yield) {
    // When the device just powered on, it sends wakeup messages.
    Port& port = this->get_port();
    asio::streambuf read_buf;
    boost::system::error_code ec;
    data_t response{};
    log(level::info, "Xsens LookForWakeup");
    size_t bytes_read = port.async_read_some(read_buf.prepare(0x100), yield[ec]);
    if (!ec) {
      read_buf.commit(bytes_read);
      auto buf_begin = asio::buffers_begin(read_buf.data());
      auto buf_end = buf_begin + bytes_read;

      cdata_t data(buf_begin, buf_end);
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
        wait(500, yield);
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


  virtual bool request_product_code(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GetProductCode");
    std::string data;
    bool result = this->exec_command(command::req_product_code, command::product_code_resp, command::error_resp, yield, &data);
    if (result) {
      log(level::info, "Product code: %", data);
    }
    return result;
  }

  virtual bool request_identifier(asio::yield_context yield) {
    this->wait(50, yield);
    log(level::info, "Xsens GetIdentifier");
    std::string data;
    bool result = this->exec_command(command::req_device_id, command::device_id_resp, command::error_resp, yield, &data);
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
    this->wait(50, yield);
    log(level::info, "Xsens GetFirmwareVersion");

    std::string data;
    bool result = this->exec_command(command::req_firmware_rev, command::firmware_rev_resp, command::error_resp, yield, &data);
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

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
    // Decrease clock adjust rate because of high sample frequency of xsens
    set_clock_adjust_rate(0.0001);
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

  bool get_output_configuration(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens GetOutputConfiguration");
    return this->exec_command(command::get_output_configuration, command::get_output_configuration_ack, command::error_resp, yield);
  }

  bool set_output_configuration(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetOutputConfiguration");
    return this->exec_command(command::set_output_configuration, command::output_configuration_ack, command::error_resp, yield);
  }

  bool set_option_flags(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetOptionFlags");
    return this->exec_command(command::set_option_flags, command::option_flags_ack, command::error_resp, yield);
  }

  bool set_string_output_type(asio::yield_context yield) override {
    this->wait(50, yield);
    log(level::info, "Xsens SetStringOutputType");
    return this->exec_command(command::set_string_output_type, command::string_output_type_ack, command::error_resp, yield);
  }
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2