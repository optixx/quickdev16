import binascii
data = open("rom.smc","r").read()
data = binascii.rlecode_hqx(data)

print '''
#include <avr/pgmspace.h>       
#define ROM_SIZE %i
const char _rom[ROM_SIZE] PROGMEM = { 
''' % len(data)
for idx,c in enumerate(data):
    c = ord(c)
    if idx<len(data)-1:
        print "0x%02x," % c,
    else:
        print "0x%02x" % c,
    if idx and idx%16==0:
        print
print '''
};
'''