/**
 * \file spirit_x3.h
 * \brief Includes for using spirit X3 parser
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#ifndef SPIRIT_X3_H_
#define SPIRIT_X3_H_

#include <boost/endian/detail/intrinsic.hpp>
// Inject missing endian_reverse into boost::endian namepace so floating point parsing in spirit works
namespace boost { namespace endian {
inline float endian_reverse(float x) BOOST_NOEXCEPT {
  uint32_t result = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_4(*reinterpret_cast<uint32_t*>(&x));
  return *reinterpret_cast<float*>(&result);
}
inline float endian_reverse(double x) BOOST_NOEXCEPT {
  uint64_t result = BOOST_ENDIAN_INTRINSIC_BYTE_SWAP_8(*reinterpret_cast<uint64_t*>(&x));
  return *reinterpret_cast<double*>(&result);
}
}}
#include <boost/endian/conversion.hpp>
#define BOOST_ENDIAN_HAS_FLOATING_POINT
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary/binary.hpp>
#include <boost/spirit/home/x3/auxiliary/eps.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/apply_visitor.hpp>

#endif
