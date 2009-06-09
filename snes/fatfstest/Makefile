# SDK Config


PLATFORM=$(shell uname)

ifeq ($(PLATFORM),Linux)
    # Linux Wine 
    SDK=/home/david/.wine/drive_c/65xx_FreeSDK
    WINE=wine
    EMU=../../tools/bsnes/bsnes
    DISASM=/home/david/Devel/arch/snes/devkit/bin/disasm
    UCON=ucon64
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


OBJS=hook.obj StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj diskio.obj ff.obj crc.obj
APP=fatfs.smc
GFX=debugfont

all:  $(APP) repair

run:
	$(EMU) $(APP)

upload:
	ucon64 -chk $(APP)
	cp -rv $(APP) /Volumes/SNES
	sync
	diskutil unmount /Volumes/SNES

disasm: $(APP)
	rm -rf  fatfs.asm
	$(DISASM) fatfs.smc >  fatfs.asm 

repair: $(APP)
	$(UCON) -snes -chk $(APP) 2>&1 >/dev/null
	rm -rf fatfs.bak

StartupSnes.obj: StartupSnes.asm
	$(AS) -V $?

ressource.obj: ressource.asm
	$(AS) -V $?

hook.obj: hook.asm
	$(AS) -V $?

%.obj: %.c
	$(CC)  -wl -wp -sop -ML -I $(INC) $?

$(APP): $(OBJS) 
	$(LD) -B -HB -M21 -V -T -Pff \
        -C3e8000,1f0000 -U000000,000000 \
        -Avectors=FFE4,7FE4 \
        -Aregistration_data=FFB0,7FB0 \
        -Aressource=18000,8000 \
        -Ahook=008000,0000 \
        -N $(OBJS) $(LIBS) -O $@

clean:
	rm -vf $(APP) *.obj *.TMP
