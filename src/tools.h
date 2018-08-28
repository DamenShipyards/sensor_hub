/**
 * \file tools.h
 * \brief Provide generic tools
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <type_traits>

template <typename E, E begin_val=static_cast<E>(0), E end_val=E::end>
struct Enum_iter {
  Enum_iter(const E &e) : val_(static_cast<value_type>(e)) {}
  Enum_iter() : val_(static_cast<value_type>(begin_val)) {}
  Enum_iter operator++() {
    ++val_;
    return *this;
  }
  E operator*() { return static_cast<E>(val_); }
  Enum_iter&& begin() { return std::move(Enum_iter(begin_val)); } 
  Enum_iter&& end() { return std::move(Enum_iter(end_val)); }
  bool operator!=(const Enum_iter& i) { return val_ != i.val_; }
private:
  typedef typename std::underlying_type<E>::type value_type;
  size_t val_;
};

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2

