/**
 * \file processor.h
 * \brief Provide interface for device data processors
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
#include <string>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

namespace prtr = boost::property_tree;


struct Scale {
  double min;
  double range;
};


struct Base_scale {

  Base_scale(const prtr::ptree& config): scale_() {
    for (Quantity_iter qi = Quantity_iter::begin(); qi != Quantity_iter::end(); ++qi) {
      double min = config.get(fmt::format("{}_min", get_quantity_name(*qi)), -32.768);
      double max = config.get(fmt::format("{}_max", get_quantity_name(*qi)),  32.768);
      scale_[*qi] = {min, max - min};
    }
  }

  uint16_t scale_to_u16(Quantity quantity, double value) const {
    try {
      const Scale& scale = scale_.at(quantity);
      value -= scale.min;
      value /= scale.range;
      value *= 0x10000;
      return static_cast<uint16_t>(value);
    }
    catch (...) {
      return 0;
    }
  }

  uint32_t scale_to_u32(Quantity quantity, double value) const {
    try {
      const Scale& scale = scale_.at(quantity);
      value -= scale.min;
      value /= scale.range;
      value *= 0x100000000;
      return static_cast<uint32_t>(value);
    }
    catch (...) {
      return 0;
    }
  }

private:
  std::map<Quantity, Scale> scale_;
};


struct Processor {
  Processor(): name_() {}

  virtual void insert_value(const Stamped_quantity&) {
  }

  virtual double operator[](size_t) {
    return 0;
  }

  virtual uint16_t get_modbus_reg(size_t, const Base_scale&) const {
    return 0;
  }

  virtual std::string get_json() const {
    return "{}";
  }

  virtual size_t size() {
    return 0;
  }

  virtual void set_param(const std::string&, const double&) { 
  }

  virtual void set_filter(const std::string&) {
  }

  std::string get_name() const {
    return name_;
  }

  void set_name(const std::string& name) {
    name_ = name;
  }

  void set_params(const std::string& params) {
    std::vector<std::string> fields;
    boost::split(fields, params, [](char c) { return c == ','; });
    for (auto& field: fields) {
      std::vector<std::string> keyval;
      boost::split(keyval, field, [](char c) { return c == '='; });
      if (keyval.size() != 2) {
        log(level::error, "Expected key=value in processor parameters. Got %", field);
        continue;
      }
      try {
        double val = std::stod(keyval[1]);
        set_param(keyval[0], val);
      }
      catch (std::exception& e) {
        log(level::error, "%. Expected floating point argument in processor parameter. Got %.", 
            e.what(), keyval[1]);
        continue;
      }
    }
  }

private:
  std::string name_;
};


using Processor_ptr = std::shared_ptr<Processor>;


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
