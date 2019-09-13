/**
 * \file main.h
 * \brief Common functionality between winservice.cpp
 *        and daemon.cpp
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2019 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
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
