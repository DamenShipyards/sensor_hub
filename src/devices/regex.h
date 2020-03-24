/**
 * \file regex.h
 * \brief Provide generic regular expression parsing device
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


#ifndef REGEX_H_
#define REGEX_H__


#include "../types.h"
#include "../device.h"
#include "../log.h"
#include "../tools.h"
#include "../functions.h"
#include "../datetime.h"
#include "../parser.h"

namespace regex {

namespace parser {

struct Regex_parser {

};

}

template <class Port, class ContextProvider>
struct Regex_device: public Port_device<Port, ContextProvider>, 
    public Port_polling_mixin<Regex_device<Port, ContextProvider> > {

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

  template <typename Iterator>
  void handle_data(double stamp, Iterator buf_begin, Iterator buf_end) {
    (void)stamp;
    (void)buf_begin;
    (void)buf_end;
  }

private:
  parser::Regex_parser parser_;
};

}  // namespace regex

#endif  // ifndef REGEX_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
