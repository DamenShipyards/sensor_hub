/**
 * \file quantities.h
 * \brief Provide centralized quantity information
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

/**
 * \brief Enumeration of available quantities
 *
 * Note the following ship and earth related conventions:
 * With respect to ships, the X coordinate is longitudinal and positive
 * pointing to the bow. Y coordinate is transerse and positive 
 * pointing to starboard. Z coordinate is vertical and positive pointing
 * *down*!
 * With respect to the earth, the X axis points north, the Y axis east
 * and the Z axis *down*.
 */
enum class Quantity {
  la, ///<  0: WGS84 latitude
  lo, ///<  1: WGS84 longitude
  h1, ///<  2: Height with respect to WGS84 ellipsoid 
  h2, ///<  3: Height with respect to MSL/Geoid99
  mx, ///<  4: X component of magnetic flux vector
  my, ///<  5: Y component of magnetic flux vector
  mz, ///<  6: Z component of magnetic flux vector
  x,  ///<  7: X position with respect to some reference point
  y,  ///<  8: Y position with respect to some reference point
  z,  ///<  9: Z position with respect to some reference point
  vx, ///< 10: X component of velocity
  vy, ///< 11: Y component of velocity
  vz, ///< 12: Z component of velocity
  ax, ///< 13: X component of acceleration
  ay, ///< 14: Y component of acceleration
  az, ///< 15: Z component of acceleration
  ro, ///< 16: Roll, rotation about X axis
  pi, ///< 17: Pitch, rotation about Y axis
  ya, ///< 18: Yaw, rotation about Z axis. Also heading.
  q1, ///< 19: Orientation quaternion component 1
  q2, ///< 20: Orientation quaternion component 2
  q3, ///< 21: Orientation quaternion component 3
  q4, ///< 22: Orientation quaternion component 4
  rr, ///< 23: Roll rate, angular velocity about X axis
  pr, ///< 24: Pitch rate, angular velocity about Y axis
  yr, ///< 25: Yaw rate, angular velocity about Z axis
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
