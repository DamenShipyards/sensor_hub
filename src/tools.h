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
#include <iostream>

#include "version.h"

#define STRIFY2(X) #X
#define STRINGIFY(X) STRIFY2(X)

/**
 * Iterator for scoped enum values
 */
template <typename E, E begin_val=static_cast<E>(0), E end_val=E::end>
struct Enum_iter {
  Enum_iter(const E &e) : val_(static_cast<value_type>(e)) {}
  Enum_iter() : val_(static_cast<value_type>(begin_val)) {}
  Enum_iter operator++() {
    ++val_;
    return *this;
  }
  Enum_iter operator++(int) {
    Enum_iter result = *this;
    ++val_;
    return result;
  }
  E operator*() { return static_cast<E>(val_); }
  static const Enum_iter& begin() { 
    static const Enum_iter b(begin_val);
    return b; 
  }
  static const Enum_iter& end() { 
    static const Enum_iter e(end_val);
    return e; 
  }
  bool operator!=(const Enum_iter& i) { return val_ != i.val_; }
  bool operator==(const Enum_iter& i) { return val_ == i.val_; }
private:
  typedef typename std::underlying_type<E>::type value_type;
  size_t val_;
};

template<typename E, template <E> typename Tr, E e>
constexpr inline auto get_enum_trait() {
  return Tr<e>::value();
}

/**
 * Sub sequence matcher
 *
 * Returns the location of sub_container's sequence in container
 * or -1 one when container doesn't contain a copy of sub_container
 */
template <class Container>
int contains_at(const Container& container, const Container& sub_container) {
  auto c_iter = container.cbegin();
  auto sc_iter = sub_container.cbegin();
  int i = 0;
  do {
    if (sc_iter == sub_container.cend())
      return i;
    if (c_iter == container.cend())
      return -1;
    if (*c_iter++ == *sc_iter) {
      auto cc_iter = c_iter;
      ++sc_iter;
      do  {
        if (sc_iter == sub_container.cend())
          return i;
        if (cc_iter == container.cend())
          return -1;
        if (*cc_iter++ != *sc_iter++) {
          sc_iter = sub_container.cbegin();
          break;
        }
      } while (true);
    }
    ++i;
  } while (true);
  return -1;
}


/**
 * Sub sequence match
 *
 * Returns whether sub_container's sequence is present in container
 */
template <class Container>
bool contains(const Container& container, const Container& sub_container) {
  return contains_at(container, sub_container) >= 0;
}


inline double sqr(const double value) {
  return value * value;
}


inline void print_version() {
  std::cout << "Damen Sensor Hub " << STRINGIFY(VERSION) << std::endl;
  std::cout << "   Git revision  : " << STRINGIFY(GITREV) << std::endl;
  std::cout << "   Build date    : " << STRINGIFY(DATE) << std::endl;
  std::cout << "Written by Jaap Versteegh <jaap.versteegh@damen.com>" << std::endl;
}

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2

