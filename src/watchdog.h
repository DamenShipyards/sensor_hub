/**
 * \file watchdog.h
 * \brief Provide interface to system watchdog
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards\n
 *            Copyright (C) 2020-2022 Orca Software
 *
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

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <fstream>
#include <boost/filesystem.hpp>

#include "log.h"

namespace fs = boost::filesystem;

struct Watchdog {

  static constexpr char const * device = "/dev/watchdog";

  ~Watchdog() {
    if (wd_device_.is_open()) {
      wd_device_ << "V";
      wd_device_.flush();
      wd_device_.close();
      log(level::info, "Put watchdog to sleep");
    }
  }

  void feed() {
    try {
      wd_device_ << ".";
      wd_device_.flush();
      log(level::debug, "Fed the dog");
    }
    catch(std::exception& e) {
      log(level::error, "Error feeding the dog: %", e.what());
    }
  }

  void enable(bool value=true) {
    if (value) {
      if (fs::exists(fs::path(device))) {
        try {
          wd_device_.open(device);
          log(level::info, "Awakened watchdog");
        }
        catch(std::exception& e) {
          log(level::error, "Error opening watchdog: %", e.what());
        }
      }
      else {
        log(level::info, "Watchdog not available");
      }
    }
  }

private:
  std::ofstream wd_device_;

};

#endif // #ifndef WATCHDOG_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2 filetype=cpp
