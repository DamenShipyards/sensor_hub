/**
 * \file main.h
 * \brief Common functionality between winservice.cpp
 *        and daemon.cpp
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

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "log.h"
#include "loop.h"
#include "tools.h"
#include "version.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

const po::variables_map& get_program_options();

// Return codes
#define PROGRAM_SUCCESS 0
#define INVALID_COMMAND_LINE 10
#define UNHANDLED_EXCEPTION 11
#define UNKNOWN_EXCEPTION 12
#define DAEMON_ALREADY_RUNNING 13
#define STOP_DAEMON_FAILED 14
#define FORK_FAILURE 15
#define DAEMON_INIT_FAILURE 16
#define DAEMON_NOT_RUNNING 17
#define PID_ERROR 18
#define DAEMON_START_FAILURE 19
