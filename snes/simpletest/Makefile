#wla-65816 -o %1.asm %1.obj
#wlalink -vr temp.prj %1.smc


AS=wla-65816
LD=wlalink

OBJS=sprite.o
APP=sprite.smc
GFX=biker.pic
EMU=/Applications/ZSNES.app/Contents/MacOS/ZSNES

all: clean $(APP)

run:  
	$(EMU) $(APP)


linkfile:
	echo "[objects]" > linkerfile.prj

%.o: %.asm
	echo "$@" >> linkerfile.prj
	$(AS) -o $?  $@

$(APP):  linkfile $(GFX) $(OBJS) $(GFX)
	$(LD) -vr linkerfile.prj  $@

clean:
	rm -vf $(APP) *.prj *.o
