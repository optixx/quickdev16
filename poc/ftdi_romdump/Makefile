all: romdump

romdump: dump.c
	gcc -c  dump.c  -I/Users/david/Devel/arch/ftdi/code/libftdi-0.7/src -I/opt/local/include 
	gcc -o romdump dump.o -L/Users/david/Devel/arch/ftdi/code/libftdi-0.7/src/.libs -lftdi -lc 

clean:
	rm -rf *.o romdump 
