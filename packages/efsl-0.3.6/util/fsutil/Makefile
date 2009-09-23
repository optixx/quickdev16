INC_BASE=../../src/base/include
INC_GLOBAL=../../src/include
INC_CONF=../../conf
INC_VFAT=../../src/fs/vfat/include
INC_LFI =../../src/hwdrivers/linuxfile/include
INCLUDE_DIRS=-I$(INC_GLOBAL) -I$(INC_BASE) -I$(INC_CONF) -I$(INC_VFAT) -I$(INC_LFI)
LIB_DIRS=-L../../lib
LIB_TARGETS=-lefsl-base -lefsl-fs-vfat -lefsl-hwd-linuxfile
CFLAGS=-g3 -ansi -Wall -O0 $(INCLUDE_DIRS)
LFLAGS= $(LIB_DIRS) $(LIB_TARGETS)
CC=gcc

all : cpi cpo cpa mkdir list

init : init.o
	gcc -c -o init.o $(CFLAGS) init.c

cp: init cp.o
	gcc -o cp $(CFLAGS) cp.o init.o $(LFLAGS)

cpi : init cpi.o 
	gcc -o cpi $(CFLAGS) cpi.o init.o $(LFLAGS)

cpo : init cpo.o
	gcc -o cpo $(CFLAGS) cpo.o init.o $(LFLAGS)

cpa : init cpa.o
	gcc -o cpa $(CFLAGS) cpa.o init.o $(LFLAGS)

mkdir : init mkdir.o
	gcc -o mkdir $(CFLAGS) mkdir.o init.o $(LFLAGS)

list : init list.o
	gcc -o list $(CFLAGS) list.o init.o $(LFLAGS)

clean :
	rm -f cpi cpi.o cpo cpo.o cpa.o cpa mkdir.o mkdir list.o list

