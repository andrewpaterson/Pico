cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(how_does_it_work
  HowDoesItWork.c
)

pico_enable_stdio_usb(how_does_it_work 1)
pico_enable_stdio_uart(how_does_it_work 1)

pico_add_extra_outputs(how_does_it_work)

target_link_libraries(how_does_it_work pico_stdlib)
