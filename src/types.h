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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/endian/conversion.hpp>

// Data types for data communicated with the sensor
typedef unsigned char byte_t;
typedef const byte_t cbyte_t;
typedef std::vector<byte_t> data_t;
typedef const data_t cdata_t;

namespace posix_time = boost::posix_time;

extern std::ostream& operator<<(std::ostream& os, cdata_t data);

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
