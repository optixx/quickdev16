# Snesram
# 2009-05-06 davdi@optixx.org
#
#
# based on
# Project snesram_bootloader
# 2007-01-07 me@anyma.ch
#
# based on
# Makefile for AVRUSBBoot
# Modified by: Thomas Fischl
# Modified: 2006-06-25

# Original file by: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile 147 2006-03-01 17:33:03Z cs $

BOOTLOADER_ADDRESS = 3800

LDFLAGS += -Wl,--section-start=.text=$(BOOTLOADER_ADDRESS)

UISP = uisp -dprog=stk500 -dserial=`echo /dev/tty.[Uu][Ss]*` -dpart=atmega16

# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega16
#-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o snesram_bootloader.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!


# symbolic targets:
all:	snesram_bootloader.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(UISP) --erase --upload --verify if=snesram_bootloader.hex
#	$(UISP) --erase --upload  if=snesram_bootloader.hex

fuse:
	$(UISP) --wr_fuse_h=0xc8
	$(UISP) --wr_fuse_l=0xef
	
avrdude: 
	avrdude -c avr910 -p atmega8 -U flash:w:snesram_bootloader.hex


clean:
	rm -f snesram_bootloader.hex snesram_bootloader.lst snesram_bootloader.obj snesram_bootloader.cof snesram_bootloader.list snesram_bootloader.map snesram_bootloader.eep.hex snesram_bootloader.bin *.o usbdrv/*.o snesram_bootloader.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
snesram_bootloader.bin:	$(OBJECTS)
	$(COMPILE) -o snesram_bootloader.bin $(OBJECTS) $(LDFLAGS)

snesram_bootloader.hex:	snesram_bootloader.bin
	rm -f snesram_bootloader.hex snesram_bootloader.eep.hex
	avr-objcopy -j .text -j .data -O ihex snesram_bootloader.bin snesram_bootloader.hex
	./checksize snesram_bootloader.bin
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	snesram_bootloader.bin
	avr-objdump -d snesram_bootloader.bin

cpp:
	$(COMPILE) -E snesram_bootloader.c
	
download:
	$(UISP) --download
