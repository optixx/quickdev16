#wla-65816 -o %1.asm %1.obj
#wlalink -vr temp.prj %1.smc


AS=wla-65816
LD=wlalink

OBJS=irqtest.o
APP=irqtest.smc
GFX=

all: clean $(APP)

run:
	zsnes $(APP)

linkfile:
	echo "[objects]" > linkerfile.prj


optixx.inc: optixx.pcx
	wine tools/pcx2snes.exe optixx.pcx -b2 -nOptixx -ooptixx.inc


%.o: %.asm
	echo "$@" >> linkerfile.prj
	$(AS) -o $?  $@

$(APP):  linkfile $(GFX) $(OBJS) $(GFX)
	$(LD) -vr linkerfile.prj  $@

clean:
	rm -vf $(APP) *.prj *.o
