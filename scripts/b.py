#!/usr/bin/python
import sys
bits = 32

try:
    if 'x' in sys.argv[1] or 'X' in sys.argv[1]:
        v = int(sys.argv[1], 16)
    else:
        v = int(sys.argv[1])
except:
    print "%s NUM" % sys.argv[0]
    sys.exit(-1)

sys.stdout.write("0b")
for i in range(bits - 1, -1, -1):
    s = 1 << i
    if v & s:
        sys.stdout.write("1")
    else:
        sys.stdout.write("0")
    if i and not i % 8:
        sys.stdout.write(" ")

print
print "0x%x" % v
print v
