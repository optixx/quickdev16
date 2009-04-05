import string
import math
import sys

M_PI = 3.14159265358979323846



def writefile(fp,val):
	global out,el
	if not len(out):
		out = "	.db "

	el+=1
	out += "$%02x," % val
	if el == cnt/8 :
		out=out[:-1]
		out+="\n"
		fp.write(out)
		out =""
		el=0


def sine(val,r,scale,stepping):
	global M_PI,flip
	re = int(math.sin(val*(M_PI*scale)/r)*r) + r
	re = re & 0xff

	if flip and val%2:
		re = (r*2) - re

	re = re * stepping
	#print "sine %s -> %s " % (val,re)
	return re


def main():
	global cnt,flip
	asmfile = sys.argv[1]
	basename = string.replace(asmfile,".s","")
	cnt = int(sys.argv[2])
	upper = int(sys.argv[3])

	
	if len(sys.argv) >= 5:
		stepping = int(sys.argv[4])
	else:
		stepping = 1


	if len(sys.argv) >= 6 and sys.argv[5]=='flip':
		flip = 1
	else:
		flip = 0


	half = int(cnt) / 2

	if cnt%8:
		print "ctn should be modulo 8"
		sys.exit(1) 

	fp = open(asmfile,'w')

	out = "\n\n\n%s:\n\n" % (basename)
	fp.write(out)
	out = ""

	for i in range(0,cnt):
		writefile(fp,sine(i,upper/2,(float(upper)/cnt),stepping))

	


out = str()
el = 0

if __name__ == '__main__':
	if len(sys.argv) >= 4:
        	main()
	else:
        	print "usage: %s  filename cnt(int) upper(int)  [stepping (int)] ['flip']" % sys.argv[0]








