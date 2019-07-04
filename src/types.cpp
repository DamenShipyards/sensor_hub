/**
 * \file types.cpp
 * \brief Implementation for simple common types
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#include "types.h"

std::ostream& operator<<(std::ostream& os, cbytes_t data) {
  os << std::hex;
  std::string sep = "";
  for (auto&& b: data) {
    os << sep << std::setfill('0') << std::setw(2) << static_cast<unsigned>(b);
    sep = ",";
  }
  os << std::dec;
  return os;
}

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
