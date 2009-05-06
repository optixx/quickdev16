# Name: Makefile
# Project: gnusb
# Author: Michael Egger me@anyma.ch
#
# Based on Makefile for Powerswitch
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile 147 2006-03-01 17:33:03Z cs $
# Target MCU: Atmel Atmega16-16

#change DEVICE o the name of your gnusb (as defined in usbconfig.h)
DEVICE = gnusb

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega16 #-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o gnusb.o main.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!


# symbolic targets:
all:	main.hex

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

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o usbdrv/*.o gnusb.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
main.bin:	$(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS)

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex


disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c

## ---------------------------------------------------
## wrapper for gnusboot bootloader functions
boot:  	all
	../../binaries/./gnusboot -enter -device $(DEVICE)
	../../binaries/./gnusboot -upload main.hex

run: 
	../../binaries/./gnusboot -clear_flag
	../../binaries/./gnusboot -leave
