/**
 * \file loop.h
 * \brief Provide interface to application main loop
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef LOOP_H_
#define LOOP_H_

#include <boost/asio.hpp>

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
 * boost::asio::io_context::stop(). When all devices and services close
 * in an orderly fashion the latter will not have been necessary. 
 */
extern void stop_loop();

/**
 * Get the application wide IO context
 */
extern boost::asio::io_context& get_context();

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
