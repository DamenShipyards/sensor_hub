/**
 * \file datetime.cpp
 * \brief Provide implementation for time handling
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#include "datetime.h"

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
    set_value(get_sys_clock() + offset_);
    return value_;
  }
  void adjust(const double& towards_time) {
    double diff = towards_time - get_time();
    offset_ += adjust_rate_ * diff;
  }
  void set_adjust_rate(const double& adjust_rate) {
    adjust_rate_ = adjust_rate;
  }
private:
  Clock(): value_(0), offset_(0), adjust_rate_(0.025) {
    auto dt_now = date_time::microsec_clock<posix_time::ptime>::universal_time();
    auto sys_now = chrono::system_clock::now().time_since_epoch();
    
    auto since_epoch =  dt_now - epoch_;
    double secs_since_epoch = static_cast<double>(since_epoch.ticks()) / static_cast<double>(since_epoch.ticks_per_second());
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
  double value_;
  double offset_;
  double adjust_rate_;
  static const double rate_;
  static const posix_time::ptime epoch_;
};


const posix_time::ptime Clock::epoch_{boost::posix_time::time_from_string("1970-01-01 00:00:00.000")}; 
const double Clock::rate_{static_cast<double>(chrono::system_clock::duration::period::num) /
                          static_cast<double>(chrono::system_clock::duration::period::den)};


double get_time() {
  return Clock::get_instance().get_time();
}

void adjust_clock(const double& towards_time) {
  Clock::get_instance().adjust(towards_time);
}

void set_adjust_rate(const double& adjust_rate) {
  Clock::get_instance().set_adjust_rate(adjust_rate);
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
