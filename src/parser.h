/**
 * \file parser.h
 * \brief Provide common functionality for parsers
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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


#ifndef PARSER_H_
#define PARSER_H_

#include "spirit_x3.h"
#include "types.h"
#include "tools.h"
#include "datetime.h"
#include "quantities.h"

#include <ios>
#include <ostream>
#include <deque>
#include <iterator>

namespace x3 = boost::spirit::x3;

using Quantity_values = std::vector<Quantity_value>;
using Stamped_quantities = std::vector<Stamped_quantity>;
using Stamped_queue = std::deque<Stamped_quantity>;

template<typename BufferType = std::deque<uint8_t> >
struct Packet_parser {
  Packet_parser(): buffer(), cur(buffer.begin()) {}
  BufferType buffer;
  using buffer_type = BufferType; 
  using iterator = typename BufferType::iterator;
  using const_iterator = typename BufferType::const_iterator;
  typename BufferType::iterator cur;

  template <typename Iterator>
  void add_and_parse(const double& stamp, Iterator begin, Iterator end) {
    if (buffer.size() > 0x1000)
      // More than 4K data in the buffer. Something is wrong. Hose it.
      buffer.clear();
    buffer.insert(buffer.end(), begin, end);
    cur = buffer.begin();
    parse(stamp);
  }
  virtual void parse(const double& stamp) = 0;
  virtual Stamped_queue& get_values() = 0;
};

#endif  // ifndef PARSER_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
