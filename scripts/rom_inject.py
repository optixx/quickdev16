# -*- coding: utf-8 -*-
import os
import sys
import struct

def usage():
    print "%s <rom> <code> <addr>" % sys.argv[0]
    sys.exit(0)


def error(msg):
    print "ERROR: %s" % msg
    sys.exit(-1)

def main():



    if len(sys.argv) != 4:
        usage()
    if not os.path.isfile(sys.argv[1]):
        error("Can't open %s file" % sys.argv[1])
    if not os.path.isfile(sys.argv[2]):
        error("Can't open %s file" % sys.argv[2])
      
    rom = open(sys.argv[1],"r").read()
    code = open(sys.argv[2],"r").read()
    out_filename = sys.argv[1].split(".")[0] + ".inject"
    out = open(out_filename,"w")
    
    irq_vector = 0x7FE0
    bank_size = 1 << 15
    code_len = len(code)
    rom_len = len(rom)
    bank_cnt = rom_len / bank_size
    try:
      addr = int(sys.argv[3],16)
    except:
        error("Expect %s in hex" % sys.argv[3])

    addr_patch = struct.pack(">H",addr)


    if  addr >  bank_size:
        error("Addr 0x%04x is not within first bank" % addr)

    if  addr + code_len >  ( bank_size - 256 ):
        error("Code is %s bytes, and so too big to fit from 0x%04x into first bank" % (code_len,addr))

    print "Rom size:        0x%08x (%i)" % (rom_len,rom_len)
    print "Code size:       0x%04x (%i)" % (code_len,code_len)
    print "Banks:           0x%04x (%i)" % (bank_cnt,bank_cnt)
    print "Patch addr:      0x%08x"  %  addr
    print "IRQ addr:        0x%08x"  %  irq_vector


    out.write(rom[:addr])
    out.write(code)
    out.write(rom[addr+code_len:irq_vector])
    out.write(addr_patch)
    out.write(rom[irq_vector+2:])
    print "%i Bytes written to %s" % (out.tell(),out_filename)
    out.close()
    print "Apply checksum fix"
    cmd = "ucon64 -snes -chk %s 2>&1 > /dev/null" % out_filename
    os.system(cmd)


if __name__ == '__main__':
    main()

