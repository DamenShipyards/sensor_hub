/**
 * \file datetime.h
 * \brief Provide interface for time handling
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/chrono.hpp>

namespace chrono = boost::chrono;
namespace posix_time = boost::posix_time;
namespace date_time = boost::date_time;

extern double get_time();
extern void adjust_clock(const double& towards_time);
extern void set_adjust_rate(const double& adjust_rate);

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
