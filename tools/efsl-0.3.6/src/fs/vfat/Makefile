CONFDIR=../../../conf
include $(CONFDIR)/config.makefile
BASEDIR=../../base/include
INCLUDEDIRS=-I../../include -Iinclude -I$(BASEDIR) -I$(CONFDIR)
CFLAGS=$(GCFLAGS) $(INCLUDEDIRS)

OBJ=dir.o fat.o file.o fs.o ls.o mkfs.o time.o ui.o

all: efsl-fs-vfat

efsl-fs-vfat: $(OBJ)
	$(AR) rcs efsl-fs-vfat.a $(OBJ)

clean:
	rm -f $(OBJ) efsl-fs-vfat.a