cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(lcd_bus_reader
  src/LCDBusReader.cpp
  src/LCDDisplay.cpp
  src/ShiftRegister.cpp
  src/GeneralPins.cpp
  src/ShiftLCDDisplay.cpp
)

target_include_directories(lcd_bus_reader PRIVATE
  src/
)

pico_enable_stdio_usb(lcd_bus_reader 1)
pico_enable_stdio_uart(lcd_bus_reader 1)

pico_add_extra_outputs(lcd_bus_reader)

target_link_libraries(lcd_bus_reader pico_stdlib)
