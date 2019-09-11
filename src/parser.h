/**
 * \file parser.h
 * \brief Provide common functionality for parsers
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "spirit_x3.h"
#include "types.h"
#include "tools.h"
#include "datetime.h"
#include "quantities.h"

#include <boost/bind.hpp>

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>

namespace x3 = boost::spirit::x3;

using Quantity_values = std::vector<Quantity_value>;
using Stamped_quantities = std::vector<Stamped_quantity>;
using Stamped_queue = std::deque<Stamped_quantity>;

struct Packet_parser {
  Packet_parser(): queue(), cur(queue.begin()) {}
  std::deque<uint8_t> queue;
  typename std::deque<uint8_t>::iterator cur;

  template <typename Iterator>
  void add_and_parse(const double& stamp, Iterator begin, Iterator end) {
    if (queue.size() > 0x1000)
      // Something is wrong. Hose the queue
      queue.clear();
    queue.insert(queue.end(), begin, end);
    parse(stamp);
  }
  virtual void parse(const double& stamp) = 0;
  virtual Stamped_queue& get_values() = 0;
};

#endif  // ifndef PARSER_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
