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


<<<<<<< HEAD:snes/fatfstest/Makefile
OBJS=StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj diskio.obj ff.obj
=======
OBJS=hook.obj StartupSnes.obj main.obj pad.obj PPU.obj debug.obj ressource.obj diskio.obj ff.obj crc.obj
>>>>>>> c1203c3519d95a92e82a6f2ebf741ad663e46e33:snes/fatfstest/Makefile
APP=fatfs.smc
GFX=debugfont

all:  $(APP) repair 

run:  
	$(EMU) $(APP)

debugger:  
	$(EMU_DEBUG) $(APP)

upload:
	ucon64 -chk $(APP)
	cp -rv $(APP) /Volumes/SNES
	sync
	diskutil unmount /Volumes/SNES

disasm: $(APP)
	rm -rf  fatfs.asm
	$(DISASM) fatfs.smc >  fatfs.asm 

StartupSnes.obj: StartupSnes.asm
	$(AS) -V $?

ressource.obj: ressource.asm
	$(AS) -V $?

hook.obj: hook.asm
	$(AS) -V $?

%.obj: %.c
<<<<<<< HEAD:snes/fatfstest/Makefile
	$(CC)  -wl -wp -sop -ML -I $(INC) $?

#$(APP): $(OBJS)
#		$(LD) -HB -M21 -V -T -Pff \
#                -C008000,0000 -U0000,0000 \
#                -Avectors=FFE4,7FE4 \
#                -Aregistration_data=FFB0,7FB0 \
#		 -Aressource=18000,8000 \
#                -N $(OBJS) $(LIBS) -O $@

$(APP): $(OBJS) hook.obj
	$(LD) -B -HB -M21 -V -T -Pff \
        -C3e8000,1f0000 -U000000,000000 \
        -Avectors=FFE4,7FE4 \
        -Aregistration_data=FFB0,7FB0 \
        -Aressource=3f8000,1f8000 \
	-Ahook=008000,0000 \
        -N $(OBJS) hook.obj $(LIBS) -O $@


repair: $(APP)
	$(UCON) -snes -chk $(APP) 2>&1 >/dev/null
	rm -rf fatfs.bak
=======
		$(CC)  -wl -wp -sop -ML -I $(INC) $?

$(APP): $(OBJS)
		$(LD) -HB -M21 -V -T -Pff \
                -C3e8000,1f0000 -U000000,000000 \
                -Avectors=FFE4,7FE4 \
                -Aregistration_data=FFB0,7FB0 \
                -Aressource=18000,8000 \
                -Ahook=8000,0000 \
                -N $(OBJS) $(LIBS) -O $@
		ucon64 -snes -chk $(APP) 2>&1 >/dev/null
>>>>>>> c1203c3519d95a92e82a6f2ebf741ad663e46e33:snes/fatfstest/Makefile



clean:
		rm -vf $(APP) *.obj *.TMP
