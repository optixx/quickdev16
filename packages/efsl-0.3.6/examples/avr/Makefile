include ../../conf/config.makefile
#BASE=c:\efsl\efsl-0.3
#INC=-I$(BASE)\src\base\include -I $(BASE)\src\include -I $(BASE)\src\fs\vfat\include -I $(BASE)\src\hwdrivers\atmega_spi\include -I $(BASE)\src\protocols\sdcard_spi\include  -I$(BASE)\conf
BASE=../../
INC=-I$(BASE)/src/base/include -I $(BASE)/src/include -I $(BASE)/src/fs/vfat/include -I $(BASE)/src/hwdrivers/atmega_spi/include -I $(BASE)/src/protocols/sdcard_spi/include  -I$(BASE)/conf
LIBS= -L $(BASE)\lib -lefsl-base -lefsl-fs-vfat -lefsl-hwd-atmega_spi -lefsl-prot-sdspi

avrtest: lib
	$(CC) $(GCFLAGS) $(INC) -o avrtest.o avrtest.c $(LIBS)
	$(OBJCOPY) -j .text -j .data -O ihex avrtest.o avrtest.hex

flash: avrtest
	avrdude -P /dev/ttyS0 -c stk500v2 -pm128 -Uflash:w:avrtest.hex

debug: avrtest
	avarice -j /dev/ttyS0 -e -p -f avrtest.o -D :4242
	avr-gdb avrtest.o

lib:
	make -C $(BASE) avr

clean:
	make -C $(BASE) clean
	rm -rf *.hex *.o
