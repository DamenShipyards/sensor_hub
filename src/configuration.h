/**
 * \file configuration.h
 * \brief Provide application configuration interface
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


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/path.hpp>

extern boost::property_tree::ptree& get_config();
extern void update_config();
extern boost::filesystem::path get_config_file();
extern void set_config_file(const boost::filesystem::path config_file, bool reload=false);

#endif  // ifndef CONFIGURATION_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
