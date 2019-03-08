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

#include "device.h"
#include "log.h"
#include "tools.h"
#include "spirit_x3.h"
#include "datetime.h"
#include "types.h"

#include <boost/bind.hpp>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>



namespace command {

extern cbyte_t packet_start;
extern cbyte_t sys_command;
extern cbyte_t conf_command;

extern cdata_t goto_config;
extern cdata_t config_ack;

extern cdata_t goto_measurement;
extern cdata_t measurement_ack;

extern cdata_t init_mt;
extern cdata_t mt_ack;

extern cdata_t set_option_flags;
extern cdata_t option_flags_ack;

extern cdata_t req_reset;
extern cdata_t reset_ack;

extern cdata_t wakeup;
extern cdata_t wakeup_ack;

extern cdata_t req_product_code;
extern cdata_t product_code_resp;

extern cdata_t req_device_id;
extern cdata_t device_id_resp;

extern cdata_t req_firmware_rev;
extern cdata_t firmware_rev_resp;

extern cdata_t req_utc_time;

extern cdata_t get_output_configuration;
extern cdata_t get_output_configuration_ack;
extern cdata_t set_output_configuration;
extern cdata_t output_configuration_ack;

extern cdata_t set_string_output_type;
extern cdata_t string_output_type_ack;

extern cdata_t error_resp;
}




namespace ubx_parser {

namespace x3 = boost::spirit::x3;

using Values_type = std::vector<Quantity_value>;
using Values_queue = std::deque<Quantity_value>;

struct Packet_parser {
  Packet_parser();
  ~Packet_parser();
  struct Data_packets;
  struct Data_visitor;
  std::deque<uint8_t> queue;
  std::vector<uint8_t> data;
  std::unique_ptr<Data_packets> data_packets;
  std::unique_ptr<Data_visitor> visitor;
  typename std::deque<uint8_t>::iterator cur;

  template <typename Iterator>
  void parse(Iterator begin, Iterator end) {
    if (queue.size() > 0x1000)
      // Something is wrong. Hose the queue
      queue.clear();
    queue.insert(queue.end(), begin, end);
    parse();
  }
  void parse();
  Values_queue& get_values();
};

} // ubx_parser



template <typename Port, typename ContextProvider>
struct Ublox: public Port_device<Port, ContextProvider> {


  bool initialize(asio::yield_context yield) override {
    bool result = true;

    if (result) {
      log(level::info, "Successfully initialized Ublox device");
      start_polling();
    }
    else {
      log(level::error, "Failed to initialize Ublox device");
      if (reset(yield)) {
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
