/**
 * \file datetime.cpp
 * \brief Provide implementation for time handling
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


#include "datetime.h"
#include "log.h"

#include <cstdint>

/**
 * Singleton clock that serves as central time keeping device
 *
 * Clock that returns a POSIX/unix timestamp (seconds since 1970-01-01 00:00:00.000 UTC)
 * in double format. The clock is monotonous and can be gradually adjusted to indicate
 * a time from another source than the system clock while still keeping the system 
 * clock pace. 
 */
struct Clock {
  Clock(Clock const&) = delete;
  void operator=(Clock const&) = delete;

  static Clock& get_instance() {
    static Clock instance;
    return instance;
  }

  double get_time() {
    set_value(get_clock());
    return value_;
  }

  void adjust(const double& towards_time) {
    adjust_diff(towards_time - get_clock());
  }

  void adjust_diff(const double& diff) {
    offset_ += adjust_rate_ * diff;
  }

  void set_adjust_rate(const double& adjust_rate) {
    adjust_rate_ = adjust_rate;
  }

private:
  Clock(): value_(0), offset_(0), adjust_rate_(DEFAULT_ADJUST_RATE) {
    auto dt_now = date_time::microsec_clock<pt::ptime>::universal_time();
    auto sys_now = chrono::system_clock::now().time_since_epoch();
    
    double secs_since_epoch = to_timestamp(dt_now);
    double sys_since_epoch = static_cast<double>(sys_now.count()) * rate_;
    offset_ = secs_since_epoch - sys_since_epoch;
  }
  double get_sys_clock() {
    auto sys_now = chrono::system_clock::now().time_since_epoch();
    return static_cast<double>(sys_now.count()) * rate_;
  }
  void set_value(const double& value) {
    if (value > value_) {
      // Clock should be monotonous, so never allow a decreasing value
      value_ = value;
    }
  }
  double get_clock() {
    return get_sys_clock() + offset_;
  }
  double value_;
  double offset_;
  double adjust_rate_;
  static const double rate_;
};


const pt::ptime unix_epoch{pt::time_from_string("1970-01-01 00:00:00.000")}; 
const double Clock::rate_{static_cast<double>(chrono::system_clock::duration::period::num) /
                          static_cast<double>(chrono::system_clock::duration::period::den)};


double get_time() {
  return Clock::get_instance().get_time();
}

void adjust_clock(const double& towards_time) {
  Clock::get_instance().adjust(towards_time);
}

void adjust_clock_diff(const double& diff) {
  Clock::get_instance().adjust_diff(diff);
}

void set_clock_adjust_rate(const double& rate) {
  log(level::info, "Setting clock adjust rate to %", rate);
  Clock::get_instance().set_adjust_rate(rate);
}

double to_timestamp(const pt::ptime& time) {
  auto since_epoch =  time - unix_epoch;
  return static_cast<double>(since_epoch.ticks()) / static_cast<double>(since_epoch.ticks_per_second());
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
