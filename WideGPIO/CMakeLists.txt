cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(wide_gpio)
target_sources(wide_gpio PRIVATE 
                Source/WideGPIO.cpp)

target_link_libraries(wide_gpio
    pico_stdlib)

target_include_directories(wide_gpio PRIVATE
  Source/
)

pico_enable_stdio_usb(wide_gpio 1)
pico_enable_stdio_uart(wide_gpio 1)

pico_add_extra_outputs(wide_gpio)

target_link_libraries(wide_gpio pico_stdlib hardware_spi)
