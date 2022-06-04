#!/bin/bash

openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program build/how_does_it_work.elf verify reset exit"
