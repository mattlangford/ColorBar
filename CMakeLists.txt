cmake_minimum_required(VERSION 2.8.11)
project(UKF)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-g -O0")

add_library(serial serial.cc)
target_link_libraries(serial ${ftdi_driver})

find_library(ftdi_driver ftd2xx)
message("FTDI driver is at: ${ftdi_driver}")
add_executable(gpio_driver neopixel_ring.cc)
target_link_libraries(gpio_driver serial)
