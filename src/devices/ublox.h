/**
 * \file ublox.h
 * \brief Provide interface to ublox device class
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


#ifndef UBLOX_H_
#define UBLOX_H_

#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../spirit_x3.h"
#include "../datetime.h"
#include "../types.h"
#include "../parser.h"

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>
#include <type_traits>


namespace ubx {

namespace command {

cbyte_t sync_1 = 0xB5;
cbyte_t sync_2 = 0x62;
cbytes_t preamble = { sync_1, sync_2 };

cbyte_t cls_nav = 0x01;
namespace nav {
  cbyte_t posllh = 0x02;
  cbyte_t status = 0x03;
  cbyte_t dop = 0x04;
  cbyte_t att = 0x05;
  cbyte_t sol = 0x06;
  cbyte_t pvt = 0x07;
  cbyte_t velned = 0x12;
  cbyte_t clock = 0x22;
  cbyte_t dgps = 0x31;
  cbyte_t sbas = 0x32;
}  // namespace nav

cbyte_t cls_ack = 0x05;
namespace ack {
  cbyte_t nak = 0x00;
  cbyte_t ack = 0x01;
}  // namespace ack

cbyte_t cls_cfg = 0x06;

namespace cfg {
  cbyte_t prt = 0x00;
  cbytes_t prt_payload_uart = {
    0x01,  // PortID: 3 -> UART
    0x00,  // Reserved
    0x00, 0x00,  // txReady (not interested)
    0xC0, 0x08, 0x00, 0x00,  // Serial port mode: 8,none,1
    0x00, 0xC2, 0x01, 0x00,  // Serial port baudrate: 115200 baud
    0x00, 0x00,  // InProtoMask: 0 -> Disable all on UART
    0x00, 0x00,  // OutProtoMask: 0 -> Disbale all on UART
    0x00, 0x00,  // Flags: some timeout we're not interested in
    0x00, 0x00,  // Reserved
  };
  cbytes_t prt_payload_usb = {
    0x03,  // PortID: 3 -> USB
    0x00,  // Reserved
    0x00, 0x00,  // txReady (not interested)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Reserved
    0x01, 0x00,  // InProtoMask: 1 -> UBX only
    0x01, 0x00,  // OutProtoMask: 1 -> UBX only
    0x00, 0x00, 0x00, 0x00,  // Reserved
  };

  cbyte_t msg = 0x01;

  cbyte_t rate = 0x08;
  cbytes_t rate_payload = {
    0xF4, 0x01, // MeasRate: 500ms -> 2Hz
    0x01, 0x00, // Output message every measurement
    0x00, 0x00, // TimeRef: UTC
  };

  cbyte_t nav5 = 0x24;
  cbytes_t nav5_payload = {
    0x47, 0x04,   // Parameters flag: dyn,el,fix; static; utc
    0x05,  // DynMode: 5 -> Sea
    0x03,  // FixMode: 3 -> 2D and 3D
    0x00, 0x00, 0x00, 0x00,  // FixedAlt: 0 -> 2D altitude
    0xFF, 0xFF, 0x00, 0x00,  // FixedAltVar: 2D altitude variance (quoi?)
    0x0A,  // MinElev: 10 -> 10 degree minimum sat elevation
    0x00,  // Reserved
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Position mask: don't care
    0x00,  // StaticHoldThresh: 0 -> Disable static navigation
    0x00,  // DGNNSTimeOut: 0 -> don't care
    0x00,  // CnoThreshNumSVs: don't care
    0x00,  // CnoThresh: don't care
    0x00, 0x00,  // Reserved
    0x00, 0x00,  // StaticHoldMaxDist: 0 -> Disable static navigation
    0x00,  // UtcStandard: 0 -> Auto
    0x00, 0x00, 0x00, 0x00, 0x00, // Reserved
  };


  cbyte_t gnss = 0x3E;
  cbytes_t gnss_payload = {
    0x00,  // Version
    0x00,  // Number of tracking channels in device (read only)
    0xFF,  // Number of tracking channels used (all)
    0x07,  // Number of configuration blocks
  };
  cbytes_t gnss_payload_gps = {
    0x00,  // GnssId: GPS
    0x08,  // Min channels
    0x10,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };
  cbytes_t gnss_payload_sbas = {
    0x01,  // GnssId: SBAS
    0x01,  // Min channels
    0x03,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };
  cbytes_t gnss_payload_galileo = {
    0x02,  // GnssId: Galileo
    0x04,  // Min channels
    0x08,  // Max channels
    0x00,  // Reserved
    0x00, 0x00, 0x01, 0x00,  // Flags: disabled + L1
  };
  cbytes_t gnss_payload_galileo_on = {
    0x02,  // GnssId: Galileo
    0x04,  // Min channels
    0x08,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };
  cbytes_t gnss_payload_beidou = {
    0x03,  // GnssId: Beidou
    0x08,  // Min channels
    0x10,  // Max channels
    0x00,  // Reserved
    0x00, 0x00, 0x01, 0x00,  // Flags: disabled + L1
  };
  cbytes_t gnss_payload_beidou_on = {
    0x03,  // GnssId: Beidou
    0x08,  // Min channels
    0x10,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };
  cbytes_t gnss_payload_imes = {
    0x04,  // GnssId: IMES
    0x00,  // Min channels
    0x00,  // Max channels
    0x00,  // Reserved
    0x00, 0x00, 0x01, 0x00,  // Flags: disabled + L1
  };
  cbytes_t gnss_payload_qzss = {
    0x05,  // GnssId: QZSS
    0x00,  // Min channels
    0x03,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };
  cbytes_t gnss_payload_glonass = {
    0x06,  // GnssId: Glonass
    0x08,  // Min channels
    0x0E,  // Max channels
    0x00,  // Reserved
    0x00, 0x00, 0x01, 0x00,  // Flags: disabled + L1
  };
  cbytes_t gnss_payload_glonass_on = {
    0x06,  // GnssId: Glonass
    0x08,  // Min channels
    0x0E,  // Max channels
    0x00,  // Reserved
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };


  cbyte_t hnr = 0x5C;
  cbytes_t hnr_payload = {
    0x0A,  // 10Hz
    0x00, 0x00, 0x00,  // Reserved
  };

  cbyte_t pms = 0x86;
  cbytes_t pms_payload = {
    0x00,  // Version
    0x00,  // PowerSetupValue: Full power
    0x00, 0x00, // Period: must be zero unless PowerSetupValue is "interval"
    0x00, 0x00, // OnTime: must be zero unless PowerSetupValue is "interval"
    0x00, 0x00, // Reserved
  };

}  // namespace cfg


cbyte_t cls_mon = 0x0A;
namespace mon {
  cbyte_t ver = 0x04;
  cbyte_t gnss = 0x28;
}  // namespace mon

cbyte_t cls_esf = 0x10;
namespace esf {
  cbyte_t meas = 0x02;
  cbyte_t raw = 0x03;
  cbyte_t status = 0x10;
  cbyte_t ins = 0x15;
}  // namespace esf


cbyte_t cls_sec = 0x27;
namespace sec {
  cbyte_t uniqid = 0x03;
}  // namespace sec

constexpr uint8_t size_offset = 4;
constexpr uint8_t data_offset = 6;

}  // namespace command

namespace parser {

struct Data_packet {
  Data_packet(): data_(), checksum_() {
    set_length(0);
  }

  Data_packet(const Data_packet& packet): data_(packet.data_), checksum_(packet.checksum_) {}

  Data_packet(const byte_t cls, const byte_t id): data_(), checksum_() {
    setup_payload(cls, id, {});
  }

  Data_packet(const byte_t cls, const byte_t id, cbytes_t& payload): data_(), checksum_() {
    setup_payload(cls, id, payload);
  }

  Data_packet(const byte_t cls, const byte_t id, const std::initializer_list<byte_t> payload_init):
      data_(), checksum_() {
    setup_payload(cls, id, payload_init);
  }

  Data_packet& operator=(const Data_packet& packet) {
    data_ = packet.data_;
    checksum_ = packet.checksum_;
    return *this;
  }

  uint8_t get_cls() const {
    return get_data_byte(0);
  }

  uint8_t get_id() const {
    return get_data_byte(1);
  }

  bytes_t& get_data() {
    return data_;
  }

  bytes_t get_packet() {
    return command::preamble << data_ << checksum_;
  }

  bool check() {
    return (get_length() == calc_length()) && (get_checksum() == calc_checksum());
  }

  uint16_t get_length() const {
    return get_data_byte(2) + (get_data_byte(3) << 8);
  }

  uint16_t get_checksum() const {
    return checksum_;
  }


  Data_packet& set_cls(cbyte_t cls) {
    set_data_byte(0, cls);
    return *this;
  }

  Data_packet& set_id(cbyte_t id) {
    set_data_byte(1, id);
    return *this;
  }

  Data_packet& set_length(const uint16_t length) {
    set_data_byte(2, length & 0xFF);
    set_data_byte(3, length >> 8);
    return *this;
  }

  Data_packet& add_data(const uint8_t value) {
    data_.push_back(value);
    return *this;
  }

  Data_packet& set_checksum(const uint16_t checksum) {
    checksum_ = checksum;
    return *this;
  }

  Data_packet& clear() {
    data_.clear();
    checksum_ = 0;
    set_length(0);
    return *this;
  }

  uint16_t calc_length() {
    if (data_.size() > 3) {
      return static_cast<uint16_t>(data_.size() - 4);
    }
    else {
      return 0;
    }
  }

  uint16_t calc_checksum() {
    byte_t chk_a = 0;
    byte_t chk_b = 0;
    auto add_byte = [&](const byte_t byte) {
      chk_a += byte;
      chk_b += chk_a;
    };
    std::for_each(data_.begin(), data_.end(), add_byte);
    return (chk_b << 8) + chk_a;
  }
private:
  bytes_t data_;
  uint16_t checksum_;

  void setup_payload(const byte_t cls, const byte_t id, const std::initializer_list<byte_t> payload_init) {
    cbytes_t payload{payload_init};
    setup_payload(cls, id, payload);
  }

  void setup_payload(const byte_t cls, const byte_t id, cbytes_t& payload) {
    data_.push_back(cls);
    data_.push_back(id);
    set_length(static_cast<uint16_t>(payload.size()));
    data_.insert(data_.end(), payload.begin(), payload.end());
    checksum_ = calc_checksum();
  }

  uint8_t get_data_byte(const size_t index) const {
    return index < data_.size() ? data_[index] : 0;
  }

  void set_data_byte(const size_t index, cbyte_t value) {
    while (data_.size() <= index) {
      data_.push_back(0);
    }
    data_[index] = value;
  }

};


struct Ublox_parser: public Packet_parser {
  Ublox_parser();
  ~Ublox_parser();
  struct Payload_visitor;
  std::unique_ptr<Payload_visitor> visitor;

  void parse(const double& stamp) override;
  Stamped_queue& get_values() override; 
};

}  // namespace parser


namespace command {

cbytes_t cfg_prt_usb = parser::Data_packet(cls_cfg, cfg::prt, cfg::prt_payload_usb).get_packet();
cbytes_t cfg_prt_uart = parser::Data_packet(cls_cfg, cfg::prt, cfg::prt_payload_uart).get_packet();
cbytes_t mon_ver = parser::Data_packet(cls_mon, mon::ver, {}).get_packet();
cbytes_t cfg_pms = parser::Data_packet(cls_cfg, cfg::pms, cfg::pms_payload).get_packet();
cbytes_t cfg_hnr = parser::Data_packet(cls_cfg, cfg::hnr, cfg::hnr_payload).get_packet();
cbytes_t cfg_rate = parser::Data_packet(cls_cfg, cfg::rate, cfg::rate_payload).get_packet();
cbytes_t cfg_nav5 = parser::Data_packet(cls_cfg, cfg::nav5, cfg::nav5_payload).get_packet();

cbytes_t gnss_glonass_payload = cfg::gnss_payload
    << cfg::gnss_payload_gps
    << cfg::gnss_payload_sbas
    << cfg::gnss_payload_galileo
    << cfg::gnss_payload_beidou
    << cfg::gnss_payload_imes
    << cfg::gnss_payload_qzss
    << cfg::gnss_payload_glonass_on;

cbytes_t gnss_galileo_payload = cfg::gnss_payload
    << cfg::gnss_payload_gps
    << cfg::gnss_payload_sbas
    << cfg::gnss_payload_galileo_on
    << cfg::gnss_payload_beidou
    << cfg::gnss_payload_imes
    << cfg::gnss_payload_qzss
    << cfg::gnss_payload_glonass;

cbytes_t gnss_beidou_payload = cfg::gnss_payload
    << cfg::gnss_payload_gps
    << cfg::gnss_payload_sbas
    << cfg::gnss_payload_galileo
    << cfg::gnss_payload_beidou_on
    << cfg::gnss_payload_imes
    << cfg::gnss_payload_qzss
    << cfg::gnss_payload_glonass;
cbytes_t cfg_gnss_glonass = parser::Data_packet(cls_cfg, cfg::gnss, gnss_glonass_payload).get_packet();
cbytes_t cfg_gnss_galileo = parser::Data_packet(cls_cfg, cfg::gnss, gnss_galileo_payload).get_packet();
cbytes_t cfg_gnss_beidou = parser::Data_packet(cls_cfg, cfg::gnss, gnss_beidou_payload).get_packet();

cbytes_t cfg_msg_nav_pvt = parser::Data_packet(cls_cfg, cfg::msg, { cls_nav, nav::pvt, 0x01 }).get_packet();
cbytes_t cfg_msg_nav_att = parser::Data_packet(cls_cfg, cfg::msg, { cls_nav, nav::att, 0x01 }).get_packet();
cbytes_t cfg_msg_esf_ins = parser::Data_packet(cls_cfg, cfg::msg, { cls_esf, esf::ins, 0x01 }).get_packet();
cbytes_t cfg_msg_esf_raw = parser::Data_packet(cls_cfg, cfg::msg, { cls_esf, esf::raw, 0x0A }).get_packet();

cbytes_t sec_uniqid = parser::Data_packet(cls_sec, sec::uniqid, {}).get_packet();

}  // namespace command


namespace response {

cbytes_t ack = { command::sync_1, command::sync_2, command::cls_ack, command::ack::ack, 0x02, 0x00, command::cls_cfg };
cbytes_t nak = { command::sync_1, command::sync_2, command::cls_ack, command::ack::nak, 0x02, 0x00, command::cls_cfg };
cbytes_t mon_ver = { command::sync_1, command::sync_2, command::cls_mon, command::mon::ver };
cbytes_t sec_uniqid = { command::sync_1, command::sync_2, command::cls_sec, command::sec::uniqid };

}  // namespace response


template <typename Port, typename ContextProvider>
struct Ublox: public Port_device<Port, ContextProvider>, public Polling_mixin<Ublox<Port, ContextProvider> > {

  template <typename Iterator>
  void handle_data(double stamp, Iterator buf_begin, Iterator buf_end) {
    parser_.add_and_parse(stamp, buf_begin, buf_end);
    auto& values = parser_.get_values();
    while (!values.empty()) {
      this->insert_value(values.front());
      values.pop_front();
    }
  }

  bool initialize(asio::yield_context yield) override {
    bool result =
        setup_ports(yield)
        && request_version(yield)
        && request_id(yield)
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

  const parser::Ublox_parser& get_parser() const {
    return parser_;
  }

  virtual bool setup_ports(asio::yield_context yield) = 0;
  virtual bool request_version(asio::yield_context yield) = 0;
  virtual bool request_id(asio::yield_context yield) = 0;
  virtual bool setup_power_management(asio::yield_context yield) = 0;
  virtual bool setup_gnss(asio::yield_context yield) = 0;
  virtual bool setup_navigation_rate(asio::yield_context yield) = 0;
  virtual bool setup_messages(asio::yield_context yield) = 0;

private:
  parser::Ublox_parser parser_;
};


template <typename Port, typename ContextProvider>
struct NEO_M8U: public Ublox<Port, ContextProvider> {

  enum dyn_model {  
    portable,
    unused,
    stationary,
    pedestrian,
    automotive,
    sea,
    airborne_1g,
    airborne_2g,
    airborne_4g,
    wrist_watch,
    bike,
  };

  enum gnss_type {
    glonass,
    galileo,
    beidou,
  };


  NEO_M8U(): Ublox<Port, ContextProvider>(), dyn_model_(portable), gnss_type_(glonass) {
    log(level::info, "Constructing Ublox_NEO_M8U");
  }

  ~NEO_M8U() override {
    log(level::info, "Destroying Ublox_NEO_M8U");
  }


  void set_options(const prtr::ptree& options) {
    std::string s = options.get("dyn_model", "portable");
    dyn_model_ = 
      s == "portable" ? portable :
      s == "stationary" ? stationary :
      s == "pedestrian" ? pedestrian :
      s == "automotive" ? automotive :
      s == "sea" ? sea :
      s == "airborne_1g" ? airborne_1g :
      s == "airborne_2g" ? airborne_2g :
      s == "airborne_4g" ? airborne_4g :
      s == "wrist_watch" ? wrist_watch :
      s == "bike" ? bike : portable;
    s = options.get("gnss_type", "glonass");
    gnss_type_ =
      s == "glonass" ? glonass :
      s == "galileo" ? galileo :
      s == "beidou" ? beidou : glonass;
  }


  bool setup_ports(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup ports");
    return this->exec_command(command::cfg_prt_usb, response::ack, response::nak, yield) 
        && this->exec_command(command::cfg_prt_uart, response::ack, response::nak, yield);
  }


  bool request_version(asio::yield_context yield) override {
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


  bool request_id(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U get unique identifier");
    // Prime the reponse with offset of response length offset in the received data
    bytes_t response = { command::size_offset, command::size_offset + 1 };
    constexpr size_t min_len = 9;
    constexpr size_t id_offset = 4;
    bool result = this->exec_command(command::sec_uniqid, response::sec_uniqid, response::nak, yield, &response);
    if (result && (response.size() - command::data_offset) >= min_len) {
      std::string serial_no = fmt::format("{:02X}{:02X}{:02X}{:02X}{:02X}",
          static_cast<uint8_t>(response[command::data_offset + id_offset + 0]),
          static_cast<uint8_t>(response[command::data_offset + id_offset + 1]),
          static_cast<uint8_t>(response[command::data_offset + id_offset + 2]),
          static_cast<uint8_t>(response[command::data_offset + id_offset + 3]),
          static_cast<uint8_t>(response[command::data_offset + id_offset + 4])
          );
      log(level::info, "Ublox device serial#: %", serial_no);
      this->set_id("ublox_" + serial_no);
    }
    return result;
  }


  bool setup_power_management(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup power management");
    return this->exec_command(command::cfg_pms, response::ack, response::nak, yield);
  }


  bool setup_gnss(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup GNSS");
    bytes_t payload = command::cfg::nav5_payload;
    payload[2] = static_cast<byte_t>(dyn_model_);
    log(level::info, "Ublox NEO M8U dynamic model: %", static_cast<int>(dyn_model_));
    bytes_t cfg_nav5 = parser::Data_packet(command::cls_cfg, command::cfg::nav5, payload).get_packet();
    return this->exec_command(cfg_nav5, response::ack, response::nak, yield) 
        && gnss_type_ == glonass ? use_glonass(yield):
           gnss_type_ == galileo ? use_galileo(yield):
           gnss_type_ == beidou ? use_beidou(yield): false;  // Use GPS + Glonass by default
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

private:
  dyn_model dyn_model_;
  gnss_type gnss_type_;

};


}  //namespace ubx

#endif  // ifndef UBLOX_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
