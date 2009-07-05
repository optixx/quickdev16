import os
import sys

def main():

    bank_size = 2 ** 15
    
    rom = open(sys.argv[1],"r").read()

    out = open(sys.argv[1].replace(".smc",".inject"),"r+")

    out.write(rom)
    
    out.seek(0x2140)
    for i in range(0,128):
        out.write(chr(i))
    out.close()


if __name__ == '__main__':
    main()

