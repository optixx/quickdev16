SHELL = sh
OPTIMIZE   = -O2
CC         = gcc
CSTD       = c99
override CFLAGS	=   -pedantic -std=$(CSTD) -Wall -Wstrict-prototypes $(OPTIMIZE) 

all: huffman-decode huffman-encode 

clean:
	rm -f huffman-decode huffman-encode bin2avrS
