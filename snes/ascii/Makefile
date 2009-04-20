
CC = wla-65816
CFLAGS = -o
LD = wlalink
LDFLAGS = -v 

SFILES = main.asm
IFILES = 
OFILES = main.o
GFXDATA = optixx_logo.bmp

all: $(OFILES) Makefile  
	$(LD) $(LDFLAGS) linkfile ascii.smc
	ucon64 -chk -swc ascii.smc

zsnes: 
	/Applications/ZSNES.app/Contents/MacOS/ZSNES ascii.swc
bsnes:
	open /Applications/BSNES.app  ascii.swc

main.o: colorlist sine gfx main.asm
	$(CC) $(CFLAGS) main.asm main.o

gfx: bmp2bitplane.py optixx_logo.bmp 	
	python bmp2bitplane.py optixx_logo.bmp

sine: sinegen.py
	python sinegen.py vsine_1.s 256 96 1
	python sinegen.py vsine_2.s 256 96 1 flip
	python sinegen.py colbarsine_1.s 256 64 3


colorlist:	bmp2col.py
	python bmp2col.py	backcolors.bmp list
	python bmp2col.py	colbar_1.bmp 
	python bmp2col.py	colbar_2.bmp 
	python bmp2col.py	colbar_3.bmp 
		
	

$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ result.rom optixx_logo.s 
