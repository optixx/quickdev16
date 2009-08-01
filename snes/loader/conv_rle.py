import binascii
data = open("rom.smc","r").read()
data = binascii.rlecode_hqx(data)

cfile = open("loader.c","w")
hfile = open("loader.h","w")

hfile.write('''
#ifndef __FIFO_H__
#define __FIFO_H__

#define ROM_SIZE %i

#endif
''' % len(data))

cfile.write('''

#include <avr/pgmspace.h>       
#include <loader.h>       

const char _rom[ROM_SIZE] PROGMEM = {
''')

for idx,c in enumerate(data):
    c = ord(c)
    if idx<len(data)-1:
        cfile.write("0x%02x," % c)
    else:
        cfile.write("0x%02x" % c)
    if idx and idx%16==0:
        cfile.write("\n")
cfile.write('''
};
''')
cfile.close()