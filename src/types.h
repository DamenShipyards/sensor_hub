/**
 * \file types.h
 * \brief Provide simple common types
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly
 * forbidden.
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <boost/date_time/posix_time/posix_time.hpp>

#include "spirit_x3.h"

// Data types for data communicated with the sensors
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;

namespace posix_time = boost::posix_time;

extern std::ostream& operator<<(std::ostream& os, cdata_t data);
//extern data_t operator<<(cdata_t data, cdata_t tail);

#endif  // define TYPES_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
