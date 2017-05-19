cmake_minimum_required(VERSION 2.8.11)
project(UKF)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-g -O0")

# FTDI Driver
find_library(ftdi_driver ftd2xx)
message("FTDI driver is at: ${ftdi_driver}")

# Python
find_package( PythonLibs 2.7 REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )

# Boost
find_package( Boost COMPONENTS python REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

# Libraries
add_library(animations animations.cc)
add_library(serial serial.cc)
target_link_libraries(serial ${ftdi_driver})

# Build the python library
add_library(neopixel_driver SHARED neopixel_ring.cc)
target_link_libraries( neopixel_driver
    serial
    animations
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)
set_target_properties( neopixel_driver PROPERTIES PREFIX "" )
