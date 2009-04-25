# SDK Config

# Linux Wine 
SDK=/home/david/.wine/drive_c/65xx_FreeSDK
WINE=wine
EMU=zsnes
EMU_DEBUG=/home/david/Devel/arch/snes/tools/zsnes_linux_debug151/src/zsnesd -d 


# OSX Crossover Office
SDK=/Users/david/.wine/drive_c/65xx_FreeSDK
CC=/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wineloader winewrapper.exe --wait-children --start -- C:/65xx_FreeSDK/bin/WDC816CC.exe
AS=/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wineloader winewrapper.exe --wait-children --start -- C:/65xx_FreeSDK/bin/WDC816AS.exe
LD=/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wineloader winewrapper.exe --wait-children --start -- C:/65xx_FreeSDK/bin/WDCLN.exe
EMU=zsnes

# Mac Wine 
SDK=/Users/david/.wine/drive_c/65xx_FreeSDK
WINE=wine
EMU=zsnes
CC=$(WINE) $(SDK)/bin/WDC816CC.exe
AS=$(WINE) $(SDK)/bin/WDC816AS.exe
LD=$(WINE) $(SDK)/bin/WDCLN.exe





PADBIN=$(WINE) tools/padbin.exe
PCX2SNES=$(WINE) tools/Pcx2Snes.exe

# Project
INC=$(SDK)/include
LIBS=$(SDK)/lib/cs
#OBJS=StartupSnes.obj main.obj pad.obj event.obj myEvents.obj PPU.obj debug.obj ressource.obj
OBJS=StartupSnes.obj main.obj pad.obj  PPU.obj debug.obj ressource.obj crc.obj
APP=crc.smc
GFX=kungfu debugfont

all:  $(APP)

run:  
	$(EMU) $(APP)

debugger:  
	$(EMU_DEBUG) $(APP)

upload:
	ucon64 $(APP)
	cp -rv $(APP) /Volumes/SNES
	sync
	diskutil unmount /Volumes/SNES


kungfu:
		$(PCX2SNES) ressource/kungfu -n -c16 -screen

debugfont:
		$(PCX2SNES) ressource/debugFont -n -c16 -s8 -o1

%.obj: %.asm
		$(AS) -V $?

%.obj: %.c
		$(CC) -wl -wp -sop -MS -I $(INC) $?

$(APP): $(OBJS)
		$(LD) -HB -M21 -V -T -Pff \
                -C008000,0000 -U0000,0000 \
                -Avectors=FFE4,7FE4 \
                -Aregistration_data=FFB0,7FB0 \
                -Aressource=18000,8000 \
                -N $(OBJS) -L$(LIBS) -O $@
		$(PADBIN) 0x40000 $(APP)

clean:
		rm -vf $(APP) *.obj
