cmake_minimum_required(VERSION 2.8)

project(SensorHub)

add_subdirectory(doc)

find_package(Boost
  1.67.0
  REQUIRED
  system date_time filesystem unit_test_framework
)

set(CXX11_FLAGS "-std=gnu++11")

set(SENSORHUB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SENSORHUB_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

enable_testing()

set(CPP_SOURCES
  ${SENSORHUB_SOURCE_DIR}/usb.cpp
  ${SENSORHUB_SOURCE_DIR}/xsens.cpp
)

include_directories(
  ${Boost_INCLUDE_DIR}
)

link_directories(
  ${Boost_LIBRARY_DIR_RELEASE}
)

add_executable(sensor_hub
  ${SENSORHUB_SOURCE_DIR}/daemon.cpp
  ${CPP_SOURCES}
)

macro(new_test NEW_TEST)
  add_executable(${NEW_TEST} 
    ${SENSORHUB_TEST_DIR}/${NEW_TEST}.cpp
    ${CPP_SOURCES}
  )
  set_target_properties(${NEW_TEST} PROPERTIES 
    LINK_FLAGS "-Wl,--no-as-needed"
    #COMPILE_FLAGS " "
    RUNTIME_OUTPUT_DIRECTORY test
  )
  target_link_libraries(${NEW_TEST} 
    boost_system boost_date_time boost_filesystem boost_unit_test_framework
  )
  add_test(${NEW_TEST} test/${NEW_TEST})
endmacro()

new_test(test_asio_buffer)

