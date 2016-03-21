# by Niklas Eiling
# atmgea328

#avr-gcc -g -Os -mmcu=atmega168p -std=gnu99 -c *.c
#avr-gcc -g -mmcu=atmega168p -o flash.elf *.o
#avr-objcopy -j .text -j .data -O ihex flash.elf flash.hex

CC = avr-gcc
CFLAGS = -Wall -Os -mmcu=atmega168p -std=gnu99
LD = avr-gcc
LDFLAGS = -mmcu=atmega168p
OBJCOPY = avr-objcopy
OBJCOPYFLAGS = -j .text -j .data -O ihex 
FLASHER = avrdude
FLASHFLAGS = -p m168p -c avrisp2 -P /dev/ttyACM0

FILES := main.o MAX7219.o twimaster.o

.PHONY: all depend clean flash
all : flash

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
flash.elf : $(FILES)
	$(LD) $(LDFLAGS) -o $@ $^
	
flash.hex : flash.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	
.depend : $(FILES)
	$(CC) -MM $(CFLAGS) $(FILES:.o=.c) > $@
	
depend : .depend
	
clean :
	rm -f *.elf *.hex *.o *.d .depend *~
	
flash : flash.hex
	$(FLASHER) $(FLASHFLAGS) -U flash:w:flash.hex

include .depend