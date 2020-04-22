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
#include "../quantities.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <ostream>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>

namespace regexp {

namespace parser {

struct Quantity_filter {
  Quantity_filter(const std::string& filter): expression(filter), multipliers(), offsets(), formats() {}
  Quantity_filter(const Quantity_filter& value) = default; 
  boost::regex expression;
  std::vector<double> multipliers;
  std::vector<double> offsets;
  std::vector<std::string> formats;
};

using Quantity_filters = std::map<Quantity, Quantity_filter>;

struct Regex_parser: public Packet_parser<std::string> {

  void parse(const double& stamp) override {
    bool matched = true;
    while (matched) {
      matched = false;
      iterator last = cur;
      for (const auto& [q, filter]: filters_) {
        Stamped_quantity sq{0.0, stamp, q};
        log(level::debug, "Looking for % in % with %", get_quantity_name(q), buffer, filter.expression);
        boost::match_results<iterator> match;
        if (boost::regex_search(cur, buffer.end(), match, filter.expression, boost::match_perl)) {
          matched = true;
          if (match[0].second > last) {
            last = match[0].second;
          }
          for (size_t i = 1; i < match.size(); ++i) {
            if (match[i].matched) {
              log(level::debug, "Found: %", match[i]);

              double value;
              std::string s = match[i];

              if (i <= filter.formats.size()) {
                auto format = filter.formats[i - 1];
                if (format == "f") {
                  if (s.find(".") == std::string::npos) {
                    boost::algorithm::replace_last(s, ",", ".");
                  }
                  boost::algorithm::replace_all(s, ",", "");
                  value = std::stod(s);
                }
                else if (format == "dt") {
                  pt::ptime dt;
                  if (s.find("T") == std::string::npos) {
                    dt = pt::time_from_string(s);
                  }
                  else {
                    dt = pt::from_iso_string(s);
                  }
                  value = to_timestamp(dt);
                }
                else {
                  std::tm tm = {};
                  std::stringstream ss(s);
                  ss >> std::get_time(&tm, format.c_str());
                  value = std::mktime(&tm);
                }
              }

              if (i <= filter.multipliers.size()) {
                value *= filter.multipliers[i - 1];
              }
              if (i <= filter.offsets.size()) {
                value += filter.offsets[i - 1];
              }
              sq.value += value;
            }
          }
          values_.push_back(sq);
        }
      }
      cur = last;
    }
    buffer.erase(buffer.begin(), cur);
  }

  Stamped_queue& get_values() override {
    return values_;
  }

  Quantity_filters& filters() {
    return filters_;
  }

private:
  Stamped_queue values_;
  Quantity_filters filters_;
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
    parser_.add_and_parse(stamp, buf_begin, buf_end);
    auto& values = parser_.get_values();
    while (!values.empty()) {
      this->insert_value(values.front());
      values.pop_front();
    }
  }

  void set_options(const prtr::ptree& options) override {
    (void)options;
    for (auto it = Quantity_iter::begin(); it != Quantity_iter::end(); ++it) {
      try {
        std::string q_name = get_quantity_name(*it);
        auto filter = parser::Quantity_filter(options.get<std::string>(q_name + ".filter"));
        for (int i = 0; i < 10; ++i) {
          filter.multipliers.push_back(options.get(fmt::format("{:s}.multiplier{:d}", q_name, i), 1.0));
          filter.offsets.push_back(options.get(fmt::format("{:s}.offset{:d}", q_name, i), 0.0));
          filter.formats.push_back(options.get(fmt::format("{:s}.format{:d}", q_name, i), "f"));
        }
        parser_.filters().emplace(std::pair(*it, filter));
      }
      catch (prtr::ptree_bad_path& e) {
        // Quantity filter not provided: fine!
      }
    }
  }

private:
  parser::Regex_parser parser_;
};

}  // namespace regex

#endif  // ifndef REGEX_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
