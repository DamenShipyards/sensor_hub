/**
 * \file ublox_impl.h
 * \brief Implementation details for u-blox device base class
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */
#ifndef XSENS_IMPL_H_
#define XSENS_IMPL_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "datetime.h"

namespace gregorian = boost::gregorian;

namespace command {

cbyte_t ubx_start = 0xFA;
cbyte_t ubx_head = 0xFF;


} // namespace data

namespace ubx_parser {

using x3::repeat;
using x3::omit;
using x3::eps;

using x3::byte_;
using x3::big_word;
using x3::big_dword;
using x3::big_bin_float;
using x3::big_bin_double;

using x3::_attr;
using x3::_val;
using x3::_pass;

struct Data_packet {
  Data_packet(): id(0), len(0) {}
  Data_packet(const uint16_t did): id(did), len(0) {}
  uint16_t id;
  int len;
  virtual Values_type get_values() const {
    return Values_type();
  }
};




} // parser


#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
