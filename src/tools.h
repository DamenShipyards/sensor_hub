/**
 * \file tools.h
 * \brief Provide generic tools
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


#ifndef TOOLS_H_
#define TOOLS_H_

#include <type_traits>
#include <algorithm>
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


template <class Container, typename Items>
struct contains_checker {
  int operator() (const Container& container, const Items& items) const {
    auto c_iter = container.cbegin();
    auto sc_iter = items.cbegin();
    int i = 0;
    do {
      if (sc_iter == items.cend()) {
        return i;
      }
      if (c_iter == container.cend())
        return -1;
      if (*c_iter++ == *sc_iter) {
        auto cc_iter = c_iter;
        ++sc_iter;
        do  {
          if (sc_iter == items.cend()) {
            return i;
          }
          if (cc_iter == container.cend())
            return -1;
          if (*cc_iter++ != *sc_iter++) {
            sc_iter = items.cbegin();
            break;
          }
        } while (true);
      }
      ++i;
    } while (true);
    return -1;
  }
};


template <class Container>
struct contains_checker<Container, typename Container::value_type> {
  int operator() (const Container& container, const typename Container::value_type& item) const {
    auto it = std::find(container.cbegin(), container.cend(), item);
    if (it == container.cend()) {
      return -1;
    }
    else {
      return std::distance(container.cbegin(), it);
    }
  }
};


template <class Container, typename Items>
inline int contains_at(const Container& container, const Items& items) {
  return contains_checker<Container, Items>()(container, items);
}


/**
 * Sub sequence match
 *
 * Returns whether item or items are present in container
 */
template <class Container, typename Items>
inline bool contains(const Container& container, const Items& items) {
  return contains_at<Container, Items>(container, items) >= 0;
}



inline double sqr(const double value) {
  return value * value;
}


inline void print_version() {
  std::cout << "Damen Sensor Hub " << STRINGIFY(VERSION) << std::endl;
  std::cout << "   Git revision  : " << STRINGIFY(GITREV) << std::endl;
  std::cout << "   Build date    : " << STRINGIFY(BUILD_DATE) << std::endl;
  std::cout << "Written by Jaap Versteegh <jaap.versteegh@damen.com>" << std::endl;
}

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2

