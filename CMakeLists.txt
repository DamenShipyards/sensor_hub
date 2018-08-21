cmake_minimum_required(VERSION 3.1)

project(SensorHub)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(doc)

find_package(Boost 1.67.0 REQUIRED 
  system date_time filesystem log log_setup unit_test_framework thread regex coroutine chrono
)

find_package(libusb-1.0 1.0.22 REQUIRED)

find_package(Threads REQUIRED)

set (CMAKE_CXX_STANDARD 14)

set(SENSORHUB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FMTLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fmt/src)
set(FMTLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fmt/include)
set(SENSORHUB_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

enable_testing()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(CPP_SOURCES
  ${SENSORHUB_SOURCE_DIR}/log.cpp
  ${SENSORHUB_SOURCE_DIR}/usb.cpp
  ${SENSORHUB_SOURCE_DIR}/config.cpp
  ${SENSORHUB_SOURCE_DIR}/xsens.cpp
  ${SENSORHUB_SOURCE_DIR}/loop.cpp
  ${SENSORHUB_SOURCE_DIR}/www.cpp
)

file(GLOB FMTLIB_SOURCES
  ${FMTLIB_SOURCE_DIR}/*.cc
)


include_directories(
  ${Boost_INCLUDE_DIR}
  ${LIBUSB_1_INCLUDE_DIRS}
  ${FMTLIB_INCLUDE_DIR}
)

link_directories(
  ${Boost_LIBRARY_DIR_RELEASE}
  ${LIBUSB_1_LIBRARIES}
)

link_libraries(
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBUSB_1_LIBRARY}
  boost_system boost_date_time boost_filesystem boost_log boost_log_setup boost_thread
)


add_executable(sensor_hub
  ${SENSORHUB_SOURCE_DIR}/daemon.cpp
  ${CPP_SOURCES}
  ${FMTLIB_SOURCES}
)

macro(new_test NEW_TEST)
  add_executable(${NEW_TEST} 
    ${SENSORHUB_TEST_DIR}/${NEW_TEST}.cpp
    ${CPP_SOURCES}
    ${FMTLIB_SOURCES}
  )
  set_target_properties(${NEW_TEST} PROPERTIES 
    LINK_FLAGS "-Wl,--no-as-needed"
    COMPILE_FLAGS "-ggdb"
    RUNTIME_OUTPUT_DIRECTORY test
  )
  target_link_libraries(${NEW_TEST} 
    ${Boost_LIBRARIES}
    ${LIBUSB_1_LIBRARY}
  )
  add_definitions(-DBOOST_TEST_DYN_LINK)
  add_test(${NEW_TEST} test/${NEW_TEST})
endmacro()

new_test(test_asio)
new_test(test_log)
new_test(test_usb)
new_test(test_serial)

