/**
 * \file ublox.h
 * \brief Provide interface to ublox device class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../spirit_x3.h"
#include "../datetime.h"
#include "../types.h"

#include <boost/bind.hpp>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>



namespace ubx_command {

extern cbyte_t packet_start;
extern cbyte_t sys_command;
}




namespace ubx_parser {

namespace x3 = boost::spirit::x3;

using Values_type = std::vector<Quantity_value>;
using Values_queue = std::deque<Quantity_value>;

struct Packet_parser {
  Packet_parser() {};
  ~Packet_parser() {};
};

} // ubx_parser



template <typename Port, typename ContextProvider>
struct Ublox: public Port_device<Port, ContextProvider> {


  bool initialize(asio::yield_context yield) override {
    bool result = true;

    if (result) {
      log(level::info, "Successfully initialized Ublox device");
    }
    else {
      log(level::error, "Failed to initialize Ublox device");
      if (this->reset(yield)) {
        log(level::info, "Successfully reset device");
      }
    }

    return result;
  }

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
  }

  const ubx_parser::Packet_parser& get_parser() const {
    return parser_;
  }
private:
  ubx_parser::Packet_parser parser_;
};


template <typename Port, typename ContextProvider>
struct Ublox_NEO_M8U: public Ublox<Port, ContextProvider> {

  Ublox_NEO_M8U(): Ublox<Port, ContextProvider>() {
    log(level::info, "Constructing Ublox_NEO_M8U");
  }

  ~Ublox_NEO_M8U() override {
    log(level::info, "Destroying Ublox_NEO_M8U");
  }

};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
