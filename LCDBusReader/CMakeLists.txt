cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(lcd_bus_reader)
target_sources(lcd_bus_reader PRIVATE 
                Source/LCDBusReader.cpp
                Source/LCDDisplay.cpp
                Source/GeneralPins.cpp
                Source/HEXDisplay.cpp
                Source/ShiftRegister.cpp
                Source/ShiftLCDDisplay.cpp
                Source/UARTComm.cpp
                Source/SPIComm.cpp
                Source/LTC6903.cpp
                Source/MAX5102.cpp
                Source/SDCard.cpp
                Source/HexToMem.cpp
                Source/W65C816Pins.cpp
                Source/W65C816Bus.cpp
                Source/W65C816Slave.cpp
                Source/W65C816Master.cpp
                Source/Keypad.cpp
                Source/PicoSound.cpp)

target_link_libraries(lcd_bus_reader
    pico_stdlib)

target_include_directories(lcd_bus_reader PRIVATE
  Source/
)

pico_enable_stdio_usb(lcd_bus_reader 1)
pico_enable_stdio_uart(lcd_bus_reader 1)

pico_add_extra_outputs(lcd_bus_reader)

target_link_libraries(lcd_bus_reader pico_stdlib hardware_spi)

	