/**
 * \file processor.h
 * \brief Provide interface for device data processors
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


#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include "quantities.h"
#include "log.h"
#include "tools.h"

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

namespace prtr = boost::property_tree;

struct Scale {
  double min;
  double max;
  double multiplier;
  double offset;
  bool overflow;
  bool signed_type;
};

template<typename T>
static T get_def_config(Quantity q, const std::string& type, const T def) {
  (void)q;
  (void)type;
  return def;
}

template<typename T>
static constexpr double type_range() {
  return std::numeric_limits<T>::max() - std::numeric_limits<T>::lowest() + 1;
}

template<typename T>
static constexpr T top_bit() {
  return 1 << (std::numeric_limits<T>::digits - 1);
}

struct Base_scale {

  Base_scale(const prtr::ptree& config): scale_() {
    for (Quantity_iter qi = Quantity_iter::begin(); qi != Quantity_iter::end(); ++qi) {
      std::string quant = get_quantity_name(*qi);
      double min = config.get(fmt::format("{}_min", quant), get_def_config(*qi, "min", -32768.0));
      double max = config.get(fmt::format("{}_max", quant), get_def_config(*qi, "max",  32768.0));
      double multiplier = config.get(fmt::format("{}_scale", quant),  get_def_config(*qi, "scale", 0));
      double offset = config.get(fmt::format("{}_offset", quant),  get_def_config(*qi, "offset", 0));
      bool overflow = config.get(fmt::format("{}_overflow", quant), get_def_config(*qi, "overflow", false));
      bool signed_type = config.get(fmt::format("{}_signed", quant), get_def_config(*qi, "signed", multiplier != 0));
      scale_[*qi] = {min, max, multiplier, offset, overflow, signed_type};
    }
  }

  template<typename T>
  typename std::enable_if<!std::numeric_limits<T>::is_signed, T>::type scale_to(Quantity quantity, double value) const {
    try {
      const Scale& scale = scale_.at(quantity);
      T result = 0;

      double min = scale.min;
      double max = scale.max;

      if (scale.multiplier != 0) {
        double range = type_range<T>() / scale.multiplier;
        min = scale.offset - range / 2.0;
        max = scale.offset + range / 2.0;
      }

      if (!scale.overflow) {
        value = std::max(min, value);
        value = std::min(max, value);
      }

      value -= min;
      value /= max - min;
      value *= type_range<T>();
      result = static_cast<T>(value);
      if (scale.signed_type) {
        result ^= top_bit<T>();
      }
      return result;
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
