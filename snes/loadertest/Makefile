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
    DISASM=/Users/david/Devel/arch/snes/devkit/bin/disasm
    UCON=ucon64
endif

CC=$(WINE) $(SDK)/bin/WDC816CC.exe
AS=$(WINE) $(SDK)/bin/WDC816AS.exe
LD=$(WINE) $(SDK)/bin/WDCLN.exe
PADBIN=$(WINE)  tools/padbin.exe

# Project

INC=$(SDK)/include
LIBS=-L$(SDK)/lib/cc 
#-L$(SDK)/lib/c134


OBJS=StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj
APP=loadertest.smc
GFX=debugfont

all:  $(APP) 

run:
	$(EMU) $(APP)


disasm: $(APP)
	rm -rf  $(APP)
	$(DISASM) $(APP) >  $(APP).asm

upload: header
	ucon64 --port=usb --xsnesram $(APP)

repair: $(APP)
	$(UCON) -snes -chk $(APP) 2>&1 >/dev/null
	rm -rf *.bak

header: $(APP)
	$(UCON) -smc  $(APP) 
	rm -rf *.bak

StartupSnes.obj: StartupSnes.asm
	$(AS) -V $?

ressource.obj: ressource.asm
	$(AS) -V $?

%.obj: %.c
	$(CC)  -wl -wp -sop -MC -I $(INC) $?

$(APP): $(OBJS) 
	$(LD) -B -HB -M21 -V -T -Pff \
        -C008000,0000 -U0000,0000 \
        -Avectors=FFE4,7FE4 \
        -Aregistration_data=FFB0,7FB0 \
        -Aressource=18000,8000 \
        -N $(OBJS) $(LIBS) -O $@
	$(PADBIN) 0x20000 $(APP)
clean:
	rm -vf $(APP) *.obj *.TMP
