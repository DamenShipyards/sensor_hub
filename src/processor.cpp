/**
 * \file processor.cpp
 * \brief Provide implementation for processor data processors
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */


#include "processor.h"


#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>


using Processor_factory_map = std::map<std::string, Processor_factory_ptr>;

/**
 * Return singleton processor factory registry
 */
Processor_factory_map& get_processor_factory_map() {
  static Processor_factory_map instance;
  return instance;
}

Processor_factory_ptr& add_processor_factory(const std::string& name, Processor_factory_ptr&& factory) {
  Processor_factory_map& factories = get_processor_factory_map();
  factories.emplace(Processor_factory_map::value_type(name, std::move(factory)));
  return factories[name];
}

Processor_ptr create_processor(const std::string& name) {
  try {
    decltype(auto) factory = get_processor_factory_map().at(name);
    return factory->get_instance();
  }
  catch (std::out_of_range&) {
    log(level::error, "Processor with name % does not appear to be registered", name);
    throw;
  }
}
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
