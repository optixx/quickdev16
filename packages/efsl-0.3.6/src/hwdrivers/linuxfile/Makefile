CONFDIR=../../../conf
include $(CONFDIR)/config.makefile
BASEDIR=../../base/include
INCLUDEDIRS=-I../../include -Iinclude -I$(BASEDIR) -I$(CONFDIR)
CFLAGS=$(GCFLAGS) $(INCLUDEDIRS)

OBJ=helper.o linuxfile.o

all: efsl-hwd-linuxfile

efsl-hwd-linuxfile: $(OBJ)
	$(AR) rcs efsl-hwd-linuxfile.a $(OBJ)

clean:
	rm -f $(OBJ) efsl-hwd-linuxfile.a