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

#include "../datetime.h"

#define _USE_MATH_DEFINES
#include <math.h>
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
  cbyte_t dop = 0x04;
  cbyte_t att = 0x05;
  cbyte_t sol = 0x06;
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
  cbytes_t gnss_payload_beidou = {
    0x03,  // GnssId: Beidou
    0x08,  // Min channels
    0x10,  // Max channels
    0x00,  // Reserved
    0x00, 0x00, 0x01, 0x00,  // Flags: disabled + L1
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
    0x01, 0x00, 0x01, 0x00,  // Flags: enabled + L1
  };


  cbyte_t hnr = 0x5C;
  cbytes_t hnr_payload = { 
    0x0A,  // 10Hz
    0x00,  // Reserved
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
using x3::big_word;
using x3::big_dword;
using x3::big_bin_float;
using x3::big_bin_double;

using x3::_attr;
using x3::_val;
using x3::_pass;


struct Data_packet {
  Data_packet(): cls_(), id_(), length_(), checksum_(), payload_() {}
  Data_packet(const byte_t cls, const byte_t id): cls_(cls), id_(id), length_(), payload_() {
    checksum_ = get_checksum();
  }
  Data_packet(const byte_t cls, const byte_t id, cbytes_t payload): cls_(cls), id_(id), payload_(payload) {
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
};


}  // namespace parser

namespace command {

cbytes_t cfg_prt_usb = parser::Data_packet(command::cls_cfg, command::cfg::prt, command::cfg::prt_payload_usb).get_data();
cbytes_t cfg_prt_uart = parser::Data_packet(command::cls_cfg, command::cfg::prt, command::cfg::prt_payload_uart).get_data();


}  // namespace command

namespace response {

cbytes_t ack = { command::sync_1, command::sync_2, command::cls_ack, command::ack::ack, 0x02, 0x00, command::cls_cfg };
cbytes_t nak = { command::sync_1, command::sync_2, command::cls_ack, command::ack::nak, 0x02, 0x00, command::cls_cfg };

}  // namespace response

}  // namespace ubx

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
