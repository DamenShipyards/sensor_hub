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


namespace ubx {


namespace command {

extern cbytes_t cfg_prt_usb;
extern cbytes_t cfg_prt_uart;

}  // namespace command

namespace response {

extern cbytes_t ack;
extern cbytes_t nak;

}  // namespace response


namespace parser {

namespace x3 = boost::spirit::x3;

using Values_type = std::vector<Quantity_value>;
using Values_queue = std::deque<Quantity_value>;

struct Packet_parser {
  Packet_parser() {};
  ~Packet_parser() {};
};

} //  namespace parser



template <typename Port, typename ContextProvider>
struct Ublox: public Port_device<Port, ContextProvider> {


  bool initialize(asio::yield_context yield) override {
    bool result =
        setup_ports(yield)
        && setup_messages(yield);

    if (result) {
      log(level::info, "Successfully initialized Ublox device");
    }
    else {
      log(level::error, "Failed to initialize Ublox device");
    }

    return result;
  }

  void use_as_time_source(const bool value) override {
    Device::use_as_time_source(value);
  }

  const parser::Packet_parser& get_parser() const {
    return parser_;
  }

  virtual bool setup_ports(asio::yield_context yield) = 0;
  virtual bool setup_messages(asio::yield_context yield) = 0;

private:
  parser::Packet_parser parser_;
};


template <typename Port, typename ContextProvider>
struct NEO_M8U: public Ublox<Port, ContextProvider> {

  NEO_M8U(): Ublox<Port, ContextProvider>() {
    log(level::info, "Constructing Ublox_NEO_M8U");
  }

  ~NEO_M8U() override {
    log(level::info, "Destroying Ublox_NEO_M8U");
  }

  bool setup_ports(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup ports");
    return this->exec_command(command::cfg_prt_usb, response::ack, response::nak, yield) ;
      //&& this->exec_command(command::cfg_prt_uart, response::ack, response::nak, yield);
  }

  bool setup_messages(asio::yield_context yield) override {
    log(level::info, "Ublox NEO M8U setup messages");
    return true;
  }
};

}  //namespace ubx

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
