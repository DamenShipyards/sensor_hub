/**
 * \file processor.h
 * \brief Provide device data processors
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include "quantities.h"
#include "log.h"
#include "tools.h"

#include <memory>
#include <vector>



struct Processor {
  Processor(): name_() {}

  virtual void insert_value(const Stamped_quantity& value) {
  }

  virtual double operator[](int index) {
    return 0;
  }

  virtual std::string get_json() {
    return "{}";
  }

  virtual size_t size() {
    return 0;
  }

  std::string get_name() {
    return name_;
  }

  void set_name(const std::string& name) {
    name_ = name;
  }

private:
  std::string name_;
};


using Processor_ptr = std::shared_ptr<Processor>;


struct Statistic {
  int n;
  double mean;
  double variance;
};


using Statistic_map = std::map<Quantity, Statistic>;


struct Statistics: public Processor {
  Statistics(): Processor(), data_(), statistics_(), period_(1.0) {}

  void insert_value(const Stamped_quantity& value) override {
    Quantity quantity = value.quantity;
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
    stat.mean = value_norm(quantity, old_mean + value_diff(quantity, avg, old_mean) * interval / (interval + span));
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
    stat.n = list.size();
  }

  double operator[](int index) override {
    int q = index / 3;
    int m = index % 3;
    auto qit = statistics_.find(static_cast<Quantity>(q));
    if (qit == statistics_.end())
      return 0;
    switch (m) {
      case 0:
        return qit->second.n;
      case 1:
        return qit->second.mean;
      case 2:
        return qit->second.variance;
      default:
        return 0;
    }
  }

  std::string get_json() override {
    return "{}";
  }

  size_t size() override {
    return 3 * static_cast<size_t>(Quantity::end);
  }

private:
  Data_list_map data_;
  Statistic_map statistics_;
  double period_;
};


struct Acceleration_peak {
  double mean;
  double standard_deviation;
  double duration;
  double peak;
};


using Acceleration_peaks = std::vector<Acceleration_peak>;


struct Acceleration_peak_history: public Processor {
  void insert_value(const Stamped_quantity& value) override {
  }

  double operator[](int index) override {
    return 0;
  }

  std::string get_json() override {
    return "{}";
  }

  size_t size() override {
    return 0;
  }
private:
};


//! Container with pointers to processors
using Processors = std::vector<Processor_ptr>;

/**
 * Base class for a processor factory
 */
struct Processor_factory_base {
  virtual Processor_ptr get_instance() {
    return Processor_ptr(nullptr);
  }
};

/**
 * Processor factory for "ProcessorType"
 */
template <class ProcessorType>
struct Processor_factory: public Processor_factory_base {
  typedef ProcessorType processor_type;
  Processor_ptr get_instance() override {
    return std::make_shared<processor_type>();
  }
};

//! Unique pointer to a processor factory
using Processor_factory_ptr = std::unique_ptr<Processor_factory_base>;

/**
 * Add a processor factory to the global processor factory registry
 */
extern Processor_factory_ptr& add_processor_factory(const std::string& name, Processor_factory_ptr&& processor_factory);

/**
 * Have a factory from the factory registry create a processor instance
 */
extern Processor_ptr create_processor(const std::string& name);

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
