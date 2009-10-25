import binascii
import os
import sys
import time
import shutil

LEN = 2**16
huffman = False
TARGET=os.getcwd()
SOURCE=sys.argv[1]
DEFLATE=os.path.basename(sys.argv[1]) + ".deflate"
PATH="/Users/david/Devel/arch/avr/code/quickdev16/scripts"

WINE="/Applications/Darwine/Wine.bundle/Contents/bin/wine"
KZIP=os.path.join(PATH,"kzip.exe")
DEFLOPT=os.path.join(PATH,"DeflOpt.exe")
ZIP2RAW=os.path.join(PATH,"zip2raw.rb")

method  = 1

if method==0:
    if os.path.isfile("rom.zip"):
        os.unlink("rom.zip")

    os.system("%s %s rom /s1 %s" % (WINE,KZIP,SOURCE))
    os.system("%s %s /a rom.zip" % (WINE,DEFLOPT))
    os.system("ruby %s rom.zip" % ZIP2RAW)

    if os.path.isfile("rom.zip"):
        os.unlink("rom.zip")
    data = open(DEFLATE).read()
    os.unlink(DEFLATE)
else:
    os.system("gzip < %s >/tmp/test" % SOURCE )
    data = open("/tmp/test").read()
    os.unlink("/tmp/test")
    data = data[10:]

zip_size = len(data)
cfile = open("/tmp/loader_test.c","w")
hfile = open("/tmp/loader_test.h","w")


hfile.write('''
#ifndef __FIFO_H__
#define __FIFO_H__

#define ROM_ZIP_SIZE     %i
''' % zip_size)
hfile.write('\n#endif\n')
hfile.close()

cfile.write('''/*
File: %s 
Time: %s
*/
''')


cfile.write('''
const char _rom[%i] = {
''' % (zip_size))
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

shutil.copy("/tmp/loader_test.c", os.path.join(TARGET,"loader_test.c"))
shutil.copy("/tmp/loader_test.h", os.path.join(TARGET,"loader_test.h"))
