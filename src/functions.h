/**
 * \file functions.h
 * \brief Provide centralized quantity information
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


#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "quantities.h"
#include "tools.h"
#include "types.h"

#include <string>

#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace gregorian = boost::gregorian;

constexpr Value_type earth_pol_radius = 6356752.3;
constexpr Value_type earth_eq_radius = 6378137.0;
constexpr Value_type earth_gravity = 9.80665;

inline double compose_time_value(
    const int year, const int month, const int day,
    const int hour, const int minute, const int second,
    const int nanosecond) {
  pt::ptime t(
      gregorian::date(year, month, day),
      pt::hours(hour) + pt::minutes(minute) + pt::seconds(second) 
      + pt::microseconds(nanosecond/1000)
  );
  // Return a Unix Time value
  return 1E-6 * (t - unix_epoch).total_microseconds();
}

inline Quantity_value compose_time_quantity(
    const int year, const int month, const int day,
    const int hour, const int minute, const int second,
    const int nanosecond) {
  // Return a Unix Time quantity value
  return Quantity_value{compose_time_value(year, month, day, hour, minute, second, nanosecond),
                        Quantity::ut};
}


constexpr Value_type sqr(const Value_type value) {
  return value * value;
}


constexpr Value_type deg_to_rad(const Value_type value) {
  return value * M_PI / 180.0;
}


constexpr Value_type rad_to_deg(const Value_type value) {
  return value * 180.0 / M_PI;
}

#ifdef _MSC_VER
#define CONSTRET inline const
#else
#define CONSTRET constexpr
#endif

CONSTRET Value_type get_dx_dla(const Value_type la) {
  Value_type phi = atan(earth_pol_radius / earth_eq_radius * tan(la));
  return earth_eq_radius * cos(phi);
}


CONSTRET Value_type get_dy_dlo(const Value_type la) {
  Value_type phi = atan(earth_pol_radius / earth_eq_radius * tan(la));
  Value_type result = sqrt(sqr(earth_eq_radius) * sqr(sin(phi)) + sqr(earth_pol_radius) * sqr(cos(phi)));
  result *= earth_eq_radius * earth_pol_radius /
            ((sqr(earth_eq_radius) - sqr(earth_pol_radius)) * sqr(cos(la))+ sqr(earth_pol_radius));
  return result;
}

CONSTRET Value_type get_earth_gravity(const Value_type la) {
  // WGS84 earth gravity
  return 9.7803253359 * ((1 + 0.00193185265241 * sqr(sin(la))) / sqrt(1 - 0.00669437999013 * sqr(sin(la))));
}

#endif
