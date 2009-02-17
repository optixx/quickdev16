# app defs
ifndef LIBUSB
		LIBUSB = "/opt/local/"
endif

# tool defs
CFLAGS = -W -Wall -g -I$(LIBUSB)/include
LIBS =  -L$(LIBUSB)/lib -lusb

all: custom_client uploader

custom_client: custom_client.o
	$(CC) -o $@ $< $(LIBS)


uploader: uploader.o
	$(CC) -o $@ $< $(LIBS)

clean:
	$(RM) custom_client uploader  *.o 
	
	
