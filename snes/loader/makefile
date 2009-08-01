# 65816 stuff
AC = wla-65816
AFLAGS = -o
LD = wlalink
LDFLAGS = -vsr
FL = snesflash
FLFLAGS = -wf
UCON = ucon64
UCONFLAGS = --port=usb --xsnesram
SFILES  = main.asm
OFILES  = $(SFILES:.asm=.o)
ROMFILE = main.smc
EMU = zsnes
# spc stuff
SPCAC = wla-spc700
SPCSFILES  = data/apu/apucode.asm
SPCOFILES  = $(SPCSFILES:.asm=.o)
SPCFILE = $(SPCSFILES:.asm=.bin)


all: spc $(OFILES) 
	$(LD) $(LDFLAGS) linkfile $(ROMFILE)
	python conv_rle.py 

$(OFILES): $(SFILES)
	$(AC) $(AFLAGS) $(SFILES)

spc: $(SPCOFILES)
	$(LD) -vsb spclinkfile $(SPCFILE)

$(SPCOFILES): $(SPCSFILES)
	$(SPCAC) $(AFLAGS) $(SPCSFILES)

header:
	$(UCON) -smc $(ROMFILE)

flash:
	$(FL) $(FLFLAGS) $(ROMFILE)

upload: header
	$(UCON) $(UCONFLAGS) $(ROMFILE)
run:
	$(EMU) $(ROMFILE)

clean:
	rm -f $(ROMFILE) rom.smc $(SPCFILE) core *~ *.o *.sym *.srm data/apu/*.o data/apu/*.sym loader.c loader.h


