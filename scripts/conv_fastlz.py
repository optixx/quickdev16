import os
import sys
import time
import shutil

LEN = 2 ** 16
huffman = False
TARGET = os.getcwd()
SOURCE = sys.argv[1]
NAME = os.path.basename(sys.argv[1])
COMPRESSED = NAME + ".fastlz"
LOC = os.path.dirname(os.path.realpath(__file__))
FASTLZ = os.path.join(LOC, "..", "tools", "fastlz", "pack")

os.system("%s %s %s" % (FASTLZ, SOURCE, COMPRESSED))
data = open(COMPRESSED).read()
os.unlink(COMPRESSED)

fastlz_size = len(data)
cfile = open("/tmp/loader.c", "w")
hfile = open("/tmp/loader.h", "w")

parts = []
cnt = len(data) / ((2 ** 15) - 1)
r = len(data) - (cnt * ((2 ** 15) - 1))
for i in range(0, cnt):
    parts.append(((2 ** 15) - 1))
parts.append(r)

hfile.write('''/*
File: %s
Time: %s
*/
#ifndef __FIFO_H__
#define __FIFO_H__

#define LOADER_NAME "%s"
#define LOADER_COMPRESS "FASTLZ"
#define ROM_FASTLZ_SIZE     %i
#define ROM_BUFFER_CNT   %i

''' % (
    os.path.basename(SOURCE),
    time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime()),
    NAME,
    fastlz_size,
    len(parts)
))


for idx, val in enumerate(parts):
    hfile.write('#define ROM_BUFFER_SIZE%02i  %i\n' % (idx + 1, val))
hfile.write('\n#endif\n')
hfile.close()
cfile.write('''/*
File: %s
Time: %s
*/
#include <avr/pgmspace.h>
#include <loader.h>
''')
addr = 0
for idx, val in enumerate(parts):
    cfile.write('''
const char _rom%02i[ROM_BUFFER_SIZE%02i] PROGMEM = {
''' % (idx + 1, idx + 1))
    l = addr
    h = addr + parts[idx]
    addr += parts[idx]
    for idx, c in enumerate(data[l:h]):
        c = ord(c)
        if idx < len(data) - 1:
            cfile.write("0x%02x," % c)
        else:
            cfile.write("0x%02x" % c)
        if idx and idx % 16 == 0:
            cfile.write("\n")
    cfile.write('''
    };
''')
cfile.write('PGM_VOID_P _rom[ROM_BUFFER_CNT]= {')
for idx, val in enumerate(parts):
    if idx < len(parts) - 1:
        cfile.write('''&_rom%02i,''' % (idx + 1))
    else:
        cfile.write('''&_rom%02i''' % (idx + 1))
cfile.write('''};
''')

cfile.write('const int _rom_size[ROM_BUFFER_CNT] = {')
for idx, val in enumerate(parts):
    if idx < len(parts) - 1:
        cfile.write('''%i,''' % (val))
    else:
        cfile.write('''%i''' % (val))
cfile.write('''};
''')
cfile.close()
shutil.copy("/tmp/loader.c", os.path.join(TARGET, "loader.c"))
shutil.copy("/tmp/loader.h", os.path.join(TARGET, "loader.h"))
print "Copy loader.h and loader.c to %s" % TARGET
