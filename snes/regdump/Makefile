# SDK Config


PLATFORM=mac

ifeq ($(PLATFORM),linux)

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
PADBIN=$(WINE) tools/padbin.exe
PCX2SNES=$(WINE) tools/Pcx2Snes.exe



# Project

INC=$(SDK)/include
LIBS=$(SDK)/lib/cc
#OBJS=StartupSnes.obj main.obj pad.obj event.obj myEvents.obj PPU.obj debug.obj ressource.obj
OBJS=StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj 
APP=regdump.smc
GFX=kungfu debugfont

all:  $(APP) check 

run:  
	$(EMU) $(APP)

debugger:  
	$(EMU_DEBUG) $(APP)

check:	
	ucon64 -chk $(APP)


debugfont:
		$(PCX2SNES) ressource/debugFont -n -c16 -s8 -o1

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
                -Aressource=38000,8000 \
                -N $(OBJS) -L$(LIBS) -O $@
		$(PADBIN) 0x20000 $(APP)

clean:
		rm -vf $(APP) *.obj
