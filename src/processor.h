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
  double mean;
  double m2;
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
    if (list.empty()) {
      stat.mean = value.value;
      stat.m2 = 0;
      list.push_back(value);
      return;
    }
    double span = list.back().stamp - list.front().stamp;
    double interval = value.stamp - list.back().stamp;
    if (interval <= 0)
      return;
    double avg = value_norm(quantity, value.value - 0.5 * value_diff(value, list.back().value));
    stat.mean = value_norm(quantity, stat.mean + interval / (interval + span) * value_diff(quantity, avg, stat.mean));
    list.push_back(value);
    while ((value.stamp - list.front().stamp) > period_) {
      Stamped_value popped = list.front();
      list.pop_front();
      if (list.size() == 1) {
        stat.mean = value.value;
        stat.m2 = 0;
        return;
      }
      avg = value_norm(quantity, popped.value - 0.5 * value_diff(quantity, popped.value, list.front().value));
      interval = list.front().stamp - popped.stamp;
      span = list.back().stamp - list.front().stamp;
      stat.mean = value_norm(quantity, stat.mean - interval / span * value_diff(quantity, avg, stat.mean));
    }
  }

  double operator[](int index) override {
    int q = index / 2;
    int m = index % 2;
    auto qit = statistics_.find(static_cast<Quantity>(q));
    if (qit == statistics_.end())
      return 0;
    if (m == 0) {
      return qit->second.mean;
    }
    else {
      auto lit = data_.find(static_cast<Quantity>(q));
      if (lit == data_.end()) 
        return 0;
      int s = static_cast<int>(lit->second.size()) - 1;
      if (s <= 0)
        return 0;
      return sqrt(qit->second.m2 / s);
    }
  }

  std::string get_json() override {
    return "{}";
  }

  size_t size() override {
    return 2 * static_cast<size_t>(Quantity::end) - 1;
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
