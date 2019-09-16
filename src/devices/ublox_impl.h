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


namespace gregorian = boost::gregorian;


namespace ubx {


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
    values.push_back({ height * 1E-3, 0.0, Quantity::hg84 });
    values.push_back({ hmsl * 1E-3, 0.0, Quantity::hmsl });
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


}  // namespace ubx


#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
