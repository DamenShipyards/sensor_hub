/**
 * \file acceleration_history.h
 * \brief Provide acceleration history processor
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



#ifndef ACCELERATION_HISTORY_H_
#define ACCELERATION_HISTORY_H_

#include "../processor.h"
#include "../functions.h"


struct Acceleration_peak {
  //! Start of peak
  double start;
  //! Duration of peak
  double duration;
  //! Absolute maximum value during peak
  double peak;
  //! Mean value during peak
  double mean;
  //! Rms of values during peak
  double rms;

  double operator[] (const size_t index) const {
    switch (index) {
      case 0:
        return start;
      case 1:
        return duration;
      case 2:
        return peak;
      case 3:
        return mean;
      case 4:
        return rms;
    }
    return 0;
  }

  static constexpr size_t size() {
    return 5;
  }

  enum field_t {
    f_start, f_duration, f_peak, f_mean, f_rms
  };
};


using Acceleration_peaks = std::deque<Acceleration_peak>;


struct Acceleration_history: public Processor {
  static constexpr unsigned x_dir = 1;
  static constexpr unsigned y_dir = 2;
  static constexpr unsigned z_dir = 4;

  Acceleration_history()
    : current_(), peaks_(), 
      value_threshold_(1.0), duration_threshold_(1.0), item_count_(10), direction_(x_dir | y_dir), 
      fax_(), fay_(), faz_() {}


  void insert_value(const Stamped_quantity& value) override {
    if ((direction_ & x_dir) && (value.quantity == Quantity::fax)) {
      if (fax_.stamp != 0) {
        handle_value();
      } 
      fax_ = value;
    } 
    else if ((direction_ & y_dir) && (value.quantity == Quantity::fay)) {
      if (fay_.stamp != 0) {
        handle_value();
      }
      fay_ = value;
    }
    else if ((direction_ & z_dir) && (value.quantity == Quantity::faz)) {
      if (faz_.stamp != 0) {
        handle_value();
      }
      faz_ = value;
    }
  }

  double operator[](const size_t index) override {
    size_t i = index / Acceleration_peak::size();
    size_t m = index % Acceleration_peak::size();
    if (i < peaks_.size()) {
      return peaks_[i][m];
    }
    return 0;
  }

  std::string get_json() const override;
  uint16_t get_modbus_reg(size_t index, const Base_scale& scaler) const override;

  size_t size() override {
    return Acceleration_peak::size() * peaks_.size();
  }

  void set_param(const std::string& name, const double& value) override { 
    if (name == "value_threshold") {
      value_threshold_ = value;
      log(level::info, "Setting value_threshold to % for %", value_threshold_, get_name());
    }
    else if (name == "duration_threshold") {
      duration_threshold_ = value;
      log(level::info, "Setting duration_threshold to % for %", duration_threshold_, get_name());
    }
    else if (name == "item_count") {
      item_count_ = static_cast<size_t>(value);
      log(level::info, "Set item_count to % for %", item_count_, get_name());
    }
    else if (name == "direction") {
      direction_ = static_cast<unsigned>(value);
      log(level::info, "Set direction to % for %", direction_, get_name());
    }
  }

private:
  void handle_value() {
    double amp = 0;
    double sqamp = 0;
    switch (direction_) {
      case x_dir:
        amp = fax_.value;
        sqamp = sqr(amp);
        break;
      case y_dir:
        amp = fay_.value;
        sqamp = sqr(amp);
        break;
      case z_dir:
        amp = faz_.value;
        sqamp = sqr(amp);
        break;
      default:
        sqamp = sqr(fax_.value) + sqr(fay_.value) + sqr(faz_.value);
        amp = sqrt(sqamp);
    }
    double stamp = std::max({fax_.stamp, fay_.stamp, faz_.stamp});
    double aamp = fabs(amp);

    if (aamp > value_threshold_) {
      if (current_.start == 0) {
        current_.start = stamp;
        current_.duration = 0;
        current_.peak = amp;
        current_.mean = amp;
        current_.rms = sqamp;
      }
      else {
        if (aamp > fabs(current_.peak)) {
          current_.peak = amp;
        }

        double new_duration = stamp - current_.start;
        double interval = new_duration - current_.duration;
        if (new_duration > 0) {
          current_.duration = new_duration;
          current_.mean += (amp - current_.mean) * interval / new_duration;
          current_.rms += (sqamp - current_.rms) * interval / new_duration;
        }
      }
    }
    else {
      if (current_.duration > duration_threshold_) {
        current_.rms = sqrt(current_.rms);
        peaks_.push_front(current_);
        while (peaks_.size() > item_count_) {
          peaks_.pop_back();
        }
      }
      current_ = peak_zero;
    }
    fax_ = zero;
    fay_ = zero;
    faz_ = zero;
  }

  Acceleration_peak current_;
  Acceleration_peaks peaks_;
  double value_threshold_;
  double duration_threshold_;
  size_t item_count_;
  unsigned direction_;

  static constexpr Acceleration_peak peak_zero{0, 0, 0, 0, 0};
  static constexpr Stamped_value zero{0, 0};
  Stamped_value fax_;
  Stamped_value fay_;
  Stamped_value faz_;
};

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
