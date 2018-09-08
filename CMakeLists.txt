cmake_minimum_required(VERSION 3.12)

project(SensorHub)

# Use compilers on the path, not /usr/bin/(g++|gcc) perse. cmake is stupid that way
execute_process(COMMAND which gcc OUTPUT_VARIABLE CMAKE_C_COMPILER)
execute_process(COMMAND which g++ OUTPUT_VARIABLE CMAKE_CXX_COMPILER)
string(STRIP ${CMAKE_C_COMPILER} CMAKE_C_COMPILER)
string(STRIP ${CMAKE_CXX_COMPILER} CMAKE_CXX_COMPILER)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.67.0 REQUIRED 
  system date_time filesystem log log_setup unit_test_framework thread regex coroutine chrono
)

find_package(libusb-1.0 1.0.22 REQUIRED)

find_package(Threads REQUIRED)
find_package(Doxygen)

set (CMAKE_CXX_STANDARD 17)

set(SENSORHUB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FMTLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fmt/src)
set(FMTLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fmt/include)
set(SENSORHUB_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

enable_testing()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(CPP_SOURCES
  ${SENSORHUB_SOURCE_DIR}/loop.cpp
  ${SENSORHUB_SOURCE_DIR}/xsens.cpp
  ${SENSORHUB_SOURCE_DIR}/device.cpp
  ${SENSORHUB_SOURCE_DIR}/datetime.cpp
  ${SENSORHUB_SOURCE_DIR}/usb.cpp
  ${SENSORHUB_SOURCE_DIR}/http.cpp
  ${SENSORHUB_SOURCE_DIR}/log.cpp
  ${SENSORHUB_SOURCE_DIR}/config.cpp
  ${SENSORHUB_SOURCE_DIR}/quantities.cpp
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
  boost_system boost_date_time boost_filesystem boost_log boost_log_setup boost_thread boost_chrono boost_coroutine
)


add_executable(sensor_hub
  ${SENSORHUB_SOURCE_DIR}/daemon.cpp
  ${CPP_SOURCES}
  ${FMTLIB_SOURCES}
)

set_target_properties(sensor_hub PROPERTIES
    LINK_FLAGS "-Wl,--no-as-needed"
    COMPILE_FLAGS "-ggdb -Wall -pedantic"
  )

macro(new_test NEW_TEST)
  add_executable(${NEW_TEST} 
    ${SENSORHUB_TEST_DIR}/${NEW_TEST}.cpp
    ${CPP_SOURCES}
    ${FMTLIB_SOURCES}
  )
  set_target_properties(${NEW_TEST} PROPERTIES 
    LINK_FLAGS "-Wl,--no-as-needed"
    COMPILE_FLAGS "-ggdb -Wall -pedantic"
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
new_test(test_device)
new_test(test_quantities)
new_test(test_datetime)
new_test(test_xsens)
new_test(test_tools)
new_test(test_parse)

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
    COMMENT "Generating Manual with TexInfo"
  )
endif()
