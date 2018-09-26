/**
 * \file xsens_impl.h
 * \brief Implementation details for Xsens device base class
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "datetime.h"

namespace gregorian = boost::gregorian;

namespace command {

cbyte_t packet_start = 0xFA;
cbyte_t sys_command = 0xFF;
cbyte_t conf_command = 0x01;

cdata_t goto_config = {packet_start, sys_command, XMID_GotoConfig, 0x00, 0xD1};
cdata_t config_ack = {packet_start, sys_command, XMID_GotoConfigAck};

cdata_t goto_measurement = {packet_start, sys_command, XMID_GotoMeasurement, 0x00, 0xF1};
cdata_t measurement_ack = {packet_start, sys_command, XMID_GotoMeasurementAck};

cdata_t set_option_flags = {packet_start, sys_command, XMID_SetOptionFlags,
  0x08,
  0x00, 0x00, 0x00,       // Option flags to set:
  0x00 |
  // 0x01 |               // Disable auto store
  ///0x02 |               // Disable auto measurement
  // 0x04 |               // EnableBeidou (instead of GLONASS)
  0x10 |                  // EnableAHS (relate Yaw only, no heading)
  0x80 |                  // EnableInRunCompassCalibration
  0x00,
  0x00, 0x00, 0x00,       // Option flags to clear
  0x00 |
  // 0x02 |               // Clear: Disable auto measurement
  0x00,
  0x21                    // Checksum
};
cdata_t option_flags_ack = {packet_start, sys_command, XMID_SetOptionFlagsAck};

cdata_t req_reset = {packet_start, sys_command, XMID_Reset, 0x00, 0xC1};
cdata_t reset_ack = {packet_start, sys_command, XMID_ResetAck, 0x00, 0xC0};

cdata_t req_device_id = {packet_start, sys_command, XMID_ReqDid, 0x00, 0x01};
cdata_t device_id_resp = {packet_start, sys_command, XMID_DeviceId};

cdata_t init_mt = {packet_start, sys_command, XMID_Initbus, 0x00, 0xFF};
cdata_t mt_ack = {packet_start, sys_command, XMID_InitBusResults};

cdata_t wakeup = {packet_start, sys_command, XMID_Wakeup, 0x00, 0xC3};
cdata_t wakeup_ack = {packet_start, sys_command, XMID_WakeupAck, 0x00, 0xC2};

cdata_t req_product_code = {packet_start, sys_command, XMID_ReqProductCode, 0x00, 0xE5};
cdata_t product_code_resp = {packet_start, sys_command, XMID_ProductCode};

cdata_t req_firmware_rev = {packet_start, sys_command, XMID_ReqFirmwareRevision, 0x00, 0xEF};
cdata_t firmware_rev_resp = {packet_start, sys_command, XMID_FirmwareRevision};

cdata_t error_resp = {packet_start, sys_command, XMID_Error};

cdata_t set_output_configuration = {
  packet_start, sys_command,
  XMID_SetOutputConfiguration,
  0x2C,                   // Length
  0x10, 0x10, 0x00, 0x00, // Utc time
  0x40, 0x20, 0x00, 0x64, // Acceleration, 100Hz
  0x40, 0x30, 0x00, 0x64, // Free Acceleration, 100Hz
  0x80, 0x20, 0x00, 0x64, // Rate of turn, 100Hz
  0x50, 0x43, 0x00, 0x0A, // LatLon, 10Hz
  0xC0, 0x20, 0x00, 0x0A, // Magnetic flux, 10Hz
  0xD0, 0x10, 0x00, 0x0A, // Velocity, 10Hz
  0x50, 0x20, 0x00, 0x0A, // Altitude above ellipsoid, 10Hz
  0x50, 0x10, 0x00, 0x0A, // Altitude above MSL, 10Hz
  0x20, 0x30, 0x00, 0x0A, // Euler angles, 10 Hz
  0x20, 0x10, 0x00, 0x0A, // Quaternion, 10 Hz
  0x70                    // Checksum
};

cdata_t output_configuration_ack = {
  packet_start, sys_command,
  XMID_SetOutputConfigurationAck
};

} // namespace data

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
  Data_packet(): id(0), len(0) {}
  Data_packet(const uint16_t did): id(did), len(0) {}
  uint16_t id;
  int len;
  virtual Values_type get_values() const {
    return Values_type();
  }
};

template <int SIZE>
struct Date_time_value: public Data_packet {
  static constexpr uint8_t valid_utc = 0x04;
  static constexpr uint16_t did = XDI_UtcTime;
  static constexpr Quantity quantity = Quantity::ut;
  uint32_t nano;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t flags;

  Values_type get_values() const override {
    using namespace posix_time;
    if (flags & valid_utc) {
      ptime t(
          gregorian::date(year, month, day), 
          hours(hour) + minutes(minute) + seconds(second) + microseconds(nano/1000)
      );
      return Values_type{{quantity, 1E-6 * (t - unix_epoch).total_microseconds()}};
    }
    else {
      return Values_type();
    }
  }

  static constexpr auto get_parse_rule() {
    return big_word(did) >> byte_(SIZE) >> big_dword >> big_word >> byte_ >> byte_ >> byte_ >> byte_ >> byte_ >> byte_;
  }
};

template<int DIM>
struct UnitaryConverter {
  static constexpr double factor(int dim, double f=1.0) {
    return (DIM == 3) && (dim > 0) ? -f :
           (DIM == 4) && (dim > 1) ? -f : f;
  }
};

template<int DIM>
struct RadConverter: UnitaryConverter<DIM> {
  static constexpr double factor(int dim) {
    return UnitaryConverter<DIM>::factor(dim, M_PI / 180.0);
  }
};

template<int DIM>
struct TeslaConverter: UnitaryConverter<DIM> {
  static constexpr double factor(int dim) {
    return UnitaryConverter<DIM>::factor(dim, 1E-4);
  }
};

template<uint16_t DID, uint16_t COORD, uint16_t FORMAT, int DIM, Quantity QUANT, 
         template<int D> typename Converter=UnitaryConverter>
struct Data_value: public Data_packet {
  typedef Converter<DIM> converter;
  static constexpr uint16_t did = DID | COORD | FORMAT;
  static constexpr Quantity quantity = QUANT;
  Data_value(): Data_packet(did), data() {}

  typedef typename std::conditional<FORMAT == XDI_SubFormatFloat, float,
          typename std::conditional<FORMAT == XDI_SubFormatFp1220, uint32_t,
          typename std::conditional<FORMAT == XDI_SubFormatFp1632, std::vector<uint16_t>,
          typename std::conditional<FORMAT == XDI_SubFormatDouble, double, void
          >::type >::type >::type >::type data_type;

  std::vector<data_type> data;

  Values_type get_values() const override {
    Values_type result;
    int dim = 0;
    Quantity_iter qi(quantity);
    for (auto& value: data) {
      result.push_back({*qi++, converter::factor(dim++) * static_cast<double>(value)});
    }
    return result;
  }

  template <uint16_t PFORMAT = FORMAT>
  static constexpr auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatFloat, int>::type n=0) {
    return big_word(did) >> byte_(DIM * 4) >> repeat(DIM)[big_bin_float];
  }
  template <uint16_t PFORMAT = FORMAT>
  static constexpr auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatDouble, int>::type n=0) {
    return big_word(did) >> byte_(DIM * 8) >> repeat(DIM)[big_bin_double];
  }
};


using Date_time = Date_time_value<12>;
using Acceleration = Data_value<XDI_Acceleration, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::ax>;
using Free_acceleration = Data_value<XDI_FreeAcceleration, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::fax>;
using Rate_of_turn = Data_value<XDI_RateOfTurn, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::rr, RadConverter>;
using Lat_lon = Data_value<XDI_LatLon, XDI_CoordSysEnu, XDI_SubFormatDouble, 2, Quantity::la, RadConverter>;
using Magnetic_flux = Data_value<XDI_MagneticField, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::mx, TeslaConverter>;
using Velocity = Data_value<XDI_VelocityXYZ, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::vx>;
using Altitude_ellipsoid = Data_value<XDI_AltitudeEllipsoid, XDI_CoordSysEnu, XDI_SubFormatFloat, 1, Quantity::h1>;
using Altitude_msl = Data_value<XDI_AltitudeMsl, XDI_CoordSysEnu, XDI_SubFormatFloat, 1, Quantity::h2>;
using Euler_angles = Data_value<XDI_EulerAngles, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::ro, RadConverter>;
using Quaternion = Data_value<XDI_Quaternion, XDI_CoordSysEnu, XDI_SubFormatFloat, 4, Quantity::q1>;

auto set_did = [](auto& ctx) { _val(ctx).id = _attr(ctx); };
auto set_len = [](auto& ctx) { _val(ctx).len = static_cast<int>(_attr(ctx)); };
auto more = [](auto& ctx) { _pass(ctx) = _val(ctx).len-- > 0; };
auto done = [](auto& ctx) { _pass(ctx) = _val(ctx).len < 0; };


x3::rule<struct unknown_data, Data_packet> const unknown_data = "unknown_data";
auto unknown_data_def = big_word[set_did] >> byte_[set_len] >> *(eps[more] >> omit[byte_]) >> eps[done];
BOOST_SPIRIT_DEFINE(unknown_data)
  
#define RULE_DEFINE(name, type) \
x3::rule<struct name, type> const name = "\"" #name "\""; \
auto name##_def = type::get_parse_rule(); \
BOOST_SPIRIT_DEFINE(name)

RULE_DEFINE(date_time, Date_time)
RULE_DEFINE(acceleration, Acceleration)
RULE_DEFINE(free_acceleration, Free_acceleration)
RULE_DEFINE(rate_of_turn, Rate_of_turn)
RULE_DEFINE(lat_lon, Lat_lon)
RULE_DEFINE(magnetic_flux, Magnetic_flux)
RULE_DEFINE(velocity, Velocity)
RULE_DEFINE(altitude_ellipsoid, Altitude_ellipsoid)
RULE_DEFINE(altitude_msl, Altitude_msl)
RULE_DEFINE(euler_angles, Euler_angles)
RULE_DEFINE(quaternion, Quaternion)

#undef RULE_DEFINE

auto data_parser = *(
    date_time | 
    acceleration | 
    free_acceleration | 
    rate_of_turn | 
    lat_lon |
    magnetic_flux |
    velocity |
    altitude_ellipsoid |
    altitude_msl |
    euler_angles |
    quaternion |
    unknown_data);

struct Packet_parser::Data_packets 
  : public std::vector<
    boost::variant<
      Data_packet, 
      Date_time,
      Acceleration,
      Free_acceleration,
      Rate_of_turn,
      Lat_lon,
      Magnetic_flux,
      Velocity,
      Altitude_ellipsoid,
      Altitude_msl,
      Euler_angles,
      Quaternion
    > > {
};

struct Packet_parser::Data_visitor {
  Values_queue values;
  void operator()(const Data_packet& data_packet) {
    for (auto& value: data_packet.get_values()) {
      values.push_back(value);
    }
  }
};


Packet_parser::Packet_parser()
    : queue(), data(), 
      data_packets(std::make_unique<Data_packets>()), 
      visitor(std::make_unique<Data_visitor>()), 
      cur(queue.begin()) {
}

Packet_parser::~Packet_parser() {
}


void Packet_parser::parse() {
  uint8_t mid = 0;
  int len = 0;
  uint8_t sum = command::sys_command;
  Packet_parser& self = *this;

  static auto set_mid = [&](auto& ctx) { mid = _attr(ctx); sum += mid; };
  static auto set_len = [&](auto& ctx) { len = _attr(ctx); sum += len; };
  static auto add_data = [&](auto& ctx) {
    uint8_t val = _attr(ctx);
    self.data.push_back(val);
    sum += val;
  };
  static auto set_chk = [&](auto& ctx) { sum += _attr(ctx); };
  static auto have_data = [&](auto& ctx) { _pass(ctx) = len-- > 0; };
  static auto have_all = [&](auto& ctx) { _pass(ctx) = len < 0; };

  //! Start of a packet
  static auto preamble = byte_(command::packet_start) >> byte_(command::sys_command);
  //! Anything that is not the start of a packet
  static auto junk = *(byte_ - preamble);
  //! The actual data we're looking for
  static auto content = *(eps[have_data] >> byte_[add_data]) >> eps[have_all];
  //! The complete packet
  static auto packet = junk >> preamble >> byte_[set_mid] >> byte_[set_len] >> content >> byte_[set_chk];

  cur = queue.begin();
  while (cur != queue.end() && x3::parse(cur, queue.end(), packet)) {
    queue.erase(queue.begin(), cur);
    cur = queue.begin(); // Need to reset cur in case the queue got emptied, which invalidates all iterators
    if (sum == 0) {
      auto dcur = data.begin();
      if (x3::parse(dcur, data.end(), data_parser, *data_packets)) {
        for (auto& data_packet: *data_packets) {
          boost::apply_visitor(*visitor, data_packet);
        }
      }
    }
    data.clear();
    data_packets->clear();
  }
}


Values_queue& Packet_parser::get_values() {
  return visitor->values;
}

} // parser

BOOST_FUSION_ADAPT_STRUCT(parser::Date_time, nano, year, month, day, hour, minute, second, flags)
BOOST_FUSION_ADAPT_STRUCT(parser::Acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Free_acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Rate_of_turn, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Lat_lon, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Magnetic_flux, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Velocity, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Altitude_ellipsoid, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Altitude_msl, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Euler_angles, data)
BOOST_FUSION_ADAPT_STRUCT(parser::Quaternion, data)

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
