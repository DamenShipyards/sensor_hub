/**
 * \file datetime.h
 * \brief Provide interface for time handling
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include <string>
#include <cmath>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/chrono.hpp>

/**
 * Adjust the clock by 2.5% of clock difference with each call to "Clock::adjust"
 *
 * This value is somewhat arbitrary, but is intended to move the clock
 * towards the desired time with a reasonable pace while avoiding large
 * clock jumps.
 */
#define DEFAULT_ADJUST_RATE 0.025

namespace chrono = boost::chrono;
namespace posix_time = boost::posix_time;
namespace date_time = boost::date_time;

/**
 * Get UTC unix time stamp from central clock
 */
extern double get_time();
extern const posix_time::ptime unix_epoch; 

/**
 * Adjust central clock
 *
 * \param towards_time UTC unix timestamp to use for adjusting the clock. The clock
 *                     will only be adjusted by a fraction that defaults to 1/40 and
 *                     can be set with set_clock_adjust_rate.
 */
extern void adjust_clock(const double& towards_time);

/**
 * Adjust central clock
 *
 * \param diff Difference between recorded time and system UTC unix timestamp to use for adjusting 
 *             the clock. The clock will only be adjusted by a fraction that defaults to 1/40
 *             and can be set with set_clock_adjust_rate
 */
extern void adjust_clock_diff(const double& diff);


/**
 * Set rate with which to adjust central clock
 *
 * \param rate Rate with which to adjust the clock with each call to adjust_clock(_diff)
 */
extern void set_clock_adjust_rate(const double& rate);


inline std::string timestamp_to_string(const double& stamp) {
  double secs = 0;
  double micros = std::modf(stamp, &secs) * 1E6;
  auto t = unix_epoch + posix_time::seconds(static_cast<long>(secs)) 
                      + posix_time::microseconds(static_cast<long>(micros));
  return posix_time::to_iso_extended_string(t);
}

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
