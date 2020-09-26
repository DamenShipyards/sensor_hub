/**
 * \file types.h
 * \brief Provide simple common types
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


#ifndef TYPES_H_
#define TYPES_H_

#include <algorithm>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>

#include "tools.h"
#include "config.h"

#ifdef HAVE_EIGEN
#include <Eigen/Dense>
#include <Eigen/Geometry>
#endif

namespace endian = boost::endian;

//! Storage type for internal values
using Value_type = double;

// Data types for data communicated with the sensors
using byte_t = unsigned char;
using cbyte_t = byte_t const;
using bytes_t = std::vector<byte_t>;
using cbytes_t = bytes_t const;

struct Bytes: public bytes_t {
  using bytes_t::bytes_t;
  Bytes& operator<<(cbytes_t tail) {
    insert(end(), tail.begin(), tail.end());
    return *this;
  }
  Bytes& operator<<(cbyte_t byte) {
    push_back(byte);
    return *this;
  }
  Bytes& operator<<(endian::order order) {
    order_ = order;
    return *this;
  }
  Bytes& operator<<(uint16_t value) {
    append(value);
    return *this;
  }
  Bytes& operator<<(uint32_t value) {
    append(value);
    return *this;
  }
private:
  boost::endian::order order_ = boost::endian::order::native;
  template <typename T>
  void append (T value) {
    if (order_ == endian::order::little) {
      endian::endian_buffer<endian::order::little, T, sizeof(T) * 8> buf; 
      buf = value;
      insert(end(), buf.data(), buf.data() + sizeof(T));
    }
    else if (order_ == endian::order::big) {
      endian::endian_buffer<endian::order::big, T, sizeof(T) * 8> buf;
      buf = value;
      insert(end(), buf.data(), buf.data() + sizeof(T));
    }
    else {
      endian::endian_buffer<endian::order::native, T, sizeof(T) * 8> buf;
      buf = value;
      insert(end(), buf.data(), buf.data() + sizeof(T));
    }
  }
};

namespace pt = boost::posix_time;

extern std::ostream& operator<<(std::ostream& os, cbytes_t data);

template <typename C>
Bytes operator<<(cbytes_t& data, const C& tail) {
  Bytes result;
  result << data << tail;
  return result;
}


#ifdef HAVE_EIGEN
#define HAVE_VECTOR
using Vector = Eigen::Matrix<Value_type, 3, 1>;
#define HAVE_QUATERNION
using Quaternion = Eigen::Quaternion<Value_type>;

struct Stamped_vector: public Vector {
  using Vector::Vector;
  double stamp;
};

struct Stamped_quaternion: public Quaternion {
  double stamp;
};

#endif


#endif  // define TYPES_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
