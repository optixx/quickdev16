
import ctypes
import sys
import os

def crc_xmodem_update(crc,data):
    crc =  ctypes.c_uint16(crc.value ^ data.value << 8)
    for i in range(0,8):
        if crc.value & 0x8000:
            crc = ctypes.c_uint16((crc.value << 1) ^ 0x1021);
        else:
            crc = ctypes.c_uint16(crc.value << 1);
    return crc


def do_crc(data):
    crc = ctypes.c_uint16(0)
    for idx,char in enumerate(data):
        crc = crc_xmodem_update(crc,ctypes.c_uint8(ord(char)))
    return crc.value


def test_performance():
    data=str()
    fd = open("/dev/urandom")
    for i in range(0,256):
        data+= fd.read(1024)
        sys.stdout.write("*")
        sys.stdout.flush()
    print
    fd.close()    
    print "%s" % do_crc(data)


def test_algo():
    data='david'
    data='d'
    print "%x" % do_crc(data)
  

def main():
    #import cProfile
    #cProfile.run('test_performance()')

    size = os.stat(sys.argv[1])[6]
    fd = open(sys.argv[1])
    addr = 0x0000
    step = 2**15
    result = []
    while addr < size:
        try:
            block = fd.read(step)
            addr += step 
        except:
            print "Done"
            break
        crc = do_crc(block)
        print "Bank: 0x%02x Addr: 0x%06x Block: 0x%04x CRC 0x%04x" % (addr/(2**15),addr,addr/512, ctypes.c_uint16(crc).value)
        result.append(hex(ctypes.c_uint16(crc).value))
    #print result

if __name__ == '__main__':
    test_algo()
    main()


