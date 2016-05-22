.PHONY: all clean distclean install uninstall

CC=gcc
CFLAGS+=-I. -Wall -W -O3
ifneq ($(findstring darwin,$(OSTYPE)),darwin) # for example "darwin7.0"
# On Mac OS X using -s gives the following warning:
# ld: warning: option -s is obsolete and being ignored
LDFLAGS+=-s
endif
TARGETS=


include config.mak


ifdef USE_DISCMAGE
LIBNAME_DM=discmage
endif

# The test for Cygwin should be done before the test for DJGPP, because the
# environment variable DJGPP can be set in Bash for people who have installed
# both GCC (and friends) ports.

GCC_WIN=0
# test cygwin before DJGPP; OSTYPE is not exported
ifeq ($(OSTYPE),cygwin)
GCC_WIN=1
endif
# test msys before DJGPP; OSTYPE is not exported; MSYS, MinGW's POSIX build environment
ifeq ($(OSTYPE),msys)
GCC_WIN=1
endif

ifeq ($(GCC_WIN),1)

ifdef USE_DISCMAGE
FULLLIBNAME_DM=$(LIBNAME_DM).dll
ifndef DLOPEN
LDFLAGS+=libdiscmage/$(LIBNAME_DM).a
endif
endif

else
ifdef DJGPP

ifdef USE_DISCMAGE
FULLLIBNAME_DM=$(LIBNAME_DM).dxe
ifndef DLOPEN
LDFLAGS+=libdiscmage/$(LIBNAME_DM).a
endif
endif

else                                    # UNIX, BeOS or Mac OS X (Darwin)

ifeq ($(findstring openbsd,$(OSTYPE)),openbsd) # for example "openbsd3.4"
# i386_iopl() is located in libi386.a
LDFLAGS+=$(LIBI386)
endif

ifdef USE_DISCMAGE
ifeq ($(findstring darwin,$(OSTYPE)),darwin) # for example "darwin7.0"
FULLLIBNAME_DM=$(LIBNAME_DM).dylib
else
FULLLIBNAME_DM=$(LIBNAME_DM).so
endif
endif

ifdef DLOPEN

ifneq ($(OSTYPE),beos)
ifeq ($(findstring freebsd,$(OSTYPE)),) # false if OSTYPE contains "freebsd"
ifeq ($(findstring openbsd,$(OSTYPE)),) # false if OSTYPE contains "openbsd"
LDFLAGS+=-ldl
endif
endif
endif

else                                    # DLOPEN
ifdef USE_DISCMAGE                      # GNU specific: "simply expanded variable"
FULLLIBNAME_DM:=$(addprefix lib,$(FULLLIBNAME_DM))
LDFLAGS+=-Llibdiscmage -l$(LIBNAME_DM)
endif

endif

endif                                   # DJGPP
endif                                   # GCC_WIN

TARGETS+=libdiscmage/$(FULLLIBNAME_DM)


ifdef USE_LIBCD64
LDFLAGS+=backup/libcd64/libcd64.a
TARGETS+=backup/libcd64/libcd64.a
endif


OBJECTS=ucon64.o ucon64_dat.o ucon64_misc.o ucon64_opts.o \
        misc/chksum.o misc/file.o misc/getopt2.o misc/misc.o misc/parallel.o \
        misc/property.o misc/string.o misc/term.o \
        console/atari.o console/coleco.o console/console.o console/dc.o \
        console/gb.o console/gba.o console/genesis.o console/jaguar.o \
        console/lynx.o console/n64.o console/nds.o console/neogeo.o \
        console/nes.o console/ngp.o console/pce.o console/psx.o console/sms.o \
        console/snes.o console/swan.o console/vboy.o \
        backup/backup.o backup/cc2.o backup/cd64.o backup/cmc.o backup/dex.o \
        backup/doctor64.o backup/doctor64jr.o backup/f2a.o backup/fal.o \
        backup/ffe.o backup/fig.o backup/gbx.o backup/gd.o \
        backup/interceptor.o backup/lynxit.o backup/mccl.o backup/mcd.o \
        backup/md-pro.o backup/mgd.o backup/msg.o backup/nfc.o \
        backup/pce-pro.o backup/pl.o backup/psxpblib.o backup/quickdev16.o \
        backup/sc.o backup/sflash.o backup/smc.o backup/smd.o \
        backup/smsgg-pro.o backup/spsc.o backup/ssc.o backup/swc.o \
        backup/tototek.o backup/ufo.o backup/yoko.o backup/z64.o \
        patch/aps.o patch/bsl.o patch/gg.o patch/ips.o patch/patch.o patch/ppf.o
ifneq ($(OSTYPE),cygwin)
OBJECTS+=misc/getopt.o
endif
ifdef USE_ZLIB
LDFLAGS+=-lz
OBJECTS+=misc/archive.o misc/map.o misc/unzip.o
endif
ifdef USE_USB
LDFLAGS+=-lusb
OBJECTS+=misc/usb.o
endif

ifdef DLOPEN
OBJECTS+=misc/dlopen.o
ifndef USE_ZLIB
ifeq ($(GCC_WIN),1)
else
ifdef DJGPP                             # DJGPP code in dlopen needs map code
OBJECTS+=misc/map.o
endif                                   # DJGPP
endif                                   # GCC_WIN
endif                                   # USE_ZLIB
else
ifeq ($(GCC_WIN),1)                     # Cygwin/MinGW code in ucon64_misc needs dlopen code
OBJECTS+=misc/dlopen.o
endif                                   # GCC_WIN
endif                                   # DLOPEN


TARGET=ucon64
ifeq ($(GCC_WIN),1)
TARGET:=$(addsuffix .exe,$(TARGET))     # adding .exe avoids "problems" with Cygwin/MinGW
else
ifdef DJGPP                             # OSTYPE is not defined by default on DOS
TARGET:=$(addsuffix .exe,$(TARGET))
endif                                   # DJGPP
endif                                   # GCC_WIN
TARGETS+=$(TARGET)

all: $(TARGETS)


CLEAN_CMD=rm -f $(TARGET) $(OBJECTS) *.core *.stackdump *~ */*~ */*/*~; \
cd libdiscmage && $(MAKE) clean; \
cd ../backup/libcd64 && $(MAKE) clean

clean:
ifeq ($(GCC_WIN),1)
	$(CLEAN_CMD)
else
ifdef DJGPP
	del *.o
	del patch\*.o
	del console\*.o
	del backup\*.o
	del misc\*.o
	del $(TARGET)
	cd libdiscmage
	$(MAKE) clean
	cd ../backup/libcd64
	$(MAKE) clean
	cd ../..
else                                    # UNIX, BeOS or Mac OS X (Darwin)
	$(CLEAN_CMD)
endif                                   # DJGPP
endif                                   # GCC_WIN


DISTCLEAN_CMD=rm -f config.mak config.log config.status config.cache config.h; \
cd libdiscmage && $(MAKE) distclean; \
cd ../backup/libcd64 && $(MAKE) clean
# libcd64 Makefile has no distclean target

distclean: clean
ifeq ($(GCC_WIN),1)
	$(DISTCLEAN_CMD)
else
ifdef DJGPP
	del config.mak
	del config.log
	del config.status
	del config.cache
	del config.h
	cd libdiscmage
	$(MAKE) distclean
	cd ../backup/libcd64
	$(MAKE) clean
	cd ../..
else
	$(DISTCLEAN_CMD)
endif                                   # DJGPP
endif                                   # GCC_WIN


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

backup/cd64.o: backup/cd64.c
	$(CC) $(CFLAGS) -Ibackup/libcd64 -c $< -o $@


ifdef USE_DISCMAGE
libdiscmage/$(FULLLIBNAME_DM):
ifeq ($(GCC_WIN),1)
	cd libdiscmage && $(MAKE)
else
ifdef DJGPP
	cd libdiscmage
	$(MAKE)
	cd ..
else
	cd libdiscmage && $(MAKE)
endif                                   # DJGPP
endif                                   # GCC_WIN
endif                                   # USE_DISCMAGE


ifdef USE_LIBCD64
backup/libcd64/libcd64.a:
ifeq ($(GCC_WIN),1)
	cd backup/libcd64 && $(MAKE)
else
ifdef DJGPP
	cd backup/libcd64
	$(MAKE)
	cd ../..
else
	cd backup/libcd64 && $(MAKE)
endif                                   # DJGPP
endif                                   # GCC_WIN
endif                                   # USE_LIBCD64


ifdef USE_LIBCD64
$(TARGET): backup/libcd64/libcd64.a $(OBJECTS)
else
$(TARGET): $(OBJECTS)
endif                                   # USE_LIBCD64
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@


install:
ifeq ($(OSTYPE),cygwin)                 # test cygwin before DJGPP
else
ifeq ($(OSTYPE),msys)                   # test msys before DJGPP
else
ifdef DJGPP
else
ifeq ($(OSTYPE),beos)
	./install_beos.sh
else
	./install.sh
endif                                   # beos
endif                                   # DJGPP
endif                                   # msys
endif                                   # cygwin
ifndef DLOPEN
	cd libdiscmage && $(MAKE) install
endif


uninstall:
ifeq ($(OSTYPE),cygwin)                 # test cygwin before DJGPP
else
ifeq ($(OSTYPE),msys)                   # test msys before DJGPP
else
ifdef DJGPP
else
ifeq ($(OSTYPE),beos)
	rm -f $(HOME)/config/bin/$(TARGET)
else
	rm -f /usr/local/bin/$(TARGET)
endif                                   # beos
endif                                   # DJGPP
endif                                   # msys
endif                                   # cygwin
ifndef DLOPEN
	cd libdiscmage && $(MAKE) uninstall
endif


# Dependencies

ARCHIVE_H_DEPS=misc/archive.h misc/unzip.h
BSWAP_H_DEPS=misc/bswap.h misc/itypes.h
CHKSUM_H_DEPS=misc/chksum.h misc/itypes.h
GETOPT2_H_DEPS=misc/getopt2.h misc/getopt.h
MISC_H_DEPS=misc/misc.h misc/snprintf.h
TERM_H_DEPS=misc/term.h misc/snprintf.h
UCON64_H_DEPS=ucon64.h misc/itypes.h ucon64_defines.h
UCON64_DAT_H_DEPS=ucon64_dat.h $(GETOPT2_H_DEPS) misc/itypes.h
UCON64_MISC_H_DEPS=ucon64_misc.h $(GETOPT2_H_DEPS) misc/itypes.h \
                   libdiscmage/libdiscmage.h
UCON64_OPTS_H_DEPS=ucon64_opts.h $(UCON64_H_DEPS)
CONSOLE_DEPS=$(GETOPT2_H_DEPS) $(UCON64_H_DEPS)
CONSOLE_H_DEPS=console/console.h $(CONSOLE_DEPS)
SNES_H_DEPS=console/snes.h $(CONSOLE_DEPS) backup/fig.h
CD64LIB_H_DEPS=backup/libcd64/ultra64/host/cd64lib.h \
               backup/libcd64/ultra64/rom.h

ucon64.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(GETOPT2_H_DEPS) \
          $(MISC_H_DEPS) misc/parallel.h misc/property.h $(TERM_H_DEPS) \
          $(UCON64_H_DEPS) $(UCON64_DAT_H_DEPS) $(UCON64_MISC_H_DEPS) \
          $(UCON64_OPTS_H_DEPS) \
          console/atari.h console/coleco.h $(CONSOLE_H_DEPS) console/dc.h \
          console/gb.h console/gba.h console/genesis.h console/jaguar.h \
          console/lynx.h console/n64.h console/nds.h console/neogeo.h \
          console/nes.h console/ngp.h console/pce.h console/psx.h \
          console/sms.h $(SNES_H_DEPS) console/swan.h console/vboy.h \
          backup/backup.h backup/cd64.h backup/cmc.h backup/dex.h \
          backup/doctor64.h backup/doctor64jr.h backup/f2a.h backup/fal.h \
          backup/gbx.h backup/gd.h backup/lynxit.h backup/mccl.h backup/mcd.h \
          backup/md-pro.h backup/msg.h backup/pce-pro.h backup/pl.h \
          backup/quickdev16.h backup/sflash.h backup/smc.h backup/smd.h \
          backup/smsgg-pro.h backup/swc.h \
          patch/aps.h patch/bsl.h patch/gg.h patch/ips.h patch/patch.h \
          patch/ppf.h
ucon64_dat.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
              misc/property.h misc/string.h $(UCON64_DAT_H_DEPS) \
              $(UCON64_MISC_H_DEPS) \
              console/atari.h console/coleco.h $(CONSOLE_H_DEPS) console/dc.h \
              console/gb.h console/gba.h console/genesis.h console/jaguar.h \
              console/lynx.h console/n64.h console/neogeo.h console/nes.h \
              console/ngp.h console/pce.h console/sms.h $(SNES_H_DEPS) \
              console/swan.h console/vboy.h backup/backup.h
ucon64_misc.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) $(CHKSUM_H_DEPS) \
               misc/file.h $(MISC_H_DEPS) misc/property.h misc/string.h \
               $(TERM_H_DEPS) $(UCON64_H_DEPS) $(UCON64_DAT_H_DEPS) \
               $(UCON64_MISC_H_DEPS) misc/dlopen.h
ucon64_opts.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
               misc/parallel.h misc/string.h $(UCON64_DAT_H_DEPS) \
               $(UCON64_MISC_H_DEPS) $(UCON64_OPTS_H_DEPS) \
               console/dc.h console/gb.h console/gba.h console/genesis.h \
               console/lynx.h console/n64.h console/nds.h console/neogeo.h \
               console/nes.h console/pce.h console/sms.h $(SNES_H_DEPS) \
               console/swan.h \
               backup/backup.h backup/cd64.h backup/cmc.h backup/dex.h \
               backup/doctor64.h backup/doctor64jr.h backup/f2a.h backup/fal.h \
               backup/gbx.h backup/gd.h backup/lynxit.h backup/mccl.h \
               backup/mcd.h backup/md-pro.h backup/msg.h backup/pce-pro.h \
               backup/pl.h backup/quickdev16.h backup/sflash.h backup/smc.h \
               backup/smd.h backup/smsgg-pro.h backup/swc.h \
               patch/aps.h patch/bsl.h patch/gg.h patch/ips.h patch/ppf.h
backup/backup.o: config.h backup/backup.h $(GETOPT2_H_DEPS)
backup/cc2.o: config.h $(UCON64_H_DEPS) backup/cc2.h $(GETOPT2_H_DEPS)
backup/cd64.o: config.h $(CD64LIB_H_DEPS) $(ARCHIVE_H_DEPS) $(MISC_H_DEPS) \
               misc/parallel.h $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) \
               backup/cd64.h $(GETOPT2_H_DEPS)
backup/cmc.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) $(MISC_H_DEPS) \
              misc/parallel.h $(TERM_H_DEPS) $(UCON64_H_DEPS) \
              $(UCON64_MISC_H_DEPS) backup/cmc.h $(GETOPT2_H_DEPS)
backup/dex.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/parallel.h \
              $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) backup/dex.h \
              backup/psxpblib.h $(GETOPT2_H_DEPS)
backup/doctor64jr.o: config.h $(ARCHIVE_H_DEPS) misc/parallel.h \
                     $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) \
                     backup/doctor64jr.h $(GETOPT2_H_DEPS)
backup/doctor64.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/parallel.h \
                   $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) backup/doctor64.h \
                   $(GETOPT2_H_DEPS)
backup/f2a.o: config.h misc/usb.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) \
              misc/file.h $(MISC_H_DEPS) misc/parallel.h misc/property.h \
              $(UCON64_MISC_H_DEPS) console/gba.h backup/f2a.h $(GETOPT2_H_DEPS)
backup/fal.o: config.h $(ARCHIVE_H_DEPS) misc/parallel.h $(TERM_H_DEPS) \
              $(UCON64_MISC_H_DEPS) console/gba.h backup/fal.h $(GETOPT2_H_DEPS)
backup/ffe.o: config.h $(MISC_H_DEPS) misc/parallel.h $(TERM_H_DEPS) \
              $(UCON64_H_DEPS) backup/ffe.h
backup/fig.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
              $(SNES_H_DEPS) backup/ffe.h backup/fig.h $(GETOPT2_H_DEPS)
backup/gbx.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
              misc/parallel.h $(TERM_H_DEPS) $(UCON64_MISC_H_DEPS) \
              console/gb.h backup/gbx.h $(GETOPT2_H_DEPS)
backup/gd.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
             misc/parallel.h misc/string.h $(TERM_H_DEPS) \
             $(UCON64_MISC_H_DEPS) $(SNES_H_DEPS) backup/gd.h $(GETOPT2_H_DEPS)
backup/interceptor.o: config.h backup/interceptor.h $(GETOPT2_H_DEPS)
backup/lynxit.o: config.h $(ARCHIVE_H_DEPS) misc/parallel.h $(TERM_H_DEPS) \
                 $(UCON64_MISC_H_DEPS) console/lynx.h backup/lynxit.h \
                 $(GETOPT2_H_DEPS)
backup/mccl.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/parallel.h \
               $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) backup/mccl.h \
               $(GETOPT2_H_DEPS)
backup/mcd.o: config.h $(ARCHIVE_H_DEPS) $(MISC_H_DEPS) misc/parallel.h \
              $(TERM_H_DEPS) $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) \
              backup/mcd.h $(GETOPT2_H_DEPS)
backup/md-pro.o: config.h $(ARCHIVE_H_DEPS) $(UCON64_H_DEPS) \
                 $(UCON64_MISC_H_DEPS) backup/tototek.h backup/md-pro.h \
                 $(GETOPT2_H_DEPS)
backup/mgd.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/string.h \
              $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) backup/mgd.h \
              $(GETOPT2_H_DEPS)
backup/msg.o: config.h $(ARCHIVE_H_DEPS) $(MISC_H_DEPS) $(UCON64_H_DEPS) \
              $(UCON64_MISC_H_DEPS) backup/ffe.h backup/msg.h $(GETOPT2_H_DEPS)
backup/nfc.o: config.h backup/nfc.h $(GETOPT2_H_DEPS)
backup/pce-pro.o: config.h $(ARCHIVE_H_DEPS) $(UCON64_H_DEPS) \
                  $(UCON64_MISC_H_DEPS) backup/tototek.h backup/pce-pro.h \
                  $(GETOPT2_H_DEPS)
backup/pl.o: config.h $(ARCHIVE_H_DEPS) misc/parallel.h $(UCON64_H_DEPS) \
             $(UCON64_MISC_H_DEPS) backup/pl.h $(GETOPT2_H_DEPS)
backup/psxpblib.o: config.h misc/parallel.h $(UCON64_H_DEPS) backup/psxpblib.h
backup/quickdev16.o: config.h $(ARCHIVE_H_DEPS) misc/itypes.h $(MISC_H_DEPS) \
                     $(TERM_H_DEPS) misc/usb.h $(UCON64_H_DEPS) \
                     $(UCON64_MISC_H_DEPS) $(SNES_H_DEPS) backup/quickdev16.h
backup/sc.o: config.h backup/sc.h $(GETOPT2_H_DEPS)
backup/sflash.o: config.h $(ARCHIVE_H_DEPS) $(UCON64_H_DEPS) \
                 $(UCON64_MISC_H_DEPS) backup/tototek.h backup/sflash.h \
                 $(GETOPT2_H_DEPS)
backup/smc.o: config.h $(ARCHIVE_H_DEPS) $(UCON64_H_DEPS) \
              $(UCON64_MISC_H_DEPS) backup/ffe.h backup/smc.h $(GETOPT2_H_DEPS)
backup/smd.o: config.h $(ARCHIVE_H_DEPS) $(MISC_H_DEPS) $(UCON64_H_DEPS) \
              $(UCON64_MISC_H_DEPS) backup/ffe.h backup/smd.h $(GETOPT2_H_DEPS)
backup/smsgg-pro.o: config.h $(ARCHIVE_H_DEPS) $(UCON64_H_DEPS) \
                    $(UCON64_MISC_H_DEPS) backup/tototek.h backup/smsgg-pro.h \
                    $(GETOPT2_H_DEPS)
backup/spsc.o: config.h backup/spsc.h $(GETOPT2_H_DEPS)
backup/ssc.o: config.h backup/ssc.h $(GETOPT2_H_DEPS)
backup/swc.o: config.h $(ARCHIVE_H_DEPS) $(MISC_H_DEPS) $(UCON64_MISC_H_DEPS) \
              $(SNES_H_DEPS) backup/ffe.h backup/swc.h $(GETOPT2_H_DEPS)
backup/tototek.o: config.h $(BSWAP_H_DEPS) $(MISC_H_DEPS) misc/parallel.h \
                  $(TERM_H_DEPS) backup/tototek.h
backup/ufo.o: config.h backup/ufo.h $(GETOPT2_H_DEPS)
backup/yoko.o: config.h backup/yoko.h $(GETOPT2_H_DEPS)
backup/z64.o: config.h backup/z64.h $(GETOPT2_H_DEPS)
console/atari.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/string.h \
                 $(TERM_H_DEPS) $(UCON64_MISC_H_DEPS) console/atari.h \
                 backup/cc2.h backup/spsc.h backup/yoko.h $(CONSOLE_DEPS)
console/coleco.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
                  console/coleco.h backup/backup.h $(CONSOLE_DEPS)
console/console.o: config.h $(CONSOLE_H_DEPS)
console/dc.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/property.h \
              misc/string.h $(UCON64_MISC_H_DEPS) console/dc.h $(CONSOLE_DEPS)
console/gba.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) misc/file.h \
               $(MISC_H_DEPS) misc/property.h misc/string.h \
               $(UCON64_MISC_H_DEPS) $(CONSOLE_H_DEPS) console/gba.h \
               backup/backup.h backup/fal.h
console/gb.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
              misc/string.h $(UCON64_MISC_H_DEPS) $(CONSOLE_H_DEPS) \
              console/gb.h console/nes.h backup/backup.h backup/mgd.h \
              backup/ssc.h
console/genesis.o: config.h $(ARCHIVE_H_DEPS) $(CHKSUM_H_DEPS) misc/file.h \
                   $(MISC_H_DEPS) $(UCON64_MISC_H_DEPS) console/genesis.h \
                   backup/md-pro.h backup/mgd.h backup/smd.h $(CONSOLE_DEPS)
console/jaguar.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
                  console/jaguar.h backup/backup.h $(CONSOLE_DEPS)
console/lynx.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) misc/file.h \
                $(UCON64_MISC_H_DEPS) console/lynx.h backup/backup.h \
                $(CONSOLE_DEPS)
console/n64.o: config.h $(ARCHIVE_H_DEPS) $(CHKSUM_H_DEPS) misc/file.h \
               $(MISC_H_DEPS) $(UCON64_MISC_H_DEPS) console/n64.h \
               backup/backup.h backup/doctor64.h backup/z64.h $(CONSOLE_DEPS)
console/nds.o: config.h $(ARCHIVE_H_DEPS) $(CHKSUM_H_DEPS) misc/file.h \
               $(MISC_H_DEPS) $(UCON64_MISC_H_DEPS) $(CONSOLE_H_DEPS) \
               console/nds.h backup/backup.h backup/nfc.h
console/neogeo.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
                  console/neogeo.h backup/backup.h $(CONSOLE_DEPS)
console/nes.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) $(CHKSUM_H_DEPS) \
               misc/file.h $(MISC_H_DEPS) misc/string.h $(UCON64_MISC_H_DEPS) \
               console/nes.h backup/smc.h $(CONSOLE_DEPS)
console/ngp.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
               console/ngp.h backup/backup.h backup/pl.h $(CONSOLE_DEPS)
console/pce.o: config.h $(ARCHIVE_H_DEPS) $(CHKSUM_H_DEPS) misc/file.h \
               $(MISC_H_DEPS) misc/string.h $(UCON64_MISC_H_DEPS) \
               console/pce.h backup/mgd.h backup/msg.h backup/pce-pro.h \
               $(CONSOLE_DEPS)
console/psx.o: config.h console/psx.h $(CONSOLE_DEPS)
console/sms.o: config.h $(ARCHIVE_H_DEPS) $(CHKSUM_H_DEPS) misc/file.h \
               $(MISC_H_DEPS) misc/string.h $(UCON64_MISC_H_DEPS) \
               console/sms.h backup/mgd.h backup/smd.h backup/smsgg-pro.h \
               $(CONSOLE_DEPS)
console/snes.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) $(CHKSUM_H_DEPS) \
                misc/file.h $(MISC_H_DEPS) misc/string.h $(UCON64_MISC_H_DEPS) \
                $(CONSOLE_H_DEPS) $(SNES_H_DEPS) backup/backup.h backup/gd.h \
                backup/mgd.h backup/swc.h backup/ufo.h
console/swan.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
                $(UCON64_MISC_H_DEPS) console/swan.h backup/backup.h \
                $(CONSOLE_DEPS)
console/vboy.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_MISC_H_DEPS) \
                $(CONSOLE_H_DEPS) console/vboy.h backup/backup.h
misc/archive.o: config.h $(ARCHIVE_H_DEPS) misc/map.h misc/unzip.h
misc/chksum.o: config.h $(BSWAP_H_DEPS) $(CHKSUM_H_DEPS) $(MISC_H_DEPS)
misc/dlopen.o: config.h misc/dlopen.h misc/dxedll_pub.h misc/map.h
misc/file.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS)
misc/getopt.o: config.h misc/getopt.h
misc/getopt2.o: config.h misc/file.h $(GETOPT2_H_DEPS)
misc/map.o: config.h misc/map.h
misc/misc.o: config.h $(ARCHIVE_H_DEPS) misc/file.h misc/itypes.h $(MISC_H_DEPS)
misc/parallel.o: config.h misc/dlopen.h misc/file.h misc/parallel.h \
                 $(UCON64_H_DEPS)
misc/property.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
                 misc/property.h misc/string.h
misc/snprintf.o: config.h
misc/string.o: config.h misc/string.h
misc/term.o: config.h $(TERM_H_DEPS) ucon64_defines.h
misc/unzip.o: config.h misc/unzip.h
misc/usb.o: config.h misc/usb.h
patch/aps.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) misc/file.h \
             $(MISC_H_DEPS) $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) patch/aps.h \
             $(GETOPT2_H_DEPS)
patch/bsl.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(UCON64_H_DEPS) \
             $(UCON64_MISC_H_DEPS) patch/bsl.h $(GETOPT2_H_DEPS)
patch/gg.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
            $(UCON64_MISC_H_DEPS) $(SNES_H_DEPS) patch/gg.h $(GETOPT2_H_DEPS)
patch/ips.o: config.h $(ARCHIVE_H_DEPS) misc/file.h $(MISC_H_DEPS) \
             $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) patch/ips.h \
             $(GETOPT2_H_DEPS)
patch/patch.o: config.h $(UCON64_H_DEPS) $(UCON64_MISC_H_DEPS) patch/patch.h \
               $(GETOPT2_H_DEPS)
patch/ppf.o: config.h $(ARCHIVE_H_DEPS) $(BSWAP_H_DEPS) misc/file.h \
             $(MISC_H_DEPS) misc/string.h $(UCON64_H_DEPS) \
             $(UCON64_MISC_H_DEPS) patch/ppf.h $(GETOPT2_H_DEPS)
