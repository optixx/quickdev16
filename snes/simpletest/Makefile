#wla-65816 -o %1.asm %1.obj
#wlalink -vr temp.prj %1.smc


AS=wla-65816
LD=wlalink

OBJS=sprite.o
APP=sprite.smc
GFX=biker.pic
EMU=/Applications/ZSNES.app/Contents/MacOS/ZSNES
DEBUG=wine /Users/david/Devel/arch/snes/tools/snes9x1/Snes9X1.43.ep9r8.exe
all: clean $(APP) raw

run:  
	$(EMU) $(APP)

debug:
	$(DEBUG) $(APP)


raw: $(APP)
    dd if=$(APP) of=raw bs=512 skip=1
    
linkfile:
	echo "[objects]" > linkerfile.prj

%.o: %.asm
	echo "$@" >> linkerfile.prj
	$(AS) -o $?  $@

$(APP):  linkfile $(GFX) $(OBJS) $(GFX)
	$(LD) -vr linkerfile.prj  $@
	ucon64 -chk $(APP)      
clean:
	rm -vf $(SWC) $(APP) *.prj *.o
