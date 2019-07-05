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
  cbyte_t msg = 0x01;
  cbyte_t rate = 0x08;
  cbyte_t nav5 = 0x24;
  cbyte_t nmea = 0x17;
  cbyte_t gnss = 0x3E;
  cbyte_t hnr = 0x5C;
  cbyte_t pms = 0x86;
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
    return command::preamble << cls_ << id_ << payload_ << get_checksum();
  }
private:
  byte_t cls_;
  byte_t id_;
  uint16_t length_;
  uint16_t checksum_;
  bytes_t payload_;

  uint16_t get_length() {
    return payload_.size();
  }

  uint16_t get_checksum() {
    byte_t chk_a = 0;
    byte_t chk_b = 0;
    static auto add_byte = [&](const byte_t byte) {
      chk_a += byte;
      chk_b += chk_a;
    };
    add_byte(cls_);
    add_byte(id_);
    std::for_each(payload_.begin(), payload_.end(), add_byte);
    return (chk_b << 8) + chk_a;
  }
};


} // parser

}  // namespace ubx

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
