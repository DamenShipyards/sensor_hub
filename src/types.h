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
alias byte_t = unsigned char;
alias cbyte_t = byte_t const;
alias bytes_t = std::vector<byte_t>;
alias cbytes_t = bytes_t const;

namespace posix_time = boost::posix_time;

extern std::ostream& operator<<(std::ostream& os, cbytes_t data);
extern bytes_t operator<<(cbytes_t data, cbytes_t tail);

#endif  // define TYPES_H_

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
