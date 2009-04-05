import os
import re
import string
import sys
import binascii
import math



class color:
	def __init__(self,r,g,b):
		self.r = r >> 3
		self.g = g >> 3
		self.b = b >> 3
		self.snes = "$%04x" % (( self.b << 10 )    | (self.g << 5)   |   self.r )

	def get(self):
		return self.snes


	def __str__(self):
		return  "r=0x%02x g=0x%02x b=0x%02x  snes=%s" % ( self.r,self.g,self.b,self.snes)





def openfile(filename):
    	return open(filename)


def mk_sint(s):
	a = ord(s[1])
	b = ord(s[0])
	return (a<<8) + b


def mk_int(s):
	a = ord(s[3])
	b = ord(s[2])
	c = ord(s[1])
	d = ord(s[0])
	return (a<<32) + (b<<16) + (c<<8) + d


def main():
	file = sys.argv[1]
	asmfile = string.replace(file,".bmp",".s")
	basename = string.replace(file,".bmp","")
	

	if len(sys.argv) >= 3 and sys.argv[2] =='list' :
		do_list = 1
	else:
		do_list = 0

	if len(sys.argv) >= 4 and sys.argv[3] =='compress':
		do_compress = 1
	else:
		do_compress = 0


	fp =openfile(file)

	header_offset = 14 + 40
	data = fp.read()
	fp.close()

	type = mk_sint(data[0:2])
	size = mk_int(data[2:6])
	width = mk_int(data[18:22])
	height = mk_int(data[22:26])
	bits_per_pixel = mk_int(data[28:32])
	num_of_colors = mk_int(data[46:50])

	bytes_per_line = width / 8
	bytes_per_line_padded = bytes_per_line + (4-(bytes_per_line % 4))


	header = data[0:header_offset]
	colors = data[header_offset: header_offset + (num_of_colors  * 4)]
	data =   data[header_offset + (num_of_colors  * 4):]

	raw = []
	for i in data:
		raw.append(ord(i))
	raw_len = len(raw)

	palette = []

	for i in range(0,len(colors),4):
		palette.append(color(ord(colors[i+2]),ord(colors[i+1]),ord(colors[i])))





	print "file:\t\t%s" % file
	print "basename:\t%s" % asmfile
	print "header info"
	print "type: \t\t%04X " % type
	print "size: \t\t%i bytes" % size
	print "width: \t\t%i pixel" % width
	print "height: \t%i pixel" % height
	print "bit per pixel:\t%i" % bits_per_pixel
	print "num of colors:\t%i" % num_of_colors
 	print "imagedata: \t%s bytes" % raw_len
	print "per line: \t%i bytes" %  bytes_per_line
	print "per line pad: \t%i bytes" %  bytes_per_line_padded







 	out='';

	fp = open(asmfile,'w')
	tile_cnt = 0
	
	
	
	color_list  = str()
	value_list = str()


	color_list="\n\n\n%s_color_list:\n\n" % (basename)
	value_list ="\n\n\n%s_color_values:\n\n" % (basename)
	out = "	.db "
	cnt=0
	last = "";
	repeat = 1;
	for i in range(raw_len-1,-1,-width):

		idx = raw[i]
		col = palette[idx].get()
	
		if col == last and do_compress:
			repeat += 1
			continue
		else:
			#print palette[idx]
			last = col
			if do_list:
				value_list  += "	.db $%02x\n" % repeat
			value_list  += "	.dw %s 	;line=0x%02x\n" % (col, (height - (i/width)) )
			out += "$%02x,$00," % repeat
			repeat = 1
			cnt +=1
			if cnt == width/20:
				cnt=0
				out = out[:-1]
				out +="\n"
				color_list += out
				out = "	.db "


	value_list  += "	.db 0\n"
	color_list  += "	.db 0\n"

	
	if do_list:
		fp.write(color_list)
	fp.write(value_list)




	fp.close()

if __name__ == '__main__':
	if len(sys.argv)>= 2:
        	main()
	else:
        	print "usage: %s  in [list] [compress]" % sys.argv[0]




