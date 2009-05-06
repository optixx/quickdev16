# Inspired by the PowerSwitch makefile ba Christian Starkjohann, 2005 by OBJECTIVE DEVELOPMENT Software GmbH

# Use the following 3 lines on Unix (uncomment the framework on Mac OS X):
USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs` #-framework CoreFoundation
EXE_SUFFIX =

# Use the following 3 lines on Windows and comment out the 3 above. You may
# have to change the include paths to where you installed libusb-win32
#USBFLAGS = -I/usr/local/include
#USBLIBS = -L/usr/local/lib -lusb
#EXE_SUFFIX = .exe

CC		= gcc
CFLAGS	= $(USBFLAGS) -O -Wall
LIBS	= $(USBLIBS)

PROGRAM = snesram_bootuploader$(EXE_SUFFIX)


all: $(PROGRAM)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROGRAM): snesram_bootuploader.o
	$(CC) -o $(PROGRAM) snesram_bootuploader.o $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f *.o $(PROGRAM)
