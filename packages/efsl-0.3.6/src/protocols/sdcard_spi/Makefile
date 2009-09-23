CONFDIR=../../../conf
include $(CONFDIR)/config.makefile
BASEDIR=../../base/include
INCLUDEDIRS=-I../../include -Iinclude -I$(BASEDIR) -I$(CONFDIR)
CFLAGS=$(GCFLAGS) $(INCLUDEDIRS)

OBJ=sd.o

all: efsl-prot-sdspi

efsl-prot-sdspi: $(OBJ)
	$(AR) rcs efsl-prot-sdspi.a $(OBJ)

clean:
	rm -f $(OBJ) efsl-prot-sdspi.a