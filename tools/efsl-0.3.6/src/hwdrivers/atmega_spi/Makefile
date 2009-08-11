CONFDIR=../../../conf
include $(CONFDIR)/config.makefile
BASEDIR=../../base/include
INCLUDEDIRS=-I../../include -Iinclude -I$(BASEDIR) -I$(CONFDIR)
CFLAGS=$(GCFLAGS) $(INCLUDEDIRS)

OBJ=atmega_spi.o

all: efsl-hwd-atmega_spi

efsl-hwd-atmega_spi: $(OBJ)
	$(AR) rcs efsl-hwd-atmega_spi.a $(OBJ)

clean:
	rm -f $(OBJ) efsl-hwd-atmega_spi.a
