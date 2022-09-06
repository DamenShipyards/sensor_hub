/**
 * \file fusion.h
 * \brief Provide fusion processor
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


#ifndef STATISTICS_H_
#define STATISTICS_H_


#include "../processor.h"

#ifdef HAVE_VECTOR

/**
 * \brief Vector that keeps track of which components have been set
 *
 * Sensor Hub quantities are scalars, but can be vector components. This
 * class is intended to collect the individual components into a vector
 * for calculation purposes.
 */

struct AssembledVector: public Stamped_vector {
  AssembledVector(const Value_type x, const Value_type y, const Value_type z):
      Stamped_vector(x, y, z), index_bits_((1 << 2) + (1 << 1) + (1 << 0)) {
  }

  AssembledVector(): Stamped_vector(), index_bits_() {
  }

  AssembledVector& set_value(const int index, const Value_type value) {
    this->operator[](index) = value;
    index_bits_ |= 1 << index;
    return *this;
  }

private:
  byte_t index_bits_;
};

#endif

struct Fusion: public Processor {
  Fusion(): Processor() {}

  void insert_value(const Stamped_quantity&) override {
  }

  double operator[](size_t) override {
    return 0.0;
  }

  std::string get_json() const override;
  uint16_t get_modbus_reg(size_t index, const Base_scale& scaler) const override;

  size_t size() override {
    return 0;
  }

  void set_param(const std::string&, const double&) override {
  }

private:
};

#endif

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
