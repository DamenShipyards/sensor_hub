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
 * down.
 * With respect to the earth, the X axis points north, the Y axis east
 * and the Z axis down.
 */
enum class Quantity {
  la,  ///<  0: Latitude with respect to WGS84 ellipsoid (GPS)
  lo,  ///<  1: Longitude with respect to WGS84 ellipsoid (GPS)
  h1,  ///<  2: Height with respect to WGS84 ellipsoid (GPS)
  h2,  ///<  3: Height with respect to MSL/Geoid99
  vog, ///<  4: Absolute value of speed vector over ground
  vtw, ///<  5: Absolute value of speed vector through water
  hdg, ///<  6: Yaw angle with respect to true north
  crs, ///<  7: Angle of speed over ground vector with respect to true north
  mn,  ///<  8: Angle of magnetic north with respect to plane through X axis and vertical
  mx,  ///<  9: X component of magnetic flux vector
  my,  ///< 10: Y component of magnetic flux vector
  mz,  ///< 11: Z component of magnetic flux vector
  x,   ///< 12: X position with respect to some reference point
  y,   ///< 13: Y position with respect to some reference point
  z,   ///< 14: Z position with respect to some reference point
  vx,  ///< 15: X component of velocity
  vy,  ///< 16: Y component of velocity
  vz,  ///< 17: Z component of velocity
  ax,  ///< 18: X component of acceleration
  ay,  ///< 19: Y component of acceleration
  az,  ///< 20: Z component of acceleration
  ro,  ///< 21: Roll, rotation about X axis with respect to some reference
  pi,  ///< 22: Pitch, rotation about Y axis with respect to some reference
  ya,  ///< 23: Yaw, rotation about Z axis with respect to some reference
  q1,  ///< 24: Orientation quaternion component 1
  q2,  ///< 25: Orientation quaternion component 2
  q3,  ///< 26: Orientation quaternion component 3
  q4,  ///< 27: Orientation quaternion component 4
  rr,  ///< 28: Roll rate, angular velocity about X axis
  pr,  ///< 29: Pitch rate, angular velocity about Y axis
  yr,  ///< 30: Yaw rate, angular velocity about Z axis
};

// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
