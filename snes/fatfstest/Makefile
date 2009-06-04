# SDK Config


PLATFORM=$(shell uname)

ifeq ($(PLATFORM),Linux)
    # Linux Wine 
    SDK=/home/david/.wine/drive_c/65xx_FreeSDK
    WINE=wine
    EMU=../../tools/bsnes/bsnes
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
LIBS=-L$(SDK)/lib/cl 
#-L$(SDK)/lib/c134


OBJS=hook.obj StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj diskio.obj ff.obj
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

hook.obj: hook.asm
		$(AS) -V $?



%.obj: %.c
		$(CC)  -wl -wp -sop -ML -I $(INC) $?

$(APP): $(OBJS)
		$(LD) -HB -M21 -V -T -Pff \
                -C008000,000000 -U000000,000000 \
                -Avectors=FFE4,7FE4 \
                -Aregistration_data=FFB0,7FB0 \
                -Aressource=3f8000,8000 \
                -N $(OBJS) $(LIBS) -O $@
		ucon64 -snes -chk $(APP) 2>&1 >/dev/null


                #-Ahook=008000,0000 \

clean:
		rm -vf $(APP) *.obj *.TMP
