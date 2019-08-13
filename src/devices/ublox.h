/**
 * \file ublox.h
 * \brief Provide interface to ublox device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../spirit_x3.h"
#include "../datetime.h"
#include "../types.h"

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>


namespace ubx {


namespace command {

extern cbytes_t cfg_prt_usb;
extern cbytes_t cfg_prt_uart;
extern cbytes_t mon_ver;
extern cbytes_t cfg_pms;
extern cbytes_t cfg_hnr;
extern cbytes_t cfg_rate;
extern cbytes_t cfg_nav5;
extern cbytes_t cfg_gnss_glonass;
extern cbytes_t cfg_gnss_galileo;
extern cbytes_t cfg_gnss_beidou;

extern cbytes_t cfg_msg_nav_pvt;
extern cbytes_t cfg_msg_nav_att;
extern cbytes_t cfg_msg_esf_ins;
extern cbytes_t cfg_msg_esf_raw;

constexpr unsigned size_offset = 4;
constexpr unsigned data_offset = 6;

}  // namespace command

namespace response {

extern cbytes_t ack;
extern cbytes_t nak;
extern cbytes_t mon_ver;

}  // namespace response


namespace parser {

namespace x3 = boost::spirit::x3;

using Values_type = std::vector<Quantity_value>;
using Values_queue = std::deque<Quantity_value>;

struct Packet_parser {
  Packet_parser() {};
  ~Packet_parser() {};
};

} //  namespace parser



template <typename Port, typename ContextProvider>
struct Ublox: public Port_device<Port, ContextProvider>, public Polling_mixin<Ublox<Port, ContextProvider> > {

  template <typename Iterator>
  void handle_data(double stamp, Iterator buf_begin, Iterator buf_end) {
    parser_.parse(buf_begin, buf_end);
    auto& values = parser_.get_values();
    while (!values.empty()) {
      this->insert_value(stamped_quantity(stamp, values.front()));
      values.pop_front();
    }
  }

  bool initialize(asio::yield_context yield) override {
    bool result =
        setup_ports(yield)
        && get_version(yield)
        && setup_power_management(yield)
        && setup_gnss(yield)
        && setup_navigation_rate(yield)
        && setup_messages(yield);

    if (result) {
      log(level::info, "Successfully initialized Ublox device");
      this->start_polling();
    }
    else {
      log(level::error, "Failed to initialize Ublox device");
    }

    return result;
  }

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
  }

  const parser::Packet_parser& get_parser() const {
    return parser_;
  }

  virtual bool setup_ports(asio::yield_context yield) = 0;
  virtual bool get_version(asio::yield_context yield) = 0;
  virtual bool setup_power_management(asio::yield_context yield) = 0;
  virtual bool setup_gnss(asio::yield_context yield) = 0;
  virtual bool setup_navigation_rate(asio::yield_context yield) = 0;
  virtual bool setup_messages(asio::yield_context yield) = 0;

private:
  parser::Packet_parser parser_;
};


template <typename Port, typename ContextProvider>
struct NEO_M8U: public Ublox<Port, ContextProvider> {

  NEO_M8U(): Ublox<Port, ContextProvider>() {
    log(level::info, "Constructing Ublox_NEO_M8U");
  }

  ~NEO_M8U() override {
    log(level::info, "Destroying Ublox_NEO_M8U");
  }

  bool setup_ports(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup ports");
    return this->exec_command(command::cfg_prt_usb, response::ack, response::nak, yield) 
        && this->exec_command(command::cfg_prt_uart, response::ack, response::nak, yield);
  }

  bool get_version(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U get version info");
    // Prime the reponse with offset of response length offset in the received data
    bytes_t response = { command::size_offset, command::size_offset + 1 };
    bool result = this->exec_command(command::mon_ver, response::mon_ver, response::nak, yield, &response);
    if (result) {
      size_t len = response.size() - command::data_offset;
      constexpr size_t sw_len = 30;
      constexpr size_t hw_len = 10;
      constexpr size_t ext_len = 30;
      if (len > sw_len) {
        std::string sw_version;
        auto read_it = response.begin() + command::data_offset;
        sw_version.insert(sw_version.end(), read_it, read_it + sw_len);
        boost::trim_right_if(sw_version, [](char c) { return c < 0x21; });
        log(level::info, "Ublox NEO M8U software version: %", sw_version);
        read_it += sw_len;
        len -= sw_len;
        if (len > hw_len) {
          std::string hw_version;
          hw_version.insert(hw_version.end(), read_it, read_it + hw_len);
          boost::trim_right_if(hw_version, [](char c) { return c < 0x21; });
          log(level::info, "Ublox NEO M8U hardware version: %", hw_version);
          read_it += hw_len;
          len -= hw_len;
          while (len > ext_len) {
            std::string extension;
            extension.insert(extension.end(), read_it, read_it + ext_len);
            boost::trim_right_if(extension, [](char c) { return c < 0x21; });
            log(level::info, "Ublox NEO M8U version extension: %", extension);
            read_it += ext_len;
            len -= ext_len;
          }
        }
      }
    }
    return result;
  }

  bool setup_power_management(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup power management");
    return this->exec_command(command::cfg_pms, response::ack, response::nak, yield);
  }

  bool setup_gnss(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup GNSS");
    return this->exec_command(command::cfg_nav5, response::ack, response::nak, yield) 
        && use_glonass(yield);  // Use GPS + Glonass by default
  }

  bool setup_navigation_rate(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup navigation rate");
    return this->exec_command(command::cfg_rate, response::ack, response::nak, yield) 
        && this->exec_command(command::cfg_hnr, response::ack, response::nak, yield);
  }

  bool setup_messages(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup messages");
    return this->exec_command(command::cfg_msg_nav_pvt, response::ack, response::nak, yield) 
        && this->exec_command(command::cfg_msg_nav_att, response::ack, response::nak, yield)
        && this->exec_command(command::cfg_msg_esf_ins, response::ack, response::nak, yield)
        && this->exec_command(command::cfg_msg_esf_raw, response::ack, response::nak, yield);
  }

  bool use_glonass(asio::yield_context yield) {
    log(level::info, "Ublox NEO M8U use GLONASS");
    return this->exec_command(command::cfg_gnss_glonass, response::ack, response::nak, yield);
  }

  bool use_galileo(asio::yield_context yield) {
    log(level::info, "Ublox NEO M8U use Galileo");
    return this->exec_command(command::cfg_gnss_galileo, response::ack, response::nak, yield);
  }

  bool use_beidou(asio::yield_context yield) {
    log(level::info, "Ublox NEO M8U use Beidou");
    return this->exec_command(command::cfg_gnss_beidou, response::ack, response::nak, yield);
  }
};

}  //namespace ubx

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
