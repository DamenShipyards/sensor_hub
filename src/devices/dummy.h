/**
 * \file dummy.h
 * \brief Provide dummy device for testing
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


#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../functions.h"
#include "../datetime.h"
#include "../types.h"

namespace dummy {

template <class ContextProvider>
struct Dummy_device: public Context_device<ContextProvider>,
    public Polling_mixin<Dummy_device<ContextProvider> > {

  // 51°49'57.0"N 4°56'14.4"E
  static constexpr Value_type la = deg_to_rad(51.83250);
  static constexpr Value_type lo = deg_to_rad(4.93733);
  static CONSTRET Value_type dla_dx = 1 / get_dx_dla(la);
  static CONSTRET Value_type dlo_dy = 1 / get_dy_dlo(la);
  static constexpr Value_type radius = 500.0;
  static constexpr Value_type freq = 2 * M_PI / 60.0;
  static constexpr Value_type velocity = freq * radius;

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


template <class ContextProvider>
struct Dummy_gps: public Dummy_device<ContextProvider> {
  using dev = Dummy_device<ContextProvider>;
  using dev::la;
  using dev::lo;
  using dev::dla_dx;
  using dev::dlo_dy;
  using dev::radius;
  using dev::freq;

  void poll_data(asio::yield_context yield) override {
    while (this->is_connected()) {
      log(level::debug, "Creating dummy gps values");
      this->wait(1000, yield);
      double t = get_time();
      this->insert_value(Stamped_quantity(t, t, Quantity::ut));
      // Fly around in a circle
      this->insert_value(Stamped_quantity(la + dla_dx * radius * cos(freq * t), t, Quantity::la));
      this->insert_value(Stamped_quantity(lo + dlo_dy * radius * sin(freq * t), t, Quantity::lo));
    }
  }

};


template <class ContextProvider>
struct Dummy_imu: public Dummy_device<ContextProvider> {
  using dev = Dummy_device<ContextProvider>;
  using dev::la;
  using dev::radius;
  using dev::freq;
  using dev::velocity;
  static constexpr Value_type centripetal = velocity * freq;

  void insert_acc_and_rot() {
    Value_type centripetal = sqr(velocity) / radius;
    double t = get_time();
    this->insert_value(Stamped_quantity(centripetal * -cos(freq * t), t, Quantity::ax));
    this->insert_value(Stamped_quantity(centripetal * -sin(freq * t), t, Quantity::ay));
    this->insert_value(Stamped_quantity(-get_earth_gravity(la), t, Quantity::az));
    this->insert_value(Stamped_quantity(0.1 * sin(t), t, Quantity::rr));
    this->insert_value(Stamped_quantity(0.1 * sin(0.61803399 * t), t, Quantity::pr));
    this->insert_value(Stamped_quantity(freq, t, Quantity::yr));
  }

  void poll_data(asio::yield_context yield) override {
    while (this->is_connected()) {
      log(level::debug, "Creating dummy imu values");
      this->wait(100, yield);
      this->insert_acc_and_rot();
    }
  }

};


template <class ContextProvider>
struct Dummy_mru: public Dummy_imu<ContextProvider> {

  using dev = Dummy_imu<ContextProvider>;
  using dev::freq;
  using dev::centripetal;

  void poll_data(asio::yield_context yield) override {
    while (this->is_connected()) {
      this->wait(100, yield);
      this->insert_acc_and_rot();
      double t = get_time();
      this->insert_value(Stamped_quantity(centripetal * -cos(freq * t), t, Quantity::fax));
      this->insert_value(Stamped_quantity(centripetal * -sin(freq * t), t, Quantity::fay));
      this->insert_value(Stamped_quantity(0, t, Quantity::faz));
    }
  }

};

}  // namespace dummy

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
