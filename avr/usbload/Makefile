# =====================================================================================
#
# ________        .__        __    ________               ____  ________
# \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
#  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
# /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
# \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
#        \__>             \/     \/        \/     \/                 \/
#                                  www.optixx.org
#
#
#        Version:  1.0
#        Created:  07/21/2009 03:32:16 PM
#         Author:  david@optixx.org
#       Based on:  custom-class, a basic USB example 
#         Author:  Christian Starkjohann 
# =====================================================================================

DEBUG   = 1
TTY     = /dev/tty.PL2303-00002126
DEVICE  = atmega644
F_CPU   = 20000000	# in Hz
TARGET  = main
AVRDUDE = avrdude -c usbasp -p $(DEVICE)
SIZE    = avr-size




ifeq ($(DEBUG),1)
    LDFLAGS =  -Wl,-u,vfprintf -lprintf_flt 
    CFLAGS  = -Iusbdrv -I. -DDEBUG_LEVEL=0 
    OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o \
               main.o usb_bulk.o uart.o fifo.o sram.o crc.o debug.o \
               dump.o timer.o watchdog.o rle.c loader.o info.o shared_memory.o \
               command.o mmc.o fat.o file.o dir.o testing.o
else 
    LDFLAGS =  -Wl,-u
    CFLAGS  = -Iusbdrv -I. -DDEBUG_LEVEL=0 -DNO_DEBUG -DNO_INFO
    OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o  main.o usb_bulk.o \
              uart.o fifo.o sram.o crc.o debug.o dump.o timer.o watchdog.o rle.c loader.o \
              info.o shared_memory.o command.o
endif

COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

##############################################################################
# Fuse values for particular devices
##############################################################################
# http://www.engbedded.com/fusecalc/
FUSE_L  = 0xf7
FUSE_H  = 0xda

all: hex

help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"

hex: main.hex
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(DEVICE)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

program: flash fuse

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i


.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@


usbdrv:
	cp -r ../../../usbdrv .

main.elf: usbdrv $(OBJECTS)	# usbdrv dependency only needed because we copy it
	$(COMPILE) -o main.elf $(OBJECTS) $(LDFLAGS) 

main.hex: main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf *.o usbdrv/*.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s
