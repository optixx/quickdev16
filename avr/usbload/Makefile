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

DEBUG   = 0
TTY     = /dev/tty.PL2303-00002126
DEVICE  = atmega644
F_CPU   = 20000000
TARGET  = main
AVRDUDE = avrdude -c usbasp -p $(DEVICE)
SIZE    = avr-size
BOOT_LOADER = 2
BOOT_COMPRESS = fastlz
BOOT_ROM01  = ../../roms/qd16boot01.smc
BOOT_ROM02  = ../../roms/qd16boot02.smc


ifeq ($(DEBUG),1)
    BOOT_DEBUG = debug
    LDFLAGS =-Wl,-u,vfprintf  
    CFLAGS  =-Iusbdrv -I. -DDEBUG_LEVEL=0 
    OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o \
               main.o usb_bulk.o uart.o fifo.o sram.o crc.o debug.o \
               dump.o timer.o watchdog.o loader.o info.o shared_memory.o \
               system.o pwm.o util.o shell.o irq.o command.o testing.o
else 
    BOOT_DEBUG = nodebug
    LDFLAGS =
    CFLAGS  =-Iusbdrv -I. -DDEBUG_LEVEL=0 -DNO_DEBUG -DNO_INFO 
    OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o  main.o usb_bulk.o \
              sram.o crc.o debug.o dump.o loader.o \
              system.o util.o info.o shared_memory.o command.o irq.o \
              pwm.o
endif

ifeq ($(BOOT_LOADER), 1)
	BOOT_ROM = $(BOOT_ROM01)
else
	BOOT_ROM = $(BOOT_ROM02)
endif 

ifeq ($(BOOT_COMPRESS), rle)
	BOOT_CONVERTER = python ../../scripts/conv_rle.py
	CFLAGS += -DBOOT_COMPRESS_RLE 
	OBJECTS += rle.o 
endif 

ifeq ($(BOOT_COMPRESS), zip)
	BOOT_CONVERTER = python ../../scripts/conv_zip.py
	CFLAGS += -DBOOT_COMPRESS_ZIP
	OBJECTS += neginf/neginf.o
endif 

ifeq ($(BOOT_COMPRESS), fastlz)
	BOOT_CONVERTER = python ../../scripts/conv_fastlz.py
	CFLAGS += -DBOOT_COMPRESS_FASTLZ
	OBJECTS += fastlz.o 
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
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"

hex: main.hex
	@echo "$(TARGET) compiled for: $(DEVICE)"
	@./checksize $(TARGET).elf

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

bootconvert:
	$(BOOT_CONVERTER) $(BOOT_ROM) 

release: main.hex
	cp -v main.hex ../../files/firmware/firmware-loader0$(BOOT_LOADER)-$(BOOT_COMPRESS)-$(BOOT_DEBUG)-$$(date +%Y%m%d).hex 

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
