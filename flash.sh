#! /bin/bash

avrdude -p m168p -c avrisp2 -P /dev/ttyACM0 -B 8 -U flash:w:flash.hex
