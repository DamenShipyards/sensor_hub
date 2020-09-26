/**
 * \file xsens_impl.h
 * \brief Implementation details for Xsens device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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

#ifndef XSENS_IMPL_H_
#define XSENS_IMPL_H_

#include "xsens.h"
#include "../functions.h"

namespace xsens {

namespace command {

cbytes_t option_flags = {
  0x00, 0x00, 0x00,       // Option flags to set:
  0x00 |
  // 0x01 |               // Disable auto store
  ///0x02 |               // Disable auto measurement
  // 0x04 |               // EnableBeidou (instead of GLONASS)
  // 0x10 |               // EnableAHS (relative Yaw only, no heading)
  0x80 |                  // EnableInRunCompassCalibration
  0x00,
  0x00, 0x00, 0x00,       // Option flags to clear
  0x00 |
  // 0x02 |               // Clear: Disable auto measurement
  0x10 |                  // Clear: EnableAHS (apparently doesn't work well with MTi-G-710)
                          //        see: https://base.xsens.com/hc/en-us/requests/1182
  0x00
};

cbytes_t error_resp = {packet_start, sys_command, XMID_Error, 0x01};

cbytes_t string_output_type = {
  0x00, 0x00
};
cbytes_t string_output_type_6 = {
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};

cbytes_t output_configuration = {
  0x10, 0x10, 0xFF, 0xFF, // Utc time
  0x40, 0x20, 0x00, 0x64, // Acceleration, 100Hz
  0x40, 0x30, 0x00, 0x64, // Free Acceleration, 100Hz
  0x80, 0x20, 0x00, 0x64, // Rate of turn, 100Hz
  0x50, 0x43, 0x00, 0x0A, // LatLon, 10Hz
  0xC0, 0x20, 0x00, 0x0A, // Magnetic flux, 10Hz
  0xD0, 0x10, 0x00, 0x0A, // Velocity, 10Hz
  0x50, 0x20, 0x00, 0x0A, // Altitude above ellipsoid, 10Hz
  0x50, 0x10, 0x00, 0x0A, // Altitude above MSL, 10Hz
  0x20, 0x30, 0x00, 0x0A, // Euler angles, 10 Hz
  0x20, 0x10, 0x00, 0x0A  // Quaternion, 10 Hz
};

cbytes_t output_configuration_630 = {
  0x40, 0x20, 0x00, 0x64, // Acceleration, 100Hz
  0x40, 0x30, 0x00, 0x64, // Free Acceleration, 100Hz
  0x80, 0x20, 0x00, 0x64, // Rate of turn, 100Hz
  0xC0, 0x20, 0x00, 0x0A, // Magnetic flux, 10Hz
  0x20, 0x30, 0x00, 0x0A, // Euler angles, 10 Hz
  0x20, 0x10, 0x00, 0x0A  // Quaternion, 10 Hz
};

} // namespace command

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

/**
 * Provides default convertion value
 *
 * All sensors values a converted when arriving from the sensor.
 *
 * @tparam DIM number of dimensions in value to convert.
 */
template<int DIM, bool FLIP=false>
struct IdentityConverter {
  static constexpr double convert(int, double value, double f=1.0) {
    return value * f;
  }
};

/**
 * Template specialization for flipped axes vectors
 */
template<>
struct IdentityConverter<3, true> {
  static constexpr double convert(int dim, double value, double f=1.0) {
    return dim > 0 ? -f * value: f * value;
  }
};

/**
 * Template specialization for flipped axes quaternions
 */
template<>
struct IdentityConverter<4, true> {
  static constexpr double convert(int dim, double value, double f=1.0) {
    return dim > 1 ? -f * value: f * value;
  }
};

template<>
struct IdentityConverter<4, false> {
  static constexpr double convert(int dim, double value, double f=1.0) {
    return (dim % 2 == 0) ? -f * value: f * value;
  }
};

/**
 * Deg to rad converter
 *
 * Converter provider for converting degrees to radians.
 *
 * @tparam DIM number of dimensions in value to convert
 */
template<int DIM, bool FLIP=false>
struct RadConverter {
  static constexpr double convert(int dim, double value) {
    return IdentityConverter<DIM, FLIP>::convert(dim, value, M_PI / 180.0);
  }
};


/**
 * Template specializations for orientation angles
 *
 * Orientation angles from the Xsens do *not* match the device axes :(.
 */
template<>
struct RadConverter<3, false> {
  static constexpr double convert(int dim, double value) {
    double result = IdentityConverter<3, true>::convert(dim, value, M_PI / 180.0);
    return dim == 0 ? M_PI  + result : result;
  }
};

template<>
struct RadConverter<3, true> {
  static constexpr double convert(int dim, double value) {
    return IdentityConverter<3, true>::convert(dim, value, M_PI / 180.0);
  }
};


/**
 * Magnetic field strength converter
 *
 * Converter for converting fields strengths
 * in Gauss to Tesla.
 *
 * @tparam DIM number of dimensions in value to convert
 */
template<int DIM, bool FLIP=false>
struct TeslaConverter {
  static constexpr double convert(int dim, double value) {
    return IdentityConverter<DIM, FLIP>::convert(dim, value, 1E-4);
  }
};


/**
 * Base class for data packets received from sensor
 *
 * Data packets arrive as part of XMID_MtData2 messages
 * received from the sensor. Each block of data in this
 * message is described by a data identifier (DID) and
 * a length.
 */
struct Data_packet {
  Data_packet(): id(0), len(0) {}
  Data_packet(const uint16_t did): id(did), len(0) {}
  uint16_t id;
  int len;
  /**
   * Return a vector of Quantity_value's contained
   * in this data packet
   */
  virtual Quantity_values get_values() const {
    return Quantity_values();
  }
};

/**
 * Date-time data structure
 */
struct Date_time: public Data_packet {
  static constexpr uint8_t len = 12;
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

  Quantity_values get_values() const override {
    if (flags & valid_utc) {
      // Return a list with one Unix Time value
      return Quantity_values{compose_time_quantity(year, month, day, hour, minute, second, nano)};
    }
    else {
      // Return an empy list
      return Quantity_values();
    }
  }

  static const auto get_parse_rule() {
    return big_word(did) >> byte_(len) >> big_dword >> big_word >> byte_ >> byte_ >> byte_ >> byte_ >> byte_ >> byte_;
  }
};


namespace detail {

template<int DIM, bool FLIP>
inline int index_permute(int i) {
  return i;
}

template<>
inline int index_permute<4, false>(int i) {
  return 2 * (i / 2) + (i + 1) % 2;
}

}

/**
 * Generic data packet
 *
 * Describes all data packets except date-time
 *
 * @tparam DID data identifier of this packet
 * @tparam COORD coordinate system use by sensor when providing this data value
 * @tparam FORMAT number format used by sensro when providing this data value
 * @tparam DIM number of values provided for this data type
 * @tparam QUANT The sensor hub quantity associated with this data
 * @tparam Converter Value converter provider for this data packet
 * @tparam FLIP whether to convert for flipped axes
 */
template<uint16_t DID, uint16_t COORD, uint16_t FORMAT, int DIM, Quantity QUANT,
         template<int D, bool F> typename Converter=IdentityConverter, bool FLIP=false >
struct Data_value: public Data_packet {
  typedef Converter<DIM, FLIP> converter;
  static constexpr uint16_t did = DID | COORD | FORMAT;
  static constexpr Quantity quantity = QUANT;
  Data_value(): Data_packet(did), data() {}

  // Associate a C++ native type with the format specifier in the data packet
  typedef typename std::conditional<FORMAT == XDI_SubFormatFloat, float,
          typename std::conditional<FORMAT == XDI_SubFormatFp1220, uint32_t,
          typename std::conditional<FORMAT == XDI_SubFormatFp1632, std::vector<uint16_t>,
          typename std::conditional<FORMAT == XDI_SubFormatDouble, double, void
          >::type >::type >::type >::type bytes_type;

  std::vector<bytes_type> data;

  /**
   * Get a list of converted sensor hub Values from the raw data that was
   * received from the sensor
   */
  Quantity_values get_values() const override {
    Quantity_values result(DIM);
    Quantity_iter qi(quantity);
    for (int dim=0; dim < DIM; ++dim) {
      // When a data packet contains multiple values, the quantities of these
      // values are always consecutive, so we can just increment the quantity
      double value =static_cast<double>(data[detail::index_permute<DIM,FLIP>(dim)]);
      result[dim] = {value_norm(*qi, converter::convert(dim, value)), *qi++};
    }
    return result;
  }


  /**
   * Get parse rule for single precision data
   */
  template <uint16_t PFORMAT = FORMAT>
  static const auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatFloat, int>::type n=0) {
    (void)n;
    return big_word(did) >> byte_(DIM * 4) >> repeat(DIM)[big_bin_float];
  }

  /**
   * Get parse rule for double precision data
   */
  template <uint16_t PFORMAT = FORMAT>
  static const auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatDouble, int>::type n=0) {
    (void)n;
    return big_word(did) >> byte_(DIM * 8) >> repeat(DIM)[big_bin_double];
  }

  // Other formats are not currently supported

private:
};


// Define specific data packet type for each data type received from sensor
template<bool FLIP> using AccelerationT = 
  Data_value<XDI_Acceleration, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::ax, IdentityConverter, FLIP>;
using Acceleration = AccelerationT<false>;
using Acceleration_flipped = AccelerationT<true>;
template<bool FLIP> using Free_accelerationT = 
  Data_value<XDI_FreeAcceleration, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::fax, IdentityConverter, FLIP>;
using Free_acceleration = Free_accelerationT<false>;
using Free_acceleration_flipped = Free_accelerationT<true>;
template<bool FLIP> using Rate_of_turnT = 
  Data_value<XDI_RateOfTurn, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::rr, IdentityConverter, FLIP>;
using Rate_of_turn = Rate_of_turnT<false>;
using Rate_of_turn_flipped = Rate_of_turnT<true>;
using Lat_lon = Data_value<XDI_LatLon, XDI_CoordSysEnu, XDI_SubFormatDouble, 2, Quantity::la, RadConverter>;
template<bool FLIP> using Magnetic_fluxT = 
  Data_value<XDI_MagneticField, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::mx, TeslaConverter, FLIP>;
using Magnetic_flux = Magnetic_fluxT<false>;
using Magnetic_flux_flipped = Magnetic_fluxT<true>;
template<bool FLIP> using VelocityT = 
  Data_value<XDI_VelocityXYZ, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::vx, IdentityConverter, FLIP>;
using Velocity = VelocityT<false>;
using Velocity_flipped = VelocityT<true>;
using Altitude_ellipsoid = Data_value<XDI_AltitudeEllipsoid, XDI_CoordSysEnu, XDI_SubFormatFloat, 1, Quantity::hg84>;
using Altitude_msl = Data_value<XDI_AltitudeMsl, XDI_CoordSysEnu, XDI_SubFormatFloat, 1, Quantity::hmsl>;
template<bool FLIP> using Euler_anglesT = 
  Data_value<XDI_EulerAngles, XDI_CoordSysEnu, XDI_SubFormatFloat, 3, Quantity::ro, RadConverter, FLIP>;
using Euler_angles = Euler_anglesT<false>;
using Euler_angles_flipped = Euler_anglesT<true>;
template<bool FLIP> using QuaternionT = 
  Data_value<XDI_Quaternion, XDI_CoordSysEnu, XDI_SubFormatFloat, 4, Quantity::q1, IdentityConverter, FLIP>;
using Quaternion = QuaternionT<false>;
using Quaternion_flipped = QuaternionT<true>;


auto set_did = [](auto& ctx) { _val(ctx).id = _attr(ctx); };
auto set_len = [](auto& ctx) { _val(ctx).len = static_cast<int>(_attr(ctx)); };
auto more = [](auto& ctx) { _pass(ctx) = _val(ctx).len-- > 0; };
auto done = [](auto& ctx) { _pass(ctx) = _val(ctx).len < 0; };

// Parse rule for data packets other than the ones we're interested in
x3::rule<struct unknown_data, Data_packet> const unknown_data = "unknown_data";
auto unknown_data_def = big_word[set_did] >> byte_[set_len] >> *(eps[more] >> omit[byte_]) >> eps[done];
BOOST_SPIRIT_DEFINE(unknown_data)

#define RULE_DEFINE(name, type) \
x3::rule<struct name, type> const name = "\"" #name "\""; \
auto name##_def = type::get_parse_rule(); \
BOOST_SPIRIT_DEFINE(name)

// Define parse rules for each individual data packet type
RULE_DEFINE(date_time, Date_time)
RULE_DEFINE(acceleration, Acceleration)
RULE_DEFINE(acceleration_flipped, Acceleration_flipped)
RULE_DEFINE(free_acceleration, Free_acceleration)
RULE_DEFINE(free_acceleration_flipped, Free_acceleration_flipped)
RULE_DEFINE(rate_of_turn, Rate_of_turn)
RULE_DEFINE(rate_of_turn_flipped, Rate_of_turn_flipped)
RULE_DEFINE(lat_lon, Lat_lon)
RULE_DEFINE(magnetic_flux, Magnetic_flux)
RULE_DEFINE(magnetic_flux_flipped, Magnetic_flux_flipped)
RULE_DEFINE(velocity, Velocity)
RULE_DEFINE(velocity_flipped, Velocity_flipped)
RULE_DEFINE(altitude_ellipsoid, Altitude_ellipsoid)
RULE_DEFINE(altitude_msl, Altitude_msl)
RULE_DEFINE(euler_angles, Euler_angles)
RULE_DEFINE(euler_angles_flipped, Euler_angles_flipped)
RULE_DEFINE(quaternion, Quaternion)
RULE_DEFINE(quaternion_flipped, Quaternion_flipped)

#undef RULE_DEFINE

// Parse rule for data packets..
static auto data_parser = *(
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

// ... and for flipped data packets
static auto flipped_parser = *(
    date_time |
    acceleration_flipped |
    free_acceleration_flipped |
    rate_of_turn_flipped |
    lat_lon |
    magnetic_flux_flipped |
    velocity_flipped |
    altitude_ellipsoid |
    altitude_msl |
    euler_angles_flipped |
    quaternion_flipped |
    unknown_data);

/**
 * Vector with parsed results
 *
 * Since the type of each packet is not known in advance
 * the elements have to be of a variant type, which will
 * require a visitor to access.
 */
struct Xsens_parser::Data_packets
  : public std::vector<
    boost::variant<
      Data_packet,
      Date_time,
      Acceleration,
      Acceleration_flipped,
      Free_acceleration,
      Free_acceleration_flipped,
      Rate_of_turn,
      Rate_of_turn_flipped,
      Lat_lon,
      Magnetic_flux,
      Magnetic_flux_flipped,
      Velocity,
      Velocity_flipped,
      Altitude_ellipsoid,
      Altitude_msl,
      Euler_angles,
      Euler_angles_flipped,
      Quaternion,
      Quaternion_flipped
    > > {
};

/**
 * Visitor for accessing parsed data packets
 */
struct Xsens_parser::Data_visitor {

  Stamped_queue values;
  double stamp;

  void operator()(const Data_packet& data_packet) {
    for (auto& value: data_packet.get_values()) {
      values.push_back(Stamped_quantity(stamp, value));
    }
  }
};


Xsens_parser::Xsens_parser()
  : Packet_parser(),
    data_packets(std::make_unique<Data_packets>()),
    visitor(std::make_unique<Data_visitor>()) {
}


Xsens_parser::~Xsens_parser() {
}


bool Xsens_parser::parse_single(const double& stamp) {
  uint8_t mid = 0;
  int len = 0;
  std::vector<uint8_t> data;
  uint8_t sum = command::sys_command;

  auto set_mid = [&](auto& ctx) { mid = _attr(ctx); sum += mid; };
  auto set_len = [&](auto& ctx) { len = _attr(ctx); sum += len; };
  auto add_data = [&](auto& ctx) {
    uint8_t val = _attr(ctx);
    data.push_back(val);
    sum += val;
  };
  auto set_chk = [&](auto& ctx) { sum += _attr(ctx); };
  auto have_data = [&](auto& ctx) { _pass(ctx) = len-- > 0; };
  auto have_all = [&](auto& ctx) { _pass(ctx) = len < 0; };

  //! Start of a packet
  auto preamble = byte_(command::packet_start) >> byte_(command::sys_command);
  //! Anything that is not the start of a packet
  auto junk = *(byte_ - preamble);
  //! The actual data we're looking for
  auto content = *(eps[have_data] >> byte_[add_data]) >> eps[have_all];
  //! The complete packet
  auto packet_rule = junk >> preamble >> byte_[set_mid] >> byte_[set_len] >> content >> byte_[set_chk];

  visitor->stamp = stamp;

  //! Look for messages in the buffer
  if (x3::parse(cur, buffer.end(), packet_rule)) {
    //! Consume the message from the buffer
    buffer.erase(buffer.begin(), cur);
    //! Verify the checksum is 0
    if (sum == 0) {
      //! The content of the message is now in "data"
      auto dcur = data.begin();
      //! We're only interested in data messages
      if (mid == XMID_MtData2) {
        //! Look for data packets in the message content
        if (flip_axes_ ?
              x3::parse(dcur, data.end(), flipped_parser, *data_packets) :
              x3::parse(dcur, data.end(), data_parser, *data_packets)) {
          for (auto& data_packet: *data_packets) {
            //! Visit each packet. The visitor will extract the data from it
            boost::apply_visitor(*visitor, data_packet);
          }
        }
        log(level::debug, "Successfully parsed packet: %", data);
      }
    }
    else {
      log(level::error, "Xsens checksum error: %", sum);
    }
    //! Reset message content
    data_packets->clear();
    return true;
  }
  else {
    return false;
  }
}

void Xsens_parser::parse(const double& stamp) {
  while (parse_single(stamp)) {
    cur = buffer.begin();
  }
}


Stamped_queue& Xsens_parser::get_values() {
  return visitor->values;
}

}  // namespace parser

}  // namespace xsens

// Magic to map parsed values which are stored in fusion::vector to the actual data packet type.
// Macro call is at global scope as stated in the manual. Trivial for anything but the
// date-time values.
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Date_time, nano, year, month, day,
                                                    hour, minute, second, flags)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Acceleration_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Free_acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Free_acceleration_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Rate_of_turn, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Rate_of_turn_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Lat_lon, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Magnetic_flux, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Magnetic_flux_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Velocity, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Velocity_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Altitude_ellipsoid, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Altitude_msl, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Euler_angles, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Euler_angles_flipped, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Quaternion, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Quaternion_flipped, data)

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
