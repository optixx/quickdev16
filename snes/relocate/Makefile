#wla-65816 -o %1.asm %1.obj
#wlalink -vr temp.prj %1.smc


AS=wla-65816
LD=wlalink

OBJS=relocate.o
APP=relocate.smc
GFX=

all: clean $(APP)

run:
	../../tools/bsnes/bsnes  $(APP)

linkfile:
	echo "[objects]" > linkerfile.prj


optixx.inc: optixx.pcx
	wine tools/pcx2snes.exe optixx.pcx -b2 -nOptixx -ooptixx.inc


%.o: %.asm
	echo "$@" >> linkerfile.prj
	$(AS) -o $?  $@

$(APP):  linkfile $(GFX) $(OBJS) $(GFX)
	$(LD) -vri linkerfile.prj  $@

clean:
	rm -vf $(APP) *.prj *.o
