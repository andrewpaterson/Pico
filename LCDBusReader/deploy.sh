#!/bin/bash

openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program build/lcd_bus_reader.elf verify reset exit"
