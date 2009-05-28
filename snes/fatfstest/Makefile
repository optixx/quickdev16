# SDK Config


PLATFORM=$(shell uname)

ifeq ($(PLATFORM),Linux)
    # Linux Wine 
    SDK=/home/david/.wine/drive_c/65xx_FreeSDK
    WINE=wine
    EMU=zsnes
    EMU_DEBUG=/home/david/Devel/arch/snes/tools/zsnes_linux_debug151/src/zsnesd -d 
else
    # Mac Wine 
    SDK=/Users/david/.wine/drive_c/65xx_FreeSDK
    WINE=wine
    EMU=zsnes
endif

CC=$(WINE) $(SDK)/bin/WDC816CC.exe
AS=$(WINE) $(SDK)/bin/WDC816AS.exe
LD=$(WINE) $(SDK)/bin/WDCLN.exe


# Project

INC=$(SDK)/include
LIBS=$(SDK)/lib/cc
OBJS=StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj
APP=fatfs.smc
GFX=debugfont

all:  $(APP)

run:  
	$(EMU) $(APP)

debugger:  
	$(EMU_DEBUG) $(APP)

upload:
	ucon64 -chk $(APP)
	cp -rv $(APP) /Volumes/SNES
	sync
	diskutil unmount /Volumes/SNES

crc:
	python ../../scripts/crc_xmodem.py $(APP)


StartupSnes.obj: StartupSnes.asm
		$(AS) -V $?

ressource.obj: ressource.asm
		$(AS) -V $?

%.obj: %.c
		$(CC)  -wl -wp -sop -MC -I $(INC) $?

$(APP): $(OBJS)
		$(LD) -HB -M21 -V -T -Pff \
                -C008000,0000 -U0000,0000 \
                -Avectors=FFE4,7FE4 \
                -Aregistration_data=FFB0,7FB0 \
                -Aressource=18000,8000 \
                -N $(OBJS) -L$(LIBS) -O $@
		ucon64 -snes -chk $(APP)

clean:
		rm -vf $(APP) *.obj
