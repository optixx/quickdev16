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


all: 
	mv -v  main.smc main.smc.last
	wget http://dforce3000.de/main.smc 
	python ../../scripts/conv_rle.py main.smc
