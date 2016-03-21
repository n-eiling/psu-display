#! /bin/bash


avr-gcc -g -Os -mmcu=atmega168p -std=gnu99 -c *.c
avr-gcc -g -mmcu=atmega168p -o flash.elf *.o
avr-objcopy -j .text -j .data -O ihex flash.elf flash.hex