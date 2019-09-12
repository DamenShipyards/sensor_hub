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
#include "../functions.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <boost/variant.hpp>


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


struct Payload {
  virtual Stamped_quantities get_values() const {
    return Stamped_quantities();
  }
};

struct Time_data {
  uint32_t itow;  // scale 10E-3
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t valid;  // 0: valid date, 1: valid time, 2: fully resolved, 3: valid mag
  enum { valid_date = 0x01, valid_time = 0x02, valid_full = 0x04, valid_mag = 0x08 };
  uint32_t tacc;  // scale 10e-9
  int32_t nano;  // scale 10e-9

  static const auto get_parse_rule() {
    return little_dword >> little_word >> byte_ >> byte_  // itow - day
           >> byte_ >> byte_ >> byte_ >> byte_ >> little_dword >> little_dword; // hour - nano
  }

  bool add_values(Stamped_quantities& values) const {
    if (valid & valid_full) {
      values.push_back({ compose_time_value(year, month, day, hour, min, sec, nano), 0.0, Quantity::ut });
      return true;
    }
    else {
      return false;
    }
  }
};

struct Position_data {
  int32_t lon;  // scale 10E-7 degrees
  int32_t lat;  // scale 10E-7 degrees
  int32_t height;  // scale 10E-3
  int32_t hmsl;  // scale 10E-3
  uint32_t hacc;  // scale 10E-3
  uint32_t vacc;  // scale 10E-3

  static const auto get_parse_rule() {
    return little_dword >> little_dword >> little_dword >> little_dword  // lon - hmsl
      >> little_dword >> little_dword;  // hacc - vacc
  }

  bool add_values_horizontal(Stamped_quantities& values) const {
    values.push_back({ lat * 1E-7 * M_PI / 180.0, 0.0, Quantity::la });
    values.push_back({ lon * 1E-7 * M_PI / 180.0, 0.0, Quantity::lo });
    values.push_back({ hacc * 1E-3, 0.0, Quantity::hacc });
    return true;
  }

  bool add_values_vertical(Stamped_quantities& values) const {
    values.push_back({ height * 1E-3, 0.0, Quantity::h1 });
    values.push_back({ hmsl * 1E-3, 0.0, Quantity::h2 });
    values.push_back({ vacc * 1E-3, 0.0, Quantity::vacc });
    return true;
  }
};

struct Velocity_data {
  int32_t veln;  // scale 1E-3
  int32_t vele;  // scale 1E-3
  int32_t veld;  // scale 1E-3
  int32_t gspeed;  // scale 1E-3
  int32_t hmot;  // scale 1E-5 degrees
  uint32_t sacc; // scale 1E-3 m/s
  uint32_t headacc;  // scale 1E-5 degrees
  uint16_t pdop;  // scale 1E-2
  /* Omitted
  uint32_t reserved1_32;
  uint16_t reserved1_16;
  */
  int32_t headveh;  // scale 1E-5 degrees

  bool add_values(Stamped_quantities& values) const {
    values.push_back({ gspeed * 1E-3, 0.0, Quantity::vog });
    values.push_back({ hmot * 1E-5 * M_PI / 180.0, 0.0, Quantity::crs });
    values.push_back({ sacc * 1E-3, 0.0, Quantity::sacc });
    values.push_back({ headacc * 1E-5 * M_PI / 180.0, 0.0, Quantity::cacc });
    values.push_back({ headveh * 1E-5 * M_PI / 180.0, 0.0, Quantity::hdg });
    values.push_back({ headacc * 1E-5 * M_PI / 180.0, 0.0, Quantity::hdac });
    return true;
  }

  static const auto get_parse_rule() {
    return little_dword >> little_dword >> little_dword // veln - veld
        >> little_dword >> little_dword >> little_dword >> little_dword // gspeed - headacc
        >> little_word >> omit[repeat(6)[byte_]] >> little_dword; // pdop - headveh
        // >> little_word >> little_dword >> little_word >> little_dword; // pdop - headveh
  }
};

struct Payload_pvt: public Payload {
	Time_data time_data;
  uint8_t fixtype;  // 0: nofix, 1: dead reck, 2: 2D fix, 3: 3D fix,
                    // 4 GNSS + dead reck, 5: time fix only
  enum { fixtype_nofix, fixtype_deadreck, fixtype_2d, fixtype_3d, fixtype_3d_deadreck, fixtype_time };
  uint8_t flags;  // 0: gnss fix ok, 1: diff solution, 2-4 pms state,
                  // 5: headveh valid, 6-7 carr solution
  enum { flags_gnss = 0x01, flags_differential = 0x02, flags_headveh_valid = 0x20 };
  uint8_t flags2;  // Time validity confirmation: oh well....
  uint8_t numsv;
  Position_data position_data;
  Velocity_data velocity_data;
  int16_t magdec;  // scale 1E-2 degrees
  uint16_t magacc;  // scale 1E-2 degrees


  static const auto get_parse_rule();

  Stamped_quantities get_values() const override {
    Stamped_quantities values;

    time_data.add_values(values);
    if (fixtype == fixtype_2d || fixtype == fixtype_3d || fixtype == fixtype_3d_deadreck) {
      position_data.add_values_horizontal(values);
      if (fixtype != fixtype_2d) {
        position_data.add_values_vertical(values);
      }
    }

    if (flags & flags_headveh_valid) {
      velocity_data.add_values(values);
    }

    return values;
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
    return byte_(command::cls_nav) >> byte_(command::nav::att) >> little_word(32)
        >> little_dword >> byte_ >> little_word >> byte_  // itow - reserved1
        >> little_dword >> little_dword >> little_dword  // roll - heading
        >> little_dword >> little_dword >> little_dword;  // accroll - accheading
  }

  Stamped_quantities get_values() const override {
    Stamped_quantities values;
    if (accroll != 0) {
      double faccroll = accroll * 1E-5 * M_PI / 180.0;
      if (faccroll < 0.05) {
        values.push_back({roll * 1E-5 * M_PI / 180.0, 0.0, Quantity::ro});
        values.push_back({faccroll, 0.0, Quantity::racc});
      }
    }
    if (accpitch != 0) {
      double faccpitch = accpitch * 1E-5 * M_PI / 180.0;
      if (faccpitch < 0.05) {
        values.push_back({pitch * 1E-5 * M_PI / 180.0, 0.0, Quantity::pi});
        values.push_back({faccpitch, 0.0, Quantity::pacc});
      }
    }
    if (accheading != 0) {
      double faccheading = accheading * 1E-5 * M_PI / 180.0;
      if (faccheading < 0.10) {
        values.push_back({heading * 1E-5 * M_PI / 180.0, 0.0, Quantity::ya});
        values.push_back({faccheading, 0.0, Quantity::yacc});
      }
    }
    return values;
  }
};


struct Payload_ins: public Payload {
  uint32_t bitfield0;
  enum {
    bitfield0_rr_valid=1 << 8,
    bitfield0_pr_valid=1 << 9,
    bitfield0_yr_valid=1 << 10,
    bitfield0_fax_valid=1 << 11,
    bitfield0_fay_valid=1 << 12,
    bitfield0_faz_valid=1 << 13
  };

  uint32_t reserved1;
  uint32_t itow;
  int32_t xangrate;  // scale 1E-3
  int32_t yangrate;  // scale 1E-3
  int32_t zangrate;  // scale 1E-3
  int32_t xaccel;  // scale 1E-2, free acceleration: no gravity
  int32_t yaccel;  // scale 1E-2, free acceleration: no gravity
  int32_t zaccel;  // scale 1E-2, free acceleration: no gravity

  static const auto get_parse_rule() {
    return byte_(command::cls_esf) >> byte_(command::esf::ins) >> little_word(36)
        >> little_dword >> little_dword  // bitfield0 - reserved1
        >> little_dword  // itow
        >> little_dword >> little_dword >> little_dword  // xangrate - zangrate
        >> little_dword >> little_dword >> little_dword;  // xaccel - zaccel
  }

  Stamped_quantities get_values() const override {
    Stamped_quantities values;
    if (bitfield0 & bitfield0_rr_valid)
      values.push_back({xangrate * 10E-3 * M_PI / 180.0, 0.0, Quantity::rr});
    if (bitfield0 & bitfield0_pr_valid)
      values.push_back({yangrate * 10E-3 * M_PI / 180.0, 0.0, Quantity::pr});
    if (bitfield0 & bitfield0_yr_valid)
      values.push_back({zangrate * 10E-3 * M_PI / 180.0, 0.0, Quantity::yr});
    if (bitfield0 & bitfield0_fax_valid)
      values.push_back({xaccel * 10E-2, 0.0, Quantity::fax});
    if (bitfield0 & bitfield0_fay_valid)
      values.push_back({yaccel * 10E-2, 0.0, Quantity::fay});
    if (bitfield0 & bitfield0_faz_valid)
      values.push_back({zaccel * 10E-2, 0.0, Quantity::faz});
    return values;
  }
};


struct Sensor_data {
  uint32_t data;  // 8 bit datatype + 24 bit data
  uint32_t stag;  // time tag
  static const auto get_parse_rule() {
    return little_dword >> little_dword;
  }

  enum { data_type_none=0, data_type_ryr=5, data_type_gtmp=12, data_type_rpr= 13, data_type_rrr=14,
         data_type_rax=16, data_type_ray=17, data_type_raz=18 };

  uint8_t get_data_type() const {
    return data >> 24;
  }

  Stamped_quantity get_value(uint32_t reftag) const {
    uint32_t shifted = (data & 0xFFFFFF) << 8;
    int signed_shifted = *reinterpret_cast<int*>(&shifted);
    Value_type value = static_cast<Value_type>(signed_shifted);
    if (reftag < stag)
        return {0.0, 0.0, Quantity::end};
    double offset = (reftag - stag) * 0.01 / -256.0;

    switch (get_data_type()) {
      case data_type_ryr:
        return {value / (4096.0 * 256.0 * 180.0) * M_PI, offset, Quantity::ryr};
      case data_type_rpr:
        return {value / (4096.0 * 256.0 * 180.0) * M_PI, offset, Quantity::rpr};
      case data_type_rrr:
        return {value / (4096.0 * 256.0 * 180.0) * M_PI, offset, Quantity::rrr};
      case data_type_rax:
        return {value / (1024.0 * 256.0), offset, Quantity::rax};
      case data_type_ray:
        return {value / (1024.0 * 256.0), offset, Quantity::ray};
      case data_type_raz:
        return {value / (1024.0 * 256.0), offset, Quantity::raz};
      case data_type_gtmp:
        return {value / (100 * 256.0), offset, Quantity::gtmp};
      default:
        return {0.0, 0.0, Quantity::end};
    }
  }
};


struct Payload_raw: public Payload {
  uint16_t length;
  uint32_t reserved1;
  std::vector<Sensor_data> sensor_data;

  static const auto get_parse_rule();

  Stamped_quantities get_values() const override {
    Stamped_quantities values;
    if (!sensor_data.empty()) {
      auto reftag = sensor_data.back().stag;
      for (auto& data: sensor_data) {
        auto value = data.get_value(reftag);
        if (value.quantity != Quantity::end) {
          values.push_back(value);
        }
      }
    }
    return values;
  }
};

#define RULE_DEFINE(name, type) \
x3::rule<struct name, type> const name = "\"" #name "\""; \
auto name##_def = type::get_parse_rule(); \
BOOST_SPIRIT_DEFINE(name)

RULE_DEFINE(t_data, Time_data)
RULE_DEFINE(p_data, Position_data)
RULE_DEFINE(v_data, Velocity_data)
RULE_DEFINE(s_data, Sensor_data)

const auto Payload_pvt::get_parse_rule() {
  return byte_(command::cls_nav) >> byte_(command::nav::pvt) >> little_word(92)
    >> t_data  // time_data
    >> byte_ >> byte_ >> byte_ >> byte_  // fixtype - numsv
    >> p_data  // postion_data
    >> v_data // velocity_data
    >> little_word >> little_word;  // magdec - magacc
}

const auto Payload_raw::get_parse_rule() {
  return byte_(command::cls_esf) >> byte_(command::esf::raw)
    >> little_word >> little_dword   // length - reserved1
    >> *s_data;  // sensor_data
}

RULE_DEFINE(nav_pvt, Payload_pvt)
RULE_DEFINE(nav_att, Payload_att)
RULE_DEFINE(esf_ins, Payload_ins)
RULE_DEFINE(esf_raw, Payload_raw)

#undef RULE_DEFINE

Ublox_parser::Ublox_parser()
  : Packet_parser(),
    visitor(std::make_unique<Payload_visitor>()) {
}


Ublox_parser::~Ublox_parser() {
}

struct Ublox_parser::Payload_visitor {
  Stamped_queue values;
  double stamp;

  void operator()(const Payload& payload) {
    for (auto& value: payload.get_values()) {
      value.stamp += stamp;
      values.push_back(value);
    }
  }
};

Stamped_queue& Ublox_parser::get_values() {
  return visitor->values;
}

using Payload_variant = boost::variant<Payload_pvt, Payload_att, Payload_ins, Payload_raw>;

auto payload_parse_rule = nav_pvt | nav_att | esf_ins | esf_raw;

} }  // namespace ubx::parser

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Time_data,
  itow, year, month, day,
  hour, min, sec,
  valid, tacc,
  nano
)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Position_data,
  lon, lat, height, hmsl,
  hacc, vacc
)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Velocity_data,
  veln, vele, veld,
  gspeed, hmot, sacc, headacc,
  pdop, headveh
)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_pvt,
  time_data,
  fixtype, flags, flags2, numsv,
  position_data,
  velocity_data,
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

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Payload_raw,
    length, reserved1, sensor_data)

BOOST_FUSION_ADAPT_STRUCT(ubx::parser::Sensor_data,
    data, stag)

namespace ubx { namespace parser {

void Ublox_parser::parse(const double& stamp) {
  Data_packet packet;
  int len = 0;
  auto set_cls = [&](auto& ctx) { packet.set_cls(_attr(ctx)); };
  auto set_id = [&](auto& ctx) { packet.set_id(_attr(ctx)); };
  auto set_len = [&](auto& ctx) {
    len = _attr(ctx);
    packet.set_length(len);
  };

  auto have_data = [&](auto& ctx) { _pass(ctx) = len-- > 0; };
  auto have_all = [&](auto& ctx) { _pass(ctx) = len < 0; };
  auto add_payload = [&](auto& ctx) {
    packet.add_data(_attr(ctx));
  };
  auto set_chk = [&](auto& ctx) { packet.set_checksum(_attr(ctx)); };

  //! Start of a packet
  auto preamble = byte_(command::sync_1) >> byte_(command::sync_2);
  //! Anything that is not the start of a packet
  auto junk = *(byte_ - preamble);
  //! The actual data we're looking for
  auto content = *(eps[have_data] >> byte_[add_payload]) >> eps[have_all];
  //! The complete packet
  auto packet_rule = junk >> preamble >> byte_[set_cls] >> byte_[set_id]
                          >> little_word[set_len] >> content >> little_word[set_chk];

  visitor->stamp = stamp;
  cur = queue.begin();
  //! Look for messages in the queue
  while (cur != queue.end() && x3::parse(cur, queue.end(), packet_rule)) {
    //! Consume the message from the queue
    cur = queue.erase(queue.begin(), cur);
    if (packet.check()) {
      Payload_variant payload;
      if (x3::parse(packet.get_data().begin(), packet.get_data().end(),
                    payload_parse_rule, payload)) {
        boost::apply_visitor(*visitor, payload);
      }
      else {
        log(level::debug, "Received an unsollicited ubx message");
      }
    }
    else {
      log(level::error, "Ublox packet check error: lenght %, %, checksum %, %",
          packet.get_length(), packet.calc_length(), packet.get_checksum(), packet.calc_checksum());
    }
    packet.clear();
  }
}

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

}  // namespace ubx


#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
