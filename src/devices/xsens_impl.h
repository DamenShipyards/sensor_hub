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

#include "../datetime.h"
#include "../types.h"
#include "../spirit_x3.h"
#include "../functions.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace xsens {

namespace command {

constexpr cbyte_t packet_start = 0xFA;
constexpr cbyte_t sys_command = 0xFF;
constexpr cbyte_t conf_command = 0x01;

// TODO: Should be reworked to avoid duplication and add automatic checksum
cbytes_t goto_config = {packet_start, sys_command, XMID_GotoConfig, 0x00, 0xD1};
cbytes_t config_ack = {packet_start, sys_command, XMID_GotoConfigAck};

cbytes_t goto_measurement = {packet_start, sys_command, XMID_GotoMeasurement, 0x00, 0xF1};
cbytes_t measurement_ack = {packet_start, sys_command, XMID_GotoMeasurementAck};

cbytes_t set_option_flags = {packet_start, sys_command, XMID_SetOptionFlags,
  0x08,
  0x00, 0x00, 0x00,       // Option flags to set:
  0x00 |
  // 0x01 |               // Disable auto store
  ///0x02 |               // Disable auto measurement
  // 0x04 |               // EnableBeidou (instead of GLONASS)
  //0x10 |                  // EnableAHS (relate Yaw only, no heading)
  0x80 |                  // EnableInRunCompassCalibration
  0x00,
  0x00, 0x00, 0x00,       // Option flags to clear
  0x00 |
  // 0x02 |               // Clear: Disable auto measurement
  0x10 |                  // Clear: EnableAHS (apparently doesn't work well with MTi-G-710)
                          //        see: https://base.xsens.com/hc/en-us/requests/1182
  0x00,
  0x21                    // Checksum
};
cbytes_t option_flags_ack = {packet_start, sys_command, XMID_SetOptionFlagsAck};

cbytes_t req_reset = {packet_start, sys_command, XMID_Reset, 0x00, 0xC1};
cbytes_t reset_ack = {packet_start, sys_command, XMID_ResetAck, 0x00, 0xC0};

cbytes_t req_device_id = {packet_start, sys_command, XMID_ReqDid, 0x00, 0x01};
cbytes_t device_id_resp = {packet_start, sys_command, XMID_DeviceId};

cbytes_t init_mt = {packet_start, sys_command, XMID_Initbus, 0x00, 0xFF};
cbytes_t mt_ack = {packet_start, sys_command, XMID_InitBusResults};

cbytes_t wakeup = {packet_start, sys_command, XMID_Wakeup, 0x00, 0xC3};
cbytes_t wakeup_ack = {packet_start, sys_command, XMID_WakeupAck, 0x00, 0xC2};

cbytes_t req_product_code = {packet_start, sys_command, XMID_ReqProductCode, 0x00, 0xE5};
cbytes_t product_code_resp = {packet_start, sys_command, XMID_ProductCode};

cbytes_t req_firmware_rev = {packet_start, sys_command, XMID_ReqFirmwareRevision, 0x00, 0xEF};
cbytes_t firmware_rev_resp = {packet_start, sys_command, XMID_FirmwareRevision};

cbytes_t error_resp = {packet_start, sys_command, XMID_Error, 0x01};

cbytes_t set_string_output_type = {packet_start, sys_command, XMID_SetStringOutputType,
  0x02,
  0x00, 0x00,
  0x71};
cbytes_t string_output_type_ack = {packet_start, sys_command, XMID_SetStringOutputTypeAck};


cbytes_t get_output_configuration = {packet_start, sys_command, XMID_ReqOutputConfiguration, 0x00, 0x41};
cbytes_t get_output_configuration_ack = {
  packet_start, sys_command,
  XMID_ReqOutputConfigurationAck,
  0x2C,                   // Length
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
  0x20, 0x10, 0x00, 0x0A, // Quaternion, 10 Hz
  0x71                    // Checksum
};

cbytes_t set_output_configuration = {
  packet_start, sys_command,
  XMID_SetOutputConfiguration,
  0x2C,                   // Length
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
  0x20, 0x10, 0x00, 0x0A, // Quaternion, 10 Hz
  0x72                    // Checksum
};

cbytes_t output_configuration_ack = {
  packet_start, sys_command,
  XMID_SetOutputConfigurationAck
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
 * The default multiplier does nothing but flip the signs of Y and
 * Z axis values to from Z up to Z down axis.
 *
 * @tparam DIM number of dimensions in value to convert.
 */
template<int DIM>
struct IdentityConverter {
  static constexpr double factor(int dim, double f=1.0) {
    return (DIM == 3) && (dim > 0) ? -f :
           (DIM == 4) && (dim > 1) ? -f : f;
  }
};


/**
 * Deg to rad converter
 *
 * Conversion factor for angles provided by the sensor in angular 
 * degrees to radians.
 * 
 * @tparam DIM number of dimensions in value to convert
 */
template<int DIM>
struct RadConverter: IdentityConverter<DIM> {
  static constexpr double factor(int dim) {
    return IdentityConverter<DIM>::factor(dim, M_PI / 180.0);
  }
};


/**
 * Magnetic field strength converter
 *
 * Conversion factor for converting fields strengths 
 * in Gauss to Tesla.
 *  
 * @tparam DIM number of dimensions in value to convert
 */
template<int DIM>
struct TeslaConverter: IdentityConverter<DIM> {
  static constexpr double factor(int dim) {
    return IdentityConverter<DIM>::factor(dim, 1E-4);
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
  virtual Values_type get_values() const {
    return Values_type();
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

  Values_type get_values() const override {
    using namespace posix_time;
    if (flags & valid_utc) {
      // Return a list with one Unix Time value
      return Values_type{compose_time(year, month, day, hour, minute, second, nano)};
    }
    else {
      // Return an empy list
      return Values_type();
    }
  }

  static const auto get_parse_rule() {
    return big_word(did) >> byte_(len) >> big_dword >> big_word >> byte_ >> byte_ >> byte_ >> byte_ >> byte_ >> byte_;
  }
};


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
 * @tparam Converter Converion factor provider for this data packet
 */
template<uint16_t DID, uint16_t COORD, uint16_t FORMAT, int DIM, Quantity QUANT,
         template<int D> typename Converter=IdentityConverter>
struct Data_value: public Data_packet {
  typedef Converter<DIM> converter;
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
  Values_type get_values() const override {
    Values_type result;
    int dim = 0;
    Quantity_iter qi(quantity);
    for (auto& value: data) {
      // When a data packet contains multiple values, the quantities of these
      // values are always consecutive, so we can just increment the quantity
      result.push_back({*qi++, converter::factor(dim++) * static_cast<double>(value)});
    }
    return result;
  }

  /**
   * Get parse rule for single precision data
   */
  template <uint16_t PFORMAT = FORMAT>
  static const auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatFloat, int>::type n=0) {
    return big_word(did) >> byte_(DIM * 4) >> repeat(DIM)[big_bin_float];
  }

  /**
   * Get parse rule for double precision data
   */
  template <uint16_t PFORMAT = FORMAT>
  static const auto get_parse_rule(typename std::enable_if<PFORMAT == XDI_SubFormatDouble, int>::type n=0) {
    return big_word(did) >> byte_(DIM * 8) >> repeat(DIM)[big_bin_double];
  }

  // Other formats are not currently supported
};


// Define specific data packet type for each data type received from sensor
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

/**
 * Parse rule for data packets
 */
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

/**
 * Visitor for accessing parsed data packets
 */
struct Xsens_parser::Data_visitor {

  Values_queue values;

  void operator()(const Data_packet& data_packet) {
    for (auto& value: data_packet.get_values()) {
      values.push_back(value);
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


void Xsens_parser::parse() {
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

  cur = queue.begin();
  //! Look for messages in the queue
  while (cur != queue.end() && x3::parse(cur, queue.end(), packet_rule)) {
    //! Consume the message from the queue
    cur = queue.erase(queue.begin(), cur);
    //! Verify the checksum is 0
    if (sum == 0) {
      //! The content of the message is now in "data"
      auto dcur = data.begin();
      //! We're only interested in data messages
      if (mid == XMID_MtData2) {
        //! Look for data packets in the message content
        if (x3::parse(dcur, data.end(), data_parser, *data_packets)) {
          for (auto& data_packet: *data_packets) {
            //! Visit each packet. The visitor will extract the data from it
            boost::apply_visitor(*visitor, data_packet);
          }
        }
      }
    }
    else {
      log(level::error, "Xsens checksum error");
    }
    //! Reset message content
    data_packets->clear();
  }
}


Values_queue& Xsens_parser::get_values() {
  return visitor->values;
}

}  // namespace parser

}  // namespace xsens

// Magic to map parsed values which are stored in fusion::vector to the actual data packet type. 
// Macro call is at global scope as stated in the manual. Trivial for anything but the date-time values.
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Date_time, nano, year, month, day, hour, minute, second, flags)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Free_acceleration, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Rate_of_turn, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Lat_lon, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Magnetic_flux, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Velocity, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Altitude_ellipsoid, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Altitude_msl, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Euler_angles, data)
BOOST_FUSION_ADAPT_STRUCT(xsens::parser::Quaternion, data)

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
