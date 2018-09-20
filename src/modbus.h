/**
 * \file modbus.h
 * \brief Provide interface for modbus server
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#ifdef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#undef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#endif
#include <modbus/server.hpp>

struct Modbus_handler: public modbus::Default_handler {
};

using Modbus_server = modbus::Server<Modbus_handler>;

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
