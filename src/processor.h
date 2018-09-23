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
  double standard_deviation;
};


using Statistic_map = std::map<Quantity, Statistic>;


struct Statistics: public Processor {
  Statistics(): Processor(), data_(), statistics_() {}
private:
  Data_list_map data_;
  Statistic_map statistics_;
};


struct Acceleration_peak {
  double mean;
  double standard_deviation;
  double duration;
  double peak;
};


using Acceleration_peaks = std::vector<Acceleration_peak>;


struct Acceleration_peak_history: public Processor {
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
