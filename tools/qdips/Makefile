USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs`

CC	= gcc
CFLAGS	= $(CPPFLAGS) $(USBFLAGS) -O -g -Wall -D_GNU_SOURCE --std=c99
LIBS	= $(USBLIBS)

NAME = qdips
OBJECTS = opendevice.o $(NAME).o


PROGRAM = $(NAME)$(EXE_SUFFIX)


all: $(PROGRAM)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROGRAM): $(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(LIBS)
clean:
	rm -f *.o $(PROGRAM)
