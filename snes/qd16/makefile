# 65816 stuff
ROMFILE = qd16boot.smc
VPATH = routines/
AC = wla-65816
AFLAGS = -o
LD = wlalink
LDFLAGS = -vdsr
FL = snesflash
FLFLAGS = -wf
sources  := $(wildcard routines/*.asm)
objects  := $(patsubst %.asm,%.o,$(sources))
linkfile := linkobjs.lst



all: $(objects)
	$(LD) $(LDFLAGS) $(linkfile) $(ROMFILE)
	
$(objects): $(sources)
	$(AC) $(AFLAGS) $(patsubst %.o,%.asm,$@) $@

flash:
	$(FL) $(FLFLAGS) $(ROMFILE)

clean:
	rm -f $(ROMFILE) $(objects) core *~ *.sym *.srm
