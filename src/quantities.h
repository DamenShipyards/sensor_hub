/**
 * \file quantities.h
 * \brief Provide centralized quantity information
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef QUANTITIES_H_
#define QUANTITIES_H_

#include <exception>
#include <sstream>
#include <utility>

#include "tools.h"

//! Storage type for quantity values
using Value_type = double;

//! Exception to be thrown when a quantity is not available or supported
struct Quantity_not_available: public std::exception {
};

/**
 * \brief Enumeration of available quantities
 *
 * Note the following ship and earth related conventions:
 * With respect to ships, the X coordinate is longitudinal and positive
 * pointing to the bow. Y coordinate is transerse and positive 
 * pointing to starboard. Z coordinate is vertical and positive pointing
 * down.
 * With respect to the earth, the X axis points north, the Y axis east
 * and the Z axis down.
 * All values are in SI units, angles are in radians.
 */
enum class Quantity {
  ut,  ///<  0: Unix/POSIX time
  la,  ///<  1: Latitude with respect to WGS84 ellipsoid (GPS)
  lo,  ///<  2: Longitude with respect to WGS84 ellipsoid (GPS)
  h1,  ///<  3: Height with respect to WGS84 ellipsoid (GPS)
  h2,  ///<  4: Height with respect to MSL/EGM2008
  vog, ///<  5: Absolute value of speed vector over ground
  vtw, ///<  6: Absolute value of speed vector through water
  hdg, ///<  7: Heading, angle of plain through X axes and vertical with respect to true north
  crs, ///<  8: Angle of velocity over ground vector with respect to true north
  mn,  ///<  9: Angle of magnetic north with respect to plane through X axis and vertical
  mx,  ///< 10: X component of magnetic flux vector
  my,  ///< 11: Y component of magnetic flux vector
  mz,  ///< 12: Z component of magnetic flux vector
  x,   ///< 13: X position with respect to some reference point
  y,   ///< 14: Y position with respect to some reference point
  z,   ///< 15: Z position with respect to some reference point
  vx,  ///< 16: X component of velocity
  vy,  ///< 17: Y component of velocity
  vz,  ///< 18: Z component of velocity
  ax,  ///< 19: X component of acceleration
  ay,  ///< 20: Y component of acceleration
  az,  ///< 21: Z component of acceleration
  ro,  ///< 22: Roll, rotation about X axis with respect to some reference
  pi,  ///< 23: Pitch, rotation about Y axis with respect to some reference
  ya,  ///< 24: Yaw, rotation about Z axis with respect to some reference
  q1,  ///< 25: Orientation quaternion component 1
  q2,  ///< 26: Orientation quaternion component 2
  q3,  ///< 27: Orientation quaternion component 3
  q4,  ///< 28: Orientation quaternion component 4
  rr,  ///< 29: Roll rate, angular velocity about X axis
  pr,  ///< 30: Pitch rate, angular velocity about Y axis
  yr,  ///< 31: Yaw rate, angular velocity about Z axis
  fax, ///< 32: X component of free acceleration, acceleration with respect to earth surface
  fay, ///< 33: Y component of free acceleration, acceleration with respect to earth surface
  faz, ///< 34: Z component of free acceleration, acceleration with respect to earth surface
  end, ///< Enumeration end marker. Do not use
};

using Quantity_iter = Enum_iter<Quantity>;
using Quantity_type = std::underlying_type<Quantity>::type;

//! Name trait for #Quantity enum
template <Quantity quantity> struct Quantity_name { };

//! Macro for adding quantity name traits DRY style.
#define QUANTITY_NAME(NAME) template <> struct Quantity_name<Quantity::NAME> { static inline constexpr char value[] = #NAME; }
QUANTITY_NAME(ut);
QUANTITY_NAME(la);
QUANTITY_NAME(lo);
QUANTITY_NAME(h1);
QUANTITY_NAME(h2);
QUANTITY_NAME(vog);
QUANTITY_NAME(vtw);
QUANTITY_NAME(hdg);
QUANTITY_NAME(crs);
QUANTITY_NAME(mn);
QUANTITY_NAME(mx);
QUANTITY_NAME(my);
QUANTITY_NAME(mz);
QUANTITY_NAME(x);
QUANTITY_NAME(y);
QUANTITY_NAME(z);
QUANTITY_NAME(vx);
QUANTITY_NAME(vy);
QUANTITY_NAME(vz);
QUANTITY_NAME(ax);
QUANTITY_NAME(ay);
QUANTITY_NAME(az);
QUANTITY_NAME(ro);
QUANTITY_NAME(pi);
QUANTITY_NAME(ya);
QUANTITY_NAME(q1);
QUANTITY_NAME(q2);
QUANTITY_NAME(q3);
QUANTITY_NAME(q4);
QUANTITY_NAME(rr);
QUANTITY_NAME(pr);
QUANTITY_NAME(yr);
QUANTITY_NAME(fax);
QUANTITY_NAME(fay);
QUANTITY_NAME(faz);

template <Quantity quantity>
constexpr inline decltype(auto) get_quantity_name() {
  return get_enum_trait<Quantity, Quantity_name, quantity>();
}

constexpr auto q_end = static_cast<Quantity_type>(Quantity::end);
using Quantity_sequence = std::make_integer_sequence<Quantity_type, q_end>;
constexpr auto quantity_sequence = Quantity_sequence();


template <typename T>
constexpr inline const char* get_quantity_name_impl(Quantity quantity) {
  return "";
}

template <typename T, Quantity_type q2, Quantity_type... qs>
constexpr inline const char* get_quantity_name_impl(Quantity quantity) {
  if (static_cast<Quantity_type>(quantity) == q2) {
    return get_quantity_name<static_cast<Quantity>(q2)>();
  }
  else {
    return get_quantity_name_impl<int, qs...>(quantity);
  }
}

template <Quantity_type... qs>
constexpr inline const char* get_quantity_name(Quantity quantity, const std::integer_sequence<Quantity_type, qs...>) {
  return get_quantity_name_impl<int, qs...>(quantity);
}

constexpr inline const char* get_quantity_name(Quantity quantity) {
  return get_quantity_name(quantity, quantity_sequence);
}

struct Data_quantity {
  Quantity quantity;
};

struct Data_stamp {
  Value_type stamp;
};

struct Data_value {
  Value_type value;
};

struct Quantity_value: public Data_quantity, public Data_value {};
struct Stamped_value: public Data_stamp, public Data_value {};
struct Stamped_quantity: public Data_quantity, public Stamped_value {};

inline Stamped_quantity stamped_quantity(double stamp, Quantity_value&& qv) {
  return {qv.quantity, stamp, qv.value};
}
inline Stamped_quantity stamped_quantity(double stamp, const Quantity_value& qv) {
  return {qv.quantity, stamp, qv.value};
}

inline std::ostream& operator<<(std::ostream& os, Quantity quantity) {
  os << get_quantity_name(quantity);
  return os;
}

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
