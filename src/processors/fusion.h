/**
 * \file fusion.h
 * \brief Provide fusion processor
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_


#include "../processor.h"


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
