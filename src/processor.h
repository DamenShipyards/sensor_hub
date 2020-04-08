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

  virtual void set_param(const std::string&, const std::string&) {
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
        set_param(keyval[0], keyval[1]);
      }
    }
  }

private:
  std::string name_;

};  // Processor


template <typename Port, class ContextProvider>
struct Port_processor: public Processor {

  Port_processor(): Processor(), port_(ContextProvider::get_context()) {}

  ~Port_processor() {}

  typedef Port port_type;

  Port& get_port() {
    return port_;
  }

  std::string get_port_status() const {
    return port_.get_status();
  }

  auto get_executor() {
    return ContextProvider::get_context().get_executor();
  }

private:
  Port port_;

};  // Port_processor


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
