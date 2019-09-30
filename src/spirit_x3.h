/**
 * \file spirit_x3.h
 * \brief Includes for using spirit X3 parser
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


#ifndef SPIRIT_X3_H_
#define SPIRIT_X3_H_

#include <cstdint>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/endian/detail/intrinsic.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION >= 107100

/*
Boost 1.71 is supposed to support double and float, but I don't see how.
It still exclitly checks whether the type is integral in endian_reverse.

It also provides and additional is_endian_reversible, which looks like
doing the same thing twice. .. but we can specialize that.
Seems like the BOOST_STATIC_ASSERTS should check that and not std::integral.
*/


#include <boost/endian/conversion.hpp>

namespace boost { namespace endian {

namespace detail {

template<> struct is_endian_reversible<float>: std::integral_constant<bool, true> {};
template<> struct is_endian_reversible<double>: std::integral_constant<bool, true> {};

}

template<> inline float endian_reverse<float>(float x) BOOST_NOEXCEPT {
  float result;
  uint32_t iresult;
  memcpy(&iresult, &x, sizeof(iresult));
#ifdef BOOST_ENDIAN_NO_INTRINSICS
  iresult = iresult << 16 | iresult >> 16;
  iresult = ((iresult << 8) & 0xff00ff00) | ((iresult >> 8) & 0x00ff00ff);
#else
  iresult = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(iresult);
#endif
  memcpy(&result, &iresult, sizeof(result));
  return result;
}

template<> inline double endian_reverse<double>(double x) BOOST_NOEXCEPT {
  double result;
  uint64_t iresult;
  memcpy(&iresult, &x, sizeof(iresult));
#ifdef BOOST_ENDIAN_NO_INTRINSICS
  iresult = iresult << 32 | iresult >> 32;
  iresult = (iresult & 0x0000ffff0000ffffULL) << 16 | (iresult & 0xffff0000ffff0000ULL) >> 16;
  iresult = (iresult & 0x00ff00ff00ff00ffULL) << 8 | (iresult & 0xff00ff00ff00ff00ULL) >> 8;
#else
  iresult = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(iresult);
#endif
  memcpy(&result, &iresult, sizeof(result));
  return result;
}

}}

#else

// Inject missing endian_reverse into boost::endian namepace so floating point parsing in spirit works
namespace boost { namespace endian {
inline float endian_reverse(float x) BOOST_NOEXCEPT {
#ifdef BOOST_ENDIAN_NO_INTRINSICS
  uint32_t step16 = x << 16 | x >> 16;
  uint32_t result = ((step16 << 8) & 0xff00ff00) | ((step16 >> 8) & 0x00ff00ff);
#else
  uint32_t result = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(*reinterpret_cast<uint32_t*>(&x));
  return *reinterpret_cast<float*>(&result);
#endif
}

inline double endian_reverse(double x) BOOST_NOEXCEPT {
#ifdef BOOST_ENDIAN_NO_INTRINSICS
  uint64_t step32 = x << 32 | x >> 32;
  uint64_t step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16 | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
  uint64_t result = (step16 & 0x00FF00FF00FF00FFULL) << 8 | (step16 & 0xFF00FF00FF00FF00ULL) >> 8;
#else
  uint64_t result = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(*reinterpret_cast<uint64_t*>(&x));
#endif
  return *reinterpret_cast<double*>(&result);
}
}}

#include <boost/endian/conversion.hpp>
#endif // if BOOST_VERSION >= 107100

#define BOOST_ENDIAN_HAS_FLOATING_POINT
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary/binary.hpp>
#include <boost/spirit/home/x3/auxiliary/eps.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/apply_visitor.hpp>

#endif
