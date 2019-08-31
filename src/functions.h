/**
 * \file functions.h
 * \brief Provide centralized quantity information
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "tools.h"
#include "types.h"
#include "quantities.h"

#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace gregorian = boost::gregorian;

inline double compose_time_value(
    const int year, const int month, const int day,
    const int hour, const int minute, const int second,
    const int nanosecond) {
  using namespace posix_time;
  ptime t(
      gregorian::date(year, month, day),
      hours(hour) + minutes(minute) + seconds(second) + microseconds(nanosecond/1000)
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
