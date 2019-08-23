/**
 * \file ublox_impl.h
 * \brief Implementation details for u-blox device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#ifndef XSENS_IMPL_H_
#define XSENS_IMPL_H_

#include "ublox.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


namespace ubx {

namespace gregorian = boost::gregorian;

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
    0xFA, 0x00, // MeasRate: 250ms -> 4Hz
    0x02, 0x00, // NavRate: 2 (1 solution per 2 measurements) -> 2Hz output
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



}  // namespace command

namespace parser {

using x3::repeat;
using x3::omit;
using x3::eps;

using x3::byte_;
using x3::little_word;
using x3::little_dword;
using x3::little_bin_float;
using x3::little_bin_double;

using x3::_attr;
using x3::_val;
using x3::_pass;


struct Data_packet {
  Data_packet(): cls_(), id_(), length_(), checksum_(), payload_() {}
  Data_packet(const byte_t cls, const byte_t id): cls_(cls), id_(id), length_(), payload_() {
    checksum_ = get_checksum();
  }
  Data_packet(const byte_t cls, const byte_t id, cbytes_t payload):
      cls_(cls), id_(id), payload_(payload) {
    length_ = get_length();
    checksum_ = get_checksum();
  }
  Data_packet(const byte_t cls, const byte_t id, const std::initializer_list<byte_t> payload):
      cls_(cls), id_(id), payload_(payload) {
    length_ = get_length();
    checksum_ = get_checksum();
  }
  Data_packet(cbytes_t data): cls_(), id_(), length_(), payload_() {
    auto l = data.size();
    if (l > 0)
      cls_ = data[0];
    if (l > 1)
      id_ = data[1];
    if (l > 3)
      length_ = data[2] + (data[3] << 8);
    for (auto i = 0; i < length_; ++i) {
      payload_.push_back(data.at(i + 4));
    }
    checksum_ = get_checksum();
  }
  bytes_t get_data() {
    return command::preamble << cls_ << id_ << length_ << payload_ << checksum_;
  }
private:
  byte_t cls_;
  byte_t id_;
  uint16_t length_;
  uint16_t checksum_;
  bytes_t payload_;

  uint16_t get_length() {
    return static_cast<uint16_t>(payload_.size());
  }

  uint16_t get_checksum() {
    byte_t chk_a = 0;
    byte_t chk_b = 0;
    auto add_byte = [&](const byte_t byte) {
      chk_a += byte;
      chk_b += chk_a;
    };
    add_byte(cls_);
    add_byte(id_);
    // Little endian so lower byte first
    add_byte(length_ & 0xFF);
    add_byte(length_ >> 8);
    std::for_each(payload_.begin(), payload_.end(), add_byte);
    return (chk_b << 8) + chk_a;
  }

  friend struct Ublox_parser;
};


struct Payload {
};


struct Payload_pvt: public Payload {
  uint32_t itow;  // scale 10E-3
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t valid;  // 0: valid date, 1: valid time, 2: fully resolved, 3: valid mag
  uint32_t tacc;  // scale 10e-9
  int32_t nano;  // scale 10e-9
  uint8_t fixtype;  // 0: nofix, 1: dead reck, 2: 2D fix, 3: 3D fix,
                    // 4 GNSS + dead reck, 5: time fix only
  uint8_t flags;  // 0: gnss fix ok, 1: diff solution, 2-4 pms state,
                  // 5: headveh valid, 6-7 carr solution
  uint8_t flags2;  // Time validity confirmation: oh well....
  uint8_t numsv;
  int32_t lon;  // scale 10E-7 degrees
  int32_t lat;  // scale 10E-7 degrees
  int32_t height;  // scale 10E-3
  int32_t hmsl;  // scale 10E-3
  uint32_t hacc;  // scale 10E-3
  uint32_t vacc;  // scale 10E-3
  int32_t veln;  // scale 10E-3
  int32_t vele;  // scale 10E-3
  int32_t veld;  // scale 10E-3
  int32_t gspeed;  // scale 10E-3
  int32_t hmot;  // scale 10E-5 degrees
  uint32_t sacc;
  uint32_t headacc;  // scale 10E-5 degrees
  uint16_t pdop;  // scale !0E-2
  uint32_t reserved1_32;
  uint16_t reserved1_16;
  int32_t headveh;  // scale 10E-5 degrees
  int16_t magdec;  // scale 10E-2 degrees
  uint16_t magacc;  // scale 10E-2 degrees

  static const auto get_parse_rule() {
    return little_dword >> little_word >> byte_ >> byte_ >> byte_ >> byte_ >> byte_ >> byte_  // itow - valid
        >> little_dword >> little_dword >> byte_ >> byte_ >> byte_ >> byte_  // tacc - numsv
        >> little_dword >> little_dword >> little_dword >> little_dword  // lon - hmsl
        >> little_dword >> little_dword >> little_dword >> little_dword >> little_dword // hacc - veld
        >> little_dword >> little_dword >> little_dword >> little_dword // gspeed - headacc
        >> little_word >> little_dword >> little_word >> little_dword // pdop - headveh
        >> little_word >> little_word;  // magdec - magacc
  }
};


struct Payload_att: public Payload {
  uint32_t itow;
  uint8_t version;
  uint16_t reserved1_16;
  uint8_t reserved1_8;
  int32_t roll;  // scale 1e-5
  int32_t pitch;  // scale 1e-5
  int32_t heading;  // scale 1e-5
  uint32_t accroll;  // scale 1e-5
  uint32_t accpitch;  // scale 1e-5
  uint32_t accheading;  // scale 1e-5

  static const auto get_parse_rule() {
    return little_dword >> byte_ >> little_word >> byte_  // itow - reserved1
        >> little_dword >> little_dword >> little_dword  // roll - heading
        >> little_dword >> little_dword >> little_dword;  // accroll - accheading
  }
};


struct Payload_ins: public Payload {
  uint32_t bitfield0;
  uint32_t reserved1;
  uint32_t itow;
  int32_t xangrate;  // scale 1E-3
  int32_t yangrate;  // scale 1E-3
  int32_t zangrate;  // scale 1E-3
  int32_t xaccel;  // scale 1E-2, free acceleration: no gravity
  int32_t yaccel;  // scale 1E-2, free acceleration: no gravity
  int32_t zaccel;  // scale 1E-2, free acceleration: no gravity

  static const auto get_parse_rule() {
    return little_dword >> little_dword  // bitfield0 - reserved1
        >> little_dword  // itow
        >> little_dword >> little_dword >> little_dword  // xangrate - zangrate
        >> little_dword >> little_dword >> little_dword;  // xaccel - zaccel
  }
};


struct Payload_raw: public Payload {
  uint32_t reserved1;
  struct Sensor_data {
    uint32_t data;  // 8 bit datatype + 24 bit data
    uint32_t stag;  // time tag
    static const auto get_parse_rule() {
      return little_dword >> little_dword;
    }
  };
  std::vector<Sensor_data> sensor_data;
  static const auto get_parse_rule() {
    return little_dword >> *(Sensor_data::get_parse_rule());
  }
};

#define RULE_DEFINE(name, type) \
x3::rule<struct name, type> const name = "\"" #name "\""; \
auto name##_def = type::get_parse_rule(); \
BOOST_SPIRIT_DEFINE(name)

RULE_DEFINE(nav_pvt, Payload_pvt)
RULE_DEFINE(nav_att, Payload_att)
RULE_DEFINE(esf_ins, Payload_ins)
RULE_DEFINE(esf_raw, Payload_raw)
RULE_DEFINE(esf_raw_data, Payload_raw::Sensor_data)

#undef RULE_DEFINE

void Ublox_parser::parse() {
  Data_packet packet;
  int len = 0;
  auto set_cls = [&](auto& ctx) { packet.cls_ = _attr(ctx); };
  auto set_id = [&](auto& ctx) { packet.id_ = _attr(ctx); };
  auto set_len = [&](auto& ctx) {
    packet.length_ = _attr(ctx);
    len = packet.length_;
  };

  auto have_data = [&](auto& ctx) { _pass(ctx) = len-- > 0; };
  auto have_all = [&](auto& ctx) { _pass(ctx) = len < 0; };
  auto add_payload = [&](auto& ctx) {
    packet.payload_.push_back(_attr(ctx));
  };
  auto set_chk = [&](auto& ctx) { packet.checksum_ = _attr(ctx); };

  //! Start of a packet
  auto preamble = byte_(command::sync_1) >> byte_(command::sync_2);
  //! Anything that is not the start of a packet
  auto junk = *(byte_ - preamble);
  //! The actual data we're looking for
  auto content = *(eps[have_data] >> byte_[add_payload]) >> eps[have_all];
  //! The complete packet
  auto packet_rule = junk >> preamble >> byte_[set_cls] >> byte_[set_id]
                          >> little_word[set_len] >> content >> little_word[set_chk];

  cur = queue.begin();
  //! Look for messages in the queue
  while (cur != queue.end() && x3::parse(cur, queue.end(), packet_rule)) {
    //! Consume the message from the queue
    cur = queue.erase(queue.begin(), cur);
    if (packet.checksum_ == packet.get_checksum()) {
      auto payload = std::make_unique<Payload>();
      log(level::debug, "Ublox parsed message with length: %", packet.length_);
      switch (packet.cls_) {
        case command::cls_nav:
          switch (packet.id_) {
            case command::nav::pvt:
              payload = std::make_unique<Payload_pvt>();
              x3::parse(packet.payload_.begin(), packet.payload_.end(),
                        Payload_pvt::get_parse_rule(), *payload);
              break;
            case command::nav::att:
              break;
            default:
              log(level::debug, "Received unrequested ubx nav message");
          }
          break;
        case command::cls_esf:
          switch (packet.id_) {
            case command::esf::ins:
              break;
            case command::esf::raw:
              break;
            default:
              log(level::debug, "Received unrequested ubx esf message");
          }
          break;
        default:
          log(level::debug, "Received unrequested ubx message");
      }
    }
    else {
      log(level::error, "Ublox checksum error: %, %", packet.checksum_, packet.get_checksum());
    }
    packet.payload_.clear();
  }

}

}  // namespace parser

namespace command {


cbytes_t cfg_prt_usb = parser::Data_packet(cls_cfg, cfg::prt, cfg::prt_payload_usb).get_data();
cbytes_t cfg_prt_uart = parser::Data_packet(cls_cfg, cfg::prt, cfg::prt_payload_uart).get_data();
cbytes_t mon_ver = parser::Data_packet(cls_mon, mon::ver, {}).get_data();
cbytes_t cfg_pms = parser::Data_packet(cls_cfg, cfg::pms, cfg::pms_payload).get_data();
cbytes_t cfg_hnr = parser::Data_packet(cls_cfg, cfg::hnr, cfg::hnr_payload).get_data();
cbytes_t cfg_rate = parser::Data_packet(cls_cfg, cfg::rate, cfg::rate_payload).get_data();
cbytes_t cfg_nav5 = parser::Data_packet(cls_cfg, cfg::nav5, cfg::nav5_payload).get_data();

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
cbytes_t cfg_gnss_glonass = parser::Data_packet(cls_cfg, cfg::gnss, gnss_glonass_payload).get_data();
cbytes_t cfg_gnss_galileo = parser::Data_packet(cls_cfg, cfg::gnss, gnss_galileo_payload).get_data();
cbytes_t cfg_gnss_beidou = parser::Data_packet(cls_cfg, cfg::gnss, gnss_beidou_payload).get_data();

cbytes_t cfg_msg_nav_pvt = parser::Data_packet(cls_cfg, cfg::msg, { cls_nav, nav::pvt, 0x0A }).get_data();
cbytes_t cfg_msg_nav_att = parser::Data_packet(cls_cfg, cfg::msg, { cls_nav, nav::att, 0x0A }).get_data();
cbytes_t cfg_msg_esf_ins = parser::Data_packet(cls_cfg, cfg::msg, { cls_esf, esf::ins, 0x64 }).get_data();
cbytes_t cfg_msg_esf_raw = parser::Data_packet(cls_cfg, cfg::msg, { cls_esf, esf::raw, 0x64 }).get_data();


}  // namespace command

namespace response {

cbytes_t ack = { command::sync_1, command::sync_2, command::cls_ack, command::ack::ack, 0x02, 0x00, command::cls_cfg };
cbytes_t nak = { command::sync_1, command::sync_2, command::cls_ack, command::ack::nak, 0x02, 0x00, command::cls_cfg };
cbytes_t mon_ver = { command::sync_1, command::sync_2, command::cls_mon, command::mon::ver };

}  // namespace response

}  // namespace ubx

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_pvt, 
  itow, year, month, day, hour, min, sec, valid,
  tacc, nano, fixtype, flags, flags2, numsv,
  lon,  lat, height, hmsl,  
  hacc, vacc, veln, vele, veld,
  gspeed, hmot, sacc, headacc,
  pdop,  reserved1_32, reserved1_16, headveh,
  magdec, magacc
)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_att, 
  itow, version, reserved1_16, reserved1_8,
  roll, pitch, heading,
  accroll, accpitch, accheading
)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_ins,
  bitfield0, reserved1,
  itow,
  xangrate, yangrate, zangrate,
  xaccel, yaccel, zaccel)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_raw, reserved1, sensor_data)
BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_raw::Sensor_data, data, stag)

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
