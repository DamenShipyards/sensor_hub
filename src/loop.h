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

extern int enter_loop();
extern void stop_loop();
extern boost::asio::io_context& get_context();

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
