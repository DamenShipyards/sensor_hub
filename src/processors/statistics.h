/**
 * \file statistics.h
 * \brief Provide statistics processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_


#include "../processor.h"


struct Statistic {
  //! Time of reception of last sample
  double time;
  //! Number of samples in statistic
  int n;
  //! Mean value of samples
  double mean;
  //! RMS value of samples with respect to mean
  double variance;

  double operator[] (const size_t index) const {
    switch (index) {
      case 0:
        return time;
      case 1:
        return n;
      case 2:
        return mean;
      case 3:
        return sqrt(variance);
    }
    return 0;
  }

  static constexpr size_t size() {
    return 4;
  }

  enum {
    f_time, f_n, f_mean, f_stddev
  } field_t;
};


using Statistic_map = std::map<Quantity, Statistic>;


struct Statistics: public Processor {
  Statistics(): Processor(), data_(), statistics_(), period_(1.0), filter_() {}

  void insert_value(const Stamped_quantity& value) override {
    Quantity quantity = value.quantity;

    if (!filter_.empty() && filter_.find(quantity) == filter_.end())
      // We have a filter and it doesn't contain quantity: ignore this value
      return;

    auto item = data_.try_emplace(quantity);
    auto& list = item.first->second;
    auto stat_item = statistics_.try_emplace(quantity);
    auto& stat = stat_item.first->second;

    // Initialize the statistic
    if (list.empty()) {
      stat.n = 1;
      stat.mean = value.value;
      stat.variance = 0;
      list.push_back(value);
      return;
    }

    // Update mean and variance with the new value
    double span = list.back().stamp - list.front().stamp;
    double interval = value.stamp - list.back().stamp;
    // Require stricly increasing sample times
    if (interval <= 0)
      return;

    // As value we won't use the sample value itself, but the average over the last interval i.e.
    // 0.5 * (new_sample + previous_sample). 
    // This is implemented as:
    // new_sample - 0.5 * (new_sample - previous_sample)
    // to avoid funny business with angles. Consider 0.5*(359+1) vs. 359-0.5*(value_diff(359,1)=-2)!
    double avg = value_norm(quantity, value.value - 0.5 * value_diff(value, list.back().value));
    double old_mean = stat.mean;
    stat.mean = value_norm(quantity, old_mean + 
        value_diff(quantity, avg, old_mean) * interval / (interval + span));
    double mean_shift_2 = sqr(value_diff(quantity, old_mean, stat.mean));
    double mean_diff_2 = sqr(value_diff(quantity, avg, stat.mean));
    stat.variance = (span * (stat.variance + mean_shift_2) + interval * mean_diff_2) / (span + interval); 
    list.push_back(value);

    // Drop values from the back that have expired "period"
    while ((value.stamp - list.front().stamp) > period_) {
      Stamped_value popped = list.front();
      list.pop_front();
      // Initialize the statistic
      if (list.size() == 1) {
        stat.n = 1;
        stat.mean = value.value;
        stat.variance = 0;
        return;
      }

      // Reverse the effect of the dropped value on the mean and variance
      span = list.back().stamp - list.front().stamp;
      interval = list.front().stamp - popped.stamp;
      avg = value_norm(quantity, popped.value - 0.5 * value_diff(quantity, popped.value, list.front().value));
      old_mean = stat.mean;
      // We can be sure "span" won't be zero because at this point there are at least two items in the
      // list with stricly increasing time stamps
      stat.mean = value_norm(quantity, old_mean - value_diff(quantity, avg, old_mean) * interval / span);
      mean_shift_2 = sqr(value_diff(quantity, old_mean, stat.mean));
      mean_diff_2 = sqr(value_diff(quantity, avg, old_mean));
      stat.variance = ((span + interval) * stat.variance - interval * mean_diff_2) / span - mean_shift_2;
    }
    // Keep a record of the number of samples
    stat.n = static_cast<int>(list.size());
    stat.time = value.stamp;
  }

  double operator[](size_t index) override {
    size_t q = index / Statistic::size();
    size_t m = index % Statistic::size();
    auto qit = statistics_.find(static_cast<Quantity>(q));
    if (qit == statistics_.end())
      return 0;
    return qit->second[m];
  }

  std::string get_json() const override;
  uint16_t get_modbus_reg(size_t index, const Base_scale& scaler) const override;

  size_t size() override {
    return Statistic::size() * static_cast<size_t>(Quantity::end);
  }

  void set_param(const std::string& name, const double& value) override { 
    if (name == "period") {
      period_ = value;
      log(level::info, "Set period to % for %", value, get_name());
    }
  }

  void set_filter(const std::string& filter) override {
    std::vector<std::string> quantities;
    log(level::info, "Set filter to % for %", filter, get_name());
    boost::split(quantities, filter, [](const char c) { return c == ','; });
    for (auto& quantity_str: quantities) {
      Quantity quantity = get_quantity(quantity_str);
      if (quantity != Quantity::end)
        filter_.insert(quantity );
    }
  }

private:
  Data_list_map data_;
  Statistic_map statistics_;
  double period_;
  std::set<Quantity> filter_;
};

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
