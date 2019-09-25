/**
 * \file functions.h
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


#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "tools.h"
#include "types.h"
#include "quantities.h"

#include <string>

#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace gregorian = boost::gregorian;

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

#endif
