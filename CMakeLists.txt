cmake_minimum_required(VERSION 3.14)

project(sensor-hub)

include(GNUInstallDirs)

# Use compilers on the path, not /usr/bin/(g++|gcc) perse. cmake is stupid that way
# Apparently, we shouldn't do this, because it's too late set CC en CXX environment variables instead
#execute_process(COMMAND which gcc OUTPUT_VARIABLE CMAKE_C_COMPILER)
#execute_process(COMMAND which g++ OUTPUT_VARIABLE CMAKE_CXX_COMPILER)
execute_process(COMMAND awk -F= "/Version/ { print $2 }"
  ${CMAKE_CURRENT_SOURCE_DIR}/version.ini OUTPUT_VARIABLE VERSION)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GITREV)
execute_process(COMMAND date +%Y OUTPUT_VARIABLE YEAR)
execute_process(COMMAND date +%Y-%m-%d OUTPUT_VARIABLE DATE)
execute_process(COMMAND sh -c "\"${CMAKE_CXX_COMPILER}\" --version | head -n 1"
  OUTPUT_VARIABLE CXX_VERSION)

#string(STRIP ${CMAKE_C_COMPILER} CMAKE_C_COMPILER)
#string(STRIP ${CMAKE_CXX_COMPILER} CMAKE_CXX_COMPILER)
string(STRIP ${CXX_VERSION} CXX_VERSION)
string(STRIP ${VERSION} VERSION)
string(STRIP ${GITREV} GITREV)
string(STRIP ${YEAR} YEAR)
string(STRIP ${DATE} DATE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

message("Building application version ${VERSION} rev ${GITREV} on ${DATE}: ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Link boost statically in release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
set(Boost_USE_STATIC_LIBS ON)
add_definitions(
  -DFMT_HEADER_ONLY
)
else()
set(BUILD_SHARED_LIBS ON)
add_definitions(
  -DBOOST_LOG_DYN_LINK
  -DBOOST_TEST_DYN_LINK
  -DFMT_HEADER_ONLY
)
endif()

find_package(Boost 1.70.0 REQUIRED
  system date_time filesystem log log_setup unit_test_framework regex coroutine chrono
  program_options iostreams
)

find_package(libusb-1.0 1.0.22 REQUIRED)

find_package(Threads REQUIRED)
find_package(Doxygen)
find_program(HAVE_TEXI2DVI texi2dvi) 
find_program(HAVE_ASCIIDOC asciidoc)
find_program(HAVE_XSLTPROC xsltproc)
set(BUILD_MANUAL ${HAVE_TEXI2DVI} AND ${HAVE_ASCIIDOC} AND ${HAVE_XSLTPROC})

# Optional
find_package(Eigen3 QUIET)
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL QUIET)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG  "-ggdb -Wall -pedantic -Wno-psabi -Wno-write-strings -Wextra -Werror -Wfatal-errors -DDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE  "-O3 -Wno-psabi -Wno-write-strings -Wfatal-errors -DNDEBUG")

set(SENSORHUB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(FMTLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/fmt/include")
set(SENSORHUB_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")
set(XSENS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/xsens/include")
set(MODBUS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/modbus/include")
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/rapidjson/include")

set(VERSION_H_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in)
set(CONFIG_H_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in)
set(VERSION_H ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h)
set(CONFIG_H ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

message(STATUS "C++: ${CXX_VERSION}")
message(STATUS "Boost: ${Boost_VERSION_STRING}")
if(EIGEN3_FOUND)
  message(STATUS "Eigen: ${Eigen3_VERSION}")
  set(USE_EIGEN Eigen3::Eigen)
else()
  message(STATUS "Eigen: Not found")
endif()
if(CGAL_FOUND)
  message(STATUS "CGAL: ${CGAL_VERSION}")
else()
  message(STATUS "CGAL: Not found")
endif()
configure_file(${VERSION_H_IN} ${VERSION_H} @ONLY)
configure_file(${CONFIG_H_IN} ${CONFIG_H} @ONLY)

enable_testing()

set(CPP_SOURCES
  ${SENSORHUB_SOURCE_DIR}/loop.cpp
  ${SENSORHUB_SOURCE_DIR}/device.cpp
  ${SENSORHUB_SOURCE_DIR}/datetime.cpp
  ${SENSORHUB_SOURCE_DIR}/usb.cpp
  ${SENSORHUB_SOURCE_DIR}/http.cpp
  ${SENSORHUB_SOURCE_DIR}/log.cpp
  ${SENSORHUB_SOURCE_DIR}/configuration.cpp
  ${SENSORHUB_SOURCE_DIR}/modbus.cpp
  ${SENSORHUB_SOURCE_DIR}/processor.cpp
  ${SENSORHUB_SOURCE_DIR}/types.cpp
  ${SENSORHUB_SOURCE_DIR}/devices/xsens.cpp
  ${SENSORHUB_SOURCE_DIR}/devices/ublox.cpp
  ${SENSORHUB_SOURCE_DIR}/devices/nmea.cpp
  ${SENSORHUB_SOURCE_DIR}/devices/regex.cpp
  ${SENSORHUB_SOURCE_DIR}/devices/dummy.cpp
  ${SENSORHUB_SOURCE_DIR}/processors/statistics.cpp
  ${SENSORHUB_SOURCE_DIR}/processors/acceleration_history.cpp
  ${SENSORHUB_SOURCE_DIR}/processors/fusion.cpp
  ${SENSORHUB_SOURCE_DIR}/processors/signalk.cpp
)


include_directories(
  ${Boost_INCLUDE_DIR}
  ${LIBUSB_1_INCLUDE_DIRS}
  ${FMTLIB_INCLUDE_DIR}
  ${XSENS_INCLUDE_DIR}
  ${MODBUS_INCLUDE_DIR}
  ${RAPIDJSON_INCLUDE_DIR}
  ${Eigen3_INCLUDE_DIRS}
)

link_directories(
  ${Boost_LIBRARY_DIR_RELEASE}
  ${LIBUSB_1_LIBRARIES}
)

add_executable(sensor_hub
  ${SENSORHUB_SOURCE_DIR}/daemon.cpp
  ${CPP_SOURCES}
)

target_link_libraries(sensor_hub
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBUSB_1_LIBRARY}
  Boost::system
  Boost::date_time
  Boost::filesystem
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::chrono
  Boost::coroutine
  ${USE_EIGEN}
)

# Link stdc++ statically in release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
target_link_options(sensor_hub
  PUBLIC -static-libgcc -static-libstdc++
)
endif()

set_target_properties(sensor_hub PROPERTIES
  LINK_FLAGS "-Wl,--no-as-needed"
)

macro(new_test NEW_TEST)
  set(TEST_SOURCES ${ARGN})
  list(TRANSFORM TEST_SOURCES PREPEND ${SENSORHUB_SOURCE_DIR}/ )
  add_executable(${NEW_TEST}
    ${SENSORHUB_TEST_DIR}/${NEW_TEST}.cpp
    ${TEST_SOURCES}
  )
  set_target_properties(${NEW_TEST} PROPERTIES
    LINK_FLAGS "-Wl,--no-as-needed"
    RUNTIME_OUTPUT_DIRECTORY test
  )
  target_link_libraries(${NEW_TEST}
    Boost::system
    Boost::date_time
    Boost::filesystem
    Boost::log
    Boost::log_setup
    Boost::chrono
    Boost::coroutine
    Boost::unit_test_framework
    ${LIBUSB_1_LIBRARY}
    ${USE_EIGEN}
  )
  add_test(${NEW_TEST} test/${NEW_TEST})
endmacro()

new_test(test_asio)
new_test(test_log log.cpp)
new_test(test_serial)
new_test(test_quantities datetime.cpp log.cpp)
new_test(test_tools)
new_test(test_spirit)
new_test(test_usb log.cpp usb.cpp)
new_test(test_device log.cpp device.cpp datetime.cpp)
new_test(test_processor
  processor.cpp
  processors/statistics.cpp
  processors/acceleration_history.cpp
  log.cpp datetime.cpp modbus.cpp)
new_test(test_datetime datetime.cpp log.cpp)
new_test(test_modbus modbus.cpp log.cpp)

new_test(test_xsens usb.cpp device.cpp log.cpp datetime.cpp types.cpp)
new_test(test_xsens_impl datetime.cpp log.cpp types.cpp)
new_test(test_ublox device.cpp log.cpp datetime.cpp types.cpp)
new_test(test_ublox_impl datetime.cpp log.cpp types.cpp)
new_test(test_fusion datetime.cpp log.cpp modbus.cpp)
new_test(test_signalk datetime.cpp log.cpp)
new_test(test_regex device.cpp datetime.cpp types.cpp log.cpp)
new_test(test_runwell device.cpp datetime.cpp types.cpp log.cpp)

install(
  TARGETS sensor_hub
  DESTINATION ${CMAKE_INSTALL_SBINDIR}
  COMPONENT runtime
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/setup/files/99-sensor_hub.rules
  DESTINATION /lib/udev/rules.d
  COMPONENT runtime
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/setup/files/sensor_hub.service
  DESTINATION /lib/systemd/system
  COMPONENT runtime
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/setup/files/sensor_hub.conf
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/sensor_hub
  COMPONENT configuration
)

if(DOXYGEN_FOUND)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
  set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)

  add_custom_target(
    doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()

if(BUILD_MANUAL)
  set(MANUAL_FILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/manual/manual.texi.in)
  set(MANUAL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manual/manual.texi)
  set(MANUAL_MAKE_IN ${CMAKE_CURRENT_SOURCE_DIR}/manual/Makefile.in)
  set(MANUAL_MAKE ${CMAKE_CURRENT_BINARY_DIR}/manual/Makefile)
  configure_file(${MANUAL_FILE_IN} ${MANUAL_FILE} @ONLY)
  configure_file(${MANUAL_MAKE_IN} ${MANUAL_MAKE} @ONLY)
  add_custom_target(
    manual ALL
    COMMAND make -j1 >/dev/null
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/manual
    COMMENT "Generating manual with TexInfo and asciidoc"
  )

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/manual/sensor_hub.pdf
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  COMPONENT documentation
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/manual/sensor_hub.info
  DESTINATION ${CMAKE_INSTALL_INFODIR}
  COMPONENT documentation
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/sensor_hub.1
  DESTINATION ${CMAKE_INSTALL_MANDIR}
  COMPONENT documentation
)
else()
  message("Not building manual because of missing texi2dvi, asciidoc or xsltproc")
endif()
