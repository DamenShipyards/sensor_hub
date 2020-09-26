/**
 * \file nmea.h
 * \brief Provide generic nmea device
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


#ifndef NMEA_H_
#define NMEA_H_

#include "../types.h"
#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../functions.h"
#include "../datetime.h"

namespace nmea {

template <class Port, class ContextProvider>
struct Generic_Nmea: public Port_device<Port, ContextProvider>, 
    public Polling_mixin<Generic_Nmea<Port, ContextProvider> > {

  bool initialize(asio::yield_context yield) override {
    bool result = true;

    if (result) {
      log(level::info, "Successfully initialized %", this->get_name());
      this->start_polling();
    }
    else {
      log(level::error, "Failed to initialize %", this->get_name());
      if (this->reset(yield)) {
        log(level::info, "Successfully reset %", this->get_name());
      }
    }

    return result;
  }

  void poll_data(asio::yield_context yield) override {
    (void)yield;
  }

};



}  // namespace nmea

#endif  // ifndef NMEA_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
