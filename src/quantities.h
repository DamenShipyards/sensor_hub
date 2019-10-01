/**
 * \file quantities.h
 * \brief Provide centralized quantity information
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


#ifndef QUANTITIES_H_
#define QUANTITIES_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <exception>
#include <sstream>
#include <utility>
#include <map>
#include <deque>
#include <list>

#include "tools.h"
#include "types.h"
#include "datetime.h"


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
  hg84,///<  3: Height with respect to WGS84 ellipsoid (GPS)
  hmsl,///<  4: Height with respect to MSL/EGM2008
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
  du,  ///< 35: Duration, time interval
  hacc,///< 36: Horizontal position accuracy
  vacc,///< 37: Vertical position accuracy
  sacc,///< 38: Speed accuracy
  cacc,///< 39: Course accuracy
  racc,///< 40: Roll accuracy
  pacc,///< 41: Pitch accuraty
  yacc,///< 42: Yaw accuraty
  hdac,///< 43: Heading accuracy
  rax, ///< 44: Raw measurement of X component of acceleration
  ray, ///< 45: Raw measurement of Y component of acceleration
  raz, ///< 46: Raw measurement of Z component of acceleration
  rrr, ///< 47: Raw measurement of Roll rate, angular velocity about X axis
  rpr, ///< 48: Raw measurement of Pitch rate, angular velocity about Y axis
  ryr, ///< 49: Raw measurement of Yaw rate, angular velocity about Z axis
  rmx, ///< 50: Raw measurement of X component of magnetic flux vector
  rmy, ///< 51: Raw measurement of Y component of magnetic flux vector
  rmz, ///< 52: Raw measurement of Z component of magnetic flux vector
  gtmp,///< 53: Gyroscope temperature
  stmp,///< 54: Sensor temperature
  wtmp,///< 55: Water temperature
  atmp,///< 56: Air temperature
  etmp,///< 57: Exhaust temperature
  otmp,///< 58: Oil temperature
  end, ///< Enumeration end marker. Do not use
};

using Quantity_iter = Enum_iter<Quantity>;
using Quantity_type = std::underlying_type<Quantity>::type;

//! Name trait for #Quantity enum
template <Quantity quantity> struct Quantity_name { };

//! Macro for adding quantity name traits DRY style.
#define QUANTITY_NAME(NAME) template <> struct Quantity_name<Quantity::NAME> { static constexpr inline char* value() { return #NAME; } }

QUANTITY_NAME(ut);
QUANTITY_NAME(la);
QUANTITY_NAME(lo);
QUANTITY_NAME(hg84);
QUANTITY_NAME(hmsl);
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
QUANTITY_NAME(hacc);
QUANTITY_NAME(vacc);
QUANTITY_NAME(sacc);
QUANTITY_NAME(cacc);
QUANTITY_NAME(racc);
QUANTITY_NAME(pacc);
QUANTITY_NAME(yacc);
QUANTITY_NAME(hdac);
QUANTITY_NAME(rax);
QUANTITY_NAME(ray);
QUANTITY_NAME(raz);
QUANTITY_NAME(rrr);
QUANTITY_NAME(rpr);
QUANTITY_NAME(ryr);
QUANTITY_NAME(rmx);
QUANTITY_NAME(rmy);
QUANTITY_NAME(rmz);
QUANTITY_NAME(gtmp);
QUANTITY_NAME(stmp);
QUANTITY_NAME(wtmp);
QUANTITY_NAME(atmp);
QUANTITY_NAME(etmp);
QUANTITY_NAME(otmp);
QUANTITY_NAME(du);
#undef QUANTITY_NAME

template <Quantity quantity>
constexpr inline decltype(auto) get_quantity_name() {
  return get_enum_trait<Quantity, Quantity_name, quantity>();
}

constexpr auto q_end = static_cast<Quantity_type>(Quantity::end);
using Quantity_sequence = std::make_integer_sequence<Quantity_type, q_end>;
constexpr auto quantity_sequence = Quantity_sequence();


template <typename T>
constexpr inline const char* get_quantity_name_impl(Quantity) {
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

inline Quantity get_quantity(std::string& quantity_name) {
  for (auto qi = Quantity_iter::begin(); qi != Quantity_iter::end(); ++qi) {
    if (quantity_name == get_quantity_name(*qi)) {
      return *qi;
    }
  }
  return Quantity::end;
}



struct Data_value {
  Data_value(): value() {};
  constexpr Data_value(const Data_value& v): value(v.value) {}
  constexpr Data_value(const Value_type& v): value(v) {}
  Data_value& operator=(const Data_value& other) {
    value = other.value;
    return *this;
  }
  Data_value& operator=(const Value_type& other) {
    value = other;
    return *this;
  }
  bool operator==(const Value_type& other) const {
    return other == this->value;
  }
  bool operator==(const Data_value& other) const {
    return other.value == this->value;
  }
  double operator[](const int index) const {
    return index == 0 ? value : 0.0;
  }
  Value_type value;
};

struct Data_stamp {
  Data_stamp(): stamp() {}
  constexpr Data_stamp(const Data_stamp& s): stamp(s.stamp) {}
  constexpr Data_stamp(const double& s): stamp(s) {}
  Data_stamp& operator=(const Data_stamp& other) {
    stamp = other.stamp;
    return *this;
  }
  Data_stamp& operator=(const double& other) {
    stamp = other;
    return *this;
  }
  bool operator==(const Data_stamp& other) const {
    return other.stamp == this->stamp;
  }
  bool simultaneous_with(const Value_type& stamp) const {
    return stamp == this->stamp;
  }
  bool simultaneous_with(const Data_stamp& stamp) const {
    return stamp == *this;
  }
  double operator[](const int index) const {
    return index == 0 ? stamp : 0.0;
  }
  double stamp;
};

struct Data_quantity {
  Data_quantity(): quantity(Quantity::end) {}
  constexpr Data_quantity(const Data_quantity& q): quantity(q.quantity) {}
  constexpr Data_quantity(const Quantity& q): quantity(q) {}
  Data_quantity& operator=(const Data_quantity& other) {
    quantity = other.quantity;
    return *this;
  }
  Data_quantity& operator=(const Quantity other) {
    quantity = other;
    return *this;
  }
  bool operator==(const Quantity& other) const {
    return other == this->quantity;
  }
  bool operator==(const Data_quantity& other) const {
    return other.quantity == this->quantity;
  }
  double operator[](const int index) const {
    return index == 0 ? static_cast<double>(quantity) : 0.0;
  }
  Quantity quantity;
};

struct Quantity_value: public Data_value, public Data_quantity {
  Quantity_value(): Data_value(), Data_quantity() {}
  constexpr Quantity_value(const Data_value& dv, const Data_quantity& dq):
    Data_value(dv), Data_quantity(dq) {}
  constexpr Quantity_value(const Value_type& v, const Quantity& q):
    Data_value(v), Data_quantity(q) {}
  using Data_value::operator==;
  using Data_quantity::operator==;
  double operator[](const int index) const {
    return index == 0 ? value : index == 1 ? static_cast<double>(quantity) : 0.0;
  }
  bool operator==(const Quantity_value& other) const {
    return Data_quantity::operator==(other)
           && Data_value::operator==(other);
  }
};

struct Stamped_value: public Data_value, public Data_stamp {
  Stamped_value(): Data_value(), Data_stamp() {}
  constexpr Stamped_value(const Data_value& dv, const Data_stamp& ds):
    Data_value(dv), Data_stamp(ds) {}
  constexpr Stamped_value(const Value_type& v, const double& s):
    Data_value(v), Data_stamp(s) {}
  using Data_value::operator==;
  using Data_stamp::operator==;
  double operator[](const int index) const {
    return index == 0 ? value : index == 1 ? stamp : 0.0;
  }
  bool operator==(const Stamped_value& other) const {
    return Data_value::operator==(other) 
           && Data_stamp::operator==(other);
  }
};

struct Stamped_quantity: public Stamped_value, Data_quantity {
  Stamped_quantity(): Stamped_value(), Data_quantity() {}
  constexpr Stamped_quantity(const Stamped_value& sv, const Data_quantity& dq):
    Stamped_value(sv), Data_quantity(dq) {}
  constexpr Stamped_quantity(const Value_type& v, const double& s, const Quantity& q):
    Stamped_value(v, s), Data_quantity(q) {}
  constexpr Stamped_quantity(const double& s, const Quantity_value& qv):
    Stamped_value(qv.value, s), Data_quantity(qv.quantity) {}
  using Stamped_value::operator==;
  using Data_quantity::operator==;
  double operator[](const int index) const {
    return index == 0 ? value : index == 1 ? stamp : index == 2 ? 
      static_cast<double>(quantity) : 0.0;
  }
  bool operator==(const Stamped_quantity& other) const {
    return Data_quantity::operator==(other) 
           && Stamped_value::operator==(other);
  }
};


using Data_queue = std::deque<Stamped_value>;
using Data_map = std::map<Quantity, Data_queue>;
using Data_list = std::list<Stamped_value>;
using Data_list_map = std::map<Quantity, Data_list>;

inline double value_norm(Quantity quantity, double value) {
  switch (quantity) {
    case Quantity::lo:
    case Quantity::ro: 
    case Quantity::pi:
    case Quantity::ya:
      while (value >= M_PI)
        value -= 2 * M_PI;
      while (value < -M_PI)
        value += 2 * M_PI;
      return value;
    case Quantity::hdg:
    case Quantity::crs: 
      while (value >= 2 * M_PI)
        value -= 2 * M_PI;
      while (value < 0)
        value += 2 * M_PI;
      return value;
    default:
      return value;
  }
}

inline double value_diff(Quantity quantity, const double& value1, const double& value2) {
  double result = value1 - value2;
  switch (quantity) {
    case Quantity::lo:
    case Quantity::hdg:
    case Quantity::crs: 
    case Quantity::ro: 
    case Quantity::pi:
    case Quantity::ya:
      while (result >= M_PI)
        result -= 2 * M_PI;
      while (result < -M_PI)
        result += 2 * M_PI;
      return result;
    default:
      return result;
  }
}

inline double value_diff(const Stamped_quantity& qvalue, const double& value) {
  return value_diff(qvalue.quantity, qvalue.value, value);
}


inline std::ostream& operator<<(std::ostream& os, const Quantity quantity) {
  return os << get_quantity_name(quantity);
}

inline std::ostream& operator<<(std::ostream& os, const Data_value& value) {
  return os << value.value;
}

inline std::ostream& operator<<(std::ostream& os, const Data_stamp& stamp) {
  return os << timestamp_to_string(stamp.stamp);
}

inline std::ostream& operator<<(std::ostream& os, const Data_quantity& quantity) {
  return os << quantity.quantity;
}

inline std::ostream& operator<<(std::ostream& os, const Quantity_value& value) {
  return os << static_cast<const Data_value&>(value) << ":" << static_cast<const Data_quantity&>(value);
}

inline std::ostream& operator<<(std::ostream& os, const Stamped_value& value) {
  return os << static_cast<const Data_value&>(value) << "@" << static_cast<const Data_stamp&>(value);
}

inline std::ostream& operator<<(std::ostream& os, const Stamped_quantity& value) {
  return os << static_cast<const Stamped_value&>(value) << ":" << static_cast<const Data_quantity&>(value);
}

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
