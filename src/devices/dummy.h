/**
 * \file dummy.h
 * \brief Provide dummy device for testing
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


#include "../types.h"
#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../datetime.h"

namespace dummy {

template <class ContextProvider>
struct Dummy_device: public Context_device<ContextProvider>, 
    public Polling_mixin<Dummy_device<ContextProvider> > {

  bool initialize(asio::yield_context yield) override {
    bool result = true;

    if (result) {
      log(level::info, "Successfully initialized dummy device");
      this->start_polling();
    }
    else {
      log(level::error, "Failed to initialize dummy device");
      if (this->reset(yield)) {
        log(level::info, "Successfully dummy Xsens device");
      }
    }

    return result;
  }

  void poll_data(asio::yield_context yield) override {
    while (this->is_connected()) {
      this->wait(1000, yield);
      double t = get_time();
      this->insert_value(Stamped_quantity(t, t, Quantity::ut));
      for (auto qi = Quantity_iter::begin(); qi != Quantity_iter::end(); ++qi) {
        Value_type v = sin(t / static_cast<int>(*qi));
        this->insert_value(Stamped_quantity(v, t, *qi));
      }
    }
  }

};

}  // namespace dummy

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
