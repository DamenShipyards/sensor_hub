cmake_minimum_required(VERSION 3.14)

project(SensorHub)


message("Building with ${CMAKE_CXX_COMPILER}: ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.70.0 REQUIRED
  system date_time filesystem log log_setup unit_test_framework regex coroutine chrono
  program_options
)

