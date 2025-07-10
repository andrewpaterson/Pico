cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(hcp65_pico_tester)
target_sources( hcp65_pico_tester PRIVATE 
                Source/HCP65PicoTester.cpp
                Source/GeneralPins.cpp
                Source/PointerFunctions.cpp
                Source/StringHelper.cpp
                Source/TesterPins.cpp
                Source/TesterState.cpp
                Source/TestFunctions.cpp
                Source/Commands.cpp)


target_link_libraries(hcp65_pico_tester
                      pico_stdlib)

target_include_directories(hcp65_pico_tester PRIVATE
  Source/
)

pico_enable_stdio_usb(hcp65_pico_tester 1)
pico_enable_stdio_uart(hcp65_pico_tester 1)

pico_add_extra_outputs(hcp65_pico_tester)

target_link_libraries(hcp65_pico_tester pico_stdlib hardware_spi)
