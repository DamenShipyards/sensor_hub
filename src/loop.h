/**
 * \file loop.h
 * \brief Provide interface to application main loop
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
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


#ifndef LOOP_H_
#define LOOP_H_

#include <boost/chrono.hpp>
#include <boost/asio.hpp>

namespace chrono = boost::chrono;
namespace asio = boost::asio;

/**
 * \mainpage Sensor Hub code documentation
 *
 * \section Introduction
 *
 * Welcome to the code documentation of sensor hub. This documentation is
 * maintained inline with the code and processed by Doxygen.
 */

/**
 * Enter the application main loop. 
 *
 * This will first setup all devices from the application configuration and 
 * then execute boost::asio::io_context::run() which will block until
 * #stop_loop() will be called
 */
extern int enter_loop();

/**
 * Stop the application main loop. 
 *
 * This will first close all devices and services and then call 
 * boost::asio::io_context::stop(). When all devices and services closed
 * in an orderly fashion, the latter will not have been redundant. 
 */
extern void stop_loop();

/**
 * Get the application wide IO context
 */
struct Context_provider {
  static boost::asio::io_context& get_context();
};

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
