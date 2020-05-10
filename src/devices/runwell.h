/**
 * \file runwell.h
 * \brief Provide interface for runwell device
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * Copyright (C) 2020 Damen Shipyards
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


#ifndef RUNWELL_H_
#define RUNWELL_H__


#include "regex.h"

namespace runwell {

template <class Port, class ContextProvider>
struct Runwell_device: public regexp::Regex_device<Port, ContextProvider> {
  using Regex = regexp::Regex_device<Port, ContextProvider>;

  Runwell_device(): Regex(),
      interval_(60), tmr_(ContextProvider::get_context()) {
    prtr::ptree regex_options;
    regex_options.put( "md0.filter", 
        "^([0-2]),[0-2],[0-9]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put( "md1.filter", 
        "^[0-2],([0-2]),[0-9]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put("sts0.filter", 
        "^[0-2],[0-2],([0-9]+),[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put( "frq.filter", 
        "^[0-2],[0-2],[0-9]+,([0-9\\-.]+),[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put("vset.filter", 
        "^[0-2],[0-2],[0-9]+,[0-9\\-.]+,([0-9\\-.]+),[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put("vsig.filter", 
        "^[0-2],[0-2],[0-9]+,[0-9\\-.]+,[0-9\\-.]+,([0-9\\-.]+),[0-9\\-.]+,[0-9\\-.]+$");
    regex_options.put("vsup.filter", 
        "^[0-2],[0-2],[0-9]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,([0-9\\-.]+),[0-9\\-.]+$");
    regex_options.put("isup.filter", 
        "^[0-2],[0-2],[0-9]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,[0-9\\-.]+,([0-9\\-.]+)$");
    Regex::set_options(regex_options);
  }

  ~Runwell_device() {
    tmr_.cancel();
  }

  bool initialize(asio::yield_context yield) override {
    bool result = request_id(yield)
                  && request_version(yield);

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

    tmr_.expires_from_now(pt::seconds(interval_));
    asio::spawn(ContextProvider::get_context(), boost::bind(&Runwell_device::request_data, this, _1));

    return result;
  }

  bool request_version(asio::yield_context yield) {
    log(level::info, "Runwell get version");
    // Prime the reponse with offset of response length offset in the received data
    const int min_len = 48;
    bytes_t response = { 0xFF, min_len }; // Mac address
    bool result = this->exec_command({ 'h', '\n' }, {}, { 'X','X','X' }, yield, &response);
    if (result) {
      std::string version;
      int j = 0;
      for (int i = 0; i < min_len; ++i) {
        if (response[i] == '\n') {
          log(level::info, "Runwell: %", version);
          version = "";
          j += 1;
        }
        else {
          if (response[i] >= 0x20) {
            version += static_cast<char>(response[i]);
          }
        }
        if (j > 1)
          break;
      }
    }
    return result;
  }

  bool request_id(asio::yield_context yield) {
    log(level::info, "Runwell get unique identifier");
    // Prime the reponse with offset of response length offset in the received data
    const int min_len = 17;
    bytes_t response = { 0xFF, min_len }; // Mac address
    bool result = this->exec_command({'a', '\n' }, {}, {'X','X','X'}, yield, &response);
    if (result && response.size() >= min_len) {
      std::string mac = "";
      mac += static_cast<char>(response[0]);
      mac += static_cast<char>(response[1]);
      mac += static_cast<char>(response[3]);
      mac += static_cast<char>(response[4]);
      mac += static_cast<char>(response[6]);
      mac += static_cast<char>(response[7]);
      mac += static_cast<char>(response[9]);
      mac += static_cast<char>(response[10]);
      mac += static_cast<char>(response[12]);
      mac += static_cast<char>(response[13]);
      mac += static_cast<char>(response[15]);
      mac += static_cast<char>(response[16]);
      log(level::info, "Runwell device mac: %", mac);
      this->set_id("runwell_" + mac);
    }
    return result;
  }

  void request_data(asio::yield_context yield) {
    boost::system::error_code ec;
    tmr_.async_wait(yield[ec]);
    if (!ec) {
      // Setup next tick
      tmr_.expires_from_now(pt::seconds(interval_));
      asio::spawn(ContextProvider::get_context(), boost::bind(&Runwell_device::request_data, this, _1));
      Port& port = this->get_port();
      std::string command = "l\n";
      asio::async_write(port, asio::buffer(command), yield);
    }
  }

  void set_options(const prtr::ptree& options) override {
    interval_ = options.get("interval", 60);
    Regex::set_options(options);
  }

private:
  int interval_;
  asio::deadline_timer tmr_;
};

}  // namespace runwell

#endif  // ifndef RUNWELL_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
