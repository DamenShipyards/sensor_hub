/**
 * \file types.cpp
 * \brief Implementation for simple common types
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
