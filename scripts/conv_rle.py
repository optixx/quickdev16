import binascii
import os
import sys
import time


LEN = 2**16

TARGET="/Users/david/Devel/arch/avr/code/quickdev16/avr/usbload"
data = open(sys.argv[1],"r").read()

print "Load %s (%i) bytes" % (sys.argv[1],len(data))
data = data[:LEN]
print "Use (%i) bytes" % (len(data))
data = binascii.rlecode_hqx(data)
print "RLE crunch (%i) bytes" % (len(data))
binfile = open("/tmp/loader.rle","w")
binfile.write(data)
binfile.close()
cfile = open("/tmp/loader.c","w")
hfile = open("/tmp/loader.h","w")

hfile.write('''
#ifndef __FIFO_H__
#define __FIFO_H__

#define ROM_SIZE %i

#endif
''' % len(data))

cfile.write('''/*
File: %s 
Time: %s
*/
#include <avr/pgmspace.h>       
#include <loader.h>       

const char _rom[ROM_SIZE] PROGMEM = {
''' % (sys.argv[1],time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())))

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

os.rename("/tmp/loader.c", os.path.join(TARGET,"loader.c"))
os.rename("/tmp/loader.h", os.path.join(TARGET,"loader.h"))

