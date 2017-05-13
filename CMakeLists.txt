cmake_minimum_required(VERSION 2.8.11)
project(UKF)

set(CMAKE_CXX_STANDARD 11)

find_library(ftdi_driver ftd2xx)
message("FTDI driver is at: ${ftdi_driver}")
add_executable(gpio_driver main.cc)
target_link_libraries(gpio_driver LINK_PUBLIC ${ftdi_driver})