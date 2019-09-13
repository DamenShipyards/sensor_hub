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
