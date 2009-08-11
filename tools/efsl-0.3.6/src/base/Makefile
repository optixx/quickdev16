CONFDIR=../../conf
include $(CONFDIR)/config.makefile
# VFAT includes do NOT belong here, remove them when VFS is implemented !
INC_VFAT=../fs/vfat/include
INCLUDEDIRS=-I../include -Iinclude -I$(CONFDIR) -I$(INC_VFAT)
CFLAGS= $(GCFLAGS) $(INCLUDEDIRS)

OBJ=debug.o disc.o efs.o extract.o sextract.o interface.o ioman.o partition.o plibc.o

all: efsl-base

efsl-base: $(OBJ)
	$(AR) rcs efsl-base.a $(OBJ)

clean:
	rm -f $(OBJ) efsl-base.a