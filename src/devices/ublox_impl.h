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

#include "../datetime.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


namespace gregorian = boost::gregorian;

namespace ubx_command {

cbyte_t start = 0xB5;
cbyte_t head = 0x62;

cbyte_t class_ack = 0x05;
cbyte_t class_cfg = 0x06;

cbyte_t id_null = 0x00;
cbyte_t id_one = 0x01;

cdata_t ack = {start, head, class_ack, id_one};
cdata_t nak = {start, head, class_ack, id_null};


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
