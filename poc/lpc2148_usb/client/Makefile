# app defs
EXE = custom_client
ifndef LIBUSB
		LIBUSB = "/opt/local/"
endif

# tool defs
CFLAGS = -W -Wall -g -I$(LIBUSB)/include
LIBS =  -L$(LIBUSB)/lib -lusb

all: $(EXE)

$(EXE): main.o
	$(CC) -o $(EXE) $< $(LIBS)

clean:
	$(RM) $(EXE) main.o 
	
	
