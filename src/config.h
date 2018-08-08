/**
 * \file config.h
 * \brief Provide application configuration interface
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <boost/property_tree/ptree.hpp>

extern boost::property_tree::ptree& get_config();

#endif
